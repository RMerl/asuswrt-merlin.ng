// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Zynq MPSoC Firmware driver
 *
 * Copyright (C) 2018-2019 Xilinx, Inc.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <log.h>
#include <zynqmp_firmware.h>
#include <asm/cache.h>
#include <asm/ptrace.h>

#if defined(CONFIG_ZYNQMP_IPI)
#include <mailbox.h>
#include <asm/arch/sys_proto.h>

#define PMUFW_PAYLOAD_ARG_CNT	8

#define XST_PM_NO_ACCESS	2002L
#define XST_PM_ALREADY_CONFIGURED	2009L

static struct zynqmp_power {
	struct mbox_chan tx_chan;
	struct mbox_chan rx_chan;
} zynqmp_power __section(".data");

#define NODE_ID_LOCATION	5

static unsigned int xpm_configobject[] = {
	/**********************************************************************/
	/* HEADER */
	2,	/* Number of remaining words in the header */
	1,	/* Number of sections included in config object */
	PM_CONFIG_OBJECT_TYPE_OVERLAY,	/* Type of Config object as overlay */
	/**********************************************************************/
	/* SLAVE SECTION */

	PM_CONFIG_SLAVE_SECTION_ID,	/* Section ID */
	1,				/* Number of slaves */

	0, /* Node ID which will be changed below */
	PM_SLAVE_FLAG_IS_SHAREABLE,
	PM_CONFIG_IPI_PSU_CORTEXA53_0_MASK |
	PM_CONFIG_IPI_PSU_CORTEXR5_0_MASK |
	PM_CONFIG_IPI_PSU_CORTEXR5_1_MASK, /* IPI Mask */
};

static unsigned int xpm_configobject_close[] = {
	/**********************************************************************/
	/* HEADER */
	2,	/* Number of remaining words in the header */
	1,	/* Number of sections included in config object */
	PM_CONFIG_OBJECT_TYPE_OVERLAY,	/* Type of Config object as overlay */
	/**********************************************************************/
	/* SET CONFIG SECTION */
	PM_CONFIG_SET_CONFIG_SECTION_ID,
	0U,	/* Loading permission to Overlay config object */
};

int zynqmp_pmufw_config_close(void)
{
	return zynqmp_pmufw_load_config_object(xpm_configobject_close,
					       sizeof(xpm_configobject_close));
}

int zynqmp_pmufw_node(u32 id)
{
	static bool check = true;
	static bool permission = true;

	if (check) {
		check = false;

		if (zynqmp_pmufw_node(NODE_OCM_BANK_0) == -EACCES) {
			printf("PMUFW:  No permission to change config object\n");
			permission = false;
		}
	}

	if (!permission)
		return 0;

	/* Record power domain id */
	xpm_configobject[NODE_ID_LOCATION] = id;

	return zynqmp_pmufw_load_config_object(xpm_configobject,
					       sizeof(xpm_configobject));
}

static int do_pm_probe(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_FIRMWARE,
					  DM_DRIVER_GET(zynqmp_power),
					  &dev);
	if (ret)
		debug("%s: Probing device failed: %d\n", __func__, ret);

	return ret;
}

static int ipi_req(const u32 *req, size_t req_len, u32 *res, size_t res_maxlen)
{
	struct zynqmp_ipi_msg msg;
	int ret;
	u32 buffer[PAYLOAD_ARG_CNT];

	if (!res)
		res = buffer;

	if (req_len > PMUFW_PAYLOAD_ARG_CNT ||
	    res_maxlen > PMUFW_PAYLOAD_ARG_CNT)
		return -EINVAL;

	if (!(zynqmp_power.tx_chan.dev) || !(zynqmp_power.rx_chan.dev)) {
		ret = do_pm_probe();
		if (ret)
			return ret;
	}

	debug("%s, Sending IPI message with ID: 0x%0x\n", __func__, req[0]);
	msg.buf = (u32 *)req;
	msg.len = req_len;
	ret = mbox_send(&zynqmp_power.tx_chan, &msg);
	if (ret) {
		debug("%s: Sending message failed\n", __func__);
		return ret;
	}

	msg.buf = res;
	msg.len = res_maxlen;
	ret = mbox_recv(&zynqmp_power.rx_chan, &msg, 100);
	if (ret)
		debug("%s: Receiving message failed\n", __func__);

	return ret;
}

unsigned int zynqmp_firmware_version(void)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];
	static u32 pm_api_version = ZYNQMP_PM_VERSION_INVALID;

	/*
	 * Get PMU version only once and later
	 * just return stored values instead of
	 * asking PMUFW again.
	 **/
	if (pm_api_version == ZYNQMP_PM_VERSION_INVALID) {

		ret = xilinx_pm_request(PM_GET_API_VERSION, 0, 0, 0, 0,
					ret_payload);
		if (ret)
			panic("PMUFW is not found - Please load it!\n");

		pm_api_version = ret_payload[1];
		if (pm_api_version < ZYNQMP_PM_VERSION)
			panic("PMUFW version error. Expected: v%d.%d\n",
			      ZYNQMP_PM_VERSION_MAJOR, ZYNQMP_PM_VERSION_MINOR);
	}

	return pm_api_version;
};

