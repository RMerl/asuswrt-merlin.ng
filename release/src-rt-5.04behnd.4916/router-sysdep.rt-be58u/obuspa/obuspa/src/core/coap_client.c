/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2017-2021  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file coap_client.c
 *
 * Implements the client portion of Constrained Application Protocol transport for USP
 *
 */

#ifdef ENABLE_COAP  // NOTE: This isn't strictly necessary as this file is not included in the build if CoAP is disabled

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/opensslv.h>

#include "common_defs.h"
#include "usp_coap.h"
#include "usp_api.h"
#include "usp-msg.pb-c.h"
#include "msg_handler.h"
#include "os_utils.h"
#include "dllist.h"
#include "dm_exec.h"
#include "retry_wait.h"
#include "usp_coap.h"
#include "text_utils.h"
#include "nu_ipaddr.h"
#include "iso8601.h"


//------------------------------------------------------------------------
// Macro to calculate the next CoAP message_id
#define NEXT_MESSAGE_ID(mid)  (((mid) + 1) & 0xFFFF)

//------------------------------------------------------------------------
// Structure representing a CoAP client, used to send a USP message to a controller
typedef struct
{
    int cont_instance;           // Instance number of the controller in Device.LocalAgent.Controller.{i}
    int mtp_instance;            // Instance number of the MTP in Device.LocalAgent.Controller.{i}.MTP.{i}
    bool enable_encryption;      // Set if encryption should be enabled for this client
    double_linked_list_t send_queue; // Queue of messages to send on this CoAP connection

    int socket_fd;               // When sending to a controller, this socket sends CoAP BLOCKs and receives CoAP ACKs
    nu_ipaddr_t  peer_addr;      // IP Address of USP controller that socket_fd is sending to
    uint16_t peer_port;          // Port on USP controller that socket_fd is connected to
    SSL *ssl;                    // SSL connection object used for this CoAP client
	BIO *rbio;                   // SSL BIO used to read DTLS packets
	BIO *wbio;                   // SSL BIO used to write DTLS packets

    unsigned message_id;         // Message ID - unique for the current block being sent
    unsigned char token[4];      // Token to identify the request being sent (same for all blocks encapsulating a single USP message)
    char uri_query_option[128];  // URI query string, telling the recipient what to send the response to

    int cur_block;               // Current block number that we're trying to send
    int block_size;              // Size of blocks (in bytes) that we're sending (the receiver may request that we send a smaller block size)
    int bytes_sent;              // Number of bytes successfully sent of the USP record in BLOCK PDUs.

    int ack_timeout_ms;          // Timeout to receiving next ACK in milliseconds. NOTE: Currently code rounds this down to the nearest number of seconds
    time_t ack_timeout_time;     // Absolute time at which we timeout waiting for an ACK
    int retransmission_counter;  // Number of times that we've retried sending the current block

    int reconnect_timeout_ms;    // Timeout to next trying to reconnect
    time_t reconnect_time;       // Time at which we try to connect the socket again. This is used if we're unable to resolve the server IP address
                                 // This variable is only valid if socket_fd==INVALID
    int reconnect_count;         // Count of number of times that we've tried reconnecting. NOTE: This also includes a count of the retransmission counter
    time_t linger_time;          // time at which we close the connection because we have no more USP Records to send

} coap_client_t;


coap_client_t coap_clients[MAX_COAP_CLIENTS];

//------------------------------------------------------------------------------
// Payload to send in CoAP queue
typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    mtp_send_item_t item;   // Information about the content to send.
    char *host;             // Hostname of the controller to send to
    coap_config_t config;   // Port, resource and whether encryption is enabled
    bool coap_reset_session_hint;       // Set if an existing DTLS session with this host should be reset.
                                        // If we know that the USP request came in on a new DTLS session, then it is likely
                                        // that the USP response must be sent back on a new DTLS session also. Wihout this,
                                        // the CoAP retry mechanism will cause the DTLS session to restart, but it is a while
                                        // before the retry is triggered, so this hint speeds up communications
    time_t expiry_time;     // Time at which this USP record should be removed from the queue
} coap_send_item_t;

//------------------------------------------------------------------------------------
// SSL context for CoAP (created for use with DTLS)
SSL_CTX *coap_client_ssl_ctx = NULL;

//------------------------------------------------------------------------------
// Defines for flags used with StartSendingCoapUspRecord()
#define SEND_CURRENT                0            // Opposite of SEND_NEXT. Sends the current queued USP Record
#define SEND_NEXT                   0x00000001   // Drops the current queued USP record, and starts sending the next
#define RETRY_CURRENT               0x00000002   // Retries sending the current queued USP Record

//------------------------------------------------------------------------------
// Defines for flags used with RetryClientSendLater()
#define ZERO_DELAY_FOR_FIRST_RECONNECT 1        // For the first reconnection attempt do not use any delay (because there has already been a delay due to a missing ACK)

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void HandleCoapAck(coap_client_t *cc);
unsigned CalcCoapClientActions(coap_client_t *cc, parsed_pdu_t *pp);
void HandleNoCoapAck(coap_client_t *cc);
void StartSendingCoapUspRecord(coap_client_t *cc, unsigned flags);
int ClientConnectToController(coap_client_t *cc, nu_ipaddr_t *peer_addr, coap_config_t *config);
void StopSendingToController(coap_client_t *cc);
void RetryClientSendLater(coap_client_t *cc, unsigned flags);
int SendCoapRstFromClient(coap_client_t *cc, parsed_pdu_t *pp);
void SendFirstCoapBlock(coap_client_t *cc);
int SendCoapBlock(coap_client_t *cc);
int WriteCoapBlock(coap_client_t *cc, unsigned char *buf, int len);
int CalcCoapInitialTimeout(void);
coap_client_t *FindUnusedCoapClient(void);
coap_client_t *FindCoapClientByInstance(int cont_instance, int mtp_instance);
void CloseCoapClientSocket(coap_client_t *cc);
void FreeCoapSendItem(coap_client_t *cc, coap_send_item_t *csi);
bool IsUspRecordInCoapQueue(coap_client_t *cc, unsigned char *pbuf, int pbuf_len);
int PerformClientDtlsConnect(coap_client_t *cc, struct sockaddr_storage *remote_addr);
void HandleCoapClientConnectionError(coap_client_t *cc);
void RemoveExpiredCoapMessages(coap_client_t *cc);

