/*
 * Shell-like utility functions
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: shutils.h 625086 2016-03-15 12:51:47Z $
 */

#ifndef _shutils_h_
#define _shutils_h_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>	//pid_t
#include <rtconfig.h>
#include <errno.h>
#include <linux/unistd.h>       /* for _syscallX macros/related stuff */
#include <sys/sysinfo.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>

#ifndef MAX_NVPARSE
#define MAX_NVPARSE 16
#endif
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif /* min */

#define ENC_XOR     (0x74)
#define DATA_WORDS_LEN (120)
#define ENC_WORDS_LEN  (384)
#define ASUSRT_STACKSIZE        0x200000

extern int doSystem(char *fmt, ...);

/*
 * Reads file and returns contents
 * @param	fd	file descriptor
 * @return	contents of file or NULL if an error occurred
 */
extern char * fd2str(int fd);

/*
 * Reads file and returns contents
 * @param	path	path to file
 * @return	contents of file or NULL if an error occurred
 */
extern char * file2str(const char *path);

/* 
 * Waits for a file descriptor to become available for reading or unblocked signal
 * @param	fd	file descriptor
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @return	1 if descriptor changed status or 0 if timed out or -1 on error
 */
extern int waitfor(int fd, int timeout);

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	path	NULL, ">output", or ">>output"
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
extern int _eval(char *const argv[], const char *path, int timeout, pid_t *ppid);

/*
 * Evaluate cmds using taskset while SMP.
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @param	cmds	command argument list
 * The normal command elements protype is as [cpu0/cpu1], [cmd_arg0, cmd_arg1, ..., NULL]
 * If smp defined, it should specify cpu0/cpu1 at the fist element,
 * if it is not specified, cpu0 will be the default choice.
 * On UP case, no need to specify cpu0/1, otherwise will be ignored. 
 */
#define CPU0	"0"
#define CPU1	"1"
#define CPU2	"2"
#define CPU3	"3"

extern int _cpu_eval(int *ppid, char *cmds[]);

/*
 * Evaluate cmds: taskset [mask] arg0, arg1, ...
 * @param	mask	CPU mask
 * other params are same as _eval.
 */
extern int _cpu_mask_eval(char *const argv[], const char *path, int timeout, pid_t *ppid, unsigned int mask);
/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @return	stdout of executed command or NULL if an error occurred
 */
//	extern char * _backtick(char *const argv[]);

/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file
 * @return	0 on success and errno on failure
 */
extern int kill_pidfile(char *pidfile);
extern int kill_pidfile_s(char *pidfile, int sig);
extern int kill_pidfile_s_rm(char *pidfile, int sig, int rm);

/*
 * fread() with automatic retry on syscall interrupt
 * @param	ptr	location to store to
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully read
 */
extern int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param	ptr	location to read from
 * @param	size	size of each element of data
 * @param	nmemb	number of elements
 * @param	stream	file stream
 * @return	number of items successfully written
 */
extern int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * Convert Ethernet address string representation to binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
extern int ether_atoe(const char *a, unsigned char *e);

/*
 * Convert Ethernet address binary data to string representation
 * @param	e	binary data
 * @param	a	string in xx:xx:xx:xx:xx:xx notation
 * @return	a
 */
extern char * ether_etoa(const unsigned char *e, char *a);

/*
 * Concatenate two strings together into a caller supplied buffer
 * @param	s1	first string
 * @param	s2	second string
 * @param	buf	buffer large enough to hold both strings
 * @return	buf
 */
static inline char * strcat_r(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}	

/* Strip trailing CR/NL from string <s> */
#define strip_new_line(s) ({					\
	char *end = (s) + strlen(s) -1;				\
	while((end >= (s)) && (*end == '\n' || *end == '\r'))	\
		*end-- = '\0';					\
	s;							\
})

/* Strip trailing CR/NL from string <s> and space ' '. */
#define chomp(s) ({ \
	char *c = (s) + strlen((s)) - 1; \
	while ((c > (s)) && (*c == '\n' || *c == '\r' || *c == ' ')) \
		*c-- = '\0'; \
	s; \
})

