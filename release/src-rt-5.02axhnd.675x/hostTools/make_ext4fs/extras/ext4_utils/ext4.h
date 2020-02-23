/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _EXT4_H
#define _EXT4_H

#include <sys/types.h>

#undef EXT4FS_DEBUG

#ifdef EXT4FS_DEBUG
#define ext4_debug(f, a...)   do {   printk(KERN_DEBUG "EXT4-fs DEBUG (%s, %d): %s:",   __FILE__, __LINE__, __func__);   printk(KERN_DEBUG f, ## a);   } while (0)
#else
#define ext4_debug(f, a...) do {} while (0)
#endif

#define EXT4_ERROR_INODE(inode, fmt, a...)   ext4_error_inode(__func__, (inode), (fmt), ## a);

#define EXT4_ERROR_FILE(file, fmt, a...)   ext4_error_file(__func__, (file), (fmt), ## a);

typedef int ext4_grpblk_t;

typedef unsigned long long ext4_fsblk_t;

typedef __u32 ext4_lblk_t;

typedef unsigned int ext4_group_t;

#define EXT4_MB_HINT_MERGE 0x0001

#define EXT4_MB_HINT_RESERVED 0x0002

#define EXT4_MB_HINT_METADATA 0x0004

#define EXT4_MB_HINT_FIRST 0x0008

#define EXT4_MB_HINT_BEST 0x0010

#define EXT4_MB_HINT_DATA 0x0020

#define EXT4_MB_HINT_NOPREALLOC 0x0040

#define EXT4_MB_HINT_GROUP_ALLOC 0x0080

#define EXT4_MB_HINT_GOAL_ONLY 0x0100

#define EXT4_MB_HINT_TRY_GOAL 0x0200

#define EXT4_MB_DELALLOC_RESERVED 0x0400

#define EXT4_MB_STREAM_ALLOC 0x0800

struct ext4_allocation_request {

 struct inode *inode;

 unsigned int len;

 ext4_lblk_t logical;

 ext4_lblk_t lleft;

 ext4_lblk_t lright;

 ext4_fsblk_t goal;

 ext4_fsblk_t pleft;

 ext4_fsblk_t pright;

 unsigned int flags;
};

#define EXT4_BAD_INO 1  
#define EXT4_ROOT_INO 2  
#define EXT4_BOOT_LOADER_INO 5  
#define EXT4_UNDEL_DIR_INO 6  
#define EXT4_RESIZE_INO 7  
#define EXT4_JOURNAL_INO 8  

#define EXT4_GOOD_OLD_FIRST_INO 11

#define EXT4_LINK_MAX 65000

#define EXT4_MIN_BLOCK_SIZE 1024
#define EXT4_MAX_BLOCK_SIZE 65536
#define EXT4_MIN_BLOCK_LOG_SIZE 10
#define EXT4_BLOCK_SIZE(s) (EXT4_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define EXT4_ADDR_PER_BLOCK(s) (EXT4_BLOCK_SIZE(s) / sizeof(__u32))
#define EXT4_BLOCK_SIZE_BITS(s) ((s)->s_log_block_size + 10)
#define EXT4_INODE_SIZE(s) (((s)->s_rev_level == EXT4_GOOD_OLD_REV) ?   EXT4_GOOD_OLD_INODE_SIZE :   (s)->s_inode_size)
#define EXT4_FIRST_INO(s) (((s)->s_rev_level == EXT4_GOOD_OLD_REV) ?   EXT4_GOOD_OLD_FIRST_INO :   (s)->s_first_ino)
#define EXT4_BLOCK_ALIGN(size, blkbits) ALIGN((size), (1 << (blkbits)))

struct ext4_group_desc
{
 __le32 bg_block_bitmap_lo;
 __le32 bg_inode_bitmap_lo;
 __le32 bg_inode_table_lo;
 __le16 bg_free_blocks_count_lo;
 __le16 bg_free_inodes_count_lo;
 __le16 bg_used_dirs_count_lo;
 __le16 bg_flags;
 __u32 bg_reserved[2];
 __le16 bg_itable_unused_lo;
 __le16 bg_checksum;
 __le32 bg_block_bitmap_hi;
 __le32 bg_inode_bitmap_hi;
 __le32 bg_inode_table_hi;
 __le16 bg_free_blocks_count_hi;
 __le16 bg_free_inodes_count_hi;
 __le16 bg_used_dirs_count_hi;
 __le16 bg_itable_unused_hi;
 __u32 bg_reserved2[3];
};

