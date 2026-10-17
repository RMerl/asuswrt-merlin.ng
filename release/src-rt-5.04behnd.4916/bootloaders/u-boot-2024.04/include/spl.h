/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 */
#ifndef	_SPL_H_
#define	_SPL_H_

#include <binman_sym.h>
#include <linker_lists.h>

/* Platform-specific defines */
#include <linux/compiler.h>
#include <asm/global_data.h>
#include <asm/spl.h>
#include <handoff.h>
#include <mmc.h>

struct blk_desc;
struct legacy_img_hdr;

/* Value in r0 indicates we booted from U-Boot */
#define UBOOT_NOT_LOADED_FROM_SPL	0x13578642

/* Boot type */
#define MMCSD_MODE_UNDEFINED	0
#define MMCSD_MODE_RAW		1
#define MMCSD_MODE_FS		2
#define MMCSD_MODE_EMMCBOOT	3

struct blk_desc;
struct legacy_img_hdr;
struct spl_boot_device;
enum boot_device;

/*
 * u_boot_first_phase() - check if this is the first U-Boot phase
 *
 * U-Boot has up to three phases: TPL, SPL and U-Boot proper. Depending on the
 * build flags we can determine whether the current build is for the first
 * phase of U-Boot or not. If there is no SPL, then this is U-Boot proper. If
 * there is SPL but no TPL, the the first phase is SPL. If there is TPL, then
 * it is the first phase.
 *
 * @returns true if this is the first phase of U-Boot
 *
 */
static inline bool u_boot_first_phase(void)
{
	if (IS_ENABLED(CONFIG_TPL)) {
		if (IS_ENABLED(CONFIG_TPL_BUILD))
			return true;
	} else if (IS_ENABLED(CONFIG_SPL)) {
		if (IS_ENABLED(CONFIG_SPL_BUILD))
			return true;
	} else {
		return true;
	}

	return false;
}

enum u_boot_phase {
	PHASE_NONE,	/* Invalid phase, signifying before U-Boot */
	PHASE_TPL,	/* Running in TPL */
	PHASE_VPL,	/* Running in VPL */
	PHASE_SPL,	/* Running in SPL */
	PHASE_BOARD_F,	/* Running in U-Boot before relocation */
	PHASE_BOARD_R,	/* Running in U-Boot after relocation */

	PHASE_COUNT,
};

/**
 * spl_phase() - Find out the phase of U-Boot
 *
 * This can be used to avoid #ifdef logic and use if() instead.
 *
 * For example, to include code only in TPL, you might do:
 *
 *    #ifdef CONFIG_TPL_BUILD
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_TPL) {
 *       ...
 *    }
 *
 * To include code only in SPL, you might do:
 *
 *    #if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_SPL) {
 *       ...
 *    }
 *
 * To include code only in U-Boot proper, you might do:
 *
 *    #ifndef CONFIG_SPL_BUILD
 *    ...
 *    #endif
 *
 * but with this you can use:
 *
 *    if (spl_phase() == PHASE_BOARD_F) {
 *       ...
 *    }
 *
 * Return: U-Boot phase
 */
static inline enum u_boot_phase spl_phase(void)
{
#ifdef CONFIG_TPL_BUILD
	return PHASE_TPL;
#elif defined(CONFIG_VPL_BUILD)
	return PHASE_VPL;
#elif defined(CONFIG_SPL_BUILD)
	return PHASE_SPL;
#else
	DECLARE_GLOBAL_DATA_PTR;

	if (!(gd->flags & GD_FLG_RELOC))
		return PHASE_BOARD_F;
	else
		return PHASE_BOARD_R;
#endif
}

/* returns true if in U-Boot proper, false if in SPL */
static inline bool spl_in_proper(void)
{
#ifdef CONFIG_SPL_BUILD
	return false;
#endif

	return true;
}

/**
 * spl_prev_phase() - Figure out the previous U-Boot phase
 *
 * Return: the previous phase from this one, e.g. if called in SPL this returns
 *	PHASE_TPL, if TPL is enabled
 */
static inline enum u_boot_phase spl_prev_phase(void)
{
#ifdef CONFIG_TPL_BUILD
	return PHASE_NONE;
#elif defined(CONFIG_VPL_BUILD)
	return PHASE_TPL;	/* VPL requires TPL */
#elif defined(CONFIG_SPL_BUILD)
	return IS_ENABLED(CONFIG_VPL) ? PHASE_VPL :
		IS_ENABLED(CONFIG_TPL) ? PHASE_TPL :
		PHASE_NONE;
#else
	return IS_ENABLED(CONFIG_SPL) ? PHASE_SPL :
		PHASE_NONE;
#endif
}

