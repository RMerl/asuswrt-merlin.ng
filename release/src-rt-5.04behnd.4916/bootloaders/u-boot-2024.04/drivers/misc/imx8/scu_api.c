// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <hang.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm.h>
#include <firmware/imx/sci/sci.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

#define B2U8(X)     (((X) != SC_FALSE) ? (u8)(0x01U) : (u8)(0x00U))

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

int sc_pm_set_clock_parent(sc_ipc_t ipc, sc_rsrc_t resource,
			   sc_pm_clk_t clk, sc_pm_clk_parent_t parent)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_SET_CLOCK_PARENT;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)clk;
	RPC_U8(&msg, 3U) = (u8)parent;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: resource:%d clk:%d: parent clk: %d, res:%d\n",
		       __func__, resource, clk, parent, RPC_R8(&msg));

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

sc_bool_t sc_pm_is_partition_started(sc_ipc_t ipc, sc_rm_pt_t pt)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;
	u8 result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_PM);
	RPC_FUNC(&msg) = (u8)(PM_FUNC_IS_PARTITION_STARTED);
	RPC_U8(&msg, 0U) = (u8)(pt);
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);

	result = RPC_R8(&msg);
	if (result != 0 && result != 1) {
		printf("%s: partition:%d res:%d\n",
		       __func__, pt, RPC_R8(&msg));
		if (ret)
			printf("%s: partition:%d res:%d\n", __func__, pt,
			       RPC_R8(&msg));
	}
	return !!result;
}

int sc_pm_resource_reset(sc_ipc_t ipc, sc_rsrc_t resource)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 2U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_PM);
	RPC_FUNC(&msg) = (u8)(PM_FUNC_RESOURCE_RESET);

	RPC_U16(&msg, 0U) = (u16)(resource);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: resource:%d res:%d\n",
		       __func__, resource, RPC_R8(&msg));

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

int sc_pad_get(sc_ipc_t ipc, sc_pad_t pad, u32 *val)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 2U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_PAD);
	RPC_FUNC(&msg) = (u8)(PAD_FUNC_GET);

	RPC_U16(&msg, 0U) = (u16)(pad);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: pad:%d: res:%d\n",
		       __func__, pad, RPC_R8(&msg));

	if (val)
		*val = (u32)RPC_U32(&msg, 0U);

	return ret;
}

/* MISC */
int sc_misc_set_control(sc_ipc_t ipc, sc_rsrc_t resource,
			sc_ctrl_t ctrl, u32 val)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_SET_CONTROL;
	RPC_U32(&msg, 0U) = (u32)ctrl;
	RPC_U32(&msg, 4U) = (u32)val;
	RPC_U16(&msg, 8U) = (u16)resource;
	RPC_SIZE(&msg) = 4U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: ctrl:%d resource:%d: res:%d\n",
		       __func__, ctrl, resource, RPC_R8(&msg));

	return ret;
}

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

int sc_rm_set_master_sid(sc_ipc_t ipc, sc_rsrc_t resource, sc_rm_sid_t sid)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_SET_MASTER_SID;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U16(&msg, 2U) = (u16)sid;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: resource:%d sid:%d: res:%d\n",
		       __func__, resource, sid, RPC_R8(&msg));

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

int sc_misc_get_boot_container(sc_ipc_t ipc, u8 *idx)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 1U;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_GET_BOOT_CONTAINER;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret < 0)
		return ret;

	if (idx)
		*idx = (u8)RPC_U8(&msg, 0U);

	return 0;
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

int sc_misc_get_temp(sc_ipc_t ipc, sc_rsrc_t resource, sc_misc_temp_t temp,
		     s16 *celsius, s8 *tenths)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (u8)MISC_FUNC_GET_TEMP;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)temp;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret < 0)
		return ret;

	if (celsius)
		*celsius = RPC_I16(&msg, 0U);

	if (tenths)
		*tenths = RPC_I8(&msg, 2U);

	return 0;
}

