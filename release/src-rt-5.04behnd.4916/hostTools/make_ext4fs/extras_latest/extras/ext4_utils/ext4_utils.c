/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ext4_utils.h"
#include "allocate.h"
#include "indirect.h"
#include "extent.h"
#include "sha1.h"

#include <sparse/sparse.h>
#ifdef REAL_UUID
#include <uuid.h>
#endif

#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>

#ifdef USE_MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/ioctl.h>
#endif

#if defined(__linux__)
#include <linux/fs.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <sys/disk.h>
#endif

int force = 0;
struct fs_info info;
struct fs_aux_info aux_info;
struct sparse_file *ext4_sparse_file;

jmp_buf setjmp_env;

/* Definition from RFC-4122 */
struct uuid {
    u32 time_low;
    u16 time_mid;
    u16 time_hi_and_version;
    u8 clk_seq_hi_res;
    u8 clk_seq_low;
    u16 node0_1;
    u32 node2_5;
};

static void sha1_hash(const char *namespace, const char *name,
    unsigned char sha1[SHA1_DIGEST_LENGTH])
{
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (const u8*)namespace, strlen(namespace));
    SHA1Update(&ctx, (const u8*)name, strlen(name));
    SHA1Final(sha1, &ctx);
}

static void generate_sha1_uuid(const char *namespace, const char *name, u8 result[16])
{
    unsigned char sha1[SHA1_DIGEST_LENGTH];
    struct uuid *uuid = (struct uuid *)result;

    sha1_hash(namespace, name, (unsigned char*)sha1);
    memcpy(uuid, sha1, sizeof(struct uuid));

    uuid->time_low = ntohl(uuid->time_low);
    uuid->time_mid = ntohs(uuid->time_mid);
    uuid->time_hi_and_version = ntohs(uuid->time_hi_and_version);
    uuid->time_hi_and_version &= 0x0FFF;
    uuid->time_hi_and_version |= (5 << 12);
    uuid->clk_seq_hi_res &= ~(1 << 6);
    uuid->clk_seq_hi_res |= 1 << 7;
}

/* returns 1 if a is a power of b */
static int is_power_of(int a, int b)
{
	while (a > b) {
		if (a % b)
			return 0;
		a /= b;
	}

	return (a == b) ? 1 : 0;
}

int bitmap_get_bit(u8 *bitmap, u32 bit)
{
	if (bitmap[bit / 8] & (1 << (bit % 8)))
		return 1;

	return 0;
}

void bitmap_clear_bit(u8 *bitmap, u32 bit)
{
	bitmap[bit / 8] &= ~(1 << (bit % 8));

	return;
}

/* Returns 1 if the bg contains a backup superblock.  On filesystems with
   the sparse_super feature, only block groups 0, 1, and powers of 3, 5,
   and 7 have backup superblocks.  Otherwise, all block groups have backup
   superblocks */
int ext4_bg_has_super_block(int bg)
{
	/* Without sparse_super, every block group has a superblock */
	if (!(info.feat_ro_compat & EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER))
		return 1;

	if (bg == 0 || bg == 1)
		return 1;

	if (is_power_of(bg, 3) || is_power_of(bg, 5) || is_power_of(bg, 7))
		return 1;

	return 0;
}

/* Function to read the primary superblock */
void read_sb(int fd, struct ext4_super_block *sb)
{
	off64_t ret;

	ret = lseek64(fd, 1024, SEEK_SET);
	if (ret < 0)
		critical_error_errno("failed to seek to superblock");

	ret = read(fd, sb, sizeof(*sb));
	if (ret < 0)
		critical_error_errno("failed to read superblock");
	if (ret != sizeof(*sb))
		critical_error("failed to read all of superblock");
}

/* Function to write a primary or backup superblock at a given offset */
void write_sb(int fd, unsigned long long offset, struct ext4_super_block *sb)
{
	off64_t ret;

	ret = lseek64(fd, offset, SEEK_SET);
	if (ret < 0)
		critical_error_errno("failed to seek to superblock");

	ret = write(fd, sb, sizeof(*sb));
	if (ret < 0)
		critical_error_errno("failed to write superblock");
	if (ret != sizeof(*sb))
		critical_error("failed to write all of superblock");
}

static void block_device_write_sb(int fd)
{
	unsigned long long offset;
	u32 i;

	/* write out the backup superblocks */
	for (i = 1; i < aux_info.groups; i++) {
		if (ext4_bg_has_super_block(i)) {
			offset = info.block_size * (aux_info.first_data_block
				+ i * info.blocks_per_group);
			write_sb(fd, offset, aux_info.backup_sb[i]);
		}
	}

	/* write out the primary superblock */
	write_sb(fd, 1024, aux_info.sb);
}

