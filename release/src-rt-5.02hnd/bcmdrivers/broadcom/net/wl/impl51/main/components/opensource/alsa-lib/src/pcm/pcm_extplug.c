/**
 * \file pcm/pcm_extplug.c
 * \ingroup Plugin_SDK
 * \brief External Filter Plugin SDK
 * \author Takashi Iwai <tiwai@suse.de>
 * \date 2005
 */
/*
 *  PCM - External Filter Plugin SDK
 *  Copyright (c) 2005 by Takashi Iwai <tiwai@suse.de>
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
#include "pcm_extplug.h"
#include "pcm_ext_parm.h"

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_extplug = "";
#endif

#ifndef DOC_HIDDEN

typedef struct snd_pcm_extplug_priv {
	snd_pcm_plugin_t plug;
	snd_pcm_extplug_t *data;
	struct snd_ext_parm params[SND_PCM_EXTPLUG_HW_PARAMS];
	struct snd_ext_parm sparams[SND_PCM_EXTPLUG_HW_PARAMS];
} extplug_priv_t;

static const int hw_params_type[SND_PCM_EXTPLUG_HW_PARAMS] = {
	[SND_PCM_EXTPLUG_HW_FORMAT] = SND_PCM_HW_PARAM_FORMAT,
	[SND_PCM_EXTPLUG_HW_CHANNELS] = SND_PCM_HW_PARAM_CHANNELS
};

#define is_mask_type(i) (hw_params_type[i] < SND_PCM_HW_PARAM_FIRST_INTERVAL)

static const unsigned int excl_parbits[SND_PCM_EXTPLUG_HW_PARAMS] = {
	[SND_PCM_EXTPLUG_HW_FORMAT] = (SND_PCM_HW_PARBIT_FORMAT|
				       SND_PCM_HW_PARBIT_SUBFORMAT |
				       SND_PCM_HW_PARBIT_SAMPLE_BITS),
	[SND_PCM_EXTPLUG_HW_CHANNELS] = (SND_PCM_HW_PARBIT_CHANNELS|
					 SND_PCM_HW_PARBIT_FRAME_BITS),
};

/*
 * set min/max values for the given parameter
 */
int snd_ext_parm_set_minmax(struct snd_ext_parm *parm, unsigned int min, unsigned int max)
{
	parm->num_list = 0;
	free(parm->list);
	parm->list = NULL;
	parm->min = min;
	parm->max = max;
	parm->active = 1;
	return 0;
}

/*
 * set the list of available values for the given parameter
 */
static int val_compar(const void *ap, const void *bp)
{
	return *(const unsigned int *)ap - *(const unsigned int *)bp;
}

int snd_ext_parm_set_list(struct snd_ext_parm *parm, unsigned int num_list, const unsigned int *list)
{
	unsigned int *new_list;

	new_list = malloc(sizeof(*new_list) * num_list);
	if (new_list == NULL)
		return -ENOMEM;
	memcpy(new_list, list, sizeof(*new_list) * num_list);
	qsort(new_list, num_list, sizeof(*new_list), val_compar);

	free(parm->list);
	parm->num_list = num_list;
	parm->list = new_list;
	parm->active = 1;
	return 0;
}

void snd_ext_parm_clear(struct snd_ext_parm *parm)
{
	free(parm->list);
	memset(parm, 0, sizeof(*parm));
}

/*
 * limit the interval to the given list
 */
int snd_interval_list(snd_interval_t *ival, int num_list, unsigned int *list)
{
	int imin, imax;
	int changed = 0;

	if (snd_interval_empty(ival))
		return -ENOENT;
	for (imin = 0; imin < num_list; imin++) {
		if (ival->min == list[imin] && ! ival->openmin)
			break;
		if (ival->min <= list[imin]) {
			ival->min = list[imin];
			ival->openmin = 0;
			changed = 1;
			break;
		}
	}
	if (imin >= num_list)
		return -EINVAL;
	for (imax = num_list - 1; imax >= imin; imax--) {
		if (ival->max == list[imax] && ! ival->openmax)
			break;
		if (ival->max >= list[imax]) {
			ival->max = list[imax];
			ival->openmax = 0;
			changed = 1;
			break;
		}
	}
	if (imax < imin)
		return -EINVAL;
	return changed;
}

