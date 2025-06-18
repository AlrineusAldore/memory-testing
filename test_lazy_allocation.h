#pragma once

#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <locale>
#include <functional>


class TestLazyAlloc {
public:
    explicit TestLazyAlloc(int size_in_MB = 64);

    void run_all() const;

private:
    static constexpr size_t ONE_MB = 1024 * 1024;
    static constexpr size_t PAGE_SIZE = 4096;               // Page size (4 KB)
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    struct BenchmarkTimes {
        double init;
        double first_write;
        double second_write;

        [[nodiscard]] double total() const {
            return init + first_write;
        }
    };

    size_t size;

    /// example: 23458286 -> 23,458,286
    template<class T>
    static std::string format_with_commas(T value)
    {
        std::stringstream ss;
        ss.imbue(std::locale(""));
        ss << std::fixed << value;
        return ss.str();
    }

    /// prints formatted time result
    static double print_timed(const char* label, const TimePoint &start, const TimePoint &end);

    /// times a full write
    double timed_write(const char* label, char* buf) const;

    /// benchmarks init vs first write vs second write vs third write
    BenchmarkTimes benchmark_allocation(const char* alloc_name, const std::function<char *()>& alloc_func) const;
};
