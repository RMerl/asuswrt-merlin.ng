// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright 2000-2002 by Hans Reiser, licensing governed by reiserfs/README
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2000, 2001  Free Software Foundation, Inc.
 *
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 *
 */

/* An implementation for the ReiserFS filesystem ported from GRUB.
 * Some parts of this code (mainly the structures and defines) are
 * from the original reiser fs code, as found in the linux kernel.
 */

#include <common.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <linux/time.h>
#include <asm/byteorder.h>
#include <reiserfs.h>

#include "reiserfs_private.h"

#undef REISERDEBUG

/* Some parts of this code (mainly the structures and defines) are
 * from the original reiser fs code, as found in the linux kernel.
 */

static char fsys_buf[FSYS_BUFLEN];
static reiserfs_error_t errnum = ERR_NONE;
static int print_possibilities;
static unsigned int filepos, filemax;

static int
substring (const char *s1, const char *s2)
{
  while (*s1 == *s2)
    {
      /* The strings match exactly. */
      if (! *(s1++))
	return 0;
      s2 ++;
    }

  /* S1 is a substring of S2. */
  if (*s1 == 0)
    return -1;

  /* S1 isn't a substring. */
  return 1;
}

static void sd_print_item (struct item_head * ih, char * item)
{
    char filetime[30];
    time_t ttime;

    if (stat_data_v1 (ih)) {
	struct stat_data_v1 * sd = (struct stat_data_v1 *)item;
	ttime = sd_v1_mtime(sd);
	ctime_r(&ttime, filetime);
	printf ("%-10s %4hd %6d %6d %9d %24.24s",
		 bb_mode_string(sd_v1_mode(sd)), sd_v1_nlink(sd),sd_v1_uid(sd), sd_v1_gid(sd),
		 sd_v1_size(sd), filetime);
    } else {
	struct stat_data * sd = (struct stat_data *)item;
	ttime = sd_v2_mtime(sd);
	ctime_r(&ttime, filetime);
	printf ("%-10s %4d %6d %6d %9d %24.24s",
		 bb_mode_string(sd_v2_mode(sd)), sd_v2_nlink(sd),sd_v2_uid(sd),sd_v2_gid(sd),
		 (__u32) sd_v2_size(sd), filetime);
    }
}

static int
journal_read (int block, int len, char *buffer)
{
  return reiserfs_devread ((INFO->journal_block + block) << INFO->blocksize_shift,
			   0, len, buffer);
}

/* Read a block from ReiserFS file system, taking the journal into
 * account.  If the block nr is in the journal, the block from the
 * journal taken.
 */
static int
block_read (unsigned int blockNr, int start, int len, char *buffer)
{
  int transactions = INFO->journal_transactions;
  int desc_block = INFO->journal_first_desc;
  int journal_mask = INFO->journal_block_count - 1;
  int translatedNr = blockNr;
  __u32 *journal_table = JOURNAL_START;
  while (transactions-- > 0)
    {
      int i = 0;
      int j_len;
      if (__le32_to_cpu(*journal_table) != 0xffffffff)
	{
	  /* Search for the blockNr in cached journal */
	  j_len = __le32_to_cpu(*journal_table++);
	  while (i++ < j_len)
	    {
	      if (__le32_to_cpu(*journal_table++) == blockNr)
		{
		  journal_table += j_len - i;
		  goto found;
		}
	    }
	}
      else
	{
	  /* This is the end of cached journal marker.  The remaining
	   * transactions are still on disk.
	   */
	  struct reiserfs_journal_desc   desc;
	  struct reiserfs_journal_commit commit;

	  if (! journal_read (desc_block, sizeof (desc), (char *) &desc))
	    return 0;

	  j_len = __le32_to_cpu(desc.j_len);
	  while (i < j_len && i < JOURNAL_TRANS_HALF)
	    if (__le32_to_cpu(desc.j_realblock[i++]) == blockNr)
	      goto found;

	  if (j_len >= JOURNAL_TRANS_HALF)
	    {
	      int commit_block = (desc_block + 1 + j_len) & journal_mask;
	      if (! journal_read (commit_block,
				  sizeof (commit), (char *) &commit))
		return 0;
	      while (i < j_len)
		if (__le32_to_cpu(commit.j_realblock[i++ - JOURNAL_TRANS_HALF]) == blockNr)
		  goto found;
	    }
	}
      goto not_found;

    found:
      translatedNr = INFO->journal_block + ((desc_block + i) & journal_mask);
#ifdef REISERDEBUG
      printf ("block_read: block %d is mapped to journal block %d.\n",
	      blockNr, translatedNr - INFO->journal_block);
#endif
      /* We must continue the search, as this block may be overwritten
       * in later transactions.
       */
    not_found:
      desc_block = (desc_block + 2 + j_len) & journal_mask;
    }
  return reiserfs_devread (translatedNr << INFO->blocksize_shift, start, len, buffer);
}

