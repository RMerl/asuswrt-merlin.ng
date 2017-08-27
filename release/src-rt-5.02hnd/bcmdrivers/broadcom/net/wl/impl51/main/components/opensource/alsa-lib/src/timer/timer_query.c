/**
 * \file timer/timer_query.c
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2001
 *
 * Timer Query Interface is designed to obtain identification of timers.
 */
/*
 *  Timer Query Interface - main file
 *  Copyright (c) 2001 by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "timer_local.h"

static int snd_timer_query_open_conf(snd_timer_query_t **timer,
				     const char *name, snd_config_t *timer_root,
				     snd_config_t *timer_conf, int mode)
{
	const char *str;
	char buf[256];
	int err;
	snd_config_t *conf, *type_conf = NULL;
	snd_config_iterator_t i, next;
	const char *id;
	const char *lib = NULL, *open_name = NULL;
	int (*open_func)(snd_timer_query_t **, const char *, snd_config_t *, snd_config_t *, int) = NULL;
#ifndef PIC
	extern void *snd_timer_query_open_symbols(void);
#endif
	void *h = NULL;
	if (snd_config_get_type(timer_conf) != SND_CONFIG_TYPE_COMPOUND) {
		if (name)
			SNDERR("Invalid type for TIMER %s definition", name);
		else
			SNDERR("Invalid type for TIMER definition");
		return -EINVAL;
	}
	err = snd_config_search(timer_conf, "type", &conf);
	if (err < 0) {
		SNDERR("type is not defined");
		return err;
	}
	err = snd_config_get_id(conf, &id);
	if (err < 0) {
		SNDERR("unable to get id");
		return err;
	}
	err = snd_config_get_string(conf, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return err;
	}
	err = snd_config_search_definition(timer_root, "timer_query_type", str, &type_conf);
	if (err >= 0) {
		if (snd_config_get_type(type_conf) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for TIMER type %s definition", str);
			err = -EINVAL;
			goto _err;
		}
		snd_config_for_each(i, next, type_conf) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (strcmp(id, "comment") == 0)
				continue;
			if (strcmp(id, "lib") == 0) {
				err = snd_config_get_string(n, &lib);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			if (strcmp(id, "open") == 0) {
				err = snd_config_get_string(n, &open_name);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					goto _err;
				}
				continue;
			}
			SNDERR("Unknown field %s", id);
			err = -EINVAL;
			goto _err;
		}
	}
	if (!open_name) {
		open_name = buf;
		snprintf(buf, sizeof(buf), "_snd_timer_query_%s_open", str);
	}
#ifndef PIC
	snd_timer_query_open_symbols();
#endif
	h = snd_dlopen(lib, RTLD_NOW);
	if (h)
		open_func = snd_dlsym(h, open_name, SND_DLSYM_VERSION(SND_TIMER_QUERY_DLSYM_VERSION));
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s", lib);
		err = -ENOENT;
	} else if (!open_func) {
		SNDERR("symbol %s is not defined inside %s", open_name, lib);
		snd_dlclose(h);
		err = -ENXIO;
	}
       _err:
	if (type_conf)
		snd_config_delete(type_conf);
	if (! err) {
		err = open_func(timer, name, timer_root, timer_conf, mode);
		if (err < 0)
			snd_dlclose(h);
		else
			(*timer)->dl_handle = h;
	}
	return err;
}

static int snd_timer_query_open_noupdate(snd_timer_query_t **timer, snd_config_t *root, const char *name, int mode)
{
	int err;
	snd_config_t *timer_conf;
	err = snd_config_search_definition(root, "timer_query", name, &timer_conf);
	if (err < 0) {
		SNDERR("Unknown timer %s", name);
		return err;
	}
	err = snd_timer_query_open_conf(timer, name, root, timer_conf, mode);
	snd_config_delete(timer_conf);
	return err;
}

/**
 * \brief Opens a new connection to the timer query interface.
 * \param timer Returned handle (NULL if not wanted)
 * \param name ASCII identifier of the RawMidi handle
 * \param mode Open mode
 * \return 0 on success otherwise a negative error code
 *
 * Opens a new connection to the RawMidi interface specified with
 * an ASCII identifier and mode.
 */
int snd_timer_query_open(snd_timer_query_t **timer, const char *name, int mode)
{
	int err;
	assert(timer && name);
	err = snd_config_update();
	if (err < 0)
		return err;
	return snd_timer_query_open_noupdate(timer, snd_config, name, mode);
}

/**
 * \brief Opens a new connection to the timer query interface using local configuration
 * \param timer Returned handle (NULL if not wanted)
 * \param name ASCII identifier of the RawMidi handle
 * \param mode Open mode
 * \param lconf Local configuration
 * \return 0 on success otherwise a negative error code
 *
 * Opens a new connection to the RawMidi interface specified with
 * an ASCII identifier and mode.
 */
int snd_timer_query_open_lconf(snd_timer_query_t **timer, const char *name,
			       int mode, snd_config_t *lconf)
{
	assert(timer && name && lconf);
	return snd_timer_query_open_noupdate(timer, lconf, name, mode);
}

