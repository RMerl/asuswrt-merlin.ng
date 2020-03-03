/*
 * HD audio interface patch for Conexant HDA audio codec
 *
 * Copyright (c) 2006 Pototskiy Akex <alex.pototskiy@gmail.com>
 * 		      Takashi Iwai <tiwai@suse.de>
 * 		      Tobin Davis  <tdavis@dsl-only.net>
 *
 *  This driver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This driver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <sound/core.h>
#include <sound/jack.h>

#include "hda_codec.h"
#include "hda_local.h"
#include "hda_auto_parser.h"
#include "hda_beep.h"
#include "hda_jack.h"
#include "hda_generic.h"

struct conexant_spec {
	struct hda_gen_spec gen;

	unsigned int beep_amp;

	/* extra EAPD pins */
	unsigned int num_eapds;
	hda_nid_t eapds[4];
	bool dynamic_eapd;
	hda_nid_t mute_led_eapd;

	unsigned int parse_flags; /* flag for snd_hda_parse_pin_defcfg() */

	/* OPLC XO specific */
	bool recording;
	bool dc_enable;
	unsigned int dc_input_bias; /* offset into olpc_xo_dc_bias */
	struct nid_path *dc_mode_path;
};


#ifdef CONFIG_SND_HDA_INPUT_BEEP
static inline void set_beep_amp(struct conexant_spec *spec, hda_nid_t nid,
				int idx, int dir)
{
	spec->gen.beep_nid = nid;
	spec->beep_amp = HDA_COMPOSE_AMP_VAL(nid, 1, idx, dir);
}
/* additional beep mixers; the actual parameters are overwritten at build */
static const struct snd_kcontrol_new cxt_beep_mixer[] = {
	HDA_CODEC_VOLUME_MONO("Beep Playback Volume", 0, 1, 0, HDA_OUTPUT),
	HDA_CODEC_MUTE_BEEP_MONO("Beep Playback Switch", 0, 1, 0, HDA_OUTPUT),
	{ } /* end */
};

/* create beep controls if needed */
static int add_beep_ctls(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	int err;

	if (spec->beep_amp) {
		const struct snd_kcontrol_new *knew;
		for (knew = cxt_beep_mixer; knew->name; knew++) {
			struct snd_kcontrol *kctl;
			kctl = snd_ctl_new1(knew, codec);
			if (!kctl)
				return -ENOMEM;
			kctl->private_value = spec->beep_amp;
			err = snd_hda_ctl_add(codec, 0, kctl);
			if (err < 0)
				return err;
		}
	}
	return 0;
}
#else
#define set_beep_amp(spec, nid, idx, dir) /* NOP */
#define add_beep_ctls(codec)	0
#endif

/*
 * Automatic parser for CX20641 & co
 */

#ifdef CONFIG_SND_HDA_INPUT_BEEP
static void cx_auto_parse_beep(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	hda_nid_t nid;

	for_each_hda_codec_node(nid, codec)
		if (get_wcaps_type(get_wcaps(codec, nid)) == AC_WID_BEEP) {
			set_beep_amp(spec, nid, 0, HDA_OUTPUT);
			break;
		}
}
#else
#define cx_auto_parse_beep(codec)
#endif

/* parse EAPDs */
static void cx_auto_parse_eapd(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	hda_nid_t nid;

	for_each_hda_codec_node(nid, codec) {
		if (get_wcaps_type(get_wcaps(codec, nid)) != AC_WID_PIN)
			continue;
		if (!(snd_hda_query_pin_caps(codec, nid) & AC_PINCAP_EAPD))
			continue;
		spec->eapds[spec->num_eapds++] = nid;
		if (spec->num_eapds >= ARRAY_SIZE(spec->eapds))
			break;
	}

	/* NOTE: below is a wild guess; if we have more than two EAPDs,
	 * it's a new chip, where EAPDs are supposed to be associated to
	 * pins, and we can control EAPD per pin.
	 * OTOH, if only one or two EAPDs are found, it's an old chip,
	 * thus it might control over all pins.
	 */
	if (spec->num_eapds > 2)
		spec->dynamic_eapd = 1;
}

static void cx_auto_turn_eapd(struct hda_codec *codec, int num_pins,
			      hda_nid_t *pins, bool on)
{
	int i;
	for (i = 0; i < num_pins; i++) {
		if (snd_hda_query_pin_caps(codec, pins[i]) & AC_PINCAP_EAPD)
			snd_hda_codec_write(codec, pins[i], 0,
					    AC_VERB_SET_EAPD_BTLENABLE,
					    on ? 0x02 : 0);
	}
}

/* turn on/off EAPD according to Master switch */
static void cx_auto_vmaster_hook(void *private_data, int enabled)
{
	struct hda_codec *codec = private_data;
	struct conexant_spec *spec = codec->spec;

	cx_auto_turn_eapd(codec, spec->num_eapds, spec->eapds, enabled);
}

