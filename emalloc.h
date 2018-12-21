#include <stdint.h>
#include <assert.h>
#include "utils/utils.h"
#include "utils/list.h"

struct emalloc_mem_pool;
typedef struct emalloc_mem_pool *pPool;

struct emalloc_mem_chunk;
struct emalloc_free_chunk;
typedef struct emalloc_mem_chunk *pChunk;
typedef struct emalloc_free_chunk *pFreeChunk;

struct emalloc_mem_chunk
{
	uint16_t size;
	pChunk prev;
	pChunk next;
};

struct emalloc_mem_pool
{
	struct list_head list;
	void *address;
	uint32_t size;
	uint32_t pos;
	uint32_t remain;
};

struct emalloc_free_chunk
{
	struct list_head list;
	pChunk addr;
};

#define MEM_CHUNK_SIZE 	sizeof(struct emalloc_mem_chunk)
#define FREE_CHUNK_SIZE sizeof(struct emalloc_free_chunk)
#define MEM_POOL_SIZE	sizeof(struct emalloc_mem_pool)

#define ADDR_TO_CHUNK(addr)			\
({						\
	pChunk temp;				\
	temp = addr - MEM_CHUNK_SIZE;		\
	temp;					\
})

#define CHUNK_TO_ADDR(chunk)				\
({							\
 	void *addr;					\
	addr = ((void *)chunk) + MEM_CHUNK_SIZE;	\
	addr;						\
})

#define ADDR_PREV_CHUNK(addr)				\
({							\
	pChunk prev;					\
 	prev = ((pChunk)(addr - MEM_CHUNK_SIZE))->prev;	\
	prev;						\
 })

// memory pool list
struct list_head gPools;
// current memory pool
pPool gPool;

/*
 *	array & linked-list
 *	to keep the freed chunk list.
 */
pFreeChunk* free_list;

#define MIN_ALLOC_SIZE 		 1
#define MAX_ALLOC_SIZE 		63
#define EXPAND_POOL_SIZE	32 * 40000

void *emalloc(uint32_t size);
void emfree(void *addr);

struct list_head* get_pools();


