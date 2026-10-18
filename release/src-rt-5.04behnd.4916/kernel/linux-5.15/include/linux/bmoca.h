/*
 * Copyright (C) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _BMOCA_H_
#define _BMOCA_H_

#include <linux/if.h>
#include <linux/types.h>
#include <linux/ioctl.h>

/* NOTE: These need to match what is defined in the API template */
#define MOCA_IE_DRV_PRINTF	0xff00
#define MOCA_IE_WDT		0xff01

#define MOCA_BAND_HIGHRF	0
#define MOCA_BAND_MIDRF		1
#define MOCA_BAND_WANRF		2
#define MOCA_BAND_EXT_D		3
#define MOCA_BAND_D_LOW		4
#define MOCA_BAND_D_HIGH	5
#define MOCA_BAND_E		6
#define MOCA_BAND_F		7
#define MOCA_BAND_G		8
#define MOCA_BAND_H		9
#define MOCA_BAND_MAX		10

#define MOCA_BAND_NAMES { \
	"highrf", "midrf", "wanrf", \
	"ext_d", "d_low", "d_high", \
	"e", "f", "g", "h"\
}

#define MOCA_BOOT_FLAGS_BONDED	(1 << 0)

#define MOCA_IOC_MAGIC		'M'

#define MOCA_IOCTL_GET_DRV_INFO_V2	_IOR(MOCA_IOC_MAGIC, 0, \
	struct moca_kdrv_info_v2)

#define MOCA_IOCTL_START	_IOW(MOCA_IOC_MAGIC, 1, struct moca_start)
#define MOCA_IOCTL_STOP		_IO(MOCA_IOC_MAGIC, 2)
#define MOCA_IOCTL_READMEM	_IOR(MOCA_IOC_MAGIC, 3, struct moca_xfer)
#define MOCA_IOCTL_WRITEMEM	_IOR(MOCA_IOC_MAGIC, 4, struct moca_xfer)

#define MOCA_IOCTL_CHECK_FOR_DATA	_IOR(MOCA_IOC_MAGIC, 5, int)
#define MOCA_IOCTL_WOL		_IOW(MOCA_IOC_MAGIC, 6, int)
#define MOCA_IOCTL_GET_DRV_INFO	_IOR(MOCA_IOC_MAGIC, 0, struct moca_kdrv_info)
#define MOCA_IOCTL_SET_CPU_RATE	_IOR(MOCA_IOC_MAGIC, 7, unsigned int)
#define MOCA_IOCTL_SET_PHY_RATE	_IOR(MOCA_IOC_MAGIC, 8, unsigned int)
#define MOCA_IOCTL_GET_3450_REG	_IOR(MOCA_IOC_MAGIC, 9, unsigned int)
#define MOCA_IOCTL_SET_3450_REG	_IOR(MOCA_IOC_MAGIC, 10, unsigned int)
#define MOCA_IOCTL_PM_SUSPEND   _IO(MOCA_IOC_MAGIC, 11)
#define MOCA_IOCTL_PM_WOL	_IO(MOCA_IOC_MAGIC, 12)

#define MOCA_DEVICE_ID_UNREGISTERED  (-1)

/* Flag used in MOCA_IOCTL_READMEM/MOCA_IOCTL_WRITE_MEM */
#define MOCA_ABSOLUTE_ADDR_FLAG 0x80000000

/* this must match MoCAOS_IFNAMSIZE */
#define MOCA_IFNAMSIZ		16

/* ID value hinting ioctl caller to use returned IFNAME as is */
#define MOCA_IFNAME_USE_ID    0xffffffff

/* Legacy version of moca_kdrv_info */
struct moca_kdrv_info_v2 {
	__u32			version;
	__u32			build_number;
	__u32			builtin_fw;

	__u32			hw_rev;
	__u32			rf_band;

	__u32			uptime;
	__s32			refcount;
	__u32			gp1;

	__s8			enet_name[MOCA_IFNAMSIZ];
	__u32			enet_id;

	__u32			macaddr_hi;
	__u32			macaddr_lo;

	__u32			phy_freq;
	__u32			device_id;
};

/* this must match MoCAOS_DrvInfo */
struct moca_kdrv_info {
	__u32			version;
	__u32			build_number;
	__u32			builtin_fw;

	__u32			hw_rev;
	__u32			rf_band;

	__u32			uptime;
	__s32			refcount;
	__u32			gp1;

	__s8			enet_name[MOCA_IFNAMSIZ];
	__u32			enet_id;

	__u32			macaddr_hi;
	__u32			macaddr_lo;

	__u32			phy_freq;
	__u32			device_id;

	__u32			chip_id;
};

struct moca_xfer {
	__u64			buf;
	__u32			len;
	__u32			moca_addr;
};

struct moca_start {
	struct moca_xfer	x;
	__u32			boot_flags;
};

