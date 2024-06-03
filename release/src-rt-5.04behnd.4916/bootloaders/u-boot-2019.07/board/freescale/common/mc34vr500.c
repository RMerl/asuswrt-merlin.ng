// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Hou Zhiqiang <Zhiqiang.Hou@freescale.com>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/mc34vr500_pmic.h>

static uint8_t swxvolt_addr[4] = { MC34VR500_SW1VOLT,
				   MC34VR500_SW2VOLT,
				   MC34VR500_SW3VOLT,
				   MC34VR500_SW4VOLT };

static uint8_t swx_set_point_base[4] = { 13, 9, 9, 9 };

int mc34vr500_get_sw_volt(uint8_t sw)
{
	struct pmic *p;
	u32 swxvolt;
	uint8_t spb;
	int sw_volt;
	int ret;

	debug("%s: Get SW%u volt from swxvolt_addr = 0x%x\n",
	      __func__, sw + 1, swxvolt_addr[sw]);
	if (sw > SW4) {
		printf("%s: Unsupported SW(sw%d)\n", __func__, sw + 1);
		return -EINVAL;
	}

	p = pmic_get("MC34VR500");
	if (!p) {
		printf("%s: Did NOT find PMIC MC34VR500\n", __func__);
		return -ENODEV;
	}

	ret = pmic_probe(p);
	if (ret)
		return ret;

	ret = pmic_reg_read(p, swxvolt_addr[sw], &swxvolt);
	if (ret) {
		printf("%s: Failed to get SW%u volt\n", __func__, sw + 1);
		return ret;
	}

	debug("%s: SW%d step point swxvolt = %u\n", __func__, sw + 1, swxvolt);
	spb = swx_set_point_base[sw];
	/* The base of SW volt is 625mV and increase by step 25mV */
	sw_volt = 625 + (swxvolt - spb) * 25;

	debug("%s: SW%u volt = %dmV\n", __func__, sw + 1, sw_volt);
	return sw_volt;
}

int mc34vr500_set_sw_volt(uint8_t sw, int sw_volt)
{
	struct pmic *p;
	u32 swxvolt;
	uint8_t spb;
	int ret;

	debug("%s: Set SW%u volt to %dmV\n", __func__, sw + 1, sw_volt);
	/* The least SW volt is 625mV, and only 4 SW outputs */
	if (sw > SW4 || sw_volt < 625)
		return -EINVAL;

	p = pmic_get("MC34VR500");
	if (!p) {
		printf("%s: Did NOT find PMIC MC34VR500\n", __func__);
		return -ENODEV;
	}

	ret = pmic_probe(p);
	if (ret)
		return ret;

	spb = swx_set_point_base[sw];
	/* The base of SW volt is 625mV and increase by step 25mV */
	swxvolt = (sw_volt - 625) / 25 + spb;
	debug("%s: SW%d step point swxvolt = %u\n", __func__, sw + 1, swxvolt);
	if (swxvolt > 63)
		return -EINVAL;

	ret = pmic_reg_write(p, swxvolt_addr[sw], swxvolt);
	if (ret)
		return ret;

	return 0;
}
