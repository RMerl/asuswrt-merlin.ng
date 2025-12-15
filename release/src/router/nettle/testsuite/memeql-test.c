#include "testutils.h"
#include "knuth-lfib.h"
#include "memops.h"

static int
memeql_sec_for_test(const void *a, const void *b, size_t n)
{
  int res;

  /* Makes valgrind trigger on any branches depending on the input
     data. */
  mark_bytes_undefined (n, a);
  mark_bytes_undefined (n, b);

  res = memeql_sec (a, b, n);
  mark_bytes_defined (sizeof(res), &res);
  return res;
}

#define MAX_SIZE 50
void
test_main(void)
{
  uint8_t orig[MAX_SIZE];
  uint8_t a[MAX_SIZE];
  uint8_t b[MAX_SIZE];
  struct knuth_lfib_ctx random_ctx;

  knuth_lfib_init (&random_ctx, 11);

  size_t size;
  for (size = 0; size < 50; size++)
    {
      size_t i;
      uint8_t bit;
      knuth_lfib_random (&random_ctx, size, orig);
      memcpy (a, orig, size);
      memcpy (b, orig, size);
      ASSERT (memeql_sec_for_test (a, b, size));
      for (i = 0; i < size; i++)
	for (bit = 0x80; bit; bit >>= 1)
	  {
	    b[i] = orig[i] ^ bit;
	    ASSERT (!memeql_sec_for_test (a, b, size));
	    b[i] = orig[i];
	  }
    }
}
