// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008 RuggedCom, Inc.
 * Richard Retanubun <RichardRetanubun@RuggedCom.com>
 */

/*
 * NOTE:
 *   when CONFIG_SYS_64BIT_LBA is not defined, lbaint_t is 32 bits; this
 *   limits the maximum size of addressable storage to < 2 tebibytes
 */

#define LOG_CATEGORY LOGC_FS

#include <common.h>
#include <blk.h>
#include <log.h>
#include <part.h>
#include <uuid.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/unaligned.h>
#include <command.h>
#include <fdtdec.h>
#include <ide.h>
#include <malloc.h>
#include <memalign.h>
#include <part_efi.h>
#include <dm/ofnode.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <linux/printk.h>
#include <u-boot/crc.h>

/* GUID for basic data partitons */
#if CONFIG_IS_ENABLED(EFI_PARTITION)
static const efi_guid_t partition_basic_data_guid = PARTITION_BASIC_DATA_GUID;
#endif

/**
 * efi_crc32() - EFI version of crc32 function
 * @buf: buffer to calculate crc32 of
 * @len - length of buf
 *
 * Description: Returns EFI-style CRC32 value for @buf
 */
static inline u32 efi_crc32(const void *buf, u32 len)
{
	return crc32(0, buf, len);
}

/*
 * Private function prototypes
 */

static int pmbr_part_valid(struct partition *part);
static int is_pmbr_valid(legacy_mbr * mbr);
static int is_gpt_valid(struct blk_desc *desc, u64 lba, gpt_header *pgpt_head,
			gpt_entry **pgpt_pte);
static gpt_entry *alloc_read_gpt_entries(struct blk_desc *desc,
					 gpt_header *pgpt_head);
static int is_pte_valid(gpt_entry * pte);
static int find_valid_gpt(struct blk_desc *desc, gpt_header *gpt_head,
			  gpt_entry **pgpt_pte);

static char *print_efiname(gpt_entry *pte)
{
	static char name[PARTNAME_SZ + 1];
	int i;
	for (i = 0; i < PARTNAME_SZ; i++) {
		u8 c;
		c = pte->partition_name[i] & 0xff;
		c = (c && !isprint(c)) ? '.' : c;
		name[i] = c;
	}
	name[PARTNAME_SZ] = 0;
	return name;
}

static const efi_guid_t system_guid = PARTITION_SYSTEM_GUID;

static int get_bootable(gpt_entry *p)
{
	int ret = 0;

	if (!memcmp(&p->partition_type_guid, &system_guid, sizeof(efi_guid_t)))
		ret |=  PART_EFI_SYSTEM_PARTITION;
	if (p->attributes.fields.legacy_bios_bootable)
		ret |=  PART_BOOTABLE;
	return ret;
}

static int validate_gpt_header(gpt_header *gpt_h, lbaint_t lba,
		lbaint_t lastlba)
{
	uint32_t crc32_backup = 0;
	uint32_t calc_crc32;

	/* Check the GPT header signature */
	if (le64_to_cpu(gpt_h->signature) != GPT_HEADER_SIGNATURE_UBOOT) {
		log_debug("%s signature is wrong: %#llX != %#llX\n",
			  "GUID Partition Table Header",
			  le64_to_cpu(gpt_h->signature),
			  GPT_HEADER_SIGNATURE_UBOOT);
		return -1;
	}

	/* Check the GUID Partition Table CRC */
	memcpy(&crc32_backup, &gpt_h->header_crc32, sizeof(crc32_backup));
	memset(&gpt_h->header_crc32, 0, sizeof(gpt_h->header_crc32));

	calc_crc32 = efi_crc32((const unsigned char *)gpt_h,
		le32_to_cpu(gpt_h->header_size));

	memcpy(&gpt_h->header_crc32, &crc32_backup, sizeof(crc32_backup));

	if (calc_crc32 != le32_to_cpu(crc32_backup)) {
		log_debug("%s: CRC is wrong: %#x != %#x\n",
			  "GUID Partition Table Header",
			  le32_to_cpu(crc32_backup), calc_crc32);
		return -1;
	}

	/*
	 * Check that the my_lba entry points to the LBA that contains the GPT
	 */
	if (le64_to_cpu(gpt_h->my_lba) != lba) {
		log_debug("GPT: my_lba incorrect: %llX != " LBAF "\n",
			  le64_to_cpu(gpt_h->my_lba), lba);
		return -1;
	}

	/*
	 * Check that the first_usable_lba and that the last_usable_lba are
	 * within the disk.
	 */
	if (le64_to_cpu(gpt_h->first_usable_lba) > lastlba) {
		log_debug("GPT: first_usable_lba incorrect: %llX > " LBAF "\n",
			  le64_to_cpu(gpt_h->first_usable_lba), lastlba);
		return -1;
	}
	if (le64_to_cpu(gpt_h->last_usable_lba) > lastlba) {
		log_debug("GPT: last_usable_lba incorrect: %llX > " LBAF "\n",
			  le64_to_cpu(gpt_h->last_usable_lba), lastlba);
		return -1;
	}

	debug("GPT: first_usable_lba: %llX last_usable_lba: %llX last lba: "
	      LBAF "\n", le64_to_cpu(gpt_h->first_usable_lba),
	      le64_to_cpu(gpt_h->last_usable_lba), lastlba);

	return 0;
}

