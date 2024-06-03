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
 *
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <linux/ioport.h>

/* PCIe core registers */
#define PCIE_CORE_CMD_STATUS_REG				0x4
#define     PCIE_CORE_CMD_IO_ACCESS_EN				BIT(0)
#define     PCIE_CORE_CMD_MEM_ACCESS_EN				BIT(1)
#define     PCIE_CORE_CMD_MEM_IO_REQ_EN				BIT(2)
#define PCIE_CORE_DEV_CTRL_STATS_REG				0xc8
#define     PCIE_CORE_DEV_CTRL_STATS_RELAX_ORDER_DISABLE	(0 << 4)
#define     PCIE_CORE_DEV_CTRL_STATS_SNOOP_DISABLE		(0 << 11)
#define PCIE_CORE_LINK_CTRL_STAT_REG				0xd0
#define     PCIE_CORE_LINK_TRAINING				BIT(5)
#define PCIE_CORE_ERR_CAPCTL_REG				0x118
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX			BIT(5)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN			BIT(6)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHECK			BIT(7)
#define     PCIE_CORE_ERR_CAPCTL_ECRC_CHECK_RCV			BIT(8)

/* PIO registers base address and register offsets */
#define PIO_BASE_ADDR				0x4000
#define PIO_CTRL				(PIO_BASE_ADDR + 0x0)
#define   PIO_CTRL_TYPE_MASK			GENMASK(3, 0)
#define   PIO_CTRL_ADDR_WIN_DISABLE		BIT(24)
#define PIO_STAT				(PIO_BASE_ADDR + 0x4)
#define   PIO_COMPLETION_STATUS_SHIFT		7
#define   PIO_COMPLETION_STATUS_MASK		GENMASK(9, 7)
#define   PIO_COMPLETION_STATUS_OK		0
#define   PIO_COMPLETION_STATUS_UR		1
#define   PIO_COMPLETION_STATUS_CRS		2
#define   PIO_COMPLETION_STATUS_CA		4
#define   PIO_NON_POSTED_REQ			BIT(10)
#define   PIO_ERR_STATUS			BIT(11)
#define PIO_ADDR_LS				(PIO_BASE_ADDR + 0x8)
#define PIO_ADDR_MS				(PIO_BASE_ADDR + 0xc)
#define PIO_WR_DATA				(PIO_BASE_ADDR + 0x10)
#define PIO_WR_DATA_STRB			(PIO_BASE_ADDR + 0x14)
#define PIO_RD_DATA				(PIO_BASE_ADDR + 0x18)
#define PIO_START				(PIO_BASE_ADDR + 0x1c)
#define PIO_ISR					(PIO_BASE_ADDR + 0x20)

/* Aardvark Control registers */
#define CONTROL_BASE_ADDR			0x4800
#define PCIE_CORE_CTRL0_REG			(CONTROL_BASE_ADDR + 0x0)
#define     PCIE_GEN_SEL_MSK			0x3
#define     PCIE_GEN_SEL_SHIFT			0x0
#define     SPEED_GEN_1				0
#define     SPEED_GEN_2				1
#define     SPEED_GEN_3				2
#define     IS_RC_MSK				1
#define     IS_RC_SHIFT				2
#define     LANE_CNT_MSK			0x18
#define     LANE_CNT_SHIFT			0x3
#define     LANE_COUNT_1			(0 << LANE_CNT_SHIFT)
#define     LANE_COUNT_2			(1 << LANE_CNT_SHIFT)
#define     LANE_COUNT_4			(2 << LANE_CNT_SHIFT)
#define     LANE_COUNT_8			(3 << LANE_CNT_SHIFT)
#define     LINK_TRAINING_EN			BIT(6)
#define PCIE_CORE_CTRL2_REG			(CONTROL_BASE_ADDR + 0x8)
#define     PCIE_CORE_CTRL2_RESERVED		0x7
#define     PCIE_CORE_CTRL2_TD_ENABLE		BIT(4)
#define     PCIE_CORE_CTRL2_STRICT_ORDER_ENABLE	BIT(5)
#define     PCIE_CORE_CTRL2_ADDRWIN_MAP_ENABLE	BIT(6)

