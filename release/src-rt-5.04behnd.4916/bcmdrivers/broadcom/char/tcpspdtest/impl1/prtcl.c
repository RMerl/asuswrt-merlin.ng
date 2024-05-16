/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
*******************************************************************************
* File Name  : prtcl.c
*
* Description: This file contains the Broadcom Tcp Speed Test Protocols Implementation.
*
*  Created on: Dec 6, 2016
*      Author: yonatani, ilanb
*******************************************************************************
*/

#include <linux/inet.h>
#include <linux/string.h>
#include "tcpspdtest.h"
#include "sock_mgr.h"
#include "tcp_engine_api.h"
#include "genl_msgs.h"

/******************************************** Defines ********************************************/
/* HTTP defines */
#define HTTP_STR_LEN           256
#define HTTP_REQ_HEAD          "HEAD /%s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\n\r\n"
#define HTTP_REQ_GET           "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nAccept: */*\r\n\r\n"

#define HTTP_POST              "POST /%s HTTP/1.1\r\nHost: %pISpc\r\nConnection: keep-alive\r\n"\
                                    "Content-Type: application/x-binary\r\nContent-Length: %llu\r\nContent-Disposition: attachment; filename=\"file\"\r\n\r\n"
#define HTTP_RESP_PREFIX       "HTTP/1.1"
#define HTTP_RESP_OK           "HTTP/1.1 200 OK"
#define HTTP_RESP_CONTENT_LEN  "Content-Length: "
#define HTTP_RESP_LOCATION     "Location:"

#define HTTP_HANDLE_FIRST_RESP_TOKEN  5

/******************************************** Typess *********************************************/
typedef enum
{
    /* Download */
    HTTP_STATE_WAIT_HEAD_RESP,
    HTTP_STATE_WAIT_HEAD_RESP_ACK,
    HTTP_STATE_HEAD_RESP_DONE,
    HTTP_STATE_HEAD_RESP_ERR,

    /* Upload */
    HTTP_UP_STATE_WAIT_POST_RESP_ACK,
    HTTP_UP_STATE_DATA_TRANSFER_START,
    HTTP_UP_STATE_DATA_TRANSFER_DONE,
} http_state_t;


/**************************************** Global / Static ****************************************/
/* Static, Externs */
static atomic_t http_state[SPDT_NUM_OF_STREAMS];

static struct timeval _time_start[SPDT_NUM_OF_STREAMS];
static struct timeval _time_stop[SPDT_NUM_OF_STREAMS];
static struct timeval _time_http_head_req[SPDT_NUM_OF_STREAMS];
static struct timeval _time_http_head_resp[SPDT_NUM_OF_STREAMS];
static struct timeval _time_http_post_req[SPDT_NUM_OF_STREAMS];

/**************************************** Implementation *****************************************/

