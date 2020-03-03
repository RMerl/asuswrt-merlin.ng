/*
 * Jack-detection handling for HD-audio
 *
 * Copyright (c) 2011 Takashi Iwai <tiwai@suse.de>
 *
 * This driver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/jack.h>
#include "hda_codec.h"
#include "hda_local.h"
#include "hda_auto_parser.h"
#include "hda_jack.h"

/**
 * is_jack_detectable - Check whether the given pin is jack-detectable
 * @codec: the HDA codec
 * @nid: pin NID
 *
 * Check whether the given pin is capable to report the jack detection.
 * The jack detection might not work by various reasons, e.g. the jack
 * detection is prohibited in the codec level, the pin config has
 * AC_DEFCFG_MISC_NO_PRESENCE bit, no unsol support, etc.
 */
bool is_jack_detectable(struct hda_codec *codec, hda_nid_t nid)
{
	if (codec->no_jack_detect)
		return false;
	if (!(snd_hda_query_pin_caps(codec, nid) & AC_PINCAP_PRES_DETECT))
		return false;
	if (get_defcfg_misc(snd_hda_codec_get_pincfg(codec, nid)) &
	     AC_DEFCFG_MISC_NO_PRESENCE)
		return false;
	if (!(get_wcaps(codec, nid) & AC_WCAP_UNSOL_CAP) &&
	    !codec->jackpoll_interval)
		return false;
	return true;
}
EXPORT_SYMBOL_GPL(is_jack_detectable);

/* execute pin sense measurement */
static u32 read_pin_sense(struct hda_codec *codec, hda_nid_t nid)
{
	u32 pincap;
	u32 val;

	if (!codec->no_trigger_sense) {
		pincap = snd_hda_query_pin_caps(codec, nid);
		if (pincap & AC_PINCAP_TRIG_REQ) /* need trigger? */
			snd_hda_codec_read(codec, nid, 0,
					AC_VERB_SET_PIN_SENSE, 0);
	}
	val = snd_hda_codec_read(codec, nid, 0,
				  AC_VERB_GET_PIN_SENSE, 0);
	if (codec->inv_jack_detect)
		val ^= AC_PINSENSE_PRESENCE;
	return val;
}

/**
 * snd_hda_jack_tbl_get - query the jack-table entry for the given NID
 * @codec: the HDA codec
 * @nid: pin NID to refer to
 */
struct hda_jack_tbl *
snd_hda_jack_tbl_get(struct hda_codec *codec, hda_nid_t nid)
{
	struct hda_jack_tbl *jack = codec->jacktbl.list;
	int i;

	if (!nid || !jack)
		return NULL;
	for (i = 0; i < codec->jacktbl.used; i++, jack++)
		if (jack->nid == nid)
			return jack;
	return NULL;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_tbl_get);

/**
 * snd_hda_jack_tbl_get_from_tag - query the jack-table entry for the given tag
 * @codec: the HDA codec
 * @tag: tag value to refer to
 */
struct hda_jack_tbl *
snd_hda_jack_tbl_get_from_tag(struct hda_codec *codec, unsigned char tag)
{
	struct hda_jack_tbl *jack = codec->jacktbl.list;
	int i;

	if (!tag || !jack)
		return NULL;
	for (i = 0; i < codec->jacktbl.used; i++, jack++)
		if (jack->tag == tag)
			return jack;
	return NULL;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_tbl_get_from_tag);

/**
 * snd_hda_jack_tbl_new - create a jack-table entry for the given NID
 * @codec: the HDA codec
 * @nid: pin NID to assign
 */
static struct hda_jack_tbl *
snd_hda_jack_tbl_new(struct hda_codec *codec, hda_nid_t nid)
{
	struct hda_jack_tbl *jack = snd_hda_jack_tbl_get(codec, nid);
	if (jack)
		return jack;
	jack = snd_array_new(&codec->jacktbl);
	if (!jack)
		return NULL;
	jack->nid = nid;
	jack->jack_dirty = 1;
	jack->tag = codec->jacktbl.used;
	return jack;
}

void snd_hda_jack_tbl_clear(struct hda_codec *codec)
{
	struct hda_jack_tbl *jack = codec->jacktbl.list;
	int i;

	for (i = 0; i < codec->jacktbl.used; i++, jack++) {
		struct hda_jack_callback *cb, *next;
#ifdef CONFIG_SND_HDA_INPUT_JACK
		/* free jack instances manually when clearing/reconfiguring */
		if (!codec->bus->shutdown && jack->jack)
			snd_device_free(codec->card, jack->jack);
#endif
		for (cb = jack->callback; cb; cb = next) {
			next = cb->next;
			kfree(cb);
		}
	}
	snd_array_free(&codec->jacktbl);
}