/*********************************************************************//**
**
** COAP_CLIENT_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_CLIENT_Init(void)
{
    int i;
    coap_client_t *cc;

    // Initialise the CoAP clients array
    memset(coap_clients, 0, sizeof(coap_clients));
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        cc->cont_instance = INVALID;
        cc->socket_fd = INVALID;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_CLIENT_InitStart
**
** Creates the SSL contexts used by this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_CLIENT_InitStart(void)
{
    // Create the DTLS client SSL context with trust store and client cert loaded
    coap_client_ssl_ctx = DEVICE_SECURITY_CreateSSLContext(DTLS_client_method(), SSL_VERIFY_PEER,
                                                           DEVICE_SECURITY_TrustCertVerifyCallback);
    if (coap_client_ssl_ctx == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_CLIENT_Destroy
**
** Frees all memory used by this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void COAP_CLIENT_Destroy(void)
{
    int i;
    coap_client_t *cc;

    // Free all CoAP clients
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if (cc->cont_instance != INVALID)
        {
            COAP_CLIENT_Stop(cc->cont_instance, cc->mtp_instance);
        }
    }

    // Free the OpenSSL context
    if (coap_client_ssl_ctx != NULL)
    {
        SSL_CTX_free(coap_client_ssl_ctx);
    }
}

/*********************************************************************//**
**
** COAP_CLIENT_Start
**
** Starts a CoAP Client to send USP messages to the specified controller
**
** \param   cont_instance -  Instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance -   Instance number of this MTP in Device.LocalAgent.Controller.{i}.MTP.{i}
** \param   endpoint_id - endpoint of controller (used only for debug)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_CLIENT_Start(int cont_instance, int mtp_instance, char *endpoint_id)
{
    coap_client_t *cc;
    int err = USP_ERR_INTERNAL_ERROR;

    COAP_LockMutex();

    // Exit if MTP thread has exited
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return USP_ERR_OK;
    }

    USP_ASSERT(FindCoapClientByInstance(cont_instance, mtp_instance)==NULL);

    // Exit if unable to find a free CoAP client slot
    cc = FindUnusedCoapClient();
    if (cc == NULL)
    {
        USP_LOG_Error("%s: Out of CoAP clients for controller endpoint %s (Device.LocalAgent.Controller.%d.MTP.%d.CoAP)", __FUNCTION__, endpoint_id, cont_instance, mtp_instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    cc->ssl = NULL;
    cc->rbio = NULL;
    cc->wbio = NULL;

    cc->cont_instance = cont_instance;
    cc->mtp_instance = mtp_instance;
    cc->socket_fd = INVALID;
    cc->message_id = rand_r(&mtp_thread_random_seed) & 0xFFFF;
    cc->reconnect_time = INVALID_TIME;
    cc->reconnect_count = 0;
    cc->reconnect_timeout_ms = CalcCoapInitialTimeout();

    cc->linger_time = INVALID_TIME;

    err = USP_ERR_OK;

exit:
    COAP_UnlockMutex();

    // Cause the MTP thread to wakeup from select() so that timeouts get recalculated based on the new state
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_CoapWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** COAP_CLIENT_Stop
**
** Stops the specified CoAP client
** NOTE: It is safe to call this function, if the instance has already been stopped
**
** \param   cont_instance -  Instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance -   Instance number of this MTP in Device.LocalAgent.Controller.{i}.MTP.{i}
**
** \return  None
**
**************************************************************************/
void COAP_CLIENT_Stop(int cont_instance, int mtp_instance)
{
    coap_client_t *cc;
    coap_send_item_t *csi;

    USP_LOG_Info("%s: Stopping CoAP client [controller_instance=%d, mtp_instance=%d]", __FUNCTION__, cont_instance, mtp_instance);

    COAP_LockMutex();

    // Exit if MTP thread has exited
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return;
    }

    // Exit if the Coap controller has already been stopped - nothing more to do
    cc = FindCoapClientByInstance(cont_instance, mtp_instance);
    if (cc == NULL)
    {
        COAP_UnlockMutex();
        return;
    }

    CloseCoapClientSocket(cc);

    // Drain the queue of outstanding messages to send, by successively removing the first item
    csi = (coap_send_item_t *) cc->send_queue.head;
    while (csi != NULL)
    {
        FreeCoapSendItem(cc, csi);
        csi = (coap_send_item_t *) cc->send_queue.head;
    }

    // Put back to init state
    memset(cc, 0, sizeof(coap_client_t));
    cc->cont_instance = INVALID;
    cc->socket_fd = INVALID;

    COAP_UnlockMutex();

    // Cause the MTP thread to wakeup from select() so that timeouts get recalculated based on the new state
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    MTP_EXEC_CoapWakeup();
}

/*********************************************************************//**
**
** COAP_CLIENT_UpdateAllSockSet
**
** Updates the set of all COAP socket fds to read/write from
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void COAP_CLIENT_UpdateAllSockSet(socket_set_t *set)
{
    int i;
    coap_client_t *cc;
    time_t cur_time;
    int timeout;        // timeout in milliseconds

    cur_time = time(NULL);
    #define CALC_TIMEOUT(res, t) res = t - cur_time; if (res < 0) { res = 0; }

    // Add all CoAP client sockets (these receive CoAP ACK packets from the controller)
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if (cc->cont_instance != INVALID)
        {
            if (cc->socket_fd != INVALID)
            {
                // If keeping socket open in case a new USP Record becomes ready to send...
                timeout = MAX_SOCKET_TIMEOUT_SECONDS;
                if (cc->linger_time != INVALID_TIME)
                {
                    CALC_TIMEOUT(timeout, cc->linger_time);
                }
                else if (cc->ack_timeout_time != INVALID_TIME)
                {
                    // Wait until timeout on receiving an ACK on this socket
                    CALC_TIMEOUT(timeout, cc->ack_timeout_time);
                }

                SOCKET_SET_AddSocketToReceiveFrom(cc->socket_fd, timeout*1000, set);
            }
            else
            {
                // We were unable to connect to the controller last time, so wait until timeout, then try again
                if (cc->reconnect_time != INVALID_TIME)
                {
                    CALC_TIMEOUT(timeout, cc->reconnect_time);
                    SOCKET_SET_UpdateTimeout(timeout*1000, set);
                }
            }
        }
    }
}

/*********************************************************************//**
**
** COAP_CLIENT_ProcessAllSocketActivity
**
** Processes the socket for the specified controller
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  Nothing
**
**************************************************************************/
void COAP_CLIENT_ProcessAllSocketActivity(socket_set_t *set)
{
    int i;
    coap_client_t *cc;
    time_t cur_time;

    cur_time = time(NULL);

    // Service all CoAP client sockets (these receive CoAP ACK packets from the controller)
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if (cc->cont_instance != INVALID)
        {
            if (cc->socket_fd != INVALID)
            {
                if (SOCKET_SET_IsReadyToRead(cc->socket_fd, set))
                {
                    // Handle ACK received
                    HandleCoapAck(cc);
                }
                else if ((cc->ack_timeout_time != INVALID_TIME) && (cur_time >= cc->ack_timeout_time))
                {
                    // Handle ACK not received within timeout period
                    HandleNoCoapAck(cc);
                }
                else if ((cc->linger_time != INVALID_TIME) && (cur_time >= cc->linger_time))
                {
                    // Handle closing down socket after linger period
                    USP_PROTOCOL("%s: Closing down CoAP client socket after linger period", __FUNCTION__);
                    StopSendingToController(cc);
                }
            }
            else
            {
                // Retry connecting to controller's CoAP server
                if ((cc->reconnect_time != INVALID_TIME) && (cur_time >= cc->reconnect_time))
                {
                    StartSendingCoapUspRecord(cc, RETRY_CURRENT);
                }
            }
        }
    }
}

