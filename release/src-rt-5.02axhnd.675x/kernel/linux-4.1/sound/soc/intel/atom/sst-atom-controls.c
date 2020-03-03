/*
 *  sst-atom-controls.c - Intel MID Platform driver DPCM ALSA controls for Mrfld
 *
 *  Copyright (C) 2013-14 Intel Corp
 *  Author: Omair Mohammed Abdullah <omair.m.abdullah@intel.com>
 *	Vinod Koul <vinod.koul@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  In the dpcm driver modelling when a particular FE/BE/Mixer/Pipe is active
 *  we forward the settings and parameters, rest we keep the values  in
 *  driver and forward when DAPM enables them
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include "sst-mfld-platform.h"
#include "sst-atom-controls.h"

static int sst_fill_byte_control(struct sst_data *drv,
					 u8 ipc_msg, u8 block,
					 u8 task_id, u8 pipe_id,
					 u16 len, void *cmd_data)
{
	struct snd_sst_bytes_v2 *byte_data = drv->byte_stream;

	byte_data->type = SST_CMD_BYTES_SET;
	byte_data->ipc_msg = ipc_msg;
	byte_data->block = block;
	byte_data->task_id = task_id;
	byte_data->pipe_id = pipe_id;

	if (len > SST_MAX_BIN_BYTES - sizeof(*byte_data)) {
		dev_err(&drv->pdev->dev, "command length too big (%u)", len);
		return -EINVAL;
	}
	byte_data->len = len;
	memcpy(byte_data->bytes, cmd_data, len);
	print_hex_dump_bytes("writing to lpe: ", DUMP_PREFIX_OFFSET,
			     byte_data, len + sizeof(*byte_data));
	return 0;
}

static int sst_fill_and_send_cmd_unlocked(struct sst_data *drv,
				 u8 ipc_msg, u8 block, u8 task_id, u8 pipe_id,
				 void *cmd_data, u16 len)
{
	int ret = 0;

	ret = sst_fill_byte_control(drv, ipc_msg,
				block, task_id, pipe_id, len, cmd_data);
	if (ret < 0)
		return ret;
	return sst->ops->send_byte_stream(sst->dev, drv->byte_stream);
}

/**
 * sst_fill_and_send_cmd - generate the IPC message and send it to the FW
 * @ipc_msg:	type of IPC (CMD, SET_PARAMS, GET_PARAMS)
 * @cmd_data:	the IPC payload
 */
static int sst_fill_and_send_cmd(struct sst_data *drv,
				 u8 ipc_msg, u8 block, u8 task_id, u8 pipe_id,
				 void *cmd_data, u16 len)
{
	int ret;

	mutex_lock(&drv->lock);
	ret = sst_fill_and_send_cmd_unlocked(drv, ipc_msg, block,
					task_id, pipe_id, cmd_data, len);
	mutex_unlock(&drv->lock);

	return ret;
}

/**
 * tx map value is a bitfield where each bit represents a FW channel
 *
 *			3 2 1 0		# 0 = codec0, 1 = codec1
 *			RLRLRLRL	# 3, 4 = reserved
 *
 * e.g. slot 0 rx map =	00001100b -> data from slot 0 goes into codec_in1 L,R
 */
static u8 sst_ssp_tx_map[SST_MAX_TDM_SLOTS] = {
	0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, /* default rx map */
};

/**
 * rx map value is a bitfield where each bit represents a slot
 *
 *			  76543210	# 0 = slot 0, 1 = slot 1
 *
 * e.g. codec1_0 tx map = 00000101b -> data from codec_out1_0 goes into slot 0, 2
 */
static u8 sst_ssp_rx_map[SST_MAX_TDM_SLOTS] = {
	0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, /* default tx map */
};

/**
 * NOTE: this is invoked with lock held
 */
static int sst_send_slot_map(struct sst_data *drv)
{
	struct sst_param_sba_ssp_slot_map cmd;

	SST_FILL_DEFAULT_DESTINATION(cmd.header.dst);
	cmd.header.command_id = SBA_SET_SSP_SLOT_MAP;
	cmd.header.length = sizeof(struct sst_param_sba_ssp_slot_map)
				- sizeof(struct sst_dsp_header);

	cmd.param_id = SBA_SET_SSP_SLOT_MAP;
	cmd.param_len = sizeof(cmd.rx_slot_map) + sizeof(cmd.tx_slot_map)
					+ sizeof(cmd.ssp_index);
	cmd.ssp_index = SSP_CODEC;

	memcpy(cmd.rx_slot_map, &sst_ssp_tx_map[0], sizeof(cmd.rx_slot_map));
	memcpy(cmd.tx_slot_map, &sst_ssp_rx_map[0], sizeof(cmd.tx_slot_map));

	return sst_fill_and_send_cmd_unlocked(drv, SST_IPC_IA_SET_PARAMS,
			SST_FLAG_BLOCKED, SST_TASK_SBA, 0, &cmd,
			      sizeof(cmd.header) + cmd.header.length);
}

int sst_slot_enum_info(struct snd_kcontrol *kcontrol,
		       struct snd_ctl_elem_info *uinfo)
{
	struct sst_enum *e = (struct sst_enum *)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = 1;
	uinfo->value.enumerated.items = e->max;

	if (uinfo->value.enumerated.item > e->max - 1)
		uinfo->value.enumerated.item = e->max - 1;
	strcpy(uinfo->value.enumerated.name,
		e->texts[uinfo->value.enumerated.item]);

	return 0;
}

/**
 * sst_slot_get - get the status of the interleaver/deinterleaver control
 *
 * Searches the map where the control status is stored, and gets the
 * channel/slot which is currently set for this enumerated control. Since it is
 * an enumerated control, there is only one possible value.
 */
static int sst_slot_get(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct sst_enum *e = (void *)kcontrol->private_value;
	struct snd_soc_component *c = snd_kcontrol_chip(kcontrol);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);
	unsigned int ctl_no = e->reg;
	unsigned int is_tx = e->tx;
	unsigned int val, mux;
	u8 *map = is_tx ? sst_ssp_rx_map : sst_ssp_tx_map;

	mutex_lock(&drv->lock);
	val = 1 << ctl_no;
	/* search which slot/channel has this bit set - there should be only one */
	for (mux = e->max; mux > 0;  mux--)
		if (map[mux - 1] & val)
			break;

	ucontrol->value.enumerated.item[0] = mux;
	mutex_unlock(&drv->lock);

	dev_dbg(c->dev, "%s - %s map = %#x\n",
			is_tx ? "tx channel" : "rx slot",
			 e->texts[mux], mux ? map[mux - 1] : -1);
	return 0;
}

/* sst_check_and_send_slot_map - helper for checking power state and sending
 * slot map cmd
 *
 * called with lock held
 */
static int sst_check_and_send_slot_map(struct sst_data *drv, struct snd_kcontrol *kcontrol)
{
	struct sst_enum *e = (void *)kcontrol->private_value;
	int ret = 0;

	if (e->w && e->w->power)
		ret = sst_send_slot_map(drv);
	else
		dev_err(&drv->pdev->dev, "Slot control: %s doesn't have DAPM widget!!!\n",
				kcontrol->id.name);
	return ret;
}

/**
 * sst_slot_put - set the status of interleaver/deinterleaver control
 *
 * (de)interleaver controls are defined in opposite sense to be user-friendly
 *
 * Instead of the enum value being the value written to the register, it is the
 * register address; and the kcontrol number (register num) is the value written
 * to the register. This is so that there can be only one value for each
 * slot/channel since there is only one control for each slot/channel.
 *
 * This means that whenever an enum is set, we need to clear the bit
 * for that kcontrol_no for all the interleaver OR deinterleaver registers
 */
