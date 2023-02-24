/**
 * mst.c - Multi sector fixup handling code. Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2004 Anton Altaparmakov
 * Copyright (c) 2006-2009 Szabolcs Szakacsits
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "mst.h"
#include "logging.h"

/*
 * Basic validation of a NTFS multi-sector record.  The record size must be a
 * multiple of the logical sector size; and the update sequence array must be
 * properly aligned, of the expected length, and must end before the last le16
 * in the first logical sector.
 */
static BOOL
is_valid_record(u32 size, u16 usa_ofs, u16 usa_count)
{
	return size % NTFS_BLOCK_SIZE == 0 &&
		usa_ofs % 2 == 0 &&
		usa_count == 1 + (size / NTFS_BLOCK_SIZE) &&
		usa_ofs + ((u32)usa_count * 2) <= NTFS_BLOCK_SIZE - 2;
}

/**
 * ntfs_mst_post_read_fixup - deprotect multi sector transfer protected data
 * @b:		pointer to the data to deprotect
 * @size:	size in bytes of @b
 *
 * Perform the necessary post read multi sector transfer fixups and detect the
 * presence of incomplete multi sector transfers. - In that case, overwrite the
 * magic of the ntfs record header being processed with "BAAD" (in memory only!)
 * and abort processing.
 *
 * Return 0 on success and -1 on error, with errno set to the error code. The
 * following error codes are defined:
 *	EINVAL	Invalid arguments or invalid NTFS record in buffer @b.
 *	EIO	Multi sector transfer error was detected. Magic of the NTFS
 *		record in @b will have been set to "BAAD".
 */
int ntfs_mst_post_read_fixup_warn(NTFS_RECORD *b, const u32 size,
					BOOL warn)
{
	u16 usa_ofs, usa_count, usn;
	u16 *usa_pos, *data_pos;

	ntfs_log_trace("Entering\n");

	/* Setup the variables. */
	usa_ofs = le16_to_cpu(b->usa_ofs);
	usa_count = le16_to_cpu(b->usa_count);

	if (!is_valid_record(size, usa_ofs, usa_count)) {
		errno = EINVAL;
		if (warn) {
			ntfs_log_perror("%s: magic: 0x%08lx  size: %ld "
					"  usa_ofs: %d  usa_count: %u",
					 __FUNCTION__,
					(long)le32_to_cpu(*(le32 *)b),
					(long)size, (int)usa_ofs,
					(unsigned int)usa_count);
		}
		return -1;
	}
	/* Position of usn in update sequence array. */
	usa_pos = (u16*)b + usa_ofs/sizeof(u16);
	/*
	 * The update sequence number which has to be equal to each of the
	 * u16 values before they are fixed up. Note no need to care for
	 * endianness since we are comparing and moving data for on disk
	 * structures which means the data is consistent. - If it is
	 * consistency the wrong endianness it doesn't make any difference.
	 */
	usn = *usa_pos;
	/*
	 * Position in protected data of first u16 that needs fixing up.
	 */
	data_pos = (u16*)b + NTFS_BLOCK_SIZE/sizeof(u16) - 1;
	/*
	 * Check for incomplete multi sector transfer(s).
	 */
	while (--usa_count) {
		if (*data_pos != usn) {
			/*
			 * Incomplete multi sector transfer detected! )-:
			 * Set the magic to "BAAD" and return failure.
			 * Note that magic_BAAD is already converted to le32.
			 */
			errno = EIO;
			ntfs_log_perror("Incomplete multi-sector transfer: "
				"magic: 0x%08x  size: %d  usa_ofs: %d  usa_count:"
				" %d  data: %d  usn: %d", le32_to_cpu(*(le32 *)b), size,
				usa_ofs, usa_count, *data_pos, usn);
			b->magic = magic_BAAD;
			return -1;
		}
		data_pos += NTFS_BLOCK_SIZE/sizeof(u16);
	}
	/* Re-setup the variables. */
	usa_count = le16_to_cpu(b->usa_count);
	data_pos = (u16*)b + NTFS_BLOCK_SIZE/sizeof(u16) - 1;
	/* Fixup all sectors. */
	while (--usa_count) {
		/*
		 * Increment position in usa and restore original data from
		 * the usa into the data buffer.
		 */
		*data_pos = *(++usa_pos);
		/* Increment position in data as well. */
		data_pos += NTFS_BLOCK_SIZE/sizeof(u16);
	}
	return 0;
}

