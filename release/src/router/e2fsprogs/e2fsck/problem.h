/*
 * problem.h --- e2fsck problem error codes
 *
 * Copyright 1996 by Theodore Ts'o
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

typedef __u32 problem_t;

struct problem_context {
	errcode_t	errcode;
	ext2_ino_t ino, ino2, dir;
	struct ext2_inode *inode;
	struct ext2_dir_entry *dirent;
	blk64_t	blk, blk2;
	e2_blkcnt_t	blkcount;
	dgrp_t		group;
	__u32		csum1, csum2;
	__u64		num, num2;
	const char *str;
};

/*
 * We define a set of "latch groups"; these are problems which are
 * handled as a set.  The user answers once for a particular latch
 * group.
 */
#define PR_LATCH_MASK	0x0ff0  /* Latch mask */
#define PR_LATCH_BLOCK	0x0010	/* Latch for illegal blocks (pass 1) */
#define PR_LATCH_BBLOCK	0x0020	/* Latch for bad block inode blocks (pass 1) */
#define PR_LATCH_IBITMAP 0x0030 /* Latch for pass 5 inode bitmap proc. */
#define PR_LATCH_BBITMAP 0x0040 /* Latch for pass 5 inode bitmap proc. */
#define PR_LATCH_RELOC	0x0050  /* Latch for superblock relocate hint */
#define PR_LATCH_DBLOCK	0x0060	/* Latch for pass 1b dup block headers */
#define PR_LATCH_LOW_DTIME 0x0070 /* Latch for pass1 orphaned list refugees */
#define PR_LATCH_TOOBIG	0x0080	/* Latch for file to big errors */
#define PR_LATCH_OPTIMIZE_DIR 0x0090 /* Latch for optimize directories */
#define PR_LATCH_BG_CHECKSUM 0x00A0  /* Latch for block group checksums */
#define PR_LATCH_OPTIMIZE_EXT 0x00B0  /* Latch for rebuild extents */

#define PR_LATCH(x)	((((x) & PR_LATCH_MASK) >> 4) - 1)

/*
 * Latch group descriptor flags
 */
#define PRL_YES		0x0001	/* Answer yes */
#define PRL_NO		0x0002	/* Answer no */
#define PRL_LATCHED	0x0004	/* The latch group is latched */
#define PRL_SUPPRESS	0x0008	/* Suppress all latch group questions */

#define PRL_VARIABLE	0x000f	/* All the flags that need to be reset */

/*
 * Pre-Pass 1 errors
 */

/* Block bitmap for group gggg is not in group */
#define PR_0_BB_NOT_GROUP			0x000001

/* Inode bitmap for group gggg is not in group */
#define PR_0_IB_NOT_GROUP			0x000002

/* Inode table for group gggg is not in group.  (block nnnn) */
#define PR_0_ITABLE_NOT_GROUP			0x000003

/* Superblock corrupt */
#define PR_0_SB_CORRUPT				0x000004

/* Filesystem size is wrong */
#define PR_0_FS_SIZE_WRONG			0x000005

/* Fragments not supported */
#define PR_0_NO_FRAGMENTS			0x000006

/* Superblock blocks_per_group = bbbb, should have been cccc */
#define PR_0_BLOCKS_PER_GROUP			0x000007

/* Superblock first_data_block = bbbb, should have been cccc */
#define PR_0_FIRST_DATA_BLOCK			0x000008

/* Filesystem did not have a UUID; generating one */
#define PR_0_ADD_UUID				0x000009

/* Relocate hint */
#define PR_0_RELOCATE_HINT			0x00000A

/* Miscellaneous superblock corruption */
#define PR_0_MISC_CORRUPT_SUPER			0x00000B

/* Error determining physical device size of filesystem */
#define PR_0_GETSIZE_ERROR			0x00000C

/* Inode count in the superblock incorrect */
#define PR_0_INODE_COUNT_WRONG			0x00000D

/* The Hurd does not support the filetype feature */
#define PR_0_HURD_CLEAR_FILETYPE		0x00000E

/* Superblock has an invalid journal (inode inum) */
#define PR_0_JOURNAL_BAD_INODE			0x00000F

/* External journal has multiple filesystem users (unsupported) */
#define PR_0_JOURNAL_UNSUPP_MULTIFS		0x000010

/* Can't find external journal */
#define PR_0_CANT_FIND_JOURNAL			0x000011

/* External journal has bad superblock */
#define PR_0_EXT_JOURNAL_BAD_SUPER		0x000012

/* Superblock has a bad journal UUID */
#define PR_0_JOURNAL_BAD_UUID			0x000013

/* Filesystem journal superblock is an unknown type */
#define PR_0_JOURNAL_UNSUPP_SUPER		0x000014

/* Journal superblock is corrupt */
#define PR_0_JOURNAL_BAD_SUPER			0x000015

/* Superblock has_journal flag is clear but has a journal */
#define PR_0_JOURNAL_HAS_JOURNAL		0x000016

/* Superblock needs_recovery flag is set but no journal is present */
#define PR_0_JOURNAL_RECOVER_SET		0x000017

/* Journal has data, but recovery flag is clear */
#define PR_0_JOURNAL_RECOVERY_CLEAR		0x000018

/* Ask if we should clear the journal */
#define PR_0_JOURNAL_RESET_JOURNAL		0x000019

/* Filesystem revision is 0, but feature flags are set */
#define PR_0_FS_REV_LEVEL			0x00001A

/* Clearing orphan inode */
#define PR_0_ORPHAN_CLEAR_INODE			0x000020

/* Illegal block found in orphaned inode */
#define PR_0_ORPHAN_ILLEGAL_BLOCK_NUM		0x000021

/* Already cleared block found in orphaned inode */
#define PR_0_ORPHAN_ALREADY_CLEARED_BLOCK	0x000022

/* Illegal orphan inode in superblock */
#define PR_0_ORPHAN_ILLEGAL_HEAD_INODE		0x000023

