/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
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

#define EMMC_BOOT_MODE_DISABLED		0
#define EMMC_BOOT_MODE_AUTO		1
#define EMMC_BOOT_MODE_EMMC41		2
#define EMMC_BOOT_MODE_EMCC45		3

#define SATA_MODE_IDE			0
#define SATA_MODE_AHCI			1

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

#define OS_SELECTION_ANDROID		1
#define OS_SELECTION_LINUX		4

#define DRAM_SPEED_800MTS		0
#define DRAM_SPEED_1066MTS		1
#define DRAM_SPEED_1333MTS		2
#define DRAM_SPEED_1600MTS		3

#define DRAM_TYPE_DDR3			0
#define DRAM_TYPE_DDR3L			1
#define DRAM_TYPE_DDR3ECC		2
#define DRAM_TYPE_LPDDR2		4
#define DRAM_TYPE_LPDDR3		5
#define DRAM_TYPE_DDR4			6

#define DIMM_WIDTH_X8			0
#define DIMM_WIDTH_X16			1
#define DIMM_WIDTH_X32			2

#define DIMM_DENSITY_1GBIT		0
#define DIMM_DENSITY_2GBIT		1
#define DIMM_DENSITY_4GBIT		2
#define DIMM_DENSITY_8GBIT		3

#define DIMM_BUS_WIDTH_8BITS		0
#define DIMM_BUS_WIDTH_16BITS		1
#define DIMM_BUS_WIDTH_32BITS		2
#define DIMM_BUS_WIDTH_64BITS		3

#define DIMM_SIDES_1RANKS		0
#define DIMM_SIDES_2RANKS		1

#define LPE_MODE_DISABLED		0
#define LPE_MODE_PCI			1
#define LPE_MODE_ACPI			2

#define LPSS_SIO_MODE_ACPI		0
#define LPSS_SIO_MODE_PCI		1

#define SCC_MODE_ACPI			0
#define SCC_MODE_PCI			1

#endif /* __FSP_CONFIGS_H__ */
