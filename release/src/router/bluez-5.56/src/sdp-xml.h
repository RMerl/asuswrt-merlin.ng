/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2005-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

void convert_sdp_record_to_xml(sdp_record_t *rec,
		void *user_data, void (*append_func) (void *, const char *));

sdp_record_t *sdp_xml_parse_record(const char *data, int size);
