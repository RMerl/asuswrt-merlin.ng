// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <errno.h>
#include <wait_bit.h>
#include <dm.h>
#include <reset-uclass.h>
#include <regmap.h>
#include <syscon.h>
#include <dt-bindings/reset/stih407-resets.h>

DECLARE_GLOBAL_DATA_PTR;

struct sti_reset {
	const struct syscfg_reset_controller_data *data;
};

/**
 * Reset channel description for a system configuration register based
 * reset controller.
 *
 * @compatible: Compatible string of the syscon containing this
 *              channel's control and ack (status) bits.
 * @reset_offset: Reset register offset in sysconf bank.
 * @reset_bit: Bit number in reset register.
 * @ack_offset: Ack reset register offset in syscon bank.
 * @ack_bit: Bit number in Ack reset register.
 * @deassert_cnt: incremented when reset is deasserted, reset can only be
 *                asserted when equal to 0
 */

struct syscfg_reset_channel_data {
	const char *compatible;
	int reset_offset;
	int reset_bit;
	int ack_offset;
	int ack_bit;
	int deassert_cnt;
};

/**
 * Description of a system configuration register based reset controller.
 *
 * @wait_for_ack: The controller will wait for reset assert and de-assert to
 *                be "ack'd" in a channel's ack field.
 * @active_low: Are the resets in this controller active low, i.e. clearing
 *              the reset bit puts the hardware into reset.
 * @nr_channels: The number of reset channels in this controller.
 * @channels: An array of reset channel descriptions.
 */
struct syscfg_reset_controller_data {
	bool wait_for_ack;
	bool active_low;
	int nr_channels;
	struct syscfg_reset_channel_data *channels;
};

/* STiH407 Peripheral powerdown definitions. */
static const char stih407_core[] = "st,stih407-core-syscfg";
static const char stih407_sbc_reg[] = "st,stih407-sbc-reg-syscfg";
static const char stih407_lpm[] = "st,stih407-lpm-syscfg";

#define _SYSCFG_RST_CH(_c, _rr, _rb, _ar, _ab)		\
	{ .compatible	= _c,				\
	  .reset_offset	= _rr,				\
	  .reset_bit	= _rb,				\
	  .ack_offset	= _ar,				\
	  .ack_bit	= _ab,				}

#define _SYSCFG_RST_CH_NO_ACK(_c, _rr, _rb)		\
	{ .compatible	= _c,				\
	  .reset_offset	= _rr,				\
	  .reset_bit	= _rb,				}

#define STIH407_SRST_CORE(_reg, _bit) \
	_SYSCFG_RST_CH_NO_ACK(stih407_core, _reg, _bit)

#define STIH407_SRST_SBC(_reg, _bit) \
	_SYSCFG_RST_CH_NO_ACK(stih407_sbc_reg, _reg, _bit)

#define STIH407_SRST_LPM(_reg, _bit) \
	_SYSCFG_RST_CH_NO_ACK(stih407_lpm, _reg, _bit)

#define STIH407_PDN_0(_bit) \
	_SYSCFG_RST_CH(stih407_core, SYSCFG_5000, _bit, SYSSTAT_5500, _bit)
#define STIH407_PDN_1(_bit) \
	_SYSCFG_RST_CH(stih407_core, SYSCFG_5001, _bit, SYSSTAT_5501, _bit)
#define STIH407_PDN_ETH(_bit, _stat) \
	_SYSCFG_RST_CH(stih407_sbc_reg, SYSCFG_4032, _bit, SYSSTAT_4520, _stat)

/* Powerdown requests control 0 */
#define SYSCFG_5000	0x0
#define SYSSTAT_5500	0x7d0
/* Powerdown requests control 1 (High Speed Links) */
#define SYSCFG_5001	0x4
#define SYSSTAT_5501	0x7d4

/* Ethernet powerdown/status/reset */
#define SYSCFG_4032	0x80
#define SYSSTAT_4520	0x820
#define SYSCFG_4002	0x8