static int validate_gpt_entries(gpt_header *gpt_h, gpt_entry *gpt_e)
{
	uint32_t calc_crc32;

	/* Check the GUID Partition Table Entry Array CRC */
	calc_crc32 = efi_crc32((const unsigned char *)gpt_e,
		le32_to_cpu(gpt_h->num_partition_entries) *
		le32_to_cpu(gpt_h->sizeof_partition_entry));

	if (calc_crc32 != le32_to_cpu(gpt_h->partition_entry_array_crc32)) {
		log_debug("%s: %#x != %#x\n",
			  "GUID Partition Table Entry Array CRC is wrong",
			  le32_to_cpu(gpt_h->partition_entry_array_crc32),
			  calc_crc32);
		return -1;
	}

	return 0;
}

static void prepare_backup_gpt_header(gpt_header *gpt_h)
{
	uint32_t calc_crc32;
	uint64_t val;

	/* recalculate the values for the Backup GPT Header */
	val = le64_to_cpu(gpt_h->my_lba);
	gpt_h->my_lba = gpt_h->alternate_lba;
	gpt_h->alternate_lba = cpu_to_le64(val);
	gpt_h->partition_entry_lba =
			cpu_to_le64(le64_to_cpu(gpt_h->last_usable_lba) + 1);
	gpt_h->header_crc32 = 0;

	calc_crc32 = efi_crc32((const unsigned char *)gpt_h,
			       le32_to_cpu(gpt_h->header_size));
	gpt_h->header_crc32 = cpu_to_le32(calc_crc32);
}

#if CONFIG_IS_ENABLED(EFI_PARTITION)
/*
 * Public Functions (include/part.h)
 */

/*
 * UUID is displayed as 32 hexadecimal digits, in 5 groups,
 * separated by hyphens, in the form 8-4-4-4-12 for a total of 36 characters
 */
int get_disk_guid(struct blk_desc *desc, char *guid)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1, desc->blksz);
	gpt_entry *gpt_pte = NULL;
	unsigned char *guid_bin;

	/* This function validates AND fills in the GPT header and PTE */
	if (find_valid_gpt(desc, gpt_head, &gpt_pte) != 1)
		return -EINVAL;

	guid_bin = gpt_head->disk_guid.b;
	uuid_bin_to_str(guid_bin, guid, UUID_STR_FORMAT_GUID);

	/* Remember to free pte */
	free(gpt_pte);
	return 0;
}

void part_print_efi(struct blk_desc *desc)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1, desc->blksz);
	gpt_entry *gpt_pte = NULL;
	int i = 0;
	unsigned char *uuid;

	/* This function validates AND fills in the GPT header and PTE */
	if (find_valid_gpt(desc, gpt_head, &gpt_pte) != 1)
		return;

	debug("%s: gpt-entry at %p\n", __func__, gpt_pte);

	printf("Part\tStart LBA\tEnd LBA\t\tName\n");
	printf("\tAttributes\n");
	printf("\tType GUID\n");
	printf("\tPartition GUID\n");

	for (i = 0; i < le32_to_cpu(gpt_head->num_partition_entries); i++) {
		/* Skip invalid PTE */
		if (!is_pte_valid(&gpt_pte[i]))
			continue;

		printf("%3d\t0x%08llx\t0x%08llx\t\"%s\"\n", (i + 1),
			le64_to_cpu(gpt_pte[i].starting_lba),
			le64_to_cpu(gpt_pte[i].ending_lba),
			print_efiname(&gpt_pte[i]));
		printf("\tattrs:\t0x%016llx\n", gpt_pte[i].attributes.raw);
		uuid = (unsigned char *)gpt_pte[i].partition_type_guid.b;
		if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))
			printf("\ttype:\t%pUl\n\t\t(%pUs)\n", uuid, uuid);
		else
			printf("\ttype:\t%pUl\n", uuid);
		uuid = (unsigned char *)gpt_pte[i].unique_partition_guid.b;
		printf("\tguid:\t%pUl\n", uuid);
	}

	/* Remember to free pte */
	free(gpt_pte);
	return;
}

int part_get_info_efi(struct blk_desc *desc, int part,
		      struct disk_partition *info)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1, desc->blksz);
	gpt_entry *gpt_pte = NULL;

	/* "part" argument must be at least 1 */
	if (part < 1) {
		log_debug("Invalid Argument(s)\n");
		return -EINVAL;
	}

	/* This function validates AND fills in the GPT header and PTE */
	if (find_valid_gpt(desc, gpt_head, &gpt_pte) != 1)
		return -EINVAL;

	if (part > le32_to_cpu(gpt_head->num_partition_entries) ||
	    !is_pte_valid(&gpt_pte[part - 1])) {
		log_debug("Invalid partition number %d\n", part);
		free(gpt_pte);
		return -EPERM;
	}

	/* The 'lbaint_t' casting may limit the maximum disk size to 2 TB */
	info->start = (lbaint_t)le64_to_cpu(gpt_pte[part - 1].starting_lba);
	/* The ending LBA is inclusive, to calculate size, add 1 to it */
	info->size = (lbaint_t)le64_to_cpu(gpt_pte[part - 1].ending_lba) + 1
		     - info->start;
	info->blksz = desc->blksz;

	snprintf((char *)info->name, sizeof(info->name), "%s",
		 print_efiname(&gpt_pte[part - 1]));
	strcpy((char *)info->type, "U-Boot");
	info->bootable = get_bootable(&gpt_pte[part - 1]);
	if (CONFIG_IS_ENABLED(PARTITION_UUIDS)) {
		uuid_bin_to_str(gpt_pte[part - 1].unique_partition_guid.b,
				(char *)disk_partition_uuid(info),
				UUID_STR_FORMAT_GUID);
	}
	if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID)) {
		uuid_bin_to_str(gpt_pte[part - 1].partition_type_guid.b,
				(char *)disk_partition_type_guid(info),
				UUID_STR_FORMAT_GUID);
	}

	log_debug("start 0x" LBAF ", size 0x" LBAF ", name %s\n", info->start,
		  info->size, info->name);

	/* Remember to free pte */
	free(gpt_pte);
	return 0;
}

