/*
 * uart driver definitions
 * Copyright 2002, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id: rwl_uart.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _rwl_uart_h_
#define _rwl_uart_h_

extern rwl_dongle_packet_t g_rwl_dongle_data;

extern void remote_uart_tx(uchar*buf);

#endif	/* _rwl_uart_h_ */
