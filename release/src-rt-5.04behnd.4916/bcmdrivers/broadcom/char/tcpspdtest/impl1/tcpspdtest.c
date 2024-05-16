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
* File Name  : tcpspdtest.c
*
* Description: This file contains the Broadcom Tcp Speed Test Implementation.
*
*  Created on: Dec 6, 2016
*      Author: yonatani, ilanb, Nikolai Iosifov
*******************************************************************************
*/

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/init.h>
#include <linux/inet.h>
#include <linux/semaphore.h>
#include "bdmf_system.h"
#include <bcm_pkt_lengths.h>
#include "proc_cmd.h"
#include "tcpspdtest.h"
#include "sock_mgr.h"
#include "tcp_engine_api.h"
#include "udp_engine_api.h"
#include "genl_msgs.h"

/******************************************** Defines ********************************************/
#define DRIVER_AUTHOR "Yoni Itah <yoni.itah@broadcom.net>"
#define DRIVER_DESC   "Tcp Termination driver for HTTP/FTP/IPERF3 Speedtest"

#define PROC_DIR      "driver/tcpspdtest"
#define CMD_PROC_FILE "cmd"

#define TCP_ENGINE_CWND_INITIAL_MIN_MSS   1         /* Minimum configure allowed value for cwnd initial value in mss */
#define TCP_ENGINE_CWND_INITIAL_MAX_MSS   4         /* Maximum configure allowed value for cwnd initial value in mss */
#define TCP_ENGINE_CWND_MAX_MIN_MSS       (1<<6)    /* Minimum configure allowed value for cwnd_max in mss */
#define TCP_ENGINE_CWND_MAX_MSS           (1<<14)   /* Defualt value for cwnd_max in mss */

/**************************************** Global / Static  ***************************************/
tcpspd_t g_tcpspd[SPDT_NUM_OF_STREAMS];

uint8_t g_stream_alloc[SPDT_NUM_OF_STREAMS];
uint8_t num_allocated_streams;
static spinlock_t stream_lock;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

#ifdef CONFIG_BCM_XRDP
int rnr_engine = 1;
#else
int rnr_engine = 0;
#endif

static tcp_engine_config_t engine_config = {
#if defined(CONFIG_BCM96856) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    .rto_list_len = 10000,
#else
    .rto_list_len = 5000,
#endif
    .pktbuf_size = BCM_PKTBUF_SIZE
};

/* The following parameters are used for Runner mode offload*/       
static tcp_engine_conn_config_t conn_config_hw = {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    .cwnd_max_mss = 64,
#elif defined(CONFIG_BCM96855) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM963158)
    .cwnd_max_mss = 256,
#else
    .cwnd_max_mss = 16384,
#endif

    .cwnd_initial_mss = 2,

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    .cwnd_initial_thr_mss = 64,
#elif defined(CONFIG_BCM96855) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM963158)
    .cwnd_initial_thr_mss = 128,
#else
    .cwnd_initial_thr_mss = 1024,
#endif
};

/* The following parameters are used for driver (host) mode offload*/       
static tcp_engine_conn_config_t conn_config_sw = {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    .cwnd_max_mss = 256,
#else
    .cwnd_max_mss = 16384,
#endif
    .cwnd_initial_mss = 2,
    .cwnd_initial_thr_mss = 1024,
};

tcp_spdt_rep_t g_spd_report[SPDT_NUM_OF_STREAMS] = {};
struct rtnl_link_stats64 g_spd_iface_stats_start[SPDT_NUM_OF_STREAMS] = {};

