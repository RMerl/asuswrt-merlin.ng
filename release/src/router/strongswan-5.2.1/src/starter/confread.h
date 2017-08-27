/* strongSwan IPsec config file parser
 * Copyright (C) 2001-2002 Mathieu Lafon
 * Arkoon Network Security
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef _IPSEC_CONFREAD_H_
#define _IPSEC_CONFREAD_H_

#include <kernel/kernel_ipsec.h>

typedef enum {
		STARTUP_NO,
		STARTUP_ADD,
		STARTUP_ROUTE,
		STARTUP_START
} startup_t;

typedef enum {
		STATE_IGNORE,
		STATE_TO_ADD,
		STATE_ADDED,
		STATE_REPLACED,
		STATE_INVALID
} starter_state_t;

typedef enum {
		/* shared with ike_version_t */
		KEY_EXCHANGE_IKE = 0,
		KEY_EXCHANGE_IKEV1 = 1,
		KEY_EXCHANGE_IKEV2 = 2,
} keyexchange_t;

typedef enum {
		STRICT_NO,
		STRICT_YES,
		STRICT_IFURI,
} strict_t;

typedef enum {
		CERT_ALWAYS_SEND,
		CERT_SEND_IF_ASKED,
		CERT_NEVER_SEND,
		CERT_YES_SEND,		/* synonym for CERT_ALWAYS_SEND */
		CERT_NO_SEND,		/* synonym for CERT_NEVER_SEND */
} certpolicy_t;

typedef enum {
		DPD_ACTION_NONE,
		DPD_ACTION_CLEAR,
		DPD_ACTION_HOLD,
		DPD_ACTION_RESTART,
		DPD_ACTION_UNKNOW,
} dpd_action_t;

typedef enum {
		/* same as in ike_cfg.h */
		FRAGMENTATION_NO,
		FRAGMENTATION_YES,
		FRAGMENTATION_FORCE,
} fragmentation_t;

typedef enum {
		/* IPsec options */
		SA_OPTION_COMPRESS      = 1 << 1, /* use IPComp */

		/* IKE and other other options */
		SA_OPTION_DONT_REKEY	= 1 << 2, /* don't rekey state either Phase */
		SA_OPTION_DONT_REAUTH	= 1 << 3, /* don't reauthenticate on rekeying, IKEv2 only */
		SA_OPTION_MODECFG_PUSH	= 1 << 4, /* is modecfg pushed by server? */
		SA_OPTION_XAUTH_SERVER  = 1 << 5, /* are we an XAUTH server? */
		SA_OPTION_MOBIKE		= 1 << 6, /* enable MOBIKE for IKEv2  */
		SA_OPTION_FORCE_ENCAP   = 1 << 7, /* force UDP encapsulation */
} sa_option_t;

typedef struct starter_end starter_end_t;

struct starter_end {
		char            *auth;
		char            *auth2;
		char            *id;
		char            *id2;
		char            *rsakey;
		char            *cert;
		char            *cert2;
		char            *ca;
		char            *ca2;
		char            *groups;
		char            *groups2;
		char            *cert_policy;
		char            *host;
		u_int           ikeport;
		char            *subnet;
		bool            modecfg;
		certpolicy_t    sendcert;
		bool            firewall;
		bool            hostaccess;
		bool            allow_any;
		char            *updown;
		u_int16_t       from_port;
		u_int16_t       to_port;
		u_int8_t        protocol;
		char            *sourceip;
		char            *dns;
};

typedef struct starter_conn starter_conn_t;

struct starter_conn {
		char            *name;
		startup_t       startup;
		starter_state_t state;

		keyexchange_t   keyexchange;
		char            *eap_identity;
		char            *aaa_identity;
		char            *xauth_identity;
		char            *authby;
		ipsec_mode_t    mode;
		bool            proxy_mode;
		fragmentation_t fragmentation;
		u_int           ikedscp;
		sa_option_t     options;
		time_t          sa_ike_life_seconds;
		time_t          sa_ipsec_life_seconds;
		time_t          sa_rekey_margin;
		u_int64_t       sa_ipsec_life_bytes;
		u_int64_t       sa_ipsec_margin_bytes;
		u_int64_t       sa_ipsec_life_packets;
		u_int64_t       sa_ipsec_margin_packets;
		unsigned long   sa_keying_tries;
		unsigned long   sa_rekey_fuzz;
		u_int32_t       reqid;
		mark_t          mark_in;
		mark_t          mark_out;
		u_int32_t       replay_window;
		u_int32_t       tfc;
		bool            install_policy;
		bool            aggressive;
		starter_end_t   left, right;

		unsigned long   id;

		char            *esp;
		char            *ah;
		char            *ike;

		time_t          dpd_delay;
		time_t          dpd_timeout;
		dpd_action_t    dpd_action;
		int             dpd_count;

		dpd_action_t    close_action;

		time_t          inactivity;

		bool            me_mediation;
		char            *me_mediated_by;
		char            *me_peerid;

		starter_conn_t *next;
};

typedef struct starter_ca starter_ca_t;

struct starter_ca {
		char            *name;
		startup_t       startup;
		starter_state_t state;

		char            *cacert;
		char            *crluri;
		char            *crluri2;
		char            *ocspuri;
		char            *ocspuri2;
		char            *certuribase;

		bool            strict;

		starter_ca_t    *next;
};

typedef struct starter_config starter_config_t;

struct starter_config {
		struct {
				bool     charonstart;
				char     *charondebug;
				bool     uniqueids;
				bool     cachecrls;
				strict_t strictcrlpolicy;
		} setup;

		/* number of encountered parsing errors */
		u_int err;
		u_int non_fatal_err;

		/* connections list */
		starter_ca_t *ca_first, *ca_last;

		/* connections list */
		starter_conn_t *conn_first, *conn_last;
};

starter_config_t *confread_load(const char *file);
void confread_free(starter_config_t *cfg);

#endif /* _IPSEC_CONFREAD_H_ */
