// SPDX-License-Identifier: GPL-2.0+
/*
 * Library to support early TI EVM EEPROM handling
 *
 * Copyright (C) 2015-2016 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla
 *	Steve Kipisz
 */

#include <common.h>
#include <eeprom.h>
#include <log.h>
#include <net.h>
#include <asm/arch/hardware.h>
#include <asm/omap_common.h>
#include <dm/uclass.h>
#include <env.h>
#include <i2c.h>
#include <mmc.h>
#include <errno.h>
#include <malloc.h>
#include <linux/printk.h>

#include "board_detect.h"

#if !CONFIG_IS_ENABLED(DM_I2C)
/**
 * ti_i2c_eeprom_init - Initialize an i2c bus and probe for a device
 * @i2c_bus: i2c bus number to initialize
 * @dev_addr: Device address to probe for
 *
 * Return: 0 on success or corresponding error on failure.
 */
static int __maybe_unused ti_i2c_eeprom_init(int i2c_bus, int dev_addr)
{
	int rc;

	if (i2c_bus >= 0) {
		rc = i2c_set_bus_num(i2c_bus);
		if (rc)
			return rc;
	}

	return i2c_probe(dev_addr);
}

/**
 * ti_i2c_eeprom_read - Read data from an EEPROM
 * @dev_addr: The device address of the EEPROM
 * @offset: Offset to start reading in the EEPROM
 * @ep: Pointer to a buffer to read into
 * @epsize: Size of buffer
 *
 * Return: 0 on success or corresponding result of i2c_read
 */
static int __maybe_unused ti_i2c_eeprom_read(int dev_addr, int offset,
					     uchar *ep, int epsize)
{
	return i2c_read(dev_addr, offset, 2, ep, epsize);
}
#endif

/**
 * ti_eeprom_string_cleanup() - Handle eeprom programming errors
 * @s:	eeprom string (should be NULL terminated)
 *
 * Some Board manufacturers do not add a NULL termination at the
 * end of string, instead some binary information is kludged in, hence
 * convert the string to just printable characters of ASCII chart.
 */
static void __maybe_unused ti_eeprom_string_cleanup(char *s)
{
	int i, l;

	l = strlen(s);
	for (i = 0; i < l; i++, s++)
		if (*s < ' ' || *s > '~') {
			*s = 0;
			break;
		}
}

__weak void gpi2c_init(void)
{
}

static int __maybe_unused ti_i2c_eeprom_get(int bus_addr, int dev_addr,
					    u32 header, u32 size, uint8_t *ep)
{
	int rc;
	uint8_t offset_test;
	bool one_byte_addressing = true;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	struct udevice *bus;

	rc = uclass_get_device_by_seq(UCLASS_I2C, bus_addr, &bus);
	if (rc)
		return rc;
	rc = dm_i2c_probe(bus, dev_addr, 0, &dev);
	if (rc)
		return rc;

	/*
	 * Read the header first then only read the other contents.
	 */
	rc = i2c_set_chip_offset_len(dev, 1);
	if (rc)
		return rc;

	/*
	 * Skip checking result here since this could be a valid i2c read fail
	 * on some boards that use 2 byte addressing.
	 * We must allow for fall through to check the data if 2 byte
	 * addressing works
	 */
	(void)dm_i2c_read(dev, 0, ep, size);

	if (*((u32 *)ep) != header)
		one_byte_addressing = false;

	/*
	 * Handle case of bad 2 byte eeproms that responds to 1 byte addressing
	 * but gets stuck in const addressing when read requests are performed
	 * on offsets. We perform an offset test to make sure it is not a 2 byte
	 * eeprom that works with 1 byte addressing but just without an offset
	 */

	rc = dm_i2c_read(dev, 0x1, &offset_test, sizeof(offset_test));

	if (offset_test != ((header >> 8) & 0xFF))
		one_byte_addressing = false;

	/* Corrupted data??? */
	if (!one_byte_addressing) {
		/*
		 * read the eeprom header using i2c again, but use only a
		 * 2 byte address (some newer boards need this..)
		 */
		rc = i2c_set_chip_offset_len(dev, 2);
		if (rc)
			return rc;

		rc = dm_i2c_read(dev, 0, ep, size);
		if (rc)
			return rc;
	}
	if (*((u32 *)ep) != header)
		return -1;
#else
	u32 byte;

	gpi2c_init();
	rc = ti_i2c_eeprom_init(bus_addr, dev_addr);
	if (rc)
		return rc;

	/*
	 * Read the header first then only read the other contents.
	 */
	byte = 1;

	/*
	 * Skip checking result here since this could be a valid i2c read fail
	 * on some boards that use 2 byte addressing.
	 * We must allow for fall through to check the data if 2 byte
	 * addressing works
	 */
	(void)i2c_read(dev_addr, 0x0, byte, ep, size);

	if (*((u32 *)ep) != header)
		one_byte_addressing = false;

	/*
	 * Handle case of bad 2 byte eeproms that responds to 1 byte addressing
	 * but gets stuck in const addressing when read requests are performed
	 * on offsets. We perform an offset test to make sure it is not a 2 byte
	 * eeprom that works with 1 byte addressing but just without an offset
	 */

	rc = i2c_read(dev_addr, 0x1, byte, &offset_test, sizeof(offset_test));

	if (offset_test != ((header >> 8) & 0xFF))
		one_byte_addressing = false;

	/* Corrupted data??? */
	if (!one_byte_addressing) {
		/*
		 * read the eeprom header using i2c again, but use only a
		 * 2 byte address (some newer boards need this..)
		 */
		byte = 2;
		rc = i2c_read(dev_addr, 0x0, byte, ep, size);
		if (rc)
			return rc;
	}
	if (*((u32 *)ep) != header)
		return -1;
#endif
	return 0;
}

