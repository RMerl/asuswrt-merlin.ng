/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file usp_coap.h
 *
 * Header file for CoAP connection
 *
 */
#ifndef USP_COAP_H
#define USP_COAP_H

#ifdef ENABLE_COAP

#include "common_defs.h"
#include "socket_set.h"
#include "usp-msg.pb-c.h"
#include "device.h"             // for mtp_conn_t


//------------------------------------------------------------------------
// Defines whose default is set by RFC7252
#define COAP_VERSION 1                  // The version number of the CoAP protocol, at the start of all CoAP PDUs
#define COAP_MAX_RETRANSMIT 4           // Maximum number of retries. The total number of times the BLOCK message is sent is one more than this.
#define COAP_ACK_TIMEOUT 2              // Minimum initial timeout for receiving an ACK message
#define MAX_COAP_PDU_SIZE  1152         // Maximum size of a CoAP PDU. This is set by the maximum UDP packet size.

#define COAP_HEADER_SIZE 4              // Number of bytes in a CoAP header. This is the smallest number of bytes for a valid CoAP PDU
#define MAX_COAP_PAYLOAD_SIZE 1024      // Maximum size of a payload in a block
#define MAX_COAP_TOKEN_SIZE 8           // Maximum number of bytes in a CoAP token
#define MAX_OPTION_HEADER_SIZE 3        // Maximum number of bytes in an option header (Option + option_delta_ext + option_len_ext)

//------------------------------------------------------------------------
// Defines for this implementation (not set by any RFC)
#define COAP_CLIENT_PAYLOAD_TX_SIZE  MAX_COAP_PAYLOAD_SIZE   // Maximum size of payload that we will send
#define COAP_CLIENT_PAYLOAD_RX_SIZE  MAX_COAP_PAYLOAD_SIZE   // Maximum size of payload that we would like to receive

#define MAX_COAP_URI_PATH  128      // Maximum size of buffer containing the URI path received in the PDU
#define MAX_COAP_URI_QUERY 128      // Maximum size of URI query received in the PDU

#define MAX_COAP_RECONNECTS (COAP_MAX_RETRANSMIT) // Maximum number of times that our CoAP client tries to connect to a CoAP server.
                                                  //  Retry occurs if unable to resolve server IP address, or a timeout occurred receiving an ACK on a DTLS session
#define RECONNECT_TIMEOUT   1      // Number of seconds to wait after our CoAP client could not connect to a CoAP server
                                   // (because unable to resolve server IP address, or DTLS failed or peer's server had silently reset the DTLS connection)

#define COAP_CLIENT_LINGER_PERIOD 300     // Number of seconds to keep the CoAP client connected to a USP Controller before disconnecting

#define DTLS_READ_TIMEOUT 2   // This corresponds to a total timeout of 5 seconds (1=>2s, 2=>5s, 3=>6s, 4=>11s, 8=>23s, 15=>30s )

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU message type (defined in RFC7252)
typedef enum
{
    kPduType_Confirmable = 0,
    kPduType_NonConfirmable = 1,
    kPduType_Acknowledgement = 2,
    kPduType_Reset = 3,
} pdu_type_t;

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU message class (defined in RFC7252)
typedef enum
{
    kPduClass_Request = 0,
    kPduClass_SuccessResponse = 2,
    kPduClass_ClientErrorResponse = 4,
    kPduClass_ServerErrorResponse = 5,
} pdu_class_t;

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU request methods (defined in RFC7252)
typedef enum
{
    kPduRequestMethod_Get = 1,
    kPduRequestMethod_Post = 2,
    kPduRequestMethod_Put = 3,
    kPduRequestMethod_Delete = 4,
} pdu_request_method_t;

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU success response codes (2.X defined in RFC7252 and RFC7959)
enum
{
    kPduSuccessRespCode_Created = 1,
    kPduSuccessRespCode_Deleted = 2,
    kPduSuccessRespCode_Valid = 3,
    kPduSuccessRespCode_Changed = 4,
    kPduSuccessRespCode_Content = 5,
    kPduSuccessRespCode_Continue = 31,
};

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU client error response codes (4.X defined in RFC7252)
enum
{
    kPduClientErrRespCode_BadRequest = 0,       // We could not understand the request due to invalid syntax
    kPduClientErrRespCode_Unauthorized = 1,
    kPduClientErrRespCode_BadOption = 2,        // We received a 'critical' option which we could not parse
    kPduClientErrRespCode_Forbidden = 3,
    kPduClientErrRespCode_NotFound = 4,         // We received a uri_path which did not match that of our USP resource
    kPduClientErrRespCode_MethodNotAllowed = 5, // We received a PDU that wasn't a POST
    kPduClientErrRespCode_NotAcceptable = 6,
    kPduClientErrRespCode_RequestEntityIncomplete = 8,  // We received a block number which was not the next expected one
    kPduClientErrRespCode_PreconditionFailed = 12,
    kPduClientErrRespCode_RequestEntityTooLarge = 13,   // We received more blocks in a USP Record, than we support
    kPduClientErrRespCode_UnsupportedContentFormat = 15, // We received a content format which was not application/octet-stream
};

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU server error response codes (5.X defined in RFC7252)
enum
{
    kPduServerErrRespCode_InternalServerError = 0,
    kPduServerErrRespCode_NotImplemented = 1,
    kPduServerErrRespCode_BadGateway = 2,
    kPduServerErrRespCode_ServiceUnavailable = 3,
    kPduServerErrRespCode_GatewayTimeout = 4,
    kPduServerErrRespCode_ProxyingNotSupported = 5, // We received the proxy-uri or proxy-scheme options, which we do not support
};

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU Option numbers (defined in RFC7252, RFC7959)
typedef enum
{
    kPduOption_Zero = 0,
    kPduOption_IfMatch = 1,
    kPduOption_UriHost = 3,
    kPduOption_ETag = 4,
    kPduOption_IfNoneMatch = 5,
    kPduOption_UriPort = 7,
    kPduOption_LocationPath = 8,
    kPduOption_UriPath = 11,
    kPduOption_ContentFormat = 12,
    kPduOption_MaxAge = 14,
    kPduOption_UriQuery = 15,
    kPduOption_Accept = 17,
    kPduOption_LocationQuery = 20,
    kPduOption_Block2 = 23,
    kPduOption_Block1 = 27,
    kPduOption_Size2 = 28,
    kPduOption_ProxyUri = 35,
    kPduOption_ProxyScheme = 39,
    kPduOption_Size1 = 60,
} pdu_option_t;

