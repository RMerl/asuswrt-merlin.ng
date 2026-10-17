#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* Environment */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial,usbkbd,spikbd\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"

#if IS_ENABLED(CONFIG_CMD_NVME)
	#define BOOT_TARGET_NVME(func) func(NVME, nvme, 0)
#else
	#define BOOT_TARGET_NVME(func)
#endif

#if IS_ENABLED(CONFIG_CMD_USB)
	#define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
	#define BOOT_TARGET_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_NVME(func) \
	BOOT_TARGET_USB(func)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	ENV_DEVICE_SETTINGS \
	BOOTENV

#endif
