/*
 *  Mixer Interface - local header file
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

#include "local.h"

typedef struct _bag1 {
	void *ptr;
	struct list_head list;
} bag1_t;

typedef struct list_head bag_t;

int bag_new(bag_t **bag);
void bag_free(bag_t *bag);
int bag_add(bag_t *bag, void *ptr);
int bag_del(bag_t *bag, void *ptr);
int bag_empty(bag_t *bag);
void bag_del_all(bag_t *bag);

typedef struct list_head *bag_iterator_t;

#define bag_iterator_entry(i) (list_entry((i), bag1_t, list)->ptr)
#define bag_for_each(pos, bag) list_for_each(pos, bag)
#define bag_for_each_safe(pos, next, bag) list_for_each_safe(pos, next, bag)

struct _snd_mixer_class {
	struct list_head list;
	snd_mixer_t *mixer;
	snd_mixer_event_t event;
	void *private_data;		
	void (*private_free)(snd_mixer_class_t *class);
	snd_mixer_compare_t compare;
};

struct _snd_mixer_elem {
	snd_mixer_elem_type_t type;
	struct list_head list;		/* links for list of all elems */
	snd_mixer_class_t *class;
	void *private_data;
	void (*private_free)(snd_mixer_elem_t *elem);
	snd_mixer_elem_callback_t callback;
	void *callback_private;
	bag_t helems;
	int compare_weight;		/* compare weight (reversed) */
};

struct _snd_mixer {
	struct list_head slaves;	/* list of all slaves */
	struct list_head classes;	/* list of all elem classes */
	struct list_head elems;		/* list of all elems */
	snd_mixer_elem_t **pelems;	/* array of all elems */
	unsigned int count;
	unsigned int alloc;
	unsigned int events;
	snd_mixer_callback_t callback;
	void *callback_private;
	snd_mixer_compare_t compare;
};

struct _snd_mixer_selem_id {
	char name[60];
	unsigned int index;
};
