// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Linaro
 * Bryan O'Donoghue <bryan.odonoghue@linaro.org>
 */

#include <fdtdec.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <linux/libfdt.h>
#include <tee/optee.h>

#define optee_hdr_err_msg \
	"OPTEE verification error:" \
	"\n\thdr=%p image=0x%08lx magic=0x%08x" \
	"\n\theader lo=0x%08x hi=0x%08x size=0x%08lx arch=0x%08x" \
	"\n\tuimage params 0x%08lx-0x%08lx\n"

#if defined(CONFIG_OPTEE_IMAGE)
static int optee_verify_image(struct optee_header *hdr, unsigned long image_len)
{
	uint32_t tee_file_size;

	tee_file_size = hdr->init_size + hdr->paged_size +
			sizeof(struct optee_header);

	if (hdr->magic != OPTEE_MAGIC ||
	    hdr->version != OPTEE_VERSION ||
	    tee_file_size != image_len) {
		return -EINVAL;
	}

	return 0;
}

int optee_verify_bootm_image(unsigned long image_addr,
			     unsigned long image_load_addr,
			     unsigned long image_len)
{
	struct optee_header *hdr = (struct optee_header *)image_addr;
	int ret;

	ret = optee_verify_image(hdr, image_len);
	if (ret)
		goto error;

	if (image_load_addr + sizeof(*hdr) != hdr->init_load_addr_lo) {
		ret = -EINVAL;
		goto error;
	}

	return ret;
error:
	printf(optee_hdr_err_msg, hdr, image_addr, hdr->magic,
	       hdr->init_load_addr_lo,
	       hdr->init_load_addr_hi, image_len, hdr->arch, image_load_addr,
	       image_load_addr + image_len);

	return ret;
}
#endif

#if defined(CONFIG_OF_LIBFDT)
static int optee_copy_firmware_node(ofnode node, void *fdt_blob)
{
	int offs, ret, len;
	const void *prop;

	offs = fdt_path_offset(fdt_blob, "/firmware");
	if (offs < 0) {
		offs = fdt_path_offset(fdt_blob, "/");
		if (offs < 0)
			return offs;

		offs = fdt_add_subnode(fdt_blob, offs, "firmware");
		if (offs < 0)
			return offs;
	}

	offs = fdt_add_subnode(fdt_blob, offs, "optee");
	if (offs < 0)
		return offs;

	/* copy the compatible property */
	prop = ofnode_get_property(node, "compatible", &len);
	if (!prop) {
		debug("missing OP-TEE compatible property");
		return -EINVAL;
	}

	ret = fdt_setprop(fdt_blob, offs, "compatible", prop, len);
	if (ret < 0)
		return ret;

	/* copy the method property */
	prop = ofnode_get_property(node, "method", &len);
	if (!prop) {
		debug("missing OP-TEE method property");
		return -EINVAL;
	}

	ret = fdt_setprop(fdt_blob, offs, "method", prop, len);
	if (ret < 0)
		return ret;

	return 0;
}

int optee_copy_fdt_nodes(void *new_blob)
{
	ofnode node, subnode;
	int ret;
	struct resource res;

	if (fdt_check_header(new_blob))
		return -EINVAL;

	/* only proceed if there is an /firmware/optee node */
	node = ofnode_path("/firmware/optee");
	if (!ofnode_valid(node)) {
		debug("No OP-TEE firmware node in old fdt, nothing to do");
		return 0;
	}

	/*
	 * Do not proceed if the target dt already has an OP-TEE node.
	 * In this case assume that the system knows better somehow,
	 * so do not interfere.
	 */
	if (fdt_path_offset(new_blob, "/firmware/optee") >= 0) {
		debug("OP-TEE Device Tree node already exists in target");
		return 0;
	}

	ret = optee_copy_firmware_node(node, new_blob);
	if (ret < 0) {
		printf("Failed to add OP-TEE firmware node\n");
		return ret;
	}

	/* optee inserts its memory regions as reserved-memory nodes */
	node = ofnode_path("/reserved-memory");
	if (ofnode_valid(node)) {
		ofnode_for_each_subnode(subnode, node) {
			const char *name = ofnode_get_name(subnode);
			if (!name)
				return -EINVAL;

			/* only handle optee reservations */
			if (strncmp(name, "optee", 5))
				continue;

			/* check if this subnode has a reg property */
			ret = ofnode_read_resource(subnode, 0, &res);
			if (!ret) {
				struct fdt_memory carveout = {
					.start = res.start,
					.end = res.end,
				};
				unsigned long flags = FDTDEC_RESERVED_MEMORY_NO_MAP;
				char *oldname, *nodename, *tmp;

				oldname = strdup(name);
				if (!oldname)
					return -ENOMEM;

				tmp = oldname;
				nodename = strsep(&tmp, "@");
				if (!nodename) {
					free(oldname);
					return -EINVAL;
				}

				ret = fdtdec_add_reserved_memory(new_blob,
								 nodename,
								 &carveout,
								 NULL, 0,
								 NULL, flags);
				free(oldname);

				if (ret < 0)
					return ret;
			}
		}
	}

	return 0;
}
#endif
