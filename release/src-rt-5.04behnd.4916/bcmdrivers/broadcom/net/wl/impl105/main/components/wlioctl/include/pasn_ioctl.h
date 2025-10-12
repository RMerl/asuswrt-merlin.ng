/*
 * PASN module IOCTL structure definitions.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id$
 */

#ifndef _pasn_ioctl_h
#define _pasn_ioctl_h

#define WL_PASN_VERSION_1	 0x0001

typedef uint8 wl_pasn_policy_t;

/* PASN subcommands ID. */
enum {
	WL_PASN_CMD_NONE		= 0,
	/* get PASN IOVAR version. */
	WL_PASN_CMD_GET_VERSION		= 1,
	/* set, enable PASN. */
	WL_PASN_CMD_ENABLE		= 2,
	/* set, disable PASN. */
	WL_PASN_CMD_DISABLE		= 3,
	/* get/set, PASN finite cyclic group id. */
	WL_PASN_CMD_CONFIG_GROUP	= 4,
	/* get/set, PASN authentication policy. */
	WL_PASN_CMD_CONFIG_POLICY	= 5,
	/* get/set, The key lifetime in minutes. */
	WL_PASN_CMD_CONFIG_KEY_LIFETIME_MIN	= 6,
	/* get/set, peer mac address of PASN session. */
	WL_PASN_CMD_CONFIG_PEER_ADDR	= 7,
	/* get/set, local mac address of PASN session. */
	WL_PASN_CMD_CONFIG_LOCAL_ADDR	= 8,
	/* get/set, chanspec of PASN session. */
	WL_PASN_CMD_CONFIG_CHANSPEC	= 9,
	/* set, start PASN session. */
	WL_PASN_CMD_START		= 10,
	/* set, stop PASN session. */
	WL_PASN_CMD_STOP		= 11,
	/* set, delete PASN session. */
	WL_PASN_CMD_DELETE		= 12,
	/* get info for PASN session with given id - wl_pasn_session_info_t. */
	WL_PASN_CMD_GET_INFO		= 13,
	/* Note: for debug purpose or future event - wl_pasn_key_info_t. */
	WL_PASN_CMD_GET_KEY_INFO	= 14,
	/* get/set. uint32. PASN event mask. */
	WL_PASN_CMD_EVENT_MASK		= 15,
	/* get. List id of active sessions. */
	WL_PASN_CMD_LIST_SESSIONS	= 16,
	/* get/set, flags of PASN session. */
	WL_PASN_CMD_CONFIG_FLAGS	= 17,
	/* get/set. uint32. PASN key idle time. */
	WL_PASN_CMD_KEY_IDLE_TIME	= 18,
	/* set PASN PTK, LTF key seed */
	WL_PASN_CMD_SET_SESSION_KEY	= 19,
	/* get/set. for testing purpose. */
	WL_PASN_CMD_TEST		= 0xfffe
};
typedef uint16 wl_pasn_cmd_t;

enum {
	/* No test action */
	WL_PASN_CMD_TEST_NONE		= 0,
	/* DUT to send auth frame with invalid MIC */
	WL_PASN_CMD_TEST_INVALID_MIC	= 1,
	/* DUT to send deauth frame to peer */
	WL_PASN_CMD_TEST_DEAUTH		= 2
};
typedef uint8 wl_pasn_cmd_test_id_t;

typedef uint16 wl_pasn_session_id_t;
/* Session id ranges are allocated to various users of PASN -
 * including internal use (e.g. automatically allocated IDs on an AP).
 * Other ranges can be defined in the future.
 */
enum {
	PASN_SID_TYPE_HOST	= 0,
	PASN_SID_TYPE_FTM	= 1,
	PASN_SID_TYPE_INTERNAL	= 2
};

#define PASN_SID_GLOBAL	0u

#define PASN_SID_HOST_START	1u
#define PASN_SID_HOST_END	1023u

#define PASN_SID_FTM_START	1024u
#define PASN_SID_FTM_END	2047u

#define PASN_SID_NAN_START	2048u
#define PASN_SID_NAN_END	3071u

#define PASN_SID_INTERNAL_START	3072u
#define PASN_SID_INTERNAL_END	32767u

typedef struct wl_pasn_iov {
	uint16			version; /* structure version will be incremented
					* when header is changed.
					*/
	uint16			len;	/* includes the id field and following data. */
	wl_pasn_cmd_t		cmd;	/* sub-command id. */
	wl_pasn_session_id_t	id;	/* session id is the input for session specific commands
					* and a reserved value for global commands.
					*/
	uint8			data[];	/* variable */
} wl_pasn_iov_t;

/* The policy used in the PASN exchange. */
/*
 * ALLOW_NO_PMKSA may be set independent of USE_PMKSA or SETUP_PMKSA.
 * SETUP_PMKSA implies USE_PMKSA i.e. attempt to use existing PMKSA before
 * attempting setup. Ignore USE_PMKSA when SETUP_PMKSA is set.
 * if USE_PMKSA or SETUP_PMKSA are not set, attempt w/o PMKSA - OWE like
 * USE_PMKSA does not imply SETUP_PMKSA - i.e. we wont attempt setting up PASN via tunneling.
 */
