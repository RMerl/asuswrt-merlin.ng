/*
 *
 * Copyright (C) 2019-2021, Broadband Forum
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
 * \file coap_server.c
 *
 * Implements the server portion of Constrained Application Protocol transport for USP
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
#include "usp-record.pb-c.h"

//------------------------------------------------------------------------
// Structure storing the last CoAP response PDU sent. Used to send the same response
typedef struct
{
    int message_id;            // CoAP message id of the last received PDU that was handled.
    unsigned char *pdu_data;   // Response that was sent when the last PDU was handled
    int len;                   // Length (in bytes) of the response in pdu_data[]
} pdu_response_t;

//------------------------------------------------------------------------
// Structure representing a CoAP server session
typedef struct
{
    int socket_fd;          // Socket that we are listening on for USP messages from a controller or INVALID if this slot is not in use
    int index;              // Index of session in sessions[] array of coap_server_t structure. Used only for debug
    SSL *ssl;               // SSL connection object used for this CoAP server
    BIO *rbio;              // SSL BIO used to read DTLS packets
    BIO *wbio;              // SSL BIO used to write DTLS packets

    bool is_first_usp_msg;  // Set if this is the first USP request message received since the server was reset.
                            // This is used as a hint to reset our CoAP client sending the USP response
                            // (because the request was received on a new DTLS session, the response will likely need to be too)

    STACK_OF(X509) *cert_chain; // Full SSL certificate chain for the CoAP connection, collected in the SSL verify callback
    ctrust_role_t role;     // role granted by the CA cert in the chain of trust with the CoAP client

    nu_ipaddr_t peer_addr;   // Current peer that sent the first block. Whilst building up a USP Record, only PDUs from this peer are accepted
    uint16_t peer_port;     // Port that peer is using to communicate with us

    unsigned char token[8]; // Token received in the first block. The server must use the same token for the rest of the blocks.
    int token_size;

    int block_count;        // Count of number of blocks received for the current USP message (ie for current CoAP message token)
    int block_size;         // Size of the blocks being received. The server must use the same size of all of the blocks making up a USP record.
    int last_block_time;    // Time at which the last block was received

    unsigned char *usp_buf; // Pointer to buffer in which the payload is appended, to form the full USP record
    int usp_buf_len;        // Length of the USP record buffer

    pdu_response_t last_response; // Last CoAP PDU response sent. Used to send the same response if we receive the same CoAP request PDU again

} coap_server_session_t;

//------------------------------------------------------------------------
// Structure representing the CoAP servers that USP Agent exports
typedef struct
{
    int instance;           // Instance number of the CoAP server in Device.LocalAgent.MTP.{i}, or INVALID if this slot is unused

    char interface[IFNAMSIZ]; // Name of network interface that this server if listening to ("any" represents all interfaces)
    char listen_addr[NU_IPADDRSTRLEN]; // IP address of network interface that this CoAP server is listening on
    int listen_port;        // Port that this server is listening on
    char *listen_resource;  // Name of our resource that the controller sends to
    bool enable_encryption; // Set if encryption is enabled for this server

    int listen_sock;        // Socket listening for new connections, this socket will get moved to one of the CoAP
                            // sessions when a new packet is received, and a new listening socket will take its place

    coap_server_session_t sessions[MAX_COAP_SERVER_SESSIONS]; // concurrent communication sessions with this server

} coap_server_t;

coap_server_t coap_servers[MAX_COAP_SERVERS];

//------------------------------------------------------------------------------------
// SSL context for CoAP (created for use with DTLS)
SSL_CTX *coap_server_ssl_ctx = NULL;

//------------------------------------------------------------------------------
// Defines to support OpenSSL's change of API signature for SSL_CTX_set_cookie_verify_cb() between different OpenSSL versions
#if OPENSSL_VERSION_NUMBER >= 0x1010000FL // SSL version 1.1.0
    #define SSL_CONST    const
#else
    #define SSL_CONST
#endif

//------------------------------------------------------------------------------
// Buffer containing the random secret that our CoAP server puts into cookies
static unsigned char coap_hmac_key[16];

//------------------------------------------------------------------------------
// Variables associated with determining whether the listening IP address of our CoAP server has changed (used by UpdateCoapServerInterfaces)
static time_t next_coap_server_if_poll_time = 0;   // Absolute time at which to next poll for IP address change

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int StartCoapListenSock(coap_server_t *cs);
void InitCoapSession(coap_server_session_t *css);
coap_server_session_t *FindCoapSession(coap_server_t *cs, nu_ipaddr_t *peer_addr);
void ReceiveCoapBlock(coap_server_t *cs, coap_server_session_t *css);
void StartCoapSession(coap_server_t *cs);
void StopCoapSession(coap_server_session_t *css);
void FreeReceivedUspRecord(coap_server_session_t *css);
unsigned CalcCoapServerActions(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp);
unsigned HandleFirstCoapBlock(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp);
unsigned HandleSubsequentCoapBlock(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp);
unsigned AppendCoapPayload(coap_server_session_t *css, parsed_pdu_t *pp);
int GetPeerAddr(int sock, nu_ipaddr_t *peer_addr, uint16_t *peer_port);
bool IsReplyToValid(coap_server_session_t *css, parsed_pdu_t *pp);
int SendCoapRstFromServer(coap_server_session_t *css, parsed_pdu_t *pp);
int SendCoapAck(coap_server_session_t *css, parsed_pdu_t *pp, unsigned action_flags);
int WriteCoapAck(unsigned char *buf, int len, parsed_pdu_t *pp, unsigned action_flags);
void SaveLastResponsePdu(pdu_response_t *last_resp, int message_id, unsigned char *buf, int len);
coap_server_t *FindUnusedCoapServer(void);
coap_server_t *FindCoapServerByInstance(int instance, char *interface);
coap_server_t *FindFirstCoapServerByInterface(char *interface, bool encryption_preference);
void CalcCoapClassForAck(parsed_pdu_t *pp, unsigned action_flags, int *pdu_class, int *response_code);
void LogRxedCoapPdu(parsed_pdu_t *pp);
int UpdateCoapServerInterfaces(void);
int PerformSessionDtlsConnect(coap_server_session_t *css);
int CalcCoapServerCookie(SSL *ssl, unsigned char *buf, unsigned int *p_len);
int VerifyCoapServerCookie(SSL *ssl, SSL_CONST unsigned char *buf, unsigned int len);

/*********************************************************************//**
**
** COAP_SERVER_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_SERVER_Init(void)
{
    int i;
    coap_server_t *cs;

    // Initialise the CoAP server array
    memset(coap_servers, 0, sizeof(coap_servers));
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        cs->instance = INVALID;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_SERVER_InitStart
**
** Creates the SSL contexts used by this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_SERVER_InitStart(void)
{
    int result;

    // Calculate a random hmac key which will be used by our CoAP server when generating DTLS cookies
    result = RAND_bytes(coap_hmac_key, sizeof(coap_hmac_key));
    if (result != 1)
    {
        USP_LOG_Error("%s: RAND_bytes() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Create the DTLS server SSL context with trust store and client cert loaded
    coap_server_ssl_ctx = DEVICE_SECURITY_CreateSSLContext(DTLS_server_method(),
                                                           SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE /*| SSL_VERIFY_FAIL_IF_NO_PEER_CERT*/,
                                                           DEVICE_SECURITY_TrustCertVerifyCallback);
    if (coap_server_ssl_ctx == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the DTLS cookie functions for the CoAP server (only the server uses these)
	SSL_CTX_set_cookie_generate_cb(coap_server_ssl_ctx, CalcCoapServerCookie);
	SSL_CTX_set_cookie_verify_cb(coap_server_ssl_ctx, VerifyCoapServerCookie);

	SSL_CTX_set_session_cache_mode(coap_server_ssl_ctx, SSL_SESS_CACHE_OFF);


    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_SERVER_Destroy
**
** Frees all memory used by this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void COAP_SERVER_Destroy(void)
{
    int i;
    coap_server_t *cs;

    // Free all CoAP servers
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance != INVALID)
        {
            COAP_SERVER_Stop(cs->instance, cs->interface, NULL);
        }
    }

    // Free the OpenSSL context
    if (coap_server_ssl_ctx != NULL)
    {
        SSL_CTX_free(coap_server_ssl_ctx);
    }
}