/* Init the journal data structure.  We try to cache as much as
 * possible in the JOURNAL_START-JOURNAL_END space, but if it is full
 * we can still read the rest from the disk on demand.
 *
 * The first number of valid transactions and the descriptor block of the
 * first valid transaction are held in INFO.  The transactions are all
 * adjacent, but we must take care of the journal wrap around.
 */
static int
journal_init (void)
{
  unsigned int block_count = INFO->journal_block_count;
  unsigned int desc_block;
  unsigned int commit_block;
  unsigned int next_trans_id;
  struct reiserfs_journal_header header;
  struct reiserfs_journal_desc   desc;
  struct reiserfs_journal_commit commit;
  __u32 *journal_table = JOURNAL_START;

  journal_read (block_count, sizeof (header), (char *) &header);
  desc_block = __le32_to_cpu(header.j_first_unflushed_offset);
  if (desc_block >= block_count)
    return 0;

  INFO->journal_first_desc = desc_block;
  next_trans_id = __le32_to_cpu(header.j_last_flush_trans_id) + 1;

#ifdef REISERDEBUG
  printf ("journal_init: last flushed %d\n",
	  __le32_to_cpu(header.j_last_flush_trans_id));
#endif

  while (1)
    {
      journal_read (desc_block, sizeof (desc), (char *) &desc);
      if (substring (JOURNAL_DESC_MAGIC, desc.j_magic) > 0
	  || __le32_to_cpu(desc.j_trans_id) != next_trans_id
	  || __le32_to_cpu(desc.j_mount_id) != __le32_to_cpu(header.j_mount_id))
	/* no more valid transactions */
	break;

      commit_block = (desc_block + __le32_to_cpu(desc.j_len) + 1) & (block_count - 1);
      journal_read (commit_block, sizeof (commit), (char *) &commit);
      if (__le32_to_cpu(desc.j_trans_id) != commit.j_trans_id
	  || __le32_to_cpu(desc.j_len) != __le32_to_cpu(commit.j_len))
	/* no more valid transactions */
	break;

#ifdef REISERDEBUG
      printf ("Found valid transaction %d/%d at %d.\n",
	      __le32_to_cpu(desc.j_trans_id), __le32_to_cpu(desc.j_mount_id), desc_block);
#endif

      next_trans_id++;
      if (journal_table < JOURNAL_END)
	{
	  if ((journal_table + 1 + __le32_to_cpu(desc.j_len)) >= JOURNAL_END)
	    {
	      /* The table is almost full; mark the end of the cached
	       * journal.*/
	      *journal_table = __cpu_to_le32(0xffffffff);
	      journal_table = JOURNAL_END;
	    }
	  else
	    {
	      unsigned int i;
	      /* Cache the length and the realblock numbers in the table.
	       * The block number of descriptor can easily be computed.
	       * and need not to be stored here.
	       */

	      /* both are in the little endian format */
	      *journal_table++ = desc.j_len;
	      for (i = 0; i < __le32_to_cpu(desc.j_len) && i < JOURNAL_TRANS_HALF; i++)
		{
		  /* both are in the little endian format */
		  *journal_table++ = desc.j_realblock[i];
#ifdef REISERDEBUG
		  printf ("block %d is in journal %d.\n",
			  __le32_to_cpu(desc.j_realblock[i]), desc_block);
#endif
		}
	      for (     ; i < __le32_to_cpu(desc.j_len); i++)
		{
		  /* both are in the little endian format */
		  *journal_table++ = commit.j_realblock[i-JOURNAL_TRANS_HALF];
#ifdef REISERDEBUG
		  printf ("block %d is in journal %d.\n",
			  __le32_to_cpu(commit.j_realblock[i-JOURNAL_TRANS_HALF]),
			  desc_block);
#endif
		}
	    }
	}
      desc_block = (commit_block + 1) & (block_count - 1);
    }
#ifdef REISERDEBUG
  printf ("Transaction %d/%d at %d isn't valid.\n",
	  __le32_to_cpu(desc.j_trans_id), __le32_to_cpu(desc.j_mount_id), desc_block);
#endif

  INFO->journal_transactions
    = next_trans_id - __le32_to_cpu(header.j_last_flush_trans_id) - 1;
  return errnum == 0;
}

