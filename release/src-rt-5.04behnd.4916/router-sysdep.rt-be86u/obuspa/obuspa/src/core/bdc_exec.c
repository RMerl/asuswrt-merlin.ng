/*
 *
 * Copyright (C) 2019-2021, Broadband Forum
 * Copyright (C) 2016-2021  CommScope, Inc
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
 * \file bdc_exec.c
 *
 * Main loop for thread which posts bulk data collection reports
 *
 */

#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <curl/curl.h>
#include <openssl/ssl.h>

#include "common_defs.h"
#include "bdc_exec.h"
#include "dm_exec.h"
#include "stomp.h"
#include "os_utils.h"
#include "device.h"
#include "rfc1123.h"

//------------------------------------------------------------------------------
// Structure containing the connection details for the specified profile
typedef struct
{
    int profile_id;     // Set to INVALID, if this slot is not in use
    CURL *curl_ctx;

    // Dynamically allocated memory that needs to be freed, when the report has been sent
    char *full_url;
    char *query_string;
    char *username;
    char *password;
    unsigned char *report;
    int report_len;
    unsigned flags;
    struct curl_slist *headers;
} bdc_connection_t;

static bdc_connection_t bdc_connection[BULKDATA_MAX_PROFILES];

//------------------------------------------------------------------------------
// Unix domain socket pair used to implement a message queue
// One socket is always used for sending, and the other always used for receiving
static int bdc_mq_sockets[2] = {-1, -1};

#define mq_rx_socket  bdc_mq_sockets[0]
#define mq_tx_socket  bdc_mq_sockets[1]

//-------------------------------------------------------------------------
// Enumeration of message types for BDC thread's message queue
typedef enum
{
    kBdcMsgType_SendReport,
    kBdcMsgType_ScheduleExit
} bdc_exec_msg_type_t;

//-------------------------------------------------------------------------
// Message enqueueing the sending of a bulk data collection report
// NOTE: All dynamically allocated buffers passed with this message, pass ownership to the BDC thread
typedef struct
{
    bdc_exec_msg_type_t msg_type; // Type of message
    int profile_id;          // Instance number of profile in Device.Bulkdata.Profile.{i}
    char *full_url;          // URL of the BDC server to post the report to
    char *query_string;      // HTTP query string, sent to the BDC server
    char *username;          // username for HTTP authentication
    char *password;          // password for HTTP authentication
    unsigned char *report;   // pointer to buffer containing the report (compressed or not)
    int report_len;          // length of buffer containing the report
    unsigned flags;          // bitmask of options for sending eg whether to use PUT instead of POST, whether the contents are Gzipped, whether to include
} bdc_exec_msg_t;

//------------------------------------------------------------------------------
// Boolean controlling whether curl logs BDC server comms to stdout+stderr
static bool show_curl_debug = false;

//------------------------------------------------------------------------------
// Curl multi-interface handle. Used to send multiple reports simultaneously
static CURLM *curl_multi_ctx;

// Number of curl easy interface handles that have been added to the curl multi-interface handle
static int num_transfers_in_progress = 0;

//------------------------------------------------------------------------------
// Flag to determine whether BDC thread should exit
static bool bdc_exit_scheduled = false;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void UpdateBdcSockSet(socket_set_t *set);
void ProcessBdcMessageQueueSocketActivity(socket_set_t *set);
int StartSendingReport(bdc_connection_t *bc);
void FreeBdcExecMsgContents(bdc_exec_msg_t *msg);
size_t bulkdata_curl_null_sink(void *buffer, size_t size, size_t nmemb, void *userp);
void PerformSendingReports(void);
void HandleBdcTransferComplete(CURL *curl_ctx, CURLcode curl_res);
bdc_transfer_result_t CalcBdcTransferResult(CURL *curl_ctx, CURLcode curl_res, int profile_id);
bdc_connection_t *FindFreeBdcConnection(void);
bdc_connection_t *FindBdcConnectionByCurlCtx(CURL *curl_ctx);
void FreeBdcConnection(bdc_connection_t *bc);
CURLcode LoadBulkDataTrustStore(CURL *curl, void *curl_sslctx, void *parm);