/**************************************** Implementation *****************************************/
static int parse_ip_addr(const char *str, struct sockaddr_storage *addr)
{
    if (!strchr(str, ':') && in4_pton(str, -1, (u8 *)&((struct sockaddr_in *)addr)->sin_addr.s_addr, -1, NULL) > 0)
    {
        addr->ss_family = AF_INET;
        return AF_INET;
    }

    if (in6_pton(str, -1, (u8 *)&((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr, -1, NULL) > 0)
    {
        addr->ss_family = AF_INET6;
        return AF_INET6;
    }
    return -1;
}

int tcpspd_cmd_connect(uint8_t stream_idx, spdt_stream_dir_t dir, tcpspd_mode_t mode, spdt_conn_params_t *params)
{
    struct inet_sock *inet;
    int rc;

    /* Allow new "connect" after "release" for procfs. */
    if (TCPSPD_STATE_ALLOCATED != g_tcpspd[stream_idx].state && TCPSPD_STATE_RELEASED != g_tcpspd[stream_idx].state)
    {
        tc_err("Connect cmd during wrong state: %d\n", g_tcpspd[stream_idx].state);
        return -1;
    }

    g_tcpspd[stream_idx].mode = mode;
    g_tcpspd[stream_idx].hw_accel = (1 == params->force_swoffl_mode ? 0 : blog_get_hw_accel());
    g_tcpspd[stream_idx].action = dir;
    memcpy(&g_tcpspd[stream_idx].stream_params.conn_params, params, sizeof(g_tcpspd[stream_idx].stream_params.conn_params));

    if (g_tcpspd[stream_idx].stream_params.conn_params.if_name[0])
        g_tcpspd[stream_idx].netdev = dev_get_by_name(&init_net, g_tcpspd[stream_idx].stream_params.conn_params.if_name );

    rc = tcpspd_sock_mgr_connect(stream_idx, &g_tcpspd[stream_idx].srv_socket, params);
    if (rc)
    {
        tc_err("[%hhu] Connection failed.\n", stream_idx);
        return rc;
    }

    tcpspd_engine_conn_config_set(stream_idx, STREAM_IS_HWACCEL(stream_idx) ? &conn_config_hw : &conn_config_sw);
    g_tcpspd[stream_idx].state = TCPSPD_STATE_CONNECTED;
    g_tcpspd[stream_idx].end_of_test_spd_report = (g_tcpspd[stream_idx].stream_params.protocol == SPDT_HTTP || g_tcpspd[stream_idx].stream_params.protocol == SPDT_FTP) ? 1 : 0;
    tc_debug("connect protocol:%d , end_of_test_spd_report:%d\n", g_tcpspd[stream_idx].stream_params.protocol, g_tcpspd[stream_idx].end_of_test_spd_report);

    inet = inet_sk(g_tcpspd[stream_idx].srv_socket->sk);
    tc_info("[%hhu] Connected to Server:%pISpc, Local port:%hu\n", stream_idx, &params->server_addr, ntohs(inet->inet_sport));

    return rc;
}

int tcpspd_cmd_oob_send(uint8_t stream_idx, char *buff, uint64_t length)
{
    int rc;
    if (!g_tcpspd[stream_idx].srv_socket)
    {
        tc_err("[%hhu] Socket not Created\n", stream_idx);
        return -1;
    }

    if (SS_CONNECTED != g_tcpspd[stream_idx].srv_socket->state)
    {
        tc_err("[%hhu] Socket state not Connected. state:%d\n", stream_idx, g_tcpspd[stream_idx].srv_socket->state);
        return -1;
    }

    if (STREAM_IS_HWACCEL(stream_idx) && g_tcpspd[stream_idx].action == SPDT_DIR_RX)
    {
        rc = tcpspd_engine_init_rnr_download(stream_idx, length, 1);
        if (rc)
        {
            tc_err("[%hhu] Failed to init runner download, rc:%d\n", stream_idx, rc);
            return -1;
        }
    }

    rc = tcpspd_engine_send_data(stream_idx, buff, length);
    if (rc)
    {
        pr_err("[%hhu] tcpspd_cmd_oob_send failed.\n", stream_idx);
        return -1;
    }

    if (g_tcpspd[stream_idx].action != SPDT_DIR_RX)
    {
        /* wait send to be acked before runner flow is activated */
        if (tcpspd_sock_mgr_wait_send_complete(stream_idx, g_tcpspd[stream_idx].srv_socket))
        {
            tc_err("[%hhu] Failed wait send to be completed\n", stream_idx);
            return -1;
        }
    }

    return rc;
}

int tcpspd_cmd_download(uint8_t stream_idx, char *host_str, char *filename, uint64_t length)
{
    int rc = 0;

    if (TCPSPD_STATE_CONNECTED != g_tcpspd[stream_idx].state)
    {
        tc_err("Download cmd during wrong state: %d\n", g_tcpspd[stream_idx].state);
        return -1;
    }

    if (g_tcpspd[stream_idx].action != SPDT_DIR_RX)
    {
        tc_err("[%hhu] Stream isn't connected as Download\n", stream_idx);
        return -1;
    }

    if (!g_tcpspd[stream_idx].srv_socket)
    {
        tc_err("[%hhu] Socket not Created\n", stream_idx);
        return -1;
    }

    if (SS_CONNECTED != g_tcpspd[stream_idx].srv_socket->state)
    {
        tc_err("[%hhu] Socket state not Connected. state:%d\n", stream_idx, g_tcpspd[stream_idx].srv_socket->state);
        return -1;
    }

    switch (g_tcpspd[stream_idx].mode)
    {
        case TCPSPD_MODE_HTTP:
            rc = tcpspd_prtcl_http_download_request(stream_idx, host_str, filename);
            break;

        case TCPSPD_MODE_FTP:
            rc = tcpspd_prtcl_ftp_download_request(stream_idx, length);
            break;

        default:
            rc = -1;
            tc_err("[%hhu] Not handled download mode: %d\n", stream_idx, g_tcpspd[stream_idx].mode);
            break;
    }

    if (!rc)
        g_tcpspd[stream_idx].state = TCPSPD_STATE_RUNNING;

    return rc;
}

int tcpspd_cmd_upload(uint8_t stream_idx, char *uri, uint64_t length)
{
    int rc = 0;

    if (TCPSPD_STATE_CONNECTED != g_tcpspd[stream_idx].state)
    {
        tc_err("Upload cmd during wrong state: %d\n", g_tcpspd[stream_idx].state);
        return -1;
    }

    if (g_tcpspd[stream_idx].action != SPDT_DIR_TX)
    {
        tc_err("[%hhu] Stream isn't connected as Upload\n", stream_idx);
        return -1;
    }

    if (!g_tcpspd[stream_idx].srv_socket)
    {
        tc_err("[%hhu] Socket not Created\n", stream_idx);
        return -1;
    }

    if (SS_CONNECTED != g_tcpspd[stream_idx].srv_socket->state)
    {
        tc_err("[%hhu] Socket state not Connected. state:%d\n", stream_idx, g_tcpspd[stream_idx].srv_socket->state);
        return -1;
    }

    switch (g_tcpspd[stream_idx].mode)
    {
        case TCPSPD_MODE_HTTP:
            rc = tcpspd_prtcl_http_upload_request(stream_idx, uri, length);
            break;

        case TCPSPD_MODE_FTP:
            rc = tcpspd_prtcl_ftp_upload_request(stream_idx, length);
            break;

        default:
            rc = -1;
            tc_err("[%hhu] Not handled upload mode: %d\n", stream_idx, g_tcpspd[stream_idx].mode);
            break;
    }

    if (!rc)
        g_tcpspd[stream_idx].state = TCPSPD_STATE_RUNNING;

    return rc;
}

int tcpspd_cmd_disconnect(uint8_t stream_idx)
{
    int rc = 0;

    rc = tcpspd_engine_disconnect(stream_idx);
    rc |= tcpspd_genl_clear_event(stream_idx);
    rc |= tcpspd_sock_mgr_disconnect(stream_idx, g_tcpspd[stream_idx].srv_socket);

    if (!rc || rc == -ENOTCONN) /* WAR: new JIRA will be opened to check why rc=-107 */
        g_tcpspd[stream_idx].state = TCPSPD_STATE_DISCONNECTED;
    else
        tc_err("[%hhu] Failed to disconnect socket. rc=%d\n", stream_idx, rc);

    if (g_tcpspd[stream_idx].netdev)
        dev_put(g_tcpspd[stream_idx].netdev);

    g_tcpspd[stream_idx].netdev = 0;

    return rc;
}

int tcpspd_cmd_release(uint8_t stream_idx)
{
    int rc;

    if (TCPSPD_STATE_DISCONNECTED != g_tcpspd[stream_idx].state)
    {
        tc_debug("[%hhu]Release cmd during wrong state:%d\n", stream_idx, g_tcpspd[stream_idx].state);
        return -1;
    }

    rc = tcpspd_sock_mgr_release(stream_idx, g_tcpspd[stream_idx].srv_socket);

    if (!rc)
        g_tcpspd[stream_idx].state = TCPSPD_STATE_RELEASED;

    return rc;
}

int tcpspd_cmd_stream_idx(uint8_t *stream_idx, uint8_t *num_streams, tcpspdtest_genl_cmd_param_t cmd_param)
{
    int i;
    int rc = 0;

    /* lock between kthreads  */
    spin_lock(&stream_lock);

    /* Allocate new stream_idx */
    if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_ALLOC)
    {
        for (i = 0; i < SPDT_NUM_OF_STREAMS; i++)
        {
            if (!g_stream_alloc[i])
                break;
        }

        if (SPDT_NUM_OF_STREAMS == i)
        {
            tc_err("Error ! no free stream_idx, num allocated streams:%u\n", num_allocated_streams);
            rc = -1;
            goto exit;
        }
        *stream_idx = i;
        g_tcpspd[*stream_idx].state = TCPSPD_STATE_ALLOCATED;
        num_allocated_streams++;
        g_stream_alloc[*stream_idx] = 1;
        tc_debug("Allocate stream_idx:%u\n", *stream_idx);
    }
    /* Free stream_idx */
    else if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_FREE)
    {
        if (*stream_idx >= SPDT_NUM_OF_STREAMS)
        {
            tc_err("Error ! Not valid stream_idx:%u\n", *stream_idx);
            return -1;
        }
        if (!g_stream_alloc[*stream_idx])
        {
            tc_err("Error ! free non allocated stream_idx:%u, num allocated streams:%u\n", *stream_idx, num_allocated_streams);
            rc = -1;
            goto exit;
        }
        g_tcpspd[*stream_idx].state = TCPSPD_STATE_FREE;
        num_allocated_streams--;
        g_stream_alloc[*stream_idx] = 0;
        tc_debug("Free stream_idx:%u\n", *stream_idx);
    }
    else
    {
        tc_err("Bad cmd_param:%u\n", cmd_param);
        rc = -1;
    }

exit:
    *num_streams = num_allocated_streams;
    spin_unlock(&stream_lock);

    return rc;
}

