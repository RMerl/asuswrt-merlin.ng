/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef BLK_H
#define BLK_H

#include <bouncebuf.h>
#include <dm/uclass-id.h>
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

#define DEFAULT_BLKSZ		512

struct udevice;

static inline bool blk_enabled(void)
{
	return CONFIG_IS_ENABLED(BLK) || IS_ENABLED(CONFIG_SPL_LEGACY_BLOCK);
}

#define BLK_VEN_SIZE		40
#define BLK_PRD_SIZE		20
#define BLK_REV_SIZE		8

#define PART_FORMAT_PCAT	0x1
#define PART_FORMAT_GPT		0x2

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
 * with dev_get_uclass_plat(dev)
 */
struct blk_desc {
	/*
	 * TODO: With driver model we should be able to use the parent
	 * device's uclass instead.
	 */
	enum uclass_id	uclass_id;	/* type of the interface */
	int		devnum;		/* device number */
	unsigned char	part_type;	/* partition type */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	hwpart;		/* HW partition, e.g. for eMMC */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
	/* device can use 48bit addr (ATA/ATAPI v7) */
	bool	lba48;
	unsigned char	atapi;		/* Use ATAPI protocol */
	unsigned char	bb;		/* Use bounce buffer */
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
 * @param iftype - uclass_id_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks to read
 * @param blksz - size in bytes of each block
 * @param buffer - buffer to contain cached data
 *
 * Return: - 1 if block returned from cache, 0 otherwise.
 */
int blkcache_read(int iftype, int dev,
		  lbaint_t start, lbaint_t blkcnt,
		  unsigned long blksz, void *buffer);

/**
 * blkcache_fill() - make data read from a block device available
 * to the block cache
 *
 * @param iftype - uclass_id_x for type of device
 * @param dev - device index of particular type
 * @param start - starting block number
 * @param blkcnt - number of blocks available
 * @param blksz - size in bytes of each block
 * @param buffer - buffer containing data to cache
 *
 */
void blkcache_fill(int iftype, int dev,
		   lbaint_t start, lbaint_t blkcnt,
		   unsigned long blksz, void const *buffer);

/**
 * blkcache_invalidate() - discard the cache for a set of blocks
 * because of a write or device (re)initialization.
 *
 * @iftype - UCLASS_ID_ for type of device, or -1 for any
 * @dev - device index of particular type, if @iftype is not -1
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

/** blkcache_free() - free all memory allocated to the block cache */
void blkcache_free(void);

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

static inline void blkcache_free(void) {}

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
	 * @dev:	Block device to update
	 * @hwpart:	Hardware partition number to select. 0 means the raw
	 *		device, 1 is the first partition, 2 is the second, etc.
	 * @return 0 if OK, -ve on error
	 */
	int (*select_hwpart)(struct udevice *dev, int hwpart);

#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
	/**
	 * buffer_aligned() - test memory alignment of block operation buffer
	 *
	 * Some devices have limited DMA capabilities and require that the
	 * buffers passed to them fit specific properties. This optional
	 * callback can be used to indicate whether a buffer alignment is
	 * suitable for the device DMA or not, and trigger use of generic
	 * bounce buffer implementation to help use of unsuitable buffers
	 * at the expense of performance degradation.
	 *
	 * @dev:	Block device associated with the request
	 * @state:	Bounce buffer state
	 * @return 1 if OK, 0 if unaligned
	 */
	int (*buffer_aligned)(struct udevice *dev, struct bounce_buffer *state);
#endif	/* CONFIG_BOUNCE_BUFFER */
};

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
 * blk_read() - Read from a block device
 *
 * @dev: Device to read from
 * @start: Start block for the read
 * @blkcnt: Number of blocks to read
 * @buf: Place to put the data
 * @return number of blocks read (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
long blk_read(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
	      void *buffer);

/**
 * blk_write() - Write to a block device
 *
 * @dev: Device to write to
 * @start: Start block for the write
 * @blkcnt: Number of blocks to write
 * @buf: Data to write
 * @return number of blocks written (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
long blk_write(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
	       const void *buffer);

/**
 * blk_erase() - Erase part of a block device
 *
 * @dev: Device to erase
 * @start: Start block for the erase
 * @blkcnt: Number of blocks to erase
 * @return number of blocks erased (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
long blk_erase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt);

/**
 * blk_find_device() - Find a block device
 *
 * This function does not activate the device. The device will be returned
 * whether or not it is activated.
 *
 * @uclass_id:	Interface type (enum uclass_id_t)
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * Return: 0 if found, -ENODEV if no device found, or other -ve error value
 */
int blk_find_device(int uclass_id, int devnum, struct udevice **devp);

/**
 * blk_get_device() - Find and probe a block device ready for use
 *
 * @uclass_id:	Interface type (enum uclass_id_t)
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * Return: 0 if found, -ENODEV if no device found, or other -ve error value
 */
int blk_get_device(int uclass_id, int devnum, struct udevice **devp);

/**
 * blk_first_device() - Find the first device for a given interface
 *
 * The device is probed ready for use
 *
 * @devnum:	Device number (specific to each interface type)
 * @devp:	the device, if found
 * Return: 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_first_device(int uclass_id, struct udevice **devp);

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
 * Return: 0 if found, -ENODEV if no device, or other -ve error value
 */
int blk_next_device(struct udevice **devp);

/**
 * blk_create_device() - Create a new block device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the block device
 * @name:	Name for the device
 * @uclass_id:	Interface type (enum uclass_id_t)
 * @devnum:	Device number, specific to the interface type, or -1 to
 *		allocate the next available number
 * @blksz:	Block size of the device in bytes (typically 512)
 * @lba:	Total number of blocks of the device
 * @devp:	the new device (which has not been probed)
 */
int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int uclass_id, int devnum, int blksz,
		      lbaint_t lba, struct udevice **devp);

