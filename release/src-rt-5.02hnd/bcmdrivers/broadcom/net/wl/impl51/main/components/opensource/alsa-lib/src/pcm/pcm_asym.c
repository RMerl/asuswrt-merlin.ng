/**
 * \file pcm/pcm_asym.c
 * \ingroup PCM_Plugins
 * \brief PCM Asymmetrical Plugin Interface
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2003
 */

#include "pcm_local.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_asym = "";
#endif

/*! \page pcm_plugins

\section pcm_plugins_asym Plugin: asym

This plugin is a combination of playback and capture PCM streams.
Slave PCMs can be defined asymmetrically for both directions.

\code
pcm.name {
        type asym               # Asym PCM
        playback STR            # Playback slave name
        # or
        playback {              # Playback slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
        capture STR             # Capture slave name
        # or
        capture {               # Capture slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
}
\endcode

For example, you can combine a dmix plugin and a dsnoop plugin as
as a single PCM for playback and capture directions, respectively.
\code
pcm.duplex {
	type asym
	playback.pcm "dmix"
	capture.pcm "dsnoop"
}
\endcode

By defining only a single direction, the resultant PCM becomes
half-duplex.

\subsection pcm_plugins_asym_funcref Function reference

<UL>
  <LI>_snd_pcm_asym_open()
</UL>

*/

/**
 * \brief Creates a new asym stream PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with copy PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_asym_open(snd_pcm_t **pcmp, const char *name ATTRIBUTE_UNUSED,
			 snd_config_t *root, snd_config_t *conf,
			 snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_config_t *slave = NULL, *sconf;
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "playback") == 0) {
			if (stream == SND_PCM_STREAM_PLAYBACK)
				slave = n;
			continue;
		}
		if (strcmp(id, "capture") == 0) {
			if (stream == SND_PCM_STREAM_CAPTURE)
				slave = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (! slave) {
		SNDERR("%s slave is not defined",
		       stream == SND_PCM_STREAM_PLAYBACK ? "playback" : "capture");
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
SND_DLSYM_BUILD_VERSION(_snd_pcm_asym_open, SND_PCM_DLSYM_VERSION);
#endif
