#include "clock.h"
#include "map.h"
#include "memory.h"
#include "random.h"

#include <cstdio>
#include <cinttypes>
#include <unordered_map>

typedef std::unordered_map<void*, void*, std::hash<void*>> hash_t;

// Benchmark Structure..........................................................

enum class Subject
{
    Map,
    Unordered_Map,
};

namespace
{
    const int table_counts_cap = 15;
    const int subjects_cap = 2;

    int table_counts[table_counts_cap] =
    {
        200000,
        400000,
        600000,
        800000,
        1000000,
        1200000,
        1400000,
        1600000,
        1800000,
        2000000,
        2200000,
        2400000,
        2600000,
        2800000,
        3000000,
    };

    Subject subjects[subjects_cap] =
    {
        Subject::Map,
        Subject::Unordered_Map,
    };
}

enum class BenchmarkType
{
    Deletion,
    Insertion,
    Iteration,
    Search,
    Search_Misses,
    Search_Half_Misses,
};

enum class TableType
{
    Random,
    Random_Both_Tables,
    Random_With_Reserve,
    Shuffle,
};

struct Benchmark
{
    s64 milliseconds[subjects_cap][table_counts_cap];
    BenchmarkType type;
    TableType table_type;
};

static const char* describe_benchmark_type(BenchmarkType type)
{
    switch(type)
    {
        default:
        case BenchmarkType::Deletion: return "Deletion";
        case BenchmarkType::Insertion: return "Insertion";
        case BenchmarkType::Iteration: return "Iteration";
        case BenchmarkType::Search: return "Search";
        case BenchmarkType::Search_Misses: return "Search Misses";
        case BenchmarkType::Search_Half_Misses: return "Search Half Misses";
    }
}

static const char* describe_subject(Subject subject)
{
    switch(subject)
    {
        default:
        case Subject::Map: return "Map";
        case Subject::Unordered_Map: return "Unordered Map";
    }
}

static const char* describe_table_type(TableType type)
{
    switch(type)
    {
        default:
        case TableType::Random: return "Random";
        case TableType::Random_Both_Tables: return "Random Both Tables";
        case TableType::Random_With_Reserve: return "Random With Reserve";
        case TableType::Shuffle: return "Shuffle";
    }
}

// Table Setup Utilities........................................................

static void fill_counting_upward(void** array, int count)
{
    for(int i = 0; i < count; i += 1)
    {
        array[i] = reinterpret_cast<void*>(i);
    }
}

static void fill_randomly(void** array, int count)
{
    Sequence sequence;
    const u64 a_prime = 1685777;
    seed(&sequence, a_prime);
    for(int i = 0; i < count; i += 1)
    {
        int j = generate(&sequence);
        array[i] = reinterpret_cast<void*>(j);
    }
}

