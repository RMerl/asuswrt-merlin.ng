/* SPDX-License-Identifier: GPL-2.0+
 *
 * QE UEC ethernet controller driver
 *
 * based on drivers/qe/uec.c from NXP
 *
 * Copyright (C) 2020 Heiko Schocher <hs@denx.de>
 */

#ifndef _DM_QE_UEC_H
#define _DM_QE_UEC_H

#define qe_uec_dbg(dev, fmt, args...)	debug("%s:" fmt, dev->name, ##args)

#include "uec.h"

/* QE UEC private structure */
struct qe_uec_priv {
	struct uec_priv *uec;
	struct phy_device *phydev;
};
#endif
