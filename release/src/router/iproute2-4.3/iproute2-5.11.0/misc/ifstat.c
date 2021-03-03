/*
 * ifstat.c	handy utility to read net interface statistics
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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <fnmatch.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <getopt.h>

#include <linux/if.h>
#include <linux/if_link.h>

#include "libnetlink.h"
#include "json_writer.h"
#include "version.h"
#include "utils.h"

int dump_zeros;
int reset_history;
int ignore_history;
int no_output;
int json_output;
int no_update;
int scan_interval;
int time_constant;
int show_errors;
double W;
char **patterns;
int npatterns;
bool is_extended;
int filter_type;
int sub_type;

char info_source[128];
int source_mismatch;

#define MAXS (sizeof(struct rtnl_link_stats)/sizeof(__u32))
#define NO_SUB_TYPE 0xffff

struct ifstat_ent {
	struct ifstat_ent	*next;
	char			*name;
	int			ifindex;
	__u64			val[MAXS];
	double			rate[MAXS];
	__u32			ival[MAXS];
};

static const char *stats[MAXS] = {
	"rx_packets",
	"tx_packets",
	"rx_bytes",
	"tx_bytes",
	"rx_errors",
	"tx_errors",
	"rx_dropped",
	"tx_dropped",
	"multicast",
	"collisions",
	"rx_length_errors",
	"rx_over_errors",
	"rx_crc_errors",
	"rx_frame_errors",
	"rx_fifo_errors",
	"rx_missed_errors",
	"tx_aborted_errors",
	"tx_carrier_errors",
	"tx_fifo_errors",
	"tx_heartbeat_errors",
	"tx_window_errors",
	"rx_compressed",
	"tx_compressed"
};

struct ifstat_ent *kern_db;
struct ifstat_ent *hist_db;

static int match(const char *id)
{
	int i;

	if (npatterns == 0)
		return 1;

	for (i = 0; i < npatterns; i++) {
		if (!fnmatch(patterns[i], id, FNM_CASEFOLD))
			return 1;
	}
	return 0;
}

static int get_nlmsg_extended(struct nlmsghdr *m, void *arg)
{
	struct if_stats_msg *ifsm = NLMSG_DATA(m);
	struct rtattr *tb[IFLA_STATS_MAX+1];
	int len = m->nlmsg_len;
	struct ifstat_ent *n;

	if (m->nlmsg_type != RTM_NEWSTATS)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifsm));
	if (len < 0)
		return -1;

	parse_rtattr(tb, IFLA_STATS_MAX, IFLA_STATS_RTA(ifsm), len);
	if (tb[filter_type] == NULL)
		return 0;

	n = malloc(sizeof(*n));
	if (!n)
		abort();

	n->ifindex = ifsm->ifindex;
	n->name = strdup(ll_index_to_name(ifsm->ifindex));

	if (sub_type == NO_SUB_TYPE) {
		memcpy(&n->val, RTA_DATA(tb[filter_type]), sizeof(n->val));
	} else {
		struct rtattr *attr;

		attr = parse_rtattr_one_nested(sub_type, tb[filter_type]);
		if (attr == NULL) {
			free(n);
			return 0;
		}
		memcpy(&n->val, RTA_DATA(attr), sizeof(n->val));
	}
	memset(&n->rate, 0, sizeof(n->rate));
	n->next = kern_db;
	kern_db = n;
	return 0;
}

static int get_nlmsg(struct nlmsghdr *m, void *arg)
{
	struct ifinfomsg *ifi = NLMSG_DATA(m);
	struct rtattr *tb[IFLA_MAX+1];
	int len = m->nlmsg_len;
	struct ifstat_ent *n;
	int i;

	if (m->nlmsg_type != RTM_NEWLINK)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	if (!(ifi->ifi_flags&IFF_UP))
		return 0;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (tb[IFLA_IFNAME] == NULL || tb[IFLA_STATS] == NULL)
		return 0;

	n = malloc(sizeof(*n));
	if (!n)
		abort();
	n->ifindex = ifi->ifi_index;
	n->name = strdup(RTA_DATA(tb[IFLA_IFNAME]));
	memcpy(&n->ival, RTA_DATA(tb[IFLA_STATS]), sizeof(n->ival));
	memset(&n->rate, 0, sizeof(n->rate));
	for (i = 0; i < MAXS; i++)
		n->val[i] = n->ival[i];
	n->next = kern_db;
	kern_db = n;
	return 0;
}

static void load_info(void)
{
	struct ifstat_ent *db, *n;
	struct rtnl_handle rth;
	__u32 filter_mask;

	if (rtnl_open(&rth, 0) < 0)
		exit(1);

	if (is_extended) {
		ll_init_map(&rth);
		filter_mask = IFLA_STATS_FILTER_BIT(filter_type);
		if (rtnl_statsdump_req_filter(&rth, AF_UNSPEC,
					      filter_mask) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}

		if (rtnl_dump_filter(&rth, get_nlmsg_extended, NULL) < 0) {
			fprintf(stderr, "Dump terminated\n");
			exit(1);
		}
	} else {
		if (rtnl_linkdump_req(&rth, AF_INET) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}

		if (rtnl_dump_filter(&rth, get_nlmsg, NULL) < 0) {
			fprintf(stderr, "Dump terminated\n");
			exit(1);
		}
	}

	rtnl_close(&rth);

	db = kern_db;
	kern_db = NULL;

	while (db) {
		n = db;
		db = db->next;
		n->next = kern_db;
		kern_db = n;
	}
}

static void load_raw_table(FILE *fp)
{
	char buf[4096];
	struct ifstat_ent *db = NULL;
	struct ifstat_ent *n;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p;
		char *next;
		int i;

		if (buf[0] == '#') {
			buf[strlen(buf)-1] = 0;
			if (info_source[0] && strcmp(info_source, buf+1))
				source_mismatch = 1;
			strlcpy(info_source, buf+1, sizeof(info_source));
			continue;
		}
		if ((n = malloc(sizeof(*n))) == NULL)
			abort();

		if (!(p = strchr(buf, ' ')))
			abort();
		*p++ = 0;

		if (sscanf(buf, "%d", &n->ifindex) != 1)
			abort();
		if (!(next = strchr(p, ' ')))
			abort();
		*next++ = 0;

		n->name = strdup(p);
		p = next;

		for (i = 0; i < MAXS; i++) {
			unsigned int rate;

			if (!(next = strchr(p, ' ')))
				abort();
			*next++ = 0;
			if (sscanf(p, "%llu", n->val+i) != 1)
				abort();
			n->ival[i] = (__u32)n->val[i];
			p = next;
			if (!(next = strchr(p, ' ')))
				abort();
			*next++ = 0;
			if (sscanf(p, "%u", &rate) != 1)
				abort();
			n->rate[i] = rate;
			p = next;
		}
		n->next = db;
		db = n;
	}

	while (db) {
		n = db;
		db = db->next;
		n->next = kern_db;
		kern_db = n;
	}
}

static void dump_raw_db(FILE *fp, int to_hist)
{
	json_writer_t *jw = json_output ? jsonw_new(fp) : NULL;
	struct ifstat_ent *n, *h;

	h = hist_db;
	if (jw) {
		jsonw_start_object(jw);
		jsonw_pretty(jw, pretty);
		jsonw_name(jw, info_source);
		jsonw_start_object(jw);
	} else
		fprintf(fp, "#%s\n", info_source);

	for (n = kern_db; n; n = n->next) {
		int i;
		unsigned long long *vals = n->val;
		double *rates = n->rate;

		if (!match(n->name)) {
			struct ifstat_ent *h1;

			if (!to_hist)
				continue;
			for (h1 = h; h1; h1 = h1->next) {
				if (h1->ifindex == n->ifindex) {
					vals = h1->val;
					rates = h1->rate;
					h = h1->next;
					break;
				}
			}
		}

		if (jw) {
			jsonw_name(jw, n->name);
			jsonw_start_object(jw);

			for (i = 0; i < MAXS && stats[i]; i++)
				jsonw_uint_field(jw, stats[i], vals[i]);
			jsonw_end_object(jw);
		} else {
			fprintf(fp, "%d %s ", n->ifindex, n->name);
			for (i = 0; i < MAXS; i++)
				fprintf(fp, "%llu %u ", vals[i],
					(unsigned int)rates[i]);
			fprintf(fp, "\n");
		}
	}
	if (jw) {
		jsonw_end_object(jw);

		jsonw_end_object(jw);
		jsonw_destroy(&jw);
	}
}

/* use communication definitions of meg/kilo etc */
static const unsigned long long giga = 1000000000ull;
static const unsigned long long mega = 1000000;
static const unsigned long long kilo = 1000;

