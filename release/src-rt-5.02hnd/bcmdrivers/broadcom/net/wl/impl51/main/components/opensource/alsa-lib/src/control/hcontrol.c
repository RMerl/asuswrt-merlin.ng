/**
 * \file control/hcontrol.c
 * \brief HCTL Interface - High Level CTL
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2000
 *
 * HCTL interface is designed to access preloaded and sorted primitive controls.
 * Callbacks may be used for event handling.
 * See \ref hcontrol page for more details.
 */
/*
 *  Control Interface - high level API
 *  Copyright (c) 2000 by Jaroslav Kysela <perex@perex.cz>
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
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

/*! \page hcontrol High level control interface

<P> High level control interface is designed to access preloaded and sorted primitive controls.

\section hcontrol_general_overview General overview

<P> High level control interface caches the accesses to primitive controls
to reduce overhead accessing the real controls in kernel drivers.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "control_local.h"
#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#ifndef DOC_HIDDEN
#define NOT_FOUND 1000000000
#endif

static int snd_hctl_compare_default(const snd_hctl_elem_t *c1,
				    const snd_hctl_elem_t *c2);

/**
 * \brief Opens an HCTL
 * \param hctlp Returned HCTL handle
 * \param name ASCII identifier of the underlying CTL handle
 * \param mode Open mode (see #SND_CTL_NONBLOCK, #SND_CTL_ASYNC)
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_open(snd_hctl_t **hctlp, const char *name, int mode)
{
	snd_ctl_t *ctl;
	int err;
	
	if ((err = snd_ctl_open(&ctl, name, mode)) < 0)
		return err;
	err = snd_hctl_open_ctl(hctlp, ctl);
	if (err < 0)
		snd_ctl_close(ctl);
	return err;
}

/**
 * \brief Opens an HCTL
 * \param hctlp Returned HCTL handle
 * \param ctl underlying CTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_open_ctl(snd_hctl_t **hctlp, snd_ctl_t *ctl)
{
	snd_hctl_t *hctl;

	assert(hctlp);
	*hctlp = NULL;
	if ((hctl = (snd_hctl_t *)calloc(1, sizeof(snd_hctl_t))) == NULL)
		return -ENOMEM;
	INIT_LIST_HEAD(&hctl->elems);
	hctl->ctl = ctl;
	*hctlp = hctl;
	return 0;
}

/**
 * \brief close HCTL handle
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 *
 * Closes the specified HCTL handle and frees all associated
 * resources.
 */
int snd_hctl_close(snd_hctl_t *hctl)
{
	int err;

	assert(hctl);
	err = snd_ctl_close(hctl->ctl);
	snd_hctl_free(hctl);
	free(hctl);
	return err;
}

/**
 * \brief get identifier of HCTL handle
 * \param hctl HCTL handle
 * \return ascii identifier of HCTL handle
 *
 * Returns the ASCII identifier of given HCTL handle. It's the same
 * identifier specified in snd_hctl_open().
 */
const char *snd_hctl_name(snd_hctl_t *hctl)
{
	assert(hctl);
	return snd_ctl_name(hctl->ctl);
}

/**
 * \brief set nonblock mode
 * \param hctl HCTL handle
 * \param nonblock 0 = block, 1 = nonblock mode
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_nonblock(snd_hctl_t *hctl, int nonblock)
{
	assert(hctl);
	return snd_ctl_nonblock(hctl->ctl, nonblock);
}

/**
 * \brief set async mode
 * \param hctl HCTL handle
 * \param sig Signal to raise: < 0 disable, 0 default (SIGIO)
 * \param pid Process ID to signal: 0 current
 * \return 0 on success otherwise a negative error code
 *
 * A signal is raised when a change happens.
 */
int snd_hctl_async(snd_hctl_t *hctl, int sig, pid_t pid)
{
	assert(hctl);
	return snd_ctl_async(hctl->ctl, sig, pid);
}