/* LMI registers base address and register offsets */
#define LMI_BASE_ADDR				0x6000
#define CFG_REG					(LMI_BASE_ADDR + 0x0)
#define     LTSSM_SHIFT				24
#define     LTSSM_MASK				0x3f
#define     LTSSM_L0				0x10

/* PCIe core controller registers */
#define CTRL_CORE_BASE_ADDR			0x18000
#define CTRL_CONFIG_REG				(CTRL_CORE_BASE_ADDR + 0x0)
#define     CTRL_MODE_SHIFT			0x0
#define     CTRL_MODE_MASK			0x1
#define     PCIE_CORE_MODE_DIRECT		0x0
#define     PCIE_CORE_MODE_COMMAND		0x1

/* Transaction types */
#define PCIE_CONFIG_RD_TYPE0			0x8
#define PCIE_CONFIG_RD_TYPE1			0x9
#define PCIE_CONFIG_WR_TYPE0			0xa
#define PCIE_CONFIG_WR_TYPE1			0xb

/* PCI_BDF shifts 8bit, so we need extra 4bit shift */
#define PCIE_BDF(dev)				(dev << 4)
#define PCIE_CONF_BUS(bus)			(((bus) & 0xff) << 20)
#define PCIE_CONF_DEV(dev)			(((dev) & 0x1f) << 15)
#define PCIE_CONF_FUNC(fun)			(((fun) & 0x7)	<< 12)
#define PCIE_CONF_REG(reg)			((reg) & 0xffc)
#define PCIE_CONF_ADDR(bus, devfn, where)	\
	(PCIE_CONF_BUS(bus) | PCIE_CONF_DEV(PCI_SLOT(devfn))	| \
	 PCIE_CONF_FUNC(PCI_FUNC(devfn)) | PCIE_CONF_REG(where))

/* PCIe Retries & Timeout definitions */
#define MAX_RETRIES				10
#define PIO_WAIT_TIMEOUT			100
#define LINK_WAIT_TIMEOUT			100000

#define CFG_RD_UR_VAL			0xFFFFFFFF
#define CFG_RD_CRS_VAL			0xFFFF0001

/**
 * struct pcie_advk - Advk PCIe controller state
 *
 * @reg_base:    The base address of the register space.
 * @first_busno: This driver supports multiple PCIe controllers.
 *               first_busno stores the bus number of the PCIe root-port
 *               number which may vary depending on the PCIe setup
 *               (PEX switches etc).
 * @device:      The pointer to PCI uclass device.
 */
