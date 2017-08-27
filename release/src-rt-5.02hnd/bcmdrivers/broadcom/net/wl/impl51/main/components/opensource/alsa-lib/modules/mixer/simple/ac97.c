/*
 *  Mixer Interface - AC97 simple abstact module
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

static struct sm_elem_ops simple_ac97_ops;

struct melem_sids sids[] = {
	{
		.sid	= SID_MASTER,
		.sname	= "Master",
		.sindex	= 0,
		.weight = 1,
		.chanmap = { 3, 0 },
		.sops = &simple_ac97_ops,
	}
};

#define SELECTORS (sizeof(selectors)/sizeof(selectors[0]))

struct helem_selector selectors[] = {
	{
		.iface =	SND_CTL_ELEM_IFACE_MIXER,
		.name =		"Master Playback Volume",
		.index = 	0,
		.sid =		SID_MASTER,
		.purpose =	PURPOSE_VOLUME,
		.caps = 	SM_CAP_PVOLUME,
	},
	{
		.iface =	SND_CTL_ELEM_IFACE_MIXER,
		.name =		"Master Playback Switch",
		.index = 	0,
		.sid =		SID_MASTER,
		.purpose =	PURPOSE_SWITCH,
		.caps = 	SM_CAP_PSWITCH,
	}
};

int alsa_mixer_simple_event(snd_mixer_class_t *class, unsigned int mask,
			    snd_hctl_elem_t *helem, snd_mixer_elem_t *melem)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	return priv->ops.event(class, mask, helem, melem);
}

int alsa_mixer_simple_init(snd_mixer_class_t *class)
{
	struct bclass_base_ops *ops;
	int err;
	
	err = mixer_simple_basic_dlopen(class, &ops);
	if (err < 0)
		return 0;
	err = ops->selreg(class, selectors, SELECTORS);
	if (err < 0)
		return err;
	err = ops->sidreg(class, sids, SELECTORS);
	if (err < 0)
		return err;
	return 0;
}
