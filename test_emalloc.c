#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "emalloc.h"
#include "utils/utils.h"
#include "utils/list.h"

#include "utils/hashmap.h"

#define ASSERT(TEST) if((TEST)) asm("int $3");
#define TEST_COUNT	0x10000

extern pPool gPool;
extern struct list_head gPools;
extern pFreeChunk* free_list;

void validate_alloc_addr(void *p);

static Hashmap *map;

typedef struct chunk_info
{
	void *addr;
	uint16_t size;
} cinfo;

void print_pool_info()
{
	pPool entry, tmp;
	uint32_t total_size = 0, total_remain = 0;
	char separator[61];
	memset(separator, '=', sizeof(separator)-1);

	printf("%s\n", separator);

	printf("%15s%15s%10s%10s%10s\n",
		"ADDR", "MEM", "SIZE", "POS", "REMAIN");
	printf("%s\n", separator);

	list_for_each_entry_safe(entry, tmp, &gPools, list) {
		printf("%15p%15p%10d%10d%10d\n",
		entry,
		entry->address, entry->size,
		entry->pos, entry->remain);
		total_size += entry->size;
		total_remain += entry->remain;
	}
	printf("%s\n", separator);
	printf("TOTAL : %*s%10d%*s%10d\n",
			22, "",
			total_size,
			10, "",
			total_remain);
}

void print_free_list(int index)
{
	int i, total = 0;
	pFreeChunk freelist;
	pFreeChunk entry;

	if (index == 0) {
		for(i = MIN_ALLOC_SIZE; i < MAX_ALLOC_SIZE; i++)
		{
			freelist = free_list[i];
			if (freelist == NULL)
				continue;

			printf("[%02d] =========================\n", i);
			int count = 0;
			list_for_each_entry(entry, &freelist->list, list) {
				printf("%p\t", entry->addr);
				count++;
			}
			printf("\n[%02d][COUNT:%05d] =========================\n", i, count);
			total += count;
		}
	} else {
		freelist = free_list[index];

		printf("[%02d] =========================\n", index);
		int count = 0;
		list_for_each_entry(entry, &freelist->list, list) {
			printf("%p\t", entry->addr);
			count++;
		}
		printf("\n[%02d][COUNT:%05d] =========================\n", index, count);
		total += count;

	}
	printf("[TOTAL COUNT: %09d]\n", total);
}

static uint16_t get_alloc_size()
{
	uint16_t res;
	res = rand() & MAX_ALLOC_SIZE-1;
	return res > 0 ? res : 1;
}

static bool get_rand_bool()
{
	return rand() % 2;
}

pChunk validate_free_addr(void *p)
{
	int i;
	pFreeChunk freelist, entry;
	pChunk res = NULL;
	for(i = MIN_ALLOC_SIZE; i < MAX_ALLOC_SIZE; i++)
	{
		freelist = free_list[i];
		if (freelist == NULL)
			continue;

		list_for_each_entry(entry, &freelist->list, list) {
			if (entry->addr == ADDR_TO_CHUNK(p)) {
				res = entry->addr;
				assert(i == res->size);
			}
		}
	}

	assert(res != NULL);
	return res;
}

bool iterator_print(void* key, void* value, void* context)
{
	cinfo *ent = (cinfo *)value;
	printf("[%p] %d\n", ent->addr, ent->size);

	return true;
}

void print_maps()
{
	hashmapForEach(map, iterator_print, map);
}

bool iterator_free_all(void* key, void* value, void* context)
{
	cinfo *ent = (cinfo *)value;
	void *p = ent->addr;
	emfree(p);

	return true;
}


bool iterator_validate_free(void* key, void* value, void* context)
{
	cinfo *ent = (cinfo *)value;
	int16_t size = ent->size;
	void *p = ent->addr;
	bool found = false;

	pFreeChunk freelist, entry;
	freelist = free_list[size];

	assert(freelist != NULL);

	list_for_each_entry(entry, &freelist->list, list) {
		if (entry->addr == ADDR_TO_CHUNK(p)) {
			found = true;
		}
	}

	assert(found);
	return true;
}

void do_freetest(int fixed_size)
{
	void *p;
	uint16_t size;
	int i;

	p = NULL;

	map = hashmapCreate(50000, hashmapDefaultHash, hashmapDefaultEquals);

	for(i = 0;i < TEST_COUNT;i++) {
		if (fixed_size > 0) {
			size = fixed_size;
		} else {
			size = get_alloc_size();
		}
		p = emalloc(size);
		// this address must be same with address that freed before.
		assert(p != NULL);
		validate_alloc_addr(p);
		cinfo *info = malloc(sizeof(cinfo));
		info->addr = p;
		info->size = size;

		// check the key has already exist in hashmap before putting it.
		if (hashmapContainsKey(map, &info->addr)) {
			printf("This address already exist\n");
			info = (cinfo *)hashmapGet(map, &info->addr);
			printf("[INFO] %p %d\n", info->addr, info->size);
		}
		hashmapPut(map, &info->addr, info);
	}

	size_t map_size = hashmapSize(map);
	assert(map_size == TEST_COUNT);

	hashmapForEach(map, iterator_free_all, map);
	// hashmapForEach(map, iterator_validate_free, map);

	pFreeChunk freelist, entry;
	for(i = MIN_ALLOC_SIZE; i < MAX_ALLOC_SIZE; i++)
	{
		freelist = free_list[i];
		if (freelist == NULL)
			continue;

		list_for_each_entry(entry, &freelist->list, list) {
			void *p = CHUNK_TO_ADDR(entry->addr);
			assert(hashmapContainsKey(map, &p));
		}
	}
}

void do_freetest_fixed_size()
{
	do_freetest(MIN_ALLOC_SIZE);
	do_freetest(MAX_ALLOC_SIZE);
}

void do_freetest_rand_size()
{
	do_freetest(0);
}


void validate_alloc_addr(void *p)
{
	assert(p != NULL);
	pChunk prev = ADDR_PREV_CHUNK(p);
	pChunk curr = ADDR_TO_CHUNK(p);
	assert(curr->prev == prev && prev->next == curr);
}

void do_alloctest()
{
	void *p;
	uint16_t size;
	int i;

	for(i = 0;i < TEST_COUNT;i++) {
		size = 32;
		p = emalloc(size);
		validate_alloc_addr(p);
	}
}

void do_random_alloctest()
{
	void *p;
	uint16_t size;
	int i;

	for(i = 0;i < TEST_COUNT;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		validate_alloc_addr(p);
	}
}


int main()
{
	srand(time(NULL));

	do_alloctest();
	do_random_alloctest();
	do_freetest_fixed_size();
	do_freetest_rand_size();
	print_pool_info();
}
