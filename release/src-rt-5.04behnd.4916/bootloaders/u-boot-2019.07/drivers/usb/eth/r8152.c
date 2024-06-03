// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Realtek Semiconductor Corp. All rights reserved.
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <usb.h>
#include <linux/mii.h>
#include <linux/bitops.h>
#include "usb_ether.h"
#include "r8152.h"

#ifndef CONFIG_DM_ETH
/* local vars */
static int curr_eth_dev; /* index for name of next device detected */

struct r8152_dongle {
	unsigned short vendor;
	unsigned short product;
};

static const struct r8152_dongle r8152_dongles[] = {
	/* Realtek */
	{ 0x0bda, 0x8050 },
	{ 0x0bda, 0x8152 },
	{ 0x0bda, 0x8153 },

	/* Samsung */
	{ 0x04e8, 0xa101 },

	/* Lenovo */
	{ 0x17ef, 0x304f },
	{ 0x17ef, 0x3052 },
	{ 0x17ef, 0x3054 },
	{ 0x17ef, 0x3057 },
	{ 0x17ef, 0x7205 },
	{ 0x17ef, 0x720a },
	{ 0x17ef, 0x720b },
	{ 0x17ef, 0x720c },

	/* TP-LINK */
	{ 0x2357, 0x0601 },

	/* Nvidia */
	{ 0x0955, 0x09ff },
};
#endif

struct r8152_version {
	unsigned short tcr;
	unsigned short version;
	bool           gmii;
};

static const struct r8152_version r8152_versions[] = {
	{ 0x4c00, RTL_VER_01, 0 },
	{ 0x4c10, RTL_VER_02, 0 },
	{ 0x5c00, RTL_VER_03, 1 },
	{ 0x5c10, RTL_VER_04, 1 },
	{ 0x5c20, RTL_VER_05, 1 },
	{ 0x5c30, RTL_VER_06, 1 },
	{ 0x4800, RTL_VER_07, 0 },
};

static
int get_registers(struct r8152 *tp, u16 value, u16 index, u16 size, void *data)
{
	ALLOC_CACHE_ALIGN_BUFFER(void *, tmp, size);
	int ret;

	ret = usb_control_msg(tp->udev, usb_rcvctrlpipe(tp->udev, 0),
		RTL8152_REQ_GET_REGS, RTL8152_REQT_READ,
		value, index, tmp, size, 500);
	memcpy(data, tmp, size);
	return ret;
}

static
int set_registers(struct r8152 *tp, u16 value, u16 index, u16 size, void *data)
{
	ALLOC_CACHE_ALIGN_BUFFER(void *, tmp, size);

	memcpy(tmp, data, size);
	return usb_control_msg(tp->udev, usb_sndctrlpipe(tp->udev, 0),
			       RTL8152_REQ_SET_REGS, RTL8152_REQT_WRITE,
			       value, index, tmp, size, 500);
}

int generic_ocp_read(struct r8152 *tp, u16 index, u16 size,
		     void *data, u16 type)
{
	u16 burst_size = 64;
	int ret;
	int txsize;

	/* both size and index must be 4 bytes align */
	if ((size & 3) || !size || (index & 3) || !data)
		return -EINVAL;

	if (index + size > 0xffff)
		return -EINVAL;

	while (size) {
		txsize = min(size, burst_size);
		ret = get_registers(tp, index, type, txsize, data);
		if (ret < 0)
			break;

		index += txsize;
		data += txsize;
		size -= txsize;
	}

	return ret;
}

int generic_ocp_write(struct r8152 *tp, u16 index, u16 byteen,
		      u16 size, void *data, u16 type)
{
	int ret;
	u16 byteen_start, byteen_end, byte_en_to_hw;
	u16 burst_size = 512;
	int txsize;

	/* both size and index must be 4 bytes align */
	if ((size & 3) || !size || (index & 3) || !data)
		return -EINVAL;

	if (index + size > 0xffff)
		return -EINVAL;

	byteen_start = byteen & BYTE_EN_START_MASK;
	byteen_end = byteen & BYTE_EN_END_MASK;

	byte_en_to_hw = byteen_start | (byteen_start << 4);
	ret = set_registers(tp, index, type | byte_en_to_hw, 4, data);
	if (ret < 0)
		return ret;

	index += 4;
	data += 4;
	size -= 4;

	if (size) {
		size -= 4;

		while (size) {
			txsize = min(size, burst_size);

			ret = set_registers(tp, index,
					    type | BYTE_EN_DWORD,
					    txsize, data);
			if (ret < 0)
				return ret;

			index += txsize;
			data += txsize;
			size -= txsize;
		}

		byte_en_to_hw = byteen_end | (byteen_end >> 4);
		ret = set_registers(tp, index, type | byte_en_to_hw, 4, data);
		if (ret < 0)
			return ret;
	}

	return ret;
}

int pla_ocp_read(struct r8152 *tp, u16 index, u16 size, void *data)
{
	return generic_ocp_read(tp, index, size, data, MCU_TYPE_PLA);
}

int pla_ocp_write(struct r8152 *tp, u16 index, u16 byteen, u16 size, void *data)
{
	return generic_ocp_write(tp, index, byteen, size, data, MCU_TYPE_PLA);
}

int usb_ocp_read(struct r8152 *tp, u16 index, u16 size, void *data)
{
	return generic_ocp_read(tp, index, size, data, MCU_TYPE_USB);
}

int usb_ocp_write(struct r8152 *tp, u16 index, u16 byteen, u16 size, void *data)
{
	return generic_ocp_write(tp, index, byteen, size, data, MCU_TYPE_USB);
}

u32 ocp_read_dword(struct r8152 *tp, u16 type, u16 index)
{
	__le32 data;

	generic_ocp_read(tp, index, sizeof(data), &data, type);

	return __le32_to_cpu(data);
}

void ocp_write_dword(struct r8152 *tp, u16 type, u16 index, u32 data)
{
	__le32 tmp = __cpu_to_le32(data);

	generic_ocp_write(tp, index, BYTE_EN_DWORD, sizeof(tmp), &tmp, type);
}