/**
 * \brief close timer query handle
 * \param timer timer handle
 * \return 0 on success otherwise a negative error code
 *
 * Closes the specified timer handle and frees all associated
 * resources.
 */
int snd_timer_query_close(snd_timer_query_t *timer)
{
	int err;
  	assert(timer);
	err = timer->ops->close(timer);
	if (timer->dl_handle)
		snd_dlclose(timer->dl_handle);
	free(timer->name);
	free(timer);
	return err;
}

/**
 * \brief obtain the next timer identification
 * \param timer timer handle
 * \param tid timer identification
 * \return 0 on success otherwise a negative error code
 *
 * if tid->dev_class is -1, then the first device is returned
 * if result tid->dev_class is -1, no more devices are left
 */
int snd_timer_query_next_device(snd_timer_query_t *timer, snd_timer_id_t *tid)
{
  	assert(timer);
  	assert(tid);
	return timer->ops->next_device(timer, tid);
}

/**
 * \brief get size of the snd_timer_ginfo_t structure in bytes
 * \return size of the snd_timer_ginfo_t structure in bytes
 */
size_t snd_timer_ginfo_sizeof(void)
{
	return sizeof(snd_timer_ginfo_t);
}

/**
 * \brief allocate a new snd_timer_ginfo_t structure
 * \param info returned pointer
 * \return 0 on success otherwise a negative error code if fails
 *
 * Allocates a new snd_timer_info_t structure using the standard
 * malloc C library function.
 */