static void http_handle_response(uint8_t stream_idx, uint8_t *payload, uint16_t payload_len)
{
#define HTTP_RESP_CODE_OK 200
#define HTTP_RESP_CODE_REDIRECT_FIRST 300
#define HTTP_RESP_CODE_REDIRECT_LAST 399

    uint8_t payload_saved_last_byte, tmp_saved;
    char url_str[512];
    char *str_start = NULL, *str_end = NULL, *str_tmp = NULL;
    int sts_code = 0;

    if ((!payload || payload_len == 0))
        return;

    payload_saved_last_byte = payload[payload_len - 1];
    payload[payload_len - 1] = '\0';

    /* This function may be called during data transfer. In order to affect performance less,
      **  just scan http header by strnstr first. If there is no desired http header, exit.
      */
    str_tmp = strnstr(payload, HTTP_RESP_PREFIX, 9);
    if (!str_tmp)
        goto exit;

    /* get status code */
    str_tmp += strlen(HTTP_RESP_PREFIX);
    sscanf(str_tmp, "%d", &sts_code);
    tc_info("\n[%hhu] sts_code=%d\n", stream_idx, sts_code);

    if (sts_code == HTTP_RESP_CODE_OK)
        tc_debug("[%hhu] Rec :%s\n", stream_idx, HTTP_RESP_OK);
    else if((sts_code >= HTTP_RESP_CODE_REDIRECT_FIRST) && (sts_code <= HTTP_RESP_CODE_REDIRECT_LAST)) /* Redirects */
    {
         /* Just handle when engine is up, otherwise exit.
              **  If sends redirect msg into netlink queue too early(engine is down), 
              **  will disturb msg sequence in queue.
              */
        if (!tcpspd_engine_is_state_up(stream_idx))
            goto exit;

        str_start=strstr(payload, HTTP_RESP_LOCATION);
        if (!str_start)
        {
            tc_err("[%hhu] Invalid Redirect New URL: %s\n", stream_idx, payload);
            goto exit;
        }

        str_end = strstr(str_start, "\r\n");
        if (!str_end)
        {
            tc_err("[%hhu] Invalid Redirect New URL: %s\n", stream_idx, payload);
            goto exit;
        }

        str_start += strlen(HTTP_RESP_LOCATION);
        tmp_saved = *str_end;
        *str_end = '\0';
        strncpy(url_str, str_start, sizeof(url_str) - 1);
        *str_end = tmp_saved;
        tc_info("[%hhu] Redirect New URL: %s\n", stream_idx, url_str);

        /* skip first spaces */
        str_tmp = url_str;
        while (*str_tmp == ' ')
            str_tmp++;

        if (strlen(str_tmp) < SPDT_OTHER_STAT_MAX_LEN)
        {
            strncpy(g_spd_report[stream_idx].msg.other, str_tmp, SPDT_OTHER_STAT_MAX_LEN - 1);
            g_spd_report[stream_idx].status = TCPSPDTEST_GENL_CMD_STATUS_REDIRECT;
            tcpspd_genl_send_event(stream_idx, &(g_spd_report[stream_idx]), g_spd_report[stream_idx].status);
        }
        else
        {
            tc_err("[%hhu] Redirect New URL too long! <%s>\n", stream_idx, str_tmp);
        }
    }
    else
    {
        strncpy(g_spd_report[stream_idx].msg.other, payload, SPDT_OTHER_STAT_MAX_LEN - 1);
        g_spd_report[stream_idx].status = TCPSPDTEST_GENL_CMD_STATUS_OTHER;
        tcpspd_genl_send_event(stream_idx, &(g_spd_report[stream_idx]), g_spd_report[stream_idx].status);
    }

exit:
    payload[payload_len - 1] = payload_saved_last_byte;
}

static void send_report(uint8_t stream_idx, uint64_t bytes)
{
    uint32_t delay_ms;
    uint64_t rate_Mbps;
    struct rtnl_link_stats64 dev_stats;

    /* take time before validating */
    do_gettimeofday(&_time_stop[stream_idx]);
    delay_ms = (_time_stop[stream_idx].tv_usec - _time_start[stream_idx].tv_usec) / 1000;
    delay_ms += (_time_stop[stream_idx].tv_sec - _time_start[stream_idx].tv_sec) * 1000;

    /* calculation in two stages to prevent wrap around */
    rate_Mbps = bytes << 3;       /* bits */
    do_div(rate_Mbps, delay_ms);  /* bits/msec */
    do_div(rate_Mbps, 1000);      /* bits/usec */

    g_spd_report[stream_idx].rate = rate_Mbps;
    g_spd_report[stream_idx].num_bytes = bytes;
    g_spd_report[stream_idx].expected_bytes = tcpspd_engine_get_expected_bytes(stream_idx);
    g_spd_report[stream_idx].time_ms = delay_ms;

    /* With Time Base the stop can happen before the stream has started, bytes, expected _bytes may be 0. check for 0 < bytes */
    g_spd_report[stream_idx].status = ((bytes && bytes >= g_spd_report[stream_idx].expected_bytes) ? TCPSPDTEST_GENL_CMD_STATUS_OK : TCPSPDTEST_GENL_CMD_STATUS_INTERRUPTED);

    if (0 == tcpspd_get_iface_stats_by_stream(stream_idx, &dev_stats))
    {
        g_spd_report[stream_idx].total_rx_bytes = dev_stats.rx_bytes - g_spd_iface_stats_start[stream_idx].rx_bytes;
        g_spd_report[stream_idx].total_tx_bytes = dev_stats.tx_bytes - g_spd_iface_stats_start[stream_idx].tx_bytes;
    }

    if (g_tcpspd[stream_idx].end_of_test_spd_report)
        tcpspd_genl_send_speed_report_msg(stream_idx, &(g_spd_report[stream_idx]), g_spd_report[stream_idx].status);

    g_tcpspd[stream_idx].state = TCPSPD_STATE_DONE_DATA;

    tc_debug("[%hhu] Kernel Upload complete with %llu bytes at %u ms GoodPut=%u Mbps\n", stream_idx, bytes,
        delay_ms, (uint32_t)rate_Mbps);
}