/* turn on/off EAPD according to Master switch (inversely!) for mute LED */
static void cx_auto_vmaster_hook_mute_led(void *private_data, int enabled)
{
	struct hda_codec *codec = private_data;
	struct conexant_spec *spec = codec->spec;

	snd_hda_codec_write(codec, spec->mute_led_eapd, 0,
			    AC_VERB_SET_EAPD_BTLENABLE,
			    enabled ? 0x00 : 0x02);
}

static int cx_auto_build_controls(struct hda_codec *codec)
{
	int err;

	err = snd_hda_gen_build_controls(codec);
	if (err < 0)
		return err;

	err = add_beep_ctls(codec);
	if (err < 0)
		return err;

	return 0;
}

static int cx_auto_init(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	snd_hda_gen_init(codec);
	if (!spec->dynamic_eapd)
		cx_auto_turn_eapd(codec, spec->num_eapds, spec->eapds, true);

	snd_hda_apply_fixup(codec, HDA_FIXUP_ACT_INIT);

	return 0;
}

static void cx_auto_reboot_notify(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;

	switch (codec->core.vendor_id) {
	case 0x14f150f2: /* CX20722 */
	case 0x14f150f4: /* CX20724 */
		break;
	default:
		return;
	}

	/* Turn the CX20722 codec into D3 to avoid spurious noises
	   from the internal speaker during (and after) reboot */
	cx_auto_turn_eapd(codec, spec->num_eapds, spec->eapds, false);

	snd_hda_codec_set_power_to_all(codec, codec->core.afg, AC_PWRST_D3);
	snd_hda_codec_write(codec, codec->core.afg, 0,
			    AC_VERB_SET_POWER_STATE, AC_PWRST_D3);
}

static void cx_auto_free(struct hda_codec *codec)
{
	cx_auto_reboot_notify(codec);
	snd_hda_gen_free(codec);
}

static const struct hda_codec_ops cx_auto_patch_ops = {
	.build_controls = cx_auto_build_controls,
	.build_pcms = snd_hda_gen_build_pcms,
	.init = cx_auto_init,
	.reboot_notify = cx_auto_reboot_notify,
	.free = cx_auto_free,
	.unsol_event = snd_hda_jack_unsol_event,
#ifdef CONFIG_PM
	.check_power_status = snd_hda_gen_check_power_status,
#endif
};

/*
 * pin fix-up
 */
enum {
	CXT_PINCFG_LENOVO_X200,
	CXT_PINCFG_LENOVO_TP410,
	CXT_PINCFG_LEMOTE_A1004,
	CXT_PINCFG_LEMOTE_A1205,
	CXT_PINCFG_COMPAQ_CQ60,
	CXT_FIXUP_STEREO_DMIC,
	CXT_FIXUP_INC_MIC_BOOST,
	CXT_FIXUP_HEADPHONE_MIC_PIN,
	CXT_FIXUP_HEADPHONE_MIC,
	CXT_FIXUP_GPIO1,
	CXT_FIXUP_ASPIRE_DMIC,
	CXT_FIXUP_THINKPAD_ACPI,
	CXT_FIXUP_OLPC_XO,
	CXT_FIXUP_CAP_MIX_AMP,
	CXT_FIXUP_TOSHIBA_P105,
	CXT_FIXUP_HP_530,
	CXT_FIXUP_CAP_MIX_AMP_5047,
	CXT_FIXUP_MUTE_LED_EAPD,
};

/* for hda_fixup_thinkpad_acpi() */
#include "thinkpad_helper.c"

static void cxt_fixup_stereo_dmic(struct hda_codec *codec,
				  const struct hda_fixup *fix, int action)
{
	struct conexant_spec *spec = codec->spec;
	spec->gen.inv_dmic_split = 1;
}

static void cxt5066_increase_mic_boost(struct hda_codec *codec,
				   const struct hda_fixup *fix, int action)
{
	if (action != HDA_FIXUP_ACT_PRE_PROBE)
		return;

	snd_hda_override_amp_caps(codec, 0x17, HDA_OUTPUT,
				  (0x3 << AC_AMPCAP_OFFSET_SHIFT) |
				  (0x4 << AC_AMPCAP_NUM_STEPS_SHIFT) |
				  (0x27 << AC_AMPCAP_STEP_SIZE_SHIFT) |
				  (0 << AC_AMPCAP_MUTE_SHIFT));
}

static void cxt_update_headset_mode(struct hda_codec *codec)
{
	/* The verbs used in this function were tested on a Conexant CX20751/2 codec. */
	int i;
	bool mic_mode = false;
	struct conexant_spec *spec = codec->spec;
	struct auto_pin_cfg *cfg = &spec->gen.autocfg;

	hda_nid_t mux_pin = spec->gen.imux_pins[spec->gen.cur_mux[0]];

	for (i = 0; i < cfg->num_inputs; i++)
		if (cfg->inputs[i].pin == mux_pin) {
			mic_mode = !!cfg->inputs[i].is_headphone_mic;
			break;
		}

	if (mic_mode) {
		snd_hda_codec_write_cache(codec, 0x1c, 0, 0x410, 0x7c); /* enable merged mode for analog int-mic */
		spec->gen.hp_jack_present = false;
	} else {
		snd_hda_codec_write_cache(codec, 0x1c, 0, 0x410, 0x54); /* disable merged mode for analog int-mic */
		spec->gen.hp_jack_present = snd_hda_jack_detect(codec, spec->gen.autocfg.hp_pins[0]);
	}

	snd_hda_gen_update_outputs(codec);
}