/*********************************************************************//**
**
** COAP_SERVER_Start
**
** Starts a CoAP Server on the specified interface and port
**
** \param   instance - instance number in Device.LocalAgent.MTP.{i} for this server
** \param   interface - Name of network interface to listen on ("any" indicates listen on all interfaces)
** \param   config - Configuration for CoAP server: port, resource and whether encryption is enabled
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_SERVER_Start(int instance, char *interface, coap_config_t *config)
{
    int j;
    coap_server_t *cs;
    coap_server_session_t *css;
    int err = USP_ERR_OK;

    COAP_LockMutex();

    // Exit if MTP thread has exited
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return USP_ERR_OK;
    }

    USP_ASSERT(FindCoapServerByInstance(instance, interface)==NULL);

    // Exit if unable to find a free CoAP server slot
    cs = FindUnusedCoapServer();
    if (cs == NULL)
    {
        USP_LOG_Error("%s: Out of CoAP servers when trying to add CoAP server for interface=%s, port %d", __FUNCTION__, interface, config->port);
        err = USP_ERR_RESOURCES_EXCEEDED;
        goto exit;
    }

    // Initialise the coap server structure, marking it as in-use
    memset(cs, 0, sizeof(coap_server_t));
    cs->instance = instance;
    USP_STRNCPY(cs->interface, interface, sizeof(cs->interface));
    cs->listen_port = config->port;
    cs->listen_resource = USP_STRDUP(config->resource);
    cs->enable_encryption = config->enable_encryption;

    // Mark all sockets and CoAP sessions as not in use yet
    cs->listen_sock = INVALID;
    for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
    {
        css = &cs->sessions[j];
        css->socket_fd = INVALID;
        css->index = j;
    }

    USP_LOG_Info("%s: Starting CoAP server on interface=%s, port=%d (%s), resource=%s", __FUNCTION__, interface, cs->listen_port, IS_ENCRYPTED_STRING(cs->enable_encryption), cs->listen_resource);

    // Start the server, ignoring any errors, as UpdateCoapServerInterfaces() will retry later
    (void)StartCoapListenSock(cs);
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
** COAP_SERVER_Stop
**
** Stops all matching CoAP Servers
** NOTE: It is safe to call this function, if the instance has already been stopped
**
** \param   instance - instance number in Device.LocalAgent.MTP.{i} for this server
** \param   interface - Name of network interface to listen on ("any" indicates listen on all interfaces)
** \param   unused - input argumentto make the signature of this function the same as COAP_StartServer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_SERVER_Stop(int instance, char *interface, coap_config_t *unused)
{
    int i;
    coap_server_t *cs;
    coap_server_session_t *css;

    USP_LOG_Info("%s: Stopping CoAP server [%d]", __FUNCTION__, instance);

    (void)unused;   // Prevent compiler warnings about unused variables

    COAP_LockMutex();

    // Exit if MTP thread has exited
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return USP_ERR_OK;
    }

    // Exit if the Coap server has already been stopped - nothing more to do
    cs = FindCoapServerByInstance(instance, interface);
    if (cs == NULL)
    {
        COAP_UnlockMutex();
        return USP_ERR_OK;
    }

    // Free all dynamically allocated buffers
    USP_SAFE_FREE(cs->listen_resource);

    // Close all session sockets and any associated SSL, BIO objects and buffers
    for (i=0; i<MAX_COAP_SERVER_SESSIONS; i++)
    {
        css = &cs->sessions[i];
        StopCoapSession(css);
    }

    // Put back to init state
    memset(cs, 0, sizeof(coap_server_t));
    cs->instance = INVALID;

    COAP_UnlockMutex();

    // Cause the MTP thread to wakeup from select() so that timeouts get recalculated based on the new state
    // We do this outside of the mutex lock to avoid an unnecessary task switch
    MTP_EXEC_CoapWakeup();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_SERVER_GetStatus
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.Status
**
** \param   instance - instance number in Device.LocalAgent.MTP.{i} for this server
**
** \return  Status of this CoAP server
**
**************************************************************************/
mtp_status_t COAP_SERVER_GetStatus(int instance)
{
    coap_server_t *cs;
    mtp_status_t status;

    COAP_LockMutex();

    // Exit if MTP thread has exited
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return kMtpStatus_Down;
    }

    // Exit if we cannot find a CoAP server with this instance - creation of the server had previously failed
    cs = FindCoapServerByInstance(instance, NULL);
    if (cs == NULL)
    {
        status = kMtpStatus_Down;
        goto exit;
    }

    // If creation of the server had previously completed, then this CoAP server is up and running
    status = kMtpStatus_Up;

exit:
    COAP_UnlockMutex();
    return status;
}

/*********************************************************************//**
**
** COAP_SERVER_UpdateAllSockSet
**
** Updates the set of all COAP socket fds to read/write from
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void COAP_SERVER_UpdateAllSockSet(socket_set_t *set)
{
    int i, j;
    coap_server_t *cs;
    coap_server_session_t *css;
    int timeout;        // timeout in milliseconds

    // Determine whether IP address of any of CoAP servers has changed (if time to poll it)
    timeout = UpdateCoapServerInterfaces();
    SOCKET_SET_UpdateTimeout(timeout*SECONDS, set);

    // Iterate over all CoAP servers
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance != INVALID)
        {
            // Add the socket listening for new connections
            if (cs->listen_sock != INVALID)
            {
                SOCKET_SET_AddSocketToReceiveFrom(cs->listen_sock, MAX_SOCKET_TIMEOUT, set);
            }

            // Iterate over all existing sessions on this interface
            for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
            {
                css = &cs->sessions[j];
                if (css->socket_fd != INVALID)
                {
                    SOCKET_SET_AddSocketToReceiveFrom(css->socket_fd, MAX_SOCKET_TIMEOUT, set);
                }
            }
        }
    }
}

/*********************************************************************//**
**
** COAP_SERVER_ProcessAllSocketActivity
**
** Processes the socket for the specified controller
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  Nothing
**
**************************************************************************/
void COAP_SERVER_ProcessAllSocketActivity(socket_set_t *set)
{
    int i, j;
    coap_server_t *cs;
    coap_server_session_t *css;

    // Service all CoAP server sockets (these receive CoAP BLOCK packets from the controller)
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance != INVALID)
        {
            // Service existing connections
            for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
            {
                css = &cs->sessions[j];
                if (css->socket_fd != INVALID)
                {
                    if (SOCKET_SET_IsReadyToRead(css->socket_fd, set))
                    {
                        ReceiveCoapBlock(cs, css);
                    }
                }
            }

            // Accept new connections
            // We do this after servicing sessions, because we don't want the socket to block when it is added to the session, because it's already been serviced here
            if (cs->listen_sock != INVALID)
            {
                if (SOCKET_SET_IsReadyToRead(cs->listen_sock, set))
                {
                    StartCoapSession(cs);
                }
            }
        }
    }
}

/*********************************************************************//**
**
** COAP_SERVER_AreNoOutstandingIncomingMessages
**
** Determines whether there are no outstanding incoming messages
**
** \param   None
**
** \return  true if no outstanding incoming messages
**
**************************************************************************/
bool COAP_SERVER_AreNoOutstandingIncomingMessages(void)
{
    int i, j;
    coap_server_t *cs;
    coap_server_session_t *css;

    // Iterate over all CoAP servers, seeing if any are currently receiving messages
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance != INVALID)
        {
            for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
            {
                css = &cs->sessions[j];
                if (css->usp_buf_len != 0)
                {
                    return false;
                }
            }
        }
    }

    // If the code gets here, then there are no outstanding incoming messages
    return true;
}

