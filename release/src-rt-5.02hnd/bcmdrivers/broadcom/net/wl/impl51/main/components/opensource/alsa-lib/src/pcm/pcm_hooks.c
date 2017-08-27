/**
 * \file pcm/pcm_hooks.c
 * \ingroup PCM_Hook
 * \brief PCM Hook Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 */
/*
 *  PCM - Hook functions
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
  
#include "pcm_local.h"
#include "pcm_generic.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_hooks = "";
#endif

#ifndef DOC_HIDDEN
struct _snd_pcm_hook {
	snd_pcm_t *pcm;
	snd_pcm_hook_func_t func;
	void *private_data;
	struct list_head list;
};

struct snd_pcm_hook_dllist {
	void *dlobj;
	struct list_head list;
};

typedef struct {
	snd_pcm_generic_t gen;
	struct list_head hooks[SND_PCM_HOOK_TYPE_LAST + 1];
	struct list_head dllist;
} snd_pcm_hooks_t;
#endif

static int hook_add_dlobj(snd_pcm_t *pcm, void *dlobj)
{
	snd_pcm_hooks_t *h = pcm->private_data;
	struct snd_pcm_hook_dllist *dl;

	dl = malloc(sizeof(*dl));
	if (!dl)
		return -ENOMEM;

	dl->dlobj = dlobj;
	list_add_tail(&dl->list, &h->dllist);
	return 0;
}

static void hook_remove_dlobj(struct snd_pcm_hook_dllist *dl)
{
	list_del(&dl->list);
	snd_dlclose(dl->dlobj);
	free(dl);
}

static int snd_pcm_hooks_close(snd_pcm_t *pcm)
{
	snd_pcm_hooks_t *h = pcm->private_data;
	struct list_head *pos, *next;
	unsigned int k;
	int res = 0, err;

	list_for_each_safe(pos, next, &h->hooks[SND_PCM_HOOK_TYPE_CLOSE]) {
		snd_pcm_hook_t *hook = list_entry(pos, snd_pcm_hook_t, list);
		err = hook->func(hook);
		if (err < 0)
			res = err;
	}
	for (k = 0; k <= SND_PCM_HOOK_TYPE_LAST; ++k) {
		struct list_head *hooks = &h->hooks[k];
		while (!list_empty(hooks)) {
			snd_pcm_hook_t *hook;
			pos = hooks->next;
			hook = list_entry(pos, snd_pcm_hook_t, list);
			snd_pcm_hook_remove(hook);
		}
	}
	while (!list_empty(&h->dllist)) {
		pos = h->dllist.next;
		hook_remove_dlobj(list_entry(pos, struct snd_pcm_hook_dllist, list));
	}
	err = snd_pcm_generic_close(pcm);
	if (err < 0)
		res = err;
	return res;
}

static int snd_pcm_hooks_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_hooks_t *h = pcm->private_data;
	struct list_head *pos, *next;
	int err = snd_pcm_generic_hw_params(pcm, params);
	if (err < 0)
		return err;
	list_for_each_safe(pos, next, &h->hooks[SND_PCM_HOOK_TYPE_HW_PARAMS]) {
		snd_pcm_hook_t *hook = list_entry(pos, snd_pcm_hook_t, list);
		err = hook->func(hook);
		if (err < 0)
			return err;
	}
	return 0;
}

static int snd_pcm_hooks_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_hooks_t *h = pcm->private_data;
	struct list_head *pos, *next;
	int err = snd_pcm_generic_hw_free(pcm);
	if (err < 0)
		return err;
	list_for_each_safe(pos, next, &h->hooks[SND_PCM_HOOK_TYPE_HW_FREE]) {
		snd_pcm_hook_t *hook = list_entry(pos, snd_pcm_hook_t, list);
		err = hook->func(hook);
		if (err < 0)
			return err;
	}
	return 0;
}

static void snd_pcm_hooks_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_hooks_t *h = pcm->private_data;
	snd_output_printf(out, "Hooks PCM\n");
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(h->gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_hooks_ops = {
	.close = snd_pcm_hooks_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_generic_hw_refine,
	.hw_params = snd_pcm_hooks_hw_params,
	.hw_free = snd_pcm_hooks_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_hooks_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static const snd_pcm_fast_ops_t snd_pcm_hooks_fast_ops = {
	.status = snd_pcm_generic_status,
	.state = snd_pcm_generic_state,
	.hwsync = snd_pcm_generic_hwsync,
	.delay = snd_pcm_generic_delay,
	.prepare = snd_pcm_generic_prepare,
	.reset = snd_pcm_generic_reset,
	.start = snd_pcm_generic_start,
	.drop = snd_pcm_generic_drop,
	.drain = snd_pcm_generic_drain,
	.pause = snd_pcm_generic_pause,
	.rewindable = snd_pcm_generic_rewindable,
	.rewind = snd_pcm_generic_rewind,
	.forwardable = snd_pcm_generic_forwardable,
	.forward = snd_pcm_generic_forward,
	.resume = snd_pcm_generic_resume,
	.link = snd_pcm_generic_link,
	.link_slaves = snd_pcm_generic_link_slaves,
	.unlink = snd_pcm_generic_unlink,
	.writei = snd_pcm_generic_writei,
	.writen = snd_pcm_generic_writen,
	.readi = snd_pcm_generic_readi,
	.readn = snd_pcm_generic_readn,
	.avail_update = snd_pcm_generic_avail_update,
	.mmap_commit = snd_pcm_generic_mmap_commit,
	.htimestamp = snd_pcm_generic_htimestamp,
	.poll_descriptors_count = snd_pcm_generic_poll_descriptors_count,
	.poll_descriptors = snd_pcm_generic_poll_descriptors,
	.poll_revents = snd_pcm_generic_poll_revents,
};

/**
 * \brief Creates a new hooks PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param slave Slave PCM
 * \param close_slave If set, slave PCM handle is closed when hooks PCM is closed
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *	    changed in future.
 */