#define EXT4_BG_INODE_UNINIT 0x0001  
#define EXT4_BG_BLOCK_UNINIT 0x0002  
#define EXT4_BG_INODE_ZEROED 0x0004  

#define EXT4_MIN_DESC_SIZE 32
#define EXT4_MIN_DESC_SIZE_64BIT 64
#define EXT4_MAX_DESC_SIZE EXT4_MIN_BLOCK_SIZE
#define EXT4_DESC_SIZE(s) (EXT4_SB(s)->s_desc_size)
#define EXT4_BLOCKS_PER_GROUP(s) ((s)->s_blocks_per_group)
#define EXT4_DESC_PER_BLOCK(s) (EXT4_BLOCK_SIZE(s) / EXT4_DESC_SIZE(s))
#define EXT4_INODES_PER_GROUP(s) ((s)->s_inodes_per_group)

#define EXT4_NDIR_BLOCKS 12
#define EXT4_IND_BLOCK EXT4_NDIR_BLOCKS
#define EXT4_DIND_BLOCK (EXT4_IND_BLOCK + 1)
#define EXT4_TIND_BLOCK (EXT4_DIND_BLOCK + 1)
#define EXT4_N_BLOCKS (EXT4_TIND_BLOCK + 1)

#define EXT4_SECRM_FL 0x00000001  
#define EXT4_UNRM_FL 0x00000002  
#define EXT4_COMPR_FL 0x00000004  
#define EXT4_SYNC_FL 0x00000008  
#define EXT4_IMMUTABLE_FL 0x00000010  
#define EXT4_APPEND_FL 0x00000020  
#define EXT4_NODUMP_FL 0x00000040  
#define EXT4_NOATIME_FL 0x00000080  

#define EXT4_DIRTY_FL 0x00000100
#define EXT4_COMPRBLK_FL 0x00000200  
#define EXT4_NOCOMPR_FL 0x00000400  
#define EXT4_ECOMPR_FL 0x00000800  

#define EXT4_INDEX_FL 0x00001000  
#define EXT4_IMAGIC_FL 0x00002000  
#define EXT4_JOURNAL_DATA_FL 0x00004000  
#define EXT4_NOTAIL_FL 0x00008000  
#define EXT4_DIRSYNC_FL 0x00010000  
#define EXT4_TOPDIR_FL 0x00020000  
#define EXT4_HUGE_FILE_FL 0x00040000  
#define EXT4_EXTENTS_FL 0x00080000  
#define EXT4_EA_INODE_FL 0x00200000  
#define EXT4_EOFBLOCKS_FL 0x00400000  
#define EXT4_RESERVED_FL 0x80000000  

#define EXT4_FL_USER_VISIBLE 0x004BDFFF  
#define EXT4_FL_USER_MODIFIABLE 0x004B80FF  

#define EXT4_FL_INHERITED (EXT4_SECRM_FL | EXT4_UNRM_FL | EXT4_COMPR_FL |  EXT4_SYNC_FL | EXT4_IMMUTABLE_FL | EXT4_APPEND_FL |  EXT4_NODUMP_FL | EXT4_NOATIME_FL |  EXT4_NOCOMPR_FL | EXT4_JOURNAL_DATA_FL |  EXT4_NOTAIL_FL | EXT4_DIRSYNC_FL)

#define EXT4_REG_FLMASK (~(EXT4_DIRSYNC_FL | EXT4_TOPDIR_FL))

#define EXT4_OTHER_FLMASK (EXT4_NODUMP_FL | EXT4_NOATIME_FL)

struct ext4_new_group_data {
 __u32 group;
 __u64 block_bitmap;
 __u64 inode_bitmap;
 __u64 inode_table;
 __u32 blocks_count;
 __u16 reserved_blocks;
 __u16 unused;
 __u32 free_blocks_count;
};

#define EXT4_GET_BLOCKS_CREATE 0x0001

