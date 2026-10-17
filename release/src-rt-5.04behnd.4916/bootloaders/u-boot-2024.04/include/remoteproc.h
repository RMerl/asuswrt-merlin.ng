/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef _RPROC_H_
#define _RPROC_H_

/*
 * Note: The platform data support is not meant for use with newer
 * platforms. This is meant only for legacy devices. This mode of
 * initialization *will* be eventually removed once all necessary
 * platforms have moved to dm/fdt.
 */
#include <dm/platdata.h>	/* For platform data support - non dt world */
#include <linux/errno.h>

/**
 * struct fw_rsc_hdr - firmware resource entry header
 * @type: resource type
 * @data: resource data
 *
 * Every resource entry begins with a 'struct fw_rsc_hdr' header providing
 * its @type. The content of the entry itself will immediately follow
 * this header, and it should be parsed according to the resource type.
 */
struct fw_rsc_hdr {
	u32 type;
	u8 data[0];
};

/**
 * enum fw_resource_type - types of resource entries
 *
 * @RSC_CARVEOUT:   request for allocation of a physically contiguous
 *		    memory region.
 * @RSC_DEVMEM:     request to iommu_map a memory-based peripheral.
 * @RSC_TRACE:	    announces the availability of a trace buffer into which
 *		    the remote processor will be writing logs.
 * @RSC_VDEV:       declare support for a virtio device, and serve as its
 *		    virtio header.
 * @RSC_PRELOAD_VENDOR: a vendor resource type that needs to be handled by
 *		    remoteproc implementations before loading
 * @RSC_POSTLOAD_VENDOR: a vendor resource type that needs to be handled by
 *		    remoteproc implementations after loading
 * @RSC_LAST:       just keep this one at the end
 *
 * For more details regarding a specific resource type, please see its
 * dedicated structure below.
 *
 * Please note that these values are used as indices to the rproc_handle_rsc
 * lookup table, so please keep them sane. Moreover, @RSC_LAST is used to
 * check the validity of an index before the lookup table is accessed, so
 * please update it as needed.
 */
enum fw_resource_type {
	RSC_CARVEOUT		= 0,
	RSC_DEVMEM		= 1,
	RSC_TRACE		= 2,
	RSC_VDEV		= 3,
	RSC_PRELOAD_VENDOR	= 4,
	RSC_POSTLOAD_VENDOR	= 5,
	RSC_LAST		= 6,
};

#define FW_RSC_ADDR_ANY (-1)

/**
 * struct fw_rsc_carveout - physically contiguous memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 *
 * This resource entry requests the host to allocate a physically contiguous
 * memory region.
 *
 * These request entries should precede other firmware resource entries,
 * as other entries might request placing other data objects inside
 * these memory regions (e.g. data/code segments, trace resource entries, ...).
 *
 * Allocating memory this way helps utilizing the reserved physical memory
 * (e.g. CMA) more efficiently, and also minimizes the number of TLB entries
 * needed to map it (in case @rproc is using an IOMMU). Reducing the TLB
 * pressure is important; it may have a substantial impact on performance.
 *
 * If the firmware is compiled with static addresses, then @da should specify
 * the expected device address of this memory region. If @da is set to
 * FW_RSC_ADDR_ANY, then the host will dynamically allocate it, and then
 * overwrite @da with the dynamically allocated address.
 *
 * We will always use @da to negotiate the device addresses, even if it
 * isn't using an iommu. In that case, though, it will obviously contain
 * physical addresses.
 *
 * Some remote processors needs to know the allocated physical address
 * even if they do use an iommu. This is needed, e.g., if they control
 * hardware accelerators which access the physical memory directly (this
 * is the case with OMAP4 for instance). In that case, the host will
 * overwrite @pa with the dynamically allocated physical address.
 * Generally we don't want to expose physical addresses if we don't have to
 * (remote processors are generally _not_ trusted), so we might want to
 * change this to happen _only_ when explicitly required by the hardware.
 *
 * @flags is used to provide IOMMU protection flags, and @name should
 * (optionally) contain a human readable name of this carveout region
 * (mainly for debugging purposes).
 */