/*
 *		Deprotect multi sector transfer protected data
 *	with a warning if an error is found.
 */

int ntfs_mst_post_read_fixup(NTFS_RECORD *b, const u32 size)
{
	return (ntfs_mst_post_read_fixup_warn(b,size,TRUE));
}

/**
 * ntfs_mst_pre_write_fixup - apply multi sector transfer protection
 * @b:		pointer to the data to protect
 * @size:	size in bytes of @b
 *
 * Perform the necessary pre write multi sector transfer fixup on the data
 * pointer to by @b of @size.
 *
 * Return 0 if fixups applied successfully or -1 if no fixups were performed
 * due to errors. In that case errno i set to the error code (EINVAL).
 *
 * NOTE: We consider the absence / invalidity of an update sequence array to
 * mean error. This means that you have to create a valid update sequence
 * array header in the ntfs record before calling this function, otherwise it
 * will fail (the header needs to contain the position of the update sequence
 * array together with the number of elements in the array). You also need to
 * initialise the update sequence number before calling this function
 * otherwise a random word will be used (whatever was in the record at that
 * position at that time).
 */
int ntfs_mst_pre_write_fixup(NTFS_RECORD *b, const u32 size)
{
	u16 usa_ofs, usa_count, usn;
	le16 le_usn;
	le16 *usa_pos, *data_pos;

	ntfs_log_trace("Entering\n");

	/* Sanity check + only fixup if it makes sense. */
	if (!b || ntfs_is_baad_record(b->magic) ||
			ntfs_is_hole_record(b->magic)) {
		errno = EINVAL;
		ntfs_log_perror("%s: bad argument", __FUNCTION__);
		return -1;
	}
	/* Setup the variables. */
	usa_ofs = le16_to_cpu(b->usa_ofs);
	usa_count = le16_to_cpu(b->usa_count);

	if (!is_valid_record(size, usa_ofs, usa_count)) {
		errno = EINVAL;
		ntfs_log_perror("%s", __FUNCTION__);
		return -1;
	}
	/* Position of usn in update sequence array. */
	usa_pos = (le16*)((u8*)b + usa_ofs);
	/*
	 * Cyclically increment the update sequence number
	 * (skipping 0 and -1, i.e. 0xffff).
	 */
	usn = le16_to_cpup(usa_pos) + 1;
	if (usn == 0xffff || !usn)
		usn = 1;
	le_usn = cpu_to_le16(usn);
	*usa_pos = le_usn;
	/* Position in data of first le16 that needs fixing up. */
	data_pos = (le16*)b + NTFS_BLOCK_SIZE/sizeof(le16) - 1;
	/* Fixup all sectors. */
	while (--usa_count) {
		/*
		 * Increment the position in the usa and save the
		 * original data from the data buffer into the usa.
		 */
		*(++usa_pos) = *data_pos;
		/* Apply fixup to data. */
		*data_pos = le_usn;
		/* Increment position in data as well. */
		data_pos += NTFS_BLOCK_SIZE/sizeof(le16);
	}
	return 0;
}

/**
 * ntfs_mst_post_write_fixup - deprotect multi sector transfer protected data
 * @b:		pointer to the data to deprotect
 *
 * Perform the necessary post write multi sector transfer fixup, not checking
 * for any errors, because we assume we have just used
 * ntfs_mst_pre_write_fixup(), thus the data will be fine or we would never
 * have gotten here.
 */
void ntfs_mst_post_write_fixup(NTFS_RECORD *b)
{
	u16 *usa_pos, *data_pos;

	u16 usa_ofs = le16_to_cpu(b->usa_ofs);
	u16 usa_count = le16_to_cpu(b->usa_count);

	ntfs_log_trace("Entering\n");

	/* Position of usn in update sequence array. */
	usa_pos = (u16*)b + usa_ofs/sizeof(u16);

	/* Position in protected data of first u16 that needs fixing up. */
	data_pos = (u16*)b + NTFS_BLOCK_SIZE/sizeof(u16) - 1;

	/* Fixup all sectors. */
	while (--usa_count) {
		/*
		 * Increment position in usa and restore original data from
		 * the usa into the data buffer.
		 */
		*data_pos = *(++usa_pos);

		/* Increment position in data as well. */
		data_pos += NTFS_BLOCK_SIZE/sizeof(u16);
	}
}

