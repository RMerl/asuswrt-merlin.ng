/**
 * \file mixer/mixer.c
 * \brief Mixer Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001
 *
 * Mixer interface is designed to access mixer elements.
 * Callbacks may be used for event handling.
 */
/*
 *  Mixer Interface - main file
 *  Copyright (c) 1998/1999/2000 by Jaroslav Kysela <perex@perex.cz>
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

/*! \page mixer Mixer interface

<P>Mixer interface is designed to access the abstracted mixer controls.
This is an abstraction layer over the hcontrol layer.

\section mixer_general_overview General overview

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mixer_local.h"

#ifndef DOC_HIDDEN
typedef struct _snd_mixer_slave {
	snd_hctl_t *hctl;
	struct list_head list;
} snd_mixer_slave_t;

#endif

static int snd_mixer_compare_default(const snd_mixer_elem_t *c1,
				     const snd_mixer_elem_t *c2);


/**
 * \brief Opens an empty mixer
 * \param mixerp Returned mixer handle
 * \param mode Open mode
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_open(snd_mixer_t **mixerp, int mode ATTRIBUTE_UNUSED)
{
	snd_mixer_t *mixer;
	assert(mixerp);
	mixer = calloc(1, sizeof(*mixer));
	if (mixer == NULL)
		return -ENOMEM;
	INIT_LIST_HEAD(&mixer->slaves);
	INIT_LIST_HEAD(&mixer->classes);
	INIT_LIST_HEAD(&mixer->elems);
	mixer->compare = snd_mixer_compare_default;
	*mixerp = mixer;
	return 0;
}

/**
 * \brief Attach an HCTL element to a mixer element
 * \param melem Mixer element
 * \param helem HCTL element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_attach(snd_mixer_elem_t *melem,
			  snd_hctl_elem_t *helem)
{
	bag_t *bag = snd_hctl_elem_get_callback_private(helem);
	int err;
	err = bag_add(bag, melem);
	if (err < 0)
		return err;
	return bag_add(&melem->helems, helem);
}

/**
 * \brief Detach an HCTL element from a mixer element
 * \param melem Mixer element
 * \param helem HCTL element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_detach(snd_mixer_elem_t *melem,
			  snd_hctl_elem_t *helem)
{
	bag_t *bag = snd_hctl_elem_get_callback_private(helem);
	int err;
	err = bag_del(bag, melem);
	assert(err >= 0);
	err = bag_del(&melem->helems, helem);
	assert(err >= 0);
	return 0;
}

/**
 * \brief Return true if a mixer element does not contain any HCTL elements
 * \param melem Mixer element
 * \return 0 if not empty, 1 if empty
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_empty(snd_mixer_elem_t *melem)
{
	return bag_empty(&melem->helems);
}

static int hctl_elem_event_handler(snd_hctl_elem_t *helem,
				   unsigned int mask)
{
	bag_t *bag = snd_hctl_elem_get_callback_private(helem);
	if (mask == SND_CTL_EVENT_MASK_REMOVE) {
		int res = 0;
		int err;
		bag_iterator_t i, n;
		bag_for_each_safe(i, n, bag) {
			snd_mixer_elem_t *melem = bag_iterator_entry(i);
			snd_mixer_class_t *class = melem->class;
			err = class->event(class, mask, helem, melem);
			if (err < 0)
				res = err;
		}
		assert(bag_empty(bag));
		bag_free(bag);
		return res;
	}
	if (mask & (SND_CTL_EVENT_MASK_VALUE | SND_CTL_EVENT_MASK_INFO)) {
		int err = 0;
		bag_iterator_t i, n;
		bag_for_each_safe(i, n, bag) {
			snd_mixer_elem_t *melem = bag_iterator_entry(i);
			snd_mixer_class_t *class = melem->class;
			err = class->event(class, mask, helem, melem);
			if (err < 0)
				return err;
		}
	}
	return 0;
}

static int hctl_event_handler(snd_hctl_t *hctl, unsigned int mask,
			      snd_hctl_elem_t *elem)
{
	snd_mixer_t *mixer = snd_hctl_get_callback_private(hctl);
	int res = 0;
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		struct list_head *pos;
		bag_t *bag;
		int err = bag_new(&bag);
		if (err < 0)
			return err;
		snd_hctl_elem_set_callback(elem, hctl_elem_event_handler);
		snd_hctl_elem_set_callback_private(elem, bag);
		list_for_each(pos, &mixer->classes) {
			snd_mixer_class_t *c;
			c = list_entry(pos, snd_mixer_class_t, list);
			err = c->event(c, mask, elem, NULL);
			if (err < 0)
				res = err;
		}
	}
	return res;
}


/**
 * \brief Attach an HCTL specified with the CTL device name to an opened mixer
 * \param mixer Mixer handle
 * \param name HCTL name (see #snd_hctl_open)
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_attach(snd_mixer_t *mixer, const char *name)
{
	snd_hctl_t *hctl;
	int err;

	err = snd_hctl_open(&hctl, name, 0);
	if (err < 0)
		return err;
	err = snd_mixer_attach_hctl(mixer, hctl);
	if (err < 0) {
		snd_hctl_close(hctl);
		return err;
	}
	return 0;
}

/**
 * \brief Attach an HCTL to an opened mixer
 * \param mixer Mixer handle
 * \param hctl the HCTL to be attached
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_attach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl)
{
	snd_mixer_slave_t *slave;
	int err;

	assert(hctl);
	slave = calloc(1, sizeof(*slave));
	if (slave == NULL)
		return -ENOMEM;
	err = snd_hctl_nonblock(hctl, 1);
	if (err < 0) {
		snd_hctl_close(hctl);
		free(slave);
		return err;
	}
	snd_hctl_set_callback(hctl, hctl_event_handler);
	snd_hctl_set_callback_private(hctl, mixer);
	slave->hctl = hctl;
	list_add_tail(&slave->list, &mixer->slaves);
	return 0;
}

/**
 * \brief Detach a previously attached HCTL to an opened mixer freeing all related resources
 * \param mixer Mixer handle
 * \param name HCTL previously attached
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_detach(snd_mixer_t *mixer, const char *name)
{
	struct list_head *pos;
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		if (strcmp(name, snd_hctl_name(s->hctl)) == 0) {
			snd_hctl_close(s->hctl);
			list_del(pos);
			free(s);
			return 0;
		}
	}
	return -ENOENT;
}

/**
 * \brief Detach a previously attached HCTL to an opened mixer freeing all related resources
 * \param mixer Mixer handle
 * \param hctl HCTL previously attached
 * \return 0 on success otherwise a negative error code
 *
 * Note: The hctl handle is not closed!
 */
