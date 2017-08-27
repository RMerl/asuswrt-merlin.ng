/**
 * \file pcm/pcm_ladspa.c
 * \ingroup PCM_Plugins
 * \brief ALSA Plugin <-> LADSPA Plugin Interface
 * \author Jaroslav Kysela <perex@perex.cz>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2001,2006
 */
/*
 *  PCM - LADSPA integration plugin
 *  Copyright (c) 2001-2006 by Jaroslav Kysela <perex@perex.cz>
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
 *
 * perex@perex.cz 2005/12/13
 *   The LADSPA plugin rewrite was sponsored by MediaNet AG
 *   http://www.medianet.ag
 */
  
#include <dirent.h>
#include <locale.h>
#include <math.h>
#include "pcm_local.h"
#include "pcm_plugin.h"

#include "ladspa.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_ladspa = "";
#endif

#ifndef DOC_HIDDEN

#define NO_ASSIGN	0xffffffff

typedef enum _snd_pcm_ladspa_policy {
	SND_PCM_LADSPA_POLICY_NONE,		/* use bindings only */
	SND_PCM_LADSPA_POLICY_DUPLICATE		/* duplicate bindings for all channels */
} snd_pcm_ladspa_policy_t;

typedef struct {
	/* This field need to be the first */
	snd_pcm_plugin_t plug;
	/* Plugin custom fields */
	struct list_head pplugins;
	struct list_head cplugins;
	unsigned int channels;			/* forced input channels, 0 = auto */
	unsigned int allocated;			/* count of allocated samples */
	LADSPA_Data *zero[2];			/* zero input or dummy output */
} snd_pcm_ladspa_t;
 
typedef struct {
        unsigned int size;
        unsigned int *array;
} snd_pcm_ladspa_array_t;

typedef struct {
        snd_pcm_ladspa_array_t channels;
        snd_pcm_ladspa_array_t ports;
	LADSPA_Data **m_data;
        LADSPA_Data **data;
} snd_pcm_ladspa_eps_t;

typedef struct snd_pcm_ladspa_instance {
	struct list_head list;
	const LADSPA_Descriptor *desc;
	LADSPA_Handle *handle;
	unsigned int depth;
	snd_pcm_ladspa_eps_t input;
	snd_pcm_ladspa_eps_t output;
	struct snd_pcm_ladspa_instance *prev;
	struct snd_pcm_ladspa_instance *next;
} snd_pcm_ladspa_instance_t;

typedef struct {
	LADSPA_PortDescriptor pdesc;		/* port description */
	unsigned int port_bindings_size;	/* size of array */
	unsigned int *port_bindings;		/* index = channel number, value = LADSPA port */
	unsigned int controls_size;		/* size of array */
	unsigned char *controls_initialized;	/* initialized by ALSA user */
	LADSPA_Data *controls;			/* index = LADSPA control port index */
} snd_pcm_ladspa_plugin_io_t;

typedef struct {
	struct list_head list;
	snd_pcm_ladspa_policy_t policy;
	char *filename;
	void *dl_handle;
	const LADSPA_Descriptor *desc;
	snd_pcm_ladspa_plugin_io_t input;
	snd_pcm_ladspa_plugin_io_t output;
	struct list_head instances;		/* one LADSPA plugin might be used multiple times */
} snd_pcm_ladspa_plugin_t;

#endif /* DOC_HIDDEN */

static unsigned int snd_pcm_ladspa_count_ports(snd_pcm_ladspa_plugin_t *lplug,
                                               LADSPA_PortDescriptor pdesc)
{
        unsigned int res = 0, idx;
        for (idx = 0; idx < lplug->desc->PortCount; idx++) {
                if ((lplug->desc->PortDescriptors[idx] & pdesc) == pdesc)
                        res++;
        }
        return res;
}

static int snd_pcm_ladspa_find_port(unsigned int *res,
				    snd_pcm_ladspa_plugin_t *lplug,
				    LADSPA_PortDescriptor pdesc,
				    unsigned int port_idx)
{
	unsigned long idx;

	for (idx = 0; idx < lplug->desc->PortCount; idx++)
		if ((lplug->desc->PortDescriptors[idx] & pdesc) == pdesc) {
			if (port_idx == 0) {
				*res = idx;
				return 0;
			}
			port_idx--;
		}
	return -EINVAL;
}

static int snd_pcm_ladspa_find_sport(unsigned int *res,
				     snd_pcm_ladspa_plugin_t *lplug,
				     LADSPA_PortDescriptor pdesc,
				     const char *port_name)
{
	unsigned long idx;

	for (idx = 0; idx < lplug->desc->PortCount; idx++)
		if ((lplug->desc->PortDescriptors[idx] & pdesc) == pdesc &&
		    !strcmp(lplug->desc->PortNames[idx], port_name)) {
			*res = idx;
			return 0;
		}
	return -EINVAL;
}

static int snd_pcm_ladspa_find_port_idx(unsigned int *res,
					snd_pcm_ladspa_plugin_t *lplug,
					LADSPA_PortDescriptor pdesc,
					unsigned int port)
{
	unsigned long idx;
	unsigned int r = 0;

	if (port >= lplug->desc->PortCount)
		return -EINVAL;
	for (idx = 0; idx < port; idx++)
		if ((lplug->desc->PortDescriptors[idx] & pdesc) == pdesc)
			r++;
	*res = r;
	return 0;
}

static void snd_pcm_ladspa_free_io(snd_pcm_ladspa_plugin_io_t *io)
{
	free(io->controls);
	free(io->controls_initialized);
}

static void snd_pcm_ladspa_free_plugins(struct list_head *plugins)
{
	while (!list_empty(plugins)) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(plugins->next, snd_pcm_ladspa_plugin_t, list);
                snd_pcm_ladspa_free_io(&plugin->input);
                snd_pcm_ladspa_free_io(&plugin->output);
		if (plugin->dl_handle)
			dlclose(plugin->dl_handle);
		free(plugin->filename);
		list_del(&plugin->list);
		free(plugin);
	}
}

static void snd_pcm_ladspa_free(snd_pcm_ladspa_t *ladspa)
{
        unsigned int idx;

	snd_pcm_ladspa_free_plugins(&ladspa->pplugins);
	snd_pcm_ladspa_free_plugins(&ladspa->cplugins);
	for (idx = 0; idx < 2; idx++) {
		free(ladspa->zero[idx]);
                ladspa->zero[idx] = NULL;
        }
        ladspa->allocated = 0;
}

static int snd_pcm_ladspa_close(snd_pcm_t *pcm)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;

	snd_pcm_ladspa_free(ladspa);
	return snd_pcm_generic_close(pcm);
}

static int snd_pcm_ladspa_hw_refine_cprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHMN };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_set_format(params, SND_PCM_FORMAT_FLOAT);
	if (err < 0)
		return err;
	err = _snd_pcm_hw_params_set_subformat(params, SND_PCM_SUBFORMAT_STD);
	if (err < 0)
		return err;
        if (ladspa->channels > 0 && pcm->stream == SND_PCM_STREAM_PLAYBACK) {
        	err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS, ladspa->channels, 0);
        	if (err < 0)
        		return err;
        }
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_ladspa_hw_refine_sprepare(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *sparams)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAPN };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	_snd_pcm_hw_params_set_format(sparams, SND_PCM_FORMAT_FLOAT);
	_snd_pcm_hw_params_set_subformat(sparams, SND_PCM_SUBFORMAT_STD);
        if (ladspa->channels > 0 && pcm->stream == SND_PCM_STREAM_CAPTURE)
                _snd_pcm_hw_param_set(sparams, SND_PCM_HW_PARAM_CHANNELS, ladspa->channels, 0);
	return 0;
}

