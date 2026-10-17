// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int log_console_emit(struct log_device *ldev, struct log_rec *rec)
{
	int fmt = gd->log_fmt;
	bool add_space = false;

	/*
	 * The output format is designed to give someone a fighting chance of
	 * figuring out which field is which:
	 *    - level is in CAPS
	 *    - cat is lower case and ends with comma
	 *    - file normally has a .c extension and ends with a colon
	 *    - line is integer and ends with a -
	 *    - function is an identifier and ends with ()
	 *    - message has a space before it unless it is on its own
	 */
	if (!(rec->flags & LOGRECF_CONT) && fmt != BIT(LOGF_MSG)) {
		add_space = true;
		if (fmt & BIT(LOGF_LEVEL))
			printf("%s.", log_get_level_name(rec->level));
		if (fmt & BIT(LOGF_CAT))
			printf("%s,", log_get_cat_name(rec->cat));
		if (fmt & BIT(LOGF_FILE))
			printf("%s:", rec->file);
		if (fmt & BIT(LOGF_LINE))
			printf("%d-", rec->line);
		if (fmt & BIT(LOGF_FUNC)) {
			if (CONFIG_IS_ENABLED(USE_TINY_PRINTF)) {
				printf("%s()", rec->func);
			} else {
				printf("%*s()", CONFIG_LOGF_FUNC_PAD,
				       rec->func);
			}
		}
	}
	if (fmt & BIT(LOGF_MSG))
		printf("%s%s", add_space ? " " : "", rec->msg);

	return 0;
}

LOG_DRIVER(console) = {
	.name	= "console",
	.emit	= log_console_emit,
	.flags	= LOGDF_ENABLE,
};