int zynqmp_pm_set_gem_config(u32 node, enum pm_gem_config_type config, u32 value)
{
	int ret;

	ret = xilinx_pm_request(PM_IOCTL, node, IOCTL_SET_GEM_CONFIG,
				config, value, NULL);
	if (ret)
		printf("%s: node %d: set_gem_config %d failed\n",
		       __func__, node, config);

	return ret;
}

int zynqmp_pm_set_sd_config(u32 node, enum pm_sd_config_type config, u32 value)
{
	int ret;

	ret = xilinx_pm_request(PM_IOCTL, node, IOCTL_SET_SD_CONFIG,
				config, value, NULL);
	if (ret)
		printf("%s: node %d: set_sd_config %d failed\n",
		       __func__, node, config);

	return ret;
}

int zynqmp_pm_feature(const u32 api_id)
{
	int ret;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	/* Check feature check API version */
	ret = xilinx_pm_request(PM_FEATURE_CHECK, api_id, 0, 0, 0,
				ret_payload);
	if (ret)
		return ret;

	/* Return feature check version */
	return ret_payload[1] & FIRMWARE_VERSION_MASK;
}

int zynqmp_pm_is_function_supported(const u32 api_id, const u32 id)
{
	int ret;
	u32 *bit_mask;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	/* Input arguments validation */
	if (id >= 64 || (api_id != PM_IOCTL && api_id != PM_QUERY_DATA))
		return -EINVAL;

	/* Check feature check API version */
	ret = xilinx_pm_request(PM_FEATURE_CHECK, PM_FEATURE_CHECK, 0, 0, 0,
				ret_payload);
	if (ret)
		return ret;

	/* Check if feature check version 2 is supported or not */
	if ((ret_payload[1] & FIRMWARE_VERSION_MASK) == PM_API_VERSION_2) {
		/*
		 * Call feature check for IOCTL/QUERY API to get IOCTL ID or
		 * QUERY ID feature status.
		 */

		ret = xilinx_pm_request(PM_FEATURE_CHECK, api_id, 0, 0, 0,
					ret_payload);
		if (ret)
			return ret;

		bit_mask = &ret_payload[2];
		if ((bit_mask[(id / 32)] & BIT((id % 32))) == 0)
			return -EOPNOTSUPP;
	} else {
		return -ENODATA;
	}

	return 0;
}

/**
 * Send a configuration object to the PMU firmware.
 *
 * @cfg_obj: Pointer to the configuration object
 * @size:    Size of @cfg_obj in bytes
 * Return:   0 on success otherwise negative errno.
 */
int zynqmp_pmufw_load_config_object(const void *cfg_obj, size_t size)
{
	int err;
	u32 ret_payload[PAYLOAD_ARG_CNT];

	if (IS_ENABLED(CONFIG_SPL_BUILD))
		printf("Loading new PMUFW cfg obj (%ld bytes)\n", size);

	flush_dcache_range((ulong)cfg_obj, (ulong)(cfg_obj + size));

	err = xilinx_pm_request(PM_SET_CONFIGURATION, (u32)(u64)cfg_obj, 0, 0,
				0, ret_payload);
	if (err == XST_PM_NO_ACCESS) {
		return -EACCES;
	}

	if (err == XST_PM_ALREADY_CONFIGURED) {
		debug("PMUFW Node is already configured\n");
		return -ENODEV;
	}

	if (err)
		printf("Cannot load PMUFW configuration object (%d)\n", err);

	if (ret_payload[0])
		printf("PMUFW returned 0x%08x status!\n", ret_payload[0]);

	if ((err || ret_payload[0]) && IS_ENABLED(CONFIG_SPL_BUILD))
		panic("PMUFW config object loading failed in EL3\n");

	return 0;
}

