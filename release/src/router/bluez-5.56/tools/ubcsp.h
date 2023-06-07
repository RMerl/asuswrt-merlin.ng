/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2005  CSR Ltd.
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef UBCSP_INCLUDE_H
#define UBCSP_INCLUDE_H

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**                                                                         **/
/** ubcsp.h                                                                 **/
/**                                                                         **/
/** MicroBCSP - a very low cost implementation of the BCSP protocol         **/
/**                                                                         **/
/*****************************************************************************/

/* If we wish to use CRC's, then change 0 to 1 in the next line */
#define UBCSP_CRC 1

/* Define some basic types - change these for your architecture */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

/* The defines below require a printf function to be available */

/* Do we want to show packet errors in debug output */
#define SHOW_PACKET_ERRORS	0

/* Do we want to show Link Establishment State transitions in debug output */
#define SHOW_LE_STATES		0

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_packet                                                            **/
/**                                                                         **/
/** This is description of a bcsp packet for the upper layer                **/
/**                                                                         **/
/*****************************************************************************/

struct ubcsp_packet
{
	uint8 channel;		/* Which Channel this packet is to/from */
	uint8 reliable;		/* Is this packet reliable */
	uint8 use_crc;		/* Does this packet use CRC data protection */
	uint16 length;		/* What is the length of the payload data */
	uint8 *payload;		/* The payload data itself - size of length */
};

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_configuration                                                     **/
/**                                                                         **/
/** This is the main configuration of the ubcsp engine                      **/
/** All state variables are stored in this structure                        **/
/**                                                                         **/
/*****************************************************************************/

enum ubcsp_link_establishment_state
{
	ubcsp_le_uninitialized,
	ubcsp_le_initialized,
	ubcsp_le_active
};

enum ubcsp_link_establishment_packet
{
	ubcsp_le_sync,
	ubcsp_le_sync_resp,
	ubcsp_le_conf,
	ubcsp_le_conf_resp,
	ubcsp_le_none
};

struct ubcsp_configuration
{
	uint8 link_establishment_state;
	uint8 link_establishment_resp;
	uint8 link_establishment_packet;

	uint8 sequence_number:3;
	uint8 ack_number:3;
	uint8 send_ack;
	struct ubcsp_packet *send_packet;
	struct ubcsp_packet *receive_packet;

	uint8 receive_header_checksum;
	uint8 receive_slip_escape;
	int32 receive_index;

	uint8 send_header_checksum;
#ifdef UBCSP_CRC
	uint8 need_send_crc;
	uint16 send_crc;
#endif
	uint8 send_slip_escape;

	uint8 *send_ptr;
	int32 send_size;
	uint8 *next_send_ptr;
	int32 next_send_size;

	int8 delay;
};

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_poll sets activity from an OR of these flags                      **/
/**                                                                         **/
/*****************************************************************************/

#define UBCSP_PACKET_SENT 0x01
#define UBCSP_PACKET_RECEIVED 0x02
#define UBCSP_PEER_RESET 0x04
#define UBCSP_PACKET_ACK 0x08

/*****************************************************************************/
/**                                                                         **/
/** This is the functional interface for ucbsp                              **/
/**                                                                         **/
/*****************************************************************************/

void ubcsp_initialize (void);
void ubcsp_send_packet (struct ubcsp_packet *send_packet);
void ubcsp_receive_packet (struct ubcsp_packet *receive_packet);
uint8 ubcsp_poll (uint8 *activity);

/*****************************************************************************/
/**                                                                         **/
/** Slip Escape Values                                                      **/
/**                                                                         **/
/*****************************************************************************/

#define SLIP_FRAME 0xC0
#define SLIP_ESCAPE 0xDB
#define SLIP_ESCAPE_FRAME 0xDC
#define SLIP_ESCAPE_ESCAPE 0xDD

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/**                                                                         **/
/** These functions need to be linked into your system                      **/
/**                                                                         **/
/*****************************************************************************/

/*****************************************************************************/
/**                                                                         **/
/** put_uart outputs a single octet over the UART Tx line                   **/
/**                                                                         **/
/*****************************************************************************/

extern void put_uart (uint8);

/*****************************************************************************/
/**                                                                         **/
/** get_uart receives a single octet over the UART Rx line                  **/
/** if no octet is available, then this returns 0                           **/
/** if an octet was read, then this is returned in the argument and         **/
/**   the function returns 1                                                **/
/**                                                                         **/
/*****************************************************************************/

extern uint8 get_uart (uint8 *);

/*****************************************************************************/
/**                                                                         **/
/** These defines should be changed to your systems concept of 100ms        **/
/**                                                                         **/
/*****************************************************************************/

#define UBCSP_POLL_TIME_IMMEDIATE   0
#define UBCSP_POLL_TIME_DELAY       25

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#endif