static int sst_slot_put(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *c = snd_soc_kcontrol_component(kcontrol);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);
	struct sst_enum *e = (void *)kcontrol->private_value;
	int i, ret = 0;
	unsigned int ctl_no = e->reg;
	unsigned int is_tx = e->tx;
	unsigned int slot_channel_no;
	unsigned int val, mux;
	u8 *map;

	map = is_tx ? sst_ssp_rx_map : sst_ssp_tx_map;

	val = 1 << ctl_no;
	mux = ucontrol->value.enumerated.item[0];
	if (mux > e->max - 1)
		return -EINVAL;

	mutex_lock(&drv->lock);
	/* first clear all registers of this bit */
	for (i = 0; i < e->max; i++)
		map[i] &= ~val;

	if (mux == 0) {
		/* kctl set to 'none' and we reset the bits so send IPC */
		ret = sst_check_and_send_slot_map(drv, kcontrol);

		mutex_unlock(&drv->lock);
		return ret;
	}

	/* offset by one to take "None" into account */
	slot_channel_no = mux - 1;
	map[slot_channel_no] |= val;

	dev_dbg(c->dev, "%s %s map = %#x\n",
			is_tx ? "tx channel" : "rx slot",
			e->texts[mux], map[slot_channel_no]);

	ret = sst_check_and_send_slot_map(drv, kcontrol);

	mutex_unlock(&drv->lock);
	return ret;
}

static int sst_send_algo_cmd(struct sst_data *drv,
			      struct sst_algo_control *bc)
{
	int len, ret = 0;
	struct sst_cmd_set_params *cmd;

	/*bc->max includes sizeof algos + length field*/
	len = sizeof(cmd->dst) + sizeof(cmd->command_id) + bc->max;

	cmd = kzalloc(len, GFP_KERNEL);
	if (cmd == NULL)
		return -ENOMEM;

	SST_FILL_DESTINATION(2, cmd->dst, bc->pipe_id, bc->module_id);
	cmd->command_id = bc->cmd_id;
	memcpy(cmd->params, bc->params, bc->max);

	ret = sst_fill_and_send_cmd_unlocked(drv, SST_IPC_IA_SET_PARAMS,
				SST_FLAG_BLOCKED, bc->task_id, 0, cmd, len);
	kfree(cmd);
	return ret;
}

/**
 * sst_find_and_send_pipe_algo - send all the algo parameters for a pipe
 *
 * The algos which are in each pipeline are sent to the firmware one by one
 *
 * Called with lock held
 */
static int sst_find_and_send_pipe_algo(struct sst_data *drv,
					const char *pipe, struct sst_ids *ids)
{
	int ret = 0;
	struct sst_algo_control *bc;
	struct sst_module *algo = NULL;

	dev_dbg(&drv->pdev->dev, "Enter: widget=%s\n", pipe);

	list_for_each_entry(algo, &ids->algo_list, node) {
		bc = (void *)algo->kctl->private_value;

		dev_dbg(&drv->pdev->dev, "Found algo control name=%s pipe=%s\n",
				algo->kctl->id.name, pipe);
		ret = sst_send_algo_cmd(drv, bc);
		if (ret)
			return ret;
	}
	return ret;
}

static int sst_algo_bytes_ctl_info(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_info *uinfo)
{
	struct sst_algo_control *bc = (void *)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
	uinfo->count = bc->max;

	return 0;
}

static int sst_algo_control_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct sst_algo_control *bc = (void *)kcontrol->private_value;
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);

	switch (bc->type) {
	case SST_ALGO_PARAMS:
		memcpy(ucontrol->value.bytes.data, bc->params, bc->max);
		break;
	default:
		dev_err(component->dev, "Invalid Input- algo type:%d\n",
				bc->type);
		return -EINVAL;

	}
	return 0;
}

static int sst_algo_control_set(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_component *cmpnt = snd_soc_kcontrol_component(kcontrol);
	struct sst_data *drv = snd_soc_component_get_drvdata(cmpnt);
	struct sst_algo_control *bc = (void *)kcontrol->private_value;

	dev_dbg(cmpnt->dev, "control_name=%s\n", kcontrol->id.name);
	mutex_lock(&drv->lock);
	switch (bc->type) {
	case SST_ALGO_PARAMS:
		memcpy(bc->params, ucontrol->value.bytes.data, bc->max);
		break;
	default:
		mutex_unlock(&drv->lock);
		dev_err(cmpnt->dev, "Invalid Input- algo type:%d\n",
				bc->type);
		return -EINVAL;
	}
	/*if pipe is enabled, need to send the algo params from here*/
	if (bc->w && bc->w->power)
		ret = sst_send_algo_cmd(drv, bc);
	mutex_unlock(&drv->lock);

	return ret;
}

static int sst_gain_ctl_info(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct sst_gain_mixer_control *mc = (void *)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = mc->stereo ? 2 : 1;
	uinfo->value.integer.min = mc->min;
	uinfo->value.integer.max = mc->max;

	return 0;
}

/**
 * sst_send_gain_cmd - send the gain algorithm IPC to the FW
 * @gv:		the stored value of gain (also contains rampduration)
 * @mute:	flag that indicates whether this was called from the
 *		digital_mute callback or directly. If called from the
 *		digital_mute callback, module will be muted/unmuted based on this
 *		flag. The flag is always 0 if called directly.
 *
 * Called with sst_data.lock held
 *
 * The user-set gain value is sent only if the user-controllable 'mute' control
 * is OFF (indicated by gv->mute). Otherwise, the mute value (MIN value) is
 * sent.
 */
static int sst_send_gain_cmd(struct sst_data *drv, struct sst_gain_value *gv,
			      u16 task_id, u16 loc_id, u16 module_id, int mute)
{
	struct sst_cmd_set_gain_dual cmd;

	dev_dbg(&drv->pdev->dev, "Enter\n");

	cmd.header.command_id = MMX_SET_GAIN;
	SST_FILL_DEFAULT_DESTINATION(cmd.header.dst);
	cmd.gain_cell_num = 1;

	if (mute || gv->mute) {
		cmd.cell_gains[0].cell_gain_left = SST_GAIN_MIN_VALUE;
		cmd.cell_gains[0].cell_gain_right = SST_GAIN_MIN_VALUE;
	} else {
		cmd.cell_gains[0].cell_gain_left = gv->l_gain;
		cmd.cell_gains[0].cell_gain_right = gv->r_gain;
	}

	SST_FILL_DESTINATION(2, cmd.cell_gains[0].dest,
			     loc_id, module_id);
	cmd.cell_gains[0].gain_time_constant = gv->ramp_duration;

	cmd.header.length = sizeof(struct sst_cmd_set_gain_dual)
				- sizeof(struct sst_dsp_header);

	/* we are with lock held, so call the unlocked api  to send */
	return sst_fill_and_send_cmd_unlocked(drv, SST_IPC_IA_SET_PARAMS,
				SST_FLAG_BLOCKED, task_id, 0, &cmd,
			      sizeof(cmd.header) + cmd.header.length);
}