static int snd_pcm_ladspa_hw_refine_schange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					    snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	err = _snd_pcm_hw_params_refine(sparams, links, params);
	if (err < 0)
		return err;
	return 0;
}
	
static int snd_pcm_ladspa_hw_refine_cchange(snd_pcm_t *pcm ATTRIBUTE_UNUSED, snd_pcm_hw_params_t *params,
					    snd_pcm_hw_params_t *sparams)
{
	int err;
	unsigned int links = (SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);
	err = _snd_pcm_hw_params_refine(params, links, sparams);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_ladspa_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	return snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_ladspa_hw_refine_cprepare,
				       snd_pcm_ladspa_hw_refine_cchange,
				       snd_pcm_ladspa_hw_refine_sprepare,
				       snd_pcm_ladspa_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
}

static int snd_pcm_ladspa_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	// snd_pcm_ladspa_t *ladspa = pcm->private_data;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_ladspa_hw_refine_cchange,
					  snd_pcm_ladspa_hw_refine_sprepare,
					  snd_pcm_ladspa_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;
	return 0;
}

static void snd_pcm_ladspa_free_eps(snd_pcm_ladspa_eps_t *eps)
{
	free(eps->channels.array);
	free(eps->ports.array);
}

static void snd_pcm_ladspa_free_instances(snd_pcm_t *pcm, snd_pcm_ladspa_t *ladspa, int cleanup)
{
	struct list_head *list, *pos, *pos1, *next1;
	unsigned int idx;
	
	list = pcm->stream == SND_PCM_STREAM_PLAYBACK ? &ladspa->pplugins : &ladspa->cplugins;
	list_for_each(pos, list) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
		list_for_each_safe(pos1, next1, &plugin->instances) {
			snd_pcm_ladspa_instance_t *instance = list_entry(pos1, snd_pcm_ladspa_instance_t, list);
			if (plugin->desc->deactivate)
				plugin->desc->deactivate(instance->handle);
			if (cleanup) {
				if (plugin->desc->cleanup)
					plugin->desc->cleanup(instance->handle);
				if (instance->input.m_data) {
				        for (idx = 0; idx < instance->input.channels.size; idx++)
						free(instance->input.m_data[idx]);
					free(instance->input.m_data);
                                }
				if (instance->output.m_data) {
				        for (idx = 0; idx < instance->output.channels.size; idx++)
						free(instance->output.m_data[idx]);
					free(instance->output.m_data);
                                }
				list_del(&(instance->list));
				snd_pcm_ladspa_free_eps(&instance->input);
				snd_pcm_ladspa_free_eps(&instance->output);
				free(instance);
			} else {
				if (plugin->desc->activate)
					plugin->desc->activate(instance->handle);
			}
		}
		if (cleanup) {
			assert(list_empty(&plugin->instances));
		}
	}
}

static int snd_pcm_ladspa_add_to_carray(snd_pcm_ladspa_array_t *array,
                                        unsigned int idx,
                                        unsigned int val)
{
        unsigned int *narray;
        unsigned int idx1;

        if (idx >= array->size) {
                narray = realloc(array->array, sizeof(unsigned int) * (idx + 1));
                if (narray == NULL)
                        return -ENOMEM;
                for (idx1 = array->size; idx1 < idx; idx1++)
                        narray[idx1] = NO_ASSIGN;
                array->array = narray;
                array->size = idx + 1;
                array->array[idx] = val;
                return 0;
        }
        if (array->array[idx] == NO_ASSIGN)
                array->array[idx] = val;
        else
                return -EINVAL;
        return 0;
}

static int snd_pcm_ladspa_add_to_array(snd_pcm_ladspa_array_t *array,
                                       unsigned int idx,
                                       unsigned int val)
{
        unsigned int *narray;
        unsigned int idx1;

        if (idx >= array->size) {
                narray = realloc(array->array, sizeof(unsigned int) * (idx + 1));
                if (narray == NULL)
                        return -ENOMEM;
                for (idx1 = array->size; idx1 < idx; idx1++)
                        narray[idx1] = NO_ASSIGN;
                array->array = narray;
                array->size = idx + 1;
        }
        array->array[idx] = val;
        return 0;
}

