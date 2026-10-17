// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Intel Corp.
 * Copyright 2019 Google LLC
 *
 * Taken partly from coreboot gpio.c
 *
 * Pinctrl is modelled as a separate device-tree node and device for each
 * 'community' (basically a set of GPIOs). The separate devices work together
 * and many functions permit any PINCTRL device to be provided as a parameter,
 * since the pad numbering is unique across all devices.
 *
 * Each pinctrl has a single child GPIO device to handle GPIO access and
 * therefore there is a simple GPIO driver included in this file.
 */

#define LOG_CATEGORY UCLASS_GPIO

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <log.h>
#include <malloc.h>
#include <p2sb.h>
#include <spl.h>
#include <asm-generic/gpio.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_pinctrl_defs.h>
#include <asm/arch/gpio.h>
#include <asm/itss.h>
#include <dm/device-internal.h>
#include <dt-bindings/gpio/gpio.h>
#include <linux/err.h>

#define GPIO_DW_SIZE(x)			(sizeof(u32) * (x))
#define PAD_CFG_OFFSET(x, dw_num)	((x) + GPIO_DW_SIZE(dw_num))
#define PAD_CFG0_OFFSET(x)		PAD_CFG_OFFSET(x, 0)
#define PAD_CFG1_OFFSET(x)		PAD_CFG_OFFSET(x, 1)

#define MISCCFG_GPE0_DW0_SHIFT 8
#define MISCCFG_GPE0_DW0_MASK (0xf << MISCCFG_GPE0_DW0_SHIFT)
#define MISCCFG_GPE0_DW1_SHIFT 12
#define MISCCFG_GPE0_DW1_MASK (0xf << MISCCFG_GPE0_DW1_SHIFT)
#define MISCCFG_GPE0_DW2_SHIFT 16
#define MISCCFG_GPE0_DW2_MASK (0xf << MISCCFG_GPE0_DW2_SHIFT)

#define GPI_SMI_STS_OFFSET(comm, group) ((comm)->gpi_smi_sts_reg_0 +	\
				((group) * sizeof(u32)))
#define GPI_SMI_EN_OFFSET(comm, group) ((comm)->gpi_smi_en_reg_0 +	\
				((group) * sizeof(u32)))
#define GPI_IS_OFFSET(comm, group) ((comm)->gpi_int_sts_reg_0 +	\
				((group) * sizeof(uint32_t)))
#define GPI_IE_OFFSET(comm, group) ((comm)->gpi_int_en_reg_0 +	\
				((group) * sizeof(uint32_t)))

/**
 * relative_pad_in_comm() - Get the relative position of a GPIO
 *
 * This finds the position of a GPIO within a community
 *
 * @comm: Community to search
 * @gpio: Pad number to look up (assumed to be valid)
 * Return: offset, 0 for first GPIO in community
 */
static size_t relative_pad_in_comm(const struct pad_community *comm,
				   uint gpio)
{
	return gpio - comm->first_pad;
}

/**
 * pinctrl_group_index() - Find group for a a pad
 *
 * Find the group within the community that the pad is a part of
 *
 * @comm: Community to search
 * @relative_pad: Pad to look up
 * Return: group number if found (see community_n_groups, etc.), or
 *	-ESPIPE if no groups, or -ENOENT if not found
 */
static int pinctrl_group_index(const struct pad_community *comm,
			       uint relative_pad)
{
	int i;

	if (!comm->groups)
		return -ESPIPE;

	/* find the base pad number for this pad's group */
	for (i = 0; i < comm->num_groups; i++) {
		if (relative_pad >= comm->groups[i].first_pad &&
		    relative_pad < comm->groups[i].first_pad +
		    comm->groups[i].size)
			return i;
	}

	return -ENOENT;
}

static int pinctrl_group_index_scaled(const struct pad_community *comm,
				      uint relative_pad, size_t scale)
{
	int ret;

	ret = pinctrl_group_index(comm, relative_pad);
	if (ret < 0)
		return ret;

	return ret * scale;
}

static int pinctrl_within_group(const struct pad_community *comm,
				uint relative_pad)
{
	int ret;

	ret = pinctrl_group_index(comm, relative_pad);
	if (ret < 0)
		return ret;

	return relative_pad - comm->groups[ret].first_pad;
}

