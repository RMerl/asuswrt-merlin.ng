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

struct avdtp;
struct avdtp_stream;
struct avdtp_local_sep;
struct avdtp_remote_sep;
struct avdtp_error {
	uint8_t category;
	union {
		uint8_t error_code;
		int posix_errno;
	} err;
};

#define AVDTP_PSM 25

/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT			0x01
#define AVDTP_REPORTING				0x02
#define AVDTP_RECOVERY				0x03
#define AVDTP_CONTENT_PROTECTION		0x04
#define AVDTP_HEADER_COMPRESSION		0x05
#define AVDTP_MULTIPLEXING			0x06
#define AVDTP_MEDIA_CODEC			0x07
#define AVDTP_DELAY_REPORTING			0x08
#define AVDTP_ERRNO				0xff

/* AVDTP error definitions */
#define AVDTP_BAD_HEADER_FORMAT			0x01
#define AVDTP_BAD_LENGTH			0x11
#define AVDTP_BAD_ACP_SEID			0x12
#define AVDTP_SEP_IN_USE			0x13
#define AVDTP_SEP_NOT_IN_USE			0x14
#define AVDTP_BAD_SERV_CATEGORY			0x17
#define AVDTP_BAD_PAYLOAD_FORMAT		0x18
#define AVDTP_NOT_SUPPORTED_COMMAND		0x19
#define AVDTP_INVALID_CAPABILITIES		0x1A
#define AVDTP_BAD_RECOVERY_TYPE			0x22
#define AVDTP_BAD_MEDIA_TRANSPORT_FORMAT	0x23
#define AVDTP_BAD_RECOVERY_FORMAT		0x25
#define AVDTP_BAD_ROHC_FORMAT			0x26
#define AVDTP_BAD_CP_FORMAT			0x27
#define AVDTP_BAD_MULTIPLEXING_FORMAT		0x28
#define AVDTP_UNSUPPORTED_CONFIGURATION		0x29
#define AVDTP_BAD_STATE				0x31

/* SEP types definitions */
#define AVDTP_SEP_TYPE_SOURCE			0x00
#define AVDTP_SEP_TYPE_SINK			0x01

/* Media types definitions */
#define AVDTP_MEDIA_TYPE_AUDIO			0x00
#define AVDTP_MEDIA_TYPE_VIDEO			0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA		0x02

typedef enum {
	AVDTP_STATE_IDLE,
	AVDTP_STATE_CONFIGURED,
	AVDTP_STATE_OPEN,
	AVDTP_STATE_STREAMING,
	AVDTP_STATE_CLOSING,
	AVDTP_STATE_ABORTING,
} avdtp_state_t;

struct avdtp_service_capability {
	uint8_t category;
	uint8_t length;
	uint8_t data[0];
} __attribute__ ((packed));

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avdtp_media_codec_capability {
	uint8_t rfa0:4;
	uint8_t media_type:4;
	uint8_t media_codec_type;
	uint8_t data[0];
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avdtp_media_codec_capability {
	uint8_t media_type:4;
	uint8_t rfa0:4;
	uint8_t media_codec_type;
	uint8_t data[0];
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

typedef void (*avdtp_stream_state_cb) (struct avdtp_stream *stream,
					avdtp_state_t old_state,
					avdtp_state_t new_state,
					struct avdtp_error *err,
					void *user_data);

typedef void (*avdtp_set_configuration_cb) (struct avdtp *session,
						struct avdtp_stream *stream,
						struct avdtp_error *err);

/* Callbacks for when a reply is received to a command that we sent */
struct avdtp_sep_cfm {
	void (*set_configuration) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					struct avdtp_error *err,
					void *user_data);
	void (*get_configuration) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					struct avdtp_error *err,
					void *user_data);
	void (*open) (struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data);
	void (*start) (struct avdtp *session, struct avdtp_local_sep *lsep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data);
	void (*suspend) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data);
	void (*close) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data);
	void (*abort) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data);
	void (*reconfigure) (struct avdtp *session,
				struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data);
	void (*delay_report) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data);
};

/*
 * Callbacks for indicating when we received a new command. The return value
 * indicates whether the command should be rejected or accepted
 */