/**
 * \brief get count of poll descriptors for HCTL handle
 * \param hctl HCTL handle
 * \return count of poll descriptors
 */
int snd_hctl_poll_descriptors_count(snd_hctl_t *hctl)
{
	assert(hctl);
	return snd_ctl_poll_descriptors_count(hctl->ctl);
}

/**
 * \brief get poll descriptors
 * \param hctl HCTL handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_hctl_poll_descriptors(snd_hctl_t *hctl, struct pollfd *pfds, unsigned int space)
{
	assert(hctl);
	return snd_ctl_poll_descriptors(hctl->ctl, pfds, space);
}

/**
 * \brief get returned events from poll descriptors
 * \param hctl HCTL handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_hctl_poll_descriptors_revents(snd_hctl_t *hctl, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	assert(hctl);
	return snd_ctl_poll_descriptors_revents(hctl->ctl, pfds, nfds, revents);
}

static int snd_hctl_throw_event(snd_hctl_t *hctl, unsigned int mask,
			 snd_hctl_elem_t *elem)
{
	if (hctl->callback)
		return hctl->callback(hctl, mask, elem);
	return 0;
}

static int snd_hctl_elem_throw_event(snd_hctl_elem_t *elem,
			      unsigned int mask)
{
	if (elem->callback)
		return elem->callback(elem, mask);
	return 0;
}

static int snd_hctl_compare_mixer_priority_lookup(const char **name, const char * const *names, int coef)
{
	int res;

	for (res = 0; *names; names++, res += coef) {
		if (!strncmp(*name, *names, strlen(*names))) {
			*name += strlen(*names);
			if (**name == ' ')
				(*name)++;
			return res+1;
		}
	}
	return NOT_FOUND;
}

static int get_compare_weight(const snd_ctl_elem_id_t *id)
{
	static const char *const names[] = {
		"Master",
		"Hardware Master",
		"Headphone",
		"Tone Control",
		"3D Control",
		"PCM",
		"Front",
		"Surround",
		"Center",
		"LFE",
		"Synth",
		"FM",
		"Wave",
		"Music",
		"DSP",
		"Line",
		"CD",
		"Mic",
		"Phone",
		"Video",
		"Zoom Video",
		"PC Speaker",
		"Aux",
		"Mono",
		"ADC",
		"Capture Source",
		"Capture",
		"Playback",
		"Loopback",
		"Analog Loopback",
		"Digital Loopback",
		"I2S",
		"IEC958",
		NULL
	};
	static const char *const names1[] = {
		"Switch",
		"Volume",
		"Playback",
		"Capture",
		"Bypass",
		"Mono",
		"Front",
		"Rear",
		"Pan",
		"Output",
		"-",
		NULL
	};
	static const char *const names2[] = {
		"Switch",
		"Volume",
		"Bypass",
		"Depth",
		"Wide",
		"Space",
		"Level",
		"Center",
		NULL
	};
	const char *name = (char *)id->name, *name1;
	int res, res1;
	
	if ((res = snd_hctl_compare_mixer_priority_lookup((const char **)&name, names, 1000000)) == NOT_FOUND)
		return NOT_FOUND;
	if (*name == '\0')
		return res;
	for (name1 = name; *name1 != '\0'; name1++);
	for (name1--; name1 != name && *name1 != ' '; name1--);
	while (name1 != name && *name1 == ' ')
		name1--;
	if (name1 != name) {
		for (; name1 != name && *name1 != ' '; name1--);
		name = name1;
		if ((res1 = snd_hctl_compare_mixer_priority_lookup((const char **)&name, names1, 1000)) == NOT_FOUND)
			return res;
		res += res1;
	} else {
		name = name1;
	}
	if ((res1 = snd_hctl_compare_mixer_priority_lookup((const char **)&name, names2, 1)) == NOT_FOUND)
		return res;
	return res + res1;
}

static int _snd_hctl_find_elem(snd_hctl_t *hctl, const snd_ctl_elem_id_t *id, int *dir)
{
	unsigned int l, u;
	snd_hctl_elem_t el;
	int c = 0;
	int idx = -1;
	assert(hctl && id);
	assert(hctl->compare);
	el.id = *id;
	el.compare_weight = get_compare_weight(id);
	l = 0;
	u = hctl->count;
	while (l < u) {
		idx = (l + u) / 2;
		c = hctl->compare(&el, hctl->pelems[idx]);
		if (c < 0)
			u = idx;
		else if (c > 0)
			l = idx + 1;
		else
			break;
	}
	*dir = c;
	return idx;
}

static int snd_hctl_elem_add(snd_hctl_t *hctl, snd_hctl_elem_t *elem)
{
	int dir;
	int idx; 
	elem->compare_weight = get_compare_weight(&elem->id);
	if (hctl->count == hctl->alloc) {
		snd_hctl_elem_t **h;
		hctl->alloc += 32;
		h = realloc(hctl->pelems, sizeof(*h) * hctl->alloc);
		if (!h) {
			hctl->alloc -= 32;
			return -ENOMEM;
		}
		hctl->pelems = h;
	}
	if (hctl->count == 0) {
		list_add_tail(&elem->list, &hctl->elems);
		hctl->pelems[0] = elem;
	} else {
		idx = _snd_hctl_find_elem(hctl, &elem->id, &dir);
		assert(dir != 0);
		if (dir > 0) {
			list_add(&elem->list, &hctl->pelems[idx]->list);
			idx++;
		} else {
			list_add_tail(&elem->list, &hctl->pelems[idx]->list);
		}
		memmove(hctl->pelems + idx + 1,
			hctl->pelems + idx,
			(hctl->count - idx) * sizeof(snd_hctl_elem_t *));
		hctl->pelems[idx] = elem;
	}
	hctl->count++;
	return snd_hctl_throw_event(hctl, SNDRV_CTL_EVENT_MASK_ADD, elem);
}

static void snd_hctl_elem_remove(snd_hctl_t *hctl, unsigned int idx)
{
	snd_hctl_elem_t *elem = hctl->pelems[idx];
	unsigned int m;
	snd_hctl_elem_throw_event(elem, SNDRV_CTL_EVENT_MASK_REMOVE);
	list_del(&elem->list);
	free(elem);
	hctl->count--;
	m = hctl->count - idx;
	if (m > 0)
		memmove(hctl->pelems + idx,
			hctl->pelems + idx + 1,
			m * sizeof(snd_hctl_elem_t *));
}

/**
 * \brief free HCTL loaded elements
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_free(snd_hctl_t *hctl)
{
	while (hctl->count > 0)
		snd_hctl_elem_remove(hctl, hctl->count - 1);
	free(hctl->pelems);
	hctl->pelems = 0;
	hctl->alloc = 0;
	INIT_LIST_HEAD(&hctl->elems);
	return 0;
}

static snd_hctl_t *compare_hctl;
static int hctl_compare(const void *a, const void *b) {
	return compare_hctl->compare(*(const snd_hctl_elem_t * const *) a,
			     *(const snd_hctl_elem_t * const *) b);
}

static void snd_hctl_sort(snd_hctl_t *hctl)
{
	unsigned int k;
#ifdef HAVE_LIBPTHREAD
	static pthread_mutex_t sync_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

	assert(hctl);
	assert(hctl->compare);
	INIT_LIST_HEAD(&hctl->elems);

#ifdef HAVE_LIBPTHREAD
	pthread_mutex_lock(&sync_lock);
#endif
	compare_hctl = hctl;
	qsort(hctl->pelems, hctl->count, sizeof(*hctl->pelems), hctl_compare);
#ifdef HAVE_LIBPTHREAD
	pthread_mutex_unlock(&sync_lock);
#endif
	for (k = 0; k < hctl->count; k++)
		list_add_tail(&hctl->pelems[k]->list, &hctl->elems);
}

/**
 * \brief Change HCTL compare function and reorder elements
 * \param hctl HCTL handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_set_compare(snd_hctl_t *hctl, snd_hctl_compare_t compare)
{
	assert(hctl);
	hctl->compare = compare == NULL ? snd_hctl_compare_default : compare;
	snd_hctl_sort(hctl);
	return 0;
}

/**
 * \brief A "don't care" fast compare functions that may be used with #snd_hctl_set_compare
 * \param c1 First HCTL element
 * \param c2 Second HCTL element
 * \return -1 if c1 < c2, 0 if c1 == c2, 1 if c1 > c2
 */
