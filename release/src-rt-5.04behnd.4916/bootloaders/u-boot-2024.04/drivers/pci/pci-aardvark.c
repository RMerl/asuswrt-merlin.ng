/*
 * ***************************************************************************
 * Copyright (C) 2015 Marvell International Ltd.
 * ***************************************************************************
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************
 */
/* pcie_advk.c
 *
 * Ported from Linux driver - driver/pci/host/pci-aardvark.c
 *
 * Author: Victor Gu <xigu@marvell.com>
 *         Hezi Shahmoon <hezi.shahmoon@marvell.com>
 *         Pali Rohár <pali@kernel.org>
 *
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ioport.h>

/* PCIe Root Port register offsets */
#define ADVK_ROOT_PORT_PCI_CFG_OFF		0x0
#define ADVK_ROOT_PORT_PCI_EXP_OFF		0xc0
#define ADVK_ROOT_PORT_PCI_ERR_OFF		0x100

/* PIO registers */
#define ADVK_PIO_BASE_ADDR			0x4000
#define ADVK_PIO_CTRL				(ADVK_PIO_BASE_ADDR + 0x0)
#define   ADVK_PIO_CTRL_TYPE_MASK		GENMASK(3, 0)
#define   ADVK_PIO_CTRL_TYPE_SHIFT		0
#define   ADVK_PIO_CTRL_TYPE_RD_TYPE0		0x8
#define   ADVK_PIO_CTRL_TYPE_RD_TYPE1		0x9
#define   ADVK_PIO_CTRL_TYPE_WR_TYPE0		0xa
#define   ADVK_PIO_CTRL_TYPE_WR_TYPE1		0xb
#define   ADVK_PIO_CTRL_ADDR_WIN_DISABLE	BIT(24)
#define ADVK_PIO_STAT				(ADVK_PIO_BASE_ADDR + 0x4)
#define   ADVK_PIO_COMPLETION_STATUS_MASK	GENMASK(9, 7)
#define   ADVK_PIO_COMPLETION_STATUS_SHIFT	7
#define   ADVK_PIO_COMPLETION_STATUS_OK		0
#define   ADVK_PIO_COMPLETION_STATUS_UR		1
#define   ADVK_PIO_COMPLETION_STATUS_CRS	2
#define   ADVK_PIO_COMPLETION_STATUS_CA		4
#define   ADVK_PIO_NON_POSTED_REQ		BIT(10)
#define   ADVK_PIO_ERR_STATUS			BIT(11)
#define ADVK_PIO_ADDR_LS			(ADVK_PIO_BASE_ADDR + 0x8)
#define ADVK_PIO_ADDR_MS			(ADVK_PIO_BASE_ADDR + 0xc)
#define ADVK_PIO_WR_DATA			(ADVK_PIO_BASE_ADDR + 0x10)
#define ADVK_PIO_WR_DATA_STRB			(ADVK_PIO_BASE_ADDR + 0x14)
#define ADVK_PIO_RD_DATA			(ADVK_PIO_BASE_ADDR + 0x18)
#define ADVK_PIO_START				(ADVK_PIO_BASE_ADDR + 0x1c)
#define ADVK_PIO_ISR				(ADVK_PIO_BASE_ADDR + 0x20)

/* Global Control registers */
#define ADVK_GLOBAL_CTRL_BASE_ADDR		0x4800
#define ADVK_GLOBAL_CTRL0			(ADVK_GLOBAL_CTRL_BASE_ADDR + 0x0)
#define     ADVK_GLOBAL_CTRL0_SPEED_GEN_MASK	GENMASK(1, 0)
#define     ADVK_GLOBAL_CTRL0_SPEED_GEN_SHIFT	0
#define     ADVK_GLOBAL_CTRL0_SPEED_GEN_1	0
#define     ADVK_GLOBAL_CTRL0_SPEED_GEN_2	1
#define     ADVK_GLOBAL_CTRL0_SPEED_GEN_3	2
#define     ADVK_GLOBAL_CTRL0_IS_RC		BIT(2)
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_MASK	GENMASK(4, 3)
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_SHIFT	3
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_1	0
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_2	1
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_4	2
#define     ADVK_GLOBAL_CTRL0_LANE_COUNT_8	3
#define     ADVK_GLOBAL_CTRL0_LINK_TRAINING_EN	BIT(6)
#define ADVK_GLOBAL_CTRL2			(ADVK_GLOBAL_CTRL_BASE_ADDR + 0x8)
#define     ADVK_GLOBAL_CTRL2_STRICT_ORDER_EN	BIT(5)
#define     ADVK_GLOBAL_CTRL2_ADDRWIN_MAP_EN	BIT(6)

