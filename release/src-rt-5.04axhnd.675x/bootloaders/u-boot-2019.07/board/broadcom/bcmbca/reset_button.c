// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <common.h>
#include "bcmbca_button.h"

#define DEFAULT_RESET_CMD	"sdk restoredefault;reset"

static void btn_hook_rst_to_dflt(unsigned long time, void *param)
{
	if (param) {
		run_command((char *)param, 0);
	} else {
		/* stop the auto boot as the default rst button behavior */
		run_command(DEFAULT_RESET_CMD, 0);
	}
}

static void btn_hook_reset_release(unsigned long time, void *param)
{
 	if (param) {
		run_command((char *)param, 0);
	}
}


/* 
 * u-boot button drv uses release event to support multiple press action with
 * different length of press time. Using the example below, if user press and
 * hold reset button for more than 5s but less 10s, the default_cmd action
 * will be taken.  If user press and hold longer than 10s then release, the
 * recovery action will be taken.  If user press and never release the button,
 * u-boot boot will stuck and wait for user to release the botton
 *
 * User can add more action items with different wait length. Button drv 
 * enumerate all of them when registering action for release event.
 *
 * If release action is not specified, it fallbacks to hold rst_to_dflt action
 * 
 *		reset_button {
 *			press {
 *				print = "Button Press -- Hold for 5s to do restore to default";
 *			};
 *			hold {
 *				rst_to_dflt = <5>;
 *			};
 *			release {
 *				reset = <0>;
 *				default = < 5>;
 *				default_cmd= "sdk restoredfaults ; sdk boot_image";
 *				recovery = <10>;
 *				recovery_cmd = "setenv ipaddr 192.168.1.1; setenv netmask 255.0.0.0 ; sdk httpd_start ; sdk poll_bg";
 *			};	
 *		};
 */

int reset_button_init(void)
{
	int ret;

	ret = register_button_action_for_event("reset_button", "release", NULL,
			btn_hook_reset_release);
	/* if board does not use release event for the reset button, fallback
	   default hold event */
	if (ret <= 1) {
		ret = register_button_action("reset_button", "rst_to_dflt",
			btn_hook_rst_to_dflt);
		if (ret < 0 && ret != -ENODEV) {
			printk("Failed to register rst_to_dflt action rc %d\n", ret);
			return -1;
		}
	}

	return 0;
}
