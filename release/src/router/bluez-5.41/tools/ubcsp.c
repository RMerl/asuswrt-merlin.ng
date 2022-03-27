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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/**                                                                         **/
/** ubcsp,c                                                                 **/
/**                                                                         **/
/** MicroBCSP - a very low cost implementation of the BCSP protocol         **/
/**                                                                         **/
/*****************************************************************************/

#include "ubcsp.h"

#if SHOW_PACKET_ERRORS || SHOW_LE_STATES
#include <stdio.h>
#include <windows.h>
#endif

static uint16 ubcsp_calc_crc (uint8 ch, uint16 crc);
static uint16 ubcsp_crc_reverse (uint16);

/*****************************************************************************/
/**                                                                         **/
/** Constant Data - ROM                                                     **/
/**                                                                         **/
/*****************************************************************************/

/* This is the storage for the link establishment messages */

static const uint8 ubcsp_le_buffer[4][4] =
	{
		{ 0xDA, 0xDC, 0xED, 0xED },
		{ 0xAC, 0xAF, 0xEF, 0xEE },
		{ 0xAD, 0xEF, 0xAC, 0xED },
		{ 0xDE, 0xAD, 0xD0, 0xD0 },
	};

/* These are the link establishment headers */
/* The two version are for the CRC and non-CRC varients */

#if UBCSP_CRC
static const uint8 ubcsp_send_le_header[4] = 
	{
		0x40, 0x41, 0x00, 0x7E
	};
#else
static const uint8 ubcsp_send_le_header[4] = 
	{
		0x00, 0x41, 0x00, 0xBE
	};
#endif

/*****************************************************************************/
/**                                                                         **/
/** Static Data - RAM                                                       **/
/**                                                                         **/
/*****************************************************************************/

/* This is the storage for all state data for ubcsp */

static struct ubcsp_configuration ubcsp_config;

/* This is the ACK packet header - this will be overwritten when
   we create an ack packet */

static uint8 ubcsp_send_ack_header[4] = 
	{
		0x00, 0x00, 0x00, 0x00
	};

/* This is the deslip lookup table */

static const uint8 ubcsp_deslip[2] =
	{
		SLIP_FRAME, SLIP_ESCAPE,
	};

/* This is a state machine table for link establishment */

static uint8 next_le_packet[16] =
	{
		ubcsp_le_sync,			// uninit
		ubcsp_le_conf,			// init
		ubcsp_le_none,			// active
		ubcsp_le_none,
		ubcsp_le_sync_resp,		// sync_resp
		ubcsp_le_sync_resp,
		ubcsp_le_none,
		ubcsp_le_none,
		ubcsp_le_none,			// conf_resp
		ubcsp_le_conf_resp,
		ubcsp_le_conf_resp,
		ubcsp_le_none,
	};

/* This is the storage required for building send and crc data */

static uint8 ubcsp_send_header[4];
static uint8 ubcsp_send_crc[2];

/* This is where the receive header is stored before the payload arrives */

static uint8 ubcsp_receive_header[4];

/*****************************************************************************/
/**                                                                         **/
/** Code - ROM or RAM                                                       **/
/**                                                                         **/
/*****************************************************************************/

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_initialize                                                        **/
/**                                                                         **/
/** This initializes the state of the ubcsp engine to a known values        **/
/**                                                                         **/
/*****************************************************************************/

