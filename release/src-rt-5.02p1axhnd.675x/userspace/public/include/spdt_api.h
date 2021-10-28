/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*!\file spdt_api.h
 * \brief Header file for Speed Test offload APIs.
 *
 * Main API functions for Speed Test offload. Supports TCP and UDP based third party speed test applications.
 * APIs assume that only single speed service application can run on the same time.
 *
 */

#ifndef __SPDT_API_H__
#define __SPDT_API_H__

#include <spdt_defs.h>
#include <tcpspdtest_defs.h> /* Temporary */

/*************** APIs ************************/

/* Add stream, to support multi streaming. Streams pool is common for both TX and RX.
 * For TCP, will create/delete the internal socket, for UDP only allocate/free a stream IDX.
 * Stream is unique per connection (i.e. same stream index will not be used for TX and for RX).
 * Args:
 *   stream_idx - An integer to represent the socket stream index on the rest of API functions
 *   proto - speed test protocol (one of the TCP or UDP) that will be used for the test
 * Return: 0 - on success, other on failure
 * */
int spdt_init(spdt_proto_t proto, uint8_t *stream_idx);

/* Delete allocated stream
 * Args:
 *   stream_idx - An integer to represent the socket stream index on the rest of API functions
 */
void spdt_uninit(uint8_t stream_idx);


/* Open a connection. For TCP, will connect the initialized inernal socket to the server; for UDP will store the params
 * in an internal context per stream only, for further connection, as UDP is stateless.
 * Args:
 *   stream_idx
 *   dir - direction rx/tx.
 *   conn_params - remote connection information (address + proto + destination port)
 * Return: 0 - on success, other on failure
 * */
int spdt_connect(uint8_t stream_idx, spdt_stream_dir_t dir, spdt_conn_params_t *conn_params);

/* Close a connection.
 * Args:
 *   stream_idx 
 * */
void spdt_disconnect(uint8_t stream_idx);


/* Send Out-Of-Band data to connected server.
 * Each protocol should implement it's own messaging system. Today implemented for TCP only.
 * Args:
 *   stream_idx 
 *   buff - Pointer to buffer to be sent
 *   length - length of buffer to be sent
 * Return: 0 - on success, other on failure
 * */
int spdt_oob_send(uint8_t stream_idx, char *buff, uint32_t length);


/* Retrieve connection information (local and remote 5-tuple, etc.) 
 * Args:
 *   stream_idx 
 *   conn_addr - output server and client socket_storage
 * Return: 0 - on success, other on failure
 * */
int spdt_get_conn_params(uint8_t stream_idx, spdt_conn_params_t *conn_params);


/* Get statistics, clear on read
 * Args:
 *   stream_idx 
 *   stat - statistics structure, will be filled according to speed test protocol the stream used during connection
 * Return: 0 - on success, other on failure
 * */
int spdt_stats_get(uint8_t stream_idx, spdt_stat_t *stat);


/* Start receiving on the opened connection (analizer mode).
 * Args:
 *   stream_idx 
 *   rx_params - Receive parameters
 *               - for TCP, size of buffer to download, for HTTP also name of the file for HTTP Get request. Receive
 *                 can be either stopped explicitly (e.g. iperf3), or when download completed (e.g. http get).
 *               - for UDP, currently not in use.
 * Return: 0 - on success, other on failure
 * */
int spdt_recv_start(uint8_t stream_idx, spdt_rx_params_t *rx_params);

/* Stop receiving on the opened connection (analizer mode). Results are available until spdt_uninit is invoked.
 * Function call is optional, spdt_disconnect can be invoked to stop receiving and close the connection. Not used in
 * currently implemented TCP protocols.
 * Args:
 *   stream_idx 
 * */
void spdt_recv_stop(uint8_t stream_idx);


/* Start/stop transmit data on the opened connection (generator mode).
 * Args:
 *   stream_idx 
 *   tx_params - Transmit parameters
 *                      - for TCP, size of buffer to upload. Transmit can be either stopped explicitly (e.g. iperf3), or
 *                        when upload completed (e.g. http put).
 *                      - for UDP, kbps, data_len, and other optional parameters 
 * Return: 0 - on success, other on failure
 * */
int spdt_send_start(uint8_t stream_idx, spdt_tx_params_t *tx_params);

/* Stop transmit data on the opened connection (generator mode). Results are available until spdt_uninit is invoked.
 * Function call is optional, spdt_disconnect can be invoked to stop receiving and close the connection. Not used in
 * currently implemented TCP protocols.
 * Args:
 *   stream_idx 
 * */
void spdt_send_stop(uint8_t stream_idx);


#endif /* __SPDT_API_H__ */

