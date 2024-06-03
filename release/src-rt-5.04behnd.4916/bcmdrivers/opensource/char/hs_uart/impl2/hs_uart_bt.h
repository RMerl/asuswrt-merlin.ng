/*
* <:copyright-BRCM:2013:GPL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#ifndef __HS_UART_BT_H
#define __HS_UART_BT_H

int __init hs_uart_decode_rx_init(void);
int __exit hs_uart_decode_rx_exit(void);
int hs_uart_decode_rx_startup(struct uart_port *port);
int hs_uart_decode_rx_shutdown(struct uart_port *port);
int hs_uart_decode_rx_error(struct uart_port *port);
int hs_uart_decode_rx_comp(struct uart_port *port);
int hs_uart_decode_rx_char(struct uart_port *port, unsigned char ch, int *stopRx);

#endif /* __HS_UART_BT_H */