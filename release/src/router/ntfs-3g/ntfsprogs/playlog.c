/*
 *		Redo or undo a list of logged actions
 *
 * Copyright (c) 2014-2017 Jean-Pierre Andre
 *
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include "types.h"
#include "endians.h"
#include "support.h"
#include "layout.h"
#include "param.h"
#include "ntfstime.h"
#include "device_io.h"
#include "device.h"
#include "logging.h"
#include "runlist.h"
#include "mft.h"
#include "inode.h"
#include "attrib.h"
#include "bitmap.h"
#include "index.h"
#include "volume.h"
#include "unistr.h"
#include "mst.h"
#include "logfile.h"
#include "ntfsrecover.h"
#include "misc.h"

struct STORE {
	struct STORE *upper;
	struct STORE *lower;
	LCN lcn;
	char data[1];
} ;

#define dump hexdump

struct STORE *cluster_door = (struct STORE*)NULL;

/* check whether a MFT or INDX record is older than action */
#define older_record(rec, logr) ((s64)(sle64_to_cpu((rec)->lsn) \
			- sle64_to_cpu((logr)->this_lsn)) < 0)
/* check whether a MFT or INDX record is newer than action */
#define newer_record(rec, logr) ((s64)(sle64_to_cpu((rec)->lsn) \
			- sle64_to_cpu((logr)->this_lsn)) > 0)

/*
 *		A few functions for debugging
 */

static int matchcount(const char *d, const char *s, int n)
{
	int m;

	m = 0;
	while ((--n >= 0) && (*d++ == *s++)) m++;
	return (m);
}

/*
static void locate(const char *s, int n, const char *p, int m)
{
	int i,j;

	for (i=0; i<=(n - m); i++)
		if (s[i] == *p) {
			j = 1;
			while ((j < m) && (s[i + j] == p[j]))
				j++;
			if (j == m)
				printf("=== found at offset 0x%x %d\n",i,i);
		}
}
*/

static u64 inode_number(const LOG_RECORD *logr)
{
	u64 offset;

	offset = ((u64)sle64_to_cpu(logr->target_vcn)
					<< clusterbits)
		+ ((u32)le16_to_cpu(logr->cluster_index)
					<< NTFS_BLOCK_SIZE_BITS);
	return (offset >> mftrecbits);
}

/*
 *		Find an in-memory copy of a needed cluster
 *
 *	Optionally, allocate a copy.
 */

static struct STORE *getclusterentry(LCN lcn, BOOL create)
{
	struct STORE **current;
	struct STORE *newone;

	current = &cluster_door;
		/* A minimal binary tree should be enough */
	while (*current && (lcn != (*current)->lcn)) {
		if (lcn > (*current)->lcn)
			current = &(*current)->upper;
		else
			current = &(*current)->lower;
	}
	if (create && !*current) {
		newone = (struct STORE*)malloc(sizeof(struct STORE)
						+ clustersz);
		if (newone) {
			newone->upper = (struct STORE*)NULL;
			newone->lower = (struct STORE*)NULL;
			newone->lcn = lcn;
			*current = newone;
		}
	}
	return (*current);
}

void freeclusterentry(struct STORE *entry)
{
	if (!entry) {
		if (cluster_door)
			freeclusterentry(cluster_door);
		cluster_door = (struct STORE*)NULL;
	} else {
		if (optv)
			printf("* cluster 0x%llx %s updated\n",
					(long long)entry->lcn,
					(optn ? "would be" : "was"));
		if (entry->upper)
			freeclusterentry(entry->upper);
		if (entry->lower)
			freeclusterentry(entry->lower);
		free(entry);
	}
}

/*
 *		Check whether an attribute type is a valid one
 */

static BOOL valid_type(ATTR_TYPES type)
{
	BOOL ok;

	switch (type) {
	case AT_STANDARD_INFORMATION :
	case AT_ATTRIBUTE_LIST :
	case AT_FILE_NAME :
	case AT_OBJECT_ID :
	case AT_SECURITY_DESCRIPTOR :
	case AT_VOLUME_NAME :
	case AT_VOLUME_INFORMATION :
	case AT_DATA :
	case AT_INDEX_ROOT :
	case AT_INDEX_ALLOCATION :
	case AT_BITMAP :
	case AT_REPARSE_POINT :
	case AT_EA_INFORMATION :
	case AT_EA :
	case AT_PROPERTY_SET :
	case AT_LOGGED_UTILITY_STREAM :
	case AT_FIRST_USER_DEFINED_ATTRIBUTE :
	case AT_END :
		ok = TRUE;
		break;
	default :
		ok = FALSE;
		break;
	}
	return (ok);
}

/*
 *		Rough check of sanity of an index list
 */

static int sanity_indx_list(const char *buffer, u32 k, u32 end)
{
	le64 inode;
	int err;
	int lth;
	BOOL done;

	err = 0;
	done = FALSE;
	while ((k <= end) && !done && !err) {
		lth = getle16(buffer,k+8);
		if (optv > 1)
			/* Usual indexes can be determined from size */
			switch (lth) {
			case 16 : /* final without subnode */
			case 24 : /* final with subnode */
				printf("index to none lth 0x%x"
					" flags 0x%x pos 0x%x\n",
					(int)lth,
					(int)getle16(buffer,k+12),(int)k);
				break;
			case 32 : /* $R in $Reparse */
					/* Badly aligned */
				memcpy(&inode, &buffer[k + 20], 8);
				printf("index to reparse of 0x%016llx lth 0x%x"
					" flags 0x%x pos 0x%x\n",
					(long long)le64_to_cpu(inode),
					(int)lth,
					(int)getle16(buffer,k+12),(int)k);
				break;
			case 40 : /* $SII in $Secure */
				printf("index to securid 0x%lx lth 0x%x"
					" flags 0x%x pos 0x%x\n",
					(long)getle32(buffer,k + 16),
					(int)lth,
					(int)getle16(buffer,k+12),(int)k);
				break;
			case 48 : /* $SDH in $Secure */
				printf("index to securid 0x%lx lth 0x%x"
					" flags 0x%x pos 0x%x\n",
					(long)getle32(buffer,k + 20),
					(int)lth,
					(int)getle16(buffer,k+12),(int)k);
				break;
			default : /* at least 80 */
				printf("index to inode 0x%016llx lth 0x%x"
					" flags 0x%x pos 0x%x\n",
					(long long)getle64(buffer,k),
					(int)lth,
					(int)getle16(buffer,k+12),(int)k);
				if ((lth < 80) || (lth & 7)) {
					printf("** Invalid index record"
						" length %d\n",lth);
					err = 1;
				}
			}
		done = (feedle16(buffer,k+12) & INDEX_ENTRY_END) || !lth;
		if (lth & 7) {
			if (optv <= 1) /* Do not repeat the warning */
				printf("** Invalid index record length %d\n",
								lth);
			err = 1;
		} else
	   		k += lth;
   	}
	if (k != end) {
		printf("** Bad index record length %ld (computed %ld)\n",
					(long)end, (long)k);
		err = 1;
	}
	if (!done) {
		printf("** Missing end of index mark\n");
		err = 1;
	}
	return (err);
}

/*
 *		Rough check of sanity of an mft record
 */

static int sanity_mft(const char *buffer)
{
	const MFT_RECORD *record;
	const ATTR_RECORD *attr;
	u64 instances;
	u32 k;
	u32 type;
	u32 prevtype;
	u16 nextinstance;
	u16 instance;
	int err;

	err = 0;
	record = (const MFT_RECORD*)buffer;
	nextinstance = le16_to_cpu(record->next_attr_instance);
	instances = 0;
	k = le16_to_cpu(record->attrs_offset);
	attr = (const ATTR_RECORD*)&buffer[k];
	prevtype = 0;
	while ((k < mftrecsz)
	    && (attr->type != AT_END)
	    && valid_type(attr->type)) {
		type = le32_to_cpu(attr->type);
		if (type < prevtype) {
			printf("** Bad type ordering 0x%lx after 0x%lx\n",
				(long)type, (long)prevtype);
			err = 1;
		}
		instance = le16_to_cpu(attr->instance);
		/* Can nextinstance wrap around ? */
		if (instance >= nextinstance) {
			printf("** Bad attr instance %d (max %d)\n",
					(int)instance, (int)nextinstance - 1);
			err = 1;
		}
		if (instance < 64) {
			/* Only check up to 64 */
			if (((u64)1 << instance) & instances) {
				printf("** Duplicated attr instance %d\n",
					(int)instance);
			}
			instances |= (u64)1 << instance;
		}
		if (optv > 1) {
			if ((attr->type == AT_FILE_NAME)
			   && buffer[k + 88]) {
				printf("attr %08lx offs 0x%x nres %d",
					(long)type, (int)k,
					(int)attr->non_resident);
				showname(" ",&buffer[k+90],
					buffer[k + 88] & 255);
			} else
				printf("attr %08lx offs 0x%x nres %d\n",
					(long)type, (int)k,
					(int)attr->non_resident);
		}
		if ((attr->type == AT_INDEX_ROOT)
		    && sanity_indx_list(buffer,
				k + le16_to_cpu(attr->value_offset) + 32,
				k + le32_to_cpu(attr->length))) {
			err = 1;
		}
		k += le32_to_cpu(attr->length);
		attr = (const ATTR_RECORD*)&buffer[k];
		prevtype = type;
	}
	if ((optv > 1) && (attr->type == AT_END))
		printf("attr %08lx offs 0x%x\n",
				(long)le32_to_cpu(attr->type), (int)k);
	if ((attr->type != AT_END)
	    || (le32_to_cpu(record->bytes_in_use) != (k + 8))
	    || (le32_to_cpu(record->bytes_allocated) < (k + 8))) {
		printf("** Bad MFT record length %ld"
				" (computed %ld allocated %ld)\n",
				(long)le32_to_cpu(record->bytes_in_use),
				(long)(k + 8),
				(long)le32_to_cpu(record->bytes_allocated));
		err = 1;
	}
	return (err);
}

/*
 *		Rough check of sanity of an index block
 */

static int sanity_indx(ntfs_volume *vol, const char *buffer)
{
	const INDEX_BLOCK *indx;
	u32 k;
	int err;

	err = 0;
	indx = (const INDEX_BLOCK*)buffer;
	k = offsetof(INDEX_BLOCK, index) +
		le32_to_cpu(indx->index.entries_offset);
	err = sanity_indx_list(buffer, k,
				le32_to_cpu(indx->index.index_length) + 24);
	if ((le32_to_cpu(indx->index.index_length)
		> le32_to_cpu(indx->index.allocated_size))
	    || (le32_to_cpu(indx->index.allocated_size)
		!= (vol->indx_record_size - 24))) {
		printf("** Bad index length %ld"
				" (usable %ld allocated %ld)\n",
				(long)le32_to_cpu(indx->index.index_length),
				(long)(vol->indx_record_size - 24),
				(long)le32_to_cpu(indx->index.allocated_size));
		err = 1;
	}
	return (err);
}


/*
 *		Allocate a buffer and read a full set of raw clusters
 *
 *	Do not use for accessing $LogFile.
 *	With option -n reading is first attempted from the memory store
 */

static char *read_raw(ntfs_volume *vol, const LOG_RECORD *logr)
{
	char *buffer;
	char *target;
	struct STORE *store;
	LCN lcn;
	int count;
	int i;
	BOOL fail;

	count = le16_to_cpu(logr->lcns_to_follow);
	if (!count) {
		printf("** Error : no lcn to read from\n");
		buffer = (char*)NULL;
	} else 
		buffer = (char*)malloc(clustersz*count);
// TODO error messages
	if (buffer) {
		fail = FALSE;
		for (i=0; (i<count) && !fail; i++) {
			store = (struct STORE*)NULL;
			lcn = sle64_to_cpu(logr->lcn_list[i]);
			target = buffer + clustersz*i;
			if (optn) {
				store = getclusterentry(lcn, FALSE);
				if (store) {
					memcpy(target, store->data, clustersz);
				if (optv)
					printf("== lcn 0x%llx from store\n",
							(long long)lcn);
				if ((optv > 1) && optc
				    && within_lcn_range(logr))
					dump(store->data, clustersz);
				}
			}
			if (!store
			   && (ntfs_pread(vol->dev, lcn << clusterbits,
                			clustersz, target) != clustersz)) {
				fail = TRUE;
			} else {
				if (!store) {
					if (optv)
						printf("== lcn 0x%llx"
							" from device\n",
							(long long)lcn);
					if ((optv > 1) && optc
					    && within_lcn_range(logr))
						dump(target, clustersz);
				}
			}
		}
		if (fail) {
			printf("** Could not read cluster 0x%llx\n",
					(long long)lcn);
			free(buffer);
			buffer = (char*)NULL;
		}
	}
	return (buffer);
}

/*
 *		Write a full set of raw clusters
 *
 *	Do not use for accessing $LogFile.
 *	With option -n a copy of the buffer is kept in memory for later use.
 */

