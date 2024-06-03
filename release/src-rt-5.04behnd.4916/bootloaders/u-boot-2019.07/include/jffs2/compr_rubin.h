/* Rubin encoder/decoder header       */
/* work started at   : aug   3, 1994  */
/* last modification : aug  15, 1994  */
/* $Id: compr_rubin.h,v 1.1 2002/01/16 23:34:32 nyet Exp $ */

#define RUBIN_REG_SIZE   16
#define UPPER_BIT_RUBIN    (((long) 1)<<(RUBIN_REG_SIZE-1))
#define LOWER_BITS_RUBIN   ((((long) 1)<<(RUBIN_REG_SIZE-1))-1)

void dynrubin_decompress(unsigned char *data_in, unsigned char *cpage_out,
		   unsigned long sourcelen, unsigned long dstlen);
