/*
 * ss.c		"sockstat", socket statistics
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fnmatch.h>
#include <getopt.h>
#include <stdbool.h>

#include "utils.h"
#include "rt_names.h"
#include "ll_map.h"
#include "libnetlink.h"
#include "namespace.h"
#include "SNAPSHOT.h"

#include <linux/tcp.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <linux/unix_diag.h>
#include <linux/netdevice.h>	/* for MAX_ADDR_LEN */
#include <linux/filter.h>
#include <linux/packet_diag.h>
#include <linux/netlink_diag.h>

#define MAGIC_SEQ 123456

#define DIAG_REQUEST(_req, _r)						    \
	struct {							    \
		struct nlmsghdr nlh;					    \
		_r;							    \
	} _req = {							    \
		.nlh = {						    \
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,		    \
			.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST,\
			.nlmsg_seq = MAGIC_SEQ,				    \
			.nlmsg_len = sizeof(_req),			    \
		},							    \
	}

#if HAVE_SELINUX
#include <selinux/selinux.h>
#else
/* Stubs for SELinux functions */
static int is_selinux_enabled(void)
{
	return -1;
}

static int getpidcon(pid_t pid, char **context)
{
	*context = NULL;
	return -1;
}

static int getfilecon(char *path, char **context)
{
	*context = NULL;
	return -1;
}

static int security_get_initial_context(char *name,  char **context)
{
	*context = NULL;
	return -1;
}
#endif

int resolve_hosts = 0;
int resolve_services = 1;
int preferred_family = AF_UNSPEC;
int show_options = 0;
int show_details = 0;
int show_users = 0;
int show_mem = 0;
int show_tcpinfo = 0;
int show_bpf = 0;
int show_proc_ctx = 0;
int show_sock_ctx = 0;
/* If show_users & show_proc_ctx only do user_ent_hash_build() once */
int user_ent_hash_build_init = 0;
int follow_events = 0;

int netid_width;
int state_width;
int addrp_width;
int addr_width;
int serv_width;
int screen_width;

static const char *TCP_PROTO = "tcp";
static const char *UDP_PROTO = "udp";
static const char *RAW_PROTO = "raw";
static const char *dg_proto = NULL;

enum
{
	TCP_DB,
	DCCP_DB,
	UDP_DB,
	RAW_DB,
	UNIX_DG_DB,
	UNIX_ST_DB,
	UNIX_SQ_DB,
	PACKET_DG_DB,
	PACKET_R_DB,
	NETLINK_DB,
	MAX_DB
};

#define PACKET_DBM ((1<<PACKET_DG_DB)|(1<<PACKET_R_DB))
#define UNIX_DBM ((1<<UNIX_DG_DB)|(1<<UNIX_ST_DB)|(1<<UNIX_SQ_DB))
#define ALL_DB ((1<<MAX_DB)-1)
#define INET_DBM ((1<<TCP_DB)|(1<<UDP_DB)|(1<<DCCP_DB)|(1<<RAW_DB))

enum {
	SS_UNKNOWN,
	SS_ESTABLISHED,
	SS_SYN_SENT,
	SS_SYN_RECV,
	SS_FIN_WAIT1,
	SS_FIN_WAIT2,
	SS_TIME_WAIT,
	SS_CLOSE,
	SS_CLOSE_WAIT,
	SS_LAST_ACK,
	SS_LISTEN,
	SS_CLOSING,
	SS_MAX
};

#define SS_ALL ((1 << SS_MAX) - 1)
#define SS_CONN (SS_ALL & ~((1<<SS_LISTEN)|(1<<SS_CLOSE)|(1<<SS_TIME_WAIT)|(1<<SS_SYN_RECV)))

#include "ssfilter.h"

struct filter
{
	int dbs;
	int states;
	int families;
	struct ssfilter *f;
};

static const struct filter default_dbs[MAX_DB] = {
	[TCP_DB] = {
		.states   = SS_CONN,
		.families = (1 << AF_INET) | (1 << AF_INET6),
	},
	[DCCP_DB] = {
		.states   = SS_CONN,
		.families = (1 << AF_INET) | (1 << AF_INET6),
	},
	[UDP_DB] = {
		.states   = (1 << SS_ESTABLISHED),
		.families = (1 << AF_INET) | (1 << AF_INET6),
	},
	[RAW_DB] = {
		.states   = (1 << SS_ESTABLISHED),
		.families = (1 << AF_INET) | (1 << AF_INET6),
	},
	[UNIX_DG_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = (1 << AF_UNIX),
	},
	[UNIX_ST_DB] = {
		.states   = SS_CONN,
		.families = (1 << AF_UNIX),
	},
	[UNIX_SQ_DB] = {
		.states   = SS_CONN,
		.families = (1 << AF_UNIX),
	},
	[PACKET_DG_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = (1 << AF_PACKET),
	},
	[PACKET_R_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = (1 << AF_PACKET),
	},
	[NETLINK_DB] = {
		.states   = (1 << SS_CLOSE),
		.families = (1 << AF_NETLINK),
	},
};

static const struct filter default_afs[AF_MAX] = {
	[AF_INET] = {
		.dbs    = INET_DBM,
		.states = SS_CONN,
	},
	[AF_INET6] = {
		.dbs    = INET_DBM,
		.states = SS_CONN,
	},
	[AF_UNIX] = {
		.dbs    = UNIX_DBM,
		.states = SS_CONN,
	},
	[AF_PACKET] = {
		.dbs    = PACKET_DBM,
		.states = (1 << SS_CLOSE),
	},
	[AF_NETLINK] = {
		.dbs    = (1 << NETLINK_DB),
		.states = (1 << SS_CLOSE),
	},
};

static int do_default = 1;
static struct filter current_filter;

static void filter_db_set(struct filter *f, int db)
{
	f->states   |= default_dbs[db].states;
	f->dbs	    |= 1 << db;
	do_default   = 0;
}

static void filter_af_set(struct filter *f, int af)
{
	f->states	   |= default_afs[af].states;
	f->families	   |= 1 << af;
	do_default	    = 0;
	preferred_family    = af;
}

static int filter_af_get(struct filter *f, int af)
{
	return f->families & (1 << af);
}

static void filter_default_dbs(struct filter *f)
{
	filter_db_set(f, UDP_DB);
	filter_db_set(f, DCCP_DB);
	filter_db_set(f, TCP_DB);
	filter_db_set(f, RAW_DB);
	filter_db_set(f, UNIX_ST_DB);
	filter_db_set(f, UNIX_DG_DB);
	filter_db_set(f, UNIX_SQ_DB);
	filter_db_set(f, PACKET_R_DB);
	filter_db_set(f, PACKET_DG_DB);
	filter_db_set(f, NETLINK_DB);
}

static void filter_states_set(struct filter *f, int states)
{
	if (states)
		f->states = (f->states | states) & states;
}

static void filter_merge_defaults(struct filter *f)
{
	int db;
	int af;

	for (db = 0; db < MAX_DB; db++) {
		if (!(f->dbs & (1 << db)))
			continue;

		if (!(default_dbs[db].families & f->families))
			f->families |= default_dbs[db].families;
	}
	for (af = 0; af < AF_MAX; af++) {
		if (!(f->families & (1 << af)))
			continue;

		if (!(default_afs[af].dbs & f->dbs))
			f->dbs |= default_afs[af].dbs;
	}
}

static FILE *generic_proc_open(const char *env, const char *name)
{
	const char *p = getenv(env);
	char store[128];

	if (!p) {
		p = getenv("PROC_ROOT") ? : "/proc";
		snprintf(store, sizeof(store)-1, "%s/%s", p, name);
		p = store;
	}

	return fopen(p, "r");
}

static FILE *net_tcp_open(void)
{
	return generic_proc_open("PROC_NET_TCP", "net/tcp");
}

static FILE *net_tcp6_open(void)
{
	return generic_proc_open("PROC_NET_TCP6", "net/tcp6");
}

static FILE *net_udp_open(void)
{
	return generic_proc_open("PROC_NET_UDP", "net/udp");
}

static FILE *net_udp6_open(void)
{
	return generic_proc_open("PROC_NET_UDP6", "net/udp6");
}

static FILE *net_raw_open(void)
{
	return generic_proc_open("PROC_NET_RAW", "net/raw");
}

static FILE *net_raw6_open(void)
{
	return generic_proc_open("PROC_NET_RAW6", "net/raw6");
}

static FILE *net_unix_open(void)
{
	return generic_proc_open("PROC_NET_UNIX", "net/unix");
}

static FILE *net_packet_open(void)
{
	return generic_proc_open("PROC_NET_PACKET", "net/packet");
}

static FILE *net_netlink_open(void)
{
	return generic_proc_open("PROC_NET_NETLINK", "net/netlink");
}

static FILE *slabinfo_open(void)
{
	return generic_proc_open("PROC_SLABINFO", "slabinfo");
}

static FILE *net_sockstat_open(void)
{
	return generic_proc_open("PROC_NET_SOCKSTAT", "net/sockstat");
}

static FILE *net_sockstat6_open(void)
{
	return generic_proc_open("PROC_NET_SOCKSTAT6", "net/sockstat6");
}

static FILE *net_snmp_open(void)
{
	return generic_proc_open("PROC_NET_SNMP", "net/snmp");
}

static FILE *ephemeral_ports_open(void)
{
	return generic_proc_open("PROC_IP_LOCAL_PORT_RANGE", "sys/net/ipv4/ip_local_port_range");
}

struct user_ent {
	struct user_ent	*next;
	unsigned int	ino;
	int		pid;
	int		fd;
	char		*process;
	char		*process_ctx;
	char		*socket_ctx;
};

#define USER_ENT_HASH_SIZE	256
struct user_ent *user_ent_hash[USER_ENT_HASH_SIZE];

static int user_ent_hashfn(unsigned int ino)
{
	int val = (ino >> 24) ^ (ino >> 16) ^ (ino >> 8) ^ ino;

	return val & (USER_ENT_HASH_SIZE - 1);
}

static void user_ent_add(unsigned int ino, char *process,
					int pid, int fd,
					char *proc_ctx,
					char *sock_ctx)
{
	struct user_ent *p, **pp;

	p = malloc(sizeof(struct user_ent));
	if (!p) {
		fprintf(stderr, "ss: failed to malloc buffer\n");
		abort();
	}
	p->next = NULL;
	p->ino = ino;
	p->pid = pid;
	p->fd = fd;
	p->process = strdup(process);
	p->process_ctx = strdup(proc_ctx);
	p->socket_ctx = strdup(sock_ctx);

	pp = &user_ent_hash[user_ent_hashfn(ino)];
	p->next = *pp;
	*pp = p;
}

static void user_ent_destroy(void)
{
	struct user_ent *p, *p_next;
	int cnt = 0;

	while (cnt != USER_ENT_HASH_SIZE) {
		p = user_ent_hash[cnt];
		while (p) {
			free(p->process);
			free(p->process_ctx);
			free(p->socket_ctx);
			p_next = p->next;
			free(p);
			p = p_next;
		}
		cnt++;
	}
}

static void user_ent_hash_build(void)
{
	const char *root = getenv("PROC_ROOT") ? : "/proc/";
	struct dirent *d;
	char name[1024];
	int nameoff;
	DIR *dir;
	char *pid_context;
	char *sock_context;
	const char *no_ctx = "unavailable";

	/* If show_users & show_proc_ctx set only do this once */
	if (user_ent_hash_build_init != 0)
		return;

	user_ent_hash_build_init = 1;

	strncpy(name, root, sizeof(name)-1);
	name[sizeof(name)-1] = 0;

	if (strlen(name) == 0 || name[strlen(name)-1] != '/')
		strcat(name, "/");

	nameoff = strlen(name);

	dir = opendir(name);
	if (!dir)
		return;

	while ((d = readdir(dir)) != NULL) {
		struct dirent *d1;
		char process[16];
		char *p;
		int pid, pos;
		DIR *dir1;
		char crap;

		if (sscanf(d->d_name, "%d%c", &pid, &crap) != 1)
			continue;

		if (getpidcon(pid, &pid_context) != 0)
			pid_context = strdup(no_ctx);

		snprintf(name + nameoff, sizeof(name) - nameoff, "%d/fd/", pid);
		pos = strlen(name);
		if ((dir1 = opendir(name)) == NULL) {
			free(pid_context);
			continue;
		}

		process[0] = '\0';
		p = process;

		while ((d1 = readdir(dir1)) != NULL) {
			const char *pattern = "socket:[";
			unsigned int ino;
			char lnk[64];
			int fd;
			ssize_t link_len;
			char tmp[1024];

			if (sscanf(d1->d_name, "%d%c", &fd, &crap) != 1)
				continue;

			snprintf(name+pos, sizeof(name) - pos, "%d", fd);

			link_len = readlink(name, lnk, sizeof(lnk)-1);
			if (link_len == -1)
				continue;
			lnk[link_len] = '\0';

			if (strncmp(lnk, pattern, strlen(pattern)))
				continue;

			sscanf(lnk, "socket:[%u]", &ino);

			snprintf(tmp, sizeof(tmp), "%s/%d/fd/%s",
					root, pid, d1->d_name);

			if (getfilecon(tmp, &sock_context) <= 0)
				sock_context = strdup(no_ctx);

			if (*p == '\0') {
				FILE *fp;

				snprintf(tmp, sizeof(tmp), "%s/%d/stat",
					root, pid);
				if ((fp = fopen(tmp, "r")) != NULL) {
					fscanf(fp, "%*d (%[^)])", p);
					fclose(fp);
				}
			}
			user_ent_add(ino, p, pid, fd,
					pid_context, sock_context);
			free(sock_context);
		}
		free(pid_context);
		closedir(dir1);
	}
	closedir(dir);
}

enum entry_types {
	USERS,
	PROC_CTX,
	PROC_SOCK_CTX
};

#define ENTRY_BUF_SIZE 512
static int find_entry(unsigned ino, char **buf, int type)
{
	struct user_ent *p;
	int cnt = 0;
	char *ptr;
	char *new_buf;
	int len, new_buf_len;
	int buf_used = 0;
	int buf_len = 0;

	if (!ino)
		return 0;

	p = user_ent_hash[user_ent_hashfn(ino)];
	ptr = *buf = NULL;
	while (p) {
		if (p->ino != ino)
			goto next;

		while (1) {
			ptr = *buf + buf_used;
			switch (type) {
			case USERS:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,fd=%d),",
					p->process, p->pid, p->fd);
				break;
			case PROC_CTX:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,proc_ctx=%s,fd=%d),",
					p->process, p->pid,
					p->process_ctx, p->fd);
				break;
			case PROC_SOCK_CTX:
				len = snprintf(ptr, buf_len - buf_used,
					"(\"%s\",pid=%d,proc_ctx=%s,fd=%d,sock_ctx=%s),",
					p->process, p->pid,
					p->process_ctx, p->fd,
					p->socket_ctx);
				break;
			default:
				fprintf(stderr, "ss: invalid type: %d\n", type);
				abort();
			}

			if (len < 0 || len >= buf_len - buf_used) {
				new_buf_len = buf_len + ENTRY_BUF_SIZE;
				new_buf = realloc(*buf, new_buf_len);
				if (!new_buf) {
					fprintf(stderr, "ss: failed to malloc buffer\n");
					abort();
				}
				*buf = new_buf;
				buf_len = new_buf_len;
				continue;
			} else {
				buf_used += len;
				break;
			}
		}
		cnt++;