struct fw_rsc_carveout {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
};

/**
 * struct fw_rsc_devmem - iommu mapping request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested region to be mapped
 *
 * This resource entry requests the host to iommu map a physically contiguous
 * memory region. This is needed in case the remote processor requires
 * access to certain memory-based peripherals; _never_ use it to access
 * regular memory.
 *
 * This is obviously only needed if the remote processor is accessing memory
 * via an iommu.
 *
 * @da should specify the required device address, @pa should specify
 * the physical address we want to map, @len should specify the size of
 * the mapping and @flags is the IOMMU protection flags. As always, @name may
 * (optionally) contain a human readable name of this mapping (mainly for
 * debugging purposes).
 *
 * Note: at this point we just "trust" those devmem entries to contain valid
 * physical addresses, but this isn't safe and will be changed: eventually we
 * want remoteproc implementations to provide us ranges of physical addresses
 * the firmware is allowed to request, and not allow firmwares to request
 * access to physical addresses that are outside those ranges.
 */
struct fw_rsc_devmem {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
};

/**
 * struct fw_rsc_trace - trace buffer declaration
 * @da: device address
 * @len: length (in bytes)
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the trace buffer
 *
 * This resource entry provides the host information about a trace buffer
 * into which the remote processor will write log messages.
 *
 * @da specifies the device address of the buffer, @len specifies
 * its size, and @name may contain a human readable name of the trace buffer.
 *
 * After booting the remote processor, the trace buffers are exposed to the
 * user via debugfs entries (called trace0, trace1, etc..).
 */
struct fw_rsc_trace {
	u32 da;
	u32 len;
	u32 reserved;
	u8 name[32];
};

/**
 * struct fw_rsc_vdev_vring - vring descriptor entry
 * @da: device address
 * @align: the alignment between the consumer and producer parts of the vring
 * @num: num of buffers supported by this vring (must be power of two)
 * @notifyid is a unique rproc-wide notify index for this vring. This notify
 * index is used when kicking a remote processor, to let it know that this
 * vring is triggered.
 * @pa: physical address
 *
 * This descriptor is not a resource entry by itself; it is part of the
 * vdev resource type (see below).
 *
 * Note that @da should either contain the device address where
 * the remote processor is expecting the vring, or indicate that
 * dynamically allocation of the vring's device address is supported.
 */
struct fw_rsc_vdev_vring {
	u32 da;
	u32 align;
	u32 num;
	u32 notifyid;
	u32 pa;
};

/**
 * struct fw_rsc_vdev - virtio device header
 * @id: virtio device id (as in virtio_ids.h)
 * @notifyid is a unique rproc-wide notify index for this vdev. This notify
 * index is used when kicking a remote processor, to let it know that the
 * status/features of this vdev have changes.
 * @dfeatures specifies the virtio device features supported by the firmware
 * @gfeatures is a place holder used by the host to write back the
 * negotiated features that are supported by both sides.
 * @config_len is the size of the virtio config space of this vdev. The config
 * space lies in the resource table immediate after this vdev header.
 * @status is a place holder where the host will indicate its virtio progress.
 * @num_of_vrings indicates how many vrings are described in this vdev header
 * @reserved: reserved (must be zero)
 * @vring is an array of @num_of_vrings entries of 'struct fw_rsc_vdev_vring'.
 *
 * This resource is a virtio device header: it provides information about
 * the vdev, and is then used by the host and its peer remote processors
 * to negotiate and share certain virtio properties.
 *
 * By providing this resource entry, the firmware essentially asks remoteproc
 * to statically allocate a vdev upon registration of the rproc (dynamic vdev
 * allocation is not yet supported).
 *
 * Note: unlike virtualization systems, the term 'host' here means
 * the Linux side which is running remoteproc to control the remote
 * processors. We use the name 'gfeatures' to comply with virtio's terms,
 * though there isn't really any virtualized guest OS here: it's the host
 * which is responsible for negotiating the final features.
 * Yeah, it's a bit confusing.
 *
 * Note: immediately following this structure is the virtio config space for
 * this vdev (which is specific to the vdev; for more info, read the virtio
 * spec). the size of the config space is specified by @config_len.
 */