/*********************************************************************//**
**
** COAP_CLIENT_QueueBinaryMessage
**
** Function called to queue a USP record to send to the specified controller (over CoAP)
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is passed to this function.
** \param   cont_instance -  Instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance -   Instance number of this MTP in Device.LocalAgent.Controller.{i}.MTP.{i}
** \param   mrt - pointer to structure containing CoAP parameters describing CoAP destination to send to
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_CLIENT_QueueBinaryMessage(mtp_send_item_t *msi, int cont_instance, int mtp_instance, mtp_reply_to_t *mrt, time_t expiry_time)
{
    coap_client_t *cc;
    coap_send_item_t *csi;
    int err = USP_ERR_GENERAL_FAILURE;
    bool is_duplicate;
    USP_ASSERT(msi != NULL);

    COAP_LockMutex();

    // Exit if MTP thread has exited
    // NOTE: This check should be unnecessary, as this function is only called from the MTP thread
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        USP_FREE(msi->pbuf);
        return USP_ERR_OK;
    }

    // Exit if unable to find the controller MTP queue for this message
    cc = FindCoapClientByInstance(cont_instance, mtp_instance);
    if (cc == NULL)
    {
        USP_LOG_Error("%s: FindCoapClientByInstance() failed for controller=%d (mtp=%d)", __FUNCTION__, cont_instance, mtp_instance);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Do not add this message to the queue, if it is already present in the queue
    // This situation could occur if a notify is being retried to be sent, but is already held up in the queue pending sending
    is_duplicate = IsUspRecordInCoapQueue(cc, msi->pbuf, msi->pbuf_len);
    if (is_duplicate)
    {
        USP_FREE(msi->pbuf);
        err = USP_ERR_OK;
        goto exit;
    }

    // Remove any queued messages that have expired (apart from the first message, which mustn't be removed because it is currently being sent out)
    RemoveExpiredCoapMessages(cc);

    // Add the item to the queue
    csi = USP_MALLOC(sizeof(coap_send_item_t));
    csi->item = *msi;  // NOTE: Ownership of the payload buffer passes to the CoAP client
    csi->host = USP_STRDUP(mrt->coap_host);
    csi->config.port = mrt->coap_port;
    csi->config.resource = USP_STRDUP(mrt->coap_resource);
    csi->config.enable_encryption = mrt->coap_encryption;
    csi->coap_reset_session_hint = mrt->coap_reset_session_hint;
    csi->expiry_time = expiry_time;

    DLLIST_LinkToTail(&cc->send_queue, csi);

    // If the queue was empty, then this will be the first item in the queue
    // So send out this item
    if (cc->send_queue.head == (void *)csi)
    {
        StartSendingCoapUspRecord(cc, SEND_CURRENT);
    }

    err = USP_ERR_OK;

exit:
    COAP_UnlockMutex();

    // If successful, cause the MTP thread to wakeup from select().
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    if (err == USP_ERR_OK)
    {
        MTP_EXEC_CoapWakeup();
    }

    return err;
}

/*********************************************************************//**
**
** COAP_CLIENT_AreAllResponsesSent
**
** Determines whether all responses have been sent, and that there are no outstanding incoming messages
**
** \param   None
**
** \return  true if all responses have been sent
**
**************************************************************************/
bool COAP_CLIENT_AreAllResponsesSent(void)
{
    int i;
    coap_client_t *cc;

    // Iterate over all CoAP clients, seeing if there are any messages which are still being sent out and have not been fully acknowledged
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if (cc->cont_instance != INVALID)
        {
            if (cc->send_queue.head != NULL)
            {
                return false;
            }
        }
    }

    // If the code gets here, then all client responses have been sent
    return true;
}

/*********************************************************************//**
**
** HandleCoapAck
**
** Called when an ACK message is received back from a controller
**
** \param   cc - pointer to structure describing coap client to update
**
** \return  Nothing
**
**************************************************************************/
void HandleCoapAck(coap_client_t *cc)
{
    int err;
    int len;
    unsigned char buf[MAX_COAP_PDU_SIZE];
    parsed_pdu_t pp;
    unsigned action_flags;

    // Exit if connection was closed
    len = COAP_ReceivePdu(cc->ssl, cc->rbio, cc->socket_fd, buf, sizeof(buf));
    if (len == -1)
    {
        HandleCoapClientConnectionError(cc);
        return;
    }

    // Exit if there is nothing to read. This could be the case for DTLS connections if performing a renegotiation
    if (len == 0)
    {
        return;
    }

    // Exit if an error occurred whilst parsing the PDU
    memset(&pp, 0, sizeof(pp));
    pp.message_id = INVALID;
    pp.mtp_reply_to.protocol = kMtpProtocol_CoAP;
    action_flags = COAP_ParsePdu(buf, len, &pp);
    if (action_flags != COAP_NO_ERROR)
    {
        goto exit;
    }

    // Determine what actions to take
    action_flags = CalcCoapClientActions(cc, &pp);

exit:
    // Perform the actions set in the action flags

    // Handle sending a RST, then go back to retrying to transmit the first block
    // NOTE: Note any errors that might be reported in an ACK, instead send a RST, because this code is a CoAP client, so it doesn't send ACKs
    if (action_flags & (SEND_RST | INDICATE_ERR_IN_ACK))
    {
        (void)SendCoapRstFromClient(cc, &pp); // Intentionally ignoring the error, since we are going back to retrying to send the first block anyway
        RetryClientSendLater(cc, 0);
        return;
    }

    // Handle going back to retransmitting the first block (if we received a RST instead of an ACK)
    if (action_flags & RESET_STATE)
    {
        RetryClientSendLater(cc, 0);
        return;
    }

    // Handle sending the next block
    if (action_flags & SEND_NEXT_BLOCK)
    {
        cc->cur_block++;
        cc->bytes_sent += cc->block_size;
        cc->message_id = NEXT_MESSAGE_ID(cc->message_id);
        cc->ack_timeout_ms = CalcCoapInitialTimeout();
        cc->retransmission_counter = 0;

        // Change the size of the next blocks being sent out, if the receiver requested it,
        //and the size they requested is less than our current (otherwise ignore the request)
        cc->block_size = MIN(cc->block_size, pp.block_size);

        // Send the next block
        err = SendCoapBlock(cc);
        if (err != USP_ERR_OK)
        {
            // If failed to send next block, then go back to retrying to transmit the first block
            RetryClientSendLater(cc, 0);
        }
        return;
    }

    // Handle sending next message, either because we've successfully sent the current message, or we're skipping sending the current message because it got an error
    if (action_flags & SEND_NEXT_USP_RECORD)
    {
        StartSendingCoapUspRecord(cc, SEND_NEXT);
        return;
    }
}

