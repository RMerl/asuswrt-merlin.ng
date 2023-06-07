/*
 * m_action.c		Action Management
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (hadi@cyberus.ca)
 *
 * TODO:
 * - parse to be passed a filedescriptor for logging purposes
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <dlfcn.h>

#include "utils.h"
#include "tc_common.h"
#include "tc_util.h"

static struct action_util *action_list;
#ifdef CONFIG_GACT
static int gact_ld; /* f*ckin backward compatibility */
#endif
static int tab_flush;

static void act_usage(void)
{
	/*XXX: In the near future add a action->print_help to improve
	 * usability
	 * This would mean new tc will not be backward compatible
	 * with any action .so from the old days. But if someone really
	 * does that, they would know how to fix this ..
	 *
	 */
	fprintf(stderr,
		"usage: tc actions <ACTSPECOP>*\n"
		"Where: 	ACTSPECOP := ACR | GD | FL\n"
		"	ACR := add | change | replace <ACTSPEC>*\n"
		"	GD := get | delete | <ACTISPEC>*\n"
		"	FL := ls | list | flush | <ACTNAMESPEC>\n"
		"	ACTNAMESPEC :=  action <ACTNAME>\n"
		"	ACTISPEC := <ACTNAMESPEC> <INDEXSPEC>\n"
		"	ACTSPEC := action <ACTDETAIL> [INDEXSPEC] [HWSTATSSPEC]\n"
		"	INDEXSPEC := index <32 bit indexvalue>\n"
		"	HWSTATSSPEC := hw_stats [ immediate | delayed | disabled ]\n"
		"	ACTDETAIL := <ACTNAME> <ACTPARAMS>\n"
		"		Example ACTNAME is gact, mirred, bpf, etc\n"
		"		Each action has its own parameters (ACTPARAMS)\n"
		"\n");

	exit(-1);
}

static int print_noaopt(struct action_util *au, FILE *f, struct rtattr *opt)
{
	if (opt && RTA_PAYLOAD(opt))
		fprintf(f, "[Unknown action, optlen=%u] ",
			(unsigned int) RTA_PAYLOAD(opt));
	return 0;
}

static int parse_noaopt(struct action_util *au, int *argc_p,
			char ***argv_p, int code, struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc)
		fprintf(stderr,
			"Unknown action \"%s\", hence option \"%s\" is unparsable\n",
			au->id, *argv);
	else
		fprintf(stderr, "Unknown action \"%s\"\n", au->id);

	return -1;
}

static struct action_util *get_action_kind(char *str)
{
	static void *aBODY;
	void *dlh;
	char buf[256];
	struct action_util *a;
#ifdef CONFIG_GACT
	int looked4gact = 0;
restart_s:
#endif
	for (a = action_list; a; a = a->next) {
		if (strcmp(a->id, str) == 0)
			return a;
	}

	snprintf(buf, sizeof(buf), "%s/m_%s.so", get_tc_lib(), str);
	dlh = dlopen(buf, RTLD_LAZY | RTLD_GLOBAL);
	if (dlh == NULL) {
		dlh = aBODY;
		if (dlh == NULL) {
			dlh = aBODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				goto noexist;
		}
	}

	snprintf(buf, sizeof(buf), "%s_action_util", str);
	a = dlsym(dlh, buf);
	if (a == NULL)
		goto noexist;

reg:
	a->next = action_list;
	action_list = a;
	return a;

noexist:
#ifdef CONFIG_GACT
	if (!looked4gact) {
		looked4gact = 1;
		strcpy(str, "gact");
		goto restart_s;
	}
#endif
	a = calloc(1, sizeof(*a));
	if (a) {
		strncpy(a->id, "noact", 15);
		a->parse_aopt = parse_noaopt;
		a->print_aopt = print_noaopt;
		goto reg;
	}
	return a;
}

static bool
new_cmd(char **argv)
{
	return (matches(*argv, "change") == 0) ||
		(matches(*argv, "replace") == 0) ||
		(matches(*argv, "delete") == 0) ||
		(matches(*argv, "get") == 0) ||
		(matches(*argv, "add") == 0);
}

