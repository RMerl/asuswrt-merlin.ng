/*
 * Copyright 2012 The Nouveau community
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
 * Authors: Martin Peres
 */
#include "priv.h"

static void
nvkm_therm_temp_set_defaults(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *priv = (void *)therm;

	priv->bios_sensor.offset_constant = 0;

	priv->bios_sensor.thrs_fan_boost.temp = 90;
	priv->bios_sensor.thrs_fan_boost.hysteresis = 3;

	priv->bios_sensor.thrs_down_clock.temp = 95;
	priv->bios_sensor.thrs_down_clock.hysteresis = 3;

	priv->bios_sensor.thrs_critical.temp = 105;
	priv->bios_sensor.thrs_critical.hysteresis = 5;

	priv->bios_sensor.thrs_shutdown.temp = 135;
	priv->bios_sensor.thrs_shutdown.hysteresis = 5; /*not that it matters */
}


static void
nvkm_therm_temp_safety_checks(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	struct nvbios_therm_sensor *s = &priv->bios_sensor;

	/* enforce a minimum hysteresis on thresholds */
	s->thrs_fan_boost.hysteresis = max_t(u8, s->thrs_fan_boost.hysteresis, 2);
	s->thrs_down_clock.hysteresis = max_t(u8, s->thrs_down_clock.hysteresis, 2);
	s->thrs_critical.hysteresis = max_t(u8, s->thrs_critical.hysteresis, 2);
	s->thrs_shutdown.hysteresis = max_t(u8, s->thrs_shutdown.hysteresis, 2);
}

/* must be called with alarm_program_lock taken ! */
void
nvkm_therm_sensor_set_threshold_state(struct nvkm_therm *therm,
				      enum nvkm_therm_thrs thrs,
				      enum nvkm_therm_thrs_state st)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	priv->sensor.alarm_state[thrs] = st;
}

/* must be called with alarm_program_lock taken ! */
enum nvkm_therm_thrs_state
nvkm_therm_sensor_get_threshold_state(struct nvkm_therm *therm,
				      enum nvkm_therm_thrs thrs)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	return priv->sensor.alarm_state[thrs];
}

static void
nv_poweroff_work(struct work_struct *work)
{
	orderly_poweroff(true);
	kfree(work);
}

void
nvkm_therm_sensor_event(struct nvkm_therm *therm, enum nvkm_therm_thrs thrs,
			enum nvkm_therm_thrs_direction dir)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	bool active;
	const char *thresolds[] = {
		"fanboost", "downclock", "critical", "shutdown"
	};
	int temperature = therm->temp_get(therm);

	if (thrs < 0 || thrs > 3)
		return;

	if (dir == NVKM_THERM_THRS_FALLING)
		nv_info(therm, "temperature (%i C) went below the '%s' threshold\n",
			temperature, thresolds[thrs]);
	else
		nv_info(therm, "temperature (%i C) hit the '%s' threshold\n",
			temperature, thresolds[thrs]);

	active = (dir == NVKM_THERM_THRS_RISING);
	switch (thrs) {
	case NVKM_THERM_THRS_FANBOOST:
		if (active) {
			nvkm_therm_fan_set(therm, true, 100);
			nvkm_therm_fan_mode(therm, NVKM_THERM_CTRL_AUTO);
		}
		break;
	case NVKM_THERM_THRS_DOWNCLOCK:
		if (priv->emergency.downclock)
			priv->emergency.downclock(therm, active);
		break;
	case NVKM_THERM_THRS_CRITICAL:
		if (priv->emergency.pause)
			priv->emergency.pause(therm, active);
		break;
	case NVKM_THERM_THRS_SHUTDOWN:
		if (active) {
			struct work_struct *work;

			work = kmalloc(sizeof(*work), GFP_ATOMIC);
			if (work) {
				INIT_WORK(work, nv_poweroff_work);
				schedule_work(work);
			}
		}
		break;
	case NVKM_THERM_THRS_NR:
		break;
	}

}

/* must be called with alarm_program_lock taken ! */
static void
nvkm_therm_threshold_hyst_polling(struct nvkm_therm *therm,
				  const struct nvbios_therm_threshold *thrs,
				  enum nvkm_therm_thrs thrs_name)
{
	enum nvkm_therm_thrs_direction direction;
	enum nvkm_therm_thrs_state prev_state, new_state;
	int temp = therm->temp_get(therm);

