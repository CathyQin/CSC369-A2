#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "sim.h"
#include "pagetable.h"

extern int debug;

extern struct frame *coremap;

int capacity;

struct node {
	int last_index;
	addr_t vaddr;
	struct node *next;
	struct node *tail;
};

// using hash tale to store
struct node **table;
// check if there is a node linkedlist at that position
int *check;
//store the next access for each index
int *info;

int current_index;
// count the total lines
int total;

int hashIndex(addr_t add) {
	return add % capacity;
}

struct node *insert(addr_t add, int index) {
	// find the hash index by calling the helper
	int hash_index = hashIndex(add);
	// initial a new node
	struct node *new_node = (struct node*) malloc(sizeof(struct node));
	new_node->add = add;
	new_node->last_index = index;
	new_node->tail = new_node;

	//check if there is a linked list at the location of hash index of table
	if(check[hash_index]) {
		// insert to the tail of linked list
		struct node *curr_linkedlist = table[hash_index];
		curr_linkedlist->tail->next = new_node;
		curr_linkedlist->tail = new_node;
	}
	else {
		// intial the a new linked list
		table[hash_index] = new_node;
	}

	return new_node;
}

struct node *search(addr_t add) {
	int hash_index = hashIndex(add);
	// check if there is a linkedlst in this hash index
	if(check[hash_index]) {
		struct node *head = table[hash_index];
		// find the node contain the same vaddr
		while(head != NULL && head->vaddr != vaddr) {
			head = head->next;
		}
		return head;
	} else{
		return NULL;
	}
}

// using recursion to free all nodes
int free_nodes(struct node *curr_node) {
	if(curr_node->next != NULL) {
		free_nodes(curr_node);
	}
	free(curr_node);
}
/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int i = 0;
	int fathest_index = -1;
	int fathest = 0;

	while(i < memsize) {
		int next_index = coremap[i].next;

		// find the largest index and return the corrponding value
		if(next_index == -1) {
			return i;
		}
		if(next_index > fathest){
			fathest_index = i;
			fathest = next_index
		}

		i++;
	}
	return fathest_index;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	current_index ++;
	coremap[p->frame >> PAGE_SHIFT].next = info[current_index];
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	FILE *tfp;
	int i;
	int count;
	char *fgets_return;
	char buf[MAXLINE];
	addr_t add = 0;
	char type;
	int total = 0;

	tfp = fopen(tracefile, "r");
	if(tfp == NULL) {
		perror("ERROR OPENING FILE");
		exit(1);
	}
	fgets_return = fgets(buf, MAXLINE, tfp);
	while(fgets_return != NULL) {
		if(buf[0] != '=') {
			total += 1;
		} else{
			continue;
		}
		fgets_return = fgets(buf, MAXLINE, tfp);
	}
	// back to the beginning of the file
	fseek(tfp, 0, SEEK_SET);

	table = (struct node**) malloc(total * sizeof(struct node));
	info = malloc(sizeof(int)*total + 1);
	check = malloc(sizeof(int)*total);

	i = 0;
	while(i<total) {
		table[i] = NULL;
		i++;
	}

	count = 0;
	fgets_return = fgets(buf, MAXLINE, tfp);
	while(fgets_return != NULL) {
		struct linked_node *cur_node;
		if(buf[0] != '=') {
			if(find_node(vaddr) == NULL){
				sscanf(buf, "%c %lx", &type, &add);
				// add the node into the linkedlist
				struct node *new_node = insert(add, count);
			}
		}
		new_node->last_index = count;
		info[count] = -1;
		count = count + 1;
		fgets_return = fgets(buf, MAXLINE, tfp);
	}

	fclose(tfp);
}