/*********************************************************************//**
**
** StartCoapListenSock
**
** Starts a listening socket on the specified CoAP Server
**
** \param   cs - pointer to coap server
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int StartCoapListenSock(coap_server_t *cs)
{
    nu_ipaddr_t nu_intf_addr;
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    sa_family_t family;
    int result;
    int err;
    bool prefer_ipv6;
    int enable = 1;
    int sock = INVALID;

    // Get preference for IPv4 or IPv6 WAN address (in case of Dual Stack CPE)
    prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

    // Exit if unable to get current IP address for specified network interface
    err = tw_ulib_get_dev_ipaddr(cs->interface, cs->listen_addr, sizeof(cs->listen_addr), prefer_ipv6);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: CoAP server's listening interface on %s is down. Retrying later", __FUNCTION__, cs->interface);
        goto exit;
    }

    // Exit if unable to convert the interface address into an nu_ipaddr structure
    err = nu_ipaddr_from_str(cs->listen_addr, &nu_intf_addr);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to convert IP address (%s)", __FUNCTION__, cs->interface);
        goto exit;
    }

    // Exit if unable to make a socket address structure to bind to
    err = nu_ipaddr_to_sockaddr(&nu_intf_addr, cs->listen_port, &saddr, &saddr_len);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to determine which address family to use when creating the listening socket
    err = nu_ipaddr_get_family(&nu_intf_addr, &family);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to create the listening socket
    USP_ASSERT(INVALID == -1);
    sock = socket(family, SOCK_DGRAM, 0);
    if (sock == INVALID)
    {
        USP_ERR_ERRNO("socket", errno);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to reuse the same port
    // This is necessary because new sessions are listened for from any peer, but existing sessions may be connected to specific peer sockets
    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (err != 0)
    {
        USP_ERR_ERRNO("setsockopt", errno);
        err = USP_ERR_INTERNAL_ERROR;
        close(sock);
        goto exit;
    }

    // Exit if unable to bind to the required network interface
    result = bind(sock, (struct sockaddr *) &saddr, saddr_len);
    if (result != 0)
    {
        USP_ERR_ERRNO("bind", errno);
        err = USP_ERR_INTERNAL_ERROR;
        close(sock);
        goto exit;
    }

    // If the code gets here, then the listening socket was started successfully
    cs->listen_sock = sock;
    err = USP_ERR_OK;

exit:
    // If an error occurred, mark the server as not listening on any address, this will cause UpdateCoapServerInterfaces()
    // to periodically try to restart the server, when the interface has an IP address
    if (err != USP_ERR_OK)
    {
        cs->listen_addr[0] = '\0';

        if (sock != INVALID)
        {
            close(sock);
        }
    }

    return err;
}

/*********************************************************************//**
**
** StartCoapSession
**
** Called after receiving a CoAP PDU on the listening socket, to start a new session for it
**
** \param   cs - pointer to coap server
**
** \return  None
**
**************************************************************************/
void StartCoapSession(coap_server_t *cs)
{
    int err;
    coap_server_session_t *css;
    nu_ipaddr_t peer_addr;
    uint16_t peer_port;
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    char buf[NU_IPADDRSTRLEN];

    // Exit if unable to determine the address of the peer sending a CoAP PDU
    err = GetPeerAddr(cs->listen_sock, &peer_addr, &peer_port);
    if (err != USP_ERR_OK)
    {
        // Restart the listening socket, if an error occurred whilst getting the peer address
        // (as this would have been caused by an error on the listening socket)
        close(cs->listen_sock);
        cs->listen_sock = INVALID;
        StartCoapListenSock(cs);     // NOTE: We can ignore any errors, as UpdateCoapServerInterfaces() will retry later
        return;
    }

    // Find a CoAP session slot, possibly killing an existing session if one is not free otherwise
    css = FindCoapSession(cs, &peer_addr);
    USP_ASSERT(css != NULL)

    // Initialise the new session
    InitCoapSession(css);

    // Move the listening socket into this session
    css->socket_fd = cs->listen_sock;
    cs->listen_sock = INVALID;

    // Create a new listening socket to replace the one we moved to the session
    StartCoapListenSock(cs);     // NOTE: We can ignore any errors, as UpdateCoapServerInterfaces() will retry later

    // Convert peer address and port to a sockaddr structure
    err = nu_ipaddr_to_sockaddr(&peer_addr, peer_port, &saddr, &saddr_len);
    USP_ASSERT(err == USP_ERR_OK);

    // Exit if unable to explicitly connect the session socket to the remote peer
    err = connect(css->socket_fd, (struct sockaddr *) &saddr, saddr_len);
    if (err != 0)
    {
        USP_ERR_ERRNO("connect", errno);
        StopCoapSession(css);
        return;
    }

    // Store the peer's IP address and port in the session structure
    USP_PROTOCOL("%s: Accepting %s CoAP session from %s, port %d (using session %d)", __FUNCTION__, IS_ENCRYPTED_STRING(cs->enable_encryption), nu_ipaddr_str(&peer_addr, buf, sizeof(buf)), peer_port, css->index);
    memcpy(&css->peer_addr, &peer_addr, sizeof(peer_addr));
    css->peer_port = peer_port;
    css->role = ROLE_NON_SSL;       // This role will be overridden if the DTLS handshake is performed

    // Perform DTLS handshake
    if (cs->enable_encryption)
    {
        err = PerformSessionDtlsConnect(css);
        if (err != USP_ERR_OK)
        {
            StopCoapSession(css);
        }
    }
}

/*********************************************************************//**
**
** InitCoapSession
**
** Initialises the CoAP session structure
**
** \param   css - pointer to structure describing coap session
**
** \return  None
**
**************************************************************************/
void InitCoapSession(coap_server_session_t *css)
{
    pdu_response_t *last_resp;

    css->socket_fd = INVALID;
    css->ssl = NULL;
    css->rbio = NULL;
    css->wbio = NULL;
    css->is_first_usp_msg = true;
    css->cert_chain = NULL;
    css->role = ROLE_DEFAULT;    // Set default role, if not determined from SSL certs
    memset(&css->peer_addr, 0, sizeof(css->peer_addr));
    css->peer_port = INVALID;
    memset(&css->token, 0, sizeof(css->token));
    css->token_size = 0;
    css->block_count = 0;
    css->block_size = 0;
    css->last_block_time = time(NULL);
    css->usp_buf = NULL;
    css->usp_buf_len = 0;

    last_resp = &css->last_response;
    last_resp->message_id = INVALID;
    last_resp->len = 0;
    last_resp->pdu_data = NULL;
}

/*********************************************************************//**
**
** GetPeerAddr
**
** Gets the address of the peer that sent a packet on the specified socket
**
** \param   sock - socket that has a packet from a peer
** \param   peer_addr - pointer to structure in which to return the IP address of the peer that sent the packet
** \param   peer_port - pointer to variable in which to return the IP port that the packet was sent from
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetPeerAddr(int sock, nu_ipaddr_t *peer_addr, uint16_t *peer_port)
{
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    int len;
    int err;
    unsigned char buf[1];

    // Exit if unable to peek the peer's address
    memset(&saddr, 0, sizeof(saddr));
    saddr_len = sizeof(saddr);
    len = recvfrom(sock, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) &saddr, &saddr_len);
    if (len == -1)
    {
        USP_ERR_ERRNO("recv", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to convert sockaddr structure to IP address and port used by controller that sent us this PDU
    err = nu_ipaddr_from_sockaddr_storage(&saddr, peer_addr, peer_port);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: nu_ipaddr_from_sockaddr_storage() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindCoapSession
**
** Gets a new CoAP session on which to process the new PDU
** NOTE: This function may shutdown an unused session in order to achieve this
**
** \param   cs - pointer to coap server
** \param   peer_addr - IP address of peer that is starting a new session
**
** \return  pointer to coap session
**
**************************************************************************/
coap_server_session_t *FindCoapSession(coap_server_t *cs, nu_ipaddr_t *peer_addr)
{
    int j;
    coap_server_session_t *css;
    time_t cur_time;
    int score;
    int max_score = 0;
    coap_server_session_t *chosen_css = NULL;

    // Exit if there is an existing unused session
    for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
    {
        css = &cs->sessions[j];
        if (css->socket_fd == INVALID)
        {
            return css;
        }
    }

    // Iterate over all existing sessions, choosing the one with the highest score
    cur_time = time(NULL);
    for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
    {
        css = &cs->sessions[j];
        if (css->socket_fd != INVALID)
        {
            // Choose to reuse sessions with longest inactive time
            #define MAX_INACTIVE_TIME 3600          // Maximum amount of time before we consider the session to be completely inactive
            USP_ASSERT(css->last_block_time > (time_t)0);
            score = cur_time - css->last_block_time;
            if (score > MAX_INACTIVE_TIME)
            {
                score = MAX_INACTIVE_TIME;
            }

            // Prioritize reuse of sessions with the same peer
            if (memcmp(peer_addr, &css->peer_addr, sizeof(css->peer_addr))==0)
            {
                score += MAX_INACTIVE_TIME+1;
            }

            if (score >= max_score)  // NOTE: Use >=, so that it always finds at least one match (even if score==0)
            {
                max_score = score;
                chosen_css = css;
            }
        }
    }

    // Stop the existing session
    USP_ASSERT(chosen_css != NULL);
    StopCoapSession(chosen_css);

    return chosen_css;
}