static const struct hw_stats_item {
	const char *str;
	__u8 type;
} hw_stats_items[] = {
	{ "immediate", TCA_ACT_HW_STATS_IMMEDIATE },
	{ "delayed", TCA_ACT_HW_STATS_DELAYED },
	{ "disabled", 0 }, /* no bit set */
};

static void print_hw_stats(const struct rtattr *arg, bool print_used)
{
	struct nla_bitfield32 *hw_stats_bf = RTA_DATA(arg);
	__u8 hw_stats;
	int i;

	hw_stats = hw_stats_bf->value & hw_stats_bf->selector;
	print_string(PRINT_FP, NULL, "\t", NULL);
	open_json_array(PRINT_ANY, print_used ? "used_hw_stats" : "hw_stats");

	for (i = 0; i < ARRAY_SIZE(hw_stats_items); i++) {
		const struct hw_stats_item *item;

		item = &hw_stats_items[i];
		if ((!hw_stats && !item->type) || hw_stats & item->type)
			print_string(PRINT_ANY, NULL, " %s", item->str);
	}
	close_json_array(PRINT_JSON, NULL);
	print_nl();
}

static int parse_hw_stats(const char *str, struct nlmsghdr *n)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hw_stats_items); i++) {
		const struct hw_stats_item *item;

		item = &hw_stats_items[i];
		if (matches(str, item->str) == 0) {
			struct nla_bitfield32 hw_stats_bf = {
				.value = item->type,
				.selector = item->type
			};

			addattr_l(n, MAX_MSG, TCA_ACT_HW_STATS,
				  &hw_stats_bf, sizeof(hw_stats_bf));
			return 0;
		}

	}
	return -1;
}

int parse_action(int *argc_p, char ***argv_p, int tca_id, struct nlmsghdr *n)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	struct rtattr *tail, *tail2;
	char k[FILTER_NAMESZ];
	int act_ck_len = 0;
	int ok = 0;
	int eap = 0; /* expect action parameters */

	int ret = 0;
	int prio = 0;
	unsigned char act_ck[TC_COOKIE_MAX_SIZE];

	if (argc <= 0)
		return -1;

	tail2 = addattr_nest(n, MAX_MSG, tca_id);

	while (argc > 0) {

		memset(k, 0, sizeof(k));

		if (strcmp(*argv, "action") == 0) {
			argc--;
			argv++;
			eap = 1;
#ifdef CONFIG_GACT
			if (!gact_ld)
				get_action_kind("gact");
#endif
			continue;
		} else if (strcmp(*argv, "flowid") == 0) {
			break;
		} else if (strcmp(*argv, "classid") == 0) {
			break;
		} else if (strcmp(*argv, "help") == 0) {
			return -1;
		} else if (new_cmd(argv)) {
			goto done0;
		} else {
			struct action_util *a = NULL;

			if (!action_a2n(*argv, NULL, false))
				strncpy(k, "gact", sizeof(k) - 1);
			else
				strncpy(k, *argv, sizeof(k) - 1);
			eap = 0;
			if (argc > 0) {
				a = get_action_kind(k);
			} else {
done0:
				if (ok)
					break;
				else
					goto done;
			}

			if (a == NULL)
				goto bad_val;


			tail = addattr_nest(n, MAX_MSG, ++prio);
			addattr_l(n, MAX_MSG, TCA_ACT_KIND, k, strlen(k) + 1);

			ret = a->parse_aopt(a, &argc, &argv,
					    TCA_ACT_OPTIONS | NLA_F_NESTED,
					    n);

			if (ret < 0) {
				fprintf(stderr, "bad action parsing\n");
				goto bad_val;
			}

			if (*argv && strcmp(*argv, "cookie") == 0) {
				size_t slen;

				NEXT_ARG();
				slen = strlen(*argv);
				if (slen > TC_COOKIE_MAX_SIZE * 2) {
					char cookie_err_m[128];

					snprintf(cookie_err_m, 128,
						 "%zd Max allowed size %d",
						 slen, TC_COOKIE_MAX_SIZE*2);
					invarg(cookie_err_m, *argv);
				}

				if (slen % 2 ||
				    hex2mem(*argv, act_ck, slen / 2) < 0)
					invarg("cookie must be a hex string\n",
					       *argv);

				act_ck_len = slen / 2;
				argc--;
				argv++;
			}

			if (act_ck_len)
				addattr_l(n, MAX_MSG, TCA_ACT_COOKIE,
					  &act_ck, act_ck_len);

			if (*argv && matches(*argv, "hw_stats") == 0) {
				NEXT_ARG();
				ret = parse_hw_stats(*argv, n);
				if (ret < 0)
					invarg("value is invalid\n", *argv);
				NEXT_ARG_FWD();
			}

			if (*argv && strcmp(*argv, "no_percpu") == 0) {
				struct nla_bitfield32 flags =
					{ TCA_ACT_FLAGS_NO_PERCPU_STATS,
					  TCA_ACT_FLAGS_NO_PERCPU_STATS };

				addattr_l(n, MAX_MSG, TCA_ACT_FLAGS, &flags,
					  sizeof(struct nla_bitfield32));
				NEXT_ARG_FWD();
			}

			addattr_nest_end(n, tail);
			ok++;
		}
	}

	if (eap > 0) {
		fprintf(stderr, "bad action empty %d\n", eap);
		goto bad_val;
	}

	addattr_nest_end(n, tail2);

