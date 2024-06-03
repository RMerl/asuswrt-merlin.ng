// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <errno.h>
#include <dm.h>

#include <spi_flash.h>
#include <spi.h>
#include <nand.h>
#include <usb.h>
#include <fs.h>
#include <mmc.h>
#ifdef CONFIG_BLK
#include <blk.h>
#endif
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>

#ifndef CONFIG_SYS_MMC_ENV_DEV
#define CONFIG_SYS_MMC_ENV_DEV	0
#endif

#if defined(CONFIG_ARMADA_8K)
#define MAIN_HDR_MAGIC		0xB105B002

struct mvebu_image_header {
	u32	magic;			/*  0-3  */
	u32	prolog_size;		/*  4-7  */
	u32	prolog_checksum;	/*  8-11 */
	u32	boot_image_size;	/* 12-15 */
	u32	boot_image_checksum;	/* 16-19 */
	u32	rsrvd0;			/* 20-23 */
	u32	load_addr;		/* 24-27 */
	u32	exec_addr;		/* 28-31 */
	u8	uart_cfg;		/*  32   */
	u8	baudrate;		/*  33   */
	u8	ext_count;		/*  34   */
	u8	aux_flags;		/*  35   */
	u32	io_arg_0;		/* 36-39 */
	u32	io_arg_1;		/* 40-43 */
	u32	io_arg_2;		/* 43-47 */
	u32	io_arg_3;		/* 48-51 */
	u32	rsrvd1;			/* 52-55 */
	u32	rsrvd2;			/* 56-59 */
	u32	rsrvd3;			/* 60-63 */
};
#elif defined(CONFIG_ARMADA_3700)	/* A3700 */
#define HASH_SUM_LEN		16
#define IMAGE_VERSION_3_6_0	0x030600
#define IMAGE_VERSION_3_5_0	0x030500

struct common_tim_data {
	u32	version;
	u32	identifier;
	u32	trusted;
	u32	issue_date;
	u32	oem_unique_id;
	u32	reserved[5];		/* Reserve 20 bytes */
	u32	boot_flash_sign;
	u32	num_images;
	u32	num_keys;
	u32	size_of_reserved;
};

struct mvebu_image_info {
	u32	image_id;
	u32	next_image_id;
	u32	flash_entry_addr;
	u32	load_addr;
	u32	image_size;
	u32	image_size_to_hash;
	u32	hash_algorithm_id;
	u32	hash[HASH_SUM_LEN];	/* Reserve 512 bits for the hash */
	u32	partition_number;
	u32	enc_algorithm_id;
	u32	encrypt_start_offset;
	u32	encrypt_size;
};
#endif /* CONFIG_ARMADA_XXX */

struct bubt_dev {
	char name[8];
	size_t (*read)(const char *file_name);
	int (*write)(size_t image_size);
	int (*active)(void);
};

static ulong get_load_addr(void)
{
	const char *addr_str;
	unsigned long addr;

	addr_str = env_get("loadaddr");
	if (addr_str)
		addr = simple_strtoul(addr_str, NULL, 16);
	else
		addr = CONFIG_SYS_LOAD_ADDR;

	return addr;
}

/********************************************************************
 *     eMMC services
 ********************************************************************/
#if CONFIG_IS_ENABLED(DM_MMC) && CONFIG_IS_ENABLED(MMC_WRITE)
static int mmc_burn_image(size_t image_size)
{
	struct mmc	*mmc;
	lbaint_t	start_lba;
	lbaint_t	blk_count;
	ulong		blk_written;
	int		err;
	const u8	mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;
#ifdef CONFIG_BLK
	struct blk_desc *blk_desc;
#endif
	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return -ENOMEDIUM;
	}

	err = mmc_init(mmc);
	if (err) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return err;
	}

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (mmc->part_num != CONFIG_SYS_MMC_ENV_PART) {
		err = mmc_switch_part(mmc_dev_num, CONFIG_SYS_MMC_ENV_PART);
		if (err) {
			printf("MMC partition switch failed\n");
			return err;
		}
	}
#endif

	/* SD reserves LBA-0 for MBR and boots from LBA-1,
	 * MMC/eMMC boots from LBA-0
	 */
	start_lba = IS_SD(mmc) ? 1 : 0;
