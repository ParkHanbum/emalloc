#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "emalloc.h"

#define ASSERT(TEST) if((TEST)) asm("int $3");

extern pPool gPool;
extern pChunk* free_list;

void print_node(pChunk node)
{
	int i=0;
	while(node)
	{
		printf("[%05d] [%p] -> [%p] [%p] %d\n",
			i++,
			node, node->prev, node->next, node->size);
		node = node->next;
	}

}
void print_pool()
{
	pChunk node;

	// get root node;
	node = gPool->address;

	print_node(node);
}

void print_free_list()
{
	int i;
	pChunk freelist;
	for(i = MIN_ALLOC_SIZE-1; i < MAX_ALLOC_SIZE;i++)
	{
		freelist = free_list[i];
		print_node(freelist);
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

	srand(time(NULL));

	for(i = 0;i < 0xFFFFFF;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		ASSERT(p == NULL);
	}
	print_pool();


	/*
	for(i = 0;i < 0xFFFFFF;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		ASSERT(p == NULL);
		emfree(p);
		print_free_list();
	}
	*/

}
