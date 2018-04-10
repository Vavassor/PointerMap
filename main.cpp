#include "benchmark.cpp"
#include "clock.cpp"
#include "map.cpp"
#include "random.cpp"
#include "tests.cpp"

int main(int argc, char** argv)
{
    Heap heap = {};
    heap_create(&heap, 0x8000000);

#if 0
    FILE* file = fopen("report.txt", "w");
#else
    FILE* file = stdout;
#endif

    test_map(&heap, file);
    run_benchmark(&heap, file);

    if(file != stdout)
    {
        fclose(file);
    }

    heap_destroy(&heap);

    return 0;
}