int __maybe_unused ti_emmc_boardid_get(void)
{
	int rc;
	struct udevice *dev;
	struct mmc *mmc;
	struct ti_common_eeprom *ep;
	struct ti_am_eeprom brdid;
	struct blk_desc *bdesc;
	uchar *buffer;

	ep = TI_EEPROM_DATA;
	if (ep->header == TI_EEPROM_HEADER_MAGIC)
		return 0;       /* EEPROM has already been read */

	/* Initialize with a known bad marker for emmc fails.. */
	ep->header = TI_DEAD_EEPROM_MAGIC;
	ep->name[0] = 0x0;
	ep->version[0] = 0x0;
	ep->serial[0] = 0x0;
	ep->config[0] = 0x0;

	/* uclass object initialization */
	rc = mmc_initialize(NULL);
	if (rc)
		return rc;

	/* Set device to /dev/mmcblk1 */
	rc = uclass_get_device(UCLASS_MMC, 1, &dev);
	if (rc)
		return rc;

	/* Grab the mmc device */
	mmc = mmc_get_mmc_dev(dev);
	if (!mmc)
		return -ENODEV;

	/* mmc hardware initialization routine */
	mmc_init(mmc);

	/* Set partition to /dev/mmcblk1boot1 */
	rc = mmc_switch_part(mmc, 2);
	if (rc)
		return rc;

	buffer = malloc(mmc->read_bl_len);
	if (!buffer)
		return -ENOMEM;

	bdesc = mmc_get_blk_desc(mmc);

	/* blk_dread returns the number of blocks read*/
	if (blk_dread(bdesc, 0L, 1, buffer) != 1) {
		rc = -EIO;
		goto cleanup;
	}

	memcpy(&brdid, buffer, sizeof(brdid));

	/* Write out the ep struct values */
	ep->header = brdid.header;
	strlcpy(ep->name, brdid.name, TI_EEPROM_HDR_NAME_LEN + 1);
	ti_eeprom_string_cleanup(ep->name);
	strlcpy(ep->version, brdid.version, TI_EEPROM_HDR_REV_LEN + 1);
	ti_eeprom_string_cleanup(ep->version);
	strlcpy(ep->serial, brdid.serial, TI_EEPROM_HDR_SERIAL_LEN + 1);
	ti_eeprom_string_cleanup(ep->serial);

cleanup:
	free(buffer);

	return rc;
}

int __maybe_unused ti_i2c_eeprom_am_set(const char *name, const char *rev)
{
	struct ti_common_eeprom *ep;

	if (!name || !rev)
		return -1;

	ep = TI_EEPROM_DATA;
	if (ep->header == TI_EEPROM_HEADER_MAGIC)
		goto already_set;

	/* Set to 0 all fields */
	memset(ep, 0, sizeof(*ep));
	strncpy(ep->name, name, TI_EEPROM_HDR_NAME_LEN);
	strncpy(ep->version, rev, TI_EEPROM_HDR_REV_LEN);
	/* Some dummy serial number to identify the platform */
	strncpy(ep->serial, "0000", TI_EEPROM_HDR_SERIAL_LEN);
	/* Mark it with a valid header */
	ep->header = TI_EEPROM_HEADER_MAGIC;

already_set:
	return 0;
}