int tcpspd_cmd_protocol(uint8_t stream_idx, spdt_proto_t *protocol, tcpspdtest_genl_cmd_param_t cmd_param)
{
    if (stream_idx >= SPDT_NUM_OF_STREAMS || !g_stream_alloc[stream_idx])
    {
        tc_err("Error ! Not valid stream_idx:%u\n", stream_idx);
        return -1;
    }

    /* Set stream protocol */
    if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_SET)
    {
        g_tcpspd[stream_idx].stream_params.protocol = *protocol;
    }
    /* Get stream protocol */
    else if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_GET)
    {
        *protocol = g_tcpspd[stream_idx].stream_params.protocol;
    }
    else
    {
        tc_err("Bad cmd_param:%u\n", cmd_param);
        return -1;
    }

    return 0;
}

int tcpspd_cmd_stream_params(uint8_t stream_idx, spdt_stream_params_t *req_stream_params, spdt_stream_params_t *stream_params, tcpspdtest_genl_cmd_param_t cmd_param)
{
    if (stream_idx >= SPDT_NUM_OF_STREAMS || !g_stream_alloc[stream_idx])
    {
        tc_err("Error ! Not valid stream_idx:%u\n", stream_idx);
        return -1;
    }

    if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_SET)
    {
        memcpy(&g_tcpspd[stream_idx].stream_params, req_stream_params, sizeof(g_tcpspd[stream_idx].stream_params));

        if (req_stream_params->protocol == SPDT_IPERF3_UDP || req_stream_params->protocol == SPDT_UDP_BASIC)
        {
            if (req_stream_params->sock_fd > 0)
            {
                if (g_tcpspd[stream_idx].stream_params.dir == SPDT_DIR_TX && !g_tcpspd[stream_idx].sk)
                {
                    int err;
                    struct socket *stream_sock;

                    stream_sock = sockfd_lookup(req_stream_params->sock_fd, &err);
                    if (!stream_sock)
                        return -1;

                    g_tcpspd[stream_idx].sk = stream_sock->sk;
                    sockfd_put(stream_sock);
                }
            }
            else
            {
                g_tcpspd[stream_idx].sk = NULL;
            }
        }
    }
    else if (cmd_param == TCPSPDTEST_GENL_CMD_PARAM_GET)
    {
        memcpy(stream_params, &g_tcpspd[stream_idx].stream_params, sizeof(*stream_params));
    }
    else
        return -1;

    return 0;    
}

