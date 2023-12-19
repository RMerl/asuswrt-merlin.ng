/*
 * features.c - netlink implementation of netdev features commands
 *
 * Implementation of "ethtool -k <dev>".
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "strset.h"
#include "bitset.h"

/* FEATURES_GET */

struct feature_results {
	uint32_t	*hw;
	uint32_t	*wanted;
	uint32_t	*active;
	uint32_t	*nochange;
	unsigned int	count;
	unsigned int	words;
};

static int prepare_feature_results(const struct nlattr *const *tb,
				   struct feature_results *dest)
{
	unsigned int count;
	int ret;

	memset(dest, '\0', sizeof(*dest));
	if (!tb[ETHTOOL_A_FEATURES_HW] || !tb[ETHTOOL_A_FEATURES_WANTED] ||
	    !tb[ETHTOOL_A_FEATURES_ACTIVE] || !tb[ETHTOOL_A_FEATURES_NOCHANGE])
		return -EFAULT;
	count = bitset_get_count(tb[ETHTOOL_A_FEATURES_HW], &ret);
	if (ret < 0)
		return -EFAULT;
	if ((bitset_get_count(tb[ETHTOOL_A_FEATURES_WANTED], &ret) != count) ||
	    (bitset_get_count(tb[ETHTOOL_A_FEATURES_ACTIVE], &ret) != count) ||
	    (bitset_get_count(tb[ETHTOOL_A_FEATURES_NOCHANGE], &ret) != count))
		return -EFAULT;
	dest->hw = get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_HW]);
	dest->wanted = get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_WANTED]);
	dest->active = get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_ACTIVE]);
	dest->nochange =
		get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_NOCHANGE]);
	if (!dest->hw || !dest->wanted || !dest->active || !dest->nochange)
		return -EFAULT;
	dest->count = count;
	dest->words = (count + 31) / 32;

	return 0;
}

static bool feature_on(const uint32_t *bitmap, unsigned int idx)
{
	return bitmap[idx / 32] & (1 << (idx % 32));
}

static void dump_feature(const struct feature_results *results,
			 const uint32_t *ref, const uint32_t *ref_mask,
			 unsigned int idx, const char *name, const char *prefix)
{
	const char *suffix = "";

	if (!name || !*name)
		return;
	if (ref) {
		if (ref_mask && !feature_on(ref_mask, idx))
			return;
		if ((!ref_mask || feature_on(ref_mask, idx)) &&
		    (feature_on(results->active, idx) == feature_on(ref, idx)))
			return;
	}

	if (!feature_on(results->hw, idx) || feature_on(results->nochange, idx))
		suffix = " [fixed]";
	else if (feature_on(results->active, idx) !=
		 feature_on(results->wanted, idx))
		suffix = feature_on(results->wanted, idx) ?
			" [requested on]" : " [requested off]";
	if (is_json_context()) {
		open_json_object(name);
		print_bool(PRINT_JSON, "active", NULL, feature_on(results->active, idx));
		print_bool(PRINT_JSON, "fixed", NULL,
			   (!feature_on(results->hw, idx) || feature_on(results->nochange, idx)));
		print_bool(PRINT_JSON, "requested", NULL, feature_on(results->wanted, idx));
		close_json_object();
	} else {
		printf("%s%s: %s%s\n", prefix, name,
		       feature_on(results->active, idx) ? "on" : "off", suffix);
	}
}

/* this assumes pattern contains no more than one asterisk */
static bool flag_pattern_match(const char *name, const char *pattern)
{
	const char *p_ast = strchr(pattern, '*');

	if (p_ast) {
		size_t name_len = strlen(name);
		size_t pattern_len = strlen(pattern);

		if (name_len + 1 < pattern_len)
			return false;
		if (strncmp(name, pattern, p_ast - pattern))
			return false;
		pattern_len -= (p_ast - pattern) + 1;
		name += name_len  - pattern_len;
		pattern = p_ast + 1;
	}
	return !strcmp(name, pattern);
}