static void cxt_update_headset_mode_hook(struct hda_codec *codec,
					 struct snd_kcontrol *kcontrol,
					 struct snd_ctl_elem_value *ucontrol)
{
	cxt_update_headset_mode(codec);
}

static void cxt_fixup_headphone_mic(struct hda_codec *codec,
				    const struct hda_fixup *fix, int action)
{
	struct conexant_spec *spec = codec->spec;

	switch (action) {
	case HDA_FIXUP_ACT_PRE_PROBE:
		spec->parse_flags |= HDA_PINCFG_HEADPHONE_MIC;
		snd_hdac_regmap_add_vendor_verb(&codec->core, 0x410);
		break;
	case HDA_FIXUP_ACT_PROBE:
		spec->gen.cap_sync_hook = cxt_update_headset_mode_hook;
		spec->gen.automute_hook = cxt_update_headset_mode;
		break;
	case HDA_FIXUP_ACT_INIT:
		cxt_update_headset_mode(codec);
		break;
	}
}

/* OPLC XO 1.5 fixup */

/* OLPC XO-1.5 supports DC input mode (e.g. for use with analog sensors)
 * through the microphone jack.
 * When the user enables this through a mixer switch, both internal and
 * external microphones are disabled. Gain is fixed at 0dB. In this mode,
 * we also allow the bias to be configured through a separate mixer
 * control. */

#define update_mic_pin(codec, nid, val)					\
	snd_hda_codec_update_cache(codec, nid, 0,			\
				   AC_VERB_SET_PIN_WIDGET_CONTROL, val)

static const struct hda_input_mux olpc_xo_dc_bias = {
	.num_items = 3,
	.items = {
		{ "Off", PIN_IN },
		{ "50%", PIN_VREF50 },
		{ "80%", PIN_VREF80 },
	},
};

static void olpc_xo_update_mic_boost(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	int ch, val;

	for (ch = 0; ch < 2; ch++) {
		val = AC_AMP_SET_OUTPUT |
			(ch ? AC_AMP_SET_RIGHT : AC_AMP_SET_LEFT);
		if (!spec->dc_enable)
			val |= snd_hda_codec_amp_read(codec, 0x17, ch, HDA_OUTPUT, 0);
		snd_hda_codec_write(codec, 0x17, 0,
				    AC_VERB_SET_AMP_GAIN_MUTE, val);
	}
}

static void olpc_xo_update_mic_pins(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	int cur_input, val;
	struct nid_path *path;

	cur_input = spec->gen.input_paths[0][spec->gen.cur_mux[0]];

	/* Set up mic pins for port-B, C and F dynamically as the recording
	 * LED is turned on/off by these pin controls
	 */
	if (!spec->dc_enable) {
		/* disable DC bias path and pin for port F */
		update_mic_pin(codec, 0x1e, 0);
		snd_hda_activate_path(codec, spec->dc_mode_path, false, false);

		/* update port B (ext mic) and C (int mic) */
		/* OLPC defers mic widget control until when capture is
		 * started because the microphone LED comes on as soon as
		 * these settings are put in place. if we did this before
		 * recording, it would give the false indication that
		 * recording is happening when it is not.
		 */
		update_mic_pin(codec, 0x1a, spec->recording ?
			       snd_hda_codec_get_pin_target(codec, 0x1a) : 0);
		update_mic_pin(codec, 0x1b, spec->recording ?
			       snd_hda_codec_get_pin_target(codec, 0x1b) : 0);
		/* enable normal mic path */
		path = snd_hda_get_path_from_idx(codec, cur_input);
		if (path)
			snd_hda_activate_path(codec, path, true, false);
	} else {
		/* disable normal mic path */
		path = snd_hda_get_path_from_idx(codec, cur_input);
		if (path)
			snd_hda_activate_path(codec, path, false, false);

		/* Even though port F is the DC input, the bias is controlled
		 * on port B.  We also leave that port as an active input (but
		 * unselected) in DC mode just in case that is necessary to
		 * make the bias setting take effect.
		 */
		if (spec->recording)
			val = olpc_xo_dc_bias.items[spec->dc_input_bias].index;
		else
			val = 0;
		update_mic_pin(codec, 0x1a, val);
		update_mic_pin(codec, 0x1b, 0);
		/* enable DC bias path and pin */
		update_mic_pin(codec, 0x1e, spec->recording ? PIN_IN : 0);
		snd_hda_activate_path(codec, spec->dc_mode_path, true, false);
	}
}

/* mic_autoswitch hook */
static void olpc_xo_automic(struct hda_codec *codec,
			    struct hda_jack_callback *jack)
{
	struct conexant_spec *spec = codec->spec;

