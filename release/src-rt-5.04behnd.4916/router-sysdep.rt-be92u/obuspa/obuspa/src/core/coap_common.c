/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2017-2024  CommScope, Inc
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

//------------------------------------------------------------------------------------
// Mutex used to protect access to this component
static pthread_mutex_t coap_access_mutex;

//------------------------------------------------------------------------------
// Structure used to walk the CoAP option list.
// It is used to maintain state between each CoAP option, and also return the current parsed option
typedef struct
{
    unsigned char *buf;             // On input : pointer to option to parse
                                    // On output: pointer to next option to parse
    int len;                        // On input : length of buffer left for this option and following options
                                    // On output: length of buffer left for next options
    int cur_option;                 // On input : Last option parsed
                                    // On output: Option just parsed
    unsigned char *option_value;    // On input : don't care
                                    // On output: pointer to buffer containing the values for the option just parsed
    int option_len;                 // On input : don't care
                                    // On output: length of the buffer containing the values for the option just parsed
} option_walker_t;

//------------------------------------------------------------------------------
// Buffer containing the textual cause of the error - to copy into the payload of an ACK or RST message
static char coap_err_message[256];

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int WalkCoapOption(option_walker_t *ow, parsed_pdu_t *pp);
int ParseCoapOption(int option, unsigned char *buf, int len, parsed_pdu_t *pp);
void AppendUriPath(char *path, int path_len, char *segment, int seg_len);
void ParseBlock1Option(unsigned char *buf, int len, parsed_pdu_t *pp);
bool ParseCoapUriQuery(char *uri_query, mtp_conn_t *mtpc);
unsigned ReadUnsignedOptionValue(unsigned char *buf, int len);
pdu_block_size_t CalcBlockSize_Int2Pdu(int block_size);
int CalcBlockSize_Pdu2Int(pdu_block_size_t pdu_block_size);
int ReceiveDtlsHandshakePacket(SSL *ssl, BIO *rbio, int socket_fd, int timeout_in_sec);
int SendDtlsRecordFragments(int socket_fd, unsigned char *buf, int len);

/*********************************************************************//**
**
** COAP_Init
**
** Initialises this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_Init(void)
{
    int err;

    // Exit if unable to initialise CoAP servers
    err = COAP_SERVER_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to initialise CoAP clients
    err = COAP_CLIENT_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&coap_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** COAP_Start
**
** Creates the SSL contexts used by this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_Start(void)
{
    int err = USP_ERR_OK;

    COAP_LockMutex();

    // Exit if unable to start CoAP servers
    err = COAP_SERVER_InitStart();
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to start CoAP clients
    err = COAP_CLIENT_InitStart();
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Code was successful
    err = USP_ERR_OK;

exit:
    COAP_UnlockMutex();
    return err;
}

/*********************************************************************//**
**
** COAP_Destroy
**
** Frees all memory used by this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void COAP_Destroy(void)
{
    COAP_LockMutex();

    COAP_SERVER_Destroy();
    COAP_CLIENT_Destroy();

    // Prevent the data model from making any other changes to the MTP thread
    is_coap_mtp_thread_exited = true;

    COAP_UnlockMutex();
}

/*********************************************************************//**
**
** COAP_UpdateAllSockSet
**
** Updates the set of all COAP socket fds to read/write from
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void COAP_UpdateAllSockSet(socket_set_t *set)
{
    COAP_LockMutex();

    COAP_SERVER_UpdateAllSockSet(set);
    COAP_CLIENT_UpdateAllSockSet(set);

    COAP_UnlockMutex();
}

/*********************************************************************//**
**
** COAP_ProcessAllSocketActivity
**
** Processes the socket for the specified controller
**
** \param   set - pointer to socket set structure containing the sockets which need processing
**
** \return  Nothing
**
**************************************************************************/
void COAP_ProcessAllSocketActivity(socket_set_t *set)
{
    COAP_LockMutex();

    // Exit if MTP thread has exited
    // NOTE: This check should be unnecessary, as this function is only called from the MTP thread
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return;
    }

    COAP_CLIENT_ProcessAllSocketActivity(set);
    COAP_SERVER_ProcessAllSocketActivity(set);

    COAP_UnlockMutex();
}

/*********************************************************************//**
**
** COAP_AreAllResponsesSent
**
** Determines whether all responses have been sent, and that there are no outstanding incoming messages
**
** \param   None
**
** \return  true if all responses have been sent
**
**************************************************************************/
bool COAP_AreAllResponsesSent(void)
{
    bool all_responses_sent;

    COAP_LockMutex();

    // Exit if MTP thread has exited
    // NOTE: This check is not strictly necessary, as only the MTP thread should be calling this function
    if (is_coap_mtp_thread_exited)
    {
        COAP_UnlockMutex();
        return true;
    }

    all_responses_sent = COAP_CLIENT_AreAllResponsesSent() && COAP_SERVER_AreNoOutstandingIncomingMessages();

    COAP_UnlockMutex();

    return all_responses_sent;
}

