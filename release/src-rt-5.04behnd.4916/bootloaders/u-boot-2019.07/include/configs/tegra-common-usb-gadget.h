/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_COMMON_USB_GADGET_H_
#define _TEGRA_COMMON_USB_GADGET_H_

#ifndef CONFIG_SPL_BUILD
/* USB gadget mode support*/
#ifndef CONFIG_TEGRA20
#define CONFIG_CI_UDC_HAS_HOSTPC
#endif
/* DFU protocol */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_1M
#define CONFIG_SYS_DFU_MAX_FILE_SIZE SZ_32M
#endif

#endif /* _TEGRA_COMMON_USB_GADGET_H_ */