#define EXT4_GET_BLOCKS_UNINIT_EXT 0x0002
#define EXT4_GET_BLOCKS_CREATE_UNINIT_EXT (EXT4_GET_BLOCKS_UNINIT_EXT|  EXT4_GET_BLOCKS_CREATE)

#define EXT4_GET_BLOCKS_DELALLOC_RESERVE 0x0004

#define EXT4_GET_BLOCKS_PRE_IO 0x0008
#define EXT4_GET_BLOCKS_CONVERT 0x0010
#define EXT4_GET_BLOCKS_IO_CREATE_EXT (EXT4_GET_BLOCKS_PRE_IO|  EXT4_GET_BLOCKS_CREATE_UNINIT_EXT)

#define EXT4_GET_BLOCKS_IO_CONVERT_EXT (EXT4_GET_BLOCKS_CONVERT|  EXT4_GET_BLOCKS_CREATE_UNINIT_EXT)

#define EXT4_FREE_BLOCKS_METADATA 0x0001
#define EXT4_FREE_BLOCKS_FORGET 0x0002
#define EXT4_FREE_BLOCKS_VALIDATED 0x0004

#define EXT4_IOC_GETFLAGS FS_IOC_GETFLAGS
#define EXT4_IOC_SETFLAGS FS_IOC_SETFLAGS
#define EXT4_IOC_GETVERSION _IOR('f', 3, long)
#define EXT4_IOC_SETVERSION _IOW('f', 4, long)
#define EXT4_IOC_GETVERSION_OLD FS_IOC_GETVERSION
#define EXT4_IOC_SETVERSION_OLD FS_IOC_SETVERSION
#define EXT4_IOC_GETRSVSZ _IOR('f', 5, long)
#define EXT4_IOC_SETRSVSZ _IOW('f', 6, long)
#define EXT4_IOC_GROUP_EXTEND _IOW('f', 7, unsigned long)
#define EXT4_IOC_GROUP_ADD _IOW('f', 8, struct ext4_new_group_input)
#define EXT4_IOC_MIGRATE _IO('f', 9)

#define EXT4_IOC_ALLOC_DA_BLKS _IO('f', 12)
#define EXT4_IOC_MOVE_EXT _IOWR('f', 15, struct move_extent)

#define EXT4_IOC32_GETFLAGS FS_IOC32_GETFLAGS
#define EXT4_IOC32_SETFLAGS FS_IOC32_SETFLAGS
#define EXT4_IOC32_GETVERSION _IOR('f', 3, int)
#define EXT4_IOC32_SETVERSION _IOW('f', 4, int)
#define EXT4_IOC32_GETRSVSZ _IOR('f', 5, int)
#define EXT4_IOC32_SETRSVSZ _IOW('f', 6, int)
#define EXT4_IOC32_GROUP_EXTEND _IOW('f', 7, unsigned int)
#define EXT4_IOC32_GETVERSION_OLD FS_IOC32_GETVERSION
#define EXT4_IOC32_SETVERSION_OLD FS_IOC32_SETVERSION

#define EXT4_MAX_BLOCK_FILE_PHYS 0xFFFFFFFF

struct ext4_inode {
 __le16 i_mode;
 __le16 i_uid;
 __le32 i_size_lo;
 __le32 i_atime;
 __le32 i_ctime;
 __le32 i_mtime;
 __le32 i_dtime;
 __le16 i_gid;
 __le16 i_links_count;
 __le32 i_blocks_lo;
 __le32 i_flags;
 union {
 struct {
 __le32 l_i_version;
 } linux1;
 struct {
 __u32 h_i_translator;
 } hurd1;
 struct {
 __u32 m_i_reserved1;
 } masix1;
 } osd1;
 __le32 i_block[EXT4_N_BLOCKS];
 __le32 i_generation;
 __le32 i_file_acl_lo;
 __le32 i_size_high;
 __le32 i_obso_faddr;
 union {
 struct {
 __le16 l_i_blocks_high;
 __le16 l_i_file_acl_high;
 __le16 l_i_uid_high;
 __le16 l_i_gid_high;
 __u32 l_i_reserved2;
 } linux2;
 struct {
 __le16 h_i_reserved1;
 __u16 h_i_mode_high;
 __u16 h_i_uid_high;
 __u16 h_i_gid_high;
 __u32 h_i_author;
 } hurd2;
 struct {
 __le16 h_i_reserved1;
 __le16 m_i_file_acl_high;
 __u32 m_i_reserved2[2];
 } masix2;
 } osd2;
 __le16 i_extra_isize;
 __le16 i_pad1;
 __le32 i_ctime_extra;
 __le32 i_mtime_extra;
 __le32 i_atime_extra;
 __le32 i_crtime;
 __le32 i_crtime_extra;
 __le32 i_version_hi;
};

