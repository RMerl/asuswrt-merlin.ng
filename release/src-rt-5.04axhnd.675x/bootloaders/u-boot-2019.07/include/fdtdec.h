/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __fdtdec_h
#define __fdtdec_h

/*
 * This file contains convenience functions for decoding useful and
 * enlightening information from FDTs. It is intended to be used by device
 * drivers and board-specific code within U-Boot. It aims to reduce the
 * amount of FDT munging required within U-Boot itself, so that driver code
 * changes to support FDT are minimized.
 */

#include <linux/libfdt.h>
#include <pci.h>

/*
 * A typedef for a physical address. Note that fdt data is always big
 * endian even on a litle endian machine.
 */
typedef phys_addr_t fdt_addr_t;
typedef phys_size_t fdt_size_t;

#ifdef CONFIG_PHYS_64BIT
#define FDT_ADDR_T_NONE (-1U)
#define fdt_addr_to_cpu(reg) be64_to_cpu(reg)
#define fdt_size_to_cpu(reg) be64_to_cpu(reg)
#define cpu_to_fdt_addr(reg) cpu_to_be64(reg)
#define cpu_to_fdt_size(reg) cpu_to_be64(reg)
typedef fdt64_t fdt_val_t;
#else
#define FDT_ADDR_T_NONE (-1U)
#define fdt_addr_to_cpu(reg) be32_to_cpu(reg)
#define fdt_size_to_cpu(reg) be32_to_cpu(reg)
#define cpu_to_fdt_addr(reg) cpu_to_be32(reg)
#define cpu_to_fdt_size(reg) cpu_to_be32(reg)
typedef fdt32_t fdt_val_t;
#endif

/* Information obtained about memory from the FDT */
struct fdt_memory {
	fdt_addr_t start;
	fdt_addr_t end;
};

struct bd_info;

#ifdef CONFIG_SPL_BUILD
#define SPL_BUILD	1
#else
#define SPL_BUILD	0
#endif

#if CONFIG_IS_ENABLED(OF_PRIOR_STAGE)
extern phys_addr_t prior_stage_fdt_address;
#endif

/*
 * Information about a resource. start is the first address of the resource
 * and end is the last address (inclusive). The length of the resource will
 * be equal to: end - start + 1.
 */
struct fdt_resource {
	fdt_addr_t start;
	fdt_addr_t end;
};

enum fdt_pci_space {
	FDT_PCI_SPACE_CONFIG = 0,
	FDT_PCI_SPACE_IO = 0x01000000,
	FDT_PCI_SPACE_MEM32 = 0x02000000,
	FDT_PCI_SPACE_MEM64 = 0x03000000,
	FDT_PCI_SPACE_MEM32_PREF = 0x42000000,
	FDT_PCI_SPACE_MEM64_PREF = 0x43000000,
};

#define FDT_PCI_ADDR_CELLS	3
#define FDT_PCI_SIZE_CELLS	2
#define FDT_PCI_REG_SIZE	\
	((FDT_PCI_ADDR_CELLS + FDT_PCI_SIZE_CELLS) * sizeof(u32))

/*
 * The Open Firmware spec defines PCI physical address as follows:
 *
 *          bits# 31 .... 24 23 .... 16 15 .... 08 07 .... 00
 *
 * phys.hi  cell:  npt000ss   bbbbbbbb   dddddfff   rrrrrrrr
 * phys.mid cell:  hhhhhhhh   hhhhhhhh   hhhhhhhh   hhhhhhhh
 * phys.lo  cell:  llllllll   llllllll   llllllll   llllllll
 *
 * where:
 *
 * n:        is 0 if the address is relocatable, 1 otherwise
 * p:        is 1 if addressable region is prefetchable, 0 otherwise
 * t:        is 1 if the address is aliased (for non-relocatable I/O) below 1MB
 *           (for Memory), or below 64KB (for relocatable I/O)
 * ss:       is the space code, denoting the address space
 * bbbbbbbb: is the 8-bit Bus Number
 * ddddd:    is the 5-bit Device Number
 * fff:      is the 3-bit Function Number
 * rrrrrrrr: is the 8-bit Register Number
 * hhhhhhhh: is a 32-bit unsigned number
 * llllllll: is a 32-bit unsigned number
 */
struct fdt_pci_addr {
	u32	phys_hi;
	u32	phys_mid;
	u32	phys_lo;
};

/**
 * Compute the size of a resource.
 *
 * @param res	the resource to operate on
 * @return the size of the resource
 */
static inline fdt_size_t fdt_resource_size(const struct fdt_resource *res)
{
	return res->end - res->start + 1;
}

/**
 * Compat types that we know about and for which we might have drivers.
 * Each is named COMPAT_<dir>_<filename> where <dir> is the directory
 * within drivers.
 */