/* HTTP, FTP download complete callback */
static void download_complete_cb(uint8_t stream_idx, void *data, uint16_t len)
{
    uint64_t bytes = *(uint64_t *)data;
    int32_t delay_ms;
    uint64_t rate_Mbps;
    struct rtnl_link_stats64 dev_stats;

    tcpspd_engine_down(stream_idx);

    if (TCPSPD_STATE_DONE_DATA == g_tcpspd[stream_idx].state)
        return; /* Second complete cb may happen with http. The resp ok for http get req is also counted as received bytes */

    do_gettimeofday(&_time_stop[stream_idx]);

    /* print last pending speed stats before engine down */
    tcpspd_engine_print_last_spdsts(stream_idx);

    delay_ms = (_time_stop[stream_idx].tv_usec - _time_start[stream_idx].tv_usec) / 1000;
    delay_ms += (_time_stop[stream_idx].tv_sec - _time_start[stream_idx].tv_sec) * 1000;

    /* do calculation in two stages to prevent wrap around */
    rate_Mbps = bytes << 3;      /* bits */
    do_div(rate_Mbps, delay_ms); /* bits/msec */
    do_div(rate_Mbps, 1000);     /* bits/usec */

    /* Prepare speed report */
    g_spd_report[stream_idx].rate = rate_Mbps;
    g_spd_report[stream_idx].num_bytes = bytes;
    g_spd_report[stream_idx].expected_bytes = tcpspd_engine_get_expected_bytes(stream_idx);
    g_spd_report[stream_idx].time_ms = delay_ms;

    if (g_tcpspd[stream_idx].mode == TCPSPD_MODE_HTTP)
    {
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_ROM_TIME].tv_sec = _time_http_head_req[stream_idx].tv_sec;
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_ROM_TIME].tv_usec = _time_http_head_req[stream_idx].tv_usec;
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_sec = _time_http_head_resp[stream_idx].tv_sec;
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_usec = _time_http_head_resp[stream_idx].tv_usec;
    }
    else
    {
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_sec = _time_start[stream_idx].tv_sec;
        g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_usec = _time_start[stream_idx].tv_usec;
    }
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_EOM_TIME].tv_sec = _time_stop[stream_idx].tv_sec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_EOM_TIME].tv_usec = _time_stop[stream_idx].tv_usec;

    /* With Time Base the stop can happen before the stream has started, bytes, expected _bytes may be 0. check for 0 < bytes */
    g_spd_report[stream_idx].status = ((bytes && bytes >= g_spd_report[stream_idx].expected_bytes) ? TCPSPDTEST_GENL_CMD_STATUS_OK : TCPSPDTEST_GENL_CMD_STATUS_INTERRUPTED);

    if (0 == tcpspd_get_iface_stats_by_stream(stream_idx, &dev_stats))
    {
        g_spd_report[stream_idx].total_rx_bytes = dev_stats.rx_bytes - g_spd_iface_stats_start[stream_idx].rx_bytes;
        g_spd_report[stream_idx].total_tx_bytes = dev_stats.tx_bytes - g_spd_iface_stats_start[stream_idx].tx_bytes;
    }

    if (g_tcpspd[stream_idx].end_of_test_spd_report)
        tcpspd_genl_send_speed_report_msg(stream_idx, &(g_spd_report[stream_idx]), g_spd_report[stream_idx].status);

    g_tcpspd[stream_idx].state = TCPSPD_STATE_DONE_DATA;

    tc_info("[%hhu] Kernel Download complete with %llu bytes at %u ms GoodPut=%u Mbps\n", stream_idx, bytes,
        delay_ms, (uint32_t)rate_Mbps);
}

