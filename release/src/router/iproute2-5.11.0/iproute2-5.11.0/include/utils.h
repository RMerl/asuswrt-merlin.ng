/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __UTILS_H__
#define __UTILS_H__ 1

#include <sys/types.h>
#include <asm/types.h>
#include <resolv.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#ifdef HAVE_LIBBSD
#include <bsd/string.h>
#endif

#include "libnetlink.h"
#include "ll_map.h"
#include "rtm_map.h"
#include "json_print.h"

extern int preferred_family;
extern int human_readable;
extern int show_stats;
extern int show_details;
extern int show_raw;
extern int resolve_hosts;
extern int oneline;
extern int brief;
extern int json;
extern int pretty;
extern int timestamp;
extern int timestamp_short;
extern const char * _SL_;
extern int max_flush_loops;
extern int batch_mode;
extern int numeric;
extern bool do_all;

#ifndef CONFDIR
#define CONFDIR		"/etc/iproute2"
#endif

#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

void incomplete_command(void) __attribute__((noreturn));

#define NEXT_ARG() do { argv++; if (--argc <= 0) incomplete_command(); } while(0)
#define NEXT_ARG_OK() (argc - 1 > 0)
#define NEXT_ARG_FWD() do { argv++; argc--; } while(0)
#define PREV_ARG() do { argv--; argc++; } while(0)

#define TIME_UNITS_PER_SEC	1000000
#define NSEC_PER_USEC 1000
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000LL

typedef struct
{
	__u16 flags;
	__u16 bytelen;
	__s16 bitlen;
	/* These next two fields match rtvia */
	__u16 family;
	__u32 data[64];
} inet_prefix;

enum {
	PREFIXLEN_SPECIFIED	= (1 << 0),
	ADDRTYPE_INET		= (1 << 1),
	ADDRTYPE_UNSPEC		= (1 << 2),
	ADDRTYPE_MULTI		= (1 << 3),

	ADDRTYPE_INET_UNSPEC	= ADDRTYPE_INET | ADDRTYPE_UNSPEC,
	ADDRTYPE_INET_MULTI	= ADDRTYPE_INET | ADDRTYPE_MULTI
};

static inline void inet_prefix_reset(inet_prefix *p)
{
	p->flags = 0;
}

static inline bool is_addrtype_inet(const inet_prefix *p)
{
	return p->flags & ADDRTYPE_INET;
}

static inline bool is_addrtype_inet_unspec(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_UNSPEC) == ADDRTYPE_INET_UNSPEC;
}

static inline bool is_addrtype_inet_multi(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_MULTI) == ADDRTYPE_INET_MULTI;
}

static inline bool is_addrtype_inet_not_unspec(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_UNSPEC) == ADDRTYPE_INET;
}

static inline bool is_addrtype_inet_not_multi(const inet_prefix *p)
{
	return (p->flags & ADDRTYPE_INET_MULTI) == ADDRTYPE_INET;
}

#define DN_MAXADDL 20
#ifndef AF_DECnet
#define AF_DECnet 12
#endif

struct dn_naddr
{
        unsigned short          a_len;
        unsigned char a_addr[DN_MAXADDL];
};

#ifndef AF_MPLS
# define AF_MPLS 28
#endif
#ifndef IPPROTO_MPLS
#define IPPROTO_MPLS	137
#endif

#ifndef CLOCK_TAI
# define CLOCK_TAI 11
#endif

#ifndef AF_XDP
# define AF_XDP 44
# if AF_MAX < 45
#  undef AF_MAX
#  define AF_MAX 45
# endif
#endif

__u32 get_addr32(const char *name);
int get_addr_1(inet_prefix *dst, const char *arg, int family);
int get_prefix_1(inet_prefix *dst, char *arg, int family);
int get_addr(inet_prefix *dst, const char *arg, int family);
int get_prefix(inet_prefix *dst, char *arg, int family);
int mask2bits(__u32 netmask);
int get_addr_rta(inet_prefix *dst, const struct rtattr *rta, int family);
int get_addr_ila(__u64 *val, const char *arg);