done:
	*argc_p = argc;
	*argv_p = argv;
	return 0;
bad_val:
	/* no need to undo things, returning from here should
	 * cause enough pain
	 */
	fprintf(stderr, "parse_action: bad value (%d:%s)!\n", argc, *argv);
	return -1;
}

static int tc_print_one_action(FILE *f, struct rtattr *arg)
{

	struct rtattr *tb[TCA_ACT_MAX + 1];
	int err = 0;
	struct action_util *a = NULL;

	if (arg == NULL)
		return -1;

	parse_rtattr_nested(tb, TCA_ACT_MAX, arg);

	if (tb[TCA_ACT_KIND] == NULL) {
		fprintf(stderr, "NULL Action!\n");
		return -1;
	}


	a = get_action_kind(RTA_DATA(tb[TCA_ACT_KIND]));
	if (a == NULL)
		return err;

	err = a->print_aopt(a, f, tb[TCA_ACT_OPTIONS]);

	if (err < 0)
		return err;

	if (brief && tb[TCA_ACT_INDEX]) {
		print_uint(PRINT_ANY, "index", "\t index %u",
			   rta_getattr_u32(tb[TCA_ACT_INDEX]));
		print_nl();
	}
	if (show_stats && tb[TCA_ACT_STATS]) {
		print_string(PRINT_FP, NULL, "\tAction statistics:", NULL);
		print_nl();
		open_json_object("stats");
		print_tcstats2_attr(f, tb[TCA_ACT_STATS], "\t", NULL);
		close_json_object();
		print_nl();
	}
	if (tb[TCA_ACT_COOKIE]) {
		int strsz = RTA_PAYLOAD(tb[TCA_ACT_COOKIE]);
		char b1[strsz * 2 + 1];

		print_string(PRINT_ANY, "cookie", "\tcookie %s",
			     hexstring_n2a(RTA_DATA(tb[TCA_ACT_COOKIE]),
					   strsz, b1, sizeof(b1)));
		print_nl();
	}
	if (tb[TCA_ACT_FLAGS]) {
		struct nla_bitfield32 *flags = RTA_DATA(tb[TCA_ACT_FLAGS]);

		if (flags->selector & TCA_ACT_FLAGS_NO_PERCPU_STATS)
			print_bool(PRINT_ANY, "no_percpu", "\tno_percpu",
				   flags->value &
				   TCA_ACT_FLAGS_NO_PERCPU_STATS);
		print_nl();
	}
	if (tb[TCA_ACT_HW_STATS])
		print_hw_stats(tb[TCA_ACT_HW_STATS], false);

	if (tb[TCA_ACT_USED_HW_STATS])
		print_hw_stats(tb[TCA_ACT_USED_HW_STATS], true);

	return 0;
}

static int
tc_print_action_flush(FILE *f, const struct rtattr *arg)
{

	struct rtattr *tb[TCA_MAX + 1];
	int err = 0;
	struct action_util *a = NULL;
	__u32 *delete_count = 0;

	parse_rtattr_nested(tb, TCA_MAX, arg);

	if (tb[TCA_KIND] == NULL) {
		fprintf(stderr, "NULL Action!\n");
		return -1;
	}

	a = get_action_kind(RTA_DATA(tb[TCA_KIND]));
	if (a == NULL)
		return err;

	delete_count = RTA_DATA(tb[TCA_FCNT]);
	fprintf(f, " %s (%d entries)\n", a->id, *delete_count);
	tab_flush = 0;
	return 0;
}