/* Write the filesystem image to a file */
void write_ext4_image(int fd, int gz, int sparse, int crc)
{
	sparse_file_write(ext4_sparse_file, fd, gz, sparse, crc);

	if (info.block_device)
		block_device_write_sb(fd);
}

/* Compute the rest of the parameters of the filesystem from the basic info */
void ext4_create_fs_aux_info()
{
	aux_info.first_data_block = (info.block_size > 1024) ? 0 : 1;
	aux_info.len_blocks = info.len / info.block_size;
	aux_info.inode_table_blocks = DIV_ROUND_UP(info.inodes_per_group * info.inode_size,
		info.block_size);
	aux_info.groups = DIV_ROUND_UP(aux_info.len_blocks - aux_info.first_data_block,
		info.blocks_per_group);
	aux_info.blocks_per_ind = info.block_size / sizeof(u32);
	aux_info.blocks_per_dind = aux_info.blocks_per_ind * aux_info.blocks_per_ind;
	aux_info.blocks_per_tind = aux_info.blocks_per_dind * aux_info.blocks_per_dind;

	aux_info.bg_desc_blocks =
		DIV_ROUND_UP(aux_info.groups * sizeof(struct ext2_group_desc),
			info.block_size);

	aux_info.default_i_flags = EXT4_NOATIME_FL;

	u32 last_group_size = aux_info.len_blocks % info.blocks_per_group;
	u32 last_header_size = 2 + aux_info.inode_table_blocks;
	if (ext4_bg_has_super_block(aux_info.groups - 1))
		last_header_size += 1 + aux_info.bg_desc_blocks +
			info.bg_desc_reserve_blocks;
	if (last_group_size > 0 && last_group_size < last_header_size) {
		aux_info.groups--;
		aux_info.len_blocks -= last_group_size;
	}

	/* A zero-filled superblock to be written firstly to the block
	 * device to mark the file-system as invalid
	 */
	aux_info.sb_zero = calloc(1, info.block_size);
	if (!aux_info.sb_zero)
		critical_error_errno("calloc");

	/* The write_data* functions expect only block aligned calls.
	 * This is not an issue, except when we write out the super
	 * block on a system with a block size > 1K.  So, we need to
	 * deal with that here.
	 */
	aux_info.sb_block = calloc(1, info.block_size);
	if (!aux_info.sb_block)
		critical_error_errno("calloc");

	if (info.block_size > 1024)
		aux_info.sb = (struct ext4_super_block *)((char *)aux_info.sb_block + 1024);
	else
		aux_info.sb = aux_info.sb_block;

	/* Alloc an array to hold the pointers to the backup superblocks */
	aux_info.backup_sb = calloc(aux_info.groups, sizeof(char *));

	if (!aux_info.sb)
		critical_error_errno("calloc");

	aux_info.bg_desc = calloc(info.block_size, aux_info.bg_desc_blocks);
	if (!aux_info.bg_desc)
		critical_error_errno("calloc");
	aux_info.xattrs = NULL;
}

void ext4_free_fs_aux_info()
{
	unsigned int i;

	for (i=0; i<aux_info.groups; i++) {
		if (aux_info.backup_sb[i])
			free(aux_info.backup_sb[i]);
	}
	free(aux_info.sb_block);
	free(aux_info.sb_zero);
	free(aux_info.bg_desc);
}

