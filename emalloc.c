#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct ealloc_mem_chunk;
typedef struct ealloc_mem_chunk * pChunk;
uint32_t pChunk_size = sizeof(pChunk);

struct ealloc_mem_chunk
{
	uint16_t size;
	pChunk prev;
	pChunk next;
};

struct ealloc_mem_pool
{
	void *address;
	uint32_t size;
	uint32_t pos;
	uint32_t remain;
};

struct ealloc_mem_pool *gEalloc_pool;

/*
 *	array & linked-list
 *	freed chunk list.
 */
pChunk* free_list;

void remove_chunk_from_freelist(pChunk p)
{
	uint32_t size = p->size;
	pChunk node, prev, next;

	if (free_list[size] == NULL) {
		printf("Something wrong \n"); exit(1);
	}

	node = free_list[size];
	while(node) {
		node = node->next;

		if (!memcmp(node, p, sizeof(struct ealloc_mem_chunk))) {
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

void append_chunk_to_freelist(pChunk p)
{
	uint32_t size = p->size;
	pChunk root, next;

	if (free_list[size] == NULL) {
		root = (pChunk)malloc(sizeof(struct ealloc_mem_chunk));
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

void efree(void *addr)
{
	pChunk p = (pChunk)(addr - sizeof(struct ealloc_mem_chunk));
	append_chunk_to_freelist(p);
}

void *find_chunk_in_freelist(uint32_t size)
{
	pChunk node;

	node = free_list[size];
	if (node == NULL)
		goto fin;

	node = node->next;
	if (node != NULL && size == node->size)
		return node + sizeof(struct ealloc_mem_chunk);

fin:
	return NULL;

}


#define EXPAND_POOL_SIZE	32 * 40000
int expand_pool_ifnecessary(uint32_t size)
{
	void *p;
	uint32_t increased_size;

	if (gEalloc_pool->address == NULL ||
		gEalloc_pool->remain <= size + pChunk_size)
	{
		// expand pool with executable permission
		p = sbrk(EXPAND_POOL_SIZE);
		if (p < 0) {
			printf("EXPAND FAILED\n");
			return -1;
		}
		gEalloc_pool->address = p;
		increased_size = sbrk(0) - p;

		gEalloc_pool->size += increased_size;
		gEalloc_pool->remain += increased_size;
	}

	return 0;
}

void *create_new_chunk(uint32_t size)
{
	static pChunk current_node;
	uint32_t allocated_size;
	void *p;

	if (expand_pool_ifnecessary(size)) {
		printf("expand failed\n"); exit(1);
	}

	p = gEalloc_pool->address + gEalloc_pool->pos;
	(*(pChunk)p).size = size;
	(*(pChunk)p).prev = current_node;
	(*(pChunk)p).next = NULL;

	if (current_node != NULL)
		current_node->next = p;

	// keep current node to link next.
	current_node = p;

	allocated_size = pChunk_size + size;
	gEalloc_pool->pos += allocated_size;
	gEalloc_pool->remain -= allocated_size;

	return p + pChunk_size;
}

#define MIN_ALLOC_SIZE 1
#define MAX_ALLOC_SIZE 64


void *ealloc(uint32_t size)
{
	void *p;

	if (!(MIN_ALLOC_SIZE < size &&
		size < MAX_ALLOC_SIZE))
	{
		return NULL;
	}

	if (gEalloc_pool == NULL) {
		gEalloc_pool = malloc(sizeof(struct ealloc_mem_pool));
	}

	p = find_chunk_in_freelist(size);
	if (p == NULL)
		p = create_new_chunk(size);

	return p;
}

void __attribute__((constructor)) init()
{
	free_list = malloc(pChunk_size * 64);
}


int main()
{
	void *p = ealloc(32);
	printf("check memory region : %p\n", p);
}