/* HTTP upload/download response callback */
static void http_response_cb(uint8_t stream_idx, void *data, uint16_t len)
{
    if (data && (len != 0))
    {
        http_handle_response(stream_idx, (uint8_t *)data, len);
    }
}

/* FTP upload complete callback */
static void upload_complete_cb(uint8_t stream_idx, void *data, uint16_t len)
{
    uint64_t bytes = *(uint64_t *)data;

    send_report(stream_idx, bytes);

    tcpspd_engine_down(stream_idx);
}

/* HTTP upload complete callback */
static void http_upload_complete_cb(uint8_t stream_idx, void *data, uint16_t len)
{
    int32_t delay_ms;
    uint64_t rate_Mbps;
    uint64_t bytes = *(uint64_t *)data;
    struct rtnl_link_stats64 dev_stats;

    tcpspd_engine_down(stream_idx);

    if (TCPSPD_STATE_DONE_DATA == g_tcpspd[stream_idx].state)
        return; /* Second complete cb may happen with http. The resp ok for http get req is also counted as received bytes */

    do_gettimeofday(&_time_stop[stream_idx]);

    atomic_set(&http_state[stream_idx], HTTP_UP_STATE_DATA_TRANSFER_DONE);

    delay_ms = (_time_stop[stream_idx].tv_usec - _time_start[stream_idx].tv_usec) / 1000;
    delay_ms += (_time_stop[stream_idx].tv_sec - _time_start[stream_idx].tv_sec) * 1000;

    /* calculation in two stages to prevent wrap around */
    rate_Mbps = bytes << 3;       /* bits */
    do_div(rate_Mbps, delay_ms);  /* bits/msec */
    do_div(rate_Mbps, 1000);      /* bits/usec */

    g_spd_report[stream_idx].rate = rate_Mbps;
    g_spd_report[stream_idx].num_bytes = bytes;
    g_spd_report[stream_idx].expected_bytes = tcpspd_engine_get_expected_bytes(stream_idx);
    g_spd_report[stream_idx].time_ms = delay_ms;

    /* Prepare tr143 speed report */
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_ROM_TIME].tv_sec = _time_http_post_req[stream_idx].tv_sec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_ROM_TIME].tv_usec = _time_http_post_req[stream_idx].tv_usec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_sec = _time_start[stream_idx].tv_sec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_BOM_TIME].tv_usec = _time_start[stream_idx].tv_usec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_EOM_TIME].tv_sec = _time_stop[stream_idx].tv_sec;
    g_spd_report[stream_idx].msg.tr143_ts[SPDT_TR143_TS_REPORT_EOM_TIME].tv_usec = _time_stop[stream_idx].tv_usec;

    /* With Time Base the stop can happen before the stream has started, bytes, expected _bytes may be 0. check for 0 < bytes */
    g_spd_report[stream_idx].status = ((bytes && bytes >= g_spd_report[stream_idx].expected_bytes) ? TCPSPDTEST_GENL_CMD_STATUS_OK : TCPSPDTEST_GENL_CMD_STATUS_INTERRUPTED);

    if (0 == tcpspd_get_iface_stats_by_stream(stream_idx, &dev_stats))
    {
        g_spd_report[stream_idx].total_rx_bytes = dev_stats.rx_bytes - g_spd_iface_stats_start[stream_idx].rx_bytes;
        g_spd_report[stream_idx].total_tx_bytes = dev_stats.tx_bytes - g_spd_iface_stats_start[stream_idx].tx_bytes;
    }

    if (g_tcpspd[stream_idx].end_of_test_spd_report)
        tcpspd_genl_send_speed_report_msg(stream_idx, &(g_spd_report[stream_idx]), g_spd_report[stream_idx].status);

    g_tcpspd[stream_idx].state = TCPSPD_STATE_DONE_DATA;

    tc_info("[%hhu] Kernel Upload complete with %llu bytes at %u ms GoodPut=%u Mbps\n", stream_idx, bytes,
            delay_ms, (uint32_t)rate_Mbps);
}