static int sst_gain_get(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct sst_gain_mixer_control *mc = (void *)kcontrol->private_value;
	struct sst_gain_value *gv = mc->gain_val;

	switch (mc->type) {
	case SST_GAIN_TLV:
		ucontrol->value.integer.value[0] = gv->l_gain;
		ucontrol->value.integer.value[1] = gv->r_gain;
		break;

	case SST_GAIN_MUTE:
		ucontrol->value.integer.value[0] = gv->mute ? 1 : 0;
		break;

	case SST_GAIN_RAMP_DURATION:
		ucontrol->value.integer.value[0] = gv->ramp_duration;
		break;

	default:
		dev_err(component->dev, "Invalid Input- gain type:%d\n",
				mc->type);
		return -EINVAL;
	}

	return 0;
}

static int sst_gain_put(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct snd_soc_component *cmpnt = snd_soc_kcontrol_component(kcontrol);
	struct sst_data *drv = snd_soc_component_get_drvdata(cmpnt);
	struct sst_gain_mixer_control *mc = (void *)kcontrol->private_value;
	struct sst_gain_value *gv = mc->gain_val;

	mutex_lock(&drv->lock);

	switch (mc->type) {
	case SST_GAIN_TLV:
		gv->l_gain = ucontrol->value.integer.value[0];
		gv->r_gain = ucontrol->value.integer.value[1];
		dev_dbg(cmpnt->dev, "%s: Volume %d, %d\n",
				mc->pname, gv->l_gain, gv->r_gain);
		break;

	case SST_GAIN_MUTE:
		gv->mute = !!ucontrol->value.integer.value[0];
		dev_dbg(cmpnt->dev, "%s: Mute %d\n", mc->pname, gv->mute);
		break;

	case SST_GAIN_RAMP_DURATION:
		gv->ramp_duration = ucontrol->value.integer.value[0];
		dev_dbg(cmpnt->dev, "%s: Ramp Delay%d\n",
					mc->pname, gv->ramp_duration);
		break;

	default:
		mutex_unlock(&drv->lock);
		dev_err(cmpnt->dev, "Invalid Input- gain type:%d\n",
				mc->type);
		return -EINVAL;
	}

	if (mc->w && mc->w->power)
		ret = sst_send_gain_cmd(drv, gv, mc->task_id,
			mc->pipe_id | mc->instance_id, mc->module_id, 0);
	mutex_unlock(&drv->lock);

	return ret;
}

static int sst_set_pipe_gain(struct sst_ids *ids,
				struct sst_data *drv, int mute);

static int sst_send_pipe_module_params(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol)
{
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);
	struct sst_ids *ids = w->priv;

	mutex_lock(&drv->lock);
	sst_find_and_send_pipe_algo(drv, w->name, ids);
	sst_set_pipe_gain(ids, drv, 0);
	mutex_unlock(&drv->lock);

	return 0;
}

static int sst_generic_modules_event(struct snd_soc_dapm_widget *w,
				     struct snd_kcontrol *k, int event)
{
	if (SND_SOC_DAPM_EVENT_ON(event))
		return sst_send_pipe_module_params(w, k);
	return 0;
}

static const DECLARE_TLV_DB_SCALE(sst_gain_tlv_common, SST_GAIN_MIN_VALUE * 10, 10, 0);

/* Look up table to convert MIXER SW bit regs to SWM inputs */
static const uint swm_mixer_input_ids[SST_SWM_INPUT_COUNT] = {
	[SST_IP_CODEC0]		= SST_SWM_IN_CODEC0,
	[SST_IP_CODEC1]		= SST_SWM_IN_CODEC1,
	[SST_IP_LOOP0]		= SST_SWM_IN_SPROT_LOOP,
	[SST_IP_LOOP1]		= SST_SWM_IN_MEDIA_LOOP1,
	[SST_IP_LOOP2]		= SST_SWM_IN_MEDIA_LOOP2,
	[SST_IP_PCM0]		= SST_SWM_IN_PCM0,
	[SST_IP_PCM1]		= SST_SWM_IN_PCM1,
	[SST_IP_MEDIA0]		= SST_SWM_IN_MEDIA0,
	[SST_IP_MEDIA1]		= SST_SWM_IN_MEDIA1,
	[SST_IP_MEDIA2]		= SST_SWM_IN_MEDIA2,
	[SST_IP_MEDIA3]		= SST_SWM_IN_MEDIA3,
};

/**
 * fill_swm_input - fill in the SWM input ids given the register
 *
 * The register value is a bit-field inicated which mixer inputs are ON. Use the
 * lookup table to get the input-id and fill it in the structure.
 */
static int fill_swm_input(struct snd_soc_component *cmpnt,
		struct swm_input_ids *swm_input, unsigned int reg)
{
	uint i, is_set, nb_inputs = 0;
	u16 input_loc_id;

	dev_dbg(cmpnt->dev, "reg: %#x\n", reg);
	for (i = 0; i < SST_SWM_INPUT_COUNT; i++) {
		is_set = reg & BIT(i);
		if (!is_set)
			continue;

		input_loc_id = swm_mixer_input_ids[i];
		SST_FILL_DESTINATION(2, swm_input->input_id,
				     input_loc_id, SST_DEFAULT_MODULE_ID);
		nb_inputs++;
		swm_input++;
		dev_dbg(cmpnt->dev, "input id: %#x, nb_inputs: %d\n",
				input_loc_id, nb_inputs);

		if (nb_inputs == SST_CMD_SWM_MAX_INPUTS) {
			dev_warn(cmpnt->dev, "SET_SWM cmd max inputs reached");
			break;
		}
	}
	return nb_inputs;
}


/**
 * called with lock held
 */
static int sst_set_pipe_gain(struct sst_ids *ids,
			struct sst_data *drv, int mute)
{
	int ret = 0;
	struct sst_gain_mixer_control *mc;
	struct sst_gain_value *gv;
	struct sst_module *gain = NULL;

	list_for_each_entry(gain, &ids->gain_list, node) {
		struct snd_kcontrol *kctl = gain->kctl;

		dev_dbg(&drv->pdev->dev, "control name=%s\n", kctl->id.name);
		mc = (void *)kctl->private_value;
		gv = mc->gain_val;

		ret = sst_send_gain_cmd(drv, gv, mc->task_id,
			mc->pipe_id | mc->instance_id, mc->module_id, mute);
		if (ret)
			return ret;
	}
	return ret;
}

static int sst_swm_mixer_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k, int event)
{
	struct sst_cmd_set_swm cmd;
	struct snd_soc_component *cmpnt = snd_soc_dapm_to_component(w->dapm);
	struct sst_data *drv = snd_soc_component_get_drvdata(cmpnt);
	struct sst_ids *ids = w->priv;
	bool set_mixer = false;
	struct soc_mixer_control *mc;
	int val = 0;
	int i = 0;

	dev_dbg(cmpnt->dev, "widget = %s\n", w->name);
	/*
	 * Identify which mixer input is on and send the bitmap of the
	 * inputs as an IPC to the DSP.
	 */
	for (i = 0; i < w->num_kcontrols; i++) {
		if (dapm_kcontrol_get_value(w->kcontrols[i])) {
			mc = (struct soc_mixer_control *)(w->kcontrols[i])->private_value;
			val |= 1 << mc->shift;
		}
	}
	dev_dbg(cmpnt->dev, "val = %#x\n", val);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
	case SND_SOC_DAPM_POST_PMD:
		set_mixer = true;
		break;
	case SND_SOC_DAPM_POST_REG:
		if (w->power)
			set_mixer = true;
		break;
	default:
		set_mixer = false;
	}

	if (set_mixer == false)
		return 0;

	if (SND_SOC_DAPM_EVENT_ON(event) ||
	    event == SND_SOC_DAPM_POST_REG)
		cmd.switch_state = SST_SWM_ON;
	else
		cmd.switch_state = SST_SWM_OFF;

	SST_FILL_DEFAULT_DESTINATION(cmd.header.dst);
	/* MMX_SET_SWM == SBA_SET_SWM */
	cmd.header.command_id = SBA_SET_SWM;

	SST_FILL_DESTINATION(2, cmd.output_id,
			     ids->location_id, SST_DEFAULT_MODULE_ID);
	cmd.nb_inputs =	fill_swm_input(cmpnt, &cmd.input[0], val);
	cmd.header.length = offsetof(struct sst_cmd_set_swm, input)
				- sizeof(struct sst_dsp_header)
				+ (cmd.nb_inputs * sizeof(cmd.input[0]));

	return sst_fill_and_send_cmd(drv, SST_IPC_IA_CMD, SST_FLAG_BLOCKED,
			      ids->task_id, 0, &cmd,
			      sizeof(cmd.header) + cmd.header.length);
}

