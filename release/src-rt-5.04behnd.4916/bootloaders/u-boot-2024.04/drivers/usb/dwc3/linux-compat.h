/* SPDX-License-Identifier: GPL-2.0 */
/**
 * linux-compat.h - DesignWare USB3 Linux Compatibiltiy Adapter  Header
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - https://www.ti.com
 *
 * Authors: Kishon Vijay Abraham I <kishon@ti.com>
 *
 */

#ifndef __DWC3_LINUX_COMPAT__
#define __DWC3_LINUX_COMPAT__

#define dev_WARN(dev, format, arg...)	debug(format, ##arg)

#endif
