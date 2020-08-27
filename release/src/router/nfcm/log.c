#include <time.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

void getFormattedTime(char * const p, int sz) 
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	//strftime(p, sz, "%Y-%m-%d %H:%M:%S", timeinfo);
	//strftime(p, sz, "%H:%M:%S", timeinfo);

	strftime(p, sz, "%T", timeinfo);
	//%T	 The time in 24-hour notation (%H:%M:%S)
}

int mylog(const char* fmt, ...) 
{
	int retval;
	// TODO: log to file also.
	// TODO: create a new log file daily
	va_list argptr;
	va_start(argptr, fmt);
	retval = vfprintf(stderr, fmt, argptr); //log to stderr
	va_end(argptr);

	return retval;
}