	/* in DC mode, we don't handle automic */
	if (!spec->dc_enable)
		snd_hda_gen_mic_autoswitch(codec, jack);
	olpc_xo_update_mic_pins(codec);
	if (spec->dc_enable)
		olpc_xo_update_mic_boost(codec);
}

/* pcm_capture hook */
static void olpc_xo_capture_hook(struct hda_pcm_stream *hinfo,
				 struct hda_codec *codec,
				 struct snd_pcm_substream *substream,
				 int action)
{
	struct conexant_spec *spec = codec->spec;

	/* toggle spec->recording flag and update mic pins accordingly
	 * for turning on/off LED
	 */
	switch (action) {
	case HDA_GEN_PCM_ACT_PREPARE:
		spec->recording = 1;
		olpc_xo_update_mic_pins(codec);
		break;
	case HDA_GEN_PCM_ACT_CLEANUP:
		spec->recording = 0;
		olpc_xo_update_mic_pins(codec);
		break;
	}
}

static int olpc_xo_dc_mode_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct hda_codec *codec = snd_kcontrol_chip(kcontrol);
	struct conexant_spec *spec = codec->spec;
	ucontrol->value.integer.value[0] = spec->dc_enable;
	return 0;
}

static int olpc_xo_dc_mode_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	struct hda_codec *codec = snd_kcontrol_chip(kcontrol);
	struct conexant_spec *spec = codec->spec;
	int dc_enable = !!ucontrol->value.integer.value[0];

	if (dc_enable == spec->dc_enable)
		return 0;

	spec->dc_enable = dc_enable;
	olpc_xo_update_mic_pins(codec);
	olpc_xo_update_mic_boost(codec);
	return 1;
}

static int olpc_xo_dc_bias_enum_get(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct hda_codec *codec = snd_kcontrol_chip(kcontrol);
	struct conexant_spec *spec = codec->spec;
	ucontrol->value.enumerated.item[0] = spec->dc_input_bias;
	return 0;
}

static int olpc_xo_dc_bias_enum_info(struct snd_kcontrol *kcontrol,
				     struct snd_ctl_elem_info *uinfo)
{
	return snd_hda_input_mux_info(&olpc_xo_dc_bias, uinfo);
}

static int olpc_xo_dc_bias_enum_put(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	struct hda_codec *codec = snd_kcontrol_chip(kcontrol);
	struct conexant_spec *spec = codec->spec;
	const struct hda_input_mux *imux = &olpc_xo_dc_bias;
	unsigned int idx;

	idx = ucontrol->value.enumerated.item[0];
	if (idx >= imux->num_items)
		idx = imux->num_items - 1;
	if (spec->dc_input_bias == idx)
		return 0;

	spec->dc_input_bias = idx;
	if (spec->dc_enable)
		olpc_xo_update_mic_pins(codec);
	return 1;
}

static const struct snd_kcontrol_new olpc_xo_mixers[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "DC Mode Enable Switch",
		.info = snd_ctl_boolean_mono_info,
		.get = olpc_xo_dc_mode_get,
		.put = olpc_xo_dc_mode_put,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "DC Input Bias Enum",
		.info = olpc_xo_dc_bias_enum_info,
		.get = olpc_xo_dc_bias_enum_get,
		.put = olpc_xo_dc_bias_enum_put,
	},
	{}
};

/* overriding mic boost put callback; update mic boost volume only when
 * DC mode is disabled
 */
static int olpc_xo_mic_boost_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct hda_codec *codec = snd_kcontrol_chip(kcontrol);
	struct conexant_spec *spec = codec->spec;
	int ret = snd_hda_mixer_amp_volume_put(kcontrol, ucontrol);
	if (ret > 0 && spec->dc_enable)
		olpc_xo_update_mic_boost(codec);
	return ret;
}

static void cxt_fixup_olpc_xo(struct hda_codec *codec,
				    const struct hda_fixup *fix, int action)
{
	struct conexant_spec *spec = codec->spec;
	int i;

	if (action != HDA_FIXUP_ACT_PROBE)
		return;

	spec->gen.mic_autoswitch_hook = olpc_xo_automic;
	spec->gen.pcm_capture_hook = olpc_xo_capture_hook;
	spec->dc_mode_path = snd_hda_add_new_path(codec, 0x1e, 0x14, 0);

	snd_hda_add_new_ctls(codec, olpc_xo_mixers);

	/* OLPC's microphone port is DC coupled for use with external sensors,
	 * therefore we use a 50% mic bias in order to center the input signal
	 * with the DC input range of the codec.
	 */
	snd_hda_codec_set_pin_target(codec, 0x1a, PIN_VREF50);

	/* override mic boost control */
	for (i = 0; i < spec->gen.kctls.used; i++) {
		struct snd_kcontrol_new *kctl =
			snd_array_elem(&spec->gen.kctls, i);
		if (!strcmp(kctl->name, "Mic Boost Volume")) {
			kctl->put = olpc_xo_mic_boost_put;
			break;
		}
	}
}