#ifdef CONFIG_BLK
	blk_count = image_size / mmc->write_bl_len;
	if (image_size % mmc->write_bl_len)
		blk_count += 1;

	blk_desc = mmc_get_blk_desc(mmc);
	if (!blk_desc) {
		printf("Error - failed to obtain block descriptor\n");
		return -ENODEV;
	}
	blk_written = blk_dwrite(blk_desc, start_lba, blk_count,
				 (void *)get_load_addr());
#else
	blk_count = image_size / mmc->block_dev.blksz;
	if (image_size % mmc->block_dev.blksz)
		blk_count += 1;

	blk_written = mmc->block_dev.block_write(mmc_dev_num,
						 start_lba, blk_count,
						 (void *)get_load_addr());
#endif /* CONFIG_BLK */
	if (blk_written != blk_count) {
		printf("Error - written %#lx blocks\n", blk_written);
		return -ENOSPC;
	}
	printf("Done!\n");

#ifdef CONFIG_SYS_MMC_ENV_PART
	if (mmc->part_num != CONFIG_SYS_MMC_ENV_PART)
		mmc_switch_part(mmc_dev_num, mmc->part_num);
#endif

	return 0;
}

static size_t mmc_read_file(const char *file_name)
{
	loff_t		act_read = 0;
	int		rc;
	struct mmc	*mmc;
	const u8	mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;

	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return 0;
	}

	if (mmc_init(mmc)) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return 0;
	}

	/* Load from data partition (0) */
	if (fs_set_blk_dev("mmc", "0", FS_TYPE_ANY)) {
		printf("Error: MMC 0 not found\n");
		return 0;
	}

	/* Perfrom file read */
	rc = fs_read(file_name, get_load_addr(), 0, 0, &act_read);
	if (rc)
		return 0;

	return act_read;
}

static int is_mmc_active(void)
{
	return 1;
}
#else /* CONFIG_DM_MMC */
static int mmc_burn_image(size_t image_size)
{
	return -ENODEV;
}

static size_t mmc_read_file(const char *file_name)
{
	return 0;
}

static int is_mmc_active(void)
{
	return 0;
}
#endif /* CONFIG_DM_MMC */

/********************************************************************
 *     SPI services
 ********************************************************************/
#ifdef CONFIG_SPI_FLASH
static int spi_burn_image(size_t image_size)
{
	int ret;
	struct spi_flash *flash;
	u32 erase_bytes;

	/* Probe the SPI bus to get the flash device */
	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
				CONFIG_ENV_SPI_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("Failed to probe SPI Flash\n");
		return -ENOMEDIUM;
	}

#ifdef CONFIG_SPI_FLASH_PROTECTION
	spi_flash_protect(flash, 0);
#endif
	erase_bytes = image_size +
		(flash->erase_size - image_size % flash->erase_size);
	printf("Erasing %d bytes (%d blocks) at offset 0 ...",
	       erase_bytes, erase_bytes / flash->erase_size);
	ret = spi_flash_erase(flash, 0, erase_bytes);
	if (ret)
		printf("Error!\n");
	else
		printf("Done!\n");

	printf("Writing %d bytes from 0x%lx to offset 0 ...",
	       (int)image_size, get_load_addr());
	ret = spi_flash_write(flash, 0, image_size, (void *)get_load_addr());
	if (ret)
		printf("Error!\n");
	else
		printf("Done!\n");

#ifdef CONFIG_SPI_FLASH_PROTECTION
	spi_flash_protect(flash, 1);
#endif

	return ret;
}

static int is_spi_active(void)
{
	return 1;
}

#else /* CONFIG_SPI_FLASH */
static int spi_burn_image(size_t image_size)
{
	return -ENODEV;
}

static int is_spi_active(void)
{
	return 0;
}
#endif /* CONFIG_SPI_FLASH */

/********************************************************************
 *     NAND services
 ********************************************************************/
#ifdef CONFIG_CMD_NAND
static int nand_burn_image(size_t image_size)
{
	int ret;
	uint32_t block_size;
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd) {
		puts("\nno devices available\n");
		return -ENOMEDIUM;
	}
	block_size = mtd->erasesize;

	/* Align U-Boot size to currently used blocksize */
	image_size = ((image_size + (block_size - 1)) & (~(block_size - 1)));

	/* Erase the U-BOOT image space */
	printf("Erasing 0x%x - 0x%x:...", 0, (int)image_size);
	ret = nand_erase(mtd, 0, image_size);
	if (ret) {
		printf("Error!\n");
		goto error;
	}
	printf("Done!\n");

	/* Write the image to flash */
	printf("Writing %d bytes from 0x%lx to offset 0 ... ",
	       (int)image_size, get_load_addr());
	ret = nand_write(mtd, 0, &image_size, (void *)get_load_addr());
	if (ret)
		printf("Error!\n");
	else
		printf("Done!\n");