/**
 * spl_next_phase() - Figure out the next U-Boot phase
 *
 * Return: the next phase from this one, e.g. if called in TPL this returns
 *	PHASE_SPL
 */
static inline enum u_boot_phase spl_next_phase(void)
{
#ifdef CONFIG_TPL_BUILD
	return IS_ENABLED(CONFIG_VPL) ? PHASE_VPL : PHASE_SPL;
#elif defined(CONFIG_VPL_BUILD)
	return PHASE_SPL;
#else
	return PHASE_BOARD_F;
#endif
}

/**
 * spl_phase_name() - Get the name of the current phase
 *
 * Return: phase name
 */
static inline const char *spl_phase_name(enum u_boot_phase phase)
{
	switch (phase) {
	case PHASE_TPL:
		return "TPL";
	case PHASE_VPL:
		return "VPL";
	case PHASE_SPL:
		return "SPL";
	case PHASE_BOARD_F:
	case PHASE_BOARD_R:
		return "U-Boot";
	default:
		return "phase?";
	}
}

/**
 * spl_phase_prefix() - Get the prefix  of the current phase
 *
 * @phase: Phase to look up
 * Return: phase prefix ("spl", "tpl", etc.)
 */
static inline const char *spl_phase_prefix(enum u_boot_phase phase)
{
	switch (phase) {
	case PHASE_TPL:
		return "tpl";
	case PHASE_VPL:
		return "vpl";
	case PHASE_SPL:
		return "spl";
	case PHASE_BOARD_F:
	case PHASE_BOARD_R:
		return "";
	default:
		return "phase?";
	}
}

/* A string name for SPL or TPL */
#ifdef CONFIG_SPL_BUILD
# ifdef CONFIG_TPL_BUILD
#  define SPL_TPL_NAME	"TPL"
# elif defined(CONFIG_VPL_BUILD)
#  define SPL_TPL_NAME	"VPL"
# else
#  define SPL_TPL_NAME	"SPL"
# endif
# define SPL_TPL_PROMPT	SPL_TPL_NAME ": "
#else
# define SPL_TPL_NAME	""
# define SPL_TPL_PROMPT	""
#endif

/**
 * enum spl_sandbox_flags - flags for sandbox's use of spl_image_info->flags
 *
 * @SPL_SANDBOXF_ARG_IS_FNAME: arg is the filename to jump to (default)
 * @SPL_SANDBOXF_ARG_IS_BUF: arg is the containing image to jump to, @offset is
 *	the start offset within the image, @size is the size of the image
 */
enum spl_sandbox_flags {
	SPL_SANDBOXF_ARG_IS_FNAME = 0,
	SPL_SANDBOXF_ARG_IS_BUF,
};

struct spl_image_info {
	const char *name;
	u8 os;
	uintptr_t load_addr;
	uintptr_t entry_point;
#if CONFIG_IS_ENABLED(LOAD_FIT) || CONFIG_IS_ENABLED(LOAD_FIT_FULL)
	void *fdt_addr;
#endif
	u32 boot_device;
	u32 offset;
	u32 size;
	u32 flags;
	void *arg;
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	ulong dcrc_data;
	ulong dcrc_length;
	ulong dcrc;
#endif
};

static inline void *spl_image_fdt_addr(struct spl_image_info *info)
{
#if CONFIG_IS_ENABLED(LOAD_FIT) || CONFIG_IS_ENABLED(LOAD_FIT_FULL)
	return info->fdt_addr;
#else
	return 0;
#endif
}

/**
 * Information required to load data from a device
 *
 * @priv: Private data for the device
 * @bl_len: Block length for reading in bytes
 * @read: Function to call to read from the device
 */
struct spl_load_info {
	void *priv;
	/**
	 * read() - Read from device
	 *
	 * @load: Information about the load state
	 * @offset: Offset to read from in bytes. This must be a multiple of
	 *          @load->bl_len.
	 * @count: Number of bytes to read. This must be a multiple of
	 *         @load->bl_len.
	 * @buf: Buffer to read into
	 * @return number of bytes read, 0 on error
	 */
	ulong (*read)(struct spl_load_info *load, ulong sector, ulong count,
		      void *buf);
#if IS_ENABLED(CONFIG_SPL_LOAD_BLOCK)
	int bl_len;
};

