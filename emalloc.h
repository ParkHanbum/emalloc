#include <stdint.h>

struct emalloc_mem_chunk;
struct emalloc_mem_pool;
typedef struct emalloc_mem_chunk *pChunk;
typedef struct emalloc_mem_pool *pPool;

struct emalloc_mem_chunk
{
	uint16_t size;
	pChunk prev;
	pChunk next;
};

struct emalloc_mem_pool
{
	void *address;
	uint32_t size;
	uint32_t pos;
	uint32_t remain;
};

#define MEM_CHUNK_SIZE 	sizeof(struct emalloc_mem_chunk)
#define MEM_POOL_SIZE	sizeof(struct emalloc_mem_pool)

pPool gPool;

/*
 *	array & linked-list
 *	to keep the freed chunk list.
 */
pChunk* free_list;



#define MIN_ALLOC_SIZE 		 1
#define MAX_ALLOC_SIZE 		63
#define EXPAND_POOL_SIZE	32 * 40000


void *emalloc(uint32_t size);
void emfree(void *addr);
