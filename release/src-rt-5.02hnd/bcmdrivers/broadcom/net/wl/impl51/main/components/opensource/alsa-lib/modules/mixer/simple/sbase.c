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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include "asoundlib.h"
#include "mixer_abst.h"
#include "sbase.h"

/*
 * Prototypes
 */

static int selem_read(snd_mixer_elem_t *elem);

/*
 * Helpers
 */

static unsigned int chanmap_to_channels(unsigned int chanmap)
{
	unsigned int i, res;
	
	for (i = 0, res = 0; i < MAX_CHANNEL; i++)
		if (chanmap & (1 << i))
			res++;
	return res;
}


static void update_ranges(struct selem_base *s)
{
	static unsigned int mask[2] = { SM_CAP_PVOLUME, SM_CAP_CVOLUME };
	static unsigned int gmask[2] = { SM_CAP_GVOLUME, SM_CAP_GVOLUME };
	unsigned int dir, ok_flag;
	struct list_head *pos;
	struct helem_base *helem;
	
	for (dir = 0; dir < 2; dir++) {
		s->dir[dir].min = 0;
		s->dir[dir].max = 0;
		ok_flag = 0;
		list_for_each(pos, &s->helems) {
			helem = list_entry(pos, struct helem_base, list);
			printf("min = %li, max = %li\n", helem->min, helem->max);
			if (helem->caps & mask[dir]) {
				s->dir[dir].min = helem->min;
				s->dir[dir].max = helem->max;
				ok_flag = 1;
				break;
			}
		}
		if (ok_flag)
			continue;
		list_for_each(pos, &s->helems) {
			helem = list_entry(pos, struct helem_base, list);
			if (helem->caps & gmask[dir]) {
				s->dir[dir].min = helem->min;
				s->dir[dir].max = helem->max;
				break;
			}
		}
	}
}

/*
 * Simple Mixer Operations
 */

static int is_ops(snd_mixer_elem_t *elem, int dir, int cmd, int val)
{
	struct selem_base *s = snd_mixer_elem_get_private(elem);

	switch (cmd) {

	case SM_OPS_IS_ACTIVE: {
		struct list_head *pos;
		struct helem_base *helem;
		list_for_each(pos, &s->helems) {
			helem = list_entry(pos, struct helem_base, list);
			if (helem->inactive)
				return 0;
		}
		return 1;
	}

	case SM_OPS_IS_MONO:
		return chanmap_to_channels(s->dir[dir].chanmap) == 1;

	case SM_OPS_IS_CHANNEL:
		if (val > MAX_CHANNEL)
			return 0;
		return !!((1 << val) & s->dir[dir].chanmap);

	case SM_OPS_IS_ENUMERATED: {
		struct helem_base *helem;
		helem = list_entry(s->helems.next, struct helem_base, list);
		return !!(helem->purpose == PURPOSE_ENUMLIST);
	}
	
	case SM_OPS_IS_ENUMCNT: {
		struct helem_base *helem;
		helem = list_entry(s->helems.next, struct helem_base, list);
		return helem->max;
	}

	}
	
	return 1;
}

static int get_range_ops(snd_mixer_elem_t *elem, int dir,
			 long *min, long *max)
{
	struct selem_base *s = snd_mixer_elem_get_private(elem);
	
	*min = s->dir[dir].min;
	*max = s->dir[dir].max;

	return 0;
}

static int get_dB_range_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			    int dir ATTRIBUTE_UNUSED,
			    long *min ATTRIBUTE_UNUSED,
			    long *max ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int set_range_ops(snd_mixer_elem_t *elem, int dir,
			 long min, long max)
{
	struct selem_base *s = snd_mixer_elem_get_private(elem);
	int err;

	s->dir[dir].forced_range = 1;
	s->dir[dir].min = min;
	s->dir[dir].max = max;
	
	if ((err = selem_read(elem)) < 0)
		return err;
	return 0;
}