struct fw_rsc_vdev {
	u32 id;
	u32 notifyid;
	u32 dfeatures;
	u32 gfeatures;
	u32 config_len;
	u8 status;
	u8 num_of_vrings;
	u8 reserved[2];
	struct fw_rsc_vdev_vring vring[0];
};

/**
 * struct rproc_mem_entry - memory entry descriptor
 * @va:	virtual address
 * @dma: dma address
 * @len: length, in bytes
 * @da: device address
 * @priv: associated data
 * @name: associated memory region name (optional)
 * @node: list node
 */
struct rproc_mem_entry {
	void *va;
	dma_addr_t dma;
	int len;
	u32 da;
	void *priv;
	char name[32];
	struct list_head node;
};

struct rproc;

typedef u32(*init_func_proto) (u32 core_id, struct rproc *cfg);

struct l3_map {
	u32 priv_addr;
	u32 l3_addr;
	u32 len;
};

struct rproc_intmem_to_l3_mapping {
	u32 num_entries;
	struct l3_map mappings[16];
};

/**
 * enum rproc_crash_type - remote processor crash types
 * @RPROC_MMUFAULT:	iommu fault
 * @RPROC_WATCHDOG:	watchdog bite
 * @RPROC_FATAL_ERROR	fatal error
 *
 * Each element of the enum is used as an array index. So that, the value of
 * the elements should be always something sane.
 *
 * Feel free to add more types when needed.
 */
enum rproc_crash_type {
	RPROC_MMUFAULT,
	RPROC_WATCHDOG,
	RPROC_FATAL_ERROR,
};

/* we currently support only two vrings per rvdev */
#define RVDEV_NUM_VRINGS 2

#define RPMSG_NUM_BUFS         (512)
#define RPMSG_BUF_SIZE         (512)
#define RPMSG_TOTAL_BUF_SPACE  (RPMSG_NUM_BUFS * RPMSG_BUF_SIZE)

/**
 * struct rproc_vring - remoteproc vring state
 * @va:	virtual address
 * @dma: dma address
 * @len: length, in bytes
 * @da: device address
 * @align: vring alignment
 * @notifyid: rproc-specific unique vring index
 * @rvdev: remote vdev
 * @vq: the virtqueue of this vring
 */
struct rproc_vring {
	void *va;
	dma_addr_t dma;
	int len;
	u32 da;
	u32 align;
	int notifyid;
	struct rproc_vdev *rvdev;
	struct virtqueue *vq;
};

/** struct rproc - structure with all processor specific information for
 * loading remotecore from boot loader.
 *
 * @num_iommus: Number of IOMMUs for this remote core. Zero indicates that the
 * processor does not have an IOMMU.
 *
 * @cma_base: Base address of the carveout for this remotecore.
 *
 * @cma_size: Length of the carveout in bytes.
 *
 * @page_table_addr: array with the physical address of the page table. We are
 * using the same page table for both IOMMU's. There is currently no strong
 * usecase for maintaining different page tables for different MMU's servicing
 * the same CPU.
 *
 * @mmu_base_addr: base address of the MMU
 *
 * @entry_point: address that is the entry point for the remote core. This
 * address is in the memory view of the remotecore.
 *
 * @load_addr: Address to which the bootloader loads the firmware from
 * persistent storage before invoking the ELF loader. Keeping this address
 * configurable allows future optimizations such as loading the firmware from
 * storage for remotecore2 via EDMA while the CPU is processing the ELF image
 * of remotecore1. This address is in the memory view of the A15.
 *
 * @firmware_name: Name of the file that is expected to contain the ELF image.
 *
 * @has_rsc_table: Flag populated after parsing the ELF binary on target.
 */