static void cxt_fixup_mute_led_eapd(struct hda_codec *codec,
				    const struct hda_fixup *fix, int action)
{
	struct conexant_spec *spec = codec->spec;

	if (action == HDA_FIXUP_ACT_PRE_PROBE) {
		spec->mute_led_eapd = 0x1b;
		spec->dynamic_eapd = 1;
		spec->gen.vmaster_mute.hook = cx_auto_vmaster_hook_mute_led;
	}
}

/*
 * Fix max input level on mixer widget to 0dB
 * (originally it has 0x2b steps with 0dB offset 0x14)
 */
static void cxt_fixup_cap_mix_amp(struct hda_codec *codec,
				  const struct hda_fixup *fix, int action)
{
	snd_hda_override_amp_caps(codec, 0x17, HDA_INPUT,
				  (0x14 << AC_AMPCAP_OFFSET_SHIFT) |
				  (0x14 << AC_AMPCAP_NUM_STEPS_SHIFT) |
				  (0x05 << AC_AMPCAP_STEP_SIZE_SHIFT) |
				  (1 << AC_AMPCAP_MUTE_SHIFT));
}

/*
 * Fix max input level on mixer widget to 0dB
 * (originally it has 0x1e steps with 0 dB offset 0x17)
 */
static void cxt_fixup_cap_mix_amp_5047(struct hda_codec *codec,
				  const struct hda_fixup *fix, int action)
{
	snd_hda_override_amp_caps(codec, 0x10, HDA_INPUT,
				  (0x17 << AC_AMPCAP_OFFSET_SHIFT) |
				  (0x17 << AC_AMPCAP_NUM_STEPS_SHIFT) |
				  (0x05 << AC_AMPCAP_STEP_SIZE_SHIFT) |
				  (1 << AC_AMPCAP_MUTE_SHIFT));
}

/* ThinkPad X200 & co with cxt5051 */
static const struct hda_pintbl cxt_pincfg_lenovo_x200[] = {
	{ 0x16, 0x042140ff }, /* HP (seq# overridden) */
	{ 0x17, 0x21a11000 }, /* dock-mic */
	{ 0x19, 0x2121103f }, /* dock-HP */
	{ 0x1c, 0x21440100 }, /* dock SPDIF out */
	{}
};

/* ThinkPad 410/420/510/520, X201 & co with cxt5066 */
static const struct hda_pintbl cxt_pincfg_lenovo_tp410[] = {
	{ 0x19, 0x042110ff }, /* HP (seq# overridden) */
	{ 0x1a, 0x21a190f0 }, /* dock-mic */
	{ 0x1c, 0x212140ff }, /* dock-HP */
	{}
};

/* Lemote A1004/A1205 with cxt5066 */
static const struct hda_pintbl cxt_pincfg_lemote[] = {
	{ 0x1a, 0x90a10020 }, /* Internal mic */
	{ 0x1b, 0x03a11020 }, /* External mic */
	{ 0x1d, 0x400101f0 }, /* Not used */
	{ 0x1e, 0x40a701f0 }, /* Not used */
	{ 0x20, 0x404501f0 }, /* Not used */
	{ 0x22, 0x404401f0 }, /* Not used */
	{ 0x23, 0x40a701f0 }, /* Not used */
	{}
};

static const struct hda_fixup cxt_fixups[] = {
	[CXT_PINCFG_LENOVO_X200] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = cxt_pincfg_lenovo_x200,
	},
	[CXT_PINCFG_LENOVO_TP410] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = cxt_pincfg_lenovo_tp410,
		.chained = true,
		.chain_id = CXT_FIXUP_THINKPAD_ACPI,
	},
	[CXT_PINCFG_LEMOTE_A1004] = {
		.type = HDA_FIXUP_PINS,
		.chained = true,
		.chain_id = CXT_FIXUP_INC_MIC_BOOST,
		.v.pins = cxt_pincfg_lemote,
	},
	[CXT_PINCFG_LEMOTE_A1205] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = cxt_pincfg_lemote,
	},
	[CXT_PINCFG_COMPAQ_CQ60] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = (const struct hda_pintbl[]) {
			/* 0x17 was falsely set up as a mic, it should 0x1d */
			{ 0x17, 0x400001f0 },
			{ 0x1d, 0x97a70120 },
			{ }
		}
	},
	[CXT_FIXUP_STEREO_DMIC] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_stereo_dmic,
	},
	[CXT_FIXUP_INC_MIC_BOOST] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt5066_increase_mic_boost,
	},
	[CXT_FIXUP_HEADPHONE_MIC_PIN] = {
		.type = HDA_FIXUP_PINS,
		.chained = true,
		.chain_id = CXT_FIXUP_HEADPHONE_MIC,
		.v.pins = (const struct hda_pintbl[]) {
			{ 0x18, 0x03a1913d }, /* use as headphone mic, without its own jack detect */
			{ }
		}
	},
	[CXT_FIXUP_HEADPHONE_MIC] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_headphone_mic,
	},
	[CXT_FIXUP_GPIO1] = {
		.type = HDA_FIXUP_VERBS,
		.v.verbs = (const struct hda_verb[]) {
			{ 0x01, AC_VERB_SET_GPIO_MASK, 0x01 },
			{ 0x01, AC_VERB_SET_GPIO_DIRECTION, 0x01 },
			{ 0x01, AC_VERB_SET_GPIO_DATA, 0x01 },
			{ }
		},
	},
	[CXT_FIXUP_ASPIRE_DMIC] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_stereo_dmic,
		.chained = true,
		.chain_id = CXT_FIXUP_GPIO1,
	},
	[CXT_FIXUP_THINKPAD_ACPI] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = hda_fixup_thinkpad_acpi,
	},
	[CXT_FIXUP_OLPC_XO] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_olpc_xo,
	},
	[CXT_FIXUP_CAP_MIX_AMP] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_cap_mix_amp,
	},
	[CXT_FIXUP_TOSHIBA_P105] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = (const struct hda_pintbl[]) {
			{ 0x10, 0x961701f0 }, /* speaker/hp */
			{ 0x12, 0x02a1901e }, /* ext mic */
			{ 0x14, 0x95a70110 }, /* int mic */
			{}
		},
	},
	[CXT_FIXUP_HP_530] = {
		.type = HDA_FIXUP_PINS,
		.v.pins = (const struct hda_pintbl[]) {
			{ 0x12, 0x90a60160 }, /* int mic */
			{}
		},
		.chained = true,
		.chain_id = CXT_FIXUP_CAP_MIX_AMP,
	},
	[CXT_FIXUP_CAP_MIX_AMP_5047] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_cap_mix_amp_5047,
	},
	[CXT_FIXUP_MUTE_LED_EAPD] = {
		.type = HDA_FIXUP_FUNC,
		.v.func = cxt_fixup_mute_led_eapd,
	},
};

static const struct snd_pci_quirk cxt5045_fixups[] = {
	SND_PCI_QUIRK(0x103c, 0x30d5, "HP 530", CXT_FIXUP_HP_530),
	SND_PCI_QUIRK(0x1179, 0xff31, "Toshiba P105", CXT_FIXUP_TOSHIBA_P105),
	/* HP, Packard Bell, Fujitsu-Siemens & Lenovo laptops have
	 * really bad sound over 0dB on NID 0x17.
	 */
	SND_PCI_QUIRK_VENDOR(0x103c, "HP", CXT_FIXUP_CAP_MIX_AMP),
	SND_PCI_QUIRK_VENDOR(0x1631, "Packard Bell", CXT_FIXUP_CAP_MIX_AMP),
	SND_PCI_QUIRK_VENDOR(0x1734, "Fujitsu", CXT_FIXUP_CAP_MIX_AMP),
	SND_PCI_QUIRK_VENDOR(0x17aa, "Lenovo", CXT_FIXUP_CAP_MIX_AMP),
	{}
};

static const struct hda_model_fixup cxt5045_fixup_models[] = {
	{ .id = CXT_FIXUP_CAP_MIX_AMP, .name = "cap-mix-amp" },
	{ .id = CXT_FIXUP_TOSHIBA_P105, .name = "toshiba-p105" },
	{ .id = CXT_FIXUP_HP_530, .name = "hp-530" },
	{}
};

static const struct snd_pci_quirk cxt5047_fixups[] = {
	/* HP laptops have really bad sound over 0 dB on NID 0x10.
	 */
	SND_PCI_QUIRK_VENDOR(0x103c, "HP", CXT_FIXUP_CAP_MIX_AMP_5047),
	{}
};

static const struct hda_model_fixup cxt5047_fixup_models[] = {
	{ .id = CXT_FIXUP_CAP_MIX_AMP_5047, .name = "cap-mix-amp" },
	{}
};

static const struct snd_pci_quirk cxt5051_fixups[] = {
	SND_PCI_QUIRK(0x103c, 0x360b, "Compaq CQ60", CXT_PINCFG_COMPAQ_CQ60),
	SND_PCI_QUIRK(0x17aa, 0x20f2, "Lenovo X200", CXT_PINCFG_LENOVO_X200),
	{}
};

static const struct hda_model_fixup cxt5051_fixup_models[] = {
	{ .id = CXT_PINCFG_LENOVO_X200, .name = "lenovo-x200" },
	{}
};