static int part_test_efi(struct blk_desc *desc)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(legacy_mbr, legacymbr, 1, desc->blksz);

	/* Read legacy MBR from block 0 and validate it */
	if ((blk_dread(desc, 0, 1, (ulong *)legacymbr) != 1)
		|| (is_pmbr_valid(legacymbr) != 1)) {
		return -1;
	}
	return 0;
}

/**
 * set_protective_mbr(): Set the EFI protective MBR
 * @param desc - block device descriptor
 *
 * Return: - zero on success, otherwise error
 */
static int set_protective_mbr(struct blk_desc *desc)
{
	/* Setup the Protective MBR */
	ALLOC_CACHE_ALIGN_BUFFER_PAD(legacy_mbr, p_mbr, 1, desc->blksz);
	if (p_mbr == NULL) {
		log_debug("calloc failed!\n");
		return -ENOMEM;
	}

	/* Read MBR to backup boot code if it exists */
	if (blk_dread(desc, 0, 1, p_mbr) != 1) {
		log_debug("** Can't read from device %d **\n",
			  desc->devnum);
		return -EIO;
	}

	/* Clear all data in MBR except of backed up boot code */
	memset((char *)p_mbr + MSDOS_MBR_BOOT_CODE_SIZE, 0, sizeof(*p_mbr) -
			MSDOS_MBR_BOOT_CODE_SIZE);

	/* Append signature */
	p_mbr->signature = MSDOS_MBR_SIGNATURE;
	p_mbr->partition_record[0].sys_ind = EFI_PMBR_OSTYPE_EFI_GPT;
	p_mbr->partition_record[0].start_sect = 1;
	p_mbr->partition_record[0].nr_sects = (u32)desc->lba - 1;

	/* Write MBR sector to the MMC device */
	if (blk_dwrite(desc, 0, 1, p_mbr) != 1) {
		log_debug("** Can't write to device %d **\n", desc->devnum);
		return -EIO;
	}

	return 0;
}

int write_gpt_table(struct blk_desc *desc, gpt_header *gpt_h, gpt_entry *gpt_e)
{
	const int pte_blk_cnt = BLOCK_CNT((gpt_h->num_partition_entries
					   * sizeof(gpt_entry)), desc);
	u32 calc_crc32;

	debug("max lba: %x\n", (u32)desc->lba);
	/* Setup the Protective MBR */
	if (set_protective_mbr(desc) < 0)
		goto err;

	/* Generate CRC for the Primary GPT Header */
	calc_crc32 = efi_crc32((const unsigned char *)gpt_e,
			      le32_to_cpu(gpt_h->num_partition_entries) *
			      le32_to_cpu(gpt_h->sizeof_partition_entry));
	gpt_h->partition_entry_array_crc32 = cpu_to_le32(calc_crc32);

	calc_crc32 = efi_crc32((const unsigned char *)gpt_h,
			      le32_to_cpu(gpt_h->header_size));
	gpt_h->header_crc32 = cpu_to_le32(calc_crc32);

	/* Write the First GPT to the block right after the Legacy MBR */
	if (blk_dwrite(desc, 1, 1, gpt_h) != 1)
		goto err;

	if (blk_dwrite(desc, le64_to_cpu(gpt_h->partition_entry_lba),
		       pte_blk_cnt, gpt_e) != pte_blk_cnt)
		goto err;

	prepare_backup_gpt_header(gpt_h);

	if (blk_dwrite(desc, (lbaint_t)le64_to_cpu(gpt_h->last_usable_lba)
		       + 1, pte_blk_cnt, gpt_e) != pte_blk_cnt)
		goto err;

	if (blk_dwrite(desc, (lbaint_t)le64_to_cpu(gpt_h->my_lba), 1,
		       gpt_h) != 1)
		goto err;

	debug("GPT successfully written to block device!\n");
	return 0;

 err:
	log_debug("** Can't write to device %d **\n", desc->devnum);
	return -EIO;
}

int gpt_fill_pte(struct blk_desc *desc,
		 gpt_header *gpt_h, gpt_entry *gpt_e,
		 struct disk_partition *partitions, int parts)
{
	lbaint_t offset = (lbaint_t)le64_to_cpu(gpt_h->first_usable_lba);
	lbaint_t last_usable_lba = (lbaint_t)
			le64_to_cpu(gpt_h->last_usable_lba);
	int i, k;
	size_t efiname_len, dosname_len;
	unsigned char *bin_uuid;
#ifdef CONFIG_PARTITION_TYPE_GUID
	char *str_type_guid;
	unsigned char *bin_type_guid;
#endif
	size_t hdr_start = gpt_h->my_lba;
	size_t hdr_end = hdr_start + 1;

