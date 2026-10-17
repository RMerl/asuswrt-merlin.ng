// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/list.h>

#include <asm/arch/board.h>
#include <asm/io.h>

#include "cgx_intf.h"
#include "cgx.h"
#include "nix.h"

static u64 cgx_rd_scrx(u8 cgx, u8 lmac, u8 index)
{
	u64 addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	return readq(addr);
}

static void cgx_wr_scrx(u8 cgx, u8 lmac, u8 index, u64 val)
{
	u64 addr;

	addr = (index == 1) ? CGX_CMR_SCRATCH1 : CGX_CMR_SCRATCH0;
	addr += CGX_SHIFT(cgx) + CMR_SHIFT(lmac);
	writeq(val, addr);
}

static u64 cgx_rd_scr0(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 0);
}

static u64 cgx_rd_scr1(u8 cgx, u8 lmac)
{
	return cgx_rd_scrx(cgx, lmac, 1);
}

static void cgx_wr_scr0(u8 cgx, u8 lmac, u64 val)
{
	return cgx_wr_scrx(cgx, lmac, 0, val);
}

static void cgx_wr_scr1(u8 cgx, u8 lmac, u64 val)
{
	return cgx_wr_scrx(cgx, lmac, 1, val);
}

static inline void set_ownership(u8 cgx, u8 lmac, u8 val)
{
	union cgx_scratchx1 scr1;

	scr1.u = cgx_rd_scr1(cgx, lmac);
	scr1.s.own_status = val;
	cgx_wr_scr1(cgx, lmac, scr1.u);
}

static int wait_for_ownership(u8 cgx, u8 lmac)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	u64 cmrx_int;
	int timeout = 5000;

	do {
		scr1.u = cgx_rd_scr1(cgx, lmac);
		scr0.u = cgx_rd_scr0(cgx, lmac);
		/* clear async events if any */
		if (scr0.s.evt_sts.evt_type == CGX_EVT_ASYNC &&
		    scr0.s.evt_sts.ack) {
			/* clear interrupt */
			cmrx_int = readq(CGX_CMR_INT +
					 CGX_SHIFT(cgx) + CMR_SHIFT(lmac));
			cmrx_int |= 0x2; // Overflw bit
			writeq(cmrx_int, CGX_CMR_INT +
					 CGX_SHIFT(cgx) + CMR_SHIFT(lmac));

			/* clear ack */
			scr0.s.evt_sts.ack = 0;
			cgx_wr_scr0(cgx, lmac, scr0.u);
		}

		if (timeout-- < 0) {
			debug("timeout waiting for ownership\n");
			return -ETIMEDOUT;
		}
		mdelay(1);
	} while ((scr1.s.own_status == CGX_OWN_FIRMWARE) &&
		  scr0.s.evt_sts.ack);

	return 0;
}

int cgx_intf_req(u8 cgx, u8 lmac, union cgx_cmd_s cmd_args, u64 *rsp,
		 int use_cmd_id_only)
{
	union cgx_scratchx1 scr1;
	union cgx_scratchx0 scr0;
	u64 cmrx_int;
	int timeout = 500;
	int err = 0;
	u8 cmd = cmd_args.cmd.id;

	if (wait_for_ownership(cgx, lmac)) {
		err = -ETIMEDOUT;
		goto error;
	}

	/* send command */
	scr1.u = cgx_rd_scr1(cgx, lmac);

	if (use_cmd_id_only) {
		scr1.s.cmd.id = cmd;
	} else {
		cmd_args.own_status = scr1.s.own_status;
		scr1.s = cmd_args;
	}
	cgx_wr_scr1(cgx, lmac, scr1.u);

	set_ownership(cgx, lmac, CGX_OWN_FIRMWARE);

	/* wait for response and ownership */
	do {
		scr0.u = cgx_rd_scr0(cgx, lmac);
		scr1.u = cgx_rd_scr1(cgx, lmac);
		mdelay(10);
	} while (timeout-- && (!scr0.s.evt_sts.ack) &&
		 (scr1.s.own_status == CGX_OWN_FIRMWARE));
	if (timeout < 0) {
		debug("%s timeout waiting for ack\n", __func__);
		err = -ETIMEDOUT;
		goto error;
	}

	if (cmd == CGX_CMD_INTF_SHUTDOWN)
		goto error;

	if (scr0.s.evt_sts.evt_type != CGX_EVT_CMD_RESP) {
		debug("%s received async event instead of cmd resp event\n",
		      __func__);
		err = -1;
		goto error;
	}
	if (scr0.s.evt_sts.id != cmd) {
		debug("%s received resp for cmd %d expected cmd %d\n",
		      __func__, scr0.s.evt_sts.id, cmd);
		err = -1;
		goto error;
	}
	if (scr0.s.evt_sts.stat != CGX_STAT_SUCCESS) {
		debug("%s cmd%d failed on cgx%u lmac%u with errcode %d\n",
		      __func__, cmd, cgx, lmac, scr0.s.link_sts.err_type);
		err = -1;
	}

error:
	/* clear interrupt */
	cmrx_int = readq(CGX_CMR_INT + CGX_SHIFT(cgx) + CMR_SHIFT(lmac));
	cmrx_int |= 0x2; // Overflw bit
	writeq(cmrx_int, CGX_CMR_INT + CGX_SHIFT(cgx) + CMR_SHIFT(lmac));

	/* clear ownership and ack */
	scr0.s.evt_sts.ack = 0;
	cgx_wr_scr0(cgx, lmac, scr0.u);

	*rsp = err ? 0 : scr0.u;

	return err;
}

