#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "emalloc.h"
#include "utils/utils.h"
#include "utils/list.h"

#define ASSERT(TEST) if((TEST)) asm("int $3");
#define TEST_COUNT	0x2000

extern pPool gPool;
extern struct list_head gPools;
extern pFreeChunk* free_list;

void validate_alloc_addr(void *p);

void print_pool_info()
{
	pPool entry, tmp;

	printf("%10s%10s%10s%10s%10s\n",
		"ADDR", "MEM", "SIZE", "POS", "REMAIN");
	list_for_each_entry_safe(entry, tmp, &gPools, list) {
		printf("%10p%10p%10lx%10lx%10lx\n",
		entry,
		entry->address, entry->size,
		entry->pos, entry->remain);
	}
}

void print_free_list()
{
	int i, total = 0;
	pFreeChunk freelist;
	pFreeChunk entry;
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

void do_freetest()
{
	void *p;
	uint16_t size;
	int i;
	void *keep[TEST_COUNT];

	p = NULL;

	for(i = 0;i < TEST_COUNT;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		// this address must be same with address that freed before.
		assert(p != NULL);
		validate_alloc_addr(p);
		keep[i] = p;
	}
	for(i = 0;i < TEST_COUNT;i++) {
		emfree(keep[i]);
		p = validate_free_addr(keep[i]);
	}
}

void do_freetest_fixed_size()
{
	void *p;
	uint16_t size;
	int i;
	void *keep[TEST_COUNT];

	p = NULL;
	size = 32;

	for(i = 0;i < TEST_COUNT;i++) {
		size = get_alloc_size();
		p = emalloc(size);
		// this address must be same with address that freed before.
		assert(p != NULL);
		validate_alloc_addr(p);
		keep[i] = p;
	}
	for(i = 0;i < TEST_COUNT;i++) {
		emfree(keep[i]);
		p = validate_free_addr(keep[i]);
	}
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
	do_freetest();
	do_freetest_fixed_size();
	print_pool_info();
}
