/*
 *  Mixer Interface - simple abstact module - base library (dlopen function)
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
#include <dlfcn.h>
#include "config.h"
#include "asoundlib.h"
#include "mixer_abst.h"
#include "sbase.h"

#define SO_PATH ALSA_PLUGIN_DIR "/smixer"

int mixer_simple_basic_dlopen(snd_mixer_class_t *class,
			      bclass_base_ops_t **ops)
{
	struct bclass_private *priv = snd_mixer_sbasic_get_private(class);
	const char *lib = "smixer-sbase.so";
	void (*initpriv)(snd_mixer_class_t *class, struct bclass_private *priv);
	char *xlib, *path;
	void *h;
	int initflag = 0;

	if (priv == NULL) {
		priv = calloc(1, sizeof(*priv));
		if (priv == NULL)
			return -ENOMEM;
		initflag = 1;
	}
	path = getenv("ALSA_MIXER_SIMPLE_MODULES");
	if (!path)
		path = SO_PATH;
	xlib = malloc(strlen(lib) + strlen(path) + 1 + 1);
	if (xlib == NULL) {
		if (initflag)
			free(priv);
		return -ENOMEM;
	}
	strcpy(xlib, path);
	strcat(xlib, "/");
	strcat(xlib, lib);
	h = snd_dlopen(xlib, RTLD_NOW);
	if (h == NULL) {
		SNDERR("Unable to open library '%s'", xlib);
		goto __error;
	}
	initpriv = dlsym(h, "alsa_mixer_sbasic_initpriv");
	if (initpriv == NULL) {
		SNDERR("Symbol 'alsa_mixer_sbasic_initpriv' was not found in '%s'", xlib);
		goto __error;
	}
	priv->ops.event = dlsym(h, "alsa_mixer_sbasic_event");
	if (priv->ops.event == NULL) {
		SNDERR("Symbol 'alsa_mixer_sbasic_event' was not found in '%s'", xlib);
		goto __error;
	}
	priv->ops.selreg = dlsym(h, "alsa_mixer_sbasic_selreg");
	if (priv->ops.selreg == NULL) {
		SNDERR("Symbol 'alsa_mixer_sbasic_selreg' was not found in '%s'", xlib);
		goto __error;
	}
	priv->ops.sidreg = dlsym(h, "alsa_mixer_sbasic_sidreg");
	if (priv->ops.sidreg == NULL) {
		SNDERR("Symbol 'alsa_mixer_sbasic_sidreg' was not found in '%s'", xlib);
		goto __error;
	}
	free(xlib);
	if (initflag)
		initpriv(class, priv);
	priv->dl_sbase = h;
	if (ops)
		*ops = &priv->ops;
	return 1;

      __error:
      	if (initflag)
      		free(priv);
	if (h == NULL)
		snd_dlclose(h);
	free(xlib);
	return -ENXIO;
}