int snd_mixer_detach_hctl(snd_mixer_t *mixer, snd_hctl_t *hctl)
{
	struct list_head *pos;
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		if (hctl == s->hctl) {
			list_del(pos);
			free(s);
			return 0;
		}
	}
	return -ENOENT;
}

/**
 * \brief Obtain a HCTL pointer associated to given name
 * \param mixer Mixer handle
 * \param name HCTL previously attached
 * \param hctl HCTL pointer
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_get_hctl(snd_mixer_t *mixer, const char *name, snd_hctl_t **hctl)
{
	struct list_head *pos;
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		if (strcmp(name, snd_hctl_name(s->hctl)) == 0) {
			*hctl = s->hctl;
			return 0;
		}
	}
	return -ENOENT;
}

static int snd_mixer_throw_event(snd_mixer_t *mixer, unsigned int mask,
			  snd_mixer_elem_t *elem)
{
	mixer->events++;
	if (mixer->callback)
		return mixer->callback(mixer, mask, elem);
	return 0;
}

static int snd_mixer_elem_throw_event(snd_mixer_elem_t *elem, unsigned int mask)
{
	elem->class->mixer->events++;
	if (elem->callback)
		return elem->callback(elem, mask);
	return 0;
}

static int _snd_mixer_find_elem(snd_mixer_t *mixer, snd_mixer_elem_t *elem, int *dir)
{
	unsigned int l, u;
	int c = 0;
	int idx = -1;
	assert(mixer && elem);
	assert(mixer->compare);
	l = 0;
	u = mixer->count;
	while (l < u) {
		idx = (l + u) / 2;
		c = mixer->compare(elem, mixer->pelems[idx]);
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

/**
 * \brief Get private data associated to give mixer element
 * \param elem Mixer element
 * \return private data
 *
 * For use by mixer element class specific code.
 */