#define get_jack_plug_state(sense) !!(sense & AC_PINSENSE_PRESENCE)

/* update the cached value and notification flag if needed */
static void jack_detect_update(struct hda_codec *codec,
			       struct hda_jack_tbl *jack)
{
	if (!jack->jack_dirty)
		return;

	if (jack->phantom_jack)
		jack->pin_sense = AC_PINSENSE_PRESENCE;
	else
		jack->pin_sense = read_pin_sense(codec, jack->nid);

	/* A gating jack indicates the jack is invalid if gating is unplugged */
	if (jack->gating_jack && !snd_hda_jack_detect(codec, jack->gating_jack))
		jack->pin_sense &= ~AC_PINSENSE_PRESENCE;

	jack->jack_dirty = 0;

	/* If a jack is gated by this one update it. */
	if (jack->gated_jack) {
		struct hda_jack_tbl *gated =
			snd_hda_jack_tbl_get(codec, jack->gated_jack);
		if (gated) {
			gated->jack_dirty = 1;
			jack_detect_update(codec, gated);
		}
	}
}

/**
 * snd_hda_set_dirty_all - Mark all the cached as dirty
 * @codec: the HDA codec
 *
 * This function sets the dirty flag to all entries of jack table.
 * It's called from the resume path in hda_codec.c.
 */
void snd_hda_jack_set_dirty_all(struct hda_codec *codec)
{
	struct hda_jack_tbl *jack = codec->jacktbl.list;
	int i;

	for (i = 0; i < codec->jacktbl.used; i++, jack++)
		if (jack->nid)
			jack->jack_dirty = 1;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_set_dirty_all);

/**
 * snd_hda_pin_sense - execute pin sense measurement
 * @codec: the CODEC to sense
 * @nid: the pin NID to sense
 *
 * Execute necessary pin sense measurement and return its Presence Detect,
 * Impedance, ELD Valid etc. status bits.
 */
u32 snd_hda_pin_sense(struct hda_codec *codec, hda_nid_t nid)
{
	struct hda_jack_tbl *jack = snd_hda_jack_tbl_get(codec, nid);
	if (jack) {
		jack_detect_update(codec, jack);
		return jack->pin_sense;
	}
	return read_pin_sense(codec, nid);
}
EXPORT_SYMBOL_GPL(snd_hda_pin_sense);

/**
 * snd_hda_jack_detect_state - query pin Presence Detect status
 * @codec: the CODEC to sense
 * @nid: the pin NID to sense
 *
 * Query and return the pin's Presence Detect status, as either
 * HDA_JACK_NOT_PRESENT, HDA_JACK_PRESENT or HDA_JACK_PHANTOM.
 */
int snd_hda_jack_detect_state(struct hda_codec *codec, hda_nid_t nid)
{
	struct hda_jack_tbl *jack = snd_hda_jack_tbl_get(codec, nid);
	if (jack && jack->phantom_jack)
		return HDA_JACK_PHANTOM;
	else if (snd_hda_pin_sense(codec, nid) & AC_PINSENSE_PRESENCE)
		return HDA_JACK_PRESENT;
	else
		return HDA_JACK_NOT_PRESENT;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_detect_state);

/**
 * snd_hda_jack_detect_enable - enable the jack-detection
 * @codec: the HDA codec
 * @nid: pin NID to enable
 * @func: callback function to register
 *
 * In the case of error, the return value will be a pointer embedded with
 * errno.  Check and handle the return value appropriately with standard
 * macros such as @IS_ERR() and @PTR_ERR().
 */