/* Wait for HTTP head response.
   The return err_id may be used for retry.
*/
static int http_wait_head_response(uint8_t stream_idx)
{
#define WAIT_LOOP 100
#define WAIT_MSEC 5

    int cnt = WAIT_LOOP;
    int _http_state;

    do
    {
        _http_state = atomic_read(&http_state[stream_idx]);
        if (HTTP_STATE_HEAD_RESP_DONE == _http_state || HTTP_STATE_HEAD_RESP_ERR == _http_state)
            break;
        mdelay(WAIT_MSEC);
    }
    while (cnt--);

    if (cnt < 0)
        return -1;

    if (HTTP_STATE_HEAD_RESP_ERR == _http_state)
        return -2;

    return 0;
}

/* Hijacked packet for HTTP processing, get content length */
static int http_nf_hook(uint8_t stream_idx, struct sk_buff *skb, unsigned int hooknum)
{
    uint8_t *str_start, *str_end, *payload, payload_saved_last_byte;
    uint16_t payload_len;
    int nf_rc = NF_ACCEPT;
    struct tcphdr *tcph = tcp_hdr(skb);

    tc_debug("%s, [%hhu] prtcl_http_state:%d\n", __func__, stream_idx, atomic_read(&http_state[stream_idx]));

    switch (atomic_read(&http_state[stream_idx]))
    {
    case HTTP_STATE_WAIT_HEAD_RESP:
        if (NF_INET_LOCAL_OUT == hooknum)
        {
            if (0 == _time_http_head_req[stream_idx].tv_usec)
                do_gettimeofday(&_time_http_head_req[stream_idx]);
            else
                tc_err("[%hhu] Bad HTTP Head Request Time!\n", stream_idx);
            break;
        }

        payload_len = skb->len - skb_network_header_len(skb) - (tcph->doff << 2);

        /* skip the ack for HTTP HEAD REQ */
        if (0 == payload_len)
            break;

        payload = (uint8_t *)tcph + (tcph->doff << 2);
        payload_saved_last_byte = payload[payload_len - 1];
        payload[payload_len - 1] = '\0';
        tc_debug("[%hhu] http payload: %s\n", stream_idx, payload);
        if (strstr(payload, HTTP_RESP_OK))
        {
            /* #ib#todo#: check with scanf */
            str_start = strstr(payload, HTTP_RESP_CONTENT_LEN);
            str_end = NULL;
            if (str_start)
                str_end = strstr(str_start, "\r\n");
            if (str_start && str_end)
            {
                uint64_t bytes = 0;
                str_start += strlen(HTTP_RESP_CONTENT_LEN);
                *str_end = '\0';
                if (kstrtou64(str_start, 10, &bytes))
                {
                    *str_end = '\r';
                    payload[payload_len - 1] = payload_saved_last_byte;
                    tc_err("[%hhu] Failed to parse %s as number of bytes\n", stream_idx, str_start);
                    break;
                }
                tcpspd_engine_set_expected_bytes(stream_idx, bytes);
                do_gettimeofday(&_time_http_head_resp[stream_idx]);
                tc_debug("[%hhu] Head response context length: %s, expected: %lld\n", stream_idx, str_start, bytes); //#ib#todo#: server send more bytes than the expected. check with download complete.
                *str_end = '\r';
                atomic_set(&http_state[stream_idx], HTTP_STATE_WAIT_HEAD_RESP_ACK);
            }
            else
                tc_err("Failed to find: '%s' in Http response\n", HTTP_RESP_CONTENT_LEN);
        }
        else if (strstr(payload, HTTP_RESP_LOCATION)) /* Redirects */
        {
            /* Note: don't call http_handle_response to handle redirect HTTP response here.
                    ** Because, if http_handle_response sends redirect msg into netlink queue too early,
                    ** will disturb msg sequence in queue.
                    */
            do_gettimeofday(&_time_http_head_resp[stream_idx]);
            atomic_set(&http_state[stream_idx], HTTP_STATE_HEAD_RESP_DONE); 
        }
        else
        {
            /* Should be: "HTTP/1.1 404 Not Found" or "HTTP/1.1 400 Bad Request" */
            atomic_set(&http_state[stream_idx], HTTP_STATE_HEAD_RESP_ERR);
        }
        payload[payload_len - 1] = payload_saved_last_byte;
        break;

        /* Wait for linux tx ack to head resp before sending http get request */
    case HTTP_STATE_WAIT_HEAD_RESP_ACK:
        if (NF_INET_LOCAL_IN == hooknum)
            break;

        if (!tcph->ack)
        {
            tc_err("[%hhu] Error http state: %d, Ack expected!\n", stream_idx, atomic_read(&http_state[stream_idx]));
            break;
        }

        /* Drop the ack for server head resp, the next http get req will ack it anyway.
           avoid ack packet tx delay (by data packets) in cpu qm queue. */
        nf_rc = NF_DROP;

        atomic_set(&http_state[stream_idx], HTTP_STATE_HEAD_RESP_DONE);
        break;
    }

    return nf_rc;
}