int __maybe_unused ti_i2c_eeprom_am_get(int bus_addr, int dev_addr)
{
	int rc;
	struct ti_am_eeprom am_ep;
	struct ti_common_eeprom *ep;

	ep = TI_EEPROM_DATA;
#ifndef CONFIG_SPL_BUILD
	if (ep->header == TI_EEPROM_HEADER_MAGIC)
		return 0; /* EEPROM has already been read */
#endif

	/* Initialize with a known bad marker for i2c fails.. */
	ep->header = TI_DEAD_EEPROM_MAGIC;
	ep->name[0] = 0x0;
	ep->version[0] = 0x0;
	ep->serial[0] = 0x0;
	ep->config[0] = 0x0;

	rc = ti_i2c_eeprom_get(bus_addr, dev_addr, TI_EEPROM_HEADER_MAGIC,
			       sizeof(am_ep), (uint8_t *)&am_ep);
	if (rc)
		return rc;

	ep->header = am_ep.header;
	strlcpy(ep->name, am_ep.name, TI_EEPROM_HDR_NAME_LEN + 1);
	ti_eeprom_string_cleanup(ep->name);

	/* BeagleBone Green '1' eeprom, board_rev: 0x1a 0x00 0x00 0x00 */
	if (am_ep.version[0] == 0x1a && am_ep.version[1] == 0x00 &&
	    am_ep.version[2] == 0x00 && am_ep.version[3] == 0x00)
		strlcpy(ep->version, "BBG1", TI_EEPROM_HDR_REV_LEN + 1);
	else
		strlcpy(ep->version, am_ep.version, TI_EEPROM_HDR_REV_LEN + 1);
	ti_eeprom_string_cleanup(ep->version);
	strlcpy(ep->serial, am_ep.serial, TI_EEPROM_HDR_SERIAL_LEN + 1);
	ti_eeprom_string_cleanup(ep->serial);
	strlcpy(ep->config, am_ep.config, TI_EEPROM_HDR_CONFIG_LEN + 1);
	ti_eeprom_string_cleanup(ep->config);

	memcpy(ep->mac_addr, am_ep.mac_addr,
	       TI_EEPROM_HDR_NO_OF_MAC_ADDR * TI_EEPROM_HDR_ETH_ALEN);

	return 0;
}

int __maybe_unused ti_i2c_eeprom_dra7_get(int bus_addr, int dev_addr)
{
	int rc, offset = 0;
	struct dra7_eeprom dra7_ep;
	struct ti_common_eeprom *ep;

	ep = TI_EEPROM_DATA;
#ifndef CONFIG_SPL_BUILD
	if (ep->header == DRA7_EEPROM_HEADER_MAGIC)
		return 0; /* EEPROM has already been read */
#endif

	/* Initialize with a known bad marker for i2c fails.. */
	ep->header = TI_DEAD_EEPROM_MAGIC;
	ep->name[0] = 0x0;
	ep->version[0] = 0x0;
	ep->serial[0] = 0x0;
	ep->config[0] = 0x0;
	ep->emif1_size = 0;
	ep->emif2_size = 0;

	rc = ti_i2c_eeprom_get(bus_addr, dev_addr, DRA7_EEPROM_HEADER_MAGIC,
			       sizeof(dra7_ep), (uint8_t *)&dra7_ep);
	if (rc)
		return rc;

	ep->header = dra7_ep.header;
	strlcpy(ep->name, dra7_ep.name, TI_EEPROM_HDR_NAME_LEN + 1);
	ti_eeprom_string_cleanup(ep->name);

	offset = dra7_ep.version_major - 1;

	/* Rev F is skipped */
	if (offset >= 5)
		offset = offset + 1;
	snprintf(ep->version, TI_EEPROM_HDR_REV_LEN + 1, "%c.%d",
		 'A' + offset, dra7_ep.version_minor);
	ti_eeprom_string_cleanup(ep->version);
	ep->emif1_size = (u64)dra7_ep.emif1_size;
	ep->emif2_size = (u64)dra7_ep.emif2_size;
	strlcpy(ep->config, dra7_ep.config, TI_EEPROM_HDR_CONFIG_LEN + 1);
	ti_eeprom_string_cleanup(ep->config);

	return 0;
}

