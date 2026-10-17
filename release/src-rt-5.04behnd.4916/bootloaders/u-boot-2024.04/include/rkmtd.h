/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Driver interface derived from:
 * /include/sandbox_host.h
 * Copyright 2022 Google LLC
 *
 * Copyright 2023 Johan Jonker <jbx6244@gmail.com>
 */

#ifndef __RKMTD__
#define __RKMTD__

#include <part_efi.h>
#include <uuid.h>

#define LBA			64 + 512 + 33

#define RK_TAG			0xFCDC8C3B
#define NFC_SYS_DATA_SIZE	4
#define BLK_SIZE		2048
#define STEP_SIZE		1024
#define BUF_SIZE		512 * 512

struct nand_para_info {
	u8 id_bytes;
	u8 nand_id[6];
	u8 vendor;
	u8 die_per_chip;
	u8 sec_per_page;
	u16 page_per_blk;
	u8 cell;
	u8 plane_per_die;
	u16 blk_per_plane;
	u16 operation_opt;
	u8 lsb_mode;
	u8 read_retry_mode;
	u8 ecc_bits;
	u8 access_freq;
	u8 opt_mode;
	u8 die_gap;
	u8 bad_block_mode;
	u8 multi_plane_mode;
	u8 slc_mode;
	u8 reserved[5];
};

struct bootblk {
	int blk;
	int boot_size;
	int offset;
};

struct rkmtd_dev {
	struct udevice *dev;
	struct blk_desc *desc;
	char *label;
	legacy_mbr *mbr;
	gpt_header *gpt_h;
	gpt_header *gpt_h2;
	gpt_entry *gpt_e;
	char *check;
	char *idb;
	char *str;
	char uuid_part_str[UUID_STR_LEN + 1];
	char uuid_disk_str[UUID_STR_LEN + 1];
	char *datbuf;
	char *oobbuf;
	struct mtd_info *mtd;
	struct nand_para_info *info;
	u16 page_table[512];
	u32 idb_need_write_back;
	struct bootblk idblock[5];
	u32 blk_counter;
	u32 boot_blks;
	u32 offset;
	u32 boot_size;
	u32 lsb_mode;
};

struct sector0 {
	u32 magic;
	u8  reserved[4];
	u32 rc4_flag;
	u16 boot_code1_offset;
	u16 boot_code2_offset;
	u8  reserved1[490];
	u16 flash_data_size;
	u16 flash_boot_size;
	u8  reserved2[2];
} __packed;

/**
 * rkmtd_rc4() - Rockchip specific RC4 Encryption Algorithm
 *
 * Encrypt Rockchip boot block header version 1 and data
 *
 * @buf: Pointer to data buffer
 * @len: Data buffer size
 */
void rkmtd_rc4(u8 *buf, u32 len);

/**
 * struct rkmtd_ops - operations supported by UCLASS_RKMTD
 */
struct rkmtd_ops {
	/**
	 * @attach_mtd: - Attach a new rkmtd driver to the device structure
	 *
	 * @attach_mtd.dev: Device to update
	 * @attach_mtd.Returns: 0 if OK, -EEXIST if a driver is already attached,
	 * other -ve on other error
	 */
	int (*attach_mtd)(struct udevice *dev);

	/**
	 * @detach_mtd: - Detach a rkmtd driver from the device structure
	 *
	 * @detach_mtd.dev: Device to detach from
	 * @detach_mtd.Returns: 0 if OK, -ENOENT if no driver is attached,
	 * other -ve on other error
	 */
	int (*detach_mtd)(struct udevice *dev);
};

#define rkmtd_get_ops(dev)        ((struct rkmtd_ops *)(dev)->driver->ops)

/**
 * rkmtd_get_cur_dev() - Get the current device
 *
 * Returns current device, or NULL if none
 */
struct udevice *rkmtd_get_cur_dev(void);

/**
 * rkmtd_set_cur_dev() - Set the current device
 *
 * Sets the current device, or clears it if @dev is NULL
 *
 * @dev: Device to set as the current one
 */
void rkmtd_set_cur_dev(struct udevice *dev);

/**
 * rkmtd_find_by_label() - Find a rkmtd device by label
 *
 * Searches all rkmtd devices to find one with the given label
 *
 * @label: Label to find
 * Returns: associated device, or NULL if not found
 */
struct udevice *rkmtd_find_by_label(const char *label);

/**
 * rkmtd_attach() - Attach a new rkmtd driver to the device structure
 *
 * @dev: Device to update
 * Returns: 0 if OK, -EEXIST if a file is already attached, other -ve on
 * other error
 */
int rkmtd_attach(struct udevice *dev);

/**
 * rkmtd_detach() - Detach a rkmtd driver from the device structure
 *
 * @dev: Device to detach from
 * Returns: 0 if OK, -ENOENT if no file is attached, other -ve on other
 * error
 */
int rkmtd_detach(struct udevice *dev);

/**
 * rkmtd_create_device() - Create a new rkmtd device
 *
 * Any existing device with the same label is removed and unbound first
 *
 * @label: Label of the attachment, e.g. "test1"
 * @devp: Returns the device created, on success
 * Returns: 0 if OK, -ve on error
 */
int rkmtd_create_device(const char *label, struct udevice **devp);

/**
 * rkmtd_create_attach_mtd() - Create a new rkmtd device and attach driver
 *
 * @label: Label of the attachment, e.g. "test1"
 * @devp: Returns the device created, on success
 * Returns: 0 if OK, -ve on error
 */
int rkmtd_create_attach_mtd(const char *label, struct udevice **devp);

#endif /* __RKMTD__ */