static u32 pinctrl_bitmask_within_group(const struct pad_community *comm,
					uint relative_pad)
{
	return 1U << pinctrl_within_group(comm, relative_pad);
}

/**
 * pinctrl_get_device() - Find the device for a particular pad
 *
 * Each pinctr, device is attached to one community and this supports a number
 * of pads. This function finds the device which controls a particular pad.
 *
 * @pad: Pad to check
 * @devp: Returns the device for that pad
 * Return: 0 if OK, -ENOTBLK if no device was found for the given pin
 */
static int pinctrl_get_device(uint pad, struct udevice **devp)
{
	struct udevice *dev;

	/*
	 * We have to probe each one of these since the community link is only
	 * attached in intel_pinctrl_of_to_plat().
	 */
	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		struct intel_pinctrl_priv *priv = dev_get_priv(dev);
		const struct pad_community *comm = priv->comm;

		if (pad >= comm->first_pad && pad <= comm->last_pad) {
			*devp = dev;
			return 0;
		}
	}
	log_debug("pad %d not found\n", pad);

	return -ENOTBLK;
}

int intel_pinctrl_get_pad(uint pad, struct udevice **devp, uint *offsetp)
{
	const struct pad_community *comm;
	struct intel_pinctrl_priv *priv;
	struct udevice *dev;
	int ret;

	ret = pinctrl_get_device(pad, &dev);
	if (ret)
		return log_msg_ret("pad", ret);
	priv = dev_get_priv(dev);
	comm = priv->comm;
	*devp = dev;
	*offsetp = relative_pad_in_comm(comm, pad);

	return 0;
}

static int pinctrl_configure_owner(struct udevice *dev,
				   const struct pad_config *cfg,
				   const struct pad_community *comm)
{
	u32 hostsw_own;
	u16 hostsw_own_offset;
	int pin;
	int ret;

	pin = relative_pad_in_comm(comm, cfg->pad);

	/*
	 * Based on the gpio pin number configure the corresponding bit in
	 * HOSTSW_OWN register. Value of 0x1 indicates GPIO Driver onwership.
	 */
	hostsw_own_offset = comm->host_own_reg_0;
	ret = pinctrl_group_index_scaled(comm, pin, sizeof(u32));
	if (ret < 0)
		return ret;
	hostsw_own_offset += ret;

	hostsw_own = pcr_read32(dev, hostsw_own_offset);

	/*
	 *The 4th bit in pad_config 1 (RO) is used to indicate if the pad
	 * needs GPIO driver ownership.  Set the bit if GPIO driver ownership
	 * requested, otherwise clear the bit.
	 */
	if (cfg->pad_config[1] & PAD_CFG1_GPIO_DRIVER)
		hostsw_own |= pinctrl_bitmask_within_group(comm, pin);
	else
		hostsw_own &= ~pinctrl_bitmask_within_group(comm, pin);

	pcr_write32(dev, hostsw_own_offset, hostsw_own);

	return 0;
}

static int gpi_enable_smi(struct udevice *dev, const struct pad_config *cfg,
			  const struct pad_community *comm)
{
	u32 value;
	u16 sts_reg;
	u16 en_reg;
	int group;
	int pin;
	int ret;

	if ((cfg->pad_config[0] & PAD_CFG0_ROUTE_SMI) != PAD_CFG0_ROUTE_SMI)
		return 0;

	pin = relative_pad_in_comm(comm, cfg->pad);
	ret = pinctrl_group_index(comm, pin);
	if (ret < 0)
		return ret;
	group = ret;

	sts_reg = GPI_SMI_STS_OFFSET(comm, group);
	value = pcr_read32(dev, sts_reg);
	/* Write back 1 to reset the sts bits */
	pcr_write32(dev, sts_reg, value);

	/* Set enable bits */
	en_reg = GPI_SMI_EN_OFFSET(comm, group);
	pcr_setbits32(dev, en_reg, pinctrl_bitmask_within_group(comm, pin));

	return 0;
}