enum fdt_compat_id {
	COMPAT_UNKNOWN,
	COMPAT_NVIDIA_TEGRA20_EMC,	/* Tegra20 memory controller */
	COMPAT_NVIDIA_TEGRA20_EMC_TABLE, /* Tegra20 memory timing table */
	COMPAT_NVIDIA_TEGRA20_NAND,	/* Tegra2 NAND controller */
	COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL,
					/* Tegra124 XUSB pad controller */
	COMPAT_NVIDIA_TEGRA210_XUSB_PADCTL,
					/* Tegra210 XUSB pad controller */
	COMPAT_SMSC_LAN9215,		/* SMSC 10/100 Ethernet LAN9215 */
	COMPAT_SAMSUNG_EXYNOS5_SROMC,	/* Exynos5 SROMC */
	COMPAT_SAMSUNG_EXYNOS_USB_PHY,	/* Exynos phy controller for usb2.0 */
	COMPAT_SAMSUNG_EXYNOS5_USB3_PHY,/* Exynos phy controller for usb3.0 */
	COMPAT_SAMSUNG_EXYNOS_TMU,	/* Exynos TMU */
	COMPAT_SAMSUNG_EXYNOS_MIPI_DSI,	/* Exynos mipi dsi */
	COMPAT_SAMSUNG_EXYNOS_DWMMC,	/* Exynos DWMMC controller */
	COMPAT_GENERIC_SPI_FLASH,	/* Generic SPI Flash chip */
	COMPAT_SAMSUNG_EXYNOS_SYSMMU,	/* Exynos sysmmu */
	COMPAT_INTEL_MICROCODE,		/* Intel microcode update */
	COMPAT_INTEL_QRK_MRC,		/* Intel Quark MRC */
	COMPAT_ALTERA_SOCFPGA_DWMAC,	/* SoCFPGA Ethernet controller */
	COMPAT_ALTERA_SOCFPGA_DWMMC,	/* SoCFPGA DWMMC controller */
	COMPAT_ALTERA_SOCFPGA_DWC2USB,	/* SoCFPGA DWC2 USB controller */
	COMPAT_INTEL_BAYTRAIL_FSP,	/* Intel Bay Trail FSP */
	COMPAT_INTEL_BAYTRAIL_FSP_MDP,	/* Intel FSP memory-down params */
	COMPAT_INTEL_IVYBRIDGE_FSP,	/* Intel Ivy Bridge FSP */
	COMPAT_SUNXI_NAND,		/* SUNXI NAND controller */
	COMPAT_ALTERA_SOCFPGA_CLK,	/* SoCFPGA Clock initialization */
	COMPAT_ALTERA_SOCFPGA_PINCTRL_SINGLE,	/* SoCFPGA pinctrl-single */
	COMPAT_ALTERA_SOCFPGA_H2F_BRG,          /* SoCFPGA hps2fpga bridge */
	COMPAT_ALTERA_SOCFPGA_LWH2F_BRG,        /* SoCFPGA lwhps2fpga bridge */
	COMPAT_ALTERA_SOCFPGA_F2H_BRG,          /* SoCFPGA fpga2hps bridge */
	COMPAT_ALTERA_SOCFPGA_F2SDR0,           /* SoCFPGA fpga2SDRAM0 bridge */
	COMPAT_ALTERA_SOCFPGA_F2SDR1,           /* SoCFPGA fpga2SDRAM1 bridge */
	COMPAT_ALTERA_SOCFPGA_F2SDR2,           /* SoCFPGA fpga2SDRAM2 bridge */
	COMPAT_ALTERA_SOCFPGA_FPGA0,		/* SOCFPGA FPGA manager */
	COMPAT_ALTERA_SOCFPGA_NOC,		/* SOCFPGA Arria 10 NOC */
	COMPAT_ALTERA_SOCFPGA_CLK_INIT,		/* SOCFPGA Arria 10 clk init */

	COMPAT_COUNT,
};

#define MAX_PHANDLE_ARGS 16
struct fdtdec_phandle_args {
	int node;
	int args_count;
	uint32_t args[MAX_PHANDLE_ARGS];
};

/**
 * fdtdec_parse_phandle_with_args() - Find a node pointed by phandle in a list
 *
 * This function is useful to parse lists of phandles and their arguments.
 *
 * Example:
 *
 * phandle1: node1 {
 *	#list-cells = <2>;
 * }
 *
 * phandle2: node2 {
 *	#list-cells = <1>;
 * }
 *
 * node3 {
 *	list = <&phandle1 1 2 &phandle2 3>;
 * }
 *
 * To get a device_node of the `node2' node you may call this:
 * fdtdec_parse_phandle_with_args(blob, node3, "list", "#list-cells", 0, 1,
 *				  &args);
 *
 * (This function is a modified version of __of_parse_phandle_with_args() from
 * Linux 3.18)
 *
 * @blob:	Pointer to device tree
 * @src_node:	Offset of device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies the phandles' arguments count,
 *		or NULL to use @cells_count
 * @cells_count: Cell count to use if @cells_name is NULL
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * @return 0 on success (with @out_args filled out if not NULL), -ENOENT if
 *	@list_name does not exist, a phandle was not found, @cells_name
 *	could not be found, the arguments were truncated or there were too
 *	many arguments.
 *
 */
int fdtdec_parse_phandle_with_args(const void *blob, int src_node,
				   const char *list_name,
				   const char *cells_name,
				   int cell_count, int index,
				   struct fdtdec_phandle_args *out_args);

/**
 * Find the next numbered alias for a peripheral. This is used to enumerate
 * all the peripherals of a certain type.
 *
 * Do the first call with *upto = 0. Assuming /aliases/<name>0 exists then
 * this function will return a pointer to the node the alias points to, and
 * then update *upto to 1. Next time you call this function, the next node
 * will be returned.
 *
 * All nodes returned will match the compatible ID, as it is assumed that
 * all peripherals use the same driver.
 *
 * @param blob		FDT blob to use
 * @param name		Root name of alias to search for
 * @param id		Compatible ID to look for
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto);

/**
 * Find the compatible ID for a given node.
 *
 * Generally each node has at least one compatible string attached to it.
 * This function looks through our list of known compatible strings and
 * returns the corresponding ID which matches the compatible string.
 *
 * @param blob		FDT blob to use
 * @param node		Node containing compatible string to find
 * @return compatible ID, or COMPAT_UNKNOWN if we cannot find a match
 */
