/*
 * strset.c - string set handling
 *
 * Implementation of local cache of ethtool string sets.
 */

#include <errno.h>
#include <string.h>

#include "../internal.h"
#include "netlink.h"
#include "nlsock.h"
#include "msgbuff.h"

struct stringset {
	const char		**strings;
	void			*raw_data;
	unsigned int		count;
	bool			loaded;
};

struct perdev_strings {
	int			ifindex;
	char			devname[ALTIFNAMSIZ];
	struct stringset	strings[ETH_SS_COUNT];
	struct perdev_strings	*next;
};

/* universal string sets */
static struct stringset global_strings[ETH_SS_COUNT];
/* linked list of string sets related to network devices */
static struct perdev_strings *device_strings;

static void drop_stringset(struct stringset *set)
{
	if (!set)
		return;

	free(set->strings);
	free(set->raw_data);
	memset(set, 0, sizeof(*set));
}

static int import_stringset(struct stringset *dest, const struct nlattr *nest)
{
	const struct nlattr *tb_stringset[ETHTOOL_A_STRINGSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb_stringset);
	const struct nlattr *string;
	unsigned int size;
	unsigned int count;
	unsigned int idx;
	int ret;

	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_stringset_info);
	if (ret < 0)
		return ret;
	if (!tb_stringset[ETHTOOL_A_STRINGSET_ID] ||
	    !tb_stringset[ETHTOOL_A_STRINGSET_COUNT] ||
	    !tb_stringset[ETHTOOL_A_STRINGSET_STRINGS])
		return -EFAULT;
	idx = mnl_attr_get_u32(tb_stringset[ETHTOOL_A_STRINGSET_ID]);
	if (idx >= ETH_SS_COUNT)
		return 0;
	if (dest[idx].loaded)
		drop_stringset(&dest[idx]);
	dest[idx].loaded = true;
	count = mnl_attr_get_u32(tb_stringset[ETHTOOL_A_STRINGSET_COUNT]);
	if (count == 0)
		return 0;

	size = mnl_attr_get_len(tb_stringset[ETHTOOL_A_STRINGSET_STRINGS]);
	ret = -ENOMEM;
	dest[idx].raw_data = malloc(size);
	if (!dest[idx].raw_data)
		goto err;
	memcpy(dest[idx].raw_data, tb_stringset[ETHTOOL_A_STRINGSET_STRINGS],
	       size);
	dest[idx].strings = calloc(count, sizeof(dest[idx].strings[0]));
	if (!dest[idx].strings)
		goto err;
	dest[idx].count = count;

	nest = dest[idx].raw_data;
	mnl_attr_for_each_nested(string, nest) {
		const struct nlattr *tb[ETHTOOL_A_STRING_MAX + 1] = {};
		DECLARE_ATTR_TB_INFO(tb);
		unsigned int i;

		if (mnl_attr_get_type(string) != ETHTOOL_A_STRINGS_STRING)
			continue;
		ret = mnl_attr_parse_nested(string, attr_cb, &tb_info);
		if (ret < 0)
			goto err;
		ret = -EFAULT;
		if (!tb[ETHTOOL_A_STRING_INDEX] || !tb[ETHTOOL_A_STRING_VALUE])
			goto err;

		i = mnl_attr_get_u32(tb[ETHTOOL_A_STRING_INDEX]);
		if (i >= count)
			goto err;
		dest[idx].strings[i] =
			mnl_attr_get_payload(tb[ETHTOOL_A_STRING_VALUE]);
	}

	return 0;
err:
	drop_stringset(&dest[idx]);
	return ret;
}

static struct perdev_strings *get_perdev_by_ifindex(int ifindex)
{
	struct perdev_strings *perdev = device_strings;

	while (perdev && perdev->ifindex != ifindex)
		perdev = perdev->next;
	if (perdev)
		return perdev;

	/* not found, allocate and insert into list */
	perdev = calloc(sizeof(*perdev), 1);
	if (!perdev)
		return NULL;
	perdev->ifindex = ifindex;
	perdev->next = device_strings;
	device_strings = perdev;

	return perdev;
}

static int strset_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_STRSET_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	char devname[ALTIFNAMSIZ] = "";
	struct stringset *dest;
	struct nlattr *attr;
	int ifindex = 0;
	int ret;

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return ret;
	if (tb[ETHTOOL_A_STRSET_HEADER]) {
		ret = get_dev_info(tb[ETHTOOL_A_STRSET_HEADER], &ifindex,
				   devname);
		if (ret < 0)
			return MNL_CB_OK;
		if (ifindex && nlctx->filter_devname &&
		    strncmp(devname, nlctx->filter_devname, ALTIFNAMSIZ))
			return MNL_CB_OK;
	}

	if (ifindex) {
		struct perdev_strings *perdev;

		perdev = get_perdev_by_ifindex(ifindex);
		if (!perdev)
			return MNL_CB_OK;
		copy_devname(perdev->devname, devname);
		dest = perdev->strings;
	} else {
		dest = global_strings;
	}

	if (!tb[ETHTOOL_A_STRSET_STRINGSETS])
		return MNL_CB_OK;
	mnl_attr_for_each_nested(attr, tb[ETHTOOL_A_STRSET_STRINGSETS]) {
		if (mnl_attr_get_type(attr) ==
		    ETHTOOL_A_STRINGSETS_STRINGSET)
			import_stringset(dest, attr);
	}

	return MNL_CB_OK;
}

static int fill_stringset_id(struct nl_msg_buff *msgbuff, unsigned int type)
{
	struct nlattr *nest_sets;
	struct nlattr *nest_set;

	nest_sets = ethnla_nest_start(msgbuff, ETHTOOL_A_STRSET_STRINGSETS);
	if (!nest_sets)
		return -EMSGSIZE;
	nest_set = ethnla_nest_start(msgbuff, ETHTOOL_A_STRINGSETS_STRINGSET);
	if (!nest_set)
		goto err;
	if (ethnla_put_u32(msgbuff, ETHTOOL_A_STRINGSET_ID, type))
		goto err;
	ethnla_nest_end(msgbuff, nest_set);
	ethnla_nest_end(msgbuff, nest_sets);
	return 0;

err:
	ethnla_nest_cancel(msgbuff, nest_sets);
	return -EMSGSIZE;
}

static int stringset_load_request(struct nl_socket *nlsk, const char *devname,
				  int type, bool is_dump)
{
	struct nl_msg_buff *msgbuff = &nlsk->msgbuff;
	int ret;

	ret = msg_init(nlsk->nlctx, msgbuff, ETHTOOL_MSG_STRSET_GET,
		       NLM_F_REQUEST | NLM_F_ACK | (is_dump ? NLM_F_DUMP : 0));
	if (ret < 0)
		return ret;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_STRSET_HEADER, devname, 0))
		return -EMSGSIZE;
	if (type >= 0) {
		ret = fill_stringset_id(msgbuff, type);
		if (ret < 0)
			return ret;
	}

	ret = nlsock_send_get_request(nlsk, strset_reply_cb);
	return ret;
}

/* interface */

const struct stringset *global_stringset(unsigned int type,
					 struct nl_socket *nlsk)
{
	int ret;

	if (type >= ETH_SS_COUNT)
		return NULL;
	if (global_strings[type].loaded)
		return &global_strings[type];
	ret = stringset_load_request(nlsk, NULL, type, false);
	return ret < 0 ? NULL : &global_strings[type];
}

const struct stringset *perdev_stringset(const char *devname, unsigned int type,
					 struct nl_socket *nlsk)
{
	const struct perdev_strings *p;
	int ret;

	if (type >= ETH_SS_COUNT)
		return NULL;
	for (p = device_strings; p; p = p->next)
		if (!strcmp(p->devname, devname))
			return &p->strings[type];

	ret = stringset_load_request(nlsk, devname, type, false);
	if (ret < 0)
		return NULL;
	for (p = device_strings; p; p = p->next)
		if (!strcmp(p->devname, devname))
			return &p->strings[type];

	return NULL;
}

unsigned int get_count(const struct stringset *set)
{
	return set->count;
}

const char *get_string(const struct stringset *set, unsigned int idx)
{
	if (!set || idx >= set->count)
		return NULL;
	return set->strings[idx];
}

int preload_global_strings(struct nl_socket *nlsk)
{
	return stringset_load_request(nlsk, NULL, -1, false);
}

int preload_perdev_strings(struct nl_socket *nlsk, const char *dev)
{
	return stringset_load_request(nlsk, dev, -1, !dev);
}

void cleanup_all_strings(void)
{
	struct perdev_strings *perdev;
	unsigned int i;

	for (i = 0; i < ETH_SS_COUNT; i++)
		drop_stringset(&global_strings[i]);

	perdev = device_strings;
	while (perdev) {
		device_strings = perdev->next;
		for (i = 0; i < ETH_SS_COUNT; i++)
			drop_stringset(&perdev->strings[i]);
		free(perdev);
		perdev = device_strings;
	}
}