u16 ocp_read_word(struct r8152 *tp, u16 type, u16 index)
{
	u32 data;
	__le32 tmp;
	u8 shift = index & 2;

	index &= ~3;

	generic_ocp_read(tp, index, sizeof(tmp), &tmp, type);

	data = __le32_to_cpu(tmp);
	data >>= (shift * 8);
	data &= 0xffff;

	return data;
}

void ocp_write_word(struct r8152 *tp, u16 type, u16 index, u32 data)
{
	u32 mask = 0xffff;
	__le32 tmp;
	u16 byen = BYTE_EN_WORD;
	u8 shift = index & 2;

	data &= mask;

	if (index & 2) {
		byen <<= shift;
		mask <<= (shift * 8);
		data <<= (shift * 8);
		index &= ~3;
	}

	tmp = __cpu_to_le32(data);

	generic_ocp_write(tp, index, byen, sizeof(tmp), &tmp, type);
}

u8 ocp_read_byte(struct r8152 *tp, u16 type, u16 index)
{
	u32 data;
	__le32 tmp;
	u8 shift = index & 3;

	index &= ~3;

	generic_ocp_read(tp, index, sizeof(tmp), &tmp, type);

	data = __le32_to_cpu(tmp);
	data >>= (shift * 8);
	data &= 0xff;

	return data;
}

void ocp_write_byte(struct r8152 *tp, u16 type, u16 index, u32 data)
{
	u32 mask = 0xff;
	__le32 tmp;
	u16 byen = BYTE_EN_BYTE;
	u8 shift = index & 3;

	data &= mask;

	if (index & 3) {
		byen <<= shift;
		mask <<= (shift * 8);
		data <<= (shift * 8);
		index &= ~3;
	}

	tmp = __cpu_to_le32(data);

	generic_ocp_write(tp, index, byen, sizeof(tmp), &tmp, type);
}

u16 ocp_reg_read(struct r8152 *tp, u16 addr)
{
	u16 ocp_base, ocp_index;

	ocp_base = addr & 0xf000;
	if (ocp_base != tp->ocp_base) {
		ocp_write_word(tp, MCU_TYPE_PLA, PLA_OCP_GPHY_BASE, ocp_base);
		tp->ocp_base = ocp_base;
	}

	ocp_index = (addr & 0x0fff) | 0xb000;
	return ocp_read_word(tp, MCU_TYPE_PLA, ocp_index);
}

void ocp_reg_write(struct r8152 *tp, u16 addr, u16 data)
{
	u16 ocp_base, ocp_index;

	ocp_base = addr & 0xf000;
	if (ocp_base != tp->ocp_base) {
		ocp_write_word(tp, MCU_TYPE_PLA, PLA_OCP_GPHY_BASE, ocp_base);
		tp->ocp_base = ocp_base;
	}

	ocp_index = (addr & 0x0fff) | 0xb000;
	ocp_write_word(tp, MCU_TYPE_PLA, ocp_index, data);
}

static void r8152_mdio_write(struct r8152 *tp, u32 reg_addr, u32 value)
{
	ocp_reg_write(tp, OCP_BASE_MII + reg_addr * 2, value);
}

static int r8152_mdio_read(struct r8152 *tp, u32 reg_addr)
{
	return ocp_reg_read(tp, OCP_BASE_MII + reg_addr * 2);
}

void sram_write(struct r8152 *tp, u16 addr, u16 data)
{
	ocp_reg_write(tp, OCP_SRAM_ADDR, addr);
	ocp_reg_write(tp, OCP_SRAM_DATA, data);
}

int r8152_wait_for_bit(struct r8152 *tp, bool ocp_reg, u16 type, u16 index,
		       const u32 mask, bool set, unsigned int timeout)
{
	u32 val;

	while (--timeout) {
		if (ocp_reg)
			val = ocp_reg_read(tp, index);
		else
			val = ocp_read_dword(tp, type, index);

		if (!set)
			val = ~val;

		if ((val & mask) == mask)
			return 0;

		mdelay(1);
	}

	debug("%s: Timeout (index=%04x mask=%08x timeout=%d)\n",
	      __func__, index, mask, timeout);

	return -ETIMEDOUT;
}

static void r8152b_reset_packet_filter(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_FMC);
	ocp_data &= ~FMC_FCR_MCU_EN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_FMC, ocp_data);
	ocp_data |= FMC_FCR_MCU_EN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_FMC, ocp_data);
}

static void rtl8152_wait_fifo_empty(struct r8152 *tp)
{
	int ret;

	ret = r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, PLA_PHY_PWR,
				 PLA_PHY_PWR_TXEMP, 1, R8152_WAIT_TIMEOUT);
	if (ret)
		debug("Timeout waiting for FIFO empty\n");

	ret = r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, PLA_TCR0,
				 TCR0_TX_EMPTY, 1, R8152_WAIT_TIMEOUT);
	if (ret)
		debug("Timeout waiting for TX empty\n");
}

static void rtl8152_nic_reset(struct r8152 *tp)
{
	int ret;
	u32 ocp_data;

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, BIST_CTRL);
	ocp_data |= BIST_CTRL_SW_RESET;
	ocp_write_dword(tp, MCU_TYPE_PLA, BIST_CTRL, ocp_data);

	ret = r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, BIST_CTRL,
				 BIST_CTRL_SW_RESET, 0, R8152_WAIT_TIMEOUT);
	if (ret)
		debug("Timeout waiting for NIC reset\n");
}

static u8 rtl8152_get_speed(struct r8152 *tp)
{
	return ocp_read_byte(tp, MCU_TYPE_PLA, PLA_PHYSTATUS);
}

static void rtl_set_eee_plus(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR);
	ocp_data &= ~EEEP_CR_EEEP_TX;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR, ocp_data);
}

static void rxdy_gated_en(struct r8152 *tp, bool enable)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_MISC_1);
	if (enable)
		ocp_data |= RXDY_GATED_EN;
	else
		ocp_data &= ~RXDY_GATED_EN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_MISC_1, ocp_data);
}

