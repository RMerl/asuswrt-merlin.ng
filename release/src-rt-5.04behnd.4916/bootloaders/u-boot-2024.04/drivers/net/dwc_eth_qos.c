// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 *
 * Portions based on U-Boot's rtl8169.c.
 */

/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id eqos_ids[]'s .data
 * field.
 *
 * The following configurations are currently supported:
 * tegra186:
 *    NVIDIA's Tegra186 chip. This configuration uses an AXI master/DMA bus, an
 *    AHB slave/register bus, contains the DMA, MTL, and MAC sub-blocks, and
 *    supports a single RGMII PHY. This configuration also has SW control over
 *    all clock and reset signals to the HW block.
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <eth_phy.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/io.h>
#ifdef CONFIG_ARCH_IMX8M
#include <asm/arch/clock.h>
#include <asm/mach-imx/sys_proto.h>
#endif
#include <linux/delay.h>
#include <linux/printk.h>

#include "dwc_eth_qos.h"

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */
static void *eqos_alloc_descs(struct eqos_priv *eqos, unsigned int num)
{
	return memalign(ARCH_DMA_MINALIGN, num * eqos->desc_size);
}

static void eqos_free_descs(void *descs)
{
	free(descs);
}

static struct eqos_desc *eqos_get_desc(struct eqos_priv *eqos,
				       unsigned int num, bool rx)
{
	return (rx ? eqos->rx_descs : eqos->tx_descs) +
	       (num * eqos->desc_size);
}

void eqos_inval_desc_generic(void *desc)
{
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + sizeof(struct eqos_desc),
				  ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

void eqos_flush_desc_generic(void *desc)
{
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + sizeof(struct eqos_desc),
				  ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static void eqos_inval_buffer_tegra186(void *buf, size_t size)
{
	unsigned long start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + size, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

void eqos_inval_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void eqos_flush_buffer_tegra186(void *buf, size_t size)
{
	flush_cache((unsigned long)buf, size);
}

void eqos_flush_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static int eqos_mdio_wait_idle(struct eqos_priv *eqos)
{
	return wait_for_bit_le32(&eqos->mac_regs->mdio_address,
				 EQOS_MAC_MDIO_ADDRESS_GB, false,
				 1000000, true);
}

static int eqos_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			  int mdio_reg)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d):\n", __func__, eqos->dev, mdio_addr,
	      mdio_reg);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_READ <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_data);
	val &= EQOS_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int eqos_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg, u16 mdio_val)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d, val=%x):\n", __func__, eqos->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &eqos->mac_regs->mdio_data);

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_WRITE <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static int eqos_start_clks_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_slave_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_slave_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err_disable_clk_slave_bus;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_ptp_ref);
	if (ret < 0) {
		pr_err("clk_enable(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	ret = clk_set_rate(&eqos->clk_ptp_ref, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}
#endif

	debug("%s: OK\n", __func__);
	return 0;

#ifdef CONFIG_CLK
err_disable_clk_ptp_ref:
	clk_disable(&eqos->clk_ptp_ref);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err_disable_clk_slave_bus:
	clk_disable(&eqos->clk_slave_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_start_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	if (clk_valid(&eqos->clk_ck) && !eqos->clk_ck_enabled) {
		ret = clk_enable(&eqos->clk_ck);
		if (ret < 0) {
			pr_err("clk_enable(clk_ck) failed: %d", ret);
			goto err_disable_clk_tx;
		}
		eqos->clk_ck_enabled = true;
	}
#endif

	debug("%s: OK\n", __func__);
	return 0;

#ifdef CONFIG_CLK
err_disable_clk_tx:
	clk_disable(&eqos->clk_tx);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
#endif
}

static int eqos_stop_clks_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_ptp_ref);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
	clk_disable(&eqos->clk_slave_bus);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_clks_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
#endif

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d", ret);
		return ret;
	}

	ret = reset_assert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_assert() failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = reset_deassert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_deassert() failed: %d", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	reset_assert(&eqos->reset_ctl);
	dm_gpio_set_value(&eqos->phy_reset_gpio, 1);

	return 0;
}

static int eqos_calibrate_pads_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	udelay(1);

	setbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_START | EQOS_AUTO_CAL_CONFIG_ENABLE);

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, true, 10, false);
	if (ret) {
		pr_err("calibrate didn't start");
		goto failed;
	}

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, false, 10, false);
	if (ret) {
		pr_err("calibrate didn't finish");
		goto failed;
	}

	ret = 0;

