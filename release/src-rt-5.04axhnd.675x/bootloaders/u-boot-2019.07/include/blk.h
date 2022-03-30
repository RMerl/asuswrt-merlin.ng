/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef BLK_H
#define BLK_H

#include <efi.h>

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#define LBAFlength "ll"
#else
typedef ulong lbaint_t;
#define LBAFlength "l"
#endif
#define LBAF "%" LBAFlength "x"
#define LBAFU "%" LBAFlength "u"

/* Interface types: */
enum if_type {
	IF_TYPE_UNKNOWN = 0,
	IF_TYPE_IDE,
	IF_TYPE_SCSI,
	IF_TYPE_ATAPI,
	IF_TYPE_USB,
	IF_TYPE_DOC,
	IF_TYPE_MMC,
	IF_TYPE_SD,
	IF_TYPE_SATA,
	IF_TYPE_HOST,
	IF_TYPE_NVME,
	IF_TYPE_EFI,
	IF_TYPE_VIRTIO,

	IF_TYPE_COUNT,			/* Number of interface types */
};

#define BLK_VEN_SIZE		40
#define BLK_PRD_SIZE		20
#define BLK_REV_SIZE		8

/*
 * Identifies the partition table type (ie. MBR vs GPT GUID) signature
 */
enum sig_type {
	SIG_TYPE_NONE,
	SIG_TYPE_MBR,
	SIG_TYPE_GUID,

	SIG_TYPE_COUNT			/* Number of signature types */
};

/*
 * With driver model (CONFIG_BLK) this is uclass platform data, accessible
 * with dev_get_uclass_platdata(dev)
 */
struct blk_desc {
	/*
	 * TODO: With driver model we should be able to use the parent
	 * device's uclass instead.
	 */
	enum if_type	if_type;	/* type of the interface */
	int		devnum;		/* device number */
	unsigned char	part_type;	/* partition type */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	hwpart;		/* HW partition, e.g. for eMMC */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
#ifdef CONFIG_LBA48
	/* device can use 48bit addr (ATA/ATAPI v7) */
	unsigned char	lba48;
#endif
	lbaint_t	lba;		/* number of blocks */
	unsigned long	blksz;		/* block size */
	int		log2blksz;	/* for convenience: log2(blksz) */
	char		vendor[BLK_VEN_SIZE + 1]; /* device vendor string */
	char		product[BLK_PRD_SIZE + 1]; /* device product number */
	char		revision[BLK_REV_SIZE + 1]; /* firmware revision */
	enum sig_type	sig_type;	/* Partition table signature type */
	union {
		uint32_t mbr_sig;	/* MBR integer signature */
		efi_guid_t guid_sig;	/* GPT GUID Signature */
	};
#if CONFIG_IS_ENABLED(BLK)
	/*
	 * For now we have a few functions which take struct blk_desc as a
	 * parameter. This field allows them to look up the associated
	 * device. Once these functions are removed we can drop this field.
	 */
	struct udevice *bdev;
#else
	unsigned long	(*block_read)(struct blk_desc *block_dev,
				      lbaint_t start,
				      lbaint_t blkcnt,
				      void *buffer);
	unsigned long	(*block_write)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt,
				       const void *buffer);
	unsigned long	(*block_erase)(struct blk_desc *block_dev,
				       lbaint_t start,
				       lbaint_t blkcnt);
	void		*priv;		/* driver private struct pointer */
#endif
};

#define BLOCK_CNT(size, blk_desc) (PAD_COUNT(size, blk_desc->blksz))
#define PAD_TO_BLOCKSIZE(size, blk_desc) \
	(PAD_SIZE(size, blk_desc->blksz))

#if CONFIG_IS_ENABLED(BLOCK_CACHE)
/**
 * blkcache_read() - attempt to read a set of blocks from cache
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks to read
 * @param blksz - size in bytes of each block
 * @param buf - buffer to contain cached data
 *
 * @return - '1' if block returned from cache, '0' otherwise.
 */