static inline int spl_get_bl_len(struct spl_load_info *info)
{
	return info->bl_len;
}

static inline void spl_set_bl_len(struct spl_load_info *info, int bl_len)
{
	info->bl_len = bl_len;
}
#else
};

static inline int spl_get_bl_len(struct spl_load_info *info)
{
	return 1;
}

static inline void spl_set_bl_len(struct spl_load_info *info, int bl_len)
{
	if (bl_len != 1)
		panic("CONFIG_SPL_LOAD_BLOCK not enabled");
}
#endif

/*
 * We need to know the position of U-Boot in memory so we can jump to it. We
 * allow any U-Boot binary to be used (u-boot.bin, u-boot-nodtb.bin,
 * u-boot.img), hence the '_any'. These is no checking here that the correct
 * image is found. For example if u-boot.img is used we don't check that
 * spl_parse_image_header() can parse a valid header.
 *
 * Similarly for SPL, so that TPL can jump to SPL.
 */
binman_sym_extern(ulong, u_boot_any, image_pos);
binman_sym_extern(ulong, u_boot_any, size);
binman_sym_extern(ulong, u_boot_spl_any, image_pos);
binman_sym_extern(ulong, u_boot_spl_any, size);
binman_sym_extern(ulong, u_boot_vpl_any, image_pos);
binman_sym_extern(ulong, u_boot_vpl_any, size);

/**
 * spl_get_image_pos() - get the image position of the next phase
 *
 * This returns the image position to use to load the next phase of U-Boot
 */
ulong spl_get_image_pos(void);

/**
 * spl_get_image_size() - get the size of the next phase
 *
 * This returns the size to use to load the next phase of U-Boot
 */
ulong spl_get_image_size(void);

/**
 * spl_get_image_text_base() - get the text base of the next phase
 *
 * This returns the address that the next stage is linked to run at, i.e.
 * CONFIG_SPL_TEXT_BASE or CONFIG_TEXT_BASE
 *
 * Return: text-base address
 */
ulong spl_get_image_text_base(void);

/**
 * spl_load_simple_fit_skip_processing() - Hook to allow skipping the FIT
 *	image processing during spl_load_simple_fit().
 *
 * Return true to skip FIT processing, false to preserve the full code flow
 * of spl_load_simple_fit().
 */
bool spl_load_simple_fit_skip_processing(void);

/**
 * spl_load_simple_fit_fix_load() - Hook to make fixes
 * after fit image header is loaded
 *
 * Returns pointer to fit
 */
void *spl_load_simple_fit_fix_load(const void *fit);

/**
 * spl_load_simple_fit() - Loads a fit image from a device.
 * @spl_image:	Image description to set up
 * @info:	Structure containing the information required to load data.
 * @offset:	Offset where FIT image is located in the device. Must be aligned
 *              to the device's bl_len.
 * @fdt:	Pointer to the copied FIT header.
 *
 * Reads the FIT image @sector in the device. Loads u-boot image to
 * specified load address and copies the dtb to end of u-boot image.
 * Returns 0 on success.
 */
int spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong offset, void *fdt);

#define SPL_COPY_PAYLOAD_ONLY	1
#define SPL_FIT_FOUND		2

/**
 * spl_load_legacy_lzma() - Load an LZMA-compressed legacy image
 * @spl_image:	Image description (already set up)
 * @load:	Structure containing the information required to load data.
 * @offset:	Pointer to image
 *
 * Load/decompress an LZMA-compressed legacy image from the device.
 *
 * Return: 0 on success, or a negative error on failure
 */
int spl_load_legacy_lzma(struct spl_image_info *spl_image,
			 struct spl_load_info *load, ulong offset);

/**
 * spl_load_legacy_img() - Loads a legacy image from a device.
 * @spl_image:	Image description to set up
 * @load:	Structure containing the information required to load data.
 * @offset:	Pointer to image
 * @hdr:	Pointer to image header
 *
 * Reads an legacy image from the device. Loads u-boot image to
 * specified load address.
 * Returns 0 on success.
 */
int spl_load_legacy_img(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev,
			struct spl_load_info *load, ulong offset,
			struct legacy_img_hdr *hdr);


