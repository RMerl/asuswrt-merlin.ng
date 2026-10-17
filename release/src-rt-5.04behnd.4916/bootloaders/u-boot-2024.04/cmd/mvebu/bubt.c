// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <net.h>
#include <vsprintf.h>
#include <errno.h>
#include <dm.h>
#include <fuse.h>
#include <mach/efuse.h>

#include <spi_flash.h>
#include <spi.h>
#include <nand.h>
#include <scsi.h>
#include <usb.h>
#include <fs.h>
#include <mmc.h>
#ifdef CONFIG_BLK
#include <blk.h>
#endif
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>

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

struct tim_boot_flash_sign {
	unsigned int id;
	const char *name;
};

struct tim_boot_flash_sign tim_boot_flash_signs[] = {
	{ 0x454d4d08, "mmc"  },
	{ 0x454d4d0b, "mmc"  },
	{ 0x5350490a, "spi"  },
	{ 0x5350491a, "nand" },
	{ 0x55415223, "uart" },
	{ 0x53415432, "sata" },
	{},
};

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
#elif defined(CONFIG_ARMADA_32BIT)

/* Structure of the main header, version 1 (Armada 370/XP/375/38x/39x) */
struct a38x_main_hdr_v1 {
	u8  blockid;               /* 0x0       */
	u8  flags;                 /* 0x1       */
	u16 nandpagesize;          /* 0x2-0x3   */
	u32 blocksize;             /* 0x4-0x7   */
	u8  version;               /* 0x8       */
	u8  headersz_msb;          /* 0x9       */
	u16 headersz_lsb;          /* 0xA-0xB   */
	u32 srcaddr;               /* 0xC-0xF   */
	u32 destaddr;              /* 0x10-0x13 */
	u32 execaddr;              /* 0x14-0x17 */
	u8  options;               /* 0x18      */
	u8  nandblocksize;         /* 0x19      */
	u8  nandbadblklocation;    /* 0x1A      */
	u8  reserved4;             /* 0x1B      */
	u16 reserved5;             /* 0x1C-0x1D */
	u8  ext;                   /* 0x1E      */
	u8  checksum;              /* 0x1F      */
};

/*
 * Header for the optional headers, version 1 (Armada 370/XP/375/38x/39x)
 */
struct a38x_opt_hdr_v1 {
	u8	headertype;
	u8	headersz_msb;
	u16	headersz_lsb;
	u8	data[0];
};
#define A38X_OPT_HDR_V1_SECURE_TYPE	0x1

struct a38x_boot_mode {
	unsigned int id;
	const char *name;
};

/* The blockid header field values used to indicate boot device of image */
struct a38x_boot_mode a38x_boot_modes[] = {
	{ 0x4D, "i2c"  },
	{ 0x5A, "spi"  },
	{ 0x69, "uart" },
	{ 0x78, "sata" },
	{ 0x8B, "nand" },
	{ 0x9C, "pex"  },
	{ 0xAE, "mmc"  },
	{},
};

#endif

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
		addr = hextoul(addr_str, NULL);
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
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	u8		part;
	u8		orig_part;
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

#ifdef CONFIG_BLK
	blk_desc = mmc_get_blk_desc(mmc);
	if (!blk_desc) {
		printf("Error - failed to obtain block descriptor\n");
		return -ENODEV;
	}
#endif

#ifdef CONFIG_SUPPORT_EMMC_BOOT
#ifdef CONFIG_BLK
	orig_part = blk_desc->hwpart;
#else
	orig_part = mmc->block_dev.hwpart;
#endif

	part = EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
	if (part == 7)
		part = 0;

#ifdef CONFIG_BLK
	err = blk_dselect_hwpart(blk_desc, part);
#else
	err = mmc_switch_part(mmc, part);
#endif

	if (err) {
		printf("Error - MMC partition switch failed\n");
		return err;
	}
#endif

	/* SD reserves LBA-0 for MBR and boots from LBA-1,
	 * MMC/eMMC boots from LBA-0 and LBA-4096
	 */
	if (IS_SD(mmc))
		start_lba = 1;
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	else if (part)
		start_lba = 0;
#endif
	else
		start_lba = 4096;
#ifdef CONFIG_BLK
	blk_count = image_size / mmc->write_bl_len;
	if (image_size % mmc->write_bl_len)
		blk_count += 1;

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

