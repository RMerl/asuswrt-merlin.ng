/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 * 	    Martin Peres
 */
#include "priv.h"

#include <core/device.h>
#include <core/option.h>
#include <subdev/bios.h>
#include <subdev/bios/fan.h>
#include <subdev/gpio.h>

struct nvkm_fanpwm_priv {
	struct nvkm_fan base;
	struct dcb_gpio_func func;
};

static int
nvkm_fanpwm_get(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *tpriv = (void *)therm;
	struct nvkm_fanpwm_priv *priv = (void *)tpriv->fan;
	struct nvkm_gpio *gpio = nvkm_gpio(therm);
	int card_type = nv_device(therm)->card_type;
	u32 divs, duty;
	int ret;

	ret = therm->pwm_get(therm, priv->func.line, &divs, &duty);
	if (ret == 0 && divs) {
		divs = max(divs, duty);
		if (card_type <= NV_40 || (priv->func.log[0] & 1))
			duty = divs - duty;
		return (duty * 100) / divs;
	}

	return gpio->get(gpio, 0, priv->func.func, priv->func.line) * 100;
}

static int
nvkm_fanpwm_set(struct nvkm_therm *therm, int percent)
{
	struct nvkm_therm_priv *tpriv = (void *)therm;
	struct nvkm_fanpwm_priv *priv = (void *)tpriv->fan;
	int card_type = nv_device(therm)->card_type;
	u32 divs, duty;
	int ret;

	divs = priv->base.perf.pwm_divisor;
	if (priv->base.bios.pwm_freq) {
		divs = 1;
		if (therm->pwm_clock)
			divs = therm->pwm_clock(therm, priv->func.line);
		divs /= priv->base.bios.pwm_freq;
	}

	duty = ((divs * percent) + 99) / 100;
	if (card_type <= NV_40 || (priv->func.log[0] & 1))
		duty = divs - duty;

	ret = therm->pwm_set(therm, priv->func.line, divs, duty);
	if (ret == 0)
		ret = therm->pwm_ctrl(therm, priv->func.line, true);
	return ret;
}

int
nvkm_fanpwm_create(struct nvkm_therm *therm, struct dcb_gpio_func *func)
{
	struct nvkm_device *device = nv_device(therm);
	struct nvkm_therm_priv *tpriv = (void *)therm;
	struct nvkm_bios *bios = nvkm_bios(therm);
	struct nvkm_fanpwm_priv *priv;
	struct nvbios_therm_fan fan;
	u32 divs, duty;

	nvbios_fan_parse(bios, &fan);

	if (!nvkm_boolopt(device->cfgopt, "NvFanPWM", func->param) ||
	    !therm->pwm_ctrl || fan.type == NVBIOS_THERM_FAN_TOGGLE ||
	     therm->pwm_get(therm, func->line, &divs, &duty) == -ENODEV)
		return -ENODEV;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	tpriv->fan = &priv->base;
	if (!priv)
		return -ENOMEM;

	priv->base.type = "PWM";
	priv->base.get = nvkm_fanpwm_get;
	priv->base.set = nvkm_fanpwm_set;
	priv->func = *func;
	return 0;
}
