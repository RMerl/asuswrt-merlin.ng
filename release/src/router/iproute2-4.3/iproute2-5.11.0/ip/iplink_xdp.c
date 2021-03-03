/*
 * iplink_xdp.c XDP program loader
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:     Daniel Borkmann <daniel@iogearbox.net>
 */

#include <stdio.h>
#include <stdlib.h>

#include <linux/bpf.h>

#include "bpf_util.h"
#include "utils.h"
#include "ip_common.h"

extern int force;

struct xdp_req {
	struct iplink_req *req;
	__u32 flags;
};

static void xdp_ebpf_cb(void *raw, int fd, const char *annotation)
{
	struct xdp_req *xdp = raw;
	struct iplink_req *req = xdp->req;
	struct rtattr *xdp_attr;

	xdp_attr = addattr_nest(&req->n, sizeof(*req), IFLA_XDP);
	addattr32(&req->n, sizeof(*req), IFLA_XDP_FD, fd);
	if (xdp->flags)
		addattr32(&req->n, sizeof(*req), IFLA_XDP_FLAGS, xdp->flags);
	addattr_nest_end(&req->n, xdp_attr);
}

static const struct bpf_cfg_ops bpf_cb_ops = {
	.ebpf_cb = xdp_ebpf_cb,
};

static int xdp_delete(struct xdp_req *xdp)
{
	xdp_ebpf_cb(xdp, -1, NULL);
	return 0;
}

int xdp_parse(int *argc, char ***argv, struct iplink_req *req,
	      const char *ifname, bool generic, bool drv, bool offload)
{
	struct bpf_cfg_in cfg = {
		.type = BPF_PROG_TYPE_XDP,
		.argc = *argc,
		.argv = *argv,
	};
	struct xdp_req xdp = {
		.req = req,
	};

	if (offload) {
		int ifindex = ll_name_to_index(ifname);

		if (!ifindex)
			incomplete_command();
		cfg.ifindex = ifindex;
	}

	if (!force)
		xdp.flags |= XDP_FLAGS_UPDATE_IF_NOEXIST;
	if (generic)
		xdp.flags |= XDP_FLAGS_SKB_MODE;
	if (drv)
		xdp.flags |= XDP_FLAGS_DRV_MODE;
	if (offload)
		xdp.flags |= XDP_FLAGS_HW_MODE;

	if (*argc == 1) {
		if (strcmp(**argv, "none") == 0 ||
		    strcmp(**argv, "off") == 0)
			return xdp_delete(&xdp);
	}

	if (bpf_parse_and_load_common(&cfg, &bpf_cb_ops, &xdp))
		return -1;

	*argc = cfg.argc;
	*argv = cfg.argv;
	return 0;
}

static void xdp_dump_json_one(struct rtattr *tb[IFLA_XDP_MAX + 1], __u32 attr,
			      __u8 mode)
{
	if (!tb[attr])
		return;

	open_json_object(NULL);
	print_uint(PRINT_JSON, "mode", NULL, mode);
	bpf_dump_prog_info(NULL, rta_getattr_u32(tb[attr]));
	close_json_object();
}

static void xdp_dump_json(struct rtattr *tb[IFLA_XDP_MAX + 1])
{
	__u32 prog_id = 0;
	__u8 mode;

	mode = rta_getattr_u8(tb[IFLA_XDP_ATTACHED]);
	if (tb[IFLA_XDP_PROG_ID])
		prog_id = rta_getattr_u32(tb[IFLA_XDP_PROG_ID]);

	open_json_object("xdp");
	print_uint(PRINT_JSON, "mode", NULL, mode);
	if (prog_id)
		bpf_dump_prog_info(NULL, prog_id);

	open_json_array(PRINT_JSON, "attached");
	if (tb[IFLA_XDP_SKB_PROG_ID] ||
	    tb[IFLA_XDP_DRV_PROG_ID] ||
	    tb[IFLA_XDP_HW_PROG_ID]) {
		xdp_dump_json_one(tb, IFLA_XDP_SKB_PROG_ID, XDP_ATTACHED_SKB);
		xdp_dump_json_one(tb, IFLA_XDP_DRV_PROG_ID, XDP_ATTACHED_DRV);
		xdp_dump_json_one(tb, IFLA_XDP_HW_PROG_ID, XDP_ATTACHED_HW);
	} else if (tb[IFLA_XDP_PROG_ID]) {
		/* Older kernel - use IFLA_XDP_PROG_ID */
		xdp_dump_json_one(tb, IFLA_XDP_PROG_ID, mode);
	}
	close_json_array(PRINT_JSON, NULL);

	close_json_object();
}

static void xdp_dump_prog_one(FILE *fp, struct rtattr *tb[IFLA_XDP_MAX + 1],
			      __u32 attr, bool link, bool details,
			      const char *pfx)
{
	__u32 prog_id;

	if (!tb[attr])
		return;

	prog_id = rta_getattr_u32(tb[attr]);
	if (!details) {
		if (prog_id && !link && attr == IFLA_XDP_PROG_ID)
			fprintf(fp, "/id:%u", prog_id);
		return;
	}

	if (prog_id) {
		fprintf(fp, "%s    prog/xdp%s ", _SL_, pfx);
		bpf_dump_prog_info(fp, prog_id);
	}
}

void xdp_dump(FILE *fp, struct rtattr *xdp, bool link, bool details)
{
	struct rtattr *tb[IFLA_XDP_MAX + 1];
	__u8 mode;

	parse_rtattr_nested(tb, IFLA_XDP_MAX, xdp);

	if (!tb[IFLA_XDP_ATTACHED])
		return;

	mode = rta_getattr_u8(tb[IFLA_XDP_ATTACHED]);
	if (mode == XDP_ATTACHED_NONE)
		return;
	else if (is_json_context())
		return details ? (void)0 : xdp_dump_json(tb);
	else if (details && link)
		/* don't print mode */;
	else if (mode == XDP_ATTACHED_DRV)
		fprintf(fp, "xdp");
	else if (mode == XDP_ATTACHED_SKB)
		fprintf(fp, "xdpgeneric");
	else if (mode == XDP_ATTACHED_HW)
		fprintf(fp, "xdpoffload");
	else if (mode == XDP_ATTACHED_MULTI)
		fprintf(fp, "xdpmulti");
	else
		fprintf(fp, "xdp[%u]", mode);

	xdp_dump_prog_one(fp, tb, IFLA_XDP_PROG_ID, link, details, "");

	if (mode == XDP_ATTACHED_MULTI) {
		xdp_dump_prog_one(fp, tb, IFLA_XDP_SKB_PROG_ID, link, details,
				  "generic");
		xdp_dump_prog_one(fp, tb, IFLA_XDP_DRV_PROG_ID, link, details,
				  "drv");
		xdp_dump_prog_one(fp, tb, IFLA_XDP_HW_PROG_ID, link, details,
				  "offload");
	}

	if (!details || !link)
		fprintf(fp, " ");
}
