/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2015, Intel Corporation
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_VPD_H__
#define __FSP_VPD_H__

struct __packed memory_upd {
	u64 signature;				/* Offset 0x0020 */
	u8 revision;				/* Offset 0x0028 */
	u8 unused2[7];				/* Offset 0x0029 */
	u16 mrc_init_tseg_size;			/* Offset 0x0030 */
	u16 mrc_init_mmio_size;			/* Offset 0x0032 */
	u8 mrc_init_spd_addr1;			/* Offset 0x0034 */
	u8 mrc_init_spd_addr2;			/* Offset 0x0035 */
	u8 mem_ch0_config;			/* Offset 0x0036 */
	u8 mem_ch1_config;			/* Offset 0x0037 */
	u32 memory_spd_ptr;			/* Offset 0x0038 */
	u8 igd_dvmt50_pre_alloc;		/* Offset 0x003c */
	u8 aperture_size;			/* Offset 0x003d */
	u8 gtt_size;				/* Offset 0x003e */
	u8 legacy_seg_decode;			/* Offset 0x003f */
	u8 enable_dvfs;				/* Offset 0x0040 */
	u8 memory_type;				/* Offset 0x0041 */
	u8 enable_ca_mirror;			/* Offset 0x0042 */
	u8 reserved[189];			/* Offset 0x0043 */
};

struct gpio_family {
	u32 confg;
	u32 confg_changes;
	u32 misc;
	u32 mmio_addr;
	wchar_t *name;
};

struct gpio_pad {
	u32 confg0;
	u32 confg0_changes;
	u32 confg1;
	u32 confg1_changes;
	u32 community;
	u32 mmio_addr;
	wchar_t *name;
	u32 misc;
};

