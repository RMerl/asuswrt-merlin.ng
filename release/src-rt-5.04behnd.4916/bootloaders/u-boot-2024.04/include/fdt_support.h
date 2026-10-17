/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 */

#ifndef __FDT_SUPPORT_H
#define __FDT_SUPPORT_H

#if !defined(USE_HOSTCC)

#include <asm/u-boot.h>
#include <linux/libfdt.h>
#include <abuf.h>

/**
 * arch_fixup_fdt() - Write arch-specific information to fdt
 *
 * Defined in arch/$(ARCH)/lib/bootm-fdt.c
 *
 * @blob:	FDT blob to write to
 * Return: 0 if ok, or -ve FDT_ERR_... on failure
 */
int arch_fixup_fdt(void *blob);

void ft_cpu_setup(void *blob, struct bd_info *bd);

void ft_pci_setup(void *blob, struct bd_info *bd);

u32 fdt_getprop_u32_default_node(const void *fdt, int off, int cell,
				const char *prop, const u32 dflt);
u32 fdt_getprop_u32_default(const void *fdt, const char *path,
				const char *prop, const u32 dflt);

/**
 * Add data to the root of the FDT before booting the OS.
 *
 * See doc/device-tree-bindings/root.txt
 *
 * @param fdt		FDT address in memory
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int fdt_root(void *fdt);

/**
 * Add chosen data the FDT before booting the OS.
 *
 * In particular, this adds the kernel command line (bootargs) to the FDT.
 *
 * @param fdt		FDT address in memory
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int fdt_chosen(void *fdt);

/**
 * Add initrd information to the FDT before booting the OS.
 *
 * Adds linux,initrd-start and linux,initrd-end properties to the /chosen node,
 * creating it if necessary.
 *
 * A memory reservation for the ramdisk is added to the FDT, or an existing one
 * (with matching @initrd_start) updated.
 *
 * If @initrd_start == @initrd_end this function does nothing and returns 0.
 *
 * @fdt: Pointer to FDT in memory
 * @initrd_start: Start of ramdisk
 * @initrd_end: End of ramdisk
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end);

void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create);
void do_fixup_by_path_u32(void *fdt, const char *path, const char *prop,
			  u32 val, int create);

static inline void do_fixup_by_path_string(void *fdt, const char *path,
					   const char *prop, const char *status)
{
	do_fixup_by_path(fdt, path, prop, status, strlen(status) + 1, 1);
}

void do_fixup_by_prop(void *fdt,
		      const char *pname, const void *pval, int plen,
		      const char *prop, const void *val, int len,
		      int create);
void do_fixup_by_prop_u32(void *fdt,
			  const char *pname, const void *pval, int plen,
			  const char *prop, u32 val, int create);
void do_fixup_by_compat(void *fdt, const char *compat,
			const char *prop, const void *val, int len, int create);
void do_fixup_by_compat_u32(void *fdt, const char *compat,
			    const char *prop, u32 val, int create);
/**
 * Setup the memory node in the DT. Creates one if none was existing before.
 * Calls fdt_fixup_memory_banks() to populate a single reg pair covering the
 * whole memory.
 *
 * @param blob		FDT blob to update
 * @param start		Begin of DRAM mapping in physical memory
 * @param size		Size of the single memory bank
 * Return: 0 if ok, or -1 or -FDT_ERR_... on error
 */
int fdt_fixup_memory(void *blob, u64 start, u64 size);

/**
 * Fill the DT memory node with multiple memory banks.
 * Creates the node if none was existing before.
 * If banks is 0, it will not touch the existing reg property. This allows
 * boards to not mess with the existing DT setup, which may have been
 * filled in properly before.
 *
 * @param blob		FDT blob to update
 * @param start		Array of size <banks> to hold the start addresses.
 * @param size		Array of size <banks> to hold the size of each region.
 * @param banks		Number of memory banks to create. If 0, the reg
 *			property will be left untouched.
 * Return: 0 if ok, or -1 or -FDT_ERR_... on error
 */
#ifdef CONFIG_ARCH_FIXUP_FDT_MEMORY
int fdt_fixup_memory_banks(void *blob, u64 start[], u64 size[], int banks);
int fdt_set_usable_memory(void *blob, u64 start[], u64 size[], int banks);
#else
static inline int fdt_fixup_memory_banks(void *blob, u64 start[], u64 size[],
					 int banks)
{
	return 0;
}
#endif