error:
	return ret;
}

static int is_nand_active(void)
{
	return 1;
}

#else /* CONFIG_CMD_NAND */
static int nand_burn_image(size_t image_size)
{
	return -ENODEV;
}

static int is_nand_active(void)
{
	return 0;
}
#endif /* CONFIG_CMD_NAND */

/********************************************************************
 *     USB services
 ********************************************************************/
#if defined(CONFIG_USB_STORAGE) && defined(CONFIG_BLK)
static size_t usb_read_file(const char *file_name)
{
	loff_t act_read = 0;
	struct udevice *dev;
	int rc;

	usb_stop();

	if (usb_init() < 0) {
		printf("Error: usb_init failed\n");
		return 0;
	}

	/* Try to recognize storage devices immediately */
	blk_first_device(IF_TYPE_USB, &dev);
	if (!dev) {
		printf("Error: USB storage device not found\n");
		return 0;
	}

	/* Always load from usb 0 */
	if (fs_set_blk_dev("usb", "0", FS_TYPE_ANY)) {
		printf("Error: USB 0 not found\n");
		return 0;
	}

	/* Perfrom file read */
	rc = fs_read(file_name, get_load_addr(), 0, 0, &act_read);
	if (rc)
		return 0;

	return act_read;
}

static int is_usb_active(void)
{
	return 1;
}

#else /* defined(CONFIG_USB_STORAGE) && defined (CONFIG_BLK) */
static size_t usb_read_file(const char *file_name)
{
	return 0;
}

static int is_usb_active(void)
{
	return 0;
}
#endif /* defined(CONFIG_USB_STORAGE) && defined (CONFIG_BLK) */

/********************************************************************
 *     Network services
 ********************************************************************/
#ifdef CONFIG_CMD_NET
static size_t tftp_read_file(const char *file_name)
{
	/* update global variable load_addr before tftp file from network */
	load_addr = get_load_addr();
	return net_loop(TFTPGET);
}

static int is_tftp_active(void)
{
	return 1;
}

#else
static size_t tftp_read_file(const char *file_name)
{
	return 0;
}

static int is_tftp_active(void)
{
	return 0;
}
#endif /* CONFIG_CMD_NET */

enum bubt_devices {
	BUBT_DEV_NET = 0,
	BUBT_DEV_USB,
	BUBT_DEV_MMC,
	BUBT_DEV_SPI,
	BUBT_DEV_NAND,

	BUBT_MAX_DEV
};

struct bubt_dev bubt_devs[BUBT_MAX_DEV] = {
	{"tftp", tftp_read_file, NULL, is_tftp_active},
	{"usb",  usb_read_file,  NULL, is_usb_active},
	{"mmc",  mmc_read_file,  mmc_burn_image, is_mmc_active},
	{"spi",  NULL, spi_burn_image,  is_spi_active},
	{"nand", NULL, nand_burn_image, is_nand_active},
};

static int bubt_write_file(struct bubt_dev *dst, size_t image_size)
{
	if (!dst->write) {
		printf("Error: Write not supported on device %s\n", dst->name);
		return -ENOTSUPP;
	}

	return dst->write(image_size);
}

#if defined(CONFIG_ARMADA_8K)
u32 do_checksum32(u32 *start, int32_t len)
{
	u32 sum = 0;
	u32 *startp = start;

	do {
		sum += *startp;
		startp++;
		len -= 4;
	} while (len > 0);

	return sum;
}