struct pcie_advk {
	void           *base;
	int            first_busno;
	struct udevice *dev;
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
 * pcie_advk_addr_valid() - Check for valid bus address
 *
 * @bdf: The PCI device to access
 * @first_busno: Bus number of the PCIe controller root complex
 *
 * Return: 1 on valid, 0 on invalid
 */
static int pcie_advk_addr_valid(pci_dev_t bdf, int first_busno)
{
	/*
	 * In PCIE-E only a single device (0) can exist
	 * on the local bus. Beyound the local bus, there might be
	 * a Switch and everything is possible.
	 */
	if ((PCI_BUS(bdf) == first_busno) && (PCI_DEV(bdf) > 0))
		return 0;

	return 1;
}

/**
 * pcie_advk_wait_pio() - Wait for PIO access to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1 micro second for PIO access to be accomplished.
 *
 * Return 1 (true) if PIO access is accomplished.
 * Return 0 (false) if PIO access is timed out.
 */
static int pcie_advk_wait_pio(struct pcie_advk *pcie)
{
	uint start, isr;
	uint count;

	for (count = 0; count < MAX_RETRIES; count++) {
		start = advk_readl(pcie, PIO_START);
		isr = advk_readl(pcie, PIO_ISR);
		if (!start && isr)
			return 1;
		/*
		 * Do not check the PIO state too frequently,
		 * 100us delay is appropriate.
		 */
		udelay(PIO_WAIT_TIMEOUT);
	}

	dev_err(pcie->dev, "config read/write timed out\n");
	return 0;
}

/**
 * pcie_advk_check_pio_status() - Validate PIO status and get the read result
 *
 * @pcie: Pointer to the PCI bus
 * @read: Read from or write to configuration space - true(read) false(write)
 * @read_val: Pointer to the read result, only valid when read is true
 *
 */
static int pcie_advk_check_pio_status(struct pcie_advk *pcie,
				      bool read,
				      uint *read_val)
{
	uint reg;
	unsigned int status;
	char *strcomp_status, *str_posted;

	reg = advk_readl(pcie, PIO_STAT);
	status = (reg & PIO_COMPLETION_STATUS_MASK) >>
		PIO_COMPLETION_STATUS_SHIFT;

	switch (status) {
	case PIO_COMPLETION_STATUS_OK:
		if (reg & PIO_ERR_STATUS) {
			strcomp_status = "COMP_ERR";
			break;
		}
		/* Get the read result */
		if (read)
			*read_val = advk_readl(pcie, PIO_RD_DATA);
		/* No error */
		strcomp_status = NULL;
		break;
	case PIO_COMPLETION_STATUS_UR:
		if (read) {
			/* For reading, UR is not an error status. */
			*read_val = CFG_RD_UR_VAL;
			strcomp_status = NULL;
		} else {
			strcomp_status = "UR";
		}
		break;
	case PIO_COMPLETION_STATUS_CRS:
		if (read) {
			/* For reading, CRS is not an error status. */
			*read_val = CFG_RD_CRS_VAL;
			strcomp_status = NULL;
		} else {
			strcomp_status = "CRS";
		}
		break;
	case PIO_COMPLETION_STATUS_CA:
		strcomp_status = "CA";
		break;
	default:
		strcomp_status = "Unknown";
		break;
	}

	if (!strcomp_status)
		return 0;

	if (reg & PIO_NON_POSTED_REQ)
		str_posted = "Non-posted";
	else
		str_posted = "Posted";

	dev_err(pcie->dev, "%s PIO Response Status: %s, %#x @ %#x\n",
		str_posted, strcomp_status, reg,
		advk_readl(pcie, PIO_ADDR_LS));

	return -EFAULT;
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
static int pcie_advk_read_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct pcie_advk *pcie = dev_get_priv(bus);
	uint reg;
	int ret;

	dev_dbg(pcie->dev, "PCIE CFG read:  (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_advk_addr_valid(bdf, pcie->first_busno)) {
		dev_dbg(pcie->dev, "- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	/* Start PIO */
	advk_writel(pcie, 0, PIO_START);
	advk_writel(pcie, 1, PIO_ISR);

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (PCI_BUS(bdf) == pcie->first_busno)
		reg |= PCIE_CONFIG_RD_TYPE0;
	else
		reg |= PCIE_CONFIG_RD_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_BDF(bdf) | PCIE_CONF_REG(offset);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);

	/* Start the transfer */
	advk_writel(pcie, 1, PIO_START);

	if (!pcie_advk_wait_pio(pcie))
		return -EINVAL;

	/* Check PIO status and get the read result */
	ret = pcie_advk_check_pio_status(pcie, true, &reg);
	if (ret)
		return ret;

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
	uint reg;

	dev_dbg(pcie->dev, "PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	dev_dbg(pcie->dev, "(addr,size,val)=(0x%04x, %d, 0x%08lx)\n",
		offset, size, value);

	if (!pcie_advk_addr_valid(bdf, pcie->first_busno)) {
		dev_dbg(pcie->dev, "- out of range\n");
		return 0;
	}

	/* Start PIO */
	advk_writel(pcie, 0, PIO_START);
	advk_writel(pcie, 1, PIO_ISR);

	/* Program the control register */
	reg = advk_readl(pcie, PIO_CTRL);
	reg &= ~PIO_CTRL_TYPE_MASK;
	if (PCI_BUS(bdf) == pcie->first_busno)
		reg |= PCIE_CONFIG_WR_TYPE0;
	else
		reg |= PCIE_CONFIG_WR_TYPE1;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Program the address registers */
	reg = PCIE_BDF(bdf) | PCIE_CONF_REG(offset);
	advk_writel(pcie, reg, PIO_ADDR_LS);
	advk_writel(pcie, 0, PIO_ADDR_MS);
	dev_dbg(pcie->dev, "\tPIO req. - addr = 0x%08x\n", reg);

	/* Program the data register */
	reg = pci_conv_size_to_32(0, value, offset, size);
	advk_writel(pcie, reg, PIO_WR_DATA);
	dev_dbg(pcie->dev, "\tPIO req. - val  = 0x%08x\n", reg);

	/* Program the data strobe */
	reg = pcie_calc_datastrobe(offset, size);
	advk_writel(pcie, reg, PIO_WR_DATA_STRB);
	dev_dbg(pcie->dev, "\tPIO req. - strb = 0x%02x\n", reg);

	/* Start the transfer */
	advk_writel(pcie, 1, PIO_START);

	if (!pcie_advk_wait_pio(pcie)) {
		dev_dbg(pcie->dev, "- wait pio timeout\n");
		return -EINVAL;
	}

	/* Check PIO status */
	pcie_advk_check_pio_status(pcie, false, &reg);

	return 0;
}

/**
 * pcie_advk_link_up() - Check if PCIe link is up or not
 *
 * @pcie: The PCI device to access
 *
 * Return 1 (true) on link up.
 * Return 0 (false) on link down.
 */
static int pcie_advk_link_up(struct pcie_advk *pcie)
{
	u32 val, ltssm_state;

	val = advk_readl(pcie, CFG_REG);
	ltssm_state = (val >> LTSSM_SHIFT) & LTSSM_MASK;
	return ltssm_state >= LTSSM_L0;
}

/**
 * pcie_advk_wait_for_link() - Wait for link training to be accomplished
 *
 * @pcie: The PCI device to access
 *
 * Wait up to 1 second for link training to be accomplished.
 *
 * Return 1 (true) if link training ends up with link up success.
 * Return 0 (false) if link training ends up with link up failure.
 */
static int pcie_advk_wait_for_link(struct pcie_advk *pcie)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < MAX_RETRIES; retries++) {
		if (pcie_advk_link_up(pcie)) {
			printf("PCIE-%d: Link up\n", pcie->first_busno);
			return 0;
		}

		udelay(LINK_WAIT_TIMEOUT);
	}