/**
 * spl_load_imx_container() - Loads a imx container image from a device.
 * @spl_image:	Image description to set up
 * @info:	Structure containing the information required to load data.
 * @sector:	Offset where container image is located in the device. Must be
 *              aligned to the device block size.
 *
 * Reads the container image @sector in the device. Loads u-boot image to
 * specified load address.
 */
int spl_load_imx_container(struct spl_image_info *spl_image,
			   struct spl_load_info *info, ulong offset);

/* SPL common functions */
void preloader_console_init(void);
u32 spl_boot_device(void);

struct spi_flash;

/**
 * spl_spi_get_uboot_offs() - Lookup function for the SPI boot offset
 * @flash: The spi flash to boot from
 *
 * Return: The offset of U-Boot within the SPI flash
 */
unsigned int spl_spi_get_uboot_offs(struct spi_flash *flash);

/**
 * spl_spi_boot_bus() - Lookup function for the SPI boot bus source.
 *
 * This function returns the SF bus to load from.
 * If not overridden, it is weakly defined in common/spl/spl_spi.c.
 */
u32 spl_spi_boot_bus(void);

/**
 * spl_spi_boot_cs() - Lookup function for the SPI boot CS source.
 *
 * This function returns the SF CS to load from.
 * If not overridden, it is weakly defined in common/spl/spl_spi.c.
 */
u32 spl_spi_boot_cs(void);

/**
 * spl_mmc_boot_mode() - Lookup function for the mode of an MMC boot source.
 * @boot_device:	ID of the device which the MMC driver wants to read
 *			from.  Common values are e.g. BOOT_DEVICE_MMC1,
 *			BOOT_DEVICE_MMC2, BOOT_DEVICE_MMC2_2.
 *
 * This function should return one of MMCSD_MODE_FS, MMCSD_MODE_EMMCBOOT, or
 * MMCSD_MODE_RAW for each MMC boot source which is defined for the target.  The
 * boot_device parameter tells which device the MMC driver is interested in.
 *
 * If not overridden, it is weakly defined in common/spl/spl_mmc.c.
 *
 * Note:  It is important to use the boot_device parameter instead of e.g.
 * spl_boot_device() as U-Boot is not always loaded from the same device as SPL.
 */
u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device);

/**
 * spl_mmc_boot_partition() - MMC partition to load U-Boot from.
 * @boot_device:	ID of the device which the MMC driver wants to load
 *			U-Boot from.
 *
 * This function should return the partition number which the SPL
 * should load U-Boot from (on the given boot_device) when
 * CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION is set.
 *
 * If not overridden, it is weakly defined in common/spl/spl_mmc.c.
 */
int spl_mmc_boot_partition(const u32 boot_device);

struct mmc;
/**
 * default_spl_mmc_emmc_boot_partition() - eMMC boot partition to load U-Boot from.
 * mmc:			Pointer for the mmc device structure
 *
 * This function should return the eMMC boot partition number which
 * the SPL should load U-Boot from (on the given boot_device).
 */
int default_spl_mmc_emmc_boot_partition(struct mmc *mmc);

/**
 * spl_mmc_emmc_boot_partition() - eMMC boot partition to load U-Boot from.
 * mmc:			Pointer for the mmc device structure
 *
 * This function should return the eMMC boot partition number which
 * the SPL should load U-Boot from (on the given boot_device).
 *
 * If not overridden, it is weakly defined in common/spl/spl_mmc.c
 * and calls default_spl_mmc_emmc_boot_partition();
 */
int spl_mmc_emmc_boot_partition(struct mmc *mmc);

void spl_set_bd(void);

/**
 * spl_mmc_get_uboot_raw_sector() - Provide raw sector of the start of U-Boot (architecture override)
 *
 * This is a weak function which by default will provide the raw sector that is
 * where the start of the U-Boot image has been written to.
 *
 * @mmc: struct mmc that describes the devie where U-Boot resides
 * @raw_sect: The raw sector number where U-Boot is by default.
 * Return: The raw sector location that U-Boot resides at
 */
unsigned long arch_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						unsigned long raw_sect);

/**
 * spl_mmc_get_uboot_raw_sector() - Provide raw sector of the start of U-Boot (board override)
 *
 * This is a weak function which by default will provide the raw sector that is
 * where the start of the U-Boot image has been written to.
 *
 * @mmc: struct mmc that describes the devie where U-Boot resides
 * @raw_sect: The raw sector number where U-Boot is by default.
 * Return: The raw sector location that U-Boot resides at
 */
