// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-2020 Toradex
 */

#include <common.h>
#include <asm/global_data.h>
#include "tdx-cfg-block.h"
#include "tdx-eeprom.h"

#include <command.h>
#include <asm/cache.h>

#include <cli.h>
#include <console.h>
#include <env.h>
#ifdef CONFIG_TDX_CFG_BLOCK_IS_IN_NOR
#include <flash.h>
#endif
#include <malloc.h>
#include <mmc.h>
#include <nand.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

#define TAG_VALID	0xcf01
#define TAG_MAC		0x0000
#define TAG_CAR_SERIAL	0x0021
#define TAG_HW		0x0008
#define TAG_INVALID	0xffff

#define TAG_FLAG_VALID	0x1

#define TDX_EEPROM_ID_MODULE		0
#define TDX_EEPROM_ID_CARRIER		1

#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_MMC)
#define TDX_CFG_BLOCK_MAX_SIZE 512
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#else
#error Toradex config block location not set
#endif

#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
#define TDX_CFG_BLOCK_EXTRA_MAX_SIZE 64
#endif

struct toradex_tag {
	u32 len:14;
	u32 flags:2;
	u32 id:16;
};

bool valid_cfgblock;
struct toradex_hw tdx_hw_tag;
struct toradex_eth_addr tdx_eth_addr;
u32 tdx_serial;
#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
u32 tdx_car_serial;
bool valid_cfgblock_carrier;
struct toradex_hw tdx_car_hw_tag;
#endif

#define TARGET_IS_ENABLED(x) IS_ENABLED(CONFIG_TARGET_ ## x)