static int get_volume_ops(snd_mixer_elem_t *elem, int dir,
			  snd_mixer_selem_channel_id_t channel, long *value)
{
	struct selem_base *s = snd_mixer_elem_get_private(elem);
	
	*value = s->dir[dir].vol[channel];
	return 0;
}

static int get_dB_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
		      int dir ATTRIBUTE_UNUSED,
		      snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
		      long *value ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int get_switch_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			  int dir ATTRIBUTE_UNUSED,
			  snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
			  int *value)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem); */
	*value = 0;
	return 0;
}

static int set_volume_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			  int dir ATTRIBUTE_UNUSED,
			  snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
			  long value ATTRIBUTE_UNUSED)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem); */
	return 0;
}

static int set_dB_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
		      int dir ATTRIBUTE_UNUSED,
		      snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
		      long value ATTRIBUTE_UNUSED,
		      int xdir ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}

static int set_switch_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			  int dir ATTRIBUTE_UNUSED,
			  snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
			  int value ATTRIBUTE_UNUSED)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem); */
	/* int changed; */
	return 0;
}

static int enum_item_name_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			      unsigned int item ATTRIBUTE_UNUSED,
			      size_t maxlen ATTRIBUTE_UNUSED,
			      char *buf ATTRIBUTE_UNUSED)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem);*/
	return 0;
}

static int get_enum_item_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			     snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
			     unsigned int *itemp ATTRIBUTE_UNUSED)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem); */
	return 0;
}

static int set_enum_item_ops(snd_mixer_elem_t *elem ATTRIBUTE_UNUSED,
			     snd_mixer_selem_channel_id_t channel ATTRIBUTE_UNUSED,
			     unsigned int item ATTRIBUTE_UNUSED)
{
	/* struct selem_base *s = snd_mixer_elem_get_private(elem); */
	return 0;
}

static struct sm_elem_ops simple_ac97_ops = {
	.is		= is_ops,
	.get_range	= get_range_ops,
	.get_dB_range	= get_dB_range_ops,
	.set_range	= set_range_ops,
	.get_volume	= get_volume_ops,
	.get_dB		= get_dB_ops,
	.set_volume	= set_volume_ops,
	.set_dB		= set_dB_ops,
	.get_switch	= get_switch_ops,
	.set_switch	= set_switch_ops,
	.enum_item_name	= enum_item_name_ops,
	.get_enum_item	= get_enum_item_ops,
	.set_enum_item	= set_enum_item_ops
};

/*
 * event handling
 */

static int selem_read(snd_mixer_elem_t *elem)
{
	printf("elem read: %p\n", elem);
	return 0;
}

static int simple_event_remove(snd_hctl_elem_t *helem,
			       snd_mixer_elem_t *melem ATTRIBUTE_UNUSED)
{
	printf("event remove: %p\n", helem);
	return 0;
}

static void selem_free(snd_mixer_elem_t *elem)
{
	struct selem_base *simple = snd_mixer_elem_get_private(elem);
	struct helem_base *hsimple;
	struct list_head *pos, *npos;

	if (simple->selem.id)
		snd_mixer_selem_id_free(simple->selem.id);
	list_for_each_safe(pos, npos, &simple->helems) {
		hsimple = list_entry(pos, struct helem_base, list);
		free(hsimple);
	}
	free(simple);
}