	size_t pte_start = gpt_h->partition_entry_lba;
	size_t pte_end = pte_start +
		gpt_h->num_partition_entries * gpt_h->sizeof_partition_entry /
		desc->blksz;

	for (i = 0; i < parts; i++) {
		/* partition starting lba */
		lbaint_t start = partitions[i].start;
		lbaint_t size = partitions[i].size;

		if (start) {
			offset = start + size;
		} else {
			start = offset;
			offset += size;
		}

		/*
		 * If our partition overlaps with either the GPT
		 * header, or the partition entry, reject it.
		 */
		if (((start < hdr_end && hdr_start < (start + size)) ||
		     (start < pte_end && pte_start < (start + size)))) {
			log_debug("Partition overlap\n");
			return -ENOSPC;
		}

		gpt_e[i].starting_lba = cpu_to_le64(start);

		if (offset > (last_usable_lba + 1)) {
			log_debug("Partitions layout exceeds disk size\n");
			return -E2BIG;
		}
		/* partition ending lba */
		if ((i == parts - 1) && (size == 0))
			/* extend the last partition to maximuim */
			gpt_e[i].ending_lba = gpt_h->last_usable_lba;
		else
			gpt_e[i].ending_lba = cpu_to_le64(offset - 1);

#ifdef CONFIG_PARTITION_TYPE_GUID
		str_type_guid = partitions[i].type_guid;
		bin_type_guid = gpt_e[i].partition_type_guid.b;
		if (strlen(str_type_guid)) {
			if (uuid_str_to_bin(str_type_guid, bin_type_guid,
					    UUID_STR_FORMAT_GUID)) {
				log_debug("Partition no. %d: invalid type guid: %s\n",
					  i, str_type_guid);
				return -EINVAL;
			}
		} else {
			/* default partition type GUID */
			memcpy(bin_type_guid,
			       &partition_basic_data_guid, 16);
		}
#else
		/* partition type GUID */
		memcpy(gpt_e[i].partition_type_guid.b,
			&partition_basic_data_guid, 16);
#endif

		if (CONFIG_IS_ENABLED(PARTITION_UUIDS)) {
			const char *str_uuid;

			str_uuid = disk_partition_uuid(&partitions[i]);
			bin_uuid = gpt_e[i].unique_partition_guid.b;

			if (uuid_str_to_bin(str_uuid, bin_uuid,
					    UUID_STR_FORMAT_GUID)) {
				log_debug("Partition no. %d: invalid guid: %s\n",
					  i, str_uuid);
				return -EINVAL;
			}
		}

		/* partition attributes */
		memset(&gpt_e[i].attributes, 0,
		       sizeof(gpt_entry_attributes));

		if (partitions[i].bootable & PART_BOOTABLE)
			gpt_e[i].attributes.fields.legacy_bios_bootable = 1;

		/* partition name */
		efiname_len = sizeof(gpt_e[i].partition_name)
			/ sizeof(efi_char16_t);
		dosname_len = sizeof(partitions[i].name);

		memset(gpt_e[i].partition_name, 0,
		       sizeof(gpt_e[i].partition_name));

		for (k = 0; k < min(dosname_len, efiname_len); k++)
			gpt_e[i].partition_name[k] =
				(efi_char16_t)(partitions[i].name[k]);

		debug("%s: name: %s offset[%d]: 0x" LBAF
		      " size[%d]: 0x" LBAF "\n",
		      __func__, partitions[i].name, i,
		      offset, i, size);
	}

	return 0;
}

static uint32_t partition_entries_offset(struct blk_desc *desc)
{
	uint32_t offset_blks = 2;
	uint32_t __maybe_unused offset_bytes;
	int __maybe_unused config_offset;

#if defined(CONFIG_EFI_PARTITION_ENTRIES_OFF)
	/*
	 * Some architectures require their SPL loader at a fixed
	 * address within the first 16KB of the disk.  To avoid an
	 * overlap with the partition entries of the EFI partition
	 * table, the first safe offset (in bytes, from the start of
	 * the disk) for the entries can be set in
	 * CONFIG_EFI_PARTITION_ENTRIES_OFF.
	 */
	offset_bytes =
		PAD_TO_BLOCKSIZE(CONFIG_EFI_PARTITION_ENTRIES_OFF, desc);
	offset_blks = offset_bytes / desc->blksz;
#endif

#if defined(CONFIG_OF_CONTROL)
	/*
	 * Allow the offset of the first partition entires (in bytes
	 * from the start of the device) to be specified as a property
	 * of the device tree '/config' node.
	 */
	config_offset = ofnode_conf_read_int(
		"u-boot,efi-partition-entries-offset", -EINVAL);
	if (config_offset != -EINVAL) {
		offset_bytes = PAD_TO_BLOCKSIZE(config_offset, desc);
		offset_blks = offset_bytes / desc->blksz;
	}
#endif

	debug("efi: partition entries offset (in blocks): %d\n", offset_blks);

	/*
	 * The earliest LBA this can be at is LBA#2 (i.e. right behind
	 * the (protective) MBR and the GPT header.
	 */
	if (offset_blks < 2)
		offset_blks = 2;

	return offset_blks;
}

