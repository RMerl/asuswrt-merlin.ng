/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Microsoft Corporation
 * Author: Sean Edmond <seanedmond@microsoft.com>
 *
 */

#ifndef __DHCP6_H__
#define __DHCP6_H__

/* Message types */
#define DHCP6_MSG_SOLICIT	1
#define DHCP6_MSG_ADVERTISE	2
#define DHCP6_MSG_REQUEST	3
#define DHCP6_MSG_REPLY		7

/* Option Codes */
#define DHCP6_OPTION_CLIENTID		1
#define DHCP6_OPTION_SERVERID		2
#define DHCP6_OPTION_IA_NA		3
#define DHCP6_OPTION_IA_TA		4
#define DHCP6_OPTION_IAADDR		5
#define DHCP6_OPTION_ORO		6
#define DHCP6_OPTION_PREFERENCE		7
#define DHCP6_OPTION_ELAPSED_TIME	8
#define DHCP6_OPTION_STATUS_CODE	13
#define DHCP6_OPTION_OPT_BOOTFILE_URL	59
#define DHCP6_OPTION_OPT_BOOTFILE_PARAM	60
#define DHCP6_OPTION_SOL_MAX_RT		82
#define DHCP6_OPTION_CLIENT_ARCH_TYPE	61
#define DHCP6_OPTION_VENDOR_CLASS	16
#define DHCP6_OPTION_NII		62

/* DUID */
#define DUID_TYPE_LL		3
#define DUID_HW_TYPE_ENET	1
#define DUID_LL_SIZE		(sizeof(struct dhcp6_option_duid_ll) + ETH_ALEN)
#define DUID_MAX_SIZE		DUID_LL_SIZE /* only supports DUID-LL currently */

/* vendor-class-data to send in vendor clas option */
#define DHCP6_VCI_STRING	"U-Boot"

#define DHCP6_MULTICAST_ADDR	"ff02::1:2"	/* DHCP multicast address */

/* DHCP6 States supported */
enum dhcp6_state {
	DHCP6_INIT,
	DHCP6_SOLICIT,
	DHCP6_REQUEST,
	DHCP6_DONE,
	DHCP6_FAIL,
};

/* DHCP6 Status codes */
enum dhcp6_status {
	DHCP6_SUCCESS = 0,
	DHCP6_UNSPEC_FAIL = 1,
	DHCP6_NO_ADDRS_AVAIL = 2,
	DHCP6_NO_BINDING = 3,
	DHCP6_NOT_ON_LINK = 4,
	DHCP6_USE_MULTICAST = 5,
	DHCP6_NO_PREFIX_AVAIL = 6,
};

/* DHCP6 message header format */
struct dhcp6_hdr {
	unsigned int msg_type : 8;	/* message type */
	unsigned int trans_id : 24;	/* transaction ID */
} __packed;

/* DHCP6 option header format */
struct dhcp6_option_hdr {
	__be16	option_id;	/* option id */
	__be16	option_len;	/* Option length */
	u8	option_data[0];	/* Option data */
} __packed;

/* DHCP6_OPTION_CLIENTID option (DUID-LL) */
struct dhcp6_option_duid_ll {
	__be16	duid_type;
	__be16	hw_type;
	u8	ll_addr[0];
} __packed;

/* DHCP6_OPTION_ELAPSED_TIME option */
struct dhcp6_option_elapsed_time {
	__be16	elapsed_time;
} __packed;

/* DHCP6_OPTION_IA_TA option */
struct dhcp6_option_ia_ta {
	__be32	iaid;
	u8	ia_ta_options[0];
} __packed;

/* DHCP6_OPTION_IA_NA option */
struct dhcp6_option_ia_na {
	__be32	iaid;
	__be32	t1;
	__be32	t2;
	u8	ia_na_options[0];
} __packed;

/* OPTION_ORO option */
struct dhcp6_option_oro  {
	__be16	req_option_code[0];
} __packed;

/* DHCP6_OPTION_CLIENT_ARCH_TYPE option */
struct dhcp6_option_client_arch {
	__be16	arch_type[0];
} __packed;

/* vendor-class-data inside OPTION_VENDOR_CLASS option */
struct vendor_class_data {
	__be16	vendor_class_len;
	u8	opaque_data[0];
} __packed;

/* DHCP6_OPTION_VENDOR_CLASS option */
struct dhcp6_option_vendor_class {
	__be32				enterprise_number;
	struct vendor_class_data	vendor_class_data[0];
} __packed;

/**
 * struct dhcp6_rx_pkt_status - Structure that holds status
 *                              from a received message
 * @client_id_match: Client ID was found and matches DUID sent
 * @server_id_found: Server ID was found in the message
 * @server_uid_ptr: Pointer to received server ID
 * @server_uid_size: Size of received server ID
 * @ia_addr_found: IA addr option was found in received message
 * @ia_addr_ipv6: The IPv6 address received in IA
 * @ia_status_code: Status code received in the IA
 * @status_code: Top-level status code received
 * @preference: Preference code received
 */