/* Illegal inode in orphaned inode list */
#define PR_0_ORPHAN_ILLEGAL_INODE		0x000024

/* Journal has unsupported read-only feature - abort */
#define PR_0_JOURNAL_UNSUPP_ROCOMPAT		0x000025

/* Journal has unsupported incompatible feature - abort */
#define PR_0_JOURNAL_UNSUPP_INCOMPAT		0x000026

/* Journal version not supported by this e2fsck */
#define PR_0_JOURNAL_UNSUPP_VERSION		0x000027

/* Moving journal from /file to hidden inode */
#define	PR_0_MOVE_JOURNAL			0x000028

/* Error moving journal to hidden file */
#define	PR_0_ERR_MOVE_JOURNAL			0x000029

/* Found invalid V2 journal superblock fields */
#define PR_0_CLEAR_V2_JOURNAL			0x00002A

/* Run journal anyway */
#define PR_0_JOURNAL_RUN			0x00002B

/* Run journal anyway by default */
#define PR_0_JOURNAL_RUN_DEFAULT		0x00002C

/* Backing up journal inode block information */
#define PR_0_BACKUP_JNL				0x00002D

/* Filesystem does not have resize_inode enabled, but
 * s_reserved_gdt_blocks is nnnn; should be zero */
#define PR_0_NONZERO_RESERVED_GDT_BLOCKS	0x00002E

/* Resize_inode not enabled, but the resize inode is non-zero */
#define PR_0_CLEAR_RESIZE_INODE			0x00002F

/* Resize inode not valid */
#define PR_0_RESIZE_INODE_INVALID		0x000030

/* Superblock last mount time is in the future */
#define PR_0_FUTURE_SB_LAST_MOUNT		0x000031

/* Superblock last write time is in the future */
#define PR_0_FUTURE_SB_LAST_WRITE		0x000032

/* Superblock hint for external superblock should be xxxx */
#define PR_0_EXTERNAL_JOURNAL_HINT		0x000033

/* Adding dirhash hint to filesystem */
#define PR_0_DIRHASH_HINT			0x000034

/* group descriptor N checksum is invalid, should be yyyy. */
#define PR_0_GDT_CSUM				0x000035

/* Group descriptor N marked uninitialized without feature set. */
#define PR_0_GDT_UNINIT				0x000036

/* Block bitmap is not initialised and Inode bitmap is -- NO LONGER USED */
/* #define PR_0_BB_UNINIT_IB_INIT			0x000037 */

/* Group descriptor N has invalid unused inodes count. */
#define PR_0_GDT_ITABLE_UNUSED			0x000038

/* Last group block bitmap is uninitialized. */
#define PR_0_BB_UNINIT_LAST			0x000039

/* Journal transaction was corrupt, replay was aborted */
#define PR_0_JNL_TXN_CORRUPT			0x00003A

/* The test_fs filesystem flag is set and ext4 is available */
#define PR_0_CLEAR_TESTFS_FLAG			0x00003B

/* Last mount time is in the future (fudged) */
#define PR_0_FUTURE_SB_LAST_MOUNT_FUDGED	0x00003C

/* Last write time is in the future (fudged) */
#define PR_0_FUTURE_SB_LAST_WRITE_FUDGED	0x00003D

/* One or more block group descriptor checksums are invalid (latch) */
#define PR_0_GDT_CSUM_LATCH			0x00003E

/* Setting free inodes count to right (was wrong) */
#define PR_0_FREE_INODE_COUNT			0x00003F

/* Setting free blocks count to right (was wrong) */
#define PR_0_FREE_BLOCK_COUNT			0x000040

/* Making quota inode hidden */
#define	PR_0_HIDE_QUOTA				0x000041

/* Superblock has invalid MMP block. */
#define PR_0_MMP_INVALID_BLK			0x000042

/* Superblock has invalid MMP magic. */
#define PR_0_MMP_INVALID_MAGIC			0x000043

/* Opening file system failed */
#define PR_0_OPEN_FAILED			0x000044

/* Checking group descriptor failed */
#define PR_0_CHECK_DESC_FAILED			0x000045

/* Superblock metadata_csum supersedes uninit_bg; both feature
 * bits cannot be set simultaneously. */
#define PR_0_META_AND_GDT_CSUM_SET		0x000046

/* Superblock MMP block checksum does not match MMP block. */
#define PR_0_MMP_CSUM_INVALID			0x000047

/* Superblock 64bit filesystem needs extents to access the whole disk */
#define PR_0_64BIT_WITHOUT_EXTENTS		0x000048

/* The first_meta_bg is too big */
#define PR_0_FIRST_META_BG_TOO_BIG		0x000049

/* External journal superblock checksum does not match superblock */
#define PR_0_EXT_JOURNAL_SUPER_CSUM_INVALID	0x00004A

/* metadata_csum_seed means nothing without metadata_csum */
#define PR_0_CSUM_SEED_WITHOUT_META_CSUM	0x00004B

/* Error initializing quota context */
#define PR_0_QUOTA_INIT_CTX			0x00004C

/* Bad required extra isize in superblock */
#define PR_0_BAD_MIN_EXTRA_ISIZE		0x00004D

/* Bad desired extra isize in superblock */
#define PR_0_BAD_WANT_EXTRA_ISIZE		0x00004E

/* Invalid quota inode number */
#define PR_0_INVALID_QUOTA_INO			0x00004F

/* Inode count in the superblock incorrect */
#define PR_0_INODE_COUNT_BIG			0x000050

/* Meta_bg and resize_inode are not compatible, remove resize_inode*/
#define PR_0_DISABLE_RESIZE_INODE		0x000051

/*
 * Pass 1 errors
 */

/* Pass 1: Checking inodes, blocks, and sizes */
#define PR_1_PASS_HEADER			0x010000

/* Root inode is not a directory */
#define PR_1_ROOT_NO_DIR			0x010001

