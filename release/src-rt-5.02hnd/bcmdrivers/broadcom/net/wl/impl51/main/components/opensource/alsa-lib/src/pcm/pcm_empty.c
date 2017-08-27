/**
 * \file pcm/pcm_empty.c
 * \ingroup PCM_Plugins
 * \brief PCM Null Plugin Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2006
 */
/*
 *  PCM - Null plugin
 *  Copyright (c) 2006 by Jaroslav Kysela <perex@perex.cz>
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
  
#include "pcm_local.h"
#include "pcm_plugin.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_empty = "";
#endif

/*! \page pcm_plugins

\section pcm_plugins_null Plugin: Null

This plugin discards contents of a PCM stream or creates a stream with zero
samples.

Note: This implementation uses devices /dev/null (playback, must be writable)
and /dev/full (capture, must be readable).

\code
pcm.name {
        type null               # Null PCM
}
\endcode

\subsection pcm_plugins_null_funcref Function reference

<UL>
  <LI>_snd_pcm_empty_open()
</UL>

*/

/**
 * \brief Creates a new Empty PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with empty PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_empty_open(snd_pcm_t **pcmp, const char *name ATTRIBUTE_UNUSED,
		        snd_config_t *root, snd_config_t *conf, 
		        snd_pcm_stream_t stream, int mode)
{
	snd_config_t *slave = NULL, *sconf;
	snd_config_iterator_t i, next;
	int err;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "slave") == 0) {
			slave = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 0);
	if (err < 0)
		return err;
	err = snd_pcm_open_named_slave(pcmp, name, root, sconf, stream,
				       mode, conf);
	snd_config_delete(sconf);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_empty_open, SND_PCM_DLSYM_VERSION);
#endif
