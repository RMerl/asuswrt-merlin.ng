#include <common.h>

int test_types(void)
{
	uintptr_t uintptr = 0;
	uint64_t uint64 = 0;
	u64 u64_val = 0;

	printf("uintptr = %lu\n", uintptr);
	printf("uint64 = %llu\n", uint64);
	printf("u64 = %llu\n", u64_val);
}
