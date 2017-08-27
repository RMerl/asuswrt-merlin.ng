/**
 * \file timer/timer.c
 * \brief Timer Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 1998-2001
 *
 * Timer Interface is designed to access timers.
 * See \ref timer page for more details.
 */
/*
 *  Timer Interface - main file
 *  Copyright (c) 1998-2001 by Jaroslav Kysela <perex@perex.cz>
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

/*! \page timer Timer interface

<P> Timer interface is designed to use internal timers in sound hardware, but
it can be driven with any timer.

\section timer_general_overview General overview

The timer implementation uses ring buffer to store information about timing
events. In this buffer is recorded count of ticks and current tick resolution
in nanoseconds.

\section timer_open Opening

Timer devices can be opened in two ways. When #SND_TIMER_OPEN_NONBLOCK flag
is used, then the open functions return immediately with -EBUSY error code when
resources are occupied with another application. When #SND_TIMER_OPEN_NONBLOCK
is not used (by default) the open functions block the application requesting
device until resources are not free.

\section timer_events Events

Events are read via snd_timer_read() function.

\section timer_examples Examples

The full featured examples with cross-links:

\par Simple timer test program
\ref example_test_timer "example code"
\par
This example shows opening a timer device and reading of timer events.

*/

/**
 * \example ../test/timer.c
 * \anchor example_test_timer
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "timer_local.h"

static int snd_timer_open_conf(snd_timer_t **timer,
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
	int (*open_func)(snd_timer_t **, const char *, snd_config_t *, snd_config_t *, int) = NULL;
#ifndef PIC
	extern void *snd_timer_open_symbols(void);
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
	err = snd_config_search_definition(timer_root, "timer_type", str, &type_conf);
	if (err >= 0) {
		if (snd_config_get_type(type_conf) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for TIMER type %s definition", str);
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
		snprintf(buf, sizeof(buf), "_snd_timer_%s_open", str);
	}
#ifndef PIC
	snd_timer_open_symbols();
#endif
	h = snd_dlopen(lib, RTLD_NOW);
	if (h)
		open_func = snd_dlsym(h, open_name, SND_DLSYM_VERSION(SND_TIMER_DLSYM_VERSION));
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

static int snd_timer_open_noupdate(snd_timer_t **timer, snd_config_t *root, const char *name, int mode)
{
	int err;
	snd_config_t *timer_conf;
	err = snd_config_search_definition(root, "timer", name, &timer_conf);
	if (err < 0) {
		SNDERR("Unknown timer %s", name);
		return err;
	}
	err = snd_timer_open_conf(timer, name, root, timer_conf, mode);
	snd_config_delete(timer_conf);
	return err;
}

/**
 * \brief Opens a new connection to the timer interface.
 * \param timer Returned handle (NULL if not wanted)
 * \param name ASCII identifier of the timer handle
 * \param mode Open mode
 * \return 0 on success otherwise a negative error code
 *
 * Opens a new connection to the timer interface specified with
 * an ASCII identifier and mode.
 */
int snd_timer_open(snd_timer_t **timer, const char *name, int mode)
{
	int err;
	assert(timer && name);
	err = snd_config_update();
	if (err < 0)
		return err;
	return snd_timer_open_noupdate(timer, snd_config, name, mode);
}

/**
 * \brief Opens a new connection to the timer interface using local configuration
 * \param timer Returned handle (NULL if not wanted)
 * \param name ASCII identifier of the timer handle
 * \param mode Open mode
 * \param lconf Local configuration
 * \return 0 on success otherwise a negative error code
 *
 * Opens a new connection to the timer interface specified with
 * an ASCII identifier and mode.
 */
int snd_timer_open_lconf(snd_timer_t **timer, const char *name,
			 int mode, snd_config_t *lconf)
{
	assert(timer && name && lconf);
	return snd_timer_open_noupdate(timer, lconf, name, mode);
}

/**
 * \brief close timer handle
 * \param timer timer handle
 * \return 0 on success otherwise a negative error code
 *
 * Closes the specified timer handle and frees all associated
 * resources.
 */
int snd_timer_close(snd_timer_t *timer)
{
	int err;
  	assert(timer);
	while (!list_empty(&timer->async_handlers)) {
		snd_async_handler_t *h = list_entry(timer->async_handlers.next, snd_async_handler_t, hlist);
		snd_async_del_handler(h);
	}
	err = timer->ops->close(timer);
	if (timer->dl_handle)
		snd_dlclose(timer->dl_handle);
	free(timer->name);
	free(timer);
	return err;
}

