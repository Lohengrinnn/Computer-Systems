//
//  mm_heap_kr2.c
//  assignment3-mm
//
//  Created by Hokang Yu, Jiong Wu on 3/6/20.
//  Copyright © 2020 JiongWu. All rights reserved.
//


#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include "memlib.h"
#include "mm_heap.h"


/** Allocation unit for header of memory blocks */
typedef union Header {
    struct {
        union Header *prev; /** prev block if on free list*/
        union Header *next; /** next block if on free list */
        bool isFree;        /** if the block is free*/
        size_t size;        /** size of this block including header */
                            /** measured in multiple of header size */
    } s;
    max_align_t _align;     /** force alignment to max align boundary */
} Header;

// forward declarations
static Header *morecore(size_t);
void visualize(const char*);

/** Empty list to get started */
static Header base;

/** Start of free memory list */
static Header *freep = NULL;

/**
 * Initialize memory allocator
 */
void mm_init() {
    mem_init();

    base.s.next = base.s.prev = freep = &base;
    base.s.isFree = false;
    base.s.size = 0;
}

/**
 * Reset memory allocator.
 */
void mm_reset(void) {
    mem_reset_brk();

    base.s.next = base.s.prev = freep = &base;
    base.s.isFree = false;
    base.s.size = 0;
}

/**
 * De-initialize memory allocator.
 */
void mm_deinit(void) {
    mem_deinit();

    base.s.prev = base.s.next = freep = &base;
    base.s.isFree = false;
    base.s.size = 0;
}

/**
 * Allocation units for nbytes bytes.
 *
 * @param nbytes number of bytes
 * @return number of units for nbytes
 */