int cgx_intf_get_mac_addr(u8 cgx, u8 lmac, u8 *mac)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_MAC_ADDR;

	ret = cgx_intf_req(cgx, lmac, cmd, &scr0.u, 1);
	if (ret)
		return -1;

	scr0.u >>= 9;
	memcpy(mac, &scr0.u, 6);

	return 0;
}

int cgx_intf_get_ver(u8 cgx, u8 lmac, u8 *ver)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_FW_VER;

	ret = cgx_intf_req(cgx, lmac, cmd, &scr0.u, 1);
	if (ret)
		return -1;

	scr0.u >>= 9;
	*ver = scr0.u & 0xFFFF;

	return 0;
}

int cgx_intf_get_link_sts(u8 cgx, u8 lmac, u64 *lnk_sts)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_LINK_STS;

	ret = cgx_intf_req(cgx, lmac, cmd, &scr0.u, 1);
	if (ret)
		return -1;

	scr0.u >>= 9;
	/* pass the same format as cgx_lnk_sts_s
	 * err_type:10, speed:4, full_duplex:1, link_up:1
	 */
	*lnk_sts = scr0.u & 0xFFFF;
	return 0;
}

int cgx_intf_link_up_dwn(u8 cgx, u8 lmac, u8 up_dwn, u64 *lnk_sts)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = up_dwn ? CGX_CMD_LINK_BRING_UP : CGX_CMD_LINK_BRING_DOWN;

	ret = cgx_intf_req(cgx, lmac, cmd, &scr0.u, 1);
	if (ret)
		return -1;

	scr0.u >>= 9;
	/* pass the same format as cgx_lnk_sts_s
	 * err_type:10, speed:4, full_duplex:1, link_up:1
	 */
	*lnk_sts = scr0.u & 0xFFFF;
	return 0;
}

void cgx_intf_shutdown(void)
{
	union cgx_scratchx0 scr0;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_INTF_SHUTDOWN;

	cgx_intf_req(0, 0, cmd, &scr0.u, 1);
}

int cgx_intf_prbs(u8 qlm, u8 mode, u32 time, u8 lane)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_PRBS;

	cmd.prbs_args.qlm = qlm;
	cmd.prbs_args.mode = mode;
	cmd.prbs_args.time = time;
	cmd.prbs_args.lane = lane;

	ret = cgx_intf_req(0, 0, cmd, &scr0.u, 0);
	if (ret)
		return -1;

	return 0;
}

enum cgx_mode {
	MODE_10G_C2C,
	MODE_10G_C2M,
	MODE_10G_KR,
	MODE_25G_C2C,
	MODE_25G_2_C2C,
	MODE_50G_C2C,
	MODE_50G_4_C2C
};

static char intf_speed_to_str[][8] = {
	"10M",
	"100M",
	"1G",
	"2.5G",
	"5G",
	"10G",
	"20G",
	"25G",
	"40G",
	"50G",
	"80G",
	"100G",
};

static void mode_to_args(int mode, struct cgx_mode_change_args *args)
{
	args->an = 0;
	args->duplex = 0;
	args->port = 0;

	switch (mode) {
	case MODE_10G_C2C:
		args->speed = CGX_LINK_10G;
		args->mode = BIT_ULL(CGX_MODE_10G_C2C_BIT);
		break;
	case MODE_10G_C2M:
		args->speed = CGX_LINK_10G;
		args->mode = BIT_ULL(CGX_MODE_10G_C2M_BIT);
		break;
	case MODE_10G_KR:
		args->speed = CGX_LINK_10G;
		args->mode = BIT_ULL(CGX_MODE_10G_KR_BIT);
		args->an = 1;
		break;
	case MODE_25G_C2C:
		args->speed = CGX_LINK_25G;
		args->mode = BIT_ULL(CGX_MODE_25G_C2C_BIT);
		break;
	case MODE_25G_2_C2C:
		args->speed = CGX_LINK_25G;
		args->mode = BIT_ULL(CGX_MODE_25G_2_C2C_BIT);
		break;
	case MODE_50G_C2C:
		args->speed = CGX_LINK_50G;
		args->mode = BIT_ULL(CGX_MODE_50G_C2C_BIT);
		break;
	case MODE_50G_4_C2C:
		args->speed = CGX_LINK_50G;
		args->mode = BIT_ULL(CGX_MODE_50G_4_C2C_BIT);
	}
}