static int write_raw(ntfs_volume *vol, const LOG_RECORD *logr,
					char *buffer)
{
	int err;
	struct STORE *store;
	LCN lcn;
	char *source;
	int count;
	int i;

	err = 0;
	count = le16_to_cpu(logr->lcns_to_follow);
	if (!count)
		printf("** Error : no lcn to write to\n");
	if (optn) {
		for (i=0; (i<count) && !err; i++) {
			lcn = sle64_to_cpu(logr->lcn_list[i]);
			source = buffer + clustersz*i;
			store = getclusterentry(lcn, TRUE);
			if (store) {
				memcpy(store->data, source, clustersz);
				if (optv)
					printf("== lcn 0x%llx to store\n",
							(long long)lcn);
				if ((optv > 1) && optc
				    && within_lcn_range(logr))
					dump(store->data, clustersz);
			} else {
				printf("** Could not store cluster 0x%llx\n",
					(long long)lcn);
				err = 1;
			}
		}
	} else {
		for (i=0; (i<count) && !err; i++) {
			lcn = sle64_to_cpu(logr->lcn_list[i]);
			if (optv)
				printf("== lcn 0x%llx to device\n",
							(long long)lcn);
			source = buffer + clustersz*i;
			if (ntfs_pwrite(vol->dev, lcn << clusterbits,
        	       			clustersz, source) != clustersz) {
				printf("** Could not write cluster 0x%llx\n",
						(long long)lcn);
				err = 1;
			}
		}
	}
	return (err);
}

/*
 *		Write a full set of raw clusters to mft_mirr
 */

static int write_mirr(ntfs_volume *vol, const LOG_RECORD *logr,
					char *buffer)
{
	int err;
	LCN lcn;
	char *source;
	int count;
	int i;

	err = 0;
	count = le16_to_cpu(logr->lcns_to_follow);
	if (!count)
		printf("** Error : no lcn to write to\n");
	if (!optn) {
		for (i=0; (i<count) && !err; i++) {
			lcn = ntfs_attr_vcn_to_lcn(vol->mftmirr_na,
				sle64_to_cpu(logr->target_vcn) + i);
			source = buffer + clustersz*i;
			if ((lcn < 0)
			    || (ntfs_pwrite(vol->dev, lcn << clusterbits,
        	       			clustersz, source) != clustersz)) {
				printf("** Could not write cluster 0x%llx\n",
						(long long)lcn);
				err = 1;
			}
		}
	}
	return (err);
}

/*
 *		Allocate a buffer and read a single protected record
 */

static char *read_protected(ntfs_volume *vol, const LOG_RECORD *logr,
			u32 size, BOOL warn)
{
	char *buffer;
	char *full;
	u32 pos;
	LCN lcn;

		/* read full clusters */
	buffer = read_raw(vol, logr);
		/*
		 * if the record is smaller than a cluster,
		 * make a partial copy and free the full buffer
		 */
	if (buffer && (size < clustersz)) {
		full = buffer;
		buffer = (char*)malloc(size);
		if (buffer) {
			pos = le16_to_cpu(logr->cluster_index)
					<< NTFS_BLOCK_SIZE_BITS;
			memcpy(buffer, full + pos, size);
		}
		free(full);
	}
	if (buffer && (ntfs_mst_post_read_fixup_warn(
				(NTFS_RECORD*)buffer, size, FALSE) < 0)) {
		if (warn) {
			lcn = sle64_to_cpu(logr->lcn_list[0]);
			printf("** Invalid protected record at 0x%llx"
					" index %d\n",
					(long long)lcn,
					(int)le16_to_cpu(logr->cluster_index));
		}
		free(buffer);
		buffer = (char*)NULL;
	}
	return (buffer);
}

/*
 *		Protect a single record, write, and deallocate the buffer
 *
 *	With option -n a copy of the buffer is kept in protected form in
 *	memory for later use.
 *	As the store only knows about clusters, if the record is smaller
 *	than a cluster, have to read, merge and write.
 */

static int write_protected(ntfs_volume *vol, const LOG_RECORD *logr,
				char *buffer, u32 size)
{
	MFT_RECORD *record;
	INDEX_BLOCK *indx;
	char *full;
	u32 pos;
	BOOL mftmirr;
	BOOL checked;
	int err;

	err = 0;
	mftmirr = FALSE;
	checked = FALSE;
	if ((size == mftrecsz) && !memcmp(buffer,"FILE",4)) {
		record = (MFT_RECORD*)buffer;
		if (optv)
			printf("update inode %ld lsn 0x%llx"
				" (record %s than action 0x%llx)\n",
				(long)le32_to_cpu(record->mft_record_number),
				(long long)sle64_to_cpu(record->lsn),
				((s64)(sle64_to_cpu(record->lsn)
				    - sle64_to_cpu(logr->this_lsn)) < 0 ?
					"older" : "newer"),
				(long long)sle64_to_cpu(logr->this_lsn));
		if (optv > 1)
			printf("mft vcn %lld index %d\n",
				(long long)sle64_to_cpu(logr->target_vcn),
				(int)le16_to_cpu(logr->cluster_index));
		err = sanity_mft(buffer);
			/* Should set to some previous lsn for undos */
		if (opts)
			record->lsn = logr->this_lsn;
		/* Duplicate on mftmirr if not overflowing its size */
		mftmirr = (((u64)sle64_to_cpu(logr->target_vcn)
				+ le16_to_cpu(logr->lcns_to_follow))
				<< clusterbits)
			<= (((u64)vol->mftmirr_size) << mftrecbits);
		checked = TRUE;
	}
	if ((size == vol->indx_record_size) && !memcmp(buffer,"INDX",4)) {
		indx = (INDEX_BLOCK*)buffer;
		if (optv)
			printf("update index lsn 0x%llx"
				" (index %s than action 0x%llx)\n",
				(long long)sle64_to_cpu(indx->lsn),
				((s64)(sle64_to_cpu(indx->lsn)
				    - sle64_to_cpu(logr->this_lsn)) < 0 ?
					"older" : "newer"),
				(long long)sle64_to_cpu(logr->this_lsn));
		err = sanity_indx(vol, buffer);
			/* Should set to some previous lsn for undos */
		if (opts)
			indx->lsn = logr->this_lsn;
		checked = TRUE;
	}
	if (!checked) {
		printf("** Error : writing protected record of unknown type\n");
		err = 1;
	}
	if (!err) {
		if (!ntfs_mst_pre_write_fixup((NTFS_RECORD*)buffer, size)) {
			/*
			 * If the record is smaller than a cluster, get a full
			 * cluster, merge and write.
			 */
			if (size < clustersz) {
				full = read_raw(vol, logr);
				if (full) {
					pos = le16_to_cpu(logr->cluster_index)
						<< NTFS_BLOCK_SIZE_BITS;
					memcpy(full + pos, buffer, size);
					err = write_raw(vol, logr, full);
					if (!err && mftmirr && !optn)
						err = write_mirr(vol, logr,
								full);
					free(full);
				} else
					err = 1;
			} else {
					/* write full clusters */
				err = write_raw(vol, logr, buffer);
				if (!err && mftmirr && !optn)
					err = write_mirr(vol, logr, buffer);
			}
		} else {
			printf("** Failed to protect record\n");
			err = 1;
		}
	}
	return (err);
}

/*
 *		Resize attribute records
 *
 *	The attribute value is resized to new size, but the attribute
 *	and MFT record must be kept aligned to 8 bytes.
 */

static int resize_attribute(MFT_RECORD *entry, ATTR_RECORD *attr, INDEX_ROOT *index,
			int rawresize, int resize)
{
	int err;
	u32 newlength;
	u32 newused;
	u32 newvalue;
	u32 indexlth;
	u32 indexalloc;

	err = 0;
	if (attr) {
		newvalue = le32_to_cpu(attr->value_length) + rawresize;
		attr->value_length = cpu_to_le32(newvalue);
		newlength = le32_to_cpu(attr->length) + resize;
		attr->length = cpu_to_le32(newlength);
	}
	if (entry) {
		newused = le32_to_cpu(entry->bytes_in_use) + resize;
		entry->bytes_in_use = cpu_to_le32(newused);
	}
	if (index) {
		indexlth = le32_to_cpu(index->index.index_length) + resize;
		index->index.index_length = cpu_to_le32(indexlth);
		indexalloc = le32_to_cpu(index->index.allocated_size) + resize;
		index->index.allocated_size = cpu_to_le32(indexalloc);
	}
	return (err);
}

/*
 *		Adjust the next attribute instance
 *
 *	If a newly created attribute matches the next instance, then
 *	the next instance has to be incremented.
 *
 *	Do the opposite when undoing an attribute creation, but
 *	do not change the next instance when deleting an attribute
 *	or undoing the deletion.
 */

static void adjust_instance(const ATTR_RECORD *attr, MFT_RECORD *entry, int increment)
{
	u16 instance;

	if (increment > 0) {
			/* Allocating a new instance ? */
		if (attr->instance == entry->next_attr_instance) {
			instance = (le16_to_cpu(entry->next_attr_instance)
					+ 1) & 0xffff;
			entry->next_attr_instance = cpu_to_le16(instance);
		}
	}
	if (increment < 0) {
			/* Freeing the latest instance ? */
		instance = (le16_to_cpu(entry->next_attr_instance)
					- 1) & 0xffff;
		if (attr->instance == cpu_to_le16(instance))
			entry->next_attr_instance = attr->instance;
	}
}

/*
 *		Adjust the highest vcn according to mapping pairs
 *
 *	The runlist has to be fully recomputed
 */

static int adjust_high_vcn(ntfs_volume *vol, ATTR_RECORD *attr)
{
	runlist_element *rl;
	runlist_element *xrl;
	VCN high_vcn;
	int err;

	err = 1;
	attr->highest_vcn = const_cpu_to_sle64(0);
	rl = ntfs_mapping_pairs_decompress(vol, attr, (runlist_element*)NULL);
	if (rl) {
		xrl = rl;
		if (xrl->length)
			xrl++;
		while ((xrl->length) && (xrl->lcn != LCN_RL_NOT_MAPPED))
			xrl++;
		high_vcn = xrl->vcn - 1;
		attr->highest_vcn = cpu_to_sle64(high_vcn);
		free(rl);
		err = 0;
	} else {
		printf("** Failed to decompress the runlist\n");
		dump((char*)attr,128);
	}
	return (err);
}

/*
 *		Check index match, to be used for undos only
 *
 *	The action UpdateFileNameRoot updates the time stamps and/or the
 *	sizes, but the lsn is not updated in the index record.
 *	As a consequence such UpdateFileNameRoot are not always undone
 *	and the actual record does not fully match the undo data.
 *	We however accept the match if the parent directory and the name
 *	match.
 *	Alternate workaround : do not check the lsn when undoing
 *	UpdateFileNameRoot
 */

static BOOL index_match_undo(const char *first, const char *second, int length)
{
	int len;
	BOOL match;

	match = !memcmp(first, second, length);
	if (!match) {
		if (optv) {
			printf("The existing index does not match :\n");
			dump(second,length);
		}
		len = (first[80] & 255)*2 + 2;
		match = (feedle64(first, 16) == feedle64(second, 16))
		    && !memcmp(first + 80, second + 80, len);
		if (match && optv)
			printf("However parent dir and name do match\n");
	}
	return (match);
}


/*
 *		Generic idempotent change to a resident attribute
 */

static int change_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
		char *buffer, const char *data, u32 target, u32 length)
{
	LCN lcn;
	ATTR_RECORD *attr;
	u32 attrend;
	int err;
	int changed;

	err = 1;
	if (action->record.undo_length != action->record.redo_length)
		printf("** Error size change in change_resident\n");
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
		printf("-> full MFT record :\n");
		dump(buffer,mftrecsz);
	}
	attrend = le16_to_cpu(action->record.record_offset)
			+ le32_to_cpu(attr->length);
	if ((target + length) > attrend) {
		printf("** Error : update overflows from attribute\n");
	}
	if (!(length & 7)
	    && ((target + length) <= attrend)
	    && (attrend <= mftrecsz)
	    && !sanity_mft(buffer)) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			memcpy(buffer + target, data, length);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int change_resident_expect(ntfs_volume *vol, const struct ACTION_RECORD *action,
		char *buffer, const char *data, const char *expected,
		u32 target, u32 length, ATTR_TYPES type)
{
	LCN lcn;
	ATTR_RECORD *attr;
	int err;
	BOOL found;

	err = 1;
	if (action->record.undo_length != action->record.redo_length)
		printf("** Error size change in change_resident\n");
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
		printf("-> full record :\n");
		dump((char*)attr, le32_to_cpu(attr->length));
	}
	if ((attr->type == type)
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		found = !memcmp(buffer + target, expected, length);
		err = 0;
		if (found) {
			memcpy(buffer + target, data, length);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "updated" : "unchanged"));
		}
	}
	return (err);
}

/*
 *		Generic idempotent change to a an index value
 *
 */