/*********************************************************************//**
**
** COAP_ReceivePdu
**
** Reads a CoAP PDU addressed to our CoAP client
** NOTE: This function is called by both CoAP client and server
**
** \param   ssl - pointer to SSL object associated with the socket, or NULL if encryption is not enabled
** \param   rbio - pointer to BIO object for reading
** \param   socket_fd - socket on which to receive the PDU
** \param   buf - pointer to buffer in which to read CoAP PDU
** \param   buflen - length of buffer in which to read CoAP PDU
**
** \return  Number of bytes read, or -1 if the remote server disconnected
**
**************************************************************************/
int COAP_ReceivePdu(SSL *ssl, BIO *rbio, int socket_fd, unsigned char *buf, int buflen)
{
    int err;
    int ssl_flags;
    int retry_count;
    int bytes_read;

    (void)rbio; // unused in this implementation

    if (ssl == NULL)
    {
        // Exit if unable to read the CoAP PDU into the buffer
        bytes_read = recv(socket_fd, buf, buflen, 0);
        if (bytes_read == -1)
        {
            USP_ERR_ERRNO("recv", errno);
        }
        return bytes_read;
    }

    // Code below is complex because a renegotiation could occur, and open SSL requires that we retry the EXACT same SSL call
    // We cope with this by retrying the SSL call until the retry has completed (or failed)
    // This code blocks until the retry has completed, or the retry has timed out
    #define ONE_SECOND_IN_MICROSECONDS (1000000)
    #define SSL_RETRY_SLEEP (ONE_SECOND_IN_MICROSECONDS/20)             // Retry 20 times a second
    #define SSL_RETRY_TIMEOUT  (5*ONE_SECOND_IN_MICROSECONDS)           // Retry for upto 5 seconds
    #define MAX_SSL_RETRY_COUNT  (SSL_RETRY_TIMEOUT/SSL_RETRY_SLEEP)
    retry_count = 0;
    while (retry_count < MAX_SSL_RETRY_COUNT)
    {
        // Exit if read some bytes successfully
        bytes_read = SSL_read(ssl, buf, buflen);
        if (bytes_read > 0)
        {
            return bytes_read;
        }

        // Determine whether to retry this call until the read has occurred - this is needed if a renegotiation occurs
        err = SSL_get_error(ssl, bytes_read);

        // Exit if CoAP peer has gracefully disconnected
	    ssl_flags = SSL_get_shutdown(ssl);
        if ((bytes_read==0) || (ssl_flags & SSL_RECEIVED_SHUTDOWN))
        {
            return -1;
        }

        // Log exceptional failure causes
        USP_LOG_ErrorSSL(__FUNCTION__, "SSL_read() failed", bytes_read, err);

        switch(err)
        {
            case SSL_ERROR_NONE:
                // NOTE: I don't think this case will ever get executed because bytes_read would be >= 0
                // If there was no SSL error or no bytes to read, then assume the CoAP peer has gracefully disconnected
                if (bytes_read <= 0)
                {
                    return -1;
                }

                return bytes_read;
                break;

            case SSL_ERROR_ZERO_RETURN:
                // Exit if CoAP server has disconnected
                // NOTE: I don't think this case will ever get executed because it would have been caught earlier at the (bytes_read==0) test
                return -1;
                break;

            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_CONNECT:
            case SSL_ERROR_WANT_ACCEPT:
            case SSL_ERROR_WANT_X509_LOOKUP:
                usleep(SSL_RETRY_SLEEP);
                retry_count++;
                break;

            default:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
                // Exit if any other error occurred. Handle these the same as a disconnect
                return -1;
                break;
        }
    }

    // If the code gets here, then the retry timed out, so perform a disconnect
    USP_LOG_Error("%s: SSL Renegotiation timed out", __FUNCTION__);

    return -1;
}