/* SBA mixers - 16 inputs */
#define SST_SBA_DECLARE_MIX_CONTROLS(kctl_name)							\
	static const struct snd_kcontrol_new kctl_name[] = {					\
		SOC_DAPM_SINGLE("codec_in0 Switch", SND_SOC_NOPM, SST_IP_CODEC0, 1, 0),		\
		SOC_DAPM_SINGLE("codec_in1 Switch", SND_SOC_NOPM, SST_IP_CODEC1, 1, 0),		\
		SOC_DAPM_SINGLE("sprot_loop_in Switch", SND_SOC_NOPM, SST_IP_LOOP0, 1, 0),	\
		SOC_DAPM_SINGLE("media_loop1_in Switch", SND_SOC_NOPM, SST_IP_LOOP1, 1, 0),	\
		SOC_DAPM_SINGLE("media_loop2_in Switch", SND_SOC_NOPM, SST_IP_LOOP2, 1, 0),	\
		SOC_DAPM_SINGLE("pcm0_in Switch", SND_SOC_NOPM, SST_IP_PCM0, 1, 0),		\
		SOC_DAPM_SINGLE("pcm1_in Switch", SND_SOC_NOPM, SST_IP_PCM1, 1, 0),		\
	}

#define SST_SBA_MIXER_GRAPH_MAP(mix_name)			\
	{ mix_name, "codec_in0 Switch",	"codec_in0" },		\
	{ mix_name, "codec_in1 Switch",	"codec_in1" },		\
	{ mix_name, "sprot_loop_in Switch",	"sprot_loop_in" },	\
	{ mix_name, "media_loop1_in Switch",	"media_loop1_in" },	\
	{ mix_name, "media_loop2_in Switch",	"media_loop2_in" },	\
	{ mix_name, "pcm0_in Switch",		"pcm0_in" },		\
	{ mix_name, "pcm1_in Switch",		"pcm1_in" }

#define SST_MMX_DECLARE_MIX_CONTROLS(kctl_name)						\
	static const struct snd_kcontrol_new kctl_name[] = {				\
		SOC_DAPM_SINGLE("media0_in Switch", SND_SOC_NOPM, SST_IP_MEDIA0, 1, 0),	\
		SOC_DAPM_SINGLE("media1_in Switch", SND_SOC_NOPM, SST_IP_MEDIA1, 1, 0),	\
		SOC_DAPM_SINGLE("media2_in Switch", SND_SOC_NOPM, SST_IP_MEDIA2, 1, 0),	\
		SOC_DAPM_SINGLE("media3_in Switch", SND_SOC_NOPM, SST_IP_MEDIA3, 1, 0),	\
	}

SST_MMX_DECLARE_MIX_CONTROLS(sst_mix_media0_controls);
SST_MMX_DECLARE_MIX_CONTROLS(sst_mix_media1_controls);

/* 18 SBA mixers */
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_pcm0_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_pcm1_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_pcm2_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_sprot_l0_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_media_l1_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_media_l2_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_voip_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_codec0_controls);
SST_SBA_DECLARE_MIX_CONTROLS(sst_mix_codec1_controls);

/*
 * sst_handle_vb_timer - Start/Stop the DSP scheduler
 *
 * The DSP expects first cmd to be SBA_VB_START, so at first startup send
 * that.
 * DSP expects last cmd to be SBA_VB_IDLE, so at last shutdown send that.
 *
 * Do refcount internally so that we send command only at first start
 * and last end. Since SST driver does its own ref count, invoke sst's
 * power ops always!
 */
int sst_handle_vb_timer(struct snd_soc_dai *dai, bool enable)
{
	int ret = 0;
	struct sst_cmd_generic cmd;
	struct sst_data *drv = snd_soc_dai_get_drvdata(dai);
	static int timer_usage;

	if (enable)
		cmd.header.command_id = SBA_VB_START;
	else
		cmd.header.command_id = SBA_IDLE;
	dev_dbg(dai->dev, "enable=%u, usage=%d\n", enable, timer_usage);

	SST_FILL_DEFAULT_DESTINATION(cmd.header.dst);
	cmd.header.length = 0;

	if (enable) {
		ret = sst->ops->power(sst->dev, true);
		if (ret < 0)
			return ret;
	}

	mutex_lock(&drv->lock);
	if (enable)
		timer_usage++;
	else
		timer_usage--;

	/*
	 * Send the command only if this call is the first enable or last
	 * disable
	 */
	if ((enable && (timer_usage == 1)) ||
	    (!enable && (timer_usage == 0))) {
		ret = sst_fill_and_send_cmd_unlocked(drv, SST_IPC_IA_CMD,
				SST_FLAG_BLOCKED, SST_TASK_SBA, 0, &cmd,
				sizeof(cmd.header) + cmd.header.length);
		if (ret && enable) {
			timer_usage--;
			enable  = false;
		}
	}
	mutex_unlock(&drv->lock);

	if (!enable)
		sst->ops->power(sst->dev, false);
	return ret;
}

/**
 * sst_ssp_config - contains SSP configuration for media UC
 */
static const struct sst_ssp_config sst_ssp_configs = {
	.ssp_id = SSP_CODEC,
	.bits_per_slot = 24,
	.slots = 4,
	.ssp_mode = SSP_MODE_MASTER,
	.pcm_mode = SSP_PCM_MODE_NETWORK,
	.duplex = SSP_DUPLEX,
	.ssp_protocol = SSP_MODE_PCM,
	.fs_width = 1,
	.fs_frequency = SSP_FS_48_KHZ,
	.active_slot_map = 0xF,
	.start_delay = 0,
};