void *snd_mixer_elem_get_private(const snd_mixer_elem_t *elem)
{
	return elem->private_data;
}

/**
 * \brief Allocate a new mixer element
 * \param elem Returned mixer element
 * \param type Mixer element type
 * \param compare_weight Mixer element compare weight
 * \param private_data Private data
 * \param private_free Private data free callback
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_new(snd_mixer_elem_t **elem,
		       snd_mixer_elem_type_t type,
		       int compare_weight,
		       void *private_data,
		       void (*private_free)(snd_mixer_elem_t *elem))
{
	snd_mixer_elem_t *melem = calloc(1, sizeof(*melem));
	if (melem == NULL)
		return -ENOMEM;
	melem->type = type;
	melem->compare_weight = compare_weight;
	melem->private_data = private_data;
	melem->private_free = private_free;
	INIT_LIST_HEAD(&melem->helems);
	*elem = melem;
	return 0;
}

/**
 * \brief Add an element for a registered mixer element class
 * \param elem Mixer element
 * \param class Mixer element class
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_add(snd_mixer_elem_t *elem, snd_mixer_class_t *class)
{
	int dir, idx;
	snd_mixer_t *mixer = class->mixer;
	elem->class = class;

	if (mixer->count == mixer->alloc) {
		snd_mixer_elem_t **m;
		mixer->alloc += 32;
		m = realloc(mixer->pelems, sizeof(*m) * mixer->alloc);
		if (!m) {
			mixer->alloc -= 32;
			return -ENOMEM;
		}
		mixer->pelems = m;
	}
	if (mixer->count == 0) {
		list_add_tail(&elem->list, &mixer->elems);
		mixer->pelems[0] = elem;
	} else {
		idx = _snd_mixer_find_elem(mixer, elem, &dir);
		assert(dir != 0);
		if (dir > 0) {
			list_add(&elem->list, &mixer->pelems[idx]->list);
			idx++;
		} else {
			list_add_tail(&elem->list, &mixer->pelems[idx]->list);
		}
		memmove(mixer->pelems + idx + 1,
			mixer->pelems + idx,
			(mixer->count - idx) * sizeof(snd_mixer_elem_t *));
		mixer->pelems[idx] = elem;
	}
	mixer->count++;
	return snd_mixer_throw_event(mixer, SND_CTL_EVENT_MASK_ADD, elem);
}

/**
 * \brief Remove a mixer element
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_remove(snd_mixer_elem_t *elem)
{
	snd_mixer_t *mixer = elem->class->mixer;
	bag_iterator_t i, n;
	int err, idx, dir;
	unsigned int m;
	assert(elem);
	assert(mixer->count);
	idx = _snd_mixer_find_elem(mixer, elem, &dir);
	if (dir != 0)
		return -EINVAL;
	bag_for_each_safe(i, n, &elem->helems) {
		snd_hctl_elem_t *helem = bag_iterator_entry(i);
		snd_mixer_elem_detach(elem, helem);
	}
	err = snd_mixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_REMOVE);
	list_del(&elem->list);
	snd_mixer_elem_free(elem);
	mixer->count--;
	m = mixer->count - idx;
	if (m > 0)
		memmove(mixer->pelems + idx,
			mixer->pelems + idx + 1,
			m * sizeof(snd_mixer_elem_t *));
	return err;
}

/**
 * \brief Free a mixer element
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
void snd_mixer_elem_free(snd_mixer_elem_t *elem)
{
	if (elem->private_free)
		elem->private_free(elem);
	free(elem);
}

/**
 * \brief Mixer element informations are changed
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_info(snd_mixer_elem_t *elem)
{
	return snd_mixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_INFO);
}

/**
 * \brief Mixer element values is changed
 * \param elem Mixer element
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_elem_value(snd_mixer_elem_t *elem)
{
	return snd_mixer_elem_throw_event(elem, SND_CTL_EVENT_MASK_VALUE);
}

/**
 * \brief Register mixer element class
 * \param class Mixer element class
 * \param mixer Mixer handle
 * \return 0 on success otherwise a negative error code
 *
 * For use by mixer element class specific code.
 */