enum {
	WL_PASN_POLICY_NONE		= 0, /* Not used. */
	WL_PASN_POLICY_ALLOW_NO_PMKSA	= 1, /* allow no pmksa, fixed pmk */
	WL_PASN_POLICY_USE_PMKSA	= 2, /* use existing pmksa for base akm */
	/* 3 is reserved for bit "OR" of ALLOW_NO_PMKSA and USE_PMKSA. */
	WL_PASN_POLICY_SETUP_PMKSA	= 4, /* setup PMKSA using base akm */
	WL_PASN_POLICY_MAX		= WL_PASN_POLICY_SETUP_PMKSA
};

enum {
	/* Initial state of PASN state machine.
	 * Initialization is allowed if PASN is enabled via configuration.
	 */
	WL_PASN_STATE_IDLE		= 0,
	/* For STA, Initiate an instance requested by user, ex. FTM.
	 * For AP, Receive PASN authentication frame with sequence 1.
	 */
	WL_PASN_STATE_INIT		= 1,
	/* STA only. Auth frame is constructed and ready for
	 * transmission. STA request for channel scheduling.
	 * It is not necessary to AP because AP will not be off channel.
	 */
	WL_PASN_STATE_SCHED_WAIT	= 2,
	/* STA only. PASN is waiting for desired security
	 * methods(FILS/SAE/FBT) t obuild tunneled protocol data for authentication
	 * frame with sequence 1.
	 */
	WL_PASN_STATE_DATA_WAIT_1	= 3,
	/* STA only. PASN authentication frame with sequence
	 * 1 is constructed and pushed to FIFO, waiting for ACK.
	 */
	WL_PASN_STATE_WAIT_ACK_1	= 4,
	/* STA only. STA is waiting for authentication frame with sequence 2. */
	WL_PASN_STATE_WAIT_AUTH_2	= 5,
	/* STA: PASN is waiting for desired security
	 * methods(FILS/SAE/FBT) to build tunneled protocol data for authentication
	 * frame with sequence 3.
	 * AP: PASN is waiting for desired security module to validate the tunneled
	 * protocol data in received authentication frame with sequence 3.
	 */
	WL_PASN_STATE_DATA_WAIT_3	= 6,
	/* STA only. PASN authentication frame with sequence 3
	 * is constructed and pushed to FIFO, waiting for ACK.
	 */
	WL_PASN_STATE_WAIT_ACK_3	= 7,
	/* STA only. The received PASN authentication frame
	 * with sequence 2 has status code as REFUSED_TEMPORARILY,
	 * and the comeback info field in PASN parameters element indicates
	 * the time for STA to retry the authentication later.
	 */
	WL_PASN_STATE_COME_BACK		= 8,
	/* AP only. PASN is waiting for desired security
	 * methods(FILS/SAE/FBT) to build tunneled protocol data for
	 * authentication frame with sequence 2.
	 */
	WL_PASN_STATE_DATA_WAIT_2	= 9,
	/* AP only. PASN authentication frame with
	 * sequence 2 is constructed and pushed to FIFO, waiting for ACK.
	 */
	WL_PASN_STATE_WAIT_ACK_2	= 10,
	/* AP only. AP is waiting for authentication frame with sequence 3. */
	WL_PASN_STATE_WAIT_AUTH_3	= 11,
	/* AP only. AP is waiting for key plumbing from external authenticator. */
	WL_PASN_STATE_WAIT_KEY_PLUMB	= 12,
	/* PASN exchange is done. */
	WL_PASN_STATE_DONE			= 13
};

typedef uint8 wl_pasn_session_state_t;
typedef int32 wl_pasn_status_t;

/* Global and bsscfg flags */
enum {
	/* PASN is enabled on bsscfg */
	WL_PASN_FLAG_BSSCFG_ENABLED	= 0x0001u
};
typedef uint16 wl_pasn_flags_t;

enum {
	/* PASN exchange will use PMKSA to derive PTKSA */
	WL_PASN_SESSION_FLAG_CACHED_PMK = 0x0001u,
	/* PASN exchange will setup PMKSA by tunneling protocol data */
	WL_PASN_SESSION_FLAG_TUNNELED_AKM = 0x0002u,
	/* PASN session will be deleted if error occurs */
	WL_PASN_SESSION_FLAG_DELETE_ON_ERR = 0x0004u,
	/* PASN session will issue scan with randmac */
	WL_PASN_SESSION_FLAG_RANDMAC = 0x0008u
};
typedef uint16 wl_pasn_session_flags_t;