/* PCIe window configuration registers */
#define ADVK_OB_WIN_BASE_ADDR			0x4c00
#define ADVK_OB_WIN_BLOCK_SIZE			0x20
#define ADVK_OB_WIN_COUNT			8
#define ADVK_OB_WIN_REG_ADDR(win, offset)	(ADVK_OB_WIN_BASE_ADDR + ADVK_OB_WIN_BLOCK_SIZE * (win) + (offset))
#define ADVK_OB_WIN_MATCH_LS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x00)
#define     ADVK_OB_WIN_ENABLE			BIT(0)
#define ADVK_OB_WIN_MATCH_MS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x04)
#define ADVK_OB_WIN_REMAP_LS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x08)
#define ADVK_OB_WIN_REMAP_MS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x0c)
#define ADVK_OB_WIN_MASK_LS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x10)
#define ADVK_OB_WIN_MASK_MS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x14)
#define ADVK_OB_WIN_ACTIONS(win)		ADVK_OB_WIN_REG_ADDR(win, 0x18)
#define ADVK_OB_WIN_DEFAULT_ACTIONS		(ADVK_OB_WIN_ACTIONS(ADVK_OB_WIN_COUNT-1) + 0x4)
#define     ADVK_OB_WIN_FUNC_NUM_MASK		GENMASK(31, 24)
#define     ADVK_OB_WIN_FUNC_NUM_SHIFT		24
#define     ADVK_OB_WIN_FUNC_NUM_ENABLE		BIT(23)
#define     ADVK_OB_WIN_BUS_NUM_BITS_MASK	GENMASK(22, 20)
#define     ADVK_OB_WIN_BUS_NUM_BITS_SHIFT	20
#define     ADVK_OB_WIN_MSG_CODE_ENABLE		BIT(22)
#define     ADVK_OB_WIN_MSG_CODE_MASK		GENMASK(21, 14)
#define     ADVK_OB_WIN_MSG_CODE_SHIFT		14
#define     ADVK_OB_WIN_MSG_PAYLOAD_LEN		BIT(12)
#define     ADVK_OB_WIN_ATTR_ENABLE		BIT(11)
#define     ADVK_OB_WIN_ATTR_TC_MASK		GENMASK(10, 8)
#define     ADVK_OB_WIN_ATTR_TC_SHIFT		8
#define     ADVK_OB_WIN_ATTR_RELAXED		BIT(7)
#define     ADVK_OB_WIN_ATTR_NOSNOOP		BIT(6)
#define     ADVK_OB_WIN_ATTR_POISON		BIT(5)
#define     ADVK_OB_WIN_ATTR_IDO		BIT(4)
#define     ADVK_OB_WIN_TYPE_MASK		GENMASK(3, 0)
#define     ADVK_OB_WIN_TYPE_SHIFT		0
#define     ADVK_OB_WIN_TYPE_MEM		0x0
#define     ADVK_OB_WIN_TYPE_IO			0x4
#define     ADVK_OB_WIN_TYPE_CONFIG_TYPE0	0x8
#define     ADVK_OB_WIN_TYPE_CONFIG_TYPE1	0x9
#define     ADVK_OB_WIN_TYPE_MSG		0xc

/* Local Management Interface registers */
#define ADVK_LMI_BASE_ADDR			0x6000
#define ADVK_LMI_PHY_CFG0			(ADVK_LMI_BASE_ADDR + 0x0)
#define     ADVK_LMI_PHY_CFG0_LTSSM_MASK	GENMASK(29, 24)
#define     ADVK_LMI_PHY_CFG0_LTSSM_SHIFT	24
#define     ADVK_LMI_PHY_CFG0_LTSSM_L0		0x10
#define     ADVK_LMI_PHY_CFG0_LTSSM_DISABLED	0x20
#define ADVK_LMI_VENDOR_ID			(ADVK_LMI_BASE_ADDR + 0x44)

/* Core Control registers */
#define ADVK_CORE_CTRL_BASE_ADDR		0x18000
#define ADVK_CORE_CTRL_CONFIG			(ADVK_CORE_CTRL_BASE_ADDR + 0x0)
#define     ADVK_CORE_CTRL_CONFIG_COMMAND_MODE	BIT(0)

/* PCIe Retries & Timeout definitions */
#define PIO_MAX_RETRIES				1500
#define PIO_WAIT_TIMEOUT			1000
#define LINK_MAX_RETRIES			10
#define LINK_WAIT_TIMEOUT			100000

#define CFG_RD_CRS_VAL				0xFFFF0001

/**
 * struct pcie_advk - Advk PCIe controller state
 *
 * @base:        The base address of the register space.
 * @sec_busno:   Bus number for the device behind the PCIe root-port.
 * @dev:         The pointer to PCI uclass device.
 * @reset_gpio:  GPIO descriptor for PERST.
 * @cfgcache:    Buffer for emulation of PCIe Root Port's PCI Bridge registers
 *               that are not available on Aardvark.
 * @cfgcrssve:   For CRSSVE emulation.
 */
struct pcie_advk {
	void			*base;
	int			sec_busno;
	struct udevice		*dev;
	struct gpio_desc	reset_gpio;
	u32			cfgcache[(0x3c - 0x10) / 4];
	bool			cfgcrssve;
};