/*********************************************************************//**
**
** PerformSessionDtlsConnect
**
** Function called to perform the DTLS Handshake when receiving from a controller
** This is called only after our CoAP server receives a packet
**
** \param   css - pointer to structure describing coap session
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PerformSessionDtlsConnect(coap_server_session_t *css)
{
    int result;
    int err;
    struct sockaddr_storage saddr;
    struct timeval timeout;

    // Exit if unable to create an SSL object
    css->ssl = SSL_new(coap_server_ssl_ctx);
    if (css->ssl == NULL)
    {
        USP_LOG_Error("%s: SSL_new() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the pointer to the variable in which to point to the certificate chain collected in the verify callback
    SSL_set_app_data(css->ssl, &css->cert_chain);

    // Exit if unable to create a read BIO
    css->rbio = BIO_new_dgram(css->socket_fd, BIO_NOCLOSE);
    if (css->rbio == NULL)
    {
        USP_LOG_Error("%s: Unable to create read BIO", __FUNCTION__);
        SSL_free(css->ssl);
        css->ssl = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to create a write BIO
    css->wbio = BIO_new_dgram(css->socket_fd, BIO_NOCLOSE);
    if (css->wbio == NULL)
    {
        USP_LOG_Error("%s: Unable to create write BIO", __FUNCTION__);
        BIO_free(css->rbio);
        SSL_free(css->ssl);
        css->ssl = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the DTLS bio for reading and writing
    SSL_set_bio(css->ssl, css->rbio, css->wbio);

    // Set timeouts for DTLSv1_listen() and SSL_accept()
    timeout.tv_sec = DTLS_READ_TIMEOUT;
    timeout.tv_usec = 0;
    BIO_ctrl(css->rbio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);
    BIO_ctrl(css->wbio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);
    SSL_set_options(css->ssl, SSL_OP_COOKIE_EXCHANGE);

    // Exit if an error occurred when listening to the server socket
    // DTLSv1_listen() responds to the 'ClientHello' by sending a 'Hello Verify Request' containing a cookie
    // then waits for the peer to sent back the 'Client Hello' with the cookie
    result = DTLSv1_listen(css->ssl, (void *) &saddr);
    if (result < 0)
    {
        err = SSL_get_error(css->ssl, result);
        USP_LOG_ErrorSSL(__FUNCTION__, "DTLSv1_listen() failed. Resetting CoAP session.", result, err);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if no packet was received (Note: This shouldn't happen as we should have only got here if a packet was ready to read)
    if (result == 0)
    {
        USP_LOG_Warning("%s: DTLSv1_listen() returned 0. Resetting CoAP session", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the BIO object to the 'connected' state
    BIO_ctrl(css->rbio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &saddr);
    BIO_ctrl(css->wbio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &saddr);

    // The following is needed for compatibility with libcoap
    // If not set, then the DTLS handshake takes a number of seconds to complete, as our OpenSSL server tries successively smaller MTUs
    // Also the maximum MTU size must be set after DTLSv1_listen(), because DTLSv1_listen() resets it
    SSL_set_mtu(css->ssl, MAX_COAP_PDU_SIZE);

    // Exit if unable to finish the DTLS handshake
    // Sends the 'ServerHello' containing server Certificate, client certificate request, and ending in 'ServerHelloDone'
    // Then waits for SSL Handshake message and finally sends a NewSessionTicket
    // NOTE: This agent must have its own cert (same as STOMP client cert), otherwise SSL_accept complains that there's 'no shared cipher'
    result = SSL_accept(css->ssl);
    if (result < 0)
    {
        err = SSL_get_error(css->ssl, result);
        USP_LOG_ErrorSSL(__FUNCTION__, "SSL_accept() failed. Resetting CoAP session", result, err);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If we have a certificate chain, then determine which role to allow for controllers on this CoAP connection
    if (css->cert_chain != NULL)
    {
        // Exit if unable to determine the role associated with the trusted root cert that signed the peer cert
        err = DEVICE_SECURITY_GetControllerTrust(css->cert_chain, &css->role);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: DEVICE_SECURITY_GetControllerTrust() failed. Resetting CoAP session", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** StopCoapSession
**
** This function tears down a CoAP session
**
** \param   css - pointer to structure describing coap session
**
** \return  None
**
**************************************************************************/
void StopCoapSession(coap_server_session_t *css)
{
    pdu_response_t *last_resp;

    // Free the USP record that has been received, setting state back, so that we can start receiving a new one
    FreeReceivedUspRecord(css);

    // Free the last response PDU
    last_resp = &css->last_response;
    USP_SAFE_FREE(last_resp->pdu_data);
    last_resp->pdu_data = NULL;

    // Exit if no socket to close
    if (css->socket_fd == INVALID)
    {
        USP_ASSERT(css->ssl==NULL);
        USP_ASSERT(css->rbio==NULL);
        USP_ASSERT(css->wbio==NULL);
        return;
    }

    // Free the certificate chain and allowed controllers list
    if (css->cert_chain != NULL)
    {
        sk_X509_pop_free(css->cert_chain, X509_free);
        css->cert_chain = NULL;
    }

    // Free the SSL object, gracefully shutting down the SSL connection
    // NOTE: This also frees the BIO object (if one exists) as it is owned by the SSL object
    if (css->ssl != NULL)
    {
        SSL_shutdown(css->ssl);     // NOTE: If the peer has already closed their socket, then this graceful shutdown will result in ICMP destination unreachable
        SSL_free(css->ssl);
        css->ssl = NULL;
        css->rbio = NULL;
        css->wbio = NULL;
    }

    // Close the socket
    close(css->socket_fd);
    css->socket_fd = INVALID;
}

/*********************************************************************//**
**
** FreeReceivedUspRecord
**
** Frees the USP Record that has been received (or partially received) and sets the block count
** state back to allow reception of a new USP Record
**
** \param   css - pointer to structure describing coap session
**
** \return  action flags determining what actions to take
**
**************************************************************************/
void FreeReceivedUspRecord(coap_server_session_t *css)
{
    // Free any partially received USP Record
    if (css->usp_buf != NULL)
    {
        USP_FREE(css->usp_buf);
        css->usp_buf = NULL;
    }

    css->usp_buf_len = 0;
    css->block_count = 0;
    css->block_size = 0;
}