int gpt_fill_header(struct blk_desc *desc, gpt_header *gpt_h, char *str_guid,
		    int parts_count)
{
	gpt_h->signature = cpu_to_le64(GPT_HEADER_SIGNATURE_UBOOT);
	gpt_h->revision = cpu_to_le32(GPT_HEADER_REVISION_V1);
	gpt_h->header_size = cpu_to_le32(sizeof(gpt_header));
	gpt_h->my_lba = cpu_to_le64(1);
	gpt_h->alternate_lba = cpu_to_le64(desc->lba - 1);
	gpt_h->last_usable_lba = cpu_to_le64(desc->lba - 34);
	gpt_h->partition_entry_lba =
		cpu_to_le64(partition_entries_offset(desc));
	gpt_h->first_usable_lba =
		cpu_to_le64(le64_to_cpu(gpt_h->partition_entry_lba) + 32);
	gpt_h->num_partition_entries = cpu_to_le32(GPT_ENTRY_NUMBERS);
	gpt_h->sizeof_partition_entry = cpu_to_le32(sizeof(gpt_entry));
	gpt_h->header_crc32 = 0;
	gpt_h->partition_entry_array_crc32 = 0;

	if (uuid_str_to_bin(str_guid, gpt_h->disk_guid.b, UUID_STR_FORMAT_GUID))
		return -1;

	return 0;
}

int gpt_restore(struct blk_desc *desc, char *str_disk_guid,
		struct disk_partition *partitions, int parts_count)
{
	gpt_header *gpt_h;
	gpt_entry *gpt_e;
	int ret, size;

	size = PAD_TO_BLOCKSIZE(sizeof(gpt_header), desc);
	gpt_h = malloc_cache_aligned(size);
	if (gpt_h == NULL) {
		log_debug("calloc failed!\n");
		return -ENOMEM;
	}
	memset(gpt_h, 0, size);

	size = PAD_TO_BLOCKSIZE(GPT_ENTRY_NUMBERS * sizeof(gpt_entry),
				desc);
	gpt_e = malloc_cache_aligned(size);
	if (gpt_e == NULL) {
		log_debug("calloc failed!\n");
		free(gpt_h);
		return -ENOMEM;
	}
	memset(gpt_e, 0, size);

	/* Generate Primary GPT header (LBA1) */
	ret = gpt_fill_header(desc, gpt_h, str_disk_guid, parts_count);
	if (ret)
		goto err;

	/* Generate partition entries */
	ret = gpt_fill_pte(desc, gpt_h, gpt_e, partitions, parts_count);
	if (ret)
		goto err;

	/* Write GPT partition table */
	ret = write_gpt_table(desc, gpt_h, gpt_e);

err:
	free(gpt_e);
	free(gpt_h);
	return ret;
}

/**
 * gpt_convert_efi_name_to_char() - convert u16 string to char string
 *
 * TODO: this conversion only supports ANSI characters
 *
 * @s:	target buffer
 * @es:	u16 string to be converted
 * @n:	size of target buffer
 */
static void gpt_convert_efi_name_to_char(char *s, void *es, int n)
{
	char *ess = es;
	int i, j;

	memset(s, '\0', n);

	for (i = 0, j = 0; j < n; i += 2, j++) {
		s[j] = ess[i];
		if (!ess[i])
			return;
	}
}

int gpt_verify_headers(struct blk_desc *desc, gpt_header *gpt_head,
		       gpt_entry **gpt_pte)
{
	/*
	 * This function validates AND
	 * fills in the GPT header and PTE
	 */
	if (is_gpt_valid(desc,
			 GPT_PRIMARY_PARTITION_TABLE_LBA,
			 gpt_head, gpt_pte) != 1) {
		log_debug("Invalid GPT\n");
		return -1;
	}

	/* Free pte before allocating again */
	free(*gpt_pte);

	/*
	 * Check that the alternate_lba entry points to the last LBA
	 */
	if (le64_to_cpu(gpt_head->alternate_lba) != (desc->lba - 1)) {
		log_debug("Misplaced Backup GPT\n");
		return -1;
	}

	if (is_gpt_valid(desc, (desc->lba - 1),
			 gpt_head, gpt_pte) != 1) {
		log_debug("Invalid Backup GPT\n");
		return -1;
	}

	return 0;
}

static void restore_primary_gpt_header(gpt_header *gpt_h, struct blk_desc *desc)
{
	u32 calc_crc32;
	u64 val;

	/* recalculate the values for the Primary GPT Header */
	val = le64_to_cpu(gpt_h->my_lba);
	gpt_h->my_lba = gpt_h->alternate_lba;
	gpt_h->alternate_lba = cpu_to_le64(val);
	gpt_h->partition_entry_lba = cpu_to_le64(partition_entries_offset(desc));

	gpt_h->header_crc32 = 0;

	calc_crc32 = efi_crc32((const unsigned char *)gpt_h,
			       le32_to_cpu(gpt_h->header_size));
	gpt_h->header_crc32 = cpu_to_le32(calc_crc32);
}

static int write_one_gpt_table(struct blk_desc *desc, gpt_header *gpt_h,
			       gpt_entry *gpt_e)
{
	const int pte_blk_cnt = BLOCK_CNT((gpt_h->num_partition_entries
					   * sizeof(gpt_entry)), desc);
	lbaint_t start;
	int ret = 0;

	start = le64_to_cpu(gpt_h->my_lba);
	if (blk_dwrite(desc, start, 1, gpt_h) != 1) {
		ret = -1;
		goto out;
	}

	start = le64_to_cpu(gpt_h->partition_entry_lba);
	if (blk_dwrite(desc, start, pte_blk_cnt, gpt_e) != pte_blk_cnt) {
		ret = -1;
		goto out;
	}

 out:
	return ret;
}

