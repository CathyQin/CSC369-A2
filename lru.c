#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int lru;

int first = 0; 

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	// as a linkedlist
	// change the lru to the next
	// change the current lru to the -1
    int last_lru = lru;
	lru = coremap[lru].next;
    coremap[lru].prev = -1;
    return last_lru;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	int frame_number = p->frame>>PAGE_SHIFT;
    if (coremap[frame_number].used) {
        // check if it is the last and not only one
        if (first != frame_number && frame_number != lru) {
			// conect frame
			struct frame temp = coremap[frame_number];
            coremap[temp.prev].next = temp.next;
            coremap[temp.next].prev = temp.prev;
        }
		// check if it the first one
        if (frame_number == lru){
			struct frame temp = coremap[frame_number];
            lru = temp.next;
            coremap[lru].prev = -1;
        }
    }
    // chekc if it has repeated
    if (frame_number != first) {
        coremap[first].next = frame_number;
        coremap[frame_number].prev = first;
    }
    // change first to current index
    first = frame_number;
	// change the flag of used
    coremap[frame_number].used = 1;
}


/* Initialize any data structures needed for this
 * replacement algorithm
 */
void lru_init() {
    lru = 0;
    coremap[0].prev = -1;
}