static int check_image_header(void)
{
	struct mvebu_image_header *hdr =
			(struct mvebu_image_header *)get_load_addr();
	u32 header_len = hdr->prolog_size;
	u32 checksum;
	u32 checksum_ref = hdr->prolog_checksum;

	/*
	 * For now compare checksum, and magic. Later we can
	 * verify more stuff on the header like interface type, etc
	 */
	if (hdr->magic != MAIN_HDR_MAGIC) {
		printf("ERROR: Bad MAGIC 0x%08x != 0x%08x\n",
		       hdr->magic, MAIN_HDR_MAGIC);
		return -ENOEXEC;
	}

	/* The checksum value is discarded from checksum calculation */
	hdr->prolog_checksum = 0;

	checksum = do_checksum32((u32 *)hdr, header_len);
	if (checksum != checksum_ref) {
		printf("Error: Bad Image checksum. 0x%x != 0x%x\n",
		       checksum, checksum_ref);
		return -ENOEXEC;
	}

	/* Restore the checksum before writing */
	hdr->prolog_checksum = checksum_ref;
	printf("Image checksum...OK!\n");

	return 0;
}
#elif defined(CONFIG_ARMADA_3700) /* Armada 3700 */
static int check_image_header(void)
{
	struct common_tim_data *hdr = (struct common_tim_data *)get_load_addr();
	int image_num;
	u8 hash_160_output[SHA1_SUM_LEN];
	u8 hash_256_output[SHA256_SUM_LEN];
	sha1_context hash1_text;
	sha256_context hash256_text;
	u8 *hash_output;
	u32 hash_algorithm_id;
	u32 image_size_to_hash;
	u32 flash_entry_addr;
	u32 *hash_value;
	u32 internal_hash[HASH_SUM_LEN];
	const u8 *buff;
	u32 num_of_image = hdr->num_images;
	u32 version = hdr->version;
	u32 trusted = hdr->trusted;

	/* bubt checksum validation only supports nontrusted images */
	if (trusted == 1) {
		printf("bypass image validation, ");
		printf("only untrusted image is supported now\n");
		return 0;
	}
	/* only supports image version 3.5 and 3.6 */
	if (version != IMAGE_VERSION_3_5_0 && version != IMAGE_VERSION_3_6_0) {
		printf("Error: Unsupported Image version = 0x%08x\n", version);
		return -ENOEXEC;
	}
	/* validate images hash value */
	for (image_num = 0; image_num < num_of_image; image_num++) {
		struct mvebu_image_info *info =
				(struct mvebu_image_info *)(get_load_addr() +
				sizeof(struct common_tim_data) +
				image_num * sizeof(struct mvebu_image_info));
		hash_algorithm_id = info->hash_algorithm_id;
		image_size_to_hash = info->image_size_to_hash;
		flash_entry_addr = info->flash_entry_addr;
		hash_value = info->hash;
		buff = (const u8 *)(get_load_addr() + flash_entry_addr);

		if (image_num == 0) {
			/*
			 * The first image includes hash values in its content.
			 * For hash calculation, we need to save the original
			 * hash values to a local variable that will be
			 * copied back for comparsion and set all zeros to
			 * the orignal hash values for calculating new value.
			 * First image original format :
			 * x...x (datum1) x...x(orig. hash values) x...x(datum2)
			 * Replaced first image format :
			 * x...x (datum1) 0...0(hash values) x...x(datum2)
			 */
			memcpy(internal_hash, hash_value,
			       sizeof(internal_hash));
			memset(hash_value, 0, sizeof(internal_hash));
		}
		if (image_size_to_hash == 0) {
			printf("Warning: Image_%d hash checksum is disabled, ",
			       image_num);
			printf("skip the image validation.\n");
			continue;
		}
		switch (hash_algorithm_id) {
		case SHA1_SUM_LEN:
			sha1_starts(&hash1_text);
			sha1_update(&hash1_text, buff, image_size_to_hash);
			sha1_finish(&hash1_text, hash_160_output);
			hash_output = hash_160_output;
			break;
		case SHA256_SUM_LEN:
			sha256_starts(&hash256_text);
			sha256_update(&hash256_text, buff, image_size_to_hash);
			sha256_finish(&hash256_text, hash_256_output);
			hash_output = hash_256_output;
			break;
		default:
			printf("Error: Unsupported hash_algorithm_id = %d\n",
			       hash_algorithm_id);
			return -ENOEXEC;
		}
		if (image_num == 0)
			memcpy(hash_value, internal_hash,
			       sizeof(internal_hash));
		if (memcmp(hash_value, hash_output, hash_algorithm_id) != 0) {
			printf("Error: Image_%d checksum is not correct\n",
			       image_num);
			return -ENOEXEC;
		}
	}
	printf("Image checksum...OK!\n");

	return 0;
}

#else /* Not ARMADA? */
static int check_image_header(void)
{
	printf("bubt cmd does not support this SoC device or family!\n");
	return -ENOEXEC;
}
#endif