int blkcache_read(int iftype, int dev,
		  lbaint_t start, lbaint_t blkcnt,
		  unsigned long blksz, void *buffer);

/**
 * blkcache_fill() - make data read from a block device available
 * to the block cache
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks available
 * @param blksz - size in bytes of each block
 * @param buf - buffer containing data to cache
 *
 */
void blkcache_fill(int iftype, int dev,
		   lbaint_t start, lbaint_t blkcnt,
		   unsigned long blksz, void const *buffer);

/**
 * blkcache_invalidate() - discard the cache for a set of blocks
 * because of a write or device (re)initialization.
 *
 * @param iftype - IF_TYPE_x for type of device
 * @param dev - device index of particular type
 */
void blkcache_invalidate(int iftype, int dev);

/**
 * blkcache_configure() - configure block cache
 *
 * @param blocks - maximum blocks per entry
 * @param entries - maximum entries in cache
 */
void blkcache_configure(unsigned blocks, unsigned entries);

/*
 * statistics of the block cache
 */
struct block_cache_stats {
	unsigned hits;
	unsigned misses;
	unsigned entries; /* current entry count */
	unsigned max_blocks_per_entry;
	unsigned max_entries;
};

/**
 * get_blkcache_stats() - return statistics and reset
 *
 * @param stats - statistics are copied here
 */
void blkcache_stats(struct block_cache_stats *stats);

#else

static inline int blkcache_read(int iftype, int dev,
				lbaint_t start, lbaint_t blkcnt,
				unsigned long blksz, void *buffer)
{
	return 0;
}

static inline void blkcache_fill(int iftype, int dev,
				 lbaint_t start, lbaint_t blkcnt,
				 unsigned long blksz, void const *buffer) {}

static inline void blkcache_invalidate(int iftype, int dev) {}

#endif

#if CONFIG_IS_ENABLED(BLK)
struct udevice;

/* Operations on block devices */
struct blk_ops {
	/**
	 * read() - read from a block device
	 *
	 * @dev:	Device to read from
	 * @start:	Start block number to read (0=first)
	 * @blkcnt:	Number of blocks to read
	 * @buffer:	Destination buffer for data read
	 * @return number of blocks read, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*read)(struct udevice *dev, lbaint_t start,
			      lbaint_t blkcnt, void *buffer);

	/**
	 * write() - write to a block device
	 *
	 * @dev:	Device to write to
	 * @start:	Start block number to write (0=first)
	 * @blkcnt:	Number of blocks to write
	 * @buffer:	Source buffer for data to write
	 * @return number of blocks written, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*write)(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt, const void *buffer);

	/**
	 * erase() - erase a section of a block device
	 *
	 * @dev:	Device to (partially) erase
	 * @start:	Start block number to erase (0=first)
	 * @blkcnt:	Number of blocks to erase
	 * @return number of blocks erased, or -ve error number (see the
	 * IS_ERR_VALUE() macro
	 */
	unsigned long (*erase)(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt);

	/**
	 * select_hwpart() - select a particular hardware partition
	 *
	 * Some devices (e.g. MMC) can support partitioning at the hardware
	 * level. This is quite separate from the normal idea of
	 * software-based partitions. MMC hardware partitions must be
	 * explicitly selected. Once selected only the region of the device
	 * covered by that partition is accessible.
	 *
	 * The MMC standard provides for two boot partitions (numbered 1 and 2),
	 * rpmb (3), and up to 4 addition general-purpose partitions (4-7).
	 *
	 * @desc:	Block device to update
	 * @hwpart:	Hardware partition number to select. 0 means the raw
	 *		device, 1 is the first partition, 2 is the second, etc.
	 * @return 0 if OK, -ve on error
	 */
	int (*select_hwpart)(struct udevice *dev, int hwpart);
};

#define blk_get_ops(dev)	((struct blk_ops *)(dev)->driver->ops)

/*
 * These functions should take struct udevice instead of struct blk_desc,
 * but this is convenient for migration to driver model. Add a 'd' prefix
 * to the function operations, so that blk_read(), etc. can be reserved for
 * functions with the correct arguments.
 */