static int snd_pcm_ladspa_connect_plugin1(snd_pcm_ladspa_plugin_t *plugin,
					  snd_pcm_ladspa_plugin_io_t *io,
					  snd_pcm_ladspa_eps_t *eps)
{
	unsigned int port, channels, idx, idx1;
	int err;

	assert(plugin->policy == SND_PCM_LADSPA_POLICY_NONE);
	channels = io->port_bindings_size > 0 ?
	                io->port_bindings_size :
	                snd_pcm_ladspa_count_ports(plugin, io->pdesc | LADSPA_PORT_AUDIO);
	for (idx = idx1 = 0; idx < channels; idx++) {
		if (io->port_bindings_size > 0)
        		port = io->port_bindings[idx];
                else {
        		err = snd_pcm_ladspa_find_port(&port, plugin, io->pdesc | LADSPA_PORT_AUDIO, idx);
        		if (err < 0) {
        		        SNDERR("unable to find audio %s port %u plugin '%s'", io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", idx, plugin->desc->Name);
        			return err;
                        }
                }
                if (port == NO_ASSIGN)
                	continue;
        	err = snd_pcm_ladspa_add_to_carray(&eps->channels, idx1, idx);
        	if (err < 0) {
        		SNDERR("unable to add channel %u for audio %s plugin '%s'", idx, io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", plugin->desc->Name);
        	        return err;
                }
        	err = snd_pcm_ladspa_add_to_array(&eps->ports, idx1, port);
        	if (err < 0) {
        		SNDERR("unable to add port %u for audio %s plugin '%s'", port, io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", plugin->desc->Name);
        	        return err;
                }
                idx1++;
	}
	return 0;
}

static int snd_pcm_ladspa_connect_plugin(snd_pcm_ladspa_plugin_t *plugin,
					 snd_pcm_ladspa_instance_t *instance)
{
        int err;
        
        err = snd_pcm_ladspa_connect_plugin1(plugin, &plugin->input, &instance->input);
        if (err < 0)
                return err;
        err = snd_pcm_ladspa_connect_plugin1(plugin, &plugin->output, &instance->output);
        if (err < 0)
                return err;
        return 0;
}

static int snd_pcm_ladspa_connect_plugin_duplicate1(snd_pcm_ladspa_plugin_t *plugin,
                                                    snd_pcm_ladspa_plugin_io_t *io,
                                                    snd_pcm_ladspa_eps_t *eps,
                                                    unsigned int idx)
{
	unsigned int port;
	int err;

	assert(plugin->policy == SND_PCM_LADSPA_POLICY_DUPLICATE);
	if (io->port_bindings_size > 0) {
		port = io->port_bindings[0];
	} else {
		err = snd_pcm_ladspa_find_port(&port, plugin, io->pdesc | LADSPA_PORT_AUDIO, 0);
		if (err < 0) {
		        SNDERR("unable to find audio %s port %u plugin '%s'", io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", (unsigned int)0, plugin->desc->Name);
			return err;
                }
	}
	err = snd_pcm_ladspa_add_to_carray(&eps->channels, 0, idx);
	if (err < 0) {
        	SNDERR("unable to add channel %u for audio %s plugin '%s'", idx, io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", plugin->desc->Name);
	        return err;
        }
        err = snd_pcm_ladspa_add_to_array(&eps->ports, 0, port);
        if (err < 0) {
        	SNDERR("unable to add port %u for audio %s plugin '%s'", port, io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", plugin->desc->Name);
        	return err;
        }
        return 0;
}

static int snd_pcm_ladspa_connect_plugin_duplicate(snd_pcm_ladspa_plugin_t *plugin,
						   snd_pcm_ladspa_plugin_io_t *in_io,
						   snd_pcm_ladspa_plugin_io_t *out_io,
						   snd_pcm_ladspa_instance_t *instance,
						   unsigned int idx)
{
	int err;

	err = snd_pcm_ladspa_connect_plugin_duplicate1(plugin, in_io, &instance->input, idx);
	if (err < 0)
	        return err;
	err = snd_pcm_ladspa_connect_plugin_duplicate1(plugin, out_io, &instance->output, idx);
	if (err < 0)
	        return err;
        return 0;
}

static void snd_pcm_ladspa_get_default_cvalue(const LADSPA_Descriptor * desc, unsigned int port, LADSPA_Data *val) 
{
        LADSPA_PortRangeHintDescriptor hdesc;

        hdesc = desc->PortRangeHints[port].HintDescriptor;
        switch (hdesc & LADSPA_HINT_DEFAULT_MASK) {
        case LADSPA_HINT_DEFAULT_MINIMUM:
                *val = desc->PortRangeHints[port].LowerBound;
                break;
        case LADSPA_HINT_DEFAULT_LOW:
                if (LADSPA_IS_HINT_LOGARITHMIC(hdesc)) {
                        *val = exp(log(desc->PortRangeHints[port].LowerBound)
                                        * 0.75
                                        + log(desc->PortRangeHints[port].UpperBound)
                                        * 0.25);
                } else {
                        *val = (desc->PortRangeHints[port].LowerBound * 0.75) +
                               (desc->PortRangeHints[port].UpperBound * 0.25);
                }
                break;
        case LADSPA_HINT_DEFAULT_MIDDLE:
                if (LADSPA_IS_HINT_LOGARITHMIC(hdesc)) {
                        *val = sqrt(desc->PortRangeHints[port].LowerBound *
                                    desc->PortRangeHints[port].UpperBound);
                } else {
                        *val = 0.5 *
                               (desc->PortRangeHints[port].LowerBound +
                                desc->PortRangeHints[port].UpperBound);
                }
                break;
        case LADSPA_HINT_DEFAULT_HIGH:
                if (LADSPA_IS_HINT_LOGARITHMIC(hdesc)) {
                        *val = exp(log(desc->PortRangeHints[port].LowerBound)
                                        * 0.25
                                        + log(desc->PortRangeHints[port].UpperBound)
                                        * 0.75);
                } else {
                        *val = (desc->PortRangeHints[port].LowerBound * 0.25) +
                               (desc->PortRangeHints[port].UpperBound * 0.75);
                }
                break;
        case LADSPA_HINT_DEFAULT_MAXIMUM:
                *val = desc->PortRangeHints[port].UpperBound;
                break;
        case LADSPA_HINT_DEFAULT_0:
                *val = 0;
                break;
        case LADSPA_HINT_DEFAULT_1:
                *val = 1;
                break;
        case LADSPA_HINT_DEFAULT_100:
                *val = 100;
                break;
        case LADSPA_HINT_DEFAULT_440:
                *val = 440;
                break;
        default:
                *val = 0;	/* reasonable default, if everything fails */
                break;
        }
}

static int snd_pcm_ladspa_connect_controls(snd_pcm_ladspa_plugin_t *plugin,
					   snd_pcm_ladspa_plugin_io_t *io,
					   snd_pcm_ladspa_instance_t *instance)
{
	unsigned long idx, midx;

	for (idx = midx = 0; idx < plugin->desc->PortCount; idx++)
		if ((plugin->desc->PortDescriptors[idx] & (io->pdesc | LADSPA_PORT_CONTROL)) == (io->pdesc | LADSPA_PORT_CONTROL)) {
			if (io->controls_size > midx) {
			        if (!io->controls_initialized[midx])
			                snd_pcm_ladspa_get_default_cvalue(plugin->desc, idx, &io->controls[midx]);
				plugin->desc->connect_port(instance->handle, idx, &io->controls[midx]);
			} else {
				return -EINVAL;
			}
			midx++;
		}
	return 0;
}

static int snd_pcm_ladspa_check_connect(snd_pcm_ladspa_plugin_t *plugin,
                                        snd_pcm_ladspa_plugin_io_t *io,
                                        snd_pcm_ladspa_eps_t *eps,
                                        unsigned int depth)
{
        unsigned int idx, midx;
        int err = 0;

	for (idx = midx = 0; idx < plugin->desc->PortCount; idx++)
		if ((plugin->desc->PortDescriptors[idx] & (io->pdesc | LADSPA_PORT_AUDIO)) == (io->pdesc | LADSPA_PORT_AUDIO)) {
                        if (eps->channels.array[midx] == NO_ASSIGN) {
                                SNDERR("%s port for plugin %s depth %u is not connected", io->pdesc & LADSPA_PORT_INPUT ? "input" : "output", plugin->desc->Name, depth);
                                err++;
                        }
			midx++;
		}
        if (err > 0) {
                SNDERR("%i connection errors total", err);
                return -EINVAL;
        }
        return 0;
}

static int snd_pcm_ladspa_allocate_instances(snd_pcm_t *pcm, snd_pcm_ladspa_t *ladspa)
{
	struct list_head *list, *pos;
	unsigned int depth, idx, count;
        unsigned int in_channel, out_channel;
        unsigned int in_channels, out_channels;
	unsigned int in_ports, out_ports;
	snd_pcm_ladspa_instance_t *instance = NULL;
	int err;
	
	list = pcm->stream == SND_PCM_STREAM_PLAYBACK ? &ladspa->pplugins : &ladspa->cplugins;
	in_channels = ladspa->channels > 0 ? ladspa->channels :
	              (pcm->stream == SND_PCM_STREAM_PLAYBACK ? pcm->channels : ladspa->plug.gen.slave->channels);
	depth = 0;
	out_channels = 0;
	list_for_each(pos, list) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
		if (pos->next == list)	/* last entry */
		        out_channels = pcm->stream == SND_PCM_STREAM_PLAYBACK ? ladspa->plug.gen.slave->channels : pcm->channels;
                in_ports = snd_pcm_ladspa_count_ports(plugin, LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO);
                out_ports = snd_pcm_ladspa_count_ports(plugin, LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO);
		count = 1;
		if (plugin->policy == SND_PCM_LADSPA_POLICY_DUPLICATE) {
                        if (in_ports == 1 && out_ports == 1)
                                count = in_channels;
                        else
                                plugin->policy = SND_PCM_LADSPA_POLICY_NONE;
                }
                in_channel = 0;
                out_channel = 0;
        	for (idx = 0; idx < count; idx++) {
			instance = (snd_pcm_ladspa_instance_t *)calloc(1, sizeof(snd_pcm_ladspa_instance_t));
			if (instance == NULL)
				return -ENOMEM;
			instance->desc = plugin->desc;
			instance->handle = plugin->desc->instantiate(plugin->desc, pcm->rate);
			instance->depth = depth;
			if (instance->handle == NULL) {
				SNDERR("Unable to create instance of LADSPA plugin '%s'", plugin->desc->Name);
				free(instance);
				return -EINVAL;
			}
			list_add_tail(&instance->list, &plugin->instances);
			if (plugin->desc->activate)
				plugin->desc->activate(instance->handle);
			if (plugin->policy == SND_PCM_LADSPA_POLICY_DUPLICATE) {
				err = snd_pcm_ladspa_connect_plugin_duplicate(plugin, &plugin->input, &plugin->output, instance, idx);
				if (err < 0) {
					SNDERR("Unable to connect duplicate port of plugin '%s' channel %u depth %u", plugin->desc->Name, idx, instance->depth);
					return err;
				}
			} else {
                		err = snd_pcm_ladspa_connect_plugin(plugin, instance);
                		if (err < 0) {
	                		SNDERR("Unable to connect plugin '%s' depth %u", plugin->desc->Name, depth);
		                	return err;
                		}
			}
			err = snd_pcm_ladspa_connect_controls(plugin, &plugin->input, instance);
			assert(err >= 0);
			err = snd_pcm_ladspa_connect_controls(plugin, &plugin->output, instance);
			assert(err >= 0);
		}
		err = snd_pcm_ladspa_check_connect(plugin, &plugin->input, &instance->input, depth);
		if (err < 0)
		        return err;
		err = snd_pcm_ladspa_check_connect(plugin, &plugin->output, &instance->output, depth);
		if (err < 0)
		        return err;
		depth++;
	}
	return 0;
}

static LADSPA_Data *snd_pcm_ladspa_allocate_zero(snd_pcm_ladspa_t *ladspa, unsigned int idx)
{
        if (ladspa->zero[idx] == NULL)
                ladspa->zero[idx] = calloc(ladspa->allocated, sizeof(LADSPA_Data));
        return ladspa->zero[idx];
}

static int snd_pcm_ladspa_allocate_memory(snd_pcm_t *pcm, snd_pcm_ladspa_t *ladspa)
{
	struct list_head *list, *pos, *pos1;
	snd_pcm_ladspa_instance_t *instance;
	unsigned int channels = 16, nchannels;
	unsigned int ichannels, ochannels;
	void **pchannels, **npchannels;
	unsigned int idx, chn;
	
        ladspa->allocated = 2048;
        if (pcm->buffer_size > ladspa->allocated)
                ladspa->allocated = pcm->buffer_size;
        if (pcm->stream == SND_PCM_STREAM_PLAYBACK) {
                ichannels = pcm->channels;
                ochannels = ladspa->plug.gen.slave->channels;
        } else {
                ichannels = ladspa->plug.gen.slave->channels;
                ochannels = pcm->channels;
        }
	pchannels = calloc(1, sizeof(void *) * channels);
	if (pchannels == NULL)
	        return -ENOMEM;
	list = pcm->stream == SND_PCM_STREAM_PLAYBACK ? &ladspa->pplugins : &ladspa->cplugins;
	list_for_each(pos, list) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
		list_for_each(pos1, &plugin->instances) {
			instance = list_entry(pos1, snd_pcm_ladspa_instance_t, list);
			nchannels = channels;
			for (idx = 0; idx < instance->input.channels.size; idx++) {
			        chn = instance->input.channels.array[idx];
			        assert(instance->input.ports.array[idx] != NO_ASSIGN);
        			if (chn >= nchannels)
        			        nchannels = chn + 1;
                        }
			for (idx = 0; idx < instance->output.channels.size; idx++) {
			        chn = instance->output.channels.array[idx];
			        assert(instance->output.ports.array[idx] != NO_ASSIGN);
        			if (chn >= nchannels)
        			        nchannels = chn + 1;
                        }
                        if (nchannels != channels) {
                                npchannels = realloc(pchannels, nchannels * sizeof(void *));
                                if (npchannels == NULL) {
                                        free(pchannels);
                                        return -ENOMEM;
                                }
                                for (idx = channels; idx < nchannels; idx++)
                                        npchannels[idx] = NULL;
                                pchannels = npchannels;
                        }
                        assert(instance->input.data == NULL);
                        assert(instance->input.m_data == NULL);
                        assert(instance->output.data == NULL);
                        assert(instance->output.m_data == NULL);
                        instance->input.data = calloc(instance->input.channels.size, sizeof(void *));
                        instance->input.m_data = calloc(instance->input.channels.size, sizeof(void *));
                        instance->output.data = calloc(instance->output.channels.size, sizeof(void *));
                        instance->output.m_data = calloc(instance->output.channels.size, sizeof(void *));
                        if (instance->input.data == NULL ||
                            instance->input.m_data == NULL ||
                            instance->output.data == NULL ||
                            instance->output.m_data == NULL)
                                return -ENOMEM;
			for (idx = 0; idx < instance->input.channels.size; idx++) {
			        chn = instance->output.channels.array[idx];
			        if (pchannels[chn] == NULL && chn < ichannels) {
			                instance->input.data[idx] = NULL;
			                continue;
                                }
			        instance->input.data[idx] = pchannels[chn];
			        if (instance->input.data[idx] == NULL) {
                                        instance->input.data[idx] = snd_pcm_ladspa_allocate_zero(ladspa, 0);
                                        if (instance->input.data[idx] == NULL)
                                                return -ENOMEM;
                                }
                        }
                        for (idx = 0; idx < instance->output.channels.size; idx++) {
			        chn = instance->output.channels.array[idx];
                                /* if LADSPA plugin has no broken inplace */
                                instance->output.data[idx] = malloc(sizeof(LADSPA_Data) * ladspa->allocated);
                                if (instance->output.data[idx] == NULL)
                                        return -ENOMEM;
                                pchannels[chn] = instance->output.m_data[idx] = instance->output.data[idx];
                        }
		}
	}
	/* OPTIMIZE: we have already allocated areas for ALSA output channels */
	/* next loop deallocates the last output LADSPA areas and connects */
	/* them to ALSA areas (NULL) or dummy area ladpsa->free[1] ; */
	/* this algorithm might be optimized to not allocate the last LADSPA outputs */
	list_for_each(pos, list) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
		list_for_each(pos1, &plugin->instances) {
			instance = list_entry(pos1, snd_pcm_ladspa_instance_t, list);
                        for (idx = 0; idx < instance->output.channels.size; idx++) {
        			chn = instance->output.channels.array[idx];
                                if (instance->output.data[idx] == pchannels[chn]) {
					free(instance->output.m_data[idx]);
					instance->output.m_data[idx] = NULL;
                                        if (chn < ochannels) {
                                                instance->output.data[idx] = NULL;
                                        } else {
                                                instance->output.data[idx] = snd_pcm_ladspa_allocate_zero(ladspa, 1);
                                                if (instance->output.data[idx] == NULL)
                                                        return -ENOMEM;
                                        }
                                }
                        }
                }
        }
	free(pchannels);
	return 0;
}

static int snd_pcm_ladspa_init(snd_pcm_t *pcm)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;
	int err;
	
	snd_pcm_ladspa_free_instances(pcm, ladspa, 1);
	err = snd_pcm_ladspa_allocate_instances(pcm, ladspa);
	if (err < 0) {
		snd_pcm_ladspa_free_instances(pcm, ladspa, 1);
		return err;
	}
	err = snd_pcm_ladspa_allocate_memory(pcm, ladspa);
	if (err < 0) {
		snd_pcm_ladspa_free_instances(pcm, ladspa, 1);
		return err;
	}
	return 0;
}

static int snd_pcm_ladspa_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;

	snd_pcm_ladspa_free_instances(pcm, ladspa, 1);
	return snd_pcm_generic_hw_free(pcm);
}

static snd_pcm_uframes_t
snd_pcm_ladspa_write_areas(snd_pcm_t *pcm,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset,
			   snd_pcm_uframes_t size,
			   const snd_pcm_channel_area_t *slave_areas,
			   snd_pcm_uframes_t slave_offset,
			   snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;
	snd_pcm_ladspa_instance_t *instance;
	struct list_head *pos, *pos1;
	LADSPA_Data *data;
	unsigned int idx, chn, size1, size2;
	
	if (size > *slave_sizep)
		size = *slave_sizep;
        size2 = size;
        while (size > 0) {
                size1 = size;
                if (size1 > ladspa->allocated)
                        size1 = ladspa->allocated;
        	list_for_each(pos, &ladspa->pplugins) {
        		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
        		list_for_each(pos1, &plugin->instances) {
        			instance = list_entry(pos1, snd_pcm_ladspa_instance_t, list);
        			for (idx = 0; idx < instance->input.channels.size; idx++) {
                                        chn = instance->input.channels.array[idx];
                                        data = instance->input.data[idx];
                                        if (data == NULL) {
                                		data = (LADSPA_Data *)((char *)areas[chn].addr + (areas[chn].first / 8));
                                       		data += offset;
                                        }
                                        instance->desc->connect_port(instance->handle, instance->input.ports.array[idx], data);
        			}
        			for (idx = 0; idx < instance->output.channels.size; idx++) {
                                        chn = instance->output.channels.array[idx];
                                        data = instance->output.data[idx];
                                        if (data == NULL) {
                                		data = (LADSPA_Data *)((char *)slave_areas[chn].addr + (areas[chn].first / 8));
                                		data += slave_offset;
                                        }
					instance->desc->connect_port(instance->handle, instance->output.ports.array[idx], data);
        			}
        			instance->desc->run(instance->handle, size1);
        		}
        	}
        	offset += size1;
        	slave_offset += size1;
        	size -= size1;
	}
	*slave_sizep = size2;
	return size2;
}

static snd_pcm_uframes_t
snd_pcm_ladspa_read_areas(snd_pcm_t *pcm,
			  const snd_pcm_channel_area_t *areas,
			  snd_pcm_uframes_t offset,
			  snd_pcm_uframes_t size,
			  const snd_pcm_channel_area_t *slave_areas,
			  snd_pcm_uframes_t slave_offset,
			  snd_pcm_uframes_t *slave_sizep)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;
	snd_pcm_ladspa_instance_t *instance;
	struct list_head *pos, *pos1;
	LADSPA_Data *data;
	unsigned int idx, chn, size1, size2;;

	if (size > *slave_sizep)
		size = *slave_sizep;
        size2 = size;
        while (size > 0) {
                size1 = size;
                if (size1 > ladspa->allocated)
                        size1 = ladspa->allocated;
        	list_for_each(pos, &ladspa->cplugins) {
        		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
        		list_for_each(pos1, &plugin->instances) {
        			instance = list_entry(pos1, snd_pcm_ladspa_instance_t, list);
        			for (idx = 0; idx < instance->input.channels.size; idx++) {
                                        chn = instance->input.channels.array[idx];
                                        data = instance->input.data[idx];
                                        if (data == NULL) {
                                		data = (LADSPA_Data *)((char *)slave_areas[chn].addr + (areas[chn].first / 8));
                                		data += slave_offset;
                                        }	
                			instance->desc->connect_port(instance->handle, instance->input.ports.array[idx], data);
        			}
        			for (idx = 0; idx < instance->output.channels.size; idx++) {
                                        chn = instance->output.channels.array[idx];
                                        data = instance->output.data[idx];
                                        if (data == NULL) {
                                		data = (LADSPA_Data *)((char *)areas[chn].addr + (areas[chn].first / 8));
                                       		data += offset;
                                        }
        		        	instance->desc->connect_port(instance->handle, instance->output.ports.array[idx], data);
        			}
        			instance->desc->run(instance->handle, size1);
        		}
        	}
        	offset += size1;
        	slave_offset += size1;
        	size -= size1;
	}
	*slave_sizep = size2;
	return size2;
}