/* Root inode has dtime set */
#define PR_1_ROOT_DTIME				0x010002

/* Reserved inode has bad mode */
#define PR_1_RESERVED_BAD_MODE			0x010003

/* Deleted inode inum has zero dtime */
#define PR_1_ZERO_DTIME				0x010004

/* Inode inum is in use, but has dtime set */
#define PR_1_SET_DTIME				0x010005

/* Inode inum is a zero-length directory */
#define PR_1_ZERO_LENGTH_DIR			0x010006

/* Group block bitmap at block conflicts with some other fs block */
#define PR_1_BB_CONFLICT			0x010007

/* Group inode bitmap at block conflicts with some other fs block */
#define PR_1_IB_CONFLICT			0x010008

/* Group inode table at block conflicts with some other fs block */
#define PR_1_ITABLE_CONFLICT			0x010009

/* Group block bitmap (block) is bad */
#define PR_1_BB_BAD_BLOCK			0x01000A

/* Group inode bitmap (block) is bad */
#define PR_1_IB_BAD_BLOCK			0x01000B

/* Inode i_size is small, should be larger */
#define PR_1_BAD_I_SIZE				0x01000C

/* Inode i_blocks is small, should be larger */
#define PR_1_BAD_I_BLOCKS			0x01000D

/* Illegal block number in inode */
#define PR_1_ILLEGAL_BLOCK_NUM			0x01000E

/* Block number overlaps filesystem metadata in inode */
#define PR_1_BLOCK_OVERLAPS_METADATA		0x01000F

/* Inode has illegal blocks (latch question) */
#define PR_1_INODE_BLOCK_LATCH			0x010010

/* Too many illegal blocks in inode */
#define	PR_1_TOO_MANY_BAD_BLOCKS		0x010011

/* Illegal block number in bad block inode */
#define PR_1_BB_ILLEGAL_BLOCK_NUM		0x010012

/* Bad block inode has illegal blocks (latch question) */
#define PR_1_INODE_BBLOCK_LATCH			0x010013

/* Duplicate or bad blocks in use! */
#define PR_1_DUP_BLOCKS_PREENSTOP		0x010014

/* Bad block number used as bad block inode indirect block */
#define PR_1_BBINODE_BAD_METABLOCK		0x010015

/* Inconsistency can't be fixed prompt */
#define PR_1_BBINODE_BAD_METABLOCK_PROMPT	0x010016

/* Bad primary block */
#define PR_1_BAD_PRIMARY_BLOCK			0x010017

/* Bad primary block prompt */
#define PR_1_BAD_PRIMARY_BLOCK_PROMPT		0x010018

/* The primary superblock block is on the bad block list */
#define PR_1_BAD_PRIMARY_SUPERBLOCK		0x010019

/* Bad primary block group descriptors */
#define PR_1_BAD_PRIMARY_GROUP_DESCRIPTOR	0x01001A

/* Warning: Group number's superblock (block) is bad */
#define PR_1_BAD_SUPERBLOCK			0x01001B

/* Warning: Group number's copy of the group descriptors has a bad block */
#define PR_1_BAD_GROUP_DESCRIPTORS		0x01001C

/* Block number claimed for no reason in process_bad_blocks */
#define PR_1_PROGERR_CLAIMED_BLOCK		0x01001D

/* Allocating number contiguous block(s) in block group number */
#define PR_1_RELOC_BLOCK_ALLOCATE		0x01001E

/* Allocating block buffer for relocating process */
#define PR_1_RELOC_MEMORY_ALLOCATE		0x01001F

/* Relocating group number's information from X to Y */
#define PR_1_RELOC_FROM_TO			0x010020

/* Relocating group number's information to X */
#define PR_1_RELOC_TO				0x010021

/* Warning: could not read block number of relocation process */
#define PR_1_RELOC_READ_ERR			0x010022

/* Warning: could not write block number of relocation process */
#define PR_1_RELOC_WRITE_ERR			0x010023

/* Error allocating inode bitmap */
#define PR_1_ALLOCATE_IBITMAP_ERROR		0x010024

/* Error allocating block bitmap */
#define PR_1_ALLOCATE_BBITMAP_ERROR		0x010025

/* Error allocating icount link information */
#define PR_1_ALLOCATE_ICOUNT			0x010026

/* Error allocating directory block array */
#define PR_1_ALLOCATE_DBCOUNT			0x010027

/* Error while scanning inodes */
#define PR_1_ISCAN_ERROR			0x010028

/* Error while iterating over blocks in inode */
#define PR_1_BLOCK_ITERATE			0x010029

/* Error storing inode count information */
#define PR_1_ICOUNT_STORE			0x01002A

/* Error storing directory block information */
#define PR_1_ADD_DBLOCK				0x01002B

/* Error reading inode (for clearing) */
#define PR_1_READ_INODE				0x01002C

/* Suppress messages prompt */
#define PR_1_SUPPRESS_MESSAGES			0x01002D

/* Imagic number has imagic flag set when fs doesn't support it */
#define PR_1_SET_IMAGIC				0x01002F

/* Immutable flag set on a device or socket inode */
#define PR_1_SET_IMMUTABLE			0x010030

/* Compression flag set on a non-compressed filesystem -- no longer used*/
/* #define PR_1_COMPR_SET			0x010031 */

/* Non-zero size on on device, fifo or socket inode */
#define PR_1_SET_NONZSIZE			0x010032

/* Filesystem has feature flag(s) set, but is a revision 0 filesystem */
#define PR_1_FS_REV_LEVEL			0x010033

/* Journal inode is not in use, but contains data */
#define PR_1_JOURNAL_INODE_NOT_CLEAR		0x010034

/* Journal is not a regular file */
#define PR_1_JOURNAL_BAD_MODE			0x010035

/* Inode that was part of the orphan list */
#define PR_1_LOW_DTIME				0x010036

/* Inodes that were part of a corrupted orphan linked list found
 * (latch question) */