/* skip the space ' ' in front of s (string) */
#define skip_space(p)	{if(p != NULL){ while(isspace(*p)) p++;}}

/* Simple version of _eval() (no timeout and wait for child termination) */
#if 1
#define eval(cmd, args...) ({ \
	char * const argv[] = { cmd, ## args, NULL }; \
	_eval(argv, NULL, 0, NULL); \
})
#else
#define eval(cmd, args...) ({ \
	char * const argv[] = { cmd, ## args, NULL }; \
	_eval(argv, ">/dev/console", 0, NULL); \
})
#endif

/* another _cpu_eval form */
#define cpu_eval(ppid, cmd, args...) ({ \
	char * argv[] = { cmd, ## args, NULL }; \
	_cpu_eval(ppid, argv); \
})

/* CMWIFI */
#ifndef foreach
#define _FOREACH_MACRO_

/* Copy each token in wordlist delimited by @sep into word */
#define __foreach(word, wordlist, next, sep) \
		for (next = &wordlist[strspn(wordlist, (sep))], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, (sep))] = '\0', \
				next = strchr(next, *(sep)); \
				strlen(word); \
				next = next ? &next[strspn(next, (sep))] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, (sep))] = '\0', \
				next = strchr(next, *(sep)))

/* Copy each token in wordlist delimited by space into word */
#define foreach(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, " ")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, " ")] = '\0', \
				next = strchr(next, ' '); \
				strlen(word); \
				next = next ? &next[strspn(next, " ")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, " ")] = '\0', \
				next = strchr(next, ' '))
#endif // endif

/* Copy each token in wordlist delimited by ascii_44 into word */
#define foreach_44(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, ",")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ",")] = '\0', \
				next = strchr(next, ','); \
				strlen(word); \
				next = next ? &next[strspn(next, ",")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ",")] = '\0', \
				next = strchr(next, ','))

/* Copy each token in wordlist delimited by ascii_58 into word */
#define foreach_58(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, ":")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ":")] = '\0', \
				next = strchr(next, ':'); \
				strlen(word); \
				next = next ? &next[strspn(next, ":")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ":")] = '\0', \
				next = strchr(next, ':'))

/* Copy each token in wordlist delimited by ascii_59 into word */
#define foreach_59(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, ";")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ";")] = '\0', \
				next = strchr(next, ';'); \
				strlen(word); \
				next = next ? &next[strspn(next, ";")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ";")] = '\0', \
				next = strchr(next, ';'))

/* Copy each token in wordlist delimited by ascii_60 into word */
#define foreach_60(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, "<")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = strchr(next, '<'); \
				strlen(word); \
				next = next ? &next[strspn(next, "<")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = strchr(next, '<'))

/* Copy each token in wordlist delimited by ascii_62 into word */
#define foreach_62(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, ">")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ">")] = '\0', \
				next = strchr(next, '>'); \
				strlen(word); \
				next = next ? &next[strspn(next, ">")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ">")] = '\0', \
				next = strchr(next, '>'))

/* Copy each token in wordlist delimited by ascii_124 into word */
#define foreach_124(word, wordlist, next) \
		for (next = &wordlist[strspn(wordlist, "|")], \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "|")] = '\0', \
				next = strchr(next, '|'); \
				strlen(word); \
				next = next ? &next[strspn(next, "|")] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "|")] = '\0', \
				next = strchr(next, '|'))

/* Copy each token in wordlist delimited by space into word and keep empty string */
#define foreach_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, ' '), \
				next = strchr(wordlist, ' '), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, " ")] = '\0', \
				next = next ? strchr(next, ' ') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, " ")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, " ")] = '\0', \
				next = strchr(next, ' '), \
				count--)

/* Copy each token in wordlist delimited by ascii_44 into word and keep empty string */
#define foreach_44_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, ','), \
				next = strchr(wordlist, ','), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ",")] = '\0', \
				next = next ? strchr(next, ',') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, ",")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ",")] = '\0', \
				next = strchr(next, ','), \
				count--)

/* Copy each token in wordlist delimited by ascii_58 into word and keep empty string */
#define foreach_58_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, ':'), \
				next = strchr(wordlist, ':'), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ":")] = '\0', \
				next = next ? strchr(next, ':') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, ":")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ":")] = '\0', \
				next = strchr(next, ':'), \
				count--)

