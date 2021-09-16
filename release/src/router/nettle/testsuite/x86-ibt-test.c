#include "testutils.h"
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) \
    && defined(__CET__) && defined(__linux__)
#include <signal.h>

static void
segfault_handler(int signo)
{
  exit(0);
}

static void
ibt_violation(void)
{
#ifdef __i386__
  unsigned int reg;
  asm volatile("lea 1f, %0\n\t"
	       "jmp *%0\n"
	       "1:" : "=r" (reg));
#else
  unsigned long long reg;
  asm volatile("lea 1f(%%rip), %0\n\t"
	       "jmp *%0\n"
	       "1:" : "=r" (reg));
#endif
}

#ifdef __i386__
static unsigned int
_get_ssp(void)
{
  unsigned int ssp;
  asm volatile("xor %0, %0\n\trdsspd %0" : "=r" (ssp));
  return ssp;
}
#else
static unsigned long long
_get_ssp(void)
{
  unsigned long long ssp;
  asm volatile("xor %0, %0\n\trdsspq %0" : "=r" (ssp));
  return ssp;
}
#endif

void
test_main(void)
{
   /* NB: This test should trigger SIGSEGV on CET platforms.  _get_ssp
      returns the address of shadow stack pointer.  If the address of
      shadow stack pointer is 0, SHSTK is disabled and we assume that
      IBT is also disabled.  */
  if (_get_ssp() == 0)
    {
      ibt_violation();
      SKIP();
    }

  signal(SIGSEGV, segfault_handler);
  ibt_violation();
  FAIL();
}
#else
void
test_main(void)
{
  SKIP();
}
#endif
