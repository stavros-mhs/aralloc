/* arena.h - v0.1 - public domain memory arena allocator
    DEPENDENCIES:
	- Requires <stdlib.h> for malloc
        - Requires <sys/mman.h> for mmap/munmap
        - POSIX systems _only_
        - NOT compatible with Windows

    API DOCUMENTATION:
	### Arena Types
    	Two types of memory arenas are supported:

	- AR_FIXED: Fixed-size arena (64KB by default)
	    * When capacity is reached, new allocations fail

	- AR_DYNAMIC:
	    * Allocates memory in chunks
	    * When current chunk reaches capacity limit a new chunk gets created
	    * Chunk capacity doubles with each expansion
	    * All previous allocations remain valid
	    * Starts at 4KB

	### API
	struct Arena *arinit(ArenaType type);
		Initializes an arena of <type>.

		Parameters:
		- type: AR_FIXED or AR_DYNAMIC

		Returns:
		- Pointer to initialed arena on success.
		- NULL on failure.

		Notes:
		- AR_FIXED arenas start with 64KB of memory (16 pages)
		- AR_DYNAMIC arenas starts with 4KB (1 page) and grow as needed.

	void arfree(Arena* arena);
		Used to deallocate an arena.

		Parameters:
		- arena: Poitnter to arena to free

		Notes:
		- aralloc-ated variables become invalid.

	void* aralloc(Arena* arena, size_t size);
		Allocates memory from the arena.

		Parameters:
		- arena: pointer to the arena
		- size: number of bytes to allocate

		Returns:
		- Pointer to allocated memory on success.
		- NULL on failure (arena full for AR_FIXED, or out of memory).

		Notes:
		- All allocations are 16-byte aligned.
		- For AR_DYNAMIC, allocations may cause expansion.

	void arreset(Arena* arena);
		Resets the arena to initial state

		Parameters:
		- arena: pointer to arena to reset.

		Notes:
		- AR_FIXED: offeset is set to 0
		- AR_DYNAMIX: Rewinds all chunk offsets, _keeps all chunks_.
		- Allocation on a reset arena overwrite previously allocated
		  data. Accessing such data is _undefined behavior_.

    USAGE:
    	Do this: #define ARENA_IMPLEMENTATION
    	before you include this file in *one* C or C++ file
	to create the implementation

    // i.e. it should look like this:
    #include ..
    #include ..
    #include ..
    ..
    #define ARENA_IMPLEMENTATION
    #include "arena.h"
    ..
	Arena *arena = arinit(AR_FIXED); // or Arena *arena = arinit(AR_DYNAMIC)
	struct my_struct *new_struct = aralloc(arena, sizeof(my_struct));
	arfree(arena);
	arreset(arena);

    LICENSE: See end of file for license information.
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

typedef enum {
	AR_FIXED,
	AR_DYNAMIC,
} ArenaType;

// Public API declarations
struct Arena *arinit(ArenaType type);
void arfree(Arena* arena);
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
#include <stdlib.h>

#define PAGE_SIZE 4096

// ensures every arena allocation is a multiple of 16 bytes
#define AR_ALIGN 16
#define AR_ALIGN_UP(n) (((n) + AR_ALIGN -1) & ~(AR_ALIGN - 1))

// ==========================================
// 		CHUNK HANDLING
// ==========================================

struct Chunk {
	char *memory;
	size_t capacity;
	size_t offset;
	struct Chunk *next;
};

struct Chunk *chunk_init(ArenaType type, size_t size) {
	if (type != AR_FIXED && type != AR_DYNAMIC) return NULL;

	struct Chunk* new_chunk = malloc(sizeof(struct Chunk));

	// can't allocate memory chunk
	if (!new_chunk) return NULL;

	new_chunk->memory = mmap(NULL, size,
                        	PROT_READ | PROT_WRITE,
                        	MAP_PRIVATE | MAP_ANONYMOUS,
                        	-1, 0);

	// new chunk has no memory, raise nomem error
	if (new_chunk->memory == MAP_FAILED) {
		free(new_chunk);
		return NULL;
	}

	new_chunk->capacity = size;
	new_chunk->offset = 0;
	new_chunk->next = NULL;

	return new_chunk;
}

void chunk_destroy (struct Chunk *chunk) {
	while (chunk) {
		struct Chunk *next = chunk->next;

		munmap(chunk->memory, chunk->capacity);

		free(chunk);
		chunk = next;
	}
	return;
}
// CHUNK HANDLING


// ==========================================
// 		ARENA HANDLING
// ==========================================
struct Arena {
	ArenaType type;
    	struct Chunk *head;
    	struct Chunk *curr;
};

struct Arena *arinit (ArenaType type) {
	if (type != AR_FIXED && type != AR_DYNAMIC) return NULL;

	struct Arena *new_arena = malloc(sizeof(struct Arena));

	if (!new_arena) return NULL;

	new_arena->type = type;

	size_t init_size = (type == AR_FIXED) ? PAGE_SIZE * 16 : PAGE_SIZE;

	new_arena->head = new_arena->curr = chunk_init(type, init_size);

	if (!new_arena->head) {
		free(new_arena);
		return NULL;
	}

	return new_arena;
}

void arfree(Arena* arena) {
	chunk_destroy(arena->head);
	free(arena);
}

void *aralloc(Arena* arena, size_t size) {
	if (!arena) return NULL;

	size = AR_ALIGN_UP(size);

	if (arena->curr->offset + size <= arena->curr->capacity) {
		void *ptr = arena->curr->memory + arena->curr->offset;
		arena->curr->offset += size;

		return ptr;
	}

	// Not enough space in chunk
	if (arena->type == AR_FIXED) return NULL;

	if (arena->type == AR_DYNAMIC) {
		size_t next_size = arena->curr->capacity * 2;

		if (next_size < size) {
			next_size = AR_ALIGN_UP(size);
		}

		struct Chunk* new_chunk = chunk_init(AR_DYNAMIC, next_size);

		if(!new_chunk) return NULL;

		arena->curr->next = new_chunk;
		arena->curr = arena->curr->next;

		void *ptr = arena->curr->memory + arena->curr->offset;
		arena->curr->offset += size;
		return ptr;
	}

	return NULL;
}

void arreset(Arena* arena) {
	if (!arena) return;

	if (arena->type == AR_FIXED) {
		arena->head->offset = 0;
		return;
	}

	if(arena->type == AR_DYNAMIC) {
		struct Chunk *curr = arena->head;
		while (curr) {
			curr->offset =0;
			curr = curr->next;
		}

		arena->curr = arena->head;
		return;
	}
}

// ARENA HANDLING

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