const struct toradex_som toradex_modules[] = {
	 [0] = { "UNKNOWN MODULE",                       0                                  },
	 [1] = { "Colibri PXA270 312MHz",                0                                  },
	 [2] = { "Colibri PXA270 520MHz",                0                                  },
	 [3] = { "Colibri PXA320 806MHz",                0                                  },
	 [4] = { "Colibri PXA300 208MHz",                0                                  },
	 [5] = { "Colibri PXA310 624MHz",                0                                  },
	 [6] = { "Colibri PXA320IT 806MHz",              0                                  },
	 [7] = { "Colibri PXA300 208MHz XT",             0                                  },
	 [8] = { "Colibri PXA270 312MHz",                0                                  },
	 [9] = { "Colibri PXA270 520MHz",                0                                  },
	[10] = { "Colibri VF50 128MB",                   TARGET_IS_ENABLED(COLIBRI_VF)      },
	[11] = { "Colibri VF61 256MB",                   TARGET_IS_ENABLED(COLIBRI_VF)      },
	[12] = { "Colibri VF61 256MB IT",                TARGET_IS_ENABLED(COLIBRI_VF)      },
	[13] = { "Colibri VF50 128MB IT",                TARGET_IS_ENABLED(COLIBRI_VF)      },
	[14] = { "Colibri iMX6S 256MB",                  TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[15] = { "Colibri iMX6DL 512MB",                 TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[16] = { "Colibri iMX6S 256MB IT",               TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[17] = { "Colibri iMX6DL 512MB IT",              TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[18] = { "UNKNOWN MODULE",                       0                                  },
	[19] = { "UNKNOWN MODULE",                       0                                  },
	[20] = { "Colibri T20 256MB",                    TARGET_IS_ENABLED(COLIBRI_T20)     },
	[21] = { "Colibri T20 512MB",                    TARGET_IS_ENABLED(COLIBRI_T20)     },
	[22] = { "Colibri T20 512MB IT",                 TARGET_IS_ENABLED(COLIBRI_T20)     },
	[23] = { "Colibri T30 1GB",                      TARGET_IS_ENABLED(COLIBRI_T30)     },
	[24] = { "Colibri T20 256MB IT",                 TARGET_IS_ENABLED(COLIBRI_T20)     },
	[25] = { "Apalis T30 2GB",                       TARGET_IS_ENABLED(APALIS_T30)      },
	[26] = { "Apalis T30 1GB",                       TARGET_IS_ENABLED(APALIS_T30)      },
	[27] = { "Apalis iMX6Q 1GB",                     TARGET_IS_ENABLED(APALIS_IMX6)     },
	[28] = { "Apalis iMX6Q 2GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[29] = { "Apalis iMX6D 512MB",                   TARGET_IS_ENABLED(APALIS_IMX6)     },
	[30] = { "Colibri T30 1GB IT",                   TARGET_IS_ENABLED(COLIBRI_T30)     },
	[31] = { "Apalis T30 1GB IT",                    TARGET_IS_ENABLED(APALIS_T30)      },
	[32] = { "Colibri iMX7S 256MB",                  TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[33] = { "Colibri iMX7D 512MB",                  TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[34] = { "Apalis TK1 2GB",                       TARGET_IS_ENABLED(APALIS_TK1)      },
	[35] = { "Apalis iMX6D 1GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[36] = { "Colibri iMX6ULL 256MB",                TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[37] = { "Apalis iMX8QM 4GB WB IT",              TARGET_IS_ENABLED(APALIS_IMX8)     },
	[38] = { "Colibri iMX8QXP 2GB WB IT",            TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[39] = { "Colibri iMX7D 1GB",                    TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[40] = { "Colibri iMX6ULL 512MB WB IT",          TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[41] = { "Colibri iMX7D 512MB EPDC",             TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[42] = { "Apalis TK1 4GB",                       TARGET_IS_ENABLED(APALIS_TK1)      },
	[43] = { "Colibri T20 512MB IT SETEK",           TARGET_IS_ENABLED(COLIBRI_T20)     },
	[44] = { "Colibri iMX6ULL 512MB IT",             TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[45] = { "Colibri iMX6ULL 512MB WB",             TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[46] = { "Apalis iMX8QXP 2GB WB IT",             0                                  },
	[47] = { "Apalis iMX8QM 4GB IT",                 TARGET_IS_ENABLED(APALIS_IMX8)     },
	[48] = { "Apalis iMX8QP 2GB WB",                 TARGET_IS_ENABLED(APALIS_IMX8)     },
	[49] = { "Apalis iMX8QP 2GB",                    TARGET_IS_ENABLED(APALIS_IMX8)     },
	[50] = { "Colibri iMX8QXP 2GB IT",               TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[51] = { "Colibri iMX8DX 1GB WB",                TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[52] = { "Colibri iMX8DX 1GB",                   TARGET_IS_ENABLED(COLIBRI_IMX8X)   },
	[53] = { "Apalis iMX8QXP 2GB ECC IT",            0                                  },
	[54] = { "Apalis iMX8DXP 1GB",                   TARGET_IS_ENABLED(APALIS_IMX8)     },
	[55] = { "Verdin iMX8M Mini Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[56] = { "Verdin iMX8M Nano Quad 1GB WB",        0                                  },
	[57] = { "Verdin iMX8M Mini DualLite 1GB",       TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[58] = { "Verdin iMX8M Plus Quad 4GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[59] = { "Verdin iMX8M Mini Quad 2GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[60] = { "Verdin iMX8M Mini DualLite 1GB WB IT", TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[61] = { "Verdin iMX8M Plus Quad 2GB",           TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[62] = { "Colibri iMX6ULL 1GB IT",               TARGET_IS_ENABLED(COLIBRI_IMX6ULL) },
	[63] = { "Verdin iMX8M Plus Quad 4GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[64] = { "Verdin iMX8M Plus Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[65] = { "Verdin iMX8M Plus QuadLite 1GB IT",    TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[66] = { "Verdin iMX8M Plus Quad 8GB WB",        TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[67] = { "Apalis iMX8QM 8GB WB IT",              TARGET_IS_ENABLED(APALIS_IMX8)     },
	[68] = { "Verdin iMX8M Mini Quad 2GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[69] = { "Verdin AM62 Quad 1GB WB IT",           TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[70] = { "Verdin iMX8M Plus Quad 8GB WB IT",     TARGET_IS_ENABLED(VERDIN_IMX8MP)   },
	[71] = { "Verdin AM62 Solo 512MB",               TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[72] = { "Verdin AM62 Solo 512MB WB IT",         TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[73] = { "Verdin AM62 Dual 1GB ET",              TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[74] = { "Verdin AM62 Dual 1GB IT",              TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[75] = { "Verdin AM62 Dual 1GB WB IT",           TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[76] = { "Verdin AM62 Quad 2GB WB IT",           TARGET_IS_ENABLED(VERDIN_AM62_A53) },
	[77] = { "Colibri iMX6S 256MB",                  TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[78] = { "Colibri iMX6S 256MB IT",               TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[79] = { "Colibri iMX6DL 512MB",                 TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[80] = { "Colibri iMX6DL 512MB IT",              TARGET_IS_ENABLED(COLIBRI_IMX6)    },
	[81] = { "Colibri iMX7D 512MB",                  TARGET_IS_ENABLED(COLIBRI_IMX7)    },
	[82] = { "Apalis iMX6D 512MB",                   TARGET_IS_ENABLED(APALIS_IMX6)     },
	[83] = { "Apalis iMX6Q 1GB",                     TARGET_IS_ENABLED(APALIS_IMX6)     },
	[84] = { "Apalis iMX6D 1GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[85] = { "Apalis iMX6Q 2GB IT",                  TARGET_IS_ENABLED(APALIS_IMX6)     },
	[86] = { "Verdin iMX8M Mini DualLite 2GB IT",    TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
	[87] = { "Verdin iMX8M Mini Quad 2GB IT",        TARGET_IS_ENABLED(VERDIN_IMX8MM)   },
};

struct pid4list {
	int pid4;
	char * const name;
};

const struct pid4list toradex_carrier_boards[] = {
	/* the code assumes unknown at index 0 */
	{0,				"UNKNOWN CARRIER BOARD"},
	{DAHLIA,			"Dahlia"},
	{VERDIN_DEVELOPMENT_BOARD,	"Verdin Development Board"},
	{YAVIA,				"Yavia"},
};

const struct pid4list toradex_display_adapters[] = {
	/* the code assumes unknown at index 0 */
	{0,				"UNKNOWN DISPLAY ADAPTER"},
	{VERDIN_DSI_TO_HDMI_ADAPTER,	"Verdin DSI to HDMI Adapter"},
	{VERDIN_DSI_TO_LVDS_ADAPTER,	"Verdin DSI to LVDS Adapter"},
};

const u32 toradex_ouis[] = {
	[0] = 0x00142dUL,
	[1] = 0x8c06cbUL,
};

const char * const get_toradex_carrier_boards(int pid4)
{
	int i, index = 0;

	for (i = 1; i < ARRAY_SIZE(toradex_carrier_boards); i++) {
		if (pid4 == toradex_carrier_boards[i].pid4) {
			index = i;
			break;
		}
	}
	return toradex_carrier_boards[index].name;
}

const char * const get_toradex_display_adapters(int pid4)
{
	int i, index = 0;

	for (i = 1; i < ARRAY_SIZE(toradex_display_adapters); i++) {
		if (pid4 == toradex_display_adapters[i].pid4) {
			index = i;
			break;
		}
	}
	return toradex_display_adapters[index].name;
}

static u32 get_serial_from_mac(struct toradex_eth_addr *eth_addr)
{
	int i;
	u32 oui = ntohl(eth_addr->oui) >> 8;
	u32 nic = ntohl(eth_addr->nic) >> 8;

	for (i = 0; i < ARRAY_SIZE(toradex_ouis); i++) {
		if (toradex_ouis[i] == oui)
			break;
	}

	return (u32)((i << 24) + nic);
}

void get_mac_from_serial(u32 tdx_serial, struct toradex_eth_addr *eth_addr)
{
	u8 oui_index = tdx_serial >> 24;
	u32 nic = tdx_serial & GENMASK(23, 0);
	u32 oui;

	if (oui_index >= ARRAY_SIZE(toradex_ouis)) {
		puts("Can't find OUI for this serial#\n");
		oui_index = 0;
	}

	oui = toradex_ouis[oui_index];

	eth_addr->oui = htonl(oui << 8);
	eth_addr->nic = htonl(nic << 8);
}

#ifdef CONFIG_TDX_CFG_BLOCK_IS_IN_MMC
static int tdx_cfg_block_mmc_storage(u8 *config_block, int write)
{
	struct mmc *mmc;
	int dev = CONFIG_TDX_CFG_BLOCK_DEV;
	int offset = CONFIG_TDX_CFG_BLOCK_OFFSET;
	uint part = CONFIG_TDX_CFG_BLOCK_PART;
	uint blk_start;
	int ret = 0;

	/* Read production parameter config block from eMMC */
	mmc = find_mmc_device(dev);
	if (!mmc) {
		puts("No MMC card found\n");
		ret = -ENODEV;
		goto out;
	}
	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -EINVAL;
	}
	if (part != mmc_get_blk_desc(mmc)->hwpart) {
		if (blk_select_hwpart_devnum(UCLASS_MMC, dev, part)) {
			puts("MMC partition switch failed\n");
			ret = -ENODEV;
			goto out;
		}
	}
	if (offset < 0)
		offset += mmc->capacity;
	blk_start = ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;

	if (!write) {
		/* Careful reads a whole block of 512 bytes into config_block */
		if (blk_dread(mmc_get_blk_desc(mmc), blk_start, 1,
			      (unsigned char *)config_block) != 1) {
			ret = -EIO;
			goto out;
		}
	} else {
		/* Just writing one 512 byte block */
		if (blk_dwrite(mmc_get_blk_desc(mmc), blk_start, 1,
			       (unsigned char *)config_block) != 1) {
			ret = -EIO;
			goto out;
		}
	}

out:
	/* Switch back to regular eMMC user partition */
	blk_select_hwpart_devnum(UCLASS_MMC, 0, 0);

	return ret;
}
#endif

#ifdef CONFIG_TDX_CFG_BLOCK_IS_IN_NAND
static int read_tdx_cfg_block_from_nand(unsigned char *config_block)
{
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;
	struct mtd_info *mtd = get_nand_dev_by_index(0);

	if (!mtd)
		return -ENODEV;

	/* Read production parameter config block from NAND page */
	return nand_read_skip_bad(mtd, CONFIG_TDX_CFG_BLOCK_OFFSET,
				  &size, NULL, TDX_CFG_BLOCK_MAX_SIZE,
				  config_block);
}

static int write_tdx_cfg_block_to_nand(unsigned char *config_block)
{
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;

	/* Write production parameter config block to NAND page */
	return nand_write_skip_bad(get_nand_dev_by_index(0),
				   CONFIG_TDX_CFG_BLOCK_OFFSET,
				   &size, NULL, TDX_CFG_BLOCK_MAX_SIZE,
				   config_block, WITH_WR_VERIFY);
}
#endif

#ifdef CONFIG_TDX_CFG_BLOCK_IS_IN_NOR
static int read_tdx_cfg_block_from_nor(unsigned char *config_block)
{
	/* Read production parameter config block from NOR flash */
	memcpy(config_block, (void *)CONFIG_TDX_CFG_BLOCK_OFFSET,
	       TDX_CFG_BLOCK_MAX_SIZE);
	return 0;
}

static int write_tdx_cfg_block_to_nor(unsigned char *config_block)
{
	/* Write production parameter config block to NOR flash */
	return flash_write((void *)config_block, CONFIG_TDX_CFG_BLOCK_OFFSET,
			   TDX_CFG_BLOCK_MAX_SIZE);
}
#endif

#ifdef CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM
static int read_tdx_cfg_block_from_eeprom(unsigned char *config_block)
{
	return read_tdx_eeprom_data(TDX_EEPROM_ID_MODULE, 0x0, config_block,
				    TDX_CFG_BLOCK_MAX_SIZE);
}

static int write_tdx_cfg_block_to_eeprom(unsigned char *config_block)
{
	return write_tdx_eeprom_data(TDX_EEPROM_ID_MODULE, 0x0, config_block,
				     TDX_CFG_BLOCK_MAX_SIZE);
}
#endif

int read_tdx_cfg_block(void)
{
	int ret = 0;
	u8 *config_block = NULL;
	struct toradex_tag *tag;
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;
	int offset;

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return -ENOMEM;
	}

	memset(config_block, 0, size);

#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_MMC)
	ret = tdx_cfg_block_mmc_storage(config_block, 0);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
	ret = read_tdx_cfg_block_from_nand(config_block);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
	ret = read_tdx_cfg_block_from_nor(config_block);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM)
	ret = read_tdx_cfg_block_from_eeprom(config_block);
#else
	ret = -EINVAL;
#endif
	if (ret)
		goto out;

	/* Expect a valid tag first */
	tag = (struct toradex_tag *)config_block;
	if (tag->flags != TAG_FLAG_VALID || tag->id != TAG_VALID) {
		valid_cfgblock = false;
		ret = -EINVAL;
		goto out;
	}
	valid_cfgblock = true;
	offset = 4;

	/*
	 * check if there is enough space for storing tag and value of the
	 * biggest element
	 */
	while (offset + sizeof(struct toradex_tag) +
	       sizeof(struct toradex_hw) < TDX_CFG_BLOCK_MAX_SIZE) {
		tag = (struct toradex_tag *)(config_block + offset);
		offset += 4;
		if (tag->id == TAG_INVALID)
			break;

		if (tag->flags == TAG_FLAG_VALID) {
			switch (tag->id) {
			case TAG_MAC:
				memcpy(&tdx_eth_addr, config_block + offset,
				       6);

				tdx_serial = get_serial_from_mac(&tdx_eth_addr);
				break;
			case TAG_HW:
				memcpy(&tdx_hw_tag, config_block + offset, 8);
				break;
			}
		}

		/* Get to next tag according to current tags length */
		offset += tag->len * 4;
	}

	/* Cap product id to avoid issues with a yet unknown one */
	if (tdx_hw_tag.prodid >= ARRAY_SIZE(toradex_modules))
		tdx_hw_tag.prodid = 0;

out:
	free(config_block);
	return ret;
}

static int parse_assembly_string(char *string_to_parse, u16 *assembly)
{
	if (string_to_parse[3] >= 'A' && string_to_parse[3] <= 'Z')
		*assembly = string_to_parse[3] - 'A';
	else if (string_to_parse[3] == '#')
		*assembly = dectoul(&string_to_parse[4], NULL);
	else
		return -EINVAL;

	return 0;
}

static int get_cfgblock_interactive(void)
{
	char message[CONFIG_SYS_CBSIZE];
	int len = 0;
	int ret = 0;
	unsigned int prodid;
	int i;

	printf("Enabled modules:\n");
	for (i = 0; i < ARRAY_SIZE(toradex_modules); i++) {
		if (toradex_modules[i].is_enabled)
			printf(" %04d %s\n", i, toradex_modules[i].name);
	}

	sprintf(message, "Enter the module ID: ");
	len = cli_readline(message);

	prodid = dectoul(console_buffer, NULL);
	if (prodid >= ARRAY_SIZE(toradex_modules) || !toradex_modules[prodid].is_enabled) {
		printf("Parsing module id failed\n");
		return -1;
	}
	tdx_hw_tag.prodid = prodid;

	len = 0;
	while (len < 4) {
		sprintf(message, "Enter the module version (e.g. V1.1B or V1.1#26): V");
		len = cli_readline(message);
	}

	tdx_hw_tag.ver_major = console_buffer[0] - '0';
	tdx_hw_tag.ver_minor = console_buffer[2] - '0';

	ret = parse_assembly_string(console_buffer, &tdx_hw_tag.ver_assembly);
	if (ret) {
		printf("Parsing module version failed\n");
		return ret;
	}

	while (len < 8) {
		sprintf(message, "Enter module serial number: ");
		len = cli_readline(message);
	}

	tdx_serial = dectoul(console_buffer, NULL);

	return 0;
}

static int get_cfgblock_barcode(char *barcode, struct toradex_hw *tag,
				u32 *serial)
{
	char revision[3] = {barcode[6], barcode[7], '\0'};

	if (strlen(barcode) < 16) {
		printf("Argument too short, barcode is 16 chars long\n");
		return -1;
	}

	/* Get hardware information from the first 8 digits */
	tag->ver_major = barcode[4] - '0';
	tag->ver_minor = barcode[5] - '0';
	tag->ver_assembly = dectoul(revision, NULL);

	barcode[4] = '\0';
	tag->prodid = dectoul(barcode, NULL);

	/* Parse second part of the barcode (serial number */
	barcode += 8;
	*serial = dectoul(barcode, NULL);

	return 0;
}

static int write_tag(u8 *config_block, int *offset, int tag_id,
		     u8 *tag_data, size_t tag_data_size)
{
	struct toradex_tag *tag;

	if (!offset || !config_block)
		return -EINVAL;

	tag = (struct toradex_tag *)(config_block + *offset);
	tag->id = tag_id;
	tag->flags = TAG_FLAG_VALID;
	/* len is provided as number of 32bit values after the tag */
	tag->len = (tag_data_size + sizeof(u32) - 1) / sizeof(u32);
	*offset += sizeof(struct toradex_tag);
	if (tag_data && tag_data_size) {
		memcpy(config_block + *offset, tag_data,
		       tag_data_size);
		*offset += tag_data_size;
	}

	return 0;
}

#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
int read_tdx_cfg_block_carrier(void)
{
	int ret = 0;
	u8 *config_block = NULL;
	struct toradex_tag *tag;
	size_t size = TDX_CFG_BLOCK_EXTRA_MAX_SIZE;
	int offset;

	/* Allocate RAM area for carrier config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return -ENOMEM;
	}

	memset(config_block, 0, size);

	ret = read_tdx_eeprom_data(TDX_EEPROM_ID_CARRIER, 0x0, config_block,
				   size);
	if (ret)
		return ret;

	/* Expect a valid tag first */
	tag = (struct toradex_tag *)config_block;
	if (tag->flags != TAG_FLAG_VALID || tag->id != TAG_VALID) {
		valid_cfgblock_carrier = false;
		ret = -EINVAL;
		goto out;
	}
	valid_cfgblock_carrier = true;
	offset = 4;

	while (offset + sizeof(struct toradex_tag) +
	       sizeof(struct toradex_hw) < TDX_CFG_BLOCK_MAX_SIZE) {
		tag = (struct toradex_tag *)(config_block + offset);
		offset += 4;
		if (tag->id == TAG_INVALID)
			break;

		if (tag->flags == TAG_FLAG_VALID) {
			switch (tag->id) {
			case TAG_CAR_SERIAL:
				memcpy(&tdx_car_serial, config_block + offset,
				       sizeof(tdx_car_serial));
				break;
			case TAG_HW:
				memcpy(&tdx_car_hw_tag, config_block +
				       offset, 8);
				break;
			}
		}

		/* Get to next tag according to current tags length */
		offset += tag->len * 4;
	}
out:
	free(config_block);
	return ret;
}

int check_pid8_sanity(char *pid8)
{
	char s_carrierid_verdin_dev[5];
	char s_carrierid_dahlia[5];

	sprintf(s_carrierid_verdin_dev, "0%d", VERDIN_DEVELOPMENT_BOARD);
	sprintf(s_carrierid_dahlia, "0%d", DAHLIA);

	/* sane value check, first 4 chars which represent carrier id */
	if (!strncmp(pid8, s_carrierid_verdin_dev, 4))
		return 0;

	if (!strncmp(pid8, s_carrierid_dahlia, 4))
		return 0;

	return -EINVAL;
}

int try_migrate_tdx_cfg_block_carrier(void)
{
	char pid8[8];
	int offset = 0;
	int ret = CMD_RET_SUCCESS;
	size_t size = TDX_CFG_BLOCK_EXTRA_MAX_SIZE;
	u8 *config_block;

	memset(pid8, 0x0, 8);
	ret = read_tdx_eeprom_data(TDX_EEPROM_ID_CARRIER, 0x0, (u8 *)pid8, 8);
	if (ret)
		return ret;

	if (check_pid8_sanity(pid8))
		return -EINVAL;

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return CMD_RET_FAILURE;
	}

	memset(config_block, 0xff, size);
	/* we try parse PID8 concatenating zeroed serial number */
	tdx_car_hw_tag.ver_major = pid8[4] - '0';
	tdx_car_hw_tag.ver_minor = pid8[5] - '0';
	tdx_car_hw_tag.ver_assembly = pid8[7] - '0';

	pid8[4] = '\0';
	tdx_car_hw_tag.prodid = dectoul(pid8, NULL);

	/* Valid Tag */
	write_tag(config_block, &offset, TAG_VALID, NULL, 0);

	/* Product Tag */
	write_tag(config_block, &offset, TAG_HW, (u8 *)&tdx_car_hw_tag,
		  sizeof(tdx_car_hw_tag));

	/* Serial Tag */
	write_tag(config_block, &offset, TAG_CAR_SERIAL, (u8 *)&tdx_car_serial,
		  sizeof(tdx_car_serial));

	memset(config_block + offset, 0, 32 - offset);
	ret = write_tdx_eeprom_data(TDX_EEPROM_ID_CARRIER, 0x0, config_block,
				    size);
	if (ret) {
		printf("Failed to write Toradex Extra config block: %d\n",
		       ret);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	printf("Successfully migrated to Toradex Config Block from PID8\n");

out:
	free(config_block);
	return ret;
}

static int get_cfgblock_carrier_interactive(void)
{
	char message[CONFIG_SYS_CBSIZE];
	int len;
	int ret = 0;

	printf("Supported carrier boards:\n");
	printf("%30s\t[ID]\n", "CARRIER BOARD NAME");
	for (int i = 0; i < ARRAY_SIZE(toradex_carrier_boards); i++)
		printf("%30s\t[%d]\n",
		       toradex_carrier_boards[i].name,
		       toradex_carrier_boards[i].pid4);

	sprintf(message, "Choose your carrier board (provide ID): ");
	len = cli_readline(message);
	tdx_car_hw_tag.prodid = dectoul(console_buffer, NULL);

	do {
		sprintf(message, "Enter carrier board version (e.g. V1.1B or V1.1#26): V");
		len = cli_readline(message);
	} while (len < 4);

	tdx_car_hw_tag.ver_major = console_buffer[0] - '0';
	tdx_car_hw_tag.ver_minor = console_buffer[2] - '0';

	ret = parse_assembly_string(console_buffer, &tdx_car_hw_tag.ver_assembly);
	if (ret) {
		printf("Parsing module version failed\n");
		return ret;
	}

	while (len < 8) {
		sprintf(message, "Enter carrier board serial number: ");
		len = cli_readline(message);
	}

	tdx_car_serial = dectoul(console_buffer, NULL);

	return 0;
}

static int do_cfgblock_carrier_create(struct cmd_tbl *cmdtp, int flag, int argc,
				      char * const argv[])
{
	u8 *config_block;
	size_t size = TDX_CFG_BLOCK_EXTRA_MAX_SIZE;
	int offset = 0;
	int ret = CMD_RET_SUCCESS;
	int err;
	int force_overwrite = 0;

	if (argc >= 3) {
		if (argv[2][0] == '-' && argv[2][1] == 'y')
			force_overwrite = 1;
	}

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return CMD_RET_FAILURE;
	}

	memset(config_block, 0xff, size);
	read_tdx_cfg_block_carrier();
	if (valid_cfgblock_carrier && !force_overwrite) {
		char message[CONFIG_SYS_CBSIZE];

		sprintf(message, "A valid Toradex Carrier config block is present, still recreate? [y/N] ");

		if (!cli_readline(message))
			goto out;

		if (console_buffer[0] != 'y' &&
		    console_buffer[0] != 'Y')
			goto out;
	}

	if (argc < 3 || (force_overwrite && argc < 4)) {
		err = get_cfgblock_carrier_interactive();
	} else {
		if (force_overwrite)
			err = get_cfgblock_barcode(argv[3], &tdx_car_hw_tag,
						   &tdx_car_serial);
		else
			err = get_cfgblock_barcode(argv[2], &tdx_car_hw_tag,
						   &tdx_car_serial);
	}

	if (err) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	/* Valid Tag */
	write_tag(config_block, &offset, TAG_VALID, NULL, 0);

	/* Product Tag */
	write_tag(config_block, &offset, TAG_HW, (u8 *)&tdx_car_hw_tag,
		  sizeof(tdx_car_hw_tag));

	/* Serial Tag */
	write_tag(config_block, &offset, TAG_CAR_SERIAL, (u8 *)&tdx_car_serial,
		  sizeof(tdx_car_serial));

	memset(config_block + offset, 0, 32 - offset);
	err = write_tdx_eeprom_data(TDX_EEPROM_ID_CARRIER, 0x0, config_block,
				    size);
	if (err) {
		printf("Failed to write Toradex Extra config block: %d\n",
		       ret);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	printf("Toradex Extra config block successfully written\n");

out:
	free(config_block);
	return ret;
}

#endif /* CONFIG_TDX_CFG_BLOCK_EXTRA */

static int do_cfgblock_create(struct cmd_tbl *cmdtp, int flag, int argc,
			      char * const argv[])
{
	u8 *config_block;
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;
	int offset = 0;
	int ret = CMD_RET_SUCCESS;
	int err;
	int force_overwrite = 0;

	if (argc >= 3) {
#ifdef CONFIG_TDX_CFG_BLOCK_EXTRA
		if (!strcmp(argv[2], "carrier"))
			return do_cfgblock_carrier_create(cmdtp, flag,
							  --argc, ++argv);
#endif /* CONFIG_TDX_CFG_BLOCK_EXTRA */
		if (argv[2][0] == '-' && argv[2][1] == 'y')
			force_overwrite = 1;
	}

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return CMD_RET_FAILURE;
	}

	memset(config_block, 0xff, size);

	read_tdx_cfg_block();
	if (valid_cfgblock) {
#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
		/*
		 * On NAND devices, recreation is only allowed if the page is
		 * empty (config block invalid...)
		 */
		printf("NAND erase block %d need to be erased before creating a Toradex config block\n",
		       CONFIG_TDX_CFG_BLOCK_OFFSET /
		       get_nand_dev_by_index(0)->erasesize);
		goto out;
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
		/*
		 * On NOR devices, recreation is only allowed if the sector is
		 * empty and write protection is off (config block invalid...)
		 */
		printf("NOR sector at offset 0x%02x need to be erased and unprotected before creating a Toradex config block\n",
		       CONFIG_TDX_CFG_BLOCK_OFFSET);
		goto out;
#else
		if (!force_overwrite) {
			char message[CONFIG_SYS_CBSIZE];

			sprintf(message,
				"A valid Toradex config block is present, still recreate? [y/N] ");

			if (!cli_readline(message))
				goto out;

			if (console_buffer[0] != 'y' &&
			    console_buffer[0] != 'Y')
				goto out;
		}
#endif
	}

	/* Parse new Toradex config block data... */
	if (argc < 3 || (force_overwrite && argc < 4)) {
		err = get_cfgblock_interactive();
	} else {
		if (force_overwrite)
			err = get_cfgblock_barcode(argv[3], &tdx_hw_tag,
						   &tdx_serial);
		else
			err = get_cfgblock_barcode(argv[2], &tdx_hw_tag,
						   &tdx_serial);
	}
	if (err) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	/* Convert serial number to MAC address (the storage format) */
	get_mac_from_serial(tdx_serial, &tdx_eth_addr);

	/* Valid Tag */
	write_tag(config_block, &offset, TAG_VALID, NULL, 0);

	/* Product Tag */
	write_tag(config_block, &offset, TAG_HW, (u8 *)&tdx_hw_tag,
		  sizeof(tdx_hw_tag));

	/* MAC Tag */
	write_tag(config_block, &offset, TAG_MAC, (u8 *)&tdx_eth_addr,
		  sizeof(tdx_eth_addr));

	memset(config_block + offset, 0, 32 - offset);
#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_MMC)
	err = tdx_cfg_block_mmc_storage(config_block, 1);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
	err = write_tdx_cfg_block_to_nand(config_block);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
	err = write_tdx_cfg_block_to_nor(config_block);
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM)
	err = write_tdx_cfg_block_to_eeprom(config_block);
#else
	err = -EINVAL;
#endif
	if (err) {
		printf("Failed to write Toradex config block: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	printf("Toradex config block successfully written\n");

out:
	free(config_block);
	return ret;
}

static int do_cfgblock(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "create")) {
		return do_cfgblock_create(cmdtp, flag, argc, argv);
	} else if (!strcmp(argv[1], "reload")) {
		ret = read_tdx_cfg_block();
		if (ret) {
			printf("Failed to reload Toradex config block: %d\n",
			       ret);
			return CMD_RET_FAILURE;
		}
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	cfgblock, 5, 0, do_cfgblock,
	"Toradex config block handling commands",
	"create [-y] [barcode] - (Re-)create Toradex config block\n"
	"create carrier [-y] [barcode] - (Re-)create Toradex Carrier config block\n"
	"cfgblock reload - Reload Toradex config block from flash"
);