int snd_mixer_class_register(snd_mixer_class_t *class, snd_mixer_t *mixer)
{
	struct list_head *pos;
	class->mixer = mixer;
	list_add_tail(&class->list, &mixer->classes);
	if (!class->event)
		return 0;
	list_for_each(pos, &mixer->slaves) {
		int err;
		snd_mixer_slave_t *slave;
		snd_hctl_elem_t *elem;
		slave = list_entry(pos, snd_mixer_slave_t, list);
		elem = snd_hctl_first_elem(slave->hctl);
		while (elem) {
			err = class->event(class, SND_CTL_EVENT_MASK_ADD, elem, NULL);
			if (err < 0)
				return err;
			elem = snd_hctl_elem_next(elem);
		}
	}
	return 0;
}

/**
 * \brief Unregister mixer element class and remove all its elements
 * \param class Mixer element class
 * \return 0 on success otherwise a negative error code
 *
 * Note that the class structure is also deallocated!
 */
int snd_mixer_class_unregister(snd_mixer_class_t *class)
{
	unsigned int k;
	snd_mixer_elem_t *e;
	snd_mixer_t *mixer = class->mixer;
	for (k = mixer->count; k > 0; k--) {
		e = mixer->pelems[k-1];
		if (e->class == class)
			snd_mixer_elem_remove(e);
	}
	if (class->private_free)
		class->private_free(class);
	list_del(&class->list);
	free(class);
	return 0;
}

/**
 * \brief Load a mixer elements
 * \param mixer Mixer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_load(snd_mixer_t *mixer)
{
	struct list_head *pos;
	list_for_each(pos, &mixer->slaves) {
		int err;
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		err = snd_hctl_load(s->hctl);
		if (err < 0)
			return err;
	}
	return 0;
}

/**
 * \brief Unload all mixer elements and free all related resources
 * \param mixer Mixer handle
 */
void snd_mixer_free(snd_mixer_t *mixer)
{
	struct list_head *pos;
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		snd_hctl_free(s->hctl);
	}
}

/**
 * \brief Close a mixer and free all related resources
 * \param mixer Mixer handle
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_close(snd_mixer_t *mixer)
{
	int res = 0;
	assert(mixer);
	while (!list_empty(&mixer->classes)) {
		snd_mixer_class_t *c;
		c = list_entry(mixer->classes.next, snd_mixer_class_t, list);
		snd_mixer_class_unregister(c);
	}
	assert(list_empty(&mixer->elems));
	assert(mixer->count == 0);
	free(mixer->pelems);
	mixer->pelems = NULL;
	while (!list_empty(&mixer->slaves)) {
		int err;
		snd_mixer_slave_t *s;
		s = list_entry(mixer->slaves.next, snd_mixer_slave_t, list);
		err = snd_hctl_close(s->hctl);
		if (err < 0)
			res = err;
		list_del(&s->list);
		free(s);
	}
	free(mixer);
	return res;
}

static int snd_mixer_compare_default(const snd_mixer_elem_t *c1,
				     const snd_mixer_elem_t *c2)
{
	int d = c1->compare_weight - c2->compare_weight;
	if (d)
		return d;
	assert(c1->class && c1->class->compare);
	assert(c2->class && c2->class->compare);
	assert(c1->class == c2->class);
	return c1->class->compare(c1, c2);
}

static int mixer_compare(const void *a, const void *b)
{
	snd_mixer_t *mixer;

	mixer = (*((const snd_mixer_elem_t * const *)a))->class->mixer;
	return mixer->compare(*(const snd_mixer_elem_t * const *)a, *(const snd_mixer_elem_t * const *)b);
}

static int snd_mixer_sort(snd_mixer_t *mixer)
{
	unsigned int k;
	assert(mixer);
	assert(mixer->compare);
	INIT_LIST_HEAD(&mixer->elems);
	qsort(mixer->pelems, mixer->count, sizeof(snd_mixer_elem_t *), mixer_compare);
	for (k = 0; k < mixer->count; k++)
		list_add_tail(&mixer->pelems[k]->list, &mixer->elems);
	return 0;
}

/**
 * \brief Change mixer compare function and reorder elements
 * \param mixer Mixer handle
 * \param compare Element compare function
 * \return 0 on success otherwise a negative error code
 */