static inline void advk_writel(struct pcie_advk *pcie, uint val, uint reg)
{
	writel(val, pcie->base + reg);
}

static inline uint advk_readl(struct pcie_advk *pcie, uint reg)
{
	return readl(pcie->base + reg);
}

/**
 * pcie_advk_link_up() - Check if PCIe link is up or not
 *
 * @pcie: The PCI device to access
 *
 * Return true on link up.
 * Return false on link down.
 */
static bool pcie_advk_link_up(struct pcie_advk *pcie)
{
	u32 val, ltssm_state;

	val = advk_readl(pcie, ADVK_LMI_PHY_CFG0);
	ltssm_state = (val & ADVK_LMI_PHY_CFG0_LTSSM_MASK) >> ADVK_LMI_PHY_CFG0_LTSSM_SHIFT;
	return ltssm_state >= ADVK_LMI_PHY_CFG0_LTSSM_L0 && ltssm_state < ADVK_LMI_PHY_CFG0_LTSSM_DISABLED;
}

/**
 * pcie_advk_addr_valid() - Check for valid bus address
 *
 * @pcie: Pointer to the PCI bus
 * @busno: Bus number of PCI device
 * @dev: Device number of PCI device
 * @func: Function number of PCI device
 * @bdf: The PCI device to access
 *
 * Return: true on valid, false on invalid
 */
static bool pcie_advk_addr_valid(struct pcie_advk *pcie,
				 int busno, u8 dev, u8 func)
{
	/* On the root bus there is only one PCI Bridge */
	if (busno == 0 && (dev != 0 || func != 0))
		return false;

	/* Access to other buses is possible when link is up */
	if (busno != 0 && !pcie_advk_link_up(pcie))
		return false;

	/*
	 * In PCI-E only a single device (0) can exist on the secondary bus.
	 * Beyond the secondary bus, there might be a Switch and anything is
	 * possible.
	 */
	if (busno == pcie->sec_busno && dev != 0)
		return false;

	return true;
}

/**
 * pcie_advk_wait_pio() - Wait for PIO access to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1.5 seconds for PIO access to be accomplished.
 *
 * Return positive - retry count if PIO access is accomplished.
 * Return negative - error if PIO access is timed out.
 */
static int pcie_advk_wait_pio(struct pcie_advk *pcie)
{
	uint start, isr;
	uint count;

	for (count = 1; count <= PIO_MAX_RETRIES; count++) {
		start = advk_readl(pcie, ADVK_PIO_START);
		isr = advk_readl(pcie, ADVK_PIO_ISR);
		if (!start && isr)
			return count;
		/*
		 * Do not check the PIO state too frequently,
		 * 100us delay is appropriate.
		 */
		udelay(PIO_WAIT_TIMEOUT);
	}

	dev_err(pcie->dev, "PIO read/write transfer time out\n");
	return -ETIMEDOUT;
}

/**
 * pcie_advk_check_pio_status() - Validate PIO status and get the read result
 *
 * @pcie: Pointer to the PCI bus
 * @allow_crs: Only for read requests, if CRS response is allowed
 * @read_val: Pointer to the read result
 *
 * Return: 0 on success
 */
static int pcie_advk_check_pio_status(struct pcie_advk *pcie,
				      bool allow_crs,
				      uint *read_val)
{
	int ret;
	uint reg;
	unsigned int status;
	char *strcomp_status, *str_posted;

	reg = advk_readl(pcie, ADVK_PIO_STAT);
	status = (reg & ADVK_PIO_COMPLETION_STATUS_MASK) >>
		ADVK_PIO_COMPLETION_STATUS_SHIFT;

	switch (status) {
	case ADVK_PIO_COMPLETION_STATUS_OK:
		if (reg & ADVK_PIO_ERR_STATUS) {
			strcomp_status = "COMP_ERR";
			ret = -EFAULT;
			break;
		}
		/* Get the read result */
		if (read_val)
			*read_val = advk_readl(pcie, ADVK_PIO_RD_DATA);
		/* No error */
		strcomp_status = NULL;
		ret = 0;
		break;
	case ADVK_PIO_COMPLETION_STATUS_UR:
		strcomp_status = "UR";
		ret = -EOPNOTSUPP;
		break;
	case ADVK_PIO_COMPLETION_STATUS_CRS:
		if (allow_crs && read_val) {
			/* For reading, CRS is not an error status. */
			*read_val = CFG_RD_CRS_VAL;
			strcomp_status = NULL;
			ret = 0;
		} else {
			strcomp_status = "CRS";
			ret = -EAGAIN;
		}
		break;
	case ADVK_PIO_COMPLETION_STATUS_CA:
		strcomp_status = "CA";
		ret = -ECANCELED;
		break;
	default:
		strcomp_status = "Unknown";
		ret = -EINVAL;
		break;
	}

	if (!strcomp_status)
		return ret;

	if (reg & ADVK_PIO_NON_POSTED_REQ)
		str_posted = "Non-posted";
	else
		str_posted = "Posted";

	dev_dbg(pcie->dev, "%s PIO Response Status: %s, %#x @ %#x\n",
		str_posted, strcomp_status, reg,
		advk_readl(pcie, ADVK_PIO_ADDR_LS));

	return ret;
}