/*
 * refine the interval parameter
 */
int snd_ext_parm_interval_refine(snd_interval_t *ival, struct snd_ext_parm *parm, int type)
{
	parm += type;
	if (! parm->active)
		return 0;
	ival->integer |= parm->integer;
	if (parm->num_list) {
		return snd_interval_list(ival, parm->num_list, parm->list);
	} else if (parm->min || parm->max) {
		snd_interval_t t;
		memset(&t, 0, sizeof(t));
		snd_interval_set_minmax(&t, parm->min, parm->max);
		t.integer = ival->integer;
		return snd_interval_refine(ival, &t);
	}
	return 0;
}

/*
 * refine the mask parameter
 */
int snd_ext_parm_mask_refine(snd_mask_t *mask, struct snd_ext_parm *parm, int type)
{
	snd_mask_t bits;
	unsigned int i;

	parm += type;
	memset(&bits, 0, sizeof(bits));
	for (i = 0; i < parm->num_list; i++)
		bits.bits[parm->list[i] / 32] |= 1U << (parm->list[i] % 32);
	return snd_mask_refine(mask, &bits);
}


/*
 * hw_refine callback
 */
static int extplug_hw_refine(snd_pcm_hw_params_t *hw_params,
			     struct snd_ext_parm *parm)
{
	int i, err, change = 0;
	for (i = 0; i < SND_PCM_EXTPLUG_HW_PARAMS; i++) {
		int type = hw_params_type[i];
		if (is_mask_type(i))
			err = snd_ext_parm_mask_refine(hw_param_mask(hw_params, type),
						       parm, i);
		else
			err = snd_ext_parm_interval_refine(hw_param_interval(hw_params, type),
							   parm, i);
		if (err < 0)
			return err;
		change |= err;
	}
	return change;
}

static int snd_pcm_extplug_hw_refine_cprepare(snd_pcm_t *pcm,
					      snd_pcm_hw_params_t *params)
{
	extplug_priv_t *ext = pcm->private_data;
	int err;
	snd_pcm_access_mask_t access_mask = { SND_PCM_ACCBIT_SHM };
	err = _snd_pcm_hw_param_set_mask(params, SND_PCM_HW_PARAM_ACCESS,
					 &access_mask);
	if (err < 0)
		return err;
	err = extplug_hw_refine(params, ext->params);
	if (err < 0)
		return err;
	params->info &= ~(SND_PCM_INFO_MMAP | SND_PCM_INFO_MMAP_VALID);
	return 0;
}

static int snd_pcm_extplug_hw_refine_sprepare(snd_pcm_t *pcm,
					      snd_pcm_hw_params_t *sparams)
{
	extplug_priv_t *ext = pcm->private_data;
	snd_pcm_access_mask_t saccess_mask = { SND_PCM_ACCBIT_MMAP };
	_snd_pcm_hw_params_any(sparams);
	_snd_pcm_hw_param_set_mask(sparams, SND_PCM_HW_PARAM_ACCESS,
				   &saccess_mask);
	extplug_hw_refine(sparams, ext->sparams);
	return 0;
}