void sc_misc_get_button_status(sc_ipc_t ipc, sc_bool_t *status)
{
	struct sc_rpc_msg_s msg;
	struct udevice *dev = gd->arch.scu_dev;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 1U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_MISC);
	RPC_FUNC(&msg) = (u8)(MISC_FUNC_GET_BUTTON_STATUS);

	misc_call(dev, SC_FALSE, &msg, 1U, &msg, 1U);

	if (status)
		*status = (sc_bool_t)(!!(RPC_U8(&msg, 0U)));
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

int sc_rm_find_memreg(sc_ipc_t ipc, sc_rm_mr_t *mr, sc_faddr_t addr_start,
		      sc_faddr_t addr_end)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_RM);
	RPC_FUNC(&msg) = (u8)(RM_FUNC_FIND_MEMREG);
	RPC_U32(&msg, 0U) = (u32)(addr_start >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)(addr_start);
	RPC_U32(&msg, 8U) = (u32)(addr_end >> 32ULL);
	RPC_U32(&msg, 12U) = (u32)(addr_end);
	RPC_SIZE(&msg) = 5U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: start:0x%llx, end:0x%llx res:%d\n", __func__, addr_start, addr_end, RPC_R8(&msg));

	if (mr)
		*mr = RPC_U8(&msg, 0U);

	return ret;
}

int sc_rm_set_memreg_permissions(sc_ipc_t ipc, sc_rm_mr_t mr,
				 sc_rm_pt_t pt, sc_rm_perm_t perm)
{
	struct udevice *dev = gd->arch.scu_dev;
	int size = sizeof(struct sc_rpc_msg_s);
	struct sc_rpc_msg_s msg;
	int ret;

	if (!dev)
		hang();

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_RM);
	RPC_FUNC(&msg) = (u8)(RM_FUNC_SET_MEMREG_PERMISSIONS);
	RPC_U8(&msg, 0U) = (u8)(mr);
	RPC_U8(&msg, 1U) = (u8)(pt);
	RPC_U8(&msg, 2U) = (u8)(perm);
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: mr:%u, pt:%u, perm:%u, res:%d\n", __func__,
		       mr, pt, perm, RPC_R8(&msg));
	}

	return ret;
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

int sc_rm_partition_alloc(sc_ipc_t ipc, sc_rm_pt_t *pt, sc_bool_t secure,
			  sc_bool_t isolated, sc_bool_t restricted,
			  sc_bool_t grant, sc_bool_t coherent)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_PARTITION_ALLOC;
	RPC_U8(&msg, 0U) = B2U8(secure);
	RPC_U8(&msg, 1U) = B2U8(isolated);
	RPC_U8(&msg, 2U) = B2U8(restricted);
	RPC_U8(&msg, 3U) = B2U8(grant);
	RPC_U8(&msg, 4U) = B2U8(coherent);
	RPC_SIZE(&msg) = 3U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: secure:%u isolated:%u restricted:%u grant:%u coherent:%u res:%d\n",
		       __func__, secure, isolated, restricted, grant, coherent,
		       RPC_R8(&msg));
	}

	if (pt)
		*pt = RPC_U8(&msg, 0U);

	return ret;
}

int sc_rm_partition_free(sc_ipc_t ipc, sc_rm_pt_t pt)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_PARTITION_FREE;
	RPC_U8(&msg, 0U) = (u8)pt;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: pt:%u res:%d\n",
		       __func__, pt, RPC_R8(&msg));
	}

	return ret;
}

int sc_rm_get_partition(sc_ipc_t ipc, sc_rm_pt_t *pt)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_GET_PARTITION;
	RPC_SIZE(&msg) = 1U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_R8(&msg));

	if (pt)
		*pt = RPC_U8(&msg, 0U);

	return ret;
}

int sc_rm_set_parent(sc_ipc_t ipc, sc_rm_pt_t pt, sc_rm_pt_t pt_parent)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_SET_PARENT;
	RPC_U8(&msg, 0U) = (u8)pt;
	RPC_U8(&msg, 1U) = (u8)pt_parent;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: pt:%u, pt_parent:%u, res:%d\n",
		       __func__, pt, pt_parent, RPC_R8(&msg));
	}

	return ret;
}

