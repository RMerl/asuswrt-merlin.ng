/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

/**
 * @defgroup error_notify_msg error_notify_msg
 * @{ @ingroup error_notify
 */

#ifndef ERROR_NOTIFY_MSG_H_
#define ERROR_NOTIFY_MSG_H_

#define ERROR_NOTIFY_SOCKET IPSEC_PIDDIR "/charon.enfy"

typedef struct error_notify_msg_t error_notify_msg_t;

/**
 * Message type, these are mapped to ALERT_* types.
 */
enum {
	ERROR_NOTIFY_RADIUS_NOT_RESPONDING = 1,
	ERROR_NOTIFY_LOCAL_AUTH_FAILED = 2,
	ERROR_NOTIFY_PEER_AUTH_FAILED = 3,
	ERROR_NOTIFY_PARSE_ERROR_HEADER = 4,
	ERROR_NOTIFY_PARSE_ERROR_BODY = 5,
	ERROR_NOTIFY_RETRANSMIT_SEND_TIMEOUT = 6,
	ERROR_NOTIFY_HALF_OPEN_TIMEOUT = 7,
	ERROR_NOTIFY_PROPOSAL_MISMATCH_IKE = 8,
	ERROR_NOTIFY_PROPOSAL_MISMATCH_CHILD = 9,
	ERROR_NOTIFY_TS_MISMATCH = 10,
	ERROR_NOTIFY_INSTALL_CHILD_SA_FAILED = 11,
	ERROR_NOTIFY_INSTALL_CHILD_POLICY_FAILED = 12,
	ERROR_NOTIFY_UNIQUE_REPLACE = 13,
	ERROR_NOTIFY_UNIQUE_KEEP = 14,
	ERROR_NOTIFY_VIP_FAILURE = 15,
	ERROR_NOTIFY_AUTHORIZATION_FAILED = 16,
	ERROR_NOTIFY_CERT_EXPIRED = 17,
	ERROR_NOTIFY_CERT_REVOKED = 18,
	ERROR_NOTIFY_NO_ISSUER_CERT = 19,
};

/**
 * Message to exchange over notify socket, strings are null-terminated.
 */
struct error_notify_msg_t {
	/** message type */
	int type;
	/** string with an error description */
	char str[384];
	/** connection name, if known */
	char name[64];
	/** peer identity, if known */
	char id[256];
	/** peer address and port, if known */
	char ip[60];
} __attribute__((packed));

#endif /** ERROR_NOTIFY_MSG_H_ @}*/