static unsigned int get_links(struct snd_ext_parm *params)
{
	int i;
	unsigned int links = (SND_PCM_HW_PARBIT_FORMAT |
			      SND_PCM_HW_PARBIT_SUBFORMAT |
			      SND_PCM_HW_PARBIT_SAMPLE_BITS |
			      SND_PCM_HW_PARBIT_CHANNELS |
			      SND_PCM_HW_PARBIT_FRAME_BITS |
			      SND_PCM_HW_PARBIT_RATE |
			      SND_PCM_HW_PARBIT_PERIODS |
			      SND_PCM_HW_PARBIT_PERIOD_SIZE |
			      SND_PCM_HW_PARBIT_PERIOD_TIME |
			      SND_PCM_HW_PARBIT_BUFFER_SIZE |
			      SND_PCM_HW_PARBIT_BUFFER_TIME |
			      SND_PCM_HW_PARBIT_TICK_TIME);

	for (i = 0; i < SND_PCM_EXTPLUG_HW_PARAMS; i++) {
		if (params[i].active)
			links &= ~excl_parbits[i];
	}
	return links;
}

static int snd_pcm_extplug_hw_refine_schange(snd_pcm_t *pcm,
					     snd_pcm_hw_params_t *params,
					     snd_pcm_hw_params_t *sparams)
{
	extplug_priv_t *ext = pcm->private_data;
	unsigned int links = get_links(ext->sparams);

	return _snd_pcm_hw_params_refine(sparams, links, params);
}
	
static int snd_pcm_extplug_hw_refine_cchange(snd_pcm_t *pcm,
					     snd_pcm_hw_params_t *params,
					     snd_pcm_hw_params_t *sparams)
{
	extplug_priv_t *ext = pcm->private_data;
	unsigned int links = get_links(ext->params);

	return _snd_pcm_hw_params_refine(params, links, sparams);
}

static int snd_pcm_extplug_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	int err = snd_pcm_hw_refine_slave(pcm, params,
				       snd_pcm_extplug_hw_refine_cprepare,
				       snd_pcm_extplug_hw_refine_cchange,
				       snd_pcm_extplug_hw_refine_sprepare,
				       snd_pcm_extplug_hw_refine_schange,
				       snd_pcm_generic_hw_refine);
	return err;
}

/*
 * hw_params callback
 */
static int snd_pcm_extplug_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{

	extplug_priv_t *ext = pcm->private_data;
	snd_pcm_t *slave = ext->plug.gen.slave;
	int err = snd_pcm_hw_params_slave(pcm, params,
					  snd_pcm_extplug_hw_refine_cchange,
					  snd_pcm_extplug_hw_refine_sprepare,
					  snd_pcm_extplug_hw_refine_schange,
					  snd_pcm_generic_hw_params);
	if (err < 0)
		return err;
	ext->data->slave_format = slave->format;
	ext->data->slave_subformat = slave->subformat;
	ext->data->slave_channels = slave->channels;
	ext->data->rate = slave->rate;
	INTERNAL(snd_pcm_hw_params_get_format)(params, &ext->data->format);
	INTERNAL(snd_pcm_hw_params_get_subformat)(params, &ext->data->subformat);
	INTERNAL(snd_pcm_hw_params_get_channels)(params, &ext->data->channels);

	if (ext->data->callback->hw_params) {
		err = ext->data->callback->hw_params(ext->data, params);
		if (err < 0)
			return err;
	}
	return 0;
}

/*
 * hw_free callback
 */
static int snd_pcm_extplug_hw_free(snd_pcm_t *pcm)
{
	extplug_priv_t *ext = pcm->private_data;

	snd_pcm_hw_free(ext->plug.gen.slave);
	if (ext->data->callback->hw_free)
		return ext->data->callback->hw_free(ext->data);
	return 0;
}

/*
 * write_areas skeleton - call transfer callback
 */
static snd_pcm_uframes_t
snd_pcm_extplug_write_areas(snd_pcm_t *pcm,
			    const snd_pcm_channel_area_t *areas,
			    snd_pcm_uframes_t offset,
			    snd_pcm_uframes_t size,
			    const snd_pcm_channel_area_t *slave_areas,
			    snd_pcm_uframes_t slave_offset,
			    snd_pcm_uframes_t *slave_sizep)
{
	extplug_priv_t *ext = pcm->private_data;

	if (size > *slave_sizep)
		size = *slave_sizep;
	size = ext->data->callback->transfer(ext->data, slave_areas, slave_offset,
					     areas, offset, size);
	*slave_sizep = size;
	return size;
}