/* Fill in the superblock memory buffer based on the filesystem parameters */
void ext4_fill_in_sb(int real_uuid)
{
	unsigned int i;
	struct ext4_super_block *sb = aux_info.sb;

	sb->s_inodes_count = info.inodes_per_group * aux_info.groups;
	sb->s_blocks_count_lo = aux_info.len_blocks;
	sb->s_r_blocks_count_lo = 0;
	sb->s_free_blocks_count_lo = 0;
	sb->s_free_inodes_count = 0;
	sb->s_first_data_block = aux_info.first_data_block;
	sb->s_log_block_size = log_2(info.block_size / 1024);
	sb->s_obso_log_frag_size = log_2(info.block_size / 1024);
	sb->s_blocks_per_group = info.blocks_per_group;
	sb->s_obso_frags_per_group = info.blocks_per_group;
	sb->s_inodes_per_group = info.inodes_per_group;
	sb->s_mtime = 0;
	sb->s_wtime = 0;
	sb->s_mnt_count = 0;
	sb->s_max_mnt_count = 10;
	sb->s_magic = EXT4_SUPER_MAGIC;
	sb->s_state = EXT4_VALID_FS;
	sb->s_errors = EXT4_ERRORS_RO;
	sb->s_minor_rev_level = 0;
	sb->s_lastcheck = 0;
	sb->s_checkinterval = 0;
	sb->s_creator_os = EXT4_OS_LINUX;
	sb->s_rev_level = EXT4_DYNAMIC_REV;
	sb->s_def_resuid = EXT4_DEF_RESUID;
	sb->s_def_resgid = EXT4_DEF_RESGID;

	sb->s_first_ino = EXT4_GOOD_OLD_FIRST_INO;
	sb->s_inode_size = info.inode_size;
	sb->s_block_group_nr = 0;
	sb->s_feature_compat = info.feat_compat;
	sb->s_feature_incompat = info.feat_incompat;
	sb->s_feature_ro_compat = info.feat_ro_compat;
	if (real_uuid == 1) {
#ifdef REAL_UUID
	    uuid_generate(sb->s_uuid);
#else
	    fprintf(stderr, "Not compiled with real UUID support\n");
	    abort();
#endif
	} else {
	    generate_sha1_uuid("extandroid/make_ext4fs", info.label, sb->s_uuid);
	}
	memset(sb->s_volume_name, 0, sizeof(sb->s_volume_name));
	strncpy(sb->s_volume_name, info.label, sizeof(sb->s_volume_name));
	memset(sb->s_last_mounted, 0, sizeof(sb->s_last_mounted));
	sb->s_algorithm_usage_bitmap = 0;

	sb->s_reserved_gdt_blocks = info.bg_desc_reserve_blocks;
	sb->s_prealloc_blocks = 0;
	sb->s_prealloc_dir_blocks = 0;

	//memcpy(sb->s_journal_uuid, sb->s_uuid, sizeof(sb->s_journal_uuid));
	if (info.feat_compat & EXT4_FEATURE_COMPAT_HAS_JOURNAL)
		sb->s_journal_inum = EXT4_JOURNAL_INO;
	sb->s_journal_dev = 0;
	sb->s_last_orphan = 0;
	sb->s_hash_seed[0] = 0; /* FIXME */
	sb->s_def_hash_version = DX_HASH_TEA;
	sb->s_reserved_char_pad = EXT4_JNL_BACKUP_BLOCKS;
	sb->s_desc_size = sizeof(struct ext2_group_desc);
	sb->s_default_mount_opts = 0; /* FIXME */
	sb->s_first_meta_bg = 0;
	sb->s_mkfs_time = 0;
	//sb->s_jnl_blocks[17]; /* FIXME */

	sb->s_blocks_count_hi = aux_info.len_blocks >> 32;
	sb->s_r_blocks_count_hi = 0;
	sb->s_free_blocks_count_hi = 0;
	sb->s_min_extra_isize = sizeof(struct ext4_inode) -
		EXT4_GOOD_OLD_INODE_SIZE;
	sb->s_want_extra_isize = sizeof(struct ext4_inode) -
		EXT4_GOOD_OLD_INODE_SIZE;
	sb->s_flags = 2;
	sb->s_raid_stride = 0;
	sb->s_mmp_interval = 0;
	sb->s_mmp_block = 0;
	sb->s_raid_stripe_width = 0;
	sb->s_log_groups_per_flex = 0;
	sb->s_kbytes_written = 0;

	for (i = 0; i < aux_info.groups; i++) {
		u64 group_start_block = aux_info.first_data_block + i *
			info.blocks_per_group;
		u32 header_size = 0;
		if (ext4_bg_has_super_block(i)) {
			if (i != 0) {
				aux_info.backup_sb[i] = calloc(info.block_size, 1);
				memcpy(aux_info.backup_sb[i], sb, sizeof(struct ext4_super_block));
				/* Update the block group nr of this backup superblock */
				aux_info.backup_sb[i]->s_block_group_nr = i;
				ext4_queue_sb(group_start_block, info.block_device ?
						aux_info.sb_zero : aux_info.backup_sb[i]);
			}
			sparse_file_add_data(ext4_sparse_file, aux_info.bg_desc,
				aux_info.bg_desc_blocks * info.block_size,
				group_start_block + 1);
			header_size = 1 + aux_info.bg_desc_blocks + info.bg_desc_reserve_blocks;
		}

		aux_info.bg_desc[i].bg_block_bitmap = group_start_block + header_size;
		aux_info.bg_desc[i].bg_inode_bitmap = group_start_block + header_size + 1;
		aux_info.bg_desc[i].bg_inode_table = group_start_block + header_size + 2;

		aux_info.bg_desc[i].bg_free_blocks_count = sb->s_blocks_per_group;
		aux_info.bg_desc[i].bg_free_inodes_count = sb->s_inodes_per_group;
		aux_info.bg_desc[i].bg_used_dirs_count = 0;
	}

	/* Queue the primary superblock to be written out - if it's a block device,
	 * queue a zero-filled block first, the correct version of superblock will
	 * be written to the block device after all other blocks are written.
	 *
	 * The file-system on the block device will not be valid until the correct
	 * version of superblocks are written, this is to avoid the likelihood of a
	 * partially created file-system.
	 */
	ext4_queue_sb(aux_info.first_data_block, info.block_device ?
				aux_info.sb_zero : aux_info.sb_block);
}