unsigned long board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
						 unsigned long raw_sect);

/**
 * spl_mmc_get_uboot_raw_sector() - Provide raw sector of the start of U-Boot
 *
 * This is a weak function which by default will provide the raw sector that is
 * where the start of the U-Boot image has been written to.
 *
 * @mmc: struct mmc that describes the devie where U-Boot resides
 * @raw_sect: The raw sector number where U-Boot is by default.
 * Return: The raw sector location that U-Boot resides at
 */
unsigned long spl_mmc_get_uboot_raw_sector(struct mmc *mmc,
					   unsigned long raw_sect);

/**
 * spl_set_header_raw_uboot() - Set up a standard SPL image structure
 *
 * This sets up the given spl_image which the standard values obtained from
 * config options: CONFIG_SYS_MONITOR_LEN, CONFIG_SYS_UBOOT_START,
 * CONFIG_TEXT_BASE.
 *
 * @spl_image: Image description to set up
 */
void spl_set_header_raw_uboot(struct spl_image_info *spl_image);

/**
 * spl_parse_image_header() - parse the image header and set up info
 *
 * This parses the legacy image header information at @header and sets up
 * @spl_image according to what is found. If no image header is found, then
 * a raw image or bootz is assumed. If CONFIG_SPL_PANIC_ON_RAW_IMAGE is
 * enabled, then this causes a panic. If CONFIG_SPL_RAW_IMAGE_SUPPORT is not
 * enabled then U-Boot gives up. Otherwise U-Boot sets up the image using
 * spl_set_header_raw_uboot(), or possibly the bootz header.
 *
 * @spl_image: Image description to set up
 * @header image header to parse
 * Return: 0 if a header was correctly parsed, -ve on error
 */
int spl_parse_image_header(struct spl_image_info *spl_image,
			   const struct spl_boot_device *bootdev,
			   const struct legacy_img_hdr *header);

void spl_board_prepare_for_linux(void);

/**
 * spl_board_prepare_for_optee() - Prepare board for an OPTEE payload
 *
 * Prepares the board for booting an OP-TEE payload. Initialization is platform
 * specific, and may include configuring the TrustZone memory, and other
 * initialization steps required by OP-TEE.
 * Note that @fdt is not used directly by OP-TEE. OP-TEE passes this @fdt to
 * its normal world target. This target is not guaranteed to be u-boot, so @fdt
 * changes that would normally be done by u-boot should be done in this step.
 *
 * @fdt: Devicetree that will be passed on, or NULL
 */
void spl_board_prepare_for_optee(void *fdt);
void spl_board_prepare_for_boot(void);
int spl_board_ubi_load_image(u32 boot_device);
int spl_board_boot_device(enum boot_device boot_dev_spl);

/**
 * spl_board_loader_name() - Return a name for the loader
 *
 * This is a weak function which might be overridden by the board code. With
 * that a board specific value for the device where the U-Boot will be loaded
 * from can be set. By default it returns NULL.
 *
 * @boot_device:	ID of the device which SPL wants to load U-Boot from.
 */
const char *spl_board_loader_name(u32 boot_device);

/**
 * jump_to_image_linux() - Jump to a Linux kernel from SPL
 *
 * This jumps into a Linux kernel using the information in @spl_image.
 *
 * @spl_image: Image description to set up
 */
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image);

/**
 * jump_to_image_optee() - Jump to OP-TEE OS from SPL
 *
 * This jumps into OP-TEE OS using the information in @spl_image.
 *
 * @spl_image: Image description to set up
 */
void __noreturn jump_to_image_optee(struct spl_image_info *spl_image);

/**
 * spl_start_uboot() - Check if SPL should start the kernel or U-Boot
 *
 * This is called by the various SPL loaders to determine whether the board
 * wants to load the kernel or U-Boot. This function should be provided by
 * the board.
 *
 * Return: 0 if SPL should start the kernel, 1 if U-Boot must be started
 */
int spl_start_uboot(void);

/**
 * spl_display_print() - Display a board-specific message in SPL
 *
 * If CONFIG_SPL_DISPLAY_PRINT is enabled, U-Boot will call this function
 * immediately after displaying the SPL console banner ("U-Boot SPL ...").
 * This function should be provided by the board.
 */