	prev_state = nvkm_therm_sensor_get_threshold_state(therm, thrs_name);

	if (temp >= thrs->temp && prev_state == NVKM_THERM_THRS_LOWER) {
		direction = NVKM_THERM_THRS_RISING;
		new_state = NVKM_THERM_THRS_HIGHER;
	} else if (temp <= thrs->temp - thrs->hysteresis &&
			prev_state == NVKM_THERM_THRS_HIGHER) {
		direction = NVKM_THERM_THRS_FALLING;
		new_state = NVKM_THERM_THRS_LOWER;
	} else
		return; /* nothing to do */

	nvkm_therm_sensor_set_threshold_state(therm, thrs_name, new_state);
	nvkm_therm_sensor_event(therm, thrs_name, direction);
}

static void
alarm_timer_callback(struct nvkm_alarm *alarm)
{
	struct nvkm_therm_priv *priv =
	container_of(alarm, struct nvkm_therm_priv, sensor.therm_poll_alarm);
	struct nvbios_therm_sensor *sensor = &priv->bios_sensor;
	struct nvkm_timer *ptimer = nvkm_timer(priv);
	struct nvkm_therm *therm = &priv->base;
	unsigned long flags;

	spin_lock_irqsave(&priv->sensor.alarm_program_lock, flags);

	nvkm_therm_threshold_hyst_polling(therm, &sensor->thrs_fan_boost,
					  NVKM_THERM_THRS_FANBOOST);

	nvkm_therm_threshold_hyst_polling(therm, &sensor->thrs_down_clock,
					  NVKM_THERM_THRS_DOWNCLOCK);

	nvkm_therm_threshold_hyst_polling(therm, &sensor->thrs_critical,
					  NVKM_THERM_THRS_CRITICAL);

	nvkm_therm_threshold_hyst_polling(therm, &sensor->thrs_shutdown,
					  NVKM_THERM_THRS_SHUTDOWN);

	spin_unlock_irqrestore(&priv->sensor.alarm_program_lock, flags);

	/* schedule the next poll in one second */
	if (therm->temp_get(therm) >= 0 && list_empty(&alarm->head))
		ptimer->alarm(ptimer, 1000000000ULL, alarm);
}

void
nvkm_therm_program_alarms_polling(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	struct nvbios_therm_sensor *sensor = &priv->bios_sensor;

	nv_debug(therm,
		 "programmed thresholds [ %d(%d), %d(%d), %d(%d), %d(%d) ]\n",
		 sensor->thrs_fan_boost.temp, sensor->thrs_fan_boost.hysteresis,
		 sensor->thrs_down_clock.temp,
		 sensor->thrs_down_clock.hysteresis,
		 sensor->thrs_critical.temp, sensor->thrs_critical.hysteresis,
		 sensor->thrs_shutdown.temp, sensor->thrs_shutdown.hysteresis);

	alarm_timer_callback(&priv->sensor.therm_poll_alarm);
}

int
nvkm_therm_sensor_init(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	priv->sensor.program_alarms(therm);
	return 0;
}

int
nvkm_therm_sensor_fini(struct nvkm_therm *therm, bool suspend)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	struct nvkm_timer *ptimer = nvkm_timer(therm);

	if (suspend)
		ptimer->alarm_cancel(ptimer, &priv->sensor.therm_poll_alarm);
	return 0;
}

void
nvkm_therm_sensor_preinit(struct nvkm_therm *therm)
{
	const char *sensor_avail = "yes";

	if (therm->temp_get(therm) < 0)
		sensor_avail = "no";

	nv_info(therm, "internal sensor: %s\n", sensor_avail);
}

int
nvkm_therm_sensor_ctor(struct nvkm_therm *therm)
{
	struct nvkm_therm_priv *priv = (void *)therm;
	struct nvkm_bios *bios = nvkm_bios(therm);

	nvkm_alarm_init(&priv->sensor.therm_poll_alarm, alarm_timer_callback);

	nvkm_therm_temp_set_defaults(therm);
	if (nvbios_therm_sensor_parse(bios, NVBIOS_THERM_DOMAIN_CORE,
				      &priv->bios_sensor))
		nv_error(therm, "nvbios_therm_sensor_parse failed\n");
	nvkm_therm_temp_safety_checks(therm);

	return 0;
}
