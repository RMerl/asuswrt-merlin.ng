/*
 * Copyright (C) 2016  Bjørn Mork <bjorn@mork.no>
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

#include "mbim.h"

static const uint8_t qmiuuid[16] = { 0xd1, 0xa3, 0x0b, 0xc2, 0xf9, 0x7a, 0x6e, 0x43,
				     0xbf, 0x65, 0xc7, 0xe2, 0x4f, 0xb0, 0xf0, 0xd3 };

bool is_mbim_qmi(struct mbim_command_message *msg)
{
	return msg->header.type == cpu_to_le32(MBIM_MESSAGE_TYPE_COMMAND_DONE) &&
		msg->command_id == cpu_to_le32(MBIM_CID_QMI_MSG) &&
		!msg->command_type &&	/* actually 'status' here */
		!memcmp(msg->service_id, qmiuuid, 16);
									    }

void mbim_qmi_cmd(struct mbim_command_message *msg, int len, uint16_t tid)
{
	msg->header.type = cpu_to_le32(MBIM_MESSAGE_TYPE_COMMAND);
	msg->header.length = cpu_to_le32(sizeof(*msg) + len);
	msg->header.transaction_id = cpu_to_le32(tid);
	msg->fragment_header.total = cpu_to_le32(1);
	msg->fragment_header.current = 0;
	memcpy(msg->service_id, qmiuuid, 16);
	msg->command_id = cpu_to_le32(MBIM_CID_QMI_MSG);
	msg->command_type = cpu_to_le32(MBIM_MESSAGE_COMMAND_TYPE_SET);
	msg->buffer_length = cpu_to_le32(len);
}
