/*
 * nl80211 userspace tool
 *
 * Copyright 2007, 2008	Johannes Berg <johannes@sipsolutions.net>
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

/* libnl 1.x compatibility code */
#if !defined(CONFIG_LIBNL20) && !defined(CONFIG_LIBNL30)
static inline struct nl_handle *nl_socket_alloc(void)
{
	return nl_handle_alloc();
}

static inline void nl_socket_free(struct nl_sock *h)
{
	nl_handle_destroy(h);
}
#endif /* CONFIG_LIBNL20 && CONFIG_LIBNL30 */

int iw_debug = 0;

static int nl80211_init(struct nl80211_state *state)
{
	int err;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_handle_destroy;
	}

	return 0;

 out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

static void nl80211_cleanup(struct nl80211_state *state)
{
	nl_socket_free(state->nl_sock);
}

static int cmd_size;

extern struct cmd __start___cmd;
extern struct cmd __stop___cmd;

#define for_each_cmd(_cmd)					\
	for (_cmd = &__start___cmd; _cmd < &__stop___cmd;		\
	     _cmd = (const struct cmd *)((char *)_cmd + cmd_size))


static void __usage_cmd(const struct cmd *cmd, char *indent, bool full)
{
	const char *start, *lend, *end;

	printf("%s", indent);

	switch (cmd->idby) {
	case CIB_NONE:
		break;
	case CIB_PHY:
		printf("phy <phyname> ");
		break;
	case CIB_NETDEV:
		printf("dev <devname> ");
		break;
	case CIB_WDEV:
		printf("wdev <idx> ");
		break;
	}
	if (cmd->parent && cmd->parent->name)
		printf("%s ", cmd->parent->name);
	printf("%s", cmd->name);
	if (cmd->args)
		printf(" %s", cmd->args);
	printf("\n");

	if (!full || !cmd->help)
		return;

	/* hack */
	if (strlen(indent))
		indent = "\t\t";
	else
		printf("\n");

	/* print line by line */
	start = cmd->help;
	end = strchr(start, '\0');
	do {
		lend = strchr(start, '\n');
		if (!lend)
			lend = end;
		printf("%s", indent);
		printf("%.*s\n", (int)(lend - start), start);
		start = lend + 1;
	} while (end != lend);

	printf("\n");
}

static void usage_options(void)
{
	printf("Options:\n");
	printf("\t--debug\t\tenable netlink debugging\n");
}

static const char *argv0;

static void usage(int argc, char **argv)
{
	const struct cmd *section, *cmd;
	bool full = argc >= 0;
	const char *sect_filt = NULL;
	const char *cmd_filt = NULL;

	if (argc > 0)
		sect_filt = argv[0];

	if (argc > 1)
		cmd_filt = argv[1];

	printf("Usage:\t%s [options] command\n", argv0);
	usage_options();
	printf("\t--version\tshow version (%s)\n", iw_version);
	printf("Commands:\n");
	for_each_cmd(section) {
		if (section->parent)
			continue;

		if (sect_filt && strcmp(section->name, sect_filt))
			continue;

		if (section->handler && !section->hidden)
			__usage_cmd(section, "\t", full);

		for_each_cmd(cmd) {
			if (section != cmd->parent)
				continue;
			if (!cmd->handler || cmd->hidden)
				continue;
			if (cmd_filt && strcmp(cmd->name, cmd_filt))
				continue;
			__usage_cmd(cmd, "\t", full);
		}
	}
	printf("\nCommands that use the netdev ('dev') can also be given the\n"
	       "'wdev' instead to identify the device.\n");
	printf("\nYou can omit the 'phy' or 'dev' if "
			"the identification is unique,\n"
			"e.g. \"iw wlan0 info\" or \"iw phy0 info\". "
			"(Don't when scripting.)\n\n"
			"Do NOT screenscrape this tool, we don't "
			"consider its output stable.\n\n");
}

static int print_help(struct nl80211_state *state,
		      struct nl_cb *cb,
		      struct nl_msg *msg,
		      int argc, char **argv,
		      enum id_input id)
{
	exit(3);
}
TOPLEVEL(help, "[command]", 0, 0, CIB_NONE, print_help,
	 "Print usage for all or a specific command, e.g.\n"
	 "\"help wowlan\" or \"help wowlan enable\".");

static void usage_cmd(const struct cmd *cmd)
{
	printf("Usage:\t%s [options] ", argv0);
	__usage_cmd(cmd, "", true);
	usage_options();
}

static void version(void)
{
	printf("iw version %s\n", iw_version);
}

