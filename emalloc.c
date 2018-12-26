#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"


static void append_addr_to_freelist(void *addr)
{
	pFreeChunk root, next;
	pChunk curr = ADDR_TO_CHUNK(addr);
	uint32_t size = curr->size;

	if (free_list[size] == NULL) {
		root = (pFreeChunk)malloc(FREE_CHUNK_SIZE);
		INIT_LIST_HEAD(&root->list);
		free_list[size] = root;
	} else {
		root = free_list[size];
	}

	pFreeChunk el = malloc(FREE_CHUNK_SIZE);
	el->addr = curr;

	list_add(&el->list, &root->list);
	assert((void *)el->list.prev == (void *)root
		&& (void *)root->list.next == (void *)el);
}

static void *find_chunk_in_freelist(uint32_t size)
{
	pFreeChunk root;
	pChunk res;

	root = free_list[size];
	if (root == NULL)
		return NULL;

	pFreeChunk p = list_next_entry(root, list);

	// root node
	if ((p - root) == 0)
		return NULL;

	assert(p->addr != NULL);

	// remove chunk from freelist
	list_del(&p->list);
	res = p->addr;
	free(p);

	return CHUNK_TO_ADDR(res);
}

static int expand_pool_ifnecessary(uint32_t size)
{
	void *p;
	pPool new_pool;
	uint32_t increased_size;

	if (gPool == NULL ||
		(gPool->address == NULL ||
		 gPool->remain <= size + MEM_CHUNK_SIZE))
	{
		new_pool = malloc(MEM_POOL_SIZE);
		// clear memory space explicitly.
		memset(new_pool, MEM_POOL_SIZE, 0);
		// expand pool with executable permission
		// TODO : make executable new allocated memory space.
		p = sbrk(EXPAND_POOL_SIZE);
		if (p < 0) {
			printf("EXPAND FAILED\n");
			return -1;
		}
		increased_size = sbrk(0) - p;
		new_pool->address = p;
		new_pool->size = increased_size;
		new_pool->remain = increased_size;
		gPool = new_pool;
		list_add(&gPool->list, &gPools);
	}

	assert(gPool != NULL);
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

	assert(gPool != NULL);
	p = gPool->address + gPool->pos;

	// make initialize node
	if (current_node == NULL)
		current_node = p;

	(*(pChunk)p).size = size;
	(*(pChunk)p).prev = current_node;
	(*(pChunk)p).next = p;

	// keep current node to link next.
	current_node->next = p;
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

	assert(p != NULL && ADDR_TO_CHUNK(p)->size != 0);
	return p;
}

void emfree(void *addr)
{
	assert(addr != NULL);
	assert(ADDR_TO_CHUNK(addr)->size != 0);
	append_addr_to_freelist(addr);
}

/*
 *	it is depends on OS policy that whether
 *	new allocated memory space will be initialized.
 *	so, it must be cleared in explicitly.
 */
static void __attribute__((constructor)) init()
{
	free_list = malloc(FREE_CHUNK_SIZE * 64);
	gPools.next = &gPools;
	gPools.prev = &gPools;
	// clear memory space explicitly.
	memset(free_list, MEM_CHUNK_SIZE * 64, 0);
}
