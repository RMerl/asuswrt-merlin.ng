#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>
// Enable the debugging for the category.
#define AWS_DBG	1
//#define NDEBUG
//void dprintf_impl(const char* date,const char* time,const char* file,const char* func, size_t line, int enable, const char* fmt, ...);

// steam type could be 
// 	0 	for syslog
//	1.	for stderr
//	2	for file
#define SYSLOG_TYPE 	1
#define STD_ERR		2
#define FILE_TYPE	4
#define CONSOLE_TYPE	8

#define APP_LOG_PATH	"/jffs/awsiot_log"
#define AWS_DEBUG_TO_FILE "/tmp/AWSIOT_DEBUG_FILE"
#define AWS_DEBUG_TO_CONSOLE "/tmp/AWSIOT_DEBUG_CONSOLE"
#define AWS_DEBUG_TO_SYSLOG "/tmp/AWSIOT_DEBUG_SYSLOG"


#define DBG_LOG_PATH                       "/jffs/awsiot_log"
#define DBG_LOG_TMP_PATH                   "/jffs/awsiot_log_tmp"


void dprintf_impl(const char* file,const char* func, size_t line, int enable, const char* fmt, ...);
void dprintf_impl2(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, va_list ap);
void dprintf_virtual(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, ...);
int open_log(const char* log_path, int stream_type);

//void closefp(FILE* fp);
void close_log();
extern FILE* gfp;
//FILE* gfp =NULL;

#define WHERESTR "[%28s][%25s] <<%38s>>, line %i: "
#define WHEREARG  __FILE__,__func__,__LINE__

long file_size(const char* filename);
void dbg_log_downsizing();

#ifndef NDEBUG
//#define WHERESTR "[%s][%s][%s] <<%s>>, line %i: "
//#define WHEREARG  __DATE__,__TIME__,__FILE__,__func__,__LINE__
//#define Cdbg(fmt, ...)  fprintf(stderr,WHERESTR  fmt "\n", WHEREARG	, __VA_ARGS__)

//#define Cdbg(enable, ...) dprintf_impl(WHEREARG, enable, 0,  __VA_ARGS__)
#define Cdbg(enable, ...) dprintf_impl(WHEREARG, enable, __VA_ARGS__)
#define Cdbg2(enable, level, ...) dprintf_virtual(WHEREARG, enable, level, __VA_ARGS__)

#else
#define Cdbg(enable, ...) // define to nothing in release mode
#define Cdbg2(enable, level, ...) // define to nothing in release mode

#endif

//#define CF(enable, path, ...) dprintf_impl(WHEREARG, enable, 1,  __VA_ARGS__)
#define CF_OPEN(path, type)				open_log(path, type);
#define CF_CLOSE()				close_log();	
#endif

