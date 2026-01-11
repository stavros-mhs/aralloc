/* arena.h - v0.1 - public domain memory arena allocator
    DEPENDENCIES:
        - Requires <string.h> for memcpy
        - Requires <sys/mman.h> for mmap/munamp
        - POSIX systems _only_
        - NOT compatible with Windows

    Do this:
        #define ARENA_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation

    // i.e. it should look like this:
    #include ..
    #include ..
    #include ..
    ..
    #define ARENA_IMPLEMENTATION
    #include "arena.h"
    ..

    USAGE:

    Arena *arena = arinit();
    void *ptr = aralloc(arena, 1024);
    arfree(arena);

    // arreset sets offset to 0. following  aralloc() calls
    // will overwrite previous data, using memory without allocating anew
    arreset(arena);

    IMPORTANT
    - arinit() returns NULL if the arena couldn't be allocated 
    - arfree() returns -1 if it couldn't free the arena
    - arreset() returns -1 if it was not given an arena
    - aralloc() returns a pointer to the newly allocated item on success
                returns NULL if it was not given an arena or if it could
                extend the arena. 

    LICENSE:

    See end of file for license information.
*/

#ifndef ARENA_H
#define ARENA_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================
//        HEADER SECTION
// ============================

#include <stddef.h>

typedef struct Arena Arena;

// Public API declarations
Arena* arinit(void);
int arfree(Arena* arena);
void* aralloc(Arena* arena, size_t size);
void arreset(Arena* arena);

#ifdef __cplusplus
}
#endif

#endif // ARENA_H

// ============================================
//           IMPLEMENTATION SECTION
// ============================================

#ifdef ARENA_IMPLEMENTATION

#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096

struct Arena {
    char *memory;
    size_t capacity;
    size_t offset;
};

struct Arena *arinit(void) {
    void *arptr = mmap(NULL, sizeof(struct Arena),
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1, 0);

    if (arptr == MAP_FAILED) return NULL;

    void *arcapacity = mmap(NULL, PAGE_SIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);

    if (arcapacity == MAP_FAILED) return NULL;

    struct Arena *arena = (struct Arena *)arptr;

    arena->memory = (char *)arcapacity ;
    arena->capacity = PAGE_SIZE;
    arena->offset = 0;

    return arena;
}

int arfree(Arena* arena) {
    if (!arena) return -1;

    // munmap failed
    if (munmap(arena->memory, arena->capacity) == -1)
        return -1;

    // munmap failed
    if (munmap (arena, PAGE_SIZE) == -1)
        return -1;

    return 0;
}

void *aralloc(Arena* arena, size_t size) {
    if (!arena) return NULL;

    if (arena->offset + size <= arena->capacity) {
        void *ptr = arena->memory + arena->offset;
        
        arena->offset += size;

        return ptr;
    } else {
        size_t new_capacity = arena->capacity * 2;

        while (new_capacity < arena->offset + size) {
            new_capacity *= 2;
        }

        void *new_mem = mmap (NULL, new_capacity,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1, 0);

        // failed to extend the arena
        if (new_mem == MAP_FAILED) {
            return NULL;
        }

        memcpy(new_mem, arena->memory, arena->offset);        
        munmap(arena->memory, arena->capacity);

        arena->memory = new_mem;
        arena->capacity = new_capacity;

        void *ptr = arena->memory + arena->offset;
        arena->offset += size;

        return ptr;
    }
}

void arreset(Arena* arena) {
    if (!arena) return;
    arena->offset = 0;
}

#endif // ARENA_IMPLEMENTATION

/*
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