/*********************************************************************//**
**
** HandleCoapClientConnectionError
**
** \param   cc - pointer to structure describing coap client to update
**
** \return  None
**
**************************************************************************/
void HandleCoapClientConnectionError(coap_client_t *cc)
{
    coap_send_item_t *csi;
    csi = (coap_send_item_t *)cc->send_queue.head;
    if (csi == NULL)
    {
        USP_PROTOCOL("%s: Connection closed gracefully by peer after we finished sending blocks", __FUNCTION__);
        StopSendingToController(cc);
    }
    else
    {
        USP_LOG_Error("%s: Connection closed by peer or error before all blocks have been ack'ed (sent %d bytes).", __FUNCTION__, cc->bytes_sent);
        RetryClientSendLater(cc, 0);
    }
}

/*********************************************************************//**
**
** CalcCoapClientActions
**
** Determines what actions to take after the CoAP client received a PDU
**
** \param   cc - pointer to structure describing coap client to update
** \param   pp - pointer to structure containing the parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned CalcCoapClientActions(coap_client_t *cc, parsed_pdu_t *pp)
{
    coap_send_item_t *csi;
    bool sent_last_block;

    // Exit if we received a RST. Retry sending the message, starting at the first block
    if (pp->pdu_type == kPduType_Reset)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) was a RST (pdu_type=%d). Restarting transmission.", __FUNCTION__, pp->message_id, pp->pdu_type);
        return RESET_STATE;
    }

    // Exit if we received a non-ACK. Send a RST in response.
    if ((pp->pdu_type == kPduType_Confirmable) || (pp->pdu_type == kPduType_NonConfirmable))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) was not an ACK (pdu_type=%d)", __FUNCTION__, pp->message_id, pp->pdu_type);
        return SEND_RST;        // Send RST for unhandled non-confirmable messages (RFC7252 section 4.3, page 23)
    }

    // If the code gets here, then an ACK was received
    USP_ASSERT(pp->pdu_type == kPduType_Acknowledgement);

    // Exit if ACK has unexpected CoAP token
    if ((pp->token_size != sizeof(cc->token)) || (memcmp(pp->token, cc->token, sizeof(cc->token)) != 0))
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has unexpected token.", __FUNCTION__, pp->message_id);
        return SEND_RST;
    }

    // Exit if ACK has unexpected message_id
    // NOTE: This is not an error. It may occur in practice if server sent out more than one ACK, and some got delayed
    if (pp->message_id != cc->message_id)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) is not an ACK for the current message_id=%d. Ignoring.", __FUNCTION__, pp->message_id, cc->message_id);
        return IGNORE_PDU;
    }

    // Exit if the ACK did not contain a successful response
    if (pp->pdu_class != kPduClass_SuccessResponse)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has unexpected response code %d.%02d. Aborting send of this USP Record.", __FUNCTION__, pp->message_id, pp->pdu_class, pp->request_response_code);
        return SEND_NEXT_USP_RECORD;
    }

    // Exit if the response code was not either 'Changed' or 'Continue'
    if ((pp->request_response_code != kPduSuccessRespCode_Changed) && (pp->request_response_code != kPduSuccessRespCode_Continue))
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has unexpected response code %d.%02d", __FUNCTION__, pp->message_id, pp->pdu_class, pp->request_response_code);
        return SEND_RST;
    }

    // Exit if we received a PDU, but there was no send item in the queue
    csi = (coap_send_item_t *)cc->send_queue.head;
    if (csi == NULL)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d), but not expecting any", __FUNCTION__, pp->message_id);
        return IGNORE_PDU;
    }

    // Exit if we got a 'Changed' response
    // NOTE: Changed response never contains a BLOCK1 option
    sent_last_block = (cc->bytes_sent + cc->block_size >= csi->item.pbuf_len) ? true : false;
    if (pp->request_response_code == kPduSuccessRespCode_Changed)
    {
        // Exit if we were not expecting a 'Changed' response, as we haven't sent all of the blocks
        // The USP message has not been sent successfully in this case
        if (sent_last_block == false)
        {
            USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) is a 'Changed' response before we had sent all blocks.", __FUNCTION__, pp->message_id);
            return SEND_RST;
        }

        USP_PROTOCOL("%s: Received CoAP ACK 'Changed' (MID=%d)", __FUNCTION__, pp->message_id);
        USP_PROTOCOL("%s: USP Message sent successfully", __FUNCTION__);
        return SEND_NEXT_USP_RECORD;
    }

    // If the code gets here, then we got a 'Continue' response
    USP_ASSERT(pp->request_response_code == kPduSuccessRespCode_Continue);

    // Exit if we didn't expect a 'Continue' response, as we've just sent out the last block.
    // The USP message has not been sent successfully in this case
    if (sent_last_block)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) got a 'Continue' response but we've sent all blocks", __FUNCTION__, pp->message_id);
        return RESET_STATE;
    }

    // Exit if the ACK did not contain a BLOCK1 option
    // (since we always send with a BLOCK1 option, we expect to receive one back in the 'Continue' ACK, acknowledging the block)
    if ((pp->options_present & BLOCK1_PRESENT)==0)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) does not contain BLOCK1 option", __FUNCTION__, pp->message_id);
        return RESET_STATE;
    }

    // Exit if the block being acknowledged is not the current block
    // NOTE: This should never occur as the message_id and block number are tied together
    if (pp->rxed_block != cc->cur_block)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) is for a different block than current (rxed_block=%d, expected=%d)", __FUNCTION__, pp->message_id, pp->rxed_block, cc->cur_block);
        return RESET_STATE;
    }

    USP_PROTOCOL("%s: Received CoAP ACK 'Continue' (MID=%d)", __FUNCTION__, pp->message_id);
    return SEND_NEXT_BLOCK;
}

/*********************************************************************//**
**
** HandleNoCoapAck
**
** Called if no CoAP ACK message is received within a timeout
**
** \param   cc - pointer to structure describing coap client to update
**
** \return  Nothing
**
**************************************************************************/
void HandleNoCoapAck(coap_client_t *cc)
{
    int err;

    // Exit if we have exhausted the number of retransmission retries for this BLOCK
    // moving on to attempt to send the next USP message
    cc->retransmission_counter++;
    cc->reconnect_count++;
    if (cc->retransmission_counter >= COAP_MAX_RETRANSMIT)
    {
        USP_LOG_Error("%s: USP Message not sent successfully. Dropping this USP Message (retry count reached)", __FUNCTION__);
        StartSendingCoapUspRecord(cc, SEND_NEXT);
        return;
    }

    // If the code gets here, we haven't exhausted the number of retries

    // For encrypted connections, the lack of an ACK probably means that we need to restart the DTLS session
    // (This is required in the case of sending to the same peer as the last USP message, but the peer had silently reset it's CoAP server session in the meantime)
    if (cc->enable_encryption)
    {
        USP_LOG_Error("%s: No ACK received in encrypted session, so restarting DTLS session.", __FUNCTION__);
        RetryClientSendLater(cc, ZERO_DELAY_FOR_FIRST_RECONNECT);
        return;
    }

    // Retry with a longer timeout period for the ACK
    cc->ack_timeout_ms *= 2;
    err = SendCoapBlock(cc);
    if (err != USP_ERR_OK)
    {
        // If an error occurred when trying to send the block, then retry sending the whole USP Record later
        RetryClientSendLater(cc, 0);
    }
}