/*
 * read_areas skeleton - call transfer callback
 */
static snd_pcm_uframes_t
snd_pcm_extplug_read_areas(snd_pcm_t *pcm,
			   const snd_pcm_channel_area_t *areas,
			   snd_pcm_uframes_t offset,
			   snd_pcm_uframes_t size,
			   const snd_pcm_channel_area_t *slave_areas,
			   snd_pcm_uframes_t slave_offset,
			   snd_pcm_uframes_t *slave_sizep)
{
	extplug_priv_t *ext = pcm->private_data;

	if (size > *slave_sizep)
		size = *slave_sizep;
	size = ext->data->callback->transfer(ext->data, areas, offset,
					     slave_areas, slave_offset, size);
	*slave_sizep = size;
	return size;
}

/*
 * call init callback
 */
static int snd_pcm_extplug_init(snd_pcm_t *pcm)
{
	extplug_priv_t *ext = pcm->private_data;
	return ext->data->callback->init(ext->data);
}

/*
 * dump setup
 */
static void snd_pcm_extplug_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	extplug_priv_t *ext = pcm->private_data;

	if (ext->data->callback->dump)
		ext->data->callback->dump(ext->data, out);
	else {
		if (ext->data->name)
			snd_output_printf(out, "%s\n", ext->data->name);
		else
			snd_output_printf(out, "External PCM Plugin\n");
		if (pcm->setup) {
			snd_output_printf(out, "Its setup is:\n");
			snd_pcm_dump_setup(pcm, out);
		}
	}
	snd_output_printf(out, "Slave: ");
	snd_pcm_dump(ext->plug.gen.slave, out);
}

static void clear_ext_params(extplug_priv_t *ext)
{
	int i;
	for (i = 0; i < SND_PCM_EXTPLUG_HW_PARAMS; i++) {
		snd_ext_parm_clear(&ext->params[i]);
		snd_ext_parm_clear(&ext->sparams[i]);
	}
}

static int snd_pcm_extplug_close(snd_pcm_t *pcm)
{
	extplug_priv_t *ext = pcm->private_data;

	snd_pcm_close(ext->plug.gen.slave);
	clear_ext_params(ext);
	if (ext->data->callback->close)
		ext->data->callback->close(ext->data);
	free(ext);
	return 0;
}

static const snd_pcm_ops_t snd_pcm_extplug_ops = {
	.close = snd_pcm_extplug_close,
	.info = snd_pcm_generic_info,
	.hw_refine = snd_pcm_extplug_hw_refine,
	.hw_params = snd_pcm_extplug_hw_params,
	.hw_free = snd_pcm_extplug_hw_free,
	.sw_params = snd_pcm_generic_sw_params,
	.channel_info = snd_pcm_generic_channel_info,
	.dump = snd_pcm_extplug_dump,
	.nonblock = snd_pcm_generic_nonblock,
	.async = snd_pcm_generic_async,
	.mmap = snd_pcm_generic_mmap,
	.munmap = snd_pcm_generic_munmap,
};

#endif /* !DOC_HIDDEN */

/*
 * Exported functions
 */