int snd_mixer_set_compare(snd_mixer_t *mixer, snd_mixer_compare_t compare)
{
	snd_mixer_compare_t compare_old;
	int err;

	assert(mixer);
	compare_old = mixer->compare;
	mixer->compare = compare == NULL ? snd_mixer_compare_default : compare;
	if ((err = snd_mixer_sort(mixer)) < 0) {
		mixer->compare = compare_old;
		return err;
	}
	return 0;
}

/**
 * \brief get count of poll descriptors for mixer handle
 * \param mixer Mixer handle
 * \return count of poll descriptors
 */
int snd_mixer_poll_descriptors_count(snd_mixer_t *mixer)
{
	struct list_head *pos;
	unsigned int c = 0;
	assert(mixer);
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		int n;
		s = list_entry(pos, snd_mixer_slave_t, list);
		n = snd_hctl_poll_descriptors_count(s->hctl);
		if (n < 0)
			return n;
		c += n;
	}
	return c;
}

/**
 * \brief get poll descriptors
 * \param mixer Mixer handle
 * \param pfds array of poll descriptors
 * \param space space in the poll descriptor array
 * \return count of filled descriptors
 */
int snd_mixer_poll_descriptors(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int space)
{
	struct list_head *pos;
	unsigned int count = 0;
	assert(mixer);
	list_for_each(pos, &mixer->slaves) {
		snd_mixer_slave_t *s;
		int n;
		s = list_entry(pos, snd_mixer_slave_t, list);
		n = snd_hctl_poll_descriptors(s->hctl, pfds, space);
		if (n < 0)
			return n;
		if (space >= (unsigned int) n) {
			count += n;
			space -= n;
			pfds += n;
		} else
			space = 0;
	}
	return count;
}

/**
 * \brief get returned events from poll descriptors
 * \param mixer Mixer handle
 * \param pfds array of poll descriptors
 * \param nfds count of poll descriptors
 * \param revents returned events
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_poll_descriptors_revents(snd_mixer_t *mixer, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
	unsigned int idx;
	unsigned short res;
        assert(mixer && pfds && revents);
	if (nfds == 0)
		return -EINVAL;
	res = 0;
	for (idx = 0; idx < nfds; idx++)
		res |= pfds->revents & (POLLIN|POLLERR|POLLNVAL);
	*revents = res;
	return 0;
}

/**
 * \brief Wait for a mixer to become ready (i.e. at least one event pending)
 * \param mixer Mixer handle
 * \param timeout maximum time in milliseconds to wait
 * \return 0 otherwise a negative error code on failure
 */
int snd_mixer_wait(snd_mixer_t *mixer, int timeout)
{
	struct pollfd spfds[16];
	struct pollfd *pfds = spfds;
	int err;
	int count;
	count = snd_mixer_poll_descriptors(mixer, pfds, sizeof(spfds) / sizeof(spfds[0]));
	if (count < 0)
		return count;
	if ((unsigned int) count > sizeof(spfds) / sizeof(spfds[0])) {
		pfds = malloc(count * sizeof(*pfds));
		if (!pfds)
			return -ENOMEM;
		err = snd_mixer_poll_descriptors(mixer, pfds, 
						 (unsigned int) count);
		assert(err == count);
	}
	err = poll(pfds, (unsigned int) count, timeout);
	if (err < 0)
		return -errno;
	return 0;
}