next:
		p = p->next;
	}
	if (buf_used) {
		ptr = *buf + buf_used;
		ptr[-1] = '\0';
	}
	return cnt;
}

/* Get stats from slab */

struct slabstat
{
	int socks;
	int tcp_ports;
	int tcp_tws;
	int tcp_syns;
	int skbs;
};

static struct slabstat slabstat;

static const char *slabstat_ids[] =
{
	"sock",
	"tcp_bind_bucket",
	"tcp_tw_bucket",
	"tcp_open_request",
	"skbuff_head_cache",
};

static int get_slabstat(struct slabstat *s)
{
	char buf[256];
	FILE *fp;
	int cnt;
	static int slabstat_valid;

	if (slabstat_valid)
		return 0;

	memset(s, 0, sizeof(*s));

	fp = slabinfo_open();
	if (!fp)
		return -1;

	cnt = sizeof(*s)/sizeof(int);

	fgets(buf, sizeof(buf), fp);
	while(fgets(buf, sizeof(buf), fp) != NULL) {
		int i;
		for (i=0; i<sizeof(slabstat_ids)/sizeof(slabstat_ids[0]); i++) {
			if (memcmp(buf, slabstat_ids[i], strlen(slabstat_ids[i])) == 0) {
				sscanf(buf, "%*s%d", ((int *)s) + i);
				cnt--;
				break;
			}
		}
		if (cnt <= 0)
			break;
	}

	slabstat_valid = 1;

	fclose(fp);
	return 0;
}

static unsigned long long cookie_sk_get(const uint32_t *cookie)
{
	return (((unsigned long long)cookie[1] << 31) << 1) | cookie[0];
}

static const char *sstate_name[] = {
	"UNKNOWN",
	[SS_ESTABLISHED] = "ESTAB",
	[SS_SYN_SENT] = "SYN-SENT",
	[SS_SYN_RECV] = "SYN-RECV",
	[SS_FIN_WAIT1] = "FIN-WAIT-1",
	[SS_FIN_WAIT2] = "FIN-WAIT-2",
	[SS_TIME_WAIT] = "TIME-WAIT",
	[SS_CLOSE] = "UNCONN",
	[SS_CLOSE_WAIT] = "CLOSE-WAIT",
	[SS_LAST_ACK] = "LAST-ACK",
	[SS_LISTEN] = 	"LISTEN",
	[SS_CLOSING] = "CLOSING",
};

static const char *sstate_namel[] = {
	"UNKNOWN",
	[SS_ESTABLISHED] = "established",
	[SS_SYN_SENT] = "syn-sent",
	[SS_SYN_RECV] = "syn-recv",
	[SS_FIN_WAIT1] = "fin-wait-1",
	[SS_FIN_WAIT2] = "fin-wait-2",
	[SS_TIME_WAIT] = "time-wait",
	[SS_CLOSE] = "unconnected",
	[SS_CLOSE_WAIT] = "close-wait",
	[SS_LAST_ACK] = "last-ack",
	[SS_LISTEN] = 	"listening",
	[SS_CLOSING] = "closing",
};

struct sockstat
{
	struct sockstat	   *next;
	unsigned int	    type;
	uint16_t	    prot;
	inet_prefix	    local;
	inet_prefix	    remote;
	int		    lport;
	int		    rport;
	int		    state;
	int		    rq, wq;
	unsigned	    ino;
	unsigned	    uid;
	int		    refcnt;
	unsigned int	    iface;
	unsigned long long  sk;
	char *name;
	char *peer_name;
};

struct dctcpstat
{
	unsigned int	ce_state;
	unsigned int	alpha;
	unsigned int	ab_ecn;
	unsigned int	ab_tot;
	bool		enabled;
};

struct tcpstat
{
	struct sockstat	    ss;
	int		    timer;
	int		    timeout;
	int		    probes;
	char		    cong_alg[16];
	double		    rto, ato, rtt, rttvar;
	int		    qack, cwnd, ssthresh, backoff;
	double		    send_bps;
	int		    snd_wscale;
	int		    rcv_wscale;
	int		    mss;
	unsigned int	    lastsnd;
	unsigned int	    lastrcv;
	unsigned int	    lastack;
	double		    pacing_rate;
	double		    pacing_rate_max;
	unsigned long long  bytes_acked;
	unsigned long long  bytes_received;
	unsigned int	    segs_out;
	unsigned int	    segs_in;
	unsigned int	    unacked;
	unsigned int	    retrans;
	unsigned int	    retrans_total;
	unsigned int	    lost;
	unsigned int	    sacked;
	unsigned int	    fackets;
	unsigned int	    reordering;
	double		    rcv_rtt;
	int		    rcv_space;
	bool		    has_ts_opt;
	bool		    has_sack_opt;
	bool		    has_ecn_opt;
	bool		    has_ecnseen_opt;
	bool		    has_fastopen_opt;
	bool		    has_wscale_opt;
	struct dctcpstat    *dctcp;
};

static void sock_state_print(struct sockstat *s, const char *sock_name)
{
	if (netid_width)
		printf("%-*s ", netid_width, sock_name);
	if (state_width)
		printf("%-*s ", state_width, sstate_name[s->state]);

	printf("%-6d %-6d ", s->rq, s->wq);
}

static void sock_details_print(struct sockstat *s)
{
	if (s->uid)
		printf(" uid:%u", s->uid);

	printf(" ino:%u", s->ino);
	printf(" sk:%llx", s->sk);
}

static void sock_addr_print_width(int addr_len, const char *addr, char *delim,
		int port_len, const char *port, const char *ifname)
{
	if (ifname) {
		printf("%*s%%%s%s%-*s ", addr_len, addr, ifname, delim,
				port_len, port);
	}
	else {
		printf("%*s%s%-*s ", addr_len, addr, delim, port_len, port);
	}
}

static void sock_addr_print(const char *addr, char *delim, const char *port,
		const char *ifname)
{
	sock_addr_print_width(addr_width, addr, delim, serv_width, port, ifname);
}

static const char *tmr_name[] = {
	"off",
	"on",
	"keepalive",
	"timewait",
	"persist",
	"unknown"
};

static const char *print_ms_timer(int timeout)
{
	static char buf[64];
	int secs, msecs, minutes;
	if (timeout < 0)
		timeout = 0;
	secs = timeout/1000;
	minutes = secs/60;
	secs = secs%60;
	msecs = timeout%1000;
	buf[0] = 0;
	if (minutes) {
		msecs = 0;
		snprintf(buf, sizeof(buf)-16, "%dmin", minutes);
		if (minutes > 9)
			secs = 0;
	}
	if (secs) {
		if (secs > 9)
			msecs = 0;
		sprintf(buf+strlen(buf), "%d%s", secs, msecs ? "." : "sec");
	}
	if (msecs)
		sprintf(buf+strlen(buf), "%03dms", msecs);
	return buf;
}

struct scache {
	struct scache *next;
	int port;
	char *name;
	const char *proto;
};

struct scache *rlist;

static void init_service_resolver(void)
{
	char buf[128];
	FILE *fp = popen("/usr/sbin/rpcinfo -p 2>/dev/null", "r");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			unsigned int progn, port;
			char proto[128], prog[128];
			if (sscanf(buf, "%u %*d %s %u %s", &progn, proto,
				   &port, prog+4) == 4) {
				struct scache *c = malloc(sizeof(*c));
				if (c) {
					c->port = port;
					memcpy(prog, "rpc.", 4);
					c->name = strdup(prog);
					if (strcmp(proto, TCP_PROTO) == 0)
						c->proto = TCP_PROTO;
					else if (strcmp(proto, UDP_PROTO) == 0)
						c->proto = UDP_PROTO;
					else
						c->proto = NULL;
					c->next = rlist;
					rlist = c;
				}
			}
		}
		pclose(fp);
	}
}

static int ip_local_port_min, ip_local_port_max;

/* Even do not try default linux ephemeral port ranges:
 * default /etc/services contains so much of useless crap
 * wouldbe "allocated" to this area that resolution
 * is really harmful. I shrug each time when seeing
 * "socks" or "cfinger" in dumps.
 */
static int is_ephemeral(int port)
{
	if (!ip_local_port_min) {
		FILE *f = ephemeral_ports_open();
		if (f) {
			fscanf(f, "%d %d",
			       &ip_local_port_min, &ip_local_port_max);
			fclose(f);
		} else {
			ip_local_port_min = 1024;
			ip_local_port_max = 4999;
		}
	}

	return (port >= ip_local_port_min && port<= ip_local_port_max);
}


static const char *__resolve_service(int port)
{
	struct scache *c;

	for (c = rlist; c; c = c->next) {
		if (c->port == port && c->proto == dg_proto)
			return c->name;
	}

	if (!is_ephemeral(port)) {
		static int notfirst;
		struct servent *se;
		if (!notfirst) {
			setservent(1);
			notfirst = 1;
		}
		se = getservbyport(htons(port), dg_proto);
		if (se)
			return se->s_name;
	}

	return NULL;
}

#define SCACHE_BUCKETS 1024
static struct scache *cache_htab[SCACHE_BUCKETS];

static const char *resolve_service(int port)
{
	static char buf[128];
	struct scache *c;
	const char *res;
	int hash;

	if (port == 0) {
		buf[0] = '*';
		buf[1] = 0;
		return buf;
	}

	if (!resolve_services)
		goto do_numeric;

	if (dg_proto == RAW_PROTO)
		return inet_proto_n2a(port, buf, sizeof(buf));


	hash = (port^(((unsigned long)dg_proto)>>2)) % SCACHE_BUCKETS;

	for (c = cache_htab[hash]; c; c = c->next) {
		if (c->port == port && c->proto == dg_proto)
			goto do_cache;
	}

	c = malloc(sizeof(*c));
	if (!c)
		goto do_numeric;
	res = __resolve_service(port);
	c->port = port;
	c->name = res ? strdup(res) : NULL;
	c->proto = dg_proto;
	c->next = cache_htab[hash];
	cache_htab[hash] = c;

do_cache:
	if (c->name)
		return c->name;

do_numeric:
	sprintf(buf, "%u", port);
	return buf;
}

static void inet_addr_print(const inet_prefix *a, int port, unsigned int ifindex)
{
	char buf[1024];
	const char *ap = buf;
	int est_len = addr_width;
	const char *ifname = NULL;

	if (a->family == AF_INET) {
		if (a->data[0] == 0) {
			buf[0] = '*';
			buf[1] = 0;
		} else {
			ap = format_host(AF_INET, 4, a->data, buf, sizeof(buf));
		}
	} else {
		ap = format_host(a->family, 16, a->data, buf, sizeof(buf));
		est_len = strlen(ap);
		if (est_len <= addr_width)
			est_len = addr_width;
		else
			est_len = addr_width + ((est_len-addr_width+3)/4)*4;
	}

	if (ifindex) {
		ifname   = ll_index_to_name(ifindex);
		est_len -= strlen(ifname) + 1;  /* +1 for percent char */
		if (est_len < 0)
			est_len = 0;
	}

	sock_addr_print_width(est_len, ap, ":", serv_width, resolve_service(port),
			ifname);
}

struct aafilter
{
	inet_prefix	addr;
	int		port;
	struct aafilter *next;
};

static int inet2_addr_match(const inet_prefix *a, const inet_prefix *p,
			    int plen)
{
	if (!inet_addr_match(a, p, plen))
		return 0;

	/* Cursed "v4 mapped" addresses: v4 mapped socket matches
	 * pure IPv4 rule, but v4-mapped rule selects only v4-mapped
	 * sockets. Fair? */
	if (p->family == AF_INET && a->family == AF_INET6) {
		if (a->data[0] == 0 && a->data[1] == 0 &&
		    a->data[2] == htonl(0xffff)) {
			inet_prefix tmp = *a;
			tmp.data[0] = a->data[3];
			return inet_addr_match(&tmp, p, plen);
		}
	}
	return 1;
}

static int unix_match(const inet_prefix *a, const inet_prefix *p)
{
	char *addr, *pattern;
	memcpy(&addr, a->data, sizeof(addr));
	memcpy(&pattern, p->data, sizeof(pattern));
	if (pattern == NULL)
		return 1;
	if (addr == NULL)
		addr = "";
	return !fnmatch(pattern, addr, 0);
}

static int run_ssfilter(struct ssfilter *f, struct sockstat *s)
{
	switch (f->type) {
		case SSF_S_AUTO:
	{
                static int low, high=65535;

		if (s->local.family == AF_UNIX) {
			char *p;
			memcpy(&p, s->local.data, sizeof(p));
			return p == NULL || (p[0] == '@' && strlen(p) == 6 &&
					     strspn(p+1, "0123456789abcdef") == 5);
		}
		if (s->local.family == AF_PACKET)
			return s->lport == 0 && s->local.data[0] == 0;
		if (s->local.family == AF_NETLINK)
			return s->lport < 0;

                if (!low) {
			FILE *fp = ephemeral_ports_open();
			if (fp) {
				fscanf(fp, "%d%d", &low, &high);
				fclose(fp);
			}
		}
		return s->lport >= low && s->lport <= high;
	}
		case SSF_DCOND:
	{
		struct aafilter *a = (void*)f->pred;
		if (a->addr.family == AF_UNIX)
			return unix_match(&s->remote, &a->addr);
		if (a->port != -1 && a->port != s->rport)
			return 0;
		if (a->addr.bitlen) {
			do {
				if (!inet2_addr_match(&s->remote, &a->addr, a->addr.bitlen))
					return 1;
			} while ((a = a->next) != NULL);
			return 0;
		}
		return 1;
	}
		case SSF_SCOND:
	{
		struct aafilter *a = (void*)f->pred;
		if (a->addr.family == AF_UNIX)
			return unix_match(&s->local, &a->addr);
		if (a->port != -1 && a->port != s->lport)
			return 0;
		if (a->addr.bitlen) {
			do {
				if (!inet2_addr_match(&s->local, &a->addr, a->addr.bitlen))
					return 1;
			} while ((a = a->next) != NULL);
			return 0;
		}
		return 1;
	}
		case SSF_D_GE:
	{
		struct aafilter *a = (void*)f->pred;
		return s->rport >= a->port;
	}
		case SSF_D_LE:
	{
		struct aafilter *a = (void*)f->pred;
		return s->rport <= a->port;
	}
		case SSF_S_GE:
	{
		struct aafilter *a = (void*)f->pred;
		return s->lport >= a->port;
	}
		case SSF_S_LE:
	{
		struct aafilter *a = (void*)f->pred;
		return s->lport <= a->port;
	}

		/* Yup. It is recursion. Sorry. */
		case SSF_AND:
		return run_ssfilter(f->pred, s) && run_ssfilter(f->post, s);
		case SSF_OR:
		return run_ssfilter(f->pred, s) || run_ssfilter(f->post, s);
		case SSF_NOT:
		return !run_ssfilter(f->pred, s);
		default:
		abort();
	}
}

