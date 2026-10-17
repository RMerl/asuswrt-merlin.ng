/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __FSP_CONFIGS_H__
#define __FSP_CONFIGS_H__

#ifndef __ASSEMBLY__
struct fsp_config_data {
	struct fsp_cfg_common	common;
	struct upd_region	fsp_upd;
};

struct fspinit_rtbuf {
	struct common_buf	common;	/* FSP common runtime data structure */
};
#endif

/* FSP user configuration settings */

#define MRC_INIT_TSEG_SIZE_1MB		1
#define MRC_INIT_TSEG_SIZE_2MB		2
#define MRC_INIT_TSEG_SIZE_4MB		4
#define MRC_INIT_TSEG_SIZE_8MB		8

#define MRC_INIT_MMIO_SIZE_1024MB	0x400
#define MRC_INIT_MMIO_SIZE_1536MB	0x600
#define MRC_INIT_MMIO_SIZE_2048MB	0x800

#define IGD_DVMT50_PRE_ALLOC_32MB	0x01
#define IGD_DVMT50_PRE_ALLOC_64MB	0x02
#define IGD_DVMT50_PRE_ALLOC_96MB	0x03
#define IGD_DVMT50_PRE_ALLOC_128MB	0x04
#define IGD_DVMT50_PRE_ALLOC_160MB	0x05
#define IGD_DVMT50_PRE_ALLOC_192MB	0x06
#define IGD_DVMT50_PRE_ALLOC_224MB	0x07
#define IGD_DVMT50_PRE_ALLOC_256MB	0x08
#define IGD_DVMT50_PRE_ALLOC_288MB	0x09
#define IGD_DVMT50_PRE_ALLOC_320MB	0x0a
#define IGD_DVMT50_PRE_ALLOC_352MB	0x0b
#define IGD_DVMT50_PRE_ALLOC_384MB	0x0c
#define IGD_DVMT50_PRE_ALLOC_416MB	0x0d
#define IGD_DVMT50_PRE_ALLOC_448MB	0x0e
#define IGD_DVMT50_PRE_ALLOC_480MB	0x0f
#define IGD_DVMT50_PRE_ALLOC_512MB	0x10

#define APERTURE_SIZE_128MB		1
#define APERTURE_SIZE_256MB		2
#define APERTURE_SIZE_512MB		3

#define GTT_SIZE_1MB			1
#define GTT_SIZE_2MB			2

#define DRAM_TYPE_DDR3			0
#define DRAM_TYPE_LPDDR3		1

#define SDCARD_MODE_DISABLED		0
#define SDCARD_MODE_PCI			1
#define SDCARD_MODE_ACPI		2

#define LPE_MODE_DISABLED		0
#define LPE_MODE_PCI			1
#define LPE_MODE_ACPI			2

#define CHV_SVID_CONFIG_0		0
#define CHV_SVID_CONFIG_1		1
#define CHV_SVID_CONFIG_2		2
#define CHV_SVID_CONFIG_3		3

#define EMMC_MODE_DISABLED		0
#define EMMC_MODE_PCI			1
#define EMMC_MODE_ACPI			2

#define SATA_SPEED_GEN1			1
#define SATA_SPEED_GEN2			2
#define SATA_SPEED_GEN3			3

#define ISP_PCI_DEV_CONFIG_1		1
#define ISP_PCI_DEV_CONFIG_2		2
#define ISP_PCI_DEV_CONFIG_3		3

#define PNP_SETTING_DISABLED		0
#define PNP_SETTING_POWER		1
#define PNP_SETTING_PERF		2
#define PNP_SETTING_POWER_AND_PERF	3

#endif /* __FSP_CONFIGS_H__ */
