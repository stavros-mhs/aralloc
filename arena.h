/* arena.h - v0.1 - public domain memory arena allocator
    DEPENDENCIES:
        - POSIX systems _only_
        - Requires <sys/mman.h> for mmap/munamp
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

    Arena *arena = arena_init();
    void *ptr = arena_alloc(arena, 1024);
    arena_free(arena);

    LICENSE:

    See end of file for license information.
*/

#ifndef ARENA_H
#define ARENA_H

#ifdef __cplusplus
extern "C" {
#endif

// ==============
// HEADER SECTION
// ==============

typedef struct Arena Arena;

// Public API declarations
Arena* arinit(void);
void   arfree(Arena* arena);
void*  aralloc(Arena* arena, size_t size);
void   arreset(Arena* arena);

#ifdef __cplusplus
}
#endif

#endif // ARENA_H

// ======================
// IMPLEMENTATION SECTION
// ======================

#ifdef ARENA_IMPLEMENTATION

struct Arena {
    char *memory;
    size_t capacity;
    size_t offset;
}

Arena* arinit(void) {
    // Implementation here
}

void arfree(Arena* arena) {
    // Implementation here
}

void *aralloc(Arena* arena, size_t size) {
    // Implementation here
}

void arreset(Arena* arena) {
    // Implementation here
}

#endif // ARENA_IMPLEMENTATION