int sc_rm_assign_resource(sc_ipc_t ipc, sc_rm_pt_t pt, sc_rsrc_t resource)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_ASSIGN_RESOURCE;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_U8(&msg, 2U) = (u8)pt;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: pt:%u, resource:%u, res:%d\n",
		       __func__, pt, resource, RPC_R8(&msg));
	}

	return ret;
}

int sc_rm_assign_pad(sc_ipc_t ipc, sc_rm_pt_t pt, sc_pad_t pad)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_ASSIGN_PAD;
	RPC_U16(&msg, 0U) = (u16)pad;
	RPC_U8(&msg, 2U) = (u8)pt;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: pt:%u, pad:%u, res:%d\n",
		       __func__, pt, pad, RPC_R8(&msg));
	}

	return ret;
}

sc_bool_t sc_rm_is_pad_owned(sc_ipc_t ipc, sc_pad_t pad)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;
	u8 result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_IS_PAD_OWNED;
	RPC_U16(&msg, 0U) = (u16)pad;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	result = RPC_R8(&msg);
	if (result != 0 && result != 1) {
		printf("%s: pad:%d res:%d\n", __func__, pad, RPC_R8(&msg));
		if (ret) {
			printf("%s: pad:%d res:%d\n", __func__,
			       pad, RPC_R8(&msg));
		}
	}

	return !!result;
}

int sc_rm_get_resource_owner(sc_ipc_t ipc, sc_rsrc_t resource,
			     sc_rm_pt_t *pt)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_RM;
	RPC_FUNC(&msg) = (u8)RM_FUNC_GET_RESOURCE_OWNER;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (pt)
		*pt = RPC_U8(&msg, 0U);

	return ret;
}

int sc_pm_cpu_start(sc_ipc_t ipc, sc_rsrc_t resource, sc_bool_t enable,
		    sc_faddr_t address)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_CPU_START;
	RPC_U32(&msg, 0U) = (u32)(address >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)address;
	RPC_U16(&msg, 8U) = (u16)resource;
	RPC_U8(&msg, 10U) = B2U8(enable);
	RPC_SIZE(&msg) = 4U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: resource:%d address:0x%llx: res:%d\n",
		       __func__, resource, address, RPC_R8(&msg));
	}

	return ret;
}

void sc_pm_reboot(sc_ipc_t ipc, sc_pm_reset_type_t type)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_PM);
	RPC_FUNC(&msg) = (u8)(PM_FUNC_REBOOT);
	RPC_U8(&msg, 0U) = (u8)(type);
	RPC_SIZE(&msg) = 2U;

	misc_call(dev, SC_TRUE, &msg, size, &msg, size);
}

int sc_pm_reset_reason(sc_ipc_t ipc, sc_pm_reset_reason_t *reason)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_RESET_REASON;
	RPC_SIZE(&msg) = 1U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_U8(&msg, 0U));

	if (reason)
		*reason = RPC_U8(&msg, 0U);

	return ret;
}

int sc_pm_get_resource_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				  sc_pm_power_mode_t *mode)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_PM;
	RPC_FUNC(&msg) = (u8)PM_FUNC_GET_RESOURCE_POWER_MODE;
	RPC_U16(&msg, 0U) = (u16)resource;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: resource:%d: res:%d\n",
		       __func__, resource, RPC_R8(&msg));
	}

	if (mode)
		*mode = RPC_U8(&msg, 0U);

	return ret;
}

int sc_timer_set_wdog_window(sc_ipc_t ipc, sc_timer_wdog_time_t window)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 2U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_TIMER);
	RPC_FUNC(&msg) = (u8)(TIMER_FUNC_SET_WDOG_WINDOW);

	RPC_U32(&msg, 0U) = (u32)(window);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: window:%u: res:%d\n",
		       __func__, window, RPC_R8(&msg));

	return ret;
}

