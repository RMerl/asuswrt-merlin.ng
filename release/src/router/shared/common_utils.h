/*
 * Common utility functions across router code
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: common_utils.h 471610 2014-04-21 09:56:36Z $
 */

#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

/* --------------------------------- Constatnts & Macros  --------------------------------- */
#ifndef min
#define min(a, b)	(((a) < (b)) ? (a) : (b))
#endif // endif

/* General Strings */
#define EMPTY_STR			""
#define NULL_CH				'\0'
#define OCTET				"%02X"
#define PREFIX_DEF			"wlXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX_"
#define MAC_FORMAT			"%02x:%02x:%02x:%02x:%02x:%02x"
/* --------------------------------- Constatnts & Macros  --------------------------------- */

/*
 * Get the timestamp in miliseconds to do timing diagnosys
 * @return	current time in miliseconds
 */
extern unsigned long long getTimestamp(void);

/*
 * Append Number (<256) to a Hex Strings as Octets
 * @param	octetstr		destination Hex String
 * @param	octetstr_len	destination String length
 * @param	num		Number (<256) (Octet)
 * @return			void
 */
extern void append_numto_hexStr(char octetstr[], int octetstr_len, int num);

/*
 * Search an Octet from a list/array of Octets linearly, and return 1 if exists
 * @param	haystack		Array of Octets
 * @param	size		Number of entries in haystack
 * @param	needle		Octect to search
 * @return			if duplicate return 1, else 0
 */
extern int is_duplicate_octet(uint8* haystack, size_t size, uint8 needle);

/*
 * Verify an Octet Range is overlapping from a list of Octets ranges linearly
 * @param	haystack		Array of Octet ranges linearly [low1,high1,low2,high2]
 * @param	size		Number of entries in haystack
 * @param	low_needle	low value in range to search
 * @param	high_needle	high value in range to search
 * @return			if range is overlapping return 1, else 0
 */
extern int is_octet_range_overlapping(uint8* haystack, size_t size,
	uint8 low_needle, uint8 high_needle);

/*
 * Safe string copy function which takes care of overflow & NULL for last char
 * @param	destination	destination char buffer
 * @param	source		source char buffer
 * @param	num		size of destination buffer
 * @return			destination char ptr is returned
 */
extern void bytes_to_hex(uchar* str, int strbuflen, uchar* utf8, int utf8buflen);

/*
 * Safe string copy function which takes care of overflow & NULL for last char
 * @param	destination	destination char buffer
 * @param	source		source char buffer
 * @param	num		size of destination buffer
 * @return			destination char ptr is returned
 */
extern void hex_to_bytes(uchar* str, int strbuflen, uchar* utf8, int utf8buflen);

/*
 * Safe string copy function which takes care of overflow & NULL for last char
 * @param	destination	destination char buffer
 * @param	source		source char buffer
 * @param	num		size of destination buffer
 * @return			destination char ptr is returned
 */
extern int get_hex_data(uchar *data_str, uchar *hex_data, int len);

/*
 * Copy source string to destination string by removing old data & allocating new heap memory
 * @param	string		destination string pointer to be allocated and to be re-filled
 * @param	newstring		source constant string to be copied
 * @return			1 sucess, 0 failure
 */
extern int reallocate_string(char** string, const char* newstring);

/*
 * Safe string copy function which takes care of overflow & NULL for last char
 * @param	destination	destination char buffer
 * @param	source		source char buffer
 * @param	num		size of destination buffer
 * @return			destination char ptr is returned
 */
extern void* memmem(const void *l, size_t l_len, const void *s, size_t s_len);

/*
 * Safe string copy function which takes care of overflow & NULL for last char
 * @param	destination	destination char buffer
 * @param	source		source char buffer
 * @param	num		size of destination buffer
 * @return			destination char ptr is returned
 */
extern char* strncpy_n(char *destination, const char *source, size_t num);

extern void dm_register_app_restart_info(int pid, int argc, char **argv,
		char *dependent_services);
extern void dm_unregister_app_restart_info(int pid);
#endif /* _COMMON_UTILS_H_ */