static void format_rate(FILE *fp, const unsigned long long *vals,
			const double *rates, int i)
{
	char temp[64];

	if (vals[i] > giga)
		fprintf(fp, "%7lluM ", vals[i]/mega);
	else if (vals[i] > mega)
		fprintf(fp, "%7lluK ", vals[i]/kilo);
	else
		fprintf(fp, "%8llu ", vals[i]);

	if (rates[i] > mega) {
		sprintf(temp, "%uM", (unsigned int)(rates[i]/mega));
		fprintf(fp, "%-6s ", temp);
	} else if (rates[i] > kilo) {
		sprintf(temp, "%uK", (unsigned int)(rates[i]/kilo));
		fprintf(fp, "%-6s ", temp);
	} else
		fprintf(fp, "%-6u ", (unsigned int)rates[i]);
}

static void format_pair(FILE *fp, const unsigned long long *vals, int i, int k)
{
	char temp[64];

	if (vals[i] > giga)
		fprintf(fp, "%7lluM ", vals[i]/mega);
	else if (vals[i] > mega)
		fprintf(fp, "%7lluK ", vals[i]/kilo);
	else
		fprintf(fp, "%8llu ", vals[i]);

	if (vals[k] > giga) {
		sprintf(temp, "%uM", (unsigned int)(vals[k]/mega));
		fprintf(fp, "%-6s ", temp);
	} else if (vals[k] > mega) {
		sprintf(temp, "%uK", (unsigned int)(vals[k]/kilo));
		fprintf(fp, "%-6s ", temp);
	} else
		fprintf(fp, "%-6u ", (unsigned int)vals[k]);
}