static void rtl8152_set_rx_mode(struct r8152 *tp)
{
	u32 ocp_data;
	__le32 tmp[2];

	tmp[0] = 0xffffffff;
	tmp[1] = 0xffffffff;

	pla_ocp_write(tp, PLA_MAR, BYTE_EN_DWORD, sizeof(tmp), tmp);

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data |= RCR_APM | RCR_AM | RCR_AB;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);
}

static int rtl_enable(struct r8152 *tp)
{
	u32 ocp_data;

	r8152b_reset_packet_filter(tp);

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_CR);
	ocp_data |= PLA_CR_RE | PLA_CR_TE;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CR, ocp_data);

	rxdy_gated_en(tp, false);

	rtl8152_set_rx_mode(tp);

	return 0;
}

static int rtl8152_enable(struct r8152 *tp)
{
	rtl_set_eee_plus(tp);

	return rtl_enable(tp);
}

static void r8153_set_rx_early_timeout(struct r8152 *tp)
{
	u32 ocp_data = tp->coalesce / 8;

	ocp_write_word(tp, MCU_TYPE_USB, USB_RX_EARLY_TIMEOUT, ocp_data);
}

static void r8153_set_rx_early_size(struct r8152 *tp)
{
	u32 ocp_data = (RTL8152_AGG_BUF_SZ - RTL8153_RMS) / 4;

	ocp_write_word(tp, MCU_TYPE_USB, USB_RX_EARLY_SIZE, ocp_data);
}

static int rtl8153_enable(struct r8152 *tp)
{
	rtl_set_eee_plus(tp);
	r8153_set_rx_early_timeout(tp);
	r8153_set_rx_early_size(tp);

	return rtl_enable(tp);
}

static void rtl_disable(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data &= ~RCR_ACPT_ALL;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);

	rxdy_gated_en(tp, true);

	rtl8152_wait_fifo_empty(tp);
	rtl8152_nic_reset(tp);
}

static void r8152_power_cut_en(struct r8152 *tp, bool enable)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_UPS_CTRL);
	if (enable)
		ocp_data |= POWER_CUT;
	else
		ocp_data &= ~POWER_CUT;
	ocp_write_word(tp, MCU_TYPE_USB, USB_UPS_CTRL, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_PM_CTRL_STATUS);
	ocp_data &= ~RESUME_INDICATE;
	ocp_write_word(tp, MCU_TYPE_USB, USB_PM_CTRL_STATUS, ocp_data);
}

static void rtl_rx_vlan_en(struct r8152 *tp, bool enable)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_CPCR);
	if (enable)
		ocp_data |= CPCR_RX_VLAN;
	else
		ocp_data &= ~CPCR_RX_VLAN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_CPCR, ocp_data);
}

static void r8153_u1u2en(struct r8152 *tp, bool enable)
{
	u8 u1u2[8];

	if (enable)
		memset(u1u2, 0xff, sizeof(u1u2));
	else
		memset(u1u2, 0x00, sizeof(u1u2));

	usb_ocp_write(tp, USB_TOLERANCE, BYTE_EN_SIX_BYTES, sizeof(u1u2), u1u2);
}

static void r8153_u2p3en(struct r8152 *tp, bool enable)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_U2P3_CTRL);
	if (enable && tp->version != RTL_VER_03 && tp->version != RTL_VER_04)
		ocp_data |= U2P3_ENABLE;
	else
		ocp_data &= ~U2P3_ENABLE;
	ocp_write_word(tp, MCU_TYPE_USB, USB_U2P3_CTRL, ocp_data);
}

static void r8153_power_cut_en(struct r8152 *tp, bool enable)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_POWER_CUT);
	if (enable)
		ocp_data |= PWR_EN | PHASE2_EN;
	else
		ocp_data &= ~(PWR_EN | PHASE2_EN);
	ocp_write_word(tp, MCU_TYPE_USB, USB_POWER_CUT, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_MISC_0);
	ocp_data &= ~PCUT_STATUS;
	ocp_write_word(tp, MCU_TYPE_USB, USB_MISC_0, ocp_data);
}

static int r8152_read_mac(struct r8152 *tp, unsigned char *macaddr)
{
	int ret;
	unsigned char enetaddr[8] = {0};

	ret = pla_ocp_read(tp, PLA_IDR, 8, enetaddr);
	if (ret < 0)
		return ret;

	memcpy(macaddr, enetaddr, ETH_ALEN);
	return 0;
}

static void r8152b_disable_aldps(struct r8152 *tp)
{
	ocp_reg_write(tp, OCP_ALDPS_CONFIG, ENPDNPS | LINKENA | DIS_SDSAVE);
	mdelay(20);
}

static void r8152b_enable_aldps(struct r8152 *tp)
{
	ocp_reg_write(tp, OCP_ALDPS_CONFIG, ENPWRSAVE | ENPDNPS |
		LINKENA | DIS_SDSAVE);
}

static void rtl8152_disable(struct r8152 *tp)
{
	r8152b_disable_aldps(tp);
	rtl_disable(tp);
	r8152b_enable_aldps(tp);
}

static void r8152b_hw_phy_cfg(struct r8152 *tp)
{
	u16 data;

	data = r8152_mdio_read(tp, MII_BMCR);
	if (data & BMCR_PDOWN) {
		data &= ~BMCR_PDOWN;
		r8152_mdio_write(tp, MII_BMCR, data);
	}

	r8152b_firmware(tp);
}

static void rtl8152_reinit_ll(struct r8152 *tp)
{
	u32 ocp_data;
	int ret;

	ret = r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, PLA_PHY_PWR,
				 PLA_PHY_PWR_LLR, 1, R8152_WAIT_TIMEOUT);
	if (ret)
		debug("Timeout waiting for link list ready\n");

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
	ocp_data |= RE_INIT_LL;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

	ret = r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, PLA_PHY_PWR,
				 PLA_PHY_PWR_LLR, 1, R8152_WAIT_TIMEOUT);
	if (ret)
		debug("Timeout waiting for link list ready\n");
}