/*********************************************************************//**
**
** StartSendingCoapUspRecord
**
** Starts sending the next USP Record, if one is queued
** This function is called after successfully sending a USP message or after failing to send one and deciding to drop it
**
** \param   cc - pointer to structure describing coap client to update
** \param   flags - flags controlling what this function does
**
** \return  Nothing
**
**************************************************************************/
void StartSendingCoapUspRecord(coap_client_t *cc, unsigned flags)
{
    int err;
    coap_send_item_t *csi;
    nu_ipaddr_t csi_peer_addr;
    bool prefer_ipv6;

    // Drop the current queued USP Record (if required)
    if (flags & SEND_NEXT)
    {
        FreeCoapSendItem(cc, (coap_send_item_t *) cc->send_queue.head);
    }

    // Clear all timeouts and failure counts
    cc->ack_timeout_time = INVALID_TIME;
    cc->reconnect_time = INVALID_TIME;
    cc->linger_time = INVALID_TIME;

    // Reset the reconnect count, if this is not a connect retry
    if ((flags & RETRY_CURRENT) == 0)
    {
        cc->reconnect_count = 0;
        cc->reconnect_timeout_ms = CalcCoapInitialTimeout();
    }

    // First remove all expired messages from the queue
    RemoveExpiredCoapMessages(cc);

    // Exit if no more USP Records to send, starting a linger timer to keep the socket
    // connected for a while, in case a USP Record becomes ready to send soon
    csi = (coap_send_item_t *)cc->send_queue.head;
    if (csi == NULL)
    {
        cc->linger_time = time(NULL) + COAP_CLIENT_LINGER_PERIOD;
        return;
    }

    // Log the message, if we are not resending it
    if ((flags & RETRY_CURRENT) == 0)
    {
        MSG_HANDLER_LogMessageToSend(&csi->item, kMtpProtocol_CoAP, csi->host, NULL);
    }

    // Attempt to interpret the host as an IP literal address (ie no DNS lookup required)
    // This will always be the case if sending a USP Response, but might not be the case for USP notifications
    err = nu_ipaddr_from_str(csi->host, &csi_peer_addr);

    // If this fails, then assume that host is a DNS hostname
    if (err != USP_ERR_OK)
    {
        // Get the preference for IPv4 or IPv6, if dual stack
        prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

        // Exit if unable to lookup the IP address of the USP controller to send to
        err = tw_ulib_diags_lookup_host(csi->host, AF_UNSPEC, prefer_ipv6, NULL, &csi_peer_addr);
        if (err != USP_ERR_OK)
        {
            RetryClientSendLater(cc, 0);
            return;
        }
    }

    // Close the socket, if the next message needs to send to a different IP address/port or the request was received on a new DTLS session
    if ((memcmp(&csi_peer_addr, &cc->peer_addr, sizeof(csi_peer_addr)) != 0) ||
        (csi->config.port != cc->peer_port) ||
        (csi->config.enable_encryption != cc->enable_encryption) ||
        (csi->coap_reset_session_hint==true))
    {
        StopSendingToController(cc);
    }

    // Calculate the initial timeout in ms. This will be doubled for each retry attempt
    cc->ack_timeout_ms = CalcCoapInitialTimeout();
    cc->retransmission_counter = 0;

    // Connect to the controller (if required)
    if (cc->socket_fd == INVALID)
    {
        err = ClientConnectToController(cc, &csi_peer_addr, &csi->config);
        if (err != USP_ERR_OK)
        {
            return;
        }
    }

    // Send the first block
    SendFirstCoapBlock(cc);
}