int gpt_repair_headers(struct blk_desc *desc)
{
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_h1, 1, desc->blksz);
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_h2, 1, desc->blksz);
	gpt_entry *gpt_e1 = NULL, *gpt_e2 = NULL;
	int is_gpt1_valid, is_gpt2_valid;
	int ret = -1;

	is_gpt1_valid = is_gpt_valid(desc, GPT_PRIMARY_PARTITION_TABLE_LBA,
				     gpt_h1, &gpt_e1);
	is_gpt2_valid = is_gpt_valid(desc, desc->lba - 1,
				     gpt_h2, &gpt_e2);

	if (is_gpt1_valid && is_gpt2_valid) {
		ret = 0;
		goto out;
	}

	if (is_gpt1_valid && !is_gpt2_valid) {
		prepare_backup_gpt_header(gpt_h1);
		ret = write_one_gpt_table(desc, gpt_h1, gpt_e1);
		goto out;
	}

	if (!is_gpt1_valid && is_gpt2_valid) {
		restore_primary_gpt_header(gpt_h2, desc);
		ret = write_one_gpt_table(desc, gpt_h2, gpt_e2);
		goto out;
	}

	if (!is_gpt1_valid && !is_gpt2_valid) {
		ret = -1;
		goto out;
	}

 out:
	if (is_gpt1_valid)
		free(gpt_e1);
	if (is_gpt2_valid)
		free(gpt_e2);

	return ret;
}

int gpt_verify_partitions(struct blk_desc *desc,
			  struct disk_partition *partitions, int parts,
			  gpt_header *gpt_head, gpt_entry **gpt_pte)
{
	char efi_str[PARTNAME_SZ + 1];
	u64 gpt_part_size;
	gpt_entry *gpt_e;
	int ret, i;

	ret = gpt_verify_headers(desc, gpt_head, gpt_pte);
	if (ret)
		return ret;

	gpt_e = *gpt_pte;

	for (i = 0; i < parts; i++) {
		if (i == gpt_head->num_partition_entries) {
			pr_err("More partitions than allowed!\n");
			return -1;
		}

		/* Check if GPT and ENV partition names match */
		gpt_convert_efi_name_to_char(efi_str, gpt_e[i].partition_name,
					     PARTNAME_SZ + 1);

		debug("%s: part: %2d name - GPT: %16s, ENV: %16s ",
		      __func__, i, efi_str, partitions[i].name);

		if (strncmp(efi_str, (char *)partitions[i].name,
			    sizeof(partitions->name))) {
			pr_err("Partition name: %s does not match %s!\n",
			      efi_str, (char *)partitions[i].name);
			return -1;
		}

		/* Check if GPT and ENV sizes match */
		gpt_part_size = le64_to_cpu(gpt_e[i].ending_lba) -
			le64_to_cpu(gpt_e[i].starting_lba) + 1;
		debug("size(LBA) - GPT: %8llu, ENV: %8llu ",
		      (unsigned long long)gpt_part_size,
		      (unsigned long long)partitions[i].size);

		if (le64_to_cpu(gpt_part_size) != partitions[i].size) {
			/* We do not check the extend partition size */
			if ((i == parts - 1) && (partitions[i].size == 0))
				continue;

			pr_err("Partition %s size: %llu does not match %llu!\n",
			      efi_str, (unsigned long long)gpt_part_size,
			      (unsigned long long)partitions[i].size);
			return -1;
		}

		/*
		 * Start address is optional - check only if provided
		 * in '$partition' variable
		 */
		if (!partitions[i].start) {
			debug("\n");
			continue;
		}

		/* Check if GPT and ENV start LBAs match */
		debug("start LBA - GPT: %8llu, ENV: %8llu\n",
		      le64_to_cpu(gpt_e[i].starting_lba),
		      (unsigned long long)partitions[i].start);

		if (le64_to_cpu(gpt_e[i].starting_lba) != partitions[i].start) {
			pr_err("Partition %s start: %llu does not match %llu!\n",
			      efi_str, le64_to_cpu(gpt_e[i].starting_lba),
			      (unsigned long long)partitions[i].start);
			return -1;
		}
	}

	return 0;
}

int is_valid_gpt_buf(struct blk_desc *desc, void *buf)
{
	gpt_header *gpt_h;
	gpt_entry *gpt_e;

	/* determine start of GPT Header in the buffer */
	gpt_h = buf + (GPT_PRIMARY_PARTITION_TABLE_LBA * desc->blksz);
	if (validate_gpt_header(gpt_h, GPT_PRIMARY_PARTITION_TABLE_LBA,
				desc->lba))
		return -1;

	/* determine start of GPT Entries in the buffer */
	gpt_e = buf + (le64_to_cpu(gpt_h->partition_entry_lba) *
		       desc->blksz);
	if (validate_gpt_entries(gpt_h, gpt_e))
		return -1;

	return 0;
}

