/*
 * Broadcom IEEE1905 vendor sepcific message handling definitions
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
 * $Id: ieee1905_vendor.c 801161 2021-07-15 10:11:00Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ieee1905_trace.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_vendor.h"
#include "ieee1905_tlv.h"

#define I5_TRACE_MODULE i5TraceMessage

/* List of callbacks registered for adding vendor TLV. This list is for particular message type */
typedef struct {
  dll_t node;               /* self referencial (next,prev) pointers of type dll_t */
  i5VendorTLVSendCbfn cbfn; /* Callback function to be called */
  const void *vndr_ctxt;    /* Argument to be passed back in callback function */
} i5VendorSendCbsList_t;

/* List of registered entries for adding vendor TLVs. Each message type can have multiple
 * callbacks i mean for each message type multiple vendors can register their callbacks
 */
typedef struct {
  dll_t node;                       /* self referencial (next,prev) pointers of type dll_t */
  i5_message_types_t message_type;  /* Message Type */
  ieee1905_glist_t cbs_list;        /* List of type  i5VendorSendCbsList_t */
} i5VendorRegisteredSendCbs_t;

/* List of callbacks registered for receiving vendor TLV. This list is for particular
 * message type
 */
typedef struct {
  dll_t node;                   /* self referencial (next,prev) pointers of type dll_t */
  unsigned char flags;          /* Callback to be called before processing standard TLVs or after
                                 * processing all the standard TLVs. This is of type
                                 * I5_VNDR_RCV_FLAGS_XXX
                                 */
  unsigned char oui[3];         /* Vendor specific OUI */
  i5VendorTLVReceiveCbfn cbfn;  /* Callback function to be called */
  const void *vndr_ctxt;        /* Argument to be passed back in callback function */
} i5VendorRcvCbsList_t;

/* List of registered entries for recieving vendor specific data for a OUI */
typedef struct {
  dll_t node;                       /* self referencial (next,prev) pointers of type dll_t */
  i5_message_types_t message_type;  /* Message Type */
  unsigned char flags;              /* Flags of type I5_VNDR_RCV_FLAGS_XXX. This is for storing
                                     * the combined values of all the OUIs
                                     */
  ieee1905_glist_t cbs_list;        /* List of type  i5VendorRcvCbsList_t */
} i5VendorRegisteredRcvCbs_t;

#define I5_VNDR_MSG_TYPE_HASH_SIZE                I5_MSG_NUM_OF_MESSAGES
#define I5_VNDR_GET_1905_MSG_TYPE_INDEX(msg_type) ((msg_type))
#define I5_VNDR_GET_MAP_MSG_TYPE_INDEX(msg_type)  (((msg_type) - I5_MSG_MAP_MSG_TYPE_START) + \
                                                    I5_MSG_1905_MSG_TYPE_END + 1)
#define I5_VNDR_MSG_TYPE_HASH(msg_type)           (((msg_type) >= I5_MSG_MAP_MSG_TYPE_START) ?\
                                                    I5_VNDR_GET_MAP_MSG_TYPE_INDEX((msg_type)) :\
                                                    I5_VNDR_GET_1905_MSG_TYPE_INDEX((msg_type)))

#define I5_VNDR_VALIDATE_HASH_MSG_TYPE_IDX(idx) \
    do { \
      if ((idx) >= I5_VNDR_MSG_TYPE_HASH_SIZE) { \
        i5TraceDirPrint("index %d: out of range\n", (idx)); \
        goto end; \
      } \
    } while(0)

/* Vendor sepcific entries */
typedef struct {
  i5VendorRegisteredSendCbs_t *cbs_to_add_vndr_tlvs_lst[I5_VNDR_MSG_TYPE_HASH_SIZE];
  i5VendorRegisteredRcvCbs_t *cbs_to_rcv_vndr_tlvs_lst[I5_VNDR_MSG_TYPE_HASH_SIZE];
} i5VndrEntry_t;