/**
 * pcie_advk_read_config() - Read from configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_advk_read_config(const struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct pcie_advk *pcie = dev_get_priv(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	int retry_count;
	bool allow_crs;
	ulong data;
	uint reg;
	int ret;

	dev_dbg(pcie->dev, "PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_advk_addr_valid(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		dev_dbg(pcie->dev, "- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	/*
	 * The configuration space of the PCI Bridge on the root bus (zero) is
	 * not accessible via PIO transfers like all other PCIe devices. PCI
	 * Bridge config registers are available directly in Aardvark memory
	 * space starting at offset zero. The PCI Bridge config space is of
	 * Type 0, but the BAR registers (including ROM BAR) don't have the same
	 * meaning as in the PCIe specification. Therefore do not access BAR
	 * registers and non-common registers (those which have different
	 * meaning for Type 0 and Type 1 config space) of the PCI Bridge
	 * and instead read their content from driver virtual cfgcache[].
	 */
	if (busno == 0) {
		if ((offset >= 0x10 && offset < 0x34) || (offset >= 0x38 && offset < 0x3c))
			data = pcie->cfgcache[(offset - 0x10) / 4];
		else
			data = advk_readl(pcie, ADVK_ROOT_PORT_PCI_CFG_OFF + (offset & ~3));

		if ((offset & ~3) == (PCI_HEADER_TYPE & ~3)) {
			/*
			 * Change Header Type of PCI Bridge device to Type 1
			 * (0x01, used by PCI Bridges) because hardwired value
			 * is Type 0 (0x00, used by Endpoint devices).
			 */
			data &= ~0x007f0000;
			data |= PCI_HEADER_TYPE_BRIDGE << 16;
		}

		if ((offset & ~3) == ADVK_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_RTCTL) {
			/* CRSSVE bit is stored only in cache */
			if (pcie->cfgcrssve)
				data |= PCI_EXP_RTCTL_CRSSVE;
		}

		if ((offset & ~3) == ADVK_ROOT_PORT_PCI_EXP_OFF + (PCI_EXP_RTCAP & ~3)) {
			/* CRS is emulated below, so set CRSVIS capability */
			data |= PCI_EXP_RTCAP_CRSVIS << 16;
		}

		*valuep = pci_conv_32_to_size(data, offset, size);

		return 0;
	}

	/*
	 * Returning fabricated CRS value (0xFFFF0001) by PCIe Root Complex to
	 * OS is allowed only for 4-byte PCI_VENDOR_ID config read request and
	 * only when CRSSVE bit in Root Port PCIe device is enabled. In all
	 * other error PCIe Root Complex must return all-ones.
	 *
	 * U-Boot currently does not support handling of CRS return value for
	 * PCI_VENDOR_ID config read request and also does not set CRSSVE bit.
	 * So it means that pcie->cfgcrssve is false. But the code is prepared
	 * for returning CRS, so that if U-Boot does support CRS in the future,
	 * it will work for Aardvark.
	 */
	allow_crs = (offset == PCI_VENDOR_ID) && (size == PCI_SIZE_32) && pcie->cfgcrssve;

	if (advk_readl(pcie, ADVK_PIO_START)) {
		dev_err(pcie->dev,
			"Previous PIO read/write transfer is still running\n");
		if (allow_crs) {
			*valuep = CFG_RD_CRS_VAL;
			return 0;
		}
		*valuep = pci_get_ff(size);
		return -EAGAIN;
	}

	/* Program the control register */
	reg = advk_readl(pcie, ADVK_PIO_CTRL);
	reg &= ~ADVK_PIO_CTRL_TYPE_MASK;
	if (busno == pcie->sec_busno)
		reg |= ADVK_PIO_CTRL_TYPE_RD_TYPE0 << ADVK_PIO_CTRL_TYPE_SHIFT;
	else
		reg |= ADVK_PIO_CTRL_TYPE_RD_TYPE1 << ADVK_PIO_CTRL_TYPE_SHIFT;
	advk_writel(pcie, reg, ADVK_PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_ECAM_OFFSET(busno, PCI_DEV(bdf), PCI_FUNC(bdf), (offset & ~0x3));
	advk_writel(pcie, reg, ADVK_PIO_ADDR_LS);
	advk_writel(pcie, 0, ADVK_PIO_ADDR_MS);

	/* Program the data strobe */
	advk_writel(pcie, 0xf, ADVK_PIO_WR_DATA_STRB);

	retry_count = 0;

retry:
	/* Start the transfer */
	advk_writel(pcie, 1, ADVK_PIO_ISR);
	advk_writel(pcie, 1, ADVK_PIO_START);

	ret = pcie_advk_wait_pio(pcie);
	if (ret < 0) {
		if (allow_crs) {
			*valuep = CFG_RD_CRS_VAL;
			return 0;
		}
		*valuep = pci_get_ff(size);
		return ret;
	}

	retry_count += ret;

	/* Check PIO status and get the read result */
	ret = pcie_advk_check_pio_status(pcie, allow_crs, &reg);
	if (ret == -EAGAIN && retry_count < PIO_MAX_RETRIES)
		goto retry;
	if (ret) {
		*valuep = pci_get_ff(size);
		return ret;
	}

	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08x)\n",
		offset, size, reg);
	*valuep = pci_conv_32_to_size(reg, offset, size);

	return 0;
}

