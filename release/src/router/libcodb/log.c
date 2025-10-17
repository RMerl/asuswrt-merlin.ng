#include <stdio.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "cosql_utils.h"
#include "log.h"
#include "codb_config.h"

int isFileExist(char *fname)
{
	struct stat fstat;
	
	if (lstat(fname,&fstat)==-1)
		return 0;
	if (S_ISREG(fstat.st_mode))
		return 1;
	
	return 0;
}

#define TIME_LEN	64
#define DEFAULT_FMT "%Y-%m-%d_%H:%M:%S"

char* alloc_codb_time_string(const char* tf, int is_msec, char** time_string)
{
    struct timeval tv;
    struct tm* ptm;
    char*  mtf = NULL;
    if(!tf) mtf = DEFAULT_FMT;
    else    mtf = (char *)tf;
    *time_string = (char*) malloc(TIME_LEN);
    memset(*time_string, 0, TIME_LEN);
    long milliseconds;

    /* Obtain the time of day, and convert it to a tm struct. */
    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    /* Format the date and time, down to a single second. */
    strftime (*time_string, TIME_LEN, mtf, ptm);
    /* Compute milliseconds from microseconds. */
    milliseconds = tv.tv_usec / 1000;
    /* Print the formatted time, in seconds, followed by a decimal point
    *    and the milliseconds. */
    if(is_msec) {
        char msec [8] ; memset(msec, 0, 8);
        sprintf (msec, ".%03ld", milliseconds);
        strcat(*time_string, msec);
    }
//  fprintf(stderr, "time string =%s", time_string );
    return *time_string; 
}

void dealloc_codb_time_string(char* ts)
{
    if (ts) free(ts);
}

void codb_dprintf_impl(const char* file, const char* func, size_t line, const char* fmt, ...) 
{
	va_list ap;
	char* ts;
	alloc_codb_time_string(NULL, 1, &ts);

	if (isFileExist(LIBCODB_DEBUG_TO_FILE)) {
		int nfd = open("/tmp/libcodb_log", O_WRONLY | O_APPEND | O_CREAT);
		if(nfd >= 0){
			dprintf(nfd, WHERESTR, ts, file, func, line);
			va_start(ap, fmt);
			vdprintf(nfd, fmt, ap);
			dprintf(nfd, "\n");
			va_end(ap);
			close(nfd);
		}
	}
	else {
       	fprintf(stderr, WHERESTR, ts, file, func, line);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		va_end(ap);
    }
	
	dealloc_codb_time_string(ts);
}