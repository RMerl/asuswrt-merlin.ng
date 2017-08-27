/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef __QMI_STRUCT_H
#define __QMI_STRUCT_H

struct qmux {
	uint16_t len;
	uint8_t flags;
	uint8_t service;
	uint8_t client;
} __packed;

struct tlv {
	uint8_t type;
	uint16_t len;
	uint8_t data[];
} __packed;

struct qmi_ctl {
	uint8_t transaction;
	uint16_t message;
	uint16_t tlv_len;
	struct tlv tlv[];
} __packed;

struct qmi_svc {
	uint16_t transaction;
	uint16_t message;
	uint16_t tlv_len;
	struct tlv tlv[];
} __packed;

struct qmi_msg {
	uint8_t marker;
	struct qmux qmux;
	uint8_t flags;
	union {
		struct qmi_ctl ctl;
		struct qmi_svc svc;
	};
} __packed;

#endif
