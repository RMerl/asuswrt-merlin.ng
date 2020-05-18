#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*#ifdef IO_NEEDS_STRPTIME */
char *io_strptime(char *buf, char *fmt, struct tm *tm);
/*#endif*/

#ifdef __cplusplus
}
#endif