/* Copy each token in wordlist delimited by ascii_59 into word and keep empty string */
#define foreach_59_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, ';'), \
				next = strchr(wordlist, ';'), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ";")] = '\0', \
				next = next ? strchr(next, ';') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, ";")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ";")] = '\0', \
				next = strchr(next, ';'), \
				count--)

/* Copy each token in wordlist delimited by ascii_60 into word and keep empty string */
#define foreach_60_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, '<'), \
				next = strchr(wordlist, '<'), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = next ? strchr(next, '<') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, "<")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "<")] = '\0', \
				next = strchr(next, '<'), \
				count--)

/* Copy each token in wordlist delimited by ascii_62 into word and keep empty string */
#define foreach_62_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, '>'), \
				next = strchr(wordlist, '>'), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ">")] = '\0', \
				next = next ? strchr(next, '>') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, ">")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, ">")] = '\0', \
				next = strchr(next, '>'), \
				count--)

/* Copy each token in wordlist delimited by ascii_124 into word and keep empty string */
#define foreach_124_keep_empty_string(count, word, wordlist, next) \
		for (count = get_char_count(wordlist, '|'), \
				next = strchr(wordlist, '|'), \
				strncpy(word, wordlist, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "|")] = '\0', \
				next = next ? strchr(next, '|') : ""; \
				count >= 0; \
				next = next ? &next[strcspn(next, "|")+1] : "", \
				strncpy(word, next, sizeof(word)), \
				word[sizeof(word) - 1] = '\0', \
				word[strcspn(word, "|")] = '\0', \
				next = strchr(next, '|'), \
				count--)

/* Return NUL instead of NULL if undefined */
#define safe_getenv(s) (getenv(s) ? : "")

static inline int safe_atoi(const char *s)
{
    return (int)strtol(s, NULL, 10);
}

#define ONE_ENTRANT()                               \
do {                                                            \
	static int served = 0;  \
	if(served ++ > 0)       \
		return;         \
} while (0)

//#define dbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } else fprintf(stderr, fmt, ## args); } while (0)
extern void dbg(const char * format, ...);
#define dbG(fmt, args...) dbg("%s(0x%04x): " fmt , __FUNCTION__ , __LINE__, ## args)

/*
 * Parse the unit and subunit from an interface string such as wlXX or wlXX.YY
 *
 * @param	ifname	interface string to parse
 * @param	unit	pointer to return the unit number, may pass NULL
 * @param	subunit	pointer to return the subunit number, may pass NULL
 * @return	Returns 0 if the string ends with digits or digits.digits, -1 otherwise.
 *		If ifname ends in digits.digits, then unit and subuint are set
 *		to the first and second values respectively. If ifname ends
 *		in just digits, unit is set to the value, and subunit is set
 *		to -1. On error both unit and subunit are -1. NULL may be passed
 *		for unit and/or subuint to ignore the value.
 */
extern int get_ifname_unit(const char* ifname, int *unit, int *subunit);

/* This utility routine builds the wl prefixes from wl_unit.
 * Input is expected to be a null terminated string
 *
 * @param       prefix          Pointer to prefix buffer
 * @param       prefix_size     Size of buffer
 * @param       Mode            If set generates unit.subunit output
 *                              if not set generates unit only
 * @param       ifname          Optional interface name string
 *
 *
 * @return                              pointer to prefix, NULL if error.
 */
extern char* make_wl_prefix(char *prefix, int prefix_size, int mode, char *ifname);

/*
 * Set the ip configuration index given the eth name
 * Updates both wlXX_ipconfig_index and lanYY_ifname.
 *
 * @param	eth_ifname 	pointer to eth interface name
 * @return	0 if successful -1 if not.
 */
extern int set_ipconfig_index(char *eth_ifname, int index);

/*
 * Get the ip configuration index if it exists given the
 * eth name.
 *
 * @param	wl_ifname 	pointer to eth interface name
 * @return	index or -1 if not found
 */
extern int get_ipconfig_index(char *eth_ifname);

/*
 * Get interfaces belonging to a specific bridge.
 *
 * @param	bridge_name 	pointer to bridge interface name
 * @return	list on interfaces beloging to the bridge
 */