int write_mbr_and_gpt_partitions(struct blk_desc *desc, void *buf)
{
	gpt_header *gpt_h;
	gpt_entry *gpt_e;
	int gpt_e_blk_cnt;
	lbaint_t lba;
	int cnt;

	if (is_valid_gpt_buf(desc, buf))
		return -1;

	/* determine start of GPT Header in the buffer */
	gpt_h = buf + (GPT_PRIMARY_PARTITION_TABLE_LBA * desc->blksz);

	/* determine start of GPT Entries in the buffer */
	gpt_e = buf + (le64_to_cpu(gpt_h->partition_entry_lba) * desc->blksz);
	gpt_e_blk_cnt = BLOCK_CNT((le32_to_cpu(gpt_h->num_partition_entries) *
				   le32_to_cpu(gpt_h->sizeof_partition_entry)),
				  desc);

	/* write MBR */
	lba = 0;	/* MBR is always at 0 */
	cnt = 1;	/* MBR (1 block) */
	if (blk_dwrite(desc, lba, cnt, buf) != cnt) {
		log_debug("failed writing '%s' (%d blks at 0x" LBAF ")\n",
			  "MBR", cnt, lba);
		return 1;
	}

	/* write Primary GPT */
	lba = GPT_PRIMARY_PARTITION_TABLE_LBA;
	cnt = 1;	/* GPT Header (1 block) */
	if (blk_dwrite(desc, lba, cnt, gpt_h) != cnt) {
		log_debug("failed writing '%s' (%d blks at 0x" LBAF ")\n",
			  "Primary GPT Header", cnt, lba);
		return 1;
	}

	lba = le64_to_cpu(gpt_h->partition_entry_lba);
	cnt = gpt_e_blk_cnt;
	if (blk_dwrite(desc, lba, cnt, gpt_e) != cnt) {
		log_debug("failed writing '%s' (%d blks at 0x" LBAF ")\n",
			  "Primary GPT Entries", cnt, lba);
		return 1;
	}

	prepare_backup_gpt_header(gpt_h);

	/* write Backup GPT */
	lba = le64_to_cpu(gpt_h->partition_entry_lba);
	cnt = gpt_e_blk_cnt;
	if (blk_dwrite(desc, lba, cnt, gpt_e) != cnt) {
		log_debug("failed writing '%s' (%d blks at 0x" LBAF ")\n",
			  "Backup GPT Entries", cnt, lba);
		return 1;
	}

	lba = le64_to_cpu(gpt_h->my_lba);
	cnt = 1;	/* GPT Header (1 block) */
	if (blk_dwrite(desc, lba, cnt, gpt_h) != cnt) {
		log_debug("failed writing '%s' (%d blks at 0x" LBAF ")\n",
			  "Backup GPT Header", cnt, lba);
		return 1;
	}

	/* Update the partition table entries*/
	part_init(desc);

	return 0;
}
#endif

/*
 * Private functions
 */
/*
 * pmbr_part_valid(): Check for EFI partition signature
 *
 * Returns: 1 if EFI GPT partition type is found.
 */
static int pmbr_part_valid(struct partition *part)
{
	if (part->sys_ind == EFI_PMBR_OSTYPE_EFI_GPT &&
		get_unaligned_le32(&part->start_sect) == 1UL) {
		return 1;
	}

	return 0;
}

/*
 * is_pmbr_valid(): test Protective MBR for validity
 *
 * @mbr: Pointer to Master Boot-Record data
 *
 * Returns: 1 if PMBR is valid, 0 otherwise.
 * Validity depends on two things:
 *  1) MSDOS signature is in the last two bytes of the MBR
 *  2) One partition of type 0xEE is found, checked by pmbr_part_valid()
 */
static int is_pmbr_valid(legacy_mbr *mbr)
{
	uint sig = le16_to_cpu(mbr->signature);
	int i = 0;

	if (sig != MSDOS_MBR_SIGNATURE) {
		log_debug("Invalid signature %x\n", sig);
		return 0;
	}
	log_debug("Signature %x valid\n", sig);

	for (i = 0; i < 4; i++) {
		if (pmbr_part_valid(&mbr->partition_record[i])) {
			return 1;
		}
	}
	return 0;
}

/**
 * is_gpt_valid() - tests one GPT header and PTEs for validity
 *
 * lba is the logical block address of the GPT header to test
 * gpt is a GPT header ptr, filled on return.
 * ptes is a PTEs ptr, filled on return.
 *
 * Description: returns 1 if valid,  0 on error, 2 if ignored header
 * If valid, returns pointers to PTEs.
 */