int read_prop(const char *dev, char *prop, long *value);
int get_hex(char c);
int get_integer(int *val, const char *arg, int base);
int get_unsigned(unsigned *val, const char *arg, int base);
int get_time_rtt(unsigned *val, const char *arg, int *raw);
#define get_byte get_u8
#define get_ushort get_u16
#define get_short get_s16
int get_s64(__s64 *val, const char *arg, int base);
int get_u64(__u64 *val, const char *arg, int base);
int get_u32(__u32 *val, const char *arg, int base);
int get_s32(__s32 *val, const char *arg, int base);
int get_u16(__u16 *val, const char *arg, int base);
int get_u8(__u8 *val, const char *arg, int base);
int get_be64(__be64 *val, const char *arg, int base);
int get_be32(__be32 *val, const char *arg, int base);
int get_be16(__be16 *val, const char *arg, int base);
int get_addr64(__u64 *ap, const char *cp);
int get_rate(unsigned int *rate, const char *str);
int get_rate64(__u64 *rate, const char *str);
int get_size(unsigned int *size, const char *str);

int hex2mem(const char *buf, uint8_t *mem, int count);
char *hexstring_n2a(const __u8 *str, int len, char *buf, int blen);
__u8 *hexstring_a2n(const char *str, __u8 *buf, int blen, unsigned int *len);
#define ADDR64_BUF_SIZE sizeof("xxxx:xxxx:xxxx:xxxx")
int addr64_n2a(__u64 addr, char *buff, size_t len);

int af_bit_len(int af);

const char *format_host_r(int af, int len, const void *addr,
			       char *buf, int buflen);
#define format_host_rta_r(af, rta, buf, buflen)	\
	format_host_r(af, RTA_PAYLOAD(rta), RTA_DATA(rta), \
		      buf, buflen)

const char *format_host(int af, int lne, const void *addr);
#define format_host_rta(af, rta) \
	format_host(af, RTA_PAYLOAD(rta), RTA_DATA(rta))
const char *rt_addr_n2a_r(int af, int len, const void *addr,
			       char *buf, int buflen);
const char *rt_addr_n2a(int af, int len, const void *addr);
#define rt_addr_n2a_rta(af, rta) \
	rt_addr_n2a(af, RTA_PAYLOAD(rta), RTA_DATA(rta))

int read_family(const char *name);
const char *family_name(int family);

void missarg(const char *) __attribute__((noreturn));
void invarg(const char *, const char *) __attribute__((noreturn));
void duparg(const char *, const char *) __attribute__((noreturn));
void duparg2(const char *, const char *) __attribute__((noreturn));
int nodev(const char *dev);
int check_ifname(const char *);
int check_altifname(const char *name);
int get_ifname(char *, const char *);
const char *get_ifname_rta(int ifindex, const struct rtattr *rta);
bool matches(const char *prefix, const char *string);
int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits);
int inet_addr_match_rta(const inet_prefix *m, const struct rtattr *rta);

const char *mpls_ntop(int af, const void *addr, char *str, size_t len);
int mpls_pton(int af, const char *src, void *addr, size_t alen);

extern int __iproute2_hz_internal;
int __get_hz(void);

static __inline__ int get_hz(void)
{
	if (__iproute2_hz_internal == 0)
		__iproute2_hz_internal = __get_hz();
	return __iproute2_hz_internal;
}

extern int __iproute2_user_hz_internal;
int __get_user_hz(void);

static __inline__ int get_user_hz(void)
{
	if (__iproute2_user_hz_internal == 0)
		__iproute2_user_hz_internal = __get_user_hz();
	return __iproute2_user_hz_internal;
}

static inline __u32 nl_mgrp(__u32 group)
{
	if (group > 31 ) {
		fprintf(stderr, "Use setsockopt for this group %d\n", group);
		exit(-1);
	}
	return group ? (1 << (group - 1)) : 0;
}