failed:
	clrbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	debug("%s: returns %d\n", __func__, ret);

	return ret;
}

static int eqos_disable_calibration_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_ENABLE);

	return 0;
}

static ulong eqos_get_tick_clk_rate_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_slave_bus);
#else
	return 0;
#endif
}

static ulong eqos_get_tick_clk_rate_stm32(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
#else
	return 0;
#endif
}

static int eqos_set_full_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	return 0;
}

static int eqos_set_half_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int eqos_set_gmii_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_100(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_10(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_FES, EQOS_MAC_CONFIGURATION_PS);

	return 0;
}

static int eqos_set_tx_clk_speed_tegra186(struct udevice *dev)
{
#ifdef CONFIG_CLK
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}
#endif

	return 0;
}

static int eqos_adjust_link(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	bool en_calibration;

	debug("%s(dev=%p):\n", __func__, dev);

	if (eqos->phy->duplex)
		ret = eqos_set_full_duplex(dev);
	else
		ret = eqos_set_half_duplex(dev);
	if (ret < 0) {
		pr_err("eqos_set_*_duplex() failed: %d", ret);
		return ret;
	}

	switch (eqos->phy->speed) {
	case SPEED_1000:
		en_calibration = true;
		ret = eqos_set_gmii_speed(dev);
		break;
	case SPEED_100:
		en_calibration = true;
		ret = eqos_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		en_calibration = false;
		ret = eqos_set_mii_speed_10(dev);
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		pr_err("eqos_set_*mii_speed*() failed: %d", ret);
		return ret;
	}

	if (en_calibration) {
		ret = eqos->config->ops->eqos_calibrate_pads(dev);
		if (ret < 0) {
			pr_err("eqos_calibrate_pads() failed: %d",
			       ret);
			return ret;
		}
	} else {
		ret = eqos->config->ops->eqos_disable_calibration(dev);
		if (ret < 0) {
			pr_err("eqos_disable_calibration() failed: %d",
			       ret);
			return ret;
		}
	}
	ret = eqos->config->ops->eqos_set_tx_clk_speed(dev);
	if (ret < 0) {
		pr_err("eqos_set_tx_clk_speed() failed: %d", ret);
		return ret;
	}

	return 0;
}

static int eqos_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	uint32_t val;

	/*
	 * This function may be called before start() or after stop(). At that
	 * time, on at least some configurations of the EQoS HW, all clocks to
	 * the EQoS HW block will be stopped, and a reset signal applied. If
	 * any register access is attempted in this state, bus timeouts or CPU
	 * hangs may occur. This check prevents that.
	 *
	 * A simple solution to this problem would be to not implement
	 * write_hwaddr(), since start() always writes the MAC address into HW
	 * anyway. However, it is desirable to implement write_hwaddr() to
	 * support the case of SW that runs subsequent to U-Boot which expects
	 * the MAC address to already be programmed into the EQoS registers,
	 * which must happen irrespective of whether the U-Boot user (or
	 * scripts) actually made use of the EQoS device, and hence
	 * irrespective of whether start() was ever called.
	 *
	 * Note that this requirement by subsequent SW is not valid for
	 * Tegra186, and is likely not valid for any non-PCI instantiation of
	 * the EQoS HW block. This function is implemented solely as
	 * future-proofing with the expectation the driver will eventually be
	 * ported to some system where the expectation above is true.
	 */
	if (!eqos->config->reg_access_always_ok && !eqos->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (plat->enetaddr[5] << 8) |
		(plat->enetaddr[4]);
	writel(val, &eqos->mac_regs->address0_high);
	val = (plat->enetaddr[3] << 24) |
		(plat->enetaddr[2] << 16) |
		(plat->enetaddr[1] << 8) |
		(plat->enetaddr[0]);
	writel(val, &eqos->mac_regs->address0_low);

	return 0;
}

static int eqos_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	ret = eqos->config->ops->eqos_get_enetaddr(dev);
	if (ret < 0)
		return ret;

	return !is_valid_ethaddr(pdata->enetaddr);
}

static int eqos_get_phy_addr(struct eqos_priv *priv, struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	int reg;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args)) {
		debug("Failed to find phy-handle");
		return -ENODEV;
	}

	priv->phy_of_node = phandle_args.node;

	reg = ofnode_read_u32_default(phandle_args.node, "reg", 0);

	return reg;
}

