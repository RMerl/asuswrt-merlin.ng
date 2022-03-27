/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
 *  Copyright (C) 2010 Signove
 *  Copyright (C) 2014 Intel Corporation. All rights reserved.
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

#define MCAP_VERSION	0x0100	/* current version 01.00 */

/* bytes to get MCAP Supported Procedures */
#define MCAP_SUP_PROC	0x06

/* maximum transmission unit for channels */
#define MCAP_CC_MTU	48
#define MCAP_DC_MTU	65535

/* MCAP Standard Op Codes */
#define MCAP_ERROR_RSP			0x00
#define MCAP_MD_CREATE_MDL_REQ		0x01
#define MCAP_MD_CREATE_MDL_RSP		0x02
#define MCAP_MD_RECONNECT_MDL_REQ	0x03
#define MCAP_MD_RECONNECT_MDL_RSP	0x04
#define MCAP_MD_ABORT_MDL_REQ		0x05
#define MCAP_MD_ABORT_MDL_RSP		0x06
#define MCAP_MD_DELETE_MDL_REQ		0x07
#define MCAP_MD_DELETE_MDL_RSP		0x08

/* MCAP Clock Sync Op Codes */
#define MCAP_MD_SYNC_CAP_REQ		0x11
#define MCAP_MD_SYNC_CAP_RSP		0x12
#define MCAP_MD_SYNC_SET_REQ		0x13
#define MCAP_MD_SYNC_SET_RSP		0x14
#define MCAP_MD_SYNC_INFO_IND		0x15

/* MCAP Response codes */
#define MCAP_SUCCESS			0x00
#define MCAP_INVALID_OP_CODE		0x01
#define MCAP_INVALID_PARAM_VALUE	0x02
#define MCAP_INVALID_MDEP		0x03
#define MCAP_MDEP_BUSY			0x04
#define MCAP_INVALID_MDL		0x05
#define MCAP_MDL_BUSY			0x06
#define MCAP_INVALID_OPERATION		0x07
#define MCAP_RESOURCE_UNAVAILABLE	0x08
#define MCAP_UNSPECIFIED_ERROR		0x09
#define MCAP_REQUEST_NOT_SUPPORTED	0x0A
#define MCAP_CONFIGURATION_REJECTED	0x0B

/* MDL IDs */
#define MCAP_MDLID_RESERVED		0x0000
#define MCAP_MDLID_INITIAL		0x0001
#define MCAP_MDLID_FINAL		0xFEFF
#define MCAP_ALL_MDLIDS			0xFFFF

/* MDEP IDs */
#define MCAP_MDEPID_INITIAL		0x00
#define MCAP_MDEPID_FINAL		0x7F

/* CSP special values */
#define MCAP_BTCLOCK_IMMEDIATE		0xffffffffUL
#define MCAP_TMSTAMP_DONTSET		0xffffffffffffffffULL
#define MCAP_BTCLOCK_MAX		0x0fffffff
#define MCAP_BTCLOCK_FIELD		(MCAP_BTCLOCK_MAX + 1)

#define	MCAP_CTRL_CACHED	0x01	/* MCL is cached */
#define	MCAP_CTRL_STD_OP	0x02	/* Support for standard op codes */
#define	MCAP_CTRL_SYNC_OP	0x04	/* Support for synchronization commands */
#define	MCAP_CTRL_CONN		0x08	/* MCL is in connecting process */
#define	MCAP_CTRL_FREE		0x10	/* MCL is marked as releasable */
#define	MCAP_CTRL_NOCACHE	0x20	/* MCL is marked as not cacheable */

/*
 * MCAP Request Packet Format
 */

typedef struct {
	uint8_t		op;
	uint16_t	mdl;
	uint8_t		mdep;
	uint8_t		conf;
} __attribute__ ((packed)) mcap_md_create_mdl_req;

typedef struct {
	uint8_t		op;
	uint16_t	mdl;
} __attribute__ ((packed)) mcap_md_req;

/* MCAP Response Packet Format */

typedef struct {
	uint8_t		op;
	uint8_t		rc;
	uint16_t	mdl;
	uint8_t		data[0];
} __attribute__ ((packed)) mcap_rsp;

/*  MCAP Clock Synchronization Protocol */

typedef struct {
	uint8_t		op;
	uint16_t	timest;
} __attribute__ ((packed)) mcap_md_sync_cap_req;

