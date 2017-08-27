/* Test case for bug 659866 */

#define _POSIX_C_SOURCE 199309L
#undef _GNU_SOURCE
#undef _XOPEN_SOURCE
#include <pthread.h>
#include <glib.h>

int
main (int argc, char *argv[])
{
  GRWLock lock;

  g_rw_lock_init (&lock);
  g_rw_lock_clear (&lock);

  return 0;
}
