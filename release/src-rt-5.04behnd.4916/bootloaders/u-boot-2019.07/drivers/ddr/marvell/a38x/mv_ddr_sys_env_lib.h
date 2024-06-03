/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_SYS_ENV_LIB_H
#define _MV_DDR_SYS_ENV_LIB_H

#include "ddr_ml_wrapper.h"

/* device revision */
#define DEV_ID_REG			0x18238
#define DEV_VERSION_ID_REG		0x1823c
#define REVISON_ID_OFFS			8
#define REVISON_ID_MASK			0xf00

#define MPP_CONTROL_REG(id)		(0x18000 + (id * 4))
#define GPP_DATA_OUT_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x00)
#define GPP_DATA_OUT_EN_REG(grp)	(MV_GPP_REGS_BASE(grp) + 0x04)
#define GPP_DATA_IN_REG(grp)		(MV_GPP_REGS_BASE(grp) + 0x10)
#define MV_GPP_REGS_BASE(unit)		(0x18100 + ((unit) * 0x40))

#define MPP_REG_NUM(GPIO_NUM)		(GPIO_NUM / 8)
#define MPP_MASK(GPIO_NUM)		(0xf << 4 * (GPIO_NUM - \
					(MPP_REG_NUM(GPIO_NUM) * 8)));
#define GPP_REG_NUM(GPIO_NUM)		(GPIO_NUM / 32)
#define GPP_MASK(GPIO_NUM)		(1 << GPIO_NUM % 32)

/* device ID */
/* Board ID numbers */
#define MARVELL_BOARD_ID_MASK		0x10

/* Customer boards for A38x */
#define A38X_CUSTOMER_BOARD_ID_BASE	0x0
#define A38X_CUSTOMER_BOARD_ID0		(A38X_CUSTOMER_BOARD_ID_BASE + 0)
#define A38X_CUSTOMER_BOARD_ID1		(A38X_CUSTOMER_BOARD_ID_BASE + 1)
#define A38X_MV_MAX_CUSTOMER_BOARD_ID	(A38X_CUSTOMER_BOARD_ID_BASE + 2)
#define A38X_MV_CUSTOMER_BOARD_NUM	(A38X_MV_MAX_CUSTOMER_BOARD_ID - \
					 A38X_CUSTOMER_BOARD_ID_BASE)

/* Marvell boards for A38x */
#define A38X_MARVELL_BOARD_ID_BASE	0x10
#define RD_NAS_68XX_ID			(A38X_MARVELL_BOARD_ID_BASE + 0)
#define DB_68XX_ID			(A38X_MARVELL_BOARD_ID_BASE + 1)
#define RD_AP_68XX_ID			(A38X_MARVELL_BOARD_ID_BASE + 2)
#define DB_AP_68XX_ID			(A38X_MARVELL_BOARD_ID_BASE + 3)
#define DB_GP_68XX_ID			(A38X_MARVELL_BOARD_ID_BASE + 4)
#define DB_BP_6821_ID			(A38X_MARVELL_BOARD_ID_BASE + 5)
#define DB_AMC_6820_ID			(A38X_MARVELL_BOARD_ID_BASE + 6)
#define A38X_MV_MAX_MARVELL_BOARD_ID	(A38X_MARVELL_BOARD_ID_BASE + 7)
#define A38X_MV_MARVELL_BOARD_NUM	(A38X_MV_MAX_MARVELL_BOARD_ID - \
					 A38X_MARVELL_BOARD_ID_BASE)

/* Marvell boards for A39x */
#define A39X_MARVELL_BOARD_ID_BASE	0x30
#define A39X_DB_69XX_ID			(A39X_MARVELL_BOARD_ID_BASE + 0)
#define A39X_RD_69XX_ID			(A39X_MARVELL_BOARD_ID_BASE + 1)
#define A39X_MV_MAX_MARVELL_BOARD_ID	(A39X_MARVELL_BOARD_ID_BASE + 2)
#define A39X_MV_MARVELL_BOARD_NUM	(A39X_MV_MAX_MARVELL_BOARD_ID - \
					 A39X_MARVELL_BOARD_ID_BASE)

struct board_wakeup_gpio {
	u32 board_id;
	int gpio_num;
};

enum suspend_wakeup_status {
	SUSPEND_WAKEUP_DISABLED,
	SUSPEND_WAKEUP_ENABLED,
	SUSPEND_WAKEUP_ENABLED_GPIO_DETECTED,
};

/*
 * GPIO status indication for Suspend Wakeup:
 * If suspend to RAM is supported and GPIO inidcation is implemented,
 * set the gpio number
 * If suspend to RAM is supported but GPIO indication is not implemented
 * set '-2'
 * If suspend to RAM is not supported set '-1'
 */
#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT
#ifdef CONFIG_ARMADA_38X
#define MV_BOARD_WAKEUP_GPIO_INFO {		\
	{A38X_CUSTOMER_BOARD_ID0,	-1 },	\
	{A38X_CUSTOMER_BOARD_ID0,	-1 },	\
};
#else
#define MV_BOARD_WAKEUP_GPIO_INFO {		\
	{A39X_CUSTOMER_BOARD_ID0,	-1 },	\
	{A39X_CUSTOMER_BOARD_ID0,	-1 },	\
};
#endif /* CONFIG_ARMADA_38X */

#else

#ifdef CONFIG_ARMADA_38X
#define MV_BOARD_WAKEUP_GPIO_INFO {	\
	{RD_NAS_68XX_ID, -2 },		\
	{DB_68XX_ID,	 -1 },		\
	{RD_AP_68XX_ID,	 -2 },		\
	{DB_AP_68XX_ID,	 -2 },		\
	{DB_GP_68XX_ID,	 -2 },		\
	{DB_BP_6821_ID,	 -2 },		\
	{DB_AMC_6820_ID, -2 },		\
};
#else
#define MV_BOARD_WAKEUP_GPIO_INFO {	\
	{A39X_RD_69XX_ID, -1 },		\
	{A39X_DB_69XX_ID, -1 },		\
};
#endif /* CONFIG_ARMADA_38X */
#endif /* CONFIG_CUSTOMER_BOARD_SUPPORT */

enum suspend_wakeup_status mv_ddr_sys_env_suspend_wakeup_check(void);
u32 mv_ddr_sys_env_get_cs_ena_from_reg(void);

#endif /* _MV_DDR_SYS_ENV_LIB_H */