/* courtesy of bridge-utils */
static inline unsigned long __tv_to_jiffies(const struct timeval *tv)
{
	unsigned long long jif;

	jif = 1000000ULL * tv->tv_sec + tv->tv_usec;

	return jif/10000;
}

static inline void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
	unsigned long long tvusec;

	tvusec = 10000ULL*jiffies;
	tv->tv_sec = tvusec/1000000;
	tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}

void print_escape_buf(const __u8 *buf, size_t len, const char *escape);

int print_timestamp(FILE *fp);
void print_nlmsg_timestamp(FILE *fp, const struct nlmsghdr *n);

unsigned int print_name_and_link(const char *fmt,
				 const char *name, struct rtattr *tb[]);

#define BIT(nr)                 (1UL << (nr))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2 * !!(cond)]))

#ifndef offsetof
# define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif

#ifndef min
# define min(x, y) ({			\
	typeof(x) _min1 = (x);		\
	typeof(y) _min2 = (y);		\
	(void) (&_min1 == &_min2);	\
	_min1 < _min2 ? _min1 : _min2; })
#endif

#ifndef __check_format_string
# define __check_format_string(pos_str, pos_args) \
	__attribute__ ((format (printf, (pos_str), (pos_args))))
#endif

#define _textify(x)	#x
#define textify(x)	_textify(x)

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

extern int cmdlineno;
ssize_t getcmdline(char **line, size_t *len, FILE *in);
int makeargs(char *line, char *argv[], int maxargs);

char *int_to_str(int val, char *buf);
int get_guid(__u64 *guid, const char *arg);
int get_real_family(int rtm_type, int rtm_family);

int cmd_exec(const char *cmd, char **argv, bool do_fork,
	     int (*setup)(void *), void *arg);
int make_path(const char *path, mode_t mode);
char *find_cgroup2_mount(bool do_mount);
__u64 get_cgroup2_id(const char *path);
char *get_cgroup2_path(__u64 id, bool full);
int get_command_name(const char *pid, char *comm, size_t len);

int get_rtnl_link_stats_rta(struct rtnl_link_stats64 *stats64,
			    struct rtattr *tb[]);

#ifdef NEED_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
#endif

void drop_cap(void);

int get_time(unsigned int *time, const char *str);
int get_time64(__s64 *time, const char *str);
char *sprint_time(__u32 time, char *buf);
char *sprint_time64(__s64 time, char *buf);

int do_batch(const char *name, bool force,
	     int (*cmd)(int argc, char *argv[], void *user), void *user);

int parse_one_of(const char *msg, const char *realval, const char * const *list,
		 size_t len, int *p_err);
bool parse_on_off(const char *msg, const char *realval, int *p_err);

int parse_mapping_num_all(__u32 *keyp, const char *key);
int parse_mapping_gen(int *argcp, char ***argvp,
		      int (*key_cb)(__u32 *keyp, const char *key),
		      int (*mapping_cb)(__u32 key, char *value, void *data),
		      void *mapping_cb_data);
int parse_mapping(int *argcp, char ***argvp, bool allow_all,
		  int (*mapping_cb)(__u32 key, char *value, void *data),
		  void *mapping_cb_data);

struct str_num_map {
	const char *str;
	unsigned int num;
};

int str_map_lookup_str(const struct str_num_map *map, const char *needle);
const char *str_map_lookup_uint(const struct str_num_map *map,
				unsigned int val);
const char *str_map_lookup_u16(const struct str_num_map *map, uint16_t val);
const char *str_map_lookup_u8(const struct str_num_map *map, uint8_t val);

unsigned int get_str_char_count(const char *str, int match);
int str_split_by_char(char *str, char **before, char **after, int match);

#define INDENT_STR_MAXLEN 32

struct indent_mem {
	int indent_level;
	char indent_str[INDENT_STR_MAXLEN + 1];
};

struct indent_mem *alloc_indent_mem(void);
void free_indent_mem(struct indent_mem *mem);
void inc_indent(struct indent_mem *mem);
void dec_indent(struct indent_mem *mem);
void print_indent(struct indent_mem *mem);

#endif /* __UTILS_H__ */
