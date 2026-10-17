/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __CEDIT_H
#define __CEDIT_H

#include <dm/ofnode_decl.h>

struct abuf;
struct expo;
struct scene;
struct video_priv;

enum {
	/* size increment for writing FDT */
	CEDIT_SIZE_INC	= 1024,
};

/* Name of the cedit node in the devicetree */
#define CEDIT_NODE_NAME		"cedit-values"

extern struct expo *cur_exp;

/**
 * cedit_arange() - Arrange objects in a configuration-editor scene
 *
 * @exp: Expo to update
 * @vid_priv: Private info of the video device
 * @scene_id: scene ID to arrange
 * Returns: 0 if OK, -ve on error
 */
int cedit_arange(struct expo *exp, struct video_priv *vid_priv, uint scene_id);

/**
 * cedit_run() - Run a configuration editor
 *
 * This accepts input until the user quits with Escape
 *
 * @exp: Expo to use
 * Returns: 0 if OK, -ve on error
 */
int cedit_run(struct expo *exp);

/**
 * cedit_prepare() - Prepare to run a cedit
 *
 * Set up the video device, select the first scene and highlight the first item.
 * This ensures that all menus have a selected item.
 *
 * @exp: Expo to use
 * @vid_privp: Set to private data for the video device
 * @scnp: Set to the first scene
 * Return: scene ID of first scene if OK, -ve on error
 */
int cedit_prepare(struct expo *exp, struct video_priv **vid_privp,
		  struct scene **scnp);

/**
 * cedit_write_settings() - Write settings in FDT format
 *
 * Sets up an FDT with the settings
 *
 * @exp: Expo to write settings from
 * @buf: Returns abuf containing the settings FDT (inited by this function)
 * Return: 0 if OK, -ve on error
 */
int cedit_write_settings(struct expo *exp, struct abuf *buf);

/**
 * cedit_read_settings() - Read settings in FDT format
 *
 * Read an FDT with the settings
 *
 * @exp: Expo to read settings into
 * @tree: Tree to read from
 * Return: 0 if OK, -ve on error
 */
int cedit_read_settings(struct expo *exp, oftree tree);

/**
 * cedit_write_settings_env() - Write settings to envrionment variables
 *
 * @exp: Expo to write settings from
 * @verbose: true to print each var as it is set
 * Return: 0 if OK, -ve on error
 */
int cedit_write_settings_env(struct expo *exp, bool verbose);

/*
 * cedit_read_settings_env() - Read settings from the environment
 *
 * @exp: Expo to read settings into
 * @verbose: true to print each var before it is read
 */
int cedit_read_settings_env(struct expo *exp, bool verbose);

/**
 * cedit_write_settings_cmos() - Write settings to CMOS RAM
 *
 * Write settings to the defined places in CMOS RAM
 *
 * @exp: Expo to write settings from
 * @dev: UCLASS_RTC device containing space for this information
 * Returns 0 if OK, -ve on error
 * @verbose: true to print a summary at the end
 */
int cedit_write_settings_cmos(struct expo *exp, struct udevice *dev,
			      bool verbose);

/**
 * cedit_read_settings_cmos() - Read settings from CMOS RAM
 *
 * Read settings from the defined places in CMO RAM
 *
 * @exp: Expo to read settings into
 * @dev: RTC device to read settings from
 * @verbose: true to print a summary at the end
 */
int cedit_read_settings_cmos(struct expo *exp, struct udevice *dev,
			     bool verbose);

#endif /* __CEDIT_H */