extern char *
get_bridged_interfaces(char *bridge_name);

/*
		remove_from_list
		Remove the specified word from the list.

		@param name word to be removed from the list
		@param list List to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
extern int _remove_from_list(const char *name, char *list, int listsize, char deli);
extern int remove_from_list(const char *name, char *list, int listsize);

/*
		add_to_list
		Add the specified interface(string) to the list as long as
		it will fit in the space left in the list.

		@param name Name of interface to be added to the list
		@param list List to modify
		@param listsize Max size the list can occupy

		@return	error code
*/
extern int add_to_list(const char *name, char *list, int listsize);

/* Compare two space-separated/null-terminated lists(str1 and str2)
 * NOTE : The individual names in the list should not exceed NVRAM_MAX_VALUE_LEN
 *
 * @param      str1    space-separated/null-terminated list
 * @param      str2    space-separated/null-terminated list
 *
 * @return     0 if both strings are same else return -1
 */
extern int compare_lists(char *str1, char *str2);

extern char *_find_in_list(const char *haystack, const char *needle, char deli);
extern char *find_in_list(const char *haystack, const char *needle);

extern char *find_next_in_list(const char *haystack, const char *needle,
        char *nextstr, int nextstrlen);

extern char *remove_dups(char *inlist, int inlist_size);

extern int nvifname_to_osifname(const char *nvifname, char *osifname_buf,
				int osifname_buf_len);
extern int osifname_to_nvifname(const char *osifname, char *nvifname_buf,
				int nvifname_buf_len);

int ure_any_enabled(void);

#define is_hwnat_loaded() module_loaded("hw_nat")

#define vstrsep(buf, sep, args...) _vstrsep(buf, sep, args, NULL)
extern int _vstrsep(char *buf, const char *sep, ...);

/* Buffer structure for collecting string-formatted data
 * using str_bprintf() API.
 * Use str_binit() to initialize before use
 */
struct strbuf {
        char *buf;              /* pointer to current position in origbuf */
        unsigned int size;      /* current (residual) size in bytes */
        char *origbuf;          /* unmodified pointer to orignal buffer */
        unsigned int origsize;  /* unmodified orignal buffer size in bytes */
};

typedef struct ping_result_s
{
	int name_valid;
	char alias[160];
	char ip_addr[64];
	int data_valid;
	unsigned long pkt_sent;
	unsigned long pkt_recv;
	double pkt_loss_rate;
	double min;
	double avg;
	double max;
}ping_result_t;

extern void str_binit(struct strbuf *b, char *buf, unsigned int size);
extern int str_bprintf(struct strbuf *b, const char *fmt, ...);
extern int generate_wireless_key(unsigned char *key);

extern int strArgs(int argc, char **argv, char *fmt, ...);
extern char *trimNL(char *str);
extern char *trimWS(char *str);
extern int get_char_count(char *str, int ch);
extern pid_t get_pid_by_name(char *name);
extern pid_t get_pid_by_thrd_name(char *name);
extern void replace_null_to_space(char *str, int len);
extern pid_t get_pid_by_process_name(char *name);
extern char *get_process_name_by_pid(const int pid);
extern char *ether_etoa2(const unsigned char *e, char *a);
extern char *ATE_FACTORY_MODE_STR();
extern char *ATE_UPGRADE_MODE_STR();
extern int hex2str(unsigned char *hex, char *str, int hex_len);
extern void reset_stacksize(int new_stacksize);
extern int arpcache(char *tgmac, char *tgip);
extern int ether_inc(unsigned char *e, const unsigned char n);
#ifdef RTCONFIG_AMAS
extern int check_if_exist_ifnames(char *need_check_ifname, char *ifname);
#endif
extern long get_sys_uptime();
extern void wait_ntp_repeat(unsigned long usec, unsigned int count);
extern int ping_target_with_size(char *target, unsigned int size, unsigned int count, unsigned int wait_time, double loss_rate);
extern int parse_ping_content(char *fname, ping_result_t *out);
extern int replace_literal_newline(char *inputstr, char *output, int buflen);
#endif /* _shutils_h_ */