void spl_display_print(void);

/**
 * struct spl_boot_device - Describes a boot device used by SPL
 *
 * @boot_device: A number indicating the BOOT_DEVICE type. There are various
 * BOOT_DEVICE... #defines and enums in U-Boot and they are not consistently
 * numbered.
 * @boot_device_name: Named boot device, or NULL if none.
 *
 * Note: Additional fields can be added here, bearing in mind that SPL is
 * size-sensitive and common fields will be present on all boards. This
 * struct can also be used to return additional information about the load
 * process if that becomes useful.
 */
struct spl_boot_device {
	uint boot_device;
	const char *boot_device_name;
};

/**
 * Holds information about a way of loading an SPL image
 *
 * @name: User-friendly name for this method (e.g. "MMC")
 * @boot_device: Boot device that this loader supports
 * @load_image: Function to call to load image
 */
struct spl_image_loader {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	const char *name;
#endif
	uint boot_device;
	/**
	 * load_image() - Load an SPL image
	 *
	 * @spl_image: place to put image information
	 * @bootdev: describes the boot device to load from
	 */
	int (*load_image)(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev);
};

/* Helper function for accessing the name */
static inline const char *spl_loader_name(const struct spl_image_loader *loader)
{
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	const char *name;
	name = spl_board_loader_name(loader->boot_device);
	return name ?: loader->name;
#else
	return NULL;
#endif
}

/* Declare an SPL image loader */
#define SPL_LOAD_IMAGE(__name)					\
	ll_entry_declare(struct spl_image_loader, __name, spl_image_loader)

/*
 * _priority is the priority of this method, 0 meaning it will be the top
 * choice for this device, 9 meaning it is the bottom choice.
 * _boot_device is the BOOT_DEVICE_... value
 * _method is the load_image function to call
 */
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_boot_device ## _priority ## _method) = { \
		.name = _name, \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#else
#define SPL_LOAD_IMAGE_METHOD(_name, _priority, _boot_device, _method) \
	SPL_LOAD_IMAGE(_boot_device ## _priority ## _method) = { \
		.boot_device = _boot_device, \
		.load_image = _method, \
	}
#endif

#define SPL_LOAD_IMAGE_GET(_priority, _boot_device, _method) \
	ll_entry_get(struct spl_image_loader, \
		     _boot_device ## _priority ## _method, spl_image_loader)

/* SPL FAT image functions */

/**
 * spl_fat_force_reregister() - Force reregistration of FAT block devices
 *
 * To avoid repeatedly looking up block devices, spl_load_image_fat keeps track
 * of whether it has already registered a block device. This is fine for most
 * cases, but when running unit tests all devices are removed and recreated
 * in-between tests. This function will force re-registration of any block
 * devices, ensuring that we don't try to use an invalid block device.
 */
void spl_fat_force_reregister(void);

int spl_load_image_fat(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_fat_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, int partition);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image);

/* SPL EXT image functions */
int spl_load_image_ext(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, int partition,
		       const char *filename);
int spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, int partition);
int spl_blk_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       enum uclass_id uclass_id, int devnum, int partnum);

/**
 * spl_early_init() - Set up device tree and driver model in SPL if enabled
 *
 * Call this function in board_init_f() if you want to use device tree and
 * driver model early, before board_init_r() is called.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_early_init(void);

/**
 * spl_init() - Set up device tree and driver model in SPL if enabled
 *
 * You can optionally call spl_early_init(), then optionally call spl_init().
 * This function will be called from board_init_r() if not called earlier.
 *
 * Both spl_early_init() and spl_init() perform a similar function except that
 * the latter will not set up the malloc() area if
 * CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN is enabled, since it is assumed to
 * already be done by a calll to spl_relocate_stack_gd() before board_init_r()
 * is reached.
 *
 * This function will be called from board_init_r() if not called earlier.
 *
 * If this is not called, then driver model will be inactive in SPL's
 * board_init_f(), and no device tree will be available.
 */
int spl_init(void);

/*
 * spl_board_init() - Do board-specific init in SPL
 *
 * If xPL_BOARD_INIT is enabled, this is called from board_init_r() before
 * jumping to the next phase.
 */
void spl_board_init(void);

/**
 * spl_was_boot_source() - check if U-Boot booted from SPL
 *
 * This will normally be true, but if U-Boot jumps to second U-Boot, it will
 * be false. This should be implemented by board-specific code.
 *
 * Return: true if U-Boot booted from SPL, else false
 */