static int phy_lookup(char *name)
{
	char buf[200];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	return atoi(buf);
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static int __handle_cmd(struct nl80211_state *state, enum id_input idby,
			int argc, char **argv, const struct cmd **cmdout)
{
	const struct cmd *cmd, *match = NULL, *sectcmd;
	struct nl_cb *cb;
	struct nl_cb *s_cb;
	struct nl_msg *msg;
	signed long long devidx = 0;
	int err, o_argc;
	const char *command, *section;
	char *tmp, **o_argv;
	enum command_identify_by command_idby = CIB_NONE;

	if (argc <= 1 && idby != II_NONE)
		return 1;

	o_argc = argc;
	o_argv = argv;

	switch (idby) {
	case II_PHY_IDX:
		command_idby = CIB_PHY;
		devidx = strtoul(*argv + 4, &tmp, 0);
		if (*tmp != '\0')
			return 1;
		argc--;
		argv++;
		break;
	case II_PHY_NAME:
		command_idby = CIB_PHY;
		devidx = phy_lookup(*argv);
		argc--;
		argv++;
		break;
	case II_NETDEV:
		command_idby = CIB_NETDEV;
		devidx = if_nametoindex(*argv);
		if (devidx == 0)
			devidx = -1;
		argc--;
		argv++;
		break;
	case II_WDEV:
		command_idby = CIB_WDEV;
		devidx = strtoll(*argv, &tmp, 0);
		if (*tmp != '\0')
			return 1;
		argc--;
		argv++;
	default:
		break;
	}

	if (devidx < 0)
		return -errno;

	section = *argv;
	argc--;
	argv++;

	for_each_cmd(sectcmd) {
		if (sectcmd->parent)
			continue;
		/* ok ... bit of a hack for the dupe 'info' section */
		if (match && sectcmd->idby != command_idby)
			continue;
		if (strcmp(sectcmd->name, section) == 0)
			match = sectcmd;
	}

	sectcmd = match;
	match = NULL;
	if (!sectcmd)
		return 1;

	if (argc > 0) {
		command = *argv;

		for_each_cmd(cmd) {
			if (!cmd->handler)
				continue;
			if (cmd->parent != sectcmd)
				continue;
			/*
			 * ignore mismatch id by, but allow WDEV
			 * in place of NETDEV
			 */
			if (cmd->idby != command_idby &&
			    !(cmd->idby == CIB_NETDEV &&
			      command_idby == CIB_WDEV))
				continue;
			if (strcmp(cmd->name, command))
				continue;
			if (argc > 1 && !cmd->args)
				continue;
			match = cmd;
			break;
		}

		if (match) {
			argc--;
			argv++;
		}
	}

	if (match)
		cmd = match;
	else {
		/* Use the section itself, if possible. */
		cmd = sectcmd;
		if (argc && !cmd->args)
			return 1;
		if (cmd->idby != command_idby &&
		    !(cmd->idby == CIB_NETDEV && command_idby == CIB_WDEV))
			return 1;
		if (!cmd->handler)
			return 1;
	}

	if (cmd->selector) {
		cmd = cmd->selector(argc, argv);
		if (!cmd)
			return 1;
	}

	if (cmdout)
		*cmdout = cmd;

	if (!cmd->cmd) {
		argc = o_argc;
		argv = o_argv;
		return cmd->handler(state, NULL, NULL, argc, argv, idby);
	}

	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return 2;
	}

	cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	s_cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	if (!cb || !s_cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}

	genlmsg_put(msg, 0, 0, state->nl80211_id, 0,
		    cmd->nl_msg_flags, cmd->cmd, 0);

	switch (command_idby) {
	case CIB_PHY:
		NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, devidx);
		break;
	case CIB_NETDEV:
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
		break;
	case CIB_WDEV:
		NLA_PUT_U64(msg, NL80211_ATTR_WDEV, devidx);
		break;
	default:
		break;
	}

	err = cmd->handler(state, cb, msg, argc, argv, idby);
	if (err)
		goto out;

	nl_socket_set_cb(state->nl_sock, s_cb);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->nl_sock, cb);
 out:
	nl_cb_put(cb);
 out_free_msg:
	nlmsg_free(msg);
	return err;
 nla_put_failure:
	fprintf(stderr, "building message failed\n");
	return 2;
}

int handle_cmd(struct nl80211_state *state, enum id_input idby,
	       int argc, char **argv)
{
	return __handle_cmd(state, idby, argc, argv, NULL);
}

int main(int argc, char **argv)
{
	struct nl80211_state nlstate;
	int err;
	const struct cmd *cmd = NULL;

	/* calculate command size including padding */
	cmd_size = abs((long)&__section_set - (long)&__section_get);
	/* strip off self */
	argc--;
	argv0 = *argv++;

	if (argc > 0 && strcmp(*argv, "--debug") == 0) {
		iw_debug = 1;
		argc--;
		argv++;
	}

	if (argc > 0 && strcmp(*argv, "--version") == 0) {
		version();
		return 0;
	}

	/* need to treat "help" command specially so it works w/o nl80211 */
	if (argc == 0 || strcmp(*argv, "help") == 0) {
		usage(argc - 1, argv + 1);
		return 0;
	}

	err = nl80211_init(&nlstate);
	if (err)
		return 1;

	if (strcmp(*argv, "dev") == 0 && argc > 1) {
		argc--;
		argv++;
		err = __handle_cmd(&nlstate, II_NETDEV, argc, argv, &cmd);
	} else if (strncmp(*argv, "phy", 3) == 0 && argc > 1) {
		if (strlen(*argv) == 3) {
			argc--;
			argv++;
			err = __handle_cmd(&nlstate, II_PHY_NAME, argc, argv, &cmd);
		} else if (*(*argv + 3) == '#')
			err = __handle_cmd(&nlstate, II_PHY_IDX, argc, argv, &cmd);
		else
			goto detect;
	} else if (strcmp(*argv, "wdev") == 0 && argc > 1) {
		argc--;
		argv++;
		err = __handle_cmd(&nlstate, II_WDEV, argc, argv, &cmd);
	} else {
		int idx;
		enum id_input idby = II_NONE;
 detect:
		if ((idx = if_nametoindex(argv[0])) != 0)
			idby = II_NETDEV;
		else if ((idx = phy_lookup(argv[0])) >= 0)
			idby = II_PHY_NAME;
		err = __handle_cmd(&nlstate, idby, argc, argv, &cmd);
	}

	if (err == 1) {
		if (cmd)
			usage_cmd(cmd);
		else
			usage(0, NULL);
	} else if (err < 0)
		fprintf(stderr, "command failed: %s (%d)\n", strerror(-err), err);

	nl80211_cleanup(&nlstate);

	return err;
}