static void snd_pcm_ladspa_dump_direction(snd_pcm_ladspa_plugin_t *plugin,
                                          snd_pcm_ladspa_plugin_io_t *io,
                                          snd_output_t *out)
{
	unsigned int idx, midx;

	if (io->port_bindings_size == 0)
		goto __control;
	snd_output_printf(out, "    Audio %s port bindings:\n", io->pdesc == LADSPA_PORT_INPUT ? "input" : "output");
	for (idx = 0; idx < io->port_bindings_size; idx++) {
		if (io->port_bindings[idx] == NO_ASSIGN) 
			snd_output_printf(out, "      %i -> NONE\n", idx);
                else
        		snd_output_printf(out, "      %i -> %i\n", idx, io->port_bindings[idx]);
	}
      __control:
      	if (io->controls_size == 0)
      		return;
	snd_output_printf(out, "    Control %s port initial values:\n", io->pdesc == LADSPA_PORT_INPUT ? "input" : "output");
	for (idx = midx = 0; idx < plugin->desc->PortCount; idx++) {
		if ((plugin->desc->PortDescriptors[idx] & (io->pdesc | LADSPA_PORT_CONTROL)) == (io->pdesc | LADSPA_PORT_CONTROL)) {
        		snd_output_printf(out, "      %i \"%s\" = %.8f\n", idx, plugin->desc->PortNames[idx], io->controls[midx]);
        		midx++;
                }
        }
}

