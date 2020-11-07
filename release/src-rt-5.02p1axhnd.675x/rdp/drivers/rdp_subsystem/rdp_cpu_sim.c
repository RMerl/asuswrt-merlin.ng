/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifdef RDP_SIM


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bdmf_system.h"
#include "rdp_cpu_sim.h"
#include "rdd_cpu_rx.h"


#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)
#define RUNNER_SIM_PORT     20001

uint32_t g_runner_sim_srv_socket = INVALID_SOCKET;
uint32_t g_runner_sim_client_socket = INVALID_SOCKET;
uint32_t g_runner_sim_connected = 0;
FILE *g_cpu_rx_file = NULL;
bdmf_session_handle g_cpu_rx_file_session = NULL;

extern bdmf_int_parm_t g_int_params[];

void cpu_runner_sim_disconnect(uint32_t *sock)
{
    if (*sock != INVALID_SOCKET)
    {
        bdmf_trace("Socket disconnected\n");
        close(*sock);
        *sock = INVALID_SOCKET;
    }
    else
        bdmf_trace("Socket already disconnected\n");
}

void cpu_runner_sim_send_data(uint32_t length, char *buffer)
{
    if (g_runner_sim_client_socket == INVALID_SOCKET)
    {
        bdmf_trace("Socket not connected yet\n");
        return;
    }
    bdmf_trace("Sending %d bytes\n", length);

    send(g_runner_sim_client_socket, (char *)&length, sizeof(length), 0);
    send(g_runner_sim_client_socket, buffer, length, 0);
}

uint32_t cpu_runner_sim_receive_data(char *buffer)
{
    uint32_t length;
    int numbytes;

    numbytes = recv(g_runner_sim_client_socket, (char *)&length, sizeof(length), 0);
    if (numbytes < 0)
    {
        bdmf_trace("Socket recv error\n");
        return 0;
    }
    numbytes = recv(g_runner_sim_client_socket, buffer, length, 0);
    if (numbytes < 0)
    {
        bdmf_trace("Socket recv error\n");
        return 0;
    }  
    return length;
}

uint32_t cpu_runner_sim_get_msg_length(sw2hw_msg *msg)
{
    uint32_t size = sizeof(msg->type);
    switch (msg->type)
    {
    case SW2HW_MSG_QM_REQUEST:
        size += sizeof(msg->qm_insert_request);
        break;
    case SW2HW_MSG_MEM_READ:
        size += sizeof(msg->mem_read);
        break;
    case SW2HW_MSG_MEM_WRITE:
        size += sizeof(msg->mem_write.size) + sizeof(msg->mem_write.address) + msg->mem_write.size;
        break;
    case SW2HW_MSG_END_OF_METHOD:
        size += sizeof(msg->end_of_method);
        break;
    case SW2HW_MSG_FPM_ALLOC:
        size += sizeof(msg->fpm_alloc);
        break;
    case SW2HW_MSG_FPM_FREE:
        size += sizeof(msg->fpm_free);
        break;
    case SW2HW_MSG_WAKEUP_RUNNER:
        size += sizeof(msg->wakeup);
        break;
    case SW2HW_MSG_COUNTER_READ:
        size += sizeof(msg->counter_read);
        break;
    default:
        break;
    }
    return size;
}

static void cpu_runner_sim_end_msg()
{
    sw2hw_msg sent_msg = {};

    sent_msg.type = SW2HW_MSG_END_OF_METHOD;
    sent_msg.end_of_method.finish = 1;
    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&sent_msg), (char *)&sent_msg);
}

int cpu_sim_rx_file_write(bdmf_session_handle session, const char *buf, uint32_t size)
{
    if (!g_cpu_rx_file)
        return 0;
    return fwrite(buf, 1, size, g_cpu_rx_file);
}

int cpu_runner_sim_connect(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int true_flag = 1, number_of_tries = 0;
    int i, rc = BDMF_ERR_INTERNAL;
    struct sockaddr_in connection_address;
    socklen_t socket_address_size;
    uint32_t port = RUNNER_SIM_PORT;
    char *peer_address;
    hw2sw_msg rcvd_msg = {};
    bdmf_session_parm_t session_parm = {};

    if (n_parms == 1)
        port = (uint32_t)parm[0].value.unumber;

    /* Create a TCP socket */
    g_runner_sim_srv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (g_runner_sim_srv_socket == INVALID_SOCKET)
    {
        bdmf_session_print(session, "Invalid Socket\n");
        goto exit;
    }

    if (setsockopt(g_runner_sim_srv_socket, SOL_SOCKET, SO_REUSEADDR, &true_flag, sizeof(int)) == INVALID_SOCKET)
    {
        bdmf_session_print(session, "setsockopt error occured with errno=%d\n", errno);
        goto exit;
    }

    /* Bind socket to local address */
    socket_address_size = sizeof(struct sockaddr_in);
    memset((char *)&connection_address, 0, socket_address_size);
    connection_address.sin_family = AF_INET;
    connection_address.sin_addr.s_addr = htonl(INADDR_ANY);
    connection_address.sin_port = htons((unsigned short) port);

    /* try to bind to different ports to allow several applications to run in parallel */
    for (number_of_tries=0 ; ; number_of_tries++)
    {
        if (number_of_tries == 256)
            goto exit;
        if (bind(g_runner_sim_srv_socket, (struct sockaddr *)&connection_address, sizeof(struct sockaddr_in)) != INVALID_SOCKET &&
            listen(g_runner_sim_srv_socket, 1) != INVALID_SOCKET)
        {
            break;  // success
        }
        else
        {
            bdmf_session_print(session, "Bind error occured for port=%d with errno=%d\n", port, errno);
            port++;
            connection_address.sin_port = htons((unsigned short) port);
        }
    }
    bdmf_session_print(session, "Bind succeeded with port=%d\n", port);

    // output 'ready file' as indication for ut to run runner simulator
    if (_segment_file_init("ready", "w+b", (uint8_t *)&port, sizeof(port)))
         goto exit;

    bdmf_session_print(session, "Waiting for runner simulator to connect... (port=%d)\n", port);

    g_runner_sim_client_socket = accept(g_runner_sim_srv_socket, (struct sockaddr *)&connection_address, &socket_address_size);
    if (g_runner_sim_client_socket == INVALID_SOCKET)
    {
        bdmf_session_print(session, "Accept error occured with errno=%d\n", errno);
        goto exit;
    }

    /* Copy peer address */
    peer_address = inet_ntoa(connection_address.sin_addr);
    bdmf_session_print(session, "Runner simulator is connected from %s\n", peer_address);

    g_runner_sim_connected = 1;
    
    /* before handling messages, open cpu-rx file session */
    g_cpu_rx_file = fopen("cpu-sim.rx", "w");

    memset(&session_parm, 0, sizeof(session_parm));
    /* TODO: can keep g_cpu_rx_file in session_parm private data? (instead of global) and use it from there in cpu_sim_rx_file_write */
    session_parm.name = "cpu-sim.rx";
    session_parm.hex_dump_format = BDMF_HEX_DUMP_FORMAT_BYTE;
    session_parm.write = cpu_sim_rx_file_write;
    rc = bdmf_session_open(&session_parm, &g_cpu_rx_file_session);
    if (rc)
    	goto exit;

    rc = BDMF_ERR_OK;
    /* wait for messages */
    while (cpu_runner_sim_receive_data((char *)&rcvd_msg) > 0)
    {
        switch (rcvd_msg.type)
        {
            case HW2SW_MSG_INTERRUPT:
            {
                bdmf_session_print(session, "Got interrupt #%d\n", rcvd_msg.interrupt.interrupt_id);
                for (i = 0; i < MAX_INT_NUM; i++)
                {
                    if (g_int_params[i].irq == rcvd_msg.interrupt.interrupt_id)
                    {
                        rc = g_int_params[i].int_cb(g_int_params[i].irq, g_int_params[i].priv);
                        if (rc != BDMF_IRQ_HANDLED)
                            goto exit;
                        rc = 0;
                        cpu_runner_sim_end_msg();
                        break;
                    }
                }
                if (i == MAX_INT_NUM)
                {
                    bdmf_session_print(session, "No isr conifgured for irq %d\n", rcvd_msg.interrupt.interrupt_id);
                    goto exit;
                }
                break;
            }
            case HW2SW_MSG_SEND_PACKET:
            {
                bdmf_session_print(session, "Received packet data: %s\n\n", rcvd_msg.send_packet.pkt_cfg);
                if (bdmfmon_parse(session, rcvd_msg.send_packet.pkt_cfg))
                    goto exit;
                cpu_runner_sim_end_msg();
                break;
            }
            case HW2SW_MSG_END_OF_SIMULATION:
            {
                goto exit;
            }
            default:
            {
                bdmf_session_print(session, "unsupported message type %d\n", rcvd_msg.type);
                rc = BDMF_ERR_PARM;
                goto exit;
            }
        }
    }

exit:
    cpu_runner_sim_disconnect(&g_runner_sim_srv_socket);
    cpu_runner_sim_disconnect(&g_runner_sim_client_socket);
    g_runner_sim_connected = 0;
    if (g_cpu_rx_file)
        fclose(g_cpu_rx_file);
    if (g_cpu_rx_file_session)
        bdmf_session_close(g_cpu_rx_file_session);
    return rc;
}

