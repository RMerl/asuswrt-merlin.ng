/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Wayne Lee <waynelee@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __RFCOMM_H
#define __RFCOMM_H

#include <endian.h>

#define RFCOMM_PSM 3

#define TRUE  1
#define FALSE 0

#define RFCOMM_MAX_CONN 10
#define BT_NBR_DATAPORTS RFCOMM_MAX_CONN

#define GET_BIT(pos,bitfield) ((bitfield[(pos)/32]) & (1 << ((pos) % 32)))
#define SET_BIT(pos,bitfield) ((bitfield[(pos)/32]) |= (1 << ((pos) % 32))) 
#define CLR_BIT(pos,bitfield) ((bitfield[(pos)/32]) &= ((1 << ((pos) % 32)) ^ (~0)))

/* Sets the P/F-bit in the control field */
#define SET_PF(ctr) ((ctr) | (1 << 4)) 
/* Clears the P/F-bit in the control field */
#define CLR_PF(ctr) ((ctr) & 0xef)
/* Returns the P/F-bit */
#define GET_PF(ctr) (((ctr) >> 4) & 0x1)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* Endian-swapping macros for structs */
#define swap_long_frame(x) ((x)->h.length.val = le16_to_cpu((x)->h.length.val))
#define swap_mcc_long_frame(x) (swap_long_frame(x))

/* Used for UIH packets */
#define SHORT_CRC_CHECK 2
/* Used for all packet exepts for the UIH packets */
#define LONG_CRC_CHECK 3
/* Short header for short UIH packets */
#define SHORT_HDR 2
/* Long header for long UIH packets */
#define LONG_HDR 3

/* FIXME: Should this one be defined here? */
#define SHORT_PAYLOAD_SIZE 127
/* Used for setting the EA field in different packets, really neccessary? */
#define EA 1
/* Yes the FCS size is only one byte */
#define FCS_SIZE 1

#define RFCOMM_MAX_HDR_SIZE 5

#define MAX_CREDITS   30
#define START_CREDITS 7
#define MIN_CREDITS   6

#define DEF_RFCOMM_MTU 127

/* The values in the control field when sending ordinary rfcomm packets */
#define SABM 0x2f	/* set asynchronous balanced mode */
#define UA   0x63	/* unnumbered acknolodgement */
#define DM   0x0f	/* disconnected mode */
#define DISC 0x43	/* disconnect */
#define UIH  0xef	/* unnumbered information with header check (only) */
#define UI   0x03	/* unnumbered information (with all data check) */

#define SABM_SIZE 4
#define UA_SIZE   4

/* The values in the type field in a multiplexer command packet */
#define PN    (0x80 >> 2)	/* parameter negotiation */
#define PSC   (0x40 >> 2)	/* power saving control */
#define CLD   (0xc0 >> 2)	/* close down */
#define TEST  (0x20 >> 2)	/* test */
#define FCON  (0xa0 >> 2)	/* flow control on */
#define FCOFF (0x60 >> 2)	/* flow control off */
#define MSC   (0xe0 >> 2)	/* modem status command */
#define NSC   (0x10 >> 2)	/* not supported command response */
#define RPN   (0x90 >> 2)	/* remote port negotiation */
#define RLS   (0x50 >> 2)	/* remote line status */
#define SNC   (0xd0 >> 2)	/* service negotiation command */

/* Define of some V.24 signals modem control signals in RFCOMM */
#define DV  0x80	/* data valid */
#define IC  0x40	/* incoming call */
#define RTR 0x08	/* ready to receive */
#define RTC 0x04	/* ready to communicate */
#define FC  0x02	/* flow control (unable to accept frames) */

#define CTRL_CHAN 0	/* The control channel is defined as DLCI 0 in rfcomm */
#define MCC_CMD 1	 /* Multiplexer command */
#define MCC_RSP 0	 /* Multiplexer response */

#if __BYTE_ORDER == __LITTLE_ENDIAN

typedef struct parameter_mask {
	uint8_t bit_rate:1;
	uint8_t data_bits:1;
	uint8_t stop_bit:1;
	uint8_t parity:1;
	uint8_t parity_type:1;
	uint8_t xon:1;
	uint8_t xoff:1;
	uint8_t res1:1;
	uint8_t xon_input:1;
	uint8_t xon_output:1;
	uint8_t rtr_input:1;
	uint8_t rtr_output:1;
	uint8_t rtc_input:1;
	uint8_t rtc_output:1;
	uint8_t res2:2;
} __attribute__ ((packed)) parameter_mask;