static inline int is_spdt_proto_udp(spdt_proto_t proto)
{
    return proto == SPDT_UDP_BASIC || proto == SPDT_IPERF3_UDP;
}

int tcpspd_cmd_num_streams(uint8_t stream_idx, uint8_t *num_streams, uint8_t *num_udp_streams)
{
    int i;
    uint8_t udp_streams = 0;

    if (stream_idx >= SPDT_NUM_OF_STREAMS)
    {
        tc_err("Error ! Not valid stream_idx:%u\n", stream_idx);
        return -1;
    }

    for (i = 0; i < SPDT_NUM_OF_STREAMS; i++)
        if (g_stream_alloc[i] && is_spdt_proto_udp(g_tcpspd[i].stream_params.protocol))
            udp_streams++;
    
    *num_udp_streams = udp_streams;
    *num_streams = num_allocated_streams;

    tc_debug("Num allocated streams:%u, udp_streams:%u\n", *num_streams, *num_udp_streams);

    return 0;
}

int tcpspd_cmd_speed_report_blocking(uint8_t stream_idx)
{
    if (TCPSPD_STATE_RUNNING != g_tcpspd[stream_idx].state && TCPSPD_STATE_RELEASED != g_tcpspd[stream_idx].state)
    {
        tc_err("[%hhu]Try to set speed report blocking in wrong state: %u\n", stream_idx, g_tcpspd[stream_idx].state);
        return -1;
    }

    g_tcpspd[stream_idx].end_of_test_spd_report = 1;

    return 0;
}

static int tcpspd_proc_cmd_alloc_stream(int argc, char *argv[])
{
    int rc;
    uint8_t stream_idx, num_streams;

    if (argc != 1)
    {
        tc_err("Usage: echo " TCPSPD_CMD_ALLOC_STREAM_STR "\n");
        return -1;
    }

    rc = tcpspd_cmd_stream_idx(&stream_idx, &num_streams, TCPSPDTEST_GENL_CMD_PARAM_ALLOC);
    if (!rc)
    {
        tc_info("You were allocated a new stream_idx %u. There are %u allocated streams, Max is %u.\n",
            stream_idx, num_streams, SPDT_NUM_OF_STREAMS);
        tc_info("Do not forget to free your stream_idx when done with it.\n");
    }

    return rc;
}

static int tcpspd_proc_cmd_free_stream(int argc, char *argv[])
{
    int rc;
    uint8_t stream_idx, num_streams;

    if (argc != 2)
    {
        goto free_usage;
    }

    if (kstrtou8(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream_idx\n", argv[1]);
        goto free_usage;
    }

    if (stream_idx >= SPDT_NUM_OF_STREAMS)
        goto free_usage;

    rc = tcpspd_cmd_stream_idx(&stream_idx, &num_streams, TCPSPDTEST_GENL_CMD_PARAM_FREE);
    if (!rc)
        tc_info("Your stream_idx %u was freed. There are %u allocated streams now, Max is %u.\n",
            stream_idx, num_streams, SPDT_NUM_OF_STREAMS);

    return rc;

free_usage:
    tc_err("Usage: echo " TCPSPD_CMD_FREE_STREAM_STR " stream_idx[0 - %u]\n", SPDT_NUM_OF_STREAMS-1);
    return -1;
}

/* Proc Connect command */
static int tcpspd_proc_cmd_connect(int argc, char *argv[])
{
    tcpspd_mode_t mode;
    spdt_stream_dir_t action;
    spdt_conn_params_t params = {};
    uint16_t port, local_port = 0, stream_idx = 0;
    int rc;

    if (argc < 5 || argc > 8)
        goto usage;

    pr_info("TCPSPDTST: Connecting... to %s %s mode\n", argv[3], argv[2]);

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        return -1;
    }
    else
    {
        if (stream_idx >= SPDT_NUM_OF_STREAMS)
            goto usage; 
    } 

    /* Get ptotocol mode */
    if (!strcmp(argv[2], TCPSPD_PROTOCOL_HTTP_STR))
        mode = TCPSPD_MODE_HTTP;
    else if (!strcmp(argv[2], TCPSPD_PROTOCOL_FTP_STR))
        mode = TCPSPD_MODE_FTP;
    else
        goto usage;

    /* Get ptotocol action */
    if (!strcmp(argv[3], TCPSPD_CMD_DOWNLOAD_STR))
        action = SPDT_DIR_RX;
    else if (!strcmp(argv[3], TCPSPD_CMD_UPLOAD_STR))
        action = SPDT_DIR_TX;
    else
        goto usage;

    /* Get server ip addr */
    rc = parse_ip_addr(argv[4], &params.server_addr);
    if (rc < 0)
    {
        tc_err("can't parse server ip %s\n", argv[4]);
        goto usage;
    }

    /* Get port */
    if (argc >= 6)
    {
        int got_port = 1;
        if (kstrtou16(argv[5], 10, &port))
        {
            got_port = 0;
            rc = parse_ip_addr(argv[5], &params.local_addr);
            if (rc < 0)
            {
                tc_err("can't parse %s as port number or IP address\n", argv[5]);
                return -1;
            }
            port = mode;
        }
        if (argc == 7)
        {
            if (got_port)
            {
                rc = parse_ip_addr(argv[6], &params.local_addr);
                if (rc < 0)
                {
                    tc_err("can't parse %s as IP address\n", argv[6]);
                    return -1;
                }
            }
            else
            {
                if (kstrtou16(argv[6], 10, &local_port))
                {
                    tc_err("can't parse %s as port number\n", argv[6]);
                    return -1;
                }
            }
        }
        if (argc == 8)
        {
            rc = parse_ip_addr(argv[6], &params.local_addr);
            if (rc < 0)
            {
                tc_err("can't parse %s as IP address\n", argv[6]);
                return -1;
            }
            if (kstrtou16(argv[7], 10, &local_port))
            {
                tc_err("can't parse %s as port number\n", argv[7]);
                return -1;
            }
        }
    }
    else
        port = mode;

    if (params.server_addr.ss_family == AF_INET)
    {
        ((struct sockaddr_in *)&params.server_addr)->sin_port = htons(port);
        ((struct sockaddr_in *)&params.local_addr)->sin_port = htons(local_port);
    }
    else
    {
        ((struct sockaddr_in6 *)&params.server_addr)->sin6_port = htons(port);
        ((struct sockaddr_in6 *)&params.local_addr)->sin6_port = htons(local_port);
    }

    /* Socket Connect to Server */
    if (tcpspd_cmd_connect(stream_idx, action, mode, &params))
    {
        tc_err("Failed to connect to server %s:%hu\n", argv[4], port);
        return -1;
    }

    pr_info("Server connected %s:%hu\n", argv[4], port);

    return 0;

usage:
    tc_err("Usage: echo connect stream_idx[0 - %u] mode[http|ftp] action[download|upload] server_ip [port] [local_ip [local_port]]\n", SPDT_NUM_OF_STREAMS-1);
    return -1;
}

/* Proc Download command */
static int tcpspd_proc_cmd_download(int argc, char *argv[])
{
    uint64_t dnld_bytes = 0;
    char *filename = NULL;
    char *host_str = NULL;
    uint16_t stream_idx = 0;

    if (argc < 3)
    {
        tc_err("Usage: echo %s  <stream idx> <number of bytes>\n", TCPSPD_CMD_DOWNLOAD_STR);
        return -1;
    }

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        return -1;
    }

    switch (g_tcpspd[stream_idx].mode)
    {
        case TCPSPD_MODE_HTTP:
            pr_info("Requesting HTTP DOWNLOAD %s\n", argv[2]);
            host_str = argv[2];
            filename = argv[3];
            break;

    case TCPSPD_MODE_FTP:
        /* FTP download num bytes */
        if (kstrtou64(argv[2], 10, &dnld_bytes))
        {
            pr_err("Failed to parse %s as number of bytes\n", argv[2]);
            return -1;
        }
        pr_info("Requesting DOWNLOAD %llu\n", dnld_bytes);
        break;

    default:
        tc_err("unsupported download mode: %d\n", g_tcpspd[stream_idx].mode);
        return -1;
    }

    return tcpspd_cmd_download(stream_idx, host_str, filename, dnld_bytes);
}

/* Proc Upload command */
static int tcpspd_proc_cmd_upload(int argc, char *argv[])
{
    uint64_t upload_bytes = 0;
    char *uri = NULL;
    uint16_t stream_idx = 0;

    if (argc < 4)
    {
        tc_err("Usage: echo %s <stream idx> <number of bytes>\n", TCPSPD_CMD_UPLOAD_STR);
        return -1;
    }

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        return -1;
    }

    switch (g_tcpspd[stream_idx].mode)
    {
    case TCPSPD_MODE_HTTP:
        /* HTTP Upload */
        uri = argv[2];
        if (kstrtou64(argv[3], 10, &upload_bytes))
        {
            tc_err("Failed to parse %s as number of bytes\n", argv[3]);
            return -1;
        }
        tc_info("Requesting HTTP UPLOAD %s size %llu\n", uri, upload_bytes);

        break;

    case TCPSPD_MODE_FTP:
        /* FTP Upload */
        if (kstrtou64(argv[2], 10, &upload_bytes))
        {
            tc_err("Failed to parse %s as number of bytes\n", argv[2]);
            return -1;
        }
        tc_info("Requesting UPLOAD %llu\n", upload_bytes);
        break;
    default:
        tc_err("unsupported upload mode: %d\n", g_tcpspd[stream_idx].mode);
        return -1;
    }
    return tcpspd_cmd_upload(stream_idx, uri, upload_bytes);
}

/* Proc Disconnect command */
static int tcpspd_proc_cmd_disconnect(int argc, char *argv[])
{
    uint16_t stream_idx = 0;

    if (argc < 2)
    {
        tc_err("Usage: echo %s <stream idx>\n", TCPSPD_CMD_DISCONNECT_STR);
        return -1;
    }

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        return -1;
    }

    return tcpspd_cmd_disconnect(stream_idx);
}

/* Proc Release command */
static int tcpspd_proc_cmd_release(int argc, char *argv[])
{
    uint16_t stream_idx = 0;

    if (argc < 2)
    {
        tc_err("Usage: echo %s <stream idx>\n", TCPSPD_CMD_RELEASE_STR);
        return -1;
    }

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        return -1;
    }

    return tcpspd_cmd_release(stream_idx);
}

/* Proc print Statistics command */
static int tcspd_proc_cmd_stats(int argc, char *argv[])
{
    int rc = 0;

    if (argc < 2)
    {
        tcpspd_engine_print_stats();
    }
    else if ((argc >= 2) && (argc < 5))
    {
        uint8_t spdsts_enabled = 0;
        uint8_t spdsts_total = 1;
        uint32_t spdsts_interval = 100;

        if (kstrtou8(argv[1], 10, &spdsts_enabled))
        {
            tc_err("can't parse %s as enable\n", argv[1]);
            rc = -1;
            goto stats_err;
        }

        if (spdsts_enabled)
        {
            if (argc < 4)
            {
                tc_err("Wrong num of argc:%u\n", argc);
                rc = -1;
                goto stats_err;
            }

            if (kstrtou8(argv[2], 10, &spdsts_total))
            {
                tc_err("can't parse %s as total\n", argv[2]);
                rc = -1;
                goto stats_err;
            }
            
            if(kstrtou32(argv[3], 10, &spdsts_interval))
            {
                tc_err("Failed to parse %s as interval\n", argv[3]);
                rc = -1;
                goto stats_err;
            }
        }   

        rc = tcpspd_engine_set_spdsts_print(spdsts_enabled, spdsts_total, spdsts_interval);
        if (rc)
        {
            tc_err("Failed to set spdsts print\n");
            return rc;
        }
    }
    else
    {
        tc_err("Wrong num of argc: %u\n", argc);
        rc = -1;
        goto stats_err;
    }
        
    return rc;

 stats_err:
    tc_err("Usage: echo %s [ <enable[0|1]> <total[0|1]-enable only> <interval_ms[100-5000]-enable only> ]\n",
        TCPSPD_CMD_STATS_STR);

    return rc;
}

static int tcspd_proc_cmd_rwnd(int argc, char *argv[])
{
    uint32_t rate_Mbps = 0;
    uint32_t rwnd_bytes = 0;
    uint16_t stream_idx = 0;
    int rc = 0;

    if (argc < 4)
    {
        tc_err("Usage: echo rwnd <stream_idx> <rwnd bytes> <rate Mbps> \n");
        rc = -1;
        goto rwnd_exit;
    }

    if (kstrtou16(argv[1], 10, &stream_idx))
    {
        tc_err("can't parse %s as stream idx\n", argv[1]);
        rc = -1;
        goto rwnd_exit;
    }
    
    if (stream_idx >= SPDT_NUM_OF_STREAMS)
    {
        tc_err("Not valid stream idx:[%hhu]\n", stream_idx);
        rc = -1;
        goto rwnd_exit;
    }

    if (kstrtou32(argv[2], 10, &rwnd_bytes))
    {
        tc_err("Failed to parse %s as rwnd bytes\n", argv[2]);
        rc = -1;
        goto rwnd_exit;
    }

    if (kstrtou32(argv[3], 10, &rate_Mbps))
    {
        tc_err("Failed to parse %s as rate Mbps\n", argv[3]);
        rc = -1;
        goto rwnd_exit;
    }

    g_tcpspd[stream_idx].rwnd_bytes = rwnd_bytes;
    g_tcpspd[stream_idx].rate_Mbps = rate_Mbps;

rwnd_exit:
    tc_info("Current rate_Mbps: %u, rwnd_bytes: %u\n", g_tcpspd[stream_idx].rate_Mbps, g_tcpspd[stream_idx].rwnd_bytes)

    return rc;
}

static int tcspd_proc_cmd_cwnd(int argc, char *argv[])
{
    uint16_t cwnd_initial_mss;
    uint16_t cwnd_initial_thr_mss;
    uint16_t cwnd_max_mss;
    int rc = 0;
    tcp_engine_conn_config_t *conn_config = NULL;

    if (argc < 4)
    {
        rc = -1;
        goto cwnd_err;
    }

    if (kstrtou16(argv[1], 10, &cwnd_initial_mss))
    {
        tc_err("Failed to parse %s as cwnd initial(mss)\n", argv[1]);
        rc = -1;
        goto cwnd_err;
    }

    if (kstrtou16(argv[2], 10, &cwnd_initial_thr_mss))
    {
        tc_err("Failed to parse %s as cwnd initial threshold(mss)\n", argv[1]);
        rc = -1;
        goto cwnd_err;
    }

    if (kstrtou16(argv[3], 10, &cwnd_max_mss))
    {
        tc_err("Failed to parse %s as cwnd_max(mss)\n", argv[2]);
        rc = -1;
        goto cwnd_err;
    }

    if (!strcmp(argv[4], "hw"))
    {
#ifdef CONFIG_BCM_XRDP
        conn_config = &conn_config_hw;
#else
        tc_err("Hardware mode is not supported on your platform\n");
        return -1;
#endif
    }
    else if (!strcmp(argv[3], "sw"))
        conn_config = &conn_config_sw;
    else
        goto cwnd_err;

    if (cwnd_initial_mss < TCP_ENGINE_CWND_INITIAL_MIN_MSS || cwnd_initial_mss > TCP_ENGINE_CWND_INITIAL_MAX_MSS)
    {
        tc_err("Fault cwnd initial value (num of mss):%u. Allowed values:%u-%u\n", cwnd_initial_mss, TCP_ENGINE_CWND_INITIAL_MIN_MSS, TCP_ENGINE_CWND_INITIAL_MAX_MSS);
        return -1;
    }
    if (cwnd_max_mss < TCP_ENGINE_CWND_MAX_MIN_MSS || cwnd_max_mss > TCP_ENGINE_CWND_MAX_MSS)
    {
        tc_err("Fault cwnd_max value (num of mss):%u. Allowed values:%u-%u\n", cwnd_max_mss, TCP_ENGINE_CWND_MAX_MIN_MSS, TCP_ENGINE_CWND_MAX_MSS);
        return -1;
    }

    conn_config->cwnd_initial_mss = cwnd_initial_mss;
    conn_config->cwnd_initial_thr_mss = cwnd_initial_thr_mss;
    conn_config->cwnd_max_mss = cwnd_max_mss;

    tc_info("Successfully set %s cwnd initial(mss):%u, cwnd initial_thr(mss):%u, cwnd_max(mss):%u\n", argv[3], cwnd_initial_mss, cwnd_initial_thr_mss, cwnd_max_mss);
    return 0;

cwnd_err:
    tc_err("Usage: echo cwnd <cwnd initial(mss)> <cwnd initial_thr(mss)> <cwnd_max(mss)> <\"hw\"|\"sw\">\n");
    tc_info("Currect runner configuration: cwnd initial(mss):%u, cwnd initial_thr(mss):%u, cwnd_max(mss):%u\n", conn_config_hw.cwnd_initial_mss, conn_config_hw.cwnd_initial_thr_mss, conn_config_hw.cwnd_max_mss);
    return rc;
}