static void r8152b_exit_oob(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data &= ~RCR_ACPT_ALL;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);

	rxdy_gated_en(tp, true);
	r8152b_hw_phy_cfg(tp);

	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_NORAML);
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CR, 0x00);

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data &= ~NOW_IS_OOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
	ocp_data &= ~MCU_BORW_EN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

	rtl8152_reinit_ll(tp);
	rtl8152_nic_reset(tp);

	/* rx share fifo credit full threshold */
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL0, RXFIFO_THR1_NORMAL);

	if (tp->udev->speed == USB_SPEED_FULL ||
	    tp->udev->speed == USB_SPEED_LOW) {
		/* rx share fifo credit near full threshold */
		ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1,
				RXFIFO_THR2_FULL);
		ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2,
				RXFIFO_THR3_FULL);
	} else {
		/* rx share fifo credit near full threshold */
		ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1,
				RXFIFO_THR2_HIGH);
		ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2,
				RXFIFO_THR3_HIGH);
	}

	/* TX share fifo free credit full threshold */
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_TXFIFO_CTRL, TXFIFO_THR_NORMAL);

	ocp_write_byte(tp, MCU_TYPE_USB, USB_TX_AGG, TX_AGG_MAX_THRESHOLD);
	ocp_write_dword(tp, MCU_TYPE_USB, USB_RX_BUF_TH, RX_THR_HIGH);
	ocp_write_dword(tp, MCU_TYPE_USB, USB_TX_DMA,
			TEST_MODE_DISABLE | TX_SIZE_ADJUST1);

	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, RTL8152_RMS);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR0);
	ocp_data |= TCR0_AUTO_FIFO;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_TCR0, ocp_data);
}

static void r8152b_enter_oob(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data &= ~NOW_IS_OOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL0, RXFIFO_THR1_OOB);
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1, RXFIFO_THR2_OOB);
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2, RXFIFO_THR3_OOB);

	rtl_disable(tp);

	rtl8152_reinit_ll(tp);

	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, RTL8152_RMS);

	rtl_rx_vlan_en(tp, false);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PAL_BDC_CR);
	ocp_data |= ALDPS_PROXY_MODE;
	ocp_write_word(tp, MCU_TYPE_PLA, PAL_BDC_CR, ocp_data);

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data |= NOW_IS_OOB | DIS_MCU_CLROOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	rxdy_gated_en(tp, false);

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data |= RCR_APM | RCR_AM | RCR_AB;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);
}

static void r8153_hw_phy_cfg(struct r8152 *tp)
{
	u32 ocp_data;
	u16 data;

	if (tp->version == RTL_VER_03 || tp->version == RTL_VER_04 ||
	    tp->version == RTL_VER_05)
		ocp_reg_write(tp, OCP_ADC_CFG, CKADSEL_L | ADC_EN | EN_EMI_L);

	data = r8152_mdio_read(tp, MII_BMCR);
	if (data & BMCR_PDOWN) {
		data &= ~BMCR_PDOWN;
		r8152_mdio_write(tp, MII_BMCR, data);
	}

	r8153_firmware(tp);

	if (tp->version == RTL_VER_03) {
		data = ocp_reg_read(tp, OCP_EEE_CFG);
		data &= ~CTAP_SHORT_EN;
		ocp_reg_write(tp, OCP_EEE_CFG, data);
	}

	data = ocp_reg_read(tp, OCP_POWER_CFG);
	data |= EEE_CLKDIV_EN;
	ocp_reg_write(tp, OCP_POWER_CFG, data);

	data = ocp_reg_read(tp, OCP_DOWN_SPEED);
	data |= EN_10M_BGOFF;
	ocp_reg_write(tp, OCP_DOWN_SPEED, data);
	data = ocp_reg_read(tp, OCP_POWER_CFG);
	data |= EN_10M_PLLOFF;
	ocp_reg_write(tp, OCP_POWER_CFG, data);
	sram_write(tp, SRAM_IMPEDANCE, 0x0b13);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR);
	ocp_data |= PFM_PWM_SWITCH;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR, ocp_data);

	/* Enable LPF corner auto tune */
	sram_write(tp, SRAM_LPF_CFG, 0xf70f);

	/* Adjust 10M Amplitude */
	sram_write(tp, SRAM_10M_AMP1, 0x00af);
	sram_write(tp, SRAM_10M_AMP2, 0x0208);
}

static void r8153_first_init(struct r8152 *tp)
{
	u32 ocp_data;

	rxdy_gated_en(tp, true);

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data &= ~RCR_ACPT_ALL;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);

	r8153_hw_phy_cfg(tp);

	rtl8152_nic_reset(tp);

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data &= ~NOW_IS_OOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
	ocp_data &= ~MCU_BORW_EN;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

	rtl8152_reinit_ll(tp);

	rtl_rx_vlan_en(tp, false);

	ocp_data = RTL8153_RMS;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, ocp_data);
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_MTPS, MTPS_JUMBO);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR0);
	ocp_data |= TCR0_AUTO_FIFO;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_TCR0, ocp_data);

	rtl8152_nic_reset(tp);

	/* rx share fifo credit full threshold */
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL0, RXFIFO_THR1_NORMAL);
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1, RXFIFO_THR2_NORMAL);
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2, RXFIFO_THR3_NORMAL);
	/* TX share fifo free credit full threshold */
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_TXFIFO_CTRL, TXFIFO_THR_NORMAL2);

	/* rx aggregation */
	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_USB_CTRL);

	ocp_data &= ~(RX_AGG_DISABLE | RX_ZERO_EN);
	ocp_write_word(tp, MCU_TYPE_USB, USB_USB_CTRL, ocp_data);
}

static void r8153_enter_oob(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data &= ~NOW_IS_OOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	rtl_disable(tp);

	rtl8152_reinit_ll(tp);

	ocp_data = RTL8153_RMS;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TEREDO_CFG);
	ocp_data &= ~TEREDO_WAKE_MASK;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_TEREDO_CFG, ocp_data);

	rtl_rx_vlan_en(tp, false);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PAL_BDC_CR);
	ocp_data |= ALDPS_PROXY_MODE;
	ocp_write_word(tp, MCU_TYPE_PLA, PAL_BDC_CR, ocp_data);

	ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
	ocp_data |= NOW_IS_OOB | DIS_MCU_CLROOB;
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

	rxdy_gated_en(tp, false);

	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
	ocp_data |= RCR_APM | RCR_AM | RCR_AB;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);
}