static struct syscfg_reset_channel_data stih407_powerdowns[] = {
	[STIH407_EMISS_POWERDOWN] = STIH407_PDN_0(1),
	[STIH407_NAND_POWERDOWN] = STIH407_PDN_0(0),
	[STIH407_USB3_POWERDOWN] = STIH407_PDN_1(6),
	[STIH407_USB2_PORT1_POWERDOWN] = STIH407_PDN_1(5),
	[STIH407_USB2_PORT0_POWERDOWN] = STIH407_PDN_1(4),
	[STIH407_PCIE1_POWERDOWN] = STIH407_PDN_1(3),
	[STIH407_PCIE0_POWERDOWN] = STIH407_PDN_1(2),
	[STIH407_SATA1_POWERDOWN] = STIH407_PDN_1(1),
	[STIH407_SATA0_POWERDOWN] = STIH407_PDN_1(0),
	[STIH407_ETH1_POWERDOWN] = STIH407_PDN_ETH(0, 2),
};

/* Reset Generator control 0/1 */
#define SYSCFG_5128	0x200
#define SYSCFG_5131	0x20c
#define SYSCFG_5132	0x210

#define LPM_SYSCFG_1	0x4	/* Softreset IRB & SBC UART */

static struct syscfg_reset_channel_data stih407_softresets[] = {
	[STIH407_ETH1_SOFTRESET] = STIH407_SRST_SBC(SYSCFG_4002, 4),
	[STIH407_MMC1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 3),
	[STIH407_USB2_PORT0_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 28),
	[STIH407_USB2_PORT1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 29),
	[STIH407_PICOPHY_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 30),
	[STIH407_IRB_SOFTRESET] = STIH407_SRST_LPM(LPM_SYSCFG_1, 6),
	[STIH407_PCIE0_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 6),
	[STIH407_PCIE1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 15),
	[STIH407_SATA0_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 7),
	[STIH407_SATA1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 16),
	[STIH407_MIPHY0_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 4),
	[STIH407_MIPHY1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 13),
	[STIH407_MIPHY2_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 22),
	[STIH407_SATA0_PWR_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 5),
	[STIH407_SATA1_PWR_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 14),
	[STIH407_DELTA_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 3),
	[STIH407_BLITTER_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 10),
	[STIH407_HDTVOUT_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 11),
	[STIH407_HDQVDP_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 12),
	[STIH407_VDP_AUX_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 14),
	[STIH407_COMPO_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 15),
	[STIH407_HDMI_TX_PHY_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 21),
	[STIH407_JPEG_DEC_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 23),
	[STIH407_VP8_DEC_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 24),
	[STIH407_GPU_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 30),
	[STIH407_HVA_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 0),
	[STIH407_ERAM_HVA_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5132, 1),
	[STIH407_LPM_SOFTRESET] = STIH407_SRST_SBC(SYSCFG_4002, 2),
	[STIH407_KEYSCAN_SOFTRESET] = STIH407_SRST_LPM(LPM_SYSCFG_1, 8),
	[STIH407_ST231_AUD_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 26),
	[STIH407_ST231_DMU_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 27),
	[STIH407_ST231_GP0_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5131, 28),
	[STIH407_ST231_GP1_SOFTRESET] = STIH407_SRST_CORE(SYSCFG_5128, 2),
};

/* PicoPHY reset/control */
#define SYSCFG_5061	0x0f4

static struct syscfg_reset_channel_data stih407_picophyresets[] = {
	[STIH407_PICOPHY0_RESET] = STIH407_SRST_CORE(SYSCFG_5061, 5),
	[STIH407_PICOPHY1_RESET] = STIH407_SRST_CORE(SYSCFG_5061, 6),
	[STIH407_PICOPHY2_RESET] = STIH407_SRST_CORE(SYSCFG_5061, 7),
};

static const struct
syscfg_reset_controller_data stih407_powerdown_controller = {
	.wait_for_ack = true,
	.nr_channels = ARRAY_SIZE(stih407_powerdowns),
	.channels = stih407_powerdowns,
};

static const struct
syscfg_reset_controller_data stih407_softreset_controller = {
	.wait_for_ack = false,
	.active_low = true,
	.nr_channels = ARRAY_SIZE(stih407_softresets),
	.channels = stih407_softresets,
};

static const struct
syscfg_reset_controller_data stih407_picophyreset_controller = {
	.wait_for_ack = false,
	.nr_channels = ARRAY_SIZE(stih407_picophyresets),
	.channels = stih407_picophyresets,
};