inline static size_t mm_units(size_t nbytes) {
    /* smallest count of Header-sized memory chunks */
    /*  (+1 additional chunk for the Header itself) needed to hold nbytes */
    return (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
}

/**
 * Allocation bytes for nunits allocation units.
 *
 * @param nunits number of units
 * @return number of bytes for nunits
 */
inline static size_t mm_bytes(size_t nunits) {
    return nunits * sizeof(Header);
}

/**
 * Get pointer to block payload.
 *
 * @param bp the block
 * @return pointer to allocated payload
 */
inline static void *mm_payload(Header *bp) {
    return bp + 1;
}

inline static void connect_dlinkedlist(Header *p, Header *n) {
    p->s.next = n;
    n->s.prev = p;
}

/**
 * Get pointer to block for payload.
 *
 * @param ap the allocated payload pointer
 */
inline static Header *mm_block(void *ap) {
    return (Header*)ap - 1;
}

inline static bool validate_header(Header *bp) {
    Header *prev = bp->s.prev;
    Header *next = bp->s.next;
    if (prev->s.next != bp) {
        return false;
    }
    if (next->s.prev != bp) {
        return false;
    }
    
    if (prev < bp && prev != &base && prev + prev->s.size != bp) {
        return false;
    }
    
    if (bp < next && bp + bp->s.size != next) {
        return false;
    }
    return true;
}

/**
 * Allocates size bytes of memory and returns a pointer to the
 * allocated memory, or NULL if request storage cannot be allocated.
 *
 * @param nbytes the number of bytes to allocate
 * @return pointer to allocated memory or NULL if not available.
 */
void *mm_malloc(size_t nbytes) {
    if (freep == NULL) {
        mm_init();
    }

    Header *prev_p = freep;

    // smallest count of Header-sized memory chunks
    //  (+1 additional chunk for the Header itself) needed to hold nbytes
    size_t nunits = mm_units(nbytes);

    // traverse the circular list to find a block
    for (Header *p = prev_p->s.next; ;prev_p = p, p = p->s.next) {
        
        if (p->s.size >= nunits && p->s.isFree) {          /* found block large enough */
            if (p->s.size == nunits) {
                p->s.isFree = false;
            } else {
                p->s.size -= nunits;
                Header *old_p = p;
                Header *next = p->s.next;
                p += p->s.size;         // address upper block to return
                p->s.size = nunits;     // set size of block
                connect_dlinkedlist(old_p, p);
                connect_dlinkedlist(p, next);
                p->s.isFree = false;
            }

            freep = prev_p;  /* move the head */
            //assert(validate_header(p));
            return mm_payload(p);
        }

        /* back where we started and nothing found - we need to allocate */
        if (p == freep) {                    /* wrapped around free list */
            p = morecore(nunits);
            if (p == NULL) {
                errno = ENOMEM;
                return NULL;                /* none left */
            }
        }
    }

    assert(false);  // shouldn't happen
    return NULL;
}


/**
 * Deallocates the memory allocation pointed to by ap.
 * If ap is a NULL pointer, no operation is performed.
 *
 * @param ap the memory to free
 */
void mm_free(void *ap) {
    // ignore null pointer
    if (ap == NULL) {
        return;
    }

    Header *bp = mm_block(ap);   /* point to block header */

    // validate size field of header block
    assert(bp->s.size > 0 && mm_bytes(bp->s.size) <= mem_heapsize());
    
    Header *p = freep;
    if (bp->s.next == NULL) {
        while (p->s.next > p)
            p = p->s.next;
        Header *n = p->s.next;
        connect_dlinkedlist(p, bp);
        connect_dlinkedlist(bp, n);
    }
    //assert(validate_header(bp));
    bp->s.isFree = true;
    Header *prev = bp->s.prev;
    Header *next = bp->s.next;
    if (bp->s.next->s.isFree && bp + bp->s.size == bp->s.next) {
        bp->s.size += bp->s.next->s.size;
        connect_dlinkedlist(bp, bp->s.next->s.next);
    }
    if (bp->s.prev->s.isFree && bp->s.prev + bp->s.prev->s.size == bp) {
        bp->s.prev->s.size += bp->s.size;
        connect_dlinkedlist(bp->s.prev, bp->s.next);
        bp = bp->s.prev;
    }

    /* reset the start of the free list */
    freep = bp->s.prev;
    //assert(validate_header(bp));
}

/**
 * Tries to change the size of the allocation pointed to by ap
 * to size, and returns ap.
 *
 * If there is not enough room to enlarge the memory allocation
 * pointed to by ap, realloc() creates a new allocation, copies
 * as much of the old data pointed to by ptr as will fit to the
 * new allocation, frees the old allocation, and returns a pointer
 * to the allocated memory.
 *
 * If ap is NULL, realloc() is identical to a call to malloc()
 * for size bytes.  If size is zero and ptr is not NULL, a minimum
 * sized object is allocated and the original object is freed.
 *
 * @param ap pointer to allocated memory
 * @param newsize required new memory size in bytes
 * @return pointer to allocated memory at least required size
 *    with original content
 */
void* mm_realloc(void *ap, size_t newsize) {
    // NULL ap acts as malloc for size newsize bytes
    if (ap == NULL) {
        return mm_malloc(newsize);
    }

    Header* bp = mm_block(ap);    // point to block header
    if (newsize > 0) {
        // return this ap if allocated block large enough
        if (bp->s.size >= mm_units(newsize)) {
            return ap;
        }
    }

    // allocate new block
    void *newap = mm_malloc(newsize);
    if (newap == NULL) {
        return NULL;
    }
    // copy old block to new block
    size_t oldsize = mm_bytes(bp->s.size-1);
    memcpy(newap, ap, (oldsize < newsize) ? oldsize : newsize);
    mm_free(ap);
    return newap;
}


/**
 * Request additional memory to be added to this process.
 *
 * @param nu the number of Header units to be added
 * @return pointer to start additional memory added
 */
static Header *morecore(size_t nu) {
    // nalloc based on page size
    size_t nalloc = mem_pagesize()/sizeof(Header);

    /* get at least NALLOC Header-chunks from the OS */
    if (nu < nalloc) {
        nu = nalloc;
    }

    size_t nbytes = mm_bytes(nu); // number of bytes
    void* p = mem_sbrk(nbytes);
    if (p == (char *) -1) {    // no space
        printf("%s no space\n", __func__);
        return NULL;
    }
    int tmp = ((int)p) & 0xfffff ;
    Header* bp = (Header*)p;
    bp->s.size = nu;
    bp->s.prev = NULL;
    bp->s.next = NULL;

    // add new space to the circular list
    mm_free(bp+1);

    return freep;
}

/**
 * Print the free list (debugging only)
 *
 * @msg the initial message to print
 */
void visualize(const char* msg) {
    fprintf(stderr, "\n--- Free list after \"%s\":\n", msg);

    if (freep == NULL) {                   /* does not exist */
        fprintf(stderr, "    List does not exist\n\n");
        return;
    }

    if (freep == freep->s.next) {          /* self-pointing list = empty */
        fprintf(stderr, "    List is empty\n\n");
        return;
    }

/*
    tmp = freep;                           // find the start of the list
    char* str = "    ";
    do {           // traverse the list
        fprintf(stderr, "%sptr: %10p size: %-3lu blks - %-5lu bytes\n",
            str, (void *)tmp, tmp->s.size, mm_bytes(tmp->s.size));
        str = " -> ";
        tmp = tmp->s.ptr;
    }  while (tmp->s.ptr > freep);
*/
    char* str = "    ";
    for (Header *p = base.s.next; p != &base; p = p->s.next) {
        fprintf(stderr, "%s p: %10p, c: %10p, n: %10p size: %3lu blks - %5lu bytes\n",
            str, (void *)p->s.prev, (void *)p, (void *)p->s.next, p->s.size, mm_bytes(p->s.size));
        str = " -> ";
    }


    fprintf(stderr, "--- end\n\n");
}


/**
 * Calculate the total amount of available free memory.
 *
 * @return the amount of free memory in bytes
 */
size_t mm_getfree(void) {
    if (freep == NULL) {
        return 0;
    }

    // point to head of free list
    Header *tmp = freep;
    size_t res = tmp->s.size;

    // scan free list and count available memory
    while (tmp->s.next > tmp) {
        tmp = tmp->s.next;
        res += tmp->s.size;
    }

    // convert header units to bytes
    return mm_bytes(res);
}