#define PR_1_ORPHAN_LIST_REFUGEES		0x010037

/* Error allocating refcount structure */
#define PR_1_ALLOCATE_REFCOUNT			0x010038

/* Error reading extended attribute block */
#define PR_1_READ_EA_BLOCK			0x010039

/* Inode number has a bad extended attribute block */
#define PR_1_BAD_EA_BLOCK			0x01003A

/* Error reading Extended Attribute block while fixing refcount -- abort */
#define PR_1_EXTATTR_READ_ABORT			0x01003B

/* Extended attribute number has reference count incorrect, should be */
#define PR_1_EXTATTR_REFCOUNT			0x01003C

/* Error writing Extended Attribute block while fixing refcount */
#define PR_1_EXTATTR_WRITE_ABORT		0x01003D

/* Extended attribute block has h_blocks > 1 */
#define PR_1_EA_MULTI_BLOCK			0x01003E

/* Allocating extended attribute region allocation structure */
#define PR_1_EA_ALLOC_REGION_ABORT		0x01003F

/* Extended Attribute block number is corrupt (allocation collision) */
#define PR_1_EA_ALLOC_COLLISION			0x010040

/* Extended attribute block number is corrupt (invalid name) */
#define PR_1_EA_BAD_NAME			0x010041

/* Extended attribute block number is corrupt (invalid value) */
#define PR_1_EA_BAD_VALUE			0x010042

/* Inode number is too big (latch question) */
#define PR_1_INODE_TOOBIG			0x010043

/* Problem causes directory to be too big */
#define PR_1_TOOBIG_DIR				0x010044

/* Problem causes file to be too big */
#define PR_1_TOOBIG_REG				0x010045

/* Problem causes symlink to be too big */
#define PR_1_TOOBIG_SYMLINK			0x010046

/* Inode has INDEX_FL flag set on filesystem without htree support  */
#define PR_1_HTREE_SET				0x010047

/* Inode number has INDEX_FL flag set but is on a directory */
#define PR_1_HTREE_NODIR			0x010048

/* htree directory has an invalid root node */
#define PR_1_HTREE_BADROOT			0x010049

/* Htree directory has an unsupported hash version */
#define PR_1_HTREE_HASHV			0x01004A

/* Htree directory uses an Incompatible htree root node flag */
#define PR_1_HTREE_INCOMPAT			0x01004B

/* Htree directory has a tree depth which is too big */
#define PR_1_HTREE_DEPTH			0x01004C

/* Bad block inode has an indirect block number that conflicts with
 * filesystem metadata */
#define PR_1_BB_FS_BLOCK			0x01004D

/* Resize inode (re)creation failed */
#define PR_1_RESIZE_INODE_CREATE		0x01004E

/* inode has a extra size i_extra_isize which is invalid */
#define PR_1_EXTRA_ISIZE			0x01004F

/* Extended attribute in inode has a namelen which is invalid */
#define PR_1_ATTR_NAME_LEN			0x010050

/* Extended attribute in inode has a value offset which is invalid */
#define PR_1_ATTR_VALUE_OFFSET			0x010051

/* extended attribute in inode has a value block which is invalid */
#define PR_1_ATTR_VALUE_BLOCK			0x010052

/* extended attribute in inode has a value size which is invalid */
#define PR_1_ATTR_VALUE_SIZE			0x010053

/* extended attribute in inode has a hash which is invalid */
#define PR_1_ATTR_HASH				0x010054

/* inode is a type but it looks like it is really a directory */
#define PR_1_TREAT_AS_DIRECTORY			0x010055

/* Error while reading extent tree in inode */
#define PR_1_READ_EXTENT			0x010056

/* Failure to iterate extents in inode */
#define PR_1_EXTENT_ITERATE_FAILURE		0x010057

/* Inode has an invalid extent starting block */
#define PR_1_EXTENT_BAD_START_BLK		0x010058

/* Inode has an invalid extent that ends beyond filesystem */
#define PR_1_EXTENT_ENDS_BEYOND			0x010059

/* inode has EXTENTS_FL flag set on filesystem without extents support */
#define PR_1_EXTENTS_SET			0x01005A

/* inode is in extents format, but superblock is missing EXTENTS feature */
#define PR_1_EXTENT_FEATURE			0x01005B

/* inode missing EXTENTS_FL, but is an extent inode */
#define PR_1_UNSET_EXTENT_FL			0x01005C

/* Fast symlink has EXTENTS_FL set */
#define PR_1_FAST_SYMLINK_EXTENT_FL		0x01005D

/* Extents are out of order */
#define PR_1_OUT_OF_ORDER_EXTENTS		0x01005E

/* Extent node header invalid */
#define PR_1_EXTENT_HEADER_INVALID		0x01005F

/* PR_1_EOFBLOCKS_FL_SET 0x010060 was here */

/* Failed to convert subcluster block bitmap */
#define PR_1_CONVERT_SUBCLUSTER			0x010061

/* Quota inode is not a regular file */
#define PR_1_QUOTA_BAD_MODE			0x010062

/* Quota inode is not in use, but contains data */
#define PR_1_QUOTA_INODE_NOT_CLEAR		0x010063

/* Quota inode is visible to the user */
#define PR_1_QUOTA_INODE_NOT_HIDDEN		0x010064

/* The bad block inode looks invalid */
#define PR_1_INVALID_BAD_INODE			0x010065

/* Extent has zero length extent */
#define PR_1_EXTENT_LENGTH_ZERO			0x010066

/* inode seems to contain garbage */
#define PR_1_INODE_IS_GARBAGE			0x010067

/* inode passes checks, but checksum does not match inode */
#define PR_1_INODE_ONLY_CSUM_INVALID		0x010068

/* Inode extended attribute is corrupt (allocation collision) */
#define PR_1_INODE_EA_ALLOC_COLLISION		0x010069