/*********************************************************************//**
**
** COAP_SendPdu
**
** Sends a CoAP PDU
** NOTE: This function is called by both CoAP client and server
**
** \param   ssl - pointer to SSL object associated with the socket, or NULL if encryption is not enabled
** \param   wbio - pointer to BIO object for writing
** \param   socket_fd - socket on which to send the PDU
** \param   buf - pointer to buffer of data to send
** \param   len - length of buffer of data to send
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int COAP_SendPdu(SSL *ssl, BIO *wbio, int socket_fd, unsigned char *buf, int len)
{
    int err;
    int retry_count;
    int ssl_flags;
    int bytes_sent = 0;

    (void)wbio; // unused in this implementation

    // Perform a simple send() if connection is not encrypted
    if (ssl == NULL)
    {
        bytes_sent = send(socket_fd, buf, len, 0);
        if (bytes_sent != len)
        {
            // NOTE: We have failed to send the new block. It will be retried by the retry mechanism if this is a client, or the remote client will retry
            USP_ERR_ERRNO("send", errno);
            return USP_ERR_OK;
        }
        return USP_ERR_OK;
    }

    // Code below is complex because a renegotiation could occur, and open SSL requires that we retry the EXACT same SSL call
    // We cope with this by retrying the SSL call until the retry has completed (or failed)
    // This code blocks until the retry has completed, or the retry has timed out
    retry_count = 0;
    while (retry_count < MAX_SSL_RETRY_COUNT)
    {
        // Try sending
        bytes_sent = SSL_write(ssl, buf, len);
        if (bytes_sent > 0)
        {
            return USP_ERR_OK;;
        }

        // Determine whether to retry this call until the write has occurred - this is needed if a renegotiation occurs
        err = SSL_get_error(ssl, bytes_sent);

        // Exit if peer has disconnected
	    ssl_flags = SSL_get_shutdown(ssl);
        if ((bytes_sent==0) || (ssl_flags & SSL_RECEIVED_SHUTDOWN))
        {
            USP_PROTOCOL("%s: Peer has disconnected", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Log exceptional failure causes
        USP_LOG_ErrorSSL(__FUNCTION__, "SSL_write() failed", bytes_sent, err);

        switch(err)
        {
		    case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
                // NOTE: I don't think these can occur. If they do and nothing was sent out, then the CoAP retry mechanism will fix it anyway.
                return USP_ERR_OK;
                break;

            case SSL_ERROR_WANT_WRITE:
                // Wait a while, then perform the renegotiation
                usleep(SSL_RETRY_SLEEP);
                retry_count++;
				break;

            default:
            case SSL_ERROR_SYSCALL:
            case SSL_ERROR_SSL:
                return USP_ERR_INTERNAL_ERROR;
                break;
        }
    }

    // If the code gets here, then SSL renegotiation failed
    USP_LOG_Error("%s: SSL Renegotiation timed out", __FUNCTION__);

    return USP_ERR_INTERNAL_ERROR;
}

/*********************************************************************//**
**
** COAP_WriteRst
**
** Writes a CoAP PDU containing a RST of the message to send
** RST is empty message with Code 0
**
** \param   message_id - message_id which this RST is a response to, or a message_id allocated by us, if one could not be parsed from the input PDU
** \param   token - pointer to buffer containing token of received PDU that caused this RST
** \param   token_len - length of buffer containing token of received PDU that caused this RST
**                NOTE: if no token could be parsed from the input PDU, then the token will be empty
** \param   buf - pointer to buffer in which to write the CoAP PDU
** \param   len - length of buffer in which to write the CoAP PDU
**
** \return  Number of bytes written to the CoAP PDU buffer
**
**************************************************************************/
int COAP_WriteRst(int message_id, unsigned char *token, int token_len, unsigned char *buf, int len)
{
    unsigned header = 0;
    unsigned char *p;

    // Calculate the header bytes
    MODIFY_BITS(31, 30, header, COAP_VERSION);
    MODIFY_BITS(29, 28, header, kPduType_Reset);
    MODIFY_BITS(27, 24, header, token_len);
    MODIFY_BITS(23, 16, header, kPduClientErrRespCode_BadRequest);
    MODIFY_BITS(15, 0, header, message_id);

    // Write the CoAP header bytes and token into the output buffer
    p = buf;
    WRITE_4_BYTES(p, header);
    memcpy(p, token, token_len);
    p += token_len;

    // Log what is going to be sent
    USP_PROTOCOL("%s: Sending CoAP RST (MID=%d)", __FUNCTION__, message_id);

    // Return the number of bytes written to the output buffer
    return p - buf;
}