int send_ssp_cmd(struct snd_soc_dai *dai, const char *id, bool enable)
{
	struct sst_cmd_sba_hw_set_ssp cmd;
	struct sst_data *drv = snd_soc_dai_get_drvdata(dai);
	const struct sst_ssp_config *config;

	dev_info(dai->dev, "Enter: enable=%d port_name=%s\n", enable, id);

	SST_FILL_DEFAULT_DESTINATION(cmd.header.dst);
	cmd.header.command_id = SBA_HW_SET_SSP;
	cmd.header.length = sizeof(struct sst_cmd_sba_hw_set_ssp)
				- sizeof(struct sst_dsp_header);

	config = &sst_ssp_configs;
	dev_dbg(dai->dev, "ssp_id: %u\n", config->ssp_id);

	if (enable)
		cmd.switch_state = SST_SWITCH_ON;
	else
		cmd.switch_state = SST_SWITCH_OFF;

	cmd.selection = config->ssp_id;
	cmd.nb_bits_per_slots = config->bits_per_slot;
	cmd.nb_slots = config->slots;
	cmd.mode = config->ssp_mode | (config->pcm_mode << 1);
	cmd.duplex = config->duplex;
	cmd.active_tx_slot_map = config->active_slot_map;
	cmd.active_rx_slot_map = config->active_slot_map;
	cmd.frame_sync_frequency = config->fs_frequency;
	cmd.frame_sync_polarity = SSP_FS_ACTIVE_HIGH;
	cmd.data_polarity = 1;
	cmd.frame_sync_width = config->fs_width;
	cmd.ssp_protocol = config->ssp_protocol;
	cmd.start_delay = config->start_delay;
	cmd.reserved1 = cmd.reserved2 = 0xFF;

	return sst_fill_and_send_cmd(drv, SST_IPC_IA_CMD, SST_FLAG_BLOCKED,
				SST_TASK_SBA, 0, &cmd,
				sizeof(cmd.header) + cmd.header.length);
}

static int sst_set_be_modules(struct snd_soc_dapm_widget *w,
			 struct snd_kcontrol *k, int event)
{
	int ret = 0;
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);

	dev_dbg(c->dev, "Enter: widget=%s\n", w->name);

	if (SND_SOC_DAPM_EVENT_ON(event)) {
		ret = sst_send_slot_map(drv);
		if (ret)
			return ret;
		ret = sst_send_pipe_module_params(w, k);
	}
	return ret;
}

static int sst_set_media_path(struct snd_soc_dapm_widget *w,
			      struct snd_kcontrol *k, int event)
{
	int ret = 0;
	struct sst_cmd_set_media_path cmd;
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);
	struct sst_ids *ids = w->priv;

	dev_dbg(c->dev, "widget=%s\n", w->name);
	dev_dbg(c->dev, "task=%u, location=%#x\n",
				ids->task_id, ids->location_id);

	if (SND_SOC_DAPM_EVENT_ON(event))
		cmd.switch_state = SST_PATH_ON;
	else
		cmd.switch_state = SST_PATH_OFF;

	SST_FILL_DESTINATION(2, cmd.header.dst,
			     ids->location_id, SST_DEFAULT_MODULE_ID);

	/* MMX_SET_MEDIA_PATH == SBA_SET_MEDIA_PATH */
	cmd.header.command_id = MMX_SET_MEDIA_PATH;
	cmd.header.length = sizeof(struct sst_cmd_set_media_path)
				- sizeof(struct sst_dsp_header);

	ret = sst_fill_and_send_cmd(drv, SST_IPC_IA_CMD, SST_FLAG_BLOCKED,
			      ids->task_id, 0, &cmd,
			      sizeof(cmd.header) + cmd.header.length);
	if (ret)
		return ret;

	if (SND_SOC_DAPM_EVENT_ON(event))
		ret = sst_send_pipe_module_params(w, k);
	return ret;
}

static int sst_set_media_loop(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k, int event)
{
	int ret = 0;
	struct sst_cmd_sba_set_media_loop_map cmd;
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct sst_data *drv = snd_soc_component_get_drvdata(c);
	struct sst_ids *ids = w->priv;

	dev_dbg(c->dev, "Enter:widget=%s\n", w->name);
	if (SND_SOC_DAPM_EVENT_ON(event))
		cmd.switch_state = SST_SWITCH_ON;
	else
		cmd.switch_state = SST_SWITCH_OFF;

	SST_FILL_DESTINATION(2, cmd.header.dst,
			     ids->location_id, SST_DEFAULT_MODULE_ID);

	cmd.header.command_id = SBA_SET_MEDIA_LOOP_MAP;
	cmd.header.length = sizeof(struct sst_cmd_sba_set_media_loop_map)
				 - sizeof(struct sst_dsp_header);
	cmd.param.part.cfg.rate = 2; /* 48khz */

	cmd.param.part.cfg.format = ids->format; /* stereo/Mono */
	cmd.param.part.cfg.s_length = 1; /* 24bit left justified */
	cmd.map = 0; /* Algo sequence: Gain - DRP - FIR - IIR */

	ret = sst_fill_and_send_cmd(drv, SST_IPC_IA_CMD, SST_FLAG_BLOCKED,
			      SST_TASK_SBA, 0, &cmd,
			      sizeof(cmd.header) + cmd.header.length);
	if (ret)
		return ret;

	if (SND_SOC_DAPM_EVENT_ON(event))
		ret = sst_send_pipe_module_params(w, k);
	return ret;
}

