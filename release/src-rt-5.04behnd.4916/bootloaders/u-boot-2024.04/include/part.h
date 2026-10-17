/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */
#ifndef _PART_H
#define _PART_H

#include <blk.h>
#include <ide.h>
#include <uuid.h>
#include <linker_lists.h>
#include <linux/errno.h>
#include <linux/list.h>

struct block_drvr {
	char *name;
	int (*select_hwpart)(int dev_num, int hwpart);
};

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
		 ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
		 ((x & 0xffff0000) ? 16 : 0))
#define LOG2_INVALID(type) ((type)((sizeof(type)<<3)-1))

/* Part types */
#define PART_TYPE_UNKNOWN	0x00
#define PART_TYPE_MAC		0x01
#define PART_TYPE_DOS		0x02
#define PART_TYPE_ISO		0x03
#define PART_TYPE_AMIGA		0x04
#define PART_TYPE_EFI		0x05

/* maximum number of partition entries supported by search */
#define DOS_ENTRY_NUMBERS	8
#define ISO_ENTRY_NUMBERS	64
#define MAC_ENTRY_NUMBERS	64
#define AMIGA_ENTRY_NUMBERS	8
/*
 * Type string for U-Boot bootable partitions
 */
#define BOOT_PART_TYPE	"U-Boot"	/* primary boot partition type	*/
#define BOOT_PART_COMP	"PPCBoot"	/* PPCBoot compatibility type	*/

/* device types */
#define DEV_TYPE_UNKNOWN	0xff	/* not connected */
#define DEV_TYPE_HARDDISK	0x00	/* harddisk */
#define DEV_TYPE_TAPE		0x01	/* Tape */
#define DEV_TYPE_CDROM		0x05	/* CD-ROM */
#define DEV_TYPE_OPDISK		0x07	/* optical disk */

#define PART_NAME_LEN 32
#define PART_TYPE_LEN 32
#define MAX_SEARCH_PARTITIONS 128

#define PART_BOOTABLE			((int)BIT(0))
#define PART_EFI_SYSTEM_PARTITION	((int)BIT(1))

struct disk_partition {
	lbaint_t	start;	/* # of first block in partition	*/
	lbaint_t	size;	/* number of blocks in partition	*/
	ulong	blksz;		/* block size in bytes			*/
	uchar	name[PART_NAME_LEN];	/* partition name			*/
	uchar	type[PART_TYPE_LEN];	/* string type description		*/
	/*
	 * The bootable is a bitmask with the following fields:
	 *
	 * PART_BOOTABLE		the MBR bootable flag is set
	 * PART_EFI_SYSTEM_PARTITION	the partition is an EFI system partition
	 */
	int	bootable;
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	char	uuid[UUID_STR_LEN + 1];	/* filesystem UUID as string, if exists	*/
#endif
#ifdef CONFIG_PARTITION_TYPE_GUID
	char	type_guid[UUID_STR_LEN + 1];	/* type GUID as string, if exists	*/
#endif
#ifdef CONFIG_DOS_PARTITION
	uchar	sys_ind;	/* partition type			*/
#endif
};

/* Accessors for struct disk_partition field ->uuid */
extern char *__invalid_use_of_disk_partition_uuid;

static inline const char *disk_partition_uuid(const struct disk_partition *info)
{
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	return info->uuid;
#else
	return __invalid_use_of_disk_partition_uuid;
#endif
}

static inline void disk_partition_set_uuid(struct disk_partition *info,
					   const char *val)
{
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	strlcpy(info->uuid, val, UUID_STR_LEN + 1);
#endif
}

static inline void disk_partition_clr_uuid(struct disk_partition *info)
{
#if CONFIG_IS_ENABLED(PARTITION_UUIDS)
	*info->uuid = '\0';
#endif
}

/* Accessors for struct disk_partition field ->type_guid */
extern char *__invalid_use_of_disk_partition_type_guid;