#define PDU_OPTION_END_MARKER 255

//------------------------------------------------------------------------
// Enumeration representing CoAP PDU content format (defined in RFC7252)
typedef enum
{
    kPduContentFormat_Text = 0,         // text/plain
    kPduContentFormat_LinkFormat = 40,  // application/link-format
    kPduContentFormat_XML = 41,         // application/xml
    kPduContentFormat_OctetStream = 42, // application/octet-stream // Only this may be used for USP records
    kPduContentFormat_EXI = 47,         // application/exi
    kPduContentFormat_JSON = 50,        // application/json
} pdu_content_format_t;

//------------------------------------------------------------------------
// Enumeration representing CoAP Block sizes (defined in RFC7959)
// NOTE: This is just used to make the code easier to read. The size is just  1 << (4 + enum)
typedef enum
{
    kPduBlockSize_16 = 0,
    kPduBlockSize_32 = 1,
    kPduBlockSize_64 = 2,
    kPduBlockSize_128 = 3,
    kPduBlockSize_256 = 4,
    kPduBlockSize_512 = 5,
    kPduBlockSize_1024 = 6,
} pdu_block_size_t;

//------------------------------------------------------------------------------
// Bitmask used to specify which options were parsed into the parsed_pdu_t structure
#define BLOCK1_PRESENT              0x00000001
#define URI_PATH_PRESENT            0x00000002
#define URI_QUERY_PRESENT           0x00000004
#define CONTENT_FORMAT_PRESENT      0x00000008
#define SIZE1_PRESENT               0x00000010

//------------------------------------------------------------------------------
// Structure representing a parsed PDU
typedef struct
{
    // CoAP Header
    unsigned coap_version;
    pdu_type_t pdu_type;
    pdu_class_t pdu_class;
    int request_response_code;

    // Token and MessageId
    unsigned token_size;
    unsigned char token[MAX_COAP_TOKEN_SIZE];
    unsigned message_id;

    // Bitmask of options that were parsed
    unsigned options_present;

    // Parsed from the Block1 option
    int rxed_block;     // Block number received in this block (counts from 0)
    int is_more_blocks; // Set to 0 if this is the last block, set to 1 if there are more blocks
    int block_size;     // Size of the received block

    // Other parsed options
    char uri_path[MAX_COAP_URI_PATH];   // NOTE: path is stored as percent decoded
    char uri_query[MAX_COAP_URI_QUERY];  // NOTE: query is stored as percent encoded
    mtp_conn_t mtp_conn;    // NOTE: pointers in this structure point to strings in pp->uri_query
    pdu_content_format_t content_format;
    unsigned total_size;     // total size of the USP record being transferred in blocks

    // Payload
    unsigned char *payload; // Pointer to payload in the buffer that contained the PDU that was parsed
    int payload_len;        // Length of the payload

} parsed_pdu_t;

//------------------------------------------------------------------------------
// Bitmask used to specify what to do in response to receiving a CoAP PDU
#define COAP_NO_ERROR           0x00000000      // No error, and nothing to do (yet)
#define SEND_RST                0x00000001      // Send a RST CoAP PDU
#define SEND_ACK                0x00000002      // Send an ACK CoAP PDU
#define INDICATE_BAD_REQUEST    0x00000004      // Send an ACK containing 4.00 Bad request
#define INDICATE_BAD_OPTION     0x00000008      // Send an ACK containing 4.02 Bad Option
#define INDICATE_NOT_FOUND      0x00000010      // Send an ACK containing 4.04 Not found
#define INDICATE_BAD_METHOD     0x00000020      // Send an ACK containing 4.05 Method not allowed
#define INDICATE_INCOMPLETE     0x00000040      // Send an ACK containing 4.08 Request entity Incomplete
#define INDICATE_TOO_LARGE      0x00000080      // Send an ACK containing 4.13 Request entity too large
#define INDICATE_BAD_CONTENT    0x00000100      // Send an ACK containing 4.15 Unsupported content format
#define INDICATE_WELL_KNOWN     0x00000200      // Send an ACK containing 2.05 Content containing the '.well-known/core' response