/*! \page pcm_external_plugins PCM External Plugin SDK

\section pcm_externals External Plugins

The external plugins are implemented in a shared object file located
at /usr/lib/alsa-lib (the exact location depends on the build option
and asoundrc configuration).  It has to be the file like
libasound_module_pcm_MYPLUGIN.so, where MYPLUGIN corresponds to your
own plugin name.

The entry point of the plugin is defined via
#SND_PCM_PLUGIN_DEFINE_FUNC() macro.  This macro defines the function
with a proper name to be referred from alsa-lib.  The function takes
the following 6 arguments:
\code
int (snd_pcm_t **pcmp, const char *name, snd_config_t *root,
	snd_config_t *conf, snd_pcm_stream_t stream, int mode)
\endcode
The first argument, pcmp, is the pointer to store the resultant PCM
handle.  The arguments name, root, stream and mode are the parameters
to be passed to the plugin constructor.  The conf is the configuration
tree for the plugin.  The arguments above are defined in the macro
itself, so don't use variables with the same names to shadow
parameters.

After parsing the configuration parameters in the given conf tree,
usually you will call the external plugin API function,
#snd_pcm_extplug_create() or #snd_pcm_ioplug_create(), depending
on the plugin type.  The PCM handle must be filled *pcmp in return.
Then this function must return either a value 0 when succeeded, or a
negative value as the error code. 

Finally, add #SND_PCM_PLUGIN_SYMBOL() with the name of your
plugin as the argument at the end.  This defines the proper versioned
symbol as the reference.

The typical code would look like below:
\code
struct myplug_info {
	snd_pcm_extplug_t ext;
	int my_own_data;
	...
};

SND_PCM_PLUGIN_DEFINE_FUNC(myplug)
{
	snd_config_iterator_t i, next;
	snd_config_t *slave = NULL;
	struct myplug_info *myplug;
	int err;

	snd_config_for_each(i, next, conf) {
		snd_config_t *n = snd_config_iterator_entry(i);
		const char *id;
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (strcmp(id, "comment") == 0 || strcmp(id, "type") == 0)
			continue;
		if (strcmp(id, "slave") == 0) {
			slave = n;
			continue;
		}
		if (strcmp(id, "my_own_parameter") == 0) {
			....
			continue;
		}
		SNDERR("Unknown field %s", id);
		return -EINVAL;
	}

	if (! slave) {
		SNDERR("No slave defined for myplug");
		return -EINVAL;
	}

	myplug = calloc(1, sizeof(*myplug));
	if (myplug == NULL)
		return -ENOMEM;

	myplug->ext.version = SND_PCM_EXTPLUG_VERSION;
	myplug->ext.name = "My Own Plugin";
	myplug->ext.callback = &my_own_callback;
	myplug->ext.private_data = myplug;
	....

	err = snd_pcm_extplug_create(&myplug->ext, name, root, conf, stream, mode);
	if (err < 0) {
		myplug_free(myplug);
		return err;
	}

	*pcmp = myplug->ext.pcm;
	return 0;
}

SND_PCM_PLUGIN_SYMBOL(myplug);
\endcode

Read the codes in alsa-plugins package for the real examples.


\section pcm_extplug External Plugin: Filter-Type Plugin

The filter-type plugin is a plugin to convert the PCM signals from the input
and feeds to the output.  Thus, this plugin always needs a slave PCM as its output.

The plugin can modify the format and the channels of the input/output PCM.
It can <i>not</i> modify the sample rate (because of simplicity reason).

The following fields have to be filled in extplug record before calling
#snd_pcm_extplug_create() : version, name, callback.
Otherfields are optional and should be initialized with zero.

The constant #SND_PCM_EXTPLUG_VERSION must be passed to the version
field for the version check in alsa-lib.  A non-NULL ASCII string
has to be passed to the name field.  The callback field contains the 
table of callback functions for this plugin (defined as
#snd_pcm_extplug_callback_t).

The driver can set an arbitrary value (pointer) to private_data
field to refer its own data in the callbacks.

The rest fields are filled by #snd_pcm_extplug_create().  The pcm field
is the resultant PCM handle.  The others are the current status of the
PCM.

The callback functions in #snd_pcm_extplug_callback_t define the real
behavior of the driver.
At least, transfer callback must be given.  This callback is called
at each time certain size of data block is transfered to the slave
PCM.  Other callbacks are optional.  

The close callback is called when the PCM is closed.  If the plugin
allocates private resources, this is the place to release them
again.  The hw_params and hw_free callbacks are called at
#snd_pcm_hw_params() and #snd_pcm_hw_free() API calls,
respectively.  The last, dump callback, is called for printing the
information of the given plugin.

The init callback is called when the PCM is at prepare state or any
initialization is issued.  Use this callback to reset the PCM instance
to a sane initial state.

The hw_params constraints can be defined via either
#snd_pcm_extplug_set_param_minmax() and #snd_pcm_extplug_set_param_list()
functions after calling #snd_pcm_extplug_create().
The former defines the minimal and maximal acceptable values for the
given hw_params parameter (SND_PCM_EXTPLUG_HW_XXX).
This function can't be used for the format parameter.  The latter
function specifies the available parameter values as the list.
As mentioned above, the rate can't be changed.  Only changeable
parameters are sample format and channels.

To define the constraints of the slave PCM configuration, use
either #snd_pcm_extplug_set_slave_param_minmax() and
#snd_pcm_extplug_set_slave_param_list().  The arguments are as same
as former functions.

To clear the parameter constraints, call #snd_pcm_extplug_params_reset()
function. 

*/