/*********************************************************************//**
**
** ReceiveCoapBlock
**
** Reads a CoAP PDU containing part of a USP message, sent from a controller
** This is expected to be a BLOCK or a single CoAP POST message
**
** \param   cs - coap server on which to process the received CoAP PDU
** \param   css - pointer to structure describing coap session
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ReceiveCoapBlock(coap_server_t *cs, coap_server_session_t *css)
{
    unsigned char buf[MAX_COAP_PDU_SIZE];
    int len;
    parsed_pdu_t pp;
    unsigned action_flags;
    int err;

    // Exit if the connection has been closed by the peer
    len = COAP_ReceivePdu(css->ssl, css->rbio, css->socket_fd, buf, sizeof(buf));
    if (len == -1)
    {
        if (css->usp_buf_len == 0)
        {
            USP_PROTOCOL("%s: Connection closed gracefully by peer after it finished sending blocks", __FUNCTION__);
        }
        else
        {
            USP_LOG_Error("%s: Connection closed by peer or error. Dropping partially received USP Record (%d bytes)", __FUNCTION__, css->usp_buf_len);
        }
        StopCoapSession(css);
        return;
    }

    // Exit if there is nothing to read.
    // This could be the case for DTLS connections if still in the process of performing DTLS handshake
    if (len == 0)
    {
        return;
    }

    css->last_block_time = time(NULL);

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
    action_flags = CalcCoapServerActions(cs, css, &pp);

exit:
    // Perform the actions set in the action flags

    // Check that code does not set contradictory actions to perform in the action flags
    USP_ASSERT( (action_flags & (SEND_ACK | SEND_RST)) != (SEND_ACK | SEND_RST) );
    USP_ASSERT( ((action_flags & USP_RECORD_COMPLETE) == 0) || ((action_flags & (SEND_RST | INDICATE_ERR_IN_ACK | INDICATE_WELL_KNOWN | RESEND_LAST_RESPONSE | RESET_STATE)) == 0) );

    // Send a CoAP RST if required
    if (action_flags & SEND_RST)
    {
        (void)SendCoapRstFromServer(css, &pp);   // Intentionlly ignoring error, as we will reset the CoAP server anyway
        action_flags |= RESET_STATE;
    }

    // Resend the last CoAP PDU (ACK or RST) if required
    if (action_flags & RESEND_LAST_RESPONSE)
    {
        err = COAP_SendPdu(css->ssl, css->wbio, css->socket_fd, css->last_response.pdu_data, css->last_response.len);
        if (err != USP_ERR_OK)
        {
            action_flags |= RESET_STATE;        // Reset the connection if client disconnected
        }
    }

    // Send a CoAP ACK if required
    if (action_flags & SEND_ACK)
    {
        err = SendCoapAck(css, &pp, action_flags);
        if ((err != USP_ERR_OK) || (action_flags & INDICATE_ERR_IN_ACK))
        {
            USP_LOG_Error("%s: Resetting agent's CoAP server after sending ACK indicating error, or unable to send ACK", __FUNCTION__);
            action_flags |= RESET_STATE;
        }
    }

    // Handle a complete USP record being received
    if (action_flags & USP_RECORD_COMPLETE)
    {
        // Log reception of message
        char time_buf[MAX_ISO8601_LEN];
        char addr_buf[NU_IPADDRSTRLEN];
        iso8601_cur_time(time_buf, sizeof(time_buf));
        nu_ipaddr_to_str(&css->peer_addr, addr_buf, sizeof(addr_buf));
        USP_LOG_Info("Message received at time %s, from host %s over CoAP", time_buf, addr_buf);

        // Post complete USP record to the data model thread (as long as the peer address in the 'reply-to' matches that of the received packet)
        if (IsReplyToValid(css, &pp))
        {
            // Create a copy of the reply-to details, modifying coap_host to be the IP literal peer address to send the response back to
            // (This is necessary as the peer's reply-to may be a hostname which has both IPv4 and IPv6 DNS records. We want to reply back using the same IP version we received on)
            mtp_reply_to_t mtp_reply_to;
            memcpy(&mtp_reply_to, &pp.mtp_reply_to, sizeof(mtp_reply_to));
            mtp_reply_to.coap_host = addr_buf;

            // The USP response message to this request should be sent back on a new DTLS session, if this USP request was received on a new DTLS session
            mtp_reply_to.coap_reset_session_hint = css->is_first_usp_msg & cs->enable_encryption;
            css->is_first_usp_msg = false;

            // Post the USP record for processing
            DM_EXEC_PostUspRecord(css->usp_buf, css->usp_buf_len, css->role, &mtp_reply_to);
            FreeReceivedUspRecord(css);
        }
    }

    // Reset state back to waiting for first block of a USP record, if peer sent a RST, or an error occurred when sending an ACK
    if (action_flags & RESET_STATE)
    {
        FreeReceivedUspRecord(css);
    }
}

/*********************************************************************//**
**
** CalcCoapServerActions
**
** Determines what actions to take after the CoAP server received a PDU
** NOTE: The caller has already checked that the PDU is from the peer sending messages to us
**
** \param   cs - pointer to structure describing coap server to update
** \param   css - pointer to structure describing coap session
** \param   pp - pointer to structure containing the parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned CalcCoapServerActions(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp)
{
    unsigned action_flags;

    // Exit if we've already received this PDU before. Resend the response, because the original response might have gone missing
    // NOTE: This could happen under normal circumstances, so isn't an error
    if (pp->message_id == css->last_response.message_id)
    {
        USP_PROTOCOL("%s: Already received CoAP PDU (MID=%d) (Resending response)", __FUNCTION__, pp->message_id);
        return RESEND_LAST_RESPONSE;
    }

    // Exit if we received a RST
    if (pp->pdu_type == kPduType_Reset)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) was a RST (pdu_type=%d)", __FUNCTION__, pp->message_id, pp->pdu_type);
        return RESET_STATE;
    }

    // Exit if our server received an ACK or a 'Non-Confirmable'
    if ((pp->pdu_type == kPduType_Acknowledgement) || (pp->pdu_type == kPduType_NonConfirmable))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) was not of the expected type (pdu_type=%d)", __FUNCTION__, pp->message_id, pp->pdu_type);
        return SEND_RST;        // Send RST for unhandled non-confirmable messages (RFC7252 section 4.3, page 23)
    }

    // If the code gets here, then a 'Confirmable' PDU was received
    USP_ASSERT(pp->pdu_type == kPduType_Confirmable);

    // Exit if CoAP PDU wasn't of expected class
    if (pp->pdu_class != kPduClass_Request)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) was not of the expected class (pdu_class=%d)", __FUNCTION__, pp->message_id, pp->pdu_class);
        return SEND_RST;
    }

    // Exit if CoAP PDU was the special case of resource discovery using a GET of '.well-known/core'
    if ((pp->request_response_code == kPduRequestMethod_Get) && (strcmp(pp->uri_path, ".well-known/core")==0))
    {
        COAP_SetErrMessage("</%s>;if=\"bbf.usp.a\";rt=\"bbf.usp.endpoint\";title=\"USP Agent\";ct=42", cs->listen_resource);
        return SEND_ACK | INDICATE_WELL_KNOWN;
    }

    // Exit if CoAP PDU wasn't of expected method
    if (pp->request_response_code != kPduRequestMethod_Post)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) was not of the expected method (pdu_method=%d)", __FUNCTION__, pp->message_id, pp->request_response_code);
        return SEND_ACK | INDICATE_BAD_METHOD;
    }

    // Handle the block, updating state and determining what to do at the end of this function
    if (css->block_count == 0)
    {
        action_flags = HandleFirstCoapBlock(cs, css, pp);
    }
    else
    {
        action_flags = HandleSubsequentCoapBlock(cs, css, pp);
    }

    // Change the response to send an ACK (4.00 Bad Request) if the USP Record could not be unpacked
    if (action_flags & USP_RECORD_COMPLETE)
    {
        UspRecord__Record *rec;
        rec = usp_record__record__unpack(pbuf_allocator, css->usp_buf_len, css->usp_buf);
        if (rec == NULL)
        {
            COAP_SetErrMessage("%s: usp_record__session_record__unpack failed. Ignoring USP Message", __FUNCTION__);
            action_flags = SEND_ACK | INDICATE_BAD_REQUEST;
        }
        else
        {
            usp_record__record__free_unpacked(rec, pbuf_allocator);
        }
    }

    return action_flags;
}

/*********************************************************************//**
**
** HandleFirstCoapBlock
**
** Handles the first block received of a USP record
**
** \param   cs - pointer to CoAP server which received the payload we're appending
** \param   css - pointer to structure describing coap session
** \param   pp - pointer to parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned HandleFirstCoapBlock(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp)
{
    unsigned action_flags = SEND_ACK | USP_RECORD_COMPLETE;     // Assume there is no block option, or no more blocks
    unsigned temp_flags;

    // Exit if content format is incorrect for USP
    if ((pp->options_present & CONTENT_FORMAT_PRESENT) && (pp->content_format != kPduContentFormat_OctetStream))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has unexpected content format for USP (content_format=%d)", __FUNCTION__, pp->message_id, pp->content_format);
        return SEND_ACK | INDICATE_BAD_CONTENT;
    }

    // Exit if no URI path was specified, or the path did not match our USP resource
    if ( ((pp->options_present & URI_PATH_PRESENT) == 0) || (strcmp(pp->uri_path, cs->listen_resource) != 0) )
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has incorrect URI path for USP (uri_path=%s)", __FUNCTION__, pp->message_id, pp->uri_path);
        return SEND_ACK | INDICATE_NOT_FOUND;
    }

    // Exit if the URI query option is not present
    if ((pp->options_present & URI_QUERY_PRESENT) == 0)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) does not contain URI query option", __FUNCTION__, pp->message_id);
        return SEND_ACK | INDICATE_BAD_REQUEST;
    }

    // Exit if the total size of the USP record being sent is too large for us to accept
    if ((pp->options_present & SIZE1_PRESENT) && (pp->total_size > MAX_USP_MSG_LEN))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) indicates total USP record size is too large (total_size=%u)", __FUNCTION__, pp->message_id, pp->total_size);
        return SEND_ACK | INDICATE_TOO_LARGE;
    }

    // Copy the token
    // NOTE: Tokens only need to be present if the Block option is present
    memcpy(css->token, pp->token, pp->token_size);
    css->token_size = pp->token_size;

    // Handle the block option, if present
    if (pp->options_present & BLOCK1_PRESENT)
    {
        // Exit if the first block we've received isn't block 0
        if (pp->rxed_block != 0)
        {
            COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has unexpected block number (block=%d)", __FUNCTION__, pp->message_id, pp->rxed_block);
            return SEND_ACK | INDICATE_INCOMPLETE;
        }

        // Exit if no token specified
        if (pp->token_size == 0)
        {
            COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has BLOCK1, but no token", __FUNCTION__, pp->message_id);
            return SEND_RST;
        }

        // Exit if the payload is not the same size as that indicated by the block option (if this is the first of many blocks)
        if ((pp->is_more_blocks == 1) && (pp->payload_len != pp->block_size))
        {
            COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has mismatching payload_len=%d and block_size=%d", __FUNCTION__, pp->message_id, pp->payload_len, pp->block_size);
            return SEND_ACK | INDICATE_INCOMPLETE;
        }

        // Store state for subsequent blocks
        css->block_size = pp->block_size;
        css->block_count = 0;                // Reset the block count, it will be incremented by AppendCoapPayload

        // If there are more blocks, then the USP record is not complete yet
        if (pp->is_more_blocks == 1)
        {
            action_flags &= (~USP_RECORD_COMPLETE);
        }
    }

    // Copy this block to the end of the USP record buffer
    temp_flags = AppendCoapPayload(css, pp);
    if (temp_flags != COAP_NO_ERROR)
    {
        return temp_flags;
    }

    LogRxedCoapPdu(pp);

    return action_flags;
}

/*********************************************************************//**
**
** HandleSubsequentCoapBlock
**
** Handles the second and subsequent blocks received of a USP record
**
** \param   cs - pointer to CoAP server which received the payload we're appending
** \param   css - pointer to structure describing coap session
** \param   pp - pointer to parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned HandleSubsequentCoapBlock(coap_server_t *cs, coap_server_session_t *css, parsed_pdu_t *pp)
{
    unsigned action_flags = SEND_ACK | USP_RECORD_COMPLETE;     // Assume there is no block option, or no more blocks
    unsigned temp_flags;

    // Exit if the token doesn't match that of the first block
    if ((pp->token_size != css->token_size) || (memcmp(pp->token, css->token, css->token_size) != 0))
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has different token from first block. Treating this as a new USP Record.", __FUNCTION__, pp->message_id);
        action_flags = HandleFirstCoapBlock(cs, css, pp);
        return action_flags;
    }

    // Exit if content format is incorrect for USP
    if ((pp->options_present & CONTENT_FORMAT_PRESENT) && (pp->content_format != kPduContentFormat_OctetStream))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has unexpected content format for USP (content_format=%d)", __FUNCTION__, pp->message_id, pp->content_format);
        return SEND_ACK | INDICATE_BAD_CONTENT;
    }

    // Exit if a URI path was specified and did not match our USP resource
    if ( (pp->options_present & URI_PATH_PRESENT) && (strcmp(pp->uri_path, cs->listen_resource) != 0) )
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has incorrect URI path for USP (uri_path=%s)", __FUNCTION__, pp->message_id, pp->uri_path);
        return SEND_ACK | INDICATE_NOT_FOUND;
    }

    // Exit if the URI query option is not present
    if ((pp->options_present & URI_QUERY_PRESENT) == 0)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) does not contain URI query option", __FUNCTION__, pp->message_id);
        return SEND_ACK | INDICATE_BAD_REQUEST;
    }

    // Exit if a block option is not present but was previously
    if ((pp->options_present & BLOCK1_PRESENT) == 0)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) is a subsequent block, but no Block1 option", __FUNCTION__, pp->message_id);
        return SEND_ACK | INDICATE_BAD_REQUEST;
    }

    // Exit if the payload is larger than that indicated by the block option
    if (pp->payload_len > pp->block_size)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has payload_len=%d larger than block_size=%d", __FUNCTION__, pp->message_id, pp->payload_len, pp->block_size);
        return SEND_ACK | INDICATE_BAD_REQUEST;
    }

    // Exit if this is not the last block and the payload is not the same size as that indicated by the block option
    if ((pp->is_more_blocks == 1) && (pp->payload_len != pp->block_size))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has mismatching payload_len=%d and block_size=%d", __FUNCTION__, pp->message_id, pp->payload_len, pp->block_size);
        return SEND_ACK | INDICATE_BAD_REQUEST;
    }

    // Exit if sender is trying to increase the block size that they send to us
    if (pp->block_size > css->block_size)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has dynamically changed larger block size (block_size=%d, previously=%d)", __FUNCTION__, pp->message_id, pp->block_size, css->block_size);
        return SEND_ACK | INDICATE_INCOMPLETE;
    }

    // Deal with the case of the sender trying to decrease the block size that they send to us
    if (pp->block_size != css->block_size)
    {
        // Calculate the new count of number of blocks we've received, based on the new block size
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has dynamically changed block size (block_size=%d, previously=%d)", __FUNCTION__, pp->message_id, pp->block_size, css->block_size);
        css->block_size = pp->block_size;
    }

    // Exit if this block is an earlier block that we've already received
    // NOTE: This could happen in practice, so just acknowledge this block, but do nothing with it
    if (pp->rxed_block < css->block_count)
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) has earlier block number than expected (block=%d, expected=%d)", __FUNCTION__, pp->message_id, pp->rxed_block, css->block_count);
        return SEND_ACK;
    }

    // Exit if the number of this block is later than we're expecting
    // NOTE: This should never happen, because the client should not send the next block until we've acknowledged the current
    if (pp->rxed_block > css->block_count)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has later block number than expected (block=%d, expected=%d)", __FUNCTION__, pp->message_id, pp->rxed_block, css->block_count);
        return SEND_ACK | INDICATE_INCOMPLETE;
    }

    // Copy this block to the end of the USP record buffer
    temp_flags = AppendCoapPayload(css, pp);
    if (temp_flags != COAP_NO_ERROR)
    {
        return temp_flags;
    }
    LogRxedCoapPdu(pp);

    // If there are more blocks, then the USP record is not complete yet
    if (pp->is_more_blocks == 1)
    {
        action_flags &= (~USP_RECORD_COMPLETE);
    }

    return action_flags;
}

/*********************************************************************//**
**
** AppendCoapPayload
**
** Appends the specified payload to the buffer in which we are building up the received USP record
**
** \param   cs - pointer to CoAP session which received the payload we're appending
** \param   pp - pointer to structure in which the parsed CoAP PDU is stored
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned AppendCoapPayload(coap_server_session_t *css, parsed_pdu_t *pp)
{
    int new_len;

    // Exit if the new size is greater than we allow
    new_len = css->usp_buf_len + pp->payload_len;
    if (new_len > MAX_USP_MSG_LEN)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) makes received USP record size too large (total_size=%u)", __FUNCTION__, pp->message_id, new_len);
        return SEND_ACK | INDICATE_TOO_LARGE;
    }

    // Increase the size of the USP record buffer
    css->usp_buf = USP_REALLOC(css->usp_buf, new_len);

    // Append the payload to the end of the USP record buffer
    memcpy(&css->usp_buf[css->usp_buf_len], pp->payload, pp->payload_len);
    css->usp_buf_len = new_len;

    css->block_count++;

    return COAP_NO_ERROR;
}

/*********************************************************************//**
**
** IsReplyToValid
**
** Validates that the host in the URI query Option's 'reply-to' matches the
** IP address of the USP controller that sent the message (containing the 'reply-to')
** This is necessary to prevent an errant USP controller using an Agent to perform a DoS attack
**
** \param   css - pointer to CoAP session which received the payload we're appending
** \param   pp - pointer to parsed CoAP PDU
**
** \return  true if the host in the 'reply-to' is valid
**
**************************************************************************/
bool IsReplyToValid(coap_server_session_t *css, parsed_pdu_t *pp)
{
    int err;
    nu_ipaddr_t reply_addr;
    nu_ipaddr_t interface_addr;
    bool prefer_ipv6;
    char buf[NU_IPADDRSTRLEN];
    char host[MAX_COAP_URI_QUERY];

    // Percent decode the received host name
    USP_STRNCPY(host, pp->mtp_reply_to.coap_host, sizeof(host));
    TEXT_UTILS_PercentDecodeString(host);

    // Attempt to interpret the host as an IP literal address (ie no DNS lookup required)
    err = nu_ipaddr_from_str(host, &reply_addr);

    // If this fails, then assume that host is a DNS hostname
    if (err != USP_ERR_OK)
    {
        // Get the preference for IPv4 or IPv6, if dual stack
        prefer_ipv6 = DEVICE_LOCAL_AGENT_GetDualStackPreference();

        // Determine address of interface that the packet was received on
        // We want to lookup a hostname on the same IPv4 or IPv6 protocol
        // NOTE: We lookup css->peer_addr, rather than use cs->listen_addr directly, because we might be listening on "any"
        // (in which case listen_addr does not contain the IP address of the interface which received the packet)
        err = nu_ipaddr_get_interface_addr_from_dest_addr(&css->peer_addr, &interface_addr);
        if (err != USP_ERR_OK)
        {
            return false;
        }

        // Exit if unable to lookup hostname
        err = tw_ulib_diags_lookup_host(host, AF_UNSPEC, prefer_ipv6, &interface_addr, &reply_addr);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Ignoring USP message. Unable to lookup Host address in URI Query option (%s)", __FUNCTION__, host);
            return false;
        }
    }

    // Exit if the address given in the reply-to does not match the address of the USP controller
    // that sent the message containing the reply-to
    if (memcmp(&reply_addr, &css->peer_addr, sizeof(nu_ipaddr_t)) != 0)
    {
        USP_LOG_Error("%s: Ignoring USP message. Host address in URI Query option (%s) does not match sender (%s)", __FUNCTION__, host, nu_ipaddr_str(&css->peer_addr, buf, sizeof(buf)) );
        return false;
    }

    // If the code gets here, then the host specified in the reply-to matches that expected
    return true;
}