int snd_hctl_compare_fast(const snd_hctl_elem_t *c1,
			  const snd_hctl_elem_t *c2)
{
	return c1->id.numid - c2->id.numid;
}

static int snd_hctl_compare_default(const snd_hctl_elem_t *c1,
				    const snd_hctl_elem_t *c2)
{
	int res, d;

	d = c1->id.iface - c2->id.iface;
	if (d != 0)
		return d;
	if (c1->id.iface == SNDRV_CTL_ELEM_IFACE_MIXER) {
		d = c1->compare_weight - c2->compare_weight;
		if (d != 0)
			return d;
	}
	d = c1->id.device - c2->id.device;
	if (d != 0)
		return d;
	d = c1->id.subdevice - c2->id.subdevice;
	if (d != 0)
		return d;
	res = strcmp((const char *)c1->id.name, (const char *)c2->id.name);
	if (res != 0)
		return res;
	return c1->id.index - c2->id.index;
}

/**
 * \brief get first element for an HCTL
 * \param hctl HCTL handle
 * \return pointer to first element
 */
snd_hctl_elem_t *snd_hctl_first_elem(snd_hctl_t *hctl)
{
	assert(hctl);
	if (list_empty(&hctl->elems))
		return NULL;
	return list_entry(hctl->elems.next, snd_hctl_elem_t, list);
}

