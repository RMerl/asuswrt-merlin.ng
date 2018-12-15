#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>

/* one provided by Aaaron Wiebe based on perl's hashing algorithm
 * (so probably pretty generic). Not for excessively large strings!
 */
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#endif
static unsigned __attribute__((nonnull(1))) int
#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
hash_from_string(void *k)
{
	char *rkey = (char*) k;
	unsigned hashval = 1;

	while (*rkey)
		hashval = hashval * 33 + *rkey++;

	return hashval;
}

int main(int argc, char *argv[])
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	if(argc != 2) {
		fprintf(stderr, "usage: test_id test-file-name\n");
		exit(1);
	}
	printf("%06ld_%04.4x", tv.tv_usec, hash_from_string(argv[1]));

	return 0;
}