/*********************************************************************//**
**
** COAP_CalcBlockOption
**
** Calculates the value of the Block1 option for this tx
**
** \param   buf - pointer to buffer in which to write the block option's value.
**                It is the callers responsibility to ensure that this is at least 3 bytes long.
** \param   cur_block - Current block number to transmit
** \param   is_more_blocks - Set to 1 if there are more blocks containing the USP record, 0 if this is the last block
** \param   block_size - Size of the blocks containing the USP record
**
** \return  Number of bytes written into the block option's value buffer
**
**************************************************************************/
int COAP_CalcBlockOption(unsigned char *buf, int cur_block, int is_more_blocks, int block_size)
{
    pdu_block_size_t pdu_block_size;
    int block_option_len;
    unsigned option = 0;

    // Convert the block size to the PDU enumeration
    pdu_block_size = CalcBlockSize_Int2Pdu(block_size);

    // Determine how many bytes to store in the block
    if (cur_block < 16)
    {
        block_option_len = 1;
    }
    else if (cur_block < 4096)
    {
        block_option_len = 2;
    }
    else
    {
        block_option_len = 3;
    }

    // Write the block option
    switch(block_option_len)
    {
        case 1:
            MODIFY_BITS(7, 4, option, cur_block);
            MODIFY_BITS(3, 3, option, is_more_blocks);
            MODIFY_BITS(2, 0, option, pdu_block_size);
            STORE_BYTE(buf, option);
            break;

        case 2:
            MODIFY_BITS(15, 4, option, cur_block);
            MODIFY_BITS(3, 3, option, is_more_blocks);
            MODIFY_BITS(2, 0, option, pdu_block_size);
            STORE_2_BYTES(buf, option);
            break;

        case 3:
            MODIFY_BITS(23, 4, option, cur_block);
            MODIFY_BITS(3, 3, option, is_more_blocks);
            MODIFY_BITS(2, 0, option, pdu_block_size);
            STORE_3_BYTES(buf, option);
            break;
    }

    return block_option_len;
}

/*********************************************************************//**
**
** COAP_WriteOption
**
** Writes the specified CoAP option to the specified buffer
**
** \param   pdu_option - CoAP option to write
** \param   option_data - pointer to buffer containing the data for the option to write
** \param   len - number of bytes of option_data
** \param   buf - pointer to output buffer in which to write option
** \param   last_pdu_option - pointer to variable in which to return the CoAP option written by this function
**                            NOTE: This is necessary as CoAP uses a delta encoding for option numbers
**
** \return  pointer to next byte in the output buffer after writing this option
**
**************************************************************************/
unsigned char *COAP_WriteOption(pdu_option_t pdu_option, unsigned char *option_data, int len, unsigned char *buf, pdu_option_t *last_pdu_option)
{
    int option_delta;
    int option_delta_ext = 0;
    int option_len;
    int option_len_ext = 0;
    unsigned option_header = 0;
    unsigned char *p;

    // Calculate option_delta, determining whether option_delta_ext is present
    option_delta = pdu_option - (*last_pdu_option);
    USP_ASSERT(option_delta >= 0);    // Ensure that the caller attempts to write options in numeric order
    if (option_delta > 12)
    {
        USP_ASSERT(option_delta < 268); // This code does not cope with 16 bit option deltas
        option_delta_ext = option_delta - 13;
        option_delta = 13;
    }

    // Calculate option_len, determining whether option_len_ext is present
    option_len = len;
    if (option_len > 12)
    {
        USP_ASSERT(option_len < 268); // This code does not cope with 16 bit option lengths
        option_len_ext = option_len - 13;
        option_len = 13;
    }

    // Calculate the option header
    MODIFY_BITS(7, 4, option_header, option_delta);
    MODIFY_BITS(3, 0, option_header, option_len);

    // Write the option header
    p = buf;
    WRITE_BYTE(p, option_header);

    // Write the option_delta_ext if necessary
    if (option_delta == 13)
    {
        WRITE_BYTE(p, option_delta_ext);
    }

    // Write the option_len_ext if necessary
    if (option_len == 13)
    {
        WRITE_BYTE(p, option_len_ext);
    }

    // Write the option data
    memcpy(p, option_data, len);
    p += len;

    // Update the last pdu option, so that the next call to this function can update the delta from the last option written
    *last_pdu_option = pdu_option;

    return p;
}