void ext4_queue_sb(u64 start_block, struct ext4_super_block *sb)
{
	sparse_file_add_data(ext4_sparse_file, sb, info.block_size, start_block);
}

void ext4_parse_sb_info(struct ext4_super_block *sb)
{
	if (sb->s_magic != EXT4_SUPER_MAGIC)
		error("superblock magic incorrect");

	if ((sb->s_state & EXT4_VALID_FS) != EXT4_VALID_FS)
		error("filesystem state not valid");

	ext4_parse_sb(sb, &info);

	ext4_create_fs_aux_info();

	memcpy(aux_info.sb, sb, sizeof(*sb));

	if (aux_info.first_data_block != sb->s_first_data_block)
		critical_error("first data block does not match");
}

void ext4_create_resize_inode()
{
	struct block_allocation *reserve_inode_alloc = create_allocation();
	u32 reserve_inode_len = 0;
	unsigned int i;

	struct ext4_inode *inode = get_inode(EXT4_RESIZE_INO);
	if (inode == NULL) {
		error("failed to get resize inode");
		return;
	}

	for (i = 0; i < aux_info.groups; i++) {
		if (ext4_bg_has_super_block(i)) {
			u64 group_start_block = aux_info.first_data_block + i *
				info.blocks_per_group;
			u32 reserved_block_start = group_start_block + 1 +
				aux_info.bg_desc_blocks;
			u32 reserved_block_len = info.bg_desc_reserve_blocks;
			append_region(reserve_inode_alloc, reserved_block_start,
				reserved_block_len, i);
			reserve_inode_len += reserved_block_len;
		}
	}

	inode_attach_resize(inode, reserve_inode_alloc);

	inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR;
	inode->i_links_count = 1;

	free_alloc(reserve_inode_alloc);
}

/* Allocate the blocks to hold a journal inode and connect them to the
   reserved journal inode */
void ext4_create_journal_inode()
{
	struct ext4_inode *inode = get_inode(EXT4_JOURNAL_INO);
	if (inode == NULL) {
		error("failed to get journal inode");
		return;
	}

	u8 *journal_data = inode_allocate_data_extents(inode,
			info.journal_blocks * info.block_size,
			info.journal_blocks * info.block_size);
	if (!journal_data) {
		error("failed to allocate extents for journal data");
		return;
	}

	inode->i_mode = S_IFREG | S_IRUSR | S_IWUSR;
	inode->i_links_count = 1;

	journal_superblock_t *jsb = (journal_superblock_t *)journal_data;
	jsb->s_header.h_magic = htonl(JBD2_MAGIC_NUMBER);
	jsb->s_header.h_blocktype = htonl(JBD2_SUPERBLOCK_V2);
	jsb->s_blocksize = htonl(info.block_size);
	jsb->s_maxlen = htonl(info.journal_blocks);
	jsb->s_nr_users = htonl(1);
	jsb->s_first = htonl(1);
	jsb->s_sequence = htonl(1);

	memcpy(aux_info.sb->s_jnl_blocks, &inode->i_block, sizeof(inode->i_block));
}

/* Update the number of free blocks and inodes in the filesystem and in each
   block group */