static int pinctrl_configure_itss(struct udevice *dev,
				  const struct pad_config *cfg,
				  uint pad_cfg_offset)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);

	if (!priv->itss_pol_cfg)
		return -ENOSYS;

	int irq;

	/*
	 * Set up ITSS polarity if pad is routed to APIC.
	 *
	 * The ITSS takes only active high interrupt signals. Therefore,
	 * if the pad configuration indicates an inversion assume the
	 * intent is for the ITSS polarity. Before forwarding on the
	 * request to the APIC there's an inversion setting for how the
	 * signal is forwarded to the APIC. Honor the inversion setting
	 * in the GPIO pad configuration so that a hardware active low
	 * signal looks that way to the APIC (double inversion).
	 */
	if (!(cfg->pad_config[0] & PAD_CFG0_ROUTE_IOAPIC))
		return 0;

	irq = pcr_read32(dev, PAD_CFG1_OFFSET(pad_cfg_offset));
	irq &= PAD_CFG1_IRQ_MASK;
	if (!irq) {
		if (spl_phase() > PHASE_TPL)
			log_err("GPIO %u doesn't support APIC routing\n",
				cfg->pad);

		return -EPROTONOSUPPORT;
	}
	irq_set_polarity(priv->itss, irq,
			 cfg->pad_config[0] & PAD_CFG0_RX_POL_INVERT);

	return 0;
}

/* Number of DWx config registers can be different for different SOCs */
static uint pad_config_offset(struct intel_pinctrl_priv *priv, uint pad)
{
	const struct pad_community *comm = priv->comm;
	size_t offset;

	offset = relative_pad_in_comm(comm, pad);
	offset *= GPIO_DW_SIZE(priv->num_cfgs);

	return offset + comm->pad_cfg_base;
}

static int pinctrl_pad_reset_config_override(const struct pad_community *comm,
					     u32 config_value)
{
	const struct reset_mapping *rst_map = comm->reset_map;
	int i;

	/* Logical reset values equal chipset values */
	if (!rst_map || !comm->num_reset_vals)
		return config_value;

	for (i = 0; i < comm->num_reset_vals; i++, rst_map++) {
		if ((config_value & PAD_CFG0_RESET_MASK) == rst_map->logical) {
			config_value &= ~PAD_CFG0_RESET_MASK;
			config_value |= rst_map->chipset;

			return config_value;
		}
	}
	if (spl_phase() > PHASE_TPL)
		log_err("Logical-to-Chipset mapping not found\n");

	return -ENOENT;
}

static const int mask[4] = {
	PAD_CFG0_TX_STATE |
	PAD_CFG0_TX_DISABLE | PAD_CFG0_RX_DISABLE | PAD_CFG0_MODE_MASK |
	PAD_CFG0_ROUTE_MASK | PAD_CFG0_RXTENCFG_MASK |
	PAD_CFG0_RXINV_MASK | PAD_CFG0_PREGFRXSEL |
	PAD_CFG0_TRIG_MASK | PAD_CFG0_RXRAW1_MASK |
	PAD_CFG0_RXPADSTSEL_MASK | PAD_CFG0_RESET_MASK,

#ifdef CONFIG_INTEL_PINCTRL_IOSTANDBY
	PAD_CFG1_IOSTERM_MASK | PAD_CFG1_PULL_MASK | PAD_CFG1_IOSSTATE_MASK,
#else
	PAD_CFG1_IOSTERM_MASK | PAD_CFG1_PULL_MASK,
#endif

	PAD_CFG2_DEBOUNCE_MASK,

	0,
};

/**
 * pinctrl_configure_pad() - Configure a pad
 *
 * @dev: Pinctrl device containing the pad (see pinctrl_get_device())
 * @cfg: Configuration to apply
 * Return: 0 if OK, -ve on error
 */
