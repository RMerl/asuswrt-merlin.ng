/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*--------------------------------------------------------------------------
 *
 * Motorola S-Record Format:
 *
 * Motorola S-Records are an industry-standard format for
 * transmitting binary files to target systems and PROM
 * programmers. LSI Logic have extended this standard to include
 * an S4-record containing an address and a symbol.
 *
 * The extended S-record standard is as follows:
 *
 * S<type><length><address><data....><checksum>
 * S4<length><address><name>,<checksum>
 *
 * Where:
 *
 * type
 *     is the record type. Where:
 *
 *     0  starting record (optional)
 *     1  data record with 16-bit address
 *     2  data record with 24-bit address
 *     3  data record with 32-bit address
 *     4  symbol record (LSI extension)
 *     5  number of data records in preceding block
 *     6  unused
 *     7  ending record for S3 records
 *     8  ending record for S2 records
 *     9  ending record for S1 records
 *
 * length
 *     is two hex characters. This defines the length of the
 *     record in bytes (not characters). It includes the address
 *     field, the data field, and the checksum field.
 *
 * address
 *     is 4, 6, or 8 characters. Corresponding to a 16-, 24-, or
 *     32-bit address. The address field for S4 records is
 *     always 32 bits.
 *
 * data
 *
 *     Are the data bytes. Each pair of hex characters represent
 *     one byte in memory.
 *
 * name
 *     Is the symbol name. The symbol is terminated by a ','.
 *
 * checksum
 *     Is the one's complement of the 8-bit checksum.
 *
 * Example
 *
 * S0030000FC
 * .
 * .
 * S325000004403C0880018D08DD900000000011000026000000003C0880012508DC50C50000B401
 * S32500000460C50100B8C50200BCC50300C0C50400C4C50500C8C50600CCC50700D0C50800D4FA
 * S32500000480C50900D8C50A00DCC50B00E0C50C00E4C50D00E8C50E00ECC50F00F0C51000F49A
 * S325000004A0C51100F8C51200FCC5130100C5140104C5150108C516010CC5170110C518011434
 * .
 * .
 * S70500000000FA
 *
 * The S0 record starts the file. The S3 records contain the
 * data. The S7 record contains the entry address and terminates
 * the download.
 *
 *--------------------------------------------------------------------------
 */

#define SREC_START	0	/* Start Record (module name)		    */
#define SREC_DATA2	1	/* Data  Record with 2 byte address	    */
#define SREC_DATA3	2	/* Data  Record with 3 byte address	    */
#define SREC_DATA4	3	/* Data  Record with 4 byte address	    */
#define SREC_COUNT	5	/* Count Record (previously transmitted)    */
#define SREC_END4	7	/* End   Record with 4 byte start address   */
#define SREC_END3	8	/* End   Record with 3 byte start address   */
#define SREC_END2	9	/* End   Record with 2 byte start address   */
#define SREC_EMPTY	10	/* Empty Record without any data	    */

#define SREC_REC_OK  SREC_EMPTY /* last code without error condition	    */

#define SREC_E_BADTYPE	-1	/* no valid S-Record		            */
#define SREC_E_NOSREC	-2	/* line format differs from s-record	    */
#define SREC_E_BADCHKS	-3	/* checksum error in an s-record line	    */

#define SREC_MAXRECLEN	(512 + 4)   /* max ASCII record length		    */
#define SREC_MAXBINLEN	255	    /* resulting binary length		    */

int srec_decode (char *input, int *count, ulong *addr, char *data);