void ext4_update_free()
{
	u32 i;

	for (i = 0; i < aux_info.groups; i++) {
		u32 bg_free_blocks = get_free_blocks(i);
		u32 bg_free_inodes = get_free_inodes(i);
		u16 crc;

		aux_info.bg_desc[i].bg_free_blocks_count = bg_free_blocks;
		aux_info.sb->s_free_blocks_count_lo += bg_free_blocks;

		aux_info.bg_desc[i].bg_free_inodes_count = bg_free_inodes;
		aux_info.sb->s_free_inodes_count += bg_free_inodes;

		aux_info.bg_desc[i].bg_used_dirs_count += get_directories(i);

		aux_info.bg_desc[i].bg_flags = get_bg_flags(i);

		crc = ext4_crc16(~0, aux_info.sb->s_uuid, sizeof(aux_info.sb->s_uuid));
		crc = ext4_crc16(crc, &i, sizeof(i));
		crc = ext4_crc16(crc, &aux_info.bg_desc[i], offsetof(struct ext2_group_desc, bg_checksum));
		aux_info.bg_desc[i].bg_checksum = crc;
	}
}

u64 get_block_device_size(int fd)
{
	u64 size = 0;
	int ret;

#if defined(__linux__)
	ret = ioctl(fd, BLKGETSIZE64, &size);
#elif defined(__APPLE__) && defined(__MACH__)
	ret = ioctl(fd, DKIOCGETBLOCKCOUNT, &size);
#else
	close(fd);
	return 0;
#endif

	if (ret)
		return 0;

	return size;
}

int is_block_device_fd(int fd)
{
#ifdef USE_MINGW
	return 0;
#else
	struct stat st;
	int ret = fstat(fd, &st);
	if (ret < 0)
		return 0;

	return S_ISBLK(st.st_mode);
#endif
}

u64 get_file_size(int fd)
{
	struct stat buf;
	int ret;
	u64 reserve_len = 0;
	s64 computed_size;

	ret = fstat(fd, &buf);
	if (ret)
		return 0;

	if (info.len < 0)
		reserve_len = -info.len;

	if (S_ISREG(buf.st_mode))
		computed_size = buf.st_size - reserve_len;
	else if (S_ISBLK(buf.st_mode))
		computed_size = get_block_device_size(fd) - reserve_len;
	else
		computed_size = 0;

	if (computed_size < 0) {
		warn("Computed filesystem size less than 0");
		computed_size = 0;
	}

	return computed_size;
}

u64 parse_num(const char *arg)
{
	char *endptr;
	u64 num = strtoull(arg, &endptr, 10);
	if (*endptr == 'k' || *endptr == 'K')
		num *= 1024LL;
	else if (*endptr == 'm' || *endptr == 'M')
		num *= 1024LL * 1024LL;
	else if (*endptr == 'g' || *endptr == 'G')
		num *= 1024LL * 1024LL * 1024LL;

	return num;
}

int read_ext(int fd, int verbose)
{
	off64_t ret;
	struct ext4_super_block sb;

	read_sb(fd, &sb);

	ext4_parse_sb_info(&sb);

	ret = lseek64(fd, info.len, SEEK_SET);
	if (ret < 0)
		critical_error_errno("failed to seek to end of input image");

	ret = lseek64(fd, info.block_size * (aux_info.first_data_block + 1), SEEK_SET);
	if (ret < 0)
		critical_error_errno("failed to seek to block group descriptors");

	ret = read(fd, aux_info.bg_desc, info.block_size * aux_info.bg_desc_blocks);
	if (ret < 0)
		critical_error_errno("failed to read block group descriptors");
	if (ret != (int)info.block_size * (int)aux_info.bg_desc_blocks)
		critical_error("failed to read all of block group descriptors");

	if (verbose) {
		printf("Found filesystem with parameters:\n");
		printf("    Size: %"PRIu64"\n", info.len);
		printf("    Block size: %d\n", info.block_size);
		printf("    Blocks per group: %d\n", info.blocks_per_group);
		printf("    Inodes per group: %d\n", info.inodes_per_group);
		printf("    Inode size: %d\n", info.inode_size);
		printf("    Label: %s\n", info.label);
		printf("    Blocks: %"PRIu64"\n", aux_info.len_blocks);
		printf("    Block groups: %d\n", aux_info.groups);
		printf("    Reserved block group size: %d\n", info.bg_desc_reserve_blocks);
		printf("    Used %d/%d inodes and %d/%d blocks\n",
			aux_info.sb->s_inodes_count - aux_info.sb->s_free_inodes_count,
			aux_info.sb->s_inodes_count,
			aux_info.sb->s_blocks_count_lo - aux_info.sb->s_free_blocks_count_lo,
			aux_info.sb->s_blocks_count_lo);
	}

	return 0;
}