struct hda_jack_callback *
snd_hda_jack_detect_enable_callback(struct hda_codec *codec, hda_nid_t nid,
				    hda_jack_callback_fn func)
{
	struct hda_jack_tbl *jack;
	struct hda_jack_callback *callback = NULL;
	int err;

	jack = snd_hda_jack_tbl_new(codec, nid);
	if (!jack)
		return ERR_PTR(-ENOMEM);
	if (func) {
		callback = kzalloc(sizeof(*callback), GFP_KERNEL);
		if (!callback)
			return ERR_PTR(-ENOMEM);
		callback->func = func;
		callback->nid = jack->nid;
		callback->next = jack->callback;
		jack->callback = callback;
	}

	if (jack->jack_detect)
		return callback; /* already registered */
	jack->jack_detect = 1;
	if (codec->jackpoll_interval > 0)
		return callback; /* No unsol if we're polling instead */
	err = snd_hda_codec_write_cache(codec, nid, 0,
					 AC_VERB_SET_UNSOLICITED_ENABLE,
					 AC_USRSP_EN | jack->tag);
	if (err < 0)
		return ERR_PTR(err);
	return callback;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_detect_enable_callback);

/**
 * snd_hda_jack_detect_enable - Enable the jack detection on the given pin
 * @codec: the HDA codec
 * @nid: pin NID to enable jack detection
 *
 * Enable the jack detection with the default callback.  Returns zero if
 * successful or a negative error code.
 */
int snd_hda_jack_detect_enable(struct hda_codec *codec, hda_nid_t nid)
{
	return PTR_ERR_OR_ZERO(snd_hda_jack_detect_enable_callback(codec, nid, NULL));
}
EXPORT_SYMBOL_GPL(snd_hda_jack_detect_enable);

/**
 * snd_hda_jack_set_gating_jack - Set gating jack.
 * @codec: the HDA codec
 * @gated_nid: gated pin NID
 * @gating_nid: gating pin NID
 *
 * Indicates the gated jack is only valid when the gating jack is plugged.
 */
int snd_hda_jack_set_gating_jack(struct hda_codec *codec, hda_nid_t gated_nid,
				 hda_nid_t gating_nid)
{
	struct hda_jack_tbl *gated = snd_hda_jack_tbl_new(codec, gated_nid);
	struct hda_jack_tbl *gating = snd_hda_jack_tbl_new(codec, gating_nid);

	if (!gated || !gating)
		return -EINVAL;

	gated->gating_jack = gating_nid;
	gating->gated_jack = gated_nid;

	return 0;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_set_gating_jack);

/**
 * snd_hda_jack_report_sync - sync the states of all jacks and report if changed
 * @codec: the HDA codec
 */
void snd_hda_jack_report_sync(struct hda_codec *codec)
{
	struct hda_jack_tbl *jack;
	int i, state;

	/* update all jacks at first */
	jack = codec->jacktbl.list;
	for (i = 0; i < codec->jacktbl.used; i++, jack++)
		if (jack->nid)
			jack_detect_update(codec, jack);

	/* report the updated jacks; it's done after updating all jacks
	 * to make sure that all gating jacks properly have been set
	 */
	jack = codec->jacktbl.list;
	for (i = 0; i < codec->jacktbl.used; i++, jack++)
		if (jack->nid) {
			if (!jack->kctl || jack->block_report)
				continue;
			state = get_jack_plug_state(jack->pin_sense);
			snd_kctl_jack_report(codec->card, jack->kctl, state);
#ifdef CONFIG_SND_HDA_INPUT_JACK
			if (jack->jack)
				snd_jack_report(jack->jack,
						state ? jack->type : 0);
#endif
		}
}
EXPORT_SYMBOL_GPL(snd_hda_jack_report_sync);

#ifdef CONFIG_SND_HDA_INPUT_JACK
/* guess the jack type from the pin-config */
static int get_input_jack_type(struct hda_codec *codec, hda_nid_t nid)
{
	unsigned int def_conf = snd_hda_codec_get_pincfg(codec, nid);
	switch (get_defcfg_device(def_conf)) {
	case AC_JACK_LINE_OUT:
	case AC_JACK_SPEAKER:
		return SND_JACK_LINEOUT;
	case AC_JACK_HP_OUT:
		return SND_JACK_HEADPHONE;
	case AC_JACK_SPDIF_OUT:
	case AC_JACK_DIG_OTHER_OUT:
		return SND_JACK_AVOUT;
	case AC_JACK_MIC_IN:
		return SND_JACK_MICROPHONE;
	default:
		return SND_JACK_LINEIN;
	}
}

static void hda_free_jack_priv(struct snd_jack *jack)
{
	struct hda_jack_tbl *jacks = jack->private_data;
	jacks->nid = 0;
	jacks->jack = NULL;
}
#endif