static void r8153_disable_aldps(struct r8152 *tp)
{
	u16 data;

	data = ocp_reg_read(tp, OCP_POWER_CFG);
	data &= ~EN_ALDPS;
	ocp_reg_write(tp, OCP_POWER_CFG, data);
	mdelay(20);
}

static void rtl8153_disable(struct r8152 *tp)
{
	r8153_disable_aldps(tp);
	rtl_disable(tp);
}

static int rtl8152_set_speed(struct r8152 *tp, u8 autoneg, u16 speed, u8 duplex)
{
	u16 bmcr, anar, gbcr;

	anar = r8152_mdio_read(tp, MII_ADVERTISE);
	anar &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
		  ADVERTISE_100HALF | ADVERTISE_100FULL);
	if (tp->supports_gmii) {
		gbcr = r8152_mdio_read(tp, MII_CTRL1000);
		gbcr &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);
	} else {
		gbcr = 0;
	}

	if (autoneg == AUTONEG_DISABLE) {
		if (speed == SPEED_10) {
			bmcr = 0;
			anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
		} else if (speed == SPEED_100) {
			bmcr = BMCR_SPEED100;
			anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
		} else if (speed == SPEED_1000 && tp->supports_gmii) {
			bmcr = BMCR_SPEED1000;
			gbcr |= ADVERTISE_1000FULL | ADVERTISE_1000HALF;
		} else {
			return -EINVAL;
		}

		if (duplex == DUPLEX_FULL)
			bmcr |= BMCR_FULLDPLX;
	} else {
		if (speed == SPEED_10) {
			if (duplex == DUPLEX_FULL)
				anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
			else
				anar |= ADVERTISE_10HALF;
		} else if (speed == SPEED_100) {
			if (duplex == DUPLEX_FULL) {
				anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
				anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
			} else {
				anar |= ADVERTISE_10HALF;
				anar |= ADVERTISE_100HALF;
			}
		} else if (speed == SPEED_1000 && tp->supports_gmii) {
			if (duplex == DUPLEX_FULL) {
				anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
				anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
				gbcr |= ADVERTISE_1000FULL | ADVERTISE_1000HALF;
			} else {
				anar |= ADVERTISE_10HALF;
				anar |= ADVERTISE_100HALF;
				gbcr |= ADVERTISE_1000HALF;
			}
		} else {
			return -EINVAL;
		}

		bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
	}

	if (tp->supports_gmii)
		r8152_mdio_write(tp, MII_CTRL1000, gbcr);

	r8152_mdio_write(tp, MII_ADVERTISE, anar);
	r8152_mdio_write(tp, MII_BMCR, bmcr);

	return 0;
}

static void rtl8152_up(struct r8152 *tp)
{
	r8152b_disable_aldps(tp);
	r8152b_exit_oob(tp);
	r8152b_enable_aldps(tp);
}

static void rtl8152_down(struct r8152 *tp)
{
	r8152_power_cut_en(tp, false);
	r8152b_disable_aldps(tp);
	r8152b_enter_oob(tp);
	r8152b_enable_aldps(tp);
}

static void rtl8153_up(struct r8152 *tp)
{
	r8153_u1u2en(tp, false);
	r8153_disable_aldps(tp);
	r8153_first_init(tp);
	r8153_u2p3en(tp, false);
}

static void rtl8153_down(struct r8152 *tp)
{
	r8153_u1u2en(tp, false);
	r8153_u2p3en(tp, false);
	r8153_power_cut_en(tp, false);
	r8153_disable_aldps(tp);
	r8153_enter_oob(tp);
}

static void r8152b_get_version(struct r8152 *tp)
{
	u32 ocp_data;
	u16 tcr;
	int i;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR1);
	tcr = (u16)(ocp_data & VERSION_MASK);

	for (i = 0; i < ARRAY_SIZE(r8152_versions); i++) {
		if (tcr == r8152_versions[i].tcr) {
			/* Found a supported version */
			tp->version = r8152_versions[i].version;
			tp->supports_gmii = r8152_versions[i].gmii;
			break;
		}
	}

	if (tp->version == RTL_VER_UNKNOWN)
		debug("r8152 Unknown tcr version 0x%04x\n", tcr);
}

static void r8152b_enable_fc(struct r8152 *tp)
{
	u16 anar;
	anar = r8152_mdio_read(tp, MII_ADVERTISE);
	anar |= ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM;
	r8152_mdio_write(tp, MII_ADVERTISE, anar);
}

static void rtl_tally_reset(struct r8152 *tp)
{
	u32 ocp_data;

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_RSTTALLY);
	ocp_data |= TALLY_RESET;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_RSTTALLY, ocp_data);
}

static void r8152b_init(struct r8152 *tp)
{
	u32 ocp_data;

	r8152b_disable_aldps(tp);

	if (tp->version == RTL_VER_01) {
		ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_LED_FEATURE);
		ocp_data &= ~LED_MODE_MASK;
		ocp_write_word(tp, MCU_TYPE_PLA, PLA_LED_FEATURE, ocp_data);
	}

	r8152_power_cut_en(tp, false);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR);
	ocp_data |= TX_10M_IDLE_EN | PFM_PWM_SWITCH;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR, ocp_data);
	ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_MAC_PWR_CTRL);
	ocp_data &= ~MCU_CLK_RATIO_MASK;
	ocp_data |= MCU_CLK_RATIO | D3_CLK_GATED_EN;
	ocp_write_dword(tp, MCU_TYPE_PLA, PLA_MAC_PWR_CTRL, ocp_data);
	ocp_data = GPHY_STS_MSK | SPEED_DOWN_MSK |
		   SPDWN_RXDV_MSK | SPDWN_LINKCHG_MSK;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_GPHY_INTR_IMR, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_USB_TIMER);
	ocp_data |= BIT(15);
	ocp_write_word(tp, MCU_TYPE_USB, USB_USB_TIMER, ocp_data);
	ocp_write_word(tp, MCU_TYPE_USB, 0xcbfc, 0x03e8);
	ocp_data &= ~BIT(15);
	ocp_write_word(tp, MCU_TYPE_USB, USB_USB_TIMER, ocp_data);

	r8152b_enable_fc(tp);
	rtl_tally_reset(tp);

	/* enable rx aggregation */
	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_USB_CTRL);

	ocp_data &= ~(RX_AGG_DISABLE | RX_ZERO_EN);
	ocp_write_word(tp, MCU_TYPE_USB, USB_USB_CTRL, ocp_data);
}

