// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) OS request (device tree fixup) functions
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_BOOT

#include <common.h>
#include <dm.h>
#include <event.h>
#include <image.h>
#include <malloc.h>
#include <rng.h>
#include <dm/ofnode.h>

#define VBE_PREFIX		"vbe,"
#define VBE_PREFIX_LEN		(sizeof(VBE_PREFIX) - 1)
#define VBE_ERR_STR_LEN		128
#define VBE_MAX_RAND_SIZE	256

struct vbe_result {
	int errnum;
	char err_str[VBE_ERR_STR_LEN];
};

typedef int (*vbe_req_func)(ofnode node, struct vbe_result *result);

static int handle_random_req(ofnode node, int default_size,
			     struct vbe_result *result)
{
	char buf[VBE_MAX_RAND_SIZE];
	struct udevice *dev;
	u32 size;
	int ret;

	if (!IS_ENABLED(CONFIG_DM_RNG))
		return -ENOTSUPP;

	if (ofnode_read_u32(node, "vbe,size", &size)) {
		if (!default_size) {
			snprintf(result->err_str, VBE_ERR_STR_LEN,
				 "Missing vbe,size property");
			return log_msg_ret("byt", -EINVAL);
		}
		size = default_size;
	}
	if (size > VBE_MAX_RAND_SIZE) {
		snprintf(result->err_str, VBE_ERR_STR_LEN,
			 "vbe,size %#x exceeds max size %#x", size,
			 VBE_MAX_RAND_SIZE);
		return log_msg_ret("siz", -E2BIG);
	}
	ret = uclass_first_device_err(UCLASS_RNG, &dev);
	if (ret) {
		snprintf(result->err_str, VBE_ERR_STR_LEN,
			 "Cannot find random-number device (err=%d)", ret);
		return log_msg_ret("wr", ret);
	}
	ret = dm_rng_read(dev, buf, size);
	if (ret) {
		snprintf(result->err_str, VBE_ERR_STR_LEN,
			 "Failed to read random-number device (err=%d)", ret);
		return log_msg_ret("rd", ret);
	}
	ret = ofnode_write_prop(node, "data", buf, size, true);
	if (ret)
		return log_msg_ret("wr", -EINVAL);

	return 0;
}

static int vbe_req_random_seed(ofnode node, struct vbe_result *result)
{
	return handle_random_req(node, 0, result);
}

static int vbe_req_aslr_move(ofnode node, struct vbe_result *result)
{
	return -ENOTSUPP;
}

static int vbe_req_aslr_rand(ofnode node, struct vbe_result *result)
{
	return handle_random_req(node, 4, result);
}

static int vbe_req_efi_runtime_rand(ofnode node, struct vbe_result *result)
{
	return handle_random_req(node, 4, result);
}

static struct vbe_req {
	const char *compat;
	vbe_req_func func;
} vbe_reqs[] = {
	/* address space layout randomization - move the OS in memory */
	{ "aslr-move", vbe_req_aslr_move },

	/* provide random data for address space layout randomization */
	{ "aslr-rand", vbe_req_aslr_rand },

	/* provide random data for EFI-runtime-services address */
	{ "efi-runtime-rand", vbe_req_efi_runtime_rand },

	/* generate random data bytes to see the OS's rand generator */
	{ "random-rand", vbe_req_random_seed },

};

static int vbe_process_request(ofnode node, struct vbe_result *result)
{
	const char *compat, *req_name;
	int i;

	compat = ofnode_read_string(node, "compatible");
	if (!compat)
		return 0;

	if (strlen(compat) <= VBE_PREFIX_LEN ||
	    strncmp(compat, VBE_PREFIX, VBE_PREFIX_LEN))
		return -EINVAL;

	req_name = compat + VBE_PREFIX_LEN; /* drop "vbe," prefix */
	for (i = 0; i < ARRAY_SIZE(vbe_reqs); i++) {
		if (!strcmp(vbe_reqs[i].compat, req_name)) {
			int ret;

			ret = vbe_reqs[i].func(node, result);
			if (ret)
				return log_msg_ret("req", ret);
			return 0;
		}
	}
	snprintf(result->err_str, VBE_ERR_STR_LEN, "Unknown request: %s",
		 req_name);

	return -ENOTSUPP;
}

/**
 * bootmeth_vbe_ft_fixup() - Process VBE OS requests and do device tree fixups
 *
 * If there are no images provided, this does nothing and returns 0.
 *
 * @ctx: Context for event
 * @event: Event to process
 * @return 0 if OK, -ve on error
 */
static int bootmeth_vbe_ft_fixup(void *ctx, struct event *event)
{
	const struct event_ft_fixup *fixup = &event->data.ft_fixup;
	const struct bootm_headers *images = fixup->images;
	ofnode parent, dest_parent, root, node;
	oftree fit;

	if (!images || !images->fit_hdr_os)
		return 0;

	/* Get the image node with requests in it */
	log_debug("fit=%p, noffset=%d\n", images->fit_hdr_os,
		  images->fit_noffset_os);
	fit = oftree_from_fdt(images->fit_hdr_os);
	root = oftree_root(fit);
	if (of_live_active()) {
		log_warning("Cannot fix up live tree\n");
		return 0;
	}
	if (!ofnode_valid(root))
		return log_msg_ret("rt", -EINVAL);
	parent = noffset_to_ofnode(root, images->fit_noffset_os);
	if (!ofnode_valid(parent))
		return log_msg_ret("img", -EINVAL);
	dest_parent = oftree_path(fixup->tree, "/chosen");
	if (!ofnode_valid(dest_parent))
		return log_msg_ret("dst", -EINVAL);

	ofnode_for_each_subnode(node, parent) {
		const char *name = ofnode_get_name(node);
		struct vbe_result result;
		ofnode dest;
		int ret;

		log_debug("copy subnode: %s\n", name);
		ret = ofnode_add_subnode(dest_parent, name, &dest);
		if (ret && ret != -EEXIST)
			return log_msg_ret("add", ret);
		ret = ofnode_copy_props(dest, node);
		if (ret)
			return log_msg_ret("cp", ret);

		*result.err_str = '\0';
		ret = vbe_process_request(dest, &result);
		if (ret) {
			result.errnum = ret;
			log_warning("Failed to process VBE request %s (err=%d)\n",
				    ofnode_get_name(dest), ret);
			if (*result.err_str) {
				char *msg = strdup(result.err_str);

				if (!msg)
					return log_msg_ret("msg", -ENOMEM);
				ret = ofnode_write_string(dest, "vbe,error",
							  msg);
				if (ret) {
					free(msg);
					return log_msg_ret("str", -ENOMEM);
				}
			}
			if (result.errnum) {
				ret = ofnode_write_u32(dest, "vbe,errnum",
						       result.errnum);
				if (ret)
					return log_msg_ret("num", -ENOMEM);
				if (result.errnum != -ENOTSUPP)
					return log_msg_ret("pro",
							   result.errnum);
				if (result.errnum == -ENOTSUPP &&
				    ofnode_read_bool(dest, "vbe,required")) {
					log_err("Cannot handle required request: %s\n",
						ofnode_get_name(dest));
					return log_msg_ret("req",
							   result.errnum);
				}
			}
		}
	}

	return 0;
}
EVENT_SPY_FULL(EVT_FT_FIXUP, bootmeth_vbe_ft_fixup);
