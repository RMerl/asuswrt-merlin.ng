#include "testutils.h"
#include "knuth-lfib.h"
#include "memops.h"

#if HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
static void
cnd_memcpy_for_test(int cnd, void *dst, const void *src, size_t n)
{
  /* Makes valgrind trigger on any branches depending on the input
     data. */
  VALGRIND_MAKE_MEM_UNDEFINED (dst, n);
  VALGRIND_MAKE_MEM_UNDEFINED (src, n);
  VALGRIND_MAKE_MEM_UNDEFINED (&cnd, sizeof(cnd));

  cnd_memcpy (cnd, dst, src, n);
  VALGRIND_MAKE_MEM_DEFINED (src, n);
  VALGRIND_MAKE_MEM_DEFINED (dst, n);
}
#else
#define cnd_memcpy_for_test cnd_memcpy
#endif

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
