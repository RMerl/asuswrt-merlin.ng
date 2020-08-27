#ifndef NDEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <log.h>
#include <time_util.h>
#include <errno.h>
#include <syslog.h>
#define LOG_PATH_LEN 150 
#define LOG_PATH_EXT "%Y-%m-%d_%H%M%S"
FILE* g_file_fp = NULL;
FILE* g_console_fp = NULL;
const char* ident =  "uploader";
char g_log_file[LOG_PATH_LEN];
int logopt = LOG_PID | LOG_CONS;
int facility = LOG_USER;
int g_stream_type =0;
int priority = LOG_ERR | LOG_USER;
int g_is_log_opened =0;

int fileExist(char *fname)
{
    struct stat fstat;
    
    if (lstat(fname,&fstat)==-1)
        return 0;
    if (S_ISREG(fstat.st_mode))
        return 1;
    
    return 0;
}

int open_file_log() {
    char* ts;
    printf("%s~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", g_log_file);
    alloc_time_string(LOG_PATH_EXT, 0, &ts);
    char *path;
    int len = strlen(g_log_file)+strlen(ts)+2;
    path = malloc(len); memset(path, 0, len);
    strcpy(path,g_log_file );
    strcat(path, ts);
    //      sprintf(path,"%s%s", g_log_file,ts);
    dealloc_time_string(ts);
    g_file_fp = fopen(path, "w+");
    if (g_file_fp == NULL)
        printf("open_log path=%s, ret=%s\n", path, strerror(errno));
    if(path) free(path);
    g_is_log_opened = 1;
    g_stream_type |= FILE_TYPE;
}

int open_log(const char* log_path, int stream_type )
{
    g_stream_type = stream_type;
    memset(g_log_file, 0, sizeof(g_log_file));
    memcpy(g_log_file, log_path, sizeof(g_log_file) < strlen(log_path) ? sizeof(g_log_file) : strlen(log_path));
    if ((stream_type & SYSLOG_TYPE) == SYSLOG_TYPE) {
        openlog(ident, logopt, facility);
        g_is_log_opened = 1;
        g_stream_type = SYSLOG_TYPE;
    }
    if ((stream_type & SYSLOG_TYPE) == STD_ERR) {
        gfp = stderr;
        g_is_log_opened = 1;
        g_stream_type |= STD_ERR;
    }
    if ((stream_type & SYSLOG_TYPE) == FILE_TYPE) {
        open_file_log();
    }
    if ((stream_type & CONSOLE_TYPE) == CONSOLE_TYPE) {
        int nfd;
        if ((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) > 0) {
            g_console_fp = fdopen(nfd, "w");
            g_stream_type |= CONSOLE_TYPE;
            g_is_log_opened = 1;
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

void dprintf_impl(const char* file,const char* func, int line, int enable, const char* fmt, ...)
{
    va_list ap;
    char* ts;
    if(!g_is_log_opened)
        return;
    if (enable) {
        alloc_time_string(NULL, 1, &ts);
        va_start(ap, fmt);
        // Log to file
        if (fileExist(UPLOADER_DEBUG_TO_FILE)) {
            if (!g_file_fp) // check if file opened.
                open_file_log();
            if (g_file_fp) {
                fprintf(g_file_fp, WHERESTR, ts, file, func, (int)line);
                vfprintf(g_file_fp, fmt, ap);
                fprintf(g_file_fp, "\n");
                fflush(g_file_fp);
            }
        }

        // Log to console
        if (fileExist(UPLOADER_DEBUG_TO_CONSOLE)) {
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
        } else {
            fprintf(stdout, WHERESTR, ts, file, func, line);
            vfprintf(stdout, fmt, ap);
            fprintf(stdout, "\n");
        }

        // Log to syslog
        //if (fileExist(UPLOADER_DEBUG_TO_SYSLOG)) {
        //    vsyslog(priority, fmt, ap);
        //}
        
        va_end(ap);
        dealloc_time_string(ts);
    }
}
    #if 0
void get_fp(const char* log_path)
{
    FILE* fp = NULL;
    if (!log_path) {
        //  fp = stderr;
        gfp = stderr;
    } else {
//      char path [LOG_PATH_LEN]={0};
        char* ts;
        alloc_time_string(LOG_PATH_EXT, 0, &ts);
        int len = strlen(log_path)+strlen(ts)+2;
        char* path = malloc(len); memset(path, 0, len);
        strcpy(path,log_path );
        strcat(path, ts);
//      sprintf(path,"%s%s", log_path,ts);
        dealloc_time_string(ts);
        fp = fopen(path, "w+");
        if (!fp) {
            fprintf(stderr, "App open log path failed errno=%d", errno);
            gfp = stderr;
        } else {
            gfp = fp;
        }
        if (path) free(path);
    }
}
    #endif
//void closefp(FILE* fp)

#endif