struct move_extent {
 __u32 reserved;
 __u32 donor_fd;
 __u64 orig_start;
 __u64 donor_start;
 __u64 len;
 __u64 moved_len;
};

#define EXT4_EPOCH_BITS 2
#define EXT4_EPOCH_MASK ((1 << EXT4_EPOCH_BITS) - 1)
#define EXT4_NSEC_MASK (~0UL << EXT4_EPOCH_BITS)

#define EXT4_FITS_IN_INODE(ext4_inode, einode, field)   ((offsetof(typeof(*ext4_inode), field) +   sizeof((ext4_inode)->field))   <= (EXT4_GOOD_OLD_INODE_SIZE +   (einode)->i_extra_isize))  
#define EXT4_INODE_SET_XTIME(xtime, inode, raw_inode)  do {   (raw_inode)->xtime = cpu_to_le32((inode)->xtime.tv_sec);   if (EXT4_FITS_IN_INODE(raw_inode, EXT4_I(inode), xtime ## _extra))   (raw_inode)->xtime ## _extra =   ext4_encode_extra_time(&(inode)->xtime);  } while (0)
#define EXT4_EINODE_SET_XTIME(xtime, einode, raw_inode)  do {   if (EXT4_FITS_IN_INODE(raw_inode, einode, xtime))   (raw_inode)->xtime = cpu_to_le32((einode)->xtime.tv_sec);   if (EXT4_FITS_IN_INODE(raw_inode, einode, xtime ## _extra))   (raw_inode)->xtime ## _extra =   ext4_encode_extra_time(&(einode)->xtime);  } while (0)
#define EXT4_INODE_GET_XTIME(xtime, inode, raw_inode)  do {   (inode)->xtime.tv_sec = (signed)le32_to_cpu((raw_inode)->xtime);   if (EXT4_FITS_IN_INODE(raw_inode, EXT4_I(inode), xtime ## _extra))   ext4_decode_extra_time(&(inode)->xtime,   raw_inode->xtime ## _extra);  } while (0)
#define EXT4_EINODE_GET_XTIME(xtime, einode, raw_inode)  do {   if (EXT4_FITS_IN_INODE(raw_inode, einode, xtime))   (einode)->xtime.tv_sec =   (signed)le32_to_cpu((raw_inode)->xtime);   if (EXT4_FITS_IN_INODE(raw_inode, einode, xtime ## _extra))   ext4_decode_extra_time(&(einode)->xtime,   raw_inode->xtime ## _extra);  } while (0)
#define i_disk_version osd1.linux1.l_i_version

#define i_reserved1 osd1.linux1.l_i_reserved1
#define i_file_acl_high osd2.linux2.l_i_file_acl_high
#define i_blocks_high osd2.linux2.l_i_blocks_high
#define i_uid_low i_uid
#define i_gid_low i_gid
#define i_uid_high osd2.linux2.l_i_uid_high
#define i_gid_high osd2.linux2.l_i_gid_high
#define i_reserved2 osd2.linux2.l_i_reserved2

#define EXT4_VALID_FS 0x0001  
#define EXT4_ERROR_FS 0x0002  
#define EXT4_ORPHAN_FS 0x0004  

#define EXT2_FLAGS_SIGNED_HASH 0x0001  
#define EXT2_FLAGS_UNSIGNED_HASH 0x0002  
#define EXT2_FLAGS_TEST_FILESYS 0x0004  