/**
 * \brief get first element for a mixer
 * \param mixer Mixer handle
 * \return pointer to first element
 */
snd_mixer_elem_t *snd_mixer_first_elem(snd_mixer_t *mixer)
{
	assert(mixer);
	if (list_empty(&mixer->elems))
		return NULL;
	return list_entry(mixer->elems.next, snd_mixer_elem_t, list);
}

/**
 * \brief get last element for a mixer
 * \param mixer Mixer handle
 * \return pointer to last element
 */
snd_mixer_elem_t *snd_mixer_last_elem(snd_mixer_t *mixer)
{
	assert(mixer);
	if (list_empty(&mixer->elems))
		return NULL;
	return list_entry(mixer->elems.prev, snd_mixer_elem_t, list);
}

/**
 * \brief get next mixer element
 * \param elem mixer element
 * \return pointer to next element
 */
snd_mixer_elem_t *snd_mixer_elem_next(snd_mixer_elem_t *elem)
{
	assert(elem);
	if (elem->list.next == &elem->class->mixer->elems)
		return NULL;
	return list_entry(elem->list.next, snd_mixer_elem_t, list);
}

/**
 * \brief get previous mixer element
 * \param elem mixer element
 * \return pointer to previous element
 */
snd_mixer_elem_t *snd_mixer_elem_prev(snd_mixer_elem_t *elem)
{
	assert(elem);
	if (elem->list.prev == &elem->class->mixer->elems)
		return NULL;
	return list_entry(elem->list.prev, snd_mixer_elem_t, list);
}

/**
 * \brief Handle pending mixer events invoking callbacks
 * \param mixer Mixer handle
 * \return Number of events that occured on success, otherwise a negative error code on failure
 */
int snd_mixer_handle_events(snd_mixer_t *mixer)
{
	struct list_head *pos;
	assert(mixer);
	mixer->events = 0;
	list_for_each(pos, &mixer->slaves) {
		int err;
		snd_mixer_slave_t *s;
		s = list_entry(pos, snd_mixer_slave_t, list);
		err = snd_hctl_handle_events(s->hctl);
		if (err < 0)
			return err;
	}
	return mixer->events;
}

/**
 * \brief Set callback function for a mixer
 * \param obj mixer handle
 * \param val callback function
 */
void snd_mixer_set_callback(snd_mixer_t *obj, snd_mixer_callback_t val)
{
	assert(obj);
	obj->callback = val;
}

/**
 * \brief Set callback private value for a mixer
 * \param mixer mixer handle
 * \param val callback private value
 */
void snd_mixer_set_callback_private(snd_mixer_t *mixer, void * val)
{
	assert(mixer);
	mixer->callback_private = val;
}

/**
 * \brief Get callback private value for a mixer
 * \param mixer mixer handle
 * \return callback private value
 */
void * snd_mixer_get_callback_private(const snd_mixer_t *mixer)
{
	assert(mixer);
	return mixer->callback_private;
}

/**
 * \brief Get elements count for a mixer
 * \param mixer mixer handle
 * \return elements count
 */
unsigned int snd_mixer_get_count(const snd_mixer_t *mixer)
{
	assert(mixer);
	return mixer->count;
}

/**
 * \brief Set callback function for a mixer element
 * \param mixer mixer element
 * \param val callback function
 */
void snd_mixer_elem_set_callback(snd_mixer_elem_t *mixer, snd_mixer_elem_callback_t val)
{
	assert(mixer);
	mixer->callback = val;
}

/**
 * \brief Set callback private value for a mixer element
 * \param mixer mixer element
 * \param val callback private value
 */
void snd_mixer_elem_set_callback_private(snd_mixer_elem_t *mixer, void * val)
{
	assert(mixer);
	mixer->callback_private = val;
}

/**
 * \brief Get callback private value for a mixer element
 * \param mixer mixer element
 * \return callback private value
 */