	printf("PCIE-%d: Link down\n", pcie->first_busno);

	return -ETIMEDOUT;
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
	u32 reg;

	/* Set to Direct mode */
	reg = advk_readl(pcie, CTRL_CONFIG_REG);
	reg &= ~(CTRL_MODE_MASK << CTRL_MODE_SHIFT);
	reg |= ((PCIE_CORE_MODE_DIRECT & CTRL_MODE_MASK) << CTRL_MODE_SHIFT);
	advk_writel(pcie, reg, CTRL_CONFIG_REG);

	/* Set PCI global control register to RC mode */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= (IS_RC_MSK << IS_RC_SHIFT);
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Set Advanced Error Capabilities and Control PF0 register */
	reg = PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHK_TX_EN |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHECK |
		PCIE_CORE_ERR_CAPCTL_ECRC_CHECK_RCV;
	advk_writel(pcie, reg, PCIE_CORE_ERR_CAPCTL_REG);

	/* Set PCIe Device Control and Status 1 PF0 register */
	reg = PCIE_CORE_DEV_CTRL_STATS_RELAX_ORDER_DISABLE |
		PCIE_CORE_DEV_CTRL_STATS_SNOOP_DISABLE;
	advk_writel(pcie, reg, PCIE_CORE_DEV_CTRL_STATS_REG);