#ifdef CONFIG_SUPPORT_EMMC_BOOT
#ifdef CONFIG_BLK
	err = blk_dselect_hwpart(blk_desc, orig_part);
#else
	err = mmc_switch_part(mmc, orig_part);
#endif
	if (err)
		printf("Error - MMC failed to switch back to original partition\n");
#endif

	if (blk_written != blk_count) {
		printf("Error - written %#lx blocks\n", blk_written);
		return -ENOSPC;
	}
	printf("Done!\n");

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
 *     SATA services
 ********************************************************************/
#if defined(CONFIG_SCSI) && defined(CONFIG_BLK)
static int sata_burn_image(size_t image_size)
{
#if defined(CONFIG_ARMADA_3700) || defined(CONFIG_ARMADA_32BIT)
	lbaint_t	start_lba;
	lbaint_t	blk_count;
	ulong		blk_written;
	struct blk_desc *blk_desc;
#ifdef CONFIG_ARMADA_3700
	struct disk_partition info;
	int		part;
#endif

	scsi_scan(false);

	blk_desc = blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0);
	if (!blk_desc)
		return -ENODEV;

#ifdef CONFIG_ARMADA_3700
	/*
	 * 64-bit Armada 3700 BootROM loads SATA firmware from
	 * GPT 'Marvell Armada 3700 Boot partition' or from
	 * MBR 'M' (0x4d) partition.
	 */
	switch (blk_desc->part_type) {
	case PART_TYPE_DOS:
		for (part = 1; part <= 4; part++) {
			info.sys_ind = 0;
			if (part_get_info(blk_desc, part, &info))
				continue;
			if (info.sys_ind == 'M')
				break;
		}
		if (part > 4) {
			printf("Error - cannot find MBR 'M' (0x4d) partition on SATA disk\n");
			return -ENODEV;
		}
		start_lba = info.start;
		break;
	case PART_TYPE_EFI:
		for (part = 1; part <= 64; part++) {
			info.type_guid[0] = 0;
			if (part_get_info(blk_desc, part, &info))
				continue;
			/* Check for GPT type GUID of 'Marvell Armada 3700 Boot partition' */
			if (strcmp(info.type_guid, "6828311A-BA55-42A4-BCDE-A89BB5EDECAE") == 0)
				break;
		}
		if (part > 64) {
			printf("Error - cannot find GPT 'Marvell Armada 3700 Boot partition' on SATA disk\n");
			return -ENODEV;
		}
		start_lba = info.start;
		break;
	default:
		printf("Error - no partitions on SATA disk\n");
		return -ENODEV;
	}
#else
	/* 32-bit Armada BootROM loads SATA firmware from the sector 1. */
	start_lba = 1;
#endif

	blk_count = image_size / blk_desc->blksz;
	if (image_size % blk_desc->blksz)
		blk_count += 1;

	blk_written = blk_dwrite(blk_desc, start_lba, blk_count,
				 (void *)get_load_addr());

	if (blk_written != blk_count) {
		printf("Error - written %#lx blocks\n", blk_written);
		return -ENOSPC;
	}

	printf("Done!\n");
	return 0;
#else
	return -ENODEV;
#endif
}

static size_t sata_read_file(const char *file_name)
{
	loff_t act_read = 0;
	struct udevice *dev;
	int rc;

	/* try to recognize storage devices immediately */
	scsi_scan(false);

	/* Try to recognize storage devices immediately */
	blk_first_device(UCLASS_SCSI, &dev);
	if (!dev) {
		printf("Error: SATA device not found\n");
		return 0;
	}

	/* Always load from scsi 0 */
	if (fs_set_blk_dev("scsi", "0", FS_TYPE_ANY)) {
		printf("Error: SATA 0 not found\n");
		return 0;
	}

	/* Perfrom file read */
	rc = fs_read(file_name, get_load_addr(), 0, 0, &act_read);
	if (rc)
		return 0;

	return act_read;
}

static int is_sata_active(void)
{
	return 1;
}
#else /* CONFIG_SCSI */
static int sata_burn_image(size_t image_size)
{
	return -ENODEV;
}

static size_t sata_read_file(const char *file_name)
{
	return 0;
}