static void snd_pcm_ladspa_dump_array(snd_output_t *out,
                                      snd_pcm_ladspa_array_t *array,
                                      snd_pcm_ladspa_plugin_t *plugin)
{
        unsigned int size = array->size;
        unsigned int val, idx = 0;

        while (size-- > 0) {
                if (idx > 0) {
                        snd_output_putc(out, ',');
                        snd_output_putc(out, ' ');
                }
                val = array->array[idx++];
                if (val == NO_ASSIGN)
                        snd_output_putc(out, '-');
                else
                        snd_output_printf(out, "%u", val);
                if (plugin && val != NO_ASSIGN)
                        snd_output_printf(out, " \"%s\"", plugin->desc->PortNames[val]);
        }
}

static void snd_pcm_ladspa_plugins_dump(struct list_head *list, snd_output_t *out)
{
	struct list_head *pos, *pos2;
	
	list_for_each(pos, list) {
		snd_pcm_ladspa_plugin_t *plugin = list_entry(pos, snd_pcm_ladspa_plugin_t, list);
		snd_output_printf(out, "    Policy: %s\n", plugin->policy == SND_PCM_LADSPA_POLICY_NONE ? "none" : "duplicate");
		snd_output_printf(out, "    Filename: %s\n", plugin->filename);
		snd_output_printf(out, "    Plugin Name: %s\n", plugin->desc->Name);
		snd_output_printf(out, "    Plugin Label: %s\n", plugin->desc->Label);
		snd_output_printf(out, "    Plugin Unique ID: %lu\n", plugin->desc->UniqueID);
                snd_output_printf(out, "    Instances:\n");
		list_for_each(pos2, &plugin->instances) {
		        snd_pcm_ladspa_instance_t *in = (snd_pcm_ladspa_instance_t *) pos2;
		        snd_output_printf(out, "      Depth: %i\n", in->depth);
		        snd_output_printf(out, "         InChannels: ");
                        snd_pcm_ladspa_dump_array(out, &in->input.channels, NULL);
                        snd_output_printf(out, "\n         InPorts: ");
                        snd_pcm_ladspa_dump_array(out, &in->input.ports, plugin);
                        snd_output_printf(out, "\n         OutChannels: ");
                        snd_pcm_ladspa_dump_array(out, &in->output.channels, NULL);
                        snd_output_printf(out, "\n         OutPorts: ");
                        snd_pcm_ladspa_dump_array(out, &in->output.ports, plugin);
                        snd_output_printf(out, "\n");
		}
		snd_pcm_ladspa_dump_direction(plugin, &plugin->input, out);
		snd_pcm_ladspa_dump_direction(plugin, &plugin->output, out);
	}
}