#define SEND_NEXT_USP_RECORD    0x02000000      // Starts sending the next USP record to the controller
#define SEND_NEXT_BLOCK         0x04000000      // Send the next block of the USP record to the controller
#define IGNORE_PDU              0x08000000      // Ignore the current PDU that was received
#define RESET_STATE             0x10000000      // Reset the state of the transmission or reception back to the beginning
#define RESEND_LAST_RESPONSE    0x20000000      // Resend the last response PDU, because we've received a duplicate message
#define ABORT_SENDING           0x40000000      // Abort sending any more PDUs, because we've received an RST
#define USP_RECORD_COMPLETE     0x80000000      // Process the USP record in cs->usp_buf

// Compound flag grouping all causes of indicating an error in an ACK
#define INDICATE_ERR_IN_ACK   (INDICATE_BAD_REQUEST | INDICATE_BAD_OPTION | INDICATE_NOT_FOUND | INDICATE_BAD_METHOD | INDICATE_INCOMPLETE | INDICATE_TOO_LARGE | INDICATE_BAD_CONTENT)


//------------------------------------------------------------------------------
// Macro used in logging to print out whether encryption is enabled or not
#define IS_ENCRYPTED_STRING(enc) (enc==true) ? "encrypted" : "unencrypted"

//------------------------------------------------------------------------------
// Structure containing CoAP configuration parameters
typedef struct
{
    unsigned port;                  // port on which this agent listens for CoAP messages
    char *resource;                 // CoAP resource path representing this agent's USP handler
    bool enable_encryption;         // Whether connections to this port are encrypted
} coap_config_t;

//------------------------------------------------------------------------------
// API
// coap_server.c
int COAP_SERVER_Init(void);
int COAP_SERVER_InitStart(void);
void COAP_SERVER_Destroy(void);
int COAP_SERVER_Start(int instance, char *interface, coap_config_t *config);
int COAP_SERVER_Stop(int instance, char *interface, coap_config_t *unused);
mtp_status_t COAP_SERVER_GetStatus(int instance);
void COAP_SERVER_UpdateAllSockSet(socket_set_t *set);
void COAP_SERVER_ProcessAllSocketActivity(socket_set_t *set);
bool COAP_SERVER_AreNoOutstandingIncomingMessages(void);
int CalcUriQueryOption(int socket_fd, bool encryption_preference, char *buf, int len);

// coap_client.c
int COAP_CLIENT_Init(void);
int COAP_CLIENT_InitStart(void);
void COAP_CLIENT_Destroy(void);
int COAP_CLIENT_Start(int cont_instance, int mtp_instance, char *endpoint_id);
void COAP_CLIENT_Stop(int cont_instance, int mtp_instance);
void COAP_CLIENT_AllowConnect(void);
void COAP_CLIENT_UpdateAllSockSet(socket_set_t *set);
void COAP_CLIENT_ProcessAllSocketActivity(socket_set_t *set);
int COAP_CLIENT_QueueBinaryMessage(mtp_send_item_t *msi, int cont_instance, int mtp_instance, mtp_conn_t *mtpc, time_t expiry_time);
bool COAP_CLIENT_AreAllResponsesSent(void);

// coap_common.c
int COAP_Init(void);
int COAP_Start(void);
void COAP_Destroy(void);
void COAP_LockMutex(void);
void COAP_UnlockMutex(void);
void COAP_UpdateAllSockSet(socket_set_t *set);
void COAP_ProcessAllSocketActivity(socket_set_t *set);
bool COAP_AreAllResponsesSent(void);
unsigned COAP_ParsePdu(unsigned char *buf, int len, parsed_pdu_t *pp);
int COAP_WriteRst(int message_id, unsigned char *token, int token_len, unsigned char *buf, int len);
int COAP_CalcBlockOption(unsigned char *buf, int cur_block, int is_more_blocks, int block_size);
unsigned char *COAP_WriteOption(pdu_option_t pdu_option, unsigned char *option_data, int len, unsigned char *buf, pdu_option_t *last_pdu_option);
void COAP_SetErrMessage(char *fmt, ...);
char *COAP_GetErrMessage(void);
int COAP_ReceivePdu(SSL *ssl, BIO *rbio, int socket_fd, unsigned char *buf, int buflen);
int COAP_SendPdu(SSL *ssl, BIO *wbio, int socket_fd, unsigned char *buf, int len);


#endif // ENABLE_COAP
#endif