static void print_head(FILE *fp)
{
	fprintf(fp, "#%s\n", info_source);
	fprintf(fp, "%-15s ", "Interface");

	fprintf(fp, "%8s/%-6s ", "RX Pkts", "Rate");
	fprintf(fp, "%8s/%-6s ", "TX Pkts", "Rate");
	fprintf(fp, "%8s/%-6s ", "RX Data", "Rate");
	fprintf(fp, "%8s/%-6s\n", "TX Data", "Rate");

	if (!show_errors) {
		fprintf(fp, "%-15s ", "");
		fprintf(fp, "%8s/%-6s ", "RX Errs", "Drop");
		fprintf(fp, "%8s/%-6s ", "TX Errs", "Drop");
		fprintf(fp, "%8s/%-6s ", "RX Over", "Rate");
		fprintf(fp, "%8s/%-6s\n", "TX Coll", "Rate");
	} else {
		fprintf(fp, "%-15s ", "");
		fprintf(fp, "%8s/%-6s ", "RX Errs", "Rate");
		fprintf(fp, "%8s/%-6s ", "RX Drop", "Rate");
		fprintf(fp, "%8s/%-6s ", "RX Over", "Rate");
		fprintf(fp, "%8s/%-6s\n", "RX Leng", "Rate");

		fprintf(fp, "%-15s ", "");
		fprintf(fp, "%8s/%-6s ", "RX Crc", "Rate");
		fprintf(fp, "%8s/%-6s ", "RX Frm", "Rate");
		fprintf(fp, "%8s/%-6s ", "RX Fifo", "Rate");
		fprintf(fp, "%8s/%-6s\n", "RX Miss", "Rate");

		fprintf(fp, "%-15s ", "");
		fprintf(fp, "%8s/%-6s ", "TX Errs", "Rate");
		fprintf(fp, "%8s/%-6s ", "TX Drop", "Rate");
		fprintf(fp, "%8s/%-6s ", "TX Coll", "Rate");
		fprintf(fp, "%8s/%-6s\n", "TX Carr", "Rate");

		fprintf(fp, "%-15s ", "");
		fprintf(fp, "%8s/%-6s ", "TX Abrt", "Rate");
		fprintf(fp, "%8s/%-6s ", "TX Fifo", "Rate");
		fprintf(fp, "%8s/%-6s ", "TX Hear", "Rate");
		fprintf(fp, "%8s/%-6s\n", "TX Wind", "Rate");
	}
}

