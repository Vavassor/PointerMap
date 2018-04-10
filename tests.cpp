#include "map.h"

#include <cstdio>

enum class Test
{
    Get,
    Get_Missing,
    Get_Overflow,
    Iterate,
    Remove,
    Remove_Overflow,
    Reserve,
};

static const char* describe_test(Test test)
{
    switch(test)
    {
        default:
        case Test::Get:             return "Get";
        case Test::Get_Missing:     return "Get Missing";
        case Test::Get_Overflow:    return "Get Overflow";
        case Test::Iterate:         return "Iterate";
        case Test::Remove:          return "Remove";
        case Test::Remove_Overflow: return "Remove Overflow";
        case Test::Reserve:         return "Reserve";
    }
}

static bool test_get(Map* map, Heap* heap)
{
    void* key = reinterpret_cast<void*>(253);
    void* value = reinterpret_cast<void*>(512);
    map_add(map, key, value, heap);
    void* found;
    bool got = map_get(map, key, &found);
    return got && found == value;
}

static bool test_get_missing(Map* map, Heap* heap)
{
    void* key = reinterpret_cast<void*>(0x12aa5f);
    void* discard;
    bool got = map_get(map, key, &discard);
    return !got;
}

static bool test_get_overflow(Map* map, Heap* heap)
{
    void* key = reinterpret_cast<void*>(0);
    void* value = reinterpret_cast<void*>(612377);
    map_add(map, key, value, heap);
    void* found;
    bool got = map_get(map, key, &found);
    return got && found == value;
}

static bool test_iterate(Map* map, Heap* heap)
{
    const int pairs_count = 256;
    void* insert_pairs[pairs_count][2];

    Sequence sequence;
    seed(&sequence, 1635899);
    for(int i = 0; i < pairs_count; i += 1)
    {
        void* key = reinterpret_cast<void*>(generate(&sequence));
        void* value = reinterpret_cast<void*>(generate(&sequence));
        insert_pairs[i][0] = key;
        insert_pairs[i][1] = value;
        map_add(map, key, value, heap);
    }

    void* pairs[pairs_count][2];
    int found = 0;

    ITERATE_MAP(it, map)
    {
        pairs[found][0] = map_iterator_get_key(it);
        pairs[found][1] = map_iterator_get_value(it);
        found += 1;
    }
    if(found != pairs_count)
    {
        return false;
    }

    auto compare = [](const void* arg0, const void* arg1) -> int
    {
        void* const* a = static_cast<void* const*>(arg0);
        void* const* b = static_cast<void* const*>(arg1);
        if(a[0] < b[0])
        {
            return -1;
        }
        else if(a[0] == b[0])
        {
            return 0;
        }
        else
        {
            return 1;
        }
    };
    qsort(pairs, pairs_count, sizeof(*pairs), compare);
    qsort(insert_pairs, pairs_count, sizeof(*insert_pairs), compare);

    int mismatches = 0;

    for(int i = 0; i < pairs_count; i += 1)
    {
        bool key_matches = insert_pairs[i][0] == pairs[i][0];
        bool value_matches = insert_pairs[i][1] == pairs[i][1];
        mismatches += !(key_matches && value_matches);
    }

    return mismatches == 0;
}

static bool test_remove(Map* map, Heap* heap)
{
    void* key = reinterpret_cast<void*>(6356);
    void* value = reinterpret_cast<void*>(711677);
    map_add(map, key, value, heap);
    void* discard;
    bool was_in = map_get(map, key, &discard);
    map_remove(map, key);
    bool is_in = map_get(map, key, &discard);
    return was_in && !is_in;
}

static bool test_remove_overflow(Map* map, Heap* heap)
{
    void* key = reinterpret_cast<void*>(0);
    void* value = reinterpret_cast<void*>(6143);
    map_add(map, key, value, heap);
    void* discard;
    bool had = map_get(map, key, &discard);
    map_remove(map, key);
    bool got = map_get(map, key, &discard);
    return had && !got;
}

static bool test_reserve(Map* map, Heap* heap)
{
    int reserve = 1254;
    bool was_smaller = reserve > map->cap;
    map_reserve(map, reserve, heap);
    bool is_enough = reserve <= map->cap;
    return was_smaller && is_enough;
}

static bool run_test(Test test, Map* map, Heap* heap)
{
    switch(test)
    {
        default:
        case Test::Get:             return test_get(map, heap);
        case Test::Get_Missing:     return test_get_missing(map, heap);
        case Test::Get_Overflow:    return test_get_overflow(map, heap);
        case Test::Iterate:         return test_iterate(map, heap);
        case Test::Remove:          return test_remove(map, heap);
        case Test::Remove_Overflow: return test_remove_overflow(map, heap);
        case Test::Reserve:         return test_reserve(map, heap);
    }
}

static void test_map(Heap* heap, FILE* file)
{
    const int tests_count = 7;
    const Test tests[tests_count] =
    {
        Test::Get,
        Test::Get_Missing,
        Test::Get_Overflow,
        Test::Iterate,
        Test::Remove,
        Test::Remove_Overflow,
        Test::Reserve,
    };
    bool which_failed[tests_count] = {};

    int failed = 0;

    for(int i = 0; i < tests_count; i += 1)
    {
        Map map = {};
        map_create(&map, heap);

        bool fail = !run_test(tests[i], &map, heap);
        failed += fail;
        which_failed[i] = fail;

        map_destroy(&map, heap);
    }

    // Report the findings.

    if(failed > 0)
    {
        fprintf(file, "test failed: %d\n", failed);
        int printed = 0;
        for(int i = 0; i < tests_count; i += 1)
        {
            const char* separator = "";
            const char* also = "";
            if(failed > 2 && printed > 0)
            {
                separator = ", ";
            }
            if(failed > 1 && printed == failed - 1)
            {
                if(failed == 2)
                {
                    also = " and ";
                }
                else
                {
                    also = "and ";
                }
            }
            if(which_failed[i])
            {
                const char* test = describe_test(tests[i]);
                fprintf(file, "%s%s%s", separator, also, test);
                printed += 1;
            }
        }
        fprintf(file, "\n\n");
    }
    else
    {
        fprintf(file, "All tests succeeded!\n\n");
    }
}
