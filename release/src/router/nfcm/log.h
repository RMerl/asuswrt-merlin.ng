#ifndef __LOG_H__
#define __LOG_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtohchanspec(i) (g_swap?dtoh16(i):i)

#ifndef ETHER_ADDR_LENGTH
#define ETHER_ADDR_LENGTH 18 // 11:22:33:44:55:66
#endif

#ifndef IFNAMESIZE
#define IFNAMESIZE        16
#endif

extern int wl_ioctl(char *name, int cmd, void *buf, int len);
extern int wl_iovar_getint(char *ifname, char *iovar, int *val);

extern char *mac2str(const unsigned char *e, char *a);
extern bool is_in_lanv4(struct in_addr *src);

extern void getFormattedTime(char * const p, int sz);
extern int mylog(const char* fmt, ...);

#ifdef _WIN32
#define __SHORT_FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __SHORT_FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define  ___LOG___(fmt,level,path, ...) do {\
	/* using local var and using a long name to avoid conflict*/ \
	char LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___[24];\
	getFormattedTime(LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___,\
		sizeof(LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___));\
	mylog("[%d] %s <%s> [%s:%d] [%s] " fmt "\n", \
		getpid(), \
		LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___, \
		level,\
		path,\
		__LINE__, \
		__FUNCTION__, \
		## __VA_ARGS__);\
} while(0)

#define  trace(fmt, ...) ___LOG___(fmt, "TRACE",__SHORT_FILE__, ## __VA_ARGS__)
#define  debug(fmt, ...) ___LOG___(fmt, "DEBUG",__SHORT_FILE__, ## __VA_ARGS__)
#define   info(fmt, ...) ___LOG___(fmt,  "INFO",__SHORT_FILE__, ## __VA_ARGS__)
#define   warn(fmt, ...) ___LOG___(fmt,  "WARN",__SHORT_FILE__, ## __VA_ARGS__)
#define  error(fmt, ...) ___LOG___(fmt, "ERROR",__SHORT_FILE__, ## __VA_ARGS__)

#define tracel(fmt, ...) ___LOG___(fmt, "TRACE",	  __FILE__, ## __VA_ARGS__)
#define debugl(fmt, ...) ___LOG___(fmt, "DEBUG",	  __FILE__, ## __VA_ARGS__)
#define  infol(fmt, ...) ___LOG___(fmt,  "INFO",	  __FILE__, ## __VA_ARGS__)
#define  warnl(fmt, ...) ___LOG___(fmt,  "WARN",	  __FILE__, ## __VA_ARGS__)
#define errorl(fmt, ...) ___LOG___(fmt, "ERROR",	  __FILE__, ## __VA_ARGS__)

#endif /* __LOG_H__ */