static const struct snd_soc_dapm_widget sst_dapm_widgets[] = {
	SST_AIF_IN("codec_in0", sst_set_be_modules),
	SST_AIF_IN("codec_in1", sst_set_be_modules),
	SST_AIF_OUT("codec_out0", sst_set_be_modules),
	SST_AIF_OUT("codec_out1", sst_set_be_modules),

	/* Media Paths */
	/* MediaX IN paths are set via ALLOC, so no SET_MEDIA_PATH command */
	SST_PATH_INPUT("media0_in", SST_TASK_MMX, SST_SWM_IN_MEDIA0, sst_generic_modules_event),
	SST_PATH_INPUT("media1_in", SST_TASK_MMX, SST_SWM_IN_MEDIA1, NULL),
	SST_PATH_INPUT("media2_in", SST_TASK_MMX, SST_SWM_IN_MEDIA2, sst_set_media_path),
	SST_PATH_INPUT("media3_in", SST_TASK_MMX, SST_SWM_IN_MEDIA3, NULL),
	SST_PATH_OUTPUT("media0_out", SST_TASK_MMX, SST_SWM_OUT_MEDIA0, sst_set_media_path),
	SST_PATH_OUTPUT("media1_out", SST_TASK_MMX, SST_SWM_OUT_MEDIA1, sst_set_media_path),

	/* SBA PCM Paths */
	SST_PATH_INPUT("pcm0_in", SST_TASK_SBA, SST_SWM_IN_PCM0, sst_set_media_path),
	SST_PATH_INPUT("pcm1_in", SST_TASK_SBA, SST_SWM_IN_PCM1, sst_set_media_path),
	SST_PATH_OUTPUT("pcm0_out", SST_TASK_SBA, SST_SWM_OUT_PCM0, sst_set_media_path),
	SST_PATH_OUTPUT("pcm1_out", SST_TASK_SBA, SST_SWM_OUT_PCM1, sst_set_media_path),
	SST_PATH_OUTPUT("pcm2_out", SST_TASK_SBA, SST_SWM_OUT_PCM2, sst_set_media_path),

	/* SBA Loops */
	SST_PATH_INPUT("sprot_loop_in", SST_TASK_SBA, SST_SWM_IN_SPROT_LOOP, NULL),
	SST_PATH_INPUT("media_loop1_in", SST_TASK_SBA, SST_SWM_IN_MEDIA_LOOP1, NULL),
	SST_PATH_INPUT("media_loop2_in", SST_TASK_SBA, SST_SWM_IN_MEDIA_LOOP2, NULL),
	SST_PATH_MEDIA_LOOP_OUTPUT("sprot_loop_out", SST_TASK_SBA, SST_SWM_OUT_SPROT_LOOP, SST_FMT_MONO, sst_set_media_loop),
	SST_PATH_MEDIA_LOOP_OUTPUT("media_loop1_out", SST_TASK_SBA, SST_SWM_OUT_MEDIA_LOOP1, SST_FMT_MONO, sst_set_media_loop),
	SST_PATH_MEDIA_LOOP_OUTPUT("media_loop2_out", SST_TASK_SBA, SST_SWM_OUT_MEDIA_LOOP2, SST_FMT_STEREO, sst_set_media_loop),

	/* Media Mixers */
	SST_SWM_MIXER("media0_out mix 0", SND_SOC_NOPM, SST_TASK_MMX, SST_SWM_OUT_MEDIA0,
		      sst_mix_media0_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("media1_out mix 0", SND_SOC_NOPM, SST_TASK_MMX, SST_SWM_OUT_MEDIA1,
		      sst_mix_media1_controls, sst_swm_mixer_event),

	/* SBA PCM mixers */
	SST_SWM_MIXER("pcm0_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_PCM0,
		      sst_mix_pcm0_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("pcm1_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_PCM1,
		      sst_mix_pcm1_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("pcm2_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_PCM2,
		      sst_mix_pcm2_controls, sst_swm_mixer_event),

	/* SBA Loop mixers */
	SST_SWM_MIXER("sprot_loop_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_SPROT_LOOP,
		      sst_mix_sprot_l0_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("media_loop1_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_MEDIA_LOOP1,
		      sst_mix_media_l1_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("media_loop2_out mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_MEDIA_LOOP2,
		      sst_mix_media_l2_controls, sst_swm_mixer_event),

	/* SBA Backend mixers */
	SST_SWM_MIXER("codec_out0 mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_CODEC0,
		      sst_mix_codec0_controls, sst_swm_mixer_event),
	SST_SWM_MIXER("codec_out1 mix 0", SND_SOC_NOPM, SST_TASK_SBA, SST_SWM_OUT_CODEC1,
		      sst_mix_codec1_controls, sst_swm_mixer_event),
};

static const struct snd_soc_dapm_route intercon[] = {
	{"media0_in", NULL, "Compress Playback"},
	{"media1_in", NULL, "Headset Playback"},
	{"media2_in", NULL, "pcm0_out"},

	{"media0_out mix 0", "media0_in Switch", "media0_in"},
	{"media0_out mix 0", "media1_in Switch", "media1_in"},
	{"media0_out mix 0", "media2_in Switch", "media2_in"},
	{"media0_out mix 0", "media3_in Switch", "media3_in"},
	{"media1_out mix 0", "media0_in Switch", "media0_in"},
	{"media1_out mix 0", "media1_in Switch", "media1_in"},
	{"media1_out mix 0", "media2_in Switch", "media2_in"},
	{"media1_out mix 0", "media3_in Switch", "media3_in"},

	{"media0_out", NULL, "media0_out mix 0"},
	{"media1_out", NULL, "media1_out mix 0"},
	{"pcm0_in", NULL, "media0_out"},
	{"pcm1_in", NULL, "media1_out"},

	{"Headset Capture", NULL, "pcm1_out"},
	{"Headset Capture", NULL, "pcm2_out"},
	{"pcm0_out", NULL, "pcm0_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("pcm0_out mix 0"),
	{"pcm1_out", NULL, "pcm1_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("pcm1_out mix 0"),
	{"pcm2_out", NULL, "pcm2_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("pcm2_out mix 0"),

	{"media_loop1_in", NULL, "media_loop1_out"},
	{"media_loop1_out", NULL, "media_loop1_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("media_loop1_out mix 0"),
	{"media_loop2_in", NULL, "media_loop2_out"},
	{"media_loop2_out", NULL, "media_loop2_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("media_loop2_out mix 0"),
	{"sprot_loop_in", NULL, "sprot_loop_out"},
	{"sprot_loop_out", NULL, "sprot_loop_out mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("sprot_loop_out mix 0"),

	{"codec_out0", NULL, "codec_out0 mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("codec_out0 mix 0"),
	{"codec_out1", NULL, "codec_out1 mix 0"},
	SST_SBA_MIXER_GRAPH_MAP("codec_out1 mix 0"),

};
static const char * const slot_names[] = {
	"none",
	"slot 0", "slot 1", "slot 2", "slot 3",
	"slot 4", "slot 5", "slot 6", "slot 7", /* not supported by FW */
};

static const char * const channel_names[] = {
	"none",
	"codec_out0_0", "codec_out0_1", "codec_out1_0", "codec_out1_1",
	"codec_out2_0", "codec_out2_1", "codec_out3_0", "codec_out3_1", /* not supported by FW */
};

#define SST_INTERLEAVER(xpname, slot_name, slotno) \
	SST_SSP_SLOT_CTL(xpname, "tx interleaver", slot_name, slotno, true, \
			 channel_names, sst_slot_get, sst_slot_put)

#define SST_DEINTERLEAVER(xpname, channel_name, channel_no) \
	SST_SSP_SLOT_CTL(xpname, "rx deinterleaver", channel_name, channel_no, false, \
			 slot_names, sst_slot_get, sst_slot_put)

static const struct snd_kcontrol_new sst_slot_controls[] = {
	SST_INTERLEAVER("codec_out", "slot 0", 0),
	SST_INTERLEAVER("codec_out", "slot 1", 1),
	SST_INTERLEAVER("codec_out", "slot 2", 2),
	SST_INTERLEAVER("codec_out", "slot 3", 3),
	SST_DEINTERLEAVER("codec_in", "codec_in0_0", 0),
	SST_DEINTERLEAVER("codec_in", "codec_in0_1", 1),
	SST_DEINTERLEAVER("codec_in", "codec_in1_0", 2),
	SST_DEINTERLEAVER("codec_in", "codec_in1_1", 3),
};

/* Gain helper with min/max set */
#define SST_GAIN(name, path_id, task_id, instance, gain_var)				\
	SST_GAIN_KCONTROLS(name, "Gain", SST_GAIN_MIN_VALUE, SST_GAIN_MAX_VALUE,	\
		SST_GAIN_TC_MIN, SST_GAIN_TC_MAX,					\
		sst_gain_get, sst_gain_put,						\
		SST_MODULE_ID_GAIN_CELL, path_id, instance, task_id,			\
		sst_gain_tlv_common, gain_var)

#define SST_VOLUME(name, path_id, task_id, instance, gain_var)				\
	SST_GAIN_KCONTROLS(name, "Volume", SST_GAIN_MIN_VALUE, SST_GAIN_MAX_VALUE,	\
		SST_GAIN_TC_MIN, SST_GAIN_TC_MAX,					\
		sst_gain_get, sst_gain_put,						\
		SST_MODULE_ID_VOLUME, path_id, instance, task_id,			\
		sst_gain_tlv_common, gain_var)

static struct sst_gain_value sst_gains[];

static const struct snd_kcontrol_new sst_gain_controls[] = {
	SST_GAIN("media0_in", SST_PATH_INDEX_MEDIA0_IN, SST_TASK_MMX, 0, &sst_gains[0]),
	SST_GAIN("media1_in", SST_PATH_INDEX_MEDIA1_IN, SST_TASK_MMX, 0, &sst_gains[1]),
	SST_GAIN("media2_in", SST_PATH_INDEX_MEDIA2_IN, SST_TASK_MMX, 0, &sst_gains[2]),
	SST_GAIN("media3_in", SST_PATH_INDEX_MEDIA3_IN, SST_TASK_MMX, 0, &sst_gains[3]),

	SST_GAIN("pcm0_in", SST_PATH_INDEX_PCM0_IN, SST_TASK_SBA, 0, &sst_gains[4]),
	SST_GAIN("pcm1_in", SST_PATH_INDEX_PCM1_IN, SST_TASK_SBA, 0, &sst_gains[5]),
	SST_GAIN("pcm1_out", SST_PATH_INDEX_PCM1_OUT, SST_TASK_SBA, 0, &sst_gains[6]),
	SST_GAIN("pcm2_out", SST_PATH_INDEX_PCM2_OUT, SST_TASK_SBA, 0, &sst_gains[7]),

	SST_GAIN("codec_in0", SST_PATH_INDEX_CODEC_IN0, SST_TASK_SBA, 0, &sst_gains[8]),
	SST_GAIN("codec_in1", SST_PATH_INDEX_CODEC_IN1, SST_TASK_SBA, 0, &sst_gains[9]),
	SST_GAIN("codec_out0", SST_PATH_INDEX_CODEC_OUT0, SST_TASK_SBA, 0, &sst_gains[10]),
	SST_GAIN("codec_out1", SST_PATH_INDEX_CODEC_OUT1, SST_TASK_SBA, 0, &sst_gains[11]),
	SST_GAIN("media_loop1_out", SST_PATH_INDEX_MEDIA_LOOP1_OUT, SST_TASK_SBA, 0, &sst_gains[12]),
	SST_GAIN("media_loop2_out", SST_PATH_INDEX_MEDIA_LOOP2_OUT, SST_TASK_SBA, 0, &sst_gains[13]),
	SST_GAIN("sprot_loop_out", SST_PATH_INDEX_SPROT_LOOP_OUT, SST_TASK_SBA, 0, &sst_gains[14]),
	SST_VOLUME("media0_in", SST_PATH_INDEX_MEDIA0_IN, SST_TASK_MMX, 0, &sst_gains[15]),
};

#define SST_GAIN_NUM_CONTROLS 3
/* the SST_GAIN macro above will create three alsa controls for each
 * instance invoked, gain, mute and ramp duration, which use the same gain
 * cell sst_gain to keep track of data
 * To calculate number of gain cell instances we need to device by 3 in
 * below caulcation for gain cell memory.
 * This gets rid of static number and issues while adding new controls
 */
static struct sst_gain_value sst_gains[ARRAY_SIZE(sst_gain_controls)/SST_GAIN_NUM_CONTROLS];

static const struct snd_kcontrol_new sst_algo_controls[] = {
	SST_ALGO_KCONTROL_BYTES("media_loop1_out", "fir", 272, SST_MODULE_ID_FIR_24,
		 SST_PATH_INDEX_MEDIA_LOOP1_OUT, 0, SST_TASK_SBA, SBA_VB_SET_FIR),
	SST_ALGO_KCONTROL_BYTES("media_loop1_out", "iir", 300, SST_MODULE_ID_IIR_24,
		SST_PATH_INDEX_MEDIA_LOOP1_OUT, 0, SST_TASK_SBA, SBA_VB_SET_IIR),
	SST_ALGO_KCONTROL_BYTES("media_loop1_out", "mdrp", 286, SST_MODULE_ID_MDRP,
		SST_PATH_INDEX_MEDIA_LOOP1_OUT, 0, SST_TASK_SBA, SBA_SET_MDRP),
	SST_ALGO_KCONTROL_BYTES("media_loop2_out", "fir", 272, SST_MODULE_ID_FIR_24,
		SST_PATH_INDEX_MEDIA_LOOP2_OUT, 0, SST_TASK_SBA, SBA_VB_SET_FIR),
	SST_ALGO_KCONTROL_BYTES("media_loop2_out", "iir", 300, SST_MODULE_ID_IIR_24,
		SST_PATH_INDEX_MEDIA_LOOP2_OUT, 0, SST_TASK_SBA, SBA_VB_SET_IIR),
	SST_ALGO_KCONTROL_BYTES("media_loop2_out", "mdrp", 286, SST_MODULE_ID_MDRP,
		SST_PATH_INDEX_MEDIA_LOOP2_OUT, 0, SST_TASK_SBA, SBA_SET_MDRP),
	SST_ALGO_KCONTROL_BYTES("sprot_loop_out", "lpro", 192, SST_MODULE_ID_SPROT,
		SST_PATH_INDEX_SPROT_LOOP_OUT, 0, SST_TASK_SBA, SBA_VB_LPRO),
	SST_ALGO_KCONTROL_BYTES("codec_in0", "dcr", 52, SST_MODULE_ID_FILT_DCR,
		SST_PATH_INDEX_CODEC_IN0, 0, SST_TASK_SBA, SBA_VB_SET_IIR),
	SST_ALGO_KCONTROL_BYTES("codec_in1", "dcr", 52, SST_MODULE_ID_FILT_DCR,
		SST_PATH_INDEX_CODEC_IN1, 0, SST_TASK_SBA, SBA_VB_SET_IIR),

};

static int sst_algo_control_init(struct device *dev)
{
	int i = 0;
	struct sst_algo_control *bc;
	/*allocate space to cache the algo parameters in the driver*/
	for (i = 0; i < ARRAY_SIZE(sst_algo_controls); i++) {
		bc = (struct sst_algo_control *)sst_algo_controls[i].private_value;
		bc->params = devm_kzalloc(dev, bc->max, GFP_KERNEL);
		if (bc->params == NULL)
			return -ENOMEM;
	}
	return 0;
}

static bool is_sst_dapm_widget(struct snd_soc_dapm_widget *w)
{
	switch (w->id) {
	case snd_soc_dapm_pga:
	case snd_soc_dapm_aif_in:
	case snd_soc_dapm_aif_out:
	case snd_soc_dapm_input:
	case snd_soc_dapm_output:
	case snd_soc_dapm_mixer:
		return true;
	default:
		return false;
	}
}

/**
 * sst_send_pipe_gains - send gains for the front-end DAIs
 *
 * The gains in the pipes connected to the front-ends are muted/unmuted
 * automatically via the digital_mute() DAPM callback. This function sends the
 * gains for the front-end pipes.
 */
int sst_send_pipe_gains(struct snd_soc_dai *dai, int stream, int mute)
{
	struct sst_data *drv = snd_soc_dai_get_drvdata(dai);
	struct snd_soc_dapm_widget *w;
	struct snd_soc_dapm_path *p = NULL;

	dev_dbg(dai->dev, "enter, dai-name=%s dir=%d\n", dai->name, stream);

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		dev_dbg(dai->dev, "Stream name=%s\n",
				dai->playback_widget->name);
		w = dai->playback_widget;
		list_for_each_entry(p, &w->sinks, list_source) {
			if (p->connected && !p->connected(w, p->sink))
				continue;

			if (p->connect && p->sink->power &&
					is_sst_dapm_widget(p->sink)) {
				struct sst_ids *ids = p->sink->priv;

				dev_dbg(dai->dev, "send gains for widget=%s\n",
						p->sink->name);
				mutex_lock(&drv->lock);
				sst_set_pipe_gain(ids, drv, mute);
				mutex_unlock(&drv->lock);
			}
		}
	} else {
		dev_dbg(dai->dev, "Stream name=%s\n",
				dai->capture_widget->name);
		w = dai->capture_widget;
		list_for_each_entry(p, &w->sources, list_sink) {
			if (p->connected && !p->connected(w, p->sink))
				continue;

			if (p->connect &&  p->source->power &&
					is_sst_dapm_widget(p->source)) {
				struct sst_ids *ids = p->source->priv;

				dev_dbg(dai->dev, "send gain for widget=%s\n",
						p->source->name);
				mutex_lock(&drv->lock);
				sst_set_pipe_gain(ids, drv, mute);
				mutex_unlock(&drv->lock);
			}
		}
	}
	return 0;
}

/**
 * sst_fill_module_list - populate the list of modules/gains for a pipe
 *
 *
 * Fills the widget pointer in the kcontrol private data, and also fills the
 * kcontrol pointer in the widget private data.
 *
 * Widget pointer is used to send the algo/gain in the .put() handler if the
 * widget is powerd on.
 *
 * Kcontrol pointer is used to send the algo/gain in the widget power ON/OFF
 * event handler. Each widget (pipe) has multiple algos stored in the algo_list.
 */
static int sst_fill_module_list(struct snd_kcontrol *kctl,
	 struct snd_soc_dapm_widget *w, int type)
{
	struct sst_module *module = NULL;
	struct snd_soc_component *c = snd_soc_dapm_to_component(w->dapm);
	struct sst_ids *ids = w->priv;
	int ret = 0;

	module = devm_kzalloc(c->dev, sizeof(*module), GFP_KERNEL);
	if (!module)
		return -ENOMEM;

	if (type == SST_MODULE_GAIN) {
		struct sst_gain_mixer_control *mc = (void *)kctl->private_value;

		mc->w = w;
		module->kctl = kctl;
		list_add_tail(&module->node, &ids->gain_list);
	} else if (type == SST_MODULE_ALGO) {
		struct sst_algo_control *bc = (void *)kctl->private_value;

		bc->w = w;
		module->kctl = kctl;
		list_add_tail(&module->node, &ids->algo_list);
	} else {
		dev_err(c->dev, "invoked for unknown type %d module %s",
				type, kctl->id.name);
		ret = -EINVAL;
	}

	return ret;
}

/**
 * sst_fill_widget_module_info - fill list of gains/algos for the pipe
 * @widget:	pipe modelled as a DAPM widget
 *
 * Fill the list of gains/algos for the widget by looking at all the card
 * controls and comparing the name of the widget with the first part of control
 * name. First part of control name contains the pipe name (widget name).
 */
static int sst_fill_widget_module_info(struct snd_soc_dapm_widget *w,
	struct snd_soc_platform *platform)
{
	struct snd_kcontrol *kctl;
	int index, ret = 0;
	struct snd_card *card = platform->component.card->snd_card;
	char *idx;

	down_read(&card->controls_rwsem);

	list_for_each_entry(kctl, &card->controls, list) {
		idx = strstr(kctl->id.name, " ");
		if (idx == NULL)
			continue;
		index  = strlen(kctl->id.name) - strlen(idx);

		if (strstr(kctl->id.name, "Volume") &&
		    !strncmp(kctl->id.name, w->name, index))
			ret = sst_fill_module_list(kctl, w, SST_MODULE_GAIN);

		else if (strstr(kctl->id.name, "params") &&
			 !strncmp(kctl->id.name, w->name, index))
			ret = sst_fill_module_list(kctl, w, SST_MODULE_ALGO);

		else if (strstr(kctl->id.name, "Switch") &&
			 !strncmp(kctl->id.name, w->name, index) &&
			 strstr(kctl->id.name, "Gain")) {
			struct sst_gain_mixer_control *mc =
						(void *)kctl->private_value;

			mc->w = w;

		} else if (strstr(kctl->id.name, "interleaver") &&
			 !strncmp(kctl->id.name, w->name, index)) {
			struct sst_enum *e = (void *)kctl->private_value;

			e->w = w;

		} else if (strstr(kctl->id.name, "deinterleaver") &&
			 !strncmp(kctl->id.name, w->name, index)) {

			struct sst_enum *e = (void *)kctl->private_value;

			e->w = w;
		}

		if (ret < 0) {
			up_read(&card->controls_rwsem);
			return ret;
		}
	}

	up_read(&card->controls_rwsem);
	return 0;
}

/**
 * sst_fill_linked_widgets - fill the parent pointer for the linked widget
 */
static void sst_fill_linked_widgets(struct snd_soc_platform *platform,
						struct sst_ids *ids)
{
	struct snd_soc_dapm_widget *w;
	unsigned int len = strlen(ids->parent_wname);

	list_for_each_entry(w, &platform->component.card->widgets, list) {
		if (!strncmp(ids->parent_wname, w->name, len)) {
			ids->parent_w = w;
			break;
		}
	}
}

/**
 * sst_map_modules_to_pipe - fill algo/gains list for all pipes
 */
static int sst_map_modules_to_pipe(struct snd_soc_platform *platform)
{
	struct snd_soc_dapm_widget *w;
	int ret = 0;

	list_for_each_entry(w, &platform->component.card->widgets, list) {
		if (is_sst_dapm_widget(w) && (w->priv)) {
			struct sst_ids *ids = w->priv;

			dev_dbg(platform->dev, "widget type=%d name=%s\n",
					w->id, w->name);
			INIT_LIST_HEAD(&ids->algo_list);
			INIT_LIST_HEAD(&ids->gain_list);
			ret = sst_fill_widget_module_info(w, platform);

			if (ret < 0)
				return ret;

			/* fill linked widgets */
			if (ids->parent_wname !=  NULL)
				sst_fill_linked_widgets(platform, ids);
		}
	}
	return 0;
}

int sst_dsp_init_v2_dpcm(struct snd_soc_platform *platform)
{
	int i, ret = 0;
	struct snd_soc_dapm_context *dapm =
			snd_soc_component_get_dapm(&platform->component);
	struct sst_data *drv = snd_soc_platform_get_drvdata(platform);
	unsigned int gains = ARRAY_SIZE(sst_gain_controls)/3;

	drv->byte_stream = devm_kzalloc(platform->dev,
					SST_MAX_BIN_BYTES, GFP_KERNEL);
	if (!drv->byte_stream)
		return -ENOMEM;

	snd_soc_dapm_new_controls(dapm, sst_dapm_widgets,
			ARRAY_SIZE(sst_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, intercon,
			ARRAY_SIZE(intercon));
	snd_soc_dapm_new_widgets(dapm->card);

	for (i = 0; i < gains; i++) {
		sst_gains[i].mute = SST_GAIN_MUTE_DEFAULT;
		sst_gains[i].l_gain = SST_GAIN_VOLUME_DEFAULT;
		sst_gains[i].r_gain = SST_GAIN_VOLUME_DEFAULT;
		sst_gains[i].ramp_duration = SST_GAIN_RAMP_DURATION_DEFAULT;
	}

	ret = snd_soc_add_platform_controls(platform, sst_gain_controls,
			ARRAY_SIZE(sst_gain_controls));
	if (ret)
		return ret;

	/* Initialize algo control params */
	ret = sst_algo_control_init(platform->dev);
	if (ret)
		return ret;
	ret = snd_soc_add_platform_controls(platform, sst_algo_controls,
			ARRAY_SIZE(sst_algo_controls));
	if (ret)
		return ret;

	ret = snd_soc_add_platform_controls(platform, sst_slot_controls,
			ARRAY_SIZE(sst_slot_controls));
	if (ret)
		return ret;

	ret = sst_map_modules_to_pipe(platform);

	return ret;
}