typedef struct rpn_values {
	uint8_t bit_rate;
	uint8_t data_bits:2;
	uint8_t stop_bit:1;
	uint8_t parity:1;
	uint8_t parity_type:2;
	uint8_t res1:2;
	uint8_t xon_input:1;
	uint8_t xon_output:1;
	uint8_t rtr_input:1;
	uint8_t rtr_output:1;
	uint8_t rtc_input:1;
	uint8_t rtc_output:1;
	uint8_t res2:2;
	uint8_t xon;
	uint8_t xoff;
	uint16_t pm;
	//parameter_mask pm;
} __attribute__ ((packed)) rpn_values;

#elif __BYTE_ORDER == __BIG_ENDIAN

typedef struct parameter_mask {
	uint8_t res1:1;
	uint8_t xoff:1;
	uint8_t xon:1;
	uint8_t parity_type:1;
	uint8_t parity:1;
	uint8_t stop_bit:1;
	uint8_t data_bits:1;
	uint8_t bit_rate:1;
	uint8_t res2:2;
	uint8_t rtc_output:1;
	uint8_t rtc_input:1;
	uint8_t rtr_output:1;
	uint8_t rtr_input:1;
	uint8_t xon_output:1;
	uint8_t xon_input:1;

} __attribute__ ((packed)) parameter_mask;

typedef struct rpn_values {
	uint8_t bit_rate;
	uint8_t res1:2;
	uint8_t parity_type:2;
	uint8_t parity:1;
	uint8_t stop_bit:1;
	uint8_t data_bits:2;
	uint8_t res2:2;
	uint8_t rtc_output:1;
	uint8_t rtc_input:1;
	uint8_t rtr_output:1;
	uint8_t rtr_input:1;
	uint8_t xon_output:1;
	uint8_t xon_input:1;
	uint8_t xon;
	uint8_t xoff;
	uint16_t pm;
	//parameter_mask pm;
} __attribute__ ((packed)) rpn_values;

#else
#error "Unknown byte order"
#endif

/* Typedefinitions of stuctures used for creating and parsing packets, for a
 * further description of the structures please se the bluetooth core
 * specification part F:1 and the ETSI TS 07.10 specification  */

#if __BYTE_ORDER == __LITTLE_ENDIAN

typedef struct address_field {
	uint8_t ea:1;
	uint8_t cr:1;
	uint8_t d:1;
	uint8_t server_chn:5;
} __attribute__ ((packed)) address_field;

typedef struct short_length {
	uint8_t ea:1;
	uint8_t len:7;
} __attribute__ ((packed)) short_length;

typedef union long_length {
	struct bits {
		uint8_t ea:1;
		unsigned short len:15;
	} __attribute__ ((packed)) bits ;
	uint16_t val ;
} __attribute__ ((packed)) long_length;

typedef struct short_frame_head {
	address_field addr;
	uint8_t control;
	short_length length;
} __attribute__ ((packed)) short_frame_head;

typedef struct short_frame {
	short_frame_head h;
	uint8_t data[0]; 
} __attribute__ ((packed)) short_frame;

typedef struct long_frame_head {
	address_field addr;
	uint8_t control;
	long_length length;
	uint8_t data[0];
} __attribute__ ((packed)) long_frame_head;

typedef struct long_frame {
	long_frame_head h;
	uint8_t data[0];
} __attribute__ ((packed)) long_frame;

/* Typedefinitions for structures used for the multiplexer commands */
typedef struct mcc_type {
	uint8_t ea:1;
	uint8_t cr:1;
	uint8_t type:6;
} __attribute__ ((packed)) mcc_type;

typedef struct mcc_short_frame_head {
	mcc_type type;
	short_length length;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_short_frame_head;

typedef struct mcc_short_frame {
	mcc_short_frame_head h;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_short_frame;

typedef struct mcc_long_frame_head {
	mcc_type type;
	long_length length;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_long_frame_head;

typedef struct mcc_long_frame {
	mcc_long_frame_head h;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_long_frame;

/* MSC-command */
typedef struct v24_signals {
	uint8_t ea:1;
	uint8_t fc:1;
	uint8_t rtc:1;
	uint8_t rtr:1;
	uint8_t reserved:2;
	uint8_t ic:1;
	uint8_t dv:1;
} __attribute__ ((packed)) v24_signals;

typedef struct break_signals {
	uint8_t ea:1;
	uint8_t b1:1;
	uint8_t b2:1;
	uint8_t b3:1;
	uint8_t len:4;
} __attribute__ ((packed)) break_signals;

typedef struct msc_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	v24_signals v24_sigs;
	//break_signals break_sigs;
	uint8_t fcs;
} __attribute__ ((packed)) msc_msg;

typedef struct rpn_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	rpn_values rpn_val;
	uint8_t fcs;
} __attribute__ ((packed)) rpn_msg;

/* RLS-command */  
typedef struct rls_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	uint8_t error:4;
	uint8_t res:4;
	uint8_t fcs;
} __attribute__ ((packed)) rls_msg;

/* PN-command */
typedef struct pn_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
/* The res1, res2 and res3 values have to be set to 0 by the sender */
	uint8_t dlci:6;
	uint8_t res1:2;
	uint8_t frame_type:4;
	uint8_t credit_flow:4;
	uint8_t prior:6;
	uint8_t res2:2;
	uint8_t ack_timer;
	uint16_t frame_size:16;
	uint8_t max_nbrof_retrans;
	uint8_t credits;
	uint8_t fcs;
} __attribute__ ((packed)) pn_msg;