/**
 * disk_partition_type_guid() - get partition type GUID
 *
 * By using this function to get the partition type GUID we can use
 * 'if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))' instead of
 * '#ifdef CONFIG_PARTITION_TYPE_GUID'.
 *
 * @info:	partition information
 * Return:	partition type GUID
 */
static inline const
char *disk_partition_type_guid(const struct disk_partition *info)
{
#ifdef CONFIG_PARTITION_TYPE_GUID
	return info->type_guid;
#else
	return __invalid_use_of_disk_partition_type_guid;
#endif
}

/**
 * disk_partition_set_type_guid() - set partition type GUID
 *
 * By using this function to set the partition type GUID we can use
 * 'if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID))' instead of
 * '#ifdef CONFIG_PARTITION_TYPE_GUID'.
 *
 * @info:	partition information
 * @val:	partition type GUID as string
 */
static inline void disk_partition_set_type_guid(struct disk_partition *info,
						const char *val)
{
#ifdef CONFIG_PARTITION_TYPE_GUID
	strlcpy(info->type_guid, val, UUID_STR_LEN + 1);
#endif
}

static inline void disk_partition_clr_type_guid(struct disk_partition *info)
{
#ifdef CONFIG_PARTITION_TYPE_GUID
	*info->type_guid = '\0';
#endif
}

/* Accessors for struct disk_partition field ->sys_ind */
extern int __invalid_use_of_disk_partition_sys_ind;

static inline uint disk_partition_sys_ind(const struct disk_partition *info)
{
#ifdef CONFIG_DOS_PARTITION
	return info->sys_ind;
#else
	return __invalid_use_of_disk_partition_sys_ind;
#endif
}

struct disk_part {
	int partnum;
	struct disk_partition gpt_part_info;
	struct list_head list;
};

/* Misc _get_dev functions */
#if CONFIG_IS_ENABLED(PARTITIONS)
/**
 * blk_get_dev() - get a pointer to a block device given its type and number
 *
 * Each interface allocates its own devices and typically struct blk_desc is
 * contained with the interface's data structure. There is no global
 * numbering for block devices, so the interface name must be provided.
 *
 * @ifname:	Interface name (e.g. "ide", "scsi")
 * @dev:	Device number (0 for first device on that interface, 1 for
 *		second, etc.
 * Return:
 * pointer to the block device, or NULL if not available, or an error occurred.
 */
struct blk_desc *blk_get_dev(const char *ifname, int dev);

struct blk_desc *mg_disk_get_dev(int dev);

/**
 * part_get_info_by_type() - Get partitions from a block device using a specific
 * partition driver
 *
 * Each interface allocates its own devices and typically struct blk_desc is
 * contained with the interface's data structure. There is no global
 * numbering for block devices, so the interface name must be provided.
 *
 * @desc:	Block device descriptor
 * @part:	Partition number to read
 * @part_type:	Partition driver to use, or PART_TYPE_UNKNOWN to automatically
 *		choose a driver
 * @info:	Returned partition information
 *
 * Return: 0 on success, negative errno on failure
 */
int part_get_info_by_type(struct blk_desc *desc, int part, int part_type,
			  struct disk_partition *info);
int part_get_info(struct blk_desc *desc, int part,
		  struct disk_partition *info);
/**
 * part_get_info_whole_disk() - get partition info for the special case of
 * a partition occupying the entire disk.
 *
 * @desc:	block device descriptor
 * @info:	returned partition information
 * Return:	0 on success
 */
int part_get_info_whole_disk(struct blk_desc *desc,
			     struct disk_partition *info);

void part_print(struct blk_desc *desc);
void part_init(struct blk_desc *desc);
void dev_print(struct blk_desc *desc);