static int is_gpt_valid(struct blk_desc *desc, u64 lba, gpt_header *pgpt_head,
			gpt_entry **pgpt_pte)
{
	/* Confirm valid arguments prior to allocation. */
	if (!desc || !pgpt_head) {
		log_debug("Invalid Argument(s)\n");
		return 0;
	}

	ALLOC_CACHE_ALIGN_BUFFER_PAD(legacy_mbr, mbr, 1, desc->blksz);

	/* Read MBR Header from device */
	if (blk_dread(desc, 0, 1, (ulong *)mbr) != 1) {
		log_debug("Can't read MBR header\n");
		return 0;
	}

	/* Read GPT Header from device */
	if (blk_dread(desc, (lbaint_t)lba, 1, pgpt_head) != 1) {
		log_debug("Can't read GPT header\n");
		return 0;
	}

	/* Invalid but nothing to yell about. */
	if (le64_to_cpu(pgpt_head->signature) == GPT_HEADER_CHROMEOS_IGNORE) {
		log_debug("ChromeOS 'IGNOREME' GPT header found and ignored\n");
		return 2;
	}

	if (validate_gpt_header(pgpt_head, (lbaint_t)lba, desc->lba))
		return 0;

	if (desc->sig_type == SIG_TYPE_NONE) {
		efi_guid_t empty = {};
		if (memcmp(&pgpt_head->disk_guid, &empty, sizeof(empty))) {
			desc->sig_type = SIG_TYPE_GUID;
			memcpy(&desc->guid_sig, &pgpt_head->disk_guid,
			       sizeof(empty));
		} else if (mbr->unique_mbr_signature != 0) {
			desc->sig_type = SIG_TYPE_MBR;
			desc->mbr_sig = mbr->unique_mbr_signature;
		}
	}

	/* Read and allocate Partition Table Entries */
	*pgpt_pte = alloc_read_gpt_entries(desc, pgpt_head);
	if (!*pgpt_pte)
		return 0;

	if (validate_gpt_entries(pgpt_head, *pgpt_pte)) {
		free(*pgpt_pte);
		return 0;
	}

	/* We're done, all's well */
	return 1;
}

/**
 * find_valid_gpt() - finds a valid GPT header and PTEs
 *
 * gpt is a GPT header ptr, filled on return.
 * ptes is a PTEs ptr, filled on return.
 *
 * Description: returns 1 if found a valid gpt,  0 on error.
 * If valid, returns pointers to PTEs.
 */
static int find_valid_gpt(struct blk_desc *desc, gpt_header *gpt_head,
			  gpt_entry **pgpt_pte)
{
	int r;

	r = is_gpt_valid(desc, GPT_PRIMARY_PARTITION_TABLE_LBA, gpt_head,
			 pgpt_pte);

	if (r != 1) {
		if (r != 2)
			log_debug("Invalid GPT\n");

		if (is_gpt_valid(desc, desc->lba - 1, gpt_head, pgpt_pte)
		    != 1) {
			log_debug("Invalid Backup GPT\n");
			return 0;
		}
		if (r != 2)
			log_debug("        Using Backup GPT\n");
	}
	return 1;
}

/**
 * alloc_read_gpt_entries(): reads partition entries from disk
 * @desc
 * @gpt - GPT header
 *
 * Description: Returns ptes on success,  NULL on error.
 * Allocates space for PTEs based on information found in @gpt.
 * Notes: remember to free pte when you're done!
 */
static gpt_entry *alloc_read_gpt_entries(struct blk_desc *desc,
					 gpt_header *pgpt_head)
{
	size_t count = 0, blk_cnt;
	lbaint_t blk;
	gpt_entry *pte = NULL;

	if (!desc || !pgpt_head) {
		log_debug("Invalid Argument(s)\n");
		return NULL;
	}

	count = le32_to_cpu(pgpt_head->num_partition_entries) *
		le32_to_cpu(pgpt_head->sizeof_partition_entry);

	log_debug("count = %u * %u = %lu\n",
		  (u32)le32_to_cpu(pgpt_head->num_partition_entries),
		  (u32)le32_to_cpu(pgpt_head->sizeof_partition_entry),
		  (ulong)count);

	/* Allocate memory for PTE, remember to FREE */
	if (count != 0) {
		pte = memalign(ARCH_DMA_MINALIGN,
			       PAD_TO_BLOCKSIZE(count, desc));
	}

	if (count == 0 || pte == NULL) {
		log_debug("ERROR: Can't allocate %#lX bytes for GPT Entries\n",
			  (ulong)count);
		return NULL;
	}

	/* Read GPT Entries from device */
	blk = le64_to_cpu(pgpt_head->partition_entry_lba);
	blk_cnt = BLOCK_CNT(count, desc);
	if (blk_dread(desc, blk, (lbaint_t)blk_cnt, pte) != blk_cnt) {
		log_debug("Can't read GPT Entries\n");
		free(pte);
		return NULL;
	}
	return pte;
}

/**
 * is_pte_valid(): validates a single Partition Table Entry
 * @gpt_entry - Pointer to a single Partition Table Entry
 *
 * Description: returns 1 if valid,  0 on error.
 */
static int is_pte_valid(gpt_entry * pte)
{
	efi_guid_t unused_guid;

	if (!pte) {
		log_debug("Invalid Argument(s)\n");
		return 0;
	}

	/* Only one validation for now:
	 * The GUID Partition Type != Unused Entry (ALL-ZERO)
	 */
	memset(unused_guid.b, 0, sizeof(unused_guid.b));

	if (memcmp(pte->partition_type_guid.b, unused_guid.b,
		sizeof(unused_guid.b)) == 0) {

		log_debug("Found an unused PTE GUID at 0x%08X\n",
			  (unsigned int)(uintptr_t)pte);

		return 0;
	} else {
		return 1;
	}
}

/*
 * Add an 'a_' prefix so it comes before 'dos' in the linker list. We need to
 * check EFI first, since a DOS partition is often used as a 'protective MBR'
 * with EFI.
 */
U_BOOT_PART_TYPE(a_efi) = {
	.name		= "EFI",
	.part_type	= PART_TYPE_EFI,
	.max_entries	= GPT_ENTRY_NUMBERS,
	.get_info	= part_get_info_ptr(part_get_info_efi),
	.print		= part_print_ptr(part_print_efi),
	.test		= part_test_efi,
};