static void print_one_json(json_writer_t *jw, const struct ifstat_ent *n,
			   const unsigned long long *vals)
{
	int i, m = show_errors ? 20 : 10;

	jsonw_name(jw, n->name);
	jsonw_start_object(jw);

	for (i = 0; i < m && stats[i]; i++)
		jsonw_uint_field(jw, stats[i], vals[i]);

	jsonw_end_object(jw);
}

static void print_one_if(FILE *fp, const struct ifstat_ent *n,
			 const unsigned long long *vals)
{
	int i;

	fprintf(fp, "%-15s ", n->name);
	for (i = 0; i < 4; i++)
		format_rate(fp, vals, n->rate, i);
	fprintf(fp, "\n");

	if (!show_errors) {
		fprintf(fp, "%-15s ", "");
		format_pair(fp, vals, 4, 6);
		format_pair(fp, vals, 5, 7);
		format_rate(fp, vals, n->rate, 11);
		format_rate(fp, vals, n->rate, 9);
		fprintf(fp, "\n");
	} else {
		fprintf(fp, "%-15s ", "");
		format_rate(fp, vals, n->rate, 4);
		format_rate(fp, vals, n->rate, 6);
		format_rate(fp, vals, n->rate, 11);
		format_rate(fp, vals, n->rate, 10);
		fprintf(fp, "\n");

		fprintf(fp, "%-15s ", "");
		format_rate(fp, vals, n->rate, 12);
		format_rate(fp, vals, n->rate, 13);
		format_rate(fp, vals, n->rate, 14);
		format_rate(fp, vals, n->rate, 15);
		fprintf(fp, "\n");

		fprintf(fp, "%-15s ", "");
		format_rate(fp, vals, n->rate, 5);
		format_rate(fp, vals, n->rate, 7);
		format_rate(fp, vals, n->rate, 9);
		format_rate(fp, vals, n->rate, 17);
		fprintf(fp, "\n");

		fprintf(fp, "%-15s ", "");
		format_rate(fp, vals, n->rate, 16);
		format_rate(fp, vals, n->rate, 18);
		format_rate(fp, vals, n->rate, 19);
		format_rate(fp, vals, n->rate, 20);
		fprintf(fp, "\n");
	}
}

static void dump_kern_db(FILE *fp)
{
	json_writer_t *jw = json_output ? jsonw_new(fp) : NULL;
	struct ifstat_ent *n;

	if (jw) {
		jsonw_start_object(jw);
		jsonw_pretty(jw, pretty);
		jsonw_name(jw, info_source);
		jsonw_start_object(jw);
	} else
		print_head(fp);

	for (n = kern_db; n; n = n->next) {
		if (!match(n->name))
			continue;

		if (jw)
			print_one_json(jw, n, n->val);
		else
			print_one_if(fp, n, n->val);
	}
	if (jw) {
		jsonw_end_object(jw);

		jsonw_end_object(jw);
		jsonw_destroy(&jw);
	}
}

static void dump_incr_db(FILE *fp)
{
	struct ifstat_ent *n, *h;
	json_writer_t *jw = json_output ? jsonw_new(fp) : NULL;

	h = hist_db;
	if (jw) {
		jsonw_start_object(jw);
		jsonw_pretty(jw, pretty);
		jsonw_name(jw, info_source);
		jsonw_start_object(jw);
	} else
		print_head(fp);

	for (n = kern_db; n; n = n->next) {
		int i;
		unsigned long long vals[MAXS];
		struct ifstat_ent *h1;

		memcpy(vals, n->val, sizeof(vals));

		for (h1 = h; h1; h1 = h1->next) {
			if (h1->ifindex == n->ifindex) {
				for (i = 0; i < MAXS; i++)
					vals[i] -= h1->val[i];
				h = h1->next;
				break;
			}
		}
		if (!match(n->name))
			continue;

		if (jw)
			print_one_json(jw, n, n->val);
		else
			print_one_if(fp, n, vals);
	}

	if (jw) {
		jsonw_end_object(jw);

		jsonw_end_object(jw);
		jsonw_destroy(&jw);
	}
}