static void r8153_init(struct r8152 *tp)
{
	int i;
	u32 ocp_data;

	r8153_disable_aldps(tp);
	r8153_u1u2en(tp, false);

	r8152_wait_for_bit(tp, 0, MCU_TYPE_PLA, PLA_BOOT_CTRL,
			   AUTOLOAD_DONE, 1, R8152_WAIT_TIMEOUT);

	for (i = 0; i < R8152_WAIT_TIMEOUT; i++) {
		ocp_data = ocp_reg_read(tp, OCP_PHY_STATUS) & PHY_STAT_MASK;
		if (ocp_data == PHY_STAT_LAN_ON || ocp_data == PHY_STAT_PWRDN)
			break;

		mdelay(1);
	}

	r8153_u2p3en(tp, false);

	if (tp->version == RTL_VER_04) {
		ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_SSPHYLINK2);
		ocp_data &= ~pwd_dn_scale_mask;
		ocp_data |= pwd_dn_scale(96);
		ocp_write_word(tp, MCU_TYPE_USB, USB_SSPHYLINK2, ocp_data);

		ocp_data = ocp_read_byte(tp, MCU_TYPE_USB, USB_USB2PHY);
		ocp_data |= USB2PHY_L1 | USB2PHY_SUSPEND;
		ocp_write_byte(tp, MCU_TYPE_USB, USB_USB2PHY, ocp_data);
	} else if (tp->version == RTL_VER_05) {
		ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_DMY_REG0);
		ocp_data &= ~ECM_ALDPS;
		ocp_write_byte(tp, MCU_TYPE_PLA, PLA_DMY_REG0, ocp_data);

		ocp_data = ocp_read_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY1);
		if (ocp_read_word(tp, MCU_TYPE_USB, USB_BURST_SIZE) == 0)
			ocp_data &= ~DYNAMIC_BURST;
		else
			ocp_data |= DYNAMIC_BURST;
		ocp_write_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY1, ocp_data);
	} else if (tp->version == RTL_VER_06) {
		ocp_data = ocp_read_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY1);
		if (ocp_read_word(tp, MCU_TYPE_USB, USB_BURST_SIZE) == 0)
			ocp_data &= ~DYNAMIC_BURST;
		else
			ocp_data |= DYNAMIC_BURST;
		ocp_write_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY1, ocp_data);
	}

	ocp_data = ocp_read_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY2);
	ocp_data |= EP4_FULL_FC;
	ocp_write_byte(tp, MCU_TYPE_USB, USB_CSR_DUMMY2, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_WDT11_CTRL);
	ocp_data &= ~TIMER11_EN;
	ocp_write_word(tp, MCU_TYPE_USB, USB_WDT11_CTRL, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_LED_FEATURE);
	ocp_data &= ~LED_MODE_MASK;
	ocp_write_word(tp, MCU_TYPE_PLA, PLA_LED_FEATURE, ocp_data);

	ocp_data = FIFO_EMPTY_1FB | ROK_EXIT_LPM;
	if (tp->version == RTL_VER_04 && tp->udev->speed != USB_SPEED_SUPER)
		ocp_data |= LPM_TIMER_500MS;
	else
		ocp_data |= LPM_TIMER_500US;
	ocp_write_byte(tp, MCU_TYPE_USB, USB_LPM_CTRL, ocp_data);

	ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_AFE_CTRL2);
	ocp_data &= ~SEN_VAL_MASK;
	ocp_data |= SEN_VAL_NORMAL | SEL_RXIDLE;
	ocp_write_word(tp, MCU_TYPE_USB, USB_AFE_CTRL2, ocp_data);

	ocp_write_word(tp, MCU_TYPE_USB, USB_CONNECT_TIMER, 0x0001);

	r8153_power_cut_en(tp, false);

	r8152b_enable_fc(tp);
	rtl_tally_reset(tp);
}

static void rtl8152_unload(struct r8152 *tp)
{
	if (tp->version != RTL_VER_01)
		r8152_power_cut_en(tp, true);
}

static void rtl8153_unload(struct r8152 *tp)
{
	r8153_power_cut_en(tp, false);
}

static int rtl_ops_init(struct r8152 *tp)
{
	struct rtl_ops *ops = &tp->rtl_ops;
	int ret = 0;

	switch (tp->version) {
	case RTL_VER_01:
	case RTL_VER_02:
	case RTL_VER_07:
		ops->init		= r8152b_init;
		ops->enable		= rtl8152_enable;
		ops->disable		= rtl8152_disable;
		ops->up			= rtl8152_up;
		ops->down		= rtl8152_down;
		ops->unload		= rtl8152_unload;
		break;

	case RTL_VER_03:
	case RTL_VER_04:
	case RTL_VER_05:
	case RTL_VER_06:
		ops->init		= r8153_init;
		ops->enable		= rtl8153_enable;
		ops->disable		= rtl8153_disable;
		ops->up			= rtl8153_up;
		ops->down		= rtl8153_down;
		ops->unload		= rtl8153_unload;
		break;

	default:
		ret = -ENODEV;
		printf("r8152 Unknown Device\n");
		break;
	}

	return ret;
}