void fdt_fixup_ethernet(void *fdt);
int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
			 const void *val, int len, int create);
void fdt_fixup_qe_firmware(void *fdt);

/**
 * Update native-mode property of display-timings node to the phandle
 * of the timings matching a display by name (case insensitive).
 *
 * see kernel Documentation/devicetree/bindings/video/display-timing.txt
 *
 * @param blob		FDT blob to update
 * @param path		path within dt
 * @param display	name of display timing to match
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int fdt_fixup_display(void *blob, const char *path, const char *display);

#if defined(CONFIG_USB_EHCI_FSL) || defined(CONFIG_USB_XHCI_FSL)
void fsl_fdt_fixup_dr_usb(void *blob, struct bd_info *bd);
#else
static inline void fsl_fdt_fixup_dr_usb(void *blob, struct bd_info *bd) {}
#endif /* defined(CONFIG_USB_EHCI_FSL) || defined(CONFIG_USB_XHCI_FSL) */

#if defined(CONFIG_SYS_FSL_SEC_COMPAT)
void fdt_fixup_crypto_node(void *blob, int sec_rev);
#else
static inline void fdt_fixup_crypto_node(void *blob, int sec_rev) {}
#endif

/**
 * Record information about a processed loadable in /fit-images (creating
 * /fit-images if necessary).
 *
 * @param blob		FDT blob to update
 * @param index	        index of this loadable
 * @param name          name of the loadable
 * @param load_addr     address the loadable was loaded to
 * @param size          number of bytes loaded
 * @param entry_point   entry point (if specified, otherwise pass -1)
 * @param type          type (if specified, otherwise pass NULL)
 * @param os            os-type (if specified, otherwise pass NULL)
 * @param arch		architecture (if specified, otherwise pass NULL)
 * Return: 0 if ok, or -1 or -FDT_ERR_... on error
 */
int fdt_record_loadable(void *blob, u32 index, const char *name,
			uintptr_t load_addr, u32 size, uintptr_t entry_point,
			const char *type, const char *os, const char *arch);

#ifdef CONFIG_PCI
#include <pci.h>
int fdt_pci_dma_ranges(void *blob, int phb_off, struct pci_controller *hose);
#endif

int fdt_find_or_add_subnode(void *fdt, int parentoffset, const char *name);

/**
 * fdt_copy_node - copy the entire node from one dtb to another
 * @src_fdt: pointer to the source device tree blob
 * @dst_fdt: pointer to the destination device tree blob
 * @node_full_path: full path of the node from the source dtb
 *
 * fdt_copy_node() copies the entire node including its subnodes from
 * one device tree blob to another. It overrides the existing property
 * if destination dtb already have such property. Otherwise, simply add
 * the new property. It does not touch any other properties in the
 * target subnodes.
 *
 * returns:
 *	0, on success
 *	-FDT_ERR_NOSPACE, no space in the destination dtb
 *	Other error code returned by fdt APIs
 */
int fdt_copy_node(void *src_fdt, void* dst_fdt, char *node_full_path);

/**
 * Add board-specific data to the FDT before booting the OS.
 *
 * Use CONFIG_SYS_FDT_PAD to ensure there is sufficient space.
 * This function is called if CONFIG_OF_BOARD_SETUP is defined
 *
 * @param blob		FDT blob to update
 * @param bd		Pointer to board data
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int ft_board_setup(void *blob, struct bd_info *bd);

/**
 * board_rng_seed() - Provide a seed to be passed via /chosen/rng-seed
 *
 * This function is called if CONFIG_BOARD_RNG_SEED is set, and must
 * be provided by the board. It should return, via @buf, some suitable
 * seed value to pass to the kernel.
 *
 * @param buf         A struct abuf for returning the seed and its size.
 * @return            0 if ok, negative on error.
 */
int board_rng_seed(struct abuf *buf);

/**
 * board_fdt_chosen_bootargs() - Arbitrarily amend fdt kernel command line
 *
 * This is used for late modification of kernel command line arguments just
 * before they are added into the /chosen node in flat device tree.
 *
 * @return: pointer to kernel command line arguments in memory
 */
char *board_fdt_chosen_bootargs(void);

/**
 * board_fdt_chosen_bootargs() - Arbitrarily amend fdt kernel command line
 *
 * This is used for late modification of kernel command line arguments just
 * before they are added into the /chosen node in flat device tree.
 *
 * @return: pointer to kernel command line arguments in memory
 */