static int pinctrl_configure_pad(struct udevice *dev,
				 const struct pad_config *cfg)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	const struct pad_community *comm = priv->comm;
	uint config_offset;
	u32 pad_conf, soc_pad_conf;
	int ret;
	int i;

	if (IS_ERR(comm))
		return PTR_ERR(comm);
	config_offset = pad_config_offset(priv, cfg->pad);
	for (i = 0; i < priv->num_cfgs; i++) {
		pad_conf = pcr_read32(dev, PAD_CFG_OFFSET(config_offset, i));

		soc_pad_conf = cfg->pad_config[i];
		if (i == 0) {
			ret = pinctrl_pad_reset_config_override(comm,
								soc_pad_conf);
			if (ret < 0)
				return ret;
			soc_pad_conf = ret;
		}
		soc_pad_conf &= mask[i];
		soc_pad_conf |= pad_conf & ~mask[i];

		log_debug("pinctrl_padcfg [0x%02x, %02zd] DW%d [0x%08x : 0x%08x : 0x%08x]\n",
			  comm->port, relative_pad_in_comm(comm, cfg->pad), i,
			  pad_conf,/* old value */
			  /* value passed from pinctrl table */
			  cfg->pad_config[i],
			  soc_pad_conf); /*new value*/
		pcr_write32(dev, PAD_CFG_OFFSET(config_offset, i),
			    soc_pad_conf);
	}
	ret = pinctrl_configure_itss(dev, cfg, config_offset);
	if (ret && ret != -ENOSYS)
		return log_msg_ret("itss config failed", ret);
	ret = pinctrl_configure_owner(dev, cfg, comm);
	if (ret)
		return ret;
	ret = gpi_enable_smi(dev, cfg, comm);
	if (ret)
		return ret;

	return 0;
}

u32 intel_pinctrl_get_config_reg_offset(struct udevice *dev, uint offset)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	const struct pad_community *comm = priv->comm;
	uint config_offset;

	assert(device_get_uclass_id(dev) == UCLASS_PINCTRL);
	config_offset = comm->pad_cfg_base + offset *
		 GPIO_DW_SIZE(priv->num_cfgs);

	return config_offset;
}

u32 intel_pinctrl_get_config_reg_addr(struct udevice *dev, uint offset)
{
	uint config_offset = intel_pinctrl_get_config_reg_offset(dev, offset);

	return (u32)(ulong)pcr_reg_address(dev, config_offset);
}

u32 intel_pinctrl_get_config_reg(struct udevice *dev, uint offset)
{
	uint config_offset = intel_pinctrl_get_config_reg_offset(dev, offset);

	return pcr_read32(dev, config_offset);
}

int intel_pinctrl_get_acpi_pin(struct udevice *dev, uint offset)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	const struct pad_community *comm = priv->comm;
	int group;

	if (IS_ENABLED(CONFIG_INTEL_PINCTRL_MULTI_ACPI_DEVICES))
		return offset;
	group = pinctrl_group_index(comm, offset);

	/* If pad base is not set then use GPIO number as ACPI pin number */
	if (comm->groups[group].acpi_pad_base == PAD_BASE_NONE)
		return comm->first_pad + offset;

	/*
	 * If this group has a non-zero pad base then compute the ACPI pin
	 * number from the pad base and the relative pad in the group.
	 */
	return comm->groups[group].acpi_pad_base +
		 pinctrl_within_group(comm, offset);
}

int pinctrl_route_gpe(struct udevice *itss, uint gpe0b, uint gpe0c, uint gpe0d)
{
	struct udevice *pinctrl_dev;
	u32 misccfg_value;
	u32 misccfg_clr;
	int ret;

	/*
	 * Get the group here for community specific MISCCFG register.
	 * If any of these returns -1 then there is some error in devicetree
	 * where the group is probably hardcoded and does not comply with the
	 * PMC group defines. So we return from here and MISCFG is set to
	 * default.
	 */
	ret = irq_route_pmc_gpio_gpe(itss, gpe0b);
	if (ret)
		return ret;
	gpe0b = ret;

	ret = irq_route_pmc_gpio_gpe(itss, gpe0c);
	if (ret)
		return ret;
	gpe0c = ret;

	ret = irq_route_pmc_gpio_gpe(itss, gpe0d);
	if (ret)
		return ret;
	gpe0d = ret;

	misccfg_value = gpe0b << MISCCFG_GPE0_DW0_SHIFT;
	misccfg_value |= gpe0c << MISCCFG_GPE0_DW1_SHIFT;
	misccfg_value |= gpe0d << MISCCFG_GPE0_DW2_SHIFT;

	/* Program GPIO_MISCCFG */
	misccfg_clr = MISCCFG_GPE0_DW2_MASK | MISCCFG_GPE0_DW1_MASK |
		MISCCFG_GPE0_DW0_MASK;

	log_debug("misccfg_clr:%x misccfg_value:%x\n", misccfg_clr,
		  misccfg_value);
	uclass_foreach_dev_probe(UCLASS_PINCTRL, pinctrl_dev) {
		pcr_clrsetbits32(pinctrl_dev, GPIO_MISCCFG, misccfg_clr,
				 misccfg_value);
	}

	return 0;
}