static int ti_i2c_eeprom_am6_parse_record(struct ti_am6_eeprom_record *record,
					  struct ti_am6_eeprom *ep,
					  char **mac_addr,
					  u8 mac_addr_max_cnt,
					  u8 *mac_addr_cnt)
{
	switch (record->header.id) {
	case TI_AM6_EEPROM_RECORD_BOARD_INFO:
		if (record->header.len != sizeof(record->data.board_info))
			return -EINVAL;

		if (!ep)
			break;

		/* Populate (and clean, if needed) the board name */
		strlcpy(ep->name, record->data.board_info.name,
			sizeof(ep->name));
		ti_eeprom_string_cleanup(ep->name);

		/* Populate selected other fields from the board info record */
		strlcpy(ep->version, record->data.board_info.version,
			sizeof(ep->version));
		strlcpy(ep->software_revision,
			record->data.board_info.software_revision,
			sizeof(ep->software_revision));
		strlcpy(ep->serial, record->data.board_info.serial,
			sizeof(ep->serial));
		break;
	case TI_AM6_EEPROM_RECORD_MAC_INFO:
		if (record->header.len != sizeof(record->data.mac_info))
			return -EINVAL;

		if (!mac_addr || !mac_addr_max_cnt)
			break;

		*mac_addr_cnt = ((record->data.mac_info.mac_control &
				 TI_AM6_EEPROM_MAC_ADDR_COUNT_MASK) >>
				 TI_AM6_EEPROM_MAC_ADDR_COUNT_SHIFT) + 1;

		/*
		 * The EEPROM can (but may not) hold a very large amount
		 * of MAC addresses, by far exceeding what we want/can store
		 * in the common memory array, so only grab what we can fit.
		 * Note that a value of 0 means 1 MAC address, and so on.
		 */
		*mac_addr_cnt = min(*mac_addr_cnt, mac_addr_max_cnt);

		memcpy(mac_addr, record->data.mac_info.mac_addr,
		       *mac_addr_cnt * TI_EEPROM_HDR_ETH_ALEN);
		break;
	case 0x00:
		/* Illegal value... Fall through... */
	case 0xFF:
		/* Illegal value... Something went horribly wrong... */
		return -EINVAL;
	default:
		pr_warn("%s: Ignoring record id %u\n", __func__,
			record->header.id);
	}

	return 0;
}

int __maybe_unused ti_i2c_eeprom_am6_get(int bus_addr, int dev_addr,
					 struct ti_am6_eeprom *ep,
					 char **mac_addr,
					 u8 mac_addr_max_cnt,
					 u8 *mac_addr_cnt)
{
	struct udevice *dev;
	struct udevice *bus;
	unsigned int eeprom_addr;
	struct ti_am6_eeprom_record_board_id board_id;
	struct ti_am6_eeprom_record record;
	int rc;
	int consecutive_bad_records = 0;

	/* Initialize with a known bad marker for i2c fails.. */
	memset(ep, 0, sizeof(*ep));
	ep->header = TI_DEAD_EEPROM_MAGIC;

	/* Read the board ID record which is always the first EEPROM record */
	rc = ti_i2c_eeprom_get(bus_addr, dev_addr, TI_EEPROM_HEADER_MAGIC,
			       sizeof(board_id), (uint8_t *)&board_id);
	if (rc)
		return rc;

	if (board_id.header.id != TI_AM6_EEPROM_RECORD_BOARD_ID) {
		pr_err("%s: Invalid board ID record!\n", __func__);
		return -EINVAL;
	}

	/* Establish DM handle to board config EEPROM */
	rc = uclass_get_device_by_seq(UCLASS_I2C, bus_addr, &bus);
	if (rc)
		return rc;
	rc = i2c_get_chip(bus, dev_addr, 1, &dev);
	if (rc)
		return rc;

	ep->header = TI_EEPROM_HEADER_MAGIC;

	/* Ready to parse TLV structure. Initialize variables... */
	*mac_addr_cnt = 0;

	/*
	 * After the all-encompassing board ID record all other records follow
	 * a TLV-type scheme. Point to the first such record and then start
	 * parsing those one by one.
	 */
	eeprom_addr = sizeof(board_id);

	while (consecutive_bad_records < 10) {
		rc = dm_i2c_read(dev, eeprom_addr, (uint8_t *)&record.header,
				 sizeof(record.header));
		if (rc)
			return rc;

		/*
		 * Check for end of list marker. If we reached it don't go
		 * any further and stop parsing right here.
		 */
		if (record.header.id == TI_AM6_EEPROM_RECORD_END_LIST)
			break;

		eeprom_addr += sizeof(record.header);

		debug("%s: dev_addr=0x%02x header.id=%u header.len=%u\n",
		      __func__, dev_addr, record.header.id,
		      record.header.len);

		/* Read record into memory if it fits */
		if (record.header.len <= sizeof(record.data)) {
			rc = dm_i2c_read(dev, eeprom_addr,
					 (uint8_t *)&record.data,
					 record.header.len);
			if (rc)
				return rc;

			/* Process record */
			rc = ti_i2c_eeprom_am6_parse_record(&record, ep,
							    mac_addr,
							    mac_addr_max_cnt,
							    mac_addr_cnt);
			if (rc) {
				pr_err("%s: EEPROM parsing error!\n", __func__);
				return rc;
			}
			consecutive_bad_records = 0;
		} else {
			/*
			 * We may get here in case of larger records which
			 * are not yet understood.
			 */
			pr_err("%s: Ignoring record id %u\n", __func__,
			       record.header.id);
			consecutive_bad_records++;
		}

		eeprom_addr += record.header.len;
	}

	return 0;
}