static int zynqmp_power_probe(struct udevice *dev)
{
	struct udevice *ipi_dev;
	ofnode ipi_node;
	int ret;

	debug("%s, (dev=%p)\n", __func__, dev);

	/*
	 * Probe all IPI parent node driver. It is important to have IPI
	 * devices available when requested by mbox_get_by* API.
	 * If IPI device isn't available, then mailbox request fails and
	 * that causes system boot failure.
	 * To avoid this make sure all IPI parent drivers are probed here,
	 * and IPI parent driver binds each child node to mailbox driver.
	 * This way mbox_get_by_* API will have correct mailbox device
	 * driver probed.
	 */
	ofnode_for_each_compatible_node(ipi_node, "xlnx,zynqmp-ipi-mailbox") {
		ret = uclass_get_device_by_ofnode(UCLASS_NOP, ipi_node, &ipi_dev);
		if (ret) {
			dev_err(dev, "failed to get IPI device from node %s\n",
				ofnode_get_name(ipi_node));
			return ret;
		}
	}

	ret = mbox_get_by_name(dev, "tx", &zynqmp_power.tx_chan);
	if (ret) {
		debug("%s: Cannot find tx mailbox\n", __func__);
		return ret;
	}

	ret = mbox_get_by_name(dev, "rx", &zynqmp_power.rx_chan);
	if (ret) {
		debug("%s: Cannot find rx mailbox\n", __func__);
		return ret;
	}

	ret = zynqmp_firmware_version();
	printf("PMUFW:\tv%d.%d\n",
	       ret >> ZYNQMP_PM_VERSION_MAJOR_SHIFT,
	       ret & ZYNQMP_PM_VERSION_MINOR_MASK);

	return 0;
};

static const struct udevice_id zynqmp_power_ids[] = {
	{ .compatible = "xlnx,zynqmp-power" },
	{ }
};

U_BOOT_DRIVER(zynqmp_power) = {
	.name = "zynqmp_power",
	.id = UCLASS_FIRMWARE,
	.of_match = zynqmp_power_ids,
	.probe = zynqmp_power_probe,
};
#endif

int __maybe_unused xilinx_pm_request(u32 api_id, u32 arg0, u32 arg1, u32 arg2,
				     u32 arg3, u32 *ret_payload)
{
	debug("%s at EL%d, API ID: 0x%0x, 0x%0x, 0x%0x, 0x%0x, 0x%0x\n",
	      __func__, current_el(), api_id, arg0, arg1, arg2, arg3);

	if (IS_ENABLED(CONFIG_SPL_BUILD) || current_el() == 3) {
#if defined(CONFIG_ZYNQMP_IPI)
		/*
		 * Use fixed payload and arg size as the EL2 call. The firmware
		 * is capable to handle PMUFW_PAYLOAD_ARG_CNT bytes but the
		 * firmware API is limited by the SMC call size
		 */
		u32 regs[] = {api_id, arg0, arg1, arg2, arg3};
		int ret;

		if (api_id == PM_FPGA_LOAD) {
			/* Swap addr_hi/low because of incompatibility */
			u32 temp = regs[1];

			regs[1] = regs[2];
			regs[2] = temp;
		}

		ret = ipi_req(regs, PAYLOAD_ARG_CNT, ret_payload,
			      PAYLOAD_ARG_CNT);
		if (ret)
			return ret;
#else
		return -EPERM;
#endif
	} else {
		/*
		 * Added SIP service call Function Identifier
		 * Make sure to stay in x0 register
		 */
		struct pt_regs regs;

		regs.regs[0] = PM_SIP_SVC | api_id;
		regs.regs[1] = ((u64)arg1 << 32) | arg0;
		regs.regs[2] = ((u64)arg3 << 32) | arg2;

		smc_call(&regs);

		if (ret_payload) {
			ret_payload[0] = (u32)regs.regs[0];
			ret_payload[1] = upper_32_bits(regs.regs[0]);
			ret_payload[2] = (u32)regs.regs[1];
			ret_payload[3] = upper_32_bits(regs.regs[1]);
			ret_payload[4] = (u32)regs.regs[2];
		}

	}
	return (ret_payload) ? ret_payload[0] : 0;
}

static const struct udevice_id zynqmp_firmware_ids[] = {
	{ .compatible = "xlnx,zynqmp-firmware" },
	{ .compatible = "xlnx,versal-firmware"},
	{ .compatible = "xlnx,versal-net-firmware"},
	{ }
};

static int zynqmp_firmware_bind(struct udevice *dev)
{
	int ret;
	struct udevice *child;

	if ((IS_ENABLED(CONFIG_SPL_BUILD) &&
	     IS_ENABLED(CONFIG_SPL_POWER_DOMAIN) &&
	     IS_ENABLED(CONFIG_ZYNQMP_POWER_DOMAIN)) ||
	     (!IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_ZYNQMP_POWER_DOMAIN))) {
		ret = device_bind_driver_to_node(dev, "zynqmp_power_domain",
						 "zynqmp_power_domain",
						 dev_ofnode(dev), &child);
		if (ret) {
			printf("zynqmp power domain driver is not bound: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

U_BOOT_DRIVER(zynqmp_firmware) = {
	.id = UCLASS_FIRMWARE,
	.name = "zynqmp_firmware",
	.of_match = zynqmp_firmware_ids,
	.bind = zynqmp_firmware_bind,
};