static void snd_pcm_ladspa_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_ladspa_t *ladspa = pcm->private_data;

	snd_output_printf(out, "LADSPA PCM\n");
	snd_output_printf(out, "  Playback:\n");
	snd_pcm_ladspa_plugins_dump(&ladspa->pplugins, out);
	snd_output_printf(out, "  Capture:\n");
	snd_pcm_ladspa_plugins_dump(&ladspa->cplugins, out);
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(ladspa->plug.gen.slave, out);
}

static const snd_pcm_ops_t snd_pcm_ladspa_ops = {
	.close = snd_pcm_ladspa_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_ladspa_hw_refine,
	.hw_params = snd_pcm_ladspa_hw_params,
	.hw_free = snd_pcm_ladspa_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_ladspa_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

static int snd_pcm_ladspa_check_file(snd_pcm_ladspa_plugin_t * const plugin,
				     const char *filename,
				     const char *label,
				     const unsigned long ladspa_id)
{
	void *handle;

	assert(filename);
	handle = dlopen(filename, RTLD_LAZY);
	if (handle) {
		LADSPA_Descriptor_Function fcn = (LADSPA_Descriptor_Function)dlsym(handle, "ladspa_descriptor");
		if (fcn) {
			long idx;
			const LADSPA_Descriptor *d;
			for (idx = 0; (d = fcn(idx)) != NULL; idx++) {
/*
 * avoid locale problems - see ALSA bug#1553
 */
                                char *labellocale;
                                struct lconv *lc;
                                if (label != NULL) {
                                        lc = localeconv ();
                                        labellocale = malloc (strlen (label) + 1);
                                        if (labellocale == NULL) {
                                        	dlclose(handle);
                                                return -ENOMEM;
					}
                                        strcpy (labellocale, label);
                                        if (strrchr(labellocale, '.'))
                                                *strrchr (labellocale, '.') = *lc->decimal_point;
                                        if (strcmp(label, d->Label) && strcmp(labellocale, d->Label)) {
                                                free(labellocale);
                                                continue;
                                        }
                                        free (labellocale);
                                }
				if (ladspa_id > 0 && d->UniqueID != ladspa_id)
					continue;
				plugin->filename = strdup(filename);
				if (plugin->filename == NULL) {
					dlclose(handle);
					return -ENOMEM;
				}
				plugin->dl_handle = handle;
				plugin->desc = d;
				return 1;
			}
		}
		dlclose(handle);
	}
	return -ENOENT;
}

static int snd_pcm_ladspa_check_dir(snd_pcm_ladspa_plugin_t * const plugin,
				    const char *path,
				    const char *label,
				    const unsigned long ladspa_id)
{
	DIR *dir;
	struct dirent * dirent;
	int len = strlen(path), err;
	int need_slash;
	char *filename;
	
	if (len < 1)
		return 0;
	need_slash = path[len - 1] != '/';
	
	dir = opendir(path);
	if (!dir)
		return -ENOENT;
		
	while (1) {
		dirent = readdir(dir);
		if (!dirent) {
			closedir(dir);
			return 0;
		}
		
		filename = malloc(len + strlen(dirent->d_name) + 1 + need_slash);
		if (filename == NULL) {
			closedir(dir);
			return -ENOMEM;
		}
		strcpy(filename, path);
		if (need_slash)
			strcat(filename, "/");
		strcat(filename, dirent->d_name);
		err = snd_pcm_ladspa_check_file(plugin, filename, label, ladspa_id);
		free(filename);
		if (err < 0 && err != -ENOENT) {
			closedir(dir);
			return err;
		}
		if (err > 0) {
			closedir(dir);
			return 1;
		}
	}
	/* never reached */
	return 0;
}

static int snd_pcm_ladspa_look_for_plugin(snd_pcm_ladspa_plugin_t * const plugin,
					  const char *path,
					  const char *label,
					  const long ladspa_id)
{
	const char *c;
	size_t l;
	int err;
	
	for (c = path; (l = strcspn(c, ": ")) > 0; ) {
		char name[l + 1];
		char *fullpath;
		memcpy(name, c, l);
		name[l] = 0;
		err = snd_user_file(name, &fullpath);
		if (err < 0)
			return err;
		err = snd_pcm_ladspa_check_dir(plugin, fullpath, label, ladspa_id);
		free(fullpath);
		if (err < 0)
			return err;
		if (err > 0)
			return 0;
		c += l;
		if (!*c)
			break;
		c++;
	}
	return -ENOENT;
}					  

static int snd_pcm_ladspa_add_default_controls(snd_pcm_ladspa_plugin_t *lplug,
					       snd_pcm_ladspa_plugin_io_t *io) 
{
	unsigned int count = 0;
	LADSPA_Data *array;
	unsigned char *initialized;
	unsigned long idx;

	for (idx = 0; idx < lplug->desc->PortCount; idx++)
		if ((lplug->desc->PortDescriptors[idx] & (io->pdesc | LADSPA_PORT_CONTROL)) == (io->pdesc | LADSPA_PORT_CONTROL))
			count++;
	array = (LADSPA_Data *)calloc(count, sizeof(LADSPA_Data));
	if (!array)
		return -ENOMEM;
	initialized = (unsigned char *)calloc(count, sizeof(unsigned char));
	if (!initialized) {
		free(array);
		return -ENOMEM;
	}
	io->controls_size = count;
	io->controls_initialized = initialized;
	io->controls = array;

	return 0;
}	

static int snd_pcm_ladspa_parse_controls(snd_pcm_ladspa_plugin_t *lplug,
					 snd_pcm_ladspa_plugin_io_t *io,
					 snd_config_t *controls) 
{
	snd_config_iterator_t i, next;
	int err;

	if (snd_config_get_type(controls) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("controls definition must be a compound");
		return -EINVAL;
	}

	snd_config_for_each(i, next, controls) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		long lval;
		unsigned int port, uval;
		double dval;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		err = safe_strtol(id, &lval);
		if (err >= 0) {
			err = snd_pcm_ladspa_find_port(&port, lplug, io->pdesc | LADSPA_PORT_CONTROL, lval);
		} else {
			err = snd_pcm_ladspa_find_sport(&port, lplug, io->pdesc | LADSPA_PORT_CONTROL, id);
		}
		if (err < 0) {
			SNDERR("Unable to find an control port (%s)", id);
			return err;
		}
		if (snd_config_get_ireal(n, &dval) < 0) {
			SNDERR("Control port %s has not an float or integer value", id);
			return err;
		}
		err = snd_pcm_ladspa_find_port_idx(&uval, lplug, io->pdesc | LADSPA_PORT_CONTROL, port);
		if (err < 0) {
			SNDERR("internal error");
			return err;
		}
		io->controls_initialized[uval] = 1;
		io->controls[uval] = (LADSPA_Data)dval;
	}

	return 0;
}