/* check filesystem types and read superblock into memory buffer */
int
reiserfs_mount (unsigned part_length)
{
  struct reiserfs_super_block super;
  int superblock = REISERFS_DISK_OFFSET_IN_BYTES >> SECTOR_BITS;
  char *cache;

  if (part_length < superblock + (sizeof (super) >> SECTOR_BITS)
      || ! reiserfs_devread (superblock, 0, sizeof (struct reiserfs_super_block),
			     (char *) &super)
      || (substring (REISER3FS_SUPER_MAGIC_STRING, super.s_magic) > 0
	  && substring (REISER2FS_SUPER_MAGIC_STRING, super.s_magic) > 0
	  && substring (REISERFS_SUPER_MAGIC_STRING, super.s_magic) > 0)
      || (/* check that this is not a copy inside the journal log */
	  sb_journal_block(&super) * sb_blocksize(&super)
	  <= REISERFS_DISK_OFFSET_IN_BYTES))
    {
      /* Try old super block position */
      superblock = REISERFS_OLD_DISK_OFFSET_IN_BYTES >> SECTOR_BITS;
      if (part_length < superblock + (sizeof (super) >> SECTOR_BITS)
	  || ! reiserfs_devread (superblock, 0, sizeof (struct reiserfs_super_block),
				 (char *) &super))
	return 0;

      if (substring (REISER2FS_SUPER_MAGIC_STRING, super.s_magic) > 0
	  && substring (REISERFS_SUPER_MAGIC_STRING, super.s_magic) > 0)
	{
	  /* pre journaling super block ? */
	  if (substring (REISERFS_SUPER_MAGIC_STRING,
			 (char*) ((int) &super + 20)) > 0)
	    return 0;

	  set_sb_blocksize(&super, REISERFS_OLD_BLOCKSIZE);
	  set_sb_journal_block(&super, 0);
	  set_sb_version(&super, 0);
	}
    }

  /* check the version number.  */
  if (sb_version(&super) > REISERFS_MAX_SUPPORTED_VERSION)
    return 0;

  INFO->version = sb_version(&super);
  INFO->blocksize = sb_blocksize(&super);
  INFO->fullblocksize_shift = log2 (sb_blocksize(&super));
  INFO->blocksize_shift = INFO->fullblocksize_shift - SECTOR_BITS;
  INFO->cached_slots =
    (FSYSREISER_CACHE_SIZE >> INFO->fullblocksize_shift) - 1;

#ifdef REISERDEBUG
  printf ("reiserfs_mount: version=%d, blocksize=%d\n",
	  INFO->version, INFO->blocksize);
#endif /* REISERDEBUG */

  /* Clear node cache. */
  memset (INFO->blocks, 0, sizeof (INFO->blocks));

  if (sb_blocksize(&super) < FSYSREISER_MIN_BLOCKSIZE
      || sb_blocksize(&super) > FSYSREISER_MAX_BLOCKSIZE
      || (SECTOR_SIZE << INFO->blocksize_shift) != sb_blocksize(&super))
    return 0;

  /* Initialize journal code.  If something fails we end with zero
   * journal_transactions, so we don't access the journal at all.
   */
  INFO->journal_transactions = 0;
  if (sb_journal_block(&super) != 0 && super.s_journal_dev == 0)
    {
      INFO->journal_block = sb_journal_block(&super);
      INFO->journal_block_count = sb_journal_size(&super);
      if (is_power_of_two (INFO->journal_block_count))
	journal_init ();

      /* Read in super block again, maybe it is in the journal */
      block_read (superblock >> INFO->blocksize_shift,
		  0, sizeof (struct reiserfs_super_block), (char *) &super);
    }

  if (! block_read (sb_root_block(&super), 0, INFO->blocksize, (char*) ROOT))
    return 0;

  cache = ROOT;
  INFO->tree_depth = __le16_to_cpu(BLOCKHEAD (cache)->blk_level);

#ifdef REISERDEBUG
  printf ("root read_in: block=%d, depth=%d\n",
	  sb_root_block(&super), INFO->tree_depth);
#endif /* REISERDEBUG */

  if (INFO->tree_depth >= MAX_HEIGHT)
    return 0;
  if (INFO->tree_depth == DISK_LEAF_NODE_LEVEL)
    {
      /* There is only one node in the whole filesystem,
       * which is simultanously leaf and root */
      memcpy (LEAF, ROOT, INFO->blocksize);
    }
  return 1;
}

