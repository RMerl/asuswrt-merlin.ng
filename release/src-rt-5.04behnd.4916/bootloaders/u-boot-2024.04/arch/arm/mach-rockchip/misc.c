/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * RK3399: Architecture common definitions
 *
 * Copyright (C) 2019 Collabora Inc - https://www.collabora.com/
 *      Rohan Garg <rohan.garg@collabora.com>
 *
 * Based on puma-rk3399.c:
 *      (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <env.h>
#include <dm.h>
#include <hash.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <misc.h>
#include <u-boot/crc.h>
#include <u-boot/sha256.h>

#include <asm/arch-rockchip/misc.h>

int rockchip_setup_macaddr(void)
{
#if CONFIG_IS_ENABLED(HASH) && CONFIG_IS_ENABLED(SHA256)
	int ret;
	const char *cpuid = env_get("cpuid#");
	u8 hash[SHA256_SUM_LEN];
	int size = sizeof(hash);
	u8 mac_addr[6];

	/* Only generate a MAC address, if none is set in the environment */
	if (env_get("ethaddr"))
		return 0;

	if (!cpuid) {
		debug("%s: could not retrieve 'cpuid#'\n", __func__);
		return -1;
	}

	ret = hash_block("sha256", (void *)cpuid, strlen(cpuid), hash, &size);
	if (ret) {
		debug("%s: failed to calculate SHA256\n", __func__);
		return -1;
	}

	/* Copy 6 bytes of the hash to base the MAC address on */
	memcpy(mac_addr, hash, 6);

	/* Make this a valid MAC address and set it */
	mac_addr[0] &= 0xfe;  /* clear multicast bit */
	mac_addr[0] |= 0x02;  /* set local assignment bit (IEEE802) */
	eth_env_set_enetaddr("ethaddr", mac_addr);

	/* Make a valid MAC address for ethernet1 */
	mac_addr[5] ^= 0x01;
	eth_env_set_enetaddr("eth1addr", mac_addr);
#endif
	return 0;
}

int rockchip_cpuid_from_efuse(const u32 cpuid_offset,
			      const u32 cpuid_length,
			      u8 *cpuid)
{
#if IS_ENABLED(CONFIG_ROCKCHIP_EFUSE) || IS_ENABLED(CONFIG_ROCKCHIP_OTP)
	struct udevice *dev;
	int ret;

	/* retrieve the device */
#if IS_ENABLED(CONFIG_ROCKCHIP_EFUSE)
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_efuse), &dev);
#elif IS_ENABLED(CONFIG_ROCKCHIP_OTP)
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
#endif
	if (ret) {
		debug("%s: could not find efuse device\n", __func__);
		return -1;
	}

	/* read the cpu_id range from the efuses */
	ret = misc_read(dev, cpuid_offset, cpuid, cpuid_length);
	if (ret < 0) {
		debug("%s: reading cpuid from the efuses failed\n",
		      __func__);
		return -1;
	}
#endif
	return 0;
}

int rockchip_cpuid_set(const u8 *cpuid, const u32 cpuid_length)
{
	u8 low[cpuid_length / 2], high[cpuid_length / 2];
	char cpuid_str[cpuid_length * 2 + 1];
	u64 serialno;
	char serialno_str[17];
	const char *oldid;
	int i;

	memset(cpuid_str, 0, sizeof(cpuid_str));
	for (i = 0; i < 16; i++)
		sprintf(&cpuid_str[i * 2], "%02x", cpuid[i]);

	debug("cpuid: %s\n", cpuid_str);

	/*
	 * Mix the cpuid bytes using the same rules as in
	 *   ${linux}/drivers/soc/rockchip/rockchip-cpuinfo.c
	 */
	for (i = 0; i < 8; i++) {
		low[i] = cpuid[1 + (i << 1)];
		high[i] = cpuid[i << 1];
	}

	serialno = crc32_no_comp(0, low, 8);
	serialno |= (u64)crc32_no_comp(serialno, high, 8) << 32;
	snprintf(serialno_str, sizeof(serialno_str), "%016llx", serialno);

	oldid = env_get("cpuid#");
	if (oldid && strcmp(oldid, cpuid_str) != 0)
		printf("cpuid: value %s present in env does not match hardware %s\n",
		       oldid, cpuid_str);

	env_set("cpuid#", cpuid_str);

	/* Only generate serial# when none is set yet */
	if (!env_get("serial#"))
		env_set("serial#", serialno_str);

	return 0;
}
