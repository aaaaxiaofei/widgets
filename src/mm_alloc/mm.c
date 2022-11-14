#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "memlib.h"

/* Constants */
#define WSIZE           4
#define DSIZE           8
#define CHUNKSIZE       (1<<12)
#define MINBLOCKSIZE    24
#define CURALLOCATED    1
#define PREALLOCATED    2

/* Pool Size Limit*/
#define POOL_START	5
#define POOL_END	12

/* Total number of pool lists */
#define TOTALLIST   8

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* Read and write 8 bytes at address p */
#define GET(p)      (*((size_t*) (p)))
#define PUT(p, val) (*((size_t*) (p)) = (val))

/* Read and write 4 bytes at address p */
#define GET4BYTES(p)     (*((unsigned*) (p)))
#define PUT4BYTES(p, val)(*((unsigned*) (p)) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)         (GET4BYTES(p) & ~0x7)
#define GET_ALLOC(p)        (GET4BYTES(p) & 0x1)

/* Read whether previous block is allocated or not*/
#define GET_PREV_ALLOC(p)   (GET4BYTES(p) & 0x2)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)    ((char*)(bp) - WSIZE)
#define FTRP(bp)    ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and prev blocks */
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))

/* Given free block addr bp, compute addresses in which the previous
   and next free blocks' addresses are stored */
#define SUCCESSOR(bp)   ((char*) ((char*)(bp)))
#define PREDECESSOR(bp)   ((char*) ((char*)(bp) + DSIZE))

/* Helper functions */
static void *coalesce(void *bp);
static void *find(size_t sizeatstart, size_t actual_size);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void addingtolist(char *bp, size_t size);
static void removefromlist(char *bp, size_t size);

/* Global variables */
static char *pool;
char *p_start;


/* The standard allocator interface from stdlib.h.  These are the
 * functions you must implement, more information on each function is
 * found below. They are declared here in case you want to use one
 * function in the implementation of another. */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/*
 * This function, defined in bulk.c, allocates a contiguous memory
 * region of at least size bytes.  It MAY NOT BE USED as the allocator
 * for pool-allocated regions.  Memory allocated using bulk_alloc()
 * must be freed by bulk_free().
 *
 * This function will return NULL on failure.
 */
extern void *bulk_alloc(size_t size);

/*
 * This function is also defined in bulk.c, and it frees an allocation
 * created with bulk_alloc().  Note that the pointer passed to this
 * function MUST have been returned by bulk_alloc(), and the size MUST
 * be the same as the size passed to bulk_alloc() when that memory was
 * allocated.  Any other usage is likely to fail, and may crash your
 * program.
 */
extern void bulk_free(void *ptr, size_t size);

/*
 * This function computes the log base 2 of the allocation block size
 * for a given allocation.  To find the allocation block size from the
 * result of this function, use 1 << block_size(x).
 *
 * Note that its results are NOT meaningful for any
 * size > 4088!
 *
 * You do NOT need to understand how this function works.  If you are
 * curious, see the gcc info page and search for __builtin_clz; it
 * basically counts the number of leading binary zeroes in the value
 * passed as its argument.
 */
static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}

/*
 * mm_init - initializes the heap by putting pointers to 
 * the beginning of each free list at beginning of heap and
 * creating the initial prologue and eplilogue blocks.
 */
int mm_init(void)
{
	/* Allocating segregated free list pointers on heap */
	if ((pool = mem_sbrk(TOTALLIST * DSIZE)) == NULL)
		return -1;

	/* Creating prologue and epilogue */
	if ((p_start = mem_sbrk(4 * WSIZE)) == NULL)
		return -1;
	/* Alignment padding */
	PUT4BYTES(p_start, 0);
	/* Prologue header */
	PUT4BYTES(p_start + WSIZE, PACK(DSIZE, 1));
	/* Prologue footer */
	PUT4BYTES(p_start + (2 * WSIZE), PACK(DSIZE, 1));
	/* Epilogue header */
	PUT4BYTES(p_start + (3 * WSIZE), PACK(0, PREALLOCATED | CURALLOCATED));

	/* Initializing the segregated list pointers on heap */
	size_t ipool;
	for (ipool = POOL_START; ipool <= POOL_END; ipool++) {
		PUT(pool + DSIZE * (ipool - POOL_START);, (size_t) NULL);
	}
	return 0;
}

