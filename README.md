# my_malloc

A small educational memory allocator for Linux, written in C. It implements `my_malloc` and `my_free` on top of `sbrk`, the same way a simple `malloc` works under the hood, and comes with a test suite that runs every test in its own process.

This is a learning project, not a replacement for the system `malloc`.

## Features

- **Block headers**: every allocation is preceded by a 32-byte header (size, in-use flag, `next`/`previous` pointers, validation marker). All blocks form a doubly linked list.
- **16-byte alignment**: requested sizes are rounded up to a multiple of 16, so returned pointers stay aligned as long as the heap start is.
- **First-fit search**: freed blocks are reused before the heap is grown.
- **Block splitting**: a free block is split when the remainder can hold a header plus at least 16 bytes.
- **Coalescing**: `my_free` merges a block with free neighbours. It also checks that the neighbours are actually adjacent in memory, because something else (e.g. libc) may have moved the program break and left a gap.
- **Returning memory to the OS**: if the last block in the list is free, at least 4 KB and touches the program break, the heap is shrunk with a negative `sbrk`.
- **Thread safety**: one global `pthread_mutex_t` protects `my_malloc` and `my_free`.
- **Basic error detection**: invalid pointers (bad marker) and double frees are reported on stderr and ignored.
- **Edge cases**: `my_malloc(0)` returns `NULL`, `my_free(NULL)` does nothing, and sizes that would overflow `size_t` return `NULL`.

## How it works

```
[ header 32B ][ user data: length B ][ header ][ user data ] ...
              ^ pointer returned to the caller
```

`my_free` finds the header by stepping back one `Blocks` struct from the pointer it receives and checks the marker before trusting any field.

## Project layout

```
include/    my_malloc.h, test_func.h
src/        my_malloc.c   the allocator
            test_funcs.c  tests and the test runner helper
            main.c        runs all tests
Makefile
```

## Build and run

```
make          # optimized build -> ./test
make debug    # -g -O0 build   -> ./test_debug (for gdb)
make clean
./test
```

The build uses `-Wall -Wextra -Iinclude -pthread`.

## Tests

Each test is a function listed in a table of `{name, function pointer}` entries. The runner forks a child process per test, so a crash (e.g. a failed `assert`) is reported as a failure of that one test instead of killing the whole run.

Covered so far:

- reuse of freed blocks, splitting (including in the middle of the list and at the tail), and requests too big for a free block
- merging with the next block, the previous block, both, and cases where blocks must not be merged (in-use neighbour, gap in memory)
- invalid pointer and double free handling, `my_malloc(0)`, `my_free(NULL)`
- data integrity (blocks do not overlap and headers stay intact)
- alignment of returned pointers
- oversized requests (`SIZE_MAX` and values near it, and requests `sbrk` cannot satisfy)
- returning memory to the kernel
- multithreaded stress test (several threads allocating, filling, verifying and freeing)

To debug a single failing test, build with `make debug` and run it under `gdb`.

## Limitations

- Linux only (relies on `sbrk`).
- A single global lock, so allocation is serialized between threads.
- The free-list search is linear (first fit).
- Pointer validation uses a marker value. This is a heuristic: reading the header of a pointer that did not come from this allocator is undefined behaviour.
- Calling `printf` (or anything else that uses libc `malloc`) between allocations can move the program break and leave gaps between blocks. The allocator handles this when merging and shrinking, but it is worth knowing when debugging.
- No `calloc` or `realloc` yet.

Licensed under the MIT License