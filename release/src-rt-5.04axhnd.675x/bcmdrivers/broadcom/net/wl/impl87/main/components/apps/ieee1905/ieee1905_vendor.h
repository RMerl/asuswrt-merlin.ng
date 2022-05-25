/*
 * Broadcom IEEE1905 vendor specific message handling include file
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: ieee1905_vendor.h 801161 2021-07-15 10:11:00Z $
 */

#ifndef _IEEE1905_VENDOR_H_
#define _IEEE1905_VENDOR_H_

#include "ieee1905_message.h"

#define I5_VNDR_OUI_LEN  3

#define I5_VNDR_RCV_FLAGS_CALL_CB_PRE_PROCESS   0x01  /* Call the receive vendor callback before
                                                       * processing any of the standard TLVs
                                                       */
#define I5_VNDR_RCV_FLAGS_CALL_CB_POST_PROCESS  0x02  /* Call the receive vendor callback after
                                                       * processing all the standard TLVs
                                                       */

/** @brief Callback function to be called before sending any standard MultiAP messages. User can
 * use it to add any vendor specific TLVs into the message.
 *
 * @param message_type  Message type of the message to be sent
 * @param dst_al_mac    Destination AL MAC address(It may be multicast address also)
 * @param vndr_ctxt     Argument passed while registering the callback
 * @param psmg          Message pointer which needs to be passed in
 *                      i5VendorTlvInsert function
 * @param reserved      This argument in reserved
*/
typedef void (*i5VendorTLVSendCbfn)(const i5_message_types_t message_type,
  const unsigned char *dst_al_mac, void *vndr_ctxt, const void *pmsg, const void *reserved);

/** @brief Callback function to be called after recieving any message to inform about the vendor
 * specific TLVs recieved. Only for the registered vendor specific OUI's will get callback.
 *
 * @param message_type  Message type of the message which is received
 * @param src_al_mac    Source AL MAC address from where the message recieved
 * @param vndr_ctxt     Argument passed while registering the callback
 * @param data          Vendor specific data recieved
 * @param len           Length of the data
*/
typedef void (*i5VendorTLVReceiveCbfn)(const i5_message_types_t message_type,
  const unsigned char *src_al_mac, void *vndr_ctxt, const unsigned char *data,
  const unsigned int len);

/** @brief Register the callback for adding a vendor specific TLV in a CMDU. So, before sending
 * any message, registered callback will be called. Whenever callback is called, the user
 * has to use "i5VendorTlvInsert" function to add the vendor specific data
 * into the CMDU
 *
 * @param message_type  Message type of the message to be sent
 * @param cbfn          Callback function to be registered
 * @param vndr_ctxt     Optional argument to be passed in users callback function when it is called
*/
void i5VendorTLVSendCbsRegister(const i5_message_types_t message_type,
  i5VendorTLVSendCbfn cbfn, const void *vndr_ctxt);

/** @brief Register the callback for recieving a vendor specific data. So, whenever any message is
 * recieved if that message has vendor specific TLVs with a registred OUI, the callback function
 * will get called.
 *
 * @param message_type    Message type of a message from where to read the vendor data
 * @param pre_or_post_cb  Whether to call callback before processing standard TLVs or after. This
 *                        is of type I5_VNDR_RCV_FLAGS_XXX
 * @param oui             Vendor specific OUI(In string format ex: "00:10:18") to be registered
 * @param cbfn            Callback function to be registered
 * @param vndr_ctxt       Optional argument to be passed in users callback function when it is
 *                        called
*/
void i5VendorTLVReceiveCbsRegister(const i5_message_types_t message_type,
  unsigned char pre_or_post_cb, const char *oui, i5VendorTLVReceiveCbfn cbfn,
  const void *vndr_ctxt);

/** @brief To add any vendor specific data to the CMDU(which is pmsg) with particular OUI
 *
 * @param psmg          Message pointer which needs to be passed. Which user gets in a callback
 * @param oui           Vendor specific OUI(In string format ex: "00:10:18") to be added in the TLV
 * @param data          Vendor specific data to be added
 * @param len           Length of the data
*/
int i5VendorTlvInsert(const void *pmsg, const char *oui,
  const unsigned char *data, const unsigned int len);

/** @brief Send any vendor specific message by inserting vendor specific data. If the destination
 * AL MAC address is multicast MAC(01:80:C2:00:00:13), then give proper relay bit(1 for relayed
 * multicast or 0 for neighbor multicast)
 *
 * @param dst_al_mac    Destination AL MAC this can be multicast MAC address also
 * @param relay         Whether to send as relayed multicast message if it is multicast
 * @param oui           Vendor specific OUI(In string format ex: "00:10:18") to be added in the TLV
 * @param data          Vendor specific data to be added
 * @param len           Length of the data
*/
int i5VendorMessageSend(const unsigned char *dst_al_mac, unsigned char relay,
  const char *oui, const unsigned char *data, const unsigned int len);
#endif /* _IEEE1905_VENDOR_H_ */