typedef struct {
	uint8_t		op;
	uint8_t		rc;
} __attribute__ ((packed)) mcap_md_sync_rsp;

typedef struct {
	uint8_t		op;
	uint8_t		rc;
	uint8_t		btclock;
	uint16_t	sltime;
	uint16_t	timestnr;
	uint16_t	timestna;
} __attribute__ ((packed)) mcap_md_sync_cap_rsp;

typedef struct {
	uint8_t		op;
	uint8_t		timestui;
	uint32_t	btclock;
	uint64_t	timestst;
} __attribute__ ((packed)) mcap_md_sync_set_req;

typedef struct {
	int8_t		op;
	uint8_t		rc;
	uint32_t	btclock;
	uint64_t	timestst;
	uint16_t	timestsa;
} __attribute__ ((packed)) mcap_md_sync_set_rsp;

typedef struct {
	uint8_t		op;
	uint32_t	btclock;
	uint64_t	timestst;
	uint16_t	timestsa;
} __attribute__ ((packed)) mcap_md_sync_info_ind;

typedef enum {
/* MCAP Error Response Codes */
	MCAP_ERROR_INVALID_OP_CODE = 1,
	MCAP_ERROR_INVALID_PARAM_VALUE,
	MCAP_ERROR_INVALID_MDEP,
	MCAP_ERROR_MDEP_BUSY,
	MCAP_ERROR_INVALID_MDL,
	MCAP_ERROR_MDL_BUSY,
	MCAP_ERROR_INVALID_OPERATION,
	MCAP_ERROR_RESOURCE_UNAVAILABLE,
	MCAP_ERROR_UNSPECIFIED_ERROR,
	MCAP_ERROR_REQUEST_NOT_SUPPORTED,
	MCAP_ERROR_CONFIGURATION_REJECTED,
/* MCAP Internal Errors */
	MCAP_ERROR_INVALID_ARGS,
	MCAP_ERROR_ALREADY_EXISTS,
	MCAP_ERROR_REQ_IGNORED,
	MCAP_ERROR_MCL_CLOSED,
	MCAP_ERROR_FAILED
} McapError;

typedef enum {
	MCAP_MDL_CB_INVALID,
	MCAP_MDL_CB_CONNECTED,		/* mcap_mdl_event_cb */
	MCAP_MDL_CB_CLOSED,		/* mcap_mdl_event_cb */
	MCAP_MDL_CB_DELETED,		/* mcap_mdl_event_cb */
	MCAP_MDL_CB_ABORTED,		/* mcap_mdl_event_cb */
	MCAP_MDL_CB_REMOTE_CONN_REQ,	/* mcap_remote_mdl_conn_req_cb */
	MCAP_MDL_CB_REMOTE_RECONN_REQ	/* mcap_remote_mdl_reconn_req_cb */
} McapMclCb;

typedef enum {
	MCL_CONNECTED,
	MCL_PENDING,
	MCL_ACTIVE,
	MCL_IDLE
} MCLState;

typedef enum {
	MCL_ACCEPTOR,
	MCL_INITIATOR
} MCLRole;

typedef enum {
	MCL_AVAILABLE,
	MCL_WAITING_RSP
} MCAPCtrl;

typedef enum {
	MDL_WAITING,
	MDL_CONNECTED,
	MDL_DELETING,
	MDL_CLOSED
} MDLState;

struct mcap_csp;
struct mcap_mdl_op_cb;
struct mcap_instance;
struct mcap_mcl;
struct mcap_mdl;
struct sync_info_ind_data;

/************ Callbacks ************/

/* MDL callbacks */

typedef void (* mcap_mdl_event_cb) (struct mcap_mdl *mdl, gpointer data);
typedef void (* mcap_mdl_operation_conf_cb) (struct mcap_mdl *mdl, uint8_t conf,
						GError *err, gpointer data);
typedef void (* mcap_mdl_operation_cb) (struct mcap_mdl *mdl, GError *err,
						gpointer data);
typedef void (* mcap_mdl_notify_cb) (GError *err, gpointer data);

/* Next function should return an MCAP appropriate response code */
typedef uint8_t (* mcap_remote_mdl_conn_req_cb) (struct mcap_mcl *mcl,
						uint8_t mdepid, uint16_t mdlid,
						uint8_t *conf, gpointer data);