i5VndrEntry_t i5_vndr_entry;

/* To init vendor module */
void i5VendorInit();
/* To deinit vendor module */
void i5VendorDeinit();
/* Called before sending any message and this will be used for calling the registered callbacks */
void i5VendorInformMessageSend(const unsigned char *dst_al_mac, const void *pmsg,
  const i5_message_types_t message_type, const void *reserved);
/* Called after recieving any message and this will be used for calling the registered callbacks */
void i5VendorInformMessageRecieve(void *pmsg, int message_type, unsigned char pre_or_post_cb,
  const unsigned char *src_al_mac);

/* To init vendor module */
void i5VendorInit()
{
  memset(&i5_vndr_entry, 0, sizeof(i5_vndr_entry));
  i5Trace("Number of messages handled=%d\n", I5_VNDR_MSG_TYPE_HASH_SIZE);

  memset(i5_vndr_entry.cbs_to_add_vndr_tlvs_lst, 0,
    (sizeof(i5VendorRegisteredSendCbs_t*) * I5_VNDR_MSG_TYPE_HASH_SIZE));
  memset(i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst, 0,
    (sizeof(i5VendorRegisteredRcvCbs_t*) * I5_VNDR_MSG_TYPE_HASH_SIZE));
}

/* To deinit vendor module */
void i5VendorDeinit()
{
  int i;

  for (i = 0; i < I5_VNDR_MSG_TYPE_HASH_SIZE; i++) {

    if (i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[i] != NULL) {
      free(i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[i]);
      i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[i] = NULL;
    }

    if (i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[i] != NULL) {
      free(i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[i]);
      i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[i] = NULL;
    }
  }
}

/* Add callback to the list for a message type */
static void i5VendorAddSendCbsToList(ieee1905_glist_t *add_to_list,
  i5VendorTLVSendCbfn cbfn, const void *vndr_ctxt)
{
  i5VendorSendCbsList_t *node;

  node = (i5VendorSendCbsList_t*)malloc(sizeof(*node));
  if (!node) {
    i5TraceDirPrint("Malloc Failed\n");
    return;
  }

  memset(node, 0, sizeof(*node));
  node->cbfn = cbfn;
  node->vndr_ctxt = vndr_ctxt;
  ieee1905_glist_append(add_to_list, (dll_t*)node);
  i5Trace("Adding Callback[%p] to add the vendor specific TLV to a particular message\n",
    cbfn);
}

/* Register the callback for adding a vendor specific TLV in a CMDU. So, before sending
 * any message, registered callback will be called. Whenever callback is called, the user
 * has to use "i5VendorTlvInsert" function to add the vendor specific data
 * into the CMDU
 */
void i5VendorTLVSendCbsRegister(const i5_message_types_t message_type,
  i5VendorTLVSendCbfn cbfn, const void *vndr_ctxt)
{
  int idx;
  i5VendorRegisteredSendCbs_t *node = NULL;

  idx = I5_VNDR_MSG_TYPE_HASH(message_type);
  I5_VNDR_VALIDATE_HASH_MSG_TYPE_IDX(idx);

  node = i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[idx];

  /* If already registered, add the callbacks to list */
  if (node != NULL) {
    i5Trace("MessageType[%d] Already registered at idx[%d] for adding vendor specific TLV\n",
      message_type, idx);
    goto add_cb;
  }

  node = (i5VendorRegisteredSendCbs_t*)malloc(sizeof(*node));
  if (!node) {
    i5TraceDirPrint("Malloc Failed\n");
    return;
  }

  memset(node, 0, sizeof(*node));
  node->message_type = message_type;
  ieee1905_glist_init(&node->cbs_list);
  i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[idx] = node;
  i5Trace("Adding MessageType[%d] to list at idx[%d] for adding vendor specific TLV\n",
    message_type, idx);

add_cb:
  /* Add callbacks to list */
  i5VendorAddSendCbsToList(&node->cbs_list, cbfn, vndr_ctxt);

end:
  return;
}