/*********************************************************************//**
**
** COAP_ParsePdu
**
** Parses the specified PDU into a structure
**
** \param   buf - pointer to buffer containing CoAP PDU to parse
** \param   len - length of buffer containing CoAP PDU to parse
** \param   pp - pointer to structure in which to store the parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
unsigned COAP_ParsePdu(unsigned char *buf, int len, parsed_pdu_t *pp)
{
    option_walker_t walker;
    unsigned header;
    unsigned action_flags = COAP_NO_ERROR;

    // Exit if size of packet is not large enough for a CoAP packet
    if (len < COAP_HEADER_SIZE)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=?) was too small (len=%d). Ignoring.", __FUNCTION__, len);
        return IGNORE_PDU;
    }

    // Parse the header
    header = READ_4_BYTES(buf, len);
    pp->coap_version = BITS(31, 30, header);
    pp->pdu_type = BITS(29, 28, header);
    pp->token_size = BITS(27, 24, header);
    pp->pdu_class = BITS(23, 21, header);
    pp->request_response_code = BITS(20, 16, header);
    pp->message_id = BITS(15, 0, header);

    // Exit if PDU has incorrect CoAP version
    if (pp->coap_version != COAP_VERSION)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has incorrect CoAP version (version=%d)", __FUNCTION__, pp->message_id, pp->coap_version);
        return SEND_RST;
    }

    // Exit if PDU is using a class reserved for future expansion
    if ((pp->pdu_class == 1) || (pp->pdu_class == 6) || (pp->pdu_class == 7))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) is using a reserved class (class=%d)", __FUNCTION__, pp->message_id, pp->pdu_class);
        return SEND_RST;
    }

    // Exit if token size is too large or message not large enough to contain the specified token
    if ((pp->token_size > sizeof(pp->token)) || (len < pp->token_size))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has incorrect token size (token_size=%d, size_left=%d)", __FUNCTION__, pp->message_id, pp->token_size, len);
        return SEND_RST;
    }

    // Copy the token
    READ_N_BYTES(pp->token, buf, pp->token_size, len);

    // Parse all options
    memset(&walker, 0, sizeof(walker));
    walker.buf = buf;
    walker.len = len;
    walker.cur_option = kPduOption_Zero;
    while ((walker.len > 0) && (walker.buf[0] != PDU_OPTION_END_MARKER))
    {
        // Exit if unable to parse the TLV metadata of the option
        action_flags = WalkCoapOption(&walker, pp);
        if (action_flags != COAP_NO_ERROR)
        {
            return action_flags;
        }

        // Exit if an error occurred in parsing the option itself
        action_flags = ParseCoapOption(walker.cur_option, walker.option_value, walker.option_len, pp);
        if (action_flags != COAP_NO_ERROR)
        {
            return action_flags;
        }
    }

    // Skip the PDU_OPTION_END_MARKER
    // NOTE: This may not be present after the options if there is no payload
    if ((walker.len > 0) && (walker.buf[0] == PDU_OPTION_END_MARKER))
    {
        buf = walker.buf + 1;
        len = walker.len - 1;
    }

    // Store pointer to the payload and it's size. This is whatever is left in the PDU.
    pp->payload = buf;
    pp->payload_len = len;

    return COAP_NO_ERROR;
}

/*********************************************************************//**
**
** COAP_LockMutex
**
** Take the mutex protecting the CoAP sub-system
**
** \param   None
**
** \return  None
**
**************************************************************************/
void COAP_LockMutex(void)
{
    OS_UTILS_LockMutex(&coap_access_mutex);
}

/*********************************************************************//**
**
** COAP_UnlockMutex
**
** Release the mutex protecting the CoAP sub-system
**
** \param   None
**
** \return  None
**
**************************************************************************/
void COAP_UnlockMutex(void)
{
    OS_UTILS_UnlockMutex(&coap_access_mutex);
}

/*********************************************************************//**
**
** COAP_SetErrMessage
**
** Stores the textual cause of the error, so that it may be copied later into the payload of the ACK or RST message
**
** \param   fmt - printf style format
**
** \return  None
**
**************************************************************************/
void COAP_SetErrMessage(char *fmt, ...)
{
    va_list ap;

    // Write the error message into the buffer, ensuring it is always zero terminated
    va_start(ap, fmt);
    vsnprintf(coap_err_message, sizeof(coap_err_message), fmt, ap);
    coap_err_message[sizeof(coap_err_message)-1] = '\0';
    va_end(ap);

    USP_PROTOCOL("%s", coap_err_message);
}

/*********************************************************************//**
**
** COAP_GetErrMessage
**
** Returns a pointer to the stored CoAP error message
**
** \param   None
**
** \return  pointer to stored error message
**
**************************************************************************/
char *COAP_GetErrMessage(void)
{
    return coap_err_message;
}

