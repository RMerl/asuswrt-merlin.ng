// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>

/* Firmware access is platform-dependent.  No generic code in uclass */
UCLASS_DRIVER(firmware) = {
	.id		= UCLASS_FIRMWARE,
	.name		= "firmware",
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.post_bind	= dm_scan_fdt_dev,
#endif
};
