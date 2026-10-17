/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_SERVAL_DEVCPU_GCB_MIIM_REGS_H_
#define _MSCC_SERVAL_DEVCPU_GCB_MIIM_REGS_H_

#include <linux/bitops.h>
#define MIIM_MII_STATUS(gi)  (0x5c + (gi * 36))
#define MIIM_MII_CMD(gi)     (0x64 + (gi * 36))
#define MIIM_MII_DATA(gi)    (0x68 + (gi * 36))

#define  MSCC_F_MII_STATUS_MIIM_STAT_BUSY(x)  ((x) ? BIT(3) : 0)

#define MSCC_F_MII_CMD_MIIM_CMD_VLD(x)        ((x) ? BIT(31) : 0)
#define MSCC_F_MII_CMD_MIIM_CMD_PHYAD(x)      (GENMASK(29, 25) & ((x) << 25))
#define MSCC_F_MII_CMD_MIIM_CMD_REGAD(x)      (GENMASK(24, 20) & ((x) << 20))
#define MSCC_F_MII_CMD_MIIM_CMD_WRDATA(x)     (GENMASK(19, 4) & ((x) << 4))
#define MSCC_F_MII_CMD_MIIM_CMD_OPR_FIELD(x)  (GENMASK(2, 1) & ((x) << 1))
#define MSCC_F_MII_CMD_MIIM_CMD_SCAN(x)       ((x) ? BIT(0) : 0)

#define MSCC_M_MII_DATA_MIIM_DATA_SUCCESS     GENMASK(17, 16)
#define MSCC_X_MII_DATA_MIIM_DATA_RDDATA(x)   (((x) >> 0) & GENMASK(15, 0))

#endif
