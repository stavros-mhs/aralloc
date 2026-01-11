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