static int tcspd_proc_cmd_rto(int argc, char *argv[])
{
    uint32_t initial_rto_usec;
    uint16_t rto_ratio;
    uint8_t is_dynamic_rto_time;
    int rc = 0;

    if (argc < 4)
    {
        rc = -1;
        goto rto_err;
    }

    if (kstrtou32(argv[1], 10, &initial_rto_usec))
    {
        tc_err("Failed to parse %s as initial_rto(usec)\n", argv[1]);
        rc = -1;
        goto rto_err;
    }

    if (kstrtou16(argv[2], 10, &rto_ratio))
    {
        tc_err("Failed to parse %s as rto_ratio\n", argv[2]);
        rc = -1;
        goto rto_err;
    }

    if (kstrtou8(argv[3], 10, &is_dynamic_rto_time) || 1 < is_dynamic_rto_time)
    {
        tc_err("Failed to parse %s as is_dynamic_rto_time. Allowed values:0|1\n", argv[3]);
        rc = -1;
        goto rto_err;
    }

    rc = tcpspd_engine_set_rto(initial_rto_usec, rto_ratio, is_dynamic_rto_time);
    if (rc)
        goto rto_err;

    tc_info("Successfully set initial_rto(usec):%u, rto_ratio:%u, is_dynmaic_rto_time:%u\n", initial_rto_usec, rto_ratio, is_dynamic_rto_time);
    return 0;

rto_err:
    tc_err("Usage: echo rto <initial_rto(usec)> <rto_ratio> <is_dynamic_rto_time>\n");
    return rc;
}

static int tcspd_proc_cmd_up_ratelimit(int argc, char *argv[])
{
    int rc = 0;
    uint8_t is_ratelimit_enabled;
    uint32_t rate_Mbps = 0;

    if (argc < 2)
    {
        tc_err("Wrong num of argc:%u\n", argc);
        rc = -1;
        goto ratelimit_err;
    }

    if (kstrtou8(argv[1], 10, &is_ratelimit_enabled))
    {
        tc_err("can't parse %s as enable\n", argv[1]);
        rc = -1;
        goto ratelimit_err;
    }

    if (is_ratelimit_enabled)
    {
        if (argc < 3)
        {
            tc_err("Wrong num of argc:%u\n", argc);
            rc = -1;
            goto ratelimit_err;
        }
        
        if(kstrtou32(argv[2], 10, &rate_Mbps))
        {
            tc_err("Failed to parse %s as rate_Mbps\n", argv[2]);
            rc = -1;
            goto ratelimit_err;
        }
    }
    else if (argc > 2)
    {
        tc_err("Wrong num of argc:%u\n", argc);
        rc = -1;
        goto ratelimit_err;
    }

    rc = tcpspd_engine_set_bucket_ratelimit(is_ratelimit_enabled, rate_Mbps);
    if (rc)
        goto ratelimit_err;

    tc_info("Bucket rate limit:%s, rate_Mbps:%u\n", is_ratelimit_enabled ? "ENABLED" : "DISABLED", rate_Mbps);
    return rc;

ratelimit_err:
    tc_err("Usage: echo %s <enable[0|1]> <rate_Mbps[10-10000]-enable only>\n", TCPSPD_CMD_UP_RATELIMIT_STR);

    return rc;
}

static int tcspd_proc_cmd_dynack(int argc, char *argv[])
{
    int rc = 0;
    uint8_t thr_upd_rate;

    if (argc != 2)
    {
        tc_err("Wrong num of argc:%u\n", argc);
        rc = -1;
        goto dynack_err;
    }

    if (kstrtou8(argv[1], 10, &thr_upd_rate))
    {
        tc_err("can't parse %s as dynack_thr_upd_rate\n", argv[1]);
        rc = -1;
        goto dynack_err;
    }

    rc = tcpspd_engine_set_dynack(thr_upd_rate);
    if (rc)
        goto dynack_err;

    tc_info("Successfully set dynack_thr_upd_rate:%u\n", thr_upd_rate);
    return rc;

dynack_err:
    tc_err("Usage: echo %s <thr_upd_rate[0-%u] 0-Disable>\n", TCPSPD_CMD_DYNACK_STR, TCP_ENGINE_DYNACK_THR_UPD_RATE_MAX);

    return rc;
}