/* Add callback to the list for a message type for receiving vendor data */
static void i5VendorAddRcvCbsToList(ieee1905_glist_t *add_to_list,
  unsigned char pre_or_post_cb, const char *oui, i5VendorTLVReceiveCbfn cbfn,
  const void *vndr_ctxt)
{
  i5VendorRcvCbsList_t *node;

  node = (i5VendorRcvCbsList_t*)malloc(sizeof(*node));
  if (!node) {
    i5TraceDirPrint("Malloc Failed\n");
    return;
  }

  memset(node, 0, sizeof(*node));
  memcpy(node->oui, oui, sizeof(node->oui));
  node->cbfn = cbfn;
  node->vndr_ctxt = vndr_ctxt;
  node->flags |= pre_or_post_cb;
  ieee1905_glist_append(add_to_list, (dll_t*)node);
  i5Trace("Adding %s Callback[%p] for receiving vendor data with OUI[0x%02x 0x%02x 0x%02x] "
    "pre_or_post_cb[0x%x]\n",
    pre_or_post_cb ? "pre" : "post", cbfn, oui[0], oui[1], oui[2], pre_or_post_cb);
}

/* Register the callback for recieving a vendor specific data. So, whenever any message is
 * recieved if that message has vendor specific TLVs with a registred OUI, the callback function
 * will get called.
 */
void i5VendorTLVReceiveCbsRegister(const i5_message_types_t message_type,
  unsigned char pre_or_post_cb, const char *oui, i5VendorTLVReceiveCbfn cbfn,
  const void *vndr_ctxt)
{
  int idx;
  i5VendorRegisteredRcvCbs_t *node = NULL;
  unsigned char binoui[I5_VNDR_OUI_LEN];

  idx = I5_VNDR_MSG_TYPE_HASH(message_type);
  I5_VNDR_VALIDATE_HASH_MSG_TYPE_IDX(idx);

  node = i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[idx];

  /* If already registered, add the callbacks to list */
  if (node != NULL) {
    i5Trace("MessageType[%d] Already registered at idx[%d] for recieving vendor specific TLV\n",
      message_type, idx);
    goto add_cb;
  }

  node = (i5VendorRegisteredRcvCbs_t*)malloc(sizeof(*node));
  if (!node) {
    i5TraceDirPrint("Malloc Failed\n");
    return;
  }

  memset(node, 0, sizeof(*node));
  node->message_type = message_type;
  ieee1905_glist_init(&node->cbs_list);
  i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[idx] = node;
  i5Trace("Adding MessageType[%d] to list at idx[%d] for recieving vendor specific TLV\n",
    message_type, idx);

add_cb:
  node->flags |= pre_or_post_cb;
  /* Add callbacks and OUI to list */
  i5String2OUI(oui, binoui, sizeof(binoui));
  i5VendorAddRcvCbsToList(&node->cbs_list, pre_or_post_cb, (const char*)binoui, cbfn, vndr_ctxt);

end:
  return;
}

/* Called before sending any message and this will be used for calling the registered callbacks */
void i5VendorInformMessageSend(const unsigned char *dst_al_mac, const void *pmsg,
  const i5_message_types_t message_type, const void *reserved)
{
  int idx;
  i5VendorRegisteredSendCbs_t *node = NULL;
  i5VendorSendCbsList_t *cbs;

  idx = I5_VNDR_MSG_TYPE_HASH(message_type);
  I5_VNDR_VALIDATE_HASH_MSG_TYPE_IDX(idx);

  node = i5_vndr_entry.cbs_to_add_vndr_tlvs_lst[idx];

  /* If message_type is found, go thorugh all the callback lists which are registered for a
   * message type
   */
  if (node != NULL && (node->message_type == message_type)) {

    foreach_iglist_item(cbs, i5VendorSendCbsList_t, node->cbs_list) {

      cbs->cbfn(message_type, dst_al_mac, (void*)cbs->vndr_ctxt, pmsg, reserved);
    }
  }

end:
  return;
}