struct rproc {
	u32 num_iommus;
	unsigned long cma_base;
	u32 cma_size;
	unsigned long page_table_addr;
	unsigned long mmu_base_addr[2];
	unsigned long load_addr;
	unsigned long entry_point;
	char *core_name;
	char *firmware_name;
	char *ptn;
	init_func_proto start_clocks;
	init_func_proto config_mmu;
	init_func_proto config_peripherals;
	init_func_proto start_core;
	u32 has_rsc_table;
	struct rproc_intmem_to_l3_mapping *intmem_to_l3_mapping;
	u32 trace_pa;
	u32 trace_len;
};

extern struct rproc *rproc_cfg_arr[2];
/**
 * enum rproc_mem_type - What type of memory model does the rproc use
 * @RPROC_INTERNAL_MEMORY_MAPPED: Remote processor uses own memory and is memory
 *	mapped to the host processor over an address range.
 *
 * Please note that this is an enumeration of memory model of different types
 * of remote processors. Few of the remote processors do have own internal
 * memories, while others use external memory for instruction and data.
 */
enum rproc_mem_type {
	RPROC_INTERNAL_MEMORY_MAPPED	= 0,
};

/**
 * struct dm_rproc_uclass_pdata - platform data for a CPU
 * @name: Platform-specific way of naming the Remote proc
 * @mem_type: one of 'enum rproc_mem_type'
 * @driver_plat_data: driver specific platform data that may be needed.
 *
 * This can be accessed with dev_get_uclass_plat() for any UCLASS_REMOTEPROC
 * device.
 *
 */
struct dm_rproc_uclass_pdata {
	const char *name;
	enum rproc_mem_type mem_type;
	void *driver_plat_data;
};

/**
 * struct dm_rproc_ops - Driver model remote proc operations.
 *
 * This defines the operations provided by remote proc driver.
 */
struct dm_rproc_ops {
	/**
	 * init() - Initialize the remoteproc device (optional)
	 *
	 * This is called after the probe is completed allowing the remote
	 * processor drivers to split up the initializations between probe and
	 * init if needed.
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*init)(struct udevice *dev);

	/**
	 * load() - Load the remoteproc device using data provided (mandatory)
	 *
	 * Load the remoteproc device with an image, do not start the device.
	 *
	 * @dev:	Remote proc device
	 * @addr:	Address of the image to be loaded
	 * @size:	Size of the image to be loaded
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*load)(struct udevice *dev, ulong addr, ulong size);

	/**
	 * start() - Start the remoteproc device (mandatory)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*start)(struct udevice *dev);

	/**
	 * stop() - Stop the remoteproc device (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*stop)(struct udevice *dev);

	/**
	 * reset() - Reset the remoteproc device (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if all ok, else appropriate error value.
	 */
	int (*reset)(struct udevice *dev);

	/**
	 * is_running() - Check if the remote processor is running (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 if running, 1 if not running, -ve on error.
	 */
	int (*is_running)(struct udevice *dev);

	/**
	 * ping() - Ping the remote device for basic communication (optional)
	 *
	 * @dev:	Remote proc device
	 * @return 0 on success, 1 if not responding, -ve on other errors.
	 */
	int (*ping)(struct udevice *dev);

	/**
	 * device_to_virt() - Return translated virtual address (optional)
	 *
	 * Translate a device address (remote processor view) to virtual
	 * address (main processor view).
	 *
	 * @dev:	Remote proc device
	 * @da:		Device address
	 * @size:	Size of the memory region @da is pointing to
	 * @return virtual address.
	 */
	void * (*device_to_virt)(struct udevice *dev, ulong da, ulong size);
	int (*add_res)(struct udevice *dev,
		       struct rproc_mem_entry *mapping);
	void * (*alloc_mem)(struct udevice *dev, unsigned long len,
			    unsigned long align);
	unsigned int (*config_pagetable)(struct udevice *dev, unsigned int virt,
					 unsigned int phys, unsigned int len);
};