static int r8152_init_common(struct r8152 *tp)
{
	u8 speed;
	int timeout = 0;
	int link_detected;

	debug("** %s()\n", __func__);

	do {
		speed = rtl8152_get_speed(tp);

		link_detected = speed & LINK_STATUS;
		if (!link_detected) {
			if (timeout == 0)
				printf("Waiting for Ethernet connection... ");
			mdelay(TIMEOUT_RESOLUTION);
			timeout += TIMEOUT_RESOLUTION;
		}
	} while (!link_detected && timeout < PHY_CONNECT_TIMEOUT);
	if (link_detected) {
		tp->rtl_ops.enable(tp);

		if (timeout != 0)
			printf("done.\n");
	} else {
		printf("unable to connect.\n");
	}

	return 0;
}

static int r8152_send_common(struct ueth_data *ueth, void *packet, int length)
{
	struct usb_device *udev = ueth->pusb_dev;
	u32 opts1, opts2 = 0;
	int err;
	int actual_len;
	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, msg,
				 PKTSIZE + sizeof(struct tx_desc));
	struct tx_desc *tx_desc = (struct tx_desc *)msg;

	debug("** %s(), len %d\n", __func__, length);

	opts1 = length | TX_FS | TX_LS;

	tx_desc->opts2 = cpu_to_le32(opts2);
	tx_desc->opts1 = cpu_to_le32(opts1);

	memcpy(msg + sizeof(struct tx_desc), (void *)packet, length);

	err = usb_bulk_msg(udev, usb_sndbulkpipe(udev, ueth->ep_out),
			   (void *)msg, length + sizeof(struct tx_desc),
			   &actual_len, USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %zu, actual = %u, err = %d\n",
	      length + sizeof(struct tx_desc), actual_len, err);

	return err;
}

#ifndef CONFIG_DM_ETH
static int r8152_init(struct eth_device *eth, bd_t *bd)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct r8152 *tp = (struct r8152 *)dev->dev_priv;

	return r8152_init_common(tp);
}

static int r8152_send(struct eth_device *eth, void *packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return r8152_send_common(dev, packet, length);
}

static int r8152_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	ALLOC_CACHE_ALIGN_BUFFER(uint8_t, recv_buf, RTL8152_AGG_BUF_SZ);
	unsigned char *pkt_ptr;
	int err;
	int actual_len;
	u16 packet_len;

	u32 bytes_process = 0;
	struct rx_desc *rx_desc;

	debug("** %s()\n", __func__);

	err = usb_bulk_msg(dev->pusb_dev,
				usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
				(void *)recv_buf,
				RTL8152_AGG_BUF_SZ,
				&actual_len,
				USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", RTL8152_AGG_BUF_SZ,
	      actual_len, err);
	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -1;
	}
	if (actual_len > RTL8152_AGG_BUF_SZ) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -1;
	}

	while (bytes_process < actual_len) {
		rx_desc = (struct rx_desc *)(recv_buf + bytes_process);
		pkt_ptr = recv_buf + sizeof(struct rx_desc) + bytes_process;

		packet_len = le32_to_cpu(rx_desc->opts1) & RX_LEN_MASK;
		packet_len -= CRC_SIZE;

		net_process_received_packet(pkt_ptr, packet_len);

		bytes_process +=
			(packet_len + sizeof(struct rx_desc) + CRC_SIZE);

		if (bytes_process % 8)
			bytes_process = bytes_process + 8 - (bytes_process % 8);
	}

	return 0;
}

static void r8152_halt(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct r8152 *tp = (struct r8152 *)dev->dev_priv;

	debug("** %s()\n", __func__);

	tp->rtl_ops.disable(tp);
}

static int r8152_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	struct r8152 *tp = (struct r8152 *)dev->dev_priv;

	unsigned char enetaddr[8] = {0};

	memcpy(enetaddr, eth->enetaddr, ETH_ALEN);

	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_CONFIG);
	pla_ocp_write(tp, PLA_IDR, BYTE_EN_SIX_BYTES, 8, enetaddr);
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_NORAML);

	debug("MAC %pM\n", eth->enetaddr);
	return 0;
}

void r8152_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

/* Probe to see if a new device is actually an realtek device */
int r8152_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int ep_in_found = 0, ep_out_found = 0;
	int i;

	struct r8152 *tp;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; i < ARRAY_SIZE(r8152_dongles); i++) {
		if (dev->descriptor.idVendor == r8152_dongles[i].vendor &&
		    dev->descriptor.idProduct == r8152_dongles[i].product)
			/* Found a supported dongle */
			break;
	}

	if (i == ARRAY_SIZE(r8152_dongles))
		return 0;

	memset(ss, 0, sizeof(struct ueth_data));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected: %#04x:%#04x\n",
	      dev->descriptor.idVendor, dev->descriptor.idProduct);

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct r8152));

	if (!ss->dev_priv)
		return 0;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and
	 * int. We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			u8 ep_addr = iface->ep_desc[i].bEndpointAddress;
			if ((ep_addr & USB_DIR_IN) && !ep_in_found) {
				ss->ep_in = ep_addr &
					USB_ENDPOINT_NUMBER_MASK;
				ep_in_found = 1;
			} else {
				if (!ep_out_found) {
					ss->ep_out = ep_addr &
						USB_ENDPOINT_NUMBER_MASK;
					ep_out_found = 1;
				}
			}
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}

	debug("Endpoints In %d Out %d Int %d\n",
	      ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("Problems with device\n");
		return 0;
	}

	dev->privptr = (void *)ss;

	tp = ss->dev_priv;
	tp->udev = dev;
	tp->intf = iface;

	r8152b_get_version(tp);

	if (rtl_ops_init(tp))
		return 0;

	tp->rtl_ops.init(tp);
	tp->rtl_ops.up(tp);

	rtl8152_set_speed(tp, AUTONEG_ENABLE,
			  tp->supports_gmii ? SPEED_1000 : SPEED_100,
			  DUPLEX_FULL);

	return 1;
}