/***************** TREE ACCESSING METHODS *****************************/

/* I assume you are familiar with the ReiserFS tree, if not go to
 * http://www.namesys.com/content_table.html
 *
 * My tree node cache is organized as following
 *   0   ROOT node
 *   1   LEAF node  (if the ROOT is also a LEAF it is copied here
 *   2-n other nodes on current path from bottom to top.
 *       if there is not enough space in the cache, the top most are
 *       omitted.
 *
 * I have only two methods to find a key in the tree:
 *   search_stat(dir_id, objectid) searches for the stat entry (always
 *       the first entry) of an object.
 *   next_key() gets the next key in tree order.
 *
 * This means, that I can only sequential reads of files are
 * efficient, but this really doesn't hurt for grub.
 */

/* Read in the node at the current path and depth into the node cache.
 * You must set INFO->blocks[depth] before.
 */
static char *
read_tree_node (unsigned int blockNr, int depth)
{
  char* cache = CACHE(depth);
  int num_cached = INFO->cached_slots;
  if (depth < num_cached)
    {
      /* This is the cached part of the path.  Check if same block is
       * needed.
       */
      if (blockNr == INFO->blocks[depth])
	return cache;
    }
  else
    cache = CACHE(num_cached);

#ifdef REISERDEBUG
  printf ("  next read_in: block=%d (depth=%d)\n",
	  blockNr, depth);
#endif /* REISERDEBUG */
  if (! block_read (blockNr, 0, INFO->blocksize, cache))
    return 0;
  /* Make sure it has the right node level */
  if (__le16_to_cpu(BLOCKHEAD (cache)->blk_level) != depth)
    {
      errnum = ERR_FSYS_CORRUPT;
      return 0;
    }

  INFO->blocks[depth] = blockNr;
  return cache;
}

/* Get the next key, i.e. the key following the last retrieved key in
 * tree order.  INFO->current_ih and
 * INFO->current_info are adapted accordingly.  */
static int
next_key (void)
{
  int depth;
  struct item_head *ih = INFO->current_ih + 1;
  char *cache;

#ifdef REISERDEBUG
  printf ("next_key:\n  old ih: key %d:%d:%d:%d version:%d\n",
	  __le32_to_cpu(INFO->current_ih->ih_key.k_dir_id),
	  __le32_to_cpu(INFO->current_ih->ih_key.k_objectid),
	  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_offset),
	  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_uniqueness),
	  __le16_to_cpu(INFO->current_ih->ih_version));
#endif /* REISERDEBUG */

  if (ih == &ITEMHEAD[__le16_to_cpu(BLOCKHEAD (LEAF)->blk_nr_item)])
    {
      depth = DISK_LEAF_NODE_LEVEL;
      /* The last item, was the last in the leaf node.
       * Read in the next block
       */
      do
	{
	  if (depth == INFO->tree_depth)
	    {
	      /* There are no more keys at all.
	       * Return a dummy item with MAX_KEY */
	      ih = (struct item_head *) &BLOCKHEAD (LEAF)->blk_right_delim_key;
	      goto found;
	    }
	  depth++;
#ifdef REISERDEBUG
	  printf ("  depth=%d, i=%d\n", depth, INFO->next_key_nr[depth]);
#endif /* REISERDEBUG */
	}
      while (INFO->next_key_nr[depth] == 0);

      if (depth == INFO->tree_depth)
	cache = ROOT;
      else if (depth <= INFO->cached_slots)
	cache = CACHE (depth);
      else
	{
	  cache = read_tree_node (INFO->blocks[depth], depth);
	  if (! cache)
	    return 0;
	}

      do
	{
	  int nr_item = __le16_to_cpu(BLOCKHEAD (cache)->blk_nr_item);
	  int key_nr = INFO->next_key_nr[depth]++;
#ifdef REISERDEBUG
	  printf ("  depth=%d, i=%d/%d\n", depth, key_nr, nr_item);
#endif /* REISERDEBUG */
	  if (key_nr == nr_item)
	    /* This is the last item in this block, set the next_key_nr to 0 */
	    INFO->next_key_nr[depth] = 0;

	  cache = read_tree_node (dc_block_number(&(DC (cache)[key_nr])), --depth);
	  if (! cache)
	    return 0;
	}
      while (depth > DISK_LEAF_NODE_LEVEL);

      ih = ITEMHEAD;
    }
 found:
  INFO->current_ih   = ih;
  INFO->current_item = &LEAF[__le16_to_cpu(ih->ih_item_location)];