void ubcsp_initialize (void)
{
	ubcsp_config.ack_number = 0;
	ubcsp_config.sequence_number = 0;
	ubcsp_config.send_ptr = 0;
	ubcsp_config.send_size = 0;
	ubcsp_config.receive_index = -4;

	ubcsp_config.delay = 0;

#if SHOW_LE_STATES
	printf ("Hello Link Uninitialized\n");
#endif

	ubcsp_config.link_establishment_state = ubcsp_le_uninitialized;
	ubcsp_config.link_establishment_packet = ubcsp_le_sync;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_send_packet                                                       **/
/**                                                                         **/
/** This sends a packet structure for sending to the ubcsp engine           **/
/** This can only be called when the activity indication from ubcsp_poll    **/
/** indicates that a packet can be sent with UBCSP_PACKET_SENT              **/
/**                                                                         **/
/*****************************************************************************/

void ubcsp_send_packet (struct ubcsp_packet *send_packet)
{
	/* Initialize the send data to the packet we want to send */

	ubcsp_config.send_packet = send_packet;

	/* we cannot send the packet at the moment
	   when we can at the moment, just set things to 0 */

	ubcsp_config.send_size = 0;
	ubcsp_config.send_ptr = 0;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_receive_packet                                                    **/
/**                                                                         **/
/** This sends a packet structure for receiving to the ubcsp engine         **/
/** This can only be called when the activity indication from ubcsp_poll    **/
/** indicates that a packet can be sent with UBCSP_PACKET_RECEIVED          **/
/**                                                                         **/
/*****************************************************************************/

void ubcsp_receive_packet (struct ubcsp_packet *receive_packet)
{
	/* Initialize the receive data to the packet we want to receive */

	ubcsp_config.receive_packet = receive_packet;

	/* setup to receive the header first */

	ubcsp_config.receive_index = -4;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_calc_crc                                                          **/
/**                                                                         **/
/** Takes the next 8 bit value ch, and updates the crc with this value      **/
/**                                                                         **/
/*****************************************************************************/


#ifdef UBCSP_CRC

static uint16 ubcsp_calc_crc (uint8 ch, uint16 crc)
{
	/* Calculate the CRC using the above 16 entry lookup table */

	static const uint16 crc_table[] =
		{
			0x0000, 0x1081, 0x2102, 0x3183,
			0x4204, 0x5285, 0x6306, 0x7387,
			0x8408, 0x9489, 0xa50a, 0xb58b,
			0xc60c, 0xd68d, 0xe70e, 0xf78f
		};

	/* Do this four bits at a time - more code, less space */

    crc = (crc >> 4) ^ crc_table[(crc ^ ch) & 0x000f];
    crc = (crc >> 4) ^ crc_table[(crc ^ (ch >> 4)) & 0x000f];

	return crc;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_crc_reverse                                                       **/
/**                                                                         **/
/** Reserves the bits in crc and returns the new value                      **/
/**                                                                         **/
/*****************************************************************************/

static uint16 ubcsp_crc_reverse (uint16 crc)
{
	int32
		b,
		rev;

	/* Reserse the bits to compute the actual CRC value */

	for (b = 0, rev=0; b < 16; b++)
	{
		rev = rev << 1;
		rev |= (crc & 1);
		crc = crc >> 1;
	}

	return rev;
}

#endif

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_put_slip_uart                                                     **/
/**                                                                         **/
/** Outputs a single octet to the uart                                      **/
/** If the octet needs to be escaped, then output the escape value          **/
/** and then store the second octet to be output later                      **/
/**                                                                         **/
/*****************************************************************************/

static void ubcsp_put_slip_uart (uint8 ch)
{
	/* output a single UART octet */

	/* If it needs to be escaped, then output the escape octet
	   and set the send_slip_escape so that the next time we
	   output the second octet for the escape correctly.
	   This is done right at the top of ubcsp_poll */

	if (ch == SLIP_FRAME)
	{
		put_uart (SLIP_ESCAPE);
		ubcsp_config.send_slip_escape = SLIP_ESCAPE_FRAME;
	}
	else if (ch == SLIP_ESCAPE)
	{
		put_uart (SLIP_ESCAPE);
		ubcsp_config.send_slip_escape = SLIP_ESCAPE_ESCAPE;
	}
	else
	{
		/* Not escaped, so just output octet */

		put_uart (ch);
	}
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_which_le_payload                                                  **/
/**                                                                         **/
/** Check the payload of this packet, and determine which of the four       **/
/** link establishment packets this was.                                    **/
/** Can return 5 if it is not a valid link establishment packet             **/
/**                                                                         **/
/*****************************************************************************/

static uint32 ubcsp_which_le_payload (const uint8 *payload)
{
	static int32
		octet,
		loop;

	/* Search through the various link establishment payloads to find
	   which one we have received */

	for (loop = 0; loop < 4; loop ++)
	{
		for (octet = 0; octet < 4; octet ++)
		{
			if (payload[octet] != ubcsp_le_buffer[loop][octet])
			{
				/* Bad match, just to loop again */
				goto bad_match_loop;
			}
		}

		/* All the octets matched, return the value */

		return loop;

		/* Jumps out of octet loop if we got a bad match */
bad_match_loop:
		{}
	}

	/* Non of the link establishment payloads matched - return invalid value */

	return 5;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_recevied_packet                                                   **/
/**                                                                         **/
/** This function is called when we have a SLIP END octet and a full        **/
/** packet header and possibly data in the receive packet                   **/
/**                                                                         **/
/*****************************************************************************/

static uint8 ubcsp_recevied_packet (void)
{
	static uint8
		receive_crc,
		receive_seq,
		receive_ack,
		activity;

#if UBCSP_CRC
	static int32
		loop;

	static uint16
		crc;
#endif

	static uint16
		length;

	/* Keep track of what activity this received packet will cause */

	activity = 0;

	/*** Do all error checks that we can ***/

	/* First check the header checksum */

	if (((ubcsp_receive_header[0] + ubcsp_receive_header[1] + ubcsp_receive_header[2] + ubcsp_receive_header[3]) & 0xff) != 0xff)
	{
		/* Header Checksum Error */

#if SHOW_PACKET_ERRORS
		printf ("\n######################## Header Checksum Error %02X %02X %02X %02X\n",
			ubcsp_receive_header[0],
			ubcsp_receive_header[1],
			ubcsp_receive_header[2],
			ubcsp_receive_header[3]);
#endif

		/* If we have a header checksum error, send an ack in return
		   this gets a packet to be resent as quickly as possible */

		ubcsp_config.send_ack = 1;

		return activity;
	}

	/* Decode the received packets header */

	ubcsp_config.receive_packet->reliable = (ubcsp_receive_header[0] & 0x80) >> 7;

	receive_crc = (ubcsp_receive_header[0] & 0x40) >> 6;
	receive_ack = (ubcsp_receive_header[0] & 0x38) >> 3;
	receive_seq = (ubcsp_receive_header[0] & 0x07);

	ubcsp_config.receive_packet->channel = (ubcsp_receive_header[1] & 0x0f);

	length =
		((ubcsp_receive_header[1] & 0xf0) >> 4) |
		(ubcsp_receive_header[2] << 4);

#if SHOW_PACKET_ERRORS
	if (ubcsp_config.receive_packet->reliable)
	{
		printf (" : %10d         Recv SEQ: %d ACK %d\n",
			GetTickCount () % 100000,
			receive_seq,
			receive_ack);
	}
	else if (ubcsp_config.receive_packet->channel != 1)
	{
		printf (" : %10d          Recv        ACK %d\n",
			GetTickCount () % 100000,
			receive_ack);
	}
#endif

	/* Check for length errors */

#if UBCSP_CRC
	if (receive_crc)
	{
		/* If this packet had a CRC, then the length of the payload 
		   should be 2 less than the received size of the payload */

		if (length + 2 != ubcsp_config.receive_index)
		{
			/* Slip Length Error */

#if SHOW_PACKET_ERRORS
			printf ("\n######################## Slip Length Error (With CRC) %d,%d\n", length, ubcsp_config.receive_index - 2);
#endif

			/* If we have a payload length error, send an ack in return
			   this gets a packet to be resent as quickly as possible */

			ubcsp_config.send_ack = 1;
			return activity;
		}

		/* We have a CRC at the end of this packet */

		ubcsp_config.receive_index -= 2;

		/* Calculate the packet CRC */

		crc = 0xffff;

		/* CRC the packet header */

		for (loop = 0; loop < 4; loop ++)
		{
			crc = ubcsp_calc_crc (ubcsp_receive_header[loop], crc);
		}

		/* CRC the packet payload - without the CRC bytes */

		for (loop = 0; loop < ubcsp_config.receive_index; loop ++)
		{
			crc = ubcsp_calc_crc (ubcsp_config.receive_packet->payload[loop], crc);
		}

		/* Reverse the CRC */

		crc = ubcsp_crc_reverse (crc);

		/* Check the CRC is correct */

		if
		(
			(((crc & 0xff00) >> 8) != ubcsp_config.receive_packet->payload[ubcsp_config.receive_index]) ||
			((crc & 0xff) != ubcsp_config.receive_packet->payload[ubcsp_config.receive_index + 1])
		)
		{
#if SHOW_PACKET_ERRORS
			printf ("\n######################## CRC Error\n");
#endif

			/* If we have a packet crc error, send an ack in return
			   this gets a packet to be resent as quickly as possible */

			ubcsp_config.send_ack = 1;
			return activity;
		}
	}
	else
	{
#endif
		/* No CRC present, so just check the length of payload with that received */

		if (length != ubcsp_config.receive_index)
		{
			/* Slip Length Error */

#if SHOW_PACKET_ERRORS
			printf ("\n######################## Slip Length Error (No CRC) %d,%d\n", length, ubcsp_config.receive_index);
#endif

			/* If we have a payload length error, send an ack in return
			   this gets a packet to be resent as quickly as possible */

			ubcsp_config.send_ack = 1;
			return activity;
		}
#if UBCSP_CRC
	}
#endif

	/*** We have a fully formed packet having passed all data integrity checks ***/

	/* Check if we have an ACK for the last packet we sent */

	if (receive_ack != ubcsp_config.sequence_number)
	{
		/* Since we only have a window size of 1, if the ACK is not equal to SEQ
		   then the packet was sent */

		if
		(
			(ubcsp_config.send_packet) &&
			(ubcsp_config.send_packet->reliable)
		)
		{
			/* We had sent a reliable packet, so clear this packet
			   Then increament the sequence number for the next packet */

			ubcsp_config.send_packet = 0;
			ubcsp_config.sequence_number ++;
			ubcsp_config.delay = 0;

			/* Notify the caller that we have SENT a packet */

			activity |= UBCSP_PACKET_SENT;
		}
	}

	/*** Now we can concentrate of the packet we have received ***/

	/* Check for Link Establishment packets */

	if (ubcsp_config.receive_packet->channel == 1)
	{
		/* Link Establishment */

		ubcsp_config.delay = 0;

		/* Find which link establishment packet this payload means
		   This could return 5, meaning none */

		switch (ubcsp_which_le_payload (ubcsp_config.receive_packet->payload))
		{
			case 0:
			{
				/* SYNC Recv'd */

#if SHOW_LE_STATES
				printf ("Recv SYNC\n");
#endif

				/* If we receive a SYNC, then we respond to it with a SYNC RESP
				   but only if we are not active.
				   If we are active, then we have a PEER RESET */

				if (ubcsp_config.link_establishment_state < ubcsp_le_active)
				{
					ubcsp_config.link_establishment_resp = 1;
				}
				else
				{
					/* Peer reset !!!! */

#if SHOW_LE_STATES
					printf ("\n\n\n\n\nPEER RESET\n\n");
#endif

					/* Reinitialize the link */

					ubcsp_initialize ();

					/* Tell the host what has happened */

					return UBCSP_PEER_RESET;
				}
				break;
			}

			case 1:
			{
				/* SYNC RESP Recv'd */

#if SHOW_LE_STATES
				printf ("Recv SYNC RESP\n");
#endif

				/* If we receive a SYNC RESP, push us into the initialized state */

				if (ubcsp_config.link_establishment_state < ubcsp_le_initialized)
				{
#if SHOW_LE_STATES
					printf ("Link Initialized\n");
#endif
					ubcsp_config.link_establishment_state = ubcsp_le_initialized;
				}

				break;
			}

			case 2:
			{
				/* CONF Recv'd */

#if SHOW_LE_STATES
				printf ("Recv CONF\n");
#endif

				/* If we receive a CONF, and we are initialized or active
				   then respond with a CONF RESP */

				if (ubcsp_config.link_establishment_state >= ubcsp_le_initialized)
				{
					ubcsp_config.link_establishment_resp = 2;
				}

				break;
			}

			case 3:
			{
				/* CONF RESP Recv'd */

#if SHOW_LE_STATES
				printf ("Recv CONF RESP\n");
#endif

				/* If we received a CONF RESP, then push us into the active state */

				if (ubcsp_config.link_establishment_state < ubcsp_le_active)
				{
#if SHOW_LE_STATES
					printf ("Link Active\n");
#endif

					ubcsp_config.link_establishment_state = ubcsp_le_active;
					ubcsp_config.send_size = 0;

					return activity | UBCSP_PACKET_SENT;
				}

				break;
			}
		}

		/* We have finished processing Link Establishment packets */
	}
	else if (ubcsp_config.receive_index)
	{
		/* We have some payload data we need to process
		   but only if we are active - otherwise, we just ignore it */

		if (ubcsp_config.link_establishment_state == ubcsp_le_active)
		{
			if (ubcsp_config.receive_packet->reliable)
			{
				/* If the packet we've just received was reliable
				   then send an ACK */

				ubcsp_config.send_ack = 1;

				/* We the sequence number we received is the same as 
				   the last ACK we sent, then we have received a packet in sequence */

				if (receive_seq == ubcsp_config.ack_number)
				{
					/* Increase the ACK number - which will be sent in the next ACK 
					   or normal packet we send */

					ubcsp_config.ack_number ++;

					/* Set the values in the receive_packet structure, so the caller
					   knows how much data we have */

					ubcsp_config.receive_packet->length = length;
					ubcsp_config.receive_packet = 0;

					/* Tell the caller that we have received a packet, and that it
					   will be ACK'ed */

					activity |= UBCSP_PACKET_RECEIVED | UBCSP_PACKET_ACK;
				}
			}
			else 
			{
				/* Set the values in the receive_packet structure, so the caller
				   knows how much data we have */

				ubcsp_config.receive_packet->length = length;
				ubcsp_config.receive_packet = 0;

				/* Tell the caller that we have received a packet */

				activity |= UBCSP_PACKET_RECEIVED;
			}
		}
	}

	/* Just return any activity that occurred */

	return activity;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_setup_packet                                                      **/
/**                                                                         **/
/** This function is called to setup a packet to be sent                    **/
/** This allows just a header, or a header and payload to be sent           **/
/** It also allows the header checksum to be precalcuated                   **/
/** or calculated here                                                      **/
/** part1 is always 4 bytes                                                 **/
/**                                                                         **/
/*****************************************************************************/

static void ubcsp_setup_packet (uint8 *part1, uint8 calc, uint8 *part2, uint16 len2)
{
	/* If we need to calculate the checksum, do that now */

	if (calc)
	{
		part1[3] =
			~(part1[0] + part1[1] + part1[2]);
	}

	/* Setup the header send pointer and size so we can clock this out */

	ubcsp_config.send_ptr = part1;
	ubcsp_config.send_size = 4;

	/* Setup the payload send pointer and size */

	ubcsp_config.next_send_ptr = part2;
	ubcsp_config.next_send_size = len2;

#if UBCSP_CRC
	/* Initialize the crc as required */

	ubcsp_config.send_crc = -1;

	ubcsp_config.need_send_crc = 1;
#endif
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_sent_packet                                                       **/
/**                                                                         **/
/** Called when we have finished sending a packet                           **/
/** If this packet was unreliable, then notify caller, and clear the data   **/
/**                                                                         **/
/*****************************************************************************/

static uint8 ubcsp_sent_packet (void)
{
	if (ubcsp_config.send_packet)
	{
		if (!ubcsp_config.send_packet->reliable)
		{
			/* We had a packet sent that was unreliable */

			/* Forget about this packet */

			ubcsp_config.send_packet = 0;

			/* Notify caller that they can send another one */

			return UBCSP_PACKET_SENT;
		}
	}

	/* We didn't have a packet, or it was reliable
	   Must wait for ACK before allowing another packet to be sent */

	return 0;
}

/*****************************************************************************/
/**                                                                         **/
/** ubcsp_poll                                                              **/
/**                                                                         **/
/** This is the main function for ubcsp                                     **/
/** It performs a number of tasks                                           **/
/**                                                                         **/
/** 1) Send another octet to the UART - escaping as required                **/
/** 2) Setup the payload to be sent after the header has been sent          **/
/** 3) Send the CRC for the packet if required                              **/
/**                                                                         **/
/** 4) Calculate the next Link Establishment State                          **/
/** 5) Send a Link Establishment packet                                     **/
/** 6) Send a normal packet if available                                    **/
/** 7) Send an ACK packet if required                                       **/
/**                                                                         **/
/** 8) Receive octets from UART and deslip them as required                 **/
/** 9) Place received octets into receive header or receive payload buffer  **/
/** 10) Process received packet when SLIP_END is received                   **/
/**                                                                         **/
/** 11) Keep track of ability of caller to delay recalling                  **/
/**                                                                         **/
/*****************************************************************************/

uint8 ubcsp_poll (uint8 *activity)
{
	uint8
		delay = UBCSP_POLL_TIME_IMMEDIATE;

	uint8
		value;

	/* Assume no activity to start with */

	*activity = 0;

	/* If we don't have to delay, then send something if we can */

	if (!ubcsp_config.delay)
	{
		/* Do we have something we are sending to send */

		if (ubcsp_config.send_size)
		{
			/* We have something to send so send it */

			if (ubcsp_config.send_slip_escape)
			{
				/* Last time we send a SLIP_ESCAPE octet
				   this time send the second escape code */

				put_uart (ubcsp_config.send_slip_escape);

				ubcsp_config.send_slip_escape = 0;
			}
			else
			{
#if UBCSP_CRC
				/* get the value to send, and calculate CRC as we go */

				value = *ubcsp_config.send_ptr ++;

				ubcsp_config.send_crc = ubcsp_calc_crc (value, ubcsp_config.send_crc);

				/* Output the octet */

				ubcsp_put_slip_uart (value);
#else
				/* Just output the octet*/

				ubcsp_put_slip_uart (*ubcsp_config.send_ptr ++);
#endif
			}

			/* If we did output a SLIP_ESCAPE, then don't process the end of a block */

			if ((!ubcsp_config.send_slip_escape) && ((ubcsp_config.send_size = ubcsp_config.send_size - 1) == 0))
			{
				/*** We are at the end of a block - either header or payload ***/

				/* setup the next block */

				ubcsp_config.send_ptr = ubcsp_config.next_send_ptr;
				ubcsp_config.send_size = ubcsp_config.next_send_size;
				ubcsp_config.next_send_ptr = 0;
				ubcsp_config.next_send_size = 0;

#if UBCSP_CRC
				/* If we have no successor block
				   then we might need to send the CRC */

				if (!ubcsp_config.send_ptr)
				{
					if (ubcsp_config.need_send_crc)
					{
						/* reverse the CRC from what we computed along the way */

						ubcsp_config.need_send_crc = 0;

						ubcsp_config.send_crc = ubcsp_crc_reverse (ubcsp_config.send_crc);

						/* Save in the send_crc buffer */

						ubcsp_send_crc[0] = (uint8) (ubcsp_config.send_crc >> 8);
						ubcsp_send_crc[1] = (uint8) ubcsp_config.send_crc;

						/* Setup to send this buffer */

						ubcsp_config.send_ptr = ubcsp_send_crc;
						ubcsp_config.send_size = 2;
					}
					else
					{
						/* We don't need to send the crc
						   either we just have, or this packet doesn't include it */

						/* Output the end of FRAME marker */

						put_uart (SLIP_FRAME);

						/* Check if this is an unreliable packet */

						*activity |= ubcsp_sent_packet ();

						/* We've sent the packet, so don't need to have be called quickly soon */

						delay = UBCSP_POLL_TIME_DELAY;
					}
				}
#else
				/* If we have no successor block
				   then we might need to send the CRC */

				if (!ubcsp_config.send_ptr)
				{
					/* Output the end of FRAME marker */

					put_uart (SLIP_FRAME);

					/* Check if this is an unreliable packet */

					*activity |= ubcsp_sent_packet ();

					/* We've sent the packet, so don't need to have be called quickly soon */

					delay = UBCSP_POLL_TIME_DELAY;
				}
#endif
			}
		}
		else if (ubcsp_config.link_establishment_packet == ubcsp_le_none)
		{
			/* We didn't have something to send
			   AND we have no Link Establishment packet to send */

			if (ubcsp_config.link_establishment_resp & 2)
			{
				/* Send the start of FRAME packet */

				put_uart (SLIP_FRAME);

				/* We did require a RESP packet - so setup the send */

				ubcsp_setup_packet ((uint8*) ubcsp_send_le_header, 0, (uint8*) ubcsp_le_buffer[ubcsp_le_conf_resp], 4);

				/* We have now "sent" this packet */

				ubcsp_config.link_establishment_resp = 0;
			}
			else if (ubcsp_config.send_packet)
			{
				/* There is a packet ready to be sent */

				/* Send the start of FRAME packet */

				put_uart (SLIP_FRAME);

				/* Encode up the packet header using ACK and SEQ numbers */

				ubcsp_send_header[0] =
					(ubcsp_config.send_packet->reliable << 7) |
#if UBCSP_CRC
					0x40 |	/* Always use CRC's */
#endif
					(ubcsp_config.ack_number << 3) | 
					(ubcsp_config.sequence_number);

				/* Encode up the packet header's channel and length */
				ubcsp_send_header[1] =
					(ubcsp_config.send_packet->channel & 0x0f) |
					((ubcsp_config.send_packet->length << 4) & 0xf0);

				ubcsp_send_header[2] =
					(ubcsp_config.send_packet->length >> 4) & 0xff;

				/* Let the ubcsp_setup_packet function calculate the header checksum */

				ubcsp_setup_packet ((uint8*) ubcsp_send_header, 1, ubcsp_config.send_packet->payload, ubcsp_config.send_packet->length);

				/* Don't need to send an ACK - we just place on in this packet */

				ubcsp_config.send_ack = 0;
				
#if SHOW_PACKET_ERRORS
				printf (" : %10d Send %d Ack %d\n",
					GetTickCount () % 100000,
					ubcsp_config.sequence_number,
					ubcsp_config.ack_number);
#endif
			}
			else if (ubcsp_config.send_ack)
			{
				/* Send the start of FRAME packet */

				put_uart (SLIP_FRAME);

#if SHOW_PACKET_ERRORS
				printf (" : %10d Send ACK %d\n",
					GetTickCount () % 100000,
					ubcsp_config.ack_number);
#endif

				/* The ack packet is already computed apart from the first octet */

				ubcsp_send_ack_header[0] =
#if UBCSP_CRC
					0x40 | 
#endif
					(ubcsp_config.ack_number << 3);

				/* Let the ubcsp_setup_packet function calculate the header checksum */

				ubcsp_setup_packet (ubcsp_send_ack_header, 1, 0, 0);

				/* We've now sent the ack */

				ubcsp_config.send_ack = 0;
			}
			else
			{
				/* We didn't have a Link Establishment response packet,
				   a normal packet or an ACK packet to send */

				delay = UBCSP_POLL_TIME_DELAY;
			}
		}
		else
		{
#if SHOW_PACKET_ERRORS
//			printf (" : %10d Send LE %d\n",
//				GetTickCount () % 100000,
//				ubcsp_config.link_establishment_packet);
#endif

			/* Send A Link Establishment Message */

			put_uart (SLIP_FRAME);

			/* Send the Link Establishment header followed by the 
			   Link Establishment packet */

			ubcsp_setup_packet ((uint8*) ubcsp_send_le_header, 0, (uint8*) ubcsp_le_buffer[ubcsp_config.link_establishment_packet], 4);

			/* start sending immediately */

			ubcsp_config.delay = 0;

			/* workout what the next link establishment packet should be */

			ubcsp_config.link_establishment_packet = next_le_packet[ubcsp_config.link_establishment_state + ubcsp_config.link_establishment_resp * 4];

			/* We have now delt with any response packet that we needed */

			ubcsp_config.link_establishment_resp = 0;

			return 0;
		}
	}

	/* We now need to receive any octets from the UART */

	while ((ubcsp_config.receive_packet) && (get_uart (&value)))
	{
		/* If the last octet was SLIP_ESCAPE, then special processing is required */

		if (ubcsp_config.receive_slip_escape)
		{
			/* WARNING - out of range values are not detected !!!
			   This will probably be caught with the checksum or CRC check */

			value = ubcsp_deslip[value - SLIP_ESCAPE_FRAME];

			ubcsp_config.receive_slip_escape = 0;
		}
		else
		{
			/* Check for the SLIP_FRAME octet - must be start or end of packet */
			if (value == SLIP_FRAME)
			{
				/* If we had a full header then we have a packet */

				if (ubcsp_config.receive_index >= 0)
				{
					/* process the received packet */

					*activity |= ubcsp_recevied_packet ();

					if (*activity & UBCSP_PACKET_ACK)
					{
						/* We need to ACK this packet, then don't delay its sending */
						ubcsp_config.delay = 0;
					}
				}

				/* Setup to receive the next packet */

				ubcsp_config.receive_index = -4;

				/* Ok, next octet */

				goto finished_receive;
			}
			else if (value == SLIP_ESCAPE)
			{
				/* If we receive a SLIP_ESCAPE,
				   then remember to process the next special octet */

				ubcsp_config.receive_slip_escape = 1;

				goto finished_receive;
			}
		}

		if (ubcsp_config.receive_index < 0)
		{
			/* We are still receiving the header */

			ubcsp_receive_header[ubcsp_config.receive_index + 4] = value;

			ubcsp_config.receive_index ++;
		}
		else if (ubcsp_config.receive_index < ubcsp_config.receive_packet->length)
		{
			/* We are receiving the payload */
			/* We might stop coming here if we are receiving a
			   packet which is longer than the receive_packet->length
			   given by the host */

			ubcsp_config.receive_packet->payload[ubcsp_config.receive_index] = value;

			ubcsp_config.receive_index ++;
		}

finished_receive:
		{
		}
	}

	if (ubcsp_config.delay > 0)
	{
		/* We were delayed so delay some more
		   this could be cancelled if we received something */

		ubcsp_config.delay --;
	}
	else
	{
		/* We had no delay, so use the delay we just decided to us */

		ubcsp_config.delay = delay;
	}

	/* Report the current delay to the user */

	return ubcsp_config.delay;
}
