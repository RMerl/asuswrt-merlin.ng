#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>

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

// nf_printf is copid from cprintf() in shutils.c
// nf_printf will print messages to console
void nf_printf(const char *format, ...)
{
	FILE *f;
	int nfd;
	va_list args;

    int debug_cprintf = nvram_get_int("debug_cprintf");
	if (debug_cprintf == 1) {
		if((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) >= 0) {
            // fdopen will associate nfd and f, so just close f is fine
			if((f = fdopen(nfd, "w")) != NULL) {
				va_start(args, format);
				vfprintf(f, format, args);
				va_end(args);
				fclose(f);
			} else
				close(nfd);
		}
	}
}

//! Byte swap unsigned short
uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

//! Byte swap short
int16_t swap_int16(int16_t val)
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32(int32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

uint64_t swap_uint48(uint64_t val)
{
	val = val & 0x0000FFFFFFFFFFFFULL;

    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);

    return ((val << 32) | (val >> 32)) >> 16;
}

int64_t swap_int64(int64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

uint64_t swap_uint64(uint64_t val)
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
}