/* Accessor */
#define rproc_get_ops(dev) ((struct dm_rproc_ops *)(dev)->driver->ops)

#if CONFIG_IS_ENABLED(REMOTEPROC)
/**
 * rproc_init() - Initialize all bound remote proc devices
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_init(void);

/**
 * rproc_dev_init() - Initialize a remote proc device based on id
 * @id:		id of the remote processor
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_dev_init(int id);

/**
 * rproc_is_initialized() - check to see if remoteproc devices are initialized
 * Return: true if all devices are initialized, false otherwise.
 */
bool rproc_is_initialized(void);

/**
 * rproc_load() - load binary or elf to a remote processor
 * @id:		id of the remote processor
 * @addr:	address in memory where the image is located
 * @size:	size of the image
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_load(int id, ulong addr, ulong size);

/**
 * rproc_start() - Start a remote processor
 * @id:		id of the remote processor
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_start(int id);

/**
 * rproc_stop() - Stop a remote processor
 * @id:		id of the remote processor
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_stop(int id);

/**
 * rproc_reset() - reset a remote processor
 * @id:		id of the remote processor
 * Return: 0 if all ok, else appropriate error value.
 */
int rproc_reset(int id);

/**
 * rproc_ping() - ping a remote processor to check if it can communicate
 * @id:		id of the remote processor
 * Return: 0 if all ok, else appropriate error value.
 *
 * NOTE: this might need communication path available, which is not implemented
 * as part of remoteproc framework - hook on to appropriate bus architecture to
 * do the same
 */
int rproc_ping(int id);

/**
 * rproc_is_running() - check to see if remote processor is running
 * @id:		id of the remote processor
 * Return: 0 if running, 1 if not running, -ve on error.
 *
 * NOTE: this may not involve actual communication capability of the remote
 * processor, but just ensures that it is out of reset and executing code.
 */
int rproc_is_running(int id);

/**
 * rproc_elf32_sanity_check() - Verify if an image is a valid ELF32 one
 *
 * Check if a valid ELF32 image exists at the given memory location. Verify
 * basic ELF32 format requirements like magic number and sections size.
 *
 * @addr:	address of the image to verify
 * @size:	size of the image
 * Return: 0 if the image looks good, else appropriate error value.
 */
int rproc_elf32_sanity_check(ulong addr, ulong size);

/**
 * rproc_elf64_sanity_check() - Verify if an image is a valid ELF32 one
 *
 * Check if a valid ELF64 image exists at the given memory location. Verify
 * basic ELF64 format requirements like magic number and sections size.
 *
 * @addr:	address of the image to verify
 * @size:	size of the image
 * Return: 0 if the image looks good, else appropriate error value.
 */
int rproc_elf64_sanity_check(ulong addr, ulong size);

/**
 * rproc_elf32_load_image() - load an ELF32 image
 * @dev:	device loading the ELF32 image
 * @addr:	valid ELF32 image address
 * @size:	size of the image
 * Return: 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf32_load_image(struct udevice *dev, unsigned long addr, ulong size);

/**
 * rproc_elf64_load_image() - load an ELF64 image
 * @dev:	device loading the ELF64 image
 * @addr:	valid ELF64 image address
 * @size:	size of the image
 * Return: 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf64_load_image(struct udevice *dev, ulong addr, ulong size);

/**
 * rproc_elf_load_image() - load an ELF image
 * @dev:	device loading the ELF image
 * @addr:	valid ELF image address
 * @size:	size of the image
 *
 * Auto detects if the image is ELF32 or ELF64 image and load accordingly.
 * Return: 0 if the image is successfully loaded, else appropriate error value.
 */
int rproc_elf_load_image(struct udevice *dev, unsigned long addr, ulong size);