enum fdt_compat_id fdtdec_lookup(const void *blob, int node);

/**
 * Find the next compatible node for a peripheral.
 *
 * Do the first call with node = 0. This function will return a pointer to
 * the next compatible node. Next time you call this function, pass the
 * value returned, and the next node will be provided.
 *
 * @param blob		FDT blob to use
 * @param node		Start node for search
 * @param id		Compatible ID to look for (enum fdt_compat_id)
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id);

/**
 * Find the next compatible subnode for a peripheral.
 *
 * Do the first call with node set to the parent and depth = 0. This
 * function will return the offset of the next compatible node. Next time
 * you call this function, pass the node value returned last time, with
 * depth unchanged, and the next node will be provided.
 *
 * @param blob		FDT blob to use
 * @param node		Start node for search
 * @param id		Compatible ID to look for (enum fdt_compat_id)
 * @param depthp	Current depth (set to 0 before first call)
 * @return offset of next compatible node, or -FDT_ERR_NOTFOUND if no more
 */
int fdtdec_next_compatible_subnode(const void *blob, int node,
		enum fdt_compat_id id, int *depthp);

/*
 * Look up an address property in a node and return the parsed address, and
 * optionally the parsed size.
 *
 * This variant assumes a known and fixed number of cells are used to
 * represent the address and size.
 *
 * You probably don't want to use this function directly except to parse
 * non-standard properties, and never to parse the "reg" property. Instead,
 * use one of the "auto" variants below, which automatically honor the
 * #address-cells and #size-cells properties in the parent node.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param index	which address to retrieve from a list of addresses. Often 0.
 * @param na	the number of cells used to represent an address
 * @param ns	the number of cells used to represent a size
 * @param sizep	a pointer to store the size into. Use NULL if not required
 * @param translate	Indicates whether to translate the returned value
 *			using the parent node's ranges property.
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr_size_fixed(const void *blob, int node,
		const char *prop_name, int index, int na, int ns,
		fdt_size_t *sizep, bool translate);

/*
 * Look up an address property in a node and return the parsed address, and
 * optionally the parsed size.
 *
 * This variant automatically determines the number of cells used to represent
 * the address and size by parsing the provided parent node's #address-cells
 * and #size-cells properties.
 *
 * @param blob	FDT blob
 * @param parent	parent node of @node
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param index	which address to retrieve from a list of addresses. Often 0.
 * @param sizep	a pointer to store the size into. Use NULL if not required
 * @param translate	Indicates whether to translate the returned value
 *			using the parent node's ranges property.
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr_size_auto_parent(const void *blob, int parent,
		int node, const char *prop_name, int index, fdt_size_t *sizep,
		bool translate);

/*
 * Look up an address property in a node and return the parsed address, and
 * optionally the parsed size.
 *
 * This variant automatically determines the number of cells used to represent
 * the address and size by parsing the parent node's #address-cells
 * and #size-cells properties. The parent node is automatically found.
 *
 * The automatic parent lookup implemented by this function is slow.
 * Consequently, fdtdec_get_addr_size_auto_parent() should be used where
 * possible.
 *
 * @param blob	FDT blob
 * @param parent	parent node of @node
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param index	which address to retrieve from a list of addresses. Often 0.
 * @param sizep	a pointer to store the size into. Use NULL if not required
 * @param translate	Indicates whether to translate the returned value
 *			using the parent node's ranges property.
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr_size_auto_noparent(const void *blob, int node,
		const char *prop_name, int index, fdt_size_t *sizep,
		bool translate);

/*
 * Look up an address property in a node and return the parsed address.
 *
 * This variant hard-codes the number of cells used to represent the address
 * and size based on sizeof(fdt_addr_t) and sizeof(fdt_size_t). It also
 * always returns the first address value in the property (index 0).
 *
 * Use of this function is not recommended due to the hard-coding of cell
 * counts. There is no programmatic validation that these hard-coded values
 * actually match the device tree content in any way at all. This assumption
 * can be satisfied by manually ensuring CONFIG_PHYS_64BIT is appropriately
 * set in the U-Boot build and exercising strict control over DT content to
 * ensure use of matching #address-cells/#size-cells properties. However, this
 * approach is error-prone; those familiar with DT will not expect the
 * assumption to exist, and could easily invalidate it. If the assumption is
 * invalidated, this function will not report the issue, and debugging will
 * be required. Instead, use fdtdec_get_addr_size_auto_parent().
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name);

/*
 * Look up an address property in a node and return the parsed address, and
 * optionally the parsed size.
 *
 * This variant hard-codes the number of cells used to represent the address
 * and size based on sizeof(fdt_addr_t) and sizeof(fdt_size_t). It also
 * always returns the first address value in the property (index 0).
 *
 * Use of this function is not recommended due to the hard-coding of cell
 * counts. There is no programmatic validation that these hard-coded values
 * actually match the device tree content in any way at all. This assumption
 * can be satisfied by manually ensuring CONFIG_PHYS_64BIT is appropriately
 * set in the U-Boot build and exercising strict control over DT content to
 * ensure use of matching #address-cells/#size-cells properties. However, this
 * approach is error-prone; those familiar with DT will not expect the
 * assumption to exist, and could easily invalidate it. If the assumption is
 * invalidated, this function will not report the issue, and debugging will
 * be required. Instead, use fdtdec_get_addr_size_auto_parent().
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param sizep	a pointer to store the size into. Use NULL if not required
 * @return address, if found, or FDT_ADDR_T_NONE if not
 */