/**
 * blk_create_devicef() - Create a new named block device
 *
 * @parent:	Parent of the new device
 * @drv_name:	Driver name to use for the block device
 * @name:	Name for the device (parent name is prepended)
 * @uclass_id:	Interface type (enum uclass_id_t)
 * @devnum:	Device number, specific to the interface type, or -1 to
 *		allocate the next available number
 * @blksz:	Block size of the device in bytes (typically 512)
 * @lba:	Total number of blocks of the device
 * @devp:	the new device (which has not been probed)
 */
int blk_create_devicef(struct udevice *parent, const char *drv_name,
		       const char *name, int uclass_id, int devnum, int blksz,
		       lbaint_t lba, struct udevice **devp);

/**
 * blk_probe_or_unbind() - Try to probe
 *
 * Try to probe the device, primarily for enumerating partitions.
 * If it fails, the device itself is unbound since it means that it won't
 * work any more.
 *
 * @dev:	The device to probe
 * Return:	0 if OK, -ve on error
 */
int blk_probe_or_unbind(struct udevice *dev);

/**
 * blk_unbind_all() - Unbind all device of the given interface type
 *
 * The devices are removed and then unbound.
 *
 * @uclass_id:	Interface type to unbind
 * Return: 0 if OK, -ve on error
 */
int blk_unbind_all(int uclass_id);

/**
 * blk_find_max_devnum() - find the maximum device number for an interface type
 *
 * Finds the last allocated device number for an interface type @uclass_id. The
 * next number is safe to use for a newly allocated device.
 *
 * @uclass_id:	Interface type to scan
 * Return: maximum device number found, or -ENODEV if none, or other -ve on
 * error
 */
int blk_find_max_devnum(enum uclass_id uclass_id);

/**
 * blk_next_free_devnum() - get the next device number for an interface type
 *
 * Finds the next number that is safe to use for a newly allocated device for
 * an interface type @uclass_id.
 *
 * @uclass_id:	Interface type to scan
 * Return: next device number safe to use, or -ve on error
 */
int blk_next_free_devnum(enum uclass_id uclass_id);

/**
 * blk_select_hwpart() - select a hardware partition
 *
 * Select a hardware partition if the device supports it (typically MMC does)
 *
 * @dev:	Device to update
 * @hwpart:	Partition number to select
 * Return: 0 if OK, -ve on error
 */
int blk_select_hwpart(struct udevice *dev, int hwpart);

/**
 * blk_find_from_parent() - find a block device by looking up its parent
 *
 * All block devices have a parent 'media' device which provides the block
 * driver for the block device, ensuring that access to the underlying medium
 * is available.
 *
 * The block device is not activated by this function. See
 * blk_get_from_parent() for that.
 *
 * @parent: Media device
 * @devp: Returns the associated block device, if any
 * Returns: 0 if OK, -ENODEV if @parent is not a media device and has no
 * UCLASS_BLK child
 */
int blk_find_from_parent(struct udevice *parent, struct udevice **devp);

/**
 * blk_get_from_parent() - obtain a block device by looking up its parent
 *
 * All block devices have a parent 'media' device which provides the block
 * driver for the block device, ensuring that access to the underlying medium
 * is available.
 *
 * The block device is probed and ready for use.
 *
 * @parent: Media device
 * @devp: Returns the associated block device, if any
 * Returns: 0 if OK, -ENODEV if @parent is not a media device and has no
 * UCLASS_BLK child
 */
int blk_get_from_parent(struct udevice *parent, struct udevice **devp);

/**
 * blk_get_devtype() - Get the device type of a block device
 *
 * @dev:	Block device to check
 * Return: device tree, i.e. the uclass name of its parent, e.g. "mmc"
 */
