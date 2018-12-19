#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"

static void remove_chunk_from_freelist(pChunk p)
{
	uint32_t size = p->size;
	pChunk node, prev, next;

	if (free_list[size] == NULL) {
		printf("Something wrong \n"); exit(1);
	}

	node = free_list[size];
	while(node) {
		node = node->next;

		if (!memcmp(node, p, MEM_CHUNK_SIZE)) {
			prev = node->next;
			next = node->prev;
			if (prev != NULL) {
				prev->next = next;
			}
			if (next != NULL) {
				next->prev = prev;
			}
		}
	}
}

static void append_chunk_to_freelist(pChunk p)
{
	uint32_t size = p->size;
	pChunk root, next;

	if (free_list[size] == NULL) {
		root = (pChunk)malloc(MEM_CHUNK_SIZE);
		root->size = 0;
		root->prev = NULL;
		root->next = NULL;
	} else {
		root = free_list[size];
	}

	next = root->next;
	if (next != NULL) {
		p->next = next;
		p->prev = root;

		root->next = p;
		next->prev = p;
	} else {
		root->next = p;
		p->prev = root;
	}
}

static void *find_chunk_in_freelist(uint32_t size)
{
	pChunk node;

	node = free_list[size];
	if (node == NULL)
		goto fin;

	node = node->next;
	if (node != NULL && size == node->size)
		return node + MEM_CHUNK_SIZE;

fin:
	return NULL;

}

static int expand_pool_ifnecessary(uint32_t size)
{
	void *p;
	uint32_t increased_size;

	if (gPool == NULL ||
		(gPool->address == NULL ||
		 gPool->remain <= size + MEM_CHUNK_SIZE))
	{
		gPool = malloc(MEM_POOL_SIZE);
		// expand pool with executable permission
		p = sbrk(EXPAND_POOL_SIZE);
		if (p < 0) {
			printf("EXPAND FAILED\n");
			return -1;
		}
		gPool->address = p;
		increased_size = sbrk(0) - p;

		gPool->size += increased_size;
		gPool->remain += increased_size;
	}

	return 0;
}

static void *create_new_chunk(uint32_t size)
{
	static pChunk current_node;
	uint32_t allocated_size;
	void *p;

	if (expand_pool_ifnecessary(size)) {
		printf("expand failed\n"); exit(1);
	}

	p = gPool->address + gPool->pos;
	(*(pChunk)p).size = size;
	(*(pChunk)p).prev = current_node;
	(*(pChunk)p).next = NULL;

	if (current_node != NULL)
		current_node->next = p;

	// keep current node to link next.
	current_node = p;

	allocated_size = MEM_CHUNK_SIZE + size;
	gPool->pos += allocated_size;
	gPool->remain -= allocated_size;

	return p + MEM_CHUNK_SIZE;
}

void *emalloc(uint32_t size)
{
	void *p;

	if (!(MIN_ALLOC_SIZE <= size &&
		size <= MAX_ALLOC_SIZE))
	{
		return NULL;
	}

	p = find_chunk_in_freelist(size);

	if (p == NULL)
		p = create_new_chunk(size);

	return p;
}

void emfree(void *addr)
{
	if (addr == NULL)
		return;
	pChunk p = (pChunk)(addr - MEM_CHUNK_SIZE);
	append_chunk_to_freelist(p);
}

static void __attribute__((constructor)) init()
{
	free_list = malloc(MEM_CHUNK_SIZE * 64);
}
