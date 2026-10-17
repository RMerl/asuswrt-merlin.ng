// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <log.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <dm/ofnode.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

static bool check_owned_resource(sc_rsrc_t rsrc_id)
{
	bool owned;

	owned = sc_rm_is_resource_owned(-1, rsrc_id);

	return owned;
}

static int disable_fdt_node(void *blob, int nodeoffset)
{
	int rc, ret;
	const char *status = "disabled";

	do {
		rc = fdt_setprop(blob, nodeoffset, "status", status,
				 strlen(status) + 1);
		if (rc) {
			if (rc == -FDT_ERR_NOSPACE) {
				ret = fdt_increase_size(blob, 512);
				if (ret)
					return ret;
			}
		}
	} while (rc == -FDT_ERR_NOSPACE);

	return rc;
}

static void update_fdt_with_owned_resources(void *blob)
{
	/*
	 * Traverses the fdt nodes, check its power domain and use
	 * the resource id in the power domain for checking whether
	 * it is owned by current partition
	 */
	struct fdtdec_phandle_args args;
	int offset = 0, depth = 0;
	u32 rsrc_id;
	int rc, i;

	for (offset = fdt_next_node(blob, offset, &depth); offset > 0;
	     offset = fdt_next_node(blob, offset, &depth)) {
		debug("Node name: %s, depth %d\n",
		      fdt_get_name(blob, offset, NULL), depth);

		if (!fdt_get_property(blob, offset, "power-domains", NULL)) {
			debug("   - ignoring node %s\n",
			      fdt_get_name(blob, offset, NULL));
			continue;
		}

		if (!fdtdec_get_is_enabled(blob, offset)) {
			debug("   - ignoring node %s\n",
			      fdt_get_name(blob, offset, NULL));
			continue;
		}

		i = 0;
		while (true) {
			rc = fdtdec_parse_phandle_with_args(blob, offset,
							    "power-domains",
							    "#power-domain-cells",
							    0, i++, &args);
			if (rc == -ENOENT) {
				break;
			} else if (rc) {
				printf("Parse power-domains of %s wrong: %d\n",
				       fdt_get_name(blob, offset, NULL), rc);
				continue;
			}

			rsrc_id = args.args[0];

			if (!check_owned_resource(rsrc_id)) {
				rc = disable_fdt_node(blob, offset);
				if (!rc) {
					printf("Disable %s rsrc %u not owned\n",
					       fdt_get_name(blob, offset, NULL),
					       rsrc_id);
				} else {
					printf("Unable to disable %s, err=%s\n",
					       fdt_get_name(blob, offset, NULL),
					       fdt_strerror(rc));
				}
			}
		}
	}
}

static int config_smmu_resource_sid(int rsrc, int sid)
{
	int err;

	err = sc_rm_set_master_sid(-1, rsrc, sid);
	debug("set_master_sid rsrc=%d sid=0x%x err=%d\n", rsrc, sid, err);
	if (err) {
		if (!check_owned_resource(rsrc)) {
			printf("%s rsrc[%d] not owned\n", __func__, rsrc);
			return -1;
		}
		pr_err("fail set_master_sid rsrc=%d sid=0x%x err=%d\n", rsrc, sid, err);
		return -EINVAL;
	}

	return 0;
}

static int config_smmu_fdt_device_sid(void *blob, int device_offset, int sid)
{
	const char *name = fdt_get_name(blob, device_offset, NULL);
	struct fdtdec_phandle_args args;
	int rsrc, ret;
	int proplen;
	const fdt32_t *prop;
	int i;

	prop = fdt_getprop(blob, device_offset, "fsl,sc_rsrc_id", &proplen);
	if (prop) {
		int i;

		debug("configure node %s sid 0x%x for %d resources\n",
		      name, sid, (int)(proplen / sizeof(fdt32_t)));
		for (i = 0; i < proplen / sizeof(fdt32_t); ++i) {
			ret = config_smmu_resource_sid(fdt32_to_cpu(prop[i]),
						       sid);
			if (ret)
				return ret;
		}

		return 0;
	}

	i = 0;
	while (true) {
		ret = fdtdec_parse_phandle_with_args(blob, device_offset,
						     "power-domains",
						     "#power-domain-cells",
						     0, i++, &args);
		if (ret == -ENOENT) {
			break;
		} else if (ret) {
			printf("Parse power-domains of node %s wrong: %d\n",
			       fdt_get_name(blob, device_offset, NULL), ret);
			continue;
		}

		debug("configure node %s sid 0x%x rsrc=%d\n",
		      name, sid, rsrc);
		rsrc = args.args[0];

		ret = config_smmu_resource_sid(rsrc, sid);
		if (ret)
			break;
	}

	return ret;
}

