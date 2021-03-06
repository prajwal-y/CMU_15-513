/*
 * mm.c
 * pyadapad - Prajwal Yadapadithaya
 *
 * Implementation of malloc using explicit list, first fit with boundary tag coalescing.
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

static uint32_t *heap_ptr;

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

static inline uint32_t* get_footer_el(uint32_t* const block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));

    return block + (block_size(block)/WSIZE) - 1;
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

//Return correct address (Since we are using only 32 bits to calclulate addresses
static inline uint32_t* fixAddress(unsigned long addr) {
    return (uint32_t *)(((unsigned long)1 << 35) | addr);
}

//Places the free block at the front of the heap
static inline void *placeFreeBlock(uint32_t* const block) {
    REQUIRES(block != NULL);
    REQUIRES(in_heap(block));
    
    //Set the next and prev pointers for the block
    PUT(block + 1, *(heap_ptr + 1));
    PUT(block + 2, (int)(unsigned long)heap_ptr);

    //Set the next pointer of the previous block
    PUT(heap_ptr + 1, (int)(unsigned long)block);

    //Set the prev pointer of the next block if the next block is not epilogue
    if(!block_free(fixAddress((unsigned long)(*(block + 1)))))
	PUT(fixAddress((unsigned long)(*(block + 1) + 8)), (int)(unsigned long)block);

    return block;
}

//Remove a block from the free block list and sets the prev and next pointers accordingly
static inline void removeBlock(uint32_t *block) {
    uint32_t *p = fixAddress((unsigned long)(*(block + 2)));
    uint32_t *n = fixAddress((unsigned long)(*(block + 1)));
    PUT(p + 1, *(block + 1));
    if(!block_free(n))
    	PUT(n + 2, *(block + 2));
}

//Coalesce free blocks if possible
static inline void *coalesce(uint32_t *block) {
    
    //If the block is received by extending heap, place it at the beginning of free list
    if(!in_heap(get_footer_el(block)+1))
	return placeFreeBlock(block);

    size_t prev_alloc = block_free(block-1);
    size_t next_alloc = block_free(get_footer_el(block)+1);
    size_t size = block_size(block);

    //If both the next and prev blocks are not free, place at the beginning of the list
    if(prev_alloc && next_alloc)
	return placeFreeBlock(block);
    //If next block is free and prev block is not free, remove the next block from the
    //free list, and place the given block at the beginning after merging with the next block.
    else if(prev_alloc && !next_alloc) {
	uint32_t *next = get_footer_el(block)+1;
        size += block_size(next);
        PUT(block, PACK(size, 0));
        PUT(get_footer_el(next), PACK(size, 0));
	removeBlock(next);
	return placeFreeBlock(block);
    }
    //If prev block is free and next block is not free, merge the prev block with current block
    else if(!prev_alloc && next_alloc) {
	uint32_t *prev = block - (block_size(block-1)/WSIZE);
	size += block_size(prev);
	PUT(prev, PACK(size, 0));
	PUT(get_footer_el(block), PACK(size, 0));
	return prev;
    }
    //If both prev and next block are free, merge all 3 blocks into 1, and place it at the beginning of the free list
    else if(!prev_alloc && !next_alloc) {
	uint32_t *next = get_footer_el(block)+1;
	uint32_t *prev = block - (block_size(block-1)/WSIZE);
	size += block_size(prev) + block_size(next);
	PUT(prev, PACK(size, 0));
	PUT(get_footer_el(next), PACK(size, 0));
	removeBlock(prev);
	removeBlock(next);
	return placeFreeBlock(prev);
    }
    return NULL;
}

//Extend the heap
static inline void *extend_heap(size_t words) {
    //Even number of words to maintain alignment
    size_t size = (words % 2) ? ((words + 1) * WSIZE) : (words * WSIZE);
    uint32_t *bp = mem_sbrk(size);
    if((long)bp == -1)
        return NULL;

    //Set the header and footer of new block
    PUT(bp, PACK(size, 0));
    PUT(get_footer_el(bp), PACK(size, 0));

    return coalesce(bp);
}

//Find the first fit
static inline void *find_first_fit(size_t size) {
    uint32_t *ptr = heap_ptr;
    unsigned int bsize = block_size(ptr);
    while(bsize > 0) {
	if((size <= bsize) && !block_free(ptr)) { //Return the first block which fits the request
	    return (void *)block_mem(ptr);
	}
	ptr = fixAddress((unsigned long)(*(ptr + 1)));
	bsize = block_size(ptr);
    }
    return NULL;
}


//Placing the allocated block in the free block
static inline void place(void *bp, size_t size) {
    uint32_t *block = (uint32_t *)bp - 1;
    size_t bsize = block_size(block);

    if((bsize-size) >= (2*DSIZE)) {
	//Update the block header and footer
	PUT(block, PACK(size, 1));
	PUT(get_footer_el(block), PACK(size, 1));

	uint32_t *temp = get_footer_el(block) + 2;
	//Set the block header and footer for the reduced free block
	PUT(temp-1, PACK(bsize-size, 0));
	PUT(get_footer_el(temp-1), PACK(bsize-size, 0));

	//Set the prev and next pointers of the block to the reduced free block
	PUT(temp, (int)(unsigned long)*(block+1));
	PUT(temp+1, (int)(unsigned long)*(block+2));

	//Fixing prev and next pointers of the adjacent freeblocks
	uint32_t *next = fixAddress((unsigned long)*(temp));
	uint32_t *prev = fixAddress((unsigned long)*(temp + 1));
	PUT(prev+1, (int)(unsigned long)(temp - 1));
	if(!block_free(next))
	    PUT(next+2, (int)(unsigned long)(temp - 1));
    }
    else {
	PUT(block, PACK(bsize, 1));
	PUT(get_footer_el(block), PACK(bsize, 1));
	uint32_t *next = fixAddress((unsigned long)*(block + 1));
	uint32_t *prev = fixAddress((unsigned long)*(block + 2));
	if(!block_free(next))
	  PUT(next+2, (int)(unsigned long)prev);
        PUT(prev+1, (int)(unsigned long)next);	
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
    heap_ptr = mem_sbrk(5*WSIZE);
    if(heap_ptr == (void *)-1)
      return -1;

    PUT(heap_ptr, 0);
    PUT(heap_ptr + 1, PACK(2*DSIZE, 1));
    PUT(heap_ptr + 2, (uint32_t)(unsigned long)(heap_ptr + 4));
    PUT(heap_ptr + 3, PACK(2*DSIZE, 1));
    PUT(heap_ptr + 4, PACK(0,1));
    heap_ptr += 1;

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
	place(bp+1, a_size);
	return bp+1;
    }
    return NULL;
}

/*
 * free
 */
void free (void *ptr) {
    if (ptr == NULL) {
        return;
    }
    //Set the block as free
    block_mark((uint32_t *)ptr-1, 0);
    //Coalesce if possible
    coalesce((uint32_t *)ptr-1);
}

/*
 * realloc
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
 * calloc
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
    printf("\n\nCheckheap begins! Will print free list\n\n");
    printf("Heap (%p):\n", (void *)heap_ptr);
    uint32_t *ptr = heap_ptr;
    unsigned int bsize = block_size(ptr);
    while(bsize > 0) {
	printf("%p:: %x::\t", (void *)ptr, *ptr);
	printf("Header: Block size= %d  Block Allocated= %d  Next: %x  Prev: %x\n", bsize, block_free(ptr), *(ptr+1), *(ptr+2));
	ptr = fixAddress((unsigned long)(*(ptr + 1)));
	bsize = block_size(ptr);
    }
    printf("%p:: %x::\t", (void *)ptr, *ptr);
    printf("Header: Block size= %d\t Block Allocated= %d\n", bsize, block_free(ptr));
    printf("End of checkheap\n\n");
    verbose = verbose;
    return 0;
}