unsigned long blk_dread(struct blk_desc *block_dev, lbaint_t start,
			lbaint_t blkcnt, void *buffer);
unsigned long blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt, const void *buffer);
unsigned long blk_derase(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt);

/**
 * blk_find_device() - Find a block device
 *
 * This function does not activate the device. The device will be returned
 * whether or not it is activated.
 *
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * @return 0 if found, -ENODEV if no device found, or other -ve error value
 */
int blk_find_device(int if_type, int devnum, struct udevice **devp);

/**
 * blk_get_device() - Find and probe a block device ready for use
 *
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * @return 0 if found, -ENODEV if no device found, or other -ve error value
 */
int blk_get_device(int if_type, int devnum, struct udevice **devp);

/**
 * blk_first_device() - Find the first device for a given interface
 *
 * The device is probed ready for use
 *
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * @return 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_first_device(int if_type, struct udevice **devp);

/**
 * blk_next_device() - Find the next device for a given interface
 *
 * This can be called repeatedly after blk_first_device() to iterate through
 * all devices of the given interface type.
 *
 * The device is probed ready for use
 *
 * @devp:	On entry, the previous device returned. On exit, the next
 *		device, if found
 * @return 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_next_device(struct udevice **devp);

/**
 * blk_create_device() - Create a new block device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the block device
 * @name:	Name for the device
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number, specific to the interface type, or -1 to
 *		allocate the next available number
 * @blksz:	Block size of the device in bytes (typically 512)
 * @lba:	Total number of blocks of the device
 * @devp:	the new device (which has not been probed)
 */
int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int if_type, int devnum, int blksz,
		      lbaint_t lba, struct udevice **devp);

/**
 * blk_create_devicef() - Create a new named block device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the block device
 * @name:	Name for the device (parent name is prepended)
 * @if_type:	Interface type (enum if_type_t)
 * @devnum:	Device number, specific to the interface type, or -1 to
 *		allocate the next available number
 * @blksz:	Block size of the device in bytes (typically 512)
 * @lba:	Total number of blocks of the device
 * @devp:	the new device (which has not been probed)
 */
int blk_create_devicef(struct udevice *parent, const char *drv_name,
		       const char *name, int if_type, int devnum, int blksz,
		       lbaint_t lba, struct udevice **devp);

/**
 * blk_unbind_all() - Unbind all device of the given interface type
 *
 * The devices are removed and then unbound.
 *
 * @if_type:	Interface type to unbind
 * @return 0 if OK, -ve on error
 */
int blk_unbind_all(int if_type);

/**
 * blk_find_max_devnum() - find the maximum device number for an interface type
 *
 * Finds the last allocated device number for an interface type @if_type. The
 * next number is safe to use for a newly allocated device.
 *
 * @if_type:	Interface type to scan
 * @return maximum device number found, or -ENODEV if none, or other -ve on
 * error
 */
int blk_find_max_devnum(enum if_type if_type);

/**
 * blk_next_free_devnum() - get the next device number for an interface type
 *
 * Finds the next number that is safe to use for a newly allocated device for
 * an interface type @if_type.
 *
 * @if_type:	Interface type to scan
 * @return next device number safe to use, or -ve on error
 */
int blk_next_free_devnum(enum if_type if_type);

/**
 * blk_select_hwpart() - select a hardware partition
 *
 * Select a hardware partition if the device supports it (typically MMC does)
 *
 * @dev:	Device to update
 * @hwpart:	Partition number to select
 * @return 0 if OK, -ve on error
 */
int blk_select_hwpart(struct udevice *dev, int hwpart);

/**
 * blk_get_from_parent() - obtain a block device by looking up its parent
 *
 * All devices with
 */
int blk_get_from_parent(struct udevice *parent, struct udevice **devp);

/**
 * blk_get_by_device() - Get the block device descriptor for the given device
 * @dev:	Instance of a storage device
 *
 * Return: With block device descriptor on success , NULL if there is no such
 *	   block device.
 */