int cgx_intf_set_mode(struct udevice *ethdev, int mode)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_MODE_CHANGE;

	mode_to_args(mode, &cmd.mode_change_args);

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 0);
	if (ret) {
		printf("Mode change command failed for %s\n", ethdev->name);
		return -1;
	}

	cmd.cmd.id = CGX_CMD_GET_LINK_STS;
	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 1);
	if (ret) {
		printf("Get Link Status failed for %s\n", ethdev->name);
		return -1;
	}

	printf("Current Link Status: ");
	if (scr0.s.link_sts.speed) {
		printf("%s\n", intf_speed_to_str[scr0.s.link_sts.speed]);
		switch (scr0.s.link_sts.fec) {
		case 0:
			printf("FEC_NONE\n");
			break;
		case 1:
			printf("FEC_BASE_R\n");
			break;
		case 2:
			printf("FEC_RS\n");
			break;
		}
		printf("Auto Negotiation %sabled\n",
		       scr0.s.link_sts.an ? "En" : "Dis");
		printf("%s Duplex\n",
		       scr0.s.link_sts.full_duplex ? "Full" : "Half");
	} else {
		printf("Down\n");
	}
	return 0;
}

int cgx_intf_get_mode(struct udevice *ethdev)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_LINK_STS;
	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 1);
	if (ret) {
		printf("Get link status failed for %s\n", ethdev->name);
		return -1;
	}
	printf("Current Interface Mode: ");
	switch (scr0.s.link_sts.mode) {
	case CGX_MODE_10G_C2C_BIT:
		printf("10G_C2C\n");
		break;
	case CGX_MODE_10G_C2M_BIT:
		printf("10G_C2M\n");
		break;
	case CGX_MODE_10G_KR_BIT:
		printf("10G_KR\n");
		break;
	case CGX_MODE_25G_C2C_BIT:
		printf("25G_C2C\n");
		break;
	case CGX_MODE_25G_2_C2C_BIT:
		printf("25G_2_C2C\n");
		break;
	case CGX_MODE_50G_C2C_BIT:
		printf("50G_C2C\n");
		break;
	case CGX_MODE_50G_4_C2C_BIT:
		printf("50G_4_C2C\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}
	return 0;
}

int cgx_intf_get_fec(struct udevice *ethdev)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_SUPPORTED_FEC;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 1);
	if (ret) {
		printf("Get supported FEC failed for %s\n", ethdev->name);
		return -1;
	}

	printf("Supported FEC type: ");
	switch (scr0.s.supported_fec.fec) {
	case 0:
		printf("FEC_NONE\n");
		break;
	case 1:
		printf("FEC_BASE_R\n");
		break;
	case 2:
		printf("FEC_RS\n");
		break;
	case 3:
		printf("FEC_BASE_R FEC_RS\n");
		break;
	}

	cmd.cmd.id = CGX_CMD_GET_LINK_STS;
	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 1);
	if (ret) {
		printf("Get active fec failed for %s\n", ethdev->name);
		return -1;
	}
	printf("Active FEC type: ");
	switch (scr0.s.link_sts.fec) {
	case 0:
		printf("FEC_NONE\n");
		break;
	case 1:
		printf("FEC_BASE_R\n");
		break;
	case 2:
		printf("FEC_RS\n");
		break;
	}
	return 0;
}

int cgx_intf_set_fec(struct udevice *ethdev, int type)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_SET_FEC;
	cmd.fec_args.fec = type;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 0);
	if (ret) {
		printf("Set FEC type %d failed for %s\n", type, ethdev->name);
		return -1;
	}
	return 0;
}

int cgx_intf_get_phy_mod_type(struct udevice *ethdev)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_GET_PHY_MOD_TYPE;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 1);
	if (ret) {
		printf("Get PHYMOD type failed for %s\n", ethdev->name);
		return -1;
	}
	printf("Current phy mod type %s\n",
	       scr0.s.phy_mod_type.mod ? "PAM4" : "NRZ");
	return 0;
}