static int tcspd_proc_cmd_show_cfg(int argc, char *argv[])
{  
    tcpspd_engine_show_global_settings();
    tc_info("Software mode cwnd:\t\t\tcwnd_initial(mss):%u, initial_thr:%u, cwnd_max(mss):%u\n", conn_config_sw.cwnd_initial_mss, conn_config_sw.cwnd_initial_thr_mss, conn_config_sw.cwnd_max_mss);
#ifdef CONFIG_BCM_XRDP
    tc_info("Hardware mode cwnd:\t\t\tcwnd_initial(mss):%u, initial_thr:%u, cwnd_max(mss):%u\n", conn_config_hw.cwnd_initial_mss, conn_config_hw.cwnd_initial_thr_mss, conn_config_hw.cwnd_max_mss);
#endif

    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = TCPSPD_CMD_ALLOC_STREAM_STR, .do_command = tcpspd_proc_cmd_alloc_stream},
    { .name = TCPSPD_CMD_FREE_STREAM_STR, .do_command = tcpspd_proc_cmd_free_stream},
    { .name = TCPSPD_CMD_CONNECT_STR, .do_command = tcpspd_proc_cmd_connect},
    { .name = TCPSPD_CMD_DISCONNECT_STR, .do_command = tcpspd_proc_cmd_disconnect},
    { .name = TCPSPD_CMD_RELEASE_STR, .do_command = tcpspd_proc_cmd_release},
    { .name = TCPSPD_CMD_DOWNLOAD_STR, .do_command = tcpspd_proc_cmd_download},
    { .name = TCPSPD_CMD_UPLOAD_STR, .do_command = tcpspd_proc_cmd_upload},
    { .name = TCPSPD_CMD_STATS_STR, .do_command = tcspd_proc_cmd_stats},
    { .name = TCPSPD_CMD_RWND_STR, .do_command = tcspd_proc_cmd_rwnd},
    { .name = TCPSPD_CMD_CWND_STR, .do_command = tcspd_proc_cmd_cwnd},
    { .name = TCPSPD_CMD_RTO_STR, .do_command = tcspd_proc_cmd_rto},
    { .name = TCPSPD_CMD_UP_RATELIMIT_STR, .do_command = tcspd_proc_cmd_up_ratelimit},
    { .name = TCPSPD_CMD_DYNACK_STR, .do_command = tcspd_proc_cmd_dynack},
    { .name = TCPSPD_CMD_SHOW_CFG_STR, .do_command = tcspd_proc_cmd_show_cfg},
};

static struct proc_cmd_table tcpspd_command_table = {
    .module_name = "tcpspdtest",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};

/* Proc Init */
static int __init tcpspdtest_proc_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
    {
        tc_err("Failed to create PROC directory %s.\n", PROC_DIR);
        return -1;
    }

    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir, &tcpspd_command_table);
    if (!cmd_proc_file)
    {
        tc_err("Failed to create %s\n", CMD_PROC_FILE);
        return -1;
    }

    return 0;
}

int tcpspd_get_iface_stats_by_stream(uint8_t stream_idx, struct rtnl_link_stats64 *dev_stats)
{
    int rc = 0;

    if (0 == g_tcpspd[stream_idx].netdev)
        return -1;

    dev_get_stats(g_tcpspd[stream_idx].netdev, dev_stats);

    return rc;
}

/* TCPSPDTEST Module Init */
static int __init tcpspdtest_init(void)
{
    int rc;

    tc_info("TCPSPDTEST init... rnr_engine[%d]\n", rnr_engine);

    spin_lock_init(&stream_lock);
    memset(&g_stream_alloc, 0, sizeof(g_stream_alloc));
    num_allocated_streams = 0;

    memset(&g_tcpspd, 0 , sizeof(g_tcpspd));

    rc = tcpspd_sock_mgr_init();
    if (rc)
    {
        tc_err("Failed to initialize soc_mgr\n");
        return -1;
    }

    rc = tcpspd_engine_init(&engine_config);
    if (rc)
    {
        tc_err("Failed to initialize tcp_engine\n");
        return -1;
    }

    rc = tcpspdtest_proc_init();
    if (rc)
    {
        tc_err("Failed to initialize proc\n");
        return -1;
    }

    rc = tcpspd_genl_init();
    if (rc)
    {
        tc_err("Failed to initialize genetlink\n");
        return -1;
    }

#ifdef CONFIG_BCM_XRDP
    rc = udpspd_engine_init();
    if (rc)
    {
        tc_err("Failed to initialize UDP speed test settings\n");
        return -1;
    }
#endif

    return 0;
}

/* TCPSPDTEST Module Clean Up */
static void __exit tcpspdtest_cleanup(void)
{
    int i;

    pr_info("TCPSPDTEST Shutting Down...\n");

    tcpspd_engine_shutdown();

    for (i = 0; i < SPDT_NUM_OF_STREAMS; i++)
        tcpspd_sock_mgr_shutdown(i);

    tcpspd_genl_shutdown();

#ifdef CONFIG_BCM_XRDP
    udpspd_engine_shutdown();
#endif

    remove_proc_subtree(PROC_DIR, proc_dir);
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION(DRIVER_DESC);
module_init(tcpspdtest_init);
module_exit(tcpspdtest_cleanup);