/*********************************************************************//**
**
** WalkCoapOption
**
** Called to walk through each CoAP option
**
** \param   ow - pointer to parameters which are used to walk the option list
**               On input:  The parameters in this structure point to the option to parse
**               On output: The parameters in this structure point to the next option to parse, and return the current option and it's values
** \param   pp - pointer to parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
int WalkCoapOption(option_walker_t *ow, parsed_pdu_t *pp)
{
    unsigned char *buf;
    int len;
    unsigned option_header;
    int option_delta;
    int option_delta_ext;
    int option_len;
    int option_len_ext;
    int buffer_required;

    // Exit if buffer length left is not enough to include the option header
    buf = ow->buf;
    len = ow->len;
    if (len < 1)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has not enough packet left for option header", __FUNCTION__, pp->message_id);
        return SEND_RST;
    }

    // Read and parse the option header
    option_header = READ_BYTE(buf, len);
    option_delta = BITS(7, 4, option_header);
    option_len = BITS(3, 0, option_header);

    // Exit if Option Delta or option len are encoded incorrectly
    // NOTE: It is an error to call this code for the PDU_OPTION_END_MARKER
    USP_ASSERT(option_header != PDU_OPTION_END_MARKER)
    if ((option_delta == 15) || (option_len==15))
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has invalid option_delta or option_len (=15)", __FUNCTION__, pp->message_id);
        return SEND_RST;
    }

    // Calculate the amount of buffer left needed to parse option_delta_ext and option_len_ext
    buffer_required = 0;
    if (option_delta > 12)
    {
        buffer_required += option_delta - 12;
    }

    if (option_len > 12)
    {
        buffer_required += option_len - 12;
    }

    // Exit if there is not enough buffer left to parse option_delta_ext and option_len_ext
    if (len < buffer_required)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has not enough packet left for option_delta_ext and option_len_ext", __FUNCTION__, pp->message_id);
        return SEND_RST;
    }

    // Parse option_delta_ext, and update option_delta with it
    if (option_delta == 13)
    {
        option_delta_ext = READ_BYTE(buf, len);
        option_delta = option_delta_ext + 13;
    }
    else if (option_delta == 14)
    {
        option_delta_ext = READ_2_BYTES(buf, len);
        option_delta = option_delta_ext + 269;
    }

    // Parse option_len_ext, and update option_len with it
    if (option_len == 13)
    {
        option_len_ext = READ_BYTE(buf, len);
        option_len = option_len_ext + 13;
    }
    else if (option_len == 14)
    {
        option_len_ext = READ_2_BYTES(buf, len);
        option_len = option_len_ext + 269;
    }

    // Exit if there is not enough buffer left for the option's value
    if (len < option_len)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has not enough packet left to contain the option's value (option=%d)", __FUNCTION__, pp->message_id, ow->cur_option + option_delta);
        return SEND_RST;
    }

    // Update the option walker to point to the next option, and return this option
    ow->buf = buf + option_len;
    ow->len = len - option_len;
    ow->cur_option = ow->cur_option + option_delta;
    ow->option_value = buf;
    ow->option_len = option_len;

    return COAP_NO_ERROR;
}

/*********************************************************************//**
**
** ParseCoapOption
**
** Parses the specified coap option
**
** \param   cs - coap server that sent the PDU containing the specified option
** \param   option - coap option
** \param   buf - pointer to buffer containing the value of the coap option
** \param   len - length of the buffer containing the value of the coap option
** \param   pp - pointer to structure in which to store the parsed CoAP PDU
**
** \return  action flags determining what actions to take
**
**************************************************************************/
int ParseCoapOption(int option, unsigned char *buf, int len, parsed_pdu_t *pp)
{
    bool is_wrong_length = false;           // assume that option is correct length
    bool result;

    // Determine if the option's value is the wrong length
    switch(option)
    {
        case kPduOption_UriHost:
        case kPduOption_UriPath:
        case kPduOption_UriQuery:
            if (len == 0)
            {
                is_wrong_length = true;
            }
            break;

        case kPduOption_ContentFormat:
            if (len > 2)
            {
                is_wrong_length = true;
            }
            break;

        case kPduOption_UriPort:
            if (len != 2)
            {
                is_wrong_length = true;
            }
            break;

        case kPduOption_Size1:
            if (len > 4)        // Size1 option is 0 to 4 bytes
            {
                is_wrong_length = true;
            }
            break;

        case kPduOption_Block1:
            if (len > 3)        // Block1 option is 0 to 3 bytes
            {
                is_wrong_length = true;
            }
            break;

        default:
            break;
    }

    // Exit if the option is the wrong length
    if (is_wrong_length)
    {
        COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) has incorrect length for an option (option=%d, len=%d)", __FUNCTION__, pp->message_id, option, len);
        return SEND_RST;
    }

    // Copy the option's parsed value into the parsed_pdu structure
    switch(option)
    {
        case kPduOption_UriHost:
        case kPduOption_UriPort:
            // NOTE: Ignore these options, as we do not host multiple virtual servers
            break;

        case kPduOption_UriPath:
            pp->options_present |= URI_PATH_PRESENT;
            AppendUriPath(pp->uri_path, sizeof(pp->uri_path), (char *)buf, len);
            break;

        case kPduOption_UriQuery:
            pp->options_present |= URI_QUERY_PRESENT;
            TEXT_UTILS_StrncpyLen(pp->uri_query, sizeof(pp->uri_query), (char *)buf, len);
            USP_PROTOCOL("%s: Received CoAP UriQueryOption='%s'", __FUNCTION__, pp->uri_query);

            result = ParseCoapUriQuery(pp->uri_query, &pp->mtp_conn);
            if (result == false)
            {
                COAP_SetErrMessage("%s: Received CoAP URI query option (%s) is incorrectly formed", __FUNCTION__, pp->uri_query);
                return SEND_ACK | INDICATE_BAD_OPTION;
            }

            break;

        case kPduOption_ContentFormat:
            pp->options_present |= CONTENT_FORMAT_PRESENT;
            pp->content_format = ReadUnsignedOptionValue(buf, len);
            break;

        case kPduOption_Block1:
            ParseBlock1Option(buf, len, pp);
            break;

        case kPduOption_Block2:
            // Ignore the Block2 option. It is used in requests to suggest a block size for the response. But USP uses POST, without piggybacked responses.
            break;

        case kPduOption_Size1:
            pp->options_present |= SIZE1_PRESENT;
            pp->total_size = ReadUnsignedOptionValue(buf, len);
            break;

        default:
            if ((option & 1) == 1)
            {
                // Odd numbered options are 'critical' and must cause the return of a 4.02 (Bad Option) if not handled
                COAP_SetErrMessage("%s: Received CoAP PDU (MID=%d) contains an unhandled critical option (option=%d)", __FUNCTION__, pp->message_id, option);
                if (pp->pdu_type == kPduType_Confirmable)
                {
                    return SEND_ACK | INDICATE_BAD_OPTION;
                }
                else
                {
                    return SEND_RST;
                }
            }
            else
            {
                // Even numbered options are 'elective' and can be ignored if we don't parse them
            }
            break;
    }

    return COAP_NO_ERROR;
}

