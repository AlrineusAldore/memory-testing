#include "test_lazy_allocation.h"


TestLazyAlloc::TestLazyAlloc(const int size_in_MB) : size(ONE_MB * size_in_MB) {}

void TestLazyAlloc::run_all() const {
    /// create all 3 ways to allocate

    std::function malloc_allocation = [this]() {
        return static_cast<char *>(malloc(size));
    };

    std::function calloc_allocation = [this]() {
        return static_cast<char *>(calloc(1, size));
    };

    std::function malloc_touch_allocation = [this]() {
        auto* buf = static_cast<char *>(malloc(size));
        // Touch one byte per page to pre-fault
        for (size_t i = 0; i < size; i += PAGE_SIZE) {
            buf[i] = 0;
        }

        return buf;
    };

    /// benchmark all 3 ways to allocate

    /// Lazy allocation - Doesnt reserve physical memory (only virtual) until actually used (first write/read)
    const BenchmarkTimes malloc_t = benchmark_allocation("malloc", malloc_allocation);

    /// Lazy allocation - In linux, the kernel already gives us zeroed pages, so it knows the calloc writing of zeroes is useless and skip it.
    /// When it skips the calloc writing, it skips the first writes therefore skips the pages mapping, making it not bypass lazy allocation.
    /// Note: When buffer size below threshold
    const BenchmarkTimes calloc_t = benchmark_allocation("calloc", calloc_allocation);
    const BenchmarkTimes touch_t =  benchmark_allocation("malloc + page touch", malloc_touch_allocation);
    /// init time here is slow as the page faults happen here in init instead of first write.
    /// first write here is fast and same as all writes

    std::cout << "=== TIME DIFFERENCES FROM LAZY ALLOCATION [[" << static_cast<double>(size) / ONE_MB << " MiB buffer]] ===" << std::endl;
    const double slowed_down_percent = (malloc_t.first_write / malloc_t.second_write - 1) * 100;
    const double speed_gain_percent = (1 - malloc_t.second_write / malloc_t.first_write) * 100;

    std::cout << "Lazy allocation slowed our first write by: " << slowed_down_percent << "%" << std::endl;
    std::cout << "Inversely, getting rid of lazy allocation would speed up our first write by: " << speed_gain_percent << "%" << std::endl;

    const double time_gained_by_touch = malloc_t.total() - touch_t.total();
    std::cout << "Saved " << time_gained_by_touch << " [us] total with [malloc + touch] (" << (time_gained_by_touch / malloc_t.total() * 100) << "% faster)\n" << std::endl;

    /// I don't know what causes this behaviour
    if (calloc_t.init < touch_t.init) {
        std::cout << "Calloc bypassed lazy allocation, like [malloc + touch]" << std::endl;
    }
    else {
        std::cout << "Calloc used lazy allocation, like [malloc]" << std::endl;
    }
}

double TestLazyAlloc::print_timed(const char* label, const TimePoint &start, const TimePoint &end) {
    using namespace std::chrono;
    std::cout << label << " time: "
              << format_with_commas(duration_cast<microseconds>(end - start).count())
              << " [us]\n";

    return duration<double, std::micro>(end - start).count();
}

double TestLazyAlloc::timed_write(const char* label, char* buf) const {
    const auto start = Clock::now();
    for (size_t i = 0; i < size; ++i) {
        buf[i] = static_cast<char>(i);
    }
    const auto end = Clock::now();

    return print_timed(label, start, end);
}

TestLazyAlloc::BenchmarkTimes TestLazyAlloc::benchmark_allocation(const char* alloc_name, const std::function<char *()>& alloc_func) const {
    std::cout << "=== "<< alloc_name << " ===\n";

    /// time the allocation
    const auto start = Clock::now();
    char* buf = alloc_func();
    const auto end = Clock::now();

    /// print and save the time to init
    const double init_t = print_timed("init", start, end);

    /// print and save the writes
    const double first_write_t = timed_write("first write", buf);
    const double second_write_t = timed_write("second write", buf);
    timed_write("third write", buf); // make sure it's equivalent to second write just in case - sanity check

    std::cout << std::endl;
    free(buf);

    return {init_t, first_write_t, second_write_t};
}