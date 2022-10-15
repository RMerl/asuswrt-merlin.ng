#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define LOG_DBG	1

//#define NDEBUG

// steam type could be 
// 	0 	for syslog
//	1.	for stderr
//	2	for file
#define SYSLOG_TYPE 	1
#define STD_ERR		2
#define FILE_TYPE	4
#define CONSOLE_TYPE	8


#define CONNDIAG_LOG_PATH						"/tmp/conndiag_log"
#define CONNDIAG_LOG_TEMP_PATH			"/tmp/conndiag_tmp_log"



#define LOG_DEBUG_TO_FILE 		"/tmp/CONNDIAG_DEBUG_FILE"
#define LOG_DEBUG_TO_CONSOLE 	"/tmp/CONNDIAG_DEBUG_CONSOLE"
#define LOG_DEBUG_TO_SYSLOG 		"/tmp/CONNDIAG_DEBUG_SYSLOG"
#define DEBUG_LOG_FILE_MAX_SIZE 	1048576

// #endif


void dprintf_impl(const char* file,const char* func, size_t line, int enable, const char* fmt, ...);
void dprintf_impl2(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, va_list ap);
void dprintf_to_file(const char* file,const char* func, size_t line, int enable, const char* fmt, ...);
void dprintf_virtual(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, ...);
int open_log(const char* log_path, int stream_type);

long get_file_size(const char* filename);
int check_file_exist(char *fname);
void debug_log_downsizing();
char* alloc_time_string(const char* tf, int is_msec, char** time_string);
void  dealloc_time_string(char* ts);

//void closefp(FILE* fp);
void close_log();
extern FILE* gfp;
//FILE* gfp =NULL;
// #define WHERESTR "%23s >> [%*s] >> func > %*s, line %i : "
#define WHERESTR "[%23s][%25s] <<%28s>>, line %zu: "
#define WHEREARG  __FILE__,__func__,__LINE__

#ifndef NDEBUG


#define Cdbg(enable, ...) dprintf_to_file(WHEREARG, enable, __VA_ARGS__)
// #define Cdbg(enable, ...) dprintf_impl(WHEREARG, enable, __VA_ARGS__)

#define Cdbg2(enable, level, ...) dprintf_virtual(WHEREARG, enable, level, __VA_ARGS__)

#else
#define Cdbg(enable, ...) // define to nothing in release mode
#define Cdbg2(enable, level, ...) // define to nothing in release mode

#endif


#define CF_OPEN(path, type)				open_log(path, type);
#define CF_CLOSE()				close_log();	



#endif

