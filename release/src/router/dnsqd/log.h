#ifndef _LOG_H_
#define _LOG_H_

#ifndef ETHER_ADDR_LENGTH
#define ETHER_ADDR_LENGTH 18 // 11:22:33:44:55:66
#endif

#ifndef IFNAMESIZE
#define IFNAMESIZE        16
#endif

#define __SHORT_FILE__ __FILE__
#define DNSSTR "%s, %i: "
#define DNSARGS  __func__,__LINE__
#define dnsdbg(fmt, ...) ___LOG___(fmt,  "INFO",__SHORT_FILE__, ## __VA_ARGS__)
#define DNS_DEBUG_TO_FILE "/tmp/DNS_DEBUG_TO_FILE"

extern void getFormattedTime(char * const p, int sz);
extern void my_printf(const char *format, ...);
#define  ___LOG___(fmt,level,path, ...) do {\
	/* using local var and using a long name to avoid conflict*/ \
	char LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___[24];\
	getFormattedTime(LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___,\
		sizeof(LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___));\
	my_printf("[%d] %s <%s> [%s:%d] [%s] " fmt "\n", \
		getpid(), \
		LAgGV3nzJsTholGvGL2eTNXmhsqYe___xxooxxoo___, \
		level,\
		path,\
		__LINE__, \
		__FUNCTION__, \
		## __VA_ARGS__);\
} while(0)

extern int isFileExist(char *fname);

#endif
