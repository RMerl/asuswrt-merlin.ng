/*
 * Synaptics TouchPad PS/2 mouse driver
 *
 *   2003 Dmitry Torokhov <dtor@mail.ru>
 *     Added support for pass-through port. Special thanks to Peter Berg Larsen
 *     for explaining various Synaptics quirks.
 *
 *   2003 Peter Osterlund <petero2@telia.com>
 *     Ported to 2.5 input device infrastructure.
 *
 *   Copyright (C) 2001 Stefan Gmeiner <riddlebox@freesurf.ch>
 *     start merging tpconfig and gpm code to a xfree-input module
 *     adding some changes and extensions (ex. 3rd and 4th button)
 *
 *   Copyright (c) 1997 C. Scott Ananian <cananian@alumni.priceton.edu>
 *   Copyright (c) 1998-2000 Bruce Kalk <kall@compass.com>
 *     code for the special synaptics commands (from the tpconfig-source)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * Trademarks are the property of their respective owners.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/input/mt.h>
#include <linux/serio.h>
#include <linux/libps2.h>
#include <linux/slab.h>
#include "psmouse.h"
#include "synaptics.h"

/*
 * The x/y limits are taken from the Synaptics TouchPad interfacing Guide,
 * section 2.3.2, which says that they should be valid regardless of the
 * actual size of the sensor.
 * Note that newer firmware allows querying device for maximum useable
 * coordinates.
 */
#define XMIN 0
#define XMAX 6143
#define YMIN 0
#define YMAX 6143
#define XMIN_NOMINAL 1472
#define XMAX_NOMINAL 5472
#define YMIN_NOMINAL 1408
#define YMAX_NOMINAL 4448

/* Size in bits of absolute position values reported by the hardware */
#define ABS_POS_BITS 13

/*
 * These values should represent the absolute maximum value that will
 * be reported for a positive position value. Some Synaptics firmware
 * uses this value to indicate a finger near the edge of the touchpad
 * whose precise position cannot be determined.
 *
 * At least one touchpad is known to report positions in excess of this
 * value which are actually negative values truncated to the 13-bit
 * reporting range. These values have never been observed to be lower
 * than 8184 (i.e. -8), so we treat all values greater than 8176 as
 * negative and any other value as positive.
 */
#define X_MAX_POSITIVE 8176
#define Y_MAX_POSITIVE 8176

/* maximum ABS_MT_POSITION displacement (in mm) */
#define DMAX 10

/*****************************************************************************
 *	Stuff we need even when we do not want native Synaptics support
 ****************************************************************************/

/*
 * Set the synaptics touchpad mode byte by special commands
 */
static int synaptics_mode_cmd(struct psmouse *psmouse, unsigned char mode)
{
	unsigned char param[1];

	if (psmouse_sliced_command(psmouse, mode))
		return -1;
	param[0] = SYN_PS_SET_MODE2;
	if (ps2_command(&psmouse->ps2dev, param, PSMOUSE_CMD_SETRATE))
		return -1;
	return 0;
}

int synaptics_detect(struct psmouse *psmouse, bool set_properties)
{
	struct ps2dev *ps2dev = &psmouse->ps2dev;
	unsigned char param[4];

	param[0] = 0;

	ps2_command(ps2dev, param, PSMOUSE_CMD_SETRES);
	ps2_command(ps2dev, param, PSMOUSE_CMD_SETRES);
	ps2_command(ps2dev, param, PSMOUSE_CMD_SETRES);
	ps2_command(ps2dev, param, PSMOUSE_CMD_SETRES);
	ps2_command(ps2dev, param, PSMOUSE_CMD_GETINFO);

	if (param[1] != 0x47)
		return -ENODEV;

	if (set_properties) {
		psmouse->vendor = "Synaptics";
		psmouse->name = "TouchPad";
	}

	return 0;
}

void synaptics_reset(struct psmouse *psmouse)
{
	/* reset touchpad back to relative mode, gestures enabled */
	synaptics_mode_cmd(psmouse, 0);
}

#ifdef CONFIG_MOUSE_PS2_SYNAPTICS

static bool cr48_profile_sensor;

#define ANY_BOARD_ID 0
struct min_max_quirk {
	const char * const *pnp_ids;
	struct {
		unsigned long int min, max;
	} board_id;
	int x_min, x_max, y_min, y_max;
};

static const struct min_max_quirk min_max_pnpid_table[] = {
	{
		(const char * const []){"LEN0033", NULL},
		{ANY_BOARD_ID, ANY_BOARD_ID},
		1024, 5052, 2258, 4832
	},
	{
		(const char * const []){"LEN0042", NULL},
		{ANY_BOARD_ID, ANY_BOARD_ID},
		1232, 5710, 1156, 4696
	},
	{
		(const char * const []){"LEN0034", "LEN0036", "LEN0037",
					"LEN0039", "LEN2002", "LEN2004",
					NULL},
		{ANY_BOARD_ID, 2961},
		1024, 5112, 2024, 4832
	},
	{
		(const char * const []){"LEN2000", NULL},
		{ANY_BOARD_ID, ANY_BOARD_ID},
		1024, 5113, 2021, 4832
	},
	{
		(const char * const []){"LEN2001", NULL},
		{ANY_BOARD_ID, ANY_BOARD_ID},
		1024, 5022, 2508, 4832
	},
	{
		(const char * const []){"LEN2006", NULL},
		{2691, 2691},
		1024, 5045, 2457, 4832
	},
	{
		(const char * const []){"LEN2006", NULL},
		{ANY_BOARD_ID, ANY_BOARD_ID},
		1264, 5675, 1171, 4688
	},
	{ }
};

/* This list has been kindly provided by Synaptics. */
static const char * const topbuttonpad_pnp_ids[] = {
	"LEN0017",
	"LEN0018",
	"LEN0019",
	"LEN0023",
	"LEN002A",
	"LEN002B",
	"LEN002C",
	"LEN002D",
	"LEN002E",
	"LEN0033", /* Helix */
	"LEN0034", /* T431s, L440, L540, T540, W540, X1 Carbon 2nd */
	"LEN0035", /* X240 */
	"LEN0036", /* T440 */
	"LEN0037", /* X1 Carbon 2nd */
	"LEN0038",
	"LEN0039", /* T440s */
	"LEN0041",
	"LEN0042", /* Yoga */
	"LEN0045",
	"LEN0047",
	"LEN0049",
	"LEN2000", /* S540 */
	"LEN2001", /* Edge E431 */
	"LEN2002", /* Edge E531 */
	"LEN2003",
	"LEN2004", /* L440 */
	"LEN2005",
	"LEN2006", /* Edge E440/E540 */
	"LEN2007",
	"LEN2008",
	"LEN2009",
	"LEN200A",
	"LEN200B",
	NULL
};

/* This list has been kindly provided by Synaptics. */
static const char * const forcepad_pnp_ids[] = {
	"SYN300D",
	"SYN3014",
	NULL
};

/*****************************************************************************
 *	Synaptics communications functions
 ****************************************************************************/

/*
 * Synaptics touchpads report the y coordinate from bottom to top, which is
 * opposite from what userspace expects.
 * This function is used to invert y before reporting.
 */
static int synaptics_invert_y(int y)
{
	return YMAX_NOMINAL + YMIN_NOMINAL - y;
}

/*
 * Send a command to the synpatics touchpad by special commands
 */
static int synaptics_send_cmd(struct psmouse *psmouse, unsigned char c, unsigned char *param)
{
	if (psmouse_sliced_command(psmouse, c))
		return -1;
	if (ps2_command(&psmouse->ps2dev, param, PSMOUSE_CMD_GETINFO))
		return -1;
	return 0;
}

/*
 * Read the model-id bytes from the touchpad
 * see also SYN_MODEL_* macros
 */
static int synaptics_model_id(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char mi[3];

	if (synaptics_send_cmd(psmouse, SYN_QUE_MODEL, mi))
		return -1;
	priv->model_id = (mi[0]<<16) | (mi[1]<<8) | mi[2];
	return 0;
}

static int synaptics_more_extended_queries(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char buf[3];

	if (synaptics_send_cmd(psmouse, SYN_QUE_MEXT_CAPAB_10, buf))
		return -1;

	priv->ext_cap_10 = (buf[0]<<16) | (buf[1]<<8) | buf[2];

	return 0;
}

/*
 * Read the board id and the "More Extended Queries" from the touchpad
 * The board id is encoded in the "QUERY MODES" response
 */
static int synaptics_query_modes(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char bid[3];

	/* firmwares prior 7.5 have no board_id encoded */
	if (SYN_ID_FULL(priv->identity) < 0x705)
		return 0;

	if (synaptics_send_cmd(psmouse, SYN_QUE_MODES, bid))
		return -1;
	priv->board_id = ((bid[0] & 0xfc) << 6) | bid[1];

	if (SYN_MEXT_CAP_BIT(bid[0]))
		return synaptics_more_extended_queries(psmouse);

	return 0;
}

/*
 * Read the firmware id from the touchpad
 */
static int synaptics_firmware_id(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char fwid[3];

	if (synaptics_send_cmd(psmouse, SYN_QUE_FIRMWARE_ID, fwid))
		return -1;
	priv->firmware_id = (fwid[0] << 16) | (fwid[1] << 8) | fwid[2];
	return 0;
}

/*
 * Read the capability-bits from the touchpad
 * see also the SYN_CAP_* macros
 */
static int synaptics_capability(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char cap[3];

	if (synaptics_send_cmd(psmouse, SYN_QUE_CAPABILITIES, cap))
		return -1;
	priv->capabilities = (cap[0] << 16) | (cap[1] << 8) | cap[2];
	priv->ext_cap = priv->ext_cap_0c = 0;

	/*
	 * Older firmwares had submodel ID fixed to 0x47
	 */
	if (SYN_ID_FULL(priv->identity) < 0x705 &&
	    SYN_CAP_SUBMODEL_ID(priv->capabilities) != 0x47) {
		return -1;
	}

	/*
	 * Unless capExtended is set the rest of the flags should be ignored
	 */
	if (!SYN_CAP_EXTENDED(priv->capabilities))
		priv->capabilities = 0;

	if (SYN_EXT_CAP_REQUESTS(priv->capabilities) >= 1) {
		if (synaptics_send_cmd(psmouse, SYN_QUE_EXT_CAPAB, cap)) {
			psmouse_warn(psmouse,
				     "device claims to have extended capabilities, but I'm not able to read them.\n");
		} else {
			priv->ext_cap = (cap[0] << 16) | (cap[1] << 8) | cap[2];

			/*
			 * if nExtBtn is greater than 8 it should be considered
			 * invalid and treated as 0
			 */
			if (SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap) > 8)
				priv->ext_cap &= 0xff0fff;
		}
	}

	if (SYN_EXT_CAP_REQUESTS(priv->capabilities) >= 4) {
		if (synaptics_send_cmd(psmouse, SYN_QUE_EXT_CAPAB_0C, cap)) {
			psmouse_warn(psmouse,
				     "device claims to have extended capability 0x0c, but I'm not able to read it.\n");
		} else {
			priv->ext_cap_0c = (cap[0] << 16) | (cap[1] << 8) | cap[2];
		}
	}

	return 0;
}

/*
 * Identify Touchpad
 * See also the SYN_ID_* macros
 */
static int synaptics_identify(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char id[3];

	if (synaptics_send_cmd(psmouse, SYN_QUE_IDENTIFY, id))
		return -1;
	priv->identity = (id[0]<<16) | (id[1]<<8) | id[2];
	if (SYN_ID_IS_SYNAPTICS(priv->identity))
		return 0;
	return -1;
}

/*
 * Read touchpad resolution and maximum reported coordinates
 * Resolution is left zero if touchpad does not support the query
 */

static int synaptics_resolution(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned char resp[3];

	if (SYN_ID_MAJOR(priv->identity) < 4)
		return 0;

	if (synaptics_send_cmd(psmouse, SYN_QUE_RESOLUTION, resp) == 0) {
		if (resp[0] != 0 && (resp[1] & 0x80) && resp[2] != 0) {
			priv->x_res = resp[0]; /* x resolution in units/mm */
			priv->y_res = resp[2]; /* y resolution in units/mm */
		}
	}

	if (SYN_EXT_CAP_REQUESTS(priv->capabilities) >= 5 &&
	    SYN_CAP_MAX_DIMENSIONS(priv->ext_cap_0c)) {
		if (synaptics_send_cmd(psmouse, SYN_QUE_EXT_MAX_COORDS, resp)) {
			psmouse_warn(psmouse,
				     "device claims to have max coordinates query, but I'm not able to read it.\n");
		} else {
			priv->x_max = (resp[0] << 5) | ((resp[1] & 0x0f) << 1);
			priv->y_max = (resp[2] << 5) | ((resp[1] & 0xf0) >> 3);
			psmouse_info(psmouse,
				     "queried max coordinates: x [..%d], y [..%d]\n",
				     priv->x_max, priv->y_max);
		}
	}

	if (SYN_CAP_MIN_DIMENSIONS(priv->ext_cap_0c) &&
	    (SYN_EXT_CAP_REQUESTS(priv->capabilities) >= 7 ||
	     /*
	      * Firmware v8.1 does not report proper number of extended
	      * capabilities, but has been proven to report correct min
	      * coordinates.
	      */
	     SYN_ID_FULL(priv->identity) == 0x801)) {
		if (synaptics_send_cmd(psmouse, SYN_QUE_EXT_MIN_COORDS, resp)) {
			psmouse_warn(psmouse,
				     "device claims to have min coordinates query, but I'm not able to read it.\n");
		} else {
			priv->x_min = (resp[0] << 5) | ((resp[1] & 0x0f) << 1);
			priv->y_min = (resp[2] << 5) | ((resp[1] & 0xf0) >> 3);
			psmouse_info(psmouse,
				     "queried min coordinates: x [%d..], y [%d..]\n",
				     priv->x_min, priv->y_min);
		}
	}

	return 0;
}

/*
 * Apply quirk(s) if the hardware matches
 */

static void synaptics_apply_quirks(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	int i;

	for (i = 0; min_max_pnpid_table[i].pnp_ids; i++) {
		if (!psmouse_matches_pnp_id(psmouse,
					    min_max_pnpid_table[i].pnp_ids))
			continue;

		if (min_max_pnpid_table[i].board_id.min != ANY_BOARD_ID &&
		    priv->board_id < min_max_pnpid_table[i].board_id.min)
			continue;

		if (min_max_pnpid_table[i].board_id.max != ANY_BOARD_ID &&
		    priv->board_id > min_max_pnpid_table[i].board_id.max)
			continue;

		priv->x_min = min_max_pnpid_table[i].x_min;
		priv->x_max = min_max_pnpid_table[i].x_max;
		priv->y_min = min_max_pnpid_table[i].y_min;
		priv->y_max = min_max_pnpid_table[i].y_max;
		psmouse_info(psmouse,
			     "quirked min/max coordinates: x [%d..%d], y [%d..%d]\n",
			     priv->x_min, priv->x_max,
			     priv->y_min, priv->y_max);
		break;
	}
}

static int synaptics_query_hardware(struct psmouse *psmouse)
{
	if (synaptics_identify(psmouse))
		return -1;
	if (synaptics_model_id(psmouse))
		return -1;
	if (synaptics_firmware_id(psmouse))
		return -1;
	if (synaptics_query_modes(psmouse))
		return -1;
	if (synaptics_capability(psmouse))
		return -1;
	if (synaptics_resolution(psmouse))
		return -1;

	synaptics_apply_quirks(psmouse);

	return 0;
}

static int synaptics_set_advanced_gesture_mode(struct psmouse *psmouse)
{
	static unsigned char param = 0xc8;
	struct synaptics_data *priv = psmouse->private;

	if (!(SYN_CAP_ADV_GESTURE(priv->ext_cap_0c) ||
	      SYN_CAP_IMAGE_SENSOR(priv->ext_cap_0c)))
		return 0;

	if (psmouse_sliced_command(psmouse, SYN_QUE_MODEL))
		return -1;

	if (ps2_command(&psmouse->ps2dev, &param, PSMOUSE_CMD_SETRATE))
		return -1;

	/* Advanced gesture mode also sends multi finger data */
	priv->capabilities |= BIT(1);

	return 0;
}

static int synaptics_set_mode(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;

	priv->mode = 0;
	if (priv->absolute_mode)
		priv->mode |= SYN_BIT_ABSOLUTE_MODE;
	if (priv->disable_gesture)
		priv->mode |= SYN_BIT_DISABLE_GESTURE;
	if (psmouse->rate >= 80)
		priv->mode |= SYN_BIT_HIGH_RATE;
	if (SYN_CAP_EXTENDED(priv->capabilities))
		priv->mode |= SYN_BIT_W_MODE;

	if (synaptics_mode_cmd(psmouse, priv->mode))
		return -1;

	if (priv->absolute_mode &&
	    synaptics_set_advanced_gesture_mode(psmouse)) {
		psmouse_err(psmouse, "Advanced gesture mode init failed.\n");
		return -1;
	}

	return 0;
}

static void synaptics_set_rate(struct psmouse *psmouse, unsigned int rate)
{
	struct synaptics_data *priv = psmouse->private;

	if (rate >= 80) {
		priv->mode |= SYN_BIT_HIGH_RATE;
		psmouse->rate = 80;
	} else {
		priv->mode &= ~SYN_BIT_HIGH_RATE;
		psmouse->rate = 40;
	}

	synaptics_mode_cmd(psmouse, priv->mode);
}

/*****************************************************************************
 *	Synaptics pass-through PS/2 port support
 ****************************************************************************/
static int synaptics_pt_write(struct serio *serio, unsigned char c)
{
	struct psmouse *parent = serio_get_drvdata(serio->parent);
	char rate_param = SYN_PS_CLIENT_CMD; /* indicates that we want pass-through port */

	if (psmouse_sliced_command(parent, c))
		return -1;
	if (ps2_command(&parent->ps2dev, &rate_param, PSMOUSE_CMD_SETRATE))
		return -1;
	return 0;
}

static int synaptics_pt_start(struct serio *serio)
{
	struct psmouse *parent = serio_get_drvdata(serio->parent);
	struct synaptics_data *priv = parent->private;

	serio_pause_rx(parent->ps2dev.serio);
	priv->pt_port = serio;
	serio_continue_rx(parent->ps2dev.serio);

	return 0;
}

static void synaptics_pt_stop(struct serio *serio)
{
	struct psmouse *parent = serio_get_drvdata(serio->parent);
	struct synaptics_data *priv = parent->private;

	serio_pause_rx(parent->ps2dev.serio);
	priv->pt_port = NULL;
	serio_continue_rx(parent->ps2dev.serio);
}

static int synaptics_is_pt_packet(unsigned char *buf)
{
	return (buf[0] & 0xFC) == 0x84 && (buf[3] & 0xCC) == 0xC4;
}

static void synaptics_pass_pt_packet(struct psmouse *psmouse,
				     struct serio *ptport,
				     unsigned char *packet)
{
	struct synaptics_data *priv = psmouse->private;
	struct psmouse *child = serio_get_drvdata(ptport);

	if (child && child->state == PSMOUSE_ACTIVATED) {
		serio_interrupt(ptport, packet[1] | priv->pt_buttons, 0);
		serio_interrupt(ptport, packet[4], 0);
		serio_interrupt(ptport, packet[5], 0);
		if (child->pktsize == 4)
			serio_interrupt(ptport, packet[2], 0);
	} else {
		serio_interrupt(ptport, packet[1], 0);
	}
}

static void synaptics_pt_activate(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	struct psmouse *child = serio_get_drvdata(priv->pt_port);

	/* adjust the touchpad to child's choice of protocol */
	if (child) {
		if (child->pktsize == 4)
			priv->mode |= SYN_BIT_FOUR_BYTE_CLIENT;
		else
			priv->mode &= ~SYN_BIT_FOUR_BYTE_CLIENT;

		if (synaptics_mode_cmd(psmouse, priv->mode))
			psmouse_warn(psmouse,
				     "failed to switch guest protocol\n");
	}
}

static void synaptics_pt_create(struct psmouse *psmouse)
{
	struct serio *serio;

	serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!serio) {
		psmouse_err(psmouse,
			    "not enough memory for pass-through port\n");
		return;
	}

	serio->id.type = SERIO_PS_PSTHRU;
	strlcpy(serio->name, "Synaptics pass-through", sizeof(serio->name));
	strlcpy(serio->phys, "synaptics-pt/serio0", sizeof(serio->name));
	serio->write = synaptics_pt_write;
	serio->start = synaptics_pt_start;
	serio->stop = synaptics_pt_stop;
	serio->parent = psmouse->ps2dev.serio;

	psmouse->pt_activate = synaptics_pt_activate;

	psmouse_info(psmouse, "serio: %s port at %s\n",
		     serio->name, psmouse->phys);
	serio_register_port(serio);
}

/*****************************************************************************
 *	Functions to interpret the absolute mode packets
 ****************************************************************************/

static void synaptics_parse_agm(const unsigned char buf[],
				struct synaptics_data *priv,
				struct synaptics_hw_state *hw)
{
	struct synaptics_hw_state *agm = &priv->agm;
	int agm_packet_type;

	agm_packet_type = (buf[5] & 0x30) >> 4;
	switch (agm_packet_type) {
	case 1:
		/* Gesture packet: (x, y, z) half resolution */
		agm->w = hw->w;
		agm->x = (((buf[4] & 0x0f) << 8) | buf[1]) << 1;
		agm->y = (((buf[4] & 0xf0) << 4) | buf[2]) << 1;
		agm->z = ((buf[3] & 0x30) | (buf[5] & 0x0f)) << 1;
		break;

	case 2:
		/* AGM-CONTACT packet: we are only interested in the count */
		priv->agm_count = buf[1];
		break;

	default:
		break;
	}
}

static void synaptics_parse_ext_buttons(const unsigned char buf[],
					struct synaptics_data *priv,
					struct synaptics_hw_state *hw)
{
	unsigned int ext_bits =
		(SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap) + 1) >> 1;
	unsigned int ext_mask = GENMASK(ext_bits - 1, 0);

	hw->ext_buttons = buf[4] & ext_mask;
	hw->ext_buttons |= (buf[5] & ext_mask) << ext_bits;
}

static int synaptics_parse_hw_state(const unsigned char buf[],
				    struct synaptics_data *priv,
				    struct synaptics_hw_state *hw)
{
	memset(hw, 0, sizeof(struct synaptics_hw_state));

	if (SYN_MODEL_NEWABS(priv->model_id)) {
		hw->w = (((buf[0] & 0x30) >> 2) |
			 ((buf[0] & 0x04) >> 1) |
			 ((buf[3] & 0x04) >> 2));

		if ((SYN_CAP_ADV_GESTURE(priv->ext_cap_0c) ||
			SYN_CAP_IMAGE_SENSOR(priv->ext_cap_0c)) &&
		    hw->w == 2) {
			synaptics_parse_agm(buf, priv, hw);
			return 1;
		}

		hw->x = (((buf[3] & 0x10) << 8) |
			 ((buf[1] & 0x0f) << 8) |
			 buf[4]);
		hw->y = (((buf[3] & 0x20) << 7) |
			 ((buf[1] & 0xf0) << 4) |
			 buf[5]);
		hw->z = buf[2];

		hw->left  = (buf[0] & 0x01) ? 1 : 0;
		hw->right = (buf[0] & 0x02) ? 1 : 0;

		if (priv->is_forcepad) {
			/*
			 * ForcePads, like Clickpads, use middle button
			 * bits to report primary button clicks.
			 * Unfortunately they report primary button not
			 * only when user presses on the pad above certain
			 * threshold, but also when there are more than one
			 * finger on the touchpad, which interferes with
			 * out multi-finger gestures.
			 */
			if (hw->z == 0) {
				/* No contacts */
				priv->press = priv->report_press = false;
			} else if (hw->w >= 4 && ((buf[0] ^ buf[3]) & 0x01)) {
				/*
				 * Single-finger touch with pressure above
				 * the threshold. If pressure stays long
				 * enough, we'll start reporting primary
				 * button. We rely on the device continuing
				 * sending data even if finger does not
				 * move.
				 */
				if  (!priv->press) {
					priv->press_start = jiffies;
					priv->press = true;
				} else if (time_after(jiffies,
						priv->press_start +
							msecs_to_jiffies(50))) {
					priv->report_press = true;
				}
			} else {
				priv->press = false;
			}

			hw->left = priv->report_press;

		} else if (SYN_CAP_CLICKPAD(priv->ext_cap_0c)) {
			/*
			 * Clickpad's button is transmitted as middle button,
			 * however, since it is primary button, we will report
			 * it as BTN_LEFT.
			 */
			hw->left = ((buf[0] ^ buf[3]) & 0x01) ? 1 : 0;

		} else if (SYN_CAP_MIDDLE_BUTTON(priv->capabilities)) {
			hw->middle = ((buf[0] ^ buf[3]) & 0x01) ? 1 : 0;
			if (hw->w == 2)
				hw->scroll = (signed char)(buf[1]);
		}

		if (SYN_CAP_FOUR_BUTTON(priv->capabilities)) {
			hw->up   = ((buf[0] ^ buf[3]) & 0x01) ? 1 : 0;
			hw->down = ((buf[0] ^ buf[3]) & 0x02) ? 1 : 0;
		}

		if (SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap) > 0 &&
		    ((buf[0] ^ buf[3]) & 0x02)) {
			synaptics_parse_ext_buttons(buf, priv, hw);
		}
	} else {
		hw->x = (((buf[1] & 0x1f) << 8) | buf[2]);
		hw->y = (((buf[4] & 0x1f) << 8) | buf[5]);

		hw->z = (((buf[0] & 0x30) << 2) | (buf[3] & 0x3F));
		hw->w = (((buf[1] & 0x80) >> 4) | ((buf[0] & 0x04) >> 1));

		hw->left  = (buf[0] & 0x01) ? 1 : 0;
		hw->right = (buf[0] & 0x02) ? 1 : 0;
	}

	/*
	 * Convert wrap-around values to negative. (X|Y)_MAX_POSITIVE
	 * is used by some firmware to indicate a finger at the edge of
	 * the touchpad whose precise position cannot be determined, so
	 * convert these values to the maximum axis value.
	 */
	if (hw->x > X_MAX_POSITIVE)
		hw->x -= 1 << ABS_POS_BITS;
	else if (hw->x == X_MAX_POSITIVE)
		hw->x = XMAX;

	if (hw->y > Y_MAX_POSITIVE)
		hw->y -= 1 << ABS_POS_BITS;
	else if (hw->y == Y_MAX_POSITIVE)
		hw->y = YMAX;

	return 0;
}

static void synaptics_report_semi_mt_slot(struct input_dev *dev, int slot,
					  bool active, int x, int y)
{
	input_mt_slot(dev, slot);
	input_mt_report_slot_state(dev, MT_TOOL_FINGER, active);
	if (active) {
		input_report_abs(dev, ABS_MT_POSITION_X, x);
		input_report_abs(dev, ABS_MT_POSITION_Y, synaptics_invert_y(y));
	}
}

static void synaptics_report_semi_mt_data(struct input_dev *dev,
					  const struct synaptics_hw_state *a,
					  const struct synaptics_hw_state *b,
					  int num_fingers)
{
	if (num_fingers >= 2) {
		synaptics_report_semi_mt_slot(dev, 0, true, min(a->x, b->x),
					      min(a->y, b->y));
		synaptics_report_semi_mt_slot(dev, 1, true, max(a->x, b->x),
					      max(a->y, b->y));
	} else if (num_fingers == 1) {
		synaptics_report_semi_mt_slot(dev, 0, true, a->x, a->y);
		synaptics_report_semi_mt_slot(dev, 1, false, 0, 0);
	} else {
		synaptics_report_semi_mt_slot(dev, 0, false, 0, 0);
		synaptics_report_semi_mt_slot(dev, 1, false, 0, 0);
	}
}

static void synaptics_report_ext_buttons(struct psmouse *psmouse,
					 const struct synaptics_hw_state *hw)
{
	struct input_dev *dev = psmouse->dev;
	struct synaptics_data *priv = psmouse->private;
	int ext_bits = (SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap) + 1) >> 1;
	char buf[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	int i;

	if (!SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap))
		return;

	/* Bug in FW 8.1 & 8.2, buttons are reported only when ExtBit is 1 */
	if ((SYN_ID_FULL(priv->identity) == 0x801 ||
	     SYN_ID_FULL(priv->identity) == 0x802) &&
	    !((psmouse->packet[0] ^ psmouse->packet[3]) & 0x02))
		return;

	if (!SYN_CAP_EXT_BUTTONS_STICK(priv->ext_cap_10)) {
		for (i = 0; i < ext_bits; i++) {
			input_report_key(dev, BTN_0 + 2 * i,
				hw->ext_buttons & (1 << i));
			input_report_key(dev, BTN_1 + 2 * i,
				hw->ext_buttons & (1 << (i + ext_bits)));
		}
		return;
	}

	/*
	 * This generation of touchpads has the trackstick buttons
	 * physically wired to the touchpad. Re-route them through
	 * the pass-through interface.
	 */
	if (!priv->pt_port)
		return;

	/* The trackstick expects at most 3 buttons */
	priv->pt_buttons = SYN_CAP_EXT_BUTTON_STICK_L(hw->ext_buttons)      |
			   SYN_CAP_EXT_BUTTON_STICK_R(hw->ext_buttons) << 1 |
			   SYN_CAP_EXT_BUTTON_STICK_M(hw->ext_buttons) << 2;

	synaptics_pass_pt_packet(psmouse, priv->pt_port, buf);
}

static void synaptics_report_buttons(struct psmouse *psmouse,
				     const struct synaptics_hw_state *hw)
{
	struct input_dev *dev = psmouse->dev;
	struct synaptics_data *priv = psmouse->private;

	input_report_key(dev, BTN_LEFT, hw->left);
	input_report_key(dev, BTN_RIGHT, hw->right);

	if (SYN_CAP_MIDDLE_BUTTON(priv->capabilities))
		input_report_key(dev, BTN_MIDDLE, hw->middle);

	if (SYN_CAP_FOUR_BUTTON(priv->capabilities)) {
		input_report_key(dev, BTN_FORWARD, hw->up);
		input_report_key(dev, BTN_BACK, hw->down);
	}

	synaptics_report_ext_buttons(psmouse, hw);
}

static void synaptics_report_mt_data(struct psmouse *psmouse,
				     const struct synaptics_hw_state *sgm,
				     int num_fingers)
{
	struct input_dev *dev = psmouse->dev;
	struct synaptics_data *priv = psmouse->private;
	const struct synaptics_hw_state *hw[2] = { sgm, &priv->agm };
	struct input_mt_pos pos[2];
	int slot[2], nsemi, i;

	nsemi = clamp_val(num_fingers, 0, 2);

	for (i = 0; i < nsemi; i++) {
		pos[i].x = hw[i]->x;
		pos[i].y = synaptics_invert_y(hw[i]->y);
	}

	input_mt_assign_slots(dev, slot, pos, nsemi, DMAX * priv->x_res);

	for (i = 0; i < nsemi; i++) {
		input_mt_slot(dev, slot[i]);
		input_mt_report_slot_state(dev, MT_TOOL_FINGER, true);
		input_report_abs(dev, ABS_MT_POSITION_X, pos[i].x);
		input_report_abs(dev, ABS_MT_POSITION_Y, pos[i].y);
		input_report_abs(dev, ABS_MT_PRESSURE, hw[i]->z);
	}

	input_mt_drop_unused(dev);

	/* Don't use active slot count to generate BTN_TOOL events. */
	input_mt_report_pointer_emulation(dev, false);

	/* Send the number of fingers reported by touchpad itself. */
	input_mt_report_finger_count(dev, num_fingers);

	synaptics_report_buttons(psmouse, sgm);

	input_sync(dev);
}

static void synaptics_image_sensor_process(struct psmouse *psmouse,
					   struct synaptics_hw_state *sgm)
{
	struct synaptics_data *priv = psmouse->private;
	int num_fingers;

	/*
	 * Update mt_state using the new finger count and current mt_state.
	 */
	if (sgm->z == 0)
		num_fingers = 0;
	else if (sgm->w >= 4)
		num_fingers = 1;
	else if (sgm->w == 0)
		num_fingers = 2;
	else if (sgm->w == 1)
		num_fingers = priv->agm_count ? priv->agm_count : 3;
	else
		num_fingers = 4;

	/* Send resulting input events to user space */
	synaptics_report_mt_data(psmouse, sgm, num_fingers);
}

/*
 *  called for each full received packet from the touchpad
 */
static void synaptics_process_packet(struct psmouse *psmouse)
{
	struct input_dev *dev = psmouse->dev;
	struct synaptics_data *priv = psmouse->private;
	struct synaptics_hw_state hw;
	int num_fingers;
	int finger_width;

	if (synaptics_parse_hw_state(psmouse->packet, priv, &hw))
		return;

	if (SYN_CAP_IMAGE_SENSOR(priv->ext_cap_0c)) {
		synaptics_image_sensor_process(psmouse, &hw);
		return;
	}

	if (hw.scroll) {
		priv->scroll += hw.scroll;

		while (priv->scroll >= 4) {
			input_report_key(dev, BTN_BACK, !hw.down);
			input_sync(dev);
			input_report_key(dev, BTN_BACK, hw.down);
			input_sync(dev);
			priv->scroll -= 4;
		}
		while (priv->scroll <= -4) {
			input_report_key(dev, BTN_FORWARD, !hw.up);
			input_sync(dev);
			input_report_key(dev, BTN_FORWARD, hw.up);
			input_sync(dev);
			priv->scroll += 4;
		}
		return;
	}

	if (hw.z > 0 && hw.x > 1) {
		num_fingers = 1;
		finger_width = 5;
		if (SYN_CAP_EXTENDED(priv->capabilities)) {
			switch (hw.w) {
			case 0 ... 1:
				if (SYN_CAP_MULTIFINGER(priv->capabilities))
					num_fingers = hw.w + 2;
				break;
			case 2:
				if (SYN_MODEL_PEN(priv->model_id))
					;   /* Nothing, treat a pen as a single finger */
				break;
			case 4 ... 15:
				if (SYN_CAP_PALMDETECT(priv->capabilities))
					finger_width = hw.w;
				break;
			}
		}
	} else {
		num_fingers = 0;
		finger_width = 0;
	}

	if (cr48_profile_sensor) {
		synaptics_report_mt_data(psmouse, &hw, num_fingers);
		return;
	}

	if (SYN_CAP_ADV_GESTURE(priv->ext_cap_0c))
		synaptics_report_semi_mt_data(dev, &hw, &priv->agm,
					      num_fingers);

	/* Post events
	 * BTN_TOUCH has to be first as mousedev relies on it when doing
	 * absolute -> relative conversion
	 */
	if (hw.z > 30) input_report_key(dev, BTN_TOUCH, 1);
	if (hw.z < 25) input_report_key(dev, BTN_TOUCH, 0);

	if (num_fingers > 0) {
		input_report_abs(dev, ABS_X, hw.x);
		input_report_abs(dev, ABS_Y, synaptics_invert_y(hw.y));
	}
	input_report_abs(dev, ABS_PRESSURE, hw.z);

	if (SYN_CAP_PALMDETECT(priv->capabilities))
		input_report_abs(dev, ABS_TOOL_WIDTH, finger_width);

	input_report_key(dev, BTN_TOOL_FINGER, num_fingers == 1);
	if (SYN_CAP_MULTIFINGER(priv->capabilities)) {
		input_report_key(dev, BTN_TOOL_DOUBLETAP, num_fingers == 2);
		input_report_key(dev, BTN_TOOL_TRIPLETAP, num_fingers == 3);
	}

	synaptics_report_buttons(psmouse, &hw);

	input_sync(dev);
}

static int synaptics_validate_byte(struct psmouse *psmouse,
				   int idx, unsigned char pkt_type)
{
	static const unsigned char newabs_mask[]	= { 0xC8, 0x00, 0x00, 0xC8, 0x00 };
	static const unsigned char newabs_rel_mask[]	= { 0xC0, 0x00, 0x00, 0xC0, 0x00 };
	static const unsigned char newabs_rslt[]	= { 0x80, 0x00, 0x00, 0xC0, 0x00 };
	static const unsigned char oldabs_mask[]	= { 0xC0, 0x60, 0x00, 0xC0, 0x60 };
	static const unsigned char oldabs_rslt[]	= { 0xC0, 0x00, 0x00, 0x80, 0x00 };
	const char *packet = psmouse->packet;

	if (idx < 0 || idx > 4)
		return 0;

	switch (pkt_type) {

	case SYN_NEWABS:
	case SYN_NEWABS_RELAXED:
		return (packet[idx] & newabs_rel_mask[idx]) == newabs_rslt[idx];

	case SYN_NEWABS_STRICT:
		return (packet[idx] & newabs_mask[idx]) == newabs_rslt[idx];

	case SYN_OLDABS:
		return (packet[idx] & oldabs_mask[idx]) == oldabs_rslt[idx];

	default:
		psmouse_err(psmouse, "unknown packet type %d\n", pkt_type);
		return 0;
	}
}

static unsigned char synaptics_detect_pkt_type(struct psmouse *psmouse)
{
	int i;

	for (i = 0; i < 5; i++)
		if (!synaptics_validate_byte(psmouse, i, SYN_NEWABS_STRICT)) {
			psmouse_info(psmouse, "using relaxed packet validation\n");
			return SYN_NEWABS_RELAXED;
		}

	return SYN_NEWABS_STRICT;
}

static psmouse_ret_t synaptics_process_byte(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;

	if (psmouse->pktcnt >= 6) { /* Full packet received */
		if (unlikely(priv->pkt_type == SYN_NEWABS))
			priv->pkt_type = synaptics_detect_pkt_type(psmouse);

		if (SYN_CAP_PASS_THROUGH(priv->capabilities) &&
		    synaptics_is_pt_packet(psmouse->packet)) {
			if (priv->pt_port)
				synaptics_pass_pt_packet(psmouse, priv->pt_port,
							 psmouse->packet);
		} else
			synaptics_process_packet(psmouse);

		return PSMOUSE_FULL_PACKET;
	}

	return synaptics_validate_byte(psmouse, psmouse->pktcnt - 1, priv->pkt_type) ?
		PSMOUSE_GOOD_DATA : PSMOUSE_BAD_DATA;
}

/*****************************************************************************
 *	Driver initialization/cleanup functions
 ****************************************************************************/
static void set_abs_position_params(struct input_dev *dev,
				    struct synaptics_data *priv, int x_code,
				    int y_code)
{
	int x_min = priv->x_min ?: XMIN_NOMINAL;
	int x_max = priv->x_max ?: XMAX_NOMINAL;
	int y_min = priv->y_min ?: YMIN_NOMINAL;
	int y_max = priv->y_max ?: YMAX_NOMINAL;
	int fuzz = SYN_CAP_REDUCED_FILTERING(priv->ext_cap_0c) ?
			SYN_REDUCED_FILTER_FUZZ : 0;

	input_set_abs_params(dev, x_code, x_min, x_max, fuzz, 0);
	input_set_abs_params(dev, y_code, y_min, y_max, fuzz, 0);
	input_abs_set_res(dev, x_code, priv->x_res);
	input_abs_set_res(dev, y_code, priv->y_res);
}

static void set_input_params(struct psmouse *psmouse,
			     struct synaptics_data *priv)
{
	struct input_dev *dev = psmouse->dev;
	int i;

	/* Things that apply to both modes */
	__set_bit(INPUT_PROP_POINTER, dev->propbit);
	__set_bit(EV_KEY, dev->evbit);
	__set_bit(BTN_LEFT, dev->keybit);
	__set_bit(BTN_RIGHT, dev->keybit);

	if (SYN_CAP_MIDDLE_BUTTON(priv->capabilities))
		__set_bit(BTN_MIDDLE, dev->keybit);

	if (!priv->absolute_mode) {
		/* Relative mode */
		__set_bit(EV_REL, dev->evbit);
		__set_bit(REL_X, dev->relbit);
		__set_bit(REL_Y, dev->relbit);
		return;
	}

	/* Absolute mode */
	__set_bit(EV_ABS, dev->evbit);
	set_abs_position_params(dev, priv, ABS_X, ABS_Y);
	input_set_abs_params(dev, ABS_PRESSURE, 0, 255, 0, 0);

	if (cr48_profile_sensor)
		input_set_abs_params(dev, ABS_MT_PRESSURE, 0, 255, 0, 0);

	if (SYN_CAP_IMAGE_SENSOR(priv->ext_cap_0c)) {
		set_abs_position_params(dev, priv, ABS_MT_POSITION_X,
					ABS_MT_POSITION_Y);
		/* Image sensors can report per-contact pressure */
		input_set_abs_params(dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
		input_mt_init_slots(dev, 2, INPUT_MT_POINTER | INPUT_MT_TRACK);

		/* Image sensors can signal 4 and 5 finger clicks */
		__set_bit(BTN_TOOL_QUADTAP, dev->keybit);
		__set_bit(BTN_TOOL_QUINTTAP, dev->keybit);
	} else if (SYN_CAP_ADV_GESTURE(priv->ext_cap_0c)) {
		set_abs_position_params(dev, priv, ABS_MT_POSITION_X,
					ABS_MT_POSITION_Y);
		/*
		 * Profile sensor in CR-48 tracks contacts reasonably well,
		 * other non-image sensors with AGM use semi-mt.
		 */
		input_mt_init_slots(dev, 2,
				    INPUT_MT_POINTER |
				    (cr48_profile_sensor ?
					INPUT_MT_TRACK : INPUT_MT_SEMI_MT));
	}

	if (SYN_CAP_PALMDETECT(priv->capabilities))
		input_set_abs_params(dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);

	__set_bit(BTN_TOUCH, dev->keybit);
	__set_bit(BTN_TOOL_FINGER, dev->keybit);

	if (SYN_CAP_MULTIFINGER(priv->capabilities)) {
		__set_bit(BTN_TOOL_DOUBLETAP, dev->keybit);
		__set_bit(BTN_TOOL_TRIPLETAP, dev->keybit);
	}

	if (SYN_CAP_FOUR_BUTTON(priv->capabilities) ||
	    SYN_CAP_MIDDLE_BUTTON(priv->capabilities)) {
		__set_bit(BTN_FORWARD, dev->keybit);
		__set_bit(BTN_BACK, dev->keybit);
	}

	if (!SYN_CAP_EXT_BUTTONS_STICK(priv->ext_cap_10))
		for (i = 0; i < SYN_CAP_MULTI_BUTTON_NO(priv->ext_cap); i++)
			__set_bit(BTN_0 + i, dev->keybit);

	__clear_bit(EV_REL, dev->evbit);
	__clear_bit(REL_X, dev->relbit);
	__clear_bit(REL_Y, dev->relbit);

	if (SYN_CAP_CLICKPAD(priv->ext_cap_0c)) {
		__set_bit(INPUT_PROP_BUTTONPAD, dev->propbit);
		if (psmouse_matches_pnp_id(psmouse, topbuttonpad_pnp_ids) &&
		    !SYN_CAP_EXT_BUTTONS_STICK(priv->ext_cap_10))
			__set_bit(INPUT_PROP_TOPBUTTONPAD, dev->propbit);
		/* Clickpads report only left button */
		__clear_bit(BTN_RIGHT, dev->keybit);
		__clear_bit(BTN_MIDDLE, dev->keybit);
	}
}

static ssize_t synaptics_show_disable_gesture(struct psmouse *psmouse,
					      void *data, char *buf)
{
	struct synaptics_data *priv = psmouse->private;

	return sprintf(buf, "%c\n", priv->disable_gesture ? '1' : '0');
}

static ssize_t synaptics_set_disable_gesture(struct psmouse *psmouse,
					     void *data, const char *buf,
					     size_t len)
{
	struct synaptics_data *priv = psmouse->private;
	unsigned int value;
	int err;

	err = kstrtouint(buf, 10, &value);
	if (err)
		return err;

	if (value > 1)
		return -EINVAL;

	if (value == priv->disable_gesture)
		return len;

	priv->disable_gesture = value;
	if (value)
		priv->mode |= SYN_BIT_DISABLE_GESTURE;
	else
		priv->mode &= ~SYN_BIT_DISABLE_GESTURE;

	if (synaptics_mode_cmd(psmouse, priv->mode))
		return -EIO;

	return len;
}

PSMOUSE_DEFINE_ATTR(disable_gesture, S_IWUSR | S_IRUGO, NULL,
		    synaptics_show_disable_gesture,
		    synaptics_set_disable_gesture);

static void synaptics_disconnect(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;

	if (!priv->absolute_mode && SYN_ID_DISGEST_SUPPORTED(priv->identity))
		device_remove_file(&psmouse->ps2dev.serio->dev,
				   &psmouse_attr_disable_gesture.dattr);

	synaptics_reset(psmouse);
	kfree(priv);
	psmouse->private = NULL;
}

static int synaptics_reconnect(struct psmouse *psmouse)
{
	struct synaptics_data *priv = psmouse->private;
	struct synaptics_data old_priv = *priv;
	unsigned char param[2];
	int retry = 0;
	int error;

	do {
		psmouse_reset(psmouse);
		if (retry) {
			/*
			 * On some boxes, right after resuming, the touchpad
			 * needs some time to finish initializing (I assume
			 * it needs time to calibrate) and start responding
			 * to Synaptics-specific queries, so let's wait a
			 * bit.
			 */
			ssleep(1);
		}
		ps2_command(&psmouse->ps2dev, param, PSMOUSE_CMD_GETID);
		error = synaptics_detect(psmouse, 0);
	} while (error && ++retry < 3);

	if (error)
		return -1;

	if (retry > 1)
		psmouse_dbg(psmouse, "reconnected after %d tries\n", retry);

	if (synaptics_query_hardware(psmouse)) {
		psmouse_err(psmouse, "Unable to query device.\n");
		return -1;
	}

	if (synaptics_set_mode(psmouse)) {
		psmouse_err(psmouse, "Unable to initialize device.\n");
		return -1;
	}

	if (old_priv.identity != priv->identity ||
	    old_priv.model_id != priv->model_id ||
	    old_priv.capabilities != priv->capabilities ||
	    old_priv.ext_cap != priv->ext_cap) {
		psmouse_err(psmouse,
			    "hardware appears to be different: id(%ld-%ld), model(%ld-%ld), caps(%lx-%lx), ext(%lx-%lx).\n",
			    old_priv.identity, priv->identity,
			    old_priv.model_id, priv->model_id,
			    old_priv.capabilities, priv->capabilities,
			    old_priv.ext_cap, priv->ext_cap);
		return -1;
	}

	return 0;
}

static bool impaired_toshiba_kbc;

static const struct dmi_system_id toshiba_dmi_table[] __initconst = {
#if defined(CONFIG_DMI) && defined(CONFIG_X86)
	{
		/* Toshiba Satellite */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "TOSHIBA"),
			DMI_MATCH(DMI_PRODUCT_NAME, "Satellite"),
		},
	},
	{
		/* Toshiba Dynabook */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "TOSHIBA"),
			DMI_MATCH(DMI_PRODUCT_NAME, "dynabook"),
		},
	},
	{
		/* Toshiba Portege M300 */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "TOSHIBA"),
			DMI_MATCH(DMI_PRODUCT_NAME, "PORTEGE M300"),
		},

	},
	{
		/* Toshiba Portege M300 */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "TOSHIBA"),
			DMI_MATCH(DMI_PRODUCT_NAME, "Portable PC"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "Version 1.0"),
		},

	},