/*********************************************************************//**
**
** BDC_EXEC_Init
**
** Initialises the functionality in this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int BDC_EXEC_Init(void)
{
    int i;
    int err;
    bdc_connection_t *bc;

    // Initialise the bulk data reports array
    for (i=0; i<NUM_ELEM(bdc_connection); i++)
    {
        bc = &bdc_connection[i];
        memset(bc, 0, sizeof(bdc_connection_t));
        bc->profile_id = INVALID;
    }

    // Exit if unable to initialize the unix domain socket pair used to implement a message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, bdc_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** BDC_EXEC_PostReportToSend
**
** Posts a message to BDC Exec thread to cause it to send a BDC report to a BDC server
** NOTE: All dynamically allocated memory passed to this function as input arguments
**       changes to be owned by BDC Exec
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
** \param   full_url - URL of the BDC server to post the report to
** \param   query_string - HTTP query string, sent to the BDC server
** \param   username - username for HTTP authentication
** \param   password - password for HTTP authentication
** \param   report - pointer to buffer containing the report (compressed or not)
** \param   report_len - length of buffer containing the report
** \param   flags - bitmask of options for sending eg whether to use PUT instead of POST, whether the contents are Gzipped, whether to include
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int BDC_EXEC_PostReportToSend(int profile_id, char *full_url, char *query_string, char *username, char *password, unsigned char *report, int report_len, unsigned flags)
{
    bdc_exec_msg_t  msg;
    int bytes_sent;

    // Form message (do this first, so that we can free message contents if a failure occurs)
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = kBdcMsgType_SendReport;
    msg.profile_id = profile_id;
    msg.full_url = full_url;
    msg.query_string = query_string;
    msg.username = username;
    msg.password = password;
    msg.report = report;
    msg.report_len = report_len;
    msg.flags = flags;

    // Exit if message queue is not setup yet
    if (mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        FreeBdcExecMsgContents(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to send the message
    bytes_sent = send(mq_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );

        // Free all buffers whose ownership has not passed to BDC exec
        FreeBdcExecMsgContents(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Message was sent successfully
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** BDC_EXEC_ScheduleExit
**
** Posts a message to BDC Exec thread to cause it to exit
**
** \param   None
**
** \return  None
**
**************************************************************************/
void BDC_EXEC_ScheduleExit(void)
{
    bdc_exec_msg_t  msg;
    int bytes_sent;

    // Form message (do this first, so that we can free message contents if a failure occurs)
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = kBdcMsgType_ScheduleExit;

    // Exit if message queue is not setup yet
    if (mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Exit if unable to send the message
    bytes_sent = send(mq_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        return;
    }
}
/*********************************************************************//**
**
** BDC_EXEC_Main
**
** Main loop of the bulk data collection thread
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *BDC_EXEC_Main(void *args)
{
    int num_sockets;
    socket_set_t set;
    bdc_connection_t *bc;
    int i;

    // Exit if uable to create a curl multi-interface handle
    curl_multi_ctx = curl_multi_init();
    if (curl_multi_ctx == NULL)
    {
        USP_LOG_Error("%s: curl_multi_init() failed", __FUNCTION__);
        return NULL;
    }

    // Main loop which multiplexes the message queue with sending reports
    while(FOREVER)
    {
        // Create the socket set to receive/transmit on (with timeout)
        UpdateBdcSockSet(&set);

        // Wait for read/write activity on message queue or timeout
        num_sockets = SOCKET_SET_Select(&set);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting Data Model thread", __FUNCTION__);
                return NULL;
                break;

            case 0:
                // A timeout occurred - fall through

            default:
                // Start sending the report contained in the message (if received a message)
                ProcessBdcMessageQueueSocketActivity(&set);

                // Allow Libcurl to send the reports
                PerformSendingReports();
                break;
        }

        // Exit this thread, if an exit is scheduled
        // NOTE: Unlike the USP MTPs, bulk data collection does not wait for all reports to be sent
        if (bdc_exit_scheduled)
        {
            goto exit;
        }
    }

exit:
    // Free all BDC connections
    for (i=0; i<NUM_ELEM(bdc_connection); i++)
    {
        bc = &bdc_connection[i];
        if (bc->profile_id != INVALID)
        {
            FreeBdcConnection(bc);
        }
    }

    // Free curl context
    curl_multi_cleanup(curl_multi_ctx);

    // Signal the data model thread that this thread has exited
    DM_EXEC_PostMtpThreadExited(BDC_EXITED);

    return NULL;
}

/*********************************************************************//**
**
** UpdateBdcSockSet
**
** Adds all sockets to wait for activity on, into the socket set
** Also updates the associated timeout for activity
** This function must be called every time before the call to select(), as select alters the socket set
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void UpdateBdcSockSet(socket_set_t *set)
{
    CURLMcode res;
    long timeout;      // in ms

    // Start from no sockets in the set
    SOCKET_SET_Clear(set);

    // Skip curl sockets if unable to determine the socket sets that curl wants to select on
    res = curl_multi_fdset(curl_multi_ctx, &set->readfds, &set->writefds, &set->execfds, &set->numfds);
    if (res != CURLM_OK)
    {
        USP_LOG_Error("%s: curl_multi_fdset() failed (%s)", __FUNCTION__, curl_multi_strerror(res));
        goto exit;
    }

    // Skip curl sockets if unable to determine the timeout (in ms) that curl wants to use
    res = curl_multi_timeout(curl_multi_ctx, &timeout);
    if (res != CURLM_OK)
    {
        USP_LOG_Error("%s: curl_multi_fdset() failed (%s)", __FUNCTION__, curl_multi_strerror(res));
        goto exit;
    }

    // If given an infinite timeout, just change it to 1 hour
    if (timeout == -1)
    {
        timeout = MAX_SOCKET_TIMEOUT;
    }

    // Set the timeout that curl wants
    SOCKET_SET_UpdateTimeout(timeout, set);

exit:
    // Add the message queue receiving socket to the socket set
    SOCKET_SET_AddSocketToReceiveFrom(mq_rx_socket, MAX_SOCKET_TIMEOUT, set);
}

/*********************************************************************//**
**
** ProcessBdcMessageQueueSocketActivity
**
** Processes any activity on the message queue receiving socket
**
** \param   set - pointer to socket set structure containing sockets with activity on them
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void ProcessBdcMessageQueueSocketActivity(socket_set_t *set)
{
    int bytes_read;
    bdc_exec_msg_t  msg;
    bdc_connection_t *bc;
    int err;

    // Exit if there is no activity on the message queue socket
    if (SOCKET_SET_IsReadyToRead(mq_rx_socket, set) == 0)
    {
        return;
    }

    // Exit if unable to read the full message received
    bytes_read = recv(mq_rx_socket, &msg, sizeof(msg), 0);
    if (bytes_read != sizeof(msg))
    {
        USP_LOG_Error("%s: recv() did not return a full message", __FUNCTION__);
        return;
    }

    // Exit if this is a ScheduleExit message
    if (msg.msg_type == kBdcMsgType_ScheduleExit)
    {
        bdc_exit_scheduled = true;
        return;
    }

    // If the code gets here, it must be a SendReport message
    // Exit if unable to find a connection slot
    USP_ASSERT(msg.msg_type == kBdcMsgType_SendReport);
    bc = FindFreeBdcConnection();
    if (bc == NULL)
    {
        USP_LOG_Error("%s: Unable to find a free BDC connection slot", __FUNCTION__);
        FreeBdcExecMsgContents(&msg);
        return;
    }

    // Fill in the connection slot
    // Ownership of dynamically allocated buffers moves from the BdcExecMsg to the Bdc connection slot
    bc->profile_id = msg.profile_id;
    bc->curl_ctx = NULL;
    bc->full_url = msg.full_url;
    bc->query_string = msg.query_string;
    bc->username = msg.username;
    bc->password = msg.password;
    bc->report = msg.report;
    bc->report_len = msg.report_len;
    bc->flags = msg.flags;
    bc->headers = NULL;

    // Attempt to start sending the report
    err = StartSendingReport(bc);
    if (err != USP_ERR_OK)
    {
        // Free the Connection slot, if an error occurred
        FreeBdcConnection(bc);
    }
}

/*********************************************************************//**
**
**  StartSendingReport
**
**  Starts Sending the BDC report by adding it to the curl multi-interface handle
**
** \param   bc - pointer to BDC connection slot containing the report and associated parameters
**
** \return  USP_ERR_OK if successfully started sending the report
**
**************************************************************************/
int StartSendingReport(bdc_connection_t *bc)
{
    CURL *curl_ctx = NULL;
    CURLMcode res;
    char buf[48];
    int date_len;
    bool use_authentication;

    // Exit if unable to create a curl context
    curl_ctx = curl_easy_init();
    if (curl_ctx == NULL)
    {
        USP_LOG_Error("%s: curl_easy_init failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set options for PUT or POST
    curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDS, (char *)bc->report);
    curl_easy_setopt(curl_ctx, CURLOPT_POSTFIELDSIZE, bc->report_len);
    if (bc->flags & BDC_FLAG_PUT)
    {
        // Curl defaults to POST, set to PUT here
        curl_easy_setopt(curl_ctx, CURLOPT_CUSTOMREQUEST, "PUT");
    }

    // Decide whether to show CURL debugging information on stderr, and server's response body on stdout
    if (show_curl_debug)
    {
        curl_easy_setopt(curl_ctx, CURLOPT_VERBOSE, 1L);
    }
    else
    {
        // By default curl always writes the server's response body to stdout
        // As we don't normally want this, Override the default write function to null sink the data
        curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, bulkdata_curl_null_sink);
    }

    // Set URL
    curl_easy_setopt(curl_ctx, CURLOPT_URL, bc->full_url);

    // Set authentication (if required)
    use_authentication = ((bc->username[0] != '\0') && (bc->password[0] != '\0'));
    if (use_authentication)
    {
        curl_easy_setopt(curl_ctx, CURLOPT_USERNAME, bc->username);
        curl_easy_setopt(curl_ctx, CURLOPT_PASSWORD, bc->password);
        curl_easy_setopt(curl_ctx, CURLOPT_HTTPAUTH, BULKDATA_HTTP_AUTH_METHOD);
    }

    // Setup SSL options
    curl_easy_setopt(curl_ctx, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl_ctx, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl_ctx, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(curl_ctx, CURLOPT_CAPATH, NULL);
    curl_easy_setopt(curl_ctx, CURLOPT_SSL_CTX_FUNCTION, *LoadBulkDataTrustStore);
    curl_easy_setopt(curl_ctx, CURLOPT_CONNECTTIMEOUT, BULKDATA_CONNECT_TIMEOUT);
    curl_easy_setopt(curl_ctx, CURLOPT_TIMEOUT, BULKDATA_TOTAL_TIMEOUT);
    curl_easy_setopt(curl_ctx, CURLOPT_FORBID_REUSE, 1);

    // Set the list of headers
    bc->headers = NULL;
    bc->headers = curl_slist_append(bc->headers, "Content-Type: application/json; charset=UTF-8");
    bc->headers = curl_slist_append(bc->headers, "BBF-Report-Format: NameValuePair");
    if (bc->flags & BDC_FLAG_GZIP)
    {
        bc->headers = curl_slist_append(bc->headers, "Content-Encoding: gzip");
    }

    // lighttpd cannot handle expect headers, but curl implicitly adds one
    // To get rid of it, explicitly clear the Expect: header's value
    // For more info on this lighttpd/curl incompatability, see http://redmine.lighttpd.net/boards/2/topics/3598
    // and https://curl.haxx.se/docs/faq.html (section 4.16: My HTTP POST or PUT requests are slow!)
    bc->headers = curl_slist_append(bc->headers, "Expect:");

    // Set the date header (if required)
    if (bc->flags & BDC_FLAG_DATE_HEADER)
    {
        #define RFC1123_DATE_STR "Date: "
        strcpy(buf, RFC1123_DATE_STR);
        date_len = sizeof(RFC1123_DATE_STR) - 1;
        RFC1123_GetCurTime(&buf[date_len], sizeof(buf)-date_len);
        bc->headers = curl_slist_append(bc->headers, buf);
    }

    curl_easy_setopt(curl_ctx, CURLOPT_HTTPHEADER, bc->headers);

#if 0
//------------------------------------
// The following code may be uncommented to change the code to do an easy perform instead of a multi-perform
//CURLcode e = curl_easy_perform(curl_ctx);
//bdc_transfer_result_t transfer_result = CalcBdcTransferResult(curl_ctx, e, bc->profile_id);
//DM_EXEC_NotifyBdcTransferResult(bc->profile_id, transfer_result);
//FreeBdcConnection(bc);
//curl_easy_cleanup(curl_ctx);
//return USP_ERR_OK;
//------------------------------------
#endif

    // Exit if unable to add this easy handle into the multi handle
    res = curl_multi_add_handle(curl_multi_ctx, curl_ctx);
    if (res != CURLM_OK)
    {
        USP_LOG_Error("%s: curl_multi_add_handle() failed (%s)", __FUNCTION__, curl_multi_strerror(res));
        curl_slist_free_all(bc->headers);
        curl_easy_cleanup(curl_ctx);
        bc->headers = NULL;
        return USP_ERR_INTERNAL_ERROR;
    }

    // Update the count of easy handles in the multi-handle
    num_transfers_in_progress++;
    bc->curl_ctx = curl_ctx;

    // NOTE: We do not have to call PerformSendingReports() here, as it will be called in BDC_EXEC_Main() anyway

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PerformSendingReports
**
** This function allows libcurl to send the reports to the BDC servers
** It also reports back to the data model, when the sending of a report has finished
**
** \param   None
**
** \return  None
**
**************************************************************************/
void PerformSendingReports(void)
{
    CURLMcode res;
    CURLMsg *curl_msg;
    int running_handles;

    int msgs_in_queue;

    // Exit if an unrecoverable error occurred whilst the multi_perform was in progress. It will retry again next time
    res = curl_multi_perform(curl_multi_ctx, &running_handles);
    if (res != CURLM_OK)
    {
        USP_LOG_Error("%s: curl_multi_perform() failed (%s)", __FUNCTION__, curl_multi_strerror(res));
        return;
    }

    // Exit if none of the connections have finished
    USP_ASSERT(running_handles <= num_transfers_in_progress);
    msgs_in_queue = num_transfers_in_progress - running_handles;
    if (msgs_in_queue == 0)
    {
        return;
    }

    // If the code gets here, at least one of the connections have finished
    while (msgs_in_queue > 0)
    {
        curl_msg = curl_multi_info_read(curl_multi_ctx, &msgs_in_queue);
        if ((curl_msg != NULL) && (curl_msg->msg == CURLMSG_DONE))
        {
            HandleBdcTransferComplete(curl_msg->easy_handle, curl_msg->data.result);
        }
    }
}

/*********************************************************************//**
**
**  HandleBdcTransferComplete
**
**  Handles a transfer completing by cleaning up the curl easy handle, freeing memory and
**  notifying the DM_EXEC thread that the transfer has completed
**
** \param   curl_ctx - curl easy handle for transfer that completed
** \param   curl_res - curl result code for the transfer
**
** \return  None
**
**************************************************************************/
void HandleBdcTransferComplete(CURL *curl_ctx, CURLcode curl_res)
{
    bdc_transfer_result_t transfer_result;
    bdc_connection_t *bc;
    int profile_id;         // Save the profile_id, so that we can notify the data model at the end of this function

    // Exit if unable to find associated connection slot
    bc = FindBdcConnectionByCurlCtx(curl_ctx);
    if (bc == NULL)
    {
        return;
    }
    profile_id = bc->profile_id;

    // Determine whether the transfer was successful or not
    transfer_result = CalcBdcTransferResult(curl_ctx, curl_res, bc->profile_id);

    // Free all memory associated with this report
    FreeBdcConnection(bc);

    // Remove the curl easy handle that has completed from the multi-handle and free it
    curl_multi_remove_handle(curl_multi_ctx, curl_ctx);
    curl_easy_cleanup(curl_ctx);

    // Update the number of transfers in progress in the curl multi-handle
    num_transfers_in_progress--;
    USP_ASSERT(num_transfers_in_progress >= 0);

    // Finally, notify the data model about the result of the transfer
    DM_EXEC_NotifyBdcTransferResult(profile_id, transfer_result);
}

/*********************************************************************//**
**
**  CalcBdcTransferResult
**
**  Calculates the result code of the transfer - either success, or cause of failure
**
** \param   curl_ctx - curl easy handle for transfer that completed
** \param   curl_res - curl result code for the transfer
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
**
** \return  result code of the transfer
**
**************************************************************************/
bdc_transfer_result_t CalcBdcTransferResult(CURL *curl_ctx, CURLcode curl_res, int profile_id)
{
    CURLcode res;
    long lval;
    bdc_transfer_result_t transfer_result;

    switch(curl_res)
    {
        case CURLE_OK:
            transfer_result = kBDCTransferResult_Success;
            break;

        case CURLE_COULDNT_RESOLVE_PROXY:   // 5
        case CURLE_COULDNT_RESOLVE_HOST:    // 6
            transfer_result = kBDCTransferResult_Failure_DNS;
            break;

#if (LIBCURL_VERSION_NUM < 0x073E00)
        // In Curl version 7.62.0 onwards, CURLE_SSL_CACERT is the same as CURLE_PEER_FAILED_VERIFICATION, so don't include it twice
        case CURLE_SSL_CACERT:              // 60 - problem with the CA cert (path?)
#endif
        case CURLE_SSL_CONNECT_ERROR:       // 35 - wrong when connecting with SSL
        case CURLE_PEER_FAILED_VERIFICATION: // 51 - peer's certificate or fingerprint wasn't verified fine
        case CURLE_SSL_CERTPROBLEM:         // 58 - problem with the local certificate
        case CURLE_SSL_CIPHER:              // 59 - couldn't use specified cipher
            transfer_result =  kBDCTransferResult_Failure_Auth;
            break;

        case CURLE_COULDNT_CONNECT:         // 7
            transfer_result = kBDCTransferResult_Failure_Connect;
            break;

        case CURLE_SEND_ERROR:              // 55 - failed sending network data
        case CURLE_RECV_ERROR:              // 56 - failure in receiving network data
        case CURLE_AGAIN:                   // 81 - socket is not ready for send/recv, wait till it's ready and try again
            transfer_result =  kBDCTransferResult_Failure_ReadWrite;
            break;

        case CURLE_OPERATION_TIMEDOUT:      // 28 - the timeout time was reached
            transfer_result =  kBDCTransferResult_Failure_Timeout;
            break;

        default:
            transfer_result =  kBDCTransferResult_Failure_Other;
            break;
    }

    // Exit if an error occurred during the transfer
    if (curl_res != CURLE_OK)
    {
        USP_LOG_Error("BULK DATA: report sending failed for profile_id=%d (curl_res=%d, %s)", profile_id, curl_res, curl_easy_strerror(curl_res));
        return transfer_result;
    }

    // Check server response for success (status 2xx)
    // Both FTP and HTTP give response codes in the 200 range for successful completion
    res = curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &lval);
    if (res != CURLE_OK)
    {
        USP_LOG_Error("%s: could not retrieve CURLINFO_RESPONSE_CODE: %s", __FUNCTION__, curl_easy_strerror(curl_res));
        return kBDCTransferResult_Failure_Other;
    }
    USP_LOG_Info("BULK DATA: Server Response Code=%ld (%s) for profile_id=%d", lval, ((lval < 200) || (lval > 299)) ? "FAILURE" : "SUCCESS", profile_id);

    // Exit if upload failed
    if ((lval < 200) || (lval > 299))
    {
        USP_LOG_Warning("BULK DATA: upload failed with server response code %ld for profile_id=%d", lval, profile_id);
        transfer_result = (lval == 401) ? kBDCTransferResult_Failure_Auth :  kBDCTransferResult_Failure_Other;
        return transfer_result;
    }

    // If the code gets here, then the report was successfully sent
    return kBDCTransferResult_Success;
}

/*********************************************************************//**
**
**  bulkdata_curl_null_sink
**
**  Registered callback for curl write function
**  This function is called with the text response of the server
**  This function overrides libcurls default write function which dumps the response to stdout
**
** \param   buffer - pointer to buffer containing the data received from the server
** \param   size - size of an element of data
** \param   nmemb - number of elements of data
** \param   userp - pointer to user context (we do not register any)
**
** \return  number of bytes processed by this function
**
**************************************************************************/
size_t bulkdata_curl_null_sink(void *buffer, size_t size, size_t nmemb, void *userp)
{
    // Skip all bytes passed to this function
    return nmemb*size;
}

/*********************************************************************//**
**
**  FindFreeBdcConnection
**
**  Finds a free connection slot
**
** \param   None
**
** \return  pointer to connection slot, or NULL if no free slot found
**
**************************************************************************/
bdc_connection_t *FindFreeBdcConnection(void)
{
    int i;
    bdc_connection_t *bc;

    // Iterate over all connections
    for (i=0; i<NUM_ELEM(bdc_connection); i++)
    {
        // Exit if found a free slot
        bc = &bdc_connection[i];
        if (bc->profile_id == INVALID)
        {
            return bc;
        }
    }

    // If the code gets here, no free connection was found
    return NULL;
}

/*********************************************************************//**
**
**  FindBdcConnectionByCurlCtx
**
**  Finds the connection slot, given a curl easy handle
**
** \param   curl_ctx - curl easy handle to find in the connection array
**
** \return  pointer to connection slot, or NULL if no connection found
**
**************************************************************************/
bdc_connection_t *FindBdcConnectionByCurlCtx(CURL *curl_ctx)
{
    int i;
    bdc_connection_t *bc;

    // Iterate over all connections
    for (i=0; i<NUM_ELEM(bdc_connection); i++)
    {
        // Exit if found the matching connection
        bc = &bdc_connection[i];
        if (bc->curl_ctx == curl_ctx)
        {
            return bc;
        }
    }

    // If the code gets here, no connection was found
    return NULL;
}

/*********************************************************************//**
**
** FreeBdcConnection
**
** Frees up the specified connection slot, freeing up all dynamically allocated memory it holds
**
** \param   bc - pointer to BDC connection slot containing the report and associated parameters
**
** \return  None
**
**************************************************************************/
void FreeBdcConnection(bdc_connection_t *bc)
{
    // Free all memory whose ownership passed to this connection slot from the BDC exec message
    USP_SAFE_FREE(bc->full_url);
    USP_SAFE_FREE(bc->query_string);
    USP_SAFE_FREE(bc->username);
    USP_SAFE_FREE(bc->password);

    if (bc->report != NULL)
    {
        free(bc->report);
    }

    // Free curl headers
    if (bc->headers != NULL)
    {
        curl_slist_free_all(bc->headers);
    }

    // Mark the slot as unused
    memset(bc, 0, sizeof(bdc_connection_t));
    bc->profile_id = INVALID;
}

/*********************************************************************//**
**
** FreeBdcExecMsgContents
**
** Frees all dynamically allocated memory whose ownership passed to BDC Exec
**
** \param   msg - pointer to BDC Exec message, whose dynamically alloacted contents we want to free
**
** \return  None
**
**************************************************************************/
void FreeBdcExecMsgContents(bdc_exec_msg_t *msg)
{
    USP_SAFE_FREE(msg->full_url);
    USP_SAFE_FREE(msg->query_string);
    USP_SAFE_FREE(msg->username);
    USP_SAFE_FREE(msg->password);

    if (msg->report != NULL)
    {
        free(msg->report);
    }
}

/*********************************************************************//**
**
** LoadBulkDataTrustStore
**
** Callback from curl to install our trust store certificates into curl's SSL context
**
** \param   curl - pointer to curl context
** \param   curl_sslctx - pointer to curl's SSL context
** \param   userptr - pointer to user data (unused)
**
** \return  CURLE_OK if successful
**
**************************************************************************/
CURLcode LoadBulkDataTrustStore(CURL *curl, void *curl_sslctx, void *parm)
{
    SSL_CTX *curl_ssl_ctx;
    X509_STORE *curl_trust_store;
    int err;

    // Exit if unable to obtain curl's trust store
    curl_ssl_ctx = (SSL_CTX *) curl_sslctx;
    curl_trust_store = SSL_CTX_get_cert_store(curl_ssl_ctx);
    if (curl_trust_store == NULL)
    {
        USP_LOG_Error("%s: SSL_CTX_get_cert_store() failed", __FUNCTION__);
        return CURLE_ABORTED_BY_CALLBACK;
    }

    // Exit if unable to load the trust store and client cert into curl's SSL context
    err = DEVICE_SECURITY_LoadTrustStore(curl_ssl_ctx, SSL_VERIFY_PEER, DEVICE_SECURITY_NoSaveTrustCertVerifyCallback);
    if (err != USP_ERR_OK)
    {
        return CURLE_ABORTED_BY_CALLBACK;
    }

    // If the code gets here, then the trust certs and client certs were loaded successfully
    return CURLE_OK;
}