static int snd_pcm_ladspa_parse_bindings(snd_pcm_ladspa_plugin_t *lplug,
					 snd_pcm_ladspa_plugin_io_t *io,
					 snd_config_t *bindings) 
{
	unsigned int count = 0;
	unsigned int *array;
	snd_config_iterator_t i, next;
	int err;

	if (snd_config_get_type(bindings) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("bindings definition must be a compound");
		return -EINVAL;
	}
	snd_config_for_each(i, next, bindings) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		long channel;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		err = safe_strtol(id, &channel);
		if (err < 0 || channel < 0) {
			SNDERR("Invalid channel number: %s", id);
			return -EINVAL;
		}
		if (lplug->policy == SND_PCM_LADSPA_POLICY_DUPLICATE && channel > 0) {
			SNDERR("Wrong channel specification for duplicate policy");
			return -EINVAL;
		}
		if (count < (unsigned int)(channel + 1))
			count = (unsigned int)(channel + 1);
	}
	if (count > 0) {
		array = (unsigned int *)calloc(count, sizeof(unsigned int));
		if (! array)
			return -ENOMEM;
		memset(array, 0xff, count * sizeof(unsigned int));
		io->port_bindings_size = count;
		io->port_bindings = array;
		snd_config_for_each(i, next, bindings) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id, *sport;
			long channel, port;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			err = safe_strtol(id, &channel);
			if (err < 0 || channel < 0) {
				assert(0);	/* should never happen */
				return -EINVAL;
			}
			err = snd_config_get_integer(n, &port);
			if (err >= 0) {
				err = snd_pcm_ladspa_find_port(&array[channel], lplug, io->pdesc | LADSPA_PORT_AUDIO, port);
				if (err < 0) {
					SNDERR("Unable to find an audio port (%li) for channel %s", port, id);
					return err;
				}
				continue;
			}
			err = snd_config_get_string(n, &sport);
			if (err < 0) {
				SNDERR("Invalid LADSPA port field type for %s", id);
				return -EINVAL;
			}
			err = snd_pcm_ladspa_find_sport(&array[channel], lplug, io->pdesc | LADSPA_PORT_AUDIO, sport);
			if (err < 0) {
				SNDERR("Unable to find an audio port (%s) for channel %s", sport, id);
				return err;
			}
		}
	}

	return 0;
}

static int snd_pcm_ladspa_parse_ioconfig(snd_pcm_ladspa_plugin_t *lplug,
					 snd_pcm_ladspa_plugin_io_t *io,
					 snd_config_t *conf)
{
	snd_config_iterator_t i, next;
	snd_config_t *bindings = NULL, *controls = NULL;
	int err;

	/* always add default controls for both input and output */
	err = snd_pcm_ladspa_add_default_controls(lplug, io);
	if (err < 0) {
		SNDERR("error adding default controls");
		return err;
	}
		
	if (conf == NULL) {
		return 0;
	}

	if (snd_config_get_type(conf) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("input or output definition must be a compound");
		return -EINVAL;
	}
	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "bindings") == 0) {
			bindings = n;
			continue;
		}
		if (strcmp(id, "controls") == 0) {
			controls = n;
			continue;
		}
	}

	/* ignore values of parameters for output controls */
	if (controls && !(io->pdesc & LADSPA_PORT_OUTPUT)) {
 		err = snd_pcm_ladspa_parse_controls(lplug, io, controls);
		if (err < 0) 
			return err;
	}

	if (bindings) {
 		err = snd_pcm_ladspa_parse_bindings(lplug, io, bindings);
		if (err < 0) 
			return err;
	}


	return 0;
}

static int snd_pcm_ladspa_add_plugin(struct list_head *list,
				     const char *path,
				     snd_config_t *plugin,
				     int reverse)
{
	snd_config_iterator_t i, next;
	const char *label = NULL, *filename = NULL;
	long ladspa_id = 0;
	int err;
	snd_pcm_ladspa_plugin_t *lplug;
	snd_pcm_ladspa_policy_t policy = SND_PCM_LADSPA_POLICY_DUPLICATE;
	snd_config_t *input = NULL, *output = NULL;

	snd_config_for_each(i, next, plugin) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "label") == 0) {
			err = snd_config_get_string(n, &label);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "id") == 0) {
			err = snd_config_get_integer(n, &ladspa_id);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "filename") == 0) {
			err = snd_config_get_string(n, &filename);
			if (err < 0)
				return err;
			continue;
		}
		if (strcmp(id, "input") == 0) {
			input = n;
			continue;
		}
		if (strcmp(id, "output") == 0) {
			output = n;
			continue;
		}
		if (strcmp(id, "policy") == 0) {
			const char *str;
			err = snd_config_get_string(n, &str);
			if (err < 0) {
				SNDERR("policy field must be a string");
				return err;
			}
			if (strcmp(str, "none") == 0)
				policy = SND_PCM_LADSPA_POLICY_NONE;
			else if (strcmp(str, "duplicate") == 0)
				policy = SND_PCM_LADSPA_POLICY_DUPLICATE;
			else {
				SNDERR("unknown policy definition");
				return -EINVAL;
			}
			continue;
		}
	}
	if (label == NULL && ladspa_id <= 0) {
		SNDERR("no plugin label or id");
		return -EINVAL;
	}
	lplug = (snd_pcm_ladspa_plugin_t *)calloc(1, sizeof(snd_pcm_ladspa_plugin_t));
	if (lplug == NULL)
		return -ENOMEM;
	lplug->policy = policy;
	lplug->input.pdesc = LADSPA_PORT_INPUT;
	lplug->output.pdesc = LADSPA_PORT_OUTPUT;
	INIT_LIST_HEAD(&lplug->instances);
	if (filename) {
		err = snd_pcm_ladspa_check_file(lplug, filename, label, ladspa_id);
		if (err < 0) {
			SNDERR("Unable to load plugin '%s' ID %li, filename '%s'", label, ladspa_id, filename);
			free(lplug);
			return err;
		}
	} else {
		err = snd_pcm_ladspa_look_for_plugin(lplug, path, label, ladspa_id);
		if (err < 0) {
			SNDERR("Unable to find or load plugin '%s' ID %li, path '%s'", label, ladspa_id, path);
			free(lplug);
			return err;
		}
	}
	if (!reverse) {
		list_add_tail(&lplug->list, list);
	} else {
		list_add(&lplug->list, list);
	}
	err = snd_pcm_ladspa_parse_ioconfig(lplug, &lplug->input, input);
	if (err < 0)
		return err;
	err = snd_pcm_ladspa_parse_ioconfig(lplug, &lplug->output, output);
	if (err < 0)
		return err;
	return 0;
}

static int snd_pcm_ladspa_build_plugins(struct list_head *list,
					const char *path,
					snd_config_t *plugins,
					int reverse)
{
	snd_config_iterator_t i, next;
	int idx = 0, hit, err;

	if (plugins == NULL)	/* nothing TODO */
		return 0;
	if (snd_config_get_type(plugins) != SND_CONFIG_TYPE_COMPOUND) {
		SNDERR("plugins must be defined inside a compound");
		return -EINVAL;
	}
	do {
		hit = 0;
		snd_config_for_each(i, next, plugins) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			long i;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not an integer", id);
				return err;
			}
			if (i == idx) {
				idx++;
				err = snd_pcm_ladspa_add_plugin(list, path, n, reverse);
				if (err < 0)
					return err;
				hit = 1;
			}
		}
	} while (hit);
	if (list_empty(list)) {
		SNDERR("empty plugin list is not accepted");
		return -EINVAL;
	}
	return 0;
}