/*********************************************************************//**
**
** SendCoapRstFromServer
**
** Sends a CoAP RST from our CoAP server
**
** \param   css - pointer to structure describing the CoAP session which is sending this RST
** \param   pp - pointer to structure containing parsed input PDU, that this ACK is responding to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendCoapRstFromServer(coap_server_session_t *css, parsed_pdu_t *pp)
{
    unsigned char buf[MAX_COAP_PDU_SIZE];
    int len;
    int err;

    // Exit if unable to create the CoAP PDU to send
    // NOTE: CoAP servers always echo the message_id of the received PDU
    len = COAP_WriteRst(pp->message_id, pp->token, pp->token_size, buf, sizeof(buf));
    USP_ASSERT(len != 0);

    // Exit if unable to send the CoAP RST packet
    err = COAP_SendPdu(css->ssl, css->wbio, css->socket_fd, buf, len);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to send RST", __FUNCTION__);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SendCoapAck
**
** Creates and sends a CoAP ACK message to the specified Coap endpoint
**
** \param   css - pointer to CoAP server session which received the message we are acknowledging
** \param   pp - pointer to structure containing block option to include in ACK
** \param   action_flags - Determines whether an error is returned in the ACK
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendCoapAck(coap_server_session_t *css, parsed_pdu_t *pp, unsigned action_flags)
{
    unsigned char buf[MAX_COAP_PDU_SIZE];
    int len;
    int err;

    // Create an ACK, to respond to the packet
    len = WriteCoapAck(buf, sizeof(buf), pp, action_flags);
    USP_ASSERT(len != 0);

    // Save this response, so that we can send it again, if we receive the same message_id again
    SaveLastResponsePdu(&css->last_response, pp->message_id, buf, len);

    // Exit if unable to send the CoAP ACK packet
    err = COAP_SendPdu(css->ssl, css->wbio, css->socket_fd, buf, len);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to send ACK", __FUNCTION__);
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** WriteCoapAck
**
** Writes a CoAP PDU containing an ACK
**
** \param   buf - pointer to buffer in which to write the CoAP PDU
** \param   len - length of buffer in which to write the CoAP PDU
** \param   pp - pointer to structure containing parsed input PDU, that this ACK is responding to
** \param   action_flags - Determines whether an error is returned in the ACK
**
** \return  Number of bytes written to the CoAP PDU buffer
**
**************************************************************************/
int WriteCoapAck(unsigned char *buf, int len, parsed_pdu_t *pp, unsigned action_flags)
{
    unsigned header = 0;
    unsigned char *p;
    unsigned char option_buf[4];
    int option_len;
    pdu_option_t last_option;
    int pdu_class;
    int response_code;
    int preferred_block_size;

    // Determine the class and response code to put in this PDU
    CalcCoapClassForAck(pp, action_flags, &pdu_class, &response_code);

    // Calculate the header bytes
    // NOTE: We use the parsed message_id, since we need to send the ACK based on the PDU we received rather than that which we expected to receive
    MODIFY_BITS(31, 30, header, COAP_VERSION);
    MODIFY_BITS(29, 28, header, kPduType_Acknowledgement);
    MODIFY_BITS(27, 24, header, pp->token_size);
    MODIFY_BITS(23, 21, header, pdu_class);
    MODIFY_BITS(20, 16, header, response_code);
    MODIFY_BITS(15, 0, header, pp->message_id);

    // Write the CoAP header bytes and token into the output buffer
    // NOTE: We use the parsed token, since we need to send the ACK based on the PDU we received rather than that which we expected to receive
    p = buf;
    WRITE_4_BYTES(p, header);
    memcpy(p, pp->token, pp->token_size);
    p += pp->token_size;
    last_option = kPduOption_Zero;

    // Exit if an error or resource discovery response needs to be sent
    if (action_flags & (INDICATE_ERR_IN_ACK | INDICATE_WELL_KNOWN))
    {
        if (action_flags & INDICATE_WELL_KNOWN)
        {
            option_buf[0] = kPduContentFormat_LinkFormat;
            p = COAP_WriteOption(kPduOption_ContentFormat, option_buf, 1, p, &last_option);
        }

        WRITE_BYTE(p, PDU_OPTION_END_MARKER);

        // Copy the textual reason for failure into the payload of the ACK
        len = strlen(COAP_GetErrMessage());
        memcpy(p, COAP_GetErrMessage(), len);
        p += len;
        goto exit;
    }

    // Add the block option, if it was present in the PDU we're acknowledging, and we want the next block
    // NOTE: Do not put the block option in the ACK unless you want another block to be sent (ie It is not present in the last ACK of a sequence of blocks)
    if ((pp->options_present & BLOCK1_PRESENT) && (pp->is_more_blocks))
    {
        preferred_block_size = MIN(COAP_CLIENT_PAYLOAD_RX_SIZE, pp->block_size);
        option_len = COAP_CalcBlockOption(option_buf, pp->rxed_block, pp->is_more_blocks, preferred_block_size);
        p = COAP_WriteOption(kPduOption_Block1, option_buf, option_len, p, &last_option);
    }

    // Add the size option (to indicate to the sender the maximum size of USP record we accept)
    if (action_flags & INDICATE_TOO_LARGE)
    {
        STORE_4_BYTES(option_buf, MAX_USP_MSG_LEN);
        p = COAP_WriteOption(kPduOption_Size1, option_buf, 4, p, &last_option);
    }

    // NOTE: Not adding an end of options marker, because no payload follows

exit:
    // Log what will be sent
    if (pp->options_present & BLOCK1_PRESENT)
    {
        char *last_block_str = (pp->is_more_blocks == 0) ? " (last)" : "";
        USP_PROTOCOL("%s: Sending CoAP ACK (MID=%d) for block=%d%s. Response code=%d.%02d", __FUNCTION__, pp->message_id, pp->rxed_block, last_block_str, pdu_class, response_code);
    }
    else
    {
        USP_PROTOCOL("%s: Sending CoAP ACK (MID=%d). Response code=%d.%02d", __FUNCTION__, pp->message_id, pdu_class, response_code);
    }

    // Return the number of bytes written to the output buffer
    return p - buf;
}