#define EXT4_MOUNT_OLDALLOC 0x00002  
#define EXT4_MOUNT_GRPID 0x00004  
#define EXT4_MOUNT_DEBUG 0x00008  
#define EXT4_MOUNT_ERRORS_CONT 0x00010  
#define EXT4_MOUNT_ERRORS_RO 0x00020  
#define EXT4_MOUNT_ERRORS_PANIC 0x00040  
#define EXT4_MOUNT_MINIX_DF 0x00080  
#define EXT4_MOUNT_NOLOAD 0x00100  
#define EXT4_MOUNT_DATA_FLAGS 0x00C00  
#define EXT4_MOUNT_JOURNAL_DATA 0x00400  
#define EXT4_MOUNT_ORDERED_DATA 0x00800  
#define EXT4_MOUNT_WRITEBACK_DATA 0x00C00  
#define EXT4_MOUNT_UPDATE_JOURNAL 0x01000  
#define EXT4_MOUNT_NO_UID32 0x02000  
#define EXT4_MOUNT_XATTR_USER 0x04000  
#define EXT4_MOUNT_POSIX_ACL 0x08000  
#define EXT4_MOUNT_NO_AUTO_DA_ALLOC 0x10000  
#define EXT4_MOUNT_BARRIER 0x20000  
#define EXT4_MOUNT_NOBH 0x40000  
#define EXT4_MOUNT_QUOTA 0x80000  
#define EXT4_MOUNT_USRQUOTA 0x100000  
#define EXT4_MOUNT_GRPQUOTA 0x200000  
#define EXT4_MOUNT_DIOREAD_NOLOCK 0x400000  
#define EXT4_MOUNT_JOURNAL_CHECKSUM 0x800000  
#define EXT4_MOUNT_JOURNAL_ASYNC_COMMIT 0x1000000  
#define EXT4_MOUNT_I_VERSION 0x2000000  
#define EXT4_MOUNT_DELALLOC 0x8000000  
#define EXT4_MOUNT_DATA_ERR_ABORT 0x10000000  
#define EXT4_MOUNT_BLOCK_VALIDITY 0x20000000  
#define EXT4_MOUNT_DISCARD 0x40000000  

#define clear_opt(o, opt) o &= ~EXT4_MOUNT_##opt
#define set_opt(o, opt) o |= EXT4_MOUNT_##opt
#define test_opt(sb, opt) (EXT4_SB(sb)->s_mount_opt &   EXT4_MOUNT_##opt)

#define ext4_set_bit ext2_set_bit
#define ext4_set_bit_atomic ext2_set_bit_atomic
#define ext4_clear_bit ext2_clear_bit
#define ext4_clear_bit_atomic ext2_clear_bit_atomic
#define ext4_test_bit ext2_test_bit
#define ext4_find_first_zero_bit ext2_find_first_zero_bit
#define ext4_find_next_zero_bit ext2_find_next_zero_bit
#define ext4_find_next_bit ext2_find_next_bit

#define EXT4_DFL_MAX_MNT_COUNT 20  
#define EXT4_DFL_CHECKINTERVAL 0  

#define EXT4_ERRORS_CONTINUE 1  
#define EXT4_ERRORS_RO 2  
#define EXT4_ERRORS_PANIC 3  
#define EXT4_ERRORS_DEFAULT EXT4_ERRORS_CONTINUE

struct ext4_super_block {
  __le32 s_inodes_count;
 __le32 s_blocks_count_lo;
 __le32 s_r_blocks_count_lo;
 __le32 s_free_blocks_count_lo;
  __le32 s_free_inodes_count;
 __le32 s_first_data_block;
 __le32 s_log_block_size;
 __le32 s_obso_log_frag_size;
  __le32 s_blocks_per_group;
 __le32 s_obso_frags_per_group;
 __le32 s_inodes_per_group;
 __le32 s_mtime;
  __le32 s_wtime;
 __le16 s_mnt_count;
 __le16 s_max_mnt_count;
 __le16 s_magic;
 __le16 s_state;
 __le16 s_errors;
 __le16 s_minor_rev_level;
  __le32 s_lastcheck;
 __le32 s_checkinterval;
 __le32 s_creator_os;
 __le32 s_rev_level;
  __le16 s_def_resuid;
 __le16 s_def_resgid;

 __le32 s_first_ino;
 __le16 s_inode_size;
 __le16 s_block_group_nr;
 __le32 s_feature_compat;
  __le32 s_feature_incompat;
 __le32 s_feature_ro_compat;
  __u8 s_uuid[16];
  char s_volume_name[16];
  char s_last_mounted[64];
  __le32 s_algorithm_usage_bitmap;

