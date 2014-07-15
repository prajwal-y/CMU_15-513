/*
 * mm.c
 * pyadapad - Prajwal Yadapadithaya
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a full description of your solution.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "contracts.h"

#include "mm.h"
#include "memlib.h"


// Create aliases for driver tests
// DO NOT CHANGE THE FOLLOWING!
#ifdef DRIVER
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif

/*
 *  Logging Functions
 *  -----------------
 *  - dbg_printf acts like printf, but will not be run in a release build.
 *  - checkheap acts like mm_checkheap, but prints the line it failed on and
 *    exits if it fails.
 */

#ifndef NDEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#define checkheap(verbose) do {if (mm_checkheap(verbose)) {  \
                             printf("Checkheap failed on line %d\n", __LINE__);\
                             exit(-1);  \
                        }}while(0)
#else
#define dbg_printf(...)
#define checkheap(...)
#endif

/*Defining constants and macros that will be used in malloc implementation*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x < y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(uint32_t *)(p))
#define PUT(p , value) (*(uint32_t *)(p) = (value))

static char *heap_ptr;

/*
 *  Helper functions
 *  ----------------
 */

// Align p to a multiple of w bytes
static inline void* align(const void const* p, unsigned char w) {
    return (void*)(((uintptr_t)(p) + (w-1)) & ~(w-1));
}

// Check if the given pointer is 8-byte aligned
static inline int aligned(const void const* p) {
    return align(p, 8) == p;
}

// Return whether the pointer is in the heap.
static int in_heap(const void* p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}


/*
 *  Block Functions
 *  ---------------
 *  TODO: Add your comment describing block functions here.
 *  The functions below act similar to the macros in the book, but calculate
 *  size in multiples of 4 bytes.
 */

// Return the size of the given block in multiples of the word size
static inline unsigned int block_size(const uint32_t* block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    return (block[0] & ~0x7);
}

// Return 0 if the block is free, 1 otherwise
static inline int block_free(const uint32_t* block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    return (block[0] & 0x1);
}

// Mark the given block as free(1)/alloced(0) by marking the header and footer.
static inline void block_mark(uint32_t* block, int free) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    unsigned int size = block_size(block);
    PUT(block, PACK(size, free));
    PUT((block + (size/WSIZE) - 1), PACK(size, free));
}

// Return a pointer to the memory malloc should return
static inline uint32_t* block_mem(uint32_t* const block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));
    REQUIRES(aligned(block + 1));

    return block + 1;
}

//Return header of the current block
static inline uint32_t* get_header(uint32_t* const bp) {
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    return bp - 1;
}

//Return footer of the current block
static inline uint32_t* get_footer(uint32_t* const bp) {
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    return bp + (block_size(get_header(bp))/WSIZE) - 2;
}

//Return block pointer of the next block
static inline uint32_t* next_bp(uint32_t* const bp) {
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    return bp + (block_size(get_header(bp))/WSIZE);
}

//Returns block pointer of the previous block
static inline uint32_t* prev_bp(uint32_t* const bp) {
    REQUIRES(bp != NULL);
    REQUIRES(in_heap(bp));

    return bp - (block_size(get_header(bp) - 1)/WSIZE);
}


// Return the header to the previous block
static inline uint32_t* block_prev(uint32_t* const block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    return block - (block_size(block - 1)/WSIZE);
}

// Return the header to the next block
static inline uint32_t* block_next(uint32_t* const block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    return block + (block_size(block)/WSIZE);
}