/**
 * pcie_calc_datastrobe() - Calculate data strobe
 *
 * @offset: The offset into the device's configuration space
 * @size: Indicates the size of access to perform
 *
 * Calculate data strobe according to offset and size
 *
 */
static uint pcie_calc_datastrobe(uint offset, enum pci_size_t size)
{
	uint bytes, data_strobe;

	switch (size) {
	case PCI_SIZE_8:
		bytes = 1;
		break;
	case PCI_SIZE_16:
		bytes = 2;
		break;
	default:
		bytes = 4;
	}

	data_strobe = GENMASK(bytes - 1, 0) << (offset & 0x3);

	return data_strobe;
}

/**
 * pcie_advk_write_config() - Write to configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_advk_write_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong value,
				  enum pci_size_t size)
{
	struct pcie_advk *pcie = dev_get_priv(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	int retry_count;
	ulong data;
	uint reg;
	int ret;

	dev_dbg(pcie->dev, "PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08lx)\n",
		offset, size, value);

	if (!pcie_advk_addr_valid(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		dev_dbg(pcie->dev, "- out of range\n");
		return 0;
	}

	/*
	 * As explained in pcie_advk_read_config(), PCI Bridge config registers
	 * are available directly in Aardvark memory space starting at offset
	 * zero. Type 1 specific registers are not available, so we write their
	 * content only into driver virtual cfgcache[].
	 */
	if (busno == 0) {
		if ((offset >= 0x10 && offset < 0x34) ||
		    (offset >= 0x38 && offset < 0x3c)) {
			data = pcie->cfgcache[(offset - 0x10) / 4];
			data = pci_conv_size_to_32(data, value, offset, size);
			/* This PCI bridge does not have configurable bars */
			if ((offset & ~3) == PCI_BASE_ADDRESS_0 ||
			    (offset & ~3) == PCI_BASE_ADDRESS_1 ||
			    (offset & ~3) == PCI_ROM_ADDRESS1)
				data = 0x0;
			pcie->cfgcache[(offset - 0x10) / 4] = data;
		} else {
			data = advk_readl(pcie, ADVK_ROOT_PORT_PCI_CFG_OFF + (offset & ~3));
			data = pci_conv_size_to_32(data, value, offset, size);
			advk_writel(pcie, data, ADVK_ROOT_PORT_PCI_CFG_OFF + (offset & ~3));
		}

		if (offset == PCI_SECONDARY_BUS ||
		    (offset == PCI_PRIMARY_BUS && size != PCI_SIZE_8))
			pcie->sec_busno = (data >> 8) & 0xff;

		if ((offset & ~3) == ADVK_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_RTCTL)
			pcie->cfgcrssve = data & PCI_EXP_RTCTL_CRSSVE;

		return 0;
	}

	if (advk_readl(pcie, ADVK_PIO_START)) {
		dev_err(pcie->dev,
			"Previous PIO read/write transfer is still running\n");
		return -EAGAIN;
	}

	/* Program the control register */
	reg = advk_readl(pcie, ADVK_PIO_CTRL);
	reg &= ~ADVK_PIO_CTRL_TYPE_MASK;
	if (busno == pcie->sec_busno)
		reg |= ADVK_PIO_CTRL_TYPE_WR_TYPE0 << ADVK_PIO_CTRL_TYPE_SHIFT;
	else
		reg |= ADVK_PIO_CTRL_TYPE_WR_TYPE1 << ADVK_PIO_CTRL_TYPE_SHIFT;
	advk_writel(pcie, reg, ADVK_PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_ECAM_OFFSET(busno, PCI_DEV(bdf), PCI_FUNC(bdf), (offset & ~0x3));
	advk_writel(pcie, reg, ADVK_PIO_ADDR_LS);
	advk_writel(pcie, 0, ADVK_PIO_ADDR_MS);
	dev_dbg(pcie->dev, "\tPIO req. - addr = 0x%08x\n", reg);

	/* Program the data register */
	reg = pci_conv_size_to_32(0, value, offset, size);
	advk_writel(pcie, reg, ADVK_PIO_WR_DATA);
	dev_dbg(pcie->dev, "\tPIO req. - val  = 0x%08x\n", reg);

	/* Program the data strobe */
	reg = pcie_calc_datastrobe(offset, size);
	advk_writel(pcie, reg, ADVK_PIO_WR_DATA_STRB);
	dev_dbg(pcie->dev, "\tPIO req. - strb = 0x%02x\n", reg);

	retry_count = 0;

retry:
	/* Start the transfer */
	advk_writel(pcie, 1, ADVK_PIO_ISR);
	advk_writel(pcie, 1, ADVK_PIO_START);

	ret = pcie_advk_wait_pio(pcie);
	if (ret < 0)
		return ret;

	retry_count += ret;

	/* Check PIO status */
	ret = pcie_advk_check_pio_status(pcie, false, NULL);
	if (ret == -EAGAIN && retry_count < PIO_MAX_RETRIES)
		goto retry;
	return ret;
}

