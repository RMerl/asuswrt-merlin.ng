/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * NVRAM variable manipulation
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id: bcmnvram.h,v 1.1 2001/10/31 18:49:25 mpl Exp $
 */

#ifndef _bcmnvram_h_
#define _bcmnvram_h_

#ifndef _LANGUAGE_ASSEMBLY

struct nvram_header {
	unsigned long magic;
	unsigned long len;
	unsigned long crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:27 init, mem. test 28, 29-31 reserved */
	unsigned long config_refresh;	/* 0:15 config, 16:31 refresh */
	unsigned long reserved;
};

struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};

/* Compatibility */
typedef struct nvram_tuple EnvRec;

/*
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NULL if undefined
 */
extern char * nvram_get(const char *name);

/* 
 * Get the value of an NVRAM variable
 * @param	name	name of variable to get
 * @return	value of variable or NUL if undefined
 */
#define nvram_safe_get(name) (nvram_get(name) ? : "")

/*
 * Match an NVRAM variable
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal to match or FALSE otherwise
 */
#define nvram_match(name, match) ({ \
	const char *value = nvram_get(name); \
	(value && !strcmp(value, match)); \
})

/*
 * Match an NVRAM variable
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string equal to invmatch or FALSE otherwise
 */
#define nvram_invmatch(name, invmatch) ({ \
	const char *value = nvram_get(name); \
	(value && strcmp(value, invmatch)); \
})

/*
 * Set the value of an NVRAM variable
 * @param	name	name of variable to set
 * @param	value	value of variable
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int nvram_set(const char *name, const char *value);

/*
 * Unset an NVRAM variable
 * @param	name	name of variable to unset
 * @return	0 on success and errno on failure
 * NOTE: use nvram_commit to commit this change to flash.
 */
extern int nvram_unset(const char *name);

/*
 * Permanently commit NVRAM variables
 * @return	0 on success and errno on failure
 */
extern int nvram_commit(void);

/*
 * Get all NVRAM variables (format name=value\0 ... \0\0)
 * @param	buf	buffer to store variables
 * @param	count	size of buffer in bytes
 * @return	0 on success and errno on failure
 */
extern int nvram_getall(char *buf, int count);

/*
 * Invalidate the current NVRAM header
 * @return	0 on success and errno on failure
 */
extern int nvram_invalidate(void);

#endif /* _LANGUAGE_ASSEMBLY */

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
#define NVRAM_VERSION		1
#define NVRAM_HEADER_SIZE	20
#define NVRAM_FIRST_LOC		0xbfcf8000
#define NVRAM_LAST_LOC		0xbfff8000
#define NVRAM_LOC_GAP		0x100000
#define NVRAM_SPACE		0x8000

#endif /* _bcmnvram_h_ */
