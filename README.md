# PointerMap
This is just a regular hash table. It uses pointer types for both its keys and
values and supports automatic growth, removal, and iteration. Using pointer
pairs allows it the flexibility to store integers, pointers, or C strings
without needing to templatize on its elements' types.

It was built to be used in [Arboretum](https://github.com/Vavassor/Arboretum).
This project tests it in isolation and also, for fun, benchmarks it against
[`std::unordered_map`](
https://en.cppreference.com/w/cpp/container/unordered_map).

The actual map is just the two files `map.h` and `map.cpp`.

## Building
This project uses a unity or single-compilation unit build, so compiling
`main.cpp` is all that's required to build the whole project. For convenience,
a couple of build scripts are also included.

- Windows: `build.bat` will compile with Visual C++.
- Linux: `build.sh` will compile with the GNU Compiler Collection. Executing
  a `.sh` script may require giving it executable permission first!

