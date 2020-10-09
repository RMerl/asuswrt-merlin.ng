/*
 * ljs.c			- List the contents of an journal superblock
 *
 * Copyright (C) 1995, 1996, 1997  Theodore Ts'o <tytso@mit.edu>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */


#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "e2p.h"
#include "ext2fs/kernel-jbd.h"

#ifdef WORDS_BIGENDIAN
#define e2p_be32(x) (x)
#else
static __u32 e2p_swab32(__u32 val)
{
	return ((val>>24) | ((val>>8)&0xFF00) |
		((val<<8)&0xFF0000) | (val<<24));
}

#define e2p_be32(x) e2p_swab32(x)
#endif

static const char *journal_checksum_type_str(__u8 type)
{
	switch (type) {
	case JBD2_CRC32C_CHKSUM:
		return "crc32c";
	default:
		return "unknown";
	}
}

void e2p_list_journal_super(FILE *f, char *journal_sb_buf,
			    int exp_block_size, int flags)
{
	journal_superblock_t *jsb = (journal_superblock_t *) journal_sb_buf;
	__u32 *mask_ptr, mask, m;
	unsigned int size;
	int j, printed = 0;
	unsigned int i, nr_users;

	fprintf(f, "%s", "Journal features:        ");
	for (i=0, mask_ptr=&jsb->s_feature_compat; i <3; i++,mask_ptr++) {
		mask = e2p_be32(*mask_ptr);
		for (j=0,m=1; j < 32; j++, m<<=1) {
			if (mask & m) {
				fprintf(f, " %s", e2p_jrnl_feature2string(i, m));
				printed++;
			}
		}
	}
	if (printed == 0)
		fprintf(f, " (none)");
	fputc('\n', f);
	fputs("Journal size:             ", f);
	size = (ntohl(jsb->s_blocksize) / 1024) * ntohl(jsb->s_maxlen);
	if (size < 8192)
		fprintf(f, "%uk\n", size);
	else
		fprintf(f, "%uM\n", size >> 10);
	nr_users = (unsigned int) ntohl(jsb->s_nr_users);
	if (exp_block_size != (int) ntohl(jsb->s_blocksize))
		fprintf(f, "Journal block size:       %u\n",
			(unsigned int)ntohl(jsb->s_blocksize));
	fprintf(f, "Journal length:           %u\n",
		(unsigned int)ntohl(jsb->s_maxlen));
	if (ntohl(jsb->s_first) != 1)
		fprintf(f, "Journal first block:      %u\n",
			(unsigned int)ntohl(jsb->s_first));
	fprintf(f, "Journal sequence:         0x%08x\n"
		"Journal start:            %u\n",
		(unsigned int)ntohl(jsb->s_sequence),
		(unsigned int)ntohl(jsb->s_start));
	if (nr_users != 1)
		fprintf(f, "Journal number of users:  %u\n", nr_users);
	if (jsb->s_feature_compat & e2p_be32(JFS_FEATURE_COMPAT_CHECKSUM))
		fprintf(f, "%s", "Journal checksum type:    crc32\n");
	if ((jsb->s_feature_incompat &
	     e2p_be32(JFS_FEATURE_INCOMPAT_CSUM_V3)) ||
	    (jsb->s_feature_incompat &
	     e2p_be32(JFS_FEATURE_INCOMPAT_CSUM_V2)))
		fprintf(f, "Journal checksum type:    %s\n"
			"Journal checksum:         0x%08x\n",
			journal_checksum_type_str(jsb->s_checksum_type),
			e2p_be32(jsb->s_checksum));
	if ((nr_users > 1) ||
	    !e2p_is_null_uuid(&jsb->s_users[0])) {
		for (i=0; i < nr_users && i < JFS_USERS_MAX; i++) {
			printf(i ? "                          %s\n"
			       : "Journal users:            %s\n",
			       e2p_uuid2str(&jsb->s_users[i * UUID_SIZE]));
		}
	}
	if (jsb->s_errno != 0)
		fprintf(f, "Journal errno:            %d\n",
			(int) ntohl(jsb->s_errno));
}