static int eqos_start(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret, i;
	ulong rate;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;
	ulong desc_pad;
	ulong addr64;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->tx_desc_idx = 0;
	eqos->rx_desc_idx = 0;

	ret = eqos->config->ops->eqos_start_resets(dev);
	if (ret < 0) {
		pr_err("eqos_start_resets() failed: %d", ret);
		goto err;
	}

	udelay(10);

	eqos->reg_access_ok = true;

	/*
	 * Assert the SWR first, the actually reset the MAC and to latch in
	 * e.g. i.MX8M Plus GPR[1] content, which selects interface mode.
	 */
	setbits_le32(&eqos->dma_regs->mode, EQOS_DMA_MODE_SWR);

	ret = wait_for_bit_le32(&eqos->dma_regs->mode,
				EQOS_DMA_MODE_SWR, false,
				eqos->config->swr_wait, false);
	if (ret) {
		pr_err("EQOS_DMA_MODE_SWR stuck");
		goto err_stop_resets;
	}

	ret = eqos->config->ops->eqos_calibrate_pads(dev);
	if (ret < 0) {
		pr_err("eqos_calibrate_pads() failed: %d", ret);
		goto err_stop_resets;
	}

	if (eqos->config->ops->eqos_get_tick_clk_rate) {
		rate = eqos->config->ops->eqos_get_tick_clk_rate(dev);

		val = (rate / 1000000) - 1;
		writel(val, &eqos->mac_regs->us_tic_counter);
	}

	/*
	 * if PHY was already connected and configured,
	 * don't need to reconnect/reconfigure again
	 */
	if (!eqos->phy) {
		int addr = -1;
		ofnode fixed_node;

		if (IS_ENABLED(CONFIG_PHY_FIXED)) {
			fixed_node = ofnode_find_subnode(dev_ofnode(dev),
							 "fixed-link");
			if (ofnode_valid(fixed_node))
				eqos->phy = fixed_phy_create(dev_ofnode(dev));
		}

		if (!eqos->phy) {
			addr = eqos_get_phy_addr(eqos, dev);
			eqos->phy = phy_connect(eqos->mii, addr, dev,
						eqos->config->interface(dev));
		}

		if (!eqos->phy) {
			pr_err("phy_connect() failed");
			ret = -ENODEV;
			goto err_stop_resets;
		}

		if (eqos->max_speed) {
			ret = phy_set_supported(eqos->phy, eqos->max_speed);
			if (ret) {
				pr_err("phy_set_supported() failed: %d", ret);
				goto err_shutdown_phy;
			}
		}

		eqos->phy->node = eqos->phy_of_node;
		ret = phy_config(eqos->phy);
		if (ret < 0) {
			pr_err("phy_config() failed: %d", ret);
			goto err_shutdown_phy;
		}
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
		pr_err("phy_startup() failed: %d", ret);
		goto err_shutdown_phy;
	}

	if (!eqos->phy->link) {
		pr_err("No link");
		ret = -EAGAIN;
		goto err_shutdown_phy;
	}

	ret = eqos_adjust_link(dev);
	if (ret < 0) {
		pr_err("eqos_adjust_link() failed: %d", ret);
		goto err_shutdown_phy;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_TSF |
		     (EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

	/* Transmit Queue weight */
	writel(0x10, &eqos->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
		     EQOS_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&eqos->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/* r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting */
	tx_fifo_sz = 128 << tx_fifo_sz;
	rx_fifo_sz = 128 << rx_fifo_sz;

	/* Allow platform to override TX/RX fifo size */
	if (eqos->tx_fifo_sz)
		tx_fifo_sz = eqos->tx_fifo_sz;
	if (eqos->rx_fifo_sz)
		rx_fifo_sz = eqos->rx_fifo_sz;

	/* r/tqs is encoded as (n / 256) - 1 */
	tqs = tx_fifo_sz / 256 - 1;
	rqs = rx_fifo_sz / 256 - 1;

	clrsetbits_le32(&eqos->mtl_regs->txq0_operation_mode,
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if (rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;

		setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			     EQOS_MTL_RXQ0_OPERATION_MODE_EHFC);

		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3;	/* Full-3K */
			rfa = 0x1;	/* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0xa;	/* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12;	/* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E;	/* Full-16K */
		}

		clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
				(rfd <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(rfa <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
	}

	/* Configure MAC */

	clrsetbits_le32(&eqos->mac_regs->rxq_ctrl0,
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			eqos->config->config_mac <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Multicast and Broadcast Queue Enable */
	setbits_le32(&eqos->mac_regs->unused_0a4,
		     0x00100000);
	/* enable promise mode */
	setbits_le32(&eqos->mac_regs->unused_004[1],
		     0x1);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     0xffff << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
	/* Assign priority for TX flow control */
	clrbits_le32(&eqos->mac_regs->txq_prty_map0,
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK <<
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
	/* Assign priority for RX flow control */
	clrbits_le32(&eqos->mac_regs->rxq_ctrl2,
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK <<
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT);
	/* Enable flow control */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
	setbits_le32(&eqos->mac_regs->rx_flow_ctrl,
		     EQOS_MAC_RX_FLOW_CTRL_RFE);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_GPSLCE |
			EQOS_MAC_CONFIGURATION_WD |
			EQOS_MAC_CONFIGURATION_JD |
			EQOS_MAC_CONFIGURATION_JE,
			EQOS_MAC_CONFIGURATION_CST |
			EQOS_MAC_CONFIGURATION_ACS);

	eqos_write_hwaddr(dev);

	/* Configure DMA */

	/* Enable OSP mode */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			EQOS_MAX_PACKET_SIZE <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	desc_pad = (eqos->desc_size - sizeof(struct eqos_desc)) /
		   eqos->config->axi_bus_width;

	setbits_le32(&eqos->dma_regs->ch0_control,
		     EQOS_DMA_CH0_CONTROL_PBLX8 |
		     (desc_pad << EQOS_DMA_CH0_CONTROL_DSL_SHIFT));

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;
	clrsetbits_le32(&eqos->dma_regs->ch0_tx_control,
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			pbl << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			8 << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
		EQOS_DMA_SYSBUS_MODE_EAME | EQOS_DMA_SYSBUS_MODE_BLEN16 |
		EQOS_DMA_SYSBUS_MODE_BLEN8 | EQOS_DMA_SYSBUS_MODE_BLEN4;
	writel(val, &eqos->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(eqos->tx_descs, 0, eqos->desc_size * EQOS_DESCRIPTORS_TX);
	memset(eqos->rx_descs, 0, eqos->desc_size * EQOS_DESCRIPTORS_RX);

	for (i = 0; i < EQOS_DESCRIPTORS_TX; i++) {
		struct eqos_desc *tx_desc = eqos_get_desc(eqos, i, false);
		eqos->config->ops->eqos_flush_desc(tx_desc);
	}

	for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
		struct eqos_desc *rx_desc = eqos_get_desc(eqos, i, true);

		addr64 = (ulong)(eqos->rx_dma_buf + (i * EQOS_MAX_PACKET_SIZE));
		rx_desc->des0 = lower_32_bits(addr64);
		rx_desc->des1 = upper_32_bits(addr64);
		rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
		mb();
		eqos->config->ops->eqos_flush_desc(rx_desc);
		eqos->config->ops->eqos_inval_buffer((void *)addr64, EQOS_MAX_PACKET_SIZE);
	}

	addr64 = (ulong)eqos_get_desc(eqos, 0, false);
	writel(upper_32_bits(addr64), &eqos->dma_regs->ch0_txdesc_list_haddress);
	writel(lower_32_bits(addr64), &eqos->dma_regs->ch0_txdesc_list_address);
	writel(EQOS_DESCRIPTORS_TX - 1,
	       &eqos->dma_regs->ch0_txdesc_ring_length);

	addr64 = (ulong)eqos_get_desc(eqos, 0, true);
	writel(upper_32_bits(addr64), &eqos->dma_regs->ch0_rxdesc_list_haddress);
	writel(lower_32_bits(addr64), &eqos->dma_regs->ch0_rxdesc_list_address);
	writel(EQOS_DESCRIPTORS_RX - 1,
	       &eqos->dma_regs->ch0_rxdesc_ring_length);

	/* Enable everything */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);
	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)eqos_get_desc(eqos, EQOS_DESCRIPTORS_RX - 1, true);
	writel(last_rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

	eqos->started = true;

	debug("%s: OK\n", __func__);
	return 0;

err_shutdown_phy:
	phy_shutdown(eqos->phy);
err_stop_resets:
	eqos->config->ops->eqos_stop_resets(dev);
err:
	pr_err("FAILED: %d", ret);
	return ret;
}

static void eqos_stop(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int i;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!eqos->started)
		return;
	eqos->started = false;
	eqos->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->txq0_debug);
		u32 trcsts = (val >> EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & EQOS_MTL_TXQ0_DEBUG_TXQSTS;
		if ((trcsts != 1) && (!txqsts))
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->rxq0_debug);
		u32 prxq = (val >> EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts))
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

	if (eqos->phy) {
		phy_shutdown(eqos->phy);
	}
	eqos->config->ops->eqos_stop_resets(dev);

	debug("%s: OK\n", __func__);
}

static int eqos_send(struct udevice *dev, void *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *tx_desc;
	int i;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	memcpy(eqos->tx_dma_buf, packet, length);
	eqos->config->ops->eqos_flush_buffer(eqos->tx_dma_buf, length);

	tx_desc = eqos_get_desc(eqos, eqos->tx_desc_idx, false);
	eqos->tx_desc_idx++;
	eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;

	tx_desc->des0 = lower_32_bits((ulong)eqos->tx_dma_buf);
	tx_desc->des1 = upper_32_bits((ulong)eqos->tx_dma_buf);
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | length;
	eqos->config->ops->eqos_flush_desc(tx_desc);

	writel((ulong)eqos_get_desc(eqos, eqos->tx_desc_idx, false),
		&eqos->dma_regs->ch0_txdesc_tail_pointer);

	for (i = 0; i < 1000000; i++) {
		eqos->config->ops->eqos_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & EQOS_DESC3_OWN))
			return 0;
		udelay(1);
	}

	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

static int eqos_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *rx_desc;
	int length;

	rx_desc = eqos_get_desc(eqos, eqos->rx_desc_idx, true);
	eqos->config->ops->eqos_inval_desc(rx_desc);
	if (rx_desc->des3 & EQOS_DESC3_OWN)
		return -EAGAIN;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	*packetp = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;
	debug("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

	eqos->config->ops->eqos_inval_buffer(*packetp, length);

	return length;
}

static int eqos_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	u32 idx, idx_mask = eqos->desc_per_cacheline - 1;
	uchar *packet_expected;
	struct eqos_desc *rx_desc;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	packet_expected = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		debug("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	eqos->config->ops->eqos_inval_buffer(packet, length);

	if ((eqos->rx_desc_idx & idx_mask) == idx_mask) {
		for (idx = eqos->rx_desc_idx - idx_mask;
		     idx <= eqos->rx_desc_idx;
		     idx++) {
			ulong addr64;

			rx_desc = eqos_get_desc(eqos, idx, true);
			rx_desc->des0 = 0;
			rx_desc->des1 = 0;
			mb();
			eqos->config->ops->eqos_flush_desc(rx_desc);
			eqos->config->ops->eqos_inval_buffer(packet, length);
			addr64 = (ulong)(eqos->rx_dma_buf + (idx * EQOS_MAX_PACKET_SIZE));
			rx_desc->des0 = lower_32_bits(addr64);
			rx_desc->des1 = upper_32_bits(addr64);
			rx_desc->des2 = 0;
			/*
			 * Make sure that if HW sees the _OWN write below,
			 * it will see all the writes to the rest of the
			 * descriptor too.
			 */
			mb();
			rx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
			eqos->config->ops->eqos_flush_desc(rx_desc);
		}
		writel((ulong)rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);
	}

	eqos->rx_desc_idx++;
	eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

	return 0;
}

static int eqos_probe_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	unsigned int desc_step;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	/* Maximum distance between neighboring descriptors, in Bytes. */
	desc_step = sizeof(struct eqos_desc) +
		    EQOS_DMA_CH0_CONTROL_DSL_MASK * eqos->config->axi_bus_width;
	if (desc_step < ARCH_DMA_MINALIGN) {
		/*
		 * The EQoS hardware implementation cannot place one descriptor
		 * per cacheline, it is necessary to place multiple descriptors
		 * per cacheline in memory and do cache management carefully.
		 */
		eqos->desc_size = BIT(fls(desc_step) - 1);
	} else {
		eqos->desc_size = ALIGN(sizeof(struct eqos_desc),
					(unsigned int)ARCH_DMA_MINALIGN);
	}
	eqos->desc_per_cacheline = ARCH_DMA_MINALIGN / eqos->desc_size;

	eqos->tx_descs = eqos_alloc_descs(eqos, EQOS_DESCRIPTORS_TX);
	if (!eqos->tx_descs) {
		debug("%s: eqos_alloc_descs(tx) failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	eqos->rx_descs = eqos_alloc_descs(eqos, EQOS_DESCRIPTORS_RX);
	if (!eqos->rx_descs) {
		debug("%s: eqos_alloc_descs(rx) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_descs;
	}

	eqos->tx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_MAX_PACKET_SIZE);
	if (!eqos->tx_dma_buf) {
		debug("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, eqos->tx_dma_buf);

	eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
	if (!eqos->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

	eqos->config->ops->eqos_inval_buffer(eqos->rx_dma_buf,
			EQOS_MAX_PACKET_SIZE * EQOS_DESCRIPTORS_RX);

	debug("%s: OK\n", __func__);
	return 0;

err_free_tx_dma_buf:
	free(eqos->tx_dma_buf);
err_free_descs:
	eqos_free_descs(eqos->rx_descs);
err_free_tx_descs:
	eqos_free_descs(eqos->tx_descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	free(eqos->rx_dma_buf);
	free(eqos->tx_dma_buf);
	eqos_free_descs(eqos->rx_descs);
	eqos_free_descs(eqos->tx_descs);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_probe_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = reset_get_by_name(dev, "eqos", &eqos->reset_ctl);
	if (ret) {
		pr_err("reset_get_by_name(rst) failed: %d", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		pr_err("gpio_request_by_name(phy reset) failed: %d", ret);
		goto err_free_reset_eqos;
	}

	ret = clk_get_by_name(dev, "slave_bus", &eqos->clk_slave_bus);
	if (ret) {
		pr_err("clk_get_by_name(slave_bus) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "master_bus", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "ptp_ref", &eqos->clk_ptp_ref);
	if (ret) {
		pr_err("clk_get_by_name(ptp_ref) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_gpio_phy_reset:
	dm_gpio_free(dev, &eqos->phy_reset_gpio);
err_free_reset_eqos:
	reset_free(&eqos->reset_ctl);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_probe_resources_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	phy_interface_t interface;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = board_interface_eth_init(dev, interface);
	if (ret)
		return -EINVAL;

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_probe;
	}

	/*  Get ETH_CLK clocks (optional) */
	ret = clk_get_by_name(dev, "eth-ck", &eqos->clk_ck);
	if (ret)
		pr_warn("No phy clock provided %d", ret);

	debug("%s: OK\n", __func__);
	return 0;

err_probe:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static phy_interface_t eqos_get_interface_tegra186(const struct udevice *dev)
{
	return PHY_INTERFACE_MODE_MII;
}

static int eqos_remove_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	dm_gpio_free(dev, &eqos->phy_reset_gpio);
	reset_free(&eqos->reset_ctl);

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_remove_resources_stm32(struct udevice *dev)
{
	debug("%s(dev=%p):\n", __func__, dev);
	return 0;
}

static int eqos_probe(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->dev = dev;
	eqos->config = (void *)dev_get_driver_data(dev);

	eqos->regs = dev_read_addr(dev);
	if (eqos->regs == FDT_ADDR_T_NONE) {
		pr_err("dev_read_addr() failed");
		return -ENODEV;
	}
	eqos->mac_regs = (void *)(eqos->regs + EQOS_MAC_REGS_BASE);
	eqos->mtl_regs = (void *)(eqos->regs + EQOS_MTL_REGS_BASE);
	eqos->dma_regs = (void *)(eqos->regs + EQOS_DMA_REGS_BASE);
	eqos->tegra186_regs = (void *)(eqos->regs + EQOS_TEGRA186_REGS_BASE);

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	ret = eqos_probe_resources_core(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources_core() failed: %d", ret);
		return ret;
	}

	ret = eqos->config->ops->eqos_probe_resources(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources() failed: %d", ret);
		goto err_remove_resources_core;
	}

	ret = eqos->config->ops->eqos_start_clks(dev);
	if (ret < 0) {
		pr_err("eqos_start_clks() failed: %d", ret);
		goto err_remove_resources_tegra;
	}

#ifdef CONFIG_DM_ETH_PHY
	eqos->mii = eth_phy_get_mdio_bus(dev);
#endif
	if (!eqos->mii) {
		eqos->mii = mdio_alloc();
		if (!eqos->mii) {
			pr_err("mdio_alloc() failed");
			ret = -ENOMEM;
			goto err_stop_clks;
		}
		eqos->mii->read = eqos_mdio_read;
		eqos->mii->write = eqos_mdio_write;
		eqos->mii->priv = eqos;
		strcpy(eqos->mii->name, dev->name);

		ret = mdio_register(eqos->mii);
		if (ret < 0) {
			pr_err("mdio_register() failed: %d", ret);
			goto err_free_mdio;
		}
	}

#ifdef CONFIG_DM_ETH_PHY
	eth_phy_set_mdio_bus(dev, eqos->mii);
#endif

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
	mdio_free(eqos->mii);
err_stop_clks:
	eqos->config->ops->eqos_stop_clks(dev);
err_remove_resources_tegra:
	eqos->config->ops->eqos_remove_resources(dev);
err_remove_resources_core:
	eqos_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	mdio_unregister(eqos->mii);
	mdio_free(eqos->mii);
	eqos->config->ops->eqos_stop_clks(dev);
	eqos->config->ops->eqos_remove_resources(dev);

	eqos_remove_resources_core(dev);

	debug("%s: OK\n", __func__);
	return 0;
}

int eqos_null_ops(struct udevice *dev)
{
	return 0;
}

static const struct eth_ops eqos_ops = {
	.start = eqos_start,
	.stop = eqos_stop,
	.send = eqos_send,
	.recv = eqos_recv,
	.free_pkt = eqos_free_pkt,
	.write_hwaddr = eqos_write_hwaddr,
	.read_rom_hwaddr	= eqos_read_rom_hwaddr,
};

static struct eqos_ops eqos_tegra186_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_tegra186,
	.eqos_flush_buffer = eqos_flush_buffer_tegra186,
	.eqos_probe_resources = eqos_probe_resources_tegra186,
	.eqos_remove_resources = eqos_remove_resources_tegra186,
	.eqos_stop_resets = eqos_stop_resets_tegra186,
	.eqos_start_resets = eqos_start_resets_tegra186,
	.eqos_stop_clks = eqos_stop_clks_tegra186,
	.eqos_start_clks = eqos_start_clks_tegra186,
	.eqos_calibrate_pads = eqos_calibrate_pads_tegra186,
	.eqos_disable_calibration = eqos_disable_calibration_tegra186,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_tegra186,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_tegra186
};

static const struct eqos_config __maybe_unused eqos_tegra186_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 10,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_20_35,
	.axi_bus_width = EQOS_AXI_WIDTH_128,
	.interface = eqos_get_interface_tegra186,
	.ops = &eqos_tegra186_ops
};

static struct eqos_ops eqos_stm32_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_stm32,
	.eqos_remove_resources = eqos_remove_resources_stm32,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_null_ops,
	.eqos_stop_clks = eqos_stop_clks_stm32,
	.eqos_start_clks = eqos_start_clks_stm32,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_null_ops,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32
};

static const struct eqos_config __maybe_unused eqos_stm32_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_stm32_ops
};

static const struct udevice_id eqos_ids[] = {
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_TEGRA186)
	{
		.compatible = "nvidia,tegra186-eqos",
		.data = (ulong)&eqos_tegra186_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_STM32)
	{
		.compatible = "st,stm32mp1-dwmac",
		.data = (ulong)&eqos_stm32_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_IMX)
	{
		.compatible = "nxp,imx8mp-dwmac-eqos",
		.data = (ulong)&eqos_imx_config
	},
	{
		.compatible = "nxp,imx93-dwmac-eqos",
		.data = (ulong)&eqos_imx_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_ROCKCHIP)
	{
		.compatible = "rockchip,rk3568-gmac",
		.data = (ulong)&eqos_rockchip_config
	},
	{
		.compatible = "rockchip,rk3588-gmac",
		.data = (ulong)&eqos_rockchip_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_QCOM)
	{
		.compatible = "qcom,qcs404-ethqos",
		.data = (ulong)&eqos_qcom_config
	},
#endif
#if IS_ENABLED(CONFIG_DWC_ETH_QOS_STARFIVE)
	{
		.compatible = "starfive,jh7110-dwmac",
		.data = (ulong)&eqos_jh7110_config
	},
#endif
	{ }
};

U_BOOT_DRIVER(eth_eqos) = {
	.name = "eth_eqos",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(eqos_ids),
	.probe = eqos_probe,
	.remove = eqos_remove,
	.ops = &eqos_ops,
	.priv_auto	= sizeof(struct eqos_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
