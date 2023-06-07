/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Andrzej Kaczmarek <andrzej.kaczmarek@codecoup.pl>
 *
 *
 */

bool a2dp_codec_cap(uint8_t codec, uint8_t losc, struct l2cap_frame *frame);

bool a2dp_codec_cfg(uint8_t codec, uint8_t losc, struct l2cap_frame *frame);