int snd_pcm_hooks_open(snd_pcm_t **pcmp, const char *name, snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_hooks_t *h;
	unsigned int k;
	int err;
	assert(pcmp && slave);
	h = calloc(1, sizeof(snd_pcm_hooks_t));
	if (!h)
		return -ENOMEM;
	h->gen.slave = slave;
	h->gen.close_slave = close_slave;
	for (k = 0; k <= SND_PCM_HOOK_TYPE_LAST; ++k) {
		INIT_LIST_HEAD(&h->hooks[k]);
	}
	INIT_LIST_HEAD(&h->dllist);
	err = snd_pcm_new(&pcm, SND_PCM_TYPE_HOOKS, name, slave->stream, slave->mode);
	if (err < 0) {
		free(h);
		return err;
	}
	pcm->ops = &snd_pcm_hooks_ops;
	pcm->fast_ops = &snd_pcm_hooks_fast_ops;
	pcm->private_data = h;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->mmap_shadow = 1;
	pcm->monotonic = slave->monotonic;
	snd_pcm_link_hw_ptr(pcm, slave);
	snd_pcm_link_appl_ptr(pcm, slave);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_hooks Plugin: hooks

This plugin is used to call some 'hook' function when this plugin is opened,
modified or closed.
Typically, it is used to change control values for a certain state
specially for the PCM (see the example below).

\code
# Hook arguments definition
hook_args.NAME {
	...			# Arbitrary arguments
}

# PCM hook type
pcm_hook_type.NAME {
	[lib STR]		# Library file (default libasound.so)
	[install STR]		# Install function (default _snd_pcm_hook_NAME_install)
}

# PCM hook definition
pcm_hook.NAME {
	type STR		# PCM Hook type (see pcm_hook_type)
	[args STR]		# Arguments for install function (see hook_args)
	# or
	[args { }]		# Arguments for install function
}

# PCM hook plugin
pcm.NAME {
	type hooks		# PCM with hooks
	slave STR		# Slave name
	# or
	slave {			# Slave definition
	  	pcm STR		# Slave PCM name
		# or
	  	pcm { }		# Slave PCM definition
	}
	hooks {
		ID STR		# Hook name (see pcm_hook)
		# or
		ID { }		# Hook definition (see pcm_hook)
	}
}
\endcode

Example:

\code
	hooks.0 {
		type ctl_elems
		hook_args [
			{
				name "Wave Surround Playback Volume"
				preserve true
				lock true
				optional true
				value [ 0 0 ]
			}
			{
				name "EMU10K1 PCM Send Volume"
				index { @func private_pcm_subdevice }
				lock true
				value [ 0 0 0 0 0 0 255 0 0 0 0 255 ]
			}
		]
	}
\endcode
Here, the controls "Wave Surround Playback Volume" and "EMU10K1 PCM Send Volume"
are set to the given values when this pcm is accessed.  Since these controls
take multi-dimensional values, the <code>value</code> field is written as
an array.
When <code>preserve</code> is true, the old values are saved and restored
when the pcm is closed.  The <code>lock</code> means that the control is
locked during this pcm is opened, and cannot be changed by others.
When <code>optional</code> is set, no error is returned but ignored
even if the specified control doesn't exist.

\subsection pcm_plugins_hooks_funcref Function reference

<UL>
  <LI>The function ctl_elems - _snd_pcm_hook_ctl_elems_install() - installs
      CTL settings described by given configuration.
  <LI>snd_pcm_hooks_open()
  <LI>_snd_pcm_hooks_open()
</UL>

*/

static int snd_pcm_hook_add_conf(snd_pcm_t *pcm, snd_config_t *root, snd_config_t *conf)
{
	int err;
	char buf[256];
	const char *str, *id;
	const char *lib = NULL, *install = NULL;
	snd_config_t *type = NULL, *args = NULL;
	snd_config_iterator_t i, next;
	int (*install_func)(snd_pcm_t *pcm, snd_config_t *args) = NULL;
	void *h = NULL;

	if (snd_config_get_type(conf) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("Invalid hook definition");
		return -EINVAL;
	}
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0)
			continue;
		if (strcmp(id, "type") == 0) {
			type = n;
			continue;
		}
		if (strcmp(id, "hook_args") == 0) {
			args = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!type) {
		SNDERR("type is not defined");
		return -EINVAL;
	}
	err = snd_config_get_id(type, &id);
	if (err < 0) {
		SNDERR("unable to get id");
		return err;
	}
	err = snd_config_get_string(type, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return err;
	}
	err = snd_config_search_definition(root, "pcm_hook_type", str, &type);
	if (err >= 0) {
		if (snd_config_get_type(type) != SND_CONFIG_TYPE_COMPOUND) {
			SNDERR("Invalid type for PCM type %s definition", str);
			goto _err;
		}
		snd_config_for_each(i, next, type) {
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
			if (strcmp(id, "install") == 0) {
				err = snd_config_get_string(n, &install);
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
	if (!install) {
		install = buf;
		snprintf(buf, sizeof(buf), "_snd_pcm_hook_%s_install", str);
	}
	h = snd_dlopen(lib, RTLD_NOW);
	install_func = h ? snd_dlsym(h, install, SND_DLSYM_VERSION(SND_PCM_DLSYM_VERSION)) : NULL;
	err = 0;
	if (!h) {
		SNDERR("Cannot open shared library %s",
		       lib ? lib : "[builtin]");
		err = -ENOENT;
	} else if (!install_func) {
		SNDERR("symbol %s is not defined inside %s", install,
		       lib ? lib : "[builtin]");
		snd_dlclose(h);
		err = -ENXIO;
	}
       _err:
	if (type)
		snd_config_delete(type);
	if (err < 0)
		return err;

	if (args && snd_config_get_string(args, &str) >= 0) {
		err = snd_config_search_definition(root, "hook_args", str, &args);
		if (err < 0)
			SNDERR("unknown hook_args %s", str);
		else
			err = install_func(pcm, args);
		snd_config_delete(args);
	} else
		err = install_func(pcm, args);

	if (err >= 0)
		err = hook_add_dlobj(pcm, h);

	if (err < 0) {
		snd_dlclose(h);
		return err;
	}
	return 0;
}

/**
 * \brief Creates a new hooks PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with hooks PCM description
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *	    changed in future.
 */
int _snd_pcm_hooks_open(snd_pcm_t **pcmp, const char *name,
			snd_config_t *root, snd_config_t *conf, 
			snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *rpcm = NULL, *spcm;
	snd_config_t *slave = NULL, *sconf;
	snd_config_t *hooks = NULL;
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
		if (strcmp(id, "hooks") == 0) {
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_COMPOUND) {
				SNDERR("Invalid type for %s", id);
				return -EINVAL;
			}
			hooks = n;
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
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_hooks_open(&rpcm, name, spcm, 1);
	if (err < 0) {
		snd_pcm_close(spcm);
		return err;
	}
	if (!hooks)
		goto _done;
	snd_config_for_each(i, next, hooks) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *str;
		if (snd_config_get_string(n, &str) >= 0) {
			err = snd_config_search_definition(root, "pcm_hook", str, &n);
			if (err < 0) {
				SNDERR("unknown pcm_hook %s", str);
			} else {
				err = snd_pcm_hook_add_conf(rpcm, root, n);
				snd_config_delete(n);
			}
		} else
			err = snd_pcm_hook_add_conf(rpcm, root, n);
		if (err < 0) {
			snd_pcm_close(rpcm);
			return err;
		}
	}
 _done:
	*pcmp = rpcm;
	return 0;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_hooks_open, SND_PCM_DLSYM_VERSION);
#endif

/**
 * \brief Get PCM handle for a PCM hook
 * \param hook PCM hook handle
 * \return PCM handle
 */
snd_pcm_t *snd_pcm_hook_get_pcm(snd_pcm_hook_t *hook)
{
	assert(hook);
	return hook->pcm;
}

/**
 * \brief Get callback function private data for a PCM hook
 * \param hook PCM hook handle
 * \return callback function private data
 */
void *snd_pcm_hook_get_private(snd_pcm_hook_t *hook)
{
	assert(hook);
	return hook->private_data;
}

/**
 * \brief Set callback function private data for a PCM hook
 * \param hook PCM hook handle
 * \param private_data The private data value
 */
void snd_pcm_hook_set_private(snd_pcm_hook_t *hook, void *private_data)
{
	assert(hook);
	hook->private_data = private_data;
}

/**
 * \brief Add a PCM hook at end of hooks chain
 * \param hookp Returned PCM hook handle
 * \param pcm PCM handle
 * \param type PCM hook type
 * \param func PCM hook callback function
 * \param private_data PCM hook private data
 * \return 0 on success otherwise a negative error code
 *
 * Warning: an hook callback function cannot remove an hook of the same type
 * different from itself
 */
int snd_pcm_hook_add(snd_pcm_hook_t **hookp, snd_pcm_t *pcm,
		     snd_pcm_hook_type_t type,
		     snd_pcm_hook_func_t func, void *private_data)
{
	snd_pcm_hook_t *h;
	snd_pcm_hooks_t *hooks;
	assert(hookp && func);
	assert(snd_pcm_type(pcm) == SND_PCM_TYPE_HOOKS);
	h = calloc(1, sizeof(*h));
	if (!h)
		return -ENOMEM;
	h->pcm = pcm;
	h->func = func;
	h->private_data = private_data;
	hooks = pcm->private_data;
	list_add_tail(&h->list, &hooks->hooks[type]);
	*hookp = h;
	return 0;
}

/**
 * \brief Remove a PCM hook
 * \param hook PCM hook handle
 * \return 0 on success otherwise a negative error code
 *
 * Warning: an hook callback cannot remove an hook of the same type
 * different from itself
 */
int snd_pcm_hook_remove(snd_pcm_hook_t *hook)
{
	assert(hook);
	list_del(&hook->list);
	free(hook);
	return 0;
}

/*
 *
 */

static int snd_pcm_hook_ctl_elems_hw_params(snd_pcm_hook_t *hook)
{
	snd_sctl_t *h = snd_pcm_hook_get_private(hook);
	return snd_sctl_install(h);
}

static int snd_pcm_hook_ctl_elems_hw_free(snd_pcm_hook_t *hook)
{
	snd_sctl_t *h = snd_pcm_hook_get_private(hook);
	return snd_sctl_remove(h);
}

static int snd_pcm_hook_ctl_elems_close(snd_pcm_hook_t *hook)
{
	snd_sctl_t *h = snd_pcm_hook_get_private(hook);
	int err = snd_sctl_free(h);
	snd_pcm_hook_set_private(hook, NULL);
	return err;
}

/**
 * \brief Install CTL settings using hardware associated with PCM handle
 * \param pcm PCM handle
 * \param conf Configuration node with CTL settings
 * \return zero on success otherwise a negative error code
 */
int _snd_pcm_hook_ctl_elems_install(snd_pcm_t *pcm, snd_config_t *conf)
{
	int err;
	int card;
	snd_pcm_info_t *info;
	char ctl_name[16];
	snd_ctl_t *ctl;
	snd_sctl_t *sctl = NULL;
	snd_config_t *pcm_conf = NULL;
	snd_pcm_hook_t *h_hw_params = NULL, *h_hw_free = NULL, *h_close = NULL;
	assert(conf);
	assert(snd_config_get_type(conf) == SND_CONFIG_TYPE_COMPOUND);
	snd_pcm_info_alloca(&info);
	err = snd_pcm_info(pcm, info);
	if (err < 0)
		return err;
	card = snd_pcm_info_get_card(info);
	if (card < 0) {
		SNDERR("No card for this PCM");
		return -EINVAL;
	}
	sprintf(ctl_name, "hw:%d", card);
	err = snd_ctl_open(&ctl, ctl_name, 0);
	if (err < 0) {
		SNDERR("Cannot open CTL %s", ctl_name);
		return err;
	}
	err = snd_config_imake_pointer(&pcm_conf, "pcm_handle", pcm);
	if (err < 0)
		goto _err;
	err = snd_sctl_build(&sctl, ctl, conf, pcm_conf, 0);
	if (err < 0)
		goto _err;
	err = snd_pcm_hook_add(&h_hw_params, pcm, SND_PCM_HOOK_TYPE_HW_PARAMS,
			       snd_pcm_hook_ctl_elems_hw_params, sctl);
	if (err < 0)
		goto _err;
	err = snd_pcm_hook_add(&h_hw_free, pcm, SND_PCM_HOOK_TYPE_HW_FREE,
			       snd_pcm_hook_ctl_elems_hw_free, sctl);
	if (err < 0)
		goto _err;
	err = snd_pcm_hook_add(&h_close, pcm, SND_PCM_HOOK_TYPE_CLOSE,
			       snd_pcm_hook_ctl_elems_close, sctl);
	if (err < 0)
		goto _err;
	snd_config_delete(pcm_conf);
	return 0;
 _err:
	if (h_hw_params)
		snd_pcm_hook_remove(h_hw_params);
	if (h_hw_free)
		snd_pcm_hook_remove(h_hw_free);
	if (h_close)
		snd_pcm_hook_remove(h_close);
	if (sctl)
		snd_sctl_free(sctl);
	if (pcm_conf)
		snd_config_delete(pcm_conf);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_hook_ctl_elems_install, SND_PCM_DLSYM_VERSION);
#endif