/**
 * \brief get last element for an HCTL
 * \param hctl HCTL handle
 * \return pointer to last element
 */
snd_hctl_elem_t *snd_hctl_last_elem(snd_hctl_t *hctl)
{
	assert(hctl);
	if (list_empty(&hctl->elems))
		return NULL;
	return list_entry(hctl->elems.prev, snd_hctl_elem_t, list);
}

/**
 * \brief get next HCTL element
 * \param elem HCTL element
 * \return pointer to next element
 */
snd_hctl_elem_t *snd_hctl_elem_next(snd_hctl_elem_t *elem)
{
	assert(elem);
	if (elem->list.next == &elem->hctl->elems)
		return NULL;
	return list_entry(elem->list.next, snd_hctl_elem_t, list);
}

/**
 * \brief get previous HCTL element
 * \param elem HCTL element
 * \return pointer to previous element
 */
snd_hctl_elem_t *snd_hctl_elem_prev(snd_hctl_elem_t *elem)
{
	assert(elem);
	if (elem->list.prev == &elem->hctl->elems)
		return NULL;
	return list_entry(elem->list.prev, snd_hctl_elem_t, list);
}

/**
 * \brief Search an HCTL element
 * \param hctl HCTL handle
 * \param id Element identifier
 * \return pointer to found HCTL element or NULL if it does not exists
 */
snd_hctl_elem_t *snd_hctl_find_elem(snd_hctl_t *hctl, const snd_ctl_elem_id_t *id)
{
	int dir;
	int res = _snd_hctl_find_elem(hctl, id, &dir);
	if (res < 0 || dir != 0)
		return NULL;
	return hctl->pelems[res];
}

/**
 * \brief Load an HCTL with all elements and sort them
 * \param hctl HCTL handle
 * \return 0 on success otherwise a negative error code
 */