/* Inode extent block passes checks, but checksum does not match extent */
#define PR_1_EXTENT_ONLY_CSUM_INVALID		0x01006A

/* Inode extended attribute block passes checks, but checksum does not
 * match block. */
#define PR_1_EA_BLOCK_ONLY_CSUM_INVALID		0x01006C

/* Interior extent node level number of inode doesn't first node down */
#define PR_1_EXTENT_INDEX_START_INVALID		0x01006D

/* Inode end of extent exceeds allowed value */
#define PR_1_EXTENT_END_OUT_OF_BOUNDS		0x01006E

/* inode has INLINE_DATA_FL flag on filesystem without inline data */
#define PR_1_INLINE_DATA_FEATURE		0x01006F

/* inode has INLINE_DATA_FL flag on filesystem without inline data */
#define PR_1_INLINE_DATA_SET			0x010070

/* Inode block conflicts with critical metadata, skipping block checks */
#define PR_1_CRITICAL_METADATA_COLLISION	0x010071

/* Directory inode block <block> should be at block <otherblock> */
#define PR_1_COLLAPSE_DBLOCK			0x010072

/* Directory inode block <block> should be at block <otherblock> */
#define PR_1_UNINIT_DBLOCK			0x010073

/* Inode logical block (physical block) violates cluster allocation */
#define PR_1_MISALIGNED_CLUSTER			0x010074

/* Inode has INLINE_DATA_FL flag but extended attribute not found */
#define PR_1_INLINE_DATA_NO_ATTR		0x010075

/* Special (device/socket/fifo) file (inode num) has extents
 * or inline-data flag set */
#define PR_1_SPECIAL_EXTENTS_IDATA		0x010076

/* Inode has extent header but inline data flag is set */
#define PR_1_CLEAR_INLINE_DATA_FOR_EXTENT	0x010077

/* Inode seems to have inline data but extent flag is set */
#define PR_1_CLEAR_EXTENT_FOR_INLINE_DATA	0x010078

/* Inode seems to have block map but inline data and extent flags set */
#define PR_1_CLEAR_EXTENT_INLINE_DATA_FLAGS	0x010079

/* Inode has inline data and extent flags but i_block contains junk */
#define PR_1_CLEAR_EXTENT_INLINE_DATA_INODE	0x01007A

/* Bad block list says the bad block list inode is bad */
#define PR_1_BADBLOCKS_IN_BADBLOCKS		0x01007B

/* Error allocating extent region allocation structure */
#define PR_1_EXTENT_ALLOC_REGION_ABORT		0x01007C

/* Inode leaf has a duplicate extent mapping */
#define PR_1_EXTENT_COLLISION			0x01007D

/* Error allocating memory for encrypted directory list */
#define PR_1_ALLOCATE_ENCRYPTED_DIRLIST		0x01007E

/* Inode extent tree could be more shallow */
#define PR_1_EXTENT_BAD_MAX_DEPTH		0x01007F

/* inode num on bigalloc filesystem cannot be block mapped */
#define PR_1_NO_BIGALLOC_BLOCKMAP_FILES		0x010080

/* Inode has corrupt extent header */
#define PR_1_MISSING_EXTENT_HEADER		0x010081

/* Timestamp(s) on inode beyond 2310-04-04 are likely pre-1970. */
#define PR_1_EA_TIME_OUT_OF_RANGE		0x010082

/* Inode has illegal EA value inode */
#define PR_1_ATTR_VALUE_EA_INODE		0x010083

/* Parent inode has invalid EA entry. EA inode does not have
 * EXT4_EA_INODE_FL flag. Delete EA entry? */
#define PR_1_ATTR_NO_EA_INODE_FL		0x010085

/* EA inode for parent inode does not have EXT4_EA_INODE_FL flag */
#define PR_1_ATTR_SET_EA_INODE_FL		0x010086

/* Offer to clear uninitialized flag on an extent */
#define PR_1_CLEAR_UNINIT_EXTENT		0x010087

/* Casefold flag set on a non-directory */
#define PR_1_CASEFOLD_NONDIR			0x010088

/* Casefold flag set, but file system is missing the casefold feature */
#define PR_1_CASEFOLD_FEATURE			0x010089


/*
 * Pass 1b errors
 */

/* Pass 1B: Rescan for duplicate/bad blocks */
#define PR_1B_PASS_HEADER	0x011000

/* Duplicate/bad block(s) header */
#define PR_1B_DUP_BLOCK_HEADER	0x011001

/* Duplicate/bad block(s) in inode */
#define PR_1B_DUP_BLOCK		0x011002

/* Duplicate/bad block(s) end */
#define PR_1B_DUP_BLOCK_END	0x011003

/* Error while scanning inodes */
#define PR_1B_ISCAN_ERROR	0x011004

/* Error allocating inode bitmap */
#define PR_1B_ALLOCATE_IBITMAP_ERROR 0x011005

/* Error while iterating over blocks */
#define PR_1B_BLOCK_ITERATE	0x011006

/* Error adjusting EA refcount */
#define PR_1B_ADJ_EA_REFCOUNT	0x011007

/* Duplicate/bad block range in inode */
#define PR_1B_DUP_RANGE		0x011008

/* Pass 1C: Scan directories for inodes with dup blocks. */
#define PR_1C_PASS_HEADER	0x012000


/* Pass 1D: Reconciling duplicate blocks */
#define PR_1D_PASS_HEADER	0x013000

/* File has duplicate blocks */
#define PR_1D_DUP_FILE		0x013001

/* List of files sharing duplicate blocks */
#define PR_1D_DUP_FILE_LIST	0x013002

/* File sharing blocks with filesystem metadata  */
#define PR_1D_SHARE_METADATA	0x013003

/* Report of how many duplicate/bad inodes */
#define PR_1D_NUM_DUP_INODES	0x013004

/* Duplicated blocks already reassigned or cloned. */
#define PR_1D_DUP_BLOCKS_DEALT	0x013005