//Coalesce free blocks if possible
static void *coalesce(void *bp) {
    size_t prev_alloc = block_free(block_prev(get_header(bp)));
    size_t next_alloc = block_free(block_next(get_header(bp)));
    size_t size = block_size(get_header(bp));

    if(prev_alloc && next_alloc)
	return bp;
    else if(prev_alloc && !next_alloc) {
	size += block_size(block_next(get_header(bp)));
	PUT(get_header(bp), PACK(size, 0));
	PUT(get_footer(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc) {
	size += block_size(block_prev(get_header(bp)));
	PUT(get_header(prev_bp(bp)), PACK(size, 0));
	PUT(get_footer(bp), PACK(size, 0));
	bp = prev_bp(bp);
    }
    else if(!prev_alloc && !next_alloc) {
	size += block_size(block_next(get_header(bp))) + block_size(block_prev(get_header(bp)));
	PUT(get_header(prev_bp(bp)), PACK(size, 0));
	PUT(get_footer(next_bp(bp)), PACK(size, 0));
	bp = prev_bp(bp);
    }
    checkheap(1);
    return bp;
}

//Extend the heap
static void *extend_heap(size_t words) {
    //Even number of words to maintain alignment
    size_t size = (words % 2) ? ((words + 1) * WSIZE) : (words * WSIZE);
    uint32_t *bp = mem_sbrk(size);
    if((long)bp == -1)
        return NULL;

    PUT(get_header(bp), PACK(size, 0));
    PUT(get_footer(bp), PACK(size, 0));
    PUT(block_next(get_header(bp)), PACK(0,1));

    return coalesce(bp);

}

//Find the best fit
/*static void *find_best_fit(size_t size) {
    uint32_t *best_bp = NULL;
    int size_dif = -1;
    uint32_t *ptr = (uint32_t *)heap_ptr + 2;
    unsigned int bsize = block_size(ptr);
    while(bsize > 0) {
	int dif = bsize - size;
	if(dif >= 0 && !block_free(ptr)) {
	    if(size_dif == -1 || (dif < size_dif)) {
		size_dif = dif;
		best_bp = block_mem(ptr);
	    }
	}
	ptr = block_next(ptr);
	bsize = block_size(ptr);
    }
    return best_bp;
}*/

//Find the first fit
static void *find_first_fit(size_t size) {
    uint32_t *ptr = (uint32_t *)heap_ptr + 2;
    unsigned int bsize = block_size(ptr);
    while(bsize > 0) {
	if((size <= bsize) && !block_free(ptr))
	    return (void *)block_mem(ptr);
	ptr = block_next(ptr);
	bsize = block_size(ptr);
    }
    return NULL;
}


//Placing the allocated block in the free block
static void place(void *bp, size_t size) {
    size_t bsize = block_size(get_header(bp));
    if((bsize-size) >= (2*DSIZE)) {
	PUT(get_header(bp), PACK(size, 1));
	PUT(get_footer(bp), PACK(size, 1));
	bp = next_bp(bp);
	PUT(get_header(bp), PACK(bsize-size, 0));
	PUT(get_footer(bp), PACK(bsize-size, 0));
    }
    else {
	PUT(get_header(bp), PACK(bsize, 1));
	PUT(get_footer(bp), PACK(bsize, 1));
    }
}

/*
 *  Malloc Implementation
 *  ---------------------
 *  The following functions deal with the user-facing malloc implementation.
 */

/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
    heap_ptr = mem_sbrk(4*WSIZE);
    if(heap_ptr == (void *)-1)
      return -1;
    
    PUT(heap_ptr, 0);
    PUT(heap_ptr + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_ptr + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_ptr + (3*WSIZE), PACK(0,1));
    heap_ptr += WSIZE;

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
	return -1;
    return 0;
}

/*
 * malloc
 */
void *malloc (size_t size) {
    size_t a_size;
    size_t e_size;
    uint32_t *bp = NULL;

    if(size == 0)
	return NULL;

    if(size <= DSIZE)
	a_size = 2*DSIZE;
    else
	a_size = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    bp = find_first_fit(a_size);
    if(bp != NULL) {
	place(bp, a_size);
	return bp;
    }
    
    e_size = MAX(a_size, CHUNKSIZE);
    bp = extend_heap(e_size/WSIZE);
    if(bp != NULL) {
	place(bp, a_size);
	return bp;
    }
    if(0)
    	in_heap(bp);
    return NULL;
}

/*
 * free
 */
void free (void *ptr) {
    if (ptr == NULL) {
        return;
    }
    block_mark(get_header(ptr), 0);
    coalesce(ptr);
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
    size_t oldsize;
    void *newptr;
    
    if(size == 0) {
	free(oldptr);
	return 0;
    }

    if(oldptr == NULL)
	return malloc(size);

    newptr = malloc(size);
    if(!newptr)
	return 0;

    oldsize = block_size(get_header(oldptr));
    if(size < oldsize)
	oldsize = size;
    memcpy(newptr, oldptr, oldsize);

    free(oldptr);

    return newptr;
}

/*
 * calloc - you may want to look at mm-naive.c
 */
void *calloc (size_t nmemb, size_t size) {
    size_t bytes = nmemb * size;
    void *newptr;

    newptr = malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}

// Returns 0 if no errors were found, otherwise returns the error
int mm_checkheap(int verbose) {
    printf("\n\nCheckheap begins!\n\n");
    printf("Heap (%p):\n", (void *)heap_ptr);
    uint32_t *ptr = (uint32_t *)heap_ptr;
    unsigned int bsize = block_size(ptr);
    while(bsize > 0) {
	printf("%p:: %x::\t", (void *)ptr, *ptr);
	printf("Header: Block size= %x\t Block Allocated= %d\n", bsize, block_free(ptr));
        ptr = block_next(ptr);
        bsize = block_size(ptr);
    }
    printf("%p:: %x::\t", (void *)ptr, *ptr);
    printf("Header: Block size= %x\t Block Allocated= %d\n", bsize, block_free(ptr));
    printf("End of checkheap\n\n");
    verbose = verbose;
    return 0;
}
