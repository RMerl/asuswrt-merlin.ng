#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include <bcmnvram.h>
#include <sys/stat.h>
#include <shared.h>  
#include <assert.h>


int asusdebuglog(int level, char *path, int conlog, int showtime, unsigned filesize, const char *msgfmt, ...);
int rm_asusdebuglog(char *path);

/* output file*/
#define LOG_NOTHING -1
#define LOG_CUSTOM 0
#define LOG_SYSTEMLOG 1
#define LOG_CONSOLE 2

/* show time*/
#define LOG_SHOWTIME 1
#define LOG_NOSHOWTIME 0


 /* priorities (these are ordered) */
//#define LOG_EMERG 0 /* system is unusable */
//#define LOG_ALERT 1 /* action must be taken immediately */
//#define LOG_CRIT 2 /* critical conditions */
//#define LOG_ERR  3 /* error conditions */
//#define LOG_WARNING 4 /* warning conditions */
//#define LOG_NOTICE 5 /* normal but significant condition */
//#define LOG_INFO 6 /* informational */
//#define LOG_DEBUG 7 /* debug-level messages */