struct blk_desc *blk_get_by_device(struct udevice *dev);

#else
#include <errno.h>
/*
 * These functions should take struct udevice instead of struct blk_desc,
 * but this is convenient for migration to driver model. Add a 'd' prefix
 * to the function operations, so that blk_read(), etc. can be reserved for
 * functions with the correct arguments.
 */
static inline ulong blk_dread(struct blk_desc *block_dev, lbaint_t start,
			      lbaint_t blkcnt, void *buffer)
{
	ulong blks_read;
	if (blkcache_read(block_dev->if_type, block_dev->devnum,
			  start, blkcnt, block_dev->blksz, buffer))
		return blkcnt;

	/*
	 * We could check if block_read is NULL and return -ENOSYS. But this
	 * bloats the code slightly (cause some board to fail to build), and
	 * it would be an error to try an operation that does not exist.
	 */
	blks_read = block_dev->block_read(block_dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(block_dev->if_type, block_dev->devnum,
			      start, blkcnt, block_dev->blksz, buffer);

	return blks_read;
}

static inline ulong blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt, const void *buffer)
{
	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return block_dev->block_write(block_dev, start, blkcnt, buffer);
}

static inline ulong blk_derase(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt)
{
	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return block_dev->block_erase(block_dev, start, blkcnt);
}

/**
 * struct blk_driver - Driver for block interface types
 *
 * This provides access to the block devices for each interface type. One
 * driver should be provided using U_BOOT_LEGACY_BLK() for each interface
 * type that is to be supported.
 *
 * @if_typename:	Interface type name
 * @if_type:		Interface type
 * @max_devs:		Maximum number of devices supported
 * @desc:		Pointer to list of devices for this interface type,
 *			or NULL to use @get_dev() instead
 */
struct blk_driver {
	const char *if_typename;
	enum if_type if_type;
	int max_devs;
	struct blk_desc *desc;
	/**
	 * get_dev() - get a pointer to a block device given its number
	 *
	 * Each interface allocates its own devices and typically
	 * struct blk_desc is contained with the interface's data structure.
	 * There is no global numbering for block devices. This method allows
	 * the device for an interface type to be obtained when @desc is NULL.
	 *
	 * @devnum:	Device number (0 for first device on that interface,
	 *		1 for second, etc.
	 * @descp:	Returns pointer to the block device on success
	 * @return 0 if OK, -ve on error
	 */
	int (*get_dev)(int devnum, struct blk_desc **descp);

	/**
	 * select_hwpart() - Select a hardware partition
	 *
	 * Some devices (e.g. MMC) can support partitioning at the hardware
	 * level. This is quite separate from the normal idea of
	 * software-based partitions. MMC hardware partitions must be
	 * explicitly selected. Once selected only the region of the device
	 * covered by that partition is accessible.
	 *
	 * The MMC standard provides for two boot partitions (numbered 1 and 2),
	 * rpmb (3), and up to 4 addition general-purpose partitions (4-7).
	 * Partition 0 is the main user-data partition.
	 *
	 * @desc:	Block device descriptor
	 * @hwpart:	Hardware partition number to select. 0 means the main
	 *		user-data partition, 1 is the first partition, 2 is
	 *		the second, etc.
	 * @return 0 if OK, other value for an error
	 */
	int (*select_hwpart)(struct blk_desc *desc, int hwpart);
};

/*
 * Declare a new U-Boot legacy block driver. New drivers should use driver
 * model (UCLASS_BLK).
 */
#define U_BOOT_LEGACY_BLK(__name)					\
	ll_entry_declare(struct blk_driver, __name, blk_driver)

struct blk_driver *blk_driver_lookup_type(int if_type);

#endif /* !CONFIG_BLK */

/**
 * blk_get_devnum_by_typename() - Get a block device by type and number
 *
 * This looks through the available block devices of the given type, returning
 * the one with the given @devnum.
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @return point to block device descriptor, or NULL if not found
 */
struct blk_desc *blk_get_devnum_by_type(enum if_type if_type, int devnum);