static int children;

static void sigchild(int signo)
{
}

static void update_db(int interval)
{
	struct ifstat_ent *n, *h;

	n = kern_db;
	kern_db = NULL;

	load_info();

	h = kern_db;
	kern_db = n;

	for (n = kern_db; n; n = n->next) {
		struct ifstat_ent *h1;

		for (h1 = h; h1; h1 = h1->next) {
			if (h1->ifindex == n->ifindex) {
				int i;

				for (i = 0; i < MAXS; i++) {
					if ((long)(h1->ival[i] - n->ival[i]) < 0) {
						memset(n->ival, 0, sizeof(n->ival));
						break;
					}
				}
				for (i = 0; i < MAXS; i++) {
					double sample;
					__u64 incr;

					if (is_extended) {
						incr = h1->val[i] - n->val[i];
						n->val[i] = h1->val[i];
					} else {
						incr = (__u32) (h1->ival[i] - n->ival[i]);
						n->val[i] += incr;
						n->ival[i] = h1->ival[i];
					}

					sample = (double)(incr*1000)/interval;
					if (interval >= scan_interval) {
						n->rate[i] += W*(sample-n->rate[i]);
					} else if (interval >= 1000) {
						if (interval >= time_constant) {
							n->rate[i] = sample;
						} else {
							double w = W*(double)interval/scan_interval;

							n->rate[i] += w*(sample-n->rate[i]);
						}
					}
				}

				while (h != h1) {
					struct ifstat_ent *tmp = h;

					h = h->next;
					free(tmp->name);
					free(tmp);
				};
				h = h1->next;
				free(h1->name);
				free(h1);
				break;
			}
		}
	}
}

#define T_DIFF(a, b) (((a).tv_sec-(b).tv_sec)*1000 + ((a).tv_usec-(b).tv_usec)/1000)


static void server_loop(int fd)
{
	struct timeval snaptime = { 0 };
	struct pollfd p;

	p.fd = fd;
	p.events = p.revents = POLLIN;

	sprintf(info_source, "%d.%lu sampling_interval=%d time_const=%d",
		getpid(), (unsigned long)random(), scan_interval/1000, time_constant/1000);

	load_info();

	for (;;) {
		int status;
		time_t tdiff;
		struct timeval now;

		gettimeofday(&now, NULL);
		tdiff = T_DIFF(now, snaptime);
		if (tdiff >= scan_interval) {
			update_db(tdiff);
			snaptime = now;
			tdiff = 0;
		}

		if (poll(&p, 1, scan_interval - tdiff) > 0
		    && (p.revents&POLLIN)) {
			int clnt = accept(fd, NULL, NULL);

			if (clnt >= 0) {
				pid_t pid;

				if (children >= 5) {
					close(clnt);
				} else if ((pid = fork()) != 0) {
					if (pid > 0)
						children++;
					close(clnt);
				} else {
					FILE *fp = fdopen(clnt, "w");

					if (fp)
						dump_raw_db(fp, 0);
					exit(0);
				}
			}
		}
		while (children && waitpid(-1, &status, WNOHANG) > 0)
			children--;
	}
}

static int verify_forging(int fd)
{
	struct ucred cred;
	socklen_t olen = sizeof(cred);

	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, (void *)&cred, &olen) ||
	    olen < sizeof(cred))
		return -1;
	if (cred.uid == getuid() || cred.uid == 0)
		return 0;
	return -1;
}

static void xstat_usage(void)
{
	fprintf(stderr,
"Usage: ifstat supported xstats:\n"
"       cpu_hits       Counts only packets that went via the CPU.\n");
}

struct extended_stats_options_t {
	char *name;
	int id;
	int sub_type;
};

/* Note: if one xstat name is subset of another, it should be before it in this
 * list.
 * Name length must be under 64 chars.
 */
static const struct extended_stats_options_t extended_stats_options[] = {
	{"cpu_hits",  IFLA_STATS_LINK_OFFLOAD_XSTATS, IFLA_OFFLOAD_XSTATS_CPU_HIT},
};