static int bubt_verify(size_t image_size)
{
	int err;

	/* Check a correct image header exists */
	err = check_image_header();
	if (err) {
		printf("Error: Image header verification failed\n");
		return err;
	}

	return 0;
}

static int bubt_read_file(struct bubt_dev *src)
{
	size_t image_size;

	if (!src->read) {
		printf("Error: Read not supported on device \"%s\"\n",
		       src->name);
		return 0;
	}

	image_size = src->read(net_boot_file_name);
	if (image_size <= 0) {
		printf("Error: Failed to read file %s from %s\n",
		       net_boot_file_name, src->name);
		return 0;
	}

	return image_size;
}

static int bubt_is_dev_active(struct bubt_dev *dev)
{
	if (!dev->active) {
		printf("Device \"%s\" not supported by U-BOOT image\n",
		       dev->name);
		return 0;
	}

	if (!dev->active()) {
		printf("Device \"%s\" is inactive\n", dev->name);
		return 0;
	}

	return 1;
}

struct bubt_dev *find_bubt_dev(char *dev_name)
{
	int dev;

	for (dev = 0; dev < BUBT_MAX_DEV; dev++) {
		if (strcmp(bubt_devs[dev].name, dev_name) == 0)
			return &bubt_devs[dev];
	}

	return 0;
}

#define DEFAULT_BUBT_SRC "tftp"

#ifndef DEFAULT_BUBT_DST
#ifdef CONFIG_MVEBU_SPI_BOOT
#define DEFAULT_BUBT_DST "spi"
#elif defined(CONFIG_MVEBU_NAND_BOOT)
#define DEFAULT_BUBT_DST "nand"
#elif defined(CONFIG_MVEBU_MMC_BOOT)
#define DEFAULT_BUBT_DST "mmc"
else
#define DEFAULT_BUBT_DST "error"
#endif
#endif /* DEFAULT_BUBT_DST */

int do_bubt_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct bubt_dev *src, *dst;
	size_t image_size;
	char src_dev_name[8];
	char dst_dev_name[8];
	char *name;
	int  err;

	if (argc < 2)
		copy_filename(net_boot_file_name,
			      CONFIG_MVEBU_UBOOT_DFLT_NAME,
			      sizeof(net_boot_file_name));
	else
		copy_filename(net_boot_file_name, argv[1],
			      sizeof(net_boot_file_name));

	if (argc >= 3) {
		strncpy(dst_dev_name, argv[2], 8);
	} else {
		name = DEFAULT_BUBT_DST;
		strncpy(dst_dev_name, name, 8);
	}

	if (argc >= 4)
		strncpy(src_dev_name, argv[3], 8);
	else
		strncpy(src_dev_name, DEFAULT_BUBT_SRC, 8);

	/* Figure out the destination device */
	dst = find_bubt_dev(dst_dev_name);
	if (!dst) {
		printf("Error: Unknown destination \"%s\"\n", dst_dev_name);
		return -EINVAL;
	}

	if (!bubt_is_dev_active(dst))
		return -ENODEV;

	/* Figure out the source device */
	src = find_bubt_dev(src_dev_name);
	if (!src) {
		printf("Error: Unknown source \"%s\"\n", src_dev_name);
		return 1;
	}

	if (!bubt_is_dev_active(src))
		return -ENODEV;

	printf("Burning U-BOOT image \"%s\" from \"%s\" to \"%s\"\n",
	       net_boot_file_name, src->name, dst->name);

	image_size = bubt_read_file(src);
	if (!image_size)
		return -EIO;

	err = bubt_verify(image_size);
	if (err)
		return err;

	err = bubt_write_file(dst, image_size);
	if (err)
		return err;

	return 0;
}

U_BOOT_CMD(
	bubt, 4, 0, do_bubt_cmd,
	"Burn a u-boot image to flash",
	"[file-name] [destination [source]]\n"
	"\t-file-name     The image file name to burn. Default = flash-image.bin\n"
	"\t-destination   Flash to burn to [spi, nand, mmc]. Default = active boot device\n"
	"\t-source        The source to load image from [tftp, usb, mmc]. Default = tftp\n"
	"Examples:\n"
	"\tbubt - Burn flash-image.bin from tftp to active boot device\n"
	"\tbubt flash-image-new.bin nand - Burn flash-image-new.bin from tftp to NAND flash\n"
	"\tbubt backup-flash-image.bin mmc usb - Burn backup-flash-image.bin from usb to MMC\n"

);
