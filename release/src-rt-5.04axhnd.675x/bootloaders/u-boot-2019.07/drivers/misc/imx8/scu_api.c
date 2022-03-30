// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <asm/arch/sci/sci.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

/* CLK and PM */
int sc_pm_set_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
			 sc_pm_clock_rate_t *rate)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_SET_CLOCK_RATE;
	RPC_U32(&msg, 0U) = *(u32 *)rate;
	RPC_U16(&msg, 4U) = (u16)resource;
	RPC_U8(&msg, 6U) = (u8)clk;
	RPC_SIZE(&msg) = 3U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: rate:%u resource:%u: clk:%u res:%d\n",
		       __func__, *rate, resource, clk, RPC_R8(&msg));

	*rate = RPC_U32(&msg, 0U);

	return ret;
}

int sc_pm_get_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
			 sc_pm_clock_rate_t *rate)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_GET_CLOCK_RATE;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)clk;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: resource:%d clk:%d: res:%d\n",
		       __func__, resource, clk, RPC_R8(&msg));
		return ret;
	}

	if (rate)
		*rate = RPC_U32(&msg, 0U);

	return 0;
}

int sc_pm_clock_enable(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
		       sc_bool_t enable, sc_bool_t autog)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_CLOCK_ENABLE;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)clk;
	RPC_U8(&msg, 3U) = (u8)enable;
	RPC_U8(&msg, 4U) = (u8)autog;
	RPC_SIZE(&msg) = 3U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: resource:%d clk:%d: enable:%d autog: %d, res:%d\n",
		       __func__, resource, clk, enable, autog, RPC_R8(&msg));

	return ret;
}

int sc_pm_set_resource_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				  sc_pm_power_mode_t mode)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_SET_RESOURCE_POWER_MODE;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)mode;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: resource:%d mode:%d: res:%d\n",
		       __func__, resource, mode, RPC_R8(&msg));

	return ret;
}

/* PAD */
int sc_pad_set(sc_ipc_t ipc, sc_pad_t pad, u32 val)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (u8)PAD_FUNC_SET;
	RPC_U32(&msg, 0U) = (u32)val;
	RPC_U16(&msg, 4U) = (u16)pad;
	RPC_SIZE(&msg) = 3U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: val:%d pad:%d: res:%d\n",
		       __func__, val, pad, RPC_R8(&msg));

	return ret;
}

/* MISC */
int sc_misc_get_control(sc_ipc_t ipc, sc_rsrc_t resource, sc_ctrl_t ctrl,
			u32 *val)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_GET_CONTROL;
	RPC_U32(&msg, 0U) = (u32)ctrl;
	RPC_U16(&msg, 4U) = (u16)resource;
	RPC_SIZE(&msg) = 3U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: ctrl:%d resource:%d: res:%d\n",
		       __func__, ctrl, resource, RPC_R8(&msg));

	if (val)
		*val = RPC_U32(&msg, 0U);

	return ret;
}

void sc_misc_get_boot_dev(sc_ipc_t ipc, sc_rsrc_t *boot_dev)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_GET_BOOT_DEV;
	RPC_SIZE(&msg) = 1U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_R8(&msg));

	if (boot_dev)
		*boot_dev = RPC_U16(&msg, 0U);
}

void sc_misc_boot_status(sc_ipc_t ipc, sc_misc_boot_status_t status)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_BOOT_STATUS;
	RPC_U8(&msg, 0U) = (u8)status;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_TRUE, &msg, size, &msg, size);
	if (ret)
		printf("%s: status:%d res:%d\n",
		       __func__, status, RPC_R8(&msg));
}

void sc_misc_build_info(sc_ipc_t ipc, u32 *build, u32 *commit)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = MISC_FUNC_BUILD_INFO;
	RPC_SIZE(&msg) = 1;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret < 0) {
		printf("%s: err: %d\n", __func__, ret);
		return;
	}

	if (build)
		*build = RPC_U32(&msg, 0);
	if (commit)
		*commit = RPC_U32(&msg, 4);
}

int sc_misc_otp_fuse_read(sc_ipc_t ipc, u32 word, u32 *val)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = MISC_FUNC_OTP_FUSE_READ;
	RPC_U32(&msg, 0) = word;
	RPC_SIZE(&msg) = 2;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret < 0)
		return ret;

	if (val)
		*val = RPC_U32(&msg, 0U);

	return 0;
}

/* RM */
sc_bool_t sc_rm_is_memreg_owned(sc_ipc_t ipc, sc_rm_mr_t mr)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;
	sc_err_t result;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_IS_MEMREG_OWNED;
	RPC_U8(&msg, 0U) = (u8)mr;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	result = RPC_R8(&msg);

	if (result != 0 && result != 1) {
		printf("%s: mr:%d res:%d\n", __func__, mr, RPC_R8(&msg));
		if (ret)
			printf("%s: mr:%d res:%d\n", __func__, mr,
			       RPC_R8(&msg));
	}

	return (sc_bool_t)result;
}

int sc_rm_get_memreg_info(sc_ipc_t ipc, sc_rm_mr_t mr, sc_faddr_t *addr_start,
			  sc_faddr_t *addr_end)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_GET_MEMREG_INFO;
	RPC_U8(&msg, 0U) = (u8)mr;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: mr:%d res:%d\n", __func__, mr, RPC_R8(&msg));

	if (addr_start)
		*addr_start = ((u64)RPC_U32(&msg, 0U) << 32U) |
			RPC_U32(&msg, 4U);

	if (addr_end)
		*addr_end = ((u64)RPC_U32(&msg, 8U) << 32U) |
			RPC_U32(&msg, 12U);

	return ret;
}

sc_bool_t sc_rm_is_resource_owned(sc_ipc_t ipc, sc_rsrc_t resource)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;
	u8 result;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_IS_RESOURCE_OWNED;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	result = RPC_R8(&msg);
	if (result != 0 && result != 1) {
		printf("%s: resource:%d res:%d\n",
		       __func__, resource, RPC_R8(&msg));
		if (ret)
			printf("%s: res:%d res:%d\n", __func__, resource,
			       RPC_R8(&msg));
	}

	return !!result;
}