int r8152_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}

	sprintf(eth->name, "%s#%d", R8152_BASE_NAME, curr_eth_dev++);
	eth->init = r8152_init;
	eth->send = r8152_send;
	eth->recv = r8152_recv;
	eth->halt = r8152_halt;
	eth->write_hwaddr = r8152_write_hwaddr;
	eth->priv = ss;

	/* Get the MAC address */
	if (r8152_read_mac(ss->dev_priv, eth->enetaddr) < 0)
		return 0;

	debug("MAC %pM\n", eth->enetaddr);
	return 1;
}
#endif /* !CONFIG_DM_ETH */

#ifdef CONFIG_DM_ETH
static int r8152_eth_start(struct udevice *dev)
{
	struct r8152 *tp = dev_get_priv(dev);

	debug("** %s (%d)\n", __func__, __LINE__);

	return r8152_init_common(tp);
}

void r8152_eth_stop(struct udevice *dev)
{
	struct r8152 *tp = dev_get_priv(dev);

	debug("** %s (%d)\n", __func__, __LINE__);

	tp->rtl_ops.disable(tp);
}

int r8152_eth_send(struct udevice *dev, void *packet, int length)
{
	struct r8152 *tp = dev_get_priv(dev);

	return r8152_send_common(&tp->ueth, packet, length);
}

int r8152_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct r8152 *tp = dev_get_priv(dev);
	struct ueth_data *ueth = &tp->ueth;
	uint8_t *ptr;
	int ret, len;
	struct rx_desc *rx_desc;
	u16 packet_len;

	len = usb_ether_get_rx_bytes(ueth, &ptr);
	debug("%s: first try, len=%d\n", __func__, len);
	if (!len) {
		if (!(flags & ETH_RECV_CHECK_DEVICE))
			return -EAGAIN;
		ret = usb_ether_receive(ueth, RTL8152_AGG_BUF_SZ);
		if (ret)
			return ret;

		len = usb_ether_get_rx_bytes(ueth, &ptr);
		debug("%s: second try, len=%d\n", __func__, len);
	}

	rx_desc = (struct rx_desc *)ptr;
	packet_len = le32_to_cpu(rx_desc->opts1) & RX_LEN_MASK;
	packet_len -= CRC_SIZE;

	if (packet_len > len - (sizeof(struct rx_desc) + CRC_SIZE)) {
		debug("Rx: too large packet: %d\n", packet_len);
		goto err;
	}

	*packetp = ptr + sizeof(struct rx_desc);
	return packet_len;

err:
	usb_ether_advance_rxbuf(ueth, -1);
	return -ENOSPC;
}

static int r8152_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct r8152 *tp = dev_get_priv(dev);

	packet_len += sizeof(struct rx_desc) + CRC_SIZE;
	packet_len = ALIGN(packet_len, 8);
	usb_ether_advance_rxbuf(&tp->ueth, packet_len);

	return 0;
}

static int r8152_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct r8152 *tp = dev_get_priv(dev);

	unsigned char enetaddr[8] = { 0 };

	debug("** %s (%d)\n", __func__, __LINE__);
	memcpy(enetaddr, pdata->enetaddr, ETH_ALEN);

	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_CONFIG);
	pla_ocp_write(tp, PLA_IDR, BYTE_EN_SIX_BYTES, 8, enetaddr);
	ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_NORAML);

	debug("MAC %pM\n", pdata->enetaddr);
	return 0;
}

int r8152_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct r8152 *tp = dev_get_priv(dev);

	debug("** %s (%d)\n", __func__, __LINE__);
	r8152_read_mac(tp, pdata->enetaddr);
	return 0;
}

static int r8152_eth_probe(struct udevice *dev)
{
	struct usb_device *udev = dev_get_parent_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct r8152 *tp = dev_get_priv(dev);
	struct ueth_data *ueth = &tp->ueth;
	int ret;

	tp->udev = udev;
	r8152_read_mac(tp, pdata->enetaddr);

	r8152b_get_version(tp);

	ret = rtl_ops_init(tp);
	if (ret)
		return ret;

	tp->rtl_ops.init(tp);
	tp->rtl_ops.up(tp);

	rtl8152_set_speed(tp, AUTONEG_ENABLE,
			  tp->supports_gmii ? SPEED_1000 : SPEED_100,
			  DUPLEX_FULL);

	return usb_ether_register(dev, ueth, RTL8152_AGG_BUF_SZ);
}

static const struct eth_ops r8152_eth_ops = {
	.start	= r8152_eth_start,
	.send	= r8152_eth_send,
	.recv	= r8152_eth_recv,
	.free_pkt = r8152_free_pkt,
	.stop	= r8152_eth_stop,
	.write_hwaddr = r8152_write_hwaddr,
	.read_rom_hwaddr = r8152_read_rom_hwaddr,
};

U_BOOT_DRIVER(r8152_eth) = {
	.name	= "r8152_eth",
	.id	= UCLASS_ETH,
	.probe = r8152_eth_probe,
	.ops	= &r8152_eth_ops,
	.priv_auto_alloc_size = sizeof(struct r8152),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id r8152_eth_id_table[] = {
	/* Realtek */
	{ USB_DEVICE(0x0bda, 0x8050) },
	{ USB_DEVICE(0x0bda, 0x8152) },
	{ USB_DEVICE(0x0bda, 0x8153) },

	/* Samsung */
	{ USB_DEVICE(0x04e8, 0xa101) },

	/* Lenovo */
	{ USB_DEVICE(0x17ef, 0x304f) },
	{ USB_DEVICE(0x17ef, 0x3052) },
	{ USB_DEVICE(0x17ef, 0x3054) },
	{ USB_DEVICE(0x17ef, 0x3057) },
	{ USB_DEVICE(0x17ef, 0x7205) },
	{ USB_DEVICE(0x17ef, 0x720a) },
	{ USB_DEVICE(0x17ef, 0x720b) },
	{ USB_DEVICE(0x17ef, 0x720c) },

	/* TP-LINK */
	{ USB_DEVICE(0x2357, 0x0601) },

	/* Nvidia */
	{ USB_DEVICE(0x0955, 0x09ff) },

	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(r8152_eth, r8152_eth_id_table);
#endif /* CONFIG_DM_ETH */

