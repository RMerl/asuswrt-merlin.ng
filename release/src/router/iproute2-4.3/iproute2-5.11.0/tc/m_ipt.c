/*
 * m_ipt.c	iptables based targets
 *		utilities mostly ripped from iptables <duh, its the linux way>
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (hadi@cyberus.ca)
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iptables.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include "utils.h"
#include "tc_util.h"
#include <linux/tc_act/tc_ipt.h>
#include <stdio.h>
#include <dlfcn.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

static const char *pname = "tc-ipt";
static const char *tname = "mangle";
static const char *pversion = "0.1";

static const char *ipthooks[] = {
	"NF_IP_PRE_ROUTING",
	"NF_IP_LOCAL_IN",
	"NF_IP_FORWARD",
	"NF_IP_LOCAL_OUT",
	"NF_IP_POST_ROUTING",
};

static struct option original_opts[] = {
	{"jump", 1, 0, 'j'},
	{0, 0, 0, 0}
};

static struct xtables_target *t_list;
static struct option *opts = original_opts;
static unsigned int global_option_offset;
#define OPTION_OFFSET 256

char *lib_dir;

void
xtables_register_target(struct xtables_target *me)
{
	me->next = t_list;
	t_list = me;

}

static void exit_tryhelp(int status)
{
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
		pname, pname);
	exit(status);
}

static void exit_error(enum xtables_exittype status, char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stderr, "%s v%s: ", pname, pversion);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
	if (status == PARAMETER_PROBLEM)
		exit_tryhelp(status);
	if (status == VERSION_PROBLEM)
		fprintf(stderr,
			"Perhaps iptables or your kernel needs to be upgraded.\n");
	exit(status);
}

/* stolen from iptables 1.2.11
They should really have them as a library so i can link to them
Email them next time i remember
*/

static void free_opts(struct option *local_opts)
{
	if (local_opts != original_opts) {
		free(local_opts);
		opts = original_opts;
		global_option_offset = 0;
	}
}

static struct option *
merge_options(struct option *oldopts, const struct option *newopts,
	      unsigned int *option_offset)
{
	struct option *merge;
	unsigned int num_old, num_new, i;

	for (num_old = 0; oldopts[num_old].name; num_old++);
	for (num_new = 0; newopts[num_new].name; num_new++);

	*option_offset = global_option_offset + OPTION_OFFSET;

	merge = malloc(sizeof(struct option) * (num_new + num_old + 1));
	memcpy(merge, oldopts, num_old * sizeof(struct option));
	for (i = 0; i < num_new; i++) {
		merge[num_old + i] = newopts[i];
		merge[num_old + i].val += *option_offset;
	}
	memset(merge + num_old + num_new, 0, sizeof(struct option));

	return merge;
}

static void *
fw_calloc(size_t count, size_t size)
{
	void *p;

	if ((p = (void *) calloc(count, size)) == NULL) {
		perror("iptables: calloc failed");
		exit(1);
	}
	return p;
}

static struct xtables_target *
find_t(char *name)
{
	struct xtables_target *m;

	for (m = t_list; m; m = m->next) {
		if (strcmp(m->name, name) == 0)
			return m;
	}

	return NULL;
}

static struct xtables_target *
get_target_name(const char *name)
{
	void *handle;
	char *error;
	char *new_name, *lname;
	struct xtables_target *m;
	char path[strlen(lib_dir) + sizeof("/libipt_.so") + strlen(name)];

#ifdef NO_SHARED_LIBS
	return NULL;
#endif

	new_name = calloc(1, strlen(name) + 1);
	lname = calloc(1, strlen(name) + 1);
	if (!new_name)
		exit_error(PARAMETER_PROBLEM, "get_target_name");
	if (!lname)
		exit_error(PARAMETER_PROBLEM, "get_target_name");

	strcpy(new_name, name);
	strcpy(lname, name);

	if (isupper(lname[0])) {
		int i;

		for (i = 0; i < strlen(name); i++) {
			lname[i] = tolower(lname[i]);
		}
	}

	if (islower(new_name[0])) {
		int i;

		for (i = 0; i < strlen(new_name); i++) {
			new_name[i] = toupper(new_name[i]);
		}
	}

	/* try libxt_xx first */
	sprintf(path, "%s/libxt_%s.so", lib_dir, new_name);
	handle = dlopen(path, RTLD_LAZY);
	if (!handle) {
		/* try libipt_xx next */
		sprintf(path, "%s/libipt_%s.so", lib_dir, new_name);
		handle = dlopen(path, RTLD_LAZY);

		if (!handle) {
			sprintf(path, "%s/libxt_%s.so", lib_dir, lname);
			handle = dlopen(path, RTLD_LAZY);
		}

		if (!handle) {
			sprintf(path, "%s/libipt_%s.so", lib_dir, lname);
			handle = dlopen(path, RTLD_LAZY);
		}
		/* ok, lets give up .. */
		if (!handle) {
			fputs(dlerror(), stderr);
			printf("\n");
			free(new_name);
			free(lname);
			return NULL;
		}
	}

	m = dlsym(handle, new_name);
	if ((error = dlerror()) != NULL) {
		m = (struct xtables_target *) dlsym(handle, lname);
		if ((error = dlerror()) != NULL) {
			m = find_t(new_name);
			if (m == NULL) {
				m = find_t(lname);
				if (m == NULL) {
					fputs(error, stderr);
					fprintf(stderr, "\n");
					dlclose(handle);
					free(new_name);
					free(lname);
					return NULL;
				}
			}
		}
	}

	free(new_name);
	free(lname);
	return m;
}