/**
 * blk_get_device_by_str() - Get a block device given its interface/hw partition
 *
 * Each interface allocates its own devices and typically struct blk_desc is
 * contained with the interface's data structure. There is no global
 * numbering for block devices, so the interface name must be provided.
 *
 * The hardware parition is not related to the normal software partitioning
 * of a device - each hardware partition is effectively a separately
 * accessible block device. When a hardware parition is selected on MMC the
 * other hardware partitions become inaccessible. The same block device is
 * used to access all hardware partitions, but its capacity may change when a
 * different hardware partition is selected.
 *
 * When a hardware partition number is given, the block device switches to
 * that hardware partition.
 *
 * @ifname:	Interface name (e.g. "ide", "scsi")
 * @dev_str:	Device and optional hw partition. This can either be a string
 *		containing the device number (e.g. "2") or the device number
 *		and hardware partition number (e.g. "2.4") for devices that
 *		support it (currently only MMC).
 * @desc:	Returns a pointer to the block device on success
 * Return: block device number (local to the interface), or -1 on error
 */
int blk_get_device_by_str(const char *ifname, const char *dev_str,
			  struct blk_desc **desc);

/**
 * blk_get_device_part_str() - Get a block device and partition
 *
 * This calls blk_get_device_by_str() to look up a device. It also looks up
 * a partition and returns information about it.
 *
 * @dev_part_str is in the format <dev>.<hw_part>:<part> where
 *
 * * <dev> is the device number,
 *
 * * <hw_part> is the optional hardware partition number and
 *
 * * <part> is the partition number.
 *
 * If @ifname is "hostfs", then this function returns the sandbox host block
 * device.
 *
 * If @ifname is "ubi", then this function returns 0, with @info set to a
 * special UBI device.
 *
 * If @dev_part_str is NULL or empty or "-", then this function looks up
 * the "bootdevice" environment variable and uses that string instead.
 *
 * If the partition string is empty then the first partition is used. If the
 * partition string is "auto" then the first bootable partition is used.
 *
 * @ifname:		Interface name (e.g. "ide", "scsi")
 * @dev_part_str:	Device and partition string
 * @desc:		Returns a pointer to the block device on success
 * @info:		Returns partition information
 * @allow_whole_dev:	true to allow the user to select partition 0
 *			(which means the whole device), false to require a valid
 *			partition number >= 1
 * Return: partition number, or -1 on error
 *
 */
int blk_get_device_part_str(const char *ifname, const char *dev_part_str,
			    struct blk_desc **desc,
			    struct disk_partition *info, int allow_whole_dev);

/**
 * part_get_info_by_name() - Search for a partition by name
 *                           among all available registered partitions
 *
 * @desc:	block device descriptor
 * @name:	the specified table entry name
 * @info:	returns the disk partition info
 *
 * Return: the partition number on match (starting on 1), -1 on no match,
 * otherwise error
 */
int part_get_info_by_name(struct blk_desc *desc, const char *name,
			  struct disk_partition *info);

/**
 * part_get_info_by_dev_and_name_or_num() - Get partition info from dev number
 *					    and part name, or dev number and
 *					    part number.
 *
 * Parse a device number and partition description (either name or number)
 * in the form of device number plus partition name separated by a "#"
 * (like "device_num#partition_name") or a device number plus a partition number
 * separated by a ":". For example both "0#misc" and "0:1" can be valid
 * partition descriptions for a given interface. If the partition is found, sets
 * desc and part_info accordingly with the information of the partition.
 *
 * @dev_iface:		Device interface
 * @dev_part_str:	Input partition description, like "0#misc" or "0:1"
 * @desc:		Place to store the device description pointer
 * @part_info:		Place to store the partition information
 * @allow_whole_dev:	true to allow the user to select partition 0
 *			(which means the whole device), false to require a valid
 *			partition number >= 1
 * Return:	the partition number on success, or negative errno on error
 */
int part_get_info_by_dev_and_name_or_num(const char *dev_iface,
					 const char *dev_part_str,
					 struct blk_desc **desc,
					 struct disk_partition *part_info,
					 int allow_whole_dev);