char *board_fdt_chosen_bootargs(void);

/*
 * The keystone2 SOC requires all 32 bit aliased addresses to be converted
 * to their 36 physical format. This has to happen after all fdt nodes
 * are added or modified by the image_setup_libfdt(). The ft_board_setup_ex()
 * called at the end of the image_setup_libfdt() is to do that convertion.
 */
void ft_board_setup_ex(void *blob, struct bd_info *bd);

/**
 * Add system-specific data to the FDT before booting the OS.
 *
 * Use CONFIG_SYS_FDT_PAD to ensure there is sufficient space.
 * This function is called if CONFIG_OF_SYSTEM_SETUP is defined
 *
 * @param blob		FDT blob to update
 * @param bd		Pointer to board data
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int ft_system_setup(void *blob, struct bd_info *bd);

void set_working_fdt_addr(ulong addr);

/**
 * fdt_shrink_to_minimum() - shrink FDT while allowing for some margin
 *
 * Shrink down the given blob to 'minimum' size + some extrasize.
 *
 * The new size is enough to hold the existing contents plus @extrasize bytes,
 * plus 5 memory reservations. Also, the end of the FDT is aligned to a 4KB
 * boundary, so it might end up up to 4KB larger than needed.
 *
 * If there is an existing memory reservation for @blob in the FDT, it is
 * updated for the new size.
 *
 * @param blob		FDT blob to update
 * @param extrasize	additional bytes needed
 * Return: 0 if ok, or -FDT_ERR_... on error
 */
int fdt_shrink_to_minimum(void *blob, uint extrasize);

int fdt_increase_size(void *fdt, int add_len);

int fdt_delete_disabled_nodes(void *blob);

struct node_info;
#if defined(CONFIG_FDT_FIXUP_PARTITIONS)
void fdt_fixup_mtdparts(void *fdt, const struct node_info *node_info,
			int node_info_size);
#else
static inline void fdt_fixup_mtdparts(void *fdt,
				      const struct node_info *node_info,
				      int node_info_size)
{
}
#endif

/**
 * copy the fixed-partition nodes from U-Boot device tree to external blob
 *
 * @param blob		FDT blob to update
 * Return: 0 if ok, or non-zero on error
 */
int fdt_copy_fixed_partitions(void *blob);

void fdt_del_node_and_alias(void *blob, const char *alias);

/**
 * Translate an address from the DT into a CPU physical address
 *
 * The translation relies on the "ranges" property.
 *
 * @param blob		Pointer to device tree blob
 * @param node_offset	Node DT offset
 * @param in_addr	Pointer to the address to translate
 * Return: translated address or OF_BAD_ADDR on error
 */
u64 fdt_translate_address(const void *blob, int node_offset,
			  const __be32 *in_addr);
/**
 * Translate a DMA address from the DT into a CPU physical address
 *
 * The translation relies on the "dma-ranges" property.
 *
 * @param blob		Pointer to device tree blob
 * @param node_offset	Node DT offset
 * @param in_addr	Pointer to the DMA address to translate
 * Return: translated DMA address or OF_BAD_ADDR on error
 */
u64 fdt_translate_dma_address(const void *blob, int node_offset,
			      const __be32 *in_addr);

/**
 * Get DMA ranges for a specifc node, this is useful to perform bus->cpu and
 * cpu->bus address translations
 *
 * @param blob		Pointer to device tree blob
 * @param node_offset	Node DT offset
 * @param cpu		Pointer to variable storing the range's cpu address
 * @param bus		Pointer to variable storing the range's bus address
 * @param size		Pointer to variable storing the range's size
 * Return: translated DMA address or OF_BAD_ADDR on error
 */
int fdt_get_dma_range(const void *blob, int node_offset, phys_addr_t *cpu,
		      dma_addr_t *bus, u64 *size);

int fdt_node_offset_by_compat_reg(void *blob, const char *compat,
					phys_addr_t compat_off);
int fdt_node_offset_by_pathf(void *blob, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));

#define fdt_for_each_node_by_compatible(node, fdt, start, compat)	\
	for (node = fdt_node_offset_by_compatible(fdt, start, compat);	\
	     node >= 0;							\
	     node = fdt_node_offset_by_compatible(fdt, node, compat))