/*********************************************************************//**
**
** AppendUriPath
**
** Appends the specified path segment to the URI path buffer
**
** \param   path - pointer to buffer containing the current URI path that we wish to append to
** \param   path_len - maximum length of the buffer containing the URI path
** \param   segment - pointer to buffer (not NULL terminated) containing the path segment to append
** \param   seg_len - length of the path segment to append
**
** \return  None
**
**************************************************************************/
void AppendUriPath(char *path, int path_len, char *segment, int seg_len)
{
    char buf[MAX_COAP_URI_PATH];
    int len;

    // Exit if this is the first segment, copying it into the buffer without a leading '/' separator
    if (path[0] == '\0')
    {
        TEXT_UTILS_StrncpyLen(path, path_len, segment, seg_len);
        return;
    }

    // Form the path segment as a NULL terminated string, with leading '/' separator in local buffer
    buf[0] = '/';
    TEXT_UTILS_StrncpyLen(&buf[1], sizeof(buf)-1, segment, seg_len);  // Minus 1 because of '/' separator at the beginning

    // Append path segment in local buffer to URI path
    len = strlen(path);
    USP_STRNCPY(&path[len], buf, path_len-len);
}

/*********************************************************************//**
**
** ParseBlock1Option
**
** Parses the Block1 option's value
**
** \param   buf - pointer to buffer containing the value of the Block1 option
** \param   len - length of the buffer containing the value of the Block1 option
** \param   pp - pointer to structure in which to store the parsed CoAP PDU
**
** \return  None
**
**************************************************************************/
void ParseBlock1Option(unsigned char *buf, int len, parsed_pdu_t *pp)
{
    unsigned value;
    pdu_block_size_t pdu_block_size;

    pp->options_present |= BLOCK1_PRESENT;

    switch(len)
    {
        default:
        case 0:
            pp->rxed_block = 0;
            pp->is_more_blocks = 0;
            pdu_block_size = kPduBlockSize_16;
            break;

        case 1:
            value = READ_BYTE(buf, len);
            pp->rxed_block = BITS(7, 4, value);
            pp->is_more_blocks = BITS(3, 3, value);
            pdu_block_size = BITS(2, 0, value);
            break;

        case 2:
            value = READ_2_BYTES(buf, len);
            pp->rxed_block = BITS(15, 4, value);
            pp->is_more_blocks = BITS(3, 3, value);
            pdu_block_size = BITS(2, 0, value);
            break;

        case 3:
            value = READ_3_BYTES(buf, len);
            pp->rxed_block = BITS(23, 4, value);
            pp->is_more_blocks = BITS(3, 3, value);
            pdu_block_size = BITS(2, 0, value);
            break;
    }

    // Convert the enumerated block size back into an integer
    pp->block_size = CalcBlockSize_Pdu2Int(pdu_block_size);
}