/*********************************************************************//**
**
** ClientConnectToController
**
** Function called to connect the client to the controller specified in the first queued message to send
**
** \param   cc - pointer to structure describing controller to send to
** \param   peer_addr - IP address of controller to connect to
** \param   config - pointer to structure containing port and whether to use encryption when contacting the controller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ClientConnectToController(coap_client_t *cc, nu_ipaddr_t *peer_addr, coap_config_t *config)
{
    char buf[NU_IPADDRSTRLEN];
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    sa_family_t family;
    int err;
    int result;

    // Copy the IP address and port that we are going to connect to into the coap client structure
    memcpy(&cc->peer_addr, peer_addr, sizeof(cc->peer_addr));
    cc->peer_port = config->port;
    cc->enable_encryption = config->enable_encryption;

    USP_PROTOCOL("%s: Connecting to %s, port %d (%s)", __FUNCTION__, nu_ipaddr_str(&cc->peer_addr, buf, sizeof(buf)), cc->peer_port, IS_ENCRYPTED_STRING(cc->enable_encryption));

    // Exit if unable to make a socket address structure to contact the CoAP server
    err = nu_ipaddr_to_sockaddr(&cc->peer_addr, cc->peer_port, &saddr, &saddr_len);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to determine which address family to use to contact the CoAP server
    // NOTE: This shouldn't fail if tw_ulib_diags_lookup_host() is correct
    err = nu_ipaddr_get_family(&cc->peer_addr, &family);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to create the socket
    cc->socket_fd = socket(family, SOCK_DGRAM, 0);
    if (cc->socket_fd == -1)
    {
        USP_ERR_ERRNO("socket", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to connect to the USP controller
    // NOTE: If the server is down, then no error is returned here. Instead it is returned when calling recv
    result = connect(cc->socket_fd, (struct sockaddr *) &saddr, saddr_len);
    if (result != 0)
    {
        USP_ERR_ERRNO("connect", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Perform the DTLS connect, if enabled
    if (cc->enable_encryption)
    {
        err = PerformClientDtlsConnect(cc, &saddr);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    // If the code gets here then the socket was successfully connected (either unencrypted or via DTLS)
    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        RetryClientSendLater(cc, 0);
    }

    return err;
}

/*********************************************************************//**
**
** PerformClientDtlsConnect
**
** Function called to perform the DTLS Handshake when sending to a controller
**
** \param   cc - pointer to structure describing controller to send to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PerformClientDtlsConnect(coap_client_t *cc, struct sockaddr_storage *remote_addr)
{
    int err;
    int result;
    struct timeval timeout;

    USP_ASSERT(cc->ssl == NULL);

    // Exit if unable to create a new SSL connection
    cc->ssl = SSL_new(coap_client_ssl_ctx);
    if (cc->ssl == NULL)
    {
        USP_LOG_Error("%s: SSL_new() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to create the read BIO
    cc->rbio = BIO_new_dgram(cc->socket_fd, BIO_CLOSE);
    if (cc->rbio == NULL)
    {
        USP_LOG_Error("%s: Failed to create read BIO", __FUNCTION__);
        SSL_free(cc->ssl);
        cc->ssl = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to create the write BIO
    cc->wbio = BIO_new_dgram(cc->socket_fd, BIO_CLOSE);
    if (cc->wbio == NULL)
    {
        USP_LOG_Error("%s: Failed to create write BIO", __FUNCTION__);
        BIO_free(cc->rbio);
        SSL_free(cc->ssl);
        cc->ssl = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set timeout for SSL_connect()
    timeout.tv_sec = DTLS_READ_TIMEOUT;
    timeout.tv_usec = 0;
    BIO_ctrl(cc->rbio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);
    BIO_ctrl(cc->wbio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);
    BIO_ctrl(cc->rbio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, remote_addr);
    BIO_ctrl(cc->wbio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, remote_addr);

    // Set BIOs to be used for reading and writing
    SSL_set_bio(cc->ssl, cc->rbio, cc->wbio);

    // Set the pointer to the variable in which to point to the certificate chain collected in the verify callback
    // We don't need the certificate chain when we are posting to a controller, only when receiving from a controller (to determine controller trust role)
    SSL_set_app_data(cc->ssl, NULL);

    // Exit if unable to perform the DTLS handshake
    result = SSL_connect(cc->ssl);
    if (result <= 0)
    {
        err = SSL_get_error(cc->ssl, result);
        USP_LOG_ErrorSSL(__FUNCTION__, "SSL_connect() failed", result, err);
        SSL_free(cc->ssl);  // Freeing the SSL object also frees the BIO objects
        cc->ssl = NULL;
        cc->rbio = NULL;
        cc->wbio = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }
    return USP_ERR_OK;
}


/*********************************************************************//**
**
** StopSendingToController
**
** Function called to stop sending to the specified controller
**
** \param   cc - pointer to structure describing controller to stop send to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void StopSendingToController(coap_client_t *cc)
{
    // NOTE: If linger timeout is 0, it is still OK to close the socket after reception of the (first received) final ACK
    // Whilst there may be more final ACKs on their way to our CoAP client, since the server only resends the ACK in response
    // to receiving a block message. In effect, the server has already assumed that the data has been posted successfully.
    CloseCoapClientSocket(cc);

    memset(&cc->peer_addr, 0, sizeof(cc->peer_addr));
    cc->peer_port = INVALID;
    cc->uri_query_option[0] = '\0';

    cc->retransmission_counter = 0;
    cc->ack_timeout_ms = 0;
    memset(cc->token, 0, sizeof(cc->token));
    cc->message_id = 0;
    cc->cur_block = 0;
    cc->block_size = 0;
    cc->bytes_sent = 0;
    cc->ack_timeout_time = INVALID_TIME;
    cc->reconnect_time = INVALID_TIME;
    cc->linger_time = INVALID_TIME;
}

/*********************************************************************//**
**
** RetryClientSendLater
**
** Function called to retry connecting to a CoAP server later
** This function is called if there was an unrecoverable error either connecting to the server or when sending to it
**
** \param   cc - pointer to structure describing controller to send to
** \param   flags - Flags determining how to calculate the timeout
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void RetryClientSendLater(coap_client_t *cc, unsigned flags)
{
    time_t cur_time;
    coap_send_item_t *csi;
    int timeout;

    // Wind back any coap client state to known values
    StopSendingToController(cc);

    // Exit if we've reached the limit of retrying to connect.
    // If so drop the current USP Record that we're trying to send, and move on to the next one
    cc->reconnect_count++;
    if (cc->reconnect_count >= MAX_COAP_RECONNECTS)
    {
        USP_LOG_Error("%s: USP Message not sent successfully. Dropping this USP Message (retry count exceeded)", __FUNCTION__);
        StartSendingCoapUspRecord(cc, SEND_NEXT);
        return;
    }

    // Otherwise, try to connect again later
    csi = (coap_send_item_t *) cc->send_queue.head;
    if (csi != NULL)
    {
        // Timeout is normally a delay, with the exception of the case where we have already delayed due to a missing ACK
        // (in which case the timeout is 0)
        timeout = (cc->reconnect_timeout_ms) / 1000;
        if ((flags & ZERO_DELAY_FOR_FIRST_RECONNECT) && (cc->reconnect_count == 2))  // Using 2 for reconnect count because we've already incremented it by this time
        {
            timeout =0;
        }

        cur_time = time(NULL);
        cc->reconnect_time = cur_time + timeout;
        USP_LOG_Error("%s: Retrying to send to %s over CoAP in %d seconds (Retry_count=%d/%d)", __FUNCTION__, csi->host, timeout, cc->reconnect_count, MAX_COAP_RECONNECTS);

        // Update the timeout to use next time, in the case of trying to connect again
        cc->reconnect_timeout_ms *= 2;
    }
}

/*********************************************************************//**
**
** SendCoapRstFromClient
**
** Sends a CoAP RST from our CoAP client
**
** \param   cc - pointer to structure describing our CoAP client which is sending this RST
** \param   pp - pointer to structure containing parsed input PDU, that this ACK is responding to
**
** \return  USP_ERR_OK if RST sent successfully
**
**************************************************************************/
int SendCoapRstFromClient(coap_client_t *cc, parsed_pdu_t *pp)
{
    unsigned char buf[MAX_COAP_PDU_SIZE];
    int len;
    int err;

    // CoAP clients always allocate message_ids, so we must allocate a message_id
    // We can use our usual message_id counter because our send has been aborted
    cc->message_id = NEXT_MESSAGE_ID(cc->message_id);
    len = COAP_WriteRst(cc->message_id, pp->token, pp->token_size, buf, sizeof(buf));
    USP_ASSERT(len != 0);

    // Exit if unable to send the CoAP RST
    err = COAP_SendPdu(cc->ssl, cc->wbio, cc->socket_fd, buf, len);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SendFirstCoapBlock
**
** Sends the first CoAP Block of a USP record
** NOTE: This function resets the state in the coap_client_t structure back to sending the first block
**       But it does not reset the failed transmission counter
**
** \param   cc - pointer to structure describing controller to send to
**
** \return  None
**
**************************************************************************/
void SendFirstCoapBlock(coap_client_t *cc)
{
    int err;
    unsigned token;
    coap_send_item_t *csi;

    // Generate a random token and initial message_id
    token = rand_r(&mtp_thread_random_seed);
    STORE_4_BYTES(cc->token, token);
    cc->message_id = NEXT_MESSAGE_ID(cc->message_id);
    cc->cur_block = 0;
    cc->block_size = COAP_CLIENT_PAYLOAD_TX_SIZE;
    cc->bytes_sent = 0;

    // Exit if unable to determine a CoAP server that the USP controller can send back responses to
    csi = (coap_send_item_t *) cc->send_queue.head;
    err = CalcUriQueryOption(cc->socket_fd, csi->config.enable_encryption, cc->uri_query_option, sizeof(cc->uri_query_option));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }
    USP_PROTOCOL("%s: Sending CoAP UriQueryOption='%s'", __FUNCTION__, cc->uri_query_option);

    // Send the first block
    err = SendCoapBlock(cc);

exit:
    // If failed to send the first block, then retry again later
    if (err != USP_ERR_OK)
    {
        RetryClientSendLater(cc, 0);
    }
}

