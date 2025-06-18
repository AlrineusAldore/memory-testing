#include "test_lazy_allocation.h"


int main() {
    const TestLazyAlloc tester(64);
    tester.run_all();

    return 0;
}