/**
 * blk_get_devnum_by_type() - Get a block device by type name, and number
 *
 * This looks up the block device type based on @if_typename, then calls
 * blk_get_devnum_by_type().
 *
 * @if_typename:	Block device type name
 * @devnum:		Device number
 * @return point to block device descriptor, or NULL if not found
 */
struct blk_desc *blk_get_devnum_by_typename(const char *if_typename,
					    int devnum);

/**
 * blk_dselect_hwpart() - select a hardware partition
 *
 * This selects a hardware partition (such as is supported by MMC). The block
 * device size may change as this effectively points the block device to a
 * partition at the hardware level. See the select_hwpart() method above.
 *
 * @desc:	Block device descriptor for the device to select
 * @hwpart:	Partition number to select
 * @return 0 if OK, -ve on error
 */
int blk_dselect_hwpart(struct blk_desc *desc, int hwpart);

/**
 * blk_list_part() - list the partitions for block devices of a given type
 *
 * This looks up the partition type for each block device of type @if_type,
 * then displays a list of partitions.
 *
 * @if_type:	Block device type
 * @return 0 if OK, -ENODEV if there is none of that type
 */
int blk_list_part(enum if_type if_type);

/**
 * blk_list_devices() - list the block devices of a given type
 *
 * This lists each block device of the type @if_type, showing the capacity
 * as well as type-specific information.
 *
 * @if_type:	Block device type
 */
void blk_list_devices(enum if_type if_type);

/**
 * blk_show_device() - show information about a given block device
 *
 * This shows the block device capacity as well as type-specific information.
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @return 0 if OK, -ENODEV for invalid device number
 */
int blk_show_device(enum if_type if_type, int devnum);

/**
 * blk_print_device_num() - show information about a given block device
 *
 * This is similar to blk_show_device() but returns an error if the block
 * device type is unknown.
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @return 0 if OK, -ENODEV for invalid device number, -ENOENT if the block
 * device is not connected
 */
int blk_print_device_num(enum if_type if_type, int devnum);

/**
 * blk_print_part_devnum() - print the partition information for a device
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @return 0 if OK, -ENOENT if the block device is not connected, -ENOSYS if
 * the interface type is not supported, other -ve on other error
 */
int blk_print_part_devnum(enum if_type if_type, int devnum);

/**
 * blk_read_devnum() - read blocks from a device
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @blkcnt:	Number of blocks to read
 * @buffer:	Address to write data to
 * @return number of blocks read, or -ve error number on error
 */
ulong blk_read_devnum(enum if_type if_type, int devnum, lbaint_t start,
		      lbaint_t blkcnt, void *buffer);

/**
 * blk_write_devnum() - write blocks to a device
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @blkcnt:	Number of blocks to write
 * @buffer:	Address to read data from
 * @return number of blocks written, or -ve error number on error
 */
ulong blk_write_devnum(enum if_type if_type, int devnum, lbaint_t start,
		       lbaint_t blkcnt, const void *buffer);

/**
 * blk_select_hwpart_devnum() - select a hardware partition
 *
 * This is similar to blk_dselect_hwpart() but it looks up the interface and
 * device number.
 *
 * @if_type:	Block device type
 * @devnum:	Device number
 * @hwpart:	Partition number to select
 * @return 0 if OK, -ve on error
 */
int blk_select_hwpart_devnum(enum if_type if_type, int devnum, int hwpart);

/**
 * blk_get_if_type_name() - Get the name of an interface type
 *
 * @if_type: Interface type to check
 * @return name of interface, or NULL if none
 */
const char *blk_get_if_type_name(enum if_type if_type);

/**
 * blk_common_cmd() - handle common commands with block devices
 *
 * @args: Number of arguments to the command (argv[0] is the command itself)
 * @argv: Command arguments
 * @if_type: Interface type
 * @cur_devnump: Current device number for this interface type
 * @return 0 if OK, CMD_RET_ERROR on error
 */
int blk_common_cmd(int argc, char * const argv[], enum if_type if_type,
		   int *cur_devnump);

#endif
