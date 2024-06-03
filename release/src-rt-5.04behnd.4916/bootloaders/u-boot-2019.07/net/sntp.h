/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Masami Komiya <mkomiya@sonare.it> 2005
 */

#ifndef __SNTP_H__
#define __SNTP_H__

#define NTP_SERVICE_PORT	123
#define SNTP_PACKET_LEN		48


/* Leap Indicator */
#define NTP_LI_NOLEAP		0x0
#define NTP_LI_61SECS		0x1
#define NTP_LI_59SECS		0x2
#define NTP_LI_ALARM		0x3

/* Version */

#define NTP_VERSION		4

/* Mode */
#define NTP_MODE_RESERVED	0
#define NTP_MODE_SYMACTIVE	1	/* Symmetric Active */
#define NTP_MODE_SYMPASSIVE	2	/* Symmetric Passive */
#define NTP_MODE_CLIENT		3
#define NTP_MODE_SERVER		4
#define NTP_MODE_BROADCAST	5
#define NTP_MODE_NTPCTRL	6	/* Reserved for NTP control message */
#define NTP_MODE_PRIVATE	7	/* Reserved for private use */

struct sntp_pkt_t {
#if __LITTLE_ENDIAN
	uchar mode:3;
	uchar vn:3;
	uchar li:2;
#else
	uchar li:2;
	uchar vn:3;
	uchar mode:3;
#endif
	uchar stratum;
	uchar poll;
	uchar precision;
	uint root_delay;
	uint root_dispersion;
	uint reference_id;
	unsigned long long reference_timestamp;
	unsigned long long originate_timestamp;
	unsigned long long receive_timestamp;
	unsigned long long transmit_timestamp;
} __attribute__((packed));

void sntp_start(void);	/* Begin SNTP */

#endif /* __SNTP_H__ */