/**
 * \brief Create an extplug instance
 * \param extplug the extplug handle
 * \param name name of the PCM
 * \param root configuration tree root
 * \param slave_conf slave configuration root
 * \param stream stream direction
 * \param mode PCM open mode
 * \return 0 if successful, or a negative error code
 *
 * Creates the extplug instance based on the given handle.
 * The slave_conf argument is mandatory, and usually taken from the config tree of the
 * PCM plugin as "slave" config value.
 * name, root, stream and mode arguments are the values used for opening the PCM.
 *
 * The callback is the mandatory field of extplug handle.  At least, start, stop and
 * pointer callbacks must be set before calling this function.
 */
int snd_pcm_extplug_create(snd_pcm_extplug_t *extplug, const char *name,
			   snd_config_t *root, snd_config_t *slave_conf,
			   snd_pcm_stream_t stream, int mode)
{
	extplug_priv_t *ext;
	int err;
	snd_pcm_t *spcm, *pcm;
	snd_config_t *sconf;

	assert(root);
	assert(extplug && extplug->callback);
	assert(extplug->callback->transfer);
	assert(slave_conf);

	if (extplug->version != SND_PCM_EXTPLUG_VERSION) {
		SNDERR("extplug: Plugin version mismatch\n");
		return -ENXIO;
	}

	err = snd_pcm_slave_conf(root, slave_conf, &sconf, 0);
	if (err < 0)
		return err;
	err = snd_pcm_open_slave(&spcm, root, sconf, stream, mode, NULL);
	snd_config_delete(sconf);
	if (err < 0)
		return err;

	ext = calloc(1, sizeof(*ext));
	if (! ext)
		return -ENOMEM;

	ext->data = extplug;
	extplug->stream = stream;

	snd_pcm_plugin_init(&ext->plug);
	ext->plug.read = snd_pcm_extplug_read_areas;
	ext->plug.write = snd_pcm_extplug_write_areas;
	ext->plug.undo_read = snd_pcm_plugin_undo_read_generic;
	ext->plug.undo_write = snd_pcm_plugin_undo_write_generic;
	ext->plug.gen.slave = spcm;
	ext->plug.gen.close_slave = 1;
	if (extplug->callback->init)
		ext->plug.init = snd_pcm_extplug_init;

	err = snd_pcm_new(&pcm, SND_PCM_TYPE_EXTPLUG, name, stream, mode);
	if (err < 0) {
		free(ext);
		return err;
	}

	extplug->pcm = pcm;
	pcm->ops = &snd_pcm_extplug_ops;
	pcm->fast_ops = &snd_pcm_plugin_fast_ops;
	pcm->private_data = ext;
	pcm->poll_fd = spcm->poll_fd;
	pcm->poll_events = spcm->poll_events;
	snd_pcm_set_hw_ptr(pcm, &ext->plug.hw_ptr, -1, 0);
	snd_pcm_set_appl_ptr(pcm, &ext->plug.appl_ptr, -1, 0);

	return 0;
}