int __maybe_unused ti_i2c_eeprom_am6_get_base(int bus_addr, int dev_addr)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;
	int ret;

	/*
	 * Always execute EEPROM read by not allowing to bypass it during the
	 * first invocation of SPL which happens on the R5 core.
	 */
#if !(defined(CONFIG_SPL_BUILD) && defined(CONFIG_CPU_V7R))
	if (ep->header == TI_EEPROM_HEADER_MAGIC) {
		debug("%s: EEPROM has already been read\n", __func__);
		return 0;
	}
#endif

	ret = ti_i2c_eeprom_am6_get(bus_addr, dev_addr, ep,
				    (char **)ep->mac_addr,
				    AM6_EEPROM_HDR_NO_OF_MAC_ADDR,
				    &ep->mac_addr_cnt);
	return ret;
}

bool __maybe_unused board_ti_k3_is(char *name_tag)
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (ep->header == TI_DEAD_EEPROM_MAGIC)
		return false;
	return !strncmp(ep->name, name_tag, AM6_EEPROM_HDR_NAME_LEN);
}

bool __maybe_unused board_ti_is(char *name_tag)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (ep->header == TI_DEAD_EEPROM_MAGIC)
		return false;
	return !strncmp(ep->name, name_tag, TI_EEPROM_HDR_NAME_LEN);
}

bool __maybe_unused board_ti_rev_is(char *rev_tag, int cmp_len)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;
	int l;

	if (ep->header == TI_DEAD_EEPROM_MAGIC)
		return false;

	l = cmp_len > TI_EEPROM_HDR_REV_LEN ? TI_EEPROM_HDR_REV_LEN : cmp_len;
	return !strncmp(ep->version, rev_tag, l);
}

char * __maybe_unused board_ti_get_rev(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	/* if ep->header == TI_DEAD_EEPROM_MAGIC, this is empty already */
	return ep->version;
}

char * __maybe_unused board_ti_get_config(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	/* if ep->header == TI_DEAD_EEPROM_MAGIC, this is empty already */
	return ep->config;
}

char * __maybe_unused board_ti_get_name(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	/* if ep->header == TI_DEAD_EEPROM_MAGIC, this is empty already */
	return ep->name;
}

void __maybe_unused
board_ti_get_eth_mac_addr(int index,
			  u8 mac_addr[TI_EEPROM_HDR_ETH_ALEN])
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (ep->header == TI_DEAD_EEPROM_MAGIC)
		goto fail;

	if (index < 0 || index >= TI_EEPROM_HDR_NO_OF_MAC_ADDR)
		goto fail;

	memcpy(mac_addr, ep->mac_addr[index], TI_EEPROM_HDR_ETH_ALEN);
	return;

fail:
	memset(mac_addr, 0, TI_EEPROM_HDR_ETH_ALEN);
}