fdt_addr_t fdtdec_get_addr_size(const void *blob, int node,
		const char *prop_name, fdt_size_t *sizep);

/**
 * Look at an address property in a node and return the pci address which
 * corresponds to the given type in the form of fdt_pci_addr.
 * The property must hold one fdt_pci_addr with a lengh.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param type		pci address type (FDT_PCI_SPACE_xxx)
 * @param prop_name	name of property to find
 * @param addr		returns pci address in the form of fdt_pci_addr
 * @return 0 if ok, -ENOENT if the property did not exist, -EINVAL if the
 *		format of the property was invalid, -ENXIO if the requested
 *		address type was not found
 */
int fdtdec_get_pci_addr(const void *blob, int node, enum fdt_pci_space type,
		const char *prop_name, struct fdt_pci_addr *addr);

/**
 * Look at the compatible property of a device node that represents a PCI
 * device and extract pci vendor id and device id from it.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param vendor	vendor id of the pci device
 * @param device	device id of the pci device
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_vendev(const void *blob, int node,
		u16 *vendor, u16 *device);

/**
 * Look at the pci address of a device node that represents a PCI device
 * and return base address of the pci device's registers.
 *
 * @param dev		device to examine
 * @param addr		pci address in the form of fdt_pci_addr
 * @param bar		returns base address of the pci device's registers
 * @return 0 if ok, negative on error
 */
int fdtdec_get_pci_bar32(struct udevice *dev, struct fdt_pci_addr *addr,
			 u32 *bar);

/**
 * Look up a 32-bit integer property in a node and return it. The property
 * must have at least 4 bytes of data. The value of the first cell is
 * returned.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
s32 fdtdec_get_int(const void *blob, int node, const char *prop_name,
		s32 default_val);

/**
 * Unsigned version of fdtdec_get_int. The property must have at least
 * 4 bytes of data. The value of the first cell is returned.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param default_val	default value to return if the property is not found
 * @return unsigned integer value, if found, or default_val if not
 */
unsigned int fdtdec_get_uint(const void *blob, int node, const char *prop_name,
			unsigned int default_val);

/**
 * Get a variable-sized number from a property
 *
 * This reads a number from one or more cells.
 *
 * @param ptr	Pointer to property
 * @param cells	Number of cells containing the number
 * @return the value in the cells
 */
u64 fdtdec_get_number(const fdt32_t *ptr, unsigned int cells);

/**
 * Look up a 64-bit integer property in a node and return it. The property
 * must have at least 8 bytes of data (2 cells). The first two cells are
 * concatenated to form a 8 bytes value, where the first cell is top half and
 * the second cell is bottom half.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
uint64_t fdtdec_get_uint64(const void *blob, int node, const char *prop_name,
		uint64_t default_val);

/**
 * Checks whether a node is enabled.
 * This looks for a 'status' property. If this exists, then returns 1 if
 * the status is 'ok' and 0 otherwise. If there is no status property,
 * it returns 1 on the assumption that anything mentioned should be enabled
 * by default.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @return integer value 0 (not enabled) or 1 (enabled)
 */
int fdtdec_get_is_enabled(const void *blob, int node);

/**
 * Make sure we have a valid fdt available to control U-Boot.
 *
 * If not, a message is printed to the console if the console is ready.
 *
 * @return 0 if all ok, -1 if not
 */
int fdtdec_prepare_fdt(void);

/**
 * Checks that we have a valid fdt available to control U-Boot.

 * However, if not then for the moment nothing is done, since this function
 * is called too early to panic().
 *
 * @returns 0
 */
int fdtdec_check_fdt(void);

/**
 * Find the nodes for a peripheral and return a list of them in the correct
 * order. This is used to enumerate all the peripherals of a certain type.
 *
 * To use this, optionally set up a /aliases node with alias properties for
 * a peripheral. For example, for usb you could have:
 *
 * aliases {
 *		usb0 = "/ehci@c5008000";
 *		usb1 = "/ehci@c5000000";
 * };
 *
 * Pass "usb" as the name to this function and will return a list of two
 * nodes offsets: /ehci@c5008000 and ehci@c5000000.
 *
 * All nodes returned will match the compatible ID, as it is assumed that
 * all peripherals use the same driver.
 *
 * If no alias node is found, then the node list will be returned in the
 * order found in the fdt. If the aliases mention a node which doesn't
 * exist, then this will be ignored. If nodes are found with no aliases,
 * they will be added in any order.
 *
 * If there is a gap in the aliases, then this function return a 0 node at
 * that position. The return value will also count these gaps.
 *
 * This function checks node properties and will not return nodes which are
 * marked disabled (status = "disabled").
 *
 * @param blob		FDT blob to use
 * @param name		Root name of alias to search for
 * @param id		Compatible ID to look for
 * @param node_list	Place to put list of found nodes
 * @param maxcount	Maximum number of nodes to find
 * @return number of nodes found on success, FDT_ERR_... on error
 */
int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount);