struct moca_regs {
	unsigned int		data_mem_offset;
	unsigned int		data_mem_size;
	unsigned int		cntl_mem_size;
	unsigned int		cntl_mem_offset;
	unsigned int		gp0_offset;
	unsigned int		gp1_offset;
	unsigned int		ringbell_offset;
	unsigned int		l2_status_offset;
	unsigned int		l2_clear_offset;
	unsigned int		l2_mask_set_offset;
	unsigned int		l2_mask_clear_offset;
	unsigned int		sw_reset_offset;
	unsigned int		led_ctrl_offset;
	unsigned int		m2m_src_offset;
	unsigned int		m2m_dst_offset;
	unsigned int		m2m_cmd_offset;
	unsigned int		m2m_status_offset;
	unsigned int		m2m_src_high_offset;
	unsigned int		m2m_dst_high_offset;
	unsigned int		moca2host_mmp_inbox_0_offset;
	unsigned int		moca2host_mmp_inbox_1_offset;
	unsigned int		moca2host_mmp_inbox_2_offset;
	unsigned int		extras_mmp_outbox_3_offset;
	unsigned int		h2m_resp_bit[2]; /* indexed by cpu */
	unsigned int		h2m_req_bit[2]; /* indexed by cpu */
	unsigned int		sideband_gmii_fc_offset;
};

/* MoCA PM states */
enum moca_pm_states {
	MOCA_ACTIVE,
	MOCA_SUSPENDING,
	MOCA_SUSPENDING_WAITING_ACK,
	MOCA_SUSPENDING_GOT_ACK,
	MOCA_SUSPENDED,
	MOCA_RESUMING,
	MOCA_RESUMING_ASSERT,
	MOCA_RESUMING_WDOG,
	MOCA_NONE
};

#ifdef __KERNEL__

static inline void mac_to_u32(uint32_t *hi, uint32_t *lo, const uint8_t *mac)
{
	*hi = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | (mac[3] << 0);
	*lo = (mac[4] << 24) | (mac[5] << 16);
}

static inline void u32_to_mac(uint8_t *mac, uint32_t hi, uint32_t lo)
{
	mac[0] = (hi >> 24) & 0xff;
	mac[1] = (hi >> 16) & 0xff;
	mac[2] = (hi >>  8) & 0xff;
	mac[3] = (hi >>  0) & 0xff;
	mac[4] = (lo >> 24) & 0xff;
	mac[5] = (lo >> 16) & 0xff;
}

struct moca_platform_data {
	char			enet_name[IFNAMSIZ];
	unsigned int		enet_id;

	u32			macaddr_hi;
	u32			macaddr_lo;

	phys_addr_t		bcm3450_i2c_base;
	int			bcm3450_i2c_addr;

	u32			hw_rev;  /* this is the chip_id */
	u32			rf_band;

	u32			chip_id;

	int			minor;
};

enum {
	HWREV_MOCA_11		= 0x1100,
	HWREV_MOCA_11_LITE	= 0x1101,
	HWREV_MOCA_11_PLUS	= 0x1102,
	HWREV_MOCA_20_ALT	= 0x2000, /* for backward compatibility */
	HWREV_MOCA_20_GEN21	= 0x2001,
	HWREV_MOCA_20_GEN22	= 0x2002,
	HWREV_MOCA_20_GEN23	= 0x2003,
	HWREV_MOCA_20_GEN24	= 0x2004,
};


#define MOCA_PROTVER_11		0x1100
#define MOCA_PROTVER_20		0x2000
#define MOCA_PROTVER_MASK	0xff00


#define MOCA_ENABLE		1
#define MOCA_DISABLE		0

/* HW specific functions */
typedef unsigned int (*read32)(void *hw_priv, uintptr_t addr);
typedef void (*write32)(void *hw_priv, uintptr_t addr, u32 data);
typedef void (*hw_reset)(void *hw_priv);
typedef void (*hw_init)(void *hw_priv, int action, int *enabled,
			int bonded_mode);
typedef int (*write_mem)(void *hw_priv, uintptr_t dst, void *src,
			  unsigned int len);
typedef void (*read_mem)(void *hw_priv, void *dst, uintptr_t src,
			 unsigned int len);
typedef void (*write_sg)(void *hw_priv, dma_addr_t dst_offset,
			 struct scatterlist *sg, int nents);
typedef void (*hw_enable_irq)(void *hw_priv);
typedef void (*hw_disable_irq)(void *hw_priv);
typedef void (*remove)(void *hw_priv);

struct moca_ops {
	read32			read32;
	write32			write32;
	hw_reset		hw_reset;
	hw_init			hw_init;
	write_mem		write_mem;
	read_mem		read_mem;
	write_sg		write_sg;
	hw_enable_irq		hw_enable_irq;
	hw_disable_irq		hw_disable_irq;
	remove			remove;

	int			dma;
	int			suspend_timeout_ms;
};

int moca_initialize(struct device *dev, struct moca_ops *moca_ops,
		    void *hw_priv, void __iomem *base, u32 chip_id,
		    int range_check_flag, int irq, int wol_irq,
		    const u8 *macaddr, unsigned int enet_ph,
		    const struct moca_regs *regs);

extern const struct dev_pm_ops moca_pm_ops;

#endif /* __KERNEL__ */

#endif /* ! _BMOCA_H_ */