	/* Program PCIe Control 2 to disable strict ordering */
	reg = PCIE_CORE_CTRL2_RESERVED |
		PCIE_CORE_CTRL2_TD_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/* Set GEN2 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~PCIE_GEN_SEL_MSK;
	reg |= SPEED_GEN_2;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Set lane X1 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg &= ~LANE_CNT_MSK;
	reg |= LANE_COUNT_1;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/* Enable link training */
	reg = advk_readl(pcie, PCIE_CORE_CTRL0_REG);
	reg |= LINK_TRAINING_EN;
	advk_writel(pcie, reg, PCIE_CORE_CTRL0_REG);

	/*
	 * Enable AXI address window location generation:
	 * When it is enabled, the default outbound window
	 * configurations (Default User Field: 0xD0074CFC)
	 * are used to transparent address translation for
	 * the outbound transactions. Thus, PCIe address
	 * windows are not required.
	 */
	reg = advk_readl(pcie, PCIE_CORE_CTRL2_REG);
	reg |= PCIE_CORE_CTRL2_ADDRWIN_MAP_ENABLE;
	advk_writel(pcie, reg, PCIE_CORE_CTRL2_REG);

	/*
	 * Bypass the address window mapping for PIO:
	 * Since PIO access already contains all required
	 * info over AXI interface by PIO registers, the
	 * address window is not required.
	 */
	reg = advk_readl(pcie, PIO_CTRL);
	reg |= PIO_CTRL_ADDR_WIN_DISABLE;
	advk_writel(pcie, reg, PIO_CTRL);

	/* Start link training */
	reg = advk_readl(pcie, PCIE_CORE_LINK_CTRL_STAT_REG);
	reg |= PCIE_CORE_LINK_TRAINING;
	advk_writel(pcie, reg, PCIE_CORE_LINK_CTRL_STAT_REG);

	/* Wait for PCIe link up */
	if (pcie_advk_wait_for_link(pcie))
		return -ENXIO;

	reg = advk_readl(pcie, PCIE_CORE_CMD_STATUS_REG);
	reg |= PCIE_CORE_CMD_MEM_ACCESS_EN |
		PCIE_CORE_CMD_IO_ACCESS_EN |
		PCIE_CORE_CMD_MEM_IO_REQ_EN;
	advk_writel(pcie, reg, PCIE_CORE_CMD_STATUS_REG);

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

#ifdef CONFIG_DM_GPIO
	struct gpio_desc reset_gpio;

	gpio_request_by_name(dev, "reset-gpio", 0, &reset_gpio,
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
	if (dm_gpio_is_valid(&reset_gpio)) {
		dev_dbg(pcie->dev, "Toggle PCIE Reset GPIO ...\n");
		dm_gpio_set_value(&reset_gpio, 0);
		mdelay(200);
		dm_gpio_set_value(&reset_gpio, 1);
	}
#else
	dev_dbg(pcie->dev, "PCIE Reset on GPIO support is missing\n");
#endif /* CONFIG_DM_GPIO */

	pcie->first_busno = dev->seq;
	pcie->dev = pci_get_controller(dev);

	return pcie_advk_setup_hw(pcie);
}

/**
 * pcie_advk_ofdata_to_platdata() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_advk_ofdata_to_platdata(struct udevice *dev)
{
	struct pcie_advk *pcie = dev_get_priv(dev);

	/* Get the register base address */
	pcie->base = (void *)dev_read_addr_index(dev, 0);
	if ((fdt_addr_t)pcie->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_advk_ops = {
	.read_config	= pcie_advk_read_config,
	.write_config	= pcie_advk_write_config,
};

static const struct udevice_id pcie_advk_ids[] = {
	{ .compatible = "marvell,armada-37xx-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_advk) = {
	.name			= "pcie_advk",
	.id			= UCLASS_PCI,
	.of_match		= pcie_advk_ids,
	.ops			= &pcie_advk_ops,
	.ofdata_to_platdata	= pcie_advk_ofdata_to_platdata,
	.probe			= pcie_advk_probe,
	.priv_auto_alloc_size	= sizeof(struct pcie_advk),
};
