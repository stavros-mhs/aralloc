## Simple Memory Arena allocator for C/C++

### Dependencies
```c
#include<stlib.h> // for malloc/free
#include<sys/mman.h> // for mmap/munmap
```

### API
```c
typedef struct Arena Arena;

typedef enum {
	AR_FIXED,
	AR_DYNAMIC,
} ArenaType;

Arena* arinit(ArenaType type);
void* aralloc(Arena*, size_t);
void arreset(Arena*);
int arfree(Arena*);
```

### Documentation

You can find documentation at the start of `arena.h`.

### When to use

- Not a `malloc` replacement.

#### Context
I learnt about memory arenas and [stb signle-file libraries](https://github.com/nothings/stb) recently and wanted to attempt to implement my own memory arena allocator in C/C++ in that spirit.


### Status

This is a learning project I'm actively using. Expect:

- Bugs and changes
- Possible API changes
- Maybe test/examples someday

If you use this, let me know how it goes! Feedback is welcome.