bool spl_was_boot_source(void);

/**
 * spl_dfu_cmd- run dfu command with chosen mmc device interface
 * @param usb_index - usb controller number
 * @param mmc_dev -  mmc device nubmer
 *
 * Return: 0 on success, otherwise error code
 */
int spl_dfu_cmd(int usbctrl, char *dfu_alt_info, char *interface, char *devstr);

/**
 * spl_mmc_clear_cache() - Clear cached MMC devices
 *
 * To avoid reinitializing MMCs, spl_mmc_load caches the most-recently-used MMC
 * device. This is fine for most cases, but when running unit tests all devices
 * are removed and recreated in-between tests. This function will clear any
 * cached state, ensuring that we don't try to use an invalid MMC.
 */
void spl_mmc_clear_cache(void);

int spl_mmc_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev);

/**
 * spl_mmc_load() - Load an image file from MMC/SD media
 *
 * @param spl_image	Image data filled in by loading process
 * @param bootdev	Describes which device to load from
 * @param filename	Name of file to load (in FS mode)
 * @param raw_part	Partition to load from (in RAW mode)
 * @param raw_sect	Sector to load from (in RAW mode)
 *
 * Return: 0 on success, otherwise error code
 */
int spl_mmc_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 const char *filename,
		 int raw_part,
		 unsigned long raw_sect);

/**
 * spl_usb_load() - Load an image file from USB mass storage
 *
 * @param spl_image	Image data filled in by loading process
 * @param bootdev	Describes which device to load from
 * @param raw_part	Fat partition to load from
 * @param filename	Name of file to load
 *
 * Return: 0 on success, otherwise error code
 */
int spl_usb_load(struct spl_image_info *spl_image,
		 struct spl_boot_device *bootdev,
		 int partition, const char *filename);

int spl_ymodem_load_image(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev);
/**
 * spl_reserve_video_from_ram_top() - Reserve framebuffer memory from end of RAM
 *
 * This enforces framebuffer reservation at SPL stage from end of RAM so that
 * next stage can directly skip this pre-reserved area before carrying out
 * further reservations. The allocation address is stored in struct video_uc_plat.
 *
 * Return: 0 on success, otherwise error code
 */
int spl_reserve_video_from_ram_top(void);

/**
 * spl_invoke_atf - boot using an ARM trusted firmware image
 */
void __noreturn spl_invoke_atf(struct spl_image_info *spl_image);

/**
 * bl2_plat_get_bl31_params() - return params for bl31.
 * @bl32_entry:	address of BL32 executable (secure)
 * @bl33_entry:	address of BL33 executable (non secure)
 * @fdt_addr:	address of Flat Device Tree
 *
 * This is a weak function which might be overridden by the board code. By
 * default it will just call bl2_plat_get_bl31_params_default().
 *
 * If you just want to manipulate or add some parameters, you can override
 * this function, call bl2_plat_get_bl31_params_default and operate on the
 * returned bl31 params.
 *
 * Return: bl31 params structure pointer
 */
struct bl31_params *bl2_plat_get_bl31_params(uintptr_t bl32_entry,
					     uintptr_t bl33_entry,
					     uintptr_t fdt_addr);

/**
 * bl2_plat_get_bl31_params_default() - prepare params for bl31.
 * @bl32_entry:	address of BL32 executable (secure)
 * @bl33_entry:	address of BL33 executable (non secure)
 * @fdt_addr:	address of Flat Device Tree
 *
 * This is the default implementation of bl2_plat_get_bl31_params(). It assigns
 * a pointer to the memory that the platform has kept aside to pass platform
 * specific and trusted firmware related information to BL31. This memory is
 * allocated by allocating memory to bl2_to_bl31_params_mem structure which is
 * a superset of all the structure whose information is passed to BL31
 *
 * NOTE: The memory is statically allocated, thus this function should be
 * called only once. All subsequent calls will overwrite any changes.
 *
 * Return: bl31 params structure pointer
 */
struct bl31_params *bl2_plat_get_bl31_params_default(uintptr_t bl32_entry,
						     uintptr_t bl33_entry,
						     uintptr_t fdt_addr);