static int change_index_value(ntfs_volume *vol, const struct ACTION_RECORD *action,
		char *buffer, const char *data, u32 target, u32 length)
{
	LCN lcn;
	u32 count;
	u32 xsize;
	int changed;
	int err;

	err = 1;
	count = le16_to_cpu(action->record.lcns_to_follow);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((target + length) <= (count << clusterbits)) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			memcpy(buffer + target, data, length);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
							buffer, xsize);
		}
		if (optv > 1) {
			printf("-> data record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

/*
 *		Add one or more resident attributes
 */

static int add_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length, u32 oldlength)
{
	LCN lcn;
	MFT_RECORD *entry;
	int err;
	BOOL found;
	int resize;

	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	resize = length - oldlength;
	if (optv > 1) {
		printf("existing data :\n");
		dump(buffer + target,length);
	}
	if (!(length & 7)
	    && !(oldlength & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		err = 0;
		if (data && length)
			found = !memcmp(buffer + target,
						data, length);
		else {
			found = TRUE;
			err = 1;
		}
		if (!found && !err) {
			/* Make space to insert the entry */
			memmove(buffer + target + resize,
				buffer + target,
				mftrecsz - target - resize);
			if (data)
				memcpy(buffer + target, data, length);
			else
				memset(buffer + target, 0, length);
			resize_attribute(entry, NULL, NULL,
						resize, resize);
			if (optv > 1) {
				printf("new data at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "expanded"));
		}
	}
	return (err);
}

/*
 *		Add one or more non-resident records
 */

static int delete_non_resident(void /*ntfs_volume *vol,
		const struct ACTION_RECORD *action,
		const char *data, u32 target, u32 length, u32 oldlength*/)
{
	int err;

	err = 1;
	printf("** delete_non_resident() not implemented\n");
	return (err);
}

/*
 *		Expand a single resident attribute
 */

static int expand_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length, u32 oldlength)
{
	LCN lcn;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	int err;
	BOOL found;
	int resize;
	u16 base;

	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("existing data :\n");
		dump(buffer + target,length);
	}
	base = 24 + 2*attr->name_length;
	resize = ((base + length - 1) | 7)
		- ((base + oldlength - 1) | 7);
	if ((target + length) <= mftrecsz) {
		/* This has to be an idempotent action */
// TODO This test is wrong !
		found = le32_to_cpu(attr->value_length) == length;
		if (found && data && length)
			found = !memcmp(buffer + target, data, length);
		err = 0;
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + resize,
				buffer + target,
				mftrecsz - target - resize);
// TODO what to do if length is not a multiple of 8 ?
			if (data)
				memcpy(buffer + target, data, length);
			else
				memset(buffer + target, 0, length);
			resize_attribute(entry, attr, NULL,
						length - oldlength, resize);
			if (optv > 1) {
				printf("new data at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
					(found ? "unchanged" : "expanded"));
		}
	}
	return (err);
}

/*
 *		Add one or more non-resident records
 */

static int add_non_resident(void /*ntfs_volume *vol,
		const struct ACTION_RECORD *action,
		const char *data, u32 target, u32 length, u32 oldlength*/)
{
	int err;

	printf("** add_non_resident() not implemented\n");
	err = 0;
	return (err);
}

/*
 *		Generic insert a new resident attribute
 */

static int insert_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length)
{
	LCN lcn;
	ATTR_RECORD *attr;
	const ATTR_RECORD *newattr;
	MFT_RECORD *entry;
	u32 newused;
	u16 links;
	int err;
	BOOL found;

	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	newattr = (const ATTR_RECORD*)data;
	if (optv > 1) {
		printf("existing record :\n");
		dump(buffer + target,length);
		if (le32_to_cpu(attr->type) < le32_to_cpu(newattr->type)) {
			printf("** Bad attribute order, full record :\n");
			dump(buffer, mftrecsz);
		}
	}
	/* Types must be in ascending order */
	if (valid_type(attr->type)
	    && (le32_to_cpu(attr->type)
		 >= le32_to_cpu(newattr->type))
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		found = !memcmp(buffer + target, data, length);
		err = 0;
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + length,
				buffer + target,
				mftrecsz - target - length);
			memcpy(buffer + target, data, length);
			newused = le32_to_cpu(entry->bytes_in_use)
						+ length;
			entry->bytes_in_use = cpu_to_le32(newused);
			if (action->record.redo_operation
			    == const_cpu_to_le16(CreateAttribute)) {
			/*
			 * For a real create, may have to adjust
			 * the next attribute instance
			 */
				adjust_instance(newattr, entry, 1);
			}
			if (newattr->type == AT_FILE_NAME) {
				links = le16_to_cpu(entry->link_count) + 1;
				entry->link_count = cpu_to_le16(links);
			}
			if (optv > 1) {
				printf("expanded record (now 0x%x"
					" bytes used) :\n",
					(int)newused);
				dump(buffer + target, 2*length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "expanded"));
		}
	}
	return (err);
}

/*
 *		Generic remove a single resident attribute
 */

static int remove_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length)
{
	LCN lcn;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	u32 newused;
	u16 links;
	int err;
	BOOL found;

	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("existing record :\n");
		dump(buffer + target,length);
	}
	if (!(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
	/* For AT_DATA the value is not always present */
		if (attr->type == AT_DATA)
			found = !memcmp(buffer + target, data, 
				le16_to_cpu(attr->value_offset));
		else
			found = !memcmp(buffer + target, data, length);
		if (!found && optv) {
			printf("data 0x%lx 0x%lx offset %d %ld\n",
				(long)le32_to_cpu(attr->type),
				(long)le32_to_cpu(AT_DATA),
				(int)offsetof(ATTR_RECORD, resident_end),
				(long)le16_to_cpu(attr->value_offset));
			printf("The existing record does not match (%d/%d)\n",
				(int)matchcount(buffer + target, data,
				length),(int)length);
			dump(data,length);
			printf("full attr :\n");
			dump((const char*)attr,mftrecsz
				- le16_to_cpu(action->record.record_offset));
		}
		err = 0;
		if (found) {
			if (attr->type == AT_FILE_NAME) {
				links = le16_to_cpu(entry->link_count) - 1;
				entry->link_count = cpu_to_le16(links);
			}
			if (action->record.redo_operation
			    == const_cpu_to_le16(CreateAttribute)) {
				adjust_instance(attr, entry, -1);
			}
			/* Remove the entry */
			memmove(buffer + target,
				buffer + target + length,
				mftrecsz - target - length);
			newused = le32_to_cpu(entry->bytes_in_use) - length;
			entry->bytes_in_use = cpu_to_le32(newused);
			if (optv > 1) {
				printf("new record at same location"
					" (now 0x%x bytes used) :\n",
					(int)newused);
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
					buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "shrinked" : "unchanged"));
		}
	}
	return (err);
}

/*
 *		Delete one or more resident attributes
 */

static int delete_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length, u32 oldlength)
{
	LCN lcn;
	MFT_RECORD *entry;
	int err;
	BOOL found;
	int resize;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	if (optv > 1) {
		printf("existing data :\n");
		dump(buffer + target,length);
	}
	resize = length - oldlength;
	if (!(length & 7)
	    && !(oldlength & 7)
	    && ((target + oldlength) <= mftrecsz)) {
		/* This has to be an idempotent action */
		err = 0;
		if (data && length)
			found = !memcmp(buffer + target, data, length);
		else {
			found = FALSE;
			err = 1;
		}
		if (!found && !err) {
			/* Remove the entry, if present */
			memmove(buffer + target,
				buffer + target - resize,
				mftrecsz - target + resize);
			resize_attribute(entry, NULL, NULL,
					length - oldlength, resize);
			if (optv > 1) {
				printf("new data at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
							buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "shrinked"));
		}
	}
	return (err);
}

static int shrink_resident(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer, const char *data, u32 target,
			u32 length, u32 oldlength)
{
	LCN lcn;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	int err;
	BOOL found;
	int resize;
	u16 base;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
				+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("existing data :\n");
		dump(buffer + target,length);
	}
	base = 24 + 2*attr->name_length;
	resize = ((base + length - 1) | 7)
		- ((base + oldlength - 1) | 7);
	if ((oldlength > length)
// TODO limit to attr length
	    && ((target + oldlength) <= mftrecsz)) {
		/* This has to be an idempotent action */
		if (data && length)
			found = !memcmp(buffer + target, data, length);
		else
{
// TODO wrong : need checking against the old data, but in known cases
// redo data is not available either and existing data is not zero.
			found = FALSE;
printf("* fake test, assuming not shrinked : value length %ld length %ld oldlength %ld\n",(long)le32_to_cpu(attr->value_length),(long)length,(long)oldlength);
//dump(buffer + target, oldlength);
}
		err = 0;
		if (!found) {
			if (length) {
				/* Relocate end of record */
// TODO restrict to bytes_in_use
				memmove(buffer + target + length,
					buffer + target + oldlength,
					mftrecsz - target - oldlength);
				/* Insert new data or zeroes */
				if (data)
					memcpy(buffer + target, data, length);
				else
					memset(buffer + target, 0, length);
			} else {
				/* Remove the entry, unless targeted size */
				memmove(buffer + target,
					buffer + target - resize,
					mftrecsz - target + resize);
			}
			resize_attribute(entry, attr, NULL,
					length - oldlength, resize);
			if (optv > 1) {
				printf("new data at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "shrinked"));
		}
	}
	return (err);
}

static int update_index(ntfs_volume *vol, const struct ACTION_RECORD *action,
		char *buffer, const char *data, u32 target, u32 length)
{
	LCN lcn;
	INDEX_BLOCK *indx;
	u32 xsize;
	BOOL changed;
	int err;

	err = 1;
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing index :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)) {
		/* This has to be an idempotent action */
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			/* Update the entry */
			memcpy(buffer + target, data, length);
			if (optv > 1) {
				printf("-> new index :\n");
				dump(&buffer[target], length);
			}
			err = write_protected(vol, &action->record,
						buffer, xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

/*
 *		Controversial deletion of file names, see undo_delete_file()
 */

static int delete_names(char *buffer)
{
	MFT_RECORD *record;
	ATTR_RECORD *attr;
	u32 used;
	u32 pos;
	int length;
	int cnt;

	record = (MFT_RECORD*)buffer;
	pos = le16_to_cpu(record->attrs_offset);
	used = le32_to_cpu(record->bytes_in_use);
	cnt = 0;
	do {
		attr = (ATTR_RECORD*)&buffer[pos];
		length = le32_to_cpu(attr->length);
		if (attr->type == AT_FILE_NAME) {
			if (optv)
				showname("Controversial deletion of ",
					&buffer[pos+90], buffer[pos+88] & 255);
			memmove(buffer + pos, buffer + pos + length,
				mftrecsz - pos - length);
			used -= length;
			cnt++;
		} else
			pos += length;
	} while ((pos < used)
		&& (le32_to_cpu(attr->type) <= le32_to_cpu(AT_FILE_NAME)));
	record->bytes_in_use = cpu_to_le32(used);
	record->link_count = cpu_to_le16(0);
	return (cnt ? 0 : 1);
}

static int rebuildname(const INDEX_ENTRY *index)
{
	ATTR_RECORD *attr;
	int headlth;
	int datalth;

	datalth = le16_to_cpu(index->length)
				- offsetof(INDEX_ENTRY,key.file_name);
	headlth = offsetof(ATTR_RECORD,resident_end);
	attr = (ATTR_RECORD*)malloc(headlth + datalth);
	if (attr) {
		attr->type = AT_FILE_NAME;
		attr->length = cpu_to_le32(headlth + datalth);
		attr->non_resident = 0;
		attr->name_length = 0;
		attr->name_offset = const_cpu_to_le16(0);
		attr->flags = const_cpu_to_le16(0);
		attr->instance = const_cpu_to_le16(0);
		attr->value_length = cpu_to_le32(
			2*index->key.file_name.file_name_length
			+ offsetof(FILE_NAME_ATTR, file_name));
		attr->value_offset = cpu_to_le16(headlth);
		attr->resident_flags = RESIDENT_ATTR_IS_INDEXED;
		memcpy(attr->resident_end, &index->key.file_name, datalth);
		free(attr);
	}
	return (0);
}

/*
 *		Controversial creation of an index allocation attribute
 *
 *	This is useful for turning the clock backward, but cannot
 *	work properly in the general case and must not be used for
 *	a real sync.
 *	The main problem is to synchronize the file names when an
 *	inode is reused with a different name.
 */

static int insert_index_allocation(ntfs_volume *vol, char *buffer, u32 offs)
{
	MFT_RECORD *record;
	ATTR_RECORD *attr;
	u32 used;
	u32 pos;
	u32 xsize;
	u16 instance;
	int length;
	int addedlength;
	int namelength;
	int err;
	static const unsigned char bitmap[] =
			{ 1, 0, 0, 0, 0, 0, 0, 0 } ;

	err = 1;
	if (opts) {
		printf("** Call to unsupported insert_index_allocation()\n");
	} else {
		record = (MFT_RECORD*)buffer;
		pos = le16_to_cpu(record->attrs_offset);
		used = le32_to_cpu(record->bytes_in_use);
		attr = (ATTR_RECORD*)&buffer[pos];
		while ((pos < used)
		    && (le32_to_cpu(attr->type) < le32_to_cpu(AT_INDEX_ROOT))) {
			pos += le32_to_cpu(attr->length);
			attr = (ATTR_RECORD*)&buffer[pos];
		}
		length = le32_to_cpu(attr->length);
		addedlength = length - 8 /* index allocation */
                     + length - 48; /* bitmap */
		if ((attr->type == AT_INDEX_ROOT)
		    && ((pos + length) == offs)
		    && ((used + addedlength) < mftrecsz)) {
			/* Make space for the attribute */
			memmove(buffer + offs + addedlength, buffer + offs,
					mftrecsz - offs - addedlength);
			record->bytes_in_use = cpu_to_le32(used + addedlength);
			/*
			 * Insert an AT_INDEX_ALLOCATION
			 */
			attr = (ATTR_RECORD*)&buffer[offs];
			attr->type = AT_INDEX_ALLOCATION;
			attr->length = cpu_to_le32(length - 8);
			attr->non_resident = 1;
			namelength = buffer[pos + 9] & 255;
			attr->name_length = namelength;
			attr->name_offset = const_cpu_to_le16(0x40);
			memcpy(buffer + offs + 0x40, buffer + pos + 0x18,
								2*namelength);
			attr->flags = const_cpu_to_le16(0);
			/* Should we really take a new instance ? */
			attr->instance = record->next_attr_instance;
			instance = le16_to_cpu(record->next_attr_instance) + 1;
			record->next_attr_instance = cpu_to_le16(instance);
			attr->lowest_vcn = const_cpu_to_sle64(0);
			attr->highest_vcn = const_cpu_to_sle64(0);
			attr->mapping_pairs_offset = cpu_to_le16(
							2*namelength + 0x40);
			attr->compression_unit = 0;
			xsize = vol->indx_record_size;
			attr->allocated_size = cpu_to_sle64(xsize);
			attr->data_size = attr->allocated_size;
			attr->initialized_size = attr->allocated_size;
			/*
			 * Insert an AT_INDEX_BITMAP
			 */
			attr = (ATTR_RECORD*)&buffer[offs + length - 8];
			attr->type = AT_BITMAP;
			attr->length = cpu_to_le32(length - 48);
			attr->non_resident = 0;
			namelength = buffer[pos + 9] & 255;
			attr->name_length = namelength;
			attr->name_offset = const_cpu_to_le16(0x18);
			memcpy(buffer + offs + length - 8 + 0x18,
					buffer + pos + 0x18, 2*namelength);
			attr->flags = const_cpu_to_le16(0);
			attr->value_length = const_cpu_to_le32(8);
			attr->value_offset = cpu_to_le16(2*namelength + 24);
			attr->resident_flags = 0;
			memcpy((char*)attr->resident_end + 2*namelength,
								bitmap, 8);
			/* Should we really take a new instance ? */
			attr->instance = record->next_attr_instance;
			instance = le16_to_cpu(record->next_attr_instance) + 1;
			record->next_attr_instance = cpu_to_le16(instance);
			err = sanity_mft(buffer);
		} else {
			printf("** index root does not match\n");
			err = 1;
		}
	}
	return (err);
}

/*
 *		Check whether a full MFT record is fed by an action
 *
 *	If so, checking the validity of existing record is pointless
 */

static BOOL check_full_mft(const struct ACTION_RECORD *action, BOOL redoing)
{
	const MFT_RECORD *record;
	const ATTR_RECORD *attr;
	u32 length;
	u32 k;
	BOOL ok;

	if (redoing) {
		record = (const MFT_RECORD*)((const char*)&action->record
				+ get_redo_offset(&action->record));
		length = le16_to_cpu(action->record.redo_length);
	} else {
		record = (const MFT_RECORD*)((const char*)&action->record
				+ get_undo_offset(&action->record));
		length = le16_to_cpu(action->record.undo_length);
	}
		/* The length in use must be fed */
	ok = !action->record.record_offset
		&& !action->record.attribute_offset
		&& (record->magic == magic_FILE)
		&& (length <= mftrecsz)
		&& (length >= (offsetof(MFT_RECORD, bytes_in_use)
			 + sizeof(record->bytes_in_use)));
	if (ok) {
		k = le16_to_cpu(record->attrs_offset);
		attr = (const ATTR_RECORD*)((const char*)record + k);
		while (((k + sizeof(attr->type)) <= length)
		    && (attr->type != AT_END)
		    && valid_type(attr->type)) {
			k += le32_to_cpu(attr->length);
			attr = (const ATTR_RECORD*)((const char*)record + k);
		}
			/* AT_END must be present */
		ok = ((k + sizeof(attr->type)) <= length)
		    && (attr->type == AT_END);
	}
	return (ok);
}

/*
 *		Check whether a full index block is fed by the log record
 *
 *	If so, checking the validity of existing record is pointless
 */

static BOOL check_full_index(const struct ACTION_RECORD *action, BOOL redoing)
{
	const INDEX_BLOCK *indx;
	u32 length;

	if (redoing) {
		indx = (const INDEX_BLOCK*)((const char*)&action->record
				+ get_redo_offset(&action->record));
		length = le16_to_cpu(action->record.redo_length);
	} else {
		indx = (const INDEX_BLOCK*)((const char*)&action->record
				+ get_undo_offset(&action->record));
		length = le16_to_cpu(action->record.undo_length);
	}
	/* the index length must be fed, so must be the full index block */
	return (!action->record.record_offset
		&& !action->record.attribute_offset
		&& (indx->magic == magic_INDX)
		&& (length >= (offsetof(INDEX_BLOCK, index.index_length) + 4))
		&& (length >= (le32_to_cpu(indx->index.index_length) + 24)));
}

/*
 *		Create an index block for undoing its deletion
 *
 *	This is useful for turning the clock backward, but cannot
 *	work properly in the general case and must not be used for
 *	a real sync.
 */

static int create_indx(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	INDEX_BLOCK *indx;
	INDEX_ENTRY_HEADER *ixhead;
	INDEX_ENTRY *ixentry;
	VCN vcn;
	int err;

	if (opts) {
		printf("** Call to unsupported create_indx()\n");
		err = 1;
	} else {
		err = 0;
		indx = (INDEX_BLOCK*)buffer;
		indx->magic = magic_INDX;
// TODO compute properly
		indx->usa_ofs = const_cpu_to_le16(0x28);
		indx->usa_count = const_cpu_to_le16(9);
		indx->lsn = action->record.this_lsn;
		vcn = sle64_to_cpu(action->record.target_vcn);
			/* beware of size change on big-endian cpus */
		indx->index_block_vcn = cpu_to_sle64(vcn);
			/* INDEX_HEADER */
		indx->index.entries_offset = const_cpu_to_le32(0x28);
		indx->index.index_length = const_cpu_to_le32(0x38);
		indx->index.allocated_size =
				cpu_to_le32(vol->indx_record_size - 24);
		indx->index.ih_flags = 0;
			/* INDEX_ENTRY_HEADER */
		ixhead = (INDEX_ENTRY_HEADER*)(buffer + 0x28);
		ixhead->length = cpu_to_le16(vol->indx_record_size - 24);
			/* terminating INDEX_ENTRY */
		ixentry = (INDEX_ENTRY*)(buffer + 0x40);
		ixentry->indexed_file = const_cpu_to_le64(0);
		ixentry->length = const_cpu_to_le16(16);
		ixentry->key_length = const_cpu_to_le16(0);
		ixentry->ie_flags = INDEX_ENTRY_END;
	}
	return (err);
}

static int redo_action37(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		printf("existing data :\n");
		dump(buffer + target,length);
	}
	if ((target + length) == mftrecsz) {
		memset(buffer + target, 0, length);
		err = write_protected(vol, &action->record,
					buffer, mftrecsz);
		if (optv > 1) {
			printf("-> MFT record trimmed\n");
		}
	} else {
		printf("** Bad action-37, inode %lld record :\n",
			(long long)inode_number(&action->record));
		printf("target %d length %d sum %d\n",
			(int)target,(int)length,(int)(target + length));
		dump(buffer,mftrecsz);
	}
	err = 0;
	return (err);
}

static int redo_add_index(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	LCN lcn;
	const char *data;
	INDEX_BLOCK *indx;
	u32 target;
	u32 length;
	u32 xsize;
	u32 indexlth;
	int err;
	BOOL found;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)) {
		/* This has to be an idempotent action */
		found = !memcmp(buffer + target, data, length);
		err = 0;
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + length,
				buffer + target,
				xsize - target - length);
			memcpy(buffer + target, data, length);
			indexlth = le32_to_cpu(indx->index.index_length)
						+ length;
			indx->index.index_length = cpu_to_le32(indexlth);
			if (optv > 1) {
				printf("-> inserted record :\n");
				dump(&buffer[target], length);
			}
			err = write_protected(vol, &action->record,
						buffer, xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(found ? "unchanged" : "inserted"));
		}
	}
	return (err);
}