/*
 * addingtolist - Adds free block to the appropriate list
 * by adding it to the front of the list
 */
static void addingtolist(char *bp, size_t size)
{
	/* Address of head of a particular list */
	char *listhead;

	/* Address pointing to the address of head of a particular list */
	char *segstart;

	size_t ipool;
	for (ipool = POOL_START; ipool <= POOL_END; ipool++) {
		if (size + DSIZE <= (1 << ipool)) {
			segstart = pool + DSIZE * (ipool - POOL_START);
			listhead = (char *) GET(segstart);
			break;
		}
	}

	if (listhead != NULL) {
		/* Set the current block as head */
		PUT(segstart, (size_t) bp);

		/* Set the current free block's previous pointer to NULL */
		PUT(PREDECESSOR(bp), (size_t) NULL);

		/* set current free block's next pointer to previous head */
		PUT(SUCCESSOR(bp), (size_t) listhead);

		/* Set the previous head's previous pointer to current free block */
		PUT(PREDECESSOR(listhead), (size_t) bp);
	} else {
		/* Set the current block as head */
		PUT(segstart, (size_t) bp);
		/* Set the free block's next and prev free block addresses to NULL */
		PUT(SUCCESSOR(bp), (size_t) NULL);
		PUT(PREDECESSOR(bp), (size_t) NULL);
	}
}

/*
 * removefromlist - Removes free block from the segregated list
 * by linking blocks to the left or right of it.
 */
static void removefromlist(char *bp, size_t size)
{
	/* Previous block address */
	char *nextaddress = (char *) GET(SUCCESSOR(bp));
	/* Next block address */
	char *prevaddress = (char *) GET(PREDECESSOR(bp));

	if (prevaddress == NULL && nextaddress != NULL) {
		/* Update head pointer of segregated list */
		size_t ipool;
		for (ipool = POOL_START; ipool <= POOL_END; ipool++) {
			if (size + DSIZE <= (1 << ipool)) {
				PUT(pool + DSIZE * (ipool - POOL_START), (size_t) nextaddress);
				break;
			}
		}
		/* Update the new head block's previous to NULL */
		PUT(PREDECESSOR(nextaddress), (size_t) NULL);

	} else if (prevaddress == NULL && nextaddress == NULL) {
		size_t ipool;
		for (ipool = POOL_START; ipool <= POOL_END; ipool++) {
			if (size + DSIZE <= (1 << ipool)) {
				PUT(pool + DSIZE * (ipool - POOL_START), (size_t) NULL);
				break;
			}
		}
	} else if (prevaddress != NULL && nextaddress == NULL) {
		/* Update new tail block's next to null */
		PUT(SUCCESSOR(prevaddress), (size_t) NULL);
	} else {
		/* Link next block's previous to current's previous block */
		PUT(PREDECESSOR(nextaddress), (size_t) prevaddress);

		/* Link previous block's next to current's next block */
		PUT(SUCCESSOR(prevaddress), (size_t) nextaddress);
	}
}

/*
 * find - Search free list to find an enough free block
 * and returns pointer or NULL if none are found.
 */
static void *find(size_t sizeatstart, size_t actual_size)
{
	char *current = NULL;

	if (sizeatstart < TOTALLIST) {
		current = (char *) GET(pool + DSIZE * sizeatstart);
	}

	/* Finding available free block in list */
	while (current != NULL) {
		if (actual_size <= GET_SIZE(HDRP(current))) {
			break;
		}
		current = (char *) GET(SUCCESSOR(current));
	}
	return current;
}

/*
 * find_fit - Searches through all free lists containing
 * blocks with size greater than size requested and returns
 * pointer to a satisfacotry free block or NULL
 */