/**
 * snd_hda_jack_add_kctl - Add a kctl for the given pin
 * @codec: the HDA codec
 * @nid: pin NID to assign
 * @name: string name for the jack
 * @idx: index number for the jack
 * @phantom_jack: flag to deal as a phantom jack
 *
 * This assigns a jack-detection kctl to the given pin.  The kcontrol
 * will have the given name and index.
 */
static int __snd_hda_jack_add_kctl(struct hda_codec *codec, hda_nid_t nid,
			  const char *name, int idx, bool phantom_jack)
{
	struct hda_jack_tbl *jack;
	struct snd_kcontrol *kctl;
	int err, state;

	jack = snd_hda_jack_tbl_new(codec, nid);
	if (!jack)
		return 0;
	if (jack->kctl)
		return 0; /* already created */
	kctl = snd_kctl_jack_new(name, idx, codec);
	if (!kctl)
		return -ENOMEM;
	err = snd_hda_ctl_add(codec, nid, kctl);
	if (err < 0)
		return err;
	jack->kctl = kctl;
	jack->phantom_jack = !!phantom_jack;

	state = snd_hda_jack_detect(codec, nid);
	snd_kctl_jack_report(codec->card, kctl, state);
#ifdef CONFIG_SND_HDA_INPUT_JACK
	if (!phantom_jack) {
		jack->type = get_input_jack_type(codec, nid);
		err = snd_jack_new(codec->card, name, jack->type,
				   &jack->jack);
		if (err < 0)
			return err;
		jack->jack->private_data = jack;
		jack->jack->private_free = hda_free_jack_priv;
		snd_jack_report(jack->jack, state ? jack->type : 0);
	}
#endif
	return 0;
}

/**
 * snd_hda_jack_add_kctl - Add a jack kctl for the given pin
 * @codec: the HDA codec
 * @nid: pin NID
 * @name: the name string for the jack ctl
 * @idx: the ctl index for the jack ctl
 *
 * This is a simple helper calling __snd_hda_jack_add_kctl().
 */
int snd_hda_jack_add_kctl(struct hda_codec *codec, hda_nid_t nid,
			  const char *name, int idx)
{
	return __snd_hda_jack_add_kctl(codec, nid, name, idx, false);
}
EXPORT_SYMBOL_GPL(snd_hda_jack_add_kctl);

/* get the unique index number for the given kctl name */
static int get_unique_index(struct hda_codec *codec, const char *name, int idx)
{
	struct hda_jack_tbl *jack;
	int i, len = strlen(name);
 again:
	jack = codec->jacktbl.list;
	for (i = 0; i < codec->jacktbl.used; i++, jack++) {
		/* jack->kctl.id contains "XXX Jack" name string with index */
		if (jack->kctl &&
		    !strncmp(name, jack->kctl->id.name, len) &&
		    !strcmp(" Jack", jack->kctl->id.name + len) &&
		    jack->kctl->id.index == idx) {
			idx++;
			goto again;
		}
	}
	return idx;
}

static int add_jack_kctl(struct hda_codec *codec, hda_nid_t nid,
			 const struct auto_pin_cfg *cfg,
			 const char *base_name)
{
	unsigned int def_conf, conn;
	char name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	int idx, err;
	bool phantom_jack;

	if (!nid)
		return 0;
	def_conf = snd_hda_codec_get_pincfg(codec, nid);
	conn = get_defcfg_connect(def_conf);
	if (conn == AC_JACK_PORT_NONE)
		return 0;
	phantom_jack = (conn != AC_JACK_PORT_COMPLEX) ||
		       !is_jack_detectable(codec, nid);

	if (base_name) {
		strlcpy(name, base_name, sizeof(name));
		idx = 0;
	} else
		snd_hda_get_pin_label(codec, nid, cfg, name, sizeof(name), &idx);
	if (phantom_jack)
		/* Example final name: "Internal Mic Phantom Jack" */
		strncat(name, " Phantom", sizeof(name) - strlen(name) - 1);
	idx = get_unique_index(codec, name, idx);
	err = __snd_hda_jack_add_kctl(codec, nid, name, idx, phantom_jack);
	if (err < 0)
		return err;

	if (!phantom_jack)
		return snd_hda_jack_detect_enable(codec, nid);
	return 0;
}

/**
 * snd_hda_jack_add_kctls - Add kctls for all pins included in the given pincfg
 * @codec: the HDA codec
 * @cfg: pin config table to parse
 */
