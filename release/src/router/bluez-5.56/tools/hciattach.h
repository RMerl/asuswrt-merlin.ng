/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <termios.h>

#ifndef N_HCI
#define N_HCI	15
#endif

#define HCIUARTSETPROTO		_IOW('U', 200, int)
#define HCIUARTGETPROTO		_IOR('U', 201, int)
#define HCIUARTGETDEVICE	_IOR('U', 202, int)
#define HCIUARTSETFLAGS		_IOW('U', 203, int)
#define HCIUARTGETFLAGS		_IOR('U', 204, int)

#define HCI_UART_H4	0
#define HCI_UART_BCSP	1
#define HCI_UART_3WIRE	2
#define HCI_UART_H4DS	3
#define HCI_UART_LL	4
#define HCI_UART_ATH3K  5
#define HCI_UART_INTEL	6
#define HCI_UART_BCM	7
#define HCI_UART_QCA	8
#define HCI_UART_AG6XX	9
#define HCI_UART_NOKIA	10
#define HCI_UART_MRVL	11

#define HCI_UART_RAW_DEVICE	0
#define HCI_UART_RESET_ON_INIT	1
#define HCI_UART_CREATE_AMP	2
#define HCI_UART_INIT_PENDING	3
#define HCI_UART_EXT_CONFIG	4
#define HCI_UART_VND_DETECT	5

int read_hci_event(int fd, unsigned char *buf, int size);
int set_speed(int fd, struct termios *ti, int speed);
int uart_speed(int speed);

int texas_init(int fd, int *speed, struct termios *ti);
int texas_post(int fd, struct termios *ti);
int texasalt_init(int fd, int speed, struct termios *ti);
int stlc2500_init(int fd, bdaddr_t *bdaddr);
int bgb2xx_init(int dd, bdaddr_t *bdaddr);
int ath3k_init(int fd, int speed, int init_speed, char *bdaddr,
						struct termios *ti);
int ath3k_post(int fd, int pm);
int qualcomm_init(int fd, int speed, struct termios *ti, const char *bdaddr);
int intel_init(int fd, int init_speed, int *speed, struct termios *ti);
int bcm43xx_init(int fd, int def_speed, int speed, struct termios *ti,
		const char *bdaddr);
