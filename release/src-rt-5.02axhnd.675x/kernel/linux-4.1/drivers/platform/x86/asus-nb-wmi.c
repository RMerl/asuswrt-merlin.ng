/*
 * Asus Notebooks WMI hotkey driver
 *
 * Copyright(C) 2010 Corentin Chary <corentin.chary@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/fb.h>
#include <linux/dmi.h>

#include "asus-wmi.h"

#define	ASUS_NB_WMI_FILE	"asus-nb-wmi"

MODULE_AUTHOR("Corentin Chary <corentin.chary@gmail.com>");
MODULE_DESCRIPTION("Asus Notebooks WMI Hotkey Driver");
MODULE_LICENSE("GPL");

#define ASUS_NB_WMI_EVENT_GUID	"0B3CBB35-E3C2-45ED-91C2-4C5A6D195D1C"

MODULE_ALIAS("wmi:"ASUS_NB_WMI_EVENT_GUID);

/*
 * WAPF defines the behavior of the Fn+Fx wlan key
 * The significance of values is yet to be found, but
 * most of the time:
 * Bit | Bluetooth | WLAN
 *  0  | Hardware  | Hardware
 *  1  | Hardware  | Software
 *  4  | Software  | Software
 */
static int wapf = -1;
module_param(wapf, uint, 0444);
MODULE_PARM_DESC(wapf, "WAPF value");

static struct quirk_entry *quirks;

static struct quirk_entry quirk_asus_unknown = {
	.wapf = 0,
};

/*
 * For those machines that need software to control bt/wifi status
 * and can't adjust brightness through ACPI interface
 * and have duplicate events(ACPI and WMI) for display toggle
 */
static struct quirk_entry quirk_asus_x55u = {
	.wapf = 4,
	.wmi_backlight_power = true,
	.no_display_toggle = true,
};

static struct quirk_entry quirk_asus_wapf4 = {
	.wapf = 4,
};

static struct quirk_entry quirk_asus_x200ca = {
	.wapf = 2,
};

static int dmi_matched(const struct dmi_system_id *dmi)
{
	quirks = dmi->driver_data;
	return 1;
}

static const struct dmi_system_id asus_quirks[] = {
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. U32U",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK Computer Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "U32U"),
		},
		/*
		 * Note this machine has a Brazos APU, and most Brazos Asus
		 * machines need quirk_asus_x55u / wmi_backlight_power but
		 * here acpi-video seems to work fine for backlight control.
		 */
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X302UA",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X302UA"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X401U",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X401U"),
		},
		.driver_data = &quirk_asus_x55u,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X401A",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X401A"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X401A1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X401A1"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X501U",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X501U"),
		},
		.driver_data = &quirk_asus_x55u,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X501A",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X501A"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X501A1",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X501A1"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X550CA",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X550CA"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X550CC",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X550CC"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X550CL",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X550CL"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X550VB",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X550VB"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X551CA",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X551CA"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X55A",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X55A"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X55C",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X55C"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X55U",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X55U"),
		},
		.driver_data = &quirk_asus_x55u,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X55VD",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X55VD"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X75A",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X75A"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X75VBP",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X75VBP"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. 1015E",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "1015E"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. 1015U",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "1015U"),
		},
		.driver_data = &quirk_asus_wapf4,
	},
	{
		.callback = dmi_matched,
		.ident = "ASUSTeK COMPUTER INC. X200CA",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "ASUSTeK COMPUTER INC."),
			DMI_MATCH(DMI_PRODUCT_NAME, "X200CA"),
		},
		.driver_data = &quirk_asus_x200ca,
	},
	{},
};

static void asus_nb_wmi_quirks(struct asus_wmi_driver *driver)
{
	quirks = &quirk_asus_unknown;
	dmi_check_system(asus_quirks);

	driver->quirks = quirks;
	driver->panel_power = FB_BLANK_UNBLANK;

	/* overwrite the wapf setting if the wapf paramater is specified */
	if (wapf != -1)
		quirks->wapf = wapf;
	else
		wapf = quirks->wapf;
}

static const struct key_entry asus_nb_wmi_keymap[] = {
	{ KE_KEY, ASUS_WMI_BRN_DOWN, { KEY_BRIGHTNESSDOWN } },
	{ KE_KEY, ASUS_WMI_BRN_UP, { KEY_BRIGHTNESSUP } },
	{ KE_KEY, 0x30, { KEY_VOLUMEUP } },
	{ KE_KEY, 0x31, { KEY_VOLUMEDOWN } },
	{ KE_KEY, 0x32, { KEY_MUTE } },
	{ KE_KEY, 0x33, { KEY_DISPLAYTOGGLE } }, /* LCD on */
	{ KE_KEY, 0x34, { KEY_DISPLAY_OFF } }, /* LCD off */
	{ KE_KEY, 0x40, { KEY_PREVIOUSSONG } },
	{ KE_KEY, 0x41, { KEY_NEXTSONG } },
	{ KE_KEY, 0x43, { KEY_STOPCD } }, /* Stop/Eject */
	{ KE_KEY, 0x45, { KEY_PLAYPAUSE } },
	{ KE_KEY, 0x4c, { KEY_MEDIA } }, /* WMP Key */
	{ KE_KEY, 0x50, { KEY_EMAIL } },
	{ KE_KEY, 0x51, { KEY_WWW } },
	{ KE_KEY, 0x55, { KEY_CALC } },
	{ KE_IGNORE, 0x57, },  /* Battery mode */
	{ KE_IGNORE, 0x58, },  /* AC mode */
	{ KE_KEY, 0x5C, { KEY_F15 } },  /* Power Gear key */
	{ KE_KEY, 0x5D, { KEY_WLAN } }, /* Wireless console Toggle */
	{ KE_KEY, 0x5E, { KEY_WLAN } }, /* Wireless console Enable */
	{ KE_KEY, 0x5F, { KEY_WLAN } }, /* Wireless console Disable */
	{ KE_KEY, 0x60, { KEY_TOUCHPAD_ON } },
	{ KE_KEY, 0x61, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD only */
	{ KE_KEY, 0x62, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT only */
	{ KE_KEY, 0x63, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT */
	{ KE_KEY, 0x64, { KEY_SWITCHVIDEOMODE } }, /* SDSP TV */
	{ KE_KEY, 0x65, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + TV */
	{ KE_KEY, 0x66, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT + TV */
	{ KE_KEY, 0x67, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT + TV */
	{ KE_KEY, 0x6B, { KEY_TOUCHPAD_TOGGLE } },
	{ KE_IGNORE, 0x6E, },  /* Low Battery notification */
	{ KE_KEY, 0x7D, { KEY_BLUETOOTH } }, /* Bluetooth Enable */
	{ KE_KEY, 0x7E, { KEY_BLUETOOTH } }, /* Bluetooth Disable */
	{ KE_KEY, 0x82, { KEY_CAMERA } },
	{ KE_KEY, 0x88, { KEY_RFKILL  } }, /* Radio Toggle Key */
	{ KE_KEY, 0x8A, { KEY_PROG1 } }, /* Color enhancement mode */
	{ KE_KEY, 0x8C, { KEY_SWITCHVIDEOMODE } }, /* SDSP DVI only */
	{ KE_KEY, 0x8D, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + DVI */
	{ KE_KEY, 0x8E, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT + DVI */
	{ KE_KEY, 0x8F, { KEY_SWITCHVIDEOMODE } }, /* SDSP TV + DVI */
	{ KE_KEY, 0x90, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT + DVI */
	{ KE_KEY, 0x91, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + TV + DVI */
	{ KE_KEY, 0x92, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT + TV + DVI */
	{ KE_KEY, 0x93, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT + TV + DVI */
	{ KE_KEY, 0x95, { KEY_MEDIA } },
	{ KE_KEY, 0x99, { KEY_PHONE } },
	{ KE_KEY, 0xA0, { KEY_SWITCHVIDEOMODE } }, /* SDSP HDMI only */
	{ KE_KEY, 0xA1, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + HDMI */
	{ KE_KEY, 0xA2, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT + HDMI */
	{ KE_KEY, 0xA3, { KEY_SWITCHVIDEOMODE } }, /* SDSP TV + HDMI */
	{ KE_KEY, 0xA4, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT + HDMI */
	{ KE_KEY, 0xA5, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + TV + HDMI */
	{ KE_KEY, 0xA6, { KEY_SWITCHVIDEOMODE } }, /* SDSP CRT + TV + HDMI */
	{ KE_KEY, 0xA7, { KEY_SWITCHVIDEOMODE } }, /* SDSP LCD + CRT + TV + HDMI */
	{ KE_KEY, 0xB5, { KEY_CALC } },
	{ KE_KEY, 0xC4, { KEY_KBDILLUMUP } },
	{ KE_KEY, 0xC5, { KEY_KBDILLUMDOWN } },
	{ KE_IGNORE, 0xC6, },  /* Ambient Light Sensor notification */
	{ KE_END, 0},
};

static struct asus_wmi_driver asus_nb_wmi_driver = {
	.name = ASUS_NB_WMI_FILE,
	.owner = THIS_MODULE,
	.event_guid = ASUS_NB_WMI_EVENT_GUID,
	.keymap = asus_nb_wmi_keymap,
	.input_name = "Asus WMI hotkeys",
	.input_phys = ASUS_NB_WMI_FILE "/input0",
	.detect_quirks = asus_nb_wmi_quirks,
};


static int __init asus_nb_wmi_init(void)
{
	return asus_wmi_register_driver(&asus_nb_wmi_driver);
}

static void __exit asus_nb_wmi_exit(void)
{
	asus_wmi_unregister_driver(&asus_nb_wmi_driver);
}

module_init(asus_nb_wmi_init);
module_exit(asus_nb_wmi_exit);
