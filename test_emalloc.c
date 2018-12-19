#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "emalloc.h"

#define ASSERT(TEST) if((TEST)) asm("int $3");

extern pPool gPool;
extern pChunk* free_list;

void print_node_reverse(pChunk node, bool reverse)
{
	int i=0;
	while(node)
	{
		printf("[%05d] [%p] -> [%p] [%p] %d\n",
			i++,
			node, node->prev, node->next, node->size);
		if (reverse) {
			node = node->prev;
		} else {
			node = node->next;
		}
	}
}

void print_pool_from_last(void *p)
{
	pChunk node = (pChunk)(p - MEM_CHUNK_SIZE);
	print_node_reverse(node, true);
}

void print_pool()
{
	pChunk node;

	// get root node;
	node = gPool->address;

	print_node_reverse(node, false);
}

void print_free_list()
{
	int i;
	pChunk freelist;
	for(i = MIN_ALLOC_SIZE-1; i < MAX_ALLOC_SIZE;i++)
	{
		freelist = free_list[i];
		print_node_reverse(freelist, false);
	}
}

static uint16_t get_alloc_size()
{
	uint16_t res;
	res = (rand() | 0x1) & MAX_ALLOC_SIZE;
	return res;
}

int main()
{
	void *p;
	uint16_t size;
	int i;

	for(i = 0;i < 10000;i++) {
		size = 32;
		p = emalloc(size);
		assert(p != NULL);
	}
	// print_pool_from_last(p);

	srand(time(NULL));
	for(i = 0;i < 10;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		assert(p != NULL);
	}
	// print_pool_from_last(p);

	for(i = 0;i < 0xFF;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		assert(p != NULL);
		emfree(p);
	}
	print_free_list();
}