void __maybe_unused
board_ti_am6_get_eth_mac_addr(int index,
			      u8 mac_addr[TI_EEPROM_HDR_ETH_ALEN])
{
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (ep->header == TI_DEAD_EEPROM_MAGIC)
		goto fail;

	if (index < 0 || index >= ep->mac_addr_cnt)
		goto fail;

	memcpy(mac_addr, ep->mac_addr[index], TI_EEPROM_HDR_ETH_ALEN);
	return;

fail:
	memset(mac_addr, 0, TI_EEPROM_HDR_ETH_ALEN);
}

u64 __maybe_unused board_ti_get_emif1_size(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (ep->header != DRA7_EEPROM_HEADER_MAGIC)
		return 0;

	return ep->emif1_size;
}

u64 __maybe_unused board_ti_get_emif2_size(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (ep->header != DRA7_EEPROM_HEADER_MAGIC)
		return 0;

	return ep->emif2_size;
}

void __maybe_unused set_board_info_env(char *name)
{
	char *unknown = "unknown";
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (name)
		env_set("board_name", name);
	else if (strlen(ep->name) != 0)
		env_set("board_name", ep->name);
	else
		env_set("board_name", unknown);

	if (strlen(ep->version) != 0)
		env_set("board_rev", ep->version);
	else
		env_set("board_rev", unknown);

	if (strlen(ep->serial) != 0)
		env_set("board_serial", ep->serial);
	else
		env_set("board_serial", unknown);
}

void __maybe_unused set_board_info_env_am6(char *name)
{
	char *unknown = "unknown";
	struct ti_am6_eeprom *ep = TI_AM6_EEPROM_DATA;

	if (name)
		env_set("board_name", name);
	else if (strlen(ep->name) != 0)
		env_set("board_name", ep->name);
	else
		env_set("board_name", unknown);

	if (strlen(ep->version) != 0)
		env_set("board_rev", ep->version);
	else
		env_set("board_rev", unknown);

	if (strlen(ep->software_revision) != 0)
		env_set("board_software_revision", ep->software_revision);
	else
		env_set("board_software_revision", unknown);

	if (strlen(ep->serial) != 0)
		env_set("board_serial", ep->serial);
	else
		env_set("board_serial", unknown);
}

static u64 mac_to_u64(u8 mac[6])
{
	int i;
	u64 addr = 0;

	for (i = 0; i < 6; i++) {
		addr <<= 8;
		addr |= mac[i];
	}

	return addr;
}

static void u64_to_mac(u64 addr, u8 mac[6])
{
	mac[5] = addr;
	mac[4] = addr >> 8;
	mac[3] = addr >> 16;
	mac[2] = addr >> 24;
	mac[1] = addr >> 32;
	mac[0] = addr >> 40;
}

void board_ti_set_ethaddr(int index)
{
	uint8_t mac_addr[6];
	int i;
	u64 mac1, mac2;
	u8 mac_addr1[6], mac_addr2[6];
	int num_macs;
	/*
	 * Export any Ethernet MAC addresses from EEPROM.
	 * The 2 MAC addresses in EEPROM define the address range.
	 */
	board_ti_get_eth_mac_addr(0, mac_addr1);
	board_ti_get_eth_mac_addr(1, mac_addr2);

	if (is_valid_ethaddr(mac_addr1) && is_valid_ethaddr(mac_addr2)) {
		mac1 = mac_to_u64(mac_addr1);
		mac2 = mac_to_u64(mac_addr2);

		/* must contain an address range */
		num_macs = mac2 - mac1 + 1;
		if (num_macs <= 0)
			return;

		if (num_macs > 50) {
			printf("%s: Too many MAC addresses: %d. Limiting to 50\n",
			       __func__, num_macs);
			num_macs = 50;
		}

		for (i = 0; i < num_macs; i++) {
			u64_to_mac(mac1 + i, mac_addr);
			if (is_valid_ethaddr(mac_addr)) {
				eth_env_set_enetaddr_by_index("eth", i + index,
							      mac_addr);
			}
		}
	}
}

void board_ti_am6_set_ethaddr(int index, int count)
{
	u8 mac_addr[6];
	int i;

	for (i = 0; i < count; i++) {
		board_ti_am6_get_eth_mac_addr(i, mac_addr);
		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr_by_index("eth", i + index,
						      mac_addr);
	}
}

bool __maybe_unused board_ti_was_eeprom_read(void)
{
	struct ti_common_eeprom *ep = TI_EEPROM_DATA;

	if (ep->header == TI_EEPROM_HEADER_MAGIC)
		return true;
	else
		return false;
}