static const char *get_filter_type(const char *name)
{
	int name_len;
	int i;

	name_len = strlen(name);
	for (i = 0; i < ARRAY_SIZE(extended_stats_options); i++) {
		const struct extended_stats_options_t *xstat;

		xstat = &extended_stats_options[i];
		if (strncmp(name, xstat->name, name_len) == 0) {
			filter_type = xstat->id;
			sub_type = xstat->sub_type;
			return xstat->name;
		}
	}

	fprintf(stderr, "invalid ifstat extension %s\n", name);
	xstat_usage();
	return NULL;
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr,
"Usage: ifstat [OPTION] [ PATTERN [ PATTERN ] ]\n"
"   -h, --help           this message\n"
"   -a, --ignore         ignore history\n"
"   -d, --scan=SECS      sample every statistics every SECS\n"
"   -e, --errors         show errors\n"
"   -j, --json           format output in JSON\n"
"   -n, --nooutput       do history only\n"
"   -p, --pretty         pretty print\n"
"   -r, --reset          reset history\n"
"   -s, --noupdate       don't update history\n"
"   -t, --interval=SECS  report average over the last SECS\n"
"   -V, --version        output version information\n"
"   -z, --zeros          show entries with zero activity\n"
"   -x, --extended=TYPE  show extended stats of TYPE\n");

	exit(-1);
}

static const struct option longopts[] = {
	{ "help", 0, 0, 'h' },
	{ "ignore",  0,  0, 'a' },
	{ "scan", 1, 0, 'd'},
	{ "errors", 0, 0, 'e' },
	{ "nooutput", 0, 0, 'n' },
	{ "json", 0, 0, 'j' },
	{ "reset", 0, 0, 'r' },
	{ "pretty", 0, 0, 'p' },
	{ "noupdate", 0, 0, 's' },
	{ "interval", 1, 0, 't' },
	{ "version", 0, 0, 'V' },
	{ "zeros", 0, 0, 'z' },
	{ "extended", 1, 0, 'x'},
	{ 0 }
};