/**
 * \brief get identifier of timer handle
 * \param timer a timer handle
 * \return ascii identifier of timer handle
 *
 * Returns the ASCII identifier of given timer handle. It's the same
 * identifier specified in snd_timer_open().
 */
const char *snd_timer_name(snd_timer_t *timer)
{
	assert(timer);
	return timer->name;
}

/**
 * \brief get type of timer handle
 * \param timer a timer handle
 * \return type of timer handle
 *
 * Returns the type #snd_timer_type_t of given timer handle.
 */
snd_timer_type_t snd_timer_type(snd_timer_t *timer)
{
	assert(timer);
	return timer->type;
}

/**
 * \brief Add an async handler for a timer
 * \param handler Returned handler handle
 * \param timer timer handle
 * \param callback Callback function
 * \param private_data Callback private data
 * \return 0 otherwise a negative error code on failure
 *
 * The asynchronous callback is called when new timer event occurs.
 */
int snd_async_add_timer_handler(snd_async_handler_t **handler, snd_timer_t *timer,
				snd_async_callback_t callback, void *private_data)
{
	int err;
	int was_empty;
	snd_async_handler_t *h;
	err = snd_async_add_handler(&h, timer->poll_fd,
				    callback, private_data);
	if (err < 0)
		return err;
	h->type = SND_ASYNC_HANDLER_TIMER;
	h->u.timer = timer;
	was_empty = list_empty(&timer->async_handlers);
	list_add_tail(&h->hlist, &timer->async_handlers);
	if (was_empty) {
		err = snd_timer_async(timer, snd_async_handler_get_signo(h), getpid());
		if (err < 0) {
			snd_async_del_handler(h);
			return err;
		}
	}
	*handler = h;
	return 0;
}

/**
 * \brief Return timer handle related to an async handler
 * \param handler Async handler handle
 * \return timer handle
 */
snd_timer_t *snd_async_handler_get_timer(snd_async_handler_t *handler)
{
	if (handler->type != SND_ASYNC_HANDLER_TIMER) {
		SNDMSG("invalid handler type %d", handler->type);
		return NULL;
	}
	return handler->u.timer;
}                                                            

/**
 * \brief get count of poll descriptors for timer handle
 * \param timer timer handle
 * \return count of poll descriptors
 */
int snd_timer_poll_descriptors_count(snd_timer_t *timer)
{
	assert(timer);
	return 1;
}

/**
 * \brief get poll descriptors
 * \param timer timer handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_timer_poll_descriptors(snd_timer_t *timer, struct pollfd *pfds, unsigned int space)
{
	assert(timer);
	if (space >= 1) {
		pfds->fd = timer->poll_fd;
		switch (timer->mode & O_ACCMODE) {
		case O_WRONLY:
			pfds->events = POLLOUT|POLLERR|POLLNVAL;
			break;
		case O_RDONLY:
			pfds->events = POLLIN|POLLERR|POLLNVAL;
			break;
		case O_RDWR:
			pfds->events = POLLOUT|POLLIN|POLLERR|POLLNVAL;
			break;
		default:
			return -EIO;
		}
		return 1;
	}
	return 0;
}

/**
 * \brief get returned events from poll descriptors
 * \param timer timer handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_timer_poll_descriptors_revents(snd_timer_t *timer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
        assert(timer && pfds && revents);
        if (nfds == 1) {
                *revents = pfds->revents;
                return 0;
        }
        return -EINVAL;
}

/**
 * \brief set nonblock mode
 * \param timer timer handle
 * \param nonblock 0 = block, 1 = nonblock mode
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_nonblock(snd_timer_t *timer, int nonblock)
{
	int err;
	assert(timer);
	if ((err = timer->ops->nonblock(timer, nonblock)) < 0)
		return err;
	if (nonblock)
		timer->mode |= SND_TIMER_OPEN_NONBLOCK;
	else
		timer->mode &= ~SND_TIMER_OPEN_NONBLOCK;
	return 0;
}

#ifndef DOC_HIDDEN
/**
 * \brief set async mode
 * \param timer timer handle
 * \param sig Signal to raise: < 0 disable, 0 default (SIGIO)
 * \param pid Process ID to signal: 0 current
 * \return 0 on success otherwise a negative error code
 *
 * A signal is raised every period.
 */
int snd_timer_async(snd_timer_t *timer, int sig, pid_t pid)
{
	assert(timer);
        if (sig == 0)
                sig = SIGIO;
	if (pid == 0)
		pid = getpid();
	return timer->ops->async(timer, sig, pid);
}
#endif