 __u8 s_prealloc_blocks;
 __u8 s_prealloc_dir_blocks;
 __le16 s_reserved_gdt_blocks;

  __u8 s_journal_uuid[16];
  __le32 s_journal_inum;
 __le32 s_journal_dev;
 __le32 s_last_orphan;
 __le32 s_hash_seed[4];
 __u8 s_def_hash_version;
 __u8 s_reserved_char_pad;
 __le16 s_desc_size;
  __le32 s_default_mount_opts;
 __le32 s_first_meta_bg;
 __le32 s_mkfs_time;
 __le32 s_jnl_blocks[17];

  __le32 s_blocks_count_hi;
 __le32 s_r_blocks_count_hi;
 __le32 s_free_blocks_count_hi;
 __le16 s_min_extra_isize;
 __le16 s_want_extra_isize;
 __le32 s_flags;
 __le16 s_raid_stride;
 __le16 s_mmp_interval;
 __le64 s_mmp_block;
 __le32 s_raid_stripe_width;
 __u8 s_log_groups_per_flex;
 __u8 s_reserved_char_pad2;
 __le16 s_reserved_pad;
 __le64 s_kbytes_written;
 __u32 s_reserved[160];
};

#define EXT4_SB(sb) (sb)

#define NEXT_ORPHAN(inode) EXT4_I(inode)->i_dtime

#define EXT4_OS_LINUX 0
#define EXT4_OS_HURD 1
#define EXT4_OS_MASIX 2
#define EXT4_OS_FREEBSD 3
#define EXT4_OS_LITES 4

#define EXT4_GOOD_OLD_REV 0  
#define EXT4_DYNAMIC_REV 1  

#define EXT4_CURRENT_REV EXT4_GOOD_OLD_REV
#define EXT4_MAX_SUPP_REV EXT4_DYNAMIC_REV

#define EXT4_GOOD_OLD_INODE_SIZE 128

#define EXT4_HAS_COMPAT_FEATURE(sb,mask)   ((EXT4_SB(sb)->s_es->s_feature_compat & cpu_to_le32(mask)) != 0)
#define EXT4_HAS_RO_COMPAT_FEATURE(sb,mask)   ((EXT4_SB(sb)->s_es->s_feature_ro_compat & cpu_to_le32(mask)) != 0)
#define EXT4_HAS_INCOMPAT_FEATURE(sb,mask)   ((EXT4_SB(sb)->s_es->s_feature_incompat & cpu_to_le32(mask)) != 0)
#define EXT4_SET_COMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_compat |= cpu_to_le32(mask)
#define EXT4_SET_RO_COMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_ro_compat |= cpu_to_le32(mask)
#define EXT4_SET_INCOMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_incompat |= cpu_to_le32(mask)
#define EXT4_CLEAR_COMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_compat &= ~cpu_to_le32(mask)
#define EXT4_CLEAR_RO_COMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_ro_compat &= ~cpu_to_le32(mask)
#define EXT4_CLEAR_INCOMPAT_FEATURE(sb,mask)   EXT4_SB(sb)->s_es->s_feature_incompat &= ~cpu_to_le32(mask)

#define EXT4_FEATURE_COMPAT_DIR_PREALLOC 0x0001
#define EXT4_FEATURE_COMPAT_IMAGIC_INODES 0x0002
#define EXT4_FEATURE_COMPAT_HAS_JOURNAL 0x0004
#define EXT4_FEATURE_COMPAT_EXT_ATTR 0x0008
#define EXT4_FEATURE_COMPAT_RESIZE_INODE 0x0010
#define EXT4_FEATURE_COMPAT_DIR_INDEX 0x0020

#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT4_FEATURE_RO_COMPAT_LARGE_FILE 0x0002
#define EXT4_FEATURE_RO_COMPAT_BTREE_DIR 0x0004
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE 0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM 0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK 0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE 0x0040

#define EXT4_FEATURE_INCOMPAT_COMPRESSION 0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE 0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER 0x0004  
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV 0x0008  
#define EXT4_FEATURE_INCOMPAT_META_BG 0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS 0x0040  
#define EXT4_FEATURE_INCOMPAT_64BIT 0x0080
#define EXT4_FEATURE_INCOMPAT_MMP 0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG 0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE 0x0400  
#define EXT4_FEATURE_INCOMPAT_DIRDATA 0x1000  