/**
 * pcie_advk_wait_for_link() - Wait for link training to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1 second for link training to be accomplished.
 */
static void pcie_advk_wait_for_link(struct pcie_advk *pcie)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_MAX_RETRIES; retries++) {
		if (pcie_advk_link_up(pcie)) {
			printf("PCIe: Link up\n");
			return;
		}

		udelay(LINK_WAIT_TIMEOUT);
	}

	printf("PCIe: Link down\n");
}

/*
 * Set PCIe address window register which could be used for memory
 * mapping.
 */
static void pcie_advk_set_ob_win(struct pcie_advk *pcie, u8 win_num,
				 phys_addr_t match, phys_addr_t remap,
				 phys_addr_t mask, u32 actions)
{
	advk_writel(pcie, ADVK_OB_WIN_ENABLE |
			  lower_32_bits(match), ADVK_OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, upper_32_bits(match), ADVK_OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, lower_32_bits(remap), ADVK_OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, upper_32_bits(remap), ADVK_OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, lower_32_bits(mask), ADVK_OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, upper_32_bits(mask), ADVK_OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, actions, ADVK_OB_WIN_ACTIONS(win_num));
}

static void pcie_advk_disable_ob_win(struct pcie_advk *pcie, u8 win_num)
{
	advk_writel(pcie, 0, ADVK_OB_WIN_MATCH_LS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_MATCH_MS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_REMAP_LS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_REMAP_MS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_MASK_LS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_MASK_MS(win_num));
	advk_writel(pcie, 0, ADVK_OB_WIN_ACTIONS(win_num));
}

static void pcie_advk_set_ob_region(struct pcie_advk *pcie, int *wins,
				    struct pci_region *region, u32 actions)
{
	phys_addr_t phys_start = region->phys_start;
	pci_addr_t bus_start = region->bus_start;
	pci_size_t size = region->size;
	phys_addr_t win_mask;
	u64 win_size;

	if (*wins == -1)
		return;

	/*
	 * The n-th PCIe window is configured by tuple (match, remap, mask)
	 * and an access to address A uses this window if A matches the
	 * match with given mask.
	 * So every PCIe window size must be a power of two and every start
	 * address must be aligned to window size. Minimal size is 64 KiB
	 * because lower 16 bits of mask must be zero. Remapped address
	 * may have set only bits from the mask.
	 */
	while (*wins < ADVK_OB_WIN_COUNT && size > 0) {
		/* Calculate the largest aligned window size */
		win_size = (1ULL << (fls64(size) - 1)) |
			   (phys_start ? (1ULL << __ffs64(phys_start)) : 0);
		win_size = 1ULL << __ffs64(win_size);
		win_mask = ~(win_size - 1);
		if (win_size < 0x10000 || (bus_start & ~win_mask))
			break;

		dev_dbg(pcie->dev,
			"Configuring PCIe window %d: [0x%llx-0x%llx] as 0x%x\n",
			*wins, (u64)phys_start, (u64)phys_start + win_size,
			actions);
		pcie_advk_set_ob_win(pcie, *wins, phys_start, bus_start,
				     win_mask, actions);

		phys_start += win_size;
		bus_start += win_size;
		size -= win_size;
		(*wins)++;
	}

	if (size > 0) {
		*wins = -1;
		dev_err(pcie->dev,
			"Invalid PCIe region [0x%llx-0x%llx]\n",
			(u64)region->phys_start,
			(u64)region->phys_start + region->size);
	}
}

/**
 * pcie_advk_setup_hw() - PCIe initailzation
 *
 * @pcie: The PCI device to access
 *
 * Return: 0 on success
 */