/**
 * \brief get size of the snd_timer_info_t structure in bytes
 * \return size of the snd_timer_info_t structure in bytes
 */
size_t snd_timer_info_sizeof()
{
	return sizeof(snd_timer_info_t);
}

/**
 * \brief allocate a new snd_timer_info_t structure
 * \param info returned pointer
 * \return 0 on success otherwise a negative error code if fails
 *
 * Allocates a new snd_timer_info_t structure using the standard
 * malloc C library function.
 */
int snd_timer_info_malloc(snd_timer_info_t **info)
{
	assert(info);
	*info = calloc(1, sizeof(snd_timer_info_t));
	if (!*info)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees the snd_timer_info_t structure
 * \param info pointer to the snd_timer_info_t structure to free
 *
 * Frees the given snd_timer_info_t structure using the standard
 * free C library function.
 */
void snd_timer_info_free(snd_timer_info_t *info)
{
	assert(info);
	free(info);
}

/**
 * \brief copy one snd_timer_info_t structure to another
 * \param dst destination snd_timer_info_t structure
 * \param src source snd_timer_info_t structure
 */
void snd_timer_info_copy(snd_timer_info_t *dst, const snd_timer_info_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief determine, if timer is slave
 * \param info pointer to #snd_timer_info_t structure
 * \return nonzero if timer is slave
 */
int snd_timer_info_is_slave(snd_timer_info_t * info)
{
	assert(info);
	return info->flags & SNDRV_TIMER_FLG_SLAVE ? 1 : 0;
}

/**
 * \brief get timer card
 * \param info pointer to #snd_timer_info_t structure
 * \return timer card number
 */
int snd_timer_info_get_card(snd_timer_info_t * info)
{
	assert(info);
	return info->card;
}

/**
 * \brief get timer id
 * \param info pointer to #snd_timer_info_t structure
 * \return timer id
 */
const char *snd_timer_info_get_id(snd_timer_info_t * info)
{
	assert(info);
	return (const char *)info->id;
}

/**
 * \brief get timer name
 * \param info pointer to #snd_timer_info_t structure
 * \return timer name
 */
const char *snd_timer_info_get_name(snd_timer_info_t * info)
{
	assert(info);
	return (const char *)info->name;
}


/**
 * \brief get timer resolution in us
 * \param info pointer to #snd_timer_info_t structure
 * \return timer resolution
 */
long snd_timer_info_get_resolution(snd_timer_info_t * info)
{
	assert(info);
	return info->resolution;
}

/**
 * \brief get information about timer handle
 * \param timer timer handle
 * \param info pointer to a snd_timer_info_t structure to be filled
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_info(snd_timer_t *timer, snd_timer_info_t * info)
{
	assert(timer);
	assert(info);
	return timer->ops->info(timer, info);
}

/**
 * \brief get size of the snd_timer_params_t structure in bytes
 * \return size of the snd_timer_params_t structure in bytes
 */
size_t snd_timer_params_sizeof()
{
	return sizeof(snd_timer_params_t);
}

/**
 * \brief allocate a new snd_timer_params_t structure
 * \param params returned pointer
 * \return 0 on success otherwise a negative error code if fails
 *
 * Allocates a new snd_timer_params_t structure using the standard
 * malloc C library function.
 */
int snd_timer_params_malloc(snd_timer_params_t **params)
{
	assert(params);
	*params = calloc(1, sizeof(snd_timer_params_t));
	if (!*params)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees the snd_timer_params_t structure
 * \param params pointer to the snd_timer_params_t structure to free
 *
 * Frees the given snd_timer_params_t structure using the standard
 * free C library function.
 */
void snd_timer_params_free(snd_timer_params_t *params)
{
	assert(params);
	free(params);
}

/**
 * \brief copy one snd_timer_params_t structure to another
 * \param dst destination snd_timer_params_t structure
 * \param src source snd_timer_params_t structure
 */
void snd_timer_params_copy(snd_timer_params_t *dst, const snd_timer_params_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief set timer auto start
 * \param params pointer to #snd_timer_params_t structure
 * \param auto_start The boolean value to set
 */
int snd_timer_params_set_auto_start(snd_timer_params_t * params, int auto_start)
{
	assert(params);
	if (auto_start)
		params->flags |= SNDRV_TIMER_PSFLG_AUTO;
	else
		params->flags &= ~SNDRV_TIMER_PSFLG_AUTO;
	return 0;
}

/**
 * \brief determine if timer has auto start flag
 * \param params pointer to #snd_timer_params_t structure
 * \return nonzero if timer has auto start flag
 */
int snd_timer_params_get_auto_start(snd_timer_params_t * params)
{
	assert(params);
	return params->flags & SNDRV_TIMER_PSFLG_AUTO ? 1 : 0;
}

/**
 * \brief set timer exclusive use
 * \param params pointer to #snd_timer_params_t structure
 * \param exclusive The boolean value to set
 */
#ifndef DOXYGEN
int INTERNAL(snd_timer_params_set_exclusive)(snd_timer_params_t * params, int exclusive)
#else
int snd_timer_params_set_exclusive(snd_timer_params_t * params, int exclusive)
#endif
{
	assert(params);
	if (exclusive)
		params->flags |= SNDRV_TIMER_PSFLG_EXCLUSIVE;
	else
		params->flags &= ~SNDRV_TIMER_PSFLG_EXCLUSIVE;
	return 0;
}
use_default_symbol_version(__snd_timer_params_set_exclusive, snd_timer_params_set_exclusive, ALSA_0.9.0);

/**
 * \brief determine if timer has exclusive flag
 * \param params pointer to #snd_timer_params_t structure
 * \return nonzero if timer has exclusive flag
 */
#ifndef DOXYGEN
int INTERNAL(snd_timer_params_get_exclusive)(snd_timer_params_t * params)
#else
int snd_timer_params_get_exclusive(snd_timer_params_t * params)
#endif
{
	assert(params);
	return params->flags & SNDRV_TIMER_PSFLG_EXCLUSIVE ? 1 : 0;
}
use_default_symbol_version(__snd_timer_params_get_exclusive, snd_timer_params_get_exclusive, ALSA_0.9.0);

/**
 * \brief set timer early event
 * \param params pointer to #snd_timer_params_t structure
 * \param early_event The boolean value to set
 */
int snd_timer_params_set_early_event(snd_timer_params_t * params, int early_event)
{
	assert(params);
	if (early_event)
		params->flags |= SNDRV_TIMER_PSFLG_EARLY_EVENT;
	else
		params->flags &= ~SNDRV_TIMER_PSFLG_EARLY_EVENT;
	return 0;
}

/**
 * \brief determine if timer has early event flag
 * \param params pointer to #snd_timer_params_t structure
 * \return nonzero if timer has early event flag set
 */
int snd_timer_params_get_early_event(snd_timer_params_t * params)
{
	assert(params);
	return params->flags & SNDRV_TIMER_PSFLG_EARLY_EVENT ? 1 : 0;
}

/**
 * \brief set timer ticks
 * \param params pointer to #snd_timer_params_t structure
 * \param ticks Ticks to set
 */
void snd_timer_params_set_ticks(snd_timer_params_t * params, long ticks)
{
	assert(params);
	params->ticks = ticks;
}

/**
 * \brief get timer ticks
 * \param params pointer to #snd_timer_params_t structure
 * \return timer ticks
 */
long snd_timer_params_get_ticks(snd_timer_params_t * params)
{
	assert(params);
	return params->ticks;
}

/**
 * \brief set timer queue size (32-1024)
 * \param params pointer to #snd_timer_params_t structure
 * \param queue_size The queue size to set
 */
void snd_timer_params_set_queue_size(snd_timer_params_t * params, long queue_size)
{
	assert(params);
	params->queue_size = queue_size;
}

/**
 * \brief get queue size
 * \param params pointer to #snd_timer_params_t structure
 * \return queue size
 */
long snd_timer_params_get_queue_size(snd_timer_params_t * params)
{
	assert(params);
	return params->queue_size;
}

/**
 * \brief set timer event filter
 * \param params pointer to #snd_timer_params_t structure
 * \param filter The event filter bits to set
 */
#ifndef DOXYGEN
void INTERNAL(snd_timer_params_set_filter)(snd_timer_params_t * params, unsigned int filter)
#else
void snd_timer_params_set_filter(snd_timer_params_t * params, unsigned int filter)
#endif
{
	assert(params);
	params->filter = filter;
}
use_default_symbol_version(__snd_timer_params_set_filter, snd_timer_params_set_filter, ALSA_0.9.0);

/**
 * \brief get timer event filter
 * \param params pointer to #snd_timer_params_t structure
 * \return timer event filter
 */
#ifndef DOXYGEN
unsigned int INTERNAL(snd_timer_params_get_filter)(snd_timer_params_t * params)
#else
unsigned int snd_timer_params_get_filter(snd_timer_params_t * params)
#endif
{
	assert(params);
	return params->filter;
}
use_default_symbol_version(__snd_timer_params_get_filter, snd_timer_params_get_filter, ALSA_0.9.0);

/**
 * \brief set parameters for timer handle
 * \param timer timer handle
 * \param params pointer to a #snd_timer_params_t structure
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_params(snd_timer_t *timer, snd_timer_params_t * params)
{
	assert(timer);
	assert(params);
	return timer->ops->params(timer, params);
}

/**
 * \brief get size of the snd_timer_status_t structure in bytes
 * \return size of the snd_timer_status_t structure in bytes
 */
size_t snd_timer_status_sizeof()
{
	return sizeof(snd_timer_status_t);
}

/**
 * \brief allocate a new snd_timer_status_t structure
 * \param status returned pointer
 * \return 0 on success otherwise a negative error code if fails
 *
 * Allocates a new snd_timer_status_t structure using the standard
 * malloc C library function.
 */
int snd_timer_status_malloc(snd_timer_status_t **status)
{
	assert(status);
	*status = calloc(1, sizeof(snd_timer_status_t));
	if (!*status)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees the snd_timer_status_t structure
 * \param status pointer to the snd_timer_status_t structure to free
 *
 * Frees the given snd_timer_status_t structure using the standard
 * free C library function.
 */
void snd_timer_status_free(snd_timer_status_t *status)
{
	assert(status);
	free(status);
}

/**
 * \brief copy one snd_timer_status_t structure to another
 * \param dst destination snd_timer_status_t structure
 * \param src source snd_timer_status_t structure
 */
void snd_timer_status_copy(snd_timer_status_t *dst, const snd_timer_status_t *src)
{
	assert(dst && src);
	*dst = *src;
}



/**
 * \brief get timestamp
 * \param status pointer to #snd_timer_status_t structure
 * \return timestamp
 */
snd_htimestamp_t snd_timer_status_get_timestamp(snd_timer_status_t * status)
{
	assert(status);
	return status->tstamp;
}

/**
 * \brief get resolution in us
 * \param status pointer to #snd_timer_status_t structure
 * \return resolution
 */
long snd_timer_status_get_resolution(snd_timer_status_t * status)
{
	assert(status);
	return status->resolution;
}

/**
 * \brief get master tick lost count
 * \param status pointer to #snd_timer_status_t structure
 * \return master tick lost count
 */
long snd_timer_status_get_lost(snd_timer_status_t * status)
{
	assert(status);
	return status->lost;
}

/**
 * \brief get overrun count
 * \param status pointer to #snd_timer_status_t structure
 * \return overrun count
 */
long snd_timer_status_get_overrun(snd_timer_status_t * status)
{
	assert(status);
	return status->overrun;
}

/**
 * \brief get count of used queue elements
 * \param status pointer to #snd_timer_status_t structure
 * \return count of used queue elements
 */
long snd_timer_status_get_queue(snd_timer_status_t * status)
{
	assert(status);
	return status->queue;
}

/**
 * \brief get status from timer handle
 * \param timer timer handle
 * \param status pointer to a #snd_timer_status_t structure to be filled
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_status(snd_timer_t *timer, snd_timer_status_t * status)
{
	assert(timer);
	assert(status);
	return timer->ops->status(timer, status);
}

/**
 * \brief start the timer
 * \param timer timer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_start(snd_timer_t *timer)
{
	assert(timer);
	return timer->ops->rt_start(timer);
}

/**
 * \brief stop the timer
 * \param timer timer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_stop(snd_timer_t *timer)
{
	assert(timer);
	return timer->ops->rt_stop(timer);
}

/**
 * \brief continue the timer
 * \param timer timer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_timer_continue(snd_timer_t *timer)
{
	assert(timer);
	return timer->ops->rt_continue(timer);
}

/**
 * \brief read bytes using timer handle
 * \param timer timer handle
 * \param buffer buffer to store the input bytes
 * \param size input buffer size in bytes
 */
ssize_t snd_timer_read(snd_timer_t *timer, void *buffer, size_t size)
{
	assert(timer);
	assert(((timer->mode & O_ACCMODE) == O_RDONLY) || ((timer->mode & O_ACCMODE) == O_RDWR));
	assert(buffer || size == 0);
	return (timer->ops->read)(timer, buffer, size);
}

/**
 * \brief (DEPRECATED) get maximum timer ticks
 * \param info pointer to #snd_timer_info_t structure
 * \return maximum timer ticks
 */
long snd_timer_info_get_ticks(snd_timer_info_t * info)
{
	assert(info);
	return 1;
}
#ifndef DOC_HIDDEN
link_warning(snd_timer_info_get_ticks, "Warning: snd_timer_info_get_ticks is deprecated");
#endif