int snd_hctl_load(snd_hctl_t *hctl)
{
	snd_ctl_elem_list_t list;
	int err = 0;
	unsigned int idx;

	assert(hctl);
	assert(hctl->ctl);
	assert(hctl->count == 0);
	assert(list_empty(&hctl->elems));
	memset(&list, 0, sizeof(list));
	if ((err = snd_ctl_elem_list(hctl->ctl, &list)) < 0)
		goto _end;
	while (list.count != list.used) {
		err = snd_ctl_elem_list_alloc_space(&list, list.count);
		if (err < 0)
			goto _end;
		if ((err = snd_ctl_elem_list(hctl->ctl, &list)) < 0)
			goto _end;
	}
	if (hctl->alloc < list.count) {
		hctl->alloc = list.count;
		free(hctl->pelems);
		hctl->pelems = malloc(hctl->alloc * sizeof(*hctl->pelems));
		if (!hctl->pelems) {
			err = -ENOMEM;
			goto _end;
		}
	}
	for (idx = 0; idx < list.count; idx++) {
		snd_hctl_elem_t *elem;
		elem = calloc(1, sizeof(snd_hctl_elem_t));
		if (elem == NULL) {
			snd_hctl_free(hctl);
			err = -ENOMEM;
			goto _end;
		}
		elem->id = list.pids[idx];
		elem->hctl = hctl;
		elem->compare_weight = get_compare_weight(&elem->id);
		hctl->pelems[idx] = elem;
		list_add_tail(&elem->list, &hctl->elems);
		hctl->count++;
	}
	if (!hctl->compare)
		hctl->compare = snd_hctl_compare_default;
	snd_hctl_sort(hctl);
	for (idx = 0; idx < hctl->count; idx++) {
		int res = snd_hctl_throw_event(hctl, SNDRV_CTL_EVENT_MASK_ADD,
					       hctl->pelems[idx]);
		if (res < 0)
			return res;
	}
	err = snd_ctl_subscribe_events(hctl->ctl, 1);
 _end:
	free(list.pids);
	return err;
}

/**
 * \brief Set callback function for an HCTL
 * \param hctl HCTL handle
 * \param callback callback function
 */
void snd_hctl_set_callback(snd_hctl_t *hctl, snd_hctl_callback_t callback)
{
	assert(hctl);
	hctl->callback = callback;
}

/**
 * \brief Set callback private value for an HCTL
 * \param hctl HCTL handle
 * \param callback_private callback private value
 */
void snd_hctl_set_callback_private(snd_hctl_t *hctl, void *callback_private)
{
	assert(hctl);
	hctl->callback_private = callback_private;
}

/**
 * \brief Get callback private value for an HCTL
 * \param hctl HCTL handle
 * \return callback private value
 */
void *snd_hctl_get_callback_private(snd_hctl_t *hctl)
{
	assert(hctl);
	return hctl->callback_private;
}

/**
 * \brief Get number of loaded elements for an HCTL
 * \param hctl HCTL handle
 * \return elements count
 */
unsigned int snd_hctl_get_count(snd_hctl_t *hctl)
{
	return hctl->count;
}

/**
 * \brief Wait for a HCTL to become ready (i.e. at least one event pending)
 * \param hctl HCTL handle
 * \param timeout maximum time in milliseconds to wait
 * \return a positive value on success otherwise a negative error code
 * \retval 0 timeout occurred
 * \retval 1 an event is pending
 */
int snd_hctl_wait(snd_hctl_t *hctl, int timeout)
{
	struct pollfd *pfd;
	unsigned short *revents;
	int i, npfds, pollio, err, err_poll;
	
	npfds = snd_hctl_poll_descriptors_count(hctl);
	if (npfds <= 0 || npfds >= 16) {
		SNDERR("Invalid poll_fds %d\n", npfds);
		return -EIO;
	}
	pfd = alloca(sizeof(*pfd) * npfds);
	revents = alloca(sizeof(*revents) * npfds);
	err = snd_hctl_poll_descriptors(hctl, pfd, npfds);
	if (err < 0)
		return err;
	if (err != npfds) {
		SNDMSG("invalid poll descriptors %d\n", err);
		return -EIO;
	}
	do {
		pollio = 0;
		err_poll = poll(pfd, npfds, timeout);
		if (err_poll < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}
		if (! err_poll)
			break;
		err = snd_hctl_poll_descriptors_revents(hctl, pfd, npfds, revents);
		if (err < 0)
			return err;
		for (i = 0; i < npfds; i++) {
			if (revents[i] & (POLLERR | POLLNVAL))
				return -EIO;
			if ((revents[i] & (POLLIN | POLLOUT)) == 0)
				continue;
			pollio++;
		}
	} while (! pollio);
	return err_poll > 0 ? 1 : 0;
}