static int simple_event_add1(snd_mixer_class_t *class,
			     snd_hctl_elem_t *helem,
			     struct helem_selector *sel)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	snd_mixer_elem_t *melem;
	snd_mixer_selem_id_t *id;
	snd_ctl_elem_info_t *info;
	struct selem_base *simple;
	struct helem_base *hsimple;
	snd_ctl_elem_type_t ctype;
	unsigned long values;
	long min, max;
	int err, new = 0;
	struct list_head *pos;
	struct bclass_sid *bsid;
	struct melem_sids *sid;
	unsigned int ui;
	
	list_for_each(pos, &priv->sids) {
		bsid = list_entry(pos, struct bclass_sid, list);
		for (ui = 0; ui < bsid->count; ui++) {
			if (bsid->sids[ui].sid == sel->sid) {
				sid = &bsid->sids[ui];
				goto __sid_ok;
			}
		}
	}
	return 0;

      __sid_ok:
	snd_ctl_elem_info_alloca(&info);
	err = snd_hctl_elem_info(helem, info);
	if (err < 0)
		return err;
	ctype = snd_ctl_elem_info_get_type(info);
	values = snd_ctl_elem_info_get_count(info);
	switch (ctype) {
	case SND_CTL_ELEM_TYPE_ENUMERATED:
		min = 0;
		max = snd_ctl_elem_info_get_items(info);
		break;
	case SND_CTL_ELEM_TYPE_INTEGER:
		min = snd_ctl_elem_info_get_min(info);
		max = snd_ctl_elem_info_get_max(info);
		break;
	default:
		min = max = 0;
		break;
	}
	
	printf("event add: %p, %p (%s)\n", helem, sel, snd_hctl_elem_get_name(helem));
	if (snd_mixer_selem_id_malloc(&id))
		return -ENOMEM;
	hsimple = calloc(1, sizeof(*hsimple));
	if (hsimple == NULL) {
		snd_mixer_selem_id_free(id);
		return -ENOMEM;
	}
	switch (sel->purpose) {
	case PURPOSE_SWITCH:
		if (ctype != SND_CTL_ELEM_TYPE_BOOLEAN) {
		      __invalid_type:
		      	snd_mixer_selem_id_free(id);
			return -EINVAL;
		}
		break;
	case PURPOSE_VOLUME:
		if (ctype != SND_CTL_ELEM_TYPE_INTEGER)
			goto __invalid_type;
		break;
	}
	hsimple->purpose = sel->purpose;
	hsimple->caps = sel->caps;
	hsimple->min = min;
	hsimple->max = max;
	snd_mixer_selem_id_set_name(id, sid->sname);
	snd_mixer_selem_id_set_index(id, sid->sindex);
	melem = snd_mixer_find_selem(snd_mixer_class_get_mixer(class), id);
	if (!melem) {
		simple = calloc(1, sizeof(*simple));
		if (!simple) {
			snd_mixer_selem_id_free(id);
			free(hsimple);
			return -ENOMEM;
		}
		simple->selem.id = id;
		simple->selem.ops = &simple_ac97_ops;
		INIT_LIST_HEAD(&simple->helems);
		simple->sid = sel->sid;
		err = snd_mixer_elem_new(&melem, SND_MIXER_ELEM_SIMPLE,
					 sid->weight,
					 simple, selem_free);
		if (err < 0) {
			snd_mixer_selem_id_free(id);
			free(hsimple);
			free(simple);
			return err;
		}
		new = 1;
	} else {
		simple = snd_mixer_elem_get_private(melem);
		snd_mixer_selem_id_free(id);
	}
	list_add_tail(&hsimple->list, &simple->helems);
	hsimple->inactive = snd_ctl_elem_info_is_inactive(info);
	err = snd_mixer_elem_attach(melem, helem);
	if (err < 0)
		goto __error;
	simple->dir[0].chanmap |= sid->chanmap[0];
	simple->dir[1].chanmap |= sid->chanmap[1];
	simple->selem.caps |= hsimple->caps;
	update_ranges(simple);
	if (new)
		err = snd_mixer_elem_add(melem, class);
	else
		err = snd_mixer_elem_info(melem);
	if (err < 0)
		return err;
	err = selem_read(melem);
	if (err < 0)
		return err;
	if (err)
		err = snd_mixer_elem_value(melem);
	return err;
      __error:
      	if (new)
      		snd_mixer_elem_free(melem);
      	return -EINVAL;
}

