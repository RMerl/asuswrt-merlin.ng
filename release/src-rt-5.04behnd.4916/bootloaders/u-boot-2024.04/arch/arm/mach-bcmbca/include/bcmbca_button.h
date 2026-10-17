/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#ifndef _BCMBCA_BUTTON_H
#define _BCMBCA_BUTTON_H

typedef void (*buttonNotifyHook_t)(unsigned long timeInMs, void *param);
int register_button_action(const char *button_name, const char *action_name,
	buttonNotifyHook_t hook);
int register_button_action_for_event(const char *button_name, const char* event_name,
	const char *action_name, buttonNotifyHook_t hook);
void bcmbca_button_init(void);
void btn_poll_block(void);
int btn_poll(void);

#endif