/*
 * This function is similar to fdtdec_find_aliases_for_id() except that it
 * adds to the node_list that is passed in. Any 0 elements are considered
 * available for allocation - others are considered already used and are
 * skipped.
 *
 * You can use this by calling fdtdec_find_aliases_for_id() with an
 * uninitialised array, then setting the elements that are returned to -1,
 * say, then calling this function, perhaps with a different compat id.
 * Any elements you get back that are >0 are new nodes added by the call
 * to this function.
 *
 * Note that if you have some nodes with aliases and some without, you are
 * sailing close to the wind. The call to fdtdec_find_aliases_for_id() with
 * one compat_id may fill in positions for which you have aliases defined
 * for another compat_id. When you later call *this* function with the second
 * compat_id, the alias positions may already be used. A debug warning may
 * be generated in this case, but it is safest to define aliases for all
 * nodes when you care about the ordering.
 */
int fdtdec_add_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount);

/**
 * Get the alias sequence number of a node
 *
 * This works out whether a node is pointed to by an alias, and if so, the
 * sequence number of that alias. Aliases are of the form <base><num> where
 * <num> is the sequence number. For example spi2 would be sequence number
 * 2.
 *
 * @param blob		Device tree blob (if NULL, then error is returned)
 * @param base		Base name for alias (before the underscore)
 * @param node		Node to look up
 * @param seqp		This is set to the sequence number if one is found,
 *			but otherwise the value is left alone
 * @return 0 if a sequence was found, -ve if not
 */
int fdtdec_get_alias_seq(const void *blob, const char *base, int node,
			 int *seqp);

/**
 * Get the highest alias number for susbystem.
 *
 * It parses all aliases and find out highest recorded alias for subsystem.
 * Aliases are of the form <base><num> where <num> is the sequence number.
 *
 * @param blob		Device tree blob (if NULL, then error is returned)
 * @param base		Base name for alias susbystem (before the number)
 *
 * @return 0 highest alias ID, -1 if not found
 */
int fdtdec_get_alias_highest_id(const void *blob, const char *base);

/**
 * Get a property from the /chosen node
 *
 * @param blob		Device tree blob (if NULL, then NULL is returned)
 * @param name		Property name to look up
 * @return Value of property, or NULL if it does not exist
 */
const char *fdtdec_get_chosen_prop(const void *blob, const char *name);

/**
 * Get the offset of the given /chosen node
 *
 * This looks up a property in /chosen containing the path to another node,
 * then finds the offset of that node.
 *
 * @param blob		Device tree blob (if NULL, then error is returned)
 * @param name		Property name, e.g. "stdout-path"
 * @return Node offset referred to by that chosen node, or -ve FDT_ERR_...
 */
int fdtdec_get_chosen_node(const void *blob, const char *name);

/*
 * Get the name for a compatible ID
 *
 * @param id		Compatible ID to look for
 * @return compatible string for that id
 */
const char *fdtdec_get_compatible(enum fdt_compat_id id);

/* Look up a phandle and follow it to its node. Then return the offset
 * of that node.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @return node offset if found, -ve error code on error
 */
int fdtdec_lookup_phandle(const void *blob, int node, const char *prop_name);

/**
 * Look up a property in a node and return its contents in an integer
 * array of given length. The property must have at least enough data for
 * the array (4*count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return 0 if ok, or -FDT_ERR_NOTFOUND if the property is not found,
 *		or -FDT_ERR_BADLAYOUT if not enough data
 */
int fdtdec_get_int_array(const void *blob, int node, const char *prop_name,
		u32 *array, int count);

/**
 * Look up a property in a node and return its contents in an integer
 * array of given length. The property must exist but may have less data that
 * expected (4*count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return number of array elements if ok, or -FDT_ERR_NOTFOUND if the
 *		property is not found
 */
int fdtdec_get_int_array_count(const void *blob, int node,
			       const char *prop_name, u32 *array, int count);

/**
 * Look up a property in a node and return a pointer to its contents as a
 * unsigned int array of given length. The property must have at least enough
 * data for the array ('count' cells). It may have more, but this will be
 * ignored. The data is not copied.
 *
 * Note that you must access elements of the array with fdt32_to_cpu(),
 * since the elements will be big endian even on a little endian machine.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param count		number of array elements
 * @return pointer to array if found, or NULL if the property is not
 *		found or there is not enough data
 */
const u32 *fdtdec_locate_array(const void *blob, int node,
			       const char *prop_name, int count);

/**
 * Look up a boolean property in a node and return it.
 *
 * A boolean properly is true if present in the device tree and false if not
 * present, regardless of its value.
 *
 * @param blob	FDT blob
 * @param node	node to examine
 * @param prop_name	name of property to find
 * @return 1 if the properly is present; 0 if it isn't present
 */
int fdtdec_get_bool(const void *blob, int node, const char *prop_name);

/*
 * Count child nodes of one parent node.
 *
 * @param blob	FDT blob
 * @param node	parent node
 * @return number of child node; 0 if there is not child node
 */
int fdtdec_get_child_count(const void *blob, int node);

/**
 * Look in the FDT for a config item with the given name and return its value
 * as a 32-bit integer. The property must have at least 4 bytes of data. The
 * value of the first cell is returned.
 *
 * @param blob		FDT blob to use
 * @param prop_name	Node property name
 * @param default_val	default value to return if the property is not found
 * @return integer value, if found, or default_val if not
 */
int fdtdec_get_config_int(const void *blob, const char *prop_name,
		int default_val);

/**
 * Look in the FDT for a config item with the given name
 * and return whether it exists.
 *
 * @param blob		FDT blob
 * @param prop_name	property name to look up
 * @return 1, if it exists, or 0 if not
 */
int fdtdec_get_config_bool(const void *blob, const char *prop_name);