/* NSC-command */
typedef struct nsc_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	mcc_type command_type;
	uint8_t fcs;
} __attribute__ ((packed)) nsc_msg;

#elif __BYTE_ORDER == __BIG_ENDIAN

typedef struct address_field {
	uint8_t server_chn:5;
	uint8_t d:1;
	uint8_t cr:1;
	uint8_t ea:1;
} __attribute__ ((packed)) address_field;

typedef struct short_length {
	uint8_t len:7;
	uint8_t ea:1;
} __attribute__ ((packed)) short_length;

typedef union long_length {
	struct bits {
		unsigned short len:15;
		uint8_t ea:1;
	} __attribute__ ((packed)) bits;
	uint16_t val;
} __attribute__ ((packed)) long_length;

typedef struct short_frame_head {
	address_field addr;
	uint8_t control;
	short_length length;
} __attribute__ ((packed)) short_frame_head;

typedef struct short_frame {
	short_frame_head h;
	uint8_t data[0];
} __attribute__ ((packed)) short_frame;

typedef struct long_frame_head {
	address_field addr;
	uint8_t control;
	long_length length;
	uint8_t data[0];
} __attribute__ ((packed)) long_frame_head;

typedef struct long_frame {
	long_frame_head h;
	uint8_t data[0];
} __attribute__ ((packed)) long_frame;

typedef struct mcc_type {
	uint8_t type:6;
	uint8_t cr:1;
	uint8_t ea:1;
} __attribute__ ((packed)) mcc_type;

typedef struct mcc_short_frame_head {
	mcc_type type;
	short_length length;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_short_frame_head;

typedef struct mcc_short_frame {
	mcc_short_frame_head h;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_short_frame;

typedef struct mcc_long_frame_head {
	mcc_type type;
	long_length length;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_long_frame_head;

typedef struct mcc_long_frame {
	mcc_long_frame_head h;
	uint8_t value[0];
} __attribute__ ((packed)) mcc_long_frame;

typedef struct v24_signals {
	uint8_t dv:1;
	uint8_t ic:1;
	uint8_t reserved:2;
	uint8_t rtr:1;
	uint8_t rtc:1;
	uint8_t fc:1;
	uint8_t ea:1;
} __attribute__ ((packed)) v24_signals;

typedef struct break_signals {
	uint8_t len:4;
	uint8_t b3:1;
	uint8_t b2:1;
	uint8_t b1:1;
	uint8_t ea:1;
} __attribute__ ((packed)) break_signals;

typedef struct msc_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	v24_signals v24_sigs;
	//break_signals break_sigs;
	uint8_t fcs;
} __attribute__ ((packed)) msc_msg;

typedef struct rpn_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	rpn_values rpn_val;
	uint8_t fcs;
} __attribute__ ((packed)) rpn_msg;

typedef struct rls_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	address_field dlci;
	uint8_t res:4;
	uint8_t error:4;
	uint8_t fcs;
} __attribute__ ((packed)) rls_msg;

typedef struct pn_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	uint8_t res1:2;
	uint8_t dlci:6;
	uint8_t credit_flow:4;
	uint8_t frame_type:4;
	uint8_t res2:2;
	uint8_t prior:6;
	uint8_t ack_timer;
	uint16_t frame_size:16;
	uint8_t max_nbrof_retrans;
	uint8_t credits;
	uint8_t fcs;
} __attribute__ ((packed)) pn_msg;

typedef struct nsc_msg {
	short_frame_head s_head;
	mcc_short_frame_head mcc_s_head;
	mcc_type command_type;
	uint8_t fcs;
} __attribute__ ((packed)) nsc_msg;

#else
#error "Unknown byte order"
#error Processor endianness unknown!
#endif

#endif /* __RFCOMM_H */