/* Clone duplicate/bad blocks? */
#define PR_1D_CLONE_QUESTION	0x013006

/* Delete file? */
#define PR_1D_DELETE_QUESTION	0x013007

/* Couldn't clone file (error) */
#define PR_1D_CLONE_ERROR	0x013008

/*
 * Pass 1e --- rebuilding extent trees
 */
/* Pass 1e: Rebuilding extent trees */
#define PR_1E_PASS_HEADER		0x014000

/* Error rehash directory */
#define PR_1E_OPTIMIZE_EXT_ERR		0x014001

/* Rebuilding extent trees */
#define PR_1E_OPTIMIZE_EXT_HEADER	0x014002

/* Rebuilding extent %d */
#define PR_1E_OPTIMIZE_EXT		0x014003

/* Rebuilding extent tree end */
#define PR_1E_OPTIMIZE_EXT_END		0x014004

/* Internal error: extent tree depth too large */
#define PR_1E_MAX_EXTENT_TREE_DEPTH	0x014005

/* Inode extent tree could be shorter */
#define PR_1E_CAN_COLLAPSE_EXTENT_TREE	0x014006

/* Inode extent tree could be narrower */
#define PR_1E_CAN_NARROW_EXTENT_TREE	0x014007

/*
 * Pass 2 errors
 */

/* Pass 2: Checking directory structure */
#define PR_2_PASS_HEADER	0x020000

/* Bad inode number for '.' */
#define PR_2_BAD_INODE_DOT	0x020001

/* Directory entry has bad inode number */
#define PR_2_BAD_INO		0x020002

/* Directory entry has deleted or unused inode */
#define PR_2_UNUSED_INODE	0x020003

/* Directory entry is link to '.' */
#define PR_2_LINK_DOT		0x020004

/* Directory entry points to inode now located in a bad block */
#define PR_2_BB_INODE		0x020005

/* Directory entry contains a link to a directory */
#define PR_2_LINK_DIR		0x020006

/* Directory entry contains a link to the root directory */
#define PR_2_LINK_ROOT		0x020007

/* Directory entry has illegal characters in its name */
#define PR_2_BAD_NAME		0x020008

/* Missing '.' in directory inode */
#define PR_2_MISSING_DOT	0x020009

/* Missing '..' in directory inode */
#define PR_2_MISSING_DOT_DOT	0x02000A

/* First entry in directory inode doesn't contain '.' */
#define PR_2_1ST_NOT_DOT	0x02000B

/* Second entry in directory inode doesn't contain '..' */
#define PR_2_2ND_NOT_DOT_DOT	0x02000C

/* i_faddr should be zero */
#define PR_2_FADDR_ZERO		0x02000D

/* i_file_acl should be zero */
#define PR_2_FILE_ACL_ZERO	0x02000E

/* i_size_high should be zero */
#define PR_2_DIR_SIZE_HIGH_ZERO	0x02000F

/* i_frag should be zero */
#define PR_2_FRAG_ZERO		0x020010

/* i_fsize should be zero */
#define PR_2_FSIZE_ZERO		0x020011

/* inode has bad mode */
#define PR_2_BAD_MODE		0x020012

/* directory corrupted */
#define PR_2_DIR_CORRUPTED	0x020013

/* filename too long */
#define PR_2_FILENAME_LONG	0x020014

/* Directory inode has a missing block (hole) */
#define PR_2_DIRECTORY_HOLE	0x020015

/* '.' is not NULL terminated */
#define PR_2_DOT_NULL_TERM	0x020016

/* '..' is not NULL terminated */
#define PR_2_DOT_DOT_NULL_TERM	0x020017

/* Illegal character device in inode */
#define PR_2_BAD_CHAR_DEV	0x020018

/* Illegal block device in inode */
#define PR_2_BAD_BLOCK_DEV	0x020019

/* Duplicate '.' entry */
#define PR_2_DUP_DOT		0x02001A

/* Duplicate '..' entry */
#define PR_2_DUP_DOT_DOT	0x02001B

/* Internal error: couldn't find dir_info */
#define PR_2_NO_DIRINFO		0x02001C

/* Final rec_len is wrong */
#define PR_2_FINAL_RECLEN	0x02001D

/* Error allocating icount structure */
#define PR_2_ALLOCATE_ICOUNT	0x02001E

/* Error iterating over directory blocks */
#define PR_2_DBLIST_ITERATE	0x02001F

/* Error reading directory block */
#define PR_2_READ_DIRBLOCK	0x020020

/* Error writing directory block */
#define PR_2_WRITE_DIRBLOCK	0x020021

/* Error allocating new directory block */
#define PR_2_ALLOC_DIRBOCK	0x020022

/* Error deallocating inode */
#define PR_2_DEALLOC_INODE	0x020023

/* Directory entry for '.' is big.  Split? */
#define PR_2_SPLIT_DOT		0x020024

/* Illegal FIFO */
#define PR_2_BAD_FIFO		0x020025

/* Illegal socket */
#define PR_2_BAD_SOCKET		0x020026

/* Directory filetype not set */
#define PR_2_SET_FILETYPE	0x020027

/* Directory filetype incorrect */
#define PR_2_BAD_FILETYPE	0x020028

/* Directory filetype set when it shouldn't be */
#define PR_2_CLEAR_FILETYPE	0x020029

/* Directory filename can't be zero-length  */
#define PR_2_NULL_NAME		0x020030

/* Invalid symlink */
#define PR_2_INVALID_SYMLINK	0x020031

/* i_file_acl (extended attribute) is bad */
#define PR_2_FILE_ACL_BAD	0x020032

/* Filesystem contains large files, but has no such flag in sb */
#define PR_2_FEATURE_LARGE_FILES 0x020033

/* Node in HTREE directory not referenced */
#define PR_2_HTREE_NOTREF	0x020034

/* Node in HTREE directory referenced twice */
#define PR_2_HTREE_DUPREF	0x020035