#ifdef REISERDEBUG
  printf ("  new ih: key %d:%d:%d:%d version:%d\n",
	  __le32_to_cpu(INFO->current_ih->ih_key.k_dir_id),
	  __le32_to_cpu(INFO->current_ih->ih_key.k_objectid),
	  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_offset),
	  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_uniqueness),
	  __le16_to_cpu(INFO->current_ih->ih_version));
#endif /* REISERDEBUG */
  return 1;
}

/* preconditions: reiserfs_mount already executed, therefore
 *   INFO block is valid
 * returns: 0 if error (errnum is set),
 *   nonzero iff we were able to find the key successfully.
 * postconditions: on a nonzero return, the current_ih and
 *   current_item fields describe the key that equals the
 *   searched key.  INFO->next_key contains the next key after
 *   the searched key.
 * side effects: messes around with the cache.
 */
static int
search_stat (__u32 dir_id, __u32 objectid)
{
  char *cache;
  int depth;
  int nr_item;
  int i;
  struct item_head *ih;
#ifdef REISERDEBUG
  printf ("search_stat:\n  key %d:%d:0:0\n", dir_id, objectid);
#endif /* REISERDEBUG */

  depth = INFO->tree_depth;
  cache = ROOT;

  while (depth > DISK_LEAF_NODE_LEVEL)
    {
      struct key *key;
      nr_item = __le16_to_cpu(BLOCKHEAD (cache)->blk_nr_item);

      key = KEY (cache);

      for (i = 0; i < nr_item; i++)
	{
	  if (__le32_to_cpu(key->k_dir_id) > dir_id
	      || (__le32_to_cpu(key->k_dir_id) == dir_id
		  && (__le32_to_cpu(key->k_objectid) > objectid
		      || (__le32_to_cpu(key->k_objectid) == objectid
			  && (__le32_to_cpu(key->u.v1.k_offset)
			      | __le32_to_cpu(key->u.v1.k_uniqueness)) > 0))))
	    break;
	  key++;
	}

#ifdef REISERDEBUG
      printf ("  depth=%d, i=%d/%d\n", depth, i, nr_item);
#endif /* REISERDEBUG */
      INFO->next_key_nr[depth] = (i == nr_item) ? 0 : i+1;
      cache = read_tree_node (dc_block_number(&(DC (cache)[i])), --depth);
      if (! cache)
	return 0;
    }

  /* cache == LEAF */
  nr_item = __le16_to_cpu(BLOCKHEAD (LEAF)->blk_nr_item);
  ih = ITEMHEAD;
  for (i = 0; i < nr_item; i++)
    {
      if (__le32_to_cpu(ih->ih_key.k_dir_id) == dir_id
	  && __le32_to_cpu(ih->ih_key.k_objectid) == objectid
	  && __le32_to_cpu(ih->ih_key.u.v1.k_offset) == 0
	  && __le32_to_cpu(ih->ih_key.u.v1.k_uniqueness) == 0)
	{
#ifdef REISERDEBUG
	  printf ("  depth=%d, i=%d/%d\n", depth, i, nr_item);
#endif /* REISERDEBUG */
	  INFO->current_ih   = ih;
	  INFO->current_item = &LEAF[__le16_to_cpu(ih->ih_item_location)];
	  return 1;
	}
      ih++;
    }
  errnum = ERR_FSYS_CORRUPT;
  return 0;
}