/**
 * \brief Creates a new LADSPA<->ALSA Plugin
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param ladspa_path The path for LADSPA plugins
 * \param channels Force input channel count to LADSPA plugin chain, 0 = no force (auto)
 * \param ladspa_pplugins The playback configuration
 * \param ladspa_cplugins The capture configuration
 * \param slave Slave PCM handle
 * \param close_slave When set, the slave PCM handle is closed with copy PCM
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_ladspa_open(snd_pcm_t **pcmp, const char *name,
			const char *ladspa_path,
			unsigned int channels,
			snd_config_t *ladspa_pplugins,
			snd_config_t *ladspa_cplugins,
			snd_pcm_t *slave, int close_slave)
{
	snd_pcm_t *pcm;
	snd_pcm_ladspa_t *ladspa;
	int err, reverse = 0;

	assert(pcmp && (ladspa_pplugins || ladspa_cplugins) && slave);

	if (!ladspa_path && !(ladspa_path = getenv("LADSPA_PATH")))
		return -ENOENT;
	ladspa = calloc(1, sizeof(snd_pcm_ladspa_t));
	if (!ladspa)
		return -ENOMEM;
	snd_pcm_plugin_init(&ladspa->plug);
	ladspa->plug.init = snd_pcm_ladspa_init;
	ladspa->plug.read = snd_pcm_ladspa_read_areas;
	ladspa->plug.write = snd_pcm_ladspa_write_areas;
	ladspa->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	ladspa->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	ladspa->plug.gen.slave = slave;
	ladspa->plug.gen.close_slave = close_slave;

	INIT_LIST_HEAD(&ladspa->pplugins);
	INIT_LIST_HEAD(&ladspa->cplugins);
	ladspa->channels = channels;

	if (slave->stream == SND_PCM_STREAM_PLAYBACK) {
		err = snd_pcm_ladspa_build_plugins(&ladspa->pplugins, ladspa_path, ladspa_pplugins, reverse);
		if (err < 0) {
			snd_pcm_ladspa_free(ladspa);
			return err;
		}
	}
	if (slave->stream == SND_PCM_STREAM_CAPTURE) {
		if (ladspa_cplugins == ladspa_pplugins)
			reverse = 1;
		err = snd_pcm_ladspa_build_plugins(&ladspa->cplugins, ladspa_path, ladspa_cplugins, reverse);
		if (err < 0) {
			snd_pcm_ladspa_free(ladspa);
			return err;
		}
	}

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_LADSPA, name, slave->stream, slave->mode);
	if (err < 0) {
		snd_pcm_ladspa_free(ladspa);
		return err;
	}
	pcm->ops = &snd_pcm_ladspa_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = ladspa;
	pcm->poll_fd = slave->poll_fd;
	pcm->poll_events = slave->poll_events;
	pcm->monotonic = slave->monotonic;
	snd_pcm_set_hw_ptr(pcm, &ladspa->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &ladspa->plug.appl_ptr, -1, 0);
	*pcmp = pcm;

	return 0;
}

/*! \page pcm_plugins

\section pcm_plugins_ladpsa Plugin: LADSPA <-> ALSA

This plugin allows to apply a set of LADPSA plugins.
The input and output format is always #SND_PCM_FORMAT_FLOAT (note: this type
can be either little or big-endian depending on architecture).

The policy duplicate means that there must be only one binding definition for
channel zero. This definition is automatically duplicated for all channels.
If the LADSPA plugin has multiple audio inputs or outputs the policy duplicate
is automatically switched to policy none.

The plugin serialization works as expected. You can eventually use more
channels (inputs / outputs) inside the LADPSA plugin chain than processed
in the ALSA plugin chain. If ALSA channel does not exist for given LADSPA
input audio port, zero samples are given to this LADSPA port. On the output
side (ALSA next plugin input), the valid channels are checked, too.
If specific ALSA channel does not exist, the LADSPA output port is
connected to a dummy sample area.

Instances of LADSPA plugins are created dynamically.

\code
pcm.name {
        type ladspa             # ALSA<->LADSPA PCM
        slave STR               # Slave name
        # or
        slave {                 # Slave definition
                pcm STR         # Slave PCM name
                # or
                pcm { }         # Slave PCM definition
        }
        [channels INT]		# count input channels (input to LADSPA plugin chain)
	[path STR]		# Path (directory) with LADSPA plugins
	plugins |		# Definition for both directions
        playback_plugins |	# Definition for playback direction
	capture_plugins {	# Definition for capture direction
		N {		# Configuration for LADPSA plugin N
			[id INT]	# LADSPA plugin ID (for example 1043)
			[label STR]	# LADSPA plugin label (for example 'delay_5s')
			[filename STR]	# Full filename of .so library with LADSPA plugin code
			[policy STR]	# Policy can be 'none' or 'duplicate'
			input | output {
				bindings {
					C INT or STR	# C - channel, INT - audio port index, STR - audio port name
				}
				controls {
				        # valid only in the input block
					I INT or REAL	# I - control port index, INT or REAL - control value
					# or
					STR INT or REAL	# STR - control port name, INT or REAL - control value
				}
			}
		}
	}
}
\endcode

\subsection pcm_plugins_ladspa_funcref Function reference

<UL>
  <LI>snd_pcm_ladspa_open()
  <LI>_snd_pcm_ladspa_open()
</UL>

*/

/**
 * \brief Creates a new LADSPA<->ALSA PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with LADSPA<->ALSA PCM description
 * \param stream Stream type
 * \param mode Stream mode
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_ladspa_open(snd_pcm_t **pcmp, const char *name,
			 snd_config_t *root, snd_config_t *conf, 
			 snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	int err;
	snd_pcm_t *spcm;
	snd_config_t *slave = NULL, *sconf;
	const char *path = NULL;
	long channels = 0;
	snd_config_t *plugins = NULL, *pplugins = NULL, *cplugins = NULL;
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
		if (strcmp(id, "path") == 0) {
			snd_config_get_string(n, &path);
			continue;
		}
		if (strcmp(id, "channels") == 0) {
			snd_config_get_integer(n, &channels);
			if (channels > 1024)
			        channels = 1024;
                        if (channels < 0)
                                channels = 0;
			continue;
		}
		if (strcmp(id, "plugins") == 0) {
			plugins = n;
			continue;
		}
		if (strcmp(id, "playback_plugins") == 0) {
			pplugins = n;
			continue;
		}
		if (strcmp(id, "capture_plugins") == 0) {
			cplugins = n;
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}
	if (!slave) {
		SNDERR("slave is not defined");
		return -EINVAL;
	}
	if (plugins) {
		if (pplugins || cplugins) {
			SNDERR("'plugins' definition cannot be combined with 'playback_plugins' or 'capture_plugins'");
			return -EINVAL;
		}
		pplugins = plugins;
		cplugins = plugins;
	}
	err = snd_pcm_slave_conf(root, slave, &sconf, 0);
	if (err < 0)
		return err;
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, conf);
	snd_config_delete(sconf);
	if (err < 0)
		return err;
	err = snd_pcm_ladspa_open(pcmp, name, path, channels, pplugins, cplugins, spcm, 1);
	if (err < 0)
		snd_pcm_close(spcm);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_ladspa_open, SND_PCM_DLSYM_VERSION);
#endif