void * snd_mixer_elem_get_callback_private(const snd_mixer_elem_t *mixer)
{
	assert(mixer);
	return mixer->callback_private;
}

/**
 * \brief Get type for a mixer element
 * \param mixer mixer element
 * \return mixer element type
 */
snd_mixer_elem_type_t snd_mixer_elem_get_type(const snd_mixer_elem_t *mixer)
{
	assert(mixer);
	return mixer->type;
}


/**
 * \brief get size of #snd_mixer_class_t
 * \return size in bytes
 */
size_t snd_mixer_class_sizeof()
{
	return sizeof(snd_mixer_class_t);
}

/**
 * \brief allocate an invalid #snd_mixer_class_t using standard malloc
 * \param ptr returned pointer
 * \return 0 on success otherwise negative error code
 */
int snd_mixer_class_malloc(snd_mixer_class_t **ptr)
{
	assert(ptr);
	*ptr = calloc(1, sizeof(snd_mixer_class_t));
	if (!*ptr)
		return -ENOMEM;
	return 0;
}

/**
 * \brief frees a previously allocated #snd_mixer_class_t
 * \param obj pointer to object to free
 */
void snd_mixer_class_free(snd_mixer_class_t *obj)
{
	if (obj->private_free)
		obj->private_free(obj);
	free(obj);
}

/**
 * \brief copy one #snd_mixer_class_t to another
 * \param dst pointer to destination
 * \param src pointer to source
 */
void snd_mixer_class_copy(snd_mixer_class_t *dst, const snd_mixer_class_t *src)
{
	assert(dst && src);
	*dst = *src;
}

/**
 * \brief Get a mixer associated to given mixer class
 * \param obj Mixer simple class identifier
 * \return mixer pointer
 */
snd_mixer_t *snd_mixer_class_get_mixer(const snd_mixer_class_t *obj)
{
	assert(obj);
	return obj->mixer;
}

/**
 * \brief Get mixer event callback associated to given mixer class
 * \param obj Mixer simple class identifier
 * \return event callback pointer
 */
snd_mixer_event_t snd_mixer_class_get_event(const snd_mixer_class_t *obj)
{
	assert(obj);
	return obj->event;
}

/**
 * \brief Get mixer private data associated to given mixer class
 * \param obj Mixer simple class identifier
 * \return event callback pointer
 */
void *snd_mixer_class_get_private(const snd_mixer_class_t *obj)
{
	assert(obj);
	return obj->private_data;
}


/**
 * \brief Get mixer compare callback associated to given mixer class
 * \param obj Mixer simple class identifier
 * \return event callback pointer
 */
snd_mixer_compare_t snd_mixer_class_get_compare(const snd_mixer_class_t *obj)
{
	assert(obj);
	return obj->compare;
}

/**
 * \brief Set mixer event callback to given mixer class
 * \param obj Mixer simple class identifier
 * \param event Event callback
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_class_set_event(snd_mixer_class_t *obj, snd_mixer_event_t event)
{
	assert(obj);
	obj->event = event;
	return 0;
}

/**
 * \brief Set mixer private data to given mixer class
 * \param obj Mixer simple class identifier
 * \param private_data class private data
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_class_set_private(snd_mixer_class_t *obj, void *private_data)
{
	assert(obj);
	obj->private_data = private_data;
	return 0;
}

/**
 * \brief Set mixer private data free callback to given mixer class
 * \param obj Mixer simple class identifier
 * \param private_free Mixer class private data free callback
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_class_set_private_free(snd_mixer_class_t *obj, void (*private_free)(snd_mixer_class_t *class))
{
	assert(obj);
	obj->private_free = private_free;
	return 0;
}

/**
 * \brief Set mixer compare callback to given mixer class
 * \param obj Mixer simple class identifier
 * \param compare the compare callback to be used
 * \return zero if success, otherwise a negative error code
 */
int snd_mixer_class_set_compare(snd_mixer_class_t *obj, snd_mixer_compare_t compare)
{
	assert(obj);
	obj->compare = compare;
	return 0;
}