int cgx_intf_set_phy_mod_type(struct udevice *ethdev, int type)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_SET_PHY_MOD_TYPE;
	cmd.phy_mod_args.mod = type;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 0);
	if (ret) {
		printf("Set PHYMOD type %d failed for %s\n", type,
		       ethdev->name);
		return -1;
	}

	return 0;
}

int cgx_intf_set_an_lbk(struct udevice *ethdev, int enable)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_AN_LOOPBACK;
	cmd.cmd_args.enable = enable;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 0);
	if (ret) {
		printf("Set AN loopback command failed on %s\n", ethdev->name);
		return -1;
	}
	printf("AN loopback %s for %s\n", enable ? "set" : "clear",
	       ethdev->name);

	return 0;
}

int cgx_intf_get_ignore(struct udevice *ethdev, int cgx, int lmac)
{
	struct rvu_pf *rvu;
	struct nix *nix;
	union cgx_scratchx0 scr0;
	int ret, cgx_id = cgx, lmac_id = lmac;
	union cgx_cmd_s cmd;

	if (ethdev) {
		rvu = dev_get_priv(ethdev);
		nix = rvu->nix;
		cgx_id = nix->lmac->cgx->cgx_id;
		lmac_id = nix->lmac->lmac_id;
	}
	cmd.cmd.id = CGX_CMD_GET_PERSIST_IGNORE;

	ret = cgx_intf_req(cgx_id, lmac_id, cmd, &scr0.u, 1);
	if (ret) {
		if (ethdev)
			printf("Get ignore command failed for %s\n",
			       ethdev->name);
		else
			printf("Get ignore command failed for CGX%d LMAC%d\n",
			       cgx_id, lmac_id);
		return -1;
	}
	if (ethdev)
		printf("Persist settings %signored for %s\n",
		       scr0.s.persist.ignore ? "" : "not ", ethdev->name);
	else
		printf("Persist settings %signored for CGX%d LMAC%d\n",
		       scr0.s.persist.ignore ? "" : "not ", cgx_id, lmac_id);

	return 0;
}

int cgx_intf_set_ignore(struct udevice *ethdev, int cgx, int lmac, int ignore)
{
	struct rvu_pf *rvu;
	struct nix *nix;
	union cgx_scratchx0 scr0;
	int ret, cgx_id = cgx, lmac_id = lmac;
	union cgx_cmd_s cmd;

	if (ethdev) {
		rvu = dev_get_priv(ethdev);
		nix = rvu->nix;
		cgx_id = nix->lmac->cgx->cgx_id;
		lmac_id = nix->lmac->lmac_id;
	}
	cmd.cmd.id = CGX_CMD_SET_PERSIST_IGNORE;
	cmd.persist_args.ignore = ignore;

	ret = cgx_intf_req(cgx_id, lmac_id, cmd, &scr0.u, 0);
	if (ret) {
		if (ethdev)
			printf("Set ignore command failed for %s\n",
			       ethdev->name);
		else
			printf("Set ignore command failed for CGX%d LMAC%d\n",
			       cgx_id, lmac_id);
		return -1;
	}

	return 0;
}

int cgx_intf_set_macaddr(struct udevice *ethdev)
{
	struct rvu_pf *rvu = dev_get_priv(ethdev);
	struct nix *nix = rvu->nix;
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;
	u64 mac, tmp;

	memcpy((void *)&tmp, nix->lmac->mac_addr, 6);
	mac = swab64(tmp) >> 16;
	cmd.cmd.id = CGX_CMD_SET_MAC_ADDR;
	cmd.mac_args.addr = mac;
	cmd.mac_args.pf_id = rvu->pfid;

	ret = cgx_intf_req(nix->lmac->cgx->cgx_id, nix->lmac->lmac_id,
			   cmd, &scr0.u, 0);
	if (ret) {
		printf("Set user mac addr failed for %s\n", ethdev->name);
		return -1;
	}

	return 0;
}

int cgx_intf_display_eye(u8 qlm, u8 lane)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_DISPLAY_EYE;

	cmd.dsp_eye_args.qlm = qlm;
	cmd.dsp_eye_args.lane = lane;

	ret = cgx_intf_req(0, 0, cmd, &scr0.u, 0);
	if (ret)
		return -1;

	return 0;
}

int cgx_intf_display_serdes(u8 qlm, u8 lane)
{
	union cgx_scratchx0 scr0;
	int ret;
	union cgx_cmd_s cmd;

	cmd.cmd.id = CGX_CMD_DISPLAY_SERDES;

	cmd.dsp_eye_args.qlm = qlm;
	cmd.dsp_eye_args.lane = lane;

	ret = cgx_intf_req(0, 0, cmd, &scr0.u, 0);
	if (ret)
		return -1;

	return 0;
}