/**
 * part_set_generic_name() - create generic partition like hda1 or sdb2
 *
 * Helper function for partition tables, which don't hold partition names
 * (DOS, ISO). Generates partition name out of the device type and partition
 * number.
 *
 * @desc:	pointer to the block device
 * @part_num:	partition number for which the name is generated
 * @name:	buffer where the name is written
 */
void part_set_generic_name(const struct blk_desc *desc, int part_num,
			   char *name);

extern const struct block_drvr block_drvr[];
#else
static inline struct blk_desc *blk_get_dev(const char *ifname, int dev)
{ return NULL; }
static inline struct blk_desc *mg_disk_get_dev(int dev) { return NULL; }

static inline int part_get_info(struct blk_desc *desc, int part,
				struct disk_partition *info) { return -1; }
static inline int part_get_info_whole_disk(struct blk_desc *desc,
					   struct disk_partition *info)
{ return -1; }
static inline void part_print(struct blk_desc *desc) {}
static inline void part_init(struct blk_desc *desc) {}
static inline void dev_print(struct blk_desc *desc) {}
static inline int blk_get_device_by_str(const char *ifname, const char *dev_str,
					struct blk_desc **desc)
{ return -1; }
static inline int blk_get_device_part_str(const char *ifname,
					  const char *dev_part_str,
					  struct blk_desc **desc,
					  struct disk_partition *info,
					  int allow_whole_dev)
{ *desc = NULL; return -1; }

static inline int part_get_info_by_name(struct blk_desc *desc, const char *name,
					struct disk_partition *info)
{
	return -ENOENT;
}

static inline int
part_get_info_by_dev_and_name_or_num(const char *dev_iface,
				     const char *dev_part_str,
				     struct blk_desc **desc,
				     struct disk_partition *part_info,
				     int allow_whole_dev)
{
	*desc = NULL;
	return -ENOSYS;
}
#endif

struct udevice;
/**
 * disk_blk_read() - read blocks from a disk partition
 *
 * @dev:	Device to read from (UCLASS_PARTITION)
 * @start:	Start block number to read in the partition (0=first)
 * @blkcnt:	Number of blocks to read
 * @buffer:	Destination buffer for data read
 * Return:	number of blocks read, or -ve error number (see the
 * IS_ERR_VALUE() macro
 */
ulong disk_blk_read(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		    void *buffer);

/**
 * disk_blk_write() - write to a disk partition
 *
 * @dev:	Device to write to (UCLASS_PARTITION)
 * @start:	Start block number to write in the partition (0=first)
 * @blkcnt:	Number of blocks to write
 * @buffer:	Source buffer for data to write
 * Return:	number of blocks written, or -ve error number (see the
 * IS_ERR_VALUE() macro
 */
ulong disk_blk_write(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		     const void *buffer);

/**
 * disk_blk_erase() - erase a section of a disk partition
 *
 * @dev:	Device to (partially) erase (UCLASS_PARTITION)
 * @start:	Start block number to erase in the partition (0=first)
 * @blkcnt:	Number of blocks to erase
 * Return:	number of blocks erased, or -ve error number (see the
 * IS_ERR_VALUE() macro
 */
ulong disk_blk_erase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt);

/*
 * We don't support printing partition information in SPL and only support
 * getting partition information in a few cases.
 */
#ifdef CONFIG_SPL_BUILD
# define part_print_ptr(x)	NULL
# if defined(CONFIG_SPL_FS_EXT4) || defined(CONFIG_SPL_FS_FAT) || \
	defined(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION)
#  define part_get_info_ptr(x)	x
# else
#  define part_get_info_ptr(x)	NULL
# endif
#else
#define part_print_ptr(x)	x
#define part_get_info_ptr(x)	x
#endif

/**
 * struct part_driver - partition driver
 */
