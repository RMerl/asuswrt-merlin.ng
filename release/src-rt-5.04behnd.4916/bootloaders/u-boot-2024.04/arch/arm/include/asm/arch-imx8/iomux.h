/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef __ASM_ARCH_IMX8_IOMUX_H__
#define __ASM_ARCH_IMX8_IOMUX_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/*
 * We use 64bits value for iomux settings.
 * High 32bits are used for padring register value,
 * low 16bits are used for pin index.
 */
typedef u64 iomux_cfg_t;

#define PADRING_IFMUX_EN_SHIFT		31
#define PADRING_IFMUX_EN_MASK		BIT(31)
#define PADRING_GP_EN_SHIFT		30
#define PADRING_GP_EN_MASK		BIT(30)
#define PADRING_IFMUX_SHIFT		27
#define PADRING_IFMUX_MASK		GENMASK(29, 27)
#define PADRING_CONFIG_SHIFT		25
#define PADRING_LPCONFIG_SHIFT		23
#define PADRING_PULL_SHIFT		5
#define PADRING_DSE_SHIFT		0

#define MUX_PAD_CTRL_SHIFT	32
#define MUX_PAD_CTRL_MASK	((iomux_cfg_t)0xFFFFFFFF << MUX_PAD_CTRL_SHIFT)
#define MUX_PAD_CTRL(x)		((iomux_cfg_t)(x) << MUX_PAD_CTRL_SHIFT)
#define MUX_MODE_SHIFT		(PADRING_IFMUX_SHIFT + MUX_PAD_CTRL_SHIFT)
#define MUX_MODE_MASK		((iomux_cfg_t)0x7 << MUX_MODE_SHIFT)
#define PIN_ID_MASK		((iomux_cfg_t)0xFFFF)

/* Valid mux alt0 to alt7 */
#define MUX_MODE_ALT(x)		(((iomux_cfg_t)(x) << MUX_MODE_SHIFT) & \
				 MUX_MODE_MASK)

void imx8_iomux_setup_pad(iomux_cfg_t pad);
void imx8_iomux_setup_multiple_pads(iomux_cfg_t const *pad_list, u32 count);
#endif	/* __ASM_ARCH_IMX8_IOMUX_H__ */