static int pcie_advk_setup_hw(struct pcie_advk *pcie)
{
	struct pci_region *io, *mem, *pref;
	int i, wins;
	u32 reg;

	/* Set from Command to Direct mode */
	reg = advk_readl(pcie, ADVK_CORE_CTRL_CONFIG);
	reg &= ~ADVK_CORE_CTRL_CONFIG_COMMAND_MODE;
	advk_writel(pcie, reg, ADVK_CORE_CTRL_CONFIG);

	/* Set PCI global control register to RC mode */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL0);
	reg |= ADVK_GLOBAL_CTRL0_IS_RC;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL0);

	/*
	 * Replace incorrect PCI vendor id value 0x1b4b by correct value 0x11ab.
	 * ADVK_LMI_VENDOR_ID contains vendor id in low 16 bits and subsystem vendor
	 * id in high 16 bits. Updating this register changes readback value of
	 * read-only vendor id bits in Root Port PCI_VENDOR_ID register. Workaround
	 * for erratum 4.1: "The value of device and vendor ID is incorrect".
	 */
	advk_writel(pcie, 0x11ab11ab, ADVK_LMI_VENDOR_ID);

	/*
	 * Change Class Code of PCI Bridge device to PCI Bridge (0x600400),
	 * because default value is Mass Storage Controller (0x010400), causing
	 * U-Boot to fail to recognize it as P2P Bridge.
	 *
	 * Note that this Aardvark PCI Bridge does not have a compliant Type 1
	 * Configuration Space and it even cannot be accessed via Aardvark's
	 * PCI config space access method. Aardvark PCI Bridge Config space is
	 * available in internal Aardvark registers starting at offset 0x0
	 * and has format of Type 0 config space.
	 *
	 * Moreover Type 0 BAR registers (ranges 0x10 - 0x28 and 0x30 - 0x34)
	 * have the same format in Marvell's specification as in PCIe
	 * specification, but their meaning is totally different (and not even
	 * the same meaning as explained in the corresponding comment in the
	 * pci_mvebu driver; aardvark is still different).
	 *
	 * So our driver converts Type 0 config space to Type 1 and reports
	 * Header Type as Type 1. Access to BAR registers and to non-existent
	 * Type 1 registers is redirected to the virtual cfgcache[] buffer,
	 * which avoids changing unrelated registers.
	 */
	reg = advk_readl(pcie, ADVK_ROOT_PORT_PCI_CFG_OFF + PCI_CLASS_REVISION);
	reg &= ~0xffffff00;
	reg |= PCI_CLASS_BRIDGE_PCI_NORMAL << 8;
	advk_writel(pcie, reg, ADVK_ROOT_PORT_PCI_CFG_OFF + PCI_CLASS_REVISION);

	/* Enable generation and checking of ECRC on PCIe Root Port */
	reg = advk_readl(pcie, ADVK_ROOT_PORT_PCI_ERR_OFF + PCI_ERR_CAP);
	reg |= PCI_ERR_CAP_ECRC_GENE | PCI_ERR_CAP_ECRC_CHKE;
	advk_writel(pcie, reg, ADVK_ROOT_PORT_PCI_ERR_OFF + PCI_ERR_CAP);

	/* Set PCIe Device Control register on PCIe Root Port */
	reg = advk_readl(pcie, ADVK_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_DEVCTL);
	reg &= ~PCI_EXP_DEVCTL_RELAX_EN;
	reg &= ~PCI_EXP_DEVCTL_NOSNOOP_EN;
	reg &= ~PCI_EXP_DEVCTL_PAYLOAD;
	reg &= ~PCI_EXP_DEVCTL_READRQ;
	reg |= PCI_EXP_DEVCTL_PAYLOAD_512B;
	reg |= PCI_EXP_DEVCTL_READRQ_512B;
	advk_writel(pcie, reg, ADVK_ROOT_PORT_PCI_EXP_OFF + PCI_EXP_DEVCTL);

	/* Program PCIe Control 2 to disable strict ordering */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL2);
	reg &= ~ADVK_GLOBAL_CTRL2_STRICT_ORDER_EN;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL2);

	/* Set GEN2 */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL0);
	reg &= ~ADVK_GLOBAL_CTRL0_SPEED_GEN_MASK;
	reg |= ADVK_GLOBAL_CTRL0_SPEED_GEN_2 << ADVK_GLOBAL_CTRL0_SPEED_GEN_SHIFT;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL0);

	/* Set lane X1 */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL0);
	reg &= ~ADVK_GLOBAL_CTRL0_LANE_COUNT_MASK;
	reg |= ADVK_GLOBAL_CTRL0_LANE_COUNT_1 << ADVK_GLOBAL_CTRL0_LANE_COUNT_SHIFT;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL0);

	/* Enable link training */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL0);
	reg |= ADVK_GLOBAL_CTRL0_LINK_TRAINING_EN;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL0);

	/*
	 * Enable AXI address window location generation:
	 * When it is enabled, the default outbound window
	 * configurations (Default User Field: 0xD0074CFC)
	 * are used to transparent address translation for
	 * the outbound transactions. Thus, PCIe address
	 * windows are not required for transparent memory
	 * access when default outbound window configuration
	 * is set for memory access.
	 */
	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL2);
	reg |= ADVK_GLOBAL_CTRL2_ADDRWIN_MAP_EN;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL2);

	/*
	 * Bypass the address window mapping for PIO:
	 * Since PIO access already contains all required
	 * info over AXI interface by PIO registers, the
	 * address window is not required.
	 */
	reg = advk_readl(pcie, ADVK_PIO_CTRL);
	reg |= ADVK_PIO_CTRL_ADDR_WIN_DISABLE;
	advk_writel(pcie, reg, ADVK_PIO_CTRL);

	/*
	 * Set memory access in Default User Field so it
	 * is not required to configure PCIe address for
	 * transparent memory access.
	 */
	advk_writel(pcie, ADVK_OB_WIN_TYPE_MEM, ADVK_OB_WIN_DEFAULT_ACTIONS);

	/*
	 * Configure PCIe address windows for non-memory or
	 * non-transparent access as by default PCIe uses
	 * transparent memory access.
	 */
	wins = 0;
	pci_get_regions(pcie->dev, &io, &mem, &pref);
	if (io)
		pcie_advk_set_ob_region(pcie, &wins, io, ADVK_OB_WIN_TYPE_IO);
	if (mem && mem->phys_start != mem->bus_start)
		pcie_advk_set_ob_region(pcie, &wins, mem, ADVK_OB_WIN_TYPE_MEM);
	if (pref && pref->phys_start != pref->bus_start)
		pcie_advk_set_ob_region(pcie, &wins, pref, ADVK_OB_WIN_TYPE_MEM);

	/* Disable remaining PCIe outbound windows */
	for (i = ((wins >= 0) ? wins : 0); i < ADVK_OB_WIN_COUNT; i++)
		pcie_advk_disable_ob_win(pcie, i);

	if (wins == -1)
		return -EINVAL;

	/* Wait for PCIe link up */
	pcie_advk_wait_for_link(pcie);

	return 0;
}