typedef uint8_t (* mcap_remote_mdl_reconn_req_cb) (struct mcap_mdl *mdl,
						gpointer data);

/* MCL callbacks */

typedef void (* mcap_mcl_event_cb) (struct mcap_mcl *mcl, gpointer data);
typedef void (* mcap_mcl_connect_cb) (struct mcap_mcl *mcl, GError *err,
								gpointer data);

/* CSP callbacks */

typedef void (* mcap_info_ind_event_cb) (struct mcap_mcl *mcl,
					struct sync_info_ind_data *data);

typedef void (* mcap_sync_cap_cb) (struct mcap_mcl *mcl,
					uint8_t mcap_err,
					uint8_t btclockres,
					uint16_t synclead,
					uint16_t tmstampres,
					uint16_t tmstampacc,
					GError *err,
					gpointer data);

typedef void (* mcap_sync_set_cb) (struct mcap_mcl *mcl,
					uint8_t mcap_err,
					uint32_t btclock,
					uint64_t timestamp,
					uint16_t accuracy,
					GError *err,
					gpointer data);

struct mcap_mdl_cb {
	mcap_mdl_event_cb		mdl_connected;	/* Remote device has created a MDL */
	mcap_mdl_event_cb		mdl_closed;	/* Remote device has closed a MDL */
	mcap_mdl_event_cb		mdl_deleted;	/* Remote device requested deleting a MDL */
	mcap_mdl_event_cb		mdl_aborted;	/* Remote device aborted the mdl creation */
	mcap_remote_mdl_conn_req_cb	mdl_conn_req;	/* Remote device requested creating a MDL */
	mcap_remote_mdl_reconn_req_cb	mdl_reconn_req;	/* Remote device requested reconnecting a MDL */
	gpointer			user_data;	/* User data */
};

struct mcap_instance {
	bdaddr_t		src;			/* Source address */
	GIOChannel		*ccio;			/* Control Channel IO */
	GIOChannel		*dcio;			/* Data Channel IO */
	GSList			*mcls;			/* MCAP instance list */
	GSList			*cached;		/* List with all cached MCLs (MAX_CACHED macro) */
	BtIOSecLevel		sec;			/* Security level */
	mcap_mcl_event_cb	mcl_connected_cb;	/* New MCL connected */
	mcap_mcl_event_cb	mcl_reconnected_cb;	/* Old MCL has been reconnected */
	mcap_mcl_event_cb	mcl_disconnected_cb;	/* MCL disconnected */
	mcap_mcl_event_cb	mcl_uncached_cb;	/* MCL has been removed from MCAP cache */
	mcap_info_ind_event_cb	mcl_sync_infoind_cb;	/* (CSP Master) Received info indication */
	gpointer		user_data;		/* Data to be provided in callbacks */
	int			ref;			/* Reference counter */

	gboolean		csp_enabled;		/* CSP: functionality enabled */
};

struct mcap_mcl {
	struct mcap_instance	*mi;		/* MCAP instance where this MCL belongs */
	bdaddr_t		addr;		/* Device address */
	GIOChannel		*cc;		/* MCAP Control Channel IO */
	guint			wid;		/* MCL Watcher id */
	GSList			*mdls;		/* List of Data Channels shorted by mdlid */
	MCLState		state;		/* Current MCL State */
	MCLRole			role;		/* Initiator or acceptor of this MCL */
	MCAPCtrl		req;		/* Request control flag */
	struct mcap_mdl_op_cb	*priv_data;	/* Temporal data to manage responses */
	struct mcap_mdl_cb	*cb;		/* MDL callbacks */
	guint			tid;		/* Timer id for waiting for a response */
	uint8_t			*lcmd;		/* Last command sent */
	int			ref;		/* References counter */
	uint8_t			ctrl;		/* MCL control flag */
	uint16_t		next_mdl;	/* id used to create next MDL */
	struct mcap_csp		*csp;		/* CSP control structure */
};

struct mcap_mdl {
	struct mcap_mcl		*mcl;		/* MCL where this MDL belongs */
	GIOChannel		*dc;		/* MCAP Data Channel IO */
	guint			wid;		/* MDL Watcher id */
	uint16_t		mdlid;		/* MDL id */
	uint8_t			mdep_id;	/* MCAP Data End Point */
	MDLState		state;		/* MDL state */
	int			ref;		/* References counter */
};