struct part_driver {
	/** @name:	partition name */
	const char *name;
	/** @part_type:	(MBR) partition type */
	int part_type;
	/** @max_entries:	maximum number of partition table entries */
	const int max_entries;
	/**
	 * @get_info:		Get information about a partition
	 *
	 * @get_info.desc:	Block device descriptor
	 * @get_info.part:	Partition number (1 = first)
	 * @get_info.info:	Returns partition information
	 */
	int (*get_info)(struct blk_desc *desc, int part,
			struct disk_partition *info);

	/**
	 * @print:		Print partition information
	 *
	 * @print.desc:	Block device descriptor
	 */
	void (*print)(struct blk_desc *desc);

	/**
	 * @test:		Test if a device contains this partition type
	 *
	 * @test.desc:		Block device descriptor
	 * @test.Return:
	 * 0 if the block device appears to contain this partition type,
	 * -ve if not
	 */
	int (*test)(struct blk_desc *desc);
};

/* Declare a new U-Boot partition 'driver' */
#define U_BOOT_PART_TYPE(__name)					\
	ll_entry_declare(struct part_driver, __name, part_driver)

#include <part_efi.h>

#if CONFIG_IS_ENABLED(EFI_PARTITION)
/* disk/part_efi.c */
/**
 * write_gpt_table() - Write the GUID Partition Table to disk
 *
 * @desc:	block device descriptor
 * @gpt_h:	pointer to GPT header representation
 * @gpt_e:	pointer to GPT partition table entries
 *
 * Return:	zero on success, otherwise error
 */
int write_gpt_table(struct blk_desc *desc, gpt_header *gpt_h, gpt_entry *gpt_e);

/**
 * gpt_fill_pte() - Fill the GPT partition table entry
 *
 * @desc:	block device descriptor
 * @gpt_h:	GPT header representation
 * @gpt_e:	GPT partition table entries
 * @partitions:	list of partitions
 * @parts:	number of partitions
 *
 * Return:	zero on success
 */
int gpt_fill_pte(struct blk_desc *desc, gpt_header *gpt_h, gpt_entry *gpt_e,
		 struct disk_partition *partitions, int parts);

/**
 * gpt_fill_header() - Fill the GPT header
 *
 * @desc:		block device descriptor
 * @gpt_h:		GPT header representation
 * @str_guid:		disk guid string representation
 * @parts_count:	number of partitions
 *
 * Return:		error on str_guid conversion error
 */
int gpt_fill_header(struct blk_desc *desc, gpt_header *gpt_h, char *str_guid,
		    int parts_count);

/**
 * gpt_restore() - Restore GPT partition table
 *
 * @desc:		block device descriptor
 * @str_disk_guid:	disk GUID
 * @partitions:		list of partitions
 * @parts_count:	number of partitions
 *
 * Return:		0 on success
 */
int gpt_restore(struct blk_desc *desc, char *str_disk_guid,
		struct disk_partition *partitions, const int parts_count);

/**
 * is_valid_gpt_buf() - Ensure that the Primary GPT information is valid
 *
 * @desc:	block device descriptor
 * @buf:	buffer which contains the MBR and Primary GPT info
 *
 * Return:	0 on success, otherwise error
 */
int is_valid_gpt_buf(struct blk_desc *desc, void *buf);

/**
 * write_mbr_and_gpt_partitions() - write MBR, Primary GPT and Backup GPT
 *
 * @desc:	block device descriptor
 * @buf:	buffer which contains the MBR and Primary GPT info
 *
 * Return:	0 on success, otherwise error
 */
int write_mbr_and_gpt_partitions(struct blk_desc *desc, void *buf);

/**
 * gpt_verify_headers() - Read and check CRC32 of the GPT's header
 *                        and partition table entries (PTE)
 *
 * As a side effect if sets gpt_head and gpt_pte so they point to GPT data.
 *
 * @desc:	block device descriptor
 * @gpt_head:	pointer to GPT header data read from medium
 * @gpt_pte:	pointer to GPT partition table enties read from medium
 *
 * Return:	0 on success, otherwise error
 */