/* HTTP download request */
int tcpspd_prtcl_http_download_request(uint8_t stream_idx, unsigned char *host_str, unsigned char *filename)
{
    uint8_t reqstr[HTTP_STR_LEN];
    int rc, len;
    uint64_t bytes;

    if (HTTP_STR_LEN <= (strlen(HTTP_REQ_HEAD) +  strlen(host_str) + strlen(filename) + INET6_ADDRSTRLEN))
    {
        tc_err("[%hhu] Http Head request too long! Max:%d\n", stream_idx, HTTP_STR_LEN);
        return -1;
    }

    len = sprintf(reqstr, HTTP_REQ_HEAD, filename, host_str);

    _time_http_head_req[stream_idx].tv_usec = 0;

    atomic_set(&http_state[stream_idx], HTTP_STATE_WAIT_HEAD_RESP);

    /* Start hijack nf_hook out/in packets for prtcl processing */
    tcpspd_sock_mgr_set_nf_hook_prtcl(stream_idx, 1);

    /* Send head req */
    if (tcpspd_sock_mgr_sendmsg(stream_idx, reqstr, len))
    {
        tc_err("[%hhu] Failed to send DOWNLOAD HEAD request to http server\n", stream_idx);
        goto head_req_fail;
    }

    /* Wait send to be acked before runner flow is activated */
    if (tcpspd_sock_mgr_wait_send_complete(stream_idx, g_tcpspd[stream_idx].srv_socket))
    {
        tc_err("[%hhu] Failed wait http head send to be completed\n", stream_idx);
        goto head_req_fail;
    }

    /* Wait for http head resp */
    rc = http_wait_head_response(stream_idx);
    if (rc)
    {
        tc_err("[%hhu] Failed wait http head response. rc:%d\n", stream_idx, rc);
        goto head_req_fail;
    }

    bytes = tcpspd_engine_get_expected_bytes(stream_idx);
    /* Stop hijack out/in packets by prtcl */
    tcpspd_sock_mgr_set_nf_hook_prtcl(stream_idx, 0);

    /* Prepare HTTP get req */
    len = sprintf(reqstr, HTTP_REQ_GET, filename, host_str);

    g_tcpspd[stream_idx].action = SPDT_DIR_RX;

    tcpspd_engine_init_tcb(stream_idx, 0, http_response_cb, download_complete_cb);
    tcpspd_engine_set_expected_bytes(stream_idx, bytes);

    if (STREAM_IS_HWACCEL(stream_idx))
    {
        rc = tcpspd_engine_init_rnr_download(stream_idx, len, 0);
        if (rc)
        {
            tc_err("[%hhu] Failed to init runner download, rc:%d\n", stream_idx, rc);
            return -1;
        } 
    }
   
    /* Add runner flow by tcp_engine. separate modules. prtcl should access rnr */
    rc = tcpspd_engine_cpu_flow_add(g_tcpspd[stream_idx].srv_socket, stream_idx);
    if (rc)
    {
        tc_err("[%hhu] Failed to add runner flow to TCP Engine, rc:%d\n", stream_idx, rc);

        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_download(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        return -1;
    }

    tcpspd_engine_set_state_up(stream_idx);

    /* Url redirection is only supported for driver mode. Block firmware mode Url redirection that happens when driver download error correction code is activated */
    if (!STREAM_IS_HWACCEL(stream_idx))
        tcpspd_engine_set_resp_handle_token(stream_idx, HTTP_HANDLE_FIRST_RESP_TOKEN);

    tcpspd_get_iface_stats_by_stream(stream_idx, &(g_spd_iface_stats_start[stream_idx]));
    do_gettimeofday(&_time_start[stream_idx]);

    /* Send HTTP get req */
    return tcpspd_engine_send_data(stream_idx, reqstr, len);

head_req_fail:
    tcpspd_sock_mgr_set_nf_hook_prtcl(stream_idx, 0);
    return -1;
}

/* HTTP upload request */
int tcpspd_prtcl_http_upload_request(uint8_t stream_idx, unsigned char *uri, uint64_t upload_size)
{
    uint8_t reqstr[HTTP_STR_LEN];
    int rc, len;

    len = snprintf(reqstr, HTTP_STR_LEN, HTTP_POST, uri, (struct sockaddr *)&g_tcpspd[stream_idx].stream_params.conn_params.server_addr, upload_size);

    atomic_set(&http_state[stream_idx], HTTP_UP_STATE_WAIT_POST_RESP_ACK);

    do_gettimeofday(&_time_http_post_req[stream_idx]);

    /* Send post request */
    if (tcpspd_engine_send_data(stream_idx, reqstr, len))
    {
        tc_err("[%hhu] Failed to send UPLOAD POST request to http server. http state: %d\n", stream_idx, atomic_read(&http_state[stream_idx]));
        rc = -1;
        goto err;
    }

    /* Wait send to be acked before runner flow is activated */
    if (tcpspd_sock_mgr_wait_send_complete(stream_idx, g_tcpspd[stream_idx].srv_socket))
    {
        tc_err("[%hhu] Failed wait http post send to be completed. http state: %d\n", stream_idx, atomic_read(&http_state[stream_idx]));
        rc = -1;
        goto err;
    }

    g_tcpspd[stream_idx].action = SPDT_DIR_TX;
    
    tcpspd_engine_init_tcb(stream_idx, 1, http_response_cb, http_upload_complete_cb);

    /* set upload size */
    tcpspd_engine_set_expected_bytes(stream_idx, upload_size);

    if (STREAM_IS_HWACCEL(stream_idx))
    {
        rc = tcpspd_engine_init_rnr_upload(stream_idx);
        /* Firmware tx BNs are allocated on the first stream only and only if rc OK */
        if (rc)
        {
            tc_err("[%hhu] Failed to init runner upload, rc:%d\n", stream_idx, rc);
            return -1;
        }
    }

    rc = tcpspd_engine_init_rto_sack(stream_idx);
    if (rc)
        return -1;
    
    /* Add runner flow by tcp_engine. separate modules. prtcl should access rnr */
    rc = tcpspd_engine_cpu_flow_add(g_tcpspd[stream_idx].srv_socket, stream_idx);
    if (rc)
    {
        tc_err("[%hhu] Failed to add runner flow to TCP Engine, rc:%d\n", stream_idx, rc);
        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_upload(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        tcpspd_engine_uninit_rto_sack(stream_idx);
        return -1;
    }

    tcpspd_engine_set_state_up(stream_idx);

    /* Url redirection is only supported for driver mode. Block firmware mode Url redirection that may happen if driver upload error correction code will be used */
    if (!STREAM_IS_HWACCEL(stream_idx))
        tcpspd_engine_set_resp_handle_token(stream_idx, HTTP_HANDLE_FIRST_RESP_TOKEN);

    atomic_set(&http_state[stream_idx], HTTP_UP_STATE_DATA_TRANSFER_START);

    tcpspd_get_iface_stats_by_stream(stream_idx, &(g_spd_iface_stats_start[stream_idx]));
    do_gettimeofday(&_time_start[stream_idx]);

    tc_debug("[%hhu] HTTP start upload_size: %llu\n", stream_idx, upload_size);

    /* On sucess, Update number of streams and start transmitting the actual data */
    rc = tcpspd_engine_start_tx(stream_idx, upload_size);
    if (rc)
    {
        tc_err("[%hhu] Failed to start tx, rc:%u\n", stream_idx, rc);
        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_upload(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        tcpspd_engine_uninit_rto_sack(stream_idx);        
    }

err:
    return rc;
}

/* FTP download request */
int tcpspd_prtcl_ftp_download_request(uint8_t stream_idx, uint64_t download_size)
{
    int rc;

    g_tcpspd[stream_idx].action = SPDT_DIR_RX;

    tcpspd_engine_init_tcb(stream_idx, 0, NULL, download_complete_cb);
    tcpspd_engine_set_expected_bytes(stream_idx, download_size);

    if (STREAM_IS_HWACCEL(stream_idx))
    {
        rc = tcpspd_engine_init_rnr_download(stream_idx, 0, 0);
        if (rc)
        {
            tc_err("[%hhu] Failed to init runner download, rc:%d\n", stream_idx, rc);
            return -1;
        }
    }

    /* Add runner flow by tcp_engine. separate modules. prtcl should access rnr */
    rc = tcpspd_engine_cpu_flow_add(g_tcpspd[stream_idx].srv_socket, stream_idx);
    if (rc)
    {
        tc_err("[%hhu] Failed to add runner flow to TCP Engine, rc:%d\n", stream_idx, rc);

        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_download(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        return -1;
    }

    tcpspd_engine_set_state_up(stream_idx);

    tcpspd_get_iface_stats_by_stream(stream_idx, &(g_spd_iface_stats_start[stream_idx]));
    do_gettimeofday(&_time_start[stream_idx]);

    return 0;
}

/* FTP upload request */
int tcpspd_prtcl_ftp_upload_request(uint8_t stream_idx, uint64_t upload_size)
{
    int rc;

    g_tcpspd[stream_idx].action = SPDT_DIR_TX;

    tcpspd_engine_init_tcb(stream_idx, 1, NULL, upload_complete_cb);

    /* set upload size */
    tcpspd_engine_set_expected_bytes(stream_idx, upload_size);
    tc_debug("[%hhu] upload_size: %llu\n", stream_idx, upload_size);

    if (STREAM_IS_HWACCEL(stream_idx))
    {
        rc = tcpspd_engine_init_rnr_upload(stream_idx);
        if (rc)
        {
            tc_err("Failed to init runner upload, rc:%d\n", rc);
            return -1;
        }
    }

    rc = tcpspd_engine_init_rto_sack(stream_idx);
    if (rc)
        return -1;

    /* Add runner flow by tcp_engine. separate modules. prtcl should access rnr */
    rc = tcpspd_engine_cpu_flow_add(g_tcpspd[stream_idx].srv_socket, stream_idx);
    if (rc)
    {
        tc_err("[%hhu] Failed to add runner flow to TCP Engine, rc:%d\n", stream_idx, rc);
        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_upload(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        tcpspd_engine_uninit_rto_sack(stream_idx);
        return -1;
    }

    tcpspd_engine_set_state_up(stream_idx);

    tcpspd_get_iface_stats_by_stream(stream_idx, &(g_spd_iface_stats_start[stream_idx]));
    do_gettimeofday(&_time_start[stream_idx]);

    /* Start transmitting the actual data */
    rc = tcpspd_engine_start_tx(stream_idx, upload_size);
    if (rc)
    {
        tc_err("[%hhu] Failed to start tx, rc:%u\n", stream_idx, rc);
        if (STREAM_IS_HWACCEL(stream_idx))
            tcpspd_engine_uninit_rnr_upload(stream_idx); /* Free allocated firmware tx BNs if it's the first stream */

        tcpspd_engine_uninit_rto_sack(stream_idx);
    }

    return 0;
}

/* Hijacked Socket packet for prtcl processing */
int tcpspd_prtcl_nf_hook(uint8_t stream_idx, struct sk_buff *skb, unsigned int hooknum)
{
    int rc;

    switch (g_tcpspd[stream_idx].mode)
    {
    case TCPSPD_MODE_HTTP:
        rc = http_nf_hook(stream_idx, skb, hooknum);
        break;
    case TCPSPD_MODE_FTP:
        rc = 0;
        break;    
    default:
        tc_err("[%hhu] Wrong protocol:%d hijacked packet\n", stream_idx, g_tcpspd[stream_idx].mode);
        rc = 1;
    }

    return rc;
}