int
tc_print_action(FILE *f, const struct rtattr *arg, unsigned short tot_acts)
{

	int i;

	if (arg == NULL)
		return 0;

	if (!tot_acts)
		tot_acts = TCA_ACT_MAX_PRIO;

	struct rtattr *tb[tot_acts + 1];

	parse_rtattr_nested(tb, tot_acts, arg);

	if (tab_flush && tb[0] && !tb[1])
		return tc_print_action_flush(f, tb[0]);

	open_json_array(PRINT_JSON, "actions");
	for (i = 0; i <= tot_acts; i++) {
		if (tb[i]) {
			open_json_object(NULL);
			print_nl();
			print_uint(PRINT_ANY, "order",
				   "\taction order %u: ", i);
			if (tc_print_one_action(f, tb[i]) < 0) {
				print_string(PRINT_FP, NULL,
					     "Error printing action\n", NULL);
			}
			close_json_object();
		}

	}
	close_json_array(PRINT_JSON, NULL);

	return 0;
}

int print_action(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct tcamsg *t = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	__u32 *tot_acts = NULL;
	struct rtattr *tb[TCA_ROOT_MAX+1];

	len -= NLMSG_LENGTH(sizeof(*t));

	if (len < 0) {
		fprintf(stderr, "Wrong len %d\n", len);
		return -1;
	}

	parse_rtattr(tb, TCA_ROOT_MAX, TA_RTA(t), len);

	if (tb[TCA_ROOT_COUNT])
		tot_acts = RTA_DATA(tb[TCA_ROOT_COUNT]);

	open_json_object(NULL);
	print_uint(PRINT_ANY, "total acts", "total acts %u",
		   tot_acts ? *tot_acts : 0);
	print_nl();
	close_json_object();
	if (tb[TCA_ACT_TAB] == NULL) {
		if (n->nlmsg_type != RTM_GETACTION)
			fprintf(stderr, "print_action: NULL kind\n");
		return -1;
	}

	if (n->nlmsg_type == RTM_DELACTION) {
		if (n->nlmsg_flags & NLM_F_ROOT) {
			fprintf(fp, "Flushed table ");
			tab_flush = 1;
		} else {
			fprintf(fp, "Deleted action ");
		}
	}

	if (n->nlmsg_type == RTM_NEWACTION) {
		if ((n->nlmsg_flags & NLM_F_CREATE) &&
		    !(n->nlmsg_flags & NLM_F_REPLACE)) {
			fprintf(fp, "Added action ");
		} else if (n->nlmsg_flags & NLM_F_REPLACE) {
			fprintf(fp, "Replaced action ");
		}
	}

	open_json_object(NULL);
	tc_print_action(fp, tb[TCA_ACT_TAB], tot_acts ? *tot_acts:0);
	close_json_object();

	return 0;
}