/**
 * Look in the FDT for a config item with the given name and return its value
 * as a string.
 *
 * @param blob          FDT blob
 * @param prop_name     property name to look up
 * @returns property string, NULL on error.
 */
char *fdtdec_get_config_string(const void *blob, const char *prop_name);

/*
 * Look up a property in a node and return its contents in a byte
 * array of given length. The property must have at least enough data for
 * the array (count bytes). It may have more, but this will be ignored.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param array		array to fill with data
 * @param count		number of array elements
 * @return 0 if ok, or -FDT_ERR_MISSING if the property is not found,
 *		or -FDT_ERR_BADLAYOUT if not enough data
 */
int fdtdec_get_byte_array(const void *blob, int node, const char *prop_name,
		u8 *array, int count);

/**
 * Look up a property in a node and return a pointer to its contents as a
 * byte array of given length. The property must have at least enough data
 * for the array (count bytes). It may have more, but this will be ignored.
 * The data is not copied.
 *
 * @param blob		FDT blob
 * @param node		node to examine
 * @param prop_name	name of property to find
 * @param count		number of array elements
 * @return pointer to byte array if found, or NULL if the property is not
 *		found or there is not enough data
 */
const u8 *fdtdec_locate_byte_array(const void *blob, int node,
			     const char *prop_name, int count);

/**
 * Obtain an indexed resource from a device property.
 *
 * @param fdt		FDT blob
 * @param node		node to examine
 * @param property	name of the property to parse
 * @param index		index of the resource to retrieve
 * @param res		returns the resource
 * @return 0 if ok, negative on error
 */
int fdt_get_resource(const void *fdt, int node, const char *property,
		     unsigned int index, struct fdt_resource *res);

/**
 * Obtain a named resource from a device property.
 *
 * Look up the index of the name in a list of strings and return the resource
 * at that index.
 *
 * @param fdt		FDT blob
 * @param node		node to examine
 * @param property	name of the property to parse
 * @param prop_names	name of the property containing the list of names
 * @param name		the name of the entry to look up
 * @param res		returns the resource
 */
int fdt_get_named_resource(const void *fdt, int node, const char *property,
			   const char *prop_names, const char *name,
			   struct fdt_resource *res);

/* Display timings from linux include/video/display_timing.h */
enum display_flags {
	DISPLAY_FLAGS_HSYNC_LOW		= 1 << 0,
	DISPLAY_FLAGS_HSYNC_HIGH	= 1 << 1,
	DISPLAY_FLAGS_VSYNC_LOW		= 1 << 2,
	DISPLAY_FLAGS_VSYNC_HIGH	= 1 << 3,

	/* data enable flag */
	DISPLAY_FLAGS_DE_LOW		= 1 << 4,
	DISPLAY_FLAGS_DE_HIGH		= 1 << 5,
	/* drive data on pos. edge */
	DISPLAY_FLAGS_PIXDATA_POSEDGE	= 1 << 6,
	/* drive data on neg. edge */
	DISPLAY_FLAGS_PIXDATA_NEGEDGE	= 1 << 7,
	DISPLAY_FLAGS_INTERLACED	= 1 << 8,
	DISPLAY_FLAGS_DOUBLESCAN	= 1 << 9,
	DISPLAY_FLAGS_DOUBLECLK		= 1 << 10,
};

/*
 * A single signal can be specified via a range of minimal and maximal values
 * with a typical value, that lies somewhere inbetween.
 */
struct timing_entry {
	u32 min;
	u32 typ;
	u32 max;
};

/*
 * Single "mode" entry. This describes one set of signal timings a display can
 * have in one setting. This struct can later be converted to struct videomode
 * (see include/video/videomode.h). As each timing_entry can be defined as a
 * range, one struct display_timing may become multiple struct videomodes.
 *
 * Example: hsync active high, vsync active low
 *
 *				    Active Video
 * Video  ______________________XXXXXXXXXXXXXXXXXXXXXX_____________________
 *	  |<- sync ->|<- back ->|<----- active ----->|<- front ->|<- sync..
 *	  |	     |	 porch  |		     |	 porch	 |
 *
 * HSync _|¯¯¯¯¯¯¯¯¯¯|___________________________________________|¯¯¯¯¯¯¯¯¯
 *
 * VSync ¯|__________|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|_________
 */
struct display_timing {
	struct timing_entry pixelclock;

	struct timing_entry hactive;		/* hor. active video */
	struct timing_entry hfront_porch;	/* hor. front porch */
	struct timing_entry hback_porch;	/* hor. back porch */
	struct timing_entry hsync_len;		/* hor. sync len */

	struct timing_entry vactive;		/* ver. active video */
	struct timing_entry vfront_porch;	/* ver. front porch */
	struct timing_entry vback_porch;	/* ver. back porch */
	struct timing_entry vsync_len;		/* ver. sync len */

	enum display_flags flags;		/* display flags */
	bool hdmi_monitor;			/* is hdmi monitor? */
};