/* Relocate external jumps by reloc. */
static void ssfilter_patch(char *a, int len, int reloc)
{
	while (len > 0) {
		struct inet_diag_bc_op *op = (struct inet_diag_bc_op*)a;
		if (op->no == len+4)
			op->no += reloc;
		len -= op->yes;
		a += op->yes;
	}
	if (len < 0)
		abort();
}

static int ssfilter_bytecompile(struct ssfilter *f, char **bytecode)
{
	switch (f->type) {
		case SSF_S_AUTO:
	{
		if (!(*bytecode=malloc(4))) abort();
		((struct inet_diag_bc_op*)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_AUTO, 4, 8 };
		return 4;
	}
		case SSF_DCOND:
		case SSF_SCOND:
	{
		struct aafilter *a = (void*)f->pred;
		struct aafilter *b;
		char *ptr;
		int  code = (f->type == SSF_DCOND ? INET_DIAG_BC_D_COND : INET_DIAG_BC_S_COND);
		int len = 0;

		for (b=a; b; b=b->next) {
			len += 4 + sizeof(struct inet_diag_hostcond);
			if (a->addr.family == AF_INET6)
				len += 16;
			else
				len += 4;
			if (b->next)
				len += 4;
		}
		if (!(ptr = malloc(len))) abort();
		*bytecode = ptr;
		for (b=a; b; b=b->next) {
			struct inet_diag_bc_op *op = (struct inet_diag_bc_op *)ptr;
			int alen = (a->addr.family == AF_INET6 ? 16 : 4);
			int oplen = alen + 4 + sizeof(struct inet_diag_hostcond);
			struct inet_diag_hostcond *cond = (struct inet_diag_hostcond*)(ptr+4);

			*op = (struct inet_diag_bc_op){ code, oplen, oplen+4 };
			cond->family = a->addr.family;
			cond->port = a->port;
			cond->prefix_len = a->addr.bitlen;
			memcpy(cond->addr, a->addr.data, alen);
			ptr += oplen;
			if (b->next) {
				op = (struct inet_diag_bc_op *)ptr;
				*op = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, len - (ptr-*bytecode)};
				ptr += 4;
			}
		}
		return ptr - *bytecode;
	}
		case SSF_D_GE:
	{
		struct aafilter *x = (void*)f->pred;
		if (!(*bytecode=malloc(8))) abort();
		((struct inet_diag_bc_op*)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_D_GE, 8, 12 };
		((struct inet_diag_bc_op*)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_D_LE:
	{
		struct aafilter *x = (void*)f->pred;
		if (!(*bytecode=malloc(8))) abort();
		((struct inet_diag_bc_op*)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_D_LE, 8, 12 };
		((struct inet_diag_bc_op*)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_S_GE:
	{
		struct aafilter *x = (void*)f->pred;
		if (!(*bytecode=malloc(8))) abort();
		((struct inet_diag_bc_op*)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_S_GE, 8, 12 };
		((struct inet_diag_bc_op*)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}
		case SSF_S_LE:
	{
		struct aafilter *x = (void*)f->pred;
		if (!(*bytecode=malloc(8))) abort();
		((struct inet_diag_bc_op*)*bytecode)[0] = (struct inet_diag_bc_op){ INET_DIAG_BC_S_LE, 8, 12 };
		((struct inet_diag_bc_op*)*bytecode)[1] = (struct inet_diag_bc_op){ 0, 0, x->port };
		return 8;
	}

		case SSF_AND:
	{
		char *a1, *a2, *a;
		int l1, l2;
		l1 = ssfilter_bytecompile(f->pred, &a1);
		l2 = ssfilter_bytecompile(f->post, &a2);
		if (!(a = malloc(l1+l2))) abort();
		memcpy(a, a1, l1);
		memcpy(a+l1, a2, l2);
		free(a1); free(a2);
		ssfilter_patch(a, l1, l2);
		*bytecode = a;
		return l1+l2;
	}
		case SSF_OR:
	{
		char *a1, *a2, *a;
		int l1, l2;
		l1 = ssfilter_bytecompile(f->pred, &a1);
		l2 = ssfilter_bytecompile(f->post, &a2);
		if (!(a = malloc(l1+l2+4))) abort();
		memcpy(a, a1, l1);
		memcpy(a+l1+4, a2, l2);
		free(a1); free(a2);
		*(struct inet_diag_bc_op*)(a+l1) = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, l2+4 };
		*bytecode = a;
		return l1+l2+4;
	}
		case SSF_NOT:
	{
		char *a1, *a;
		int l1;
		l1 = ssfilter_bytecompile(f->pred, &a1);
		if (!(a = malloc(l1+4))) abort();
		memcpy(a, a1, l1);
		free(a1);
		*(struct inet_diag_bc_op*)(a+l1) = (struct inet_diag_bc_op){ INET_DIAG_BC_JMP, 4, 8 };
		*bytecode = a;
		return l1+4;
	}
		default:
		abort();
	}
}

static int remember_he(struct aafilter *a, struct hostent *he)
{
	char **ptr = he->h_addr_list;
	int cnt = 0;
	int len;

	if (he->h_addrtype == AF_INET)
		len = 4;
	else if (he->h_addrtype == AF_INET6)
		len = 16;
	else
		return 0;

	while (*ptr) {
		struct aafilter *b = a;
		if (a->addr.bitlen) {
			if ((b = malloc(sizeof(*b))) == NULL)
				return cnt;
			*b = *a;
			b->next = a->next;
			a->next = b;
		}
		memcpy(b->addr.data, *ptr, len);
		b->addr.bytelen = len;
		b->addr.bitlen = len*8;
		b->addr.family = he->h_addrtype;
		ptr++;
		cnt++;
	}
	return cnt;
}

static int get_dns_host(struct aafilter *a, const char *addr, int fam)
{
	static int notfirst;
	int cnt = 0;
	struct hostent *he;

	a->addr.bitlen = 0;
	if (!notfirst) {
		sethostent(1);
		notfirst = 1;
	}
	he = gethostbyname2(addr, fam == AF_UNSPEC ? AF_INET : fam);
	if (he)
		cnt = remember_he(a, he);
	if (fam == AF_UNSPEC) {
		he = gethostbyname2(addr, AF_INET6);
		if (he)
			cnt += remember_he(a, he);
	}
	return !cnt;
}

static int xll_initted = 0;

static void xll_init(void)
{
	struct rtnl_handle rth;
	if (rtnl_open(&rth, 0) < 0)
		exit(1);

	ll_init_map(&rth);
	rtnl_close(&rth);
	xll_initted = 1;
}

static const char *xll_index_to_name(int index)
{
	if (!xll_initted)
		xll_init();
	return ll_index_to_name(index);
}

static int xll_name_to_index(const char *dev)
{
	if (!xll_initted)
		xll_init();
	return ll_name_to_index(dev);
}

void *parse_hostcond(char *addr, bool is_port)
{
	char *port = NULL;
	struct aafilter a = { .port = -1 };
	struct aafilter *res;
	int fam = preferred_family;
	struct filter *f = &current_filter;

	if (fam == AF_UNIX || strncmp(addr, "unix:", 5) == 0) {
		char *p;
		a.addr.family = AF_UNIX;
		if (strncmp(addr, "unix:", 5) == 0)
			addr+=5;
		p = strdup(addr);
		a.addr.bitlen = 8*strlen(p);
		memcpy(a.addr.data, &p, sizeof(p));
		fam = AF_UNIX;
		goto out;
	}

	if (fam == AF_PACKET || strncmp(addr, "link:", 5) == 0) {
		a.addr.family = AF_PACKET;
		a.addr.bitlen = 0;
		if (strncmp(addr, "link:", 5) == 0)
			addr+=5;
		port = strchr(addr, ':');
		if (port) {
			*port = 0;
			if (port[1] && strcmp(port+1, "*")) {
				if (get_integer(&a.port, port+1, 0)) {
					if ((a.port = xll_name_to_index(port+1)) <= 0)
						return NULL;
				}
			}
		}
		if (addr[0] && strcmp(addr, "*")) {
			unsigned short tmp;
			a.addr.bitlen = 32;
			if (ll_proto_a2n(&tmp, addr))
				return NULL;
			a.addr.data[0] = ntohs(tmp);
		}
		fam = AF_PACKET;
		goto out;
	}

	if (fam == AF_NETLINK || strncmp(addr, "netlink:", 8) == 0) {
		a.addr.family = AF_NETLINK;
		a.addr.bitlen = 0;
		if (strncmp(addr, "netlink:", 8) == 0)
			addr+=8;
		port = strchr(addr, ':');
		if (port) {
			*port = 0;
			if (port[1] && strcmp(port+1, "*")) {
				if (get_integer(&a.port, port+1, 0)) {
					if (strcmp(port+1, "kernel") == 0)
						a.port = 0;
					else
						return NULL;
				}
			}
		}
		if (addr[0] && strcmp(addr, "*")) {
			a.addr.bitlen = 32;
			if (nl_proto_a2n(&a.addr.data[0], addr) == -1)
				return NULL;
		}
		fam = AF_NETLINK;
		goto out;
	}

	if (fam == AF_INET || !strncmp(addr, "inet:", 5)) {
		fam = AF_INET;
		if (!strncmp(addr, "inet:", 5))
			addr += 5;
	} else if (fam == AF_INET6 || !strncmp(addr, "inet6:", 6)) {
		fam = AF_INET6;
		if (!strncmp(addr, "inet6:", 6))
			addr += 6;
	}

	/* URL-like literal [] */
	if (addr[0] == '[') {
		addr++;
		if ((port = strchr(addr, ']')) == NULL)
			return NULL;
		*port++ = 0;
	} else if (addr[0] == '*') {
		port = addr+1;
	} else {
		port = strrchr(strchr(addr, '/') ? : addr, ':');
	}

	if (is_port)
		port = addr;

	if (port && *port) {
		if (*port == ':')
			*port++ = 0;

		if (*port && *port != '*') {
			if (get_integer(&a.port, port, 0)) {
				struct servent *se1 = NULL;
				struct servent *se2 = NULL;
				if (current_filter.dbs&(1<<UDP_DB))
					se1 = getservbyname(port, UDP_PROTO);
				if (current_filter.dbs&(1<<TCP_DB))
					se2 = getservbyname(port, TCP_PROTO);
				if (se1 && se2 && se1->s_port != se2->s_port) {
					fprintf(stderr, "Error: ambiguous port \"%s\".\n", port);
					return NULL;
				}
				if (!se1)
					se1 = se2;
				if (se1) {
					a.port = ntohs(se1->s_port);
				} else {
					struct scache *s;
					for (s = rlist; s; s = s->next) {
						if ((s->proto == UDP_PROTO &&
						     (current_filter.dbs&(1<<UDP_DB))) ||
						    (s->proto == TCP_PROTO &&
						     (current_filter.dbs&(1<<TCP_DB)))) {
							if (s->name && strcmp(s->name, port) == 0) {
								if (a.port > 0 && a.port != s->port) {
									fprintf(stderr, "Error: ambiguous port \"%s\".\n", port);
									return NULL;
								}
								a.port = s->port;
							}
						}
					}
					if (a.port <= 0) {
						fprintf(stderr, "Error: \"%s\" does not look like a port.\n", port);
						return NULL;
					}
				}
			}
		}
	}
	if (!is_port && addr && *addr && *addr != '*') {
		if (get_prefix_1(&a.addr, addr, fam)) {
			if (get_dns_host(&a, addr, fam)) {
				fprintf(stderr, "Error: an inet prefix is expected rather than \"%s\".\n", addr);
				return NULL;
			}
		}
	}

out:
	if (fam != AF_UNSPEC) {
		f->families = 0;
		filter_af_set(f, fam);
		filter_states_set(f, 0);
	}

	res = malloc(sizeof(*res));
	if (res)
		memcpy(res, &a, sizeof(a));
	return res;
}

static char *proto_name(int protocol)
{
	switch (protocol) {
	case 0:
		return "raw";
	case IPPROTO_UDP:
		return "udp";
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_DCCP:
		return "dccp";
	}

	return "???";
}

static void inet_stats_print(struct sockstat *s, int protocol)
{
	char *buf = NULL;

	sock_state_print(s, proto_name(protocol));

	inet_addr_print(&s->local, s->lport, s->iface);
	inet_addr_print(&s->remote, s->rport, 0);

	if (show_proc_ctx || show_sock_ctx) {
		if (find_entry(s->ino, &buf,
				(show_proc_ctx & show_sock_ctx) ?
				PROC_SOCK_CTX : PROC_CTX) > 0) {
			printf(" users:(%s)", buf);
			free(buf);
		}
	} else if (show_users) {
		if (find_entry(s->ino, &buf, USERS) > 0) {
			printf(" users:(%s)", buf);
			free(buf);
		}
	}
}

static int proc_parse_inet_addr(char *loc, char *rem, int family, struct
		sockstat *s)
{
	s->local.family = s->remote.family = family;
	if (family == AF_INET) {
		sscanf(loc, "%x:%x", s->local.data, (unsigned*)&s->lport);
		sscanf(rem, "%x:%x", s->remote.data, (unsigned*)&s->rport);
		s->local.bytelen = s->remote.bytelen = 4;
		return 0;
	} else {
		sscanf(loc, "%08x%08x%08x%08x:%x",
		       s->local.data,
		       s->local.data + 1,
		       s->local.data + 2,
		       s->local.data + 3,
		       &s->lport);
		sscanf(rem, "%08x%08x%08x%08x:%x",
		       s->remote.data,
		       s->remote.data + 1,
		       s->remote.data + 2,
		       s->remote.data + 3,
		       &s->rport);
		s->local.bytelen = s->remote.bytelen = 16;
		return 0;
	}
	return -1;
}

static int proc_inet_split_line(char *line, char **loc, char **rem, char **data)
{
	char *p;

	if ((p = strchr(line, ':')) == NULL)
		return -1;

	*loc = p+2;
	if ((p = strchr(*loc, ':')) == NULL)
		return -1;

	p[5] = 0;
	*rem = p+6;
	if ((p = strchr(*rem, ':')) == NULL)
		return -1;

	p[5] = 0;
	*data = p+6;
	return 0;
}

static char *sprint_bw(char *buf, double bw)
{
	if (bw > 1000000.)
		sprintf(buf,"%.1fM", bw / 1000000.);
	else if (bw > 1000.)
		sprintf(buf,"%.1fK", bw / 1000.);
	else
		sprintf(buf, "%g", bw);

	return buf;
}

static void tcp_stats_print(struct tcpstat *s)
{
	char b1[64];

	if (s->has_ts_opt)
		printf(" ts");
	if (s->has_sack_opt)
		printf(" sack");
	if (s->has_ecn_opt)
		printf(" ecn");
	if (s->has_ecnseen_opt)
		printf(" ecnseen");
	if (s->has_fastopen_opt)
		printf(" fastopen");
	if (s->cong_alg[0])
		printf(" %s", s->cong_alg);
	if (s->has_wscale_opt)
		printf(" wscale:%d,%d", s->snd_wscale, s->rcv_wscale);
	if (s->rto)
		printf(" rto:%g", s->rto);
	if (s->backoff)
		printf(" backoff:%u", s->backoff);
	if (s->rtt)
		printf(" rtt:%g/%g", s->rtt, s->rttvar);
	if (s->ato)
		printf(" ato:%g", s->ato);

	if (s->qack)
		printf(" qack:%d", s->qack);
	if (s->qack & 1)
		printf(" bidir");

	if (s->mss)
		printf(" mss:%d", s->mss);
	if (s->cwnd)
		printf(" cwnd:%d", s->cwnd);
	if (s->ssthresh)
		printf(" ssthresh:%d", s->ssthresh);

	if (s->bytes_acked)
		printf(" bytes_acked:%llu", s->bytes_acked);
	if (s->bytes_received)
		printf(" bytes_received:%llu", s->bytes_received);
	if (s->segs_out)
		printf(" segs_out:%u", s->segs_out);
	if (s->segs_in)
		printf(" segs_in:%u", s->segs_in);

	if (s->dctcp && s->dctcp->enabled) {
		struct dctcpstat *dctcp = s->dctcp;

		printf(" dctcp:(ce_state:%u,alpha:%u,ab_ecn:%u,ab_tot:%u)",
				dctcp->ce_state, dctcp->alpha, dctcp->ab_ecn,
				dctcp->ab_tot);
	} else if (s->dctcp) {
		printf(" dctcp:fallback_mode");
	}

	if (s->send_bps)
		printf(" send %sbps", sprint_bw(b1, s->send_bps));
	if (s->lastsnd)
		printf(" lastsnd:%u", s->lastsnd);
	if (s->lastrcv)
		printf(" lastrcv:%u", s->lastrcv);
	if (s->lastack)
		printf(" lastack:%u", s->lastack);

	if (s->pacing_rate) {
		printf(" pacing_rate %sbps", sprint_bw(b1, s->pacing_rate));
		if (s->pacing_rate_max)
				printf("/%sbps", sprint_bw(b1,
							s->pacing_rate_max));
	}

	if (s->unacked)
		printf(" unacked:%u", s->unacked);
	if (s->retrans || s->retrans_total)
		printf(" retrans:%u/%u", s->retrans, s->retrans_total);
	if (s->lost)
		printf(" lost:%u", s->lost);
	if (s->sacked && s->ss.state != SS_LISTEN)
		printf(" sacked:%u", s->sacked);
	if (s->fackets)
		printf(" fackets:%u", s->fackets);
	if (s->reordering != 3)
		printf(" reordering:%d", s->reordering);
	if (s->rcv_rtt)
		printf(" rcv_rtt:%g", s->rcv_rtt);
	if (s->rcv_space)
		printf(" rcv_space:%d", s->rcv_space);
}

static void tcp_timer_print(struct tcpstat *s)
{
	if (s->timer) {
		if (s->timer > 4)
			s->timer = 5;
		printf(" timer:(%s,%s,%d)",
				tmr_name[s->timer],
				print_ms_timer(s->timeout),
				s->retrans);
	}
}

static int tcp_show_line(char *line, const struct filter *f, int family)
{
	int rto = 0, ato = 0;
	struct tcpstat s = {};
	char *loc, *rem, *data;
	char opt[256];
	int n;
	int hz = get_user_hz();

	if (proc_inet_split_line(line, &loc, &rem, &data))
		return -1;

	int state = (data[1] >= 'A') ? (data[1] - 'A' + 10) : (data[1] - '0');
	if (!(f->states & (1 << state)))
		return 0;

	proc_parse_inet_addr(loc, rem, family, &s.ss);

	if (f->f && run_ssfilter(f->f, &s.ss) == 0)
		return 0;

	opt[0] = 0;
	n = sscanf(data, "%x %x:%x %x:%x %x %d %d %u %d %llx %d %d %d %d %d %[^\n]\n",
		   &s.ss.state, &s.ss.wq, &s.ss.rq,
		   &s.timer, &s.timeout, &s.retrans, &s.ss.uid, &s.probes,
		   &s.ss.ino, &s.ss.refcnt, &s.ss.sk, &rto, &ato, &s.qack, &s.cwnd,
		   &s.ssthresh, opt);

	if (n < 17)
		opt[0] = 0;

	if (n < 12) {
		rto = 0;
		s.cwnd = 2;
		s.ssthresh = -1;
		ato = s.qack = 0;
	}

	s.retrans   = s.timer != 1 ? s.probes : s.retrans;
	s.timeout   = (s.timeout * 1000 + hz - 1) / hz;
	s.ato	    = (double)ato / hz;
	s.qack	   /= 2;
	s.rto	    = (double)rto;
	s.ssthresh  = s.ssthresh == -1 ? 0 : s.ssthresh;
	s.rto	    = s.rto != 3 * hz  ? s.rto / hz : 0;

	inet_stats_print(&s.ss, IPPROTO_TCP);

	if (show_options)
		tcp_timer_print(&s);

	if (show_details) {
		sock_details_print(&s.ss);
		if (opt[0])
			printf(" opt:\"%s\"", opt);
	}

	if (show_tcpinfo)
		tcp_stats_print(&s);

	printf("\n");
	return 0;
}

static int generic_record_read(FILE *fp,
			       int (*worker)(char*, const struct filter *, int),
			       const struct filter *f, int fam)
{
	char line[256];

	/* skip header */
	if (fgets(line, sizeof(line), fp) == NULL)
		goto outerr;

	while (fgets(line, sizeof(line), fp) != NULL) {
		int n = strlen(line);
		if (n == 0 || line[n-1] != '\n') {
			errno = -EINVAL;
			return -1;
		}
		line[n-1] = 0;

		if (worker(line, f, fam) < 0)
			return 0;
	}
outerr:

	return ferror(fp) ? -1 : 0;
}

static void print_skmeminfo(struct rtattr *tb[], int attrtype)
{
	const __u32 *skmeminfo;

	if (!tb[attrtype]) {
		if (attrtype == INET_DIAG_SKMEMINFO) {
			if (!tb[INET_DIAG_MEMINFO])
				return;

			const struct inet_diag_meminfo *minfo =
				RTA_DATA(tb[INET_DIAG_MEMINFO]);

			printf(" mem:(r%u,w%u,f%u,t%u)",
					minfo->idiag_rmem,
					minfo->idiag_wmem,
					minfo->idiag_fmem,
					minfo->idiag_tmem);
		}
		return;
	}

	skmeminfo = RTA_DATA(tb[attrtype]);

	printf(" skmem:(r%u,rb%u,t%u,tb%u,f%u,w%u,o%u",
	       skmeminfo[SK_MEMINFO_RMEM_ALLOC],
	       skmeminfo[SK_MEMINFO_RCVBUF],
	       skmeminfo[SK_MEMINFO_WMEM_ALLOC],
	       skmeminfo[SK_MEMINFO_SNDBUF],
	       skmeminfo[SK_MEMINFO_FWD_ALLOC],
	       skmeminfo[SK_MEMINFO_WMEM_QUEUED],
	       skmeminfo[SK_MEMINFO_OPTMEM]);

	if (RTA_PAYLOAD(tb[attrtype]) >=
		(SK_MEMINFO_BACKLOG + 1) * sizeof(__u32))
		printf(",bl%u", skmeminfo[SK_MEMINFO_BACKLOG]);

	printf(")");
}

#define TCPI_HAS_OPT(info, opt) !!(info->tcpi_options & (opt))

static void tcp_show_info(const struct nlmsghdr *nlh, struct inet_diag_msg *r,
		struct rtattr *tb[])
{
	double rtt = 0;
	struct tcpstat s = {};

	s.ss.state = r->idiag_state;

	print_skmeminfo(tb, INET_DIAG_SKMEMINFO);

	if (tb[INET_DIAG_INFO]) {
		struct tcp_info *info;
		int len = RTA_PAYLOAD(tb[INET_DIAG_INFO]);

		/* workaround for older kernels with less fields */
		if (len < sizeof(*info)) {
			info = alloca(sizeof(*info));
			memcpy(info, RTA_DATA(tb[INET_DIAG_INFO]), len);
			memset((char *)info + len, 0, sizeof(*info) - len);
		} else
			info = RTA_DATA(tb[INET_DIAG_INFO]);

		if (show_options) {
			s.has_ts_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_TIMESTAMPS);
			s.has_sack_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_SACK);
			s.has_ecn_opt	   = TCPI_HAS_OPT(info, TCPI_OPT_ECN);
			s.has_ecnseen_opt  = TCPI_HAS_OPT(info, TCPI_OPT_ECN_SEEN);
			s.has_fastopen_opt = TCPI_HAS_OPT(info, TCPI_OPT_SYN_DATA);
		}

		if (tb[INET_DIAG_CONG])
			strncpy(s.cong_alg,
				rta_getattr_str(tb[INET_DIAG_CONG]),
				sizeof(s.cong_alg) - 1);

		if (TCPI_HAS_OPT(info, TCPI_OPT_WSCALE)) {
			s.has_wscale_opt  = true;
			s.snd_wscale	  = info->tcpi_snd_wscale;
			s.rcv_wscale	  = info->tcpi_rcv_wscale;
		}

		if (info->tcpi_rto && info->tcpi_rto != 3000000)
			s.rto = (double)info->tcpi_rto / 1000;

		s.backoff	 = info->tcpi_backoff;
		s.rtt		 = (double)info->tcpi_rtt / 1000;
		s.rttvar	 = (double)info->tcpi_rttvar / 1000;
		s.ato		 = (double)info->tcpi_ato / 1000;
		s.mss		 = info->tcpi_snd_mss;
		s.rcv_space	 = info->tcpi_rcv_space;
		s.rcv_rtt	 = (double)info->tcpi_rcv_rtt / 1000;
		s.lastsnd	 = info->tcpi_last_data_sent;
		s.lastrcv	 = info->tcpi_last_data_recv;
		s.lastack	 = info->tcpi_last_ack_recv;
		s.unacked	 = info->tcpi_unacked;
		s.retrans	 = info->tcpi_retrans;
		s.retrans_total  = info->tcpi_total_retrans;
		s.lost		 = info->tcpi_lost;
		s.sacked	 = info->tcpi_sacked;
		s.reordering	 = info->tcpi_reordering;
		s.rcv_space	 = info->tcpi_rcv_space;
		s.cwnd		 = info->tcpi_snd_cwnd;

		if (info->tcpi_snd_ssthresh < 0xFFFF)
			s.ssthresh = info->tcpi_snd_ssthresh;

		rtt = (double) info->tcpi_rtt;
		if (tb[INET_DIAG_VEGASINFO]) {
			const struct tcpvegas_info *vinfo
				= RTA_DATA(tb[INET_DIAG_VEGASINFO]);

			if (vinfo->tcpv_enabled &&
					vinfo->tcpv_rtt && vinfo->tcpv_rtt != 0x7fffffff)
				rtt =  vinfo->tcpv_rtt;
		}

		if (tb[INET_DIAG_DCTCPINFO]) {
			struct dctcpstat *dctcp = malloc(sizeof(struct
						dctcpstat));

			const struct tcp_dctcp_info *dinfo
				= RTA_DATA(tb[INET_DIAG_DCTCPINFO]);

			dctcp->enabled	= !!dinfo->dctcp_enabled;
			dctcp->ce_state = dinfo->dctcp_ce_state;
			dctcp->alpha	= dinfo->dctcp_alpha;
			dctcp->ab_ecn	= dinfo->dctcp_ab_ecn;
			dctcp->ab_tot	= dinfo->dctcp_ab_tot;
			s.dctcp		= dctcp;
		}

		if (rtt > 0 && info->tcpi_snd_mss && info->tcpi_snd_cwnd) {
			s.send_bps = (double) info->tcpi_snd_cwnd *
				(double)info->tcpi_snd_mss * 8000000. / rtt;
		}

		if (info->tcpi_pacing_rate &&
				info->tcpi_pacing_rate != ~0ULL) {
			s.pacing_rate = info->tcpi_pacing_rate * 8.;

			if (info->tcpi_max_pacing_rate &&
					info->tcpi_max_pacing_rate != ~0ULL)
				s.pacing_rate_max = info->tcpi_max_pacing_rate * 8.;
		}
		s.bytes_acked = info->tcpi_bytes_acked;
		s.bytes_received = info->tcpi_bytes_received;
		s.segs_out = info->tcpi_segs_out;
		s.segs_in = info->tcpi_segs_in;
		tcp_stats_print(&s);
		free(s.dctcp);
	}
}

static int inet_show_sock(struct nlmsghdr *nlh, struct filter *f, int protocol)
{
	struct rtattr * tb[INET_DIAG_MAX+1];
	struct inet_diag_msg *r = NLMSG_DATA(nlh);
	struct sockstat s = {};

	parse_rtattr(tb, INET_DIAG_MAX, (struct rtattr*)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	s.state		= r->idiag_state;
	s.local.family  = s.remote.family = r->idiag_family;
	s.lport		= ntohs(r->id.idiag_sport);
	s.rport		= ntohs(r->id.idiag_dport);
	s.wq		= r->idiag_wqueue;
	s.rq		= r->idiag_rqueue;
	s.ino		= r->idiag_inode;
	s.uid		= r->idiag_uid;
	s.iface		= r->id.idiag_if;
	s.sk		= cookie_sk_get(&r->id.idiag_cookie[0]);

	if (s.local.family == AF_INET) {
		s.local.bytelen = s.remote.bytelen = 4;
	} else {
		s.local.bytelen = s.remote.bytelen = 16;
	}

	memcpy(s.local.data, r->id.idiag_src, s.local.bytelen);
	memcpy(s.remote.data, r->id.idiag_dst, s.local.bytelen);

	if (f && f->f && run_ssfilter(f->f, &s) == 0)
		return 0;

	if (tb[INET_DIAG_PROTOCOL])
		protocol = *(__u8 *)RTA_DATA(tb[INET_DIAG_PROTOCOL]);

	inet_stats_print(&s, protocol);

	if (show_options) {
		struct tcpstat t = {};

		t.timer = r->idiag_timer;
		t.timeout = r->idiag_expires;
		t.retrans = r->idiag_retrans;
		tcp_timer_print(&t);
	}

	if (show_details) {
		sock_details_print(&s);
		if (s.local.family == AF_INET6 && tb[INET_DIAG_SKV6ONLY]) {
			unsigned char v6only;
			v6only = *(__u8 *)RTA_DATA(tb[INET_DIAG_SKV6ONLY]);
			printf(" v6only:%u", v6only);
		}
		if (tb[INET_DIAG_SHUTDOWN]) {
			unsigned char mask;
			mask = *(__u8 *)RTA_DATA(tb[INET_DIAG_SHUTDOWN]);
			printf(" %c-%c", mask & 1 ? '-' : '<', mask & 2 ? '-' : '>');
		}
	}

	if (show_mem || show_tcpinfo) {
		printf("\n\t");
		tcp_show_info(nlh, r, tb);
	}

	printf("\n");
	return 0;
}

static int tcpdiag_send(int fd, int protocol, struct filter *f)
{
	struct sockaddr_nl nladdr;
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req r;
	} req;
	char    *bc = NULL;
	int	bclen;
	struct msghdr msg;
	struct rtattr rta;
	struct iovec iov[3];

	if (protocol == IPPROTO_UDP)
		return -1;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	if (protocol == IPPROTO_TCP)
		req.nlh.nlmsg_type = TCPDIAG_GETSOCK;
	else
		req.nlh.nlmsg_type = DCCPDIAG_GETSOCK;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = MAGIC_SEQ;
	memset(&req.r, 0, sizeof(req.r));
	req.r.idiag_family = AF_INET;
	req.r.idiag_states = f->states;
	if (show_mem) {
		req.r.idiag_ext |= (1<<(INET_DIAG_MEMINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_SKMEMINFO-1));
	}

	if (show_tcpinfo) {
		req.r.idiag_ext |= (1<<(INET_DIAG_INFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_VEGASINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_CONG-1));
	}

	iov[0] = (struct iovec){
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	if (f->f) {
		bclen = ssfilter_bytecompile(f->f, &bc);
		rta.rta_type = INET_DIAG_REQ_BYTECODE;
		rta.rta_len = RTA_LENGTH(bclen);
		iov[1] = (struct iovec){ &rta, sizeof(rta) };
		iov[2] = (struct iovec){ bc, bclen };
		req.nlh.nlmsg_len += RTA_LENGTH(bclen);
	}

	msg = (struct msghdr) {
		.msg_name = (void*)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = f->f ? 3 : 1,
	};

	if (sendmsg(fd, &msg, 0) < 0) {
		close(fd);
		return -1;
	}

	return 0;
}

static int sockdiag_send(int family, int fd, int protocol, struct filter *f)
{
	struct sockaddr_nl nladdr;
	DIAG_REQUEST(req, struct inet_diag_req_v2 r);
	char    *bc = NULL;
	int	bclen;
	struct msghdr msg;
	struct rtattr rta;
	struct iovec iov[3];

	if (family == PF_UNSPEC)
		return tcpdiag_send(fd, protocol, f);

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	memset(&req.r, 0, sizeof(req.r));
	req.r.sdiag_family = family;
	req.r.sdiag_protocol = protocol;
	req.r.idiag_states = f->states;
	if (show_mem) {
		req.r.idiag_ext |= (1<<(INET_DIAG_MEMINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_SKMEMINFO-1));
	}

	if (show_tcpinfo) {
		req.r.idiag_ext |= (1<<(INET_DIAG_INFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_VEGASINFO-1));
		req.r.idiag_ext |= (1<<(INET_DIAG_CONG-1));
	}

	iov[0] = (struct iovec){
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	if (f->f) {
		bclen = ssfilter_bytecompile(f->f, &bc);
		rta.rta_type = INET_DIAG_REQ_BYTECODE;
		rta.rta_len = RTA_LENGTH(bclen);
		iov[1] = (struct iovec){ &rta, sizeof(rta) };
		iov[2] = (struct iovec){ bc, bclen };
		req.nlh.nlmsg_len += RTA_LENGTH(bclen);
	}

	msg = (struct msghdr) {
		.msg_name = (void*)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = f->f ? 3 : 1,
	};

	if (sendmsg(fd, &msg, 0) < 0) {
		close(fd);
		return -1;
	}

	return 0;
}

struct inet_diag_arg {
	struct filter *f;
	int protocol;
};

static int show_one_inet_sock(const struct sockaddr_nl *addr,
		struct nlmsghdr *h, void *arg)
{
	int err;
	struct inet_diag_arg *diag_arg = arg;
	struct inet_diag_msg *r = NLMSG_DATA(h);

	if (!(diag_arg->f->families & (1 << r->idiag_family)))
		return 0;
	if ((err = inet_show_sock(h, diag_arg->f, diag_arg->protocol)) < 0)
		return err;

	return 0;
}

static int inet_show_netlink(struct filter *f, FILE *dump_fp, int protocol)
{
	int err = 0;
	struct rtnl_handle rth;
	int family = PF_INET;
	struct inet_diag_arg arg = { .f = f, .protocol = protocol };

	if (rtnl_open_byproto(&rth, 0, NETLINK_SOCK_DIAG))
		return -1;
	rth.dump = MAGIC_SEQ;
	rth.dump_fp = dump_fp;
	if (preferred_family == PF_INET6)
		family = PF_INET6;

again:
	if ((err = sockdiag_send(family, rth.fd, protocol, f)))
		goto Exit;

	if ((err = rtnl_dump_filter(&rth, show_one_inet_sock, &arg))) {
		if (family != PF_UNSPEC) {
			family = PF_UNSPEC;
			goto again;
		}
		goto Exit;
	}
	if (family == PF_INET && preferred_family != PF_INET) {
		family = PF_INET6;
		goto again;
	}

Exit:
	rtnl_close(&rth);
	return err;
}

static int tcp_show_netlink_file(struct filter *f)
{
	FILE	*fp;
	char	buf[16384];

	if ((fp = fopen(getenv("TCPDIAG_FILE"), "r")) == NULL) {
		perror("fopen($TCPDIAG_FILE)");
		return -1;
	}

	while (1) {
		int status, err;
		struct nlmsghdr *h = (struct nlmsghdr*)buf;

		status = fread(buf, 1, sizeof(*h), fp);
		if (status < 0) {
			perror("Reading header from $TCPDIAG_FILE");
			return -1;
		}
		if (status != sizeof(*h)) {
			perror("Unexpected EOF reading $TCPDIAG_FILE");
			return -1;
		}

		status = fread(h+1, 1, NLMSG_ALIGN(h->nlmsg_len-sizeof(*h)), fp);

		if (status < 0) {
			perror("Reading $TCPDIAG_FILE");
			return -1;
		}
		if (status + sizeof(*h) < h->nlmsg_len) {
			perror("Unexpected EOF reading $TCPDIAG_FILE");
			return -1;
		}

		/* The only legal exit point */
		if (h->nlmsg_type == NLMSG_DONE)
			return 0;

		if (h->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
			if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
				fprintf(stderr, "ERROR truncated\n");
			} else {
				errno = -err->error;
				perror("TCPDIAG answered");
			}
			return -1;
		}

		err = inet_show_sock(h, f, IPPROTO_TCP);
		if (err < 0)
			return err;
	}
}

static int tcp_show(struct filter *f, int socktype)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int bufsize = 64*1024;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = TCP_PROTO;

	if (getenv("TCPDIAG_FILE"))
		return tcp_show_netlink_file(f);

	if (!getenv("PROC_NET_TCP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, socktype) == 0)
		return 0;

	/* Sigh... We have to parse /proc/net/tcp... */


	/* Estimate amount of sockets and try to allocate
	 * huge buffer to read all the table at one read.
	 * Limit it by 16MB though. The assumption is: as soon as
	 * kernel was able to hold information about N connections,
	 * it is able to give us some memory for snapshot.
	 */
	if (1) {
		get_slabstat(&slabstat);

		int guess = slabstat.socks+slabstat.tcp_syns;
		if (f->states&(1<<SS_TIME_WAIT))
			guess += slabstat.tcp_tws;
		if (guess > (16*1024*1024)/128)
			guess = (16*1024*1024)/128;
		guess *= 128;
		if (guess > bufsize)
			bufsize = guess;
	}
	while (bufsize >= 64*1024) {
		if ((buf = malloc(bufsize)) != NULL)
			break;
		bufsize /= 2;
	}
	if (buf == NULL) {
		errno = ENOMEM;
		return -1;
	}

	if (f->families & (1<<AF_INET)) {
		if ((fp = net_tcp_open()) == NULL)
			goto outerr;

		setbuffer(fp, buf, bufsize);
		if (generic_record_read(fp, tcp_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families & (1<<AF_INET6)) &&
	    (fp = net_tcp6_open()) != NULL) {
		setbuffer(fp, buf, bufsize);
		if (generic_record_read(fp, tcp_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}

	free(buf);
	return 0;

outerr:
	do {
		int saved_errno = errno;
		free(buf);
		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}


static int dgram_show_line(char *line, const struct filter *f, int family)
{
	struct sockstat s = {};
	char *loc, *rem, *data;
	char opt[256];
	int n;

	if (proc_inet_split_line(line, &loc, &rem, &data))
		return -1;

	int state = (data[1] >= 'A') ? (data[1] - 'A' + 10) : (data[1] - '0');
	if (!(f->states & (1 << state)))
		return 0;

	proc_parse_inet_addr(loc, rem, family, &s);

	if (f->f && run_ssfilter(f->f, &s) == 0)
		return 0;

	opt[0] = 0;
	n = sscanf(data, "%x %x:%x %*x:%*x %*x %d %*d %u %d %llx %[^\n]\n",
	       &s.state, &s.wq, &s.rq,
	       &s.uid, &s.ino,
	       &s.refcnt, &s.sk, opt);

	if (n < 9)
		opt[0] = 0;

	inet_stats_print(&s, dg_proto == UDP_PROTO ? IPPROTO_UDP : 0);

	if (show_details && opt[0])
		printf(" opt:\"%s\"", opt);

	printf("\n");
	return 0;
}

static int udp_show(struct filter *f)
{
	FILE *fp = NULL;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = UDP_PROTO;

	if (!getenv("PROC_NET_UDP") && !getenv("PROC_ROOT")
	    && inet_show_netlink(f, NULL, IPPROTO_UDP) == 0)
		return 0;

	if (f->families&(1<<AF_INET)) {
		if ((fp = net_udp_open()) == NULL)
			goto outerr;
		if (generic_record_read(fp, dgram_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families&(1<<AF_INET6)) &&
	    (fp = net_udp6_open()) != NULL) {
		if (generic_record_read(fp, dgram_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}
	return 0;

outerr:
	do {
		int saved_errno = errno;
		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}

static int raw_show(struct filter *f)
{
	FILE *fp = NULL;

	if (!filter_af_get(f, AF_INET) && !filter_af_get(f, AF_INET6))
		return 0;

	dg_proto = RAW_PROTO;

	if (f->families&(1<<AF_INET)) {
		if ((fp = net_raw_open()) == NULL)
			goto outerr;
		if (generic_record_read(fp, dgram_show_line, f, AF_INET))
			goto outerr;
		fclose(fp);
	}

	if ((f->families&(1<<AF_INET6)) &&
	    (fp = net_raw6_open()) != NULL) {
		if (generic_record_read(fp, dgram_show_line, f, AF_INET6))
			goto outerr;
		fclose(fp);
	}
	return 0;

outerr:
	do {
		int saved_errno = errno;
		if (fp)
			fclose(fp);
		errno = saved_errno;
		return -1;
	} while (0);
}

int unix_state_map[] = { SS_CLOSE, SS_SYN_SENT,
			 SS_ESTABLISHED, SS_CLOSING };

#define MAX_UNIX_REMEMBER (1024*1024/sizeof(struct sockstat))

static void unix_list_free(struct sockstat *list)
{
	while (list) {
		struct sockstat *s = list;

		list = list->next;
		free(s->name);
		free(s);
	}
}

static const char *unix_netid_name(int type)
{
	const char *netid;

	switch (type) {
	case SOCK_STREAM:
		netid = "u_str";
		break;
	case SOCK_SEQPACKET:
		netid = "u_seq";
		break;
	case SOCK_DGRAM:
	default:
		netid = "u_dgr";
		break;
	}
	return netid;
}

static bool unix_type_skip(struct sockstat *s, struct filter *f)
{
	if (s->type == SOCK_STREAM && !(f->dbs&(1<<UNIX_ST_DB)))
		return true;
	if (s->type == SOCK_DGRAM && !(f->dbs&(1<<UNIX_DG_DB)))
		return true;
	if (s->type == SOCK_SEQPACKET && !(f->dbs&(1<<UNIX_SQ_DB)))
		return true;
	return false;
}

static bool unix_use_proc(void)
{
	return getenv("PROC_NET_UNIX") || getenv("PROC_ROOT");
}

static void unix_stats_print(struct sockstat *list, struct filter *f)
{
	struct sockstat *s;
	char *peer;
	char *ctx_buf = NULL;
	bool use_proc = unix_use_proc();
	char port_name[30] = {};

	for (s = list; s; s = s->next) {
		if (!(f->states & (1 << s->state)))
			continue;
		if (unix_type_skip(s, f))
			continue;

		peer = "*";
		if (s->peer_name)
			peer = s->peer_name;

		if (s->rport && use_proc) {
			struct sockstat *p;

			for (p = list; p; p = p->next) {
				if (s->rport == p->lport)
					break;
			}

			if (!p) {
				peer = "?";
			} else {
				peer = p->name ? : "*";
			}
		}

		if (use_proc && f->f) {
			struct sockstat st;
			st.local.family = AF_UNIX;
			st.remote.family = AF_UNIX;
			memcpy(st.local.data, &s->name, sizeof(s->name));
			if (strcmp(peer, "*") == 0)
				memset(st.remote.data, 0, sizeof(peer));
			else
				memcpy(st.remote.data, &peer, sizeof(peer));
			if (run_ssfilter(f->f, &st) == 0)
				continue;
		}

		sock_state_print(s, unix_netid_name(s->type));

		sock_addr_print(s->name ?: "*", " ",
				int_to_str(s->lport, port_name), NULL);
		sock_addr_print(peer, " ", int_to_str(s->rport, port_name),
				NULL);

		if (show_proc_ctx || show_sock_ctx) {
			if (find_entry(s->ino, &ctx_buf,
					(show_proc_ctx & show_sock_ctx) ?
					PROC_SOCK_CTX : PROC_CTX) > 0) {
				printf(" users:(%s)", ctx_buf);
				free(ctx_buf);
			}
		} else if (show_users) {
			if (find_entry(s->ino, &ctx_buf, USERS) > 0) {
				printf(" users:(%s)", ctx_buf);
				free(ctx_buf);
			}
		}
		printf("\n");
	}
}

static int unix_show_sock(const struct sockaddr_nl *addr, struct nlmsghdr *nlh,
		void *arg)
{
	struct filter *f = (struct filter *)arg;
	struct unix_diag_msg *r = NLMSG_DATA(nlh);
	struct rtattr *tb[UNIX_DIAG_MAX+1];
	char name[128];
	struct sockstat stat = { .name = "*", .peer_name = "*" };

	parse_rtattr(tb, UNIX_DIAG_MAX, (struct rtattr*)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	stat.type  = r->udiag_type;
	stat.state = r->udiag_state;
	stat.ino   = stat.lport = r->udiag_ino;
	stat.local.family = stat.remote.family = AF_UNIX;

	if (unix_type_skip(&stat, f))
		return 0;

	if (tb[UNIX_DIAG_RQLEN]) {
		struct unix_diag_rqlen *rql = RTA_DATA(tb[UNIX_DIAG_RQLEN]);
		stat.rq = rql->udiag_rqueue;
		stat.wq = rql->udiag_wqueue;
	}
	if (tb[UNIX_DIAG_NAME]) {
		int len = RTA_PAYLOAD(tb[UNIX_DIAG_NAME]);

		memcpy(name, RTA_DATA(tb[UNIX_DIAG_NAME]), len);
		name[len] = '\0';
		if (name[0] == '\0')
			name[0] = '@';
		stat.name = &name[0];
		memcpy(stat.local.data, &stat.name, sizeof(stat.name));
	}
	if (tb[UNIX_DIAG_PEER])
		stat.rport = rta_getattr_u32(tb[UNIX_DIAG_PEER]);

	if (f->f && run_ssfilter(f->f, &stat) == 0)
		return 0;

	unix_stats_print(&stat, f);

	if (show_mem) {
		printf("\t");
		print_skmeminfo(tb, UNIX_DIAG_MEMINFO);
	}
	if (show_details) {
		if (tb[UNIX_DIAG_SHUTDOWN]) {
			unsigned char mask;
			mask = *(__u8 *)RTA_DATA(tb[UNIX_DIAG_SHUTDOWN]);
			printf(" %c-%c", mask & 1 ? '-' : '<', mask & 2 ? '-' : '>');
		}
	}
	if (show_mem || show_details)
		printf("\n");

	return 0;
}

static int handle_netlink_request(struct filter *f, struct nlmsghdr *req,
		size_t size, rtnl_filter_t show_one_sock)
{
	int ret = -1;
	struct rtnl_handle rth;

	if (rtnl_open_byproto(&rth, 0, NETLINK_SOCK_DIAG))
		return -1;

	rth.dump = MAGIC_SEQ;

	if (rtnl_send(&rth, req, size) < 0)
		goto Exit;

	if (rtnl_dump_filter(&rth, show_one_sock, f))
		goto Exit;

	ret = 0;
Exit:
	rtnl_close(&rth);
	return ret;
}

static int unix_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct unix_diag_req r);

	req.r.sdiag_family = AF_UNIX;
	req.r.udiag_states = f->states;
	req.r.udiag_show = UDIAG_SHOW_NAME | UDIAG_SHOW_PEER | UDIAG_SHOW_RQLEN;
	if (show_mem)
		req.r.udiag_show |= UDIAG_SHOW_MEMINFO;

	return handle_netlink_request(f, &req.nlh, sizeof(req), unix_show_sock);
}

static int unix_show(struct filter *f)
{
	FILE *fp;
	char buf[256];
	char name[128];
	int  newformat = 0;
	int  cnt;
	struct sockstat *list = NULL;

	if (!filter_af_get(f, AF_UNIX))
		return 0;

	if (!unix_use_proc() && unix_show_netlink(f) == 0)
		return 0;

	if ((fp = net_unix_open()) == NULL)
		return -1;
	fgets(buf, sizeof(buf)-1, fp);

	if (memcmp(buf, "Peer", 4) == 0)
		newformat = 1;
	cnt = 0;

	while (fgets(buf, sizeof(buf)-1, fp)) {
		struct sockstat *u, **insp;
		int flags;

		if (!(u = calloc(1, sizeof(*u))))
			break;
		u->name = NULL;
		u->peer_name = NULL;

		if (sscanf(buf, "%x: %x %x %x %x %x %d %s",
			   &u->rport, &u->rq, &u->wq, &flags, &u->type,
			   &u->state, &u->ino, name) < 8)
			name[0] = 0;

		u->lport = u->ino;
		u->local.family = u->remote.family = AF_UNIX;

		if (flags & (1 << 16)) {
			u->state = SS_LISTEN;
		} else {
			u->state = unix_state_map[u->state-1];
			if (u->type == SOCK_DGRAM && u->state == SS_CLOSE && u->rport)
				u->state = SS_ESTABLISHED;
		}

		if (!newformat) {
			u->rport = 0;
			u->rq = 0;
			u->wq = 0;
		}

		insp = &list;
		while (*insp) {
			if (u->type < (*insp)->type ||
			    (u->type == (*insp)->type &&
			     u->ino < (*insp)->ino))
				break;
			insp = &(*insp)->next;
		}
		u->next = *insp;
		*insp = u;

		if (name[0]) {
			if ((u->name = malloc(strlen(name)+1)) == NULL)
				break;
			strcpy(u->name, name);
		}
		if (++cnt > MAX_UNIX_REMEMBER) {
			unix_stats_print(list, f);
			unix_list_free(list);
			list = NULL;
			cnt = 0;
		}
	}
	fclose(fp);
	if (list) {
		unix_stats_print(list, f);
		unix_list_free(list);
		list = NULL;
		cnt = 0;
	}

	return 0;
}

static int packet_stats_print(struct sockstat *s, const struct filter *f)
{
	char *buf = NULL;
	const char *addr, *port;
	char ll_name[16];

	if (f->f) {
		s->local.family = AF_PACKET;
		s->remote.family = AF_PACKET;
		s->local.data[0] = s->prot;
		if (run_ssfilter(f->f, s) == 0)
			return 1;
	}

	sock_state_print(s, s->type == SOCK_RAW ? "p_raw" : "p_dgr");

	if (s->prot == 3)
		addr = "*";
	else
		addr = ll_proto_n2a(htons(s->prot), ll_name, sizeof(ll_name));

	if (s->iface == 0)
		port = "*";
	else
		port = xll_index_to_name(s->iface);

	sock_addr_print(addr, ":", port, NULL);
	sock_addr_print("", "*", "", NULL);

	if (show_proc_ctx || show_sock_ctx) {
		if (find_entry(s->ino, &buf,
					(show_proc_ctx & show_sock_ctx) ?
					PROC_SOCK_CTX : PROC_CTX) > 0) {
			printf(" users:(%s)", buf);
			free(buf);
		}
	} else if (show_users) {
		if (find_entry(s->ino, &buf, USERS) > 0) {
			printf(" users:(%s)", buf);
			free(buf);
		}
	}

	if (show_details)
		sock_details_print(s);

	return 0;
}

static void packet_show_ring(struct packet_diag_ring *ring)
{
	printf("blk_size:%d", ring->pdr_block_size);
	printf(",blk_nr:%d", ring->pdr_block_nr);
	printf(",frm_size:%d", ring->pdr_frame_size);
	printf(",frm_nr:%d", ring->pdr_frame_nr);
	printf(",tmo:%d", ring->pdr_retire_tmo);
	printf(",features:0x%x", ring->pdr_features);
}

static int packet_show_sock(const struct sockaddr_nl *addr,
		struct nlmsghdr *nlh, void *arg)
{
	const struct filter *f = arg;
	struct packet_diag_msg *r = NLMSG_DATA(nlh);
	struct packet_diag_info *pinfo = NULL;
	struct packet_diag_ring *ring_rx = NULL, *ring_tx = NULL;
	struct rtattr *tb[PACKET_DIAG_MAX+1];
	struct sockstat stat = {};
	uint32_t fanout = 0;
	bool has_fanout = false;

	parse_rtattr(tb, PACKET_DIAG_MAX, (struct rtattr*)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	/* use /proc/net/packet if all info are not available */
	if (!tb[PACKET_DIAG_MEMINFO])
		return -1;

	stat.type   = r->pdiag_type;
	stat.prot   = r->pdiag_num;
	stat.ino    = r->pdiag_ino;
	stat.state  = SS_CLOSE;
	stat.sk	    = cookie_sk_get(&r->pdiag_cookie[0]);

	if (tb[PACKET_DIAG_MEMINFO]) {
		__u32 *skmeminfo = RTA_DATA(tb[PACKET_DIAG_MEMINFO]);
		stat.rq = skmeminfo[SK_MEMINFO_RMEM_ALLOC];
	}

	if (tb[PACKET_DIAG_INFO]) {
		pinfo = RTA_DATA(tb[PACKET_DIAG_INFO]);
		stat.lport = stat.iface = pinfo->pdi_index;
	}

	if (tb[PACKET_DIAG_UID])
		stat.uid = *(__u32 *)RTA_DATA(tb[PACKET_DIAG_UID]);

	if (tb[PACKET_DIAG_RX_RING])
		ring_rx = RTA_DATA(tb[PACKET_DIAG_RX_RING]);

	if (tb[PACKET_DIAG_TX_RING])
		ring_tx = RTA_DATA(tb[PACKET_DIAG_TX_RING]);

	if (tb[PACKET_DIAG_FANOUT]) {
		has_fanout = true;
		fanout = *(uint32_t *)RTA_DATA(tb[PACKET_DIAG_FANOUT]);
	}

	if (packet_stats_print(&stat, f))
		return 0;

	if (show_details) {
		if (pinfo) {
			printf("\n\tver:%d", pinfo->pdi_version);
			printf(" cpy_thresh:%d", pinfo->pdi_copy_thresh);
			printf(" flags( ");
			if (pinfo->pdi_flags & PDI_RUNNING)
				printf("running");
			if (pinfo->pdi_flags & PDI_AUXDATA)
				printf(" auxdata");
			if (pinfo->pdi_flags & PDI_ORIGDEV)
				printf(" origdev");
			if (pinfo->pdi_flags & PDI_VNETHDR)
				printf(" vnethdr");
			if (pinfo->pdi_flags & PDI_LOSS)
				printf(" loss");
			if (!pinfo->pdi_flags)
				printf("0");
			printf(" )");
		}
		if (ring_rx) {
			printf("\n\tring_rx(");
			packet_show_ring(ring_rx);
			printf(")");
		}
		if (ring_tx) {
			printf("\n\tring_tx(");
			packet_show_ring(ring_tx);
			printf(")");
		}
		if (has_fanout) {
			uint16_t type = (fanout >> 16) & 0xffff;

			printf("\n\tfanout(");
			printf("id:%d,", fanout & 0xffff);
			printf("type:");

			if (type == 0)
				printf("hash");
			else if (type == 1)
				printf("lb");
			else if (type == 2)
				printf("cpu");
			else if (type == 3)
				printf("roll");
			else if (type == 4)
				printf("random");
			else if (type == 5)
				printf("qm");
			else
				printf("0x%x", type);

			printf(")");
		}
	}

	if (show_bpf && tb[PACKET_DIAG_FILTER]) {
		struct sock_filter *fil =
		       RTA_DATA(tb[PACKET_DIAG_FILTER]);
		int num = RTA_PAYLOAD(tb[PACKET_DIAG_FILTER]) /
			  sizeof(struct sock_filter);

		printf("\n\tbpf filter (%d): ", num);
		while (num) {
			printf(" 0x%02x %u %u %u,",
			      fil->code, fil->jt, fil->jf, fil->k);
			num--;
			fil++;
		}
	}
	printf("\n");
	return 0;
}

static int packet_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct packet_diag_req r);

	req.r.sdiag_family = AF_PACKET;
	req.r.pdiag_show = PACKET_SHOW_INFO | PACKET_SHOW_MEMINFO |
		PACKET_SHOW_FILTER | PACKET_SHOW_RING_CFG | PACKET_SHOW_FANOUT;

	return handle_netlink_request(f, &req.nlh, sizeof(req), packet_show_sock);
}

static int packet_show_line(char *buf, const struct filter *f, int fam)
{
	unsigned long long sk;
	struct sockstat stat = {};
	int type, prot, iface, state, rq, uid, ino;

	sscanf(buf, "%llx %*d %d %x %d %d %u %u %u",
			&sk,
			&type, &prot, &iface, &state,
			&rq, &uid, &ino);

	if (stat.type == SOCK_RAW && !(f->dbs&(1<<PACKET_R_DB)))
		return 0;
	if (stat.type == SOCK_DGRAM && !(f->dbs&(1<<PACKET_DG_DB)))
		return 0;

	stat.type  = type;
	stat.prot  = prot;
	stat.lport = stat.iface = iface;
	stat.state = state;
	stat.rq    = rq;
	stat.uid   = uid;
	stat.ino   = ino;
	stat.state = SS_CLOSE;

	if (packet_stats_print(&stat, f))
		return 0;

	printf("\n");
	return 0;
}

static int packet_show(struct filter *f)
{
	FILE *fp;
	int rc = 0;

	if (!filter_af_get(f, AF_PACKET) || !(f->states & (1 << SS_CLOSE)))
		return 0;

	if (!getenv("PROC_NET_PACKET") && !getenv("PROC_ROOT") &&
			packet_show_netlink(f) == 0)
		return 0;

	if ((fp = net_packet_open()) == NULL)
		return -1;
	if (generic_record_read(fp, packet_show_line, f, AF_PACKET))
		rc = -1;

	fclose(fp);
	return rc;
}

static int netlink_show_one(struct filter *f,
				int prot, int pid, unsigned groups,
				int state, int dst_pid, unsigned dst_group,
				int rq, int wq,
				unsigned long long sk, unsigned long long cb)
{
	struct sockstat st;
	SPRINT_BUF(prot_buf) = {};
	const char *prot_name;
	char procname[64] = {};

	st.state = SS_CLOSE;
	st.rq	 = rq;
	st.wq	 = wq;

	if (f->f) {
		st.local.family = AF_NETLINK;
		st.remote.family = AF_NETLINK;
		st.rport = -1;
		st.lport = pid;
		st.local.data[0] = prot;
		if (run_ssfilter(f->f, &st) == 0)
			return 1;
	}

	sock_state_print(&st, "nl");

	if (resolve_services)
		prot_name = nl_proto_n2a(prot, prot_buf, sizeof(prot_buf));
	else
		prot_name = int_to_str(prot, prot_buf);

	if (pid == -1) {
		procname[0] = '*';
	} else if (resolve_services) {
		int done = 0;
		if (!pid) {
			done = 1;
			strncpy(procname, "kernel", 6);
		} else if (pid > 0) {
			FILE *fp;
			snprintf(procname, sizeof(procname), "%s/%d/stat",
				getenv("PROC_ROOT") ? : "/proc", pid);
			if ((fp = fopen(procname, "r")) != NULL) {
				if (fscanf(fp, "%*d (%[^)])", procname) == 1) {
					snprintf(procname+strlen(procname),
						sizeof(procname)-strlen(procname),
						"/%d", pid);
					done = 1;
				}
				fclose(fp);
			}
		}
		if (!done)
			int_to_str(pid, procname);
	} else {
		int_to_str(pid, procname);
	}

	sock_addr_print(prot_name, ":", procname, NULL);

	if (state == NETLINK_CONNECTED) {
		char dst_group_buf[30];
		char dst_pid_buf[30];
		sock_addr_print(int_to_str(dst_group, dst_group_buf), ":",
				int_to_str(dst_pid, dst_pid_buf), NULL);
	} else {
		sock_addr_print("", "*", "", NULL);
	}

	char *pid_context = NULL;
	if (show_proc_ctx) {
		/* The pid value will either be:
		 *   0 if destination kernel - show kernel initial context.
		 *   A valid process pid - use getpidcon.
		 *   A unique value allocated by the kernel or netlink user
		 *   to the process - show context as "not available".
		 */
		if (!pid)
			security_get_initial_context("kernel", &pid_context);
		else if (pid > 0)
			getpidcon(pid, &pid_context);

		if (pid_context != NULL) {
			printf("proc_ctx=%-*s ", serv_width, pid_context);
			free(pid_context);
		} else {
			printf("proc_ctx=%-*s ", serv_width, "unavailable");
		}
	}

	if (show_details) {
		printf(" sk=%llx cb=%llx groups=0x%08x", sk, cb, groups);
	}
	printf("\n");

	return 0;
}

static int netlink_show_sock(const struct sockaddr_nl *addr,
		struct nlmsghdr *nlh, void *arg)
{
	struct filter *f = (struct filter *)arg;
	struct netlink_diag_msg *r = NLMSG_DATA(nlh);
	struct rtattr *tb[NETLINK_DIAG_MAX+1];
	int rq = 0, wq = 0;
	unsigned long groups = 0;

	parse_rtattr(tb, NETLINK_DIAG_MAX, (struct rtattr*)(r+1),
		     nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

	if (tb[NETLINK_DIAG_GROUPS] && RTA_PAYLOAD(tb[NETLINK_DIAG_GROUPS]))
		groups = *(unsigned long *) RTA_DATA(tb[NETLINK_DIAG_GROUPS]);

	if (tb[NETLINK_DIAG_MEMINFO]) {
		const __u32 *skmeminfo;
		skmeminfo = RTA_DATA(tb[NETLINK_DIAG_MEMINFO]);

		rq = skmeminfo[SK_MEMINFO_RMEM_ALLOC];
		wq = skmeminfo[SK_MEMINFO_WMEM_ALLOC];
	}

	if (netlink_show_one(f, r->ndiag_protocol, r->ndiag_portid, groups,
			 r->ndiag_state, r->ndiag_dst_portid, r->ndiag_dst_group,
			 rq, wq, 0, 0)) {
		return 0;
	}

	if (show_mem) {
		printf("\t");
		print_skmeminfo(tb, NETLINK_DIAG_MEMINFO);
		printf("\n");
	}

	return 0;
}

static int netlink_show_netlink(struct filter *f)
{
	DIAG_REQUEST(req, struct netlink_diag_req r);

	req.r.sdiag_family = AF_NETLINK;
	req.r.sdiag_protocol = NDIAG_PROTO_ALL;
	req.r.ndiag_show = NDIAG_SHOW_GROUPS | NDIAG_SHOW_MEMINFO;

	return handle_netlink_request(f, &req.nlh, sizeof(req), netlink_show_sock);
}

static int netlink_show(struct filter *f)
{
	FILE *fp;
	char buf[256];
	int prot, pid;
	unsigned groups;
	int rq, wq, rc;
	unsigned long long sk, cb;

	if (!filter_af_get(f, AF_NETLINK) || !(f->states & (1 << SS_CLOSE)))
		return 0;

	if (!getenv("PROC_NET_NETLINK") && !getenv("PROC_ROOT") &&
		netlink_show_netlink(f) == 0)
		return 0;

	if ((fp = net_netlink_open()) == NULL)
		return -1;
	fgets(buf, sizeof(buf)-1, fp);

	while (fgets(buf, sizeof(buf)-1, fp)) {
		sscanf(buf, "%llx %d %d %x %d %d %llx %d",
		       &sk,
		       &prot, &pid, &groups, &rq, &wq, &cb, &rc);

		netlink_show_one(f, prot, pid, groups, 0, 0, 0, rq, wq, sk, cb);
	}

	fclose(fp);
	return 0;
}

struct sock_diag_msg {
	__u8 sdiag_family;
};

static int generic_show_sock(const struct sockaddr_nl *addr,
		struct nlmsghdr *nlh, void *arg)
{
	struct sock_diag_msg *r = NLMSG_DATA(nlh);
	struct inet_diag_arg inet_arg = { .f = arg, .protocol = IPPROTO_MAX };

	switch (r->sdiag_family) {
	case AF_INET:
	case AF_INET6:
		return show_one_inet_sock(addr, nlh, &inet_arg);
	case AF_UNIX:
		return unix_show_sock(addr, nlh, arg);
	case AF_PACKET:
		return packet_show_sock(addr, nlh, arg);
	case AF_NETLINK:
		return netlink_show_sock(addr, nlh, arg);
	default:
		return -1;
	}
}

static int handle_follow_request(struct filter *f)
{
	int ret = -1;
	int groups = 0;
	struct rtnl_handle rth;

	if (f->families & (1 << AF_INET) && f->dbs & (1 << TCP_DB))
		groups |= 1 << (SKNLGRP_INET_TCP_DESTROY - 1);
	if (f->families & (1 << AF_INET) && f->dbs & (1 << UDP_DB))
		groups |= 1 << (SKNLGRP_INET_UDP_DESTROY - 1);
	if (f->families & (1 << AF_INET6) && f->dbs & (1 << TCP_DB))
		groups |= 1 << (SKNLGRP_INET6_TCP_DESTROY - 1);
	if (f->families & (1 << AF_INET6) && f->dbs & (1 << UDP_DB))
		groups |= 1 << (SKNLGRP_INET6_UDP_DESTROY - 1);

	if (groups == 0)
		return -1;

	if (rtnl_open_byproto(&rth, groups, NETLINK_SOCK_DIAG))
		return -1;

	rth.dump = 0;
	rth.local.nl_pid = 0;

	if (rtnl_dump_filter(&rth, generic_show_sock, f))
		goto Exit;

	ret = 0;
Exit:
	rtnl_close(&rth);
	return ret;
}

struct snmpstat
{
	int tcp_estab;
};

static int get_snmp_int(char *proto, char *key, int *result)
{
	char buf[1024];
	FILE *fp;
	int protolen = strlen(proto);
	int keylen = strlen(key);

	*result = 0;

	if ((fp = net_snmp_open()) == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p = buf;
		int  pos = 0;
		if (memcmp(buf, proto, protolen))
			continue;
		while ((p = strchr(p, ' ')) != NULL) {
			pos++;
			p++;
			if (memcmp(p, key, keylen) == 0 &&
			    (p[keylen] == ' ' || p[keylen] == '\n'))
				break;
		}
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		if (memcmp(buf, proto, protolen))
			break;
		p = buf;
		while ((p = strchr(p, ' ')) != NULL) {
			p++;
			if (--pos == 0) {
				sscanf(p, "%d", result);
				fclose(fp);
				return 0;
			}
		}
	}

	fclose(fp);
	errno = ESRCH;
	return -1;
}


/* Get stats from sockstat */

struct ssummary
{
	int socks;
	int tcp_mem;
	int tcp_total;
	int tcp_orphans;
	int tcp_tws;
	int tcp4_hashed;
	int udp4;
	int raw4;
	int frag4;
	int frag4_mem;
	int tcp6_hashed;
	int udp6;
	int raw6;
	int frag6;
	int frag6_mem;
};

static void get_sockstat_line(char *line, struct ssummary *s)
{
	char id[256], rem[256];

	if (sscanf(line, "%[^ ] %[^\n]\n", id, rem) != 2)
		return;

	if (strcmp(id, "sockets:") == 0)
		sscanf(rem, "%*s%d", &s->socks);
	else if (strcmp(id, "UDP:") == 0)
		sscanf(rem, "%*s%d", &s->udp4);
	else if (strcmp(id, "UDP6:") == 0)
		sscanf(rem, "%*s%d", &s->udp6);
	else if (strcmp(id, "RAW:") == 0)
		sscanf(rem, "%*s%d", &s->raw4);
	else if (strcmp(id, "RAW6:") == 0)
		sscanf(rem, "%*s%d", &s->raw6);
	else if (strcmp(id, "TCP6:") == 0)
		sscanf(rem, "%*s%d", &s->tcp6_hashed);
	else if (strcmp(id, "FRAG:") == 0)
		sscanf(rem, "%*s%d%*s%d", &s->frag4, &s->frag4_mem);
	else if (strcmp(id, "FRAG6:") == 0)
		sscanf(rem, "%*s%d%*s%d", &s->frag6, &s->frag6_mem);
	else if (strcmp(id, "TCP:") == 0)
		sscanf(rem, "%*s%d%*s%d%*s%d%*s%d%*s%d",
		       &s->tcp4_hashed,
		       &s->tcp_orphans, &s->tcp_tws, &s->tcp_total, &s->tcp_mem);
}

static int get_sockstat(struct ssummary *s)
{
	char buf[256];
	FILE *fp;

	memset(s, 0, sizeof(*s));

	if ((fp = net_sockstat_open()) == NULL)
		return -1;
	while(fgets(buf, sizeof(buf), fp) != NULL)
		get_sockstat_line(buf, s);
	fclose(fp);

	if ((fp = net_sockstat6_open()) == NULL)
		return 0;
	while(fgets(buf, sizeof(buf), fp) != NULL)
		get_sockstat_line(buf, s);
	fclose(fp);

	return 0;
}

static int print_summary(void)
{
	struct ssummary s;
	struct snmpstat sn;

	if (get_sockstat(&s) < 0)
		perror("ss: get_sockstat");
	if (get_snmp_int("Tcp:", "CurrEstab", &sn.tcp_estab) < 0)
		perror("ss: get_snmpstat");

	get_slabstat(&slabstat);

	printf("Total: %d (kernel %d)\n", s.socks, slabstat.socks);

	printf("TCP:   %d (estab %d, closed %d, orphaned %d, synrecv %d, timewait %d/%d), ports %d\n",
	       s.tcp_total + slabstat.tcp_syns + s.tcp_tws,
	       sn.tcp_estab,
	       s.tcp_total - (s.tcp4_hashed+s.tcp6_hashed-s.tcp_tws),
	       s.tcp_orphans,
	       slabstat.tcp_syns,
	       s.tcp_tws, slabstat.tcp_tws,
	       slabstat.tcp_ports
	       );

	printf("\n");
	printf("Transport Total     IP        IPv6\n");
	printf("*	  %-9d %-9s %-9s\n", slabstat.socks, "-", "-");
	printf("RAW	  %-9d %-9d %-9d\n", s.raw4+s.raw6, s.raw4, s.raw6);
	printf("UDP	  %-9d %-9d %-9d\n", s.udp4+s.udp6, s.udp4, s.udp6);
	printf("TCP	  %-9d %-9d %-9d\n", s.tcp4_hashed+s.tcp6_hashed, s.tcp4_hashed, s.tcp6_hashed);
	printf("INET	  %-9d %-9d %-9d\n",
	       s.raw4+s.udp4+s.tcp4_hashed+
	       s.raw6+s.udp6+s.tcp6_hashed,
	       s.raw4+s.udp4+s.tcp4_hashed,
	       s.raw6+s.udp6+s.tcp6_hashed);
	printf("FRAG	  %-9d %-9d %-9d\n", s.frag4+s.frag6, s.frag4, s.frag6);

	printf("\n");

	return 0;
}

static void _usage(FILE *dest)
{
	fprintf(dest,
"Usage: ss [ OPTIONS ]\n"
"       ss [ OPTIONS ] [ FILTER ]\n"
"   -h, --help          this message\n"
"   -V, --version       output version information\n"
"   -n, --numeric       don't resolve service names\n"
"   -r, --resolve       resolve host names\n"
"   -a, --all           display all sockets\n"
"   -l, --listening     display listening sockets\n"
"   -o, --options       show timer information\n"
"   -e, --extended      show detailed socket information\n"
"   -m, --memory        show socket memory usage\n"
"   -p, --processes     show process using socket\n"
"   -i, --info          show internal TCP information\n"
"   -s, --summary       show socket usage summary\n"
"   -b, --bpf           show bpf filter socket information\n"
"   -E, --events        continually display sockets as they are destroyed\n"
"   -Z, --context       display process SELinux security contexts\n"
"   -z, --contexts      display process and socket SELinux security contexts\n"
"   -N, --net           switch to the specified network namespace name\n"
"\n"
"   -4, --ipv4          display only IP version 4 sockets\n"
"   -6, --ipv6          display only IP version 6 sockets\n"
"   -0, --packet        display PACKET sockets\n"
"   -t, --tcp           display only TCP sockets\n"
"   -u, --udp           display only UDP sockets\n"
"   -d, --dccp          display only DCCP sockets\n"
"   -w, --raw           display only RAW sockets\n"
"   -x, --unix          display only Unix domain sockets\n"
"   -f, --family=FAMILY display sockets of type FAMILY\n"
"\n"
"   -A, --query=QUERY, --socket=QUERY\n"
"       QUERY := {all|inet|tcp|udp|raw|unix|unix_dgram|unix_stream|unix_seqpacket|packet|netlink}[,QUERY]\n"
"\n"
"   -D, --diag=FILE     Dump raw information about TCP sockets to FILE\n"
"   -F, --filter=FILE   read filter information from FILE\n"
"       FILTER := [ state STATE-FILTER ] [ EXPRESSION ]\n"
"       STATE-FILTER := {all|connected|synchronized|bucket|big|TCP-STATES}\n"
"         TCP-STATES := {established|syn-sent|syn-recv|fin-wait-{1,2}|time-wait|closed|close-wait|last-ack|listen|closing}\n"
"          connected := {established|syn-sent|syn-recv|fin-wait-{1,2}|time-wait|close-wait|last-ack|closing}\n"
"       synchronized := {established|syn-recv|fin-wait-{1,2}|time-wait|close-wait|last-ack|closing}\n"
"             bucket := {syn-recv|time-wait}\n"
"                big := {established|syn-sent|fin-wait-{1,2}|closed|close-wait|last-ack|listen|closing}\n"
		);
}

static void help(void) __attribute__((noreturn));
static void help(void)
{
	_usage(stdout);
	exit(0);
}

static void usage(void) __attribute__((noreturn));
static void usage(void)
{
	_usage(stderr);
	exit(-1);
}


static int scan_state(const char *state)
{
	int i;
	if (strcasecmp(state, "close") == 0 ||
	    strcasecmp(state, "closed") == 0)
		return (1<<SS_CLOSE);
	if (strcasecmp(state, "syn-rcv") == 0)
		return (1<<SS_SYN_RECV);
	if (strcasecmp(state, "established") == 0)
		return (1<<SS_ESTABLISHED);
	if (strcasecmp(state, "all") == 0)
		return SS_ALL;
	if (strcasecmp(state, "connected") == 0)
		return SS_ALL & ~((1<<SS_CLOSE)|(1<<SS_LISTEN));
	if (strcasecmp(state, "synchronized") == 0)
		return SS_ALL & ~((1<<SS_CLOSE)|(1<<SS_LISTEN)|(1<<SS_SYN_SENT));
	if (strcasecmp(state, "bucket") == 0)
		return (1<<SS_SYN_RECV)|(1<<SS_TIME_WAIT);
	if (strcasecmp(state, "big") == 0)
		return SS_ALL & ~((1<<SS_SYN_RECV)|(1<<SS_TIME_WAIT));
	for (i=0; i<SS_MAX; i++) {
		if (strcasecmp(state, sstate_namel[i]) == 0)
			return (1<<i);
	}

	fprintf(stderr, "ss: wrong state name: %s\n", state);
	exit(-1);
}

static const struct option long_opts[] = {
	{ "numeric", 0, 0, 'n' },
	{ "resolve", 0, 0, 'r' },
	{ "options", 0, 0, 'o' },
	{ "extended", 0, 0, 'e' },
	{ "memory", 0, 0, 'm' },
	{ "info", 0, 0, 'i' },
	{ "processes", 0, 0, 'p' },
	{ "bpf", 0, 0, 'b' },
	{ "events", 0, 0, 'E' },
	{ "dccp", 0, 0, 'd' },
	{ "tcp", 0, 0, 't' },
	{ "udp", 0, 0, 'u' },
	{ "raw", 0, 0, 'w' },
	{ "unix", 0, 0, 'x' },
	{ "all", 0, 0, 'a' },
	{ "listening", 0, 0, 'l' },
	{ "ipv4", 0, 0, '4' },
	{ "ipv6", 0, 0, '6' },
	{ "packet", 0, 0, '0' },
	{ "family", 1, 0, 'f' },
	{ "socket", 1, 0, 'A' },
	{ "query", 1, 0, 'A' },
	{ "summary", 0, 0, 's' },
	{ "diag", 1, 0, 'D' },
	{ "filter", 1, 0, 'F' },
	{ "version", 0, 0, 'V' },
	{ "help", 0, 0, 'h' },
	{ "context", 0, 0, 'Z' },
	{ "contexts", 0, 0, 'z' },
	{ "net", 1, 0, 'N' },
	{ 0 }

};

int main(int argc, char *argv[])
{
	int saw_states = 0;
	int saw_query = 0;
	int do_summary = 0;
	const char *dump_tcpdiag = NULL;
	FILE *filter_fp = NULL;
	int ch;
	int state_filter = 0;

	while ((ch = getopt_long(argc, argv, "dhaletuwxnro460spbEf:miA:D:F:vVzZN:",
				 long_opts, NULL)) != EOF) {
		switch(ch) {
		case 'n':
			resolve_services = 0;
			break;
		case 'r':
			resolve_hosts = 1;
			break;
		case 'o':
			show_options = 1;
			break;
		case 'e':
			show_options = 1;
			show_details++;
			break;
		case 'm':
			show_mem = 1;
			break;
		case 'i':
			show_tcpinfo = 1;
			break;
		case 'p':
			show_users++;
			user_ent_hash_build();
			break;
		case 'b':
			show_options = 1;
			show_bpf++;
			break;
		case 'E':
			follow_events = 1;
			break;
		case 'd':
			filter_db_set(&current_filter, DCCP_DB);
			break;
		case 't':
			filter_db_set(&current_filter, TCP_DB);
			break;
		case 'u':
			filter_db_set(&current_filter, UDP_DB);
			break;
		case 'w':
			filter_db_set(&current_filter, RAW_DB);
			break;
		case 'x':
			filter_af_set(&current_filter, AF_UNIX);
			break;
		case 'a':
			state_filter = SS_ALL;
			break;
		case 'l':
			state_filter = (1 << SS_LISTEN) | (1 << SS_CLOSE);
			break;
		case '4':
			filter_af_set(&current_filter, AF_INET);
			break;
		case '6':
			filter_af_set(&current_filter, AF_INET6);
			break;
		case '0':
			filter_af_set(&current_filter, AF_PACKET);
			break;
		case 'f':
			if (strcmp(optarg, "inet") == 0)
				filter_af_set(&current_filter, AF_INET);
			else if (strcmp(optarg, "inet6") == 0)
				filter_af_set(&current_filter, AF_INET6);
			else if (strcmp(optarg, "link") == 0)
				filter_af_set(&current_filter, AF_PACKET);
			else if (strcmp(optarg, "unix") == 0)
				filter_af_set(&current_filter, AF_UNIX);
			else if (strcmp(optarg, "netlink") == 0)
				filter_af_set(&current_filter, AF_NETLINK);
			else if (strcmp(optarg, "help") == 0)
				help();
			else {
				fprintf(stderr, "ss: \"%s\" is invalid family\n",
						optarg);
				usage();
			}
			break;
		case 'A':
		{
			char *p, *p1;
			if (!saw_query) {
				current_filter.dbs = 0;
				state_filter = state_filter ?
				               state_filter : SS_CONN;
				saw_query = 1;
				do_default = 0;
			}
			p = p1 = optarg;
			do {
				if ((p1 = strchr(p, ',')) != NULL)
					*p1 = 0;
				if (strcmp(p, "all") == 0) {
					filter_default_dbs(&current_filter);
				} else if (strcmp(p, "inet") == 0) {
					filter_db_set(&current_filter, UDP_DB);
					filter_db_set(&current_filter, DCCP_DB);
					filter_db_set(&current_filter, TCP_DB);
					filter_db_set(&current_filter, RAW_DB);
				} else if (strcmp(p, "udp") == 0) {
					filter_db_set(&current_filter, UDP_DB);
				} else if (strcmp(p, "dccp") == 0) {
					filter_db_set(&current_filter, DCCP_DB);
				} else if (strcmp(p, "tcp") == 0) {
					filter_db_set(&current_filter, TCP_DB);
				} else if (strcmp(p, "raw") == 0) {
					filter_db_set(&current_filter, RAW_DB);
				} else if (strcmp(p, "unix") == 0) {
					filter_db_set(&current_filter, UNIX_ST_DB);
					filter_db_set(&current_filter, UNIX_DG_DB);
					filter_db_set(&current_filter, UNIX_SQ_DB);
				} else if (strcasecmp(p, "unix_stream") == 0 ||
					   strcmp(p, "u_str") == 0) {
					filter_db_set(&current_filter, UNIX_ST_DB);
				} else if (strcasecmp(p, "unix_dgram") == 0 ||
					   strcmp(p, "u_dgr") == 0) {
					filter_db_set(&current_filter, UNIX_DG_DB);
				} else if (strcasecmp(p, "unix_seqpacket") == 0 ||
					   strcmp(p, "u_seq") == 0) {
					filter_db_set(&current_filter, UNIX_SQ_DB);
				} else if (strcmp(p, "packet") == 0) {
					filter_db_set(&current_filter, PACKET_R_DB);
					filter_db_set(&current_filter, PACKET_DG_DB);
				} else if (strcmp(p, "packet_raw") == 0 ||
					   strcmp(p, "p_raw") == 0) {
					filter_db_set(&current_filter, PACKET_R_DB);
				} else if (strcmp(p, "packet_dgram") == 0 ||
					   strcmp(p, "p_dgr") == 0) {
					filter_db_set(&current_filter, PACKET_DG_DB);
				} else if (strcmp(p, "netlink") == 0) {
					filter_db_set(&current_filter, NETLINK_DB);
				} else {
					fprintf(stderr, "ss: \"%s\" is illegal socket table id\n", p);
					usage();
				}
				p = p1 + 1;
			} while (p1);
			break;
		}
		case 's':
			do_summary = 1;
			break;
		case 'D':
			dump_tcpdiag = optarg;
			break;
		case 'F':
			if (filter_fp) {
				fprintf(stderr, "More than one filter file\n");
				exit(-1);
			}
			if (optarg[0] == '-')
				filter_fp = stdin;
			else
				filter_fp = fopen(optarg, "r");
			if (!filter_fp) {
				perror("fopen filter file");
				exit(-1);
			}
			break;
		case 'v':
		case 'V':
			printf("ss utility, iproute2-ss%s\n", SNAPSHOT);
			exit(0);
		case 'z':
			show_sock_ctx++;
		case 'Z':
			if (is_selinux_enabled() <= 0) {
				fprintf(stderr, "ss: SELinux is not enabled.\n");
				exit(1);
			}
			show_proc_ctx++;
			user_ent_hash_build();
			break;
		case 'N':
			if (netns_switch(optarg))
				exit(1);
			break;
		case 'h':
			help();
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (do_summary) {
		print_summary();
		if (do_default && argc == 0)
			exit(0);
	}

	while (argc > 0) {
		if (strcmp(*argv, "state") == 0) {
			NEXT_ARG();
			if (!saw_states)
				state_filter = 0;
			state_filter |= scan_state(*argv);
			saw_states = 1;
		} else if (strcmp(*argv, "exclude") == 0 ||
			   strcmp(*argv, "excl") == 0) {
			NEXT_ARG();
			if (!saw_states)
				state_filter = SS_ALL;
			state_filter &= ~scan_state(*argv);
			saw_states = 1;
		} else {
			break;
		}
		argc--; argv++;
	}

	if (do_default) {
		state_filter = state_filter ? state_filter : SS_CONN;
		filter_default_dbs(&current_filter);
	}

	filter_states_set(&current_filter, state_filter);
	filter_merge_defaults(&current_filter);

	if (resolve_services && resolve_hosts &&
	    (current_filter.dbs&(UNIX_DBM|(1<<TCP_DB)|(1<<UDP_DB)|(1<<DCCP_DB))))
		init_service_resolver();


	if (current_filter.dbs == 0) {
		fprintf(stderr, "ss: no socket tables to show with such filter.\n");
		exit(0);
	}
	if (current_filter.families == 0) {
		fprintf(stderr, "ss: no families to show with such filter.\n");
		exit(0);
	}
	if (current_filter.states == 0) {
		fprintf(stderr, "ss: no socket states to show with such filter.\n");
		exit(0);
	}

	if (dump_tcpdiag) {
		FILE *dump_fp = stdout;
		if (!(current_filter.dbs & (1<<TCP_DB))) {
			fprintf(stderr, "ss: tcpdiag dump requested and no tcp in filter.\n");
			exit(0);
		}
		if (dump_tcpdiag[0] != '-') {
			dump_fp = fopen(dump_tcpdiag, "w");
			if (!dump_tcpdiag) {
				perror("fopen dump file");
				exit(-1);
			}
		}
		inet_show_netlink(&current_filter, dump_fp, IPPROTO_TCP);
		fflush(dump_fp);
		exit(0);
	}

	if (ssfilter_parse(&current_filter.f, argc, argv, filter_fp))
		usage();

	netid_width = 0;
	if (current_filter.dbs&(current_filter.dbs-1))
		netid_width = 5;

	state_width = 0;
	if (current_filter.states&(current_filter.states-1))
		state_width = 10;

	screen_width = 80;
	if (isatty(STDOUT_FILENO)) {
		struct winsize w;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
			if (w.ws_col > 0)
				screen_width = w.ws_col;
		}
	}

	addrp_width = screen_width;
	addrp_width -= netid_width+1;
	addrp_width -= state_width+1;
	addrp_width -= 14;

	if (addrp_width&1) {
		if (netid_width)
			netid_width++;
		else if (state_width)
			state_width++;
	}

	addrp_width /= 2;
	addrp_width--;

	serv_width = resolve_services ? 7 : 5;

	if (addrp_width < 15+serv_width+1)
		addrp_width = 15+serv_width+1;

	addr_width = addrp_width - serv_width - 1;

	if (netid_width)
		printf("%-*s ", netid_width, "Netid");
	if (state_width)
		printf("%-*s ", state_width, "State");
	printf("%-6s %-6s ", "Recv-Q", "Send-Q");

	/* Make enough space for the local/remote port field */
	addr_width -= 13;
	serv_width += 13;

	printf("%*s:%-*s %*s:%-*s\n",
	       addr_width, "Local Address", serv_width, "Port",
	       addr_width, "Peer Address", serv_width, "Port");

	fflush(stdout);

	if (follow_events)
		exit(handle_follow_request(&current_filter));

	if (current_filter.dbs & (1<<NETLINK_DB))
		netlink_show(&current_filter);
	if (current_filter.dbs & PACKET_DBM)
		packet_show(&current_filter);
	if (current_filter.dbs & UNIX_DBM)
		unix_show(&current_filter);
	if (current_filter.dbs & (1<<RAW_DB))
		raw_show(&current_filter);
	if (current_filter.dbs & (1<<UDP_DB))
		udp_show(&current_filter);
	if (current_filter.dbs & (1<<TCP_DB))
		tcp_show(&current_filter, IPPROTO_TCP);
	if (current_filter.dbs & (1<<DCCP_DB))
		tcp_show(&current_filter, IPPROTO_DCCP);

	if (show_users || show_proc_ctx || show_sock_ctx)
		user_ent_destroy();

	return 0;
}