/* PASN session state information. */
typedef struct wl_pasn_session_info {
	wl_pasn_session_id_t	id;	/* The id of pasn auth exchange. */
	wl_pasn_session_state_t	state;	/* The state of pasn auth exchange. */
	uint8			PAD[1];
	wl_pasn_status_t	status;	/* The status code of pasn auth exchange. bcmerror.h */
	struct ether_addr	local_addr; /* The source mac address of pasn auth exchange. */
	struct ether_addr	peer_addr; /* The peer mac address of pasn auth exchange. */
	uint8	akm;			/* The AKM used in pasn auth exchange.
					* see rsn_akm_t in bcmwpa.h
					*/
	uint8	pairwise_cipher;	/* The pairwise cipher used in pasn auth exchange. */
	uint16	comeback_after_ms;	/* time in miliseconds after which the non-AP STA is
					* requested to retry the PASN authentication.
					*/
	uint16	cyclic_group_id;	/* Finite cyclic group id, IANA id, see bcm_ec.h */
	wl_pasn_session_flags_t	flags;	/* session flags. */
	uint32	key_lifetime_ms;	/* key lifetime negotiated in authentication. */
} wl_pasn_session_info_t;

#define WL_PASN_MAX_CC_GROUP 6

/* PASN global information. */
typedef struct wl_pasn_info {
	wl_pasn_flags_t	flags;		/* flags. */
	wl_pasn_policy_t policy;	/* authentication policy supported. */
	uint8	auth_num_retries;	/* authentication frame retry limit. */
	uint8	num_groups;		/* number of cyclic groups. */
	uint8	max_auth_session;	/* Maximum number of sessions. */
	uint8	active_auth_session;	/* number of active sessions. */
	uint8	PAD[1];
	uint32	key_lifetime_min;	/* key lifetime in minutes. */
	uint32	event_mask;
	uint16	auth_timeout_ms;	/* timeout waiting for auth frame. */
	uint16	group_offset;		/* the offset of cyclic_group_id. */
	uint32	key_idle_sec;		/* Idle time of PASN keys in seconds. */
	/* uint16 cyclic_group_id[];
	 * Finite cyclic group id list, IANA id, see bcm_ec.h.
	 */
} wl_pasn_info_t;

/* Note: for storing key in pasn.
 *
 * stores TK, ltf key seed derived from PASN exchange.
 */
#define WPA_TK_MAX_LEN          32
#define WPA_LTF_KEYSEED_MAX_LEN 48
#define WPA_KDK_MAX_LEN         32
typedef struct wl_pasn_key_info {
	wl_pasn_session_id_t sid; /* The id of pasn auth exchange. */
	struct ether_addr local_addr; /* The source mac address of pasn auth exchange. */
	struct ether_addr peer_addr;	/* The peer mac address of pasn auth exchange. */
	uint32  cipher;		/* cipher to be used with PASN */
	uint16  tk_len;		/* cipher to be used with PASN */
	uint8   tk[WPA_TK_MAX_LEN]; /* transient key */
	uint16  ltf_keyseed_len;	/* ltf key seed len */
	uint8   ltf_keyseed[WPA_LTF_KEYSEED_MAX_LEN]; /* ltf key seed */
	uint16  kdk_len;
	uint8   kdk[WPA_KDK_MAX_LEN];
} wl_pasn_key_info_t;

/* WL_PASN_CMD_LIST_SESSIONS */
typedef struct wl_pasn_active_session_list {
	uint16	num_sids;
	wl_pasn_session_id_t	sids[];
} wl_pasn_active_session_list_t;

/* WL_PASN_CMD_CONFIG_GROUP */
typedef struct wl_pasn_group_id_list {
	uint16	num_groups;		/* number of cyclic groups. */
	uint16	cyclic_group_id[];	/* Finite cyclic group id list, IANA id, see bcm_ec.h. */
} wl_pasn_group_id_list_t;

/* pasn sub-event type. */
enum {
	WL_PASN_EVENT_NONE		= 0, /* reserved */
	WL_PASN_EVENT_CREATED		= 1, /* create a new session */
	WL_PASN_EVENT_STARTED		= 2, /* session is started */
	WL_PASN_EVENT_SCHED_WAIT	= 3, /* wait for channel scheduling . */
	WL_PASN_EVENT_DATA_WAIT		= 4, /* wait for wrapped data to be ready. */
	WL_PASN_EVENT_ACK_WAIT		= 5, /* wait for ACK */
	WL_PASN_EVENT_AUTH_WAIT		= 6, /* wait for next authentication frame from peer. */
	WL_PASN_EVENT_COME_BACK		= 7, /* wait for certain time to start auth again */
	WL_PASN_EVENT_DESTROYED		= 8, /* session is deleted. ie. PTKSA is deleted. */
	WL_PASN_EVENT_DONE		= 9, /* session is done. */
	WL_PASN_EVENT_REQUEST		= 10 /* Request PASN session setup. */
};
typedef uint16 wl_pasn_event_type_t;

/* pasn event */
typedef struct wl_pasn_event {
	uint16	version;
	uint16	len;			/* includes the entire structure and following data. */
	wl_pasn_session_id_t	sid;	/* session id */
	wl_pasn_event_type_t	type;	/* sub event type */
	wl_pasn_session_state_t	state;	/* current pasn authentication state.  */
	wl_pasn_session_state_t	prev_state; /* previous pasn authentication state.  */
	uint8	PAD[2];
	struct ether_addr       peer_addr; /* peer mac address to do PASN exchange. */
	uint8	data[];			/* variable */
} wl_pasn_event_t;

#endif /* _pasn_ioctl_h */