int dump_features(const struct nlattr *const *tb,
		  const struct stringset *feature_names)
{
	unsigned int *feature_flags = NULL;
	struct feature_results results;
	unsigned int i, j;
	int ret;

	ret = prepare_feature_results(tb, &results);
	if (ret < 0)
		return -EFAULT;
	feature_flags = calloc(results.count, sizeof(feature_flags[0]));
	if (!feature_flags)
		return -ENOMEM;

	/* map netdev features to legacy flags */
	for (i = 0; i < results.count; i++) {
		const char *name = get_string(feature_names, i);
		feature_flags[i] = UINT_MAX;

		if (!name || !*name)
			continue;
		for (j = 0; j < OFF_FLAG_DEF_SIZE; j++) {
			const char *flag_name = off_flag_def[j].kernel_name;

			if (flag_pattern_match(name, flag_name)) {
				feature_flags[i] = j;
				break;
			}
		}
	}
	/* show legacy flags and their matching features first */
	for (i = 0; i < OFF_FLAG_DEF_SIZE; i++) {
		unsigned int n_match = 0;
		bool flag_value = false;

		/* no kernel with netlink interface supports UFO */
		if (off_flag_def[i].value == ETH_FLAG_UFO)
			continue;

		for (j = 0; j < results.count; j++) {
			if (feature_flags[j] == i) {
				n_match++;
				flag_value = flag_value ||
					feature_on(results.active, j);
			}
		}
		if (n_match != 1) {
			if (is_json_context()) {
				open_json_object(off_flag_def[i].long_name);
				print_bool(PRINT_JSON, "active", NULL, flag_value);
				print_null(PRINT_JSON, "fixed", NULL, NULL);
				print_null(PRINT_JSON, "requested", NULL, NULL);
				close_json_object();
			} else {
				printf("%s: %s\n", off_flag_def[i].long_name,
				       flag_value ? "on" : "off");
			}
		}
		if (n_match == 0)
			continue;
		for (j = 0; j < results.count; j++) {
			const char *name = get_string(feature_names, j);

			if (feature_flags[j] != i)
				continue;
			if (n_match == 1)
				dump_feature(&results, NULL, NULL, j,
					     off_flag_def[i].long_name, "");
			else
				dump_feature(&results, NULL, NULL, j, name,
					     "\t");
		}
	}
	/* and, finally, remaining netdev_features not matching legacy flags */
	for (i = 0; i < results.count; i++) {
		const char *name = get_string(feature_names, i);

		if (!name || !*name || feature_flags[i] != UINT_MAX)
			continue;
		dump_feature(&results, NULL, NULL, i, name, "");
	}

	free(feature_flags);
	return 0;
}

int features_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_FEATURES_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	const struct stringset *feature_names;
	struct nl_context *nlctx = data;
	bool silent;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	if (!nlctx->is_monitor) {
		ret = netlink_init_ethnl2_socket(nlctx);
		if (ret < 0)
			return MNL_CB_ERROR;
	}
	feature_names = global_stringset(ETH_SS_FEATURES, nlctx->ethnl2_socket);

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return silent ? MNL_CB_OK : MNL_CB_ERROR;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_FEATURES_HEADER]);
	if (!dev_ok(nlctx))
		return MNL_CB_OK;

	if (silent)
		putchar('\n');
	open_json_object(NULL);
	print_string(PRINT_ANY, "ifname", "Features for %s:\n", nlctx->devname);
	ret = dump_features(tb, feature_names);
	close_json_object();
	return (silent || !ret) ? MNL_CB_OK : MNL_CB_ERROR;
}

int nl_gfeatures(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_FEATURES_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_FEATURES_GET,
				      ETHTOOL_A_FEATURES_HEADER,
				      ETHTOOL_FLAG_COMPACT_BITSETS);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, features_reply_cb);
	delete_json_obj();

	return ret;
}

/* FEATURES_SET */

struct sfeatures_context {
	bool			nothing_changed;
	uint32_t		req_mask[0];
};

static int find_feature(const char *name,
			const struct stringset *feature_names)
{
	const unsigned int count = get_count(feature_names);
	unsigned int i;

	for (i = 0; i < count; i++)
		if (!strcmp(name, get_string(feature_names, i)))
			return i;

	return -1;
}

static int fill_feature(struct nl_msg_buff *msgbuff, const char *name, bool val)
{
	struct nlattr *bit_attr;

	bit_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_BITSET_BITS_BIT);
	if (!bit_attr)
		return -EMSGSIZE;
	if (ethnla_put_strz(msgbuff, ETHTOOL_A_BITSET_BIT_NAME, name))
		return -EMSGSIZE;
	if (ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_BIT_VALUE, val))
		return -EMSGSIZE;
	mnl_attr_nest_end(msgbuff->nlhdr, bit_attr);

	return 0;
}

static void set_sf_req_mask(struct nl_context *nlctx, unsigned int idx)
{
	struct sfeatures_context *sfctx = nlctx->cmd_private;

	sfctx->req_mask[idx / 32] |= (1 << (idx % 32));
}

static int fill_legacy_flag(struct nl_context *nlctx, const char *flag_name,
			    const struct stringset *feature_names, bool val)
{
	struct nl_msg_buff *msgbuff = &nlctx->ethnl_socket->msgbuff;
	const unsigned int count = get_count(feature_names);
	unsigned int i, j;
	int ret;

	for (i = 0; i < OFF_FLAG_DEF_SIZE; i++) {
		const char *pattern;

		if (strcmp(flag_name, off_flag_def[i].short_name) &&
		    strcmp(flag_name, off_flag_def[i].long_name))
			continue;
		pattern = off_flag_def[i].kernel_name;

		for (j = 0; j < count; j++) {
			const char *name = get_string(feature_names, j);

			if (flag_pattern_match(name, pattern)) {
				ret = fill_feature(msgbuff, name, val);
				if (ret < 0)
					return ret;
				set_sf_req_mask(nlctx, j);
			}
		}

		return 0;
	}

	return 1;
}

int fill_sfeatures_bitmap(struct nl_context *nlctx,
			  const struct stringset *feature_names)
{
	struct nl_msg_buff *msgbuff = &nlctx->ethnl_socket->msgbuff;
	struct nlattr *bitset_attr;
	struct nlattr *bits_attr;
	int ret;

	ret = -EMSGSIZE;
	bitset_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_FEATURES_WANTED);
	if (!bitset_attr)
		return ret;
	bits_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_BITSET_BITS);
	if (!bits_attr)
		goto err;

	while (nlctx->argc > 0) {
		bool val;

		if (!strcmp(*nlctx->argp, "--")) {
			nlctx->argp++;
			nlctx->argc--;
			break;
		}
		ret = -EINVAL;
		if (nlctx->argc < 2 ||
		    (strcmp(nlctx->argp[1], "on") &&
		     strcmp(nlctx->argp[1], "off"))) {
			fprintf(stderr,
				"ethtool (%s): flag '%s' for parameter '%s' is"
				" not followed by 'on' or 'off'\n",
				nlctx->cmd, nlctx->argp[1], nlctx->param);
			goto err;
		}

		val = !strcmp(nlctx->argp[1], "on");
		ret = fill_legacy_flag(nlctx, nlctx->argp[0], feature_names,
				       val);
		if (ret > 0) {
			ret = fill_feature(msgbuff, nlctx->argp[0], val);
			if (ret == 0) {
				int idx = find_feature(nlctx->argp[0],
						       feature_names);

				if (idx >= 0)
					set_sf_req_mask(nlctx, idx);
			}
		}
		if (ret < 0)
			goto err;

		nlctx->argp += 2;
		nlctx->argc -= 2;
	}

	ethnla_nest_end(msgbuff, bits_attr);
	ethnla_nest_end(msgbuff, bitset_attr);
	return 0;
err:
	ethnla_nest_cancel(msgbuff, bitset_attr);
	return ret;
}

static void show_feature_changes(struct nl_context *nlctx,
				 const struct nlattr *const *tb)
{
	struct sfeatures_context *sfctx = nlctx->cmd_private;
	const struct stringset *feature_names;
	const uint32_t *wanted_mask;
	const uint32_t *active_mask;
	const uint32_t *wanted_val;
	const uint32_t *active_val;
	unsigned int count, words;
	unsigned int i;
	bool diff;
	int ret;

	feature_names = global_stringset(ETH_SS_FEATURES, nlctx->ethnl_socket);
	count = get_count(feature_names);
	words = DIV_ROUND_UP(count, 32);

	if (!tb[ETHTOOL_A_FEATURES_WANTED] || !tb[ETHTOOL_A_FEATURES_ACTIVE])
		goto err;
	if (bitset_get_count(tb[ETHTOOL_A_FEATURES_WANTED], &ret) != count ||
	    ret < 0)
		goto err;
	if (bitset_get_count(tb[ETHTOOL_A_FEATURES_ACTIVE], &ret) != count ||
	    ret < 0)
		goto err;
	wanted_val = get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_WANTED]);
	wanted_mask = get_compact_bitset_mask(tb[ETHTOOL_A_FEATURES_WANTED]);
	active_val = get_compact_bitset_value(tb[ETHTOOL_A_FEATURES_ACTIVE]);
	active_mask = get_compact_bitset_mask(tb[ETHTOOL_A_FEATURES_ACTIVE]);
	if (!wanted_val || !wanted_mask || !active_val || !active_mask)
		goto err;

	sfctx->nothing_changed = true;
	diff = false;
	for (i = 0; i < words; i++) {
		if (wanted_mask[i] != sfctx->req_mask[i])
			sfctx->nothing_changed = false;
		if (wanted_mask[i] || (active_mask[i] & ~sfctx->req_mask[i]))
			diff = true;
	}
	if (!diff)
		return;

	/* result is not exactly as requested, show differences */
	printf("Actual changes:\n");
	for (i = 0; i < count; i++) {
		const char *name = get_string(feature_names, i);

		if (!name)
			continue;
		if (!feature_on(wanted_mask, i) && !feature_on(active_mask, i))
			continue;
		printf("%s: ", name);
		if (feature_on(wanted_mask, i))
			/* we requested a value but result is different */
			printf("%s [requested %s]",
			       feature_on(wanted_val, i) ? "off" : "on",
			       feature_on(wanted_val, i) ? "on" : "off");
		else if (!feature_on(sfctx->req_mask, i))
			/* not requested but changed anyway */
			printf("%s [not requested]",
			       feature_on(active_val, i) ? "on" : "off");
		else
			printf("%s", feature_on(active_val, i) ? "on" : "off");
		fputc('\n', stdout);
	}

	return;
err:
	fprintf(stderr, "malformed diff info from kernel\n");
}

int sfeatures_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct genlmsghdr *ghdr = (const struct genlmsghdr *)(nlhdr + 1);
	const struct nlattr *tb[ETHTOOL_A_FEATURES_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	const char *devname;
	int ret;

	if (ghdr->cmd != ETHTOOL_MSG_FEATURES_SET_REPLY) {
		fprintf(stderr, "warning: unexpected reply message type %u\n",
			ghdr->cmd);
		return MNL_CB_OK;
	}
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return ret;
	devname = get_dev_name(tb[ETHTOOL_A_FEATURES_HEADER]);
	if (strcmp(devname, nlctx->devname)) {
		fprintf(stderr, "warning: unexpected message for device %s\n",
			devname);
		return MNL_CB_OK;
	}

	show_feature_changes(nlctx, tb);
	return MNL_CB_OK;
}

int nl_sfeatures(struct cmd_context *ctx)
{
	const struct stringset *feature_names;
	struct nl_context *nlctx = ctx->nlctx;
	struct sfeatures_context *sfctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	unsigned int words;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_FEATURES_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-K";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->cmd_private = &sfctx;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	feature_names = global_stringset(ETH_SS_FEATURES, nlctx->ethnl_socket);
	words = (get_count(feature_names) + 31) / 32;
	sfctx = malloc(sizeof(*sfctx) + words * sizeof(sfctx->req_mask[0]));
	if (!sfctx)
		return -ENOMEM;
	memset(sfctx, '\0',
	       sizeof(*sfctx) + words * sizeof(sfctx->req_mask[0]));
	nlctx->cmd_private = sfctx;

	nlctx->devname = ctx->devname;
	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_FEATURES_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_FEATURES_HEADER, ctx->devname,
			       ETHTOOL_FLAG_COMPACT_BITSETS))
		return -EMSGSIZE;
	ret = fill_sfeatures_bitmap(nlctx, feature_names);
	if (ret < 0)
		return ret;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 92;
	ret = nlsock_process_reply(nlsk, sfeatures_reply_cb, nlctx);
	if (sfctx->nothing_changed) {
		fprintf(stderr, "Could not change any device features\n");
		return nlctx->exit_code ?: 1;
	}
	if (ret == 0)
		return 0;
	return nlctx->exit_code ?: 92;
}
