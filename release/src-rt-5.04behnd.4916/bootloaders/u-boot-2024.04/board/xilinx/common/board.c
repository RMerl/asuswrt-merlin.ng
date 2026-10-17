// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2022, Xilinx, Inc.
 * (C) Copyright 2022 - 2023, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <efi.h>
#include <efi_loader.h>
#include <env.h>
#include <image.h>
#include <init.h>
#include <lmb.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <malloc.h>
#include "board.h"
#include <dm.h>
#include <i2c_eeprom.h>
#include <net.h>
#include <generated/dt.h>
#include <rng.h>
#include <slre.h>
#include <soc.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <uuid.h>

#include "fru.h"

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
#if defined(XILINX_BOOT_IMAGE_GUID)
	{
		.image_type_id = XILINX_BOOT_IMAGE_GUID,
		.fw_name = u"XILINX-BOOT",
		.image_index = 1,
	},
#endif
#if defined(XILINX_UBOOT_IMAGE_GUID)
	{
		.image_type_id = XILINX_UBOOT_IMAGE_GUID,
		.fw_name = u"XILINX-UBOOT",
		.image_index = 2,
	},
#endif
};

struct efi_capsule_update_info update_info = {
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

#define EEPROM_HEADER_MAGIC		0xdaaddeed
#define EEPROM_HDR_MANUFACTURER_LEN	16
#define EEPROM_HDR_NAME_LEN		16
#define EEPROM_HDR_REV_LEN		8
#define EEPROM_HDR_SERIAL_LEN		20
#define EEPROM_HDR_NO_OF_MAC_ADDR	4
#define EEPROM_HDR_ETH_ALEN		ETH_ALEN
#define EEPROM_HDR_UUID_LEN		16

struct xilinx_board_description {
	u32 header;
	char manufacturer[EEPROM_HDR_MANUFACTURER_LEN + 1];
	char name[EEPROM_HDR_NAME_LEN + 1];
	char revision[EEPROM_HDR_REV_LEN + 1];
	char serial[EEPROM_HDR_SERIAL_LEN + 1];
	u8 mac_addr[EEPROM_HDR_NO_OF_MAC_ADDR][EEPROM_HDR_ETH_ALEN + 1];
	char uuid[EEPROM_HDR_UUID_LEN + 1];
};

static int highest_id = -1;
static struct xilinx_board_description *board_info;

#define XILINX_I2C_DETECTION_BITS	sizeof(struct fru_common_hdr)

/* Variable which stores pointer to array which stores eeprom content */
struct xilinx_legacy_format {
	char board_sn[18]; /* 0x0 */
	char unused0[14]; /* 0x12 */
	char eth_mac[ETH_ALEN]; /* 0x20 */
	char unused1[170]; /* 0x26 */
	char board_name[11]; /* 0xd0 */
	char unused2[5]; /* 0xdc */
	char board_revision[3]; /* 0xe0 */
	char unused3[29]; /* 0xe3 */
};

static void xilinx_eeprom_legacy_cleanup(char *eeprom, int size)
{
	int i;
	unsigned char byte;

	for (i = 0; i < size; i++) {
		byte = eeprom[i];

		/* Remove all non printable chars but ignore MAC address */
		if ((i < offsetof(struct xilinx_legacy_format, eth_mac) ||
		     i >= offsetof(struct xilinx_legacy_format, unused1)) &&
		     (byte < '!' || byte > '~')) {
			eeprom[i] = 0;
			continue;
		}

		/* Convert strings to lower case */
		if (byte >= 'A' && byte <= 'Z')
			eeprom[i] = byte + 'a' - 'A';
	}
}

static int xilinx_read_eeprom_legacy(struct udevice *dev, char *name,
				     struct xilinx_board_description *desc)
{
	int ret, size;
	struct xilinx_legacy_format *eeprom_content;
	bool eth_valid = false;

	size = sizeof(*eeprom_content);

	eeprom_content = calloc(1, size);
	if (!eeprom_content)
		return -ENOMEM;

	debug("%s: I2C EEPROM read pass data at %p\n", __func__,
	      eeprom_content);

	ret = dm_i2c_read(dev, 0, (uchar *)eeprom_content, size);
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		free(eeprom_content);
		return ret;
	}

	xilinx_eeprom_legacy_cleanup((char *)eeprom_content, size);