static int is_sata_active(void)
{
	return 0;
}
#endif /* CONFIG_SCSI */

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
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("Failed to probe SPI Flash\n");
		return -ENOMEDIUM;
	}

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

	/* Erase the U-Boot image space */
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
	blk_first_device(UCLASS_USB, &dev);
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
	int ret;

	/*
	 * update global variable image_load_addr before tftp file from network
	 */
	image_load_addr = get_load_addr();
	ret = net_loop(TFTPGET);
	return ret > 0 ? ret : 0;
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
	BUBT_DEV_SATA,
	BUBT_DEV_SPI,
	BUBT_DEV_NAND,

	BUBT_MAX_DEV
};

static struct bubt_dev bubt_devs[BUBT_MAX_DEV] = {
	{"tftp", tftp_read_file, NULL, is_tftp_active},
	{"usb",  usb_read_file,  NULL, is_usb_active},
	{"mmc",  mmc_read_file,  mmc_burn_image, is_mmc_active},
	{"sata", sata_read_file, sata_burn_image,  is_sata_active},
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
static u32 do_checksum32(u32 *start, int32_t len)
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
	u32 checksum;
	u32 checksum_ref;

	/*
	 * For now compare checksum, and magic. Later we can
	 * verify more stuff on the header like interface type, etc
	 */
	if (hdr->magic != MAIN_HDR_MAGIC) {
		printf("ERROR: Bad MAGIC 0x%08x != 0x%08x\n",
		       hdr->magic, MAIN_HDR_MAGIC);
		return -ENOEXEC;
	}

	checksum_ref = hdr->prolog_checksum;
	checksum = do_checksum32((u32 *)hdr, hdr->prolog_size);
	checksum -= hdr->prolog_checksum;
	if (checksum != checksum_ref) {
		printf("Error: Bad Prolog checksum. 0x%x != 0x%x\n",
		       checksum, checksum_ref);
		return -ENOEXEC;
	}