int main(int argc, char *argv[])
{
	char hist_name[128];
	struct sockaddr_un sun;
	FILE *hist_fp = NULL;
	const char *stats_type = NULL;
	int ch;
	int fd;

	is_extended = false;
	while ((ch = getopt_long(argc, argv, "hjpvVzrnasd:t:ex:",
			longopts, NULL)) != EOF) {
		switch (ch) {
		case 'z':
			dump_zeros = 1;
			break;
		case 'r':
			reset_history = 1;
			break;
		case 'a':
			ignore_history = 1;
			break;
		case 's':
			no_update = 1;
			break;
		case 'n':
			no_output = 1;
			break;
		case 'e':
			show_errors = 1;
			break;
		case 'j':
			json_output = 1;
			break;
		case 'p':
			pretty = 1;
			break;
		case 'd':
			scan_interval = atoi(optarg) * 1000;
			if (scan_interval <= 0) {
				fprintf(stderr, "ifstat: invalid scan interval\n");
				exit(-1);
			}
			break;
		case 't':
			time_constant = atoi(optarg);
			if (time_constant <= 0) {
				fprintf(stderr, "ifstat: invalid time constant divisor\n");
				exit(-1);
			}
			break;
		case 'x':
			stats_type = optarg;
			is_extended = true;
			break;
		case 'v':
		case 'V':
			printf("ifstat utility, iproute2-%s\n", version);
			exit(0);
		case 'h':
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (stats_type) {
		stats_type = get_filter_type(stats_type);
		if (!stats_type)
			exit(-1);
	}

	sun.sun_family = AF_UNIX;
	sun.sun_path[0] = 0;
	sprintf(sun.sun_path+1, "ifstat%d", getuid());

	if (scan_interval > 0) {
		if (time_constant == 0)
			time_constant = 60;
		time_constant *= 1000;
		W = 1 - 1/exp(log(10)*(double)scan_interval/time_constant);
		if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("ifstat: socket");
			exit(-1);
		}
		if (bind(fd, (struct sockaddr *)&sun, 2+1+strlen(sun.sun_path+1)) < 0) {
			perror("ifstat: bind");
			exit(-1);
		}
		if (listen(fd, 5) < 0) {
			perror("ifstat: listen");
			exit(-1);
		}
		if (daemon(0, 0)) {
			perror("ifstat: daemon");
			exit(-1);
		}
		signal(SIGPIPE, SIG_IGN);
		signal(SIGCHLD, sigchild);
		server_loop(fd);
		exit(0);
	}

	patterns = argv;
	npatterns = argc;

	if (getenv("IFSTAT_HISTORY"))
		snprintf(hist_name, sizeof(hist_name),
			 "%s", getenv("IFSTAT_HISTORY"));
	else
		if (!stats_type)
			snprintf(hist_name, sizeof(hist_name),
				 "%s/.ifstat.u%d", P_tmpdir, getuid());
		else
			snprintf(hist_name, sizeof(hist_name),
				 "%s/.%s_ifstat.u%d", P_tmpdir, stats_type,
				 getuid());

	if (reset_history)
		unlink(hist_name);

	if (!ignore_history || !no_update) {
		struct stat stb;

		fd = open(hist_name, O_RDWR|O_CREAT|O_NOFOLLOW, 0600);
		if (fd < 0) {
			perror("ifstat: open history file");
			exit(-1);
		}
		if ((hist_fp = fdopen(fd, "r+")) == NULL) {
			perror("ifstat: fdopen history file");
			exit(-1);
		}
		if (flock(fileno(hist_fp), LOCK_EX)) {
			perror("ifstat: flock history file");
			exit(-1);
		}
		if (fstat(fileno(hist_fp), &stb) != 0) {
			perror("ifstat: fstat history file");
			exit(-1);
		}
		if (stb.st_nlink != 1 || stb.st_uid != getuid()) {
			fprintf(stderr, "ifstat: something is so wrong with history file, that I prefer not to proceed.\n");
			exit(-1);
		}
		if (!ignore_history) {
			FILE *tfp;
			long uptime = -1;

			if ((tfp = fopen("/proc/uptime", "r")) != NULL) {
				if (fscanf(tfp, "%ld", &uptime) != 1)
					uptime = -1;
				fclose(tfp);
			}
			if (uptime >= 0 && time(NULL) >= stb.st_mtime+uptime) {
				fprintf(stderr, "ifstat: history is aged out, resetting\n");
				if (ftruncate(fileno(hist_fp), 0))
					perror("ifstat: ftruncate");
			}
		}

		load_raw_table(hist_fp);

		hist_db = kern_db;
		kern_db = NULL;
	}

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0 &&
	    (connect(fd, (struct sockaddr *)&sun, 2+1+strlen(sun.sun_path+1)) == 0
	     || (strcpy(sun.sun_path+1, "ifstat0"),
		 connect(fd, (struct sockaddr *)&sun, 2+1+strlen(sun.sun_path+1)) == 0))
	    && verify_forging(fd) == 0) {
		FILE *sfp = fdopen(fd, "r");

		if (!sfp) {
			fprintf(stderr, "ifstat: fdopen failed: %s\n",
				strerror(errno));
			close(fd);
		} else  {
			load_raw_table(sfp);
			if (hist_db && source_mismatch) {
				fprintf(stderr, "ifstat: history is stale, ignoring it.\n");
				hist_db = NULL;
			}
			fclose(sfp);
		}
	} else {
		if (fd >= 0)
			close(fd);
		if (hist_db && info_source[0] && strcmp(info_source, "kernel")) {
			fprintf(stderr, "ifstat: history is stale, ignoring it.\n");
			hist_db = NULL;
			info_source[0] = 0;
		}
		load_info();
		if (info_source[0] == 0)
			strcpy(info_source, "kernel");
	}

	if (!no_output) {
		if (ignore_history || hist_db == NULL)
			dump_kern_db(stdout);
		else
			dump_incr_db(stdout);
	}

	if (!no_update) {
		if (ftruncate(fileno(hist_fp), 0))
			perror("ifstat: ftruncate");
		rewind(hist_fp);

		json_output = 0;
		dump_raw_db(hist_fp, 1);
		fclose(hist_fp);
	}
	exit(0);
}