	/* Terminating \0 chars are the part of desc fields already */
	strlcpy(desc->name, eeprom_content->board_name,
		sizeof(eeprom_content->board_name) + 1);
	strlcpy(desc->revision, eeprom_content->board_revision,
		sizeof(eeprom_content->board_revision) + 1);
	strlcpy(desc->serial, eeprom_content->board_sn,
		sizeof(eeprom_content->board_sn) + 1);

	eth_valid = is_valid_ethaddr((const u8 *)eeprom_content->eth_mac);
	if (eth_valid)
		memcpy(desc->mac_addr[0], eeprom_content->eth_mac, ETH_ALEN);

	printf("Xilinx I2C Legacy format at %s:\n", name);
	printf(" Board name:\t%s\n", desc->name);
	printf(" Board rev:\t%s\n", desc->revision);
	printf(" Board SN:\t%s\n", desc->serial);

	if (eth_valid)
		printf(" Ethernet mac:\t%pM\n", desc->mac_addr);

	desc->header = EEPROM_HEADER_MAGIC;

	free(eeprom_content);

	return ret;
}

static bool xilinx_detect_legacy(u8 *buffer)
{
	int i;
	char c;

	for (i = 0; i < XILINX_I2C_DETECTION_BITS; i++) {
		c = buffer[i];

		if (c < '0' || c > '9')
			return false;
	}

	return true;
}

static int xilinx_read_eeprom_fru(struct udevice *dev, char *name,
				  struct xilinx_board_description *desc)
{
	int i, ret, eeprom_size;
	u8 *fru_content;
	u8 id = 0;

	/* FIXME this is shortcut - if eeprom type is wrong it will fail */
	eeprom_size = i2c_eeprom_size(dev);

	fru_content = calloc(1, eeprom_size);
	if (!fru_content)
		return -ENOMEM;

	debug("%s: I2C EEPROM read pass data at %p\n", __func__,
	      fru_content);

	ret = dm_i2c_read(dev, 0, (uchar *)fru_content,
			  eeprom_size);
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		goto end;
	}

	fru_capture((unsigned long)fru_content);
	if (gd->flags & GD_FLG_RELOC || (_DEBUG && IS_ENABLED(CONFIG_DTB_RESELECT))) {
		printf("Xilinx I2C FRU format at %s:\n", name);
		ret = fru_display(0);
		if (ret) {
			printf("FRU format decoding failed.\n");
			goto end;
		}
	}

	if (desc->header == EEPROM_HEADER_MAGIC) {
		debug("Information already filled\n");
		ret = -EINVAL;
		goto end;
	}

	/* It is clear that FRU was captured and structures were filled */
	strlcpy(desc->manufacturer, (char *)fru_data.brd.manufacturer_name,
		sizeof(desc->manufacturer));
	strlcpy(desc->uuid, (char *)fru_data.brd.uuid,
		sizeof(desc->uuid));
	strlcpy(desc->name, (char *)fru_data.brd.product_name,
		sizeof(desc->name));
	for (i = 0; i < sizeof(desc->name); i++) {
		if (desc->name[i] == ' ')
			desc->name[i] = '\0';
	}
	strlcpy(desc->revision, (char *)fru_data.brd.rev,
		sizeof(desc->revision));
	for (i = 0; i < sizeof(desc->revision); i++) {
		if (desc->revision[i] == ' ')
			desc->revision[i] = '\0';
	}
	strlcpy(desc->serial, (char *)fru_data.brd.serial_number,
		sizeof(desc->serial));

	while (id < EEPROM_HDR_NO_OF_MAC_ADDR) {
		if (is_valid_ethaddr((const u8 *)fru_data.mac.macid[id]))
			memcpy(&desc->mac_addr[id],
			       (char *)fru_data.mac.macid[id], ETH_ALEN);
		id++;
	}

	desc->header = EEPROM_HEADER_MAGIC;

end:
	free(fru_content);
	return ret;
}

static bool xilinx_detect_fru(u8 *buffer)
{
	u8 checksum = 0;
	int i;

	checksum = fru_checksum((u8 *)buffer, sizeof(struct fru_common_hdr));
	if (checksum) {
		debug("%s Common header CRC FAIL\n", __func__);
		return false;
	}

	bool all_zeros = true;
	/* Checksum over all zeros is also zero that's why detect this case */
	for (i = 0; i < sizeof(struct fru_common_hdr); i++) {
		if (buffer[i] != 0)
			all_zeros = false;
	}

	if (all_zeros)
		return false;

	debug("%s Common header CRC PASS\n", __func__);
	return true;
}