void rdp_cpu_qm_req(uint32_t *cpu_tx_descriptor, uint16_t qm_queue_num)
{
    int i;
    sw2hw_msg qm_req_msg = {};

    qm_req_msg.type = SW2HW_MSG_QM_REQUEST;
    qm_req_msg.qm_insert_request.queue_id = qm_queue_num;

    memcpy(qm_req_msg.qm_insert_request.pd, (uint8_t *)cpu_tx_descriptor, 16);
    bdmf_print("sending pd to QM queue: %d\n", qm_req_msg.qm_insert_request.queue_id);
    bdmf_print("Original PD data:");
    for (i = 0; i < 4; i++)
        bdmf_print("%x ", cpu_tx_descriptor[i]);
    bdmf_print("\n");
    bdmf_print("sending pd to QM PD data:");
    for (i = 0; i < 16; i++)
        bdmf_print("%x ", qm_req_msg.qm_insert_request.pd[i]);
    bdmf_print("\n");

    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&qm_req_msg), (char *)&qm_req_msg);
}

int rdp_cpu_fpm_alloc(uint32_t packet_len, uint32_t *buff_num)
{
    uint32_t pool_id;
    sw2hw_msg fpm_req_msg = {};
    hw2sw_msg res_msg = {};
        
    if (packet_len <= 512)
        pool_id = 3;
    else if (packet_len <= 1024)
        pool_id = 2;
    else if (packet_len <= 2048)
        pool_id = 1;
    else if (packet_len <= 4096)
        pool_id = 0;
        
    fpm_req_msg.type = SW2HW_MSG_FPM_ALLOC;
    fpm_req_msg.fpm_alloc.pool_id = pool_id;

    bdmf_print("sending fpm alloc request with pool_id=%d\n", pool_id);

    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&fpm_req_msg), (char *)&fpm_req_msg);

    /* get response */
    if (cpu_runner_sim_receive_data((char*)&res_msg) > 0)
    {
    	*buff_num = (pool_id << 16) | res_msg.fpm_alloc_response.buffer_num;
    	return BDMF_ERR_OK;
    }
    return BDMF_ERR_NORES;
}

void rdp_cpu_fpm_free(uint32_t buffer_num)
{
    sw2hw_msg fpm_req_msg = {};

    fpm_req_msg.type = SW2HW_MSG_FPM_FREE;
    fpm_req_msg.fpm_free.buffer_num = buffer_num;

    bdmf_print("sending fpm free request for buffer=%d\n", buffer_num);

    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&fpm_req_msg), (char *)&fpm_req_msg);
}

void rdp_cpu_runner_wakeup(uint32_t runner_id, uint32_t task_id)
{
    sw2hw_msg wakeup_req_msg = {};

    wakeup_req_msg.type = SW2HW_MSG_WAKEUP_RUNNER;
    wakeup_req_msg.wakeup.runner_id = runner_id;
    wakeup_req_msg.wakeup.task_id = task_id;

    bdmf_print("sending wakeup request for runner=%d task=%d\n", runner_id, task_id);

    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&wakeup_req_msg), (char *)&wakeup_req_msg);
}


int rdp_cpu_counter_read(uint32_t group_id, uint32_t start_counter, uint32_t* cntr_arr, uint32_t num_of_counters, bdmf_boolean* cntr_double, uint8_t* cn0_bytes)
{
    sw2hw_msg cntr_req_msg = {};
    hw2sw_msg res_msg = {};
       
    cntr_req_msg.type = SW2HW_MSG_COUNTER_READ;
    cntr_req_msg.counter_read.group_id = group_id;
    cntr_req_msg.counter_read.start_counter = start_counter;
    cntr_req_msg.counter_read.count = num_of_counters;

    bdmf_print("sending counter read request with counter_group=%d, counter_id=%d, num_of_counters = %d\n", group_id, start_counter, num_of_counters);

    cpu_runner_sim_send_data(cpu_runner_sim_get_msg_length(&cntr_req_msg), (char *)&cntr_req_msg);

    /* get response */
    if (cpu_runner_sim_receive_data((char*)&res_msg) > 0)
    {
    	bdmf_print("get counter value 32b = %x size = %d\n", ((uint32_t *)&res_msg.counter_read_response.data)[0], res_msg.counter_read_response.size);
    	memcpy(cntr_arr, &res_msg.counter_read_response.data[0], res_msg.counter_read_response.size);
    	*cntr_double = res_msg.counter_read_response.cntr_double;
    	*cn0_bytes = res_msg.counter_read_response.cn0_bytes;
    	return BDMF_ERR_OK;
    }
    return BDMF_ERR_NORES;
}


#endif

