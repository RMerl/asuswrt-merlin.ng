/*
 *  zcrypt 2.1.0
 *
 *  Copyright IBM Corp. 2001, 2012
 *  Author(s): Robert Burroughs
 *	       Eric Rossman (edrossma@us.ibm.com)
 *
 *  Hotplug & misc device support: Jochen Roehrig (roehrig@de.ibm.com)
 *  Major cleanup & driver split: Martin Schwidefsky <schwidefsky@de.ibm.com>
 *  MSGTYPE restruct:		  Holger Dengler <hd@linux.vnet.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _ZCRYPT_MSGTYPE6_H_
#define _ZCRYPT_MSGTYPE6_H_

#include <asm/zcrypt.h>

#define MSGTYPE06_NAME			"zcrypt_msgtype6"
#define MSGTYPE06_VARIANT_DEFAULT	0
#define MSGTYPE06_VARIANT_NORNG		1
#define MSGTYPE06_VARIANT_EP11		2

#define MSGTYPE06_MAX_MSG_SIZE		(12*1024)

/**
 * The type 6 message family is associated with PCICC or PCIXCC cards.
 *
 * It contains a message header followed by a CPRB, both of which
 * are described below.
 *
 * Note that all reserved fields must be zeroes.
 */
struct type6_hdr {
	unsigned char reserved1;	/* 0x00				*/
	unsigned char type;		/* 0x06				*/
	unsigned char reserved2[2];	/* 0x0000			*/
	unsigned char right[4];		/* 0x00000000			*/
	unsigned char reserved3[2];	/* 0x0000			*/
	unsigned char reserved4[2];	/* 0x0000			*/
	unsigned char apfs[4];		/* 0x00000000			*/
	unsigned int  offset1;		/* 0x00000058 (offset to CPRB)	*/
	unsigned int  offset2;		/* 0x00000000			*/
	unsigned int  offset3;		/* 0x00000000			*/
	unsigned int  offset4;		/* 0x00000000			*/
	unsigned char agent_id[16];	/* PCICC:			*/
					/*    0x0100			*/
					/*    0x4343412d4150504c202020	*/
					/*    0x010101			*/
					/* PCIXCC:			*/
					/*    0x4341000000000000	*/
					/*    0x0000000000000000	*/
	unsigned char rqid[2];		/* rqid.  internal to 603	*/
	unsigned char reserved5[2];	/* 0x0000			*/
	unsigned char function_code[2];	/* for PKD, 0x5044 (ascii 'PD')	*/
	unsigned char reserved6[2];	/* 0x0000			*/
	unsigned int  ToCardLen1;	/* (request CPRB len + 3) & -4	*/
	unsigned int  ToCardLen2;	/* db len 0x00000000 for PKD	*/
	unsigned int  ToCardLen3;	/* 0x00000000			*/
	unsigned int  ToCardLen4;	/* 0x00000000			*/
	unsigned int  FromCardLen1;	/* response buffer length	*/
	unsigned int  FromCardLen2;	/* db len 0x00000000 for PKD	*/
	unsigned int  FromCardLen3;	/* 0x00000000			*/
	unsigned int  FromCardLen4;	/* 0x00000000			*/
} __packed;

/**
 * The type 86 message family is associated with PCICC and PCIXCC cards.
 *
 * It contains a message header followed by a CPRB.  The CPRB is
 * the same as the request CPRB, which is described above.
 *
 * If format is 1, an error condition exists and no data beyond
 * the 8-byte message header is of interest.
 *
 * The non-error message is shown below.
 *
 * Note that all reserved fields must be zeroes.
 */
struct type86_hdr {
	unsigned char reserved1;	/* 0x00				*/
	unsigned char type;		/* 0x86				*/
	unsigned char format;		/* 0x01 (error) or 0x02 (ok)	*/
	unsigned char reserved2;	/* 0x00				*/
	unsigned char reply_code;	/* reply code (see above)	*/
	unsigned char reserved3[3];	/* 0x000000			*/
} __packed;

#define TYPE86_RSP_CODE 0x86
#define TYPE87_RSP_CODE 0x87
#define TYPE86_FMT2	0x02

struct type86_fmt2_ext {
	unsigned char	  reserved[4];	/* 0x00000000			*/
	unsigned char	  apfs[4];	/* final status			*/
	unsigned int	  count1;	/* length of CPRB + parameters	*/
	unsigned int	  offset1;	/* offset to CPRB		*/
	unsigned int	  count2;	/* 0x00000000			*/
	unsigned int	  offset2;	/* db offset 0x00000000 for PKD	*/
	unsigned int	  count3;	/* 0x00000000			*/
	unsigned int	  offset3;	/* 0x00000000			*/
	unsigned int	  count4;	/* 0x00000000			*/
	unsigned int	  offset4;	/* 0x00000000			*/
} __packed;

/**
 * Prepare a type6 CPRB message for random number generation
 *
 * @ap_dev: AP device pointer
 * @ap_msg: pointer to AP message
 */
static inline void rng_type6CPRB_msgX(struct ap_device *ap_dev,
			       struct ap_message *ap_msg,
			       unsigned random_number_length)
{
	struct {
		struct type6_hdr hdr;
		struct CPRBX cprbx;
		char function_code[2];
		short int rule_length;
		char rule[8];
		short int verb_length;
		short int key_length;
	} __packed * msg = ap_msg->message;
	static struct type6_hdr static_type6_hdrX = {
		.type		= 0x06,
		.offset1	= 0x00000058,
		.agent_id	= {'C', 'A'},
		.function_code	= {'R', 'L'},
		.ToCardLen1	= sizeof(*msg) - sizeof(msg->hdr),
		.FromCardLen1	= sizeof(*msg) - sizeof(msg->hdr),
	};
	static struct CPRBX local_cprbx = {
		.cprb_len	= 0x00dc,
		.cprb_ver_id	= 0x02,
		.func_id	= {0x54, 0x32},
		.req_parml	= sizeof(*msg) - sizeof(msg->hdr) -
				  sizeof(msg->cprbx),
		.rpl_msgbl	= sizeof(*msg) - sizeof(msg->hdr),
	};

	msg->hdr = static_type6_hdrX;
	msg->hdr.FromCardLen2 = random_number_length,
	msg->cprbx = local_cprbx;
	msg->cprbx.rpl_datal = random_number_length,
	msg->cprbx.domain = AP_QID_QUEUE(ap_dev->qid);
	memcpy(msg->function_code, msg->hdr.function_code, 0x02);
	msg->rule_length = 0x0a;
	memcpy(msg->rule, "RANDOM  ", 8);
	msg->verb_length = 0x02;
	msg->key_length = 0x02;
	ap_msg->length = sizeof(*msg);
}

int zcrypt_msgtype6_init(void);
void zcrypt_msgtype6_exit(void);

#endif /* _ZCRYPT_MSGTYPE6_H_ */