static int redo_add_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	INDEX_ROOT *index;
	u32 target;
	u32 length;
	int err;
	BOOL found;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	index = (INDEX_ROOT*)(((char*)attr)
			+ le16_to_cpu(attr->value_offset));
	if (optv > 1) {
		printf("existing index :\n");
		dump(buffer + target,length);
	}
	if ((attr->type == AT_INDEX_ROOT)
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		found = !memcmp(buffer + target, data, length);
		err = 0;
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + length,
				buffer + target,
				mftrecsz - target - length);
			memcpy(buffer + target, data, length);
			resize_attribute(entry, attr, index, length, length);
			if (optv > 1) {
				printf("new index at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
					buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "expanded"));
		}
	}
	return (err);
}

static int redo_compensate(ntfs_volume *vol __attribute__((unused)),
			const struct ACTION_RECORD *action,
			char *buffer __attribute__((unused)))
{
	u64 lsn;
	s64 diff;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	lsn = sle64_to_cpu(action->record.this_lsn);
	diff = lsn - restart_lsn;
	if (diff > 0)
		restart_lsn = lsn;
	return (0);
}

static int redo_create_file(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	MFT_RECORD *record;
	u32 target;
	u32 length;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	record = (MFT_RECORD*)buffer;
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(buffer,mftrecsz);
	}
	if ((target + length) <= mftrecsz) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed || !(record->flags & MFT_RECORD_IN_USE)) {
			memcpy(buffer + target, data, length);
			record->flags |= MFT_RECORD_IN_USE;
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer,mftrecsz);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	} else {
		err = 1; /* record overflows */
	}
	return (err);
}

static int redo_create_attribute(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
// Could also be AT_DATA or AT_INDEX_ALLOCATION
	if (!action->record.undo_length)
		err = insert_resident(vol, action, buffer, data,
				target, length);
	return (err);
}

static int redo_delete_attribute(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (!action->record.redo_length)
		err = remove_resident(vol, action, buffer, data,
				target, length);
	return (err);
}

static int redo_delete_file(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	MFT_RECORD *record;
	u32 target;
	u32 length;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(buffer,mftrecsz);
	}
	record = (MFT_RECORD*)buffer;
	if ((target + length) <= mftrecsz) {
		/* write a void mft entry (needed ?) */
		changed = (length && memcmp(buffer + target, data, length))
			|| (record->flags & MFT_RECORD_IN_USE);
		err = 0;
		if (changed) {
			memcpy(buffer + target, data, length);
			record->flags &= ~MFT_RECORD_IN_USE;
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer,mftrecsz);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int redo_delete_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	INDEX_BLOCK *indx;
	u32 target;
	u32 length;
	u32 xsize;
	u32 indexlth;
	int err;
	BOOL found;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)) {
		/* This has to be an idempotent action */
		found = (action->record.undo_operation
				== const_cpu_to_le16(CompensationlogRecord))
		    || !memcmp(buffer + target, data, length);
		err = 0;
		if (found) {
			/* Remove the entry */
			memmove(buffer + target,
				buffer + target + length,
				xsize - target - length);
			indexlth = le32_to_cpu(indx->index.index_length)
						- length;
			indx->index.index_length = cpu_to_le32(indexlth);
			err = write_protected(vol, &action->record,
						buffer, xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(found ? "removed" : "unchanged"));
		}
	}
	return (err);
}

static int redo_delete_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	INDEX_ROOT *index;
	BOOL found;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);

	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	index = (INDEX_ROOT*)(((char*)attr)
			+ le16_to_cpu(attr->value_offset));
	if (optv > 1) {
		printf("existing index :\n");
		dump(buffer + target,length);
	}
	if ((attr->type == AT_INDEX_ROOT)
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		found = (action->record.undo_operation
				== const_cpu_to_le16(CompensationlogRecord))
			|| !memcmp(buffer + target, data, length);
		err = 0;
		/* Only delete if present */
		if (found) {
			/* Remove the entry */
			memmove(buffer + target,
				buffer + target + length,
				mftrecsz - target - length);
			resize_attribute(entry, attr, index, -length, -length);
			if (optv > 1) {
				printf("new index at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
					buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "shrinked" : "updated"));
		}
	}
	return (err);
}

static int redo_force_bits(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const struct BITMAP_ACTION *data;
	u32 i;
	int err;
	int wanted;
	u32 firstbit;
	u32 count;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = (const struct BITMAP_ACTION*)
			(((const char*)&action->record)
				+ get_redo_offset(&action->record));
	firstbit = le32_to_cpu(data->firstbit);
	count = le32_to_cpu(data->count);
	if (action->record.redo_operation
			== const_cpu_to_le16(SetBitsInNonResidentBitMap))
		wanted = 1;
	else
		wanted = 0;
// TODO consistency undo_offset == redo_offset, etc.
// firstbit + count < 8*clustersz (multiple clusters possible ?)
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx firstbit %d count %d wanted %d\n",
			(long long)lcn,(int)firstbit,(int)count,(int)wanted);
	}
	for (i=0; i<count; i++)
		ntfs_bit_set((u8*)buffer, firstbit + i, wanted);
	if (!write_raw(vol, &action->record, buffer)) {
		err = 0;
		if (optv > 1)
			printf("-> record updated\n");
	}
	if (err)
		printf("** redo_clearbits failed\n");
	return (err);
}

static int redo_open_attribute(ntfs_volume *vol __attribute__((unused)),
				const struct ACTION_RECORD *action)
{
	const char *data;
	struct ATTR *pa;
	const ATTR_OLD *attr_old;
	const ATTR_NEW *attr_new;
	const char *name;
	le64 inode;
	u32 namelen;
	u32 length;
	u32 extra;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	extra = get_extra_offset(&action->record);
	if (action->record.undo_length) {
		name = ((const char*)&action->record) + extra;
		namelen = le32_to_cpu(action->record.client_data_length)
				+ LOG_RECORD_HEAD_SZ - extra;
		/* fix namelen which was aligned modulo 8 */
		namelen = fixnamelen(name, namelen);
		if (optv > 1) {
			printf("-> length %d namelen %d",(int)length,
							(int)namelen);
			showname(", ", name, namelen/2);
		}
	} else {
		name = "";
		namelen = 0;
	}
	pa = getattrentry(le16_to_cpu(action->record.target_attribute),0);
	if (pa) {
		if (optv) {
			/*
			 * If the actions have been displayed, the
			 * attribute has already been fed. Check
			 * whether it matches what we have in store.
			 */
			switch (length) {
			case sizeof(ATTR_OLD) :
				attr_old = (const ATTR_OLD*)data;
					/* Badly aligned */
				memcpy(&inode, &attr_old->inode, 8);
				err = (MREF(le64_to_cpu(inode)) != pa->inode)
				    || (attr_old->type != pa->type);
				break;
			case sizeof(ATTR_NEW) :
				attr_new = (const ATTR_NEW*)data;
				err = (MREF(le64_to_cpu(attr_new->inode))
							!= pa->inode)
				    || (attr_new->type != pa->type);
				break;
			default : err = 1;
			}
			if (!err) {
				err = (namelen != pa->namelen)
					|| (namelen
				    	&& memcmp(name, pa->name, namelen));
			}
			if (optv > 1)
				printf("-> attribute %s the recorded one\n",
					(err ? "does not match" : "matches"));
		} else {
			copy_attribute(pa, data, length);
			pa->namelen = namelen;
			if (namelen)
				memcpy(pa->name, data, namelen);
			err = 0;
		}
	} else
		if (optv)
			printf("* Unrecorded attribute\n");
	return (err);
}

