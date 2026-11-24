# my_malloc

A small educational memory allocator for Linux, written in C. It implements `my_malloc`, `my_free`, `my_calloc` and `my_realloc` on top of `sbrk`, the same way a simple `malloc` works under the hood, and comes with a test suite that runs every test in its own process.

This is a learning project, not a replacement for the system `malloc`.

## Features

- **Block headers**: every allocation is preceded by a 32-byte header (size, in-use flag, `next`/`previous` pointers, validation marker). All blocks form a doubly linked list.
- **16-byte alignment**: requested sizes are rounded up to a multiple of 16, so returned pointers stay aligned as long as the heap start is.
- **First-fit search**: freed blocks are reused before the heap is grown.
- **Block splitting**: a free block is split when the remainder can hold a header plus at least 16 bytes.
- **Coalescing**: `my_free` merges a block with free neighbours. It also checks that the neighbours are actually adjacent in memory, because something else (e.g. libc) may have moved the program break and left a gap.
- **Returning memory to the OS**: if the last block in the list is free, at least 4 KB and touches the program break, the heap is shrunk with a negative `sbrk`.
- **`my_calloc`**: allocates `num * size` bytes zeroed, and checks the multiplication for overflow before allocating.
- **`my_realloc`**: resizes a block, preserving its contents up to the smaller of the old and new sizes.
- **Thread safety**: one global `pthread_mutex_t` protects the shared block list (see the contract below).
- **Basic error detection**: invalid pointers (bad marker) and double frees are reported on stderr and ignored.

## API and behaviour

```c
void *my_malloc(size_t size);
void  my_free(void *ptr);
void *my_calloc(size_t num, size_t size);
void *my_realloc(void *ptr, size_t new_size);
```

| Call | Behaviour |
|---|---|
| `my_malloc(0)` | returns `NULL` |
| `my_malloc(n)` for a size that would overflow `size_t`, or that `sbrk` cannot satisfy | returns `NULL` |
| `my_free(NULL)` | does nothing |
| `my_free(p)` for an invalid or already freed pointer | prints a message on stderr and does nothing |
| `my_calloc(0, n)` / `my_calloc(n, 0)` | returns `NULL` |
| `my_calloc(num, size)` where `num * size` overflows | returns `NULL` |
| `my_realloc(NULL, n)` | same as `my_malloc(n)` |
| `my_realloc(p, 0)` | frees `p` and returns `NULL` |
| `my_realloc(p, n)` with `n` not larger than the current block | returns the same pointer |
| `my_realloc(p, n)` with a larger `n` | allocates a new block, copies the old contents, frees the old block and returns the new pointer |
| `my_realloc(p, n)` on an invalid or already freed pointer | prints a message on stderr and returns `NULL` |
| `my_realloc(p, n)` when the allocation fails | returns `NULL` and leaves the old block untouched |

Because a failed `my_realloc` leaves the old block valid, store the result in a temporary variable and only overwrite your pointer when it is not `NULL`:

```c
void *tmp = my_realloc(p, new_size);
if (tmp != NULL) p = tmp;
```

Note that `my_realloc(p, 0)` returns `NULL` *and* frees `p`, so `NULL` does not always mean "the old block is still valid".

### Thread-safety contract

Calls from different threads on **different** blocks are safe. Concurrent calls on the **same** pointer (for example one thread freeing a block while another reallocates it) are a bug in the caller and are undefined, as with the system `malloc`.

## How it works

```
[ header 32B ][ user data: length B ][ header ][ user data ] ...
              ^ pointer returned to the caller
```

`my_free` and `my_realloc` find the header by stepping back one `Blocks` struct from the pointer they receive and check the marker before trusting any field.

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
- `my_calloc`: zeroing of recycled (dirty) blocks, overflow of `num * size`, zero parameters
- `my_realloc`: `NULL` and zero arguments, growing, shrinking, invalid and freed pointers, failed allocation (old block stays intact), and resizing a block between two neighbours without damaging them
- multithreaded stress test (several threads allocating, filling, reallocating, verifying and freeing)

To debug a single failing test, build with `make debug` and run it under `gdb`.

## Limitations

- Linux only (relies on `sbrk`).
- A single global lock, so allocation is serialized between threads.
- The free-list search is linear (first fit).
- `my_realloc` never grows a block in place (it always allocates, copies and frees), and shrinking does not give the extra space back.
- Pointer validation uses a marker value. This is a heuristic: reading the header of a pointer that did not come from this allocator is undefined behaviour.
- The allocator shares the program break with libc. Calling `printf` (or anything else that uses libc `malloc`) between allocations can move the break and leave gaps between blocks. The allocator handles this when merging and shrinking, but in a multithreaded program where other threads use libc `malloc`, returning memory with a negative `sbrk` can race with them, so do not mix the two in that situation.

## License

Licensed under the MIT License.