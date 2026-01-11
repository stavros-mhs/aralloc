## Simple Memory Arena allocator for C/C++

### Dependencies
```c
#include<string.h> // for memcpy
#include<sys/mman.h> // for mmap/munmap
```

### API
```c
Arena* arinit(void);
void* aralloc(Arena*, size_t);
void arreset(Arena*);
int arfree(Arena*);
```

### Documentation

You can find some basic documentation near the start of `arena.h` but to be honest, the whole implementation is ~100 lines. You can read all of it in a matter of minutes and it should be easy to understand what's going on if you're familiar with how mmap works.

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