/*
 * module-eeprom.c - netlink implementation of module eeprom get command
 *
 * ethtool -m <dev>
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "../sff-common.h"
#include "../qsfp.h"
#include "../cmis.h"
#include "../internal.h"
#include "../common.h"
#include "../list.h"
#include "netlink.h"
#include "parser.h"

#define ETH_I2C_ADDRESS_LOW	0x50
#define ETH_I2C_MAX_ADDRESS	0x7F

struct cmd_params {
	u8 dump_hex;
	u8 dump_raw;
	u32 offset;
	u32 length;
	u32 page;
	u32 bank;
	u32 i2c_address;
};

static const struct param_parser getmodule_params[] = {
	{
		.arg		= "hex",
		.handler	= nl_parse_u8bool,
		.dest_offset	= offsetof(struct cmd_params, dump_hex),
		.min_argc	= 1,
	},
	{
		.arg		= "raw",
		.handler	= nl_parse_u8bool,
		.dest_offset	= offsetof(struct cmd_params, dump_raw),
		.min_argc	= 1,
	},
	{
		.arg		= "offset",
		.handler	= nl_parse_direct_u32,
		.dest_offset	= offsetof(struct cmd_params, offset),
		.min_argc	= 1,
	},
	{
		.arg		= "length",
		.handler	= nl_parse_direct_u32,
		.dest_offset	= offsetof(struct cmd_params, length),
		.min_argc	= 1,
	},
	{
		.arg		= "page",
		.handler	= nl_parse_direct_u32,
		.dest_offset	= offsetof(struct cmd_params, page),
		.min_argc	= 1,
	},
	{
		.arg		= "bank",
		.handler	= nl_parse_direct_u32,
		.dest_offset	= offsetof(struct cmd_params, bank),
		.min_argc	= 1,
	},
	{
		.arg		= "i2c",
		.handler	= nl_parse_direct_u32,
		.dest_offset	= offsetof(struct cmd_params, i2c_address),
		.min_argc	= 1,
	},
	{}
};

static struct list_head eeprom_page_list = LIST_HEAD_INIT(eeprom_page_list);

struct eeprom_page_entry {
	struct list_head list;	/* Member of eeprom_page_list */
	void *data;
};

static int eeprom_page_list_add(void *data)
{
	struct eeprom_page_entry *entry;

	entry = malloc(sizeof(*entry));
	if (!entry)
		return -ENOMEM;

	entry->data = data;
	list_add(&entry->list, &eeprom_page_list);

	return 0;
}

static void eeprom_page_list_flush(void)
{
	struct eeprom_page_entry *entry;
	struct list_head *head, *next;

	list_for_each_safe(head, next, &eeprom_page_list) {
		entry = (struct eeprom_page_entry *) head;
		free(entry->data);
		list_del(head);
		free(entry);
	}
}

static int get_eeprom_page_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_MODULE_EEPROM_DATA + 1] = {};
	struct ethtool_module_eeprom *request = data;
	DECLARE_ATTR_TB_INFO(tb);
	u8 *eeprom_data;
	int ret;

	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return ret;

	if (!tb[ETHTOOL_A_MODULE_EEPROM_DATA])
		return MNL_CB_ERROR;

	eeprom_data = mnl_attr_get_payload(tb[ETHTOOL_A_MODULE_EEPROM_DATA]);
	request->data = malloc(request->length);
	if (!request->data)
		return MNL_CB_ERROR;
	memcpy(request->data, eeprom_data, request->length);

	ret = eeprom_page_list_add(request->data);
	if (ret < 0)
		goto err_list_add;

	return MNL_CB_OK;

err_list_add:
	free(request->data);
	return MNL_CB_ERROR;
}

int nl_get_eeprom_page(struct cmd_context *ctx,
		       struct ethtool_module_eeprom *request)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsock;
	struct nl_msg_buff *msg;
	int ret;

	if (!request || request->i2c_address > ETH_I2C_MAX_ADDRESS)
		return -EINVAL;

	nlsock = nlctx->ethnl_socket;
	msg = &nlsock->msgbuff;

	ret = nlsock_prep_get_request(nlsock, ETHTOOL_MSG_MODULE_EEPROM_GET,
				      ETHTOOL_A_MODULE_EEPROM_HEADER, 0);
	if (ret < 0)
		return ret;

	if (ethnla_put_u32(msg, ETHTOOL_A_MODULE_EEPROM_LENGTH,
			   request->length) ||
	    ethnla_put_u32(msg, ETHTOOL_A_MODULE_EEPROM_OFFSET,
			   request->offset) ||
	    ethnla_put_u8(msg, ETHTOOL_A_MODULE_EEPROM_PAGE,
			  request->page) ||
	    ethnla_put_u8(msg, ETHTOOL_A_MODULE_EEPROM_BANK,
			  request->bank) ||
	    ethnla_put_u8(msg, ETHTOOL_A_MODULE_EEPROM_I2C_ADDRESS,
			  request->i2c_address))
		return -EMSGSIZE;

	ret = nlsock_sendmsg(nlsock, NULL);
	if (ret < 0)
		return ret;
	return nlsock_process_reply(nlsock, get_eeprom_page_reply_cb,
				    (void *)request);
}

static int eeprom_dump_hex(struct cmd_context *ctx)
{
	struct ethtool_module_eeprom request = {
		.length = 128,
		.i2c_address = ETH_I2C_ADDRESS_LOW,
	};
	int ret;

	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;

	dump_hex(stdout, request.data, request.length, request.offset);

	return 0;
}

static int eeprom_parse(struct cmd_context *ctx)
{
	struct ethtool_module_eeprom request = {
		.length = 1,
		.i2c_address = ETH_I2C_ADDRESS_LOW,
	};
	int ret;

	/* Fetch the SFF-8024 Identifier Value. For all supported standards, it
	 * is located at I2C address 0x50, byte 0. See section 4.1 in SFF-8024,
	 * revision 4.9.
	 */
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;

	switch (request.data[0]) {
#ifdef ETHTOOL_ENABLE_PRETTY_DUMP
	case SFF8024_ID_SFP:
		return sff8079_show_all_nl(ctx);
	case SFF8024_ID_QSFP:
	case SFF8024_ID_QSFP28:
	case SFF8024_ID_QSFP_PLUS:
		return sff8636_show_all_nl(ctx);
	case SFF8024_ID_QSFP_DD:
	case SFF8024_ID_OSFP:
	case SFF8024_ID_DSFP:
		return cmis_show_all_nl(ctx);
#endif
	default:
		/* If we cannot recognize the memory map, default to dumping
		 * the first 128 bytes in hex.
		 */
		return eeprom_dump_hex(ctx);
	}
}

int nl_getmodule(struct cmd_context *ctx)
{
	struct cmd_params getmodule_cmd_params = {};
	struct ethtool_module_eeprom request = {0};
	struct nl_context *nlctx = ctx->nlctx;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_MODULE_EEPROM_GET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-m";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	ret = nl_parser(nlctx, getmodule_params, &getmodule_cmd_params, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return ret;

	if (getmodule_cmd_params.dump_hex && getmodule_cmd_params.dump_raw) {
		fprintf(stderr, "Hex and raw dump cannot be specified together\n");
		return -EINVAL;
	}

	/* When complete hex/raw dump of the EEPROM is requested, fallback to
	 * ioctl. Netlink can only request specific pages.
	 */
	if ((getmodule_cmd_params.dump_hex || getmodule_cmd_params.dump_raw) &&
	    !getmodule_cmd_params.page && !getmodule_cmd_params.bank &&
	    !getmodule_cmd_params.i2c_address) {
		nlctx->ioctl_fallback = true;
		return -EOPNOTSUPP;
	}

#ifdef ETHTOOL_ENABLE_PRETTY_DUMP
	if (getmodule_cmd_params.page || getmodule_cmd_params.bank ||
	    getmodule_cmd_params.offset || getmodule_cmd_params.length)
#endif
		getmodule_cmd_params.dump_hex = true;

	request.offset = getmodule_cmd_params.offset;
	request.length = getmodule_cmd_params.length ?: 128;
	request.page = getmodule_cmd_params.page;
	request.bank = getmodule_cmd_params.bank;
	request.i2c_address = getmodule_cmd_params.i2c_address ?: ETH_I2C_ADDRESS_LOW;

	if (request.page && !request.offset)
		request.offset = 128;

	if (getmodule_cmd_params.dump_hex || getmodule_cmd_params.dump_raw) {
		ret = nl_get_eeprom_page(ctx, &request);
		if (ret < 0)
			goto cleanup;

		if (getmodule_cmd_params.dump_raw)
			fwrite(request.data, 1, request.length, stdout);
		else
			dump_hex(stdout, request.data, request.length,
				 request.offset);
	} else {
		ret = eeprom_parse(ctx);
		if (ret < 0)
			goto cleanup;
	}

cleanup:
	eeprom_page_list_flush();
	return ret;
}