static int simple_event_add(snd_mixer_class_t *class, snd_hctl_elem_t *helem)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	struct bclass_selector *sel;
	struct helem_selector *hsel;
	struct list_head *pos;
	snd_ctl_elem_iface_t iface = snd_hctl_elem_get_interface(helem);
	const char *name = snd_hctl_elem_get_name(helem);
	unsigned int index = snd_hctl_elem_get_index(helem);
	unsigned int ui;
	int err;

	list_for_each(pos, &priv->selectors) {
		sel = list_entry(pos, struct bclass_selector, list);
		for (ui = 0; ui < sel->count; ui++) {
			hsel = &sel->selectors[ui];
			if (hsel->iface == iface && !strcmp(hsel->name, name) && hsel->index == index) {
				err = simple_event_add1(class, helem, hsel);
				if (err < 0)
					return err;	/* early exit? */
			}
		}
	}
	return 0;
}

int alsa_mixer_sbasic_event(snd_mixer_class_t *class, unsigned int mask,
			    snd_hctl_elem_t *helem, snd_mixer_elem_t *melem)
{
	int err;
	if (mask == SND_CTL_EVENT_MASK_REMOVE)
		return simple_event_remove(helem, melem);
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		err = simple_event_add(class, helem);
		if (err < 0)
			return err;
	}
	if (mask & SND_CTL_EVENT_MASK_INFO) {
		err = simple_event_remove(helem, melem);
		if (err < 0)
			return err;
		err = simple_event_add(class, helem);
		if (err < 0)
			return err;
		return 0;
	}
	if (mask & SND_CTL_EVENT_MASK_VALUE) {
		err = selem_read(melem);
		if (err < 0)
			return err;
		if (err) {
			err = snd_mixer_elem_value(melem);
			if (err < 0)
				return err;
		}
	}
	return 0;
}

static void sbasic_cpriv_free(snd_mixer_class_t *class)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	struct bclass_selector *sel;
	struct bclass_sid *sid;
	struct list_head *pos, *pos1;

	list_for_each_safe(pos, pos1, &priv->selectors) {
		sel = list_entry(pos, struct bclass_selector, list);
		free(sel);
	}
	list_for_each_safe(pos, pos1, &priv->sids) {
		sid = list_entry(pos, struct bclass_sid, list);
		free(sid);
	}
	free(priv);
}

void alsa_mixer_sbasic_initpriv(snd_mixer_class_t *class,
				struct bclass_private *priv)
{
	INIT_LIST_HEAD(&priv->selectors);
	INIT_LIST_HEAD(&priv->sids);
	snd_mixer_sbasic_set_private(class, priv);
	snd_mixer_sbasic_set_private_free(class, sbasic_cpriv_free);
}

int alsa_mixer_sbasic_selreg(snd_mixer_class_t *class,
			     struct helem_selector *selectors,
			     unsigned int count)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	struct bclass_selector *sel = calloc(1, sizeof(*sel));

	if (sel == NULL)
		return -ENOMEM;
	if (priv == NULL) {
		priv = calloc(1, sizeof(*priv));
		if (priv == NULL) {
			free(sel);
			return -ENOMEM;
		}
	}
	sel->selectors = selectors;
	sel->count = count;
	list_add_tail(&sel->list, &priv->selectors);
	return 0;
}

int alsa_mixer_sbasic_sidreg(snd_mixer_class_t *class,
			     struct melem_sids *sids,
			     unsigned int count)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	struct bclass_sid *sid = calloc(1, sizeof(*sid));

	if (sid == NULL)
		return -ENOMEM;
	if (priv == NULL) {
		priv = calloc(1, sizeof(*priv));
		if (priv == NULL) {
			free(sid);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&priv->selectors);
		INIT_LIST_HEAD(&priv->sids);
		snd_mixer_sbasic_set_private(class, priv);
		snd_mixer_sbasic_set_private_free(class, sbasic_cpriv_free);
	}
	sid->sids = sids;
	sid->count = count;
	list_add(&sid->list, &priv->sids);
	return 0;
}
