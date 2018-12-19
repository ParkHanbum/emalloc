
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

static uint8_t getRand()
{
	return rand() & 0xFFFF;
}

static uint32_t getAllocSize()
{
	uint32_t res;
	res = 0x1000 * getRand();
	return res;
}

int main()
{
	void *p1, *p2, *p3, *p4;
	void *malloc;
	uint32_t size;

	srand(time(NULL));

	for(int i=0;i < 5;i++)
	{
		printf("==========================================\n");
		printf("[MEM]\t");
		size = getAllocSize();
		p1 = sbrk(size);
		printf("%p - %p\t", p1, p1 + size);
		size = getAllocSize();
		p2 = sbrk(size);
		printf("%p - %p\n", p2, p2 + size);

		printf("[MEM]\t");
		size = getAllocSize();
		p3 = sbrk(size);
		printf("%p - %p\t", p3, p3 + size);
		size = getAllocSize();
		p4 = sbrk(size);
		printf("%p - %p\n", p4, p4 + size);
	}
}
