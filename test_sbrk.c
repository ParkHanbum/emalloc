
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

void alloc_and_check(uint32_t sz)
{
	void *p = sbrk(sz);
	printf("check memory region : %p\n", p);
	p = sbrk(0);
	printf("check memory region : %p\n", p);
}

int main()
{
	alloc_and_check(0x91000);
	alloc_and_check(0xFC000);
	alloc_and_check(0x25000);
	alloc_and_check(0xC2000);
	alloc_and_check(0xD8000);
	alloc_and_check(0xA7000);
	alloc_and_check(0x72000);
	alloc_and_check(0x32000);
}
