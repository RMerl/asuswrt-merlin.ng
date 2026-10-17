// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Microchip
 *		      Wenyou Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <w1.h>
#include <w1-eeprom.h>
#include <dm/device-internal.h>

#define AT91_PDA_EEPROM_ID_OFFSET		15
#define AT91_PDA_EEPROM_ID_LENGTH		5
#define AT91_PDA_EEPROM_DEFAULT_BUS		0

char *get_cpu_name(void);

void dummy(void)
{
}

#if defined CONFIG_W1
void at91_pda_detect(void)
{
	struct udevice *bus, *dev;
	u8 buf[AT91_PDA_EEPROM_ID_LENGTH + 1] = {0};
	int ret;
	int pda = 0;

	ret = w1_get_bus(AT91_PDA_EEPROM_DEFAULT_BUS, &bus);
	if (ret)
		goto pda_detect_err;

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		ret = device_probe(dev);
		if (ret) {
			continue;
		} else {
			ret = w1_eeprom_read_buf(dev, AT91_PDA_EEPROM_ID_OFFSET,
						 (u8 *)buf, AT91_PDA_EEPROM_ID_LENGTH);
			if (ret)
				goto pda_detect_err;
			break;
		}
	}
	pda = dectoul((const char *)buf, NULL);

	switch (pda) {
	case 7000:
		if (buf[4] == 'B')
			printf("PDA TM7000B detected\n");
		else
			printf("PDA TM7000 detected\n");
		break;
	case 4300:
		printf("PDA TM4300 detected\n");
		break;
	case 5000:
		printf("PDA TM5000 detected\n");
		break;
	}

pda_detect_err:
	env_set("pda", (const char *)buf);
}
#else
void at91_pda_detect(void)
{
}
#endif

void at91_prepare_cpu_var(void)
{
	env_set("cpu", get_cpu_name());
}
