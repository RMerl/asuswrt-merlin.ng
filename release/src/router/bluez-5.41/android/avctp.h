/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define AVCTP_CONTROL_PSM		23
#define AVCTP_BROWSING_PSM		27

#define AVCTP_HEADER_LENGTH		3
#define AVC_HEADER_LENGTH		3

#define AVC_DATA_OFFSET			AVCTP_HEADER_LENGTH + AVC_HEADER_LENGTH
#define AVC_DATA_MTU			512

/* ctype entries */
#define AVC_CTYPE_CONTROL		0x0
#define AVC_CTYPE_STATUS		0x1
#define AVC_CTYPE_NOTIFY		0x3
#define AVC_CTYPE_NOT_IMPLEMENTED	0x8
#define AVC_CTYPE_ACCEPTED		0x9
#define AVC_CTYPE_REJECTED		0xA
#define AVC_CTYPE_STABLE		0xC
#define AVC_CTYPE_CHANGED		0xD
#define AVC_CTYPE_INTERIM		0xF

/* opcodes */
#define AVC_OP_VENDORDEP		0x00
#define AVC_OP_UNITINFO			0x30
#define AVC_OP_SUBUNITINFO		0x31
#define AVC_OP_PASSTHROUGH		0x7c

/* subunits of interest */
#define AVC_SUBUNIT_PANEL		0x09

/* operands in passthrough commands */
#define AVC_SELECT			0x00
#define AVC_UP				0x01
#define AVC_DOWN			0x02
#define AVC_LEFT			0x03
#define AVC_RIGHT			0x04
#define AVC_ROOT_MENU			0x09
#define AVC_CONTENTS_MENU		0x0b
#define AVC_FAVORITE_MENU		0x0c
#define AVC_EXIT			0x0d
#define AVC_ON_DEMAND_MENU		0x0e
#define AVC_APPS_MENU			0x0f
#define AVC_0				0x20
#define AVC_1				0x21
#define AVC_2				0x22
#define AVC_3				0x23
#define AVC_4				0x24
#define AVC_5				0x25
#define AVC_6				0x26
#define AVC_7				0x27
#define AVC_8				0x28
#define AVC_9				0x29
#define AVC_DOT				0x2a
#define AVC_ENTER			0x2b
#define AVC_CHANNEL_UP			0x30
#define AVC_CHANNEL_DOWN		0x31
#define AVC_CHANNEL_PREVIOUS		0x32
#define AVC_INPUT_SELECT		0x34
#define AVC_INFO			0x35
#define AVC_HELP			0x36
#define AVC_PAGE_UP			0x37
#define AVC_PAGE_DOWN			0x38
#define AVC_LOCK			0x3a
#define AVC_POWER			0x40
#define AVC_VOLUME_UP			0x41
#define AVC_VOLUME_DOWN			0x42
#define AVC_MUTE			0x43
#define AVC_PLAY			0x44
#define AVC_STOP			0x45
#define AVC_PAUSE			0x46
#define AVC_RECORD			0x47
#define AVC_REWIND			0x48
#define AVC_FAST_FORWARD		0x49
#define AVC_EJECT			0x4a
#define AVC_FORWARD			0x4b
#define AVC_BACKWARD			0x4c
#define AVC_LIST			0x4d
#define AVC_F1				0x71
#define AVC_F2				0x72
#define AVC_F3				0x73
#define AVC_F4				0x74
#define AVC_F5				0x75
#define AVC_F6				0x76
#define AVC_F7				0x77
#define AVC_F8				0x78
#define AVC_F9				0x79
#define AVC_RED				0x7a
#define AVC_GREEN			0x7b
#define AVC_BLUE			0x7c
#define AVC_YELLOW			0x7c

#define AVC_VENDOR_UNIQUE		0x7e

#define AVC_VENDOR_NEXT_GROUP		0x00
#define AVC_VENDOR_PREV_GROUP		0x01

struct avctp;

typedef bool (*avctp_passthrough_cb) (struct avctp *session,
					uint8_t op, bool pressed,
					void *user_data);
typedef ssize_t (*avctp_control_pdu_cb) (struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data);
typedef gboolean (*avctp_rsp_cb) (struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t *operands,
					size_t operand_count, void *user_data);
typedef gboolean (*avctp_browsing_rsp_cb) (struct avctp *session,
					uint8_t *operands, size_t operand_count,
					void *user_data);
typedef ssize_t (*avctp_browsing_pdu_cb) (struct avctp *session,
					uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data);

typedef void (*avctp_destroy_cb_t) (void *user_data);

struct avctp *avctp_new(int fd, size_t imtu, size_t omtu, uint16_t version);
void avctp_set_destroy_cb(struct avctp *session, avctp_destroy_cb_t cb,
							void *user_data);

int avctp_init_uinput(struct avctp *session, const char *name,
							const char *address);
int avctp_connect_browsing(struct avctp *session, int fd, size_t imtu,
								size_t omtu);

void avctp_shutdown(struct avctp *session);

unsigned int avctp_register_passthrough_handler(struct avctp *session,
						avctp_passthrough_cb cb,
						void *user_data);
bool avctp_unregister_passthrough_handler(struct avctp *session,
							unsigned int id);

unsigned int avctp_register_pdu_handler(struct avctp *session, uint8_t opcode,
						avctp_control_pdu_cb cb,
						void *user_data);
bool avctp_unregister_pdu_handler(struct avctp *session, unsigned int id);

unsigned int avctp_register_browsing_pdu_handler(struct avctp *session,
						avctp_browsing_pdu_cb cb,
						void *user_data,
						avctp_destroy_cb_t destroy);
bool avctp_unregister_browsing_pdu_handler(struct avctp *session,
							unsigned int id);

int avctp_send_passthrough(struct avctp *session, uint8_t op, uint8_t *params,
							size_t params_len);
int avctp_send_vendor(struct avctp *session, uint8_t transaction,
				uint8_t code, uint8_t subunit,
				const struct iovec *iov, int iov_cnt);
int avctp_send_vendor_req(struct avctp *session, uint8_t code, uint8_t subunit,
					const struct iovec *iov, int iov_cnt,
					avctp_rsp_cb func, void *user_data);
int avctp_send_browsing(struct avctp *session, uint8_t transaction,
					const struct iovec *iov, int iov_cnt);
int avctp_send_browsing_req(struct avctp *session,
				const struct iovec *iov, int iov_cnt,
				avctp_browsing_rsp_cb func, void *user_data);