/*********************************************************************//**
**
** SendCoapBlock
**
** Sends a CoAP Block using the specified CoAP client
**
** \param   cc - pointer to structure describing controller to send to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendCoapBlock(coap_client_t *cc)
{
    unsigned char buf[MAX_COAP_PDU_SIZE];
    time_t cur_time;
    int len;
    int err;

    // Exit if unable to create the CoAP PDU to send
    len = WriteCoapBlock(cc, buf, sizeof(buf));
    USP_ASSERT(len != 0);

    // Calculate the absolute time to timeout waiting for an ACK for this packet
    cur_time = time(NULL);
    cc->ack_timeout_time = cur_time + (cc->ack_timeout_ms)/1000;

    // Exit if unable to send the CoAP block
    err = COAP_SendPdu(cc->ssl, cc->wbio, cc->socket_fd, buf, len);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WriteCoapBlock
**
** Writes a CoAP PDU containing a block of the message to send
**
** \param   cc - pointer to structure describing controller to send to
** \param   buf - pointer to buffer in which to write the CoAP PDU
** \param   len - length of buffer in which to write the CoAP PDU
**
** \return  Number of bytes written to the CoAP PDU buffer, or 0 if buffer is too small
**
**************************************************************************/
int WriteCoapBlock(coap_client_t *cc, unsigned char *buf, int len)
{
    int err;
    unsigned header = 0;
    unsigned char *p;
    coap_send_item_t *csi;
    int is_more_blocks;
    int bytes_remaining;
    int payload_size;
    unsigned char port_option[2];
    unsigned char content_format_option[1];
    unsigned char block_option[3];
    unsigned char size_option[2];
    pdu_option_t last_option;
    int block_option_len;
    int pdu_size;
    char peer_addr_str[NU_IPADDRSTRLEN];
    str_vector_t uri_path;
    int i;
    int total_uri_path_len;

    // Calculate the port and content format options
    csi = (coap_send_item_t *) cc->send_queue.head;
    STORE_2_BYTES(port_option, csi->config.port);
    STORE_BYTE(content_format_option, kPduContentFormat_OctetStream);

    // Calculate the block option
    bytes_remaining = csi->item.pbuf_len - cc->bytes_sent;
    is_more_blocks = (bytes_remaining <= cc->block_size) ? 0 : 1;
    block_option_len = COAP_CalcBlockOption(block_option, cc->cur_block, is_more_blocks, cc->block_size);

    // Calculate the size option (this option contains the total size of the message)
    STORE_2_BYTES(size_option, csi->item.pbuf_len);

    // Exit if unable to convert the destination address to a string literal
    err = nu_ipaddr_to_str(&cc->peer_addr, peer_addr_str, sizeof(peer_addr_str));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: nu_ipaddr_to_str() failed", __FUNCTION__);
        return 0;
    }

    // Split the resource path into separate components, then percent decode each component and calculate their total size
    total_uri_path_len = 0;
    TEXT_UTILS_SplitString(csi->config.resource, &uri_path, "/");
    for (i=0; i<uri_path.num_entries; i++)
    {
        TEXT_UTILS_PercentDecodeString(uri_path.vector[i]);
        total_uri_path_len += strlen(uri_path.vector[i]);
    }

    // Exit if the buffer is not large enough to contain everything
    payload_size = (bytes_remaining >= cc->block_size) ? cc->block_size : bytes_remaining;
    #define NUM_OPTIONS 6       // Number of options that this function intends to write (not including the URI path options)
    pdu_size = COAP_HEADER_SIZE + sizeof(cc->token) +
               (NUM_OPTIONS + uri_path.num_entries)*MAX_OPTION_HEADER_SIZE +
               strlen(peer_addr_str) + sizeof(port_option) + total_uri_path_len + sizeof(content_format_option) +
               strlen(cc->uri_query_option) + block_option_len + sizeof(size_option) +
               1 + payload_size;  // Plus 1 for PDU_OPTION_END_MARKER
    if (pdu_size > len)
    {
        STR_VECTOR_Destroy(&uri_path);
        USP_LOG_Error("%s: Buffer too small to write CoAP Block PDU (pdu_size=%d, buf_len=%d)", __FUNCTION__, pdu_size, len);
        return 0;
    }

    // Calculate the header bytes
    MODIFY_BITS(31, 30, header, COAP_VERSION);
    MODIFY_BITS(29, 28, header, kPduType_Confirmable);
    MODIFY_BITS(27, 24, header, sizeof(cc->token));
    MODIFY_BITS(23, 21, header, kPduClass_Request);
    MODIFY_BITS(20, 16, header, kPduRequestMethod_Post);
    MODIFY_BITS(15, 0, header, cc->message_id);

    // Write the CoAP header bytes and token into the output buffer
    p = buf;
    WRITE_4_BYTES(p, header);
    memcpy(p, cc->token, sizeof(cc->token));
    p += sizeof(cc->token);

    // Write the options into the output buffer
    // NOTE: Do not change the order of the options, they must be in numeric order
    // NOTE: If options are added, update the NUM_OPTIONS define
    csi = (coap_send_item_t *) cc->send_queue.head;
    last_option = kPduOption_Zero;
    p = COAP_WriteOption(kPduOption_UriHost, (unsigned char *)peer_addr_str, strlen(peer_addr_str), p, &last_option);
    p = COAP_WriteOption(kPduOption_UriPort, port_option, sizeof(port_option), p, &last_option);

    // Write the URI path options
    for (i=0; i<uri_path.num_entries; i++)
    {
        p = COAP_WriteOption(kPduOption_UriPath, (unsigned char *)uri_path.vector[i], strlen(uri_path.vector[i]), p, &last_option);
    }

    p = COAP_WriteOption(kPduOption_ContentFormat, content_format_option, sizeof(content_format_option), p, &last_option);
    p = COAP_WriteOption(kPduOption_UriQuery, (unsigned char *)cc->uri_query_option, strlen(cc->uri_query_option), p, &last_option);
    p = COAP_WriteOption(kPduOption_Block1, block_option, block_option_len, p, &last_option);
    p = COAP_WriteOption(kPduOption_Size1, size_option, sizeof(size_option), p, &last_option);

    // Write the end of options marker into the output buffer
    WRITE_BYTE(p, PDU_OPTION_END_MARKER);

    // Write the payload into the output buffer
    memcpy(p, &csi->item.pbuf[cc->bytes_sent], payload_size);
    p += payload_size;

    // Log a message
    USP_PROTOCOL("%s: Sending CoAP PDU (MID=%d) block=%d%s (%d bytes). RetryCount=%d/%d, Timeout=%d ms", __FUNCTION__, cc->message_id, cc->cur_block, (is_more_blocks == 0) ? " (last)" : "", payload_size, cc->retransmission_counter, COAP_MAX_RETRANSMIT, cc->ack_timeout_ms);
    STR_VECTOR_Destroy(&uri_path);

    // Return the number of bytes written to the output buffer
    return p - buf;
}

