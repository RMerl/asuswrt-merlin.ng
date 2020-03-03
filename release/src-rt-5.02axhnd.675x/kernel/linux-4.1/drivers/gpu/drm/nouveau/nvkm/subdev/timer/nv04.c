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
 */
#include "nv04.h"

#include <core/device.h>

static u64
nv04_timer_read(struct nvkm_timer *ptimer)
{
	struct nv04_timer_priv *priv = (void *)ptimer;
	u32 hi, lo;

	do {
		hi = nv_rd32(priv, NV04_PTIMER_TIME_1);
		lo = nv_rd32(priv, NV04_PTIMER_TIME_0);
	} while (hi != nv_rd32(priv, NV04_PTIMER_TIME_1));

	return ((u64)hi << 32 | lo);
}

static void
nv04_timer_alarm_trigger(struct nvkm_timer *ptimer)
{
	struct nv04_timer_priv *priv = (void *)ptimer;
	struct nvkm_alarm *alarm, *atemp;
	unsigned long flags;
	LIST_HEAD(exec);

	/* move any due alarms off the pending list */
	spin_lock_irqsave(&priv->lock, flags);
	list_for_each_entry_safe(alarm, atemp, &priv->alarms, head) {
		if (alarm->timestamp <= ptimer->read(ptimer))
			list_move_tail(&alarm->head, &exec);
	}

	/* reschedule interrupt for next alarm time */
	if (!list_empty(&priv->alarms)) {
		alarm = list_first_entry(&priv->alarms, typeof(*alarm), head);
		nv_wr32(priv, NV04_PTIMER_ALARM_0, alarm->timestamp);
		nv_wr32(priv, NV04_PTIMER_INTR_EN_0, 0x00000001);
	} else {
		nv_wr32(priv, NV04_PTIMER_INTR_EN_0, 0x00000000);
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	/* execute any pending alarm handlers */
	list_for_each_entry_safe(alarm, atemp, &exec, head) {
		list_del_init(&alarm->head);
		alarm->func(alarm);
	}
}

static void
nv04_timer_alarm(struct nvkm_timer *ptimer, u64 time, struct nvkm_alarm *alarm)
{
	struct nv04_timer_priv *priv = (void *)ptimer;
	struct nvkm_alarm *list;
	unsigned long flags;

	alarm->timestamp = ptimer->read(ptimer) + time;

	/* append new alarm to list, in soonest-alarm-first order */
	spin_lock_irqsave(&priv->lock, flags);
	if (!time) {
		if (!list_empty(&alarm->head))
			list_del(&alarm->head);
	} else {
		list_for_each_entry(list, &priv->alarms, head) {
			if (list->timestamp > alarm->timestamp)
				break;
		}
		list_add_tail(&alarm->head, &list->head);
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	/* process pending alarms */
	nv04_timer_alarm_trigger(ptimer);
}

static void
nv04_timer_alarm_cancel(struct nvkm_timer *ptimer, struct nvkm_alarm *alarm)
{
	struct nv04_timer_priv *priv = (void *)ptimer;
	unsigned long flags;
	spin_lock_irqsave(&priv->lock, flags);
	list_del_init(&alarm->head);
	spin_unlock_irqrestore(&priv->lock, flags);
}

static void
nv04_timer_intr(struct nvkm_subdev *subdev)
{
	struct nv04_timer_priv *priv = (void *)subdev;
	u32 stat = nv_rd32(priv, NV04_PTIMER_INTR_0);

	if (stat & 0x00000001) {
		nv04_timer_alarm_trigger(&priv->base);
		nv_wr32(priv, NV04_PTIMER_INTR_0, 0x00000001);
		stat &= ~0x00000001;
	}

	if (stat) {
		nv_error(priv, "unknown stat 0x%08x\n", stat);
		nv_wr32(priv, NV04_PTIMER_INTR_0, stat);
	}
}

int
nv04_timer_fini(struct nvkm_object *object, bool suspend)
{
	struct nv04_timer_priv *priv = (void *)object;
	if (suspend)
		priv->suspend_time = nv04_timer_read(&priv->base);
	nv_wr32(priv, NV04_PTIMER_INTR_EN_0, 0x00000000);
	return nvkm_timer_fini(&priv->base, suspend);
}

static int
nv04_timer_init(struct nvkm_object *object)
{
	struct nvkm_device *device = nv_device(object);
	struct nv04_timer_priv *priv = (void *)object;
	u32 m = 1, f, n, d, lo, hi;
	int ret;

	ret = nvkm_timer_init(&priv->base);
	if (ret)
		return ret;

	/* aim for 31.25MHz, which gives us nanosecond timestamps */
	d = 1000000 / 32;

	/* determine base clock for timer source */
#if 0 /*XXX*/
	if (device->chipset < 0x40) {
		n = nvkm_hw_get_clock(device, PLL_CORE);
	} else
#endif
	if (device->chipset <= 0x40) {
		/*XXX: figure this out */
		f = -1;
		n = 0;
	} else {
		f = device->crystal;
		n = f;
		while (n < (d * 2)) {
			n += (n / m);
			m++;
		}

		nv_wr32(priv, 0x009220, m - 1);
	}

	if (!n) {
		nv_warn(priv, "unknown input clock freq\n");
		if (!nv_rd32(priv, NV04_PTIMER_NUMERATOR) ||
		    !nv_rd32(priv, NV04_PTIMER_DENOMINATOR)) {
			nv_wr32(priv, NV04_PTIMER_NUMERATOR, 1);
			nv_wr32(priv, NV04_PTIMER_DENOMINATOR, 1);
		}
		return 0;
	}

	/* reduce ratio to acceptable values */
	while (((n % 5) == 0) && ((d % 5) == 0)) {
		n /= 5;
		d /= 5;
	}

	while (((n % 2) == 0) && ((d % 2) == 0)) {
		n /= 2;
		d /= 2;
	}

	while (n > 0xffff || d > 0xffff) {
		n >>= 1;
		d >>= 1;
	}

	/* restore the time before suspend */
	lo = priv->suspend_time;
	hi = (priv->suspend_time >> 32);

	nv_debug(priv, "input frequency : %dHz\n", f);
	nv_debug(priv, "input multiplier: %d\n", m);
	nv_debug(priv, "numerator       : 0x%08x\n", n);
	nv_debug(priv, "denominator     : 0x%08x\n", d);
	nv_debug(priv, "timer frequency : %dHz\n", (f * m) * d / n);
	nv_debug(priv, "time low        : 0x%08x\n", lo);
	nv_debug(priv, "time high       : 0x%08x\n", hi);

	nv_wr32(priv, NV04_PTIMER_NUMERATOR, n);
	nv_wr32(priv, NV04_PTIMER_DENOMINATOR, d);
	nv_wr32(priv, NV04_PTIMER_INTR_0, 0xffffffff);
	nv_wr32(priv, NV04_PTIMER_INTR_EN_0, 0x00000000);
	nv_wr32(priv, NV04_PTIMER_TIME_1, hi);
	nv_wr32(priv, NV04_PTIMER_TIME_0, lo);
	return 0;
}

void
nv04_timer_dtor(struct nvkm_object *object)
{
	struct nv04_timer_priv *priv = (void *)object;
	return nvkm_timer_destroy(&priv->base);
}

int
nv04_timer_ctor(struct nvkm_object *parent, struct nvkm_object *engine,
		struct nvkm_oclass *oclass, void *data, u32 size,
		struct nvkm_object **pobject)
{
	struct nv04_timer_priv *priv;
	int ret;

	ret = nvkm_timer_create(parent, engine, oclass, &priv);
	*pobject = nv_object(priv);
	if (ret)
		return ret;

	priv->base.base.intr = nv04_timer_intr;
	priv->base.read = nv04_timer_read;
	priv->base.alarm = nv04_timer_alarm;
	priv->base.alarm_cancel = nv04_timer_alarm_cancel;
	priv->suspend_time = 0;

	INIT_LIST_HEAD(&priv->alarms);
	spin_lock_init(&priv->lock);
	return 0;
}

struct nvkm_oclass
nv04_timer_oclass = {
	.handle = NV_SUBDEV(TIMER, 0x04),
	.ofuncs = &(struct nvkm_ofuncs) {
		.ctor = nv04_timer_ctor,
		.dtor = nv04_timer_dtor,
		.init = nv04_timer_init,
		.fini = nv04_timer_fini,
	}
};
