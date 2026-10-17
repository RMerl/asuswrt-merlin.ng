// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY LOGC_ARCH

#include <fdt_support.h>
#include <log.h>

#if defined(__riscv)
int arch_fixup_fdt(void *blob)
{
	int ret;

	ret = fdt_find_or_add_subnode(blob, 0, "chosen");;
	if (ret < 0)
		goto err;
	ret = fdt_setprop_u32(blob, ret, "boot-hartid", 1);
	if (ret < 0)
		goto err;
	return 0;
err:
	log_err("Setting /chosen/boot-hartid failed: %s\n", fdt_strerror(ret));
	return ret;
}
#endif