/* Node in HTREE directory has bad min hash */
#define PR_2_HTREE_MIN_HASH	0x020036

/* Node in HTREE directory has bad max hash */
#define PR_2_HTREE_MAX_HASH	0x020037

/* Clear invalid HTREE directory */
#define PR_2_HTREE_CLEAR	0x020038

/* Clear the htree flag forcibly */
/* #define PR_2_HTREE_FCLR	0x020039 */

/* Bad block in htree interior node */
#define PR_2_HTREE_BADBLK	0x02003A

/* Error adjusting EA refcount */
#define PR_2_ADJ_EA_REFCOUNT	0x02003B

/* Invalid HTREE root node */
#define PR_2_HTREE_BAD_ROOT	0x02003C

/* Invalid HTREE limit */
#define PR_2_HTREE_BAD_LIMIT	0x02003D

/* Invalid HTREE count */
#define PR_2_HTREE_BAD_COUNT	0x02003E

/* HTREE interior node has out-of-order hashes in table */
#define PR_2_HTREE_HASH_ORDER	0x02003F

/* Node in HTREE directory has bad depth */
#define PR_2_HTREE_BAD_DEPTH	0x020040

/* Duplicate directory entry found */
#define PR_2_DUPLICATE_DIRENT	0x020041

/* Non-unique filename found */
#define PR_2_NON_UNIQUE_FILE	0x020042

/* Duplicate directory entry found */
#define PR_2_REPORT_DUP_DIRENT	0x020043

/* i_blocks_hi should be zero */
#define PR_2_BLOCKS_HI_ZERO	0x020044

/* Unexpected HTREE block */
#define PR_2_UNEXPECTED_HTREE_BLOCK	0x020045

/* Inode found in group where _INODE_UNINIT is set */
#define PR_2_INOREF_BG_INO_UNINIT	0x020046

/* Inode found in group unused inodes area */
#define PR_2_INOREF_IN_UNUSED		0x020047

/* i_file_acl_hi should be zero */
#define PR_2_I_FILE_ACL_HI_ZERO		0x020048

/* htree root node fails checksum */
#define PR_2_HTREE_ROOT_CSUM_INVALID	0x020049

/* htree node fails checksum */
#define PR_2_HTREE_NODE_CSUM_INVALID	0x02004A

/* no space in leaf for checksum */
#define PR_2_LEAF_NODE_MISSING_CSUM	0x02004C

/* dir leaf node passes checks, but fails checksum */
#define PR_2_LEAF_NODE_ONLY_CSUM_INVALID	0x02004D

/* bad inline directory size */
#define PR_2_BAD_INLINE_DIR_SIZE	0x02004E

/* fixing inline dir size failed */
#define PR_2_FIX_INLINE_DIR_FAILED	0x02004F

/* Encrypted directory entry is too short */
#define PR_2_BAD_ENCRYPTED_NAME		0x020050

/*
 * Pass 3 errors
 */

/* Pass 3: Checking directory connectivity */
#define PR_3_PASS_HEADER		0x030000

/* Root inode not allocated */
#define PR_3_NO_ROOT_INODE		0x030001

/* No room in lost+found */
#define PR_3_EXPAND_LF_DIR		0x030002

/* Unconnected directory inode */
#define PR_3_UNCONNECTED_DIR		0x030003

/* /lost+found not found */
#define PR_3_NO_LF_DIR			0x030004

/* .. entry is incorrect */
#define PR_3_BAD_DOT_DOT		0x030005

/* Bad or non-existent /lost+found.  Cannot reconnect */
#define PR_3_NO_LPF			0x030006

/* Could not expand /lost+found */
#define PR_3_CANT_EXPAND_LPF		0x030007

/* Could not reconnect inode */
#define PR_3_CANT_RECONNECT		0x030008

/* Error while trying to find /lost+found */
#define PR_3_ERR_FIND_LPF		0x030009

/* Error in ext2fs_new_block while creating /lost+found */
#define PR_3_ERR_LPF_NEW_BLOCK		0x03000A

/* Error in ext2fs_new_inode while creating /lost+found */
#define PR_3_ERR_LPF_NEW_INODE		0x03000B

/* Error in ext2fs_new_dir_block while creating /lost+found */
#define PR_3_ERR_LPF_NEW_DIR_BLOCK	0x03000C

/* Error while writing directory block for /lost+found */
#define PR_3_ERR_LPF_WRITE_BLOCK	0x03000D

/* Error while adjusting inode count */
#define PR_3_ADJUST_INODE		0x03000E

/* Couldn't fix parent directory -- error */
#define PR_3_FIX_PARENT_ERR		0x03000F

/* Couldn't fix parent directory -- couldn't find it */
#define PR_3_FIX_PARENT_NOFIND		0x030010

/* Error allocating inode bitmap */
#define PR_3_ALLOCATE_IBITMAP_ERROR	0x030011

/* Error creating root directory */
#define PR_3_CREATE_ROOT_ERROR		0x030012

/* Error creating lost and found directory */
#define PR_3_CREATE_LPF_ERROR		0x030013

/* Root inode is not directory; aborting */
#define PR_3_ROOT_NOT_DIR_ABORT		0x030014

/* Cannot proceed without a root inode. */
#define PR_3_NO_ROOT_INODE_ABORT	0x030015

/* Internal error: couldn't find dir_info */
#define PR_3_NO_DIRINFO			0x030016

/* Lost+found is not a directory */
#define PR_3_LPF_NOTDIR			0x030017

/* Lost+found has inline data */
#define PR_3_LPF_INLINE_DATA		0x030018

/* Cannot allocate lost+found */
#define PR_3_LPF_NO_SPACE		0x030019

/* Insufficient space to recover lost files */
#define PR_3_NO_SPACE_TO_RECOVER	0x03001A

/* Lost+found is encrypted */
#define PR_3_LPF_ENCRYPTED		0x03001B