/**
 * fdtdec_decode_display_timing() - decode display timings
 *
 * Decode display timings from the supplied 'display-timings' node.
 * See doc/device-tree-bindings/video/display-timing.txt for binding
 * information.
 *
 * @param blob		FDT blob
 * @param node		'display-timing' node containing the timing subnodes
 * @param index		Index number to read (0=first timing subnode)
 * @param config	Place to put timings
 * @return 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int fdtdec_decode_display_timing(const void *blob, int node, int index,
				 struct display_timing *config);

/**
 * fdtdec_setup_mem_size_base_fdt() - decode and setup gd->ram_size and
 * gd->ram_start
 *
 * Decode the /memory 'reg' property to determine the size and start of the
 * first memory bank, populate the global data with the size and start of the
 * first bank of memory.
 *
 * This function should be called from a boards dram_init(). This helper
 * function allows for boards to query the device tree for DRAM size and start
 * address instead of hard coding the value in the case where the memory size
 * and start address cannot be detected automatically.
 *
 * @param blob		FDT blob
 *
 * @return 0 if OK, -EINVAL if the /memory node or reg property is missing or
 * invalid
 */
int fdtdec_setup_mem_size_base_fdt(const void *blob);

/**
 * fdtdec_setup_mem_size_base() - decode and setup gd->ram_size and
 * gd->ram_start
 *
 * Decode the /memory 'reg' property to determine the size and start of the
 * first memory bank, populate the global data with the size and start of the
 * first bank of memory.
 *
 * This function should be called from a boards dram_init(). This helper
 * function allows for boards to query the device tree for DRAM size and start
 * address instead of hard coding the value in the case where the memory size
 * and start address cannot be detected automatically.
 *
 * @return 0 if OK, -EINVAL if the /memory node or reg property is missing or
 * invalid
 */
int fdtdec_setup_mem_size_base(void);

/**
 * fdtdec_setup_memory_banksize_fdt() - decode and populate gd->bd->bi_dram
 *
 * Decode the /memory 'reg' property to determine the address and size of the
 * memory banks. Use this data to populate the global data board info with the
 * phys address and size of memory banks.
 *
 * This function should be called from a boards dram_init_banksize(). This
 * helper function allows for boards to query the device tree for memory bank
 * information instead of hard coding the information in cases where it cannot
 * be detected automatically.
 *
 * @param blob		FDT blob
 *
 * @return 0 if OK, -EINVAL if the /memory node or reg property is missing or
 * invalid
 */
int fdtdec_setup_memory_banksize_fdt(const void *blob);

/**
 * fdtdec_setup_memory_banksize() - decode and populate gd->bd->bi_dram
 *
 * Decode the /memory 'reg' property to determine the address and size of the
 * memory banks. Use this data to populate the global data board info with the
 * phys address and size of memory banks.
 *
 * This function should be called from a boards dram_init_banksize(). This
 * helper function allows for boards to query the device tree for memory bank
 * information instead of hard coding the information in cases where it cannot
 * be detected automatically.
 *
 * @return 0 if OK, -EINVAL if the /memory node or reg property is missing or
 * invalid
 */
int fdtdec_setup_memory_banksize(void);

/**
 * fdtdec_set_ethernet_mac_address() - set MAC address for default interface
 *
 * Looks up the default interface via the "ethernet" alias (in the /aliases
 * node) and stores the given MAC in its "local-mac-address" property. This
 * is useful on platforms that store the MAC address in a custom location.
 * Board code can call this in the late init stage to make sure that the
 * interface device tree node has the right MAC address configured for the
 * Ethernet uclass to pick it up.
 *
 * Typically the FDT passed into this function will be U-Boot's control DTB.
 * Given that a lot of code may be holding offsets to various nodes in that
 * tree, this code will only set the "local-mac-address" property in-place,
 * which means that it needs to exist and have space for the 6-byte address.
 * This ensures that the operation is non-destructive and does not invalidate
 * offsets that other drivers may be using.
 *
 * @param fdt FDT blob
 * @param mac buffer containing the MAC address to set
 * @param size size of MAC address
 * @return 0 on success or a negative error code on failure
 */
int fdtdec_set_ethernet_mac_address(void *fdt, const u8 *mac, size_t size);

/**
 * fdtdec_set_phandle() - sets the phandle of a given node
 *
 * @param blob		FDT blob
 * @param node		offset in the FDT blob of the node whose phandle is to
 *			be set
 * @param phandle	phandle to set for the given node
 * @return 0 on success or a negative error code on failure
 */
static inline int fdtdec_set_phandle(void *blob, int node, uint32_t phandle)
{
	return fdt_setprop_u32(blob, node, "phandle", phandle);
}

/**
 * fdtdec_add_reserved_memory() - add or find a reserved-memory node
 *
 * If a reserved-memory node already exists for the given carveout, a phandle
 * for that node will be returned. Otherwise a new node will be created and a
 * phandle corresponding to it will be returned.
 *
 * See Documentation/devicetree/bindings/reserved-memory/reserved-memory.txt
 * for details on how to use reserved memory regions.
 *
 * As an example, consider the following code snippet:
 *
 *     struct fdt_memory fb = {
 *         .start = 0x92cb3000,
 *         .end = 0x934b2fff,
 *     };
 *     uint32_t phandle;
 *
 *     fdtdec_add_reserved_memory(fdt, "framebuffer", &fb, &phandle);
 *
 * This results in the following subnode being added to the top-level
 * /reserved-memory node:
 *
 *     reserved-memory {
 *         #address-cells = <0x00000002>;
 *         #size-cells = <0x00000002>;
 *         ranges;
 *
 *         framebuffer@92cb3000 {
 *             reg = <0x00000000 0x92cb3000 0x00000000 0x00800000>;
 *             phandle = <0x0000004d>;
 *         };
 *     };
 *
 * If the top-level /reserved-memory node does not exist, it will be created.
 * The phandle returned from the function call can be used to reference this
 * reserved memory region from other nodes.
 *
 * See fdtdec_set_carveout() for a more elaborate example.
 *
 * @param blob		FDT blob
 * @param basename	base name of the node to create
 * @param carveout	information about the carveout region
 * @param phandlep	return location for the phandle of the carveout region
 * @return 0 on success or a negative error code on failure
 */
