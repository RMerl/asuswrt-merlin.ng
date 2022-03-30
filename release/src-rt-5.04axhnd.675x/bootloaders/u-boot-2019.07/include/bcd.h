/* Permission is hereby granted to copy, modify and redistribute this code
 * in terms of the GNU Library General Public License, Version 2 or later,
 * at your option.
 */

/* inline functions to translate to/from binary and binary-coded decimal
 * (frequently found in RTC chips).
 */

#ifndef _BCD_H
#define _BCD_H

static inline unsigned int bcd2bin(unsigned int val)
{
	return ((val) & 0x0f) + ((val & 0xff) >> 4) * 10;
}

static inline unsigned int bin2bcd(unsigned int val)
{
	return (((val / 10) << 4) | (val % 10));
}

#endif /* _BCD_H */