static int tc_action_gd(int cmd, unsigned int flags,
			int *argc_p, char ***argv_p)
{
	char k[FILTER_NAMESZ];
	struct action_util *a = NULL;
	int argc = *argc_p;
	char **argv = *argv_p;
	int prio = 0;
	int ret = 0;
	__u32 i = 0;
	struct rtattr *tail;
	struct rtattr *tail2;
	struct nlmsghdr *ans = NULL;

	struct {
		struct nlmsghdr         n;
		struct tcamsg           t;
		char                    buf[MAX_MSG];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcamsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.t.tca_family = AF_UNSPEC,
	};

	argc -= 1;
	argv += 1;


	tail = addattr_nest(&req.n, MAX_MSG, TCA_ACT_TAB);

	while (argc > 0) {
		if (strcmp(*argv, "action") == 0) {
			argc--;
			argv++;
			continue;
		} else if (strcmp(*argv, "help") == 0) {
			return -1;
		}

		strncpy(k, *argv, sizeof(k) - 1);
		a = get_action_kind(k);
		if (a == NULL) {
			fprintf(stderr, "Error: non existent action: %s\n", k);
			ret = -1;
			goto bad_val;
		}
		if (strcmp(a->id, k) != 0) {
			fprintf(stderr, "Error: non existent action: %s\n", k);
			ret = -1;
			goto bad_val;
		}

		argc -= 1;
		argv += 1;
		if (argc <= 0) {
			fprintf(stderr,
				"Error: no index specified action: %s\n", k);
			ret = -1;
			goto bad_val;
		}

		if (matches(*argv, "index") == 0) {
			NEXT_ARG();
			if (get_u32(&i, *argv, 10)) {
				fprintf(stderr, "Illegal \"index\"\n");
				ret = -1;
				goto bad_val;
			}
			argc -= 1;
			argv += 1;
		} else {
			fprintf(stderr,
				"Error: no index specified action: %s\n", k);
			ret = -1;
			goto bad_val;
		}

		tail2 = addattr_nest(&req.n, MAX_MSG, ++prio);
		addattr_l(&req.n, MAX_MSG, TCA_ACT_KIND, k, strlen(k) + 1);
		if (i > 0)
			addattr32(&req.n, MAX_MSG, TCA_ACT_INDEX, i);
		addattr_nest_end(&req.n, tail2);

	}

	addattr_nest_end(&req.n, tail);

	req.n.nlmsg_seq = rth.dump = ++rth.seq;

	if (rtnl_talk(&rth, &req.n, cmd == RTM_DELACTION ? NULL : &ans) < 0) {
		fprintf(stderr, "We have an error talking to the kernel\n");
		return 1;
	}

	if (cmd == RTM_GETACTION) {
		new_json_obj(json);
		ret = print_action(ans, stdout);
		if (ret < 0) {
			fprintf(stderr, "Dump terminated\n");
			free(ans);
			delete_json_obj();
			return 1;
		}
		delete_json_obj();
	}
	free(ans);

	*argc_p = argc;
	*argv_p = argv;
bad_val:
	return ret;
}

static int tc_action_modify(int cmd, unsigned int flags,
			    int *argc_p, char ***argv_p)
{
	int argc = *argc_p;
	char **argv = *argv_p;
	int ret = 0;
	struct {
		struct nlmsghdr         n;
		struct tcamsg           t;
		char                    buf[MAX_MSG];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcamsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.t.tca_family = AF_UNSPEC,
	};
	struct rtattr *tail = NLMSG_TAIL(&req.n);

	argc -= 1;
	argv += 1;
	if (parse_action(&argc, &argv, TCA_ACT_TAB, &req.n)) {
		fprintf(stderr, "Illegal \"action\"\n");
		return -1;
	}
	tail->rta_len = (void *) NLMSG_TAIL(&req.n) - (void *) tail;

	if (rtnl_talk(&rth, &req.n, NULL) < 0) {
		fprintf(stderr, "We have an error talking to the kernel\n");
		ret = -1;
	}

	*argc_p = argc;
	*argv_p = argv;

	return ret;
}

static int tc_act_list_or_flush(int *argc_p, char ***argv_p, int event)
{
	struct rtattr *tail, *tail2, *tail3, *tail4;
	int ret = 0, prio = 0, msg_size = 0;
	struct action_util *a = NULL;
	struct nla_bitfield32 flag_select = { 0 };
	char **argv = *argv_p;
	__u32 msec_since = 0;
	int argc = *argc_p;
	char k[FILTER_NAMESZ];
	struct {
		struct nlmsghdr         n;
		struct tcamsg           t;
		char                    buf[MAX_MSG];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcamsg)),
		.t.tca_family = AF_UNSPEC,
	};

	tail = addattr_nest(&req.n, MAX_MSG, TCA_ACT_TAB);
	tail2 = NLMSG_TAIL(&req.n);

	strncpy(k, *argv, sizeof(k) - 1);
#ifdef CONFIG_GACT
	if (!gact_ld)
		get_action_kind("gact");