/**
 * \brief Get a ctl handle associated to the given hctl handle
 * \param hctl HCTL handle
 * \return a ctl handle otherwise NULL
 */
snd_ctl_t *snd_hctl_ctl(snd_hctl_t *hctl)
{
	return hctl->ctl;
}

static int snd_hctl_handle_event(snd_hctl_t *hctl, snd_ctl_event_t *event)
{
	snd_hctl_elem_t *elem;
	int res;

	assert(hctl);
	assert(hctl->ctl);
	switch (event->type) {
	case SND_CTL_EVENT_ELEM:
		break;
	default:
		return 0;
	}
	if (event->data.elem.mask == SNDRV_CTL_EVENT_MASK_REMOVE) {
		int dir;
		res = _snd_hctl_find_elem(hctl, &event->data.elem.id, &dir);
		assert(res >= 0 && dir == 0);
		if (res < 0 || dir != 0)
			return -ENOENT;
		snd_hctl_elem_remove(hctl, (unsigned int) res);
		return 0;
	}
	if (event->data.elem.mask & SNDRV_CTL_EVENT_MASK_ADD) {
		elem = calloc(1, sizeof(snd_hctl_elem_t));
		if (elem == NULL)
			return -ENOMEM;
		elem->id = event->data.elem.id;
		elem->hctl = hctl;
		res = snd_hctl_elem_add(hctl, elem);
		if (res < 0)
			return res;
	}
	if (event->data.elem.mask & (SNDRV_CTL_EVENT_MASK_VALUE |
				     SNDRV_CTL_EVENT_MASK_INFO)) {
		elem = snd_hctl_find_elem(hctl, &event->data.elem.id);
		assert(elem);
		if (!elem)
			return -ENOENT;
		res = snd_hctl_elem_throw_event(elem, event->data.elem.mask &
						(SNDRV_CTL_EVENT_MASK_VALUE |
						 SNDRV_CTL_EVENT_MASK_INFO));
		if (res < 0)
			return res;
	}
	return 0;
}

/**
 * \brief Handle pending HCTL events invoking callbacks
 * \param hctl HCTL handle
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_handle_events(snd_hctl_t *hctl)
{
	snd_ctl_event_t event;
	int res;
	unsigned int count = 0;
	
	assert(hctl);
	assert(hctl->ctl);
	while ((res = snd_ctl_read(hctl->ctl, &event)) != 0 &&
	       res != -EAGAIN) {
		if (res < 0)
			return res;
		res = snd_hctl_handle_event(hctl, &event);
		if (res < 0)
			return res;
		count++;
	}
	return count;
}

/**
 * \brief Get information for an HCTL element
 * \param elem HCTL element
 * \param info HCTL element information
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_info(snd_hctl_elem_t *elem, snd_ctl_elem_info_t *info)
{
	assert(elem);
	assert(elem->hctl);
	assert(info);
	info->id = elem->id;
	return snd_ctl_elem_info(elem->hctl->ctl, info);
}

/**
 * \brief Get value for an HCTL element
 * \param elem HCTL element
 * \param value HCTL element value
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_read(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	assert(elem);
	assert(elem->hctl);
	assert(value);
	value->id = elem->id;
	return snd_ctl_elem_read(elem->hctl->ctl, value);
}

/**
 * \brief Set value for an HCTL element
 * \param elem HCTL element
 * \param value HCTL element value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_write(snd_hctl_elem_t *elem, snd_ctl_elem_value_t * value)
{
	assert(elem);
	assert(elem->hctl);
	assert(value);
	value->id = elem->id;
	return snd_ctl_elem_write(elem->hctl->ctl, value);
}

/**
 * \brief Get TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \param tlv_size size of TLV array in bytes
 * \return 0 otherwise a negative error code on failure
 */
