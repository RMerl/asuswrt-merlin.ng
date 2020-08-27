#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TIME_LEN	64
#define DEFAULT_FMT "%Y-%m-%d_%H:%M:%S"
char* alloc_time_string(const char* tf, int is_msec, char** time_string)
{
    struct timeval tv;
    struct tm* ptm;
    char*  mtf = NULL;
    if(!tf) mtf = DEFAULT_FMT;
    else    mtf = tf;
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

char* alloc_utctime_string(const char* tf, int is_msec, char** time_string)
{
    time_t t;
    struct timeval tv;
    struct tm *ptm;
    char*  mtf = NULL;
    if(!tf) mtf = DEFAULT_FMT;
    else    mtf = tf;
    *time_string = (char*) malloc(TIME_LEN);
    memset(*time_string, 0, TIME_LEN);
    long milliseconds;

    /* Obtain the time of day, and convert it to a tm struct. */
    gettimeofday (&tv, NULL);
    localtime (&tv.tv_sec);
    ptm = gmtime(&tv.tv_sec);
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

void dealloc_time_string(char* ts)
{
    if (ts) free(ts);
}