phys_addr_t sti_reset_get_regmap(const char *compatible)
{
	struct udevice *syscon;
	struct regmap *regmap;
	int node, ret;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
					     compatible);
	if (node < 0) {
		pr_err("unable to find %s node\n", compatible);
		return node;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_SYSCON, node, &syscon);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset failed: %d\n",
		      __func__, ret);
		return ret;
	}

	regmap = syscon_get_regmap(syscon);
	if (!regmap) {
		pr_err("unable to get regmap for %s\n", syscon->name);
		return -ENODEV;
	}

	return regmap->ranges[0].start;
}

static int sti_reset_program_hw(struct reset_ctl *reset_ctl, int assert)
{
	struct udevice *dev = reset_ctl->dev;
	struct syscfg_reset_controller_data *reset_desc =
		(struct syscfg_reset_controller_data *)(dev->driver_data);
	struct syscfg_reset_channel_data *ch;
	phys_addr_t base;
	u32 ctrl_val = reset_desc->active_low ? !assert : !!assert;
	void __iomem *reg;

	/* check if reset id is inside available range */
	if (reset_ctl->id >= reset_desc->nr_channels)
		return -EINVAL;

	/* get reset sysconf register base address */
	base = sti_reset_get_regmap(reset_desc->channels[reset_ctl->id].compatible);

	ch = &reset_desc->channels[reset_ctl->id];

	/* check the deassert counter to assert reset when it reaches 0 */
	if (!assert) {
		ch->deassert_cnt++;
		if (ch->deassert_cnt > 1)
			return 0;
	} else {
		if (ch->deassert_cnt > 0) {
			ch->deassert_cnt--;
			if (ch->deassert_cnt > 0)
				return 0;
		} else
			pr_err("Reset balancing error: reset_ctl=%p dev=%p id=%lu\n",
			      reset_ctl, reset_ctl->dev, reset_ctl->id);
	}

	reg = (void __iomem *)base + ch->reset_offset;

	if (ctrl_val)
		generic_set_bit(ch->reset_bit, reg);
	else
		generic_clear_bit(ch->reset_bit, reg);

	if (!reset_desc->wait_for_ack)
		return 0;

	reg = (void __iomem *)base + ch->ack_offset;
	if (wait_for_bit_le32(reg, BIT(ch->ack_bit), ctrl_val,
			      1000, false)) {
		pr_err("Stuck on waiting ack reset_ctl=%p dev=%p id=%lu\n",
		      reset_ctl, reset_ctl->dev, reset_ctl->id);

		return -ETIMEDOUT;
	}

	return 0;
}

static int sti_reset_request(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int sti_reset_free(struct reset_ctl *reset_ctl)
{
	return 0;
}

static int sti_reset_assert(struct reset_ctl *reset_ctl)
{
	return sti_reset_program_hw(reset_ctl, true);
}

static int sti_reset_deassert(struct reset_ctl *reset_ctl)
{
	return sti_reset_program_hw(reset_ctl, false);
}

struct reset_ops sti_reset_ops = {
	.request = sti_reset_request,
	.free = sti_reset_free,
	.rst_assert = sti_reset_assert,
	.rst_deassert = sti_reset_deassert,
};

static int sti_reset_probe(struct udevice *dev)
{
	struct sti_reset *priv = dev_get_priv(dev);

	priv->data = (void *)dev_get_driver_data(dev);

	return 0;
}

static const struct udevice_id sti_reset_ids[] = {
	{
		.compatible = "st,stih407-picophyreset",
		.data = (ulong)&stih407_picophyreset_controller,
	},
	{
		.compatible = "st,stih407-powerdown",
		.data = (ulong)&stih407_powerdown_controller,
	},
	{
		.compatible = "st,stih407-softreset",
		.data = (ulong)&stih407_softreset_controller,
	},
	{ }
};

U_BOOT_DRIVER(sti_reset) = {
	.name = "sti_reset",
	.id = UCLASS_RESET,
	.of_match = sti_reset_ids,
	.probe = sti_reset_probe,
	.priv_auto_alloc_size = sizeof(struct sti_reset),
	.ops = &sti_reset_ops,
};