/*
 * Pass 3a --- rehashing directories
 */
/* Pass 3a: Reindexing directories */
#define PR_3A_PASS_HEADER		0x031000

/* Error iterating over directories */
#define PR_3A_OPTIMIZE_ITER		0x031001

/* Error rehash directory */
#define PR_3A_OPTIMIZE_DIR_ERR		0x031002

/* Rehashing dir header */
#define PR_3A_OPTIMIZE_DIR_HEADER	0x031003

/* Rehashing directory %d */
#define PR_3A_OPTIMIZE_DIR		0x031004

/* Rehashing dir end */
#define PR_3A_OPTIMIZE_DIR_END		0x031005

/* Pass 3B is really just 1E */

/*
 * Pass 4 errors
 */

/* Pass 4: Checking reference counts */
#define PR_4_PASS_HEADER		0x040000

/* Unattached zero-length inode */
#define PR_4_ZERO_LEN_INODE		0x040001

/* Unattached inode */
#define PR_4_UNATTACHED_INODE		0x040002

/* Inode ref count wrong */
#define PR_4_BAD_REF_COUNT		0x040003

/* Inconsistent inode count information cached */
#define PR_4_INCONSISTENT_COUNT		0x040004

/* Extended attribute inode ref count wrong */
#define PR_4_EA_INODE_REF_COUNT		0x040005

/* directory exceeds max links, but no DIR_NLINK feature in superblock */
#define PR_4_DIR_NLINK_FEATURE		0x040006

/* Directory ref count set to overflow but it doesn't have to be */
#define PR_4_DIR_OVERFLOW_REF_COUNT	0x040007

/*
 * Pass 5 errors
 */

/* Pass 5: Checking group summary information */
#define PR_5_PASS_HEADER		0x050000

/* Padding at end of inode bitmap is not set. */
#define PR_5_INODE_BMAP_PADDING		0x050001

/* Padding at end of block bitmap is not set. */
#define PR_5_BLOCK_BMAP_PADDING		0x050002

/* Block bitmap differences header */
#define PR_5_BLOCK_BITMAP_HEADER	0x050003

/* Block not used, but marked in bitmap */
#define PR_5_BLOCK_UNUSED		0x050004

/* Block used, but not marked used in bitmap */
#define PR_5_BLOCK_USED			0x050005

/* Block bitmap differences end */
#define PR_5_BLOCK_BITMAP_END		0x050006

/* Inode bitmap differences header */
#define PR_5_INODE_BITMAP_HEADER	0x050007

/* Inode not used, but marked in bitmap */
#define PR_5_INODE_UNUSED		0x050008

/* Inode used, but not marked used in bitmap */
#define PR_5_INODE_USED			0x050009

/* Inode bitmap differences end */
#define PR_5_INODE_BITMAP_END		0x05000A

/* Free inodes count for group wrong */
#define PR_5_FREE_INODE_COUNT_GROUP	0x05000B

/* Directories count for group wrong */
#define PR_5_FREE_DIR_COUNT_GROUP	0x05000C

/* Free inodes count wrong */
#define PR_5_FREE_INODE_COUNT		0x05000D

/* Free blocks count for group wrong */
#define PR_5_FREE_BLOCK_COUNT_GROUP	0x05000E

/* Free blocks count wrong */
#define PR_5_FREE_BLOCK_COUNT		0x05000F

/* Programming error: bitmap endpoints don't match */
#define PR_5_BMAP_ENDPOINTS		0x050010

/* Internal error: fudging end of bitmap */
#define PR_5_FUDGE_BITMAP_ERROR		0x050011

/* Error copying in replacement inode bitmap */
#define PR_5_COPY_IBITMAP_ERROR		0x050012

/* Error copying in replacement block bitmap */
#define PR_5_COPY_BBITMAP_ERROR		0x050013

/* Block range not used, but marked in bitmap */
#define PR_5_BLOCK_RANGE_UNUSED		0x050014

/* Block range used, but not marked used in bitmap */
#define PR_5_BLOCK_RANGE_USED		0x050015

/* Inode range not used, but marked in bitmap */
#define PR_5_INODE_RANGE_UNUSED		0x050016

/* Inode range used, but not marked used in bitmap */
#define PR_5_INODE_RANGE_USED		0x050017

/* Block in use but group is marked BLOCK_UNINIT */
#define PR_5_BLOCK_UNINIT		0x050018

/* Inode in use but group is marked INODE_UNINIT */
#define PR_5_INODE_UNINIT		0x050019

/* Inode bitmap checksum does not match */
#define PR_5_INODE_BITMAP_CSUM_INVALID	0x05001A

/* Block bitmap checksum does not match */
#define PR_5_BLOCK_BITMAP_CSUM_INVALID	0x05001B

/*
 * Post-Pass 5 errors
 */

/* Recreate the journal if E2F_FLAG_JOURNAL_INODE flag is set */
#define PR_6_RECREATE_JOURNAL		0x060001

/* Update quota information if it is inconsistent */
#define PR_6_UPDATE_QUOTAS		0x060002

/* Error setting block group checksum info */
#define PR_6_SET_BG_CHECKSUM		0x060003

/* Error writing file system info */
#define PR_6_FLUSH_FILESYSTEM		0x060004

/* Error flushing writes to storage device */
#define PR_6_IO_FLUSH			0x060005

/* Error updating quota information */
#define PR_6_WRITE_QUOTAS		0x060006


/*
 * Function declarations
 */
int fix_problem(e2fsck_t ctx, problem_t code, struct problem_context *pctx);
int end_problem_latch(e2fsck_t ctx, int mask);
int set_latch_flags(int mask, int setflags, int clearflags);
int get_latch_flags(int mask, int *value);
void clear_problem_context(struct problem_context *pctx);

/* message.c */
void print_e2fsck_message(FILE *f, e2fsck_t ctx, const char *msg,
			  struct problem_context *pctx, int first,
			  int recurse);