static int redo_sizes(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(ATTR_RECORD, allocated_size);
	err = change_resident(vol, action, buffer,
			data, target, length);
	return (err);
}

static int redo_update_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
			/* target is left-justified to creation time */
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(INDEX_ENTRY, key.file_name.creation_time);
	err = update_index(vol, action, buffer, data, target, length);
	return (err);
}

static int redo_update_index_value(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 length;
	u32 target;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
				+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	err = change_index_value(vol, action, buffer, data, target, length);
	return (err);
}

static int redo_update_mapping(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	u32 target;
	u32 length;
	u32 source;
	u32 alen;
	u32 newused;
	int resize;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	resize = length - le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (!attr->non_resident) {
		printf("** Error : update_mapping on resident attr\n");
	}
	if (valid_type(attr->type)
	    && attr->non_resident
	    && !(resize & 7)
	    && ((target + length) <= mftrecsz)) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			/* Adjust space for new mapping pairs */
			source = target - resize;
			if (resize > 0) {
				memmove(buffer + target + length,
					buffer + source + length,
					mftrecsz - target - length);
			}
			if (resize < 0) {
				memmove(buffer + target + length,
					buffer + source + length,
					mftrecsz - source - length);
			}
			memcpy(buffer + target, data, length);
				/* Resize the attribute */
			alen = le32_to_cpu(attr->length) + resize;
			attr->length = cpu_to_le32(alen);
				/* Resize the mft record */
			newused = le32_to_cpu(entry->bytes_in_use)
					+ resize;
			entry->bytes_in_use = cpu_to_le32(newused);
				/* Compute the new highest_vcn */
			err = adjust_high_vcn(vol, attr);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			if (!err) {
				err = write_protected(vol,
					&action->record,
					buffer, mftrecsz);
			}
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int redo_update_resident(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 target;
	u32 length;
	u32 oldlength;
	u32 end;
	u32 redo;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	end = le32_to_cpu(action->record.client_data_length)
			+ LOG_RECORD_HEAD_SZ;
	length = le16_to_cpu(action->record.redo_length);
	redo = get_redo_offset(&action->record);
	if ((redo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + redo;
	oldlength = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (length == oldlength) {
		if (optv > 1) {
			lcn = sle64_to_cpu(action->record.lcn_list[0]);
			printf("-> inode %lld lcn 0x%llx target 0x%x"
				" length %d\n",
				(long long)inode_number(&action->record),
				(long long)lcn, (int)target, (int)length);
		}
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= mftrecsz) {
			changed = (action->record.undo_operation
				    == const_cpu_to_le16(CompensationlogRecord))
				|| memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_protected(vol, &action->record,
					buffer, mftrecsz);
			}
			if (optv > 1) {
				printf("-> MFT record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		if (length > oldlength)
			err = expand_resident(vol, action, buffer, data,
					target, length, oldlength);
		else
			err = shrink_resident(vol, action, buffer, data,
					target, length, oldlength);
	}
	return (err);
}

static int redo_update_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	const char *expected;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	expected = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
		/* the fixup is right-justified to the name length */
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(INDEX_ENTRY, key.file_name.file_name_length)
		- length;
	if (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord))
		err = change_resident(vol, action, buffer, data,
			target, length);
	else
		err = change_resident_expect(vol, action, buffer, data,
			expected, target, length, AT_INDEX_ROOT);
	return (err);
}

static int redo_update_root_vcn(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	const char *expected;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	expected = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	if (length == 8) {
		target = le16_to_cpu(action->record.record_offset)
			+ le16_to_cpu(action->record.attribute_offset);
		/* target is right-justified to end of attribute */
		target += getle16(buffer, target + 8) - length;
		err = change_resident_expect(vol, action, buffer, data,
				expected, target, length, AT_INDEX_ROOT);
	}
	return (err);
}

static int redo_update_value(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 length;
	u32 target;
	u32 count;
	u32 redo;
	u32 end;
	u32 i;
	int changed;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	length = le16_to_cpu(action->record.redo_length);
	redo = get_redo_offset(&action->record);
	end = le32_to_cpu(action->record.client_data_length)
				+ LOG_RECORD_HEAD_SZ;
		/* sometimes there is no redo data */
	if ((redo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + redo;
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	count = le16_to_cpu(action->record.lcns_to_follow);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((target + length) <= (count << clusterbits)) {
		if (data)
			changed = memcmp(buffer + target, data, length);
		else {
			for (i=0; (i<length) && !buffer[target+i]; i++) { }
			changed = length && (i < length);
		}
		err = 0;
		if (changed) {
			if (data)
				memcpy(buffer + target, data, length);
			else
				memset(buffer + target, 0, length);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			err = write_raw(vol, &action->record, buffer);
		}
		if (optv > 1) {
			printf("-> data record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}

	return (err);
}

static int redo_update_vcn(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	if (length == 8) {
		target = le16_to_cpu(action->record.record_offset)
			+ le16_to_cpu(action->record.attribute_offset);
		/* target is right-justified to end of attribute */
		target += getle16(buffer, target + 8) - length;
		err = update_index(vol, action, buffer, data, target, length);
	}
	return (err);
}

static int redo_write_end(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 target;
	u32 length;
	u32 oldlength;
	u32 end;
	u32 redo;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	end = le32_to_cpu(action->record.client_data_length)
			+ LOG_RECORD_HEAD_SZ;
	length = le16_to_cpu(action->record.redo_length);
	redo = get_redo_offset(&action->record);
	if ((redo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + redo;
	oldlength = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (length == oldlength) {
		if (optv > 1) {
			lcn = sle64_to_cpu(action->record.lcn_list[0]);
			printf("-> inode %lld lcn 0x%llx target 0x%x"
				" length %d\n",
				(long long)inode_number(&action->record),
				(long long)lcn, (int)target, (int)length);
		}
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= mftrecsz) {
			changed = memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_protected(vol, &action->record,
						buffer, mftrecsz);
			}
			if (optv > 1) {
				printf("-> MFT record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		if (length > oldlength)
			err = add_resident(vol, action, buffer, data,
					target, length, oldlength);
		else
			err = delete_resident(vol, action, buffer, data,
					target, length, oldlength);
	}
	return (err);
}

static int redo_write_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	INDEX_BLOCK *indx;
	u32 target;
	u32 length;
	u32 xsize;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
			/* target is left-justified to creation time */
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)buffer;
	if (action->record.record_offset) {
		printf("** Non-null record_offset in redo_write_index()\n");
	}
	if (optv > 1) {
		printf("-> existing index :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)) {
		/* This has to be an idempotent action */
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			/* Update the entry */
			memcpy(buffer + target, data, length);
			/* If truncating, set the new size */
			indx->index.index_length =
					cpu_to_le32(target + length - 0x18);
			if (optv > 1) {
				printf("-> new index :\n");
				dump(&buffer[target], length);
			}
			err = write_protected(vol, &action->record,
					buffer, xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int undo_action37(ntfs_volume *vol __attribute__((unused)),
				const struct ACTION_RECORD *action,
				char *buffer __attribute__((unused)))
{
/*
	const char *data;
	u32 target;
	u32 length;
*/
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
/*
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
*/
	printf("* Ignored action-37, inode %lld record :\n",
			(long long)inode_number(&action->record));
	err = 0;
	return (err);
}

static int undo_add_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	INDEX_BLOCK *indx;
	u32 target;
	u32 length;
	u32 xsize;
	u32 indexlth;
	int err;
	BOOL found;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)) {
		/* This has to be an idempotent action */
		found = index_match_undo(buffer + target, data, length);
		err = 0;
		if (found) {
			/* Remove the entry */
			memmove(buffer + target,
				buffer + target + length,
				xsize - target - length);
			indexlth = le32_to_cpu(indx->index.index_length)
					- length;
			indx->index.index_length = cpu_to_le32(indexlth);
			err = write_protected(vol, &action->record,
						buffer, xsize);
		} else {
			sanity_indx(vol,buffer);
			printf("full record :\n");
			dump(buffer,xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(found ? "removed" : "unchanged")); 
		}
	}
	return (err);
}

static int undo_add_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	INDEX_ROOT *index;
	BOOL found;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	index = (INDEX_ROOT*)(((char*)attr)
			+ le16_to_cpu(attr->value_offset));
	if (optv > 1) {
		printf("existing index :\n");
		dump(buffer + target,length);
	}
	if ((attr->type == AT_INDEX_ROOT)
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		found = index_match_undo(buffer + target, data, length);
		err = 0;
		if (found && !older_record(entry, &action->record)) {
			/* Remove the entry */
			memmove(buffer + target,
				buffer + target + length,
				mftrecsz - target - length);
			resize_attribute(entry, attr, index, -length, -length);
			if (optv > 1) {
				printf("new index at same location :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "shrinked" : "unchanged"));
		}
	}
	return (err);
}

static int undo_create_attribute(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (!action->record.undo_length)
		err = remove_resident(vol, action, buffer, data,
				target, length);
	return (err);
}

static int undo_delete_attribute(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (!action->record.redo_length)
		err = insert_resident(vol, action, buffer, data,
				target, length);
	return (err);
}

static int undo_delete_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	INDEX_BLOCK *indx;
	u32 target;
	u32 length;
	u32 xsize;
	u32 indexlth;
	int err;
	BOOL found;

// MERGE with redo_add_root_index() ?
	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	xsize = vol->indx_record_size;
	indx = (INDEX_BLOCK*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((indx->magic == magic_INDX)
	    && !(length & 7)
	    && ((target + length) <= xsize)
	    && !sanity_indx(vol,buffer)) {
		/* This has to be an idempotent action */
		found = !memcmp(buffer + target, data, length);
		err = 0;
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + length,
				buffer + target,
				xsize - target - length);
			memcpy(buffer + target, data, length);
			indexlth = le32_to_cpu(indx->index.index_length)
					+ length;
			indx->index.index_length = cpu_to_le32(indexlth);
			if (optv > 1) {
				printf("-> inserted record :\n");
				dump(&buffer[target], length);
			}
			/* rebuildname() has no effect currently, should drop */
			rebuildname((const INDEX_ENTRY*)data);
			err = write_protected(vol, &action->record,
						buffer, xsize);
		}
		if (optv > 1) {
			printf("-> INDX record %s\n",
				(found ? "unchanged" : "inserted"));
		}
	}
	return (err);
}

static int undo_delete_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	INDEX_ROOT *index;
	u32 target;
	u32 length;
	int err;
	BOOL found;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	index = (INDEX_ROOT*)(((char*)attr)
			+ le16_to_cpu(attr->value_offset));
	if (attr->type != AT_INDEX_ROOT) {
		printf("** Unexpected attr type 0x%lx\n",
				(long)le32_to_cpu(attr->type));
		printf("existing mft\n");
		dump((char*)buffer,512);
		printf("existing index\n");
		dump(buffer + target,length);
	}
	if (optv > 1) {
		printf("existing index :\n");
		dump(buffer + target,length);
	}
	if ((attr->type == AT_INDEX_ROOT)
	    && !(length & 7)
	    && ((target + length) <= mftrecsz)) {
		/* This has to be an idempotent action */
		found = !memcmp(buffer + target, data, length);
		err = 0;
			/* Do not insert if present */
		if (!found) {
			/* Make space to insert the entry */
			memmove(buffer + target + length,
				buffer + target,
				mftrecsz - target - length);
			memcpy(buffer + target, data, length);
			resize_attribute(entry, attr, index, length, length);
			if (optv > 1) {
				printf("new index :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(found ? "unchanged" : "expanded"));
		}
	}
	return (err);
}

static int undo_create_file(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	MFT_RECORD *record;
	u32 target;
	u32 length;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
		/* redo initialize, clearing the in_use flag ? */
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	record = (MFT_RECORD*)buffer;
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(buffer,mftrecsz);
	}
	if ((target + length) <= mftrecsz) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed || (record->flags & MFT_RECORD_IN_USE)) {
			memcpy(buffer + target, data, length);
			record->flags &= ~MFT_RECORD_IN_USE;
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer,mftrecsz);
			}
			err = write_protected(vol, &action->record,
						buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int undo_delete_file(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	MFT_RECORD *record;
	u32 target;
	u32 length;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length);
	}
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(buffer,mftrecsz);
	}
	record = (MFT_RECORD*)buffer;
	if ((target + length) <= mftrecsz) {
		changed = memcmp(buffer + target, data, length)
			|| !(record->flags & MFT_RECORD_IN_USE);
		err = 0;
		if (changed) {
			memcpy(buffer + target, data, length);
		/*
		 * Unclear what we should do for recreating a file.
		 * Only 24 bytes are available, the used length is not known,
		 * the number of links suggests we should keep the current
		 * names... If so, when will they be deleted ?
		 * We will have to make stamp changes in the standard
		 * information attribute, so better not to delete it.
		 * Should we create a data or index attribute ?
		 * Here, we assume we should delete the file names when
		 * the record now appears to not be in use and there are
		 * links.
		 */
			if (record->link_count
			    && !(record->flags & MFT_RECORD_IN_USE))
				err = delete_names(buffer);
			record->flags |= MFT_RECORD_IN_USE;
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer,mftrecsz);
			}
			if (!err)
				err = write_protected(vol,
					&action->record,
					buffer, mftrecsz);
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int undo_force_bits(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const struct BITMAP_ACTION *data;
	u32 i;
	int err;
	int wanted;
	u32 firstbit;
	u32 count;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = (const struct BITMAP_ACTION*)
			(((const char*)&action->record)
				+ get_redo_offset(&action->record));
	firstbit = le32_to_cpu(data->firstbit);
	count = le32_to_cpu(data->count);
	if (action->record.redo_operation
			== const_cpu_to_le16(SetBitsInNonResidentBitMap))
		wanted = 0;
	else
		wanted = 1;
// TODO consistency undo_offset == redo_offset, etc.
// firstbit + count < 8*clustersz (multiple clusters possible ?)
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx firstbit %d count %d wanted %d\n",
			(long long)lcn,(int)firstbit,(int)count,(int)wanted);
	}
	for (i=0; i<count; i++)
		ntfs_bit_set((u8*)buffer, firstbit + i, wanted);
	if (!write_raw(vol, &action->record, buffer)) {
		err = 0;
		if (optv > 1)
			printf("-> record updated\n");
	}
	if (err)
		printf("** redo_clearbits failed\n");
	return (err);
}

static int undo_open_attribute(ntfs_volume *vol __attribute__((unused)),
				const struct ACTION_RECORD *action)
{
	const char *data;
	struct ATTR *pa;
	const ATTR_OLD *attr_old;
	const ATTR_NEW *attr_new;
	const char *name;
	le64 inode;
	u32 namelen;
	u32 length;
	u32 extra;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.redo_length);
	extra = get_extra_offset(&action->record);
	if (action->record.undo_length) {
		name = ((const char*)&action->record) + extra;
		namelen = le32_to_cpu(action->record.client_data_length)
				+ LOG_RECORD_HEAD_SZ - extra;
		/* fix namelen which was aligned modulo 8 */
		namelen = fixnamelen(name, namelen);
		if (optv > 1) {
			printf("-> length %d namelen %d",(int)length,
							(int)namelen);
			showname(", ", name, namelen/2);
		}
	} else {
		namelen = 0;
		name = "";
	}
	pa = getattrentry(le16_to_cpu(action->record.target_attribute),0);
// TODO Only process is attr is not older ?
	if (pa) {
		/* check whether the redo attr matches what we have in store */
		switch (length) {
		case sizeof(ATTR_OLD) :
			attr_old = (const ATTR_OLD*)data;
				/* Badly aligned */
			memcpy(&inode, &attr_old->inode, 8);
			err = (MREF(le64_to_cpu(inode)) != pa->inode)
			    || (attr_old->type != pa->type);
			break;
		case sizeof(ATTR_NEW) :
			attr_new = (const ATTR_NEW*)data;
			err = (MREF(le64_to_cpu(attr_new->inode))!= pa->inode)
			    || (attr_new->type != pa->type);
			break;
		default : err = 1;
		}
		if (!err) {
			err = (namelen != pa->namelen)
				|| (namelen
				    && memcmp(name, pa->name, namelen));
		}
		if (optv > 1)
			printf("-> attribute %s the recorded one\n",
				(err ? "does not match" : "matches"));
	} else
		if (optv)
			printf("* Unrecorded attribute\n");
err = 0;
	return (err);
}

static int undo_sizes(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	MFT_RECORD *entry;
	ATTR_RECORD *attr;
	u32 target;
	u32 length;
	u32 offs;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(ATTR_RECORD, allocated_size);
	entry = (MFT_RECORD*)buffer;
	if (!(entry->flags & MFT_RECORD_IS_DIRECTORY))
		err = change_resident(vol, action, buffer,
			data, target, length);
	else {
		/* On a directory, may have to build an index allocation */
		offs = le16_to_cpu(action->record.record_offset);
		attr = (ATTR_RECORD*)(buffer + offs);
		if (attr->type != AT_INDEX_ALLOCATION) {
			err = insert_index_allocation(vol, buffer, offs);
			if (!err)
				err = change_resident(vol, action, buffer,
						data, target, length);
		} else
			err = change_resident(vol, action, buffer,
					data, target, length);
	}
	return (err);
}

static int undo_update_index(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
			/* target is left-justified to creation time */
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(INDEX_ENTRY, key.file_name.creation_time);
	err = update_index(vol, action, buffer, data, target, length);
	return (err);
}

static int undo_update_index_value(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 length;
	u32 target;
	int changed;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
				+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	if ((target + length) <= vol->indx_record_size) {
		changed = length && memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			memcpy(buffer + target, data, length);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			err = write_protected(vol, &action->record, buffer,
						vol->indx_record_size);
		}
		if (optv > 1) {
			printf("-> data record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int undo_update_vcn(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	const char *data;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	if (length == 8) {
		target = le16_to_cpu(action->record.record_offset)
			+ le16_to_cpu(action->record.attribute_offset);
		/* target is right-justified to end of attribute */
		target += getle16(buffer, target + 8) - length;
		err = update_index(vol, action, buffer, data, target, length);
	}
	return (err);
}

static int undo_update_mapping(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	LCN lcn;
	const char *data;
	ATTR_RECORD *attr;
	MFT_RECORD *entry;
	u32 target;
	u32 length;
	u32 source;
	u32 alen;
	u32 newused;
	int err;
	int changed;
	int resize;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	resize = length - le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> inode %lld lcn 0x%llx target 0x%x new length %d resize %d\n",
			(long long)inode_number(&action->record),
			(long long)lcn, (int)target, (int)length, (int)resize);
	}
// TODO share with redo_update_mapping()
	if (optv > 1) {
		printf("-> existing record :\n");
		dump(&buffer[target], length);
	}
	entry = (MFT_RECORD*)buffer;
	attr = (ATTR_RECORD*)(buffer
			+ le16_to_cpu(action->record.record_offset));
	if (!attr->non_resident) {
		printf("** Error : update_mapping on resident attr\n");
	}
	if (valid_type(attr->type)
	    && attr->non_resident
	    && !(resize & 7)
	    && ((target + length) <= mftrecsz)) {
		changed = memcmp(buffer + target, data, length);
		err = 0;
		if (changed) {
			/* Adjust space for new mapping pairs */
			source = target - resize;
			if (resize > 0) {
				memmove(buffer + target + length,
					buffer + source + length,
					mftrecsz - target - length);
			}
			if (resize < 0) {
				memmove(buffer + target + length,
					buffer + source + length,
					mftrecsz - source - length);
			}
			memcpy(buffer + target, data, length);
				/* Resize the attribute */
			alen = le32_to_cpu(attr->length) + resize;
			attr->length = cpu_to_le32(alen);
				/* Resize the mft record */
			newused = le32_to_cpu(entry->bytes_in_use)
					+ resize;
			entry->bytes_in_use = cpu_to_le32(newused);
				/* Compute the new highest_vcn */
			err = adjust_high_vcn(vol, attr);
			if (optv > 1) {
				printf("-> new record :\n");
				dump(buffer + target, length);
			}
			if (!err) {
				err = write_protected(vol,
					&action->record, buffer,
					mftrecsz);
			}
		}
		if (optv > 1) {
			printf("-> MFT record %s\n",
				(changed ? "updated" : "unchanged"));
		}
	}
	return (err);
}

static int undo_update_resident(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 target;
	u32 length;
	u32 oldlength;
	u32 end;
	u32 undo;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	end = le32_to_cpu(action->record.client_data_length)
			+ LOG_RECORD_HEAD_SZ;
	length = le16_to_cpu(action->record.undo_length);
	undo = get_undo_offset(&action->record);
	if ((undo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + undo;
	oldlength = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (length == oldlength) {
		if (optv > 1) {
			lcn = sle64_to_cpu(action->record.lcn_list[0]);
			printf("-> inode %lld lcn 0x%llx target 0x%x length %d\n",
				(long long)inode_number(&action->record),
				(long long)lcn, (int)target, (int)length);
		}
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= mftrecsz) {
			changed = memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_protected(vol, &action->record,
						buffer, mftrecsz);
			}
			if (optv > 1) {
				printf("-> MFT record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		if (length > oldlength)
			err = expand_resident(vol, action, buffer, data,
					target, length, oldlength);
		else
			err = shrink_resident(vol, action, buffer, data,
					target, length, oldlength);
	}
	return (err);
}

static int undo_update_root_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	const char *expected;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	expected = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
		/* the fixup is right-justified to the name length */
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset)
		+ offsetof(INDEX_ENTRY, key.file_name.file_name_length)
		- length;
	err = change_resident_expect(vol, action, buffer, data, expected,
			target, length, AT_INDEX_ROOT);
	return (err);
}

static int undo_update_root_vcn(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	const char *data;
	const char *expected;
	u32 target;
	u32 length;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
			+ get_undo_offset(&action->record);
	expected = ((const char*)&action->record)
			+ get_redo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	if (length == 8) {
		target = le16_to_cpu(action->record.record_offset)
			+ le16_to_cpu(action->record.attribute_offset);
		/* target is right-justified to end of attribute */
		target += getle16(buffer, target + 8) - length;
		err = change_resident_expect(vol, action, buffer, data,
				expected, target, length, AT_INDEX_ROOT);
	}
	return (err);
}

static int undo_update_value(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 length;
	u32 target;
	u32 count;
	int changed;
	int err;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	data = ((const char*)&action->record)
				+ get_undo_offset(&action->record);
	length = le16_to_cpu(action->record.undo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	count = le16_to_cpu(action->record.lcns_to_follow);
	if (optv > 1) {
		lcn = sle64_to_cpu(action->record.lcn_list[0]);
		printf("-> lcn 0x%llx target 0x%x length %d\n",
			(long long)lcn, (int)target, (int)length);
	}
	if (length) {
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= (count << clusterbits)) {
			changed = memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_raw(vol, &action->record, buffer);
			}
			if (optv > 1) {
				printf("-> data record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		/*
		 * No undo data, we cannot undo, sometimes the redo
		 * data even overflows from record.
		 * Just ignore for now.
		 */
		if (optv)
			printf("Cannot undo, there is no undo data\n");
		err = 0;
	}

	return (err);
}

static int undo_write_end(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 target;
	u32 length;
	u32 oldlength;
	u32 end;
	u32 undo;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	end = le32_to_cpu(action->record.client_data_length)
			+ LOG_RECORD_HEAD_SZ;
	length = le16_to_cpu(action->record.undo_length);
	undo = get_undo_offset(&action->record);
	if ((undo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + undo;
	oldlength = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (length == oldlength) {
		if (optv > 1) {
			lcn = sle64_to_cpu(action->record.lcn_list[0]);
			printf("-> inode %lld lcn 0x%llx target 0x%x"
				" length %d\n",
				(long long)inode_number(&action->record),
				(long long)lcn, (int)target, (int)length);
		}
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= mftrecsz) {
			changed = memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_protected(vol, &action->record,
						buffer, mftrecsz);
			}
			if (optv > 1) {
				printf("-> MFT record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		if (length > oldlength)
			err = add_resident(vol, action, buffer, data,
					target, length, oldlength);
		else
			err = delete_resident(vol, action, buffer, data,
					target, length, oldlength);
	}
	return (err);
}

static int undo_write_index(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	LCN lcn;
	const char *data;
	u32 target;
	u32 length;
	u32 oldlength;
	u32 end;
	u32 undo;
	int err;
	int changed;

	if (optv > 1)
		printf("-> %s()\n",__func__);
	err = 1;
	end = le32_to_cpu(action->record.client_data_length)
			+ LOG_RECORD_HEAD_SZ;
	length = le16_to_cpu(action->record.undo_length);
	undo = get_undo_offset(&action->record);
	if ((undo + length) > end)
		data = (char*)NULL;
	else
		data = ((const char*)&action->record) + undo;
	oldlength = le16_to_cpu(action->record.redo_length);
	target = le16_to_cpu(action->record.record_offset)
		+ le16_to_cpu(action->record.attribute_offset);
	if (length == oldlength) {
		if (optv > 1) {
			lcn = sle64_to_cpu(action->record.lcn_list[0]);
			printf("-> inode %lld lcn 0x%llx target 0x%x"
				" length %d\n",
				(long long)inode_number(&action->record),
				(long long)lcn, (int)target, (int)length);
		}
		if (optv > 1) {
			printf("-> existing record :\n");
			dump(&buffer[target], length);
		}
		if ((target + length) <= mftrecsz) {
			changed = memcmp(buffer + target, data, length);
			err = 0;
			if (changed) {
				memcpy(buffer + target, data, length);
				if (optv > 1) {
					printf("-> new record :\n");
					dump(buffer + target, length);
				}
				err = write_protected(vol, &action->record,
						buffer, mftrecsz);
			}
			if (optv > 1) {
				printf("-> MFT record %s\n",
					(changed ? "updated" : "unchanged"));
			}
		}
	} else {
		if (length > oldlength)
			err = add_non_resident(/*vol, action, data,
					target, length, oldlength*/);
		else
			err = delete_non_resident(/*vol, action, data,
					target, length, oldlength*/);
	}
	return (err);
}

enum ACTION_KIND { ON_NONE, ON_MFT, ON_INDX, ON_RAW } ;

static enum ACTION_KIND get_action_kind(const struct ACTION_RECORD *action)
{
	struct ATTR *pa;
	const char *data;
	enum ACTION_KIND kind;
		/*
		 * If we are sure the action was defined by Vista
		 * or subsequent, just use attribute_flags.
		 * Unfortunately, only on some cases we can determine
		 * the action was defined by Win10 (or subsequent).
		 */
	if (action->record.log_record_flags
			& (LOG_RECORD_DELETING | LOG_RECORD_ADDING)) {
		if (action->record.attribute_flags & ACTS_ON_INDX)
			kind = ON_INDX;
		else
			if (action->record.attribute_flags & ACTS_ON_MFT)
				kind = ON_MFT;
			else
				kind = ON_RAW;
	} else {
		/*
		 * In other cases, we have to rely on the attribute
		 * definition, but this has defects when undoing.
		 */
		pa = getattrentry(le16_to_cpu(
					action->record.target_attribute),0);
		if (!pa || !pa->type) {
		/*
		 * Even when the attribute has not been recorded,
		 * we can sometimes tell the record does not apply
		 * to MFT or INDX : such records always have a zero
		 * record_offset, and if attribute_offset is zero, their
		 * magic can be checked. If neither condition is true,
		 * the action cannot apply to MFT or INDX.
		 * (this is useful for undoing)
		 */
			data = (const char*)&action->record
				+ get_redo_offset(&action->record);
			if (action->record.record_offset
			    || (!action->record.attribute_offset
				&& (le16_to_cpu(action->record.redo_length)
									>= 4)
				&& memcmp(data,"FILE",4)
				&& memcmp(data,"INDX",4))) {
					kind = ON_RAW;
			} else {
				printf("** Error : attribute 0x%x"
					" is not defined\n",
					(int)le16_to_cpu(
					action->record.target_attribute));
				kind = ON_NONE;
			}
		} else {
			if (pa->type == AT_INDEX_ALLOCATION)
				kind = ON_INDX;
			else
				kind = ON_RAW;
		}
	}
	return (kind);
}


/*
 *		Display the redo actions which were executed
 *
 *	Useful for getting indications on the coverage of a test
 */

void show_redos(void)
{
	int i;

	if (optv && redos_met) {
		printf("Redo actions which were executed :\n");
		for (i=0; i<64; i++)
			if ((((u64)1) << i) & redos_met)
				printf("%s\n", actionname(i));
	}
}

static int distribute_redos(ntfs_volume *vol,
			const struct ACTION_RECORD *action, char *buffer)
{
	int rop, uop;
	int err;

	err = 0;
	rop = le16_to_cpu(action->record.redo_operation);
	uop = le16_to_cpu(action->record.undo_operation);
	switch (rop) {
	case AddIndexEntryAllocation :
		if (action->record.undo_operation
		    == const_cpu_to_le16(DeleteIndexEntryAllocation))
			err = redo_add_index(vol, action, buffer);
		break;
	case AddIndexEntryRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(DeleteIndexEntryRoot))
			err = redo_add_root_index(vol, action, buffer);
		break;
	case ClearBitsInNonResidentBitMap :
		if ((action->record.undo_operation
			== const_cpu_to_le16(SetBitsInNonResidentBitMap))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_force_bits(vol, action, buffer);
		break;
	case CompensationlogRecord :
		if (action->record.undo_operation
		    == const_cpu_to_le16(Noop))
			err = redo_compensate(vol, action, buffer);
		break;
	case CreateAttribute :
		if ((action->record.undo_operation
			== const_cpu_to_le16(DeleteAttribute))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_create_attribute(vol, action, buffer);
		break;
	case DeallocateFileRecordSegment :
		if ((action->record.undo_operation
			== const_cpu_to_le16(InitializeFileRecordSegment))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_delete_file(vol, action, buffer);
		break;
	case DeleteAttribute :
		if ((action->record.undo_operation
			== const_cpu_to_le16(CreateAttribute))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_delete_attribute(vol, action, buffer);
		break;
	case DeleteIndexEntryAllocation :
		if ((action->record.undo_operation
			== const_cpu_to_le16(AddIndexEntryAllocation))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_delete_index(vol, action, buffer);
		break;
	case DeleteIndexEntryRoot :
		if ((action->record.undo_operation
			== const_cpu_to_le16(AddIndexEntryRoot))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_delete_root_index(vol, action, buffer);
		break;
	case InitializeFileRecordSegment :
		if (action->record.undo_operation
		        == const_cpu_to_le16(Noop))
			err = redo_create_file(vol, action, buffer);
		break;
	case OpenNonResidentAttribute :
		if (action->record.undo_operation
		        == const_cpu_to_le16(Noop))
			err = redo_open_attribute(vol, action);
		break;
	case SetBitsInNonResidentBitMap :
		if (action->record.undo_operation
		    == const_cpu_to_le16(ClearBitsInNonResidentBitMap))
			err = redo_force_bits(vol, action, buffer);
		break;
	case SetIndexEntryVcnAllocation :
		if ((action->record.undo_operation
			== const_cpu_to_le16(SetIndexEntryVcnAllocation))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_update_vcn(vol, action, buffer);
		break;
	case SetIndexEntryVcnRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(SetIndexEntryVcnRoot))
			err = redo_update_root_vcn(vol, action, buffer);
		break;
	case SetNewAttributeSizes :
		if ((action->record.undo_operation
			== const_cpu_to_le16(SetNewAttributeSizes))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_sizes(vol, action, buffer);
		break;
	case UpdateFileNameAllocation :
		if ((action->record.undo_operation
			== const_cpu_to_le16(UpdateFileNameAllocation))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_update_index(vol, action, buffer);
		break;
	case UpdateFileNameRoot :
		if ((action->record.undo_operation
			== const_cpu_to_le16(UpdateFileNameRoot))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_update_root_index(vol, action, buffer);
		break;
	case UpdateMappingPairs :
		if (action->record.undo_operation
		    == const_cpu_to_le16(UpdateMappingPairs))
			err = redo_update_mapping(vol, action, buffer);
		break;
	case UpdateNonResidentValue :
		switch (get_action_kind(action)) {
		case ON_INDX :
			err = redo_update_index_value(vol, action, buffer);
			break;
		case ON_RAW :
			err = redo_update_value(vol, action, buffer);
			break;
		default :
			printf("** Bad attribute type\n");
			err = 1;
		}
		break;
	case UpdateResidentValue :
		if ((action->record.undo_operation
			== const_cpu_to_le16(UpdateResidentValue))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_update_resident(vol, action, buffer);
		break;
	case Win10Action37 :
		if (action->record.undo_operation
		    == const_cpu_to_le16(Noop))
			err = redo_action37(vol, action, buffer);
		break;
	case WriteEndofFileRecordSegment :
		if (action->record.undo_operation
		    == const_cpu_to_le16(WriteEndofFileRecordSegment))
			err = redo_write_end(vol, action, buffer);
		break;
	case WriteEndOfIndexBuffer :
		if ((action->record.undo_operation
			== const_cpu_to_le16(WriteEndOfIndexBuffer))
		    || (action->record.undo_operation
			== const_cpu_to_le16(CompensationlogRecord)))
			err = redo_write_index(vol, action, buffer);
		break;
	case AttributeNamesDump :
	case DirtyPageTableDump :
	case ForgetTransaction :
	case Noop :
	case OpenAttributeTableDump :
		break;
	default :
		printf("** Unsupported redo %s\n", actionname(rop));
		err = 1;
		break;
	}
	redos_met |= ((u64)1) << rop;
	if (err)
		printf("* Redoing action %d %s (%s) failed\n",
			action->num,actionname(rop), actionname(uop));
	return (err);
}

/*
 *		Redo a single action
 *
 *	The record the action acts on is read and, when it is an MFT or
 *	INDX one, the need for redoing is checked.
 *
 *	When this is an action which creates a new MFT or INDX record
 *	and the old one cannot be read (usually because it was not
 *	initialized), a zeroed buffer is allocated.
 */

static int play_one_redo(ntfs_volume *vol, const struct ACTION_RECORD *action)
{
	MFT_RECORD *entry;
	INDEX_BLOCK *indx;
	char *buffer;
	s64 this_lsn;
	s64 data_lsn;
	u32 xsize;
	int err;
	BOOL warn;
	BOOL executed;
	enum ACTION_KIND kind;
	u16 rop;
	u16 uop;

	err = 0;
	rop = le16_to_cpu(action->record.redo_operation);
	uop = le16_to_cpu(action->record.undo_operation);
	this_lsn = sle64_to_cpu(action->record.this_lsn);
	if (optv)
		printf("Redo action %d %s (%s) 0x%llx\n",
			action->num,
			actionname(rop), actionname(uop),
			(long long)sle64_to_cpu(
				action->record.this_lsn));
	buffer = (char*)NULL;
	switch (rop) {
			/* Actions always acting on MFT */
	case AddIndexEntryRoot :
	case CreateAttribute :
	case DeallocateFileRecordSegment :
	case DeleteAttribute :
	case DeleteIndexEntryRoot :
	case InitializeFileRecordSegment :
	case SetIndexEntryVcnRoot :
	case SetNewAttributeSizes :
	case UpdateFileNameRoot :
	case UpdateMappingPairs :
	case UpdateResidentValue :
	case Win10Action37 :
	case WriteEndofFileRecordSegment :
		kind = ON_MFT;
		break;
			/* Actions always acting on INDX */
	case AddIndexEntryAllocation :
	case DeleteIndexEntryAllocation :
	case SetIndexEntryVcnAllocation :
	case UpdateFileNameAllocation :
	case WriteEndOfIndexBuffer :
		kind = ON_INDX;
		break;
			/* Actions never acting on MFT or INDX */
	case ClearBitsInNonResidentBitMap :
	case SetBitsInNonResidentBitMap :
		kind = ON_RAW;
		break;
			/* Actions which may act on MFT */
	case Noop : /* on MFT if DeallocateFileRecordSegment */
		kind = ON_NONE;
		break;
			/* Actions which may act on INDX */
	case UpdateNonResidentValue :
		/* Known cases : INDX, $SDS, ATTR_LIST */
		kind = get_action_kind(action);
		if (kind == ON_NONE)
			err = 1;
		break;
	case CompensationlogRecord :
	case OpenNonResidentAttribute :
		/* probably not important */
		kind = ON_NONE;
		break;
			/* Actions currently ignored */
	case AttributeNamesDump :
	case DirtyPageTableDump :
	case ForgetTransaction :
	case OpenAttributeTableDump :
	case TransactionTableDump :
		kind = ON_NONE;
		break;
			/* Actions with no known use case */
	case CommitTransaction :
	case DeleteDirtyClusters :
	case EndTopLevelAction :
	case HotFix :
	case PrepareTransaction :
	case UpdateRecordDataAllocation :
	case UpdateRecordDataRoot :
	case Win10Action35 :
	case Win10Action36 :
	default :
		err = 1;
		kind = ON_NONE;
		break;
	}
	executed = FALSE;
	switch (kind) {
	case ON_MFT :
/*
 the check below cannot be used on WinXP
if (!(action->record.attribute_flags & ACTS_ON_MFT))
printf("** %s (action %d) not acting on MFT\n",actionname(rop),(int)action->num);
*/
		/* Check whether data is to be discarded */
		warn = (rop != InitializeFileRecordSegment)
			|| !check_full_mft(action,TRUE);
		buffer = read_protected(vol, &action->record,
					mftrecsz, warn);
		entry = (MFT_RECORD*)buffer;
		if (entry && (entry->magic == magic_FILE)) {
			data_lsn = sle64_to_cpu(entry->lsn);
			/*
			 * Beware of records not updated
			 * during the last session which may
			 * have a stale lsn (consequence
			 * of ntfs-3g resetting the log)
			 */
			executed = ((s64)(data_lsn - this_lsn) >= 0)
			    && (((s64)(data_lsn - latest_lsn)) <= 0)
			    && !exception(action->num);
		} else {
			if (!warn) {
				/* Old record not needed */
				if (!buffer)
					buffer = (char*)calloc(1, mftrecsz);
				if (buffer)
					executed = FALSE;
				else
					err = 1;
			} else {
				printf("** %s (action %d) not"
					" acting on MFT\n",
					actionname(rop),
					(int)action->num);
				err = 1;
			}
		}
		break;
	case ON_INDX :
/*
 the check below cannot be used on WinXP
if (!(action->record.attribute_flags & ACTS_ON_INDX))
printf("** %s (action %d) not acting on INDX\n",actionname(rop),(int)action->num);
*/
		xsize = vol->indx_record_size;
		/* Check whether data is to be discarded */
		warn = (rop != UpdateNonResidentValue)
			|| !check_full_index(action,TRUE);
		buffer = read_protected(vol, &action->record,
						xsize, warn);
		indx = (INDEX_BLOCK*)buffer;
		if (indx && (indx->magic == magic_INDX)) {
			data_lsn = sle64_to_cpu(indx->lsn);
			/*
			 * Beware of records not updated
			 * during the last session which may
			 * have a stale lsn (consequence
			 * of ntfs-3g resetting the log)
			 */
			executed = ((s64)(data_lsn - this_lsn) >= 0)
			    && (((s64)(data_lsn - latest_lsn)) <= 0)
			    && ! exception(action->num);
		} else {
			if (!warn) {
				/* Old record not needed */
				if (!buffer)
					buffer = (char*)calloc(1, xsize);
				if (buffer)
					executed = FALSE;
				else
					err = 1;
			} else {
				printf("** %s (action %d) not"
					" acting on INDX\n",
					actionname(rop),
					(int)action->num);
				err = 1;
			}
		}
		break;
	case ON_RAW :
		if (action->record.attribute_flags
				& (ACTS_ON_INDX | ACTS_ON_MFT)) {
			printf("** Error : action %s on MFT"
				" or INDX\n",
				actionname(rop));
			err = 1;
		} else {
			buffer = read_raw(vol, &action->record);
			if (!buffer)
				err = 1;
		}
		break;
	default :
		buffer = (char*)NULL;
		break;
	}
	if (!err && (!executed || !opts)) {
		err = distribute_redos(vol, action, buffer);
		redocount++;
	} else {
		if (optv)
			printf("Action %d %s (%s) not redone\n",
				action->num,
				actionname(rop),
				actionname(uop));
	}
	if (buffer)
		free(buffer);
	return (err);
}


/*
 *		Play the redo actions from earliest to latest
 *
 *	Currently we can only redo the last undone transaction,
 *	otherwise the attribute table would be out of phase.
 */

int play_redos(ntfs_volume *vol, const struct ACTION_RECORD *firstaction)
{
	const struct ACTION_RECORD *action;
	int err;

	err = 0;
	action = firstaction;
	while (action && !err) {
			/* Only committed actions should be redone */
		if ((!optc || within_lcn_range(&action->record))
		    && (action->flags & ACTION_TO_REDO))
			err = play_one_redo(vol, action);
		if (!err)
			action = action->next;
	}
	return (err);
}

static int distribute_undos(ntfs_volume *vol, const struct ACTION_RECORD *action,
			char *buffer)
{
	int rop, uop;
	int err;

	err = 0;
	rop = le16_to_cpu(action->record.redo_operation);
	uop = le16_to_cpu(action->record.undo_operation);
	switch (rop) {
	case AddIndexEntryAllocation :
		if (action->record.undo_operation
		    == const_cpu_to_le16(DeleteIndexEntryAllocation))
			err = undo_add_index(vol, action, buffer);
		break;
	case AddIndexEntryRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(DeleteIndexEntryRoot))
			err = undo_add_root_index(vol, action, buffer);
		break;
	case ClearBitsInNonResidentBitMap :
		if (action->record.undo_operation
		    == const_cpu_to_le16(SetBitsInNonResidentBitMap))
			err = undo_force_bits(vol, action, buffer);
		break;
	case CreateAttribute :
		if (action->record.undo_operation
		    == const_cpu_to_le16(DeleteAttribute))
			err = undo_create_attribute(vol, action, buffer);
		break;
	case DeallocateFileRecordSegment :
		if (action->record.undo_operation
		    == const_cpu_to_le16(InitializeFileRecordSegment))
			err = undo_delete_file(vol, action, buffer);
		break;
	case DeleteAttribute :
		if (action->record.undo_operation
		    == const_cpu_to_le16(CreateAttribute))
			err = undo_delete_attribute(vol, action, buffer);
		break;
	case DeleteIndexEntryAllocation :
		if (action->record.undo_operation
		    == const_cpu_to_le16(AddIndexEntryAllocation))
			err = undo_delete_index(vol, action, buffer);
		break;
	case DeleteIndexEntryRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(AddIndexEntryRoot))
			err = undo_delete_root_index(vol, action, buffer);
		break;
	case InitializeFileRecordSegment :
		if (action->record.undo_operation
		        == const_cpu_to_le16(Noop))
			err = undo_create_file(vol, action, buffer);
		break;
	case OpenNonResidentAttribute :
		if (action->record.undo_operation
		        == const_cpu_to_le16(Noop))
			err = undo_open_attribute(vol, action);
		break;
	case SetBitsInNonResidentBitMap :
		if (action->record.undo_operation
		    == const_cpu_to_le16(ClearBitsInNonResidentBitMap))
			err = undo_force_bits(vol, action, buffer);
		break;
	case SetIndexEntryVcnAllocation :
		if (action->record.undo_operation
		    == const_cpu_to_le16(SetIndexEntryVcnAllocation))
			err = undo_update_vcn(vol, action, buffer);
		break;
	case SetIndexEntryVcnRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(SetIndexEntryVcnRoot))
			err = undo_update_root_vcn(vol, action, buffer);
		break;
	case SetNewAttributeSizes :
		if (action->record.undo_operation
		    == const_cpu_to_le16(SetNewAttributeSizes))
			err = undo_sizes(vol, action, buffer);
		break;
	case UpdateFileNameAllocation :
		if (action->record.undo_operation
		    == const_cpu_to_le16(UpdateFileNameAllocation))
			err = undo_update_index(vol, action, buffer);
		break;
	case UpdateFileNameRoot :
		if (action->record.undo_operation
		        == const_cpu_to_le16(UpdateFileNameRoot))
			err = undo_update_root_index(vol, action, buffer);
		break;
	case UpdateMappingPairs :
		if (action->record.undo_operation
		    == const_cpu_to_le16(UpdateMappingPairs))
			err = undo_update_mapping(vol, action, buffer);
		break;
	case UpdateNonResidentValue :
		switch (get_action_kind(action)) {
		case ON_INDX :
			err = undo_update_index_value(vol, action, buffer);
			break;
		case ON_RAW :
			err = undo_update_value(vol, action, buffer);
			break;
		default :
			printf("** Bad attribute type\n");
			err = 1;
		}
		break;
	case UpdateResidentValue :
		if (action->record.undo_operation
		    == const_cpu_to_le16(UpdateResidentValue))
			err = undo_update_resident(vol, action, buffer);
		break;
	case Win10Action37 :
		if (action->record.undo_operation
		    == const_cpu_to_le16(Noop))
			err = undo_action37(vol, action, buffer);
		break;
	case WriteEndofFileRecordSegment :
		if (action->record.undo_operation
		    == const_cpu_to_le16(WriteEndofFileRecordSegment))
			err = undo_write_end(vol, action, buffer);
		break;
	case WriteEndOfIndexBuffer :
		if (action->record.undo_operation
		    == const_cpu_to_le16(WriteEndOfIndexBuffer))
			err = undo_write_index(vol, action, buffer);
		break;
	case AttributeNamesDump :
	case CompensationlogRecord :
	case DirtyPageTableDump :
	case ForgetTransaction :
	case Noop :
	case OpenAttributeTableDump :
		break;
	default :
		printf("** Unsupported undo %s\n", actionname(rop));
		err = 1;
		break;
	}
	if (err)
		printf("* Undoing action %d %s (%s) failed\n",
			action->num,actionname(rop), actionname(uop));
	return (err);
}

/*
 *		Undo a single action
 *
 *	The record the action acts on is read and, when it is an MFT or
 *	INDX one, the need for undoing is checked.
 */

static int play_one_undo(ntfs_volume *vol, const struct ACTION_RECORD *action)
{
	MFT_RECORD *entry;
	INDEX_BLOCK *indx;
	char *buffer;
	u32 xsize;
	u16 rop;
	u16 uop;
	int err;
	BOOL executed;
	enum ACTION_KIND kind;

	err = 0;
	rop = le16_to_cpu(action->record.redo_operation);
	uop = le16_to_cpu(action->record.undo_operation);
	if (optv)
		printf("Undo action %d %s (%s) lsn 0x%llx\n",
			action->num,
			actionname(rop), actionname(uop),
			(long long)sle64_to_cpu(
				action->record.this_lsn));
	buffer = (char*)NULL;
	executed = FALSE;
	kind = ON_NONE;
	switch (rop) {
			/* Actions always acting on MFT */
	case AddIndexEntryRoot :
	case CreateAttribute :
	case DeallocateFileRecordSegment :
	case DeleteAttribute :
	case DeleteIndexEntryRoot :
	case InitializeFileRecordSegment :
	case SetIndexEntryVcnRoot :
	case SetNewAttributeSizes :
	case UpdateFileNameRoot :
	case UpdateMappingPairs :
	case UpdateResidentValue :
	case Win10Action37 :
	case WriteEndofFileRecordSegment :
		kind = ON_MFT;
		break;
			/* Actions always acting on INDX */
	case AddIndexEntryAllocation :
	case DeleteIndexEntryAllocation :
	case SetIndexEntryVcnAllocation :
	case UpdateFileNameAllocation :
	case WriteEndOfIndexBuffer :
		kind = ON_INDX;
		break;
			/* Actions never acting on MFT or INDX */
	case ClearBitsInNonResidentBitMap :
	case SetBitsInNonResidentBitMap :
		kind = ON_RAW;
		break;
			/* Actions which may act on MFT */
	case Noop : /* on MFT if DeallocateFileRecordSegment */
		break;
			/* Actions which may act on INDX */
	case UpdateNonResidentValue :
		/* Known cases : INDX, $SDS, ATTR_LIST */
		kind = get_action_kind(action);
		if (kind == ON_NONE)
			err = 1;
		break;
	case OpenNonResidentAttribute :
		/* probably not important */
		kind = ON_NONE;
		break;
			/* Actions currently ignored */
	case AttributeNamesDump :
	case CommitTransaction :
	case CompensationlogRecord :
	case DeleteDirtyClusters :
	case DirtyPageTableDump :
	case EndTopLevelAction :
	case ForgetTransaction :
	case HotFix :
	case OpenAttributeTableDump :
	case PrepareTransaction :
	case TransactionTableDump :
	case UpdateRecordDataAllocation :
	case UpdateRecordDataRoot :
	case Win10Action35 :
	case Win10Action36 :
		kind = ON_NONE;
		break;
	}
	switch (kind) {
	case ON_MFT :
/*
 the check below cannot be used on WinXP
if (!(action->record.attribute_flags & ACTS_ON_MFT))
printf("** %s (action %d) not acting on MFT\n",actionname(rop),(int)action->num);
*/
		buffer = read_protected(vol, &action->record, mftrecsz, TRUE);
		entry = (MFT_RECORD*)buffer;
		if (entry) {
			if (entry->magic == magic_FILE) {
				executed = !older_record(entry,
					&action->record);
				if (!executed
				    && exception(action->num))
					executed = TRUE;
if (optv > 1)
printf("record lsn 0x%llx is %s than action %d lsn 0x%llx\n",
(long long)sle64_to_cpu(entry->lsn),
(executed ? "not older" : "older"),
(int)action->num,
(long long)sle64_to_cpu(action->record.this_lsn));
			} else {
				printf("** %s (action %d) not acting on MFT\n",
					actionname(rop), (int)action->num);
				err = 1;
			}
		} else {
			/*
			 * Could not read the MFT record :
			 * if this is undoing a record create (from scratch)
			 * which did not take place, there is nothing to redo,
			 * otherwise this is an error.
			 */
			if (check_full_mft(action,TRUE))
				executed = FALSE;
			else
				err = 1;
		}
		break;
	case ON_INDX :
/*
 the check below cannot be used on WinXP
if (!(action->record.attribute_flags & ACTS_ON_INDX))
printf("** %s (action %d) not acting on INDX\n",actionname(rop),(int)action->num);
*/
		xsize = vol->indx_record_size;
		buffer = read_protected(vol, &action->record, xsize, TRUE);
		indx = (INDEX_BLOCK*)buffer;
		if (indx) {
			if (indx->magic == magic_INDX) {
				executed = !older_record(indx,
					&action->record);
				if (!executed
				    && exception(action->num))
					executed = TRUE;
if (optv > 1)
printf("index lsn 0x%llx is %s than action %d lsn 0x%llx\n",
(long long)sle64_to_cpu(indx->lsn),
(executed ? "not older" : "older"),
(int)action->num,
(long long)sle64_to_cpu(action->record.this_lsn));
			} else {
				printf("** %s (action %d) not acting on INDX\n",
					actionname(rop), (int)action->num);
				err = 1;
			}
		} else {
			/*
			 * Could not read the INDX record :
			 * if this is undoing a record create (from scratch)
			 * which did not take place, there is nothing to redo,
			 * otherwise this must be an error.
			 * However, after deleting the last index allocation
			 * in a block, the block is apparently zeroed
			 * and cannot be read. In this case we have to
			 * create an initial index block and apply the undo.
			 */
			if (check_full_index(action,TRUE))
				executed = FALSE;
			else {
				err = 1;
				if (uop == AddIndexEntryAllocation) {
					executed = TRUE;
					buffer = (char*)calloc(1, xsize);
					if (buffer)
						err = create_indx(vol,
							action, buffer);
				}
			}
		}
		break;
	case ON_RAW :
		if (action->record.attribute_flags
				& (ACTS_ON_INDX | ACTS_ON_MFT)) {
			printf("** Error : action %s on MFT or INDX\n",
				actionname(rop));
			err = 1;
		} else {
			buffer = read_raw(vol, &action->record);
			if (!buffer)
				err = 1;
		}
		executed = TRUE;
		break;
	default :
		executed = TRUE;
		buffer = (char*)NULL;
		break;
	}
	if (!err && executed) {
		err = distribute_undos(vol, action, buffer);
		undocount++;
	}
	if (buffer)
		free(buffer);

	return (err);
}

/*
 *		Play the undo actions from latest to earliest
 *
 *	For structured record, a check is made on the lsn to only
 *	try to undo the actions which were executed. This implies
 *	identifying actions on a structured record.
 *
 *	Returns 0 if successful
 */

int play_undos(ntfs_volume *vol, const struct ACTION_RECORD *lastaction)
{
	const struct ACTION_RECORD *action;
	int err;

	err = 0;
	action = lastaction;
	while (action && !err) {
		if (!optc || within_lcn_range(&action->record))
			err = play_one_undo(vol, action);
		if (!err)
			action = action->prev;
	}
	return (err);
}