/**
 * bl2_plat_get_bl31_params_v2() - return params for bl31
 * @bl32_entry:	address of BL32 executable (secure)
 * @bl33_entry:	address of BL33 executable (non secure)
 * @fdt_addr:	address of Flat Device Tree
 *
 * This function does the same as bl2_plat_get_bl31_params() except that is is
 * used for the new LOAD_IMAGE_V2 option, which uses a slightly different
 * method to pass the parameters.
 *
 * Return: bl31 params structure pointer
 */
struct bl_params *bl2_plat_get_bl31_params_v2(uintptr_t bl32_entry,
					      uintptr_t bl33_entry,
					      uintptr_t fdt_addr);

/**
 * bl2_plat_get_bl31_params_v2_default() - prepare params for bl31.
 * @bl32_entry:	address of BL32 executable (secure)
 * @bl33_entry:	address of BL33 executable (non secure)
 * @fdt_addr:	address of Flat Device Tree
 *
 * This is the default implementation of bl2_plat_get_bl31_params_v2(). It
 * prepares the linked list of the bl31 params, populates the image types and
 * set the entry points for bl32 and bl33 (if available).
 *
 * NOTE: The memory is statically allocated, thus this function should be
 * called only once. All subsequent calls will overwrite any changes.
 *
 * Return: bl31 params structure pointer
 */
struct bl_params *bl2_plat_get_bl31_params_v2_default(uintptr_t bl32_entry,
						      uintptr_t bl33_entry,
						      uintptr_t fdt_addr);
/**
 * spl_optee_entry - entry function for optee
 *
 * args defind in op-tee project
 * https://github.com/OP-TEE/optee_os/
 * core/arch/arm/kernel/generic_entry_a32.S
 * @arg0: pagestore
 * @arg1: (ARMv7 standard bootarg #1)
 * @arg2: device tree address, (ARMv7 standard bootarg #2)
 * @arg3: non-secure entry address (ARMv7 bootarg #0)
 */
void __noreturn spl_optee_entry(void *arg0, void *arg1, void *arg2, void *arg3);

/**
 * spl_invoke_opensbi - boot using a RISC-V OpenSBI image
 */
void __noreturn spl_invoke_opensbi(struct spl_image_info *spl_image);

/**
 * board_return_to_bootrom - allow for boards to continue with the boot ROM
 *
 * If a board (e.g. the Rockchip RK3368 boards) provide some
 * supporting functionality for SPL in their boot ROM and the SPL
 * stage wants to return to the ROM code to continue booting, boards
 * can implement 'board_return_to_bootrom'.
 */
int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev);

/**
 * board_spl_fit_size_align - specific size align before processing payload
 *
 */
ulong board_spl_fit_size_align(ulong size);

/**
 * spl_perform_fixups() - arch/board-specific callback before processing
 *                        the boot-payload
 */
void spl_perform_fixups(struct spl_image_info *spl_image);

/*
 * spl_get_load_buffer() - get buffer for loading partial image data
 *
 * Returns memory area which can be populated by partial image data,
 * ie. uImage or fitImage header.
 */
struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size);

/**
 * board_spl_fit_append_fdt_skip(): test whether DTO application should be skipped
 * @name:	DTO node name within fitImage images node
 *
 * A board-specific function used to indicate whether a DTO from fitImage
 * configuration node 'fdt' property DT and DTO list should be applied onto
 * the base DT or not applied.
 *
 * This is useful in case of DTOs which implement e.g. different board revision
 * details, where such DTO should be applied on one board revision, and should
 * not be applied on another board revision.
 *
 * Return:	0 to indicate DTO is not skipped, all else to indicate DTO is skipped.
 */
int board_spl_fit_append_fdt_skip(const char *name);

void board_boot_order(u32 *spl_boot_list);
void spl_save_restore_data(void);

/**
 * spl_load_fit_image() - Fully parse and a FIT image in SPL
 *
 * @spl_image: SPL Image data to fill in
 * @header: Pointer to FIT image
 * Return 0 if OK, -ve on error
 */
int spl_load_fit_image(struct spl_image_info *spl_image,
		       const struct legacy_img_hdr *header);

/*
 * spl_decompression_enabled() - check decompression support is enabled for SPL build
 *
 * Returns  true  if decompression support is enabled, else False
 */
static inline bool spl_decompression_enabled(void)
{
	return IS_ENABLED(CONFIG_SPL_GZIP) || IS_ENABLED(CONFIG_SPL_LZMA);
}
#endif
