#ifndef NDEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "conn_diag_log.h"


#define TIME_LEN    64
#define DEFAULT_FMT "%Y-%m-%d_%H:%M:%S"



#define LOG_PATH_LEN 512
#define LOG_PATH_EXT "%Y-%m-%d_%H%M%S"
FILE* g_file_fp = NULL;
FILE* g_console_fp = NULL;
const char* ident =  "conn_diag";
int logopt = LOG_PID | LOG_CONS;
int facility = LOG_USER;
int g_stream_type =0;
int priority = LOG_ERR | LOG_USER;
int g_is_log_opened = 0;


int file_exist(char *fname)
{
    struct stat fstat;
    if (lstat(fname,&fstat)==-1)
        return 0;
    if (S_ISREG(fstat.st_mode))
        return 1;
   
    return 0;
}

int open_log(const char* log_path, int stream_type )
{


    if(g_is_log_opened)
        return 0;


    g_stream_type = stream_type;
    if ((stream_type & SYSLOG_TYPE) == SYSLOG_TYPE) {
        openlog(ident, logopt, facility);
        g_is_log_opened = 1;
        g_stream_type = SYSLOG_TYPE;
    }
    if ((stream_type & STD_ERR) == STD_ERR) {
        g_is_log_opened = 1;
        g_stream_type |= STD_ERR;
    }
    if ((stream_type & FILE_TYPE) == FILE_TYPE) {

        char* ts;
        alloc_time_string(LOG_PATH_EXT, 0, &ts);
        char *path;
        int len = strlen(log_path)+strlen(ts)+2;
        path = malloc(len);
        memset(path, 0, len);
        strcpy(path,log_path );

        dealloc_time_string(ts);
        // g_file_fp = fopen(path, "w+");
        g_file_fp = fopen(path, "a+");
        if(path) free(path);


        g_is_log_opened = 1;
        g_stream_type |= FILE_TYPE;
    }
    if ((stream_type & CONSOLE_TYPE) == CONSOLE_TYPE) {
        int nfd;
        if ((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) > 0) {
            g_console_fp = fdopen(nfd, "w");
            g_stream_type |= CONSOLE_TYPE;
        }
    }

    return 0;
}

void close_log()
{
    if(g_file_fp){
        fclose(g_file_fp);
        g_file_fp = NULL;
    }

    if (g_console_fp) {
        fclose(g_console_fp);
        g_console_fp = NULL;
    }

    if((g_stream_type & SYSLOG_TYPE) == SYSLOG_TYPE) 
        closelog();
}


void dprintf_impl(const char* file,const char* func, size_t line, int enable, const char* fmt, ...)
{

    va_list ap;
    va_start(ap, fmt);
    dprintf_impl2(file, func, line, enable, 0, fmt, ap);
    va_end(ap);
}


void dprintf_virtual(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    dprintf_impl2(file, func, line, enable, level, fmt, ap);
    va_end(ap);
}




void dprintf_impl2(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, va_list ap)
{



    if (enable) {
        char* ts;
        alloc_time_string(NULL, 1, &ts);

        // va_list ap;

        
        // Log to file
        if (file_exist(LOG_DEBUG_TO_FILE)) {
            if (g_file_fp) {
                fprintf(g_file_fp, WHERESTR, ts, file, func, line);
                // va_start(ap, fmt);
                vfprintf(g_file_fp, fmt, ap);
                // va_end(ap);
                fprintf(g_file_fp, "\n");
                fflush(g_file_fp);
            }
        }

        // Log to console
        if (file_exist(LOG_DEBUG_TO_CONSOLE)) {
            if (g_console_fp) {
                fprintf(g_console_fp, WHERESTR, ts, file, func, line);
                vfprintf(g_console_fp, fmt, ap);
                fprintf(g_console_fp, "\n");
            }
            else {
                fprintf(stderr, WHERESTR, ts, file, func, line);
                vfprintf(stderr, fmt, ap);
                fprintf(stderr, "\n");
            }
        }
        // Log to syslog
        if (file_exist(LOG_DEBUG_TO_SYSLOG)) {
            vsyslog(priority, fmt, ap);
        }
        
        // va_end(ap);
        dealloc_time_string(ts);
    }
}

