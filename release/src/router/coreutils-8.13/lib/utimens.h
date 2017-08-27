#include <time.h>
int fdutimens (int, char const *, struct timespec const [2]);
int utimens (char const *, struct timespec const [2]);
int lutimens (char const *, struct timespec const [2]);

#if GNULIB_FDUTIMENSAT
# include <fcntl.h>
# include <sys/stat.h>

int fdutimensat (int fd, int dir, char const *name, struct timespec const [2],
                 int atflag);

/* Using this function makes application code slightly more readable.  */
static inline int
lutimensat (int dir, char const *file, struct timespec const times[2])
{
  return utimensat (dir, file, times, AT_SYMLINK_NOFOLLOW);
}
#endif