int sc_seco_authenticate(sc_ipc_t ipc, sc_seco_auth_cmd_t cmd,
			 sc_faddr_t addr)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_SECO;
	RPC_FUNC(&msg) = (u8)SECO_FUNC_AUTHENTICATE;
	RPC_U32(&msg, 0U) = (u32)(addr >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)addr;
	RPC_U8(&msg, 8U) = (u8)cmd;
	RPC_SIZE(&msg) = 4U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_R8(&msg));

	return ret;
}

int sc_seco_forward_lifecycle(sc_ipc_t ipc, u32 change)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_SECO;
	RPC_FUNC(&msg) = (u8)SECO_FUNC_FORWARD_LIFECYCLE;
	RPC_U32(&msg, 0U) = (u32)change;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: change:%u, res:%d\n", __func__,
		       change, RPC_R8(&msg));
	}

	return ret;
}

int sc_seco_chip_info(sc_ipc_t ipc, u16 *lc, u16 *monotonic, u32 *uid_l,
		      u32 *uid_h)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_SECO;
	RPC_FUNC(&msg) = (u8)SECO_FUNC_CHIP_INFO;
	RPC_SIZE(&msg) = 1U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_R8(&msg));

	if (uid_l)
		*uid_l = RPC_U32(&msg, 0U);

	if (uid_h)
		*uid_h = RPC_U32(&msg, 4U);

	if (lc)
		*lc = RPC_U16(&msg, 8U);

	if (monotonic)
		*monotonic = RPC_U16(&msg, 10U);

	return ret;
}

void sc_seco_build_info(sc_ipc_t ipc, u32 *version, u32 *commit)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_BUILD_INFO);
	RPC_SIZE(&msg) = 1U;

	misc_call(dev, SC_FALSE, &msg, size, &msg, size);

	if (version)
		*version = RPC_U32(&msg, 0U);

	if (commit)
		*commit = RPC_U32(&msg, 4U);
}

int sc_seco_v2x_build_info(sc_ipc_t ipc, u32 *version, u32 *commit)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 1U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_V2X_BUILD_INFO);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: res:%d\n", __func__, RPC_R8(&msg));

	if (version)
		*version = RPC_U32(&msg, 0U);

	if (commit)
		*commit = RPC_U32(&msg, 4U);

	return ret;
}

int sc_seco_get_event(sc_ipc_t ipc, u8 idx, u32 *event)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_SECO;
	RPC_FUNC(&msg) = (u8)SECO_FUNC_GET_EVENT;
	RPC_U8(&msg, 0U) = (u8)idx;
	RPC_SIZE(&msg) = 2U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s: idx: %u, res:%d\n", __func__, idx, RPC_R8(&msg));

	if (event)
		*event = RPC_U32(&msg, 0U);

	return ret;
}

int sc_seco_gen_key_blob(sc_ipc_t ipc, u32 id, sc_faddr_t load_addr,
			 sc_faddr_t export_addr, u16 max_size)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (u8)SC_RPC_SVC_SECO;
	RPC_FUNC(&msg) = (u8)SECO_FUNC_GEN_KEY_BLOB;
	RPC_U32(&msg, 0U) = (u32)(load_addr >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)load_addr;
	RPC_U32(&msg, 8U) = (u32)(export_addr >> 32ULL);
	RPC_U32(&msg, 12U) = (u32)export_addr;
	RPC_U32(&msg, 16U) = (u32)id;
	RPC_U16(&msg, 20U) = (u16)max_size;
	RPC_SIZE(&msg) = 7U;

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret) {
		printf("%s: id: %u, load_addr 0x%llx, export_addr 0x%llx, res:%d\n",
		       __func__, id, load_addr, export_addr, RPC_R8(&msg));
	}

	return ret;
}

int sc_seco_get_mp_key(sc_ipc_t ipc, sc_faddr_t dst_addr,
			u16 dst_size)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 4U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_GET_MP_KEY);

	RPC_U32(&msg, 0U) = (u32)(dst_addr >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)(dst_addr);
	RPC_U16(&msg, 8U) = (u16)(dst_size);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s, dst_addr:0x%llx, res:%d\n",
		       __func__, dst_addr, RPC_R8(&msg));

	return ret;
}

