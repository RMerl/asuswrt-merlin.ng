#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#if defined(RTCONFIG_NOTIFICATION_CENTER) && (defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA))
/* IFTTT DEBUG DEFINE SETTING 
---------------------------------*/
#define COMMON_IFTTT_DEBUG                     "/tmp/IFTTT_ALEXA"
#define COMMON_IFTTT_LOG_FILE                  "/tmp/IFTTT_ALEXA.log"

extern int isFileExist(char *fname);

#define IFTTT_DEBUG(fmt,args...) \
	if(isFileExist(COMMON_IFTTT_DEBUG) > 0) { \
		Debug2File(COMMON_IFTTT_LOG_FILE, "[Tunnel][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}
#endif
// Enable the debugging for the category.
#define WB_DBG	1
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
#define STDOUT_TYPE	16

#define WB_DEBUG_TO_FILE "/tmp/WB_DEBUG_FILE"
#define WB_DEBUG_TO_CONSOLE "/tmp/WB_DEBUG_CONSOLE"
#define WB_DEBUG_TO_SYSLOG "/tmp/WB_DEBUG_SYSLOG"
#define WB_DEBUG_TO_STDOUT "/tmp/WB_DEBUG_STDOUT"

#if defined(RTCONFIG_NOTIFICATION_CENTER) && (defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA))
void Debug2File(const char *FilePath, const char * format, ...);
#endif

void dprintf_impl(const char* file,const char* func, size_t line, int enable, const char* fmt, ...);
void dprintf_impl2(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, va_list ap);
void dprintf_virtual(const char* file,const char* func, size_t line, int enable, int level, const char* fmt, ...);
int open_log(const char* log_path, int stream_type);
//void closefp(FILE* fp);
void close_log();
#define WHERESTR "[%32s][%25s] <<%38s>>, line %i: "
#define WHEREARG  __FILE__,__func__,__LINE__

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