/*********************************************************************//**
**
** CalcCoapClassForAck
**
** Calculate the class and response code for the CoAP header of the ACK message
**
** \param   pp - pointer to structure containing parsed input PDU, that this ACK is responding to
** \param   action_flags - Determines whether an error is returned in the ACK
** \param   pdu_class - pointer to variable in which to return the class to put in the ACK
** \param   response_code - pointer to variable in which to return the response code to put in the ACK
**
** \return  Nothing
**
**************************************************************************/
void CalcCoapClassForAck(parsed_pdu_t *pp, unsigned action_flags, int *pdu_class, int *response_code)
{
    // Determine class of the ACK message
    if (action_flags & INDICATE_ERR_IN_ACK)
    {
        *pdu_class = kPduClass_ClientErrorResponse;
    }
    else
    {
        *pdu_class = kPduClass_SuccessResponse;
    }

    // Determine response code of the ACK message
    if (action_flags & INDICATE_BAD_REQUEST)
    {
        *response_code = kPduClientErrRespCode_BadRequest;
    }
    else if (action_flags & INDICATE_BAD_OPTION)
    {
        *response_code = kPduClientErrRespCode_BadOption;
    }
    else if (action_flags & INDICATE_NOT_FOUND)
    {
        *response_code = kPduClientErrRespCode_NotFound;
    }
    else if (action_flags & INDICATE_BAD_METHOD)
    {
        *response_code = kPduClientErrRespCode_MethodNotAllowed;
    }
    else if (action_flags & INDICATE_INCOMPLETE)
    {
        *response_code = kPduClientErrRespCode_RequestEntityIncomplete;
    }
    else if (action_flags & INDICATE_TOO_LARGE)
    {
        *response_code = kPduClientErrRespCode_RequestEntityTooLarge;
    }
    else if (action_flags & INDICATE_BAD_CONTENT)
    {
        *response_code = kPduClientErrRespCode_UnsupportedContentFormat;
    }
    else if (action_flags & INDICATE_WELL_KNOWN)
    {
        *response_code = kPduSuccessRespCode_Content;
    }
    else if (action_flags & USP_RECORD_COMPLETE)
    {
        *response_code = kPduSuccessRespCode_Changed;
    }
    else if (pp->options_present & BLOCK1_PRESENT)
    {
        *response_code = kPduSuccessRespCode_Continue;
    }
    else
    {
        // For non USP record packets
        *response_code = kPduSuccessRespCode_Content;
    }
}

/*********************************************************************//**
**
** SaveLastResponsePdu
**
** Saves the response PDU being sent out for the specified message_id
**
** \param   last_resp - pointer to structure containing the last response
** \param   message_id - message_id of the message that this is the response to
** \param   buf - pointer to buffer containing the response PDU
** \param   len - length of buffer containing the response PDU
**
** \return  None
**
**************************************************************************/
void SaveLastResponsePdu(pdu_response_t *last_resp, int message_id, unsigned char *buf, int len)
{
    // Free the last PDU
    USP_SAFE_FREE(last_resp->pdu_data);

    // Before replacing it with this new one
    last_resp->pdu_data = USP_MALLOC(len);
    memcpy(last_resp->pdu_data, buf, len);

    last_resp->message_id = message_id;
    last_resp->len = len;
}

/*********************************************************************//**
**
** FindUnusedCoapServer
**
** Finds an unused CoAP server slot
**
** \param   None
**
** \return  pointer to free CoAP server, or NULL if none found
**
**************************************************************************/
coap_server_t *FindUnusedCoapServer(void)
{
    int i;
    coap_server_t *cs;

    // Iterte over all CoAP servers, trying to find a free slot
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance == INVALID)
        {
            return cs;
        }
    }

    // If the code gets here, then no free CoAP servers were found
    return NULL;
}

/*********************************************************************//**
**
** FindCoapServerByInstance
**
** Finds the coap server entry with the specified instance number (from Device.LocalAgent.MTP.{i})
**
** \param   instance - instance number in Device.LocalAgent.MTP.{i} for this server
** \param   interface - Name of network interface to listen on. NULL indicates just find the first
**
** \return  pointer to matching CoAP server, or NULL if none found
**
**************************************************************************/
coap_server_t *FindCoapServerByInstance(int instance, char *interface)
{
    int i;
    coap_server_t *cs;

    // Iterate over all CoAP servers, trying to find a matching slot
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if (cs->instance == instance)
        {
            if ((interface==NULL) || (strcmp(cs->interface, interface)==0))
            {
                return cs;
            }
        }
    }

    // If the code gets here, then no matching CoAP servers were found
    return NULL;
}

/*********************************************************************//**
**
** FindFirstCoapServerByInterface
**
** Finds the first coap server listening on the specified interface
** NOTE: There may be more than one coap server on the specified interface - just listening on a different port
**       In this case, we return the first one
**
** \param   interface - Name of network interface to listen on. NULL indicates just find the first
** \parm    encryption_preference - set if we are sending to the USP Controller using encryption
**                   (in which case we try to find a server that the USP Controller can reply to, which is also encrypted)
**
** \return  pointer to matching CoAP server, or NULL if none found
**
**************************************************************************/
coap_server_t *FindFirstCoapServerByInterface(char *interface, bool encryption_preference)
{
    int i;
    coap_server_t *cs;
    coap_server_t *first_match = NULL;
    coap_server_t *any_match = NULL;

    // Iterate over all CoAP servers, trying to find a slot that matches both interface and encryption preference
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if ((cs->instance != INVALID) && (strcmp(cs->interface, interface)==0))
        {
            if (cs->enable_encryption==encryption_preference)
            {
                return cs;
            }

            // If encryption preference was not met, but interface was, take note of this CoAP server as a fallback
            if (first_match == NULL)
            {
                first_match = cs;
            }
        }
    }

    // If the code gets here, then no perfectly matching CoAP server was found
    // However there might be a server listening on all network interfaces which matches the encryption preference
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if ((cs->instance != INVALID) && (strcmp(cs->interface, "any")==0))
        {
            if (cs->enable_encryption==encryption_preference)
            {
                return cs;
            }

            // If encryption preference was not met, take note of this CoAP server as a fallback
            if (any_match == NULL)
            {
                any_match = cs;
            }
        }
    }

    // If the code gets here, then there was no server which matched the encryption preference
    // So return the first server that matches just by interface
    if (first_match != NULL)
    {
        return first_match;
    }

    if (any_match != NULL)
    {
        return any_match;
    }

    // If the code gets here, then there is no CoAP server which listens on the interface
    return NULL;
}

