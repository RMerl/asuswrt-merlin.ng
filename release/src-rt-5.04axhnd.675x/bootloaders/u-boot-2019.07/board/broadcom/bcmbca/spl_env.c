/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <environment.h>
#include <hexdump.h>
#include <stdlib.h>
#include <string.h>
#include "bca_common.h"
#include "spl_env.h"

/**
 * find_spl_env_val - get pointer to named environment value 
 * @buffer: pointer to the magic number of the, already loaded and validated, environment buffer
 * @name: name of the environment variable to find
 *
 * returns:
 *     pointer to the first character after the NAME= string in the environment if found
 *     NULL if not found
 */
char *find_spl_env_val(const char *buffer, const char *name)
{
	uint32_t len;
	env_t *ep;
	int nmlen;
	uint32_t *d = (uint32_t *) buffer;
	if (buffer == NULL) {
		return NULL;
	}
	len = d[1];

	nmlen = strlen(name);
#if BCM_DEBUG_SPLENV
	printf("look for env %s\n", name);
#endif /* BCM_DEBUG_SPLENV */
	ep = (env_t *) (buffer + 8);
	char *c;
	char previous = '\0';
	c = (char *)&ep->data[0];
	while ((c < (char *)&ep->data[len - 4])
	       && ((c[0] != '\0') || (c[1] != '\0'))) {
		if ((previous == '\0')
		    && (0 == strncmp(name, (char *)c, nmlen))
		    && (c[nmlen] == '=')) {
			c = c + nmlen + 1;
#if BCM_DEBUG_SPLENV
			{
				int i;
				i = strlen(c);
				if (i > 80) {
					printf("found %d byte value\n",i);
				} else {
					printf("found %s\n", c);
				}
			}
#endif /* BCM_DEBUG_SPLENV */
			return c;
		}
		previous = c[0];
		c++;
	}
	return NULL;
}

/**
 * get_spl_env_val - get named environment value 
 * @buffer: pointer to the magic number of the, already loaded and validated, environment buffer
 * @name: name of the environment variable to find
 * @out: pointer to buffer to which data will be copied
 * @maxlen: max bytes to copy to the output buffer
 *
 * returns:
 *     0 if successful
 *     1 if not found
 */
int get_spl_env_val(const char *buffer, const char *name, char *out, int maxlen)
{
	char *c;
	c = find_spl_env_val(buffer, name);
	if (NULL != c) {
		strncpy(out, c, maxlen);
		return (0);
	}
	return 1;
}

int validate_metadata(char *cp, int *valid, int *committed, int *seq)
{
	uint32_t *d, crc, got;
	env_t *ep;
	char buf[16];
	d = (uint32_t *) cp;
	ep = (env_t *) & d[2];
	memcpy(&crc, &ep->crc, sizeof(crc));
	got = crc32(0, ep->data, (d[1] - 4) & 0xffff);
	if (got == crc) {
		unsigned long iargs[3];
		int n, i;
		n = parse_env_nums(find_spl_env_val
				   (cp, "VALID"), 3, iargs, NULL);
		for (i = 0; i < n; i++) {
			if ((iargs[i] >= 1) && (iargs[i] <= 2)) {
				valid[iargs[i] - 1] = iargs[i];
			}
		}
		if (0 == get_spl_env_val(cp, "COMMITTED", buf, 10)) {
			*committed = (buf[0] == '1') ? 1 : (buf[0] == '2') ? 2 : 0;
		}

		// preset the sequence numbers incase the field doesn't exist
		if ((*committed == 1) && valid[0])
		{
			seq[0] = 1;
			if (valid[1])
				seq[1] = 0;
		}
		else if ((*committed == 2) && valid[1])
		{
			seq[1] = 1;
			if (valid[0])
				seq[0] = 0;
		}

		n = parse_env_nums(find_spl_env_val
				   (cp, "SEQ"), 3, iargs, NULL);
		for (i = 0; i < n; i++) {
			seq[i] = iargs[i];
		}

		return (0);
	}
#if 0
	printf("at %p for %d bytes expected %x got %x\n",&ep->data, d[0]-4, crc,got);
        print_hex_dump_bytes("vdate: ", DUMP_PREFIX_ADDRESS, cp, 256+32);
	got = crc32(0, ep->data, (d[0] - 4) & 0xffff);
	printf("just for grins %x\n",got);
#endif
	/* Error out all fields to indicate no committed and valid images */
	valid[0] = 0;
	valid[1] = 0;
	*committed = 0;
	seq[0] = -1;
	seq[1] = -1;
	return (1);
}

/* UBOOT has multiple conflicting CRC32 implementations, so share the one that works */
uint32_t the_env_crc32(int a, void *b, int len) {
	return(crc32(a,b,len));
}