static int config_smmu_fdt(void *blob)
{
	int offset, proplen, i, ret;
	const fdt32_t *prop;
	const char *name;

	/* Legacy smmu bindings, still used by xen. */
	offset = fdt_node_offset_by_compatible(blob, 0, "arm,mmu-500");
	prop = fdt_getprop(blob, offset, "mmu-masters", &proplen);
	if (offset > 0 && prop) {
		debug("found legacy mmu-masters property\n");

		for (i = 0; i < proplen / 8; ++i) {
			u32 phandle = fdt32_to_cpu(prop[2 * i]);
			int sid = fdt32_to_cpu(prop[2 * i + 1]);
			int device_offset;

			device_offset = fdt_node_offset_by_phandle(blob,
								   phandle);
			if (device_offset < 0) {
				pr_err("Not find device from mmu_masters: %d",
				       device_offset);
				continue;
			}
			ret = config_smmu_fdt_device_sid(blob, device_offset,
							 sid);
			if (ret)
				return ret;
		}

		/* Ignore new bindings if old bindings found, just like linux. */
		return 0;
	}

	/* Generic smmu bindings */
	offset = 0;
	while ((offset = fdt_next_node(blob, offset, NULL)) > 0) {
		name = fdt_get_name(blob, offset, NULL);
		prop = fdt_getprop(blob, offset, "iommus", &proplen);
		if (!prop)
			continue;
		debug("node %s iommus proplen %d\n", name, proplen);

		if (proplen == 12) {
			int sid = fdt32_to_cpu(prop[1]);

			config_smmu_fdt_device_sid(blob, offset, sid);
		} else if (proplen != 4) {
			debug("node %s ignore unexpected iommus proplen=%d\n",
			      name, proplen);
		}
	}

	return 0;
}

static int ft_add_optee_node(void *fdt, struct bd_info *bd)
{
	const char *path, *subpath;
	int offs;

	/*
	 * No TEE space allocated indicating no TEE running, so no
	 * need to add optee node in dts
	 */
	if (!boot_pointer[1])
		return 0;

	offs = fdt_increase_size(fdt, 512);
	if (offs) {
		printf("No Space for dtb\n");
		return 1;
	}

	path = "/firmware";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		path = "/";
		offs = fdt_path_offset(fdt, path);

		if (offs < 0) {
			printf("Could not find root node.\n");
			return offs;
		}

		subpath = "firmware";
		offs = fdt_add_subnode(fdt, offs, subpath);
		if (offs < 0) {
			printf("Could not create %s node.\n", subpath);
			return offs;
		}
	}

	subpath = "optee";
	offs = fdt_add_subnode(fdt, offs, subpath);
	if (offs < 0) {
		printf("Could not create %s node.\n", subpath);
		return offs;
	}

	fdt_setprop_string(fdt, offs, "compatible", "linaro,optee-tz");
	fdt_setprop_string(fdt, offs, "method", "smc");

	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;
	int off;

	if (CONFIG_BOOTAUX_RESERVED_MEM_BASE) {
		off = fdt_add_mem_rsv(blob, CONFIG_BOOTAUX_RESERVED_MEM_BASE,
				      CONFIG_BOOTAUX_RESERVED_MEM_SIZE);
		if (off < 0)
			printf("Failed	to reserve memory for bootaux: %s\n",
			       fdt_strerror(off));
	}

	update_fdt_with_owned_resources(blob);

	if (is_imx8qm()) {
		ret = config_smmu_fdt(blob);
		if (ret)
			return ret;
	}

	return ft_add_optee_node(blob, bd);
}