struct dhcp6_rx_pkt_status {
	bool			client_id_match;
	bool			server_id_found;
	uchar			*server_uid_ptr;
	u16			server_uid_size;
	bool			ia_addr_found;
	struct in6_addr		ia_addr_ipv6;
	enum dhcp6_status	ia_status_code;
	enum dhcp6_status	status_code;
	u8			preference;
};

/**
 * struct dhcp6_server_uid - Structure that holds the server UID
 *                             received from an ADVERTISE and saved
 *                             given the server selection criteria.
 * @uid_ptr: Dynamically allocated and copied server UID
 * @uid_size: Size of the server UID in uid_ptr (in bytes)
 * @preference: Preference code associated with this server UID
 */
struct dhcp6_server_uid {
	uchar	*uid_ptr;
	u16	uid_size;
	u8	preference;
};

/**
 * struct dhcp6_sm_params - Structure that holds DHCP6
 *                          state machine parameters
 * @curr_state: current DHCP6 state
 * @next_state: next DHCP6 state
 * @dhcp6_start_ms: timestamp DHCP6 start
 * @dhcp6_retry_start_ms: timestamp of current TX message start
 * @dhcp6_retry_ms: timestamp of last retransmission
 * @retry_cnt: retry count
 * @trans_id: transaction ID
 * @ia_id: transmitted IA ID
 * @irt_ms: Initial retransmission time (in ms)
 * @mrt_ms: Maximum retransmission time (in ms)
 * @mrc: Maximum retransmission count
 * @mrd_ms: Maximum retransmission duration (in ms)
 * @rt_ms: retransmission timeout (is ms)
 * @rt_prev_ms: previous retransmission timeout
 * @rx_status: Status from received message
 * @server_uid: Saved Server UID for selected server
 * @duid: pointer to transmitted Client DUID
 */
struct dhcp6_sm_params {
	enum dhcp6_state		curr_state;
	enum dhcp6_state		next_state;
	ulong				dhcp6_start_ms;
	ulong				dhcp6_retry_start_ms;
	ulong				dhcp6_retry_ms;
	u32				retry_cnt;
	u32				trans_id;
	u32				ia_id;
	int				irt_ms;
	int				mrt_ms;
	int				mrc;
	int				mrd_ms;
	int				rt_ms;
	int				rt_prev_ms;
	struct dhcp6_rx_pkt_status	rx_status;
	struct dhcp6_server_uid		server_uid;
	char				duid[DUID_MAX_SIZE];
};

/* Starts a DHCPv6 4-message exchange as a DHCPv6 client. On successful exchange,
 * the DHCPv6 state machine will transition from internal states:
 *	DHCP6_INIT->DHCP6_SOLICIT->DHCP6_REQUEST->DHCP6_DONE
 *
 * Transmitted SOLICIT and REQUEST packets will set/request the minimum required
 * DHCPv6 options to PXE boot.
 *
 * After a successful exchange, the DHCPv6 assigned address will be set in net_ip6
 *
 * Additionally, the following will be set after receiving these options:
 * DHCP6_OPTION_OPT_BOOTFILE_URL (option 59) -> net_server_ip6, net_boot_file_name
 * DHCP6_OPTION_OPT_BOOTFILE_PARAM (option 60) - > pxelinux_configfile
 *
 * Illustration of a 4-message exchange with 2 servers (copied from
 * https://www.rfc-editor.org/rfc/rfc8415):
 *
 *               Server                          Server
 *           (not selected)      Client        (selected)
 *
 *                 v               v               v
 *                 |               |               |
 *                 |     Begins initialization     |
 *                 |               |               |
 *    start of     | _____________/|\_____________ |
 *    4-message    |/ Solicit      | Solicit      \|
 *    exchange     |               |               |
 *             Determines          |          Determines
 *            configuration        |         configuration
 *                 |               |               |
 *                 |\              |  ____________/|
 *                 | \________     | /Advertise    |
 *                 | Advertise\    |/              |
 *                 |           \   |               |
 *                 |      Collects Advertises      |
 *                 |             \ |               |
 *                 |     Selects configuration     |
 *                 |               |               |
 *                 | _____________/|\_____________ |
 *                 |/ Request      |  Request     \|
 *                 |               |               |
 *                 |               |     Commits configuration
 *                 |               |               |
 *    end of       |               | _____________/|
 *    4-message    |               |/ Reply        |
 *    exchange     |               |               |
 *                 |    Initialization complete    |
 *                 |               |               |
 */
void dhcp6_start(void);

#endif /* __DHCP6_H__ */