/**
 * \brief Delete the extplug instance
 * \param extplug the extplug handle to delete
 * \return 0 if successful, or a negative error code
 *
 * The destructor of extplug instance.
 * Closes the PCM and deletes the associated resources.
 */
int snd_pcm_extplug_delete(snd_pcm_extplug_t *extplug)
{
	return snd_pcm_close(extplug->pcm);
}


/**
 * \brief Reset extplug parameters
 * \param extplug the extplug handle
 *
 * Resets the all parameters for the given extplug handle.
 */
void snd_pcm_extplug_params_reset(snd_pcm_extplug_t *extplug)
{
	extplug_priv_t *ext = extplug->pcm->private_data;
	clear_ext_params(ext);
}

/**
 * \brief Set slave parameter as the list
 * \param extplug the extplug handle
 * \param type parameter type
 * \param num_list number of available values
 * \param list the list of available values
 * \return 0 if successful, or a negative error code
 *
 * Sets the slave parameter as the list.
 * The available values of the given parameter type of the slave PCM is restricted
 * to the ones of the given list.
 */
int snd_pcm_extplug_set_slave_param_list(snd_pcm_extplug_t *extplug, int type, unsigned int num_list, const unsigned int *list)
{
	extplug_priv_t *ext = extplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_EXTPLUG_HW_PARAMS) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	return snd_ext_parm_set_list(&ext->sparams[type], num_list, list);
}

/**
 * \brief Set slave parameter as the min/max values
 * \param extplug the extplug handle
 * \param type parameter type
 * \param min the minimum value
 * \param max the maximum value
 * \return 0 if successful, or a negative error code
 *
 * Sets the slave parameter as the min/max values.
 * The available values of the given parameter type of the slave PCM is restricted
 * between the given minimum and maximum values.
 */
int snd_pcm_extplug_set_slave_param_minmax(snd_pcm_extplug_t *extplug, int type, unsigned int min, unsigned int max)
{
	extplug_priv_t *ext = extplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_EXTPLUG_HW_PARAMS) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	if (is_mask_type(type)) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	return snd_ext_parm_set_minmax(&ext->sparams[type], min, max);
}

/**
 * \brief Set master parameter as the list
 * \param extplug the extplug handle
 * \param type parameter type
 * \param num_list number of available values
 * \param list the list of available values
 * \return 0 if successful, or a negative error code
 *
 * Sets the master parameter as the list.
 * The available values of the given parameter type of this PCM (as input) is restricted
 * to the ones of the given list.
 */
int snd_pcm_extplug_set_param_list(snd_pcm_extplug_t *extplug, int type, unsigned int num_list, const unsigned int *list)
{
	extplug_priv_t *ext = extplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_EXTPLUG_HW_PARAMS) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	return snd_ext_parm_set_list(&ext->params[type], num_list, list);
}

/**
 * \brief Set master parameter as the min/max values
 * \param extplug the extplug handle
 * \param type parameter type
 * \param min the minimum value
 * \param max the maximum value
 * \return 0 if successful, or a negative error code
 *
 * Sets the master parameter as the min/max values.
 * The available values of the given parameter type of this PCM (as input) is restricted
 * between the given minimum and maximum values.
 */
int snd_pcm_extplug_set_param_minmax(snd_pcm_extplug_t *extplug, int type, unsigned int min, unsigned int max)
{
	extplug_priv_t *ext = extplug->pcm->private_data;
	if (type < 0 && type >= SND_PCM_EXTPLUG_HW_PARAMS) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	if (is_mask_type(type)) {
		SNDERR("EXTPLUG: invalid parameter type %d", type);
		return -EINVAL;
	}
	return snd_ext_parm_set_minmax(&ext->params[type], min, max);
}