/* To add any vendor specific data to the CMDU(which is pmsg) with particular OUI */
int i5VendorTlvInsert(const void *pmsg, const char *oui,
  const unsigned char *data, const unsigned int len)
{
  unsigned char binoui[I5_VNDR_OUI_LEN];

  i5String2OUI(oui, binoui, sizeof(binoui));

  return i5TlvVendorSpecificTypeInsert((i5_message_type*)pmsg, (unsigned char*)data,
    (unsigned int)len, (const char*)binoui);
}

/* Called after recieving any message and this will be used for calling the registered callbacks */
void i5VendorInformMessageRecieve(void *pmsg, int message_type, unsigned char pre_or_post_cb,
  const unsigned char *src_al_mac)
{
  int idx;
  i5VendorRegisteredRcvCbs_t *node = NULL;
  i5VendorRcvCbsList_t *cbs = NULL;

  idx = I5_VNDR_MSG_TYPE_HASH(message_type);
  I5_VNDR_VALIDATE_HASH_MSG_TYPE_IDX(idx);

  node = i5_vndr_entry.cbs_to_rcv_vndr_tlvs_lst[idx];

  /* If message_type is found, go thorugh all the OUI callback lists which are registered for a
   * message type
   */
  if (node != NULL && (node->message_type == message_type)) {

    /* If this message type doesn't have pre or post callback time set, return */
    if (!(node->flags & pre_or_post_cb)) {
      goto end;
    }

    foreach_iglist_item(cbs, i5VendorRcvCbsList_t, node->cbs_list) {

      unsigned char *tlvs = NULL;
      unsigned int tlv_len = 0;

      /* If this OUI doesn't have pre or post callback time set, check for other OUI */
      if (!(cbs->flags & pre_or_post_cb)) {
        continue;
      }

      if (i5MessageGetVendorSpecificTlvForOUI(pmsg, cbs->oui, &tlvs, &tlv_len)) {

        cbs->cbfn(message_type, src_al_mac, (void*)cbs->vndr_ctxt, tlvs, tlv_len);
      }

      if (tlvs) {
        free(tlvs);
        tlvs = NULL;
      }
    }
  }

end:
  return;
}

/* Send any vendor specific message by inserting vendor specific data */
int i5VendorMessageSend(const unsigned char *dst_al_mac, unsigned char relay,
  const char *oui, const unsigned char *data, const unsigned int len)
{
  ieee1905_vendor_data msg_data;
  i5_dm_device_type *pDevice;
  unsigned char binoui[I5_VNDR_OUI_LEN];

  memcpy(msg_data.neighbor_al_mac, dst_al_mac, MAC_ADDR_LEN);
  msg_data.vendorSpec_msg = (unsigned char*)data;
  msg_data.vendorSpec_len = (unsigned int)len;

  i5String2OUI(oui, binoui, sizeof(binoui));

  /* For multicast message */
  if (memcmp(msg_data.neighbor_al_mac, I5_MULTICAST_MAC, MAC_ADDR_LEN) == 0) {
    return i5MessageVendorSpecificMessageSend(NULL, msg_data.neighbor_al_mac, &msg_data, relay,
      (const char*)binoui);
  }

  /* For unicast message */
  pDevice = i5DmDeviceFind(msg_data.neighbor_al_mac);
  if (pDevice == NULL) {
    i5TraceError("In Device " I5_MAC_FMT " Neighbor["I5_MAC_DELIM_FMT"] not found\n",
      I5_MAC_PRM(i5_config.i5_mac_address), I5_MAC_PRM(msg_data.neighbor_al_mac));
    return IEEE1905_AL_MAC_NOT_FOUND;
  }

  return i5MessageVendorSpecificMessageSend(pDevice->psock, msg_data.neighbor_al_mac, &msg_data,
    relay, (const char*)binoui);
}