int
reiserfs_read (char *buf, unsigned len)
{
  unsigned int blocksize;
  unsigned int offset;
  unsigned int to_read;
  char *prev_buf = buf;

#ifdef REISERDEBUG
  printf ("reiserfs_read: filepos=%d len=%d, offset=%Lx\n",
	  filepos, len, (__u64) IH_KEY_OFFSET (INFO->current_ih) - 1);
#endif /* REISERDEBUG */

  if (__le32_to_cpu(INFO->current_ih->ih_key.k_objectid) != INFO->fileinfo.k_objectid
      || IH_KEY_OFFSET (INFO->current_ih) > filepos + 1)
    {
      search_stat (INFO->fileinfo.k_dir_id, INFO->fileinfo.k_objectid);
      goto get_next_key;
    }

  while (! errnum)
    {
      if (__le32_to_cpu(INFO->current_ih->ih_key.k_objectid) != INFO->fileinfo.k_objectid) {
	break;
      }

      offset = filepos - IH_KEY_OFFSET (INFO->current_ih) + 1;
      blocksize = __le16_to_cpu(INFO->current_ih->ih_item_len);

#ifdef REISERDEBUG
      printf ("  loop: filepos=%d len=%d, offset=%d blocksize=%d\n",
	      filepos, len, offset, blocksize);
#endif /* REISERDEBUG */

      if (IH_KEY_ISTYPE(INFO->current_ih, TYPE_DIRECT)
	  && offset < blocksize)
	{
#ifdef REISERDEBUG
	  printf ("direct_read: offset=%d, blocksize=%d\n",
		  offset, blocksize);
#endif /* REISERDEBUG */
	  to_read = blocksize - offset;
	  if (to_read > len)
	    to_read = len;

	  memcpy (buf, INFO->current_item + offset, to_read);
	  goto update_buf_len;
	}
      else if (IH_KEY_ISTYPE(INFO->current_ih, TYPE_INDIRECT))
	{
	  blocksize = (blocksize >> 2) << INFO->fullblocksize_shift;
#ifdef REISERDEBUG
	  printf ("indirect_read: offset=%d, blocksize=%d\n",
		  offset, blocksize);
#endif /* REISERDEBUG */

	  while (offset < blocksize)
	    {
	      __u32 blocknr = __le32_to_cpu(((__u32 *) INFO->current_item)
		[offset >> INFO->fullblocksize_shift]);
	      int blk_offset = offset & (INFO->blocksize-1);
	      to_read = INFO->blocksize - blk_offset;
	      if (to_read > len)
		to_read = len;

	      /* Journal is only for meta data.  Data blocks can be read
	       * directly without using block_read
	       */
	      reiserfs_devread (blocknr << INFO->blocksize_shift,
				blk_offset, to_read, buf);
	    update_buf_len:
	      len -= to_read;
	      buf += to_read;
	      offset += to_read;
	      filepos += to_read;
	      if (len == 0)
		goto done;
	    }
	}
    get_next_key:
      next_key ();
    }
 done:
  return errnum ? 0 : buf - prev_buf;
}


/* preconditions: reiserfs_mount already executed, therefore
 *   INFO block is valid
 * returns: 0 if error, nonzero iff we were able to find the file successfully
 * postconditions: on a nonzero return, INFO->fileinfo contains the info
 *   of the file we were trying to look up, filepos is 0 and filemax is
 *   the size of the file.
 */