#endif
	{ }
};

static bool broken_olpc_ec;

static const struct dmi_system_id olpc_dmi_table[] __initconst = {
#if defined(CONFIG_DMI) && defined(CONFIG_OLPC)
	{
		/* OLPC XO-1 or XO-1.5 */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "OLPC"),
			DMI_MATCH(DMI_PRODUCT_NAME, "XO"),
		},
	},
#endif
	{ }
};

static const struct dmi_system_id __initconst cr48_dmi_table[] = {
#if defined(CONFIG_DMI) && defined(CONFIG_X86)
	{
		/* Cr-48 Chromebook (Codename Mario) */
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "IEC"),
			DMI_MATCH(DMI_PRODUCT_NAME, "Mario"),
		},
	},
#endif
	{ }
};

void __init synaptics_module_init(void)
{
	impaired_toshiba_kbc = dmi_check_system(toshiba_dmi_table);
	broken_olpc_ec = dmi_check_system(olpc_dmi_table);
	cr48_profile_sensor = dmi_check_system(cr48_dmi_table);
}

static int __synaptics_init(struct psmouse *psmouse, bool absolute_mode)
{
	struct synaptics_data *priv;
	int err = -1;

	/*
	 * The OLPC XO has issues with Synaptics' absolute mode; the constant
	 * packet spew overloads the EC such that key presses on the keyboard
	 * are missed.  Given that, don't even attempt to use Absolute mode.
	 * Relative mode seems to work just fine.
	 */
	if (absolute_mode && broken_olpc_ec) {
		psmouse_info(psmouse,
			     "OLPC XO detected, not enabling Synaptics protocol.\n");
		return -ENODEV;
	}

	psmouse->private = priv = kzalloc(sizeof(struct synaptics_data), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	psmouse_reset(psmouse);

	if (synaptics_query_hardware(psmouse)) {
		psmouse_err(psmouse, "Unable to query device.\n");
		goto init_fail;
	}

	priv->absolute_mode = absolute_mode;
	if (SYN_ID_DISGEST_SUPPORTED(priv->identity))
		priv->disable_gesture = true;

	/*
	 * Unfortunately ForcePad capability is not exported over PS/2,
	 * so we have to resort to checking PNP IDs.
	 */
	priv->is_forcepad = psmouse_matches_pnp_id(psmouse, forcepad_pnp_ids);

	if (synaptics_set_mode(psmouse)) {
		psmouse_err(psmouse, "Unable to initialize device.\n");
		goto init_fail;
	}

	priv->pkt_type = SYN_MODEL_NEWABS(priv->model_id) ? SYN_NEWABS : SYN_OLDABS;

	psmouse_info(psmouse,
		     "Touchpad model: %ld, fw: %ld.%ld, id: %#lx, caps: %#lx/%#lx/%#lx, board id: %lu, fw id: %lu\n",
		     SYN_ID_MODEL(priv->identity),
		     SYN_ID_MAJOR(priv->identity), SYN_ID_MINOR(priv->identity),
		     priv->model_id,
		     priv->capabilities, priv->ext_cap, priv->ext_cap_0c,
		     priv->board_id, priv->firmware_id);

	set_input_params(psmouse, priv);

	/*
	 * Encode touchpad model so that it can be used to set
	 * input device->id.version and be visible to userspace.
	 * Because version is __u16 we have to drop something.
	 * Hardware info bits seem to be good candidates as they
	 * are documented to be for Synaptics corp. internal use.
	 */
	psmouse->model = ((priv->model_id & 0x00ff0000) >> 8) |
			  (priv->model_id & 0x000000ff);

	if (absolute_mode) {
		psmouse->protocol_handler = synaptics_process_byte;
		psmouse->pktsize = 6;
	} else {
		/* Relative mode follows standard PS/2 mouse protocol */
		psmouse->protocol_handler = psmouse_process_byte;
		psmouse->pktsize = 3;
	}

	psmouse->set_rate = synaptics_set_rate;
	psmouse->disconnect = synaptics_disconnect;
	psmouse->reconnect = synaptics_reconnect;
	psmouse->cleanup = synaptics_reset;
	/* Synaptics can usually stay in sync without extra help */
	psmouse->resync_time = 0;

	if (SYN_CAP_PASS_THROUGH(priv->capabilities))
		synaptics_pt_create(psmouse);

	/*
	 * Toshiba's KBC seems to have trouble handling data from
	 * Synaptics at full rate.  Switch to a lower rate (roughly
	 * the same rate as a standard PS/2 mouse).
	 */
	if (psmouse->rate >= 80 && impaired_toshiba_kbc) {
		psmouse_info(psmouse,
			     "Toshiba %s detected, limiting rate to 40pps.\n",
			     dmi_get_system_info(DMI_PRODUCT_NAME));
		psmouse->rate = 40;
	}

	if (!priv->absolute_mode && SYN_ID_DISGEST_SUPPORTED(priv->identity)) {
		err = device_create_file(&psmouse->ps2dev.serio->dev,
					 &psmouse_attr_disable_gesture.dattr);
		if (err) {
			psmouse_err(psmouse,
				    "Failed to create disable_gesture attribute (%d)",
				    err);
			goto init_fail;
		}
	}

	return 0;

 init_fail:
	kfree(priv);
	return err;
}

int synaptics_init(struct psmouse *psmouse)
{
	return __synaptics_init(psmouse, true);
}

int synaptics_init_relative(struct psmouse *psmouse)
{
	return __synaptics_init(psmouse, false);
}

#else /* CONFIG_MOUSE_PS2_SYNAPTICS */

void __init synaptics_module_init(void)
{
}

int synaptics_init(struct psmouse *psmouse)
{
	return -ENOSYS;
}

#endif /* CONFIG_MOUSE_PS2_SYNAPTICS */
