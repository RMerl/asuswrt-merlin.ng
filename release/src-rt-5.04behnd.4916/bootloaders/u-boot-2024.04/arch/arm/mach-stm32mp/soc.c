// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 */

#include <env.h>
#include <misc.h>
#include <net.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>

/* max: 8 OTP for 5 mac address on stm32mp2*/
#define MAX_NB_OTP	8

/* used when CONFIG_DISPLAY_CPUINFO is activated */
int print_cpuinfo(void)
{
	char name[SOC_NAME_SIZE];

	get_soc_name(name);
	printf("CPU: %s\n", name);

	return 0;
}

int setup_serial_number(void)
{
	char serial_string[25];
	u32 otp[3] = {0, 0, 0 };
	struct udevice *dev;
	int ret;

	if (env_get("serial#"))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_SERIAL),
			otp, sizeof(otp));
	if (ret < 0)
		return ret;

	sprintf(serial_string, "%08X%08X%08X", otp[0], otp[1], otp[2]);
	env_set("serial#", serial_string);

	return 0;
}

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the OTP.
 */
__weak int setup_mac_address(void)
{
	int ret;
	int i;
	u32 otp[MAX_NB_OTP];
	uchar enetaddr[ARP_HLEN];
	struct udevice *dev;
	int nb_eth, nb_otp, index;

	if (!IS_ENABLED(CONFIG_NET))
		return 0;

	nb_eth = get_eth_nb();
	if (!nb_eth)
		return 0;

	/* 6 bytes for each MAC addr and 4 bytes for each OTP */
	nb_otp = DIV_ROUND_UP(ARP_HLEN * nb_eth, 4);
	if (nb_otp > MAX_NB_OTP) {
		log_err("invalid number of OTP = %d, max = %d\n", nb_otp, MAX_NB_OTP);
		return -EINVAL;
	}

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_MAC), otp, 4 * nb_otp);
	if (ret < 0)
		return ret;

	for (index = 0; index < nb_eth; index++) {
		/* MAC already in environment */
		if (eth_env_get_enetaddr_by_index("eth", index, enetaddr))
			continue;

		for (i = 0; i < ARP_HLEN; i++)
			enetaddr[i] = ((uint8_t *)&otp)[i + ARP_HLEN * index];

		/* skip FF:FF:FF:FF:FF:FF */
		if (is_broadcast_ethaddr(enetaddr))
			continue;

		if (!is_valid_ethaddr(enetaddr)) {
			log_err("invalid MAC address %d in OTP %pM\n",
				index, enetaddr);
			return -EINVAL;
		}
		log_debug("OTP MAC address %d = %pM\n", index, enetaddr);
		ret = eth_env_set_enetaddr_by_index("eth", index, enetaddr);
		if (ret) {
			log_err("Failed to set mac address %pM from OTP: %d\n",
				enetaddr, ret);
			return ret;
		}
	}

	return 0;
}