struct avdtp_sep_ind {
	gboolean (*get_capability) (struct avdtp *session,
					struct avdtp_local_sep *sep,
					GSList **caps, uint8_t *err,
					void *user_data);
	gboolean (*set_configuration) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					struct avdtp_stream *stream,
					GSList *caps,
					avdtp_set_configuration_cb cb,
					void *user_data);
	gboolean (*get_configuration) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					uint8_t *err, void *user_data);
	gboolean (*open) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data);
	gboolean (*start) (struct avdtp *session, struct avdtp_local_sep *lsep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data);
	gboolean (*suspend) (struct avdtp *session,
				struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data);
	gboolean (*close) (struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data);
	void (*abort) (struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data);
	gboolean (*reconfigure) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					uint8_t *err, void *user_data);
	gboolean (*delayreport) (struct avdtp *session,
					struct avdtp_local_sep *lsep,
					uint8_t rseid, uint16_t delay,
					uint8_t *err, void *user_data);
};

typedef void (*avdtp_discover_cb_t) (struct avdtp *session, GSList *seps,
					struct avdtp_error *err, void *user_data);
typedef void (*avdtp_disconnect_cb_t) (void *user_data);

struct avdtp *avdtp_new(int fd, size_t imtu, size_t omtu, uint16_t version,
							struct queue *lseps);

unsigned int avdtp_add_disconnect_cb(struct avdtp *session,
						avdtp_disconnect_cb_t cb,
						void *user_data);
gboolean avdtp_remove_disconnect_cb(struct avdtp *session, unsigned int id);

void avdtp_shutdown(struct avdtp *session);

void avdtp_unref(struct avdtp *session);
struct avdtp *avdtp_ref(struct avdtp *session);

struct avdtp_service_capability *avdtp_service_cap_new(uint8_t category,
							const void *data,
							int size);

struct avdtp_service_capability *avdtp_get_codec(struct avdtp_remote_sep *sep);

int avdtp_discover(struct avdtp *session, avdtp_discover_cb_t cb,
			void *user_data);

gboolean avdtp_has_stream(struct avdtp *session, struct avdtp_stream *stream);

unsigned int avdtp_stream_add_cb(struct avdtp *session,
					struct avdtp_stream *stream,
					avdtp_stream_state_cb cb, void *data);
gboolean avdtp_stream_remove_cb(struct avdtp *session,
				struct avdtp_stream *stream,
				unsigned int id);

gboolean avdtp_stream_set_transport(struct avdtp_stream *stream, int fd,
						size_t imtu, size_t omtu);
gboolean avdtp_stream_get_transport(struct avdtp_stream *stream, int *sock,
					uint16_t *imtu, uint16_t *omtu,
					GSList **caps);
struct avdtp_service_capability *avdtp_stream_get_codec(
						struct avdtp_stream *stream);
gboolean avdtp_stream_has_capabilities(struct avdtp_stream *stream,
					GSList *caps);
struct avdtp_remote_sep *avdtp_stream_get_remote_sep(
						struct avdtp_stream *stream);

int avdtp_set_configuration(struct avdtp *session,
				struct avdtp_remote_sep *rsep,
				struct avdtp_local_sep *lsep,
				GSList *caps,
				struct avdtp_stream **stream);

int avdtp_get_configuration(struct avdtp *session,
				struct avdtp_stream *stream);

int avdtp_open(struct avdtp *session, struct avdtp_stream *stream);
int avdtp_start(struct avdtp *session, struct avdtp_stream *stream);
int avdtp_suspend(struct avdtp *session, struct avdtp_stream *stream);
int avdtp_close(struct avdtp *session, struct avdtp_stream *stream,
		gboolean immediate);
int avdtp_abort(struct avdtp *session, struct avdtp_stream *stream);
int avdtp_delay_report(struct avdtp *session, struct avdtp_stream *stream,
							uint16_t delay);

struct avdtp_local_sep *avdtp_register_sep(struct queue *lseps, uint8_t type,
						uint8_t media_type,
						uint8_t codec_type,
						gboolean delay_reporting,
						struct avdtp_sep_ind *ind,
						struct avdtp_sep_cfm *cfm,
						void *user_data);
void avdtp_sep_set_vendor_codec(struct avdtp_local_sep *sep, uint32_t vendor_id,
							uint16_t codec_id);

/* Find a matching pair of local and remote SEP ID's */
struct avdtp_remote_sep *avdtp_find_remote_sep(struct avdtp *session,
						struct avdtp_local_sep *lsep);

int avdtp_unregister_sep(struct queue *lseps, struct avdtp_local_sep *sep);

avdtp_state_t avdtp_sep_get_state(struct avdtp_local_sep *sep);

void avdtp_error_init(struct avdtp_error *err, uint8_t type, int id);
const char *avdtp_strerror(struct avdtp_error *err);
uint8_t avdtp_error_category(struct avdtp_error *err);
int avdtp_error_error_code(struct avdtp_error *err);
int avdtp_error_posix_errno(struct avdtp_error *err);