	checksum_ref = hdr->boot_image_checksum;
	checksum = do_checksum32((u32 *)((u8 *)hdr + hdr->prolog_size), hdr->boot_image_size);
	if (checksum != checksum_ref) {
		printf("Error: Bad Image checksum. 0x%x != 0x%x\n",
		       checksum, checksum_ref);
		return -ENOEXEC;
	}

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
	u8 hash_512_output[SHA512_SUM_LEN];
	sha1_context hash1_text;
	sha256_context hash256_text;
	sha512_context hash512_text;
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
		case SHA512_SUM_LEN:
			sha512_starts(&hash512_text);
			sha512_update(&hash512_text, buff, image_size_to_hash);
			sha512_finish(&hash512_text, hash_512_output);
			hash_output = hash_512_output;
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
#elif defined(CONFIG_ARMADA_32BIT)
static size_t a38x_header_size(const struct a38x_main_hdr_v1 *h)
{
	if (h->version == 1)
		return (h->headersz_msb << 16) | le16_to_cpu(h->headersz_lsb);

	printf("Error: Invalid A38x image (header version 0x%x unknown)!\n",
	       h->version);
	return 0;
}

static uint8_t image_checksum8(const void *start, size_t len)
{
	u8 csum = 0;
	const u8 *p = start;

	while (len) {
		csum += *p;
		++p;
		--len;
	}

	return csum;
}

static uint32_t image_checksum32(const void *start, size_t len)
{
	u32 csum = 0;
	const u32 *p = start;

	while (len) {
		csum += *p;
		++p;
		len -= sizeof(u32);
	}

	return csum;
}

static int check_image_header(void)
{
	u8 checksum;
	u32 checksum32, exp_checksum32;
	u32 offset, size;
	const struct a38x_main_hdr_v1 *hdr =
		(struct a38x_main_hdr_v1 *)get_load_addr();
	const size_t hdr_size = a38x_header_size(hdr);

	if (!hdr_size)
		return -ENOEXEC;

	checksum = image_checksum8(hdr, hdr_size);
	checksum -= hdr->checksum;
	if (checksum != hdr->checksum) {
		printf("Error: Bad A38x image header checksum. 0x%x != 0x%x\n",
		       checksum, hdr->checksum);
		return -ENOEXEC;
	}

	offset = le32_to_cpu(hdr->srcaddr);
	size = le32_to_cpu(hdr->blocksize);

	if (hdr->blockid == 0x78) { /* SATA id */
		struct blk_desc *blk_dev = IS_ENABLED(BLK) ? blk_get_devnum_by_uclass_id(UCLASS_SCSI, 0) : NULL;
		unsigned long blksz = blk_dev ? blk_dev->blksz : 512;
		offset *= blksz;
	}

	if (offset % 4 != 0 || size < 4 || size % 4 != 0) {
		printf("Error: Bad A38x image blocksize.\n");
		return -ENOEXEC;
	}

	checksum32 = image_checksum32((u8 *)hdr + offset, size - 4);
	exp_checksum32 = *(u32 *)((u8 *)hdr + offset + size - 4);
	if (checksum32 != exp_checksum32) {
		printf("Error: Bad A38x image data checksum. 0x%08x != 0x%08x\n",
		       checksum32, exp_checksum32);
		return -ENOEXEC;
	}

	printf("Image checksum...OK!\n");
	return 0;
}

#if defined(CONFIG_ARMADA_38X)
static int a38x_image_is_secure(const struct a38x_main_hdr_v1 *hdr)
{
	const size_t hdr_size = a38x_header_size(hdr);
	struct a38x_opt_hdr_v1 *ohdr;
	u32 ohdr_size;

	if (hdr->version != 1)
		return 0;

	if (!hdr->ext)
		return 0;

	ohdr = (struct a38x_opt_hdr_v1 *)(hdr + 1);
	do {
		if (ohdr->headertype == A38X_OPT_HDR_V1_SECURE_TYPE)
			return 1;

		ohdr_size = (ohdr->headersz_msb << 16) | le16_to_cpu(ohdr->headersz_lsb);

		if (!*((u8 *)ohdr + ohdr_size - 4))
			break;

		ohdr = (struct a38x_opt_hdr_v1 *)((u8 *)ohdr + ohdr_size);
		if ((u8 *)ohdr >= (u8 *)hdr + hdr_size)
			break;
	} while (1);

	return 0;
}
#endif
#else /* Not ARMADA? */
static int check_image_header(void)
{
	printf("bubt cmd does not support this SoC device or family!\n");
	return -ENOEXEC;
}
#endif

#if defined(CONFIG_ARMADA_3700) || defined(CONFIG_ARMADA_38X)
static u64 fuse_read_u64(u32 bank)
{
	u32 val[2];
	int ret;

	ret = fuse_read(bank, 0, &val[0]);
	if (ret < 0)
		return -1;

	ret = fuse_read(bank, 1, &val[1]);
	if (ret < 0)
		return -1;

	return ((u64)val[1] << 32) | val[0];
}
#endif

#if defined(CONFIG_ARMADA_3700)
static inline u8 maj3(u8 val)
{
	/* return majority vote of 3 bits */
	return ((val & 0x7) == 3 || (val & 0x7) > 4) ? 1 : 0;
}
#endif

static int bubt_check_boot_mode(const struct bubt_dev *dst)
{
#if defined(CONFIG_ARMADA_3700) || defined(CONFIG_ARMADA_32BIT)
	int mode;
#if defined(CONFIG_ARMADA_3700) || defined(CONFIG_ARMADA_38X)
	int secure_mode;
#endif
#if defined(CONFIG_ARMADA_3700)
	const struct tim_boot_flash_sign *boot_modes = tim_boot_flash_signs;
	const struct common_tim_data *hdr =
		(struct common_tim_data *)get_load_addr();
	u32 id = hdr->boot_flash_sign;
	int is_secure = hdr->trusted != 0;
	u64 otp_secure_bits = fuse_read_u64(1);
	int otp_secure_boot = ((maj3(otp_secure_bits >> 0) << 0) |
			       (maj3(otp_secure_bits >> 4) << 1)) == 2;
	unsigned int otp_boot_device = (maj3(otp_secure_bits >> 48) << 0) |
				       (maj3(otp_secure_bits >> 52) << 1) |
				       (maj3(otp_secure_bits >> 56) << 2) |
				       (maj3(otp_secure_bits >> 60) << 3);
#elif defined(CONFIG_ARMADA_32BIT)
	const struct a38x_boot_mode *boot_modes = a38x_boot_modes;
	const struct a38x_main_hdr_v1 *hdr =
		(struct a38x_main_hdr_v1 *)get_load_addr();
	u32 id = hdr->blockid;
#if defined(CONFIG_ARMADA_38X)
	int is_secure = a38x_image_is_secure(hdr);
	u64 otp_secure_bits = fuse_read_u64(EFUSE_LINE_SECURE_BOOT);
	int otp_secure_boot = otp_secure_bits & 0x1;
	unsigned int otp_boot_device = (otp_secure_bits >> 8) & 0x7;
#endif
#endif

	for (mode = 0; boot_modes[mode].name; mode++) {
		if (boot_modes[mode].id == id)
			break;
	}

	if (!boot_modes[mode].name) {
		printf("Error: unknown boot device in image header: 0x%x\n", id);
		return -ENOEXEC;
	}

	if (strcmp(boot_modes[mode].name, dst->name) != 0) {
		printf("Error: image meant to be booted from \"%s\", not \"%s\"!\n",
		       boot_modes[mode].name, dst->name);
		return -ENOEXEC;
	}

#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_3700)
	if (otp_secure_bits == (u64)-1) {
		printf("Error: cannot read OTP secure bits\n");
		return -ENOEXEC;
	} else {
		if (otp_secure_boot && !is_secure) {
			printf("Error: secure boot is enabled in OTP but image does not have secure boot header!\n");
			return -ENOEXEC;
		} else if (!otp_secure_boot && is_secure) {
#if defined(CONFIG_ARMADA_3700)
			/*
			 * Armada 3700 BootROM rejects trusted image when secure boot is not enabled.
			 * Armada 385 BootROM accepts image with secure boot header also when secure boot is not enabled.
			 */
			printf("Error: secure boot is disabled in OTP but image has secure boot header!\n");
			return -ENOEXEC;
#endif
		} else if (otp_boot_device && otp_boot_device != id) {
			for (secure_mode = 0; boot_modes[secure_mode].name; secure_mode++) {
				if (boot_modes[secure_mode].id == otp_boot_device)
					break;
			}
			printf("Error: boot source is set to \"%s\" in OTP but image is for \"%s\"!\n",
			       boot_modes[secure_mode].name ?: "unknown", dst->name);
			return -ENOEXEC;
		}
	}
#endif
#endif
	return 0;
}

static int bubt_verify(const struct bubt_dev *dst)
{
	int err;

	/* Check a correct image header exists */
	err = check_image_header();
	if (err) {
		printf("Error: Image header verification failed\n");
		return err;
	}

	err = bubt_check_boot_mode(dst);
	if (err) {
		printf("Error: Image boot mode verification failed\n");
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
		printf("Device \"%s\" not supported by U-Boot image\n",
		       dev->name);
		return 0;
	}

	if (!dev->active()) {
		printf("Device \"%s\" is inactive\n", dev->name);
		return 0;
	}

	return 1;
}

static struct bubt_dev *find_bubt_dev(char *dev_name)
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
#elif defined(CONFIG_MVEBU_SATA_BOOT)
#define DEFAULT_BUBT_DST "sata"
#else
#define DEFAULT_BUBT_DST "error"
#endif
#endif /* DEFAULT_BUBT_DST */

static int do_bubt_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
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
		return 1;
	}

	if (!bubt_is_dev_active(dst))
		return 1;

	/* Figure out the source device */
	src = find_bubt_dev(src_dev_name);
	if (!src) {
		printf("Error: Unknown source \"%s\"\n", src_dev_name);
		return 1;
	}

	if (!bubt_is_dev_active(src))
		return -ENODEV;

	printf("Burning U-Boot image \"%s\" from \"%s\" to \"%s\"\n",
	       net_boot_file_name, src->name, dst->name);

	image_size = bubt_read_file(src);
	if (!image_size)
		return 1;

	err = bubt_verify(dst);
	if (err)
		return 1;

	err = bubt_write_file(dst, image_size);
	if (err)
		return 1;

	return 0;
}

U_BOOT_CMD(
	bubt, 4, 0, do_bubt_cmd,
	"Burn a u-boot image to flash",
	"[file-name] [destination [source]]\n"
	"\t-file-name     The image file name to burn. Default = " CONFIG_MVEBU_UBOOT_DFLT_NAME "\n"
	"\t-destination   Flash to burn to [spi, nand, mmc, sata]. Default = " DEFAULT_BUBT_DST "\n"
	"\t-source        The source to load image from [tftp, usb, mmc, sata]. Default = " DEFAULT_BUBT_SRC "\n"
	"Examples:\n"
	"\tbubt - Burn flash-image.bin from tftp to active boot device\n"
	"\tbubt flash-image-new.bin nand - Burn flash-image-new.bin from tftp to NAND flash\n"
	"\tbubt backup-flash-image.bin mmc usb - Burn backup-flash-image.bin from usb to MMC\n"

);