static void set_revision(char *name, u_int8_t revision)
{
	/* Old kernel sources don't have ".revision" field,
	*  but we stole a byte from name. */
	name[IPT_FUNCTION_MAXNAMELEN - 2] = '\0';
	name[IPT_FUNCTION_MAXNAMELEN - 1] = revision;
}

/*
 * we may need to check for version mismatch
*/
static int build_st(struct xtables_target *target, struct ipt_entry_target *t)
{
	if (target) {
		size_t size;

		size =
		    XT_ALIGN(sizeof(struct ipt_entry_target)) + target->size;

		if (t == NULL) {
			target->t = fw_calloc(1, size);
			target->t->u.target_size = size;

			if (target->init != NULL)
				target->init(target->t);
			set_revision(target->t->u.user.name, target->revision);
		} else {
			target->t = t;
		}
		strcpy(target->t->u.user.name, target->name);
		return 0;
	}

	return -1;
}

static int parse_ipt(struct action_util *a, int *argc_p,
		     char ***argv_p, int tca_id, struct nlmsghdr *n)
{
	struct xtables_target *m = NULL;
	struct ipt_entry fw;
	struct rtattr *tail;
	int c;
	int rargc = *argc_p;
	char **argv = *argv_p;
	int argc = 0, iargc = 0;
	char k[FILTER_NAMESZ];
	int size = 0;
	int iok = 0, ok = 0;
	__u32 hook = 0, index = 0;

	lib_dir = getenv("IPTABLES_LIB_DIR");
	if (!lib_dir)
		lib_dir = IPT_LIB_DIR;

	{
		int i;

		for (i = 0; i < rargc; i++) {
			if (!argv[i] || strcmp(argv[i], "action") == 0)
				break;
		}
		iargc = argc = i;
	}

	if (argc <= 2) {
		fprintf(stderr, "bad arguments to ipt %d vs %d\n", argc, rargc);
		return -1;
	}

	while (1) {
		c = getopt_long(argc, argv, "j:", opts, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'j':
			m = get_target_name(optarg);
			if (m != NULL) {

				if (build_st(m, NULL) < 0) {
					printf(" %s error\n", m->name);
					return -1;
				}
				opts =
				    merge_options(opts, m->extra_opts,
						  &m->option_offset);
			} else {
				fprintf(stderr, " failed to find target %s\n\n", optarg);
				return -1;
			}
			ok++;
			break;

		default:
			memset(&fw, 0, sizeof(fw));
			if (m) {
				m->parse(c - m->option_offset, argv, 0,
					 &m->tflags, NULL, &m->t);
			} else {
				fprintf(stderr, " failed to find target %s\n\n", optarg);
				return -1;

			}
			ok++;
			break;

		}
	}

	if (iargc > optind) {
		if (matches(argv[optind], "index") == 0) {
			if (get_u32(&index, argv[optind + 1], 10)) {
				fprintf(stderr, "Illegal \"index\"\n");
				free_opts(opts);
				return -1;
			}
			iok++;

			optind += 2;
		}
	}

	if (!ok && !iok) {
		fprintf(stderr, " ipt Parser BAD!! (%s)\n", *argv);
		return -1;
	}

	/* check that we passed the correct parameters to the target */
	if (m)
		m->final_check(m->tflags);

	{
		struct tcmsg *t = NLMSG_DATA(n);

		if (t->tcm_parent != TC_H_ROOT
		    && t->tcm_parent == TC_H_MAJ(TC_H_INGRESS)) {
			hook = NF_IP_PRE_ROUTING;
		} else {
			hook = NF_IP_POST_ROUTING;
		}
	}

	tail = addattr_nest(n, MAX_MSG, tca_id);
	fprintf(stdout, "tablename: %s hook: %s\n ", tname, ipthooks[hook]);
	fprintf(stdout, "\ttarget: ");

	if (m)
		m->print(NULL, m->t, 0);
	fprintf(stdout, " index %d\n", index);

	if (strlen(tname) > 16) {
		size = 16;
		k[15] = 0;
	} else {
		size = 1 + strlen(tname);
	}
	strncpy(k, tname, size);

	addattr_l(n, MAX_MSG, TCA_IPT_TABLE, k, size);
	addattr_l(n, MAX_MSG, TCA_IPT_HOOK, &hook, 4);
	addattr_l(n, MAX_MSG, TCA_IPT_INDEX, &index, 4);
	if (m)
		addattr_l(n, MAX_MSG, TCA_IPT_TARG, m->t, m->t->u.target_size);
	addattr_nest_end(n, tail);

	argc -= optind;
	argv += optind;
	*argc_p = rargc - iargc;
	*argv_p = argv;

	optind = 0;
	free_opts(opts);
	/* Clear flags if target will be used again */
        m->tflags = 0;
        m->used = 0;
	/* Free allocated memory */
	if (m->t)
	    free(m->t);


	return 0;

}

static int
print_ipt(struct action_util *au, FILE * f, struct rtattr *arg)
{
	struct rtattr *tb[TCA_IPT_MAX + 1];
	struct ipt_entry_target *t = NULL;
	struct xtables_target *m;
	__u32 hook;

	if (arg == NULL)
		return 0;

	lib_dir = getenv("IPTABLES_LIB_DIR");
	if (!lib_dir)
		lib_dir = IPT_LIB_DIR;

	parse_rtattr_nested(tb, TCA_IPT_MAX, arg);

	if (tb[TCA_IPT_TABLE] == NULL) {
		fprintf(stderr,  "Missing ipt table name, assuming mangle\n");
	} else {
		fprintf(f, "tablename: %s ",
			rta_getattr_str(tb[TCA_IPT_TABLE]));
	}

	if (tb[TCA_IPT_HOOK] == NULL) {
		fprintf(stderr, "Missing ipt hook name\n ");
		return -1;
	}

	hook = rta_getattr_u32(tb[TCA_IPT_HOOK]);
	fprintf(f, " hook: %s\n", ipthooks[hook]);

	if (tb[TCA_IPT_TARG] == NULL) {
		fprintf(stderr, "Missing ipt target parameters\n");
		return -1;
	}


	t = RTA_DATA(tb[TCA_IPT_TARG]);
	m = get_target_name(t->u.user.name);
	if (m != NULL) {
		if (build_st(m, t) < 0) {
			fprintf(stderr, " %s error\n", m->name);
			return -1;
		}

		opts =
			merge_options(opts, m->extra_opts,
				      &m->option_offset);
	} else {
		fprintf(stderr, " failed to find target %s\n\n",
			t->u.user.name);
		return -1;
	}

	fprintf(f, "\ttarget ");
	m->print(NULL, m->t, 0);
	if (tb[TCA_IPT_INDEX] == NULL) {
		fprintf(stderr, "Missing ipt target index\n");
	} else {
		__u32 index;

		index = rta_getattr_u32(tb[TCA_IPT_INDEX]);
		fprintf(f, "\n\tindex %u", index);
	}

	if (tb[TCA_IPT_CNT]) {
		struct tc_cnt *c  = RTA_DATA(tb[TCA_IPT_CNT]);

		fprintf(f, " ref %d bind %d", c->refcnt, c->bindcnt);
	}
	if (show_stats) {
		if (tb[TCA_IPT_TM]) {
			struct tcf_t *tm = RTA_DATA(tb[TCA_IPT_TM]);

			print_tm(f, tm);
		}
	}
	fprintf(f, "\n");

	free_opts(opts);

	return 0;
}

struct action_util ipt_action_util = {
	.id = "ipt",
	.parse_aopt = parse_ipt,
	.print_aopt = print_ipt,
};