void dprintf_to_file(const char* file,const char* func, size_t line, int enable, const char* fmt, ...)
{
    if (enable) {

        char* ts;
        alloc_time_string(NULL, 1, &ts);

        va_list ap;

        // Log to file
        if (file_exist(LOG_DEBUG_TO_FILE)) {


            long filesize = get_file_size(CONNDIAG_LOG_PATH);

            // printf("filesize =%ld\n", filesize);
            // if File > 1M
            if (filesize > DEBUG_LOG_FILE_MAX_SIZE) {
                // printf("filesize (%ld) > DEBUG_LOG_FILE_MAX_SIZE(%d)\n", filesize, DEBUG_LOG_FILE_MAX_SIZE);
                debug_log_downsizing();
            }


            int nfd = open(CONNDIAG_LOG_PATH, O_WRONLY | O_APPEND | O_CREAT);
            dprintf(nfd, WHERESTR, ts, file, func, line);
            va_start(ap, fmt);
            vdprintf(nfd, fmt, ap);
            va_end(ap);
            dprintf(nfd, "\n");
            close(nfd);

        }
        dealloc_time_string(ts);
    }
}


long get_file_size(const char* filename) {

  FILE *fp = fopen(filename,"r"); 
  if(!fp) {
    return -1;
  }

  fseek(fp,0L,SEEK_END); 
  long size = ftell(fp); 
  fclose(fp); 
  return size; 
} 


int check_file_exist(char *fname) {
    struct stat fstat;
    
    if (lstat(fname,&fstat)==-1)
        return 0;
    if (S_ISREG(fstat.st_mode))
        return 1;
    
    return 0;
}


void debug_log_downsizing() {

    FILE *fp;

    char log_line[1023];

    if ((fp = fopen(CONNDIAG_LOG_PATH, "r")) == NULL) {
        return;
    }
  
    FILE *fpTmp = fopen(CONNDIAG_LOG_TEMP_PATH, "a+");

    if (NULL == fpTmp) {
        return;
    }

    int i = 0;

    while (fgets(log_line, 1023, fp) != NULL) {
        i++;

        if (i <= 2000) {
            //- remove 2000 lines
            continue;
        } 
        else {
            fprintf(fpTmp, "%s", log_line);
        }
    }

    fclose(fpTmp);
    fclose(fp);

    unlink(CONNDIAG_LOG_PATH);

    // move CONNDIAG_LOG_TEMP_PATH to CONNDIAG_LOG_PATH
    if (!check_file_exist(CONNDIAG_LOG_PATH)) {
        char cmd[128];
        snprintf(cmd, 128, "mv %s %s", CONNDIAG_LOG_TEMP_PATH, CONNDIAG_LOG_PATH);
        system(cmd);
    }

    sleep(1);
    unlink(CONNDIAG_LOG_TEMP_PATH);
    
}




char* alloc_time_string(const char* tf, int is_msec, char** time_string)
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
    // fprintf(stderr, "time string =%s", *time_string );
    // printf("time string =%s\n", *time_string );
    return *time_string; 
}

char* alloc_utctime_string(const char* tf, int is_msec, char** time_string)
{
    struct timeval tv;
    struct tm *ptm;
    char*  mtf = NULL;
    if(!tf) mtf = DEFAULT_FMT;
    else    mtf = (char *)tf;
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
    // fprintf(stderr, "time string =%s", time_string );
    // printf("time string =%s\n", *time_string );
    return *time_string;
}

void dealloc_time_string(char* ts)
{   
    if (ts) free(ts);
}



#endif