#endif
	a = get_action_kind(k);
	if (a == NULL) {
		fprintf(stderr, "bad action %s\n", k);
		goto bad_val;
	}
	if (strcmp(a->id, k) != 0) {
		fprintf(stderr, "bad action %s\n", k);
		goto bad_val;
	}
	strncpy(k, *argv, sizeof(k) - 1);

	argc -= 1;
	argv += 1;

	if (argc && (strcmp(*argv, "since") == 0)) {
		NEXT_ARG();
		if (get_u32(&msec_since, *argv, 0))
			invarg("dump time \"since\" is invalid", *argv);
	}

	addattr_l(&req.n, MAX_MSG, ++prio, NULL, 0);
	addattr_l(&req.n, MAX_MSG, TCA_ACT_KIND, k, strlen(k) + 1);
	tail2->rta_len = (void *) NLMSG_TAIL(&req.n) - (void *) tail2;
	addattr_nest_end(&req.n, tail);

	tail3 = NLMSG_TAIL(&req.n);
	flag_select.value |= TCA_ACT_FLAG_LARGE_DUMP_ON;
	flag_select.selector |= TCA_ACT_FLAG_LARGE_DUMP_ON;
	if (brief) {
		flag_select.value |= TCA_ACT_FLAG_TERSE_DUMP;
		flag_select.selector |= TCA_ACT_FLAG_TERSE_DUMP;
	}
	addattr_l(&req.n, MAX_MSG, TCA_ROOT_FLAGS, &flag_select,
		  sizeof(struct nla_bitfield32));
	tail3->rta_len = (void *) NLMSG_TAIL(&req.n) - (void *) tail3;
	if (msec_since) {
		tail4 = NLMSG_TAIL(&req.n);
		addattr32(&req.n, MAX_MSG, TCA_ROOT_TIME_DELTA, msec_since);
		tail4->rta_len = (void *) NLMSG_TAIL(&req.n) - (void *) tail4;
	}
	msg_size = NLMSG_ALIGN(req.n.nlmsg_len)
		- NLMSG_ALIGN(sizeof(struct nlmsghdr));

	if (event == RTM_GETACTION) {
		if (rtnl_dump_request(&rth, event,
				      (void *)&req.t, msg_size) < 0) {
			perror("Cannot send dump request");
			return 1;
		}
		new_json_obj(json);
		ret = rtnl_dump_filter(&rth, print_action, stdout);
		delete_json_obj();
	}

	if (event == RTM_DELACTION) {
		req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len);
		req.n.nlmsg_type = RTM_DELACTION;
		req.n.nlmsg_flags |= NLM_F_ROOT;
		req.n.nlmsg_flags |= NLM_F_REQUEST;
		if (rtnl_talk(&rth, &req.n, NULL) < 0) {
			fprintf(stderr, "We have an error flushing\n");
			return 1;
		}

	}

bad_val:

	*argc_p = argc;
	*argv_p = argv;
	return ret;
}

int do_action(int argc, char **argv)
{

	int ret = 0;

	while (argc > 0) {

		if (matches(*argv, "add") == 0) {
			ret =  tc_action_modify(RTM_NEWACTION,
						NLM_F_EXCL | NLM_F_CREATE,
						&argc, &argv);
		} else if (matches(*argv, "change") == 0 ||
			  matches(*argv, "replace") == 0) {
			ret = tc_action_modify(RTM_NEWACTION,
					       NLM_F_CREATE | NLM_F_REPLACE,
					       &argc, &argv);
		} else if (matches(*argv, "delete") == 0) {
			argc -= 1;
			argv += 1;
			ret = tc_action_gd(RTM_DELACTION, 0,  &argc, &argv);
		} else if (matches(*argv, "get") == 0) {
			argc -= 1;
			argv += 1;
			ret = tc_action_gd(RTM_GETACTION, 0,  &argc, &argv);
		} else if (matches(*argv, "list") == 0 ||
			   matches(*argv, "show") == 0 ||
			   matches(*argv, "lst") == 0) {
			if (argc <= 2) {
				act_usage();
				return -1;
			}

			argc -= 2;
			argv += 2;
			return tc_act_list_or_flush(&argc, &argv,
						    RTM_GETACTION);
		} else if (matches(*argv, "flush") == 0) {
			if (argc <= 2) {
				act_usage();
				return -1;
			}

			argc -= 2;
			argv += 2;
			return tc_act_list_or_flush(&argc, &argv,
						    RTM_DELACTION);
		} else if (matches(*argv, "help") == 0) {
			act_usage();
			return -1;
		} else {
			fprintf(stderr,
				"Command \"%s\" is unknown, try \"tc actions help\".\n",
				*argv);
			return -1;
		}

		if (ret < 0)
			return -1;
	}

	return 0;
}