const char *blk_get_devtype(struct udevice *dev);

/**
 * blk_get_by_device() - Get the block device descriptor for the given device
 * @dev:	Instance of a storage device (the parent of the block device)
 *
 * Return: With block device descriptor on success , NULL if there is no such
 *	   block device.
 */
struct blk_desc *blk_get_by_device(struct udevice *dev);

/**
 * blk_get_desc() - Get the block device descriptor for the given device number
 *
 * @uclass_id:	Interface type
 * @devnum:	Device number (0 = first)
 * @descp:	Returns block device descriptor on success
 * Return: 0 on success, -ENODEV if there is no such device and no device
 * with a higher device number, -ENOENT if there is no such device but there
 * is one with a higher number, or other -ve on other error.
 */
int blk_get_desc(enum uclass_id uclass_id, int devnum, struct blk_desc **descp);

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
	if (blkcache_read(block_dev->uclass_id, block_dev->devnum,
			  start, blkcnt, block_dev->blksz, buffer))
		return blkcnt;

	/*
	 * We could check if block_read is NULL and return -ENOSYS. But this
	 * bloats the code slightly (cause some board to fail to build), and
	 * it would be an error to try an operation that does not exist.
	 */
	blks_read = block_dev->block_read(block_dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(block_dev->uclass_id, block_dev->devnum,
			      start, blkcnt, block_dev->blksz, buffer);

	return blks_read;
}

static inline ulong blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt, const void *buffer)
{
	blkcache_invalidate(block_dev->uclass_id, block_dev->devnum);
	return block_dev->block_write(block_dev, start, blkcnt, buffer);
}

static inline ulong blk_derase(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt)
{
	blkcache_invalidate(block_dev->uclass_id, block_dev->devnum);
	return block_dev->block_erase(block_dev, start, blkcnt);
}

/**
 * struct blk_driver - Driver for block interface types
 *
 * This provides access to the block devices for each interface type. One
 * driver should be provided using U_BOOT_LEGACY_BLK() for each interface
 * type that is to be supported.
 *
 * @uclass_idname:	Interface type name
 * @uclass_id:		Interface type
 * @max_devs:		Maximum number of devices supported
 * @desc:		Pointer to list of devices for this interface type,
 *			or NULL to use @get_dev() instead
 */