static int
reiserfs_dir (char *dirname)
{
  struct reiserfs_de_head *de_head;
  char *rest, ch;
  __u32 dir_id, objectid, parent_dir_id = 0, parent_objectid = 0;
#ifndef STAGE1_5
  int do_possibilities = 0;
#endif /* ! STAGE1_5 */
  char linkbuf[PATH_MAX];	/* buffer for following symbolic links */
  int link_count = 0;
  int mode;

  dir_id = REISERFS_ROOT_PARENT_OBJECTID;
  objectid = REISERFS_ROOT_OBJECTID;

  while (1)
    {
#ifdef REISERDEBUG
      printf ("dirname=%s\n", dirname);
#endif /* REISERDEBUG */

      /* Search for the stat info first. */
      if (! search_stat (dir_id, objectid))
	return 0;

#ifdef REISERDEBUG
       printf ("sd_mode=%x sd_size=%d\n",
	       stat_data_v1(INFO->current_ih) ? sd_v1_mode((struct stat_data_v1 *) INFO->current_item) :
						sd_v2_mode((struct stat_data *) (INFO->current_item)),
	       stat_data_v1(INFO->current_ih) ? sd_v1_size((struct stat_data_v1 *) INFO->current_item) :
						sd_v2_size((struct stat_data *) INFO->current_item)
	      );

#endif /* REISERDEBUG */
      mode = stat_data_v1(INFO->current_ih) ?
	       sd_v1_mode((struct stat_data_v1 *) INFO->current_item) :
	       sd_v2_mode((struct stat_data *) INFO->current_item);

      /* If we've got a symbolic link, then chase it. */
      if (S_ISLNK (mode))
	{
	  unsigned int len;
	  if (++link_count > MAX_LINK_COUNT)
	    {
	      errnum = ERR_SYMLINK_LOOP;
	      return 0;
	    }

	  /* Get the symlink size. */
	  filemax = stat_data_v1(INFO->current_ih) ?
		     sd_v1_size((struct stat_data_v1 *) INFO->current_item) :
		     sd_v2_size((struct stat_data *) INFO->current_item);

	  /* Find out how long our remaining name is. */
	  len = 0;
	  while (dirname[len] && !isspace (dirname[len]))
	    len++;

	  if (filemax + len > sizeof (linkbuf) - 1)
	    {
	      errnum = ERR_FILELENGTH;
	      return 0;
	    }

	  /* Copy the remaining name to the end of the symlink data.
	     Note that DIRNAME and LINKBUF may overlap! */
	  memmove (linkbuf + filemax, dirname, len+1);

	  INFO->fileinfo.k_dir_id = dir_id;
	  INFO->fileinfo.k_objectid = objectid;
	  filepos = 0;
	  if (! next_key ()
	      || reiserfs_read (linkbuf, filemax) != filemax)
	    {
	      if (! errnum)
		errnum = ERR_FSYS_CORRUPT;
	      return 0;
	    }

#ifdef REISERDEBUG
	  printf ("symlink=%s\n", linkbuf);
#endif /* REISERDEBUG */

	  dirname = linkbuf;
	  if (*dirname == '/')
	    {
	      /* It's an absolute link, so look it up in root. */
	      dir_id = REISERFS_ROOT_PARENT_OBJECTID;
	      objectid = REISERFS_ROOT_OBJECTID;
	    }
	  else
	    {
	      /* Relative, so look it up in our parent directory. */
	      dir_id   = parent_dir_id;
	      objectid = parent_objectid;
	    }

	  /* Now lookup the new name. */
	  continue;
	}

      /* if we have a real file (and we're not just printing possibilities),
	 then this is where we want to exit */

      if (! *dirname || isspace (*dirname))
	{
	  if (! S_ISREG (mode))
	    {
	      errnum = ERR_BAD_FILETYPE;
	      return 0;
	    }

	  filepos = 0;
	  filemax = stat_data_v1(INFO->current_ih) ?
		      sd_v1_size((struct stat_data_v1 *) INFO->current_item) :
		      sd_v2_size((struct stat_data *) INFO->current_item);
#if 0
	  /* If this is a new stat data and size is > 4GB set filemax to
	   * maximum
	   */
	  if (__le16_to_cpu(INFO->current_ih->ih_version) == ITEM_VERSION_2
	      && sd_size_hi((struct stat_data *) INFO->current_item) > 0)
	    filemax = 0xffffffff;
#endif
	  INFO->fileinfo.k_dir_id = dir_id;
	  INFO->fileinfo.k_objectid = objectid;
	  return next_key ();
	}

      /* continue with the file/directory name interpretation */
      while (*dirname == '/')
	dirname++;
      if (! S_ISDIR (mode))
	{
	  errnum = ERR_BAD_FILETYPE;
	  return 0;
	}
      for (rest = dirname; (ch = *rest) && ! isspace (ch) && ch != '/'; rest++);
      *rest = 0;

# ifndef STAGE1_5
      if (print_possibilities && ch != '/')
	do_possibilities = 1;
# endif /* ! STAGE1_5 */

      while (1)
	{
	  char *name_end;
	  int num_entries;

	  if (! next_key ())
	    return 0;
#ifdef REISERDEBUG
	  printf ("ih: key %d:%d:%d:%d version:%d\n",
		  __le32_to_cpu(INFO->current_ih->ih_key.k_dir_id),
		  __le32_to_cpu(INFO->current_ih->ih_key.k_objectid),
		  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_offset),
		  __le32_to_cpu(INFO->current_ih->ih_key.u.v1.k_uniqueness),
		  __le16_to_cpu(INFO->current_ih->ih_version));
#endif /* REISERDEBUG */

	  if (__le32_to_cpu(INFO->current_ih->ih_key.k_objectid) != objectid)
	    break;

	  name_end = INFO->current_item + __le16_to_cpu(INFO->current_ih->ih_item_len);
	  de_head = (struct reiserfs_de_head *) INFO->current_item;
	  num_entries = __le16_to_cpu(INFO->current_ih->u.ih_entry_count);
	  while (num_entries > 0)
	    {
	      char *filename = INFO->current_item + deh_location(de_head);
	      char  tmp = *name_end;
	      if ((deh_state(de_head) & DEH_Visible))
		{
		  int cmp;
		  /* Directory names in ReiserFS are not null
		   * terminated.  We write a temporary 0 behind it.
		   * NOTE: that this may overwrite the first block in
		   * the tree cache.  That doesn't hurt as long as we
		   * don't call next_key () in between.
		   */
		  *name_end = 0;
		  cmp = substring (dirname, filename);
		  *name_end = tmp;
# ifndef STAGE1_5
		  if (do_possibilities)
		    {
		      if (cmp <= 0)
			{
			  char fn[PATH_MAX];
			  struct fsys_reiser_info info_save;

			  if (print_possibilities > 0)
			    print_possibilities = -print_possibilities;
			  *name_end = 0;
			  strcpy(fn, filename);
			  *name_end = tmp;

			  /* If NAME is "." or "..", do not count it.  */
			  if (strcmp (fn, ".") != 0 && strcmp (fn, "..") != 0) {
			    memcpy(&info_save, INFO, sizeof(struct fsys_reiser_info));
			    search_stat (deh_dir_id(de_head), deh_objectid(de_head));
			    sd_print_item(INFO->current_ih, INFO->current_item);
			    printf(" %s\n", fn);
			    search_stat (dir_id, objectid);
			    memcpy(INFO, &info_save, sizeof(struct fsys_reiser_info));
			  }
			}
		    }
		  else
# endif /* ! STAGE1_5 */
		    if (cmp == 0)
		      goto found;
		}
	      /* The beginning of this name marks the end of the next name.
	       */
	      name_end = filename;
	      de_head++;
	      num_entries--;
	    }
	}

# ifndef STAGE1_5
      if (print_possibilities < 0)
	return 1;
# endif /* ! STAGE1_5 */

      errnum = ERR_FILE_NOT_FOUND;
      *rest = ch;
      return 0;

    found:
      *rest = ch;
      dirname = rest;

      parent_dir_id = dir_id;
      parent_objectid = objectid;
      dir_id = deh_dir_id(de_head);
      objectid = deh_objectid(de_head);
    }
}

/*
 * U-Boot interface functions
 */

/*
 * List given directory
 *
 * RETURN: 0 - OK, else grub_error_t errnum
 */
int
reiserfs_ls (char *dirname)
{
	char *dir_slash;
	int res;

	errnum = 0;
	dir_slash = malloc(strlen(dirname) + 1);
	if (dir_slash == NULL) {
		return ERR_NUMBER_OVERFLOW;
	}
	strcpy(dir_slash, dirname);
	/* add "/" to the directory name */
	strcat(dir_slash, "/");

	print_possibilities = 1;
	res = reiserfs_dir (dir_slash);
	free(dir_slash);
	if (!res || errnum) {
		return errnum;
	}

	return 0;
}

/*
 * Open file for reading
 *
 * RETURN: >0 - OK, size of opened file
 *         <0 - ERROR  -grub_error_t errnum
 */
int
reiserfs_open (char *filename)
{
	/* open the file */
	errnum = 0;
	print_possibilities = 0;
	if (!reiserfs_dir (filename) || errnum) {
		return -errnum;
	}
	return filemax;
}