int snd_timer_ginfo_malloc(snd_timer_ginfo_t **info)
{
	assert(info);
	*info = calloc(1, sizeof(snd_timer_ginfo_t));
	if (!*info)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees the snd_timer_ginfo_t structure
 * \param info pointer to the snd_timer_ginfo_t structure to free
 *
 * Frees the given snd_timer_info_t structure using the standard
 * free C library function.
 */
void snd_timer_ginfo_free(snd_timer_ginfo_t *info)
{
	assert(info);
	free(info);  
}
  
/**
 * \brief copy one snd_timer_info_t structure to another
 * \param dst destination snd_timer_info_t structure
 * \param src source snd_timer_info_t structure
 */
void snd_timer_ginfo_copy(snd_timer_ginfo_t *dst, const snd_timer_ginfo_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief set timer identification
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \param tid pointer to #snd_timer_id_t structure
 * \return zero on success otherwise a negative error number
 */
int snd_timer_ginfo_set_tid(snd_timer_ginfo_t *obj, snd_timer_id_t *tid)
{
	obj->tid = *((snd_timer_id_t *)tid);
	return 0;
}

/**
 * \brief get timer identification
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return pointer to snd_timer_id_t
 */
snd_timer_id_t *snd_timer_ginfo_get_tid(snd_timer_ginfo_t *obj)
{
	return (snd_timer_id_t *)&obj->tid;
}

/**
 * \brief get timer flags
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer flags
 */
unsigned int snd_timer_ginfo_get_flags(snd_timer_ginfo_t *obj)
{
	return obj->flags;
}

/**
 * \brief get associated card with timer
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return associated card
 */
int snd_timer_ginfo_get_card(snd_timer_ginfo_t *obj)
{
	return obj->card;
}

/**
 * \brief get timer identification
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer identification
 */
char *snd_timer_ginfo_get_id(snd_timer_ginfo_t *obj)
{
	return (char *)obj->id;
}

/**
 * \brief get timer name
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer name
 */
char *snd_timer_ginfo_get_name(snd_timer_ginfo_t *obj)
{
	return (char *)obj->name;
}

/**
 * \brief get timer resolution in ns
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer resolution in ns
 */
unsigned long snd_timer_ginfo_get_resolution(snd_timer_ginfo_t *obj)
{
	return obj->resolution;
}

/**
 * \brief get timer minimal resolution in ns
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer minimal resolution in ns
 */
unsigned long snd_timer_ginfo_get_resolution_min(snd_timer_ginfo_t *obj)
{
	return obj->resolution_min;
}

/**
 * \brief get timer maximal resolution in ns
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return timer maximal resolution in ns
 */
unsigned long snd_timer_ginfo_get_resolution_max(snd_timer_ginfo_t *obj)
{
	return obj->resolution_max;
}

/**
 * \brief get current timer clients
 * \param obj pointer to #snd_timer_ginfo_t structure
 * \return current timer clients
 */
unsigned int snd_timer_ginfo_get_clients(snd_timer_ginfo_t *obj)
{
	return obj->clients;
}

/**
 * \brief obtain the timer global information
 * \param timer timer handle
 * \param info timer information
 * \return 0 on success otherwise a negative error code
 */
#ifndef DOXYGEN
int INTERNAL(snd_timer_query_info)(snd_timer_query_t *timer, snd_timer_ginfo_t *info)
#else
int snd_timer_query_info(snd_timer_query_t *timer, snd_timer_ginfo_t *info)
#endif
{
  	assert(timer);
  	assert(info);
	return timer->ops->info(timer, info);
}
use_default_symbol_version(__snd_timer_query_info, snd_timer_query_info, ALSA_0.9.0);

/**
 * \brief set the timer global parameters
 * \param timer timer handle
 * \param params timer parameters
 * \return 0 on success otherwise a negative error code
 */
#ifndef DOXYGEN
int INTERNAL(snd_timer_query_params)(snd_timer_query_t *timer, snd_timer_gparams_t *params)
#else
int snd_timer_query_params(snd_timer_query_t *timer, snd_timer_gparams_t *params)
#endif
{
  	assert(timer);
  	assert(params);
	return timer->ops->params(timer, params);
}
use_default_symbol_version(__snd_timer_query_params, snd_timer_query_params, ALSA_0.9.0);

/**
 * \brief get the timer global status
 * \param timer timer handle
 * \param status timer status
 * \return 0 on success otherwise a negative error code
 */
#ifndef DOXYGEN
int INTERNAL(snd_timer_query_status)(snd_timer_query_t *timer, snd_timer_gstatus_t *status)
#else
int snd_timer_query_status(snd_timer_query_t *timer, snd_timer_gstatus_t *status)
#endif
{
  	assert(timer);
  	assert(status);
	return timer->ops->status(timer, status);
}
use_default_symbol_version(__snd_timer_query_status, snd_timer_query_status, ALSA_0.9.0);

/**
 * \brief get size of the snd_timer_id_t structure in bytes
 * \return size of the snd_timer_id_t structure in bytes
 */
size_t snd_timer_id_sizeof()
{
	return sizeof(snd_timer_id_t);
}

/**
 * \brief allocate a new snd_timer_id_t structure
 * \param info returned pointer
 * \return 0 on success otherwise a negative error code if fails
 *
 * Allocates a new snd_timer_id_t structure using the standard
 * malloc C library function.
 */
int snd_timer_id_malloc(snd_timer_id_t **info)
{
	assert(info);
	*info = calloc(1, sizeof(snd_timer_id_t));
	if (!*info)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees the snd_timer_id_t structure
 * \param info pointer to the snd_timer_id_t structure to free
 *
 * Frees the given snd_timer_id_t structure using the standard
 * free C library function.
 */
void snd_timer_id_free(snd_timer_id_t *info)
{
	assert(info);
	free(info);
}

/**
 * \brief copy one snd_timer_id_t structure to another
 * \param dst destination snd_timer_id_t structure
 * \param src source snd_timer_id_t structure
 */
void snd_timer_id_copy(snd_timer_id_t *dst, const snd_timer_id_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief set timer class
 * \param tid pointer to #snd_timer_id_t structure
 * \param dev_class class of timer device
 */
void snd_timer_id_set_class(snd_timer_id_t * tid, int dev_class)
{
	assert(tid);
	tid->dev_class = dev_class;
}

/**
 * \brief get timer class
 * \param tid pointer to #snd_timer_id_t structure
 * \return timer class
 */
int snd_timer_id_get_class(snd_timer_id_t * tid)
{
	assert(tid);
	return tid->dev_class;
}

/**
 * \brief set timer sub-class
 * \param tid pointer to #snd_timer_id_t structure
 * \param dev_sclass sub-class of timer device
 */
void snd_timer_id_set_sclass(snd_timer_id_t * tid, int dev_sclass)
{
	assert(tid);
	tid->dev_sclass = dev_sclass;
}

/**
 * \brief get timer sub-class
 * \param tid pointer to #snd_timer_id_t structure
 * \return timer sub-class
 */
int snd_timer_id_get_sclass(snd_timer_id_t * tid)
{
	assert(tid);
	return tid->dev_sclass;
}

/**
 * \brief set timer card
 * \param tid pointer to #snd_timer_id_t structure
 * \param card card number
 */
void snd_timer_id_set_card(snd_timer_id_t * tid, int card)
{
	assert(tid);
	tid->card = card;
}

/**
 * \brief get timer card
 * \param tid pointer to #snd_timer_id_t structure
 * \return timer card number
 */
int snd_timer_id_get_card(snd_timer_id_t * tid)
{
	assert(tid);
	return tid->card;
}

/**
 * \brief set timer device
 * \param tid pointer to #snd_timer_id_t structure
 * \param device device number
 */
void snd_timer_id_set_device(snd_timer_id_t * tid, int device)
{
	assert(tid);
	tid->device = device;
}

/**
 * \brief get timer device
 * \param tid pointer to #snd_timer_id_t structure
 * \return timer device number
 */
int snd_timer_id_get_device(snd_timer_id_t * tid)
{
	assert(tid);
	return tid->device;
}

/**
 * \brief set timer subdevice
 * \param tid pointer to #snd_timer_id_t structure
 * \param subdevice subdevice number
 */
void snd_timer_id_set_subdevice(snd_timer_id_t * tid, int subdevice)
{
	assert(tid);
	tid->subdevice = subdevice;
}

/**
 * \brief get timer subdevice
 * \param tid pointer to #snd_timer_id_t structure
 * \return timer subdevice number
 */
int snd_timer_id_get_subdevice(snd_timer_id_t * tid)
{
	assert(tid);
	return tid->subdevice;
}