/**
 * pcie_advk_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_advk_probe(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);

	gpio_request_by_name(dev, "reset-gpios", 0, &pcie->reset_gpio,
			     GPIOD_IS_OUT);
	/*
	 * Issue reset to add-in card through the dedicated GPIO.
	 * Some boards are connecting the card reset pin to common system
	 * reset wire and others are using separate GPIO port.
	 * In the last case we have to release a reset of the addon card
	 * using this GPIO.
	 *
	 * FIX-ME:
	 *     The PCIe RESET signal is not supposed to be released along
	 *     with the SOC RESET signal. It should be lowered as early as
	 *     possible before PCIe PHY initialization. Moreover, the PCIe
	 *     clock should be gated as well.
	 */
	if (dm_gpio_is_valid(&pcie->reset_gpio)) {
		dev_dbg(dev, "Toggle PCIE Reset GPIO ...\n");
		dm_gpio_set_value(&pcie->reset_gpio, 1);
		mdelay(200);
		dm_gpio_set_value(&pcie->reset_gpio, 0);
	} else {
		dev_warn(dev, "PCIE Reset on GPIO support is missing\n");
	}

	pcie->dev = pci_get_controller(dev);

	/* PCI Bridge support 32-bit I/O and 64-bit prefetch mem addressing */
	pcie->cfgcache[(PCI_IO_BASE - 0x10) / 4] =
		PCI_IO_RANGE_TYPE_32 | (PCI_IO_RANGE_TYPE_32 << 8);
	pcie->cfgcache[(PCI_PREF_MEMORY_BASE - 0x10) / 4] =
		PCI_PREF_RANGE_TYPE_64 | (PCI_PREF_RANGE_TYPE_64 << 16);

	return pcie_advk_setup_hw(pcie);
}

static int pcie_advk_remove(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);
	u32 reg;
	int i;

	for (i = 0; i < ADVK_OB_WIN_COUNT; i++)
		pcie_advk_disable_ob_win(pcie, i);

	reg = advk_readl(pcie, ADVK_ROOT_PORT_PCI_CFG_OFF + PCI_COMMAND);
	reg &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	advk_writel(pcie, reg, ADVK_ROOT_PORT_PCI_CFG_OFF + PCI_COMMAND);

	reg = advk_readl(pcie, ADVK_GLOBAL_CTRL0);
	reg &= ~ADVK_GLOBAL_CTRL0_LINK_TRAINING_EN;
	advk_writel(pcie, reg, ADVK_GLOBAL_CTRL0);

	return 0;
}

/**
 * pcie_advk_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_advk_of_to_plat(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);

	/* Get the register base address */
	pcie->base = dev_read_addr_ptr(dev);
	if (!pcie->base)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_advk_ops = {
	.read_config	= pcie_advk_read_config,
	.write_config	= pcie_advk_write_config,
};

static const struct udevice_id pcie_advk_ids[] = {
	{ .compatible = "marvell,armada-3700-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_advk) = {
	.name			= "pcie_advk",
	.id			= UCLASS_PCI,
	.of_match		= pcie_advk_ids,
	.ops			= &pcie_advk_ops,
	.of_to_plat	= pcie_advk_of_to_plat,
	.probe			= pcie_advk_probe,
	.remove			= pcie_advk_remove,
	.flags			= DM_FLAG_OS_PREPARE,
	.priv_auto	= sizeof(struct pcie_advk),
};