int gpt_verify_headers(struct blk_desc *desc, gpt_header *gpt_head,
		       gpt_entry **gpt_pte);

/**
 * gpt_repair_headers() - Function to repair the GPT's header
 *                        and partition table entries (PTE)
 *
 * @desc:	block device descriptor
 *
 * Return:	0 on success, otherwise error
 */
int gpt_repair_headers(struct blk_desc *desc);

/**
 * gpt_verify_partitions() - Function to check if partitions' name, start and
 *                           size correspond to '$partitions' env variable
 *
 * This function checks if on medium stored GPT data is in sync with information
 * provided in '$partitions' environment variable. Specificially, name, start
 * and size of the partition is checked.
 *
 * @desc:	block device descriptor
 * @partitions:	partition data read from '$partitions' env variable
 * @parts:	number of partitions read from '$partitions' env variable
 * @gpt_head:	pointer to GPT header data read from medium
 * @gpt_pte:	pointer to GPT partition table enties read from medium
 *
 * Return:	0 on success, otherwise error
 */
int gpt_verify_partitions(struct blk_desc *desc,
			  struct disk_partition *partitions, int parts,
			  gpt_header *gpt_head, gpt_entry **gpt_pte);


/**
 * get_disk_guid() - Read the GUID string from a device's GPT
 *
 * This function reads the GUID string from a block device whose descriptor
 * is provided.
 *
 * @desc:	block device descriptor
 * @guid:	pre-allocated string in which to return the GUID
 *
 * Return:	0 on success, otherwise error
 */
int get_disk_guid(struct blk_desc *desc, char *guid);

#endif

#if CONFIG_IS_ENABLED(DOS_PARTITION)
/**
 * is_valid_dos_buf() - Ensure that a DOS MBR image is valid
 *
 * @buf:	buffer which contains the MBR
 *
 * Return:	0 on success, otherwise error
 */
int is_valid_dos_buf(void *buf);

/**
 * write_mbr_sector() - write DOS MBR
 *
 * @desc:	block device descriptor
 * @buf:	buffer which contains the MBR
 *
 * Return:	0 on success, otherwise error
 */
int write_mbr_sector(struct blk_desc *desc, void *buf);

int write_mbr_partitions(struct blk_desc *dev,
		struct disk_partition *p, int count, unsigned int disksig);
int layout_mbr_partitions(struct disk_partition *p, int count,
			  lbaint_t total_sectors);

#endif

#if CONFIG_IS_ENABLED(PARTITIONS)
/**
 * part_driver_get_count() - get partition driver count
 *
 * Return:	number of partition drivers
 */
static inline int part_driver_get_count(void)
{
	return ll_entry_count(struct part_driver, part_driver);
}

/**
 * part_driver_get_first() - get first partition driver
 *
 * Return:	pointer to first partition driver on success, otherwise NULL
 */
static inline struct part_driver *part_driver_get_first(void)
{
	return ll_entry_start(struct part_driver, part_driver);
}

/**
 * part_get_type_by_name() - Get partition type by name
 *
 * @name:	Name of partition type to look up (not case-sensitive)
 * Return:
 * Corresponding partition type (PART\_TYPE\_...) or PART\_TYPE\_UNKNOWN
 */
int part_get_type_by_name(const char *name);

/**
 * part_get_bootable() - Find the first bootable partition
 *
 * @desc:	Block-device descriptor
 * Return:	first bootable partition, or 0 if there is none
 */
int part_get_bootable(struct blk_desc *desc);

#else
static inline int part_driver_get_count(void)
{ return 0; }

static inline struct part_driver *part_driver_get_first(void)
{ return NULL; }

static inline bool part_get_bootable(struct blk_desc *desc)
{ return false; }

#endif /* CONFIG_PARTITIONS */

#endif /* _PART_H */
