/*
 * Broadcom IEEE1905 MultiAP-R3 Messages Include file
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: ieee1905_message_r3.h 818746 2022-11-22 05:56:42Z $
 */

#ifndef _IEEE1905_MESSAGE_R3_H_
#define _IEEE1905_MESSAGE_R3_H_

#if defined(MULTIAP)

#include "ieee1905_message.h"

#define I5_DPP_PUB_AF_AUTHENTICATION_REQ        0
#define I5_DPP_PUB_AF_AUTHENTICATION_RESP       1
#define I5_DPP_PUB_AF_AUTHENTICATION_CONFIRM    2
#define I5_DPP_PUB_AF_PEER_DISCOVERY_REQ        5
#define I5_DPP_PUB_AF_PEER_DISCOVERY_RESP       6
#define I5_DPP_PUB_AF_CONFIGURATION_RESULT      11
#define I5_DPP_PUB_AF_CONNECTION_STATUS_RESULT  12
#define I5_DPP_PUB_AF_PRESENCE_ANNOUNCEMENT     13
#define I5_DPP_PUB_AF_RECONFIG_ANNOUNCEMENT     14u
#define I5_DPP_PUB_AF_RECONFIG_AUTH_REQ         15u
#define I5_DPP_PUB_AF_RECONFIG_AUTH_RESP        16u
#define I5_DPP_PUB_AF_RECONFIG_AUTH_CONFIRM     17u
#define I5_DPP_PUB_AF_GAS_FRAME                 255

/* Send DPP CCE Indication message to a Multi AP Device */
void i5MessageDPPCCEIndicationSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, uint8 enable);
/* Send Proxied Encap DPP message */
void i5MessageProxiedEncapDPPSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_1905_encap_dpp_t *dpp_1905_encap,
  i5_dpp_chirp_value_t *dpp_chirp);
/* Send Chirp Notification message */
void i5MessageChirpNotificationSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_dpp_chirp_value_t *dpp_chirp);
/* Send 1905 Encap EAPOL message */
void i5Message1905EncapEAPOLSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_1905_encap_eapol_t *encap_1905_eapol);
/* Send Direct Encap DPP message */
void i5MessageDirectEncapDPPSend(i5_socket_type *psock,
  unsigned char *neighbor_al_mac_address, i5_direct_encap_dpp_t *dpp_direct_encap);
/* Send Agent List message */
void i5MessageAgentListSend(i5_dm_device_type *pdevice);
/* Send Reconfiguration Trigger Message */
void i5MessageReconfigurationTriggerSend(i5_dm_device_type *pDevice);
/* Send DPP Bootstrapping URI Notification Message */
void i5MessageDPPBootstrappingURINotificationSend(i5_socket_type *psock,
  unsigned char *dst_mac, i5_dpp_bootstrap_uri_notification_t *uri_notification);
/* Receive DPP Bootstrapping URI Notification Message */
void i5MessageDPPBootstrappingURINotificationReceive(i5_message_type *pmsg);
#endif /* MULTIAP */
#endif /* _IEEE1905_MESSAGE_R3_H_ */