static const struct snd_pci_quirk cxt5066_fixups[] = {
	SND_PCI_QUIRK(0x1025, 0x0543, "Acer Aspire One 522", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK(0x1025, 0x054c, "Acer Aspire 3830TG", CXT_FIXUP_ASPIRE_DMIC),
	SND_PCI_QUIRK(0x1025, 0x054f, "Acer Aspire 4830T", CXT_FIXUP_ASPIRE_DMIC),
	SND_PCI_QUIRK(0x1043, 0x138d, "Asus", CXT_FIXUP_HEADPHONE_MIC_PIN),
	SND_PCI_QUIRK(0x152d, 0x0833, "OLPC XO-1.5", CXT_FIXUP_OLPC_XO),
	SND_PCI_QUIRK(0x17aa, 0x20f2, "Lenovo T400", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x215e, "Lenovo T410", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x215f, "Lenovo T510", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x21ce, "Lenovo T420", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x21cf, "Lenovo T520", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x21da, "Lenovo X220", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x21db, "Lenovo X220-tablet", CXT_PINCFG_LENOVO_TP410),
	SND_PCI_QUIRK(0x17aa, 0x38af, "Lenovo IdeaPad Z560", CXT_FIXUP_MUTE_LED_EAPD),
	SND_PCI_QUIRK(0x17aa, 0x390b, "Lenovo G50-80", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK(0x17aa, 0x3975, "Lenovo U300s", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK(0x17aa, 0x3977, "Lenovo IdeaPad U310", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK(0x17aa, 0x3978, "Lenovo G50-70", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK(0x17aa, 0x397b, "Lenovo S205", CXT_FIXUP_STEREO_DMIC),
	SND_PCI_QUIRK_VENDOR(0x17aa, "Thinkpad", CXT_FIXUP_THINKPAD_ACPI),
	SND_PCI_QUIRK(0x1c06, 0x2011, "Lemote A1004", CXT_PINCFG_LEMOTE_A1004),
	SND_PCI_QUIRK(0x1c06, 0x2012, "Lemote A1205", CXT_PINCFG_LEMOTE_A1205),
	{}
};

static const struct hda_model_fixup cxt5066_fixup_models[] = {
	{ .id = CXT_FIXUP_STEREO_DMIC, .name = "stereo-dmic" },
	{ .id = CXT_FIXUP_GPIO1, .name = "gpio1" },
	{ .id = CXT_FIXUP_HEADPHONE_MIC_PIN, .name = "headphone-mic-pin" },
	{ .id = CXT_PINCFG_LENOVO_TP410, .name = "tp410" },
	{ .id = CXT_FIXUP_THINKPAD_ACPI, .name = "thinkpad" },
	{ .id = CXT_PINCFG_LEMOTE_A1004, .name = "lemote-a1004" },
	{ .id = CXT_PINCFG_LEMOTE_A1205, .name = "lemote-a1205" },
	{ .id = CXT_FIXUP_OLPC_XO, .name = "olpc-xo" },
	{ .id = CXT_FIXUP_MUTE_LED_EAPD, .name = "mute-led-eapd" },
	{}
};

/* add "fake" mute amp-caps to DACs on cx5051 so that mixer mute switches
 * can be created (bko#42825)
 */
static void add_cx5051_fake_mutes(struct hda_codec *codec)
{
	struct conexant_spec *spec = codec->spec;
	static hda_nid_t out_nids[] = {
		0x10, 0x11, 0
	};
	hda_nid_t *p;

	for (p = out_nids; *p; p++)
		snd_hda_override_amp_caps(codec, *p, HDA_OUTPUT,
					  AC_AMPCAP_MIN_MUTE |
					  query_amp_caps(codec, *p, HDA_OUTPUT));
	spec->gen.dac_min_mute = true;
}

static int patch_conexant_auto(struct hda_codec *codec)
{
	struct conexant_spec *spec;
	int err;

	codec_info(codec, "%s: BIOS auto-probing.\n", codec->core.chip_name);

	spec = kzalloc(sizeof(*spec), GFP_KERNEL);
	if (!spec)
		return -ENOMEM;
	snd_hda_gen_spec_init(&spec->gen);
	codec->spec = spec;

	cx_auto_parse_beep(codec);
	cx_auto_parse_eapd(codec);
	spec->gen.own_eapd_ctl = 1;
	if (spec->dynamic_eapd)
		spec->gen.vmaster_mute.hook = cx_auto_vmaster_hook;

	switch (codec->core.vendor_id) {
	case 0x14f15045:
		codec->single_adc_amp = 1;
		spec->gen.mixer_nid = 0x17;
		spec->gen.add_stereo_mix_input = HDA_HINT_STEREO_MIX_AUTO;
		snd_hda_pick_fixup(codec, cxt5045_fixup_models,
				   cxt5045_fixups, cxt_fixups);
		break;
	case 0x14f15047:
		codec->pin_amp_workaround = 1;
		spec->gen.mixer_nid = 0x19;
		spec->gen.add_stereo_mix_input = HDA_HINT_STEREO_MIX_AUTO;
		snd_hda_pick_fixup(codec, cxt5047_fixup_models,
				   cxt5047_fixups, cxt_fixups);
		break;
	case 0x14f15051:
		add_cx5051_fake_mutes(codec);
		codec->pin_amp_workaround = 1;
		snd_hda_pick_fixup(codec, cxt5051_fixup_models,
				   cxt5051_fixups, cxt_fixups);
		break;
	default:
		codec->pin_amp_workaround = 1;
		snd_hda_pick_fixup(codec, cxt5066_fixup_models,
				   cxt5066_fixups, cxt_fixups);
		break;
	}

	/* Show mute-led control only on HP laptops
	 * This is a sort of white-list: on HP laptops, EAPD corresponds
	 * only to the mute-LED without actualy amp function.  Meanwhile,
	 * others may use EAPD really as an amp switch, so it might be
	 * not good to expose it blindly.
	 */
	switch (codec->core.subsystem_id >> 16) {
	case 0x103c:
		spec->gen.vmaster_mute_enum = 1;
		break;
	}

	snd_hda_apply_fixup(codec, HDA_FIXUP_ACT_PRE_PROBE);

	err = snd_hda_parse_pin_defcfg(codec, &spec->gen.autocfg, NULL,
				       spec->parse_flags);
	if (err < 0)
		goto error;

	err = snd_hda_gen_parse_auto_config(codec, &spec->gen.autocfg);
	if (err < 0)
		goto error;

	codec->patch_ops = cx_auto_patch_ops;

	/* Some laptops with Conexant chips show stalls in S3 resume,
	 * which falls into the single-cmd mode.
	 * Better to make reset, then.
	 */
	if (!codec->bus->core.sync_write) {
		codec_info(codec,
			   "Enable sync_write for stable communication\n");
		codec->bus->core.sync_write = 1;
		codec->bus->allow_bus_reset = 1;
	}

	snd_hda_apply_fixup(codec, HDA_FIXUP_ACT_PROBE);

	return 0;

 error:
	cx_auto_free(codec);
	return err;
}

/*
 */

static const struct hda_codec_preset snd_hda_preset_conexant[] = {
	{ .id = 0x14f15045, .name = "CX20549 (Venice)",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15047, .name = "CX20551 (Waikiki)",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15051, .name = "CX20561 (Hermosa)",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15066, .name = "CX20582 (Pebble)",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15067, .name = "CX20583 (Pebble HSF)",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15068, .name = "CX20584",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15069, .name = "CX20585",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f1506c, .name = "CX20588",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f1506e, .name = "CX20590",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15097, .name = "CX20631",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15098, .name = "CX20632",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150a1, .name = "CX20641",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150a2, .name = "CX20642",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150ab, .name = "CX20651",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150ac, .name = "CX20652",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150b8, .name = "CX20664",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150b9, .name = "CX20665",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150f1, .name = "CX20721",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150f2, .name = "CX20722",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150f3, .name = "CX20723",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f150f4, .name = "CX20724",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f1510f, .name = "CX20751/2",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15110, .name = "CX20751/2",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15111, .name = "CX20753/4",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15113, .name = "CX20755",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15114, .name = "CX20756",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f15115, .name = "CX20757",
	  .patch = patch_conexant_auto },
	{ .id = 0x14f151d7, .name = "CX20952",
	  .patch = patch_conexant_auto },
	{} /* terminator */
};

MODULE_ALIAS("snd-hda-codec-id:14f15045");
MODULE_ALIAS("snd-hda-codec-id:14f15047");
MODULE_ALIAS("snd-hda-codec-id:14f15051");
MODULE_ALIAS("snd-hda-codec-id:14f15066");
MODULE_ALIAS("snd-hda-codec-id:14f15067");
MODULE_ALIAS("snd-hda-codec-id:14f15068");
MODULE_ALIAS("snd-hda-codec-id:14f15069");
MODULE_ALIAS("snd-hda-codec-id:14f1506c");
MODULE_ALIAS("snd-hda-codec-id:14f1506e");
MODULE_ALIAS("snd-hda-codec-id:14f15097");
MODULE_ALIAS("snd-hda-codec-id:14f15098");
MODULE_ALIAS("snd-hda-codec-id:14f150a1");
MODULE_ALIAS("snd-hda-codec-id:14f150a2");
MODULE_ALIAS("snd-hda-codec-id:14f150ab");
MODULE_ALIAS("snd-hda-codec-id:14f150ac");
MODULE_ALIAS("snd-hda-codec-id:14f150b8");
MODULE_ALIAS("snd-hda-codec-id:14f150b9");
MODULE_ALIAS("snd-hda-codec-id:14f150f1");
MODULE_ALIAS("snd-hda-codec-id:14f150f2");
MODULE_ALIAS("snd-hda-codec-id:14f150f3");
MODULE_ALIAS("snd-hda-codec-id:14f150f4");
MODULE_ALIAS("snd-hda-codec-id:14f1510f");
MODULE_ALIAS("snd-hda-codec-id:14f15110");
MODULE_ALIAS("snd-hda-codec-id:14f15111");
MODULE_ALIAS("snd-hda-codec-id:14f15113");
MODULE_ALIAS("snd-hda-codec-id:14f15114");
MODULE_ALIAS("snd-hda-codec-id:14f15115");
MODULE_ALIAS("snd-hda-codec-id:14f151d7");

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Conexant HD-audio codec");

static struct hda_codec_driver conexant_driver = {
	.preset = snd_hda_preset_conexant,
};

module_hda_codec_driver(conexant_driver);