int pinctrl_gpi_clear_int_cfg(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_PINCTRL, &uc);
	if (ret)
		return log_msg_ret("pinctrl uc", ret);
	uclass_foreach_dev(dev, uc) {
		struct intel_pinctrl_priv *priv = dev_get_priv(dev);
		const struct pad_community *comm = priv->comm;
		uint sts_value;
		int group;

		for (group = 0; group < comm->num_gpi_regs; group++) {
			/* Clear the enable register */
			pcr_write32(dev, GPI_IE_OFFSET(comm, group), 0);

			/* Read and clear the set status register bits*/
			sts_value = pcr_read32(dev,
					       GPI_IS_OFFSET(comm, group));
			pcr_write32(dev, GPI_IS_OFFSET(comm, group), sts_value);
		}
	}

	return 0;
}

int pinctrl_config_pads(struct udevice *dev, u32 *pads, int pads_count)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	const u32 *ptr;
	int i;

	log_debug("%s: pads_count=%d\n", __func__, pads_count);
	for (ptr = pads, i = 0; i < pads_count;
	     ptr += 1 + priv->num_cfgs, i++) {
		struct udevice *pad_dev = NULL;
		struct pad_config *cfg;
		int ret;

		cfg = (struct pad_config *)ptr;
		ret = pinctrl_get_device(cfg->pad, &pad_dev);
		if (ret)
			return ret;
		ret = pinctrl_configure_pad(pad_dev, cfg);
		if (ret)
			return ret;
	}

	return 0;
}

int pinctrl_read_pads(struct udevice *dev, ofnode node, const char *prop,
		      u32 **padsp, int *pad_countp)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	u32 *pads;
	int size;
	int ret;

	*padsp = NULL;
	*pad_countp = 0;
	size = ofnode_read_size(node, prop);
	if (size < 0)
		return 0;

	pads = malloc(size);
	if (!pads)
		return -ENOMEM;
	size /= sizeof(fdt32_t);
	ret = ofnode_read_u32_array(node, prop, pads, size);
	if (ret) {
		free(pads);
		return ret;
	}
	*pad_countp = size / (1 + priv->num_cfgs);
	*padsp = pads;

	return 0;
}

int pinctrl_count_pads(struct udevice *dev, u32 *pads, int size)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	int count = 0;
	int i;

	for (i = 0; i < size;) {
		u32 val;
		int j;

		for (val = j = 0; j < priv->num_cfgs + 1; j++)
			val |= pads[i + j];
		if (!val)
			break;
		count++;
		i += priv->num_cfgs + 1;
	}

	return count;
}

int pinctrl_config_pads_for_node(struct udevice *dev, ofnode node)
{
	int pads_count;
	u32 *pads;
	int ret;

	if (device_get_uclass_id(dev) != UCLASS_PINCTRL)
		return log_msg_ret("uclass", -EPROTONOSUPPORT);
	ret = pinctrl_read_pads(dev, node, "pads", &pads, &pads_count);
	if (ret)
		return log_msg_ret("no pads", ret);
	ret = pinctrl_config_pads(dev, pads, pads_count);
	free(pads);
	if (ret)
		return log_msg_ret("pad config", ret);

	return 0;
}

int intel_pinctrl_of_to_plat(struct udevice *dev,
			     const struct pad_community *comm, int num_cfgs)
{
	struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);

	if (!comm) {
		if (spl_phase() > PHASE_TPL)
			log_err("Cannot find community for pid %d\n",
				pplat->pid);
		return -EDOM;
	}
	priv->comm = comm;
	priv->num_cfgs = num_cfgs;

	return 0;
}

int intel_pinctrl_probe(struct udevice *dev)
{
	struct intel_pinctrl_priv *priv = dev_get_priv(dev);
	int ret;

	priv->itss_pol_cfg = true;
	ret = irq_first_device_type(X86_IRQT_ITSS, &priv->itss);
	if (ret)
		return log_msg_ret("Cannot find ITSS", ret);

	return 0;
}

const struct pinctrl_ops intel_pinctrl_ops = {
	/* No operations are supported, but DM expects this to be present */
};