int snd_hctl_elem_tlv_read(snd_hctl_elem_t *elem, unsigned int *tlv, unsigned int tlv_size)
{
	assert(elem);
	assert(tlv);
	assert(tlv_size >= 12);
	return snd_ctl_elem_tlv_read(elem->hctl->ctl, &elem->id, tlv, tlv_size);
}

/**
 * \brief Set TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_tlv_write(snd_hctl_elem_t *elem, const unsigned int *tlv)
{
	assert(elem);
	assert(tlv);
	assert(tlv[1] >= 4);
	return snd_ctl_elem_tlv_write(elem->hctl->ctl, &elem->id, tlv);
}

/**
 * \brief Set TLV value for an HCTL element
 * \param elem HCTL element
 * \param tlv TLV array for value
 * \retval 0 on success
 * \retval >1 on success when value was changed
 * \retval <0 a negative error code on failure
 */
int snd_hctl_elem_tlv_command(snd_hctl_elem_t *elem, const unsigned int *tlv)
{
	assert(elem);
	assert(tlv);
	assert(tlv[1] >= 4);
	return snd_ctl_elem_tlv_command(elem->hctl->ctl, &elem->id, tlv);
}

/**
 * \brief Get HCTL handle for an HCTL element
 * \param elem HCTL element
 * \return HCTL handle
 */
snd_hctl_t *snd_hctl_elem_get_hctl(snd_hctl_elem_t *elem)
{
	assert(elem);
	return elem->hctl;
}

/**
 * \brief Get CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \param ptr Pointer to returned CTL element identifier
 */
void snd_hctl_elem_get_id(const snd_hctl_elem_t *obj, snd_ctl_elem_id_t *ptr)
{
	assert(obj && ptr);
	*ptr = obj->id;
}

/**
 * \brief Get element numeric identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return element numeric identifier
 */
unsigned int snd_hctl_elem_get_numid(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->id.numid;
}

/**
 * \brief Get interface part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return interface part of element identifier
 */
snd_ctl_elem_iface_t snd_hctl_elem_get_interface(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->id.iface;
}

/**
 * \brief Get device part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return device part of element identifier
 */
unsigned int snd_hctl_elem_get_device(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->id.device;
}

/**
 * \brief Get subdevice part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return subdevice part of element identifier
 */
unsigned int snd_hctl_elem_get_subdevice(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->id.subdevice;
}

/**
 * \brief Get name part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return name part of element identifier
 */
const char *snd_hctl_elem_get_name(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return (const char *)obj->id.name;
}

/**
 * \brief Get index part of CTL element identifier of a CTL element id/value
 * \param obj CTL element id/value
 * \return index part of element identifier
 */
unsigned int snd_hctl_elem_get_index(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->id.index;
}

/**
 * \brief Set callback function for an HCTL element
 * \param obj HCTL element
 * \param val callback function
 */
void snd_hctl_elem_set_callback(snd_hctl_elem_t *obj, snd_hctl_elem_callback_t val)
{
	assert(obj);
	obj->callback = val;
}

/**
 * \brief Set callback private value for an HCTL element
 * \param obj HCTL element
 * \param val callback private value
 */
void snd_hctl_elem_set_callback_private(snd_hctl_elem_t *obj, void * val)
{
	assert(obj);
	obj->callback_private = val;
}

/**
 * \brief Get callback private value for an HCTL element
 * \param obj HCTL element
 * \return callback private value
 */
void * snd_hctl_elem_get_callback_private(const snd_hctl_elem_t *obj)
{
	assert(obj);
	return obj->callback_private;
}
