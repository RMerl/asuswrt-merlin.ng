/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
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

#define cmd_uim_verify_pin1_cb no_cb
static enum qmi_cmd_result
cmd_uim_verify_pin1_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_uim_verify_pin_request data = {
		QMI_INIT_SEQUENCE(session_information,
			.session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1,
			""
		),
		QMI_INIT_SEQUENCE(info,
			.pin_id = QMI_UIM_PIN_ID_PIN1,
			.pin_value = arg
		)
	};
	qmi_set_uim_verify_pin_request(msg, &data);
	return QMI_CMD_REQUEST;
}

#define cmd_uim_verify_pin2_cb no_cb
static enum qmi_cmd_result
cmd_uim_verify_pin2_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_uim_verify_pin_request data = {
		QMI_INIT_SEQUENCE(session_information,
			.session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1,
			""
		),
		QMI_INIT_SEQUENCE(info,
			.pin_id = QMI_UIM_PIN_ID_PIN2,
			.pin_value = arg
		)
	};
	qmi_set_uim_verify_pin_request(msg, &data);
	return QMI_CMD_REQUEST;
}