static void shuffle(void** array, int count)
{
    Sequence sequence;
    const u64 a_prime = 1685777;
    seed(&sequence, a_prime);
    for(int i = 0; i < count; i += 1)
    {
        int j = random_int_range(&sequence, i, count - 1);
        void* temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// Helpers for Map..............................................................

static void delete_table(Map* map, void** table, int table_count)
{
    for(int i = 0; i < table_count; i += 1)
    {
        map_remove(map, table[i]);
    }
}

static void insert_table(Map* map, void** table, int table_count, Heap* heap)
{
    void* dummy = reinterpret_cast<void*>(1);
    for(int i = 0; i < table_count; i += 1)
    {
        map_add(map, table[i], dummy, heap);
    }
}

static void search_table(Map* map, void** table, int table_count)
{
    for(int i = 0; i < table_count; i += 1)
    {
        void* value;
        bool got = map_get(map, table[i], &value);
        if(got)
        {
            escape(value);
        }
    }
}

static void delete_random_half_and_shuffle(Map* map, void** table,
    int table_count)
{
    shuffle(table, table_count);

    int half = table_count / 2;
    for(int i = 0; i < half; i += 1)
    {
        map_remove(map, table[i]);
    }

    shuffle(table, table_count);
}

static void iterate_map(Map* map)
{
    ITERATE_MAP(it, map)
    {
        void* value = map_iterator_get_value(it);
        escape(value);
    }
}

// Helpers for std::unordered_map...............................................

static void delete_table(hash_t* map, void** table, int table_count)
{
    for(int i = 0; i < table_count; i += 1)
    {
        map->erase(table[i]);
    }
}

static void insert_table(hash_t* map, void** table, int table_count)
{
    void* dummy = reinterpret_cast<void*>(1);
    for(int i = 0; i < table_count; i += 1)
    {
        map->insert(hash_t::value_type(table[i], dummy));
    }
}

static void search_table(hash_t* map, void** table, int table_count)
{
    for(int i = 0; i < table_count; i += 1)
    {
        auto found = map->find(table[i]);
        if(found != map->end())
        {
            escape(found->second);
        }
    }
}

static void delete_random_half_and_shuffle(hash_t* map, void** table,
    int table_count)
{
    shuffle(table, table_count);

    int half = table_count / 2;
    for(int i = 0; i < half; i += 1)
    {
        map->erase(table[i]);
    }

    shuffle(table, table_count);
}

static void iterate_map(hash_t* map)
{
    for(auto it : *map)
    {
        escape(it.second);
    }
}

// The Actual Benchmark.........................................................

static void setup_tables(Benchmark* benchmark, void** table, void*** miss_table,
    int table_count, Subject subject, Map* map, hash_t* u_map, Heap* heap)
{
    switch(benchmark->table_type)
    {
        case TableType::Random:
        {
            fill_randomly(table, table_count);
            break;
        }
        case TableType::Random_Both_Tables:
        {
            fill_randomly(table, table_count);
            *miss_table = HEAP_ALLOCATE(heap, void*, table_count);
            fill_randomly(*miss_table, table_count);
            break;
        }
        case TableType::Random_With_Reserve:
        {
            fill_randomly(table, table_count);
            switch(subject)
            {
                case Subject::Map:
                {
                    map_reserve(map, table_count, heap);
                    break;
                }
                case Subject::Unordered_Map:
                {
                    u_map->reserve(table_count);
                    break;
                }
            }
            break;
        }
        case TableType::Shuffle:
        {
            fill_counting_upward(table, table_count);
            shuffle(table, table_count);
            break;
        }
    }
}

static s64 benchmark_map(Benchmark* benchmark, Map* map, void** table,
    void** miss_table, int table_count, Clock* clock, Heap* heap)
{
    s64 milliseconds = 0;

    switch(benchmark->type)
    {
        case BenchmarkType::Deletion:
        {
            insert_table(map, table, table_count, heap);
            shuffle(table, table_count);

            s64 start = start_timing(clock);
            delete_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Insertion:
        {
            s64 start = start_timing(clock);
            insert_table(map, table, table_count, heap);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Iteration:
        {
            insert_table(map, table, table_count, heap);

            s64 start = start_timing(clock);
            iterate_map(map);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search:
        {
            insert_table(map, table, table_count, heap);
            shuffle(table, table_count);

            s64 start = start_timing(clock);
            search_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search_Misses:
        {
            insert_table(map, table, table_count, heap);

            s64 start = start_timing(clock);
            search_table(map, miss_table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search_Half_Misses:
        {
            insert_table(map, table, table_count, heap);
            delete_random_half_and_shuffle(map, table, table_count);

            s64 start = start_timing(clock);
            search_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
    }

    return milliseconds;
}

static s64 benchmark_unordered_map(Benchmark* benchmark, hash_t* map,
    void** table, void** miss_table, int table_count, Clock* clock)
{
    s64 milliseconds = 0;

    switch(benchmark->type)
    {
        case BenchmarkType::Deletion:
        {
            insert_table(map, table, table_count);
            shuffle(table, table_count);

            s64 start = start_timing(clock);
            delete_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Insertion:
        {
            s64 start = start_timing(clock);
            insert_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Iteration:
        {
            insert_table(map, table, table_count);

            s64 start = start_timing(clock);
            iterate_map(map);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search:
        {
            insert_table(map, table, table_count);
            shuffle(table, table_count);

            s64 start = start_timing(clock);
            search_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search_Misses:
        {
            insert_table(map, table, table_count);

            s64 start = start_timing(clock);
            search_table(map, miss_table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
        case BenchmarkType::Search_Half_Misses:
        {
            insert_table(map, table, table_count);
            delete_random_half_and_shuffle(map, table, table_count);

            s64 start = start_timing(clock);
            search_table(map, table, table_count);
            milliseconds = stop_timing(clock, start);
            break;
        }
    }

    return milliseconds;
}

static void run_benchmark(Heap* heap, FILE* file)
{
    // Set up for the benchmarks.

    Clock clock;
    set_up_clock(&clock);

    const int benchmarks_count = 9;
    Benchmark benchmarks[benchmarks_count];

    benchmarks[0].type = BenchmarkType::Insertion;
    benchmarks[0].table_type = TableType::Shuffle;

    benchmarks[1].type = BenchmarkType::Insertion;
    benchmarks[1].table_type = TableType::Random;

    benchmarks[2].type = BenchmarkType::Insertion;
    benchmarks[2].table_type = TableType::Random_With_Reserve;

    benchmarks[3].type = BenchmarkType::Deletion;
    benchmarks[3].table_type = TableType::Random;

    benchmarks[4].type = BenchmarkType::Search;
    benchmarks[4].table_type = TableType::Shuffle;

    benchmarks[5].type = BenchmarkType::Search;
    benchmarks[5].table_type = TableType::Random;

    benchmarks[6].type = BenchmarkType::Search_Misses;
    benchmarks[6].table_type = TableType::Random_Both_Tables;

    benchmarks[7].type = BenchmarkType::Search_Half_Misses;
    benchmarks[7].table_type = TableType::Random;

    benchmarks[8].type = BenchmarkType::Iteration;
    benchmarks[8].table_type = TableType::Random;

    // Go through all the benchmark specifications and run each of their
    // benchmarks accordingly.

    for(int i = 0; i < benchmarks_count; i += 1)
    {
        Benchmark* benchmark = &benchmarks[i];

        for(int j = 0; j < subjects_cap; j += 1)
        {
            Subject subject = subjects[j];

            for(int k = 0; k < table_counts_cap; k += 1)
            {
                int table_count = table_counts[k];

                Map map = {};
                map_create(&map, heap);
                hash_t u_map;
                void** table = HEAP_ALLOCATE(heap, void*, table_count);
                void** miss_table = nullptr;

                setup_tables(benchmark, table, &miss_table, table_count,
                        subject, &map, &u_map, heap);

                s64 milliseconds = 0;
                switch(subject)
                {
                    case Subject::Map:
                    {
                        milliseconds = benchmark_map(benchmark, &map, table,
                                miss_table, table_count, &clock, heap);
                        break;
                    }
                    case Subject::Unordered_Map:
                    {
                        milliseconds = benchmark_unordered_map(benchmark,
                                &u_map, table, miss_table, table_count, &clock);
                        break;
                    }
                }

                benchmark->milliseconds[j][k] = milliseconds;

                SAFE_HEAP_DEALLOCATE(heap, table);
                SAFE_HEAP_DEALLOCATE(heap, miss_table);
                map_destroy(&map, heap);
            }
        }
    }

    // Report the findings recorded for each benchmark.

    for(int i = 0; i < benchmarks_count; i += 1)
    {
        Benchmark* benchmark = &benchmarks[i];

        const char* type = describe_benchmark_type(benchmark->type);
        const char* table_type = describe_table_type(benchmark->table_type);
        fprintf(file, "benchmark: %s — table setup: %s\n", type, table_type);

        for(int j = 0; j < subjects_cap; j += 1)
        {
            const char* subject = describe_subject(subjects[j]);
            fprintf(file, " %13s |", subject);
        }
        fprintf(file, " in table\n");

        for(int j = 0; j < table_counts_cap; j += 1)
        {
            for(int k = 0; k < subjects_cap; k += 1)
            {
                s64 milliseconds = benchmark->milliseconds[k][j];
                fprintf(file, " %11" PRId64 "ms |", milliseconds);
            }
            fprintf(file, " %7d\n", table_counts[j]);
        }

        fprintf(file, "\n");
    }
}