static int xilinx_read_eeprom_single(char *name,
				     struct xilinx_board_description *desc)
{
	int ret;
	struct udevice *dev;
	ofnode eeprom;
	u8 buffer[XILINX_I2C_DETECTION_BITS];

	eeprom = ofnode_get_aliases_node(name);
	if (!ofnode_valid(eeprom))
		return -ENODEV;

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, eeprom, &dev);
	if (ret)
		return ret;

	ret = dm_i2c_read(dev, 0, buffer, sizeof(buffer));
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		return ret;
	}

	debug("%s: i2c memory detected: %s\n", __func__, name);

	if (IS_ENABLED(CONFIG_CMD_FRU) && xilinx_detect_fru(buffer))
		return xilinx_read_eeprom_fru(dev, name, desc);

	if (xilinx_detect_legacy(buffer))
		return xilinx_read_eeprom_legacy(dev, name, desc);

	return -ENODEV;
}

__maybe_unused int xilinx_read_eeprom(void)
{
	int id;
	char name_buf[8]; /* 8 bytes should be enough for nvmem+number */
	struct xilinx_board_description *desc;

	highest_id = dev_read_alias_highest_id("nvmem");
	/* No nvmem aliases present */
	if (highest_id < 0)
		return -EINVAL;

	board_info = calloc(1, sizeof(*desc) * (highest_id + 1));
	if (!board_info)
		return -ENOMEM;

	debug("%s: Highest ID %d, board_info %p\n", __func__,
	      highest_id, board_info);

	for (id = 0; id <= highest_id; id++) {
		snprintf(name_buf, sizeof(name_buf), "nvmem%d", id);

		/* Alloc structure */
		desc = &board_info[id];

		/* Ignoring return value for supporting multiple chips */
		xilinx_read_eeprom_single(name_buf, desc);
	}

	/*
	 * Consider to clean board_info structure when board/cards are not
	 * detected.
	 */

	return 0;
}

#if defined(CONFIG_OF_BOARD)
void *board_fdt_blob_setup(int *err)
{
	void *fdt_blob;

	*err = 0;
	if (!IS_ENABLED(CONFIG_SPL_BUILD) &&
	    !IS_ENABLED(CONFIG_VERSAL_NO_DDR) &&
	    !IS_ENABLED(CONFIG_ZYNQMP_NO_DDR)) {
		fdt_blob = (void *)CONFIG_XILINX_OF_BOARD_DTB_ADDR;

		if (fdt_magic(fdt_blob) == FDT_MAGIC)
			return fdt_blob;

		debug("DTB is not passed via %p\n", fdt_blob);
	}

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		/*
		 * FDT is at end of BSS unless it is in a different memory
		 * region
		 */
		if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS))
			fdt_blob = (ulong *)_image_binary_end;
		else
			fdt_blob = (ulong *)__bss_end;
	} else {
		/* FDT is at end of image */
		fdt_blob = (ulong *)_end;
	}

	if (fdt_magic(fdt_blob) == FDT_MAGIC)
		return fdt_blob;

	debug("DTB is also not passed via %p\n", fdt_blob);

	*err = -EINVAL;
	return NULL;
}
#endif

#if defined(CONFIG_BOARD_LATE_INIT)
static int env_set_by_index(const char *name, int index, char *data)
{
	char var[32];

	if (!index)
		sprintf(var, "board_%s", name);
	else
		sprintf(var, "card%d_%s", index, name);

	return env_set(var, data);
}