/**
 * rproc_elf_get_boot_addr() - Get rproc's boot address.
 * @dev:	device loading the ELF image
 * @addr:	valid ELF image address
 *
 * This function returns the entry point address of the ELF
 * image.
 */
ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr);

/**
 * rproc_elf32_load_rsc_table() - load the resource table from an ELF32 image
 *
 * Search for the resource table in an ELF32 image, and if found, copy it to
 * device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * Return: 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf32_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size);
/**
 * rproc_elf64_load_rsc_table() - load the resource table from an ELF64 image
 *
 * Search for the resource table in an ELF64 image, and if found, copy it to
 * device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * Return: 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf64_load_rsc_table(struct udevice *dev, ulong fw_addr,
			       ulong fw_size, ulong *rsc_addr, ulong *rsc_size);
/**
 * rproc_elf_load_rsc_table() - load the resource table from an ELF image
 *
 * Auto detects if the image is ELF32 or ELF64 image and search accordingly for
 * the resource table, and if found, copy it to device memory.
 *
 * @dev:	device loading the resource table
 * @fw_addr:	ELF image address
 * @fw_size:	size of the ELF image
 * @rsc_addr:	pointer to the found resource table address. Updated on
 *		operation success
 * @rsc_size:	pointer to the found resource table size. Updated on operation
 *		success
 *
 * Return: 0 if a valid resource table is successfully loaded, -ENODATA if there
 * is no resource table (which is optional), or another appropriate error value.
 */
int rproc_elf_load_rsc_table(struct udevice *dev, ulong fw_addr,
			     ulong fw_size, ulong *rsc_addr, ulong *rsc_size);

unsigned long rproc_parse_resource_table(struct udevice *dev,
					 struct rproc *cfg);

struct resource_table *rproc_find_resource_table(struct udevice *dev,
						 unsigned int addr,
						 int *tablesz);
#else
static inline int rproc_init(void) { return -ENOSYS; }
static inline int rproc_dev_init(int id) { return -ENOSYS; }
static inline bool rproc_is_initialized(void) { return false; }
static inline int rproc_load(int id, ulong addr, ulong size) { return -ENOSYS; }
static inline int rproc_start(int id) { return -ENOSYS; }
static inline int rproc_stop(int id) { return -ENOSYS; }
static inline int rproc_reset(int id) { return -ENOSYS; }
static inline int rproc_ping(int id) { return -ENOSYS; }
static inline int rproc_is_running(int id) { return -ENOSYS; }
static inline int rproc_elf32_sanity_check(ulong addr,
					   ulong size) { return -ENOSYS; }
static inline int rproc_elf64_sanity_check(ulong addr,
					   ulong size) { return -ENOSYS; }
static inline int rproc_elf_sanity_check(ulong addr,
					 ulong size) { return -ENOSYS; }
static inline int rproc_elf32_load_image(struct udevice *dev,
					 unsigned long addr, ulong size)
{ return -ENOSYS; }
static inline int rproc_elf64_load_image(struct udevice *dev, ulong addr,
					 ulong size)
{ return -ENOSYS; }
static inline int rproc_elf_load_image(struct udevice *dev, ulong addr,
				       ulong size)
{ return -ENOSYS; }
static inline ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr)
{ return 0; }
static inline int rproc_elf32_load_rsc_table(struct udevice *dev, ulong fw_addr,
					     ulong fw_size, ulong *rsc_addr,
					     ulong *rsc_size)
{ return -ENOSYS; }
static inline int rproc_elf64_load_rsc_table(struct udevice *dev, ulong fw_addr,
					     ulong fw_size, ulong *rsc_addr,
					     ulong *rsc_size)
{ return -ENOSYS; }
static inline int rproc_elf_load_rsc_table(struct udevice *dev, ulong fw_addr,
					   ulong fw_size, ulong *rsc_addr,
					   ulong *rsc_size)
{ return -ENOSYS; }
#endif

#endif	/* _RPROC_H_ */
