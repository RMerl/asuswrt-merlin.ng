/*
 *  Mixer Interface - simple abstact module - base library
 *  Copyright (c) 2005 by Jaroslav Kysela <perex@perex.cz>
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

#ifndef __SMIXER_BASE_H

#include "list.h"

#define MAX_CHANNEL	6

#define SID_MASTER	0
#define SID_HEADPHONE	1
#define SID_FRONT	2
#define SID_PCM		3
#define SID_CD		4

struct melem_sids {
	unsigned short sid;
	const char *sname;
	unsigned short sindex;
	unsigned short weight;
	unsigned int chanmap[2];
	struct sm_elem_ops *sops;
};

#define PURPOSE_VOLUME		0
#define PURPOSE_SWITCH		1
#define PURPOSE_ENUMLIST	2

struct helem_selector {
	snd_ctl_elem_iface_t iface;
	const char *name;
	unsigned short index;
	unsigned short sid;
	unsigned short purpose;
	unsigned short caps;
};

struct helem_base {
	struct list_head list;
	snd_hctl_elem_t *helem;
	unsigned short purpose;
	unsigned int caps;
	unsigned int inactive: 1;
	long min, max;
	unsigned int count;
};

struct selem_base {
	sm_selem_t selem;
	struct list_head helems;
	unsigned short sid;
	struct {
		unsigned int chanmap;
		unsigned int forced_range: 1;
		long min, max;
		long vol[MAX_CHANNEL];
	} dir[2];
};

struct bclass_selector {
	struct list_head list;
	struct helem_selector *selectors;
	unsigned int count;
};

struct bclass_sid {
	struct list_head list;
	struct melem_sids *sids;
	unsigned int count;
};

typedef struct bclass_base_ops {
	int (*event)(snd_mixer_class_t *class, unsigned int mask,
		     snd_hctl_elem_t *helem, snd_mixer_elem_t *melem);
	int (*selreg)(snd_mixer_class_t *class,
		      struct helem_selector *selectors,
		      unsigned int count);
	int (*sidreg)(snd_mixer_class_t *class,
		      struct melem_sids *sids,
		      unsigned int count);
} bclass_base_ops_t;

struct bclass_private {
	struct list_head selectors;
	struct list_head sids;
	void *dl_sbase;
	bclass_base_ops_t ops;
};

int mixer_simple_basic_dlopen(snd_mixer_class_t *class,
			      bclass_base_ops_t **ops);

#endif /* __SMIXER_BASE_H */