int snd_hda_jack_add_kctls(struct hda_codec *codec,
			   const struct auto_pin_cfg *cfg)
{
	const hda_nid_t *p;
	int i, err;

	for (i = 0; i < cfg->num_inputs; i++) {
		/* If we have headphone mics; make sure they get the right name
		   before grabbed by output pins */
		if (cfg->inputs[i].is_headphone_mic) {
			if (auto_cfg_hp_outs(cfg) == 1)
				err = add_jack_kctl(codec, auto_cfg_hp_pins(cfg)[0],
						    cfg, "Headphone Mic");
			else
				err = add_jack_kctl(codec, cfg->inputs[i].pin,
						    cfg, "Headphone Mic");
		} else
			err = add_jack_kctl(codec, cfg->inputs[i].pin, cfg,
					    NULL);
		if (err < 0)
			return err;
	}

	for (i = 0, p = cfg->line_out_pins; i < cfg->line_outs; i++, p++) {
		err = add_jack_kctl(codec, *p, cfg, NULL);
		if (err < 0)
			return err;
	}
	for (i = 0, p = cfg->hp_pins; i < cfg->hp_outs; i++, p++) {
		if (*p == *cfg->line_out_pins) /* might be duplicated */
			break;
		err = add_jack_kctl(codec, *p, cfg, NULL);
		if (err < 0)
			return err;
	}
	for (i = 0, p = cfg->speaker_pins; i < cfg->speaker_outs; i++, p++) {
		if (*p == *cfg->line_out_pins) /* might be duplicated */
			break;
		err = add_jack_kctl(codec, *p, cfg, NULL);
		if (err < 0)
			return err;
	}
	for (i = 0, p = cfg->dig_out_pins; i < cfg->dig_outs; i++, p++) {
		err = add_jack_kctl(codec, *p, cfg, NULL);
		if (err < 0)
			return err;
	}
	err = add_jack_kctl(codec, cfg->dig_in_pin, cfg, NULL);
	if (err < 0)
		return err;
	err = add_jack_kctl(codec, cfg->mono_out_pin, cfg, NULL);
	if (err < 0)
		return err;
	return 0;
}
EXPORT_SYMBOL_GPL(snd_hda_jack_add_kctls);

static void call_jack_callback(struct hda_codec *codec,
			       struct hda_jack_tbl *jack)
{
	struct hda_jack_callback *cb;

	for (cb = jack->callback; cb; cb = cb->next)
		cb->func(codec, cb);
	if (jack->gated_jack) {
		struct hda_jack_tbl *gated =
			snd_hda_jack_tbl_get(codec, jack->gated_jack);
		if (gated) {
			for (cb = gated->callback; cb; cb = cb->next)
				cb->func(codec, cb);
		}
	}
}

/**
 * snd_hda_jack_unsol_event - Handle an unsolicited event
 * @codec: the HDA codec
 * @res: the unsolicited event data
 */
void snd_hda_jack_unsol_event(struct hda_codec *codec, unsigned int res)
{
	struct hda_jack_tbl *event;
	int tag = (res >> AC_UNSOL_RES_TAG_SHIFT) & 0x7f;

	event = snd_hda_jack_tbl_get_from_tag(codec, tag);
	if (!event)
		return;
	event->jack_dirty = 1;

	call_jack_callback(codec, event);
	snd_hda_jack_report_sync(codec);
}
EXPORT_SYMBOL_GPL(snd_hda_jack_unsol_event);

/**
 * snd_hda_jack_poll_all - Poll all jacks
 * @codec: the HDA codec
 *
 * Poll all detectable jacks with dirty flag, update the status, call
 * callbacks and call snd_hda_jack_report_sync() if any changes are found.
 */
void snd_hda_jack_poll_all(struct hda_codec *codec)
{
	struct hda_jack_tbl *jack = codec->jacktbl.list;
	int i, changes = 0;

	for (i = 0; i < codec->jacktbl.used; i++, jack++) {
		unsigned int old_sense;
		if (!jack->nid || !jack->jack_dirty || jack->phantom_jack)
			continue;
		old_sense = get_jack_plug_state(jack->pin_sense);
		jack_detect_update(codec, jack);
		if (old_sense == get_jack_plug_state(jack->pin_sense))
			continue;
		changes = 1;
		call_jack_callback(codec, jack);
	}
	if (changes)
		snd_hda_jack_report_sync(codec);
}
EXPORT_SYMBOL_GPL(snd_hda_jack_poll_all);