/*********************************************************************//**
**
** CalcCoapServerCookie
**
** Called by OpenSSL to generate a cookie for a given peer
** The cookie is generated according to RFC 4347: Cookie = HMAC(Secret, Client-IP, Client-Parameters)
**
** \param   ssl - pointer to SSL object, ultimately specifying the peer
** \param   buf - pointer to buffer in which to return the cookie
** \param   p_len - pointer to variable in which to return the length of the cookie
**
** \return  1 if successful, 0 otherwise
**
**************************************************************************/
int CalcCoapServerCookie(SSL *ssl, unsigned char *buf, unsigned int *p_len)
{
    struct sockaddr_storage peer;
    unsigned char *result;
    int err;

    // Exit if unable to extract peer IP address and port
    memset(&peer, 0, sizeof(peer));
    err = BIO_dgram_get_peer(SSL_get_rbio(ssl), &peer);
    if (err <= 0)
    {
        USP_LOG_Error("%s: BIO_dgram_get_peer() failed", __FUNCTION__);
        return 0;
    }

    // Exit if unable to calculate HMAC of peer address and port using our secret hmac key
    result = HMAC( EVP_sha1(),
                   (const void*) coap_hmac_key, sizeof(coap_hmac_key),
                   (const unsigned char*) &peer, sizeof(peer),
                   buf, p_len);
    if (result == NULL)
    {
        USP_LOG_Error("%s: HMAC() failed", __FUNCTION__);
        return 0;
    }

    return 1;
}

/*********************************************************************//**
**
** VerifyCoapServerCookie
**
** Called by OpenSSL to verify that the cookie being returned by the peer matches the one sent to it
** The cookie is generated according to RFC 4347: Cookie = HMAC(Secret, Client-IP, Client-Parameters)
**
** \param   cc - pointer to structure describing coap client to update
** \param   buf - pointer to buffer containing cookie returned by peer
** \param   len - length of buffer containing cookie returned by peer
**
** \return  1 if cookie is correct, 0 if cookie is incorrect
**
**************************************************************************/
int VerifyCoapServerCookie(SSL *ssl, SSL_CONST unsigned char *buf, unsigned int len)
{
    unsigned char expected[EVP_MAX_MD_SIZE];
    unsigned int expected_len;
    int err;

    // Exit if unable to calculate the cookie that we sent to the peer
    expected_len = sizeof(expected);        // I don't think that this is necessary
    err = CalcCoapServerCookie(ssl, expected, &expected_len);
    if (err != 1)
    {
        USP_LOG_Error("%s: CalcCoapServerCookie() failed", __FUNCTION__);
        return 0;
    }

    // Exit if the received cookie did not match the one sent
    if ((len != expected_len) || (memcmp(buf, expected, len) != 0))
    {
        USP_LOG_Error("%s: Received DTLS cookie did not match that sent", __FUNCTION__);
        return 0;
    }

    // If the code gets here, then the received cookie is correct
    return 1;
}

/*********************************************************************//**
**
** CalcUriQueryOption
**
** Calculates the value of the URI Query option for the specified coap client
**
** \param   socket_fd - connected socket which the CoAP client is using to send to the USP Controller
** \parm    encryption_preference - set if we are sending to the USP Controller using encryption
**                   (in which case we try to find a server that the USP Controller can reply to, which is also encrypted)
** \param   buf - buffer in which to return the URI query option
** \param   len - length of buffer in which to return the URI query option
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcUriQueryOption(int socket_fd, bool encryption_preference, char *buf, int len)
{
    int err;
    char src_addr[NU_IPADDRSTRLEN];
    char interface[IFNAMSIZ];
    coap_server_t *cs;
    char *protocol;
    char resource_name[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to determine the source IP address of the client socket
    USP_ASSERT(socket_fd != INVALID);
    err = nu_ipaddr_get_interface_addr_from_sock_fd(socket_fd, src_addr, sizeof(src_addr));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: nu_ipaddr_get_interface_addr_from_sock_fd() failed", __FUNCTION__);
        return err;
    }

    // Exit if unable to determine the network interface used by the client socket
    err = nu_ipaddr_get_interface_name_from_src_addr(src_addr, interface, sizeof(interface));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: nu_ipaddr_get_interface_name_from_src_addr(%s) failed", __FUNCTION__, src_addr);
        return err;
    }

    // Exit if we don't have any coap servers listening on the interface (that our coap client is sending on)
    cs = FindFirstCoapServerByInterface(interface, encryption_preference);
    if (cs == NULL)
    {
        USP_LOG_Error("%s: No CoAP servers listening on interface=%s", __FUNCTION__, interface);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Percent encode our resource name according to step 6 in section 6.5 (Composing URIs from Options) of RFC7252
    TEXT_UTILS_PercentEncodeString(cs->listen_resource, resource_name, sizeof(resource_name), ".~-_!$&'()*+,;=:@/", USE_UPPERCASE_HEX_DIGITS);


    // Fill in the URI query option. This specifies where the USP controller should send responses to
    // NOTE: We use src_addr instead of cs->listen_addr in the reply-to because our CoAP server might be listening on "any" interface
    protocol = (cs->enable_encryption) ? "coaps" : "coap";
    USP_SNPRINTF(buf, len, "reply-to=%s://%s:%d/%s", protocol, src_addr, cs->listen_port, resource_name);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** LogRxedCoapPdu
**
** Logs the CoAP PDU that has been received
**
** \param   pp - pointer to parsed CoAP PDU
**
** \return  None
**
**************************************************************************/
void LogRxedCoapPdu(parsed_pdu_t *pp)
{
    if (pp->options_present & BLOCK1_PRESENT)
    {
        char *last_block_str = (pp->is_more_blocks == 0) ? " (last)" : "";
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) block=%d%s (%d bytes)", __FUNCTION__, pp->message_id, pp->rxed_block, last_block_str, pp->payload_len);
    }
    else
    {
        USP_PROTOCOL("%s: Received CoAP PDU (MID=%d) (%d bytes)", __FUNCTION__, pp->message_id, pp->payload_len);
    }
}

/*********************************************************************//**
**
** UpdateCoapServerInterfaces
**
** Called to determine whether the IP address used for any of our CoAP servers has changed
** NOTE: This function only checks the IP address periodically
**
** \param   None
**
** \return  Number of seconds remaining until next time to poll the interfaces for IP address change
**
**************************************************************************/
int UpdateCoapServerInterfaces(void)
{
    int i, j;
    coap_server_t *cs;
    coap_server_session_t *css;
    bool has_changed;
    time_t cur_time;
    int timeout;
    static bool is_first_time = true; // The first time this function is called, it just sets up the IP address and next_coap_server_if_poll_time
    bool has_addr = false;

    // Exit if it's not yet time to poll the network interface addresses
    cur_time = time(NULL);
    if (is_first_time == false)
    {
        timeout = next_coap_server_if_poll_time - cur_time;
        if (timeout > 0)
        {
            goto exit;
        }
    }

    // Iterate over all CoAP servers that are attached to a single network interface
    for (i=0; i<MAX_COAP_SERVERS; i++)
    {
        cs = &coap_servers[i];
        if ((cs->instance != INVALID) && (strcmp(cs->interface, "any") != 0))
        {
            has_changed = nu_ipaddr_has_interface_addr_changed(cs->interface, cs->listen_addr, &has_addr);
            if ((has_changed) && (has_addr))
            {
                USP_LOG_Error("%s: Restarting CoAP server on interface=%s after IP address change", __FUNCTION__, cs->interface);
                for (j=0; j<MAX_COAP_SERVER_SESSIONS; j++)
                {
                    css = &cs->sessions[j];
                    StopCoapSession(css);
                }

                // Attempt to restart CoAP listening socket for this server
                close(cs->listen_sock);
                cs->listen_sock = INVALID;
                StartCoapListenSock(cs);     // NOTE: We can ignore any errors, as UpdateCoapServerInterfaces() will retry later
            }
        }
    }

    // Set next time to poll for IP address change
    #define COAP_SERVER_IP_ADDR_POLL_PERIOD 5
    timeout = COAP_SERVER_IP_ADDR_POLL_PERIOD;
    next_coap_server_if_poll_time = cur_time + timeout;
    is_first_time = false;

exit:
    return timeout;
}


#endif // ENABLE_COAP
