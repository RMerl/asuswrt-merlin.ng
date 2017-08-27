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

#define __uqmi_wms_commands \
	__uqmi_command(wms_list_messages, list-messages, no, QMI_SERVICE_WMS), \
	__uqmi_command(wms_delete_message, delete-message, required, QMI_SERVICE_WMS), \
	__uqmi_command(wms_get_message, get-message, required, QMI_SERVICE_WMS), \
	__uqmi_command(wms_get_raw_message, get-raw-message, required, QMI_SERVICE_WMS), \
	__uqmi_command(wms_send_message_smsc, send-message-smsc, required, CMD_TYPE_OPTION), \
	__uqmi_command(wms_send_message_target, send-message-target, required, CMD_TYPE_OPTION), \
	__uqmi_command(wms_send_message_flash, send-message-flash, no, CMD_TYPE_OPTION), \
	__uqmi_command(wms_send_message, send-message, required, QMI_SERVICE_WMS)

#define wms_helptext \
		"  --list-messages:                  List SMS messages\n" \
		"  --delete-message <id>:            Delete SMS message at index <id>\n" \
		"  --get-message <id>:               Get SMS message at index <id>\n" \
		"  --get-raw-message <id>:           Get SMS raw message contents at index <id>\n" \
		"  --send-message <data>:            Send SMS message (use options below)\n" \
		"    --send-message-smsc <nr>:       SMSC number\n" \
		"    --send-message-target <nr>:     Destination number (required)\n" \
		"    --send-message-flash:           Send as Flash SMS\n" \