/*********************************************************************//**
**
** CalcCoapInitialTimeout
**
** Selects a random initial timeout between ACK_TIMEOUT and ACK_TIMEOUT*ACK_RANDOM_FACTOR
**
** \param   None
**
** \return  initial timeout in milliseconds
**
**************************************************************************/
int CalcCoapInitialTimeout(void)
{
    int ack_random_factor;
    ack_random_factor = rand_r(&mtp_thread_random_seed) % 500;
    return COAP_ACK_TIMEOUT*(1000 + ack_random_factor);
}

/*********************************************************************//**
**
** FindUnusedCoapClient
**
** Finds an unused CoAP client slot
**
** \param   None
**
** \return  pointer to free CoAP client, or NULL if none found
**
**************************************************************************/
coap_client_t *FindUnusedCoapClient(void)
{
    int i;
    coap_client_t *cc;

    // Iterate over all CoAP controllers, trying to find a free slot
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if (cc->cont_instance == INVALID)
        {
            return cc;
        }
    }

    // If the code gets here, then no free CoAP clients were found
    return NULL;
}


/*********************************************************************//**
**
** FindCoapClientByInstance
**
** Finds a coap client by it's instance numbers
**
** \param   cont_instance -  Instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   mtp_instance -   Instance number of this MTP in Device.LocalAgent.Controller.{i}.MTP.{i}
**
** \return  pointer to matching CoAP client, or NULL if none found
**
**************************************************************************/
coap_client_t *FindCoapClientByInstance(int cont_instance, int mtp_instance)
{
    int i;
    coap_client_t *cc;

    // Iterate over all CoAP clients, trying to find a match
    for (i=0; i<MAX_COAP_CLIENTS; i++)
    {
        cc = &coap_clients[i];
        if ((cc->cont_instance == cont_instance) && (cc->mtp_instance == mtp_instance))
        {
            return cc;
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** CloseCoapClientSocket
**
** Closes our CoAP client socket and SSL, BIO objects
**
** \param   cc - Coap Client to close down socket on
**
** \return  None
**
**************************************************************************/
void CloseCoapClientSocket(coap_client_t *cc)
{
    // Exit if no socket to close
    if (cc->socket_fd == INVALID)
    {
        USP_ASSERT(cc->ssl==NULL);
        USP_ASSERT(cc->rbio==NULL);
        USP_ASSERT(cc->wbio==NULL);
        return;
    }

    USP_PROTOCOL("%s: Closing connection", __FUNCTION__);

    // Free the SSL object
    // NOTE: This also frees the BIO object (if one exists) as it is owned by the SSL object
    if (cc->ssl != NULL)
    {
        SSL_shutdown(cc->ssl);

        SSL_free(cc->ssl);
        cc->ssl = NULL;
        cc->rbio = NULL;
        cc->wbio = NULL;
    }

    // Close the socket
    close(cc->socket_fd);

    // Zero out all state associated with the socket
    cc->socket_fd = INVALID;
    memset(&cc->peer_addr, 0, sizeof(cc->peer_addr));
    cc->peer_port = INVALID;
}

/*********************************************************************//**
**
** RemoveExpiredCoapMessages
**
** Removes all expired messages from the queue of messages to send
** NOTE: This mechanism can be used to prevent the queue from filling up needlessly if the controller is offine
**
** \param   cc - pointer to structure describing coap client to update
**
** \return  None
**
**************************************************************************/
void RemoveExpiredCoapMessages(coap_client_t *cc)
{
    time_t cur_time;
    coap_send_item_t *csi;
    coap_send_item_t *next;

    // Exit if queue is empty
    csi = (coap_send_item_t *)cc->send_queue.head;
    if (csi == NULL)
    {
        return;
    }

    // This CoAP client always attempts to send the item at the head of the queue
    // So skip this item because we don't want to be removing an item which is currently being sent out
    csi = (coap_send_item_t *) csi->link.next;

    // Iterate over the rest of the items in the queue, removing any which have expired
    cur_time = time(NULL);
    while (csi != NULL)
    {
        next = (coap_send_item_t *) csi->link.next;
        if (cur_time > csi->expiry_time)
        {
            FreeCoapSendItem(cc, csi);
        }

        csi = next;
    }
}

/*********************************************************************//**
**
** FreeCoapSendItem
**
** Frees the specified CoAP send item in the queue of items to send
**
** \param   cc - pointer to structure describing coap client to update
**
** \return  Nothing
**
**************************************************************************/
void FreeCoapSendItem(coap_client_t *cc, coap_send_item_t *csi)
{
    USP_ASSERT(csi != NULL);

    // Remove and free the specified item in the queue
    USP_FREE(csi->item.pbuf);
    USP_FREE(csi->host);
    USP_FREE(csi->config.resource);
    DLLIST_Unlink(&cc->send_queue, csi);
    USP_FREE(csi);
}

/*********************************************************************//**
**
** IsUspRecordInCoapQueue
**
** Determines whether the specified USP record is already queued, waiting to be sent
** This is used to avoid duplicate records being placed in the queue, which could occur under notification retry conditions
**
** \param   cc - coap client which has USP records queued to send
** \param   pbuf - pointer to buffer containing USP Record to match against
** \param   pbuf_len - length of buffer containing USP Record to match against
**
** \return  true if the message is already queued
**
**************************************************************************/
bool IsUspRecordInCoapQueue(coap_client_t *cc, unsigned char *pbuf, int pbuf_len)
{
    coap_send_item_t *csi;

    // Iterate over USP Records in the CoAP client's queue
    csi = (coap_send_item_t *) cc->send_queue.head;
    while (csi != NULL)
    {
        // Exit if the USP record is already in the queue
        if ((csi->item.pbuf_len == pbuf_len) && (memcmp(csi->item.pbuf, pbuf, pbuf_len)==0))
        {
             return true;
        }

        // Move to next message in the queue
        csi = (coap_send_item_t *) csi->link.next;
    }

    // If the code gets here, then the USP record is not in the queue
    return false;
}





#endif // ENABLE_COAP