#define EXT4_FEATURE_COMPAT_SUPP EXT2_FEATURE_COMPAT_EXT_ATTR
#define EXT4_FEATURE_INCOMPAT_SUPP (EXT4_FEATURE_INCOMPAT_FILETYPE|   EXT4_FEATURE_INCOMPAT_RECOVER|   EXT4_FEATURE_INCOMPAT_META_BG|   EXT4_FEATURE_INCOMPAT_EXTENTS|   EXT4_FEATURE_INCOMPAT_64BIT|   EXT4_FEATURE_INCOMPAT_FLEX_BG)
#define EXT4_FEATURE_RO_COMPAT_SUPP (EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER|   EXT4_FEATURE_RO_COMPAT_LARGE_FILE|   EXT4_FEATURE_RO_COMPAT_GDT_CSUM|   EXT4_FEATURE_RO_COMPAT_DIR_NLINK |   EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE |   EXT4_FEATURE_RO_COMPAT_BTREE_DIR |  EXT4_FEATURE_RO_COMPAT_HUGE_FILE)

#define EXT4_DEF_RESUID 0
#define EXT4_DEF_RESGID 0

#define EXT4_DEF_INODE_READAHEAD_BLKS 32

#define EXT4_DEFM_DEBUG 0x0001
#define EXT4_DEFM_BSDGROUPS 0x0002
#define EXT4_DEFM_XATTR_USER 0x0004
#define EXT4_DEFM_ACL 0x0008
#define EXT4_DEFM_UID16 0x0010
#define EXT4_DEFM_JMODE 0x0060
#define EXT4_DEFM_JMODE_DATA 0x0020
#define EXT4_DEFM_JMODE_ORDERED 0x0040
#define EXT4_DEFM_JMODE_WBACK 0x0060

#define EXT4_DEF_MIN_BATCH_TIME 0
#define EXT4_DEF_MAX_BATCH_TIME 15000  

#define EXT4_FLEX_SIZE_DIR_ALLOC_SCHEME 4

#define EXT4_NAME_LEN 255

struct ext4_dir_entry {
 __le32 inode;
 __le16 rec_len;
 __le16 name_len;
 char name[EXT4_NAME_LEN];
};

struct ext4_dir_entry_2 {
 __le32 inode;
 __le16 rec_len;
 __u8 name_len;
 __u8 file_type;
 char name[EXT4_NAME_LEN];
};

#define EXT4_FT_UNKNOWN 0
#define EXT4_FT_REG_FILE 1
#define EXT4_FT_DIR 2
#define EXT4_FT_CHRDEV 3
#define EXT4_FT_BLKDEV 4
#define EXT4_FT_FIFO 5
#define EXT4_FT_SOCK 6
#define EXT4_FT_SYMLINK 7

#define EXT4_FT_MAX 8

#define EXT4_DIR_PAD 4
#define EXT4_DIR_ROUND (EXT4_DIR_PAD - 1)
#define EXT4_DIR_REC_LEN(name_len) (((name_len) + 8 + EXT4_DIR_ROUND) &   ~EXT4_DIR_ROUND)
#define EXT4_MAX_REC_LEN ((1<<16)-1)

#define is_dx(dir) (EXT4_HAS_COMPAT_FEATURE(dir->i_sb,   EXT4_FEATURE_COMPAT_DIR_INDEX) &&   (EXT4_I(dir)->i_flags & EXT4_INDEX_FL))
#define EXT4_DIR_LINK_MAX(dir) (!is_dx(dir) && (dir)->i_nlink >= EXT4_LINK_MAX)
#define EXT4_DIR_LINK_EMPTY(dir) ((dir)->i_nlink == 2 || (dir)->i_nlink == 1)

#define DX_HASH_LEGACY 0
#define DX_HASH_HALF_MD4 1
#define DX_HASH_TEA 2
#define DX_HASH_LEGACY_UNSIGNED 3
#define DX_HASH_HALF_MD4_UNSIGNED 4
#define DX_HASH_TEA_UNSIGNED 5

#endif