int sc_seco_update_mpmr(sc_ipc_t ipc, sc_faddr_t addr, u8 size_m,
			u8 lock)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 4U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_UPDATE_MPMR);

	RPC_U32(&msg, 0U) = (u32)(addr >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)(addr);
	RPC_U8(&msg, 8U) = (u8)(size_m);
	RPC_U8(&msg, 9U) = (u8)(lock);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s, addr:0x%llx, size_m:%x, lock:0x%x, res:%d\n",
		       __func__, addr, size_m, lock, RPC_R8(&msg));
	return ret;
}

int sc_seco_get_mp_sign(sc_ipc_t ipc, sc_faddr_t msg_addr,
			u16 msg_size, sc_faddr_t dst_addr,
			u16 dst_size)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 6U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_GET_MP_SIGN);

	RPC_U32(&msg, 0U) = (u32)(msg_addr >> 32ULL);
	RPC_U32(&msg, 4U) = (u32)(msg_addr);
	RPC_U32(&msg, 8U) = (u32)(dst_addr >> 32ULL);
	RPC_U32(&msg, 12U) = (u32)(dst_addr);
	RPC_U16(&msg, 16U) = (u16)(msg_size);
	RPC_U16(&msg, 18U) = (u16)(dst_size);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s, msg_addr:0x%llx, msg_size:%x, dst_addr:0x%llx,"
		       "dst_size:%x, res:%d\n", __func__, msg_addr, msg_size,
		       dst_addr, dst_size, RPC_R8(&msg));

	return ret;
}

int sc_seco_secvio_config(sc_ipc_t ipc, u8 id, u8 access,
			  u32 *data0, u32 *data1, u32 *data2, u32 *data3,
			  u32 *data4, u8 size)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int msg_size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 7U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_SECVIO_CONFIG);

	RPC_U32(&msg, 0U) = (u32)(*data0);
	RPC_U32(&msg, 4U) = (u32)(*data1);
	RPC_U32(&msg, 8U) = (u32)(*data2);
	RPC_U32(&msg, 12U) = (u32)(*data3);
	RPC_U32(&msg, 16U) = (u32)(*data4);
	RPC_U8(&msg, 20U) = (u8)(id);
	RPC_U8(&msg, 21U) = (u8)(access);
	RPC_U8(&msg, 22U) = (u8)(size);

	ret = misc_call(dev, SC_FALSE, &msg, msg_size, &msg, msg_size);
	if (ret)
		printf("%s, id:0x%x, access:%x, res:%d\n",
		       __func__, id, access, RPC_R8(&msg));

	*data0 = (u32)RPC_U32(&msg, 0U);
	*data1 = (u32)RPC_U32(&msg, 4U);
	*data2 = (u32)RPC_U32(&msg, 8U);
	*data3 = (u32)RPC_U32(&msg, 12U);
	*data4 = (u32)RPC_U32(&msg, 16U);

	return ret;
}

int sc_seco_secvio_dgo_config(sc_ipc_t ipc, u8 id, u8 access, u32 *data)
{
	struct udevice *dev = gd->arch.scu_dev;
	struct sc_rpc_msg_s msg;
	int size = sizeof(struct sc_rpc_msg_s);
	int ret;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SIZE(&msg) = 3U;
	RPC_SVC(&msg) = (u8)(SC_RPC_SVC_SECO);
	RPC_FUNC(&msg) = (u8)(SECO_FUNC_SECVIO_DGO_CONFIG);

	RPC_U32(&msg, 0U) = (u32)(*data);
	RPC_U8(&msg, 4U) = (u8)(id);
	RPC_U8(&msg, 5U) = (u8)(access);

	ret = misc_call(dev, SC_FALSE, &msg, size, &msg, size);
	if (ret)
		printf("%s, id:0x%x, access:%x, res:%d\n",
		       __func__, id, access, RPC_R8(&msg));

	if (data)
		*data = RPC_U32(&msg, 0U);

	return ret;
}