int fdtdec_add_reserved_memory(void *blob, const char *basename,
			       const struct fdt_memory *carveout,
			       uint32_t *phandlep);

/**
 * fdtdec_get_carveout() - reads a carveout from an FDT
 *
 * Reads information about a carveout region from an FDT. The carveout is a
 * referenced by its phandle that is read from a given property in a given
 * node.
 *
 * @param blob		FDT blob
 * @param node		name of a node
 * @param name		name of the property in the given node that contains
 *			the phandle for the carveout
 * @param index		index of the phandle for which to read the carveout
 * @param carveout	return location for the carveout information
 * @return 0 on success or a negative error code on failure
 */
int fdtdec_get_carveout(const void *blob, const char *node, const char *name,
			unsigned int index, struct fdt_memory *carveout);

/**
 * fdtdec_set_carveout() - sets a carveout region for a given node
 *
 * Sets a carveout region for a given node. If a reserved-memory node already
 * exists for the carveout, the phandle for that node will be reused. If no
 * such node exists, a new one will be created and a phandle to it stored in
 * a specified property of the given node.
 *
 * As an example, consider the following code snippet:
 *
 *     const char *node = "/host1x@50000000/dc@54240000";
 *     struct fdt_memory fb = {
 *         .start = 0x92cb3000,
 *         .end = 0x934b2fff,
 *     };
 *
 *     fdtdec_set_carveout(fdt, node, "memory-region", 0, "framebuffer", &fb);
 *
 * dc@54200000 is a display controller and was set up by the bootloader to
 * scan out the framebuffer specified by "fb". This would cause the following
 * reserved memory region to be added:
 *
 *     reserved-memory {
 *         #address-cells = <0x00000002>;
 *         #size-cells = <0x00000002>;
 *         ranges;
 *
 *         framebuffer@92cb3000 {
 *             reg = <0x00000000 0x92cb3000 0x00000000 0x00800000>;
 *             phandle = <0x0000004d>;
 *         };
 *     };
 *
 * A "memory-region" property will also be added to the node referenced by the
 * offset parameter.
 *
 *     host1x@50000000 {
 *         ...
 *
 *         dc@54240000 {
 *             ...
 *             memory-region = <0x0000004d>;
 *             ...
 *         };
 *
 *         ...
 *     };
 *
 * @param blob		FDT blob
 * @param node		name of the node to add the carveout to
 * @param prop_name	name of the property in which to store the phandle of
 *			the carveout
 * @param index		index of the phandle to store
 * @param name		base name of the reserved-memory node to create
 * @param carveout	information about the carveout to add
 * @return 0 on success or a negative error code on failure
 */
int fdtdec_set_carveout(void *blob, const char *node, const char *prop_name,
			unsigned int index, const char *name,
			const struct fdt_memory *carveout);

/**
 * Set up the device tree ready for use
 */
int fdtdec_setup(void);

#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
/**
 * fdtdec_resetup()  - Set up the device tree again
 *
 * The main difference with fdtdec_setup() is that it returns if the fdt has
 * changed because a better match has been found.
 * This is typically used for boards that rely on a DM driver to detect the
 * board type. This function sould be called by the board code after the stuff
 * needed by board_fit_config_name_match() to operate porperly is available.
 * If this functions signals that a rescan is necessary, the board code must
 * unbind all the drivers using dm_uninit() and then rescan the DT with
 * dm_init_and_scan().
 *
 * @param rescan Returns a flag indicating that fdt has changed and rescanning
 *               the fdt is required
 *
 * @return 0 if OK, -ve on error
 */
int fdtdec_resetup(int *rescan);
#endif

/**
 * Board-specific FDT initialization. Returns the address to a device tree blob.
 * Called when CONFIG_OF_BOARD is defined, or if CONFIG_OF_SEPARATE is defined
 * and the board implements it.
 */
void *board_fdt_blob_setup(void);

/*
 * Decode the size of memory
 *
 * RAM size is normally set in a /memory node and consists of a list of
 * (base, size) cells in the 'reg' property. This information is used to
 * determine the total available memory as well as the address and size
 * of each bank.
 *
 * Optionally the memory configuration can vary depending on a board id,
 * typically read from strapping resistors or an EEPROM on the board.
 *
 * Finally, memory size can be detected (within certain limits) by probing
 * the available memory. It is safe to do so within the limits provides by
 * the board's device tree information. This makes it possible to produce
 * boards with different memory sizes, where the device tree specifies the
 * maximum memory configuration, and the smaller memory configuration is
 * probed.
 *
 * This function decodes that information, returning the memory base address,
 * size and bank information. See the memory.txt binding for full
 * documentation.
 *
 * @param blob		Device tree blob
 * @param area		Name of node to check (NULL means "/memory")
 * @param board_id	Board ID to look up
 * @param basep		Returns base address of first memory bank (NULL to
 *			ignore)
 * @param sizep		Returns total memory size (NULL to ignore)
 * @param bd		Updated with the memory bank information (NULL to skip)
 * @return 0 if OK, -ve on error
 */
int fdtdec_decode_ram_size(const void *blob, const char *area, int board_id,
			   phys_addr_t *basep, phys_size_t *sizep,
			   struct bd_info *bd);

#endif