static void *find_fit(size_t asize)
{
	size_t sizeatstart;
	char *bp = NULL;

	size_t ipool;
	for (ipool = POOL_START; ipool <= POOL_END; ipool++) {
		if (asize + DSIZE <= (1 << ipool)) {
			for (sizeatstart = 0; sizeatstart < TOTALLIST; sizeatstart++) {
				if ((bp = find(sizeatstart, asize)) != NULL)
					return bp;
			}
		}
	}
	return bp;
}

/*
 * coalesce - combines free blocks from adjacent list
 */
static void *coalesce(void *bp)
{
	/* Prev and Next */
	size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	char *prev_hd;
	char *prev_blk;
	char *next_blk;

	/* Size Constant */
	size_t size = GET_SIZE(HDRP(bp));
	size_t nsize = 0;
	size_t psize = 0;


	/* Prev and Next both allocated */
	if (prev_alloc && next_alloc) {
		return bp;
	}
	/* Prev allocated, Next free */
	else if (prev_alloc && !next_alloc) {
		next_blk = NEXT_BLKP(bp);

		/* remove current free block and next free block from lists */
		removefromlist(bp, size);
		removefromlist(next_blk, GET_SIZE(HDRP(next_blk)));

		/* new block size is current free size plus next free size */
		size += GET_SIZE(HDRP(next_blk));

		/* change header to reflect new size */
		PUT4BYTES(HDRP(bp), PACK(size, prev_alloc));

		/* change footer to reflect new size */
		PUT4BYTES(FTRP(bp), PACK(size, prev_alloc));

		/* add new free block to segregated list */
		addingtolist(bp, size);

		/* return current pointer to free block 
		   since block expanded to next */
		return bp;
	}
	/* Prev free, Next allocated */
	else if (!prev_alloc && next_alloc) {

		/* get previous block's location and header location */
		prev_blk = PREV_BLKP(bp);
		prev_hd = HDRP(prev_blk);

		psize = GET_SIZE(prev_hd);

		/* remove current free block and prev free block from lists */
		removefromlist(bp, size);
		removefromlist(prev_blk, psize);

		/* size is current free size plus prev free size */
		size += psize;

		/* change header to reflect new size */
		PUT4BYTES(prev_hd, PACK(size, GET_PREV_ALLOC(prev_hd)));

		/* change footer to reflect new size */
		PUT4BYTES(FTRP(prev_blk), GET4BYTES(prev_hd));

		/* add new free block to segregated list */
		addingtolist(prev_blk, size);

		/* return prev pointer to prev block */
		return prev_blk;
	} else {
		/* Get prev block and header locations */
		prev_blk = PREV_BLKP(bp);
		prev_hd = HDRP(prev_blk);

		next_blk = NEXT_BLKP(bp);

		psize = GET_SIZE(prev_hd);
		nsize = GET_SIZE(HDRP(next_blk));

		/* Remove free blocks */
		removefromlist(bp, size);
		removefromlist(prev_blk, psize);
		removefromlist(next_blk, nsize);
		size += psize + nsize;

		/* Change header to new size */
		PUT4BYTES(prev_hd, PACK(size, GET_PREV_ALLOC(prev_hd)));

		/* Change footer to new size */
		PUT4BYTES(FTRP(prev_blk), GET4BYTES(prev_hd));

		/* Add new free block to pool list */
		addingtolist(prev_blk, size);

		/* Return prev pointer to free block since block expanded to prev */
		return prev_blk;
	}
}

/*
 *  place - Allocate memory at address bp.
 */
