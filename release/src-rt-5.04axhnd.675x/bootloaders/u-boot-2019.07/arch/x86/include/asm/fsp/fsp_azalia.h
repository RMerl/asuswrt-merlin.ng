/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Google, Inc
 */

#ifndef _FSP_AZALIA_H_
#define _FSP_AZALIA_H_

struct __packed azalia_verb_table_header {
	u32 vendor_device_id;
	u16 sub_system_id;
	u8 revision_id;		/* 0xff applies to all steppings */
	u8 front_panel_support;
	u16 number_of_rear_jacks;
	u16 number_of_front_jacks;
};

struct __packed azalia_verb_table {
	struct azalia_verb_table_header header;
	const u32 *data;
};

struct __packed azalia_config {
	u8 pme_enable:1;
	u8 docking_supported:1;
	u8 docking_attached:1;
	u8 hdmi_codec_enable:1;
	u8 azalia_v_ci_enable:1;
	u8 rsvdbits:3;
	/* number of verb tables provided by platform */
	u8 verb_table_num;
	const struct azalia_verb_table *verb_table;
	/* delay timer after azalia reset */
	u16 reset_wait_timer_ms;
};

#endif