int board_late_init_xilinx(void)
{
	u32 ret = 0;
	int i, id, macid = 0;
	struct xilinx_board_description *desc;
	phys_size_t bootm_size = gd->ram_top - gd->ram_base;
	u64 bootscr_flash_offset, bootscr_flash_size;

	if (!IS_ENABLED(CONFIG_MICROBLAZE)) {
		ulong scriptaddr;
		u64 bootscr_address;
		u64 bootscr_offset;

		/* Fetch bootscr_address/bootscr_offset from DT and update */
		if (!ofnode_read_bootscript_address(&bootscr_address,
						    &bootscr_offset)) {
			if (bootscr_offset)
				ret |= env_set_hex("scriptaddr",
						   gd->ram_base +
						   bootscr_offset);
			else
				ret |= env_set_hex("scriptaddr",
						   bootscr_address);
		} else {
			/* Update scriptaddr(bootscr offset) from env */
			scriptaddr = env_get_hex("scriptaddr", 0);
			ret |= env_set_hex("scriptaddr",
					   gd->ram_base + scriptaddr);
		}
	}

	if (!ofnode_read_bootscript_flash(&bootscr_flash_offset,
					  &bootscr_flash_size)) {
		ret |= env_set_hex("script_offset_f", bootscr_flash_offset);
		ret |= env_set_hex("script_size_f", bootscr_flash_size);
	} else {
		debug("!!! Please define bootscr-flash-offset via DT !!!\n");
		ret |= env_set_hex("script_offset_f",
				   CONFIG_BOOT_SCRIPT_OFFSET);
	}

	if (IS_ENABLED(CONFIG_ARCH_ZYNQ) || IS_ENABLED(CONFIG_MICROBLAZE))
		bootm_size = min(bootm_size, (phys_size_t)(SZ_512M + SZ_256M));

	ret |= env_set_addr("bootm_low", (void *)gd->ram_base);
	ret |= env_set_addr("bootm_size", (void *)bootm_size);

	for (id = 0; id <= highest_id; id++) {
		desc = &board_info[id];
		if (desc && desc->header == EEPROM_HEADER_MAGIC) {
			if (desc->manufacturer[0])
				ret |= env_set_by_index("manufacturer", id,
							desc->manufacturer);
			if (desc->name[0])
				ret |= env_set_by_index("name", id,
							desc->name);
			if (desc->revision[0])
				ret |= env_set_by_index("rev", id,
							desc->revision);
			if (desc->serial[0])
				ret |= env_set_by_index("serial", id,
							desc->serial);

			if (desc->uuid[0]) {
				unsigned char uuid[UUID_STR_LEN + 1];
				unsigned char *t = desc->uuid;

				memset(uuid, 0, UUID_STR_LEN + 1);

				sprintf(uuid, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					t[0], t[1], t[2], t[3], t[4], t[5],
					t[6], t[7], t[8], t[9], t[10], t[11],
					t[12], t[13], t[14], t[15]);
				ret |= env_set_by_index("uuid", id, uuid);
			}

			if (!CONFIG_IS_ENABLED(NET))
				continue;

			for (i = 0; i < EEPROM_HDR_NO_OF_MAC_ADDR; i++) {
				if (is_valid_ethaddr((const u8 *)desc->mac_addr[i]))
					ret |= eth_env_set_enetaddr_by_index("eth",
							macid++, desc->mac_addr[i]);
			}
		}
	}

	if (ret)
		printf("%s: Saving run time variables FAILED\n", __func__);

	return 0;
}
#endif

static char *board_name = DEVICE_TREE;

int __maybe_unused board_fit_config_name_match(const char *name)
{
	debug("%s: Check %s, default %s\n", __func__, name, board_name);

#if !defined(CONFIG_SPL_BUILD)
	if (IS_ENABLED(CONFIG_REGEX)) {
		struct slre slre;
		int ret;

		ret = slre_compile(&slre, name);
		if (ret) {
			ret = slre_match(&slre, board_name, strlen(board_name),
					 NULL);
			debug("%s: name match ret = %d\n", __func__,  ret);
			return !ret;
		}
	}
#endif

	if (!strcmp(name, board_name))
		return 0;

	return -1;
}

#if IS_ENABLED(CONFIG_DTB_RESELECT)
#define MAX_NAME_LENGTH	50

char * __maybe_unused __weak board_name_decode(void)
{
	char *board_local_name;
	struct xilinx_board_description *desc;
	int i, id;

	board_local_name = calloc(1, MAX_NAME_LENGTH);
	if (!board_info)
		return NULL;

	for (id = 0; id <= highest_id; id++) {
		desc = &board_info[id];

		/* No board description */
		if (!desc)
			goto error;

		/* Board is not detected */
		if (desc->header != EEPROM_HEADER_MAGIC)
			continue;

		/* The first string should be soc name */
		if (!id)
			strcat(board_local_name, CONFIG_SYS_BOARD);

		/*
		 * For two purpose here:
		 * soc_name- eg: zynqmp-
		 * and between base board and CC eg: ..revA-sck...
		 */
		strcat(board_local_name, "-");

		if (desc->name[0]) {
			/* For DT composition name needs to be lowercase */
			for (i = 0; i < sizeof(desc->name); i++)
				desc->name[i] = tolower(desc->name[i]);

			strcat(board_local_name, desc->name);
		}
		if (desc->revision[0]) {
			strcat(board_local_name, "-rev");

			/* And revision needs to be uppercase */
			for (i = 0; i < sizeof(desc->revision); i++)
				desc->revision[i] = toupper(desc->revision[i]);

			strcat(board_local_name, desc->revision);
		}
	}

	/*
	 * Longer strings will end up with buffer overflow and potential
	 * attacks that's why check it
	 */
	if (strlen(board_local_name) >= MAX_NAME_LENGTH)
		panic("Board name can't be determined\n");

	if (strlen(board_local_name))
		return board_local_name;

error:
	free(board_local_name);
	return NULL;
}