int fdt_set_phandle(void *fdt, int nodeoffset, uint32_t phandle);
unsigned int fdt_create_phandle(void *fdt, int nodeoffset);
unsigned int fdt_create_phandle_by_compatible(void *fdt, const char *compat);
unsigned int fdt_create_phandle_by_pathf(void *fdt, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
int fdt_add_edid(void *blob, const char *compat, unsigned char *buf);

int fdt_verify_alias_address(void *fdt, int anode, const char *alias,
			      u64 addr);
u64 fdt_get_base_address(const void *fdt, int node);
int fdt_read_range(void *fdt, int node, int n, uint64_t *child_addr,
		   uint64_t *addr, uint64_t *len);

enum fdt_status {
	FDT_STATUS_OKAY,
	FDT_STATUS_DISABLED,
	FDT_STATUS_FAIL,
};
int fdt_set_node_status(void *fdt, int nodeoffset, enum fdt_status status);
static inline int fdt_status_okay(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_OKAY);
}
static inline int fdt_status_disabled(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_DISABLED);
}
static inline int fdt_status_fail(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_FAIL);
}

int fdt_set_status_by_alias(void *fdt, const char *alias,
			    enum fdt_status status);
static inline int fdt_status_okay_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_OKAY);
}
static inline int fdt_status_disabled_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_DISABLED);
}
static inline int fdt_status_fail_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_FAIL);
}

int fdt_set_status_by_compatible(void *fdt, const char *compat,
				 enum fdt_status status);
static inline int fdt_status_okay_by_compatible(void *fdt, const char *compat)
{
	return fdt_set_status_by_compatible(fdt, compat, FDT_STATUS_OKAY);
}
static inline int fdt_status_disabled_by_compatible(void *fdt,
						    const char *compat)
{
	return fdt_set_status_by_compatible(fdt, compat, FDT_STATUS_DISABLED);
}
static inline int fdt_status_fail_by_compatible(void *fdt, const char *compat)
{
	return fdt_set_status_by_compatible(fdt, compat, FDT_STATUS_FAIL);
}

int fdt_set_status_by_pathf(void *fdt, enum fdt_status status, const char *fmt,
			    ...) __attribute__ ((format (printf, 3, 4)));
#define fdt_status_okay_by_pathf(fdt, fmt, ...) \
	fdt_set_status_by_pathf((fdt), FDT_STATUS_OKAY, (fmt), ##__VA_ARGS__)
#define fdt_status_disabled_by_pathf(fdt, fmt, ...) \
	fdt_set_status_by_pathf((fdt), FDT_STATUS_DISABLED, (fmt), ##__VA_ARGS__)
#define fdt_status_fail_by_pathf(fdt, fmt, ...) \
	fdt_set_status_by_pathf((fdt), FDT_STATUS_FAIL, (fmt), ##__VA_ARGS__)

/* Helper to read a big number; size is in cells (not bytes) */
static inline u64 fdt_read_number(const fdt32_t *cell, int size)
{
	u64 r = 0;
	while (size--)
		r = (r << 32) | fdt32_to_cpu(*(cell++));
	return r;
}

void fdt_support_default_count_cells(const void *blob, int parentoffset,
					int *addrc, int *sizec);
int ft_verify_fdt(void *fdt);
int arch_fixup_memory_node(void *blob);

int fdt_setup_simplefb_node(void *fdt, int node, u64 base_address, u32 width,
			    u32 height, u32 stride, const char *format);

int fdt_overlay_apply_verbose(void *fdt, void *fdto);

int fdt_valid(struct fdt_header **blobp);

/**
 * fdt_get_cells_len() - Get the length of a type of cell in top-level nodes
 *
 * Returns the length of the cell type in bytes (4 or 8).
 *
 * @blob: Pointer to device tree blob
 * @nr_cells_name: Name to lookup, e.g. "#address-cells"
 */
int fdt_get_cells_len(const void *blob, char *nr_cells_name);

#endif /* !USE_HOSTCC */

#ifdef USE_HOSTCC
int fdtdec_get_int(const void *blob, int node, const char *prop_name,
		int default_val);

/*
 * Count child nodes of one parent node.
 *
 * @param blob	FDT blob
 * @param node	parent node
 * Return: number of child node; 0 if there is not child node
 */
int fdtdec_get_child_count(const void *blob, int node);
#endif
#ifdef CONFIG_FMAN_ENET
int fdt_update_ethernet_dt(void *blob);
#endif
#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *blob);
#endif
#ifdef CONFIG_CMD_PSTORE
void fdt_fixup_pstore(void *blob);
#endif
#endif /* ifndef __FDT_SUPPORT_H */
