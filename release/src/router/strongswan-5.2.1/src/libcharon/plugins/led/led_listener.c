/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "led_listener.h"

#include <errno.h>

#include <daemon.h>
#include <threading/mutex.h>
#include <processing/jobs/callback_job.h>

typedef struct private_led_listener_t private_led_listener_t;

/**
 * Private data of an led_listener_t object.
 */
struct private_led_listener_t {

	/**
	 * Public led_listener_t interface.
	 */
	led_listener_t public;

	/**
	 * Mutex
	 */
	mutex_t *mutex;

	/**
	 * Number of established IKE_SAs
	 */
	int count;

	/**
	 * LED blink on/off time, in ms
	 */
	int blink_time;

	/**
	 * Activity LED fd, if any
	 */
	FILE *activity;

	/**
	 * Activity LED maximum brightness
	 */
	int activity_max;
};

/**
 * Open a LED brightness control file, get max brightness
 */
static FILE *open_led(char *name, int *max_brightness)
{
	char path[PATH_MAX];
	FILE *f;

	if (!name)
	{
		return NULL;
	}

	*max_brightness = 1;
	snprintf(path, sizeof(path), "/sys/class/leds/%s/max_brightness", name);
	f = fopen(path, "r");
	if (f)
	{
		if (fscanf(f, "%d\n", max_brightness) != 1)
		{
			DBG1(DBG_CFG, "reading max brightness for '%s' failed: %s, using 1",
				 name, strerror(errno));
		}
		fclose(f);
	}
	else
	{
		DBG1(DBG_CFG, "reading max_brightness for '%s' failed: %s, using 1",
			 name, strerror(errno));
	}

	snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", name);
	f = fopen(path, "w");
	if (!f)
	{
		DBG1(DBG_CFG, "opening LED file '%s' failed: %s", path, strerror(errno));
	}
	return f;
}

/**
 * Set a LED to a given brightness
 */
static void set_led(FILE *led, int brightness)
{
	if (led)
	{
		if (fprintf(led, "%d\n", brightness) <= 0 ||
			fflush(led) != 0)
		{
			DBG1(DBG_CFG, "setting LED brightness failed: %s", strerror(errno));
		}
	}
}

/**
 * Plugin unloaded?
 */
static bool plugin_gone = FALSE;

/**
 * Reset activity LED after timeout
 */
static job_requeue_t reset_activity_led(private_led_listener_t *this)
{
	if (!plugin_gone)
	{	/* TODO: fix race */
		this->mutex->lock(this->mutex);
		if (this->count)
		{
			set_led(this->activity, this->activity_max);
		}
		else
		{
			set_led(this->activity, 0);
		}
		this->mutex->unlock(this->mutex);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Blink the activity LED
 */
static void blink_activity(private_led_listener_t *this)
{
	if (this->activity)
	{
		this->mutex->lock(this->mutex);
		if (this->count)
		{
			set_led(this->activity, 0);
		}
		else
		{
			set_led(this->activity, this->activity_max);
		}
		lib->scheduler->schedule_job_ms(lib->scheduler, (job_t*)
			callback_job_create_with_prio((callback_job_cb_t)reset_activity_led,
						this, NULL, NULL, JOB_PRIO_CRITICAL), this->blink_time);
		this->mutex->unlock(this->mutex);
	}
}

METHOD(listener_t, ike_state_change, bool,
	private_led_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	this->mutex->lock(this->mutex);
	if (state == IKE_ESTABLISHED && ike_sa->get_state(ike_sa) != IKE_ESTABLISHED)
	{
		this->count++;
		if (this->count == 1)
		{
			set_led(this->activity, this->activity_max);
		}
	}
	if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED && state != IKE_ESTABLISHED)
	{
		this->count--;
		if (this->count == 0)
		{
			set_led(this->activity, 0);
		}
	}
	this->mutex->unlock(this->mutex);
	return TRUE;
}

METHOD(listener_t, message_hook, bool,
	private_led_listener_t *this, ike_sa_t *ike_sa,
	message_t *message, bool incoming, bool plain)
{
	if (plain && (incoming || message->get_request(message)))
	{
		blink_activity(this);
	}
	return TRUE;
}

METHOD(led_listener_t, destroy, void,
	private_led_listener_t *this)
{
	this->mutex->lock(this->mutex);
	set_led(this->activity, 0);
	plugin_gone = TRUE;
	this->mutex->unlock(this->mutex);
	if (this->activity)
	{
		fclose(this->activity);
	}
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
led_listener_t *led_listener_create()
{
	private_led_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_state_change = _ike_state_change,
				.message = _message_hook,
			},
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.blink_time = lib->settings->get_int(lib->settings,
								"%s.plugins.led.blink_time", 50, lib->ns),
	);

	this->activity = open_led(lib->settings->get_str(lib->settings,
								"%s.plugins.led.activity_led", NULL, lib->ns),
								&this->activity_max);
	set_led(this->activity, 0);

	return &this->public;
}
