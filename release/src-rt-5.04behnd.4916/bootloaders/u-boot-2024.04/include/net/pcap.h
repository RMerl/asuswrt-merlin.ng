/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019
 * Ramon Fried <rfried.dev@gmail.com>
 */

/**
 * pcap_init() - Initialize PCAP memory buffer
 *
 * @paddr	physicaly memory address to store buffer
 * @size	maximum size of capture file in memory
 *
 * Return:	0 on success, -ERROR on error
 */
int pcap_init(phys_addr_t paddr, unsigned long size);

/**
 * pcap_start_stop() - start / stop pcap capture
 *
 * @start	if true, start capture if false stop capture
 *
 * Return:	0 on success, -ERROR on error
 */
int pcap_start_stop(bool start);

/**
 * pcap_clear() - clear pcap capture buffer and statistics
 *
 * Return:	0 on success, -ERROR on error
 */
int pcap_clear(void);

/**
 * pcap_print_status() - print status of pcap capture
 *
 * Return:	0 on success, -ERROR on error
 */
int pcap_print_status(void);

/**
 * pcap_active() - check if pcap is enabled
 *
 * Return:	TRUE if active, FALSE if not.
 */
bool pcap_active(void);

/**
 * pcap_post() - Post a packet to PCAP file
 *
 * @packet:	packet to post
 * @len:	packet length in bytes
 * @outgoing	packet direction (outgoing/incoming)
 * Return:	0 on success, -ERROR on error
 */
int pcap_post(const void *packet, size_t len, bool outgoing);
