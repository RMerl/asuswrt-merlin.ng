// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <status_led.h>

#ifndef CONFIG_PCA9551_I2C_ADDR
#error "CONFIG_PCA9551_I2C_ADDR not defined!"
#endif

#define PCA9551_REG_INPUT	0x00	/* Input register (read only) */
#define PCA9551_REG_PSC0	0x01	/* Frequency prescaler 0 */
#define PCA9551_REG_PWM0	0x02	/* PWM0 */
#define PCA9551_REG_PSC1	0x03	/* Frequency prescaler 1 */
#define PCA9551_REG_PWM1	0x04	/* PWM1 */
#define PCA9551_REG_LS0		0x05	/* LED0 to LED3 selector */
#define PCA9551_REG_LS1		0x06	/* LED4 to LED7 selector */

#define PCA9551_CTRL_AI		(1 << 4)	/* Auto-increment flag */

#define PCA9551_LED_STATE_ON		0x00
#define PCA9551_LED_STATE_OFF		0x01
#define PCA9551_LED_STATE_BLINK0	0x02
#define PCA9551_LED_STATE_BLINK1	0x03

struct pca9551_blink_rate {
	u8 psc;	/* Frequency preescaler, see PCA9551_7.pdf p. 6 */
	u8 pwm;	/* Pulse width modulation, see PCA9551_7.pdf p. 6 */
};

static int freq_last = -1;
static int mask_last = -1;
static int idx_last = -1;
static int mode_last;

static int pca9551_led_get_state(int led, int *state)
{
	unsigned int reg;
	u8 shift, buf;
	int ret;

	if (led < 0 || led > 7) {
		return -EINVAL;
	} else if (led < 4) {
		reg = PCA9551_REG_LS0;
		shift = led << 1;
	} else {
		reg = PCA9551_REG_LS1;
		shift = (led - 4) << 1;
	}

	ret = i2c_read(CONFIG_PCA9551_I2C_ADDR, reg, 1, &buf, 1);
	if (ret)
		return ret;

	*state = (buf >> shift) & 0x03;
	return 0;
}

static int pca9551_led_set_state(int led, int state)
{
	unsigned int reg;
	u8 shift, buf, mask;
	int ret;

	if (led < 0 || led > 7) {
		return -EINVAL;
	} else if (led < 4) {
		reg = PCA9551_REG_LS0;
		shift = led << 1;
	} else {
		reg = PCA9551_REG_LS1;
		shift = (led - 4) << 1;
	}
	mask = 0x03 << shift;

	ret = i2c_read(CONFIG_PCA9551_I2C_ADDR, reg, 1, &buf, 1);
	if (ret)
		return ret;

	buf = (buf & ~mask) | ((state & 0x03) << shift);

	ret = i2c_write(CONFIG_PCA9551_I2C_ADDR, reg, 1, &buf, 1);
	if (ret)
		return ret;

	return 0;
}

static int pca9551_led_set_blink_rate(int idx, struct pca9551_blink_rate rate)
{
	unsigned int reg;
	int ret;

	switch (idx) {
	case 0:
		reg = PCA9551_REG_PSC0;
		break;
	case 1:
		reg = PCA9551_REG_PSC1;
		break;
	default:
		return -EINVAL;
	}
	reg |= PCA9551_CTRL_AI;

	ret = i2c_write(CONFIG_PCA9551_I2C_ADDR, reg, 1, (u8 *)&rate, 2);
	if (ret)
		return ret;

	return 0;
}

/*
 * Functions referenced by cmd_led.c or status_led.c
 */
void __led_init(led_id_t id, int state)
{
}

void __led_set(led_id_t mask, int state)
{
	if (state == CONFIG_LED_STATUS_OFF)
		pca9551_led_set_state(mask, PCA9551_LED_STATE_OFF);
	else
		pca9551_led_set_state(mask, PCA9551_LED_STATE_ON);
}

void __led_toggle(led_id_t mask)
{
	int state = 0;

	pca9551_led_get_state(mask, &state);
	pca9551_led_set_state(mask, !state);
}

void __led_blink(led_id_t mask, int freq)
{
	struct pca9551_blink_rate rate;
	int mode;
	int idx;

	if ((freq == freq_last) || (mask == mask_last)) {
		idx = idx_last;
		mode = mode_last;
	} else {
		/* Toggle blink index */
		if (idx_last == 0) {
			idx = 1;
			mode = PCA9551_LED_STATE_BLINK1;
		} else {
			idx = 0;
			mode = PCA9551_LED_STATE_BLINK0;
		}

		idx_last = idx;
		mode_last = mode;
	}
	freq_last = freq;
	mask_last = mask;

	rate.psc = ((freq * 38) / 1000) - 1;
	rate.pwm = 128;		/* 50% duty cycle */

	pca9551_led_set_blink_rate(idx, rate);
	pca9551_led_set_state(mask, mode);
}