struct __packed silicon_upd {
	u64 signature;				/* Offset 0x0100 */
	u8 revision;				/* Offset 0x0108 */
	u8 unused3[7];				/* Offset 0x0109 */
	u8 sdcard_mode;				/* Offset 0x0110 */
	u8 enable_hsuart0;			/* Offset 0x0111 */
	u8 enable_hsuart1;			/* Offset 0x0112 */
	u8 enable_azalia;			/* Offset 0x0113 */
	struct azalia_config *azalia_cfg_ptr;	/* Offset 0x0114 */
	u8 enable_sata;				/* Offset 0x0118 */
	u8 enable_xhci;				/* Offset 0x0119 */
	u8 lpe_mode;				/* Offset 0x011a */
	u8 enable_dma0;				/* Offset 0x011b */
	u8 enable_dma1;				/* Offset 0x011c */
	u8 enable_i2c0;				/* Offset 0x011d */
	u8 enable_i2c1;				/* Offset 0x011e */
	u8 enable_i2c2;				/* Offset 0x011f */
	u8 enable_i2c3;				/* Offset 0x0120 */
	u8 enable_i2c4;				/* Offset 0x0121 */
	u8 enable_i2c5;				/* Offset 0x0122 */
	u8 enable_i2c6;				/* Offset 0x0123 */
	u32 graphics_config_ptr;		/* Offset 0x0124 */
	struct gpio_family *gpio_familiy_ptr;	/* Offset 0x0128 */
	struct gpio_pad *gpio_pad_ptr;		/* Offset 0x012c */
	u8 disable_punit_pwr_config;		/* Offset 0x0130 */
	u8 chv_svid_config;			/* Offset 0x0131 */
	u8 disable_dptf;			/* Offset 0x0132 */
	u8 emmc_mode;				/* Offset 0x0133 */
	u8 usb3_clk_ssc;			/* Offset 0x0134 */
	u8 disp_clk_ssc;			/* Offset 0x0135 */
	u8 sata_clk_ssc;			/* Offset 0x0136 */
	u8 usb2_port0_pe_txi_set;		/* Offset 0x0137 */
	u8 usb2_port0_txi_set;			/* Offset 0x0138 */
	u8 usb2_port0_tx_emphasis_en;		/* Offset 0x0139 */
	u8 usb2_port0_tx_pe_half;		/* Offset 0x013a */
	u8 usb2_port1_pe_txi_set;		/* Offset 0x013b */
	u8 usb2_port1_txi_set;			/* Offset 0x013c */
	u8 usb2_port1_tx_emphasis_en;		/* Offset 0x013d */
	u8 usb2_port1_tx_pe_half;		/* Offset 0x013e */
	u8 usb2_port2_pe_txi_set;		/* Offset 0x013f */
	u8 usb2_port2_txi_set;			/* Offset 0x0140 */
	u8 usb2_port2_tx_emphasis_en;		/* Offset 0x0141 */
	u8 usb2_port2_tx_pe_half;		/* Offset 0x0142 */
	u8 usb2_port3_pe_txi_set;		/* Offset 0x0143 */
	u8 usb2_port3_txi_set;			/* Offset 0x0144 */
	u8 usb2_port3_tx_emphasis_en;		/* Offset 0x0145 */
	u8 usb2_port3_tx_pe_half;		/* Offset 0x0146 */
	u8 usb2_port4_pe_txi_set;		/* Offset 0x0147 */
	u8 usb2_port4_txi_set;			/* Offset 0x0148 */
	u8 usb2_port4_tx_emphasis_en;		/* Offset 0x0149 */
	u8 usb2_port4_tx_pe_half;		/* Offset 0x014a */
	u8 usb3_lane0_ow2tap_gen2_deemph3p5;	/* Offset 0x014b */
	u8 usb3_lane1_ow2tap_gen2_deemph3p5;	/* Offset 0x014c */
	u8 usb3_lane2_ow2tap_gen2_deemph3p5;	/* Offset 0x014d */
	u8 usb3_lane3_ow2tap_gen2_deemph3p5;	/* Offset 0x014e */
	u8 sata_speed;				/* Offset 0x014f */
	u8 usb_ssic_port;			/* Offset 0x0150 */
	u8 usb_hsic_port;			/* Offset 0x0151 */
	u8 pcie_rootport_speed;			/* Offset 0x0152 */
	u8 enable_ssic;				/* Offset 0x0153 */
	u32 logo_ptr;				/* Offset 0x0154 */
	u32 logo_size;				/* Offset 0x0158 */
	u8 rtc_lock;				/* Offset 0x015c */
	u8 pmic_i2c_bus;			/* Offset 0x015d */
	u8 enable_isp;				/* Offset 0x015e */
	u8 isp_pci_dev_config;			/* Offset 0x015f */
	u8 turbo_mode;				/* Offset 0x0160 */
	u8 pnp_settings;			/* Offset 0x0161 */
	u8 sd_detect_chk;			/* Offset 0x0162 */
	u8 reserved[411];			/* Offset 0x0163 */
};

#define MEMORY_UPD_ID	0x244450554d454d24	/* '$MEMUPD$' */
#define SILICON_UPD_ID	0x244450555f495324	/* '$SI_UPD$' */

struct __packed upd_region {
	u64 signature;				/* Offset 0x0000 */
	u8 revision;				/* Offset 0x0008 */
	u8 unused0[7];				/* Offset 0x0009 */
	u32 memory_upd_offset;			/* Offset 0x0010 */
	u32 silicon_upd_offset;			/* Offset 0x0014 */
	u64 unused1;				/* Offset 0x0018 */
	struct memory_upd memory_upd;		/* Offset 0x0020 */
	struct silicon_upd silicon_upd;		/* Offset 0x0100 */
	u16 terminator;				/* Offset 0x02fe */
};

#define VPD_IMAGE_ID	0x2450534657534224	/* '$BSWFSP$' */

struct __packed vpd_region {
	u64 sign;				/* Offset 0x0000 */
	u32 img_rev;				/* Offset 0x0008 */
	u32 upd_offset;				/* Offset 0x000c */
};

#endif /* __FSP_VPD_H__ */