bool __maybe_unused __weak board_detection(void)
{
	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM)) {
		int ret;

		ret = xilinx_read_eeprom();
		return !ret ? true : false;
	}

	return false;
}

bool __maybe_unused __weak soc_detection(void)
{
	return false;
}

char * __maybe_unused __weak soc_name_decode(void)
{
	return NULL;
}

int embedded_dtb_select(void)
{
	if (soc_detection()) {
		char *soc_local_name;

		soc_local_name = soc_name_decode();
		if (soc_local_name) {
			board_name = soc_local_name;
			printf("Detected SOC name: %s\n", board_name);

			/* Time to change DTB on fly */
			/* Both ways should work here */
			/* fdtdec_resetup(&rescan); */
			return fdtdec_setup();
		}
	}

	if (board_detection()) {
		char *board_local_name;

		board_local_name = board_name_decode();
		if (board_local_name) {
			board_name = board_local_name;
			printf("Detected name: %s\n", board_name);

			/* Time to change DTB on fly */
			/* Both ways should work here */
			/* fdtdec_resetup(&rescan); */
			fdtdec_setup();
		}
	}
	return 0;
}
#endif

#if defined(CONFIG_LMB)

#ifndef MMU_SECTION_SIZE
#define MMU_SECTION_SIZE        (1 * 1024 * 1024)
#endif

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	phys_size_t size;
	phys_addr_t reg;
	struct lmb lmb;

	if (!total_size)
		return gd->ram_top;

	if (!IS_ALIGNED((ulong)gd->fdt_blob, 0x8))
		panic("Not 64bit aligned DT location: %p\n", gd->fdt_blob);

	/* found enough not-reserved memory to relocated U-Boot */
	lmb_init(&lmb);
	lmb_add(&lmb, gd->ram_base, gd->ram_size);
	boot_fdt_add_mem_rsv_regions(&lmb, (void *)gd->fdt_blob);
	size = ALIGN(CONFIG_SYS_MALLOC_LEN + total_size, MMU_SECTION_SIZE);
	reg = lmb_alloc(&lmb, size, MMU_SECTION_SIZE);

	if (!reg)
		reg = gd->ram_top - size;

	return reg + size;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
#define MAX_RAND_SIZE 8
int ft_board_setup(void *blob, struct bd_info *bd)
{
	size_t n = MAX_RAND_SIZE;
	struct udevice *dev;
	u8 buf[MAX_RAND_SIZE];
	int nodeoffset, ret;

	if (uclass_get_device(UCLASS_RNG, 0, &dev) || !dev) {
		debug("No RNG device\n");
		return 0;
	}

	if (dm_rng_read(dev, buf, n)) {
		debug("Reading RNG failed\n");
		return 0;
	}

	if (!blob) {
		debug("No FDT memory address configured. Please configure\n"
		      "the FDT address via \"fdt addr <address>\" command.\n"
		      "Aborting!\n");
		return 0;
	}

	ret = fdt_check_header(blob);
	if (ret < 0) {
		debug("fdt_chosen: %s\n", fdt_strerror(ret));
		return ret;
	}

	nodeoffset = fdt_find_or_add_subnode(blob, 0, "chosen");
	if (nodeoffset < 0) {
		debug("Reading chosen node failed\n");
		return nodeoffset;
	}

	ret = fdt_setprop(blob, nodeoffset, "kaslr-seed", buf, sizeof(buf));
	if (ret < 0) {
		debug("Unable to set kaslr-seed on chosen node: %s\n", fdt_strerror(ret));
		return ret;
	}

	return 0;
}
#endif