/*********************************************************************//**
**
** ParseCoapUriQuery
**
** Parses the URI Query option into an mtp_conn structure
** The format of the URI Query option is:
**    reply_to=coap[s]:// hostname [':' port] '/' resource
**
** NOTE: On exit, the strings in the mtp_conn structure will point to within the uri_query (input) buffer
**
** \param   uri_query - pointer to buffer containing the URI query option to parse
**                      NOTE: This buffer will be altered by this function
** \param   mtpc - pointer to structure containing the parsed 'reply-to'
**
** \return  true if parsed successfully
**
**************************************************************************/
bool ParseCoapUriQuery(char *uri_query, mtp_conn_t *mtpc)
{
    char *p;
    char *p_slash;
    char *p_colon;
    char *hostname_end;
    char *endptr;

    // Determine if the reply is to an encrypted port or not (and set the default port based on encryption status)
    #define URI_QUERY_COAP  "reply-to=coap://"
    #define URI_QUERY_COAPS "reply-to=coaps://"
    p = uri_query;
    if (strncmp(p, URI_QUERY_COAP, sizeof(URI_QUERY_COAP)-1) == 0)
    {
        mtpc->coap.encryption = false;
        mtpc->coap.port = 5683;
        p += sizeof(URI_QUERY_COAP)-1;
    }
    else if (strncmp(p, URI_QUERY_COAPS, sizeof(URI_QUERY_COAPS)-1) == 0)
    {
        mtpc->coap.encryption = true;
        mtpc->coap.port = 5684;
        p += sizeof(URI_QUERY_COAPS)-1;
    }
    else
    {
        return false;
    }

    // Exit if the rest of the string does not contain a slash character separating the hostname (and possibly port) from the resource
    p_slash = strchr(p, '/');
    if (p_slash == NULL)
    {
        return false;
    }
    hostname_end = p_slash;

    // Determine whether an optional port number is present, before the slash
    p_colon = strchr(p, ':');
    if ((p_colon != NULL) && (p_colon < p_slash))
    {
        // Exit if not all of the characters from the colon to the slash are part of the port value
        mtpc->coap.port = strtol(&p_colon[1], &endptr, 10);
        if (endptr != p_slash)
        {
            return false;
        }
        hostname_end = p_colon;
    }

    *hostname_end = '\0';               // Terminate the hostname in the input buffer
    mtpc->coap.host = p;
    mtpc->coap.resource = &p_slash[1];
    mtpc->protocol = kMtpProtocol_CoAP;
    mtpc->is_reply_to_specified = true;

    return true;
}

/*********************************************************************//**
**
** ReadUnsignedOptionValue
**
** Reads the value of an option contained in a variable number of bytes in network byte order
**
** \param   buf - pointer to buffer containing the value of the option
** \param   len - length of the buffer containing the value of the option
**
** \return  value of the option
**
**************************************************************************/
unsigned ReadUnsignedOptionValue(unsigned char *buf, int len)
{
    unsigned value = 0;

    switch(len)
    {
        case 0:
            value = 0;
            break;

        case 1:
            value = READ_BYTE(buf, len);
            break;


        case 2:
            value = READ_2_BYTES(buf, len);
            break;

        case 3:
            value = READ_3_BYTES(buf, len);
            break;

        case 4:
            value = READ_4_BYTES(buf, len);
            break;

        default:
            TERMINATE_BAD_CASE(len);
            break;
    }

    return value;
}

/*********************************************************************//**
**
** CalcBlockSize_Int2Pdu
**
** Converts the block size integer into an enumeration suitable to be used in Block option
**
** \param   block_size - size of block in bytes
**
** \return  Enumerated value representing block size
**
**************************************************************************/
pdu_block_size_t CalcBlockSize_Int2Pdu(int block_size)
{
    pdu_block_size_t pdu_block_size = kPduBlockSize_16;

    switch(block_size)
    {
        case 1024:
            pdu_block_size = kPduBlockSize_1024;
            break;

        case 512:
            pdu_block_size = kPduBlockSize_512;
            break;

        case 256:
            pdu_block_size = kPduBlockSize_256;
            break;

        case 128:
            pdu_block_size = kPduBlockSize_128;
            break;

        case 64:
            pdu_block_size = kPduBlockSize_64;
            break;

        case 32:
            pdu_block_size = kPduBlockSize_32;
            break;

        case 16:
            pdu_block_size = kPduBlockSize_16;
            break;

        default:
            TERMINATE_BAD_CASE(block_size);
            break;
    }

    return pdu_block_size;
}

/*********************************************************************//**
**
** CalcBlockSize_Pdu2Int
**
** Converts the pdu block size enumeration into an integer
**
** \param   pdu_block_size - size of block in enumeration
**
** \return  Number of bytes in block
**
**************************************************************************/
int CalcBlockSize_Pdu2Int(pdu_block_size_t pdu_block_size)
{
    int block_size;
    block_size = 1 << (4 + pdu_block_size);

    return block_size;
}







#endif // ENABLE_COAP