struct sync_info_ind_data {
	uint32_t	btclock;
	uint64_t	timestamp;
	uint16_t	accuracy;
};

/************ Operations ************/

/* MDL operations */

gboolean mcap_create_mdl(struct mcap_mcl *mcl,
				uint8_t mdepid,
				uint8_t conf,
				mcap_mdl_operation_conf_cb connect_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
gboolean mcap_reconnect_mdl(struct mcap_mdl *mdl,
				mcap_mdl_operation_cb reconnect_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
gboolean mcap_delete_all_mdls(struct mcap_mcl *mcl,
				mcap_mdl_notify_cb delete_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
gboolean mcap_delete_mdl(struct mcap_mdl *mdl,
				mcap_mdl_notify_cb delete_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
gboolean mcap_connect_mdl(struct mcap_mdl *mdl,
				uint8_t mode,
				uint16_t dcpsm,
				mcap_mdl_operation_cb connect_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
gboolean mcap_mdl_abort(struct mcap_mdl *mdl,
				mcap_mdl_notify_cb abort_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);

int mcap_mdl_get_fd(struct mcap_mdl *mdl);
uint16_t mcap_mdl_get_mdlid(struct mcap_mdl *mdl);
struct mcap_mdl *mcap_mdl_ref(struct mcap_mdl *mdl);
void mcap_mdl_unref(struct mcap_mdl *mdl);

/* MCL operations */

gboolean mcap_create_mcl(struct mcap_instance *mi,
				const bdaddr_t *addr,
				uint16_t ccpsm,
				mcap_mcl_connect_cb connect_cb,
				gpointer user_data,
				GDestroyNotify destroy,
				GError **err);
void mcap_close_mcl(struct mcap_mcl *mcl, gboolean cache);
gboolean mcap_mcl_set_cb(struct mcap_mcl *mcl, gpointer user_data,
					GError **gerr, McapMclCb cb1, ...);
void mcap_mcl_get_addr(struct mcap_mcl *mcl, bdaddr_t *addr);
struct mcap_mcl *mcap_mcl_ref(struct mcap_mcl *mcl);
void mcap_mcl_unref(struct mcap_mcl *mcl);

/* CSP operations */

void mcap_enable_csp(struct mcap_instance *mi);
void mcap_disable_csp(struct mcap_instance *mi);
uint64_t mcap_get_timestamp(struct mcap_mcl *mcl,
				struct timespec *given_time);
uint32_t mcap_get_btclock(struct mcap_mcl *mcl);

void mcap_sync_cap_req(struct mcap_mcl *mcl,
			uint16_t reqacc,
			mcap_sync_cap_cb cb,
			gpointer user_data,
			GError **err);

void mcap_sync_set_req(struct mcap_mcl *mcl,
			uint8_t update,
			uint32_t btclock,
			uint64_t timestamp,
			mcap_sync_set_cb cb,
			gpointer user_data,
			GError **err);

/* MCAP main operations */

struct mcap_instance *mcap_create_instance(const bdaddr_t *src,
					BtIOSecLevel sec, uint16_t ccpsm,
					uint16_t dcpsm,
					mcap_mcl_event_cb mcl_connected,
					mcap_mcl_event_cb mcl_reconnected,
					mcap_mcl_event_cb mcl_disconnected,
					mcap_mcl_event_cb mcl_uncached,
					mcap_info_ind_event_cb mcl_sync_info_ind,
					gpointer user_data,
					GError **gerr);
void mcap_release_instance(struct mcap_instance *mi);

struct mcap_instance *mcap_instance_ref(struct mcap_instance *mi);
void mcap_instance_unref(struct mcap_instance *mi);

uint16_t mcap_get_ctrl_psm(struct mcap_instance *mi, GError **err);
uint16_t mcap_get_data_psm(struct mcap_instance *mi, GError **err);

gboolean mcap_set_data_chan_mode(struct mcap_instance *mi, uint8_t mode,
								GError **err);

int mcap_send_data(int sock, const void *buf, uint32_t size);

void proc_sync_cmd(struct mcap_mcl *mcl, uint8_t *cmd, uint32_t len);
void mcap_sync_init(struct mcap_mcl *mcl);
void mcap_sync_stop(struct mcap_mcl *mcl);