static void place(void *bp, size_t asize)
{
	size_t csize = GET_SIZE(HDRP(bp));
	size_t remainsize = csize - asize;

	char *nextaddress = NEXT_BLKP(bp);
	removefromlist(bp, csize);

	/* Split if remainsize is greater than min size */
	if (remainsize >= MINBLOCKSIZE) {
		/* Update new header information, store info bits */
		PUT4BYTES(HDRP(bp),
				PACK(asize,
					GET_PREV_ALLOC(HDRP(bp)) | CURALLOCATED));

		/* Update next block's address to remaining free block's address */
		nextaddress = NEXT_BLKP(bp);

		/* Inform next adjacent free block the prev block is allocated */
		PUT4BYTES(HDRP(nextaddress), remainsize | PREALLOCATED);
		PUT4BYTES(FTRP(nextaddress), remainsize | PREALLOCATED);

		/* Add remaining free block to appropriate segregated list */
		addingtolist(nextaddress, remainsize);

	} else {
		/* Update new header information, store info bits */
		PUT4BYTES(HDRP(bp), PACK(csize, GET_PREV_ALLOC(HDRP(bp)) | CURALLOCATED));

		/* Inform next adjacent block the prev block is allocated */
		PUT4BYTES(HDRP(nextaddress), GET4BYTES(HDRP(nextaddress)) | PREALLOCATED);

		/* Update next adjacent block's footer only if free */
		if (!GET_ALLOC(HDRP(nextaddress)))
			PUT4BYTES(FTRP(nextaddress), GET4BYTES(HDRP(nextaddress)));
	}
}

/*
 * malloc - returns a pointer to an allocated block payload
 */
void *malloc(size_t size) {
	size_t asize;
	char *bp;

	/* Ignore spurious requests */
	if (size <= 0)
		return NULL;

	/* Adjust block size to include overhead and alignment requests */
	if (size <= 2 * DSIZE)
		asize = MINBLOCKSIZE;
	else
		/* Rounds up to the nearest multiple of alignment */
		asize = (((size_t) (size + WSIZE) + 7) & ~0x7);

	/* Search the free list for a fit */
	if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return bp;
	}

	/* No fit found. Use Bulk allocator */
	return bulk_alloc(size);
}

/*
 * You must also implement calloc().  It should create allocations
 * compatible with those created by malloc().  In particular, any
 * allocations of a total size <= 4088 bytes must be pool allocated,
 * while larger allocations must use the bulk allocator.
 *
 * calloc() (see man 3 calloc) returns a cleared allocation large enough
 * to hold nmemb elements of size size.  It is cleared by setting every
 * byte of the allocation to 0.  You should use the function memset()
 * for this (see man 3 memset).
 */
void *calloc(size_t nmemb, size_t size) {
	size_t i;
	size_t tsize = nmemb * size;
	char *ptr = malloc(tsize);

	char *temp = ptr;

	for (i = 0; i < nmemb; i++) {
		*temp = 0;
		temp = temp + size;
	}
	return ptr;
}

/*
 * You must also implement realloc().  It should create allocations
 * compatible with those created by malloc(), honoring the pool
 * alocation and bulk allocation rules.  It must move data from the
 * previously-allocated block to the newly-allocated block if it cannot
 * resize the given block directly.  See man 3 realloc for more
 * information on what this means.
 *
 * It is not possible to implement realloc() using bulk_alloc() without
 * additional metadata, so the given code is NOT a working
 * implementation!
 */
void *realloc(void *ptr, size_t size) {

	char *newaddress;
	size_t copysize;

	/* Special case */
	if (oldptr == NULL)
		return malloc(size);

	/* Special case */
	if (size == 0) {
		free(oldptr);
		return NULL;
	}

	/* Allocate new free block from existing content */
	newaddress = malloc(size);
	copysize = MIN(size, GET_SIZE(HDRP(oldptr)) - WSIZE);

	/* Move from source to dest */
	memmove(newaddress, oldptr, copysize);

	/* Free */
	free(oldptr);

	return newaddress;
}

/*
 * Free the memory allocated to ptr
 */
void free(void *ptr) {
	char *nxtblkheader;
	size_t size;

	if (ptr == NULL)
		return;

	size = GET_SIZE(HDRP(ptr));
	nxtblkheader = HDRP(NEXT_BLKP(ptr));

	/* Update header and footer to unallocated */
	PUT4BYTES(HDRP(ptr), size | GET_PREV_ALLOC(HDRP(ptr)));
	PUT4BYTES(FTRP(ptr), GET4BYTES(HDRP(ptr)));

	/* Update next block, its previous is no longer allocated */
	PUT4BYTES(nxtblkheader,
			GET_SIZE(nxtblkheader) | GET_ALLOC(nxtblkheader));

	/* Add free block to appropriate segregated list */
	addingtolist(ptr, size);

	coalesce(ptr);
}