struct blk_driver {
	const char *uclass_idname;
	enum uclass_id uclass_id;
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

struct blk_driver *blk_driver_lookup_type(int uclass_id);

#endif /* !CONFIG_BLK */

/**
 * blk_get_devnum_by_uclass_idname() - Get a block device by type and number
 *
 * This looks through the available block devices of the given type, returning
 * the one with the given @devnum.
 *
 * @uclass_id:	Block device type
 * @devnum:	Device number
 * Return: point to block device descriptor, or NULL if not found
 */
struct blk_desc *blk_get_devnum_by_uclass_id(enum uclass_id uclass_id, int devnum);

/**
 * blk_get_devnum_by_uclass_id() - Get a block device by type name, and number
 *
 * This looks up the block device type based on @uclass_idname, then calls
 * blk_get_devnum_by_uclass_id().
 *
 * @uclass_idname:	Block device type name
 * @devnum:		Device number
 * Return: point to block device descriptor, or NULL if not found
 */
struct blk_desc *blk_get_devnum_by_uclass_idname(const char *uclass_idname,
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
 * Return: 0 if OK, -ve on error
 */
int blk_dselect_hwpart(struct blk_desc *desc, int hwpart);

/**
 * blk_list_part() - list the partitions for block devices of a given type
 *
 * This looks up the partition type for each block device of type @uclass_id,
 * then displays a list of partitions.
 *
 * @uclass_id:	Block device type
 * Return: 0 if OK, -ENODEV if there is none of that type
 */
int blk_list_part(enum uclass_id uclass_id);

/**
 * blk_list_devices() - list the block devices of a given type
 *
 * This lists each block device of the type @uclass_id, showing the capacity
 * as well as type-specific information.
 *
 * @uclass_id:	Block device type
 */
void blk_list_devices(enum uclass_id uclass_id);

/**
 * blk_show_device() - show information about a given block device
 *
 * This shows the block device capacity as well as type-specific information.
 *
 * @uclass_id:	Block device type
 * @devnum:	Device number
 * Return: 0 if OK, -ENODEV for invalid device number
 */
int blk_show_device(enum uclass_id uclass_id, int devnum);

/**
 * blk_print_device_num() - show information about a given block device
 *
 * This is similar to blk_show_device() but returns an error if the block
 * device type is unknown.
 *
 * @uclass_id:	Block device type
 * @devnum:	Device number
 * Return: 0 if OK, -ENODEV for invalid device number, -ENOENT if the block
 * device is not connected
 */
int blk_print_device_num(enum uclass_id uclass_id, int devnum);

/**
 * blk_print_part_devnum() - print the partition information for a device
 *
 * @uclass_id:	Block device type
 * @devnum:	Device number
 * Return: 0 if OK, -ENOENT if the block device is not connected, -ENOSYS if
 * the interface type is not supported, other -ve on other error
 */
int blk_print_part_devnum(enum uclass_id uclass_id, int devnum);

/**
 * blk_select_hwpart_devnum() - select a hardware partition
 *
 * This is similar to blk_dselect_hwpart() but it looks up the interface and
 * device number.
 *
 * @uclass_id:	Block device type
 * @devnum:	Device number
 * @hwpart:	Partition number to select
 * Return: 0 if OK, -ve on error
 */
int blk_select_hwpart_devnum(enum uclass_id uclass_id, int devnum, int hwpart);

/**
 * blk_get_uclass_name() - Get the name of an interface type
 *
 * @uclass_id: Interface type to check
 * Return: name of interface, or NULL if none
 */
const char *blk_get_uclass_name(enum uclass_id uclass_id);

/**
 * blk_common_cmd() - handle common commands with block devices
 *
 * @args: Number of arguments to the command (argv[0] is the command itself)
 * @argv: Command arguments
 * @uclass_id: Interface type
 * @cur_devnump: Current device number for this interface type
 * Return: 0 if OK, CMD_RET_ERROR on error
 */
int blk_common_cmd(int argc, char *const argv[], enum uclass_id uclass_id,
		   int *cur_devnump);

enum blk_flag_t {
	BLKF_FIXED	= 1 << 0,
	BLKF_REMOVABLE	= 1 << 1,
	BLKF_BOTH	= BLKF_FIXED | BLKF_REMOVABLE,
};

/**
 * blk_first_device_err() - Get the first block device
 *
 * The device returned is probed if necessary, and ready for use
 *
 * @flags: Indicates type of device to return
 * @devp: Returns pointer to the first device in that uclass, or NULL if none
 * Return: 0 if found, -ENODEV if not found, other -ve on error
 */
int blk_first_device_err(enum blk_flag_t flags, struct udevice **devp);

/**
 * blk_next_device_err() - Get the next block device
 *
 * The device returned is probed if necessary, and ready for use
 *
 * @flags: Indicates type of device to return
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the uclass if no error occurred, or -ENODEV if
 * there is no next device.
 * Return: 0 if found, -ENODEV if not found, other -ve on error
 */
int blk_next_device_err(enum blk_flag_t flags, struct udevice **devp);

/**
 * blk_find_first() - Return the first matching block device
 * @flags: Indicates type of device to return
 * @devp:	Returns pointer to device, or NULL on error
 *
 * The device is not prepared for use - this is an internal function.
 * The function uclass_get_device_tail() can be used to probe the device.
 *
 * Note that some devices are considered removable until they have been probed
 *
 * @return 0 if found, -ENODEV if not found
 */
int blk_find_first(enum blk_flag_t flags, struct udevice **devp);

/**
 * blk_find_next() - Return the next matching block device
 * @flags: Indicates type of device to return
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the same uclass, or NULL if none
 *
 * The device is not prepared for use - this is an internal function.
 * The function uclass_get_device_tail() can be used to probe the device.
 *
 * Note that some devices are considered removable until they have been probed
 *
 * @return 0 if found, -ENODEV if not found
 */
int blk_find_next(enum blk_flag_t flags, struct udevice **devp);

/**
 * blk_foreach() - iterate through block devices
 *
 * This creates a for() loop which works through the available block devices in
 * order from start to end.
 *
 * If for some reason the uclass cannot be found, this does nothing.
 *
 * @_flags: Indicates type of device to return
 * @_pos: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 */
#define blk_foreach(_flags, _pos) \
	for (int _ret = blk_find_first(_flags, &_pos); !_ret && _pos; \
	     _ret = blk_find_next(_flags, &_pos))

/**
 * blk_foreach_probe() - Helper function to iteration through block devices
 *
 * This creates a for() loop which works through the available devices in
 * a uclass in order from start to end. Devices are probed if necessary,
 * and ready for use.
 *
 * @flags: Indicates type of device to return
 * @dev: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 */
#define blk_foreach_probe(flags, pos)	\
	for (int _ret = blk_first_device_err(flags, &(pos)); \
	     !_ret && pos; \
	     _ret = blk_next_device_err(flags, &(pos)))

/**
 * blk_count_devices() - count the number of devices of a particular type
 *
 * @flags: Indicates type of device to find
 * Return: number of devices matching those flags
 */
int blk_count_devices(enum blk_flag_t flag);

#endif
