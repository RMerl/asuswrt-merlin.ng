// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <s_record.h>

static int hex1_bin (char  c);
static int hex2_bin (char *s);

int srec_decode (char *input, int *count, ulong *addr, char *data)
{
	int	i;
	int	v;				/* conversion buffer	*/
	int	srec_type;			/* S-Record type	*/
	unsigned char chksum;			/* buffer for checksum	*/

	chksum = 0;

	/* skip anything before 'S', and the 'S' itself.
	 * Return error if not found
	 */

	for (; *input; ++input) {
		if (*input == 'S') {		/* skip 'S' */
			++input;
			break;
		}
	}
	if (*input == '\0') {			/* no more data?	*/
		return (SREC_EMPTY);
	}

	v = *input++;				/* record type		*/

	if ((*count = hex2_bin(input)) < 0) {
		return (SREC_E_NOSREC);
	}

	chksum += *count;
	input  += 2;

	switch (v) {				/* record type		*/

	case '0':				/* start record		*/
		srec_type = SREC_START;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	case '1':
		srec_type = SREC_DATA2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	case '2':
		srec_type = SREC_DATA3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		break;
	case '3':				/* data record with a	*/
		srec_type = SREC_DATA4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		break;
/***	case '4'  ***/
	case '5':			/* count record, addr field contains */
		srec_type = SREC_COUNT;	/* a 2 byte record counter	*/
		*count    = 0;			/* no data		*/
		break;
/***	case '6' -- not used  ***/
	case '7':				/* end record with a	*/
		srec_type = SREC_END4;		/* 4 byte addr field	*/
		*count   -= 5;			/* - checksum and addr	*/
		break;
	case '8':				/* end record with a	*/
		srec_type = SREC_END3;		/* 3 byte addr field	*/
		*count   -= 4;			/* - checksum and addr	*/
		break;
	case '9':				/* end record with a	*/
		srec_type = SREC_END2;		/* 2 byte addr field	*/
		*count   -= 3;			/* - checksum and addr	*/
		break;
	default:
		return (SREC_E_BADTYPE);
	}

	/* read address field */
	*addr = 0;

	switch (v) {
	case '3':				/* 4 byte addr field	*/
	case '7':
		if ((v = hex2_bin(input)) < 0) {
			return (SREC_E_NOSREC);
		}
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '2':				/* 3 byte addr field	*/
	case '8':
		if ((v = hex2_bin(input)) < 0) {
			return (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;
		/* FALL THRU */
	case '0':				/* 2 byte addr field	*/
	case '1':
	case '5':
	case '9':
		if ((v = hex2_bin(input)) < 0) {
			return (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		if ((v = hex2_bin(input)) < 0) {
			return (SREC_E_NOSREC);
		}
		*addr <<= 8;
		*addr  += v;
		chksum += v;
		input  += 2;

		break;
	default:
		return (SREC_E_BADTYPE);
	}

	/* convert data and calculate checksum */
	for (i=0; i < *count; ++i) {
		if ((v = hex2_bin(input)) < 0) {
			return (SREC_E_NOSREC);
		}
		data[i] = v;
		chksum += v;
		input  += 2;
	}

	/* read anc check checksum */
	if ((v = hex2_bin(input)) < 0) {
		return (SREC_E_NOSREC);
	}

	if ((unsigned char)v != (unsigned char)~chksum) {
		return (SREC_E_BADCHKS);
	}

	return (srec_type);
}

static int hex1_bin (char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c + 10 - 'a');
	if (c >= 'A' && c <= 'F')
		return (c + 10 - 'A');
	return (-1);
}

static int hex2_bin (char *s)
{
	int i, j;

	if ((i = hex1_bin(*s++)) < 0) {
		return (-1);
	}
	if ((j = hex1_bin(*s)) < 0) {
		return (-1);
	}

	return ((i<<4) + j);
}
