/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_PCIE_H__
#define __CVMX_PCIE_H__

#define CVMX_PCIE_MAX_PORTS 4
#define CVMX_PCIE_PORTS                                                                            \
	((OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN73XX)) ?                      \
		       CVMX_PCIE_MAX_PORTS :                                                             \
		       (OCTEON_IS_MODEL(OCTEON_CN70XX) ? 3 : 2))

/*
 * The physical memory base mapped by BAR1.  256MB at the end of the
 * first 4GB.
 */
#define CVMX_PCIE_BAR1_PHYS_BASE ((1ull << 32) - (1ull << 28))
#define CVMX_PCIE_BAR1_PHYS_SIZE BIT_ULL(28)

/*
 * The RC base of BAR1.  gen1 has a 39-bit BAR2, gen2 has 41-bit BAR2,
 * place BAR1 so it is the same for both.
 */
#define CVMX_PCIE_BAR1_RC_BASE BIT_ULL(41)

typedef union {
	u64 u64;
	struct {
		u64 upper : 2;		 /* Normally 2 for XKPHYS */
		u64 reserved_49_61 : 13; /* Must be zero */
		u64 io : 1;		 /* 1 for IO space access */
		u64 did : 5;		 /* PCIe DID = 3 */
		u64 subdid : 3;		 /* PCIe SubDID = 1 */
		u64 reserved_38_39 : 2;	 /* Must be zero */
		u64 node : 2;		 /* Numa node number */
		u64 es : 2;		 /* Endian swap = 1 */
		u64 port : 2;		 /* PCIe port 0,1 */
		u64 reserved_29_31 : 3;	 /* Must be zero */
		u64 ty : 1;
		u64 bus : 8;
		u64 dev : 5;
		u64 func : 3;
		u64 reg : 12;
	} config;
	struct {
		u64 upper : 2;		 /* Normally 2 for XKPHYS */
		u64 reserved_49_61 : 13; /* Must be zero */
		u64 io : 1;		 /* 1 for IO space access */
		u64 did : 5;		 /* PCIe DID = 3 */
		u64 subdid : 3;		 /* PCIe SubDID = 2 */
		u64 reserved_38_39 : 2;	 /* Must be zero */
		u64 node : 2;		 /* Numa node number */
		u64 es : 2;		 /* Endian swap = 1 */
		u64 port : 2;		 /* PCIe port 0,1 */
		u64 address : 32;	 /* PCIe IO address */
	} io;
	struct {
		u64 upper : 2;		 /* Normally 2 for XKPHYS */
		u64 reserved_49_61 : 13; /* Must be zero */
		u64 io : 1;		 /* 1 for IO space access */
		u64 did : 5;		 /* PCIe DID = 3 */
		u64 subdid : 3;		 /* PCIe SubDID = 3-6 */
		u64 reserved_38_39 : 2;	 /* Must be zero */
		u64 node : 2;		 /* Numa node number */
		u64 address : 36;	 /* PCIe Mem address */
	} mem;
} cvmx_pcie_address_t;

/**
 * Return the Core virtual base address for PCIe IO access. IOs are
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: 64bit Octeon IO base address for read/write
 */
u64 cvmx_pcie_get_io_base_address(int pcie_port);

/**
 * Size of the IO address region returned at address
 * cvmx_pcie_get_io_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: Size of the IO window
 */
u64 cvmx_pcie_get_io_size(int pcie_port);

/**
 * Return the Core virtual base address for PCIe MEM access. Memory is
 * read/written as an offset from this address.
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: 64bit Octeon IO base address for read/write
 */
u64 cvmx_pcie_get_mem_base_address(int pcie_port);

/**
 * Size of the Mem address region returned at address
 * cvmx_pcie_get_mem_base_address()
 *
 * @param pcie_port PCIe port the IO is for
 *
 * Return: Size of the Mem window
 */
u64 cvmx_pcie_get_mem_size(int pcie_port);

/**
 * Initialize a PCIe port for use in host(RC) mode. It doesn't enumerate the bus.
 *
 * @param pcie_port PCIe port to initialize
 *
 * Return: Zero on success
 */
int cvmx_pcie_rc_initialize(int pcie_port);

/**
 * Shutdown a PCIe port and put it in reset
 *
 * @param pcie_port PCIe port to shutdown
 *
 * Return: Zero on success
 */
int cvmx_pcie_rc_shutdown(int pcie_port);

/**
 * Read 8bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
u8 cvmx_pcie_config_read8(int pcie_port, int bus, int dev, int fn, int reg);

/**
 * Read 16bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
u16 cvmx_pcie_config_read16(int pcie_port, int bus, int dev, int fn, int reg);

/**
 * Read 32bits from a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 *
 * Return: Result of the read
 */
u32 cvmx_pcie_config_read32(int pcie_port, int bus, int dev, int fn, int reg);

/**
 * Write 8bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write8(int pcie_port, int bus, int dev, int fn, int reg, u8 val);

/**
 * Write 16bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write16(int pcie_port, int bus, int dev, int fn, int reg, u16 val);

/**
 * Write 32bits to a Device's config space
 *
 * @param pcie_port PCIe port the device is on
 * @param bus       Sub bus
 * @param dev       Device ID
 * @param fn        Device sub function
 * @param reg       Register to access
 * @param val       Value to write
 */
void cvmx_pcie_config_write32(int pcie_port, int bus, int dev, int fn, int reg, u32 val);

/**
 * Read a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to read from
 * @param cfg_offset Address to read
 *
 * Return: Value read
 */
u32 cvmx_pcie_cfgx_read(int pcie_port, u32 cfg_offset);
u32 cvmx_pcie_cfgx_read_node(int node, int pcie_port, u32 cfg_offset);

/**
 * Write a PCIe config space register indirectly. This is used for
 * registers of the form PCIEEP_CFG??? and PCIERC?_CFG???.
 *
 * @param pcie_port  PCIe port to write to
 * @param cfg_offset Address to write
 * @param val        Value to write
 */
void cvmx_pcie_cfgx_write(int pcie_port, u32 cfg_offset, u32 val);
void cvmx_pcie_cfgx_write_node(int node, int pcie_port, u32 cfg_offset, u32 val);

/**
 * Write a 32bit value to the Octeon NPEI register space
 *
 * @param address Address to write to
 * @param val     Value to write
 */
static inline void cvmx_pcie_npei_write32(u64 address, u32 val)
{
	cvmx_write64_uint32(address ^ 4, val);
	cvmx_read64_uint32(address ^ 4);
}

/**
 * Read a 32bit value from the Octeon NPEI register space
 *
 * @param address Address to read
 * Return: The result
 */
static inline u32 cvmx_pcie_npei_read32(u64 address)
{
	return cvmx_read64_uint32(address ^ 4);
}

/**
 * Initialize a PCIe port for use in target(EP) mode.
 *
 * @param pcie_port PCIe port to initialize
 *
 * Return: Zero on success
 */
int cvmx_pcie_ep_initialize(int pcie_port);

/**
 * Wait for posted PCIe read/writes to reach the other side of
 * the internal PCIe switch. This will insure that core
 * read/writes are posted before anything after this function
 * is called. This may be necessary when writing to memory that
 * will later be read using the DMA/PKT engines.
 *
 * @param pcie_port PCIe port to wait for
 */
void cvmx_pcie_wait_for_pending(int pcie_port);

/**
 * Returns if a PCIe port is in host or target mode.
 *
 * @param pcie_port PCIe port number (PEM number)
 *
 * Return: 0 if PCIe port is in target mode, !0 if in host mode.
 */
int cvmx_pcie_is_host_mode(int pcie_port);

#endif
