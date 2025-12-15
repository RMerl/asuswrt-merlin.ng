#include "testutils.h"
#include "knuth-lfib.h"
#include "memops.h"

static void
cnd_memcpy_for_test(int cnd, void *dst, const void *src, size_t n)
{
  /* Makes valgrind trigger on any branches depending on the input
     data. */
  mark_bytes_undefined (n, dst);
  mark_bytes_undefined (n, src);
  mark_bytes_undefined (sizeof(cnd), &cnd);

  cnd_memcpy (cnd, dst, src, n);
  mark_bytes_defined (n, src);
  mark_bytes_defined (n, dst);
}

#define MAX_SIZE 50
void
test_main(void)
{
  uint8_t src[MAX_SIZE];
  uint8_t dst[MAX_SIZE];
  uint8_t res[MAX_SIZE];
  struct knuth_lfib_ctx random_ctx;

  knuth_lfib_init (&random_ctx, 11);

  size_t size;
  for (size = 1; size < 50; size++)
    {
      knuth_lfib_random (&random_ctx, size, src);
      knuth_lfib_random (&random_ctx, size, dst);
      memcpy (res, dst, size);
      cnd_memcpy_for_test (0, res, src, size);

      ASSERT (memcmp (res, dst, size) == 0);
      cnd_memcpy_for_test (1, res, src, size);
      ASSERT (memcmp (res, src, size) == 0);
    }
}
