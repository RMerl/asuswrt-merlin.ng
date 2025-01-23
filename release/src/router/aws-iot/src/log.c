#ifndef NDEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "api.h"
#include "time_util.h"
#include <errno.h>
#include <syslog.h>
#define LOG_PATH_LEN 250 
#define LOG_PATH_EXT "%Y-%m-%d_%H%M%S"
FILE* g_file_fp = NULL;
FILE* g_console_fp = NULL;
const char* ident =  "awsiot";
int logopt = LOG_PID | LOG_CONS;
int facility = LOG_USER;
int g_stream_type =0;
int priority = LOG_ERR | LOG_USER;
int g_is_log_opened =0;
long FILE_MAX_SIZE = 524288; //- 512K

#define API_DBG 1

int fileExist(char *fname)
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
    g_stream_type = stream_type;
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
    if ((stream_type & FILE_TYPE) == FILE_TYPE) {

        // //char* ts;
        // //alloc_time_string(LOG_PATH_EXT, 0, &ts);
        // char *path;
        // int len = strlen(log_path)/*+strlen(ts)*/+2;
        // path = malloc(len); 
        // memset(path, 0, len);
        // strcpy(path,log_path );
        // //strcat(path, ts);
        // //      sprintf(path,"%s%s", log_path,ts);
        // //dealloc_time_string(ts);
        // g_file_fp = fopen(path, "w+");
        // if(path) free(path);
        // g_is_log_opened = 1;
        // g_stream_type |= FILE_TYPE;


        char* ts;
        alloc_time_string(LOG_PATH_EXT, 0, &ts);
        char *path;
        int len = strlen(log_path)+strlen(ts)+2;
        path = malloc(len);
        memset(path, 0, len);
        strcpy(path,log_path );
        // strcat(path, ts);
        //      sprintf(path,"%s%s", log_path,ts);
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

    long filesize = file_size(FEEDBACK_LOG_PATH);

    // if File > 512K , downsizing -> 1200 line
    if(filesize > FILE_MAX_SIZE) {
        feedback_log_downsizing();
    }

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
    //va_list ap;
    if(!g_is_log_opened)
        return;
    if (enable) {
        char* ts;
        alloc_time_string(NULL, 1, &ts);


        //va_start(ap, fmt);
        // Log to file
        
        if (g_file_fp) {
            fprintf(g_file_fp, WHERESTR, ts, file, func, line);
            vfprintf(g_file_fp, fmt, ap);
            fprintf(g_file_fp, "\n");
            fflush(g_file_fp);
        }

        // if (fileExist(AWS_DEBUG_TO_FILE)) {
        //     if (g_file_fp) {
        //         fprintf(g_file_fp, WHERESTR, ts, file, func, line);
        //         vfprintf(g_file_fp, fmt, ap);
        //         fprintf(g_file_fp, "\n");
        //         fflush(g_file_fp);
        //     }
        // }

        // Log to console
        if (fileExist(AWS_DEBUG_TO_CONSOLE)) {
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
        if (level == 1 || fileExist(AWS_DEBUG_TO_SYSLOG)) {
            vsyslog(priority, fmt, ap);
            // fprintf(stdout, WHERESTR, ts, file, func, line);
            // vfprintf(stdout, fmt, ap);
            // fprintf(stdout, "\n");
        }
        
        //va_end(ap);
        dealloc_time_string(ts);
    }
}



long file_size(const char* filename) 
{

  FILE *fp=fopen(filename,"r"); 
  if(!fp) return -1; 
  fseek(fp,0L,SEEK_END); 
  long size = ftell(fp); 
  fclose(fp); 
  return size; 
} 



void feedback_log_downsizing() {

// int file_number = get_file_number(FEEDBACK_LOG_PATH);

// if(file_number >= 1200) {

  FILE *fp;

  char file_info[1023];

  if ((fp = fopen(FEEDBACK_LOG_PATH, "r")) == NULL) {

      Cdbg(API_DBG, "%s open_file_error", FEEDBACK_LOG_PATH);

      return;
  }
  

  FILE *fpTmp = fopen(FEEDBACK_LOG_TMP_PATH, "a+");

  if( NULL == fpTmp ){
    Cdbg(API_DBG, "write [%s] file error -> open failure", FEEDBACK_LOG_TMP_PATH);
    return;
  }

  int i = 0;

  while(fgets(file_info, 1023, fp) != NULL)
  {

      i++;

      // Cdbg(API_DBG, "%s", file_info);

      if(i <= 2000) {

        continue;

      } else {

        fprintf(fpTmp, "%s", file_info);

      }

  }

  fclose(fpTmp);

  fclose(fp);

    // remove FEEDBACK_LOG_TMP_PATH to FEEDBACK_LOG_PATH
  if(remove(FEEDBACK_LOG_PATH) == 0 ) {

    char cmd[128];

    snprintf(cmd, 128, "mv %s %s", FEEDBACK_LOG_TMP_PATH, FEEDBACK_LOG_PATH);
    int ret = system(cmd);
    if(ret == 0) {
      Cdbg(API_DBG, "feedback log [%s] rename to [%s] successfully ", FEEDBACK_LOG_TMP_PATH, FEEDBACK_LOG_PATH);
    } else {
      Cdbg(API_DBG, "Error: [%s] unable rename to [%s]", FEEDBACK_LOG_TMP_PATH, FEEDBACK_LOG_PATH);
    }
  } else {
    Cdbg(API_DBG, "error : remove [%s] file fail", FEEDBACK_LOG_PATH);
  }


// } 

}


// void dprintf_impl(const char* file,const char* func, size_t line, int enable, const char* fmt, ...)
// {
//     printf("g_is_log_opened : %d \n" ,  g_is_log_opened);
//     printf("enable : %d \n" ,  enable);
//     printf("g_file_fp : %d \n" ,  g_file_fp);

//     va_list ap;
//     char* ts;
//     if(!g_is_log_opened)
//         return;
//     if (enable) {

//         alloc_time_string(NULL, 1, &ts);
//         va_start(ap, fmt);
//         // Log to file
//         if (fileExist(AWS_DEBUG_TO_FILE)) {
//             if (g_file_fp) {
//                 fprintf(g_file_fp, WHERESTR, ts, file, func, line);
//                 vfprintf(g_file_fp, fmt, ap);
//                 fprintf(g_file_fp, "\n");
//                 fflush(g_file_fp);
//             }
//         }

//         // Log to console
//         if (fileExist(AWS_DEBUG_TO_CONSOLE)) {
//             if (g_console_fp) {
//                 fprintf(g_console_fp, WHERESTR, ts, file, func, line);
//                 vfprintf(g_console_fp, fmt, ap);
//                 fprintf(g_console_fp, "\n");
//             }
//             else {
//                 fprintf(stderr, WHERESTR, ts, file, func, line);
//                 vfprintf(stderr, fmt, ap);
//                 fprintf(stderr, "\n");
//             }
//         } else {
//             fprintf(stdout, WHERESTR, ts, sizeof(file), file, sizeof(func), func, line);
//             vfprintf(stdout, fmt, ap);
//             fprintf(stdout, "\n");
//         }

//         // Log to syslog
//         if (fileExist(AWS_DEBUG_TO_SYSLOG)) {
//             vsyslog(priority, fmt, ap);
//         }
        
//         // va_end(ap);
//         dealloc_time_string(ts);
//     }
// }
    #if 0
void get_fp(const char* log_path)
{
    FILE* fp = NULL;
    if (!log_path) {
        //	fp = stderr;
        gfp = stderr;
    } else {
//		char path [LOG_PATH_LEN]={0};
        char* ts;
        alloc_time_string(LOG_PATH_EXT, 0, &ts);
        int len = strlen(log_path)+strlen(ts)+2;
        char* path = malloc(len); memset(path, 0, len);
        strcpy(path,log_path );
        strcat(path, ts);
//		sprintf(path,"%s%s", log_path,ts);
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


