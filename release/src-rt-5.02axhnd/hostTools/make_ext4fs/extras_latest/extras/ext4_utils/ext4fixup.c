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
#include "make_ext4fs.h"
#include "ext4_extents.h"
#include "allocate.h"
#include "ext4fixup.h"

#include <sparse/sparse.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef USE_MINGW
#include <sys/mman.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define lseek64 lseek
#define off64_t off_t
#endif

/* The inode block count for a file/directory is in units of 512 byte blocks,
 * _NOT_ the filesystem block size!
 */
#define INODE_BLOCK_SIZE 512

#define MAX_EXT4_BLOCK_SIZE 4096

/* The two modes the recurse_dir() can be in */
#define SANITY_CHECK_PASS 1
#define MARK_INODE_NUMS   2
#define UPDATE_INODE_NUMS 3

/* Magic numbers to indicate what state the update process is in */
#define MAGIC_STATE_MARKING_INUMS  0x7000151515565512ll
#define MAGIC_STATE_UPDATING_INUMS 0x6121131211735123ll
#define MAGIC_STATE_UPDATING_SB    0x15e1715151558477ll

/* Internal state variables corresponding to the magic numbers */
#define STATE_UNSET          0
#define STATE_MARKING_INUMS  1
#define STATE_UPDATING_INUMS 2
#define STATE_UPDATING_SB    3

/* Used for automated testing of this programs ability to stop and be restarted wthout error */
static int bail_phase = 0;
static int bail_loc = 0;
static int bail_count = 0;
static int count = 0;

/* global flags */
static int verbose = 0;
static int no_write = 0;

static int new_inodes_per_group = 0;

static int no_write_fixup_state = 0;

static int compute_new_inum(unsigned int old_inum)
{
    unsigned int group, offset;

    group = (old_inum - 1) / info.inodes_per_group;
    offset = (old_inum -1) % info.inodes_per_group;

    return (group * new_inodes_per_group) + offset + 1;
}

static int get_fs_fixup_state(int fd)
{
    unsigned long long magic;
    int ret, len;

    if (no_write) {
        return no_write_fixup_state;
    }

    lseek64(fd, 0, SEEK_SET);
    len = read(fd, &magic, sizeof(magic));
    if (len != sizeof(magic)) {
        critical_error("cannot read fixup_state\n");
    }

    switch (magic) {
        case MAGIC_STATE_MARKING_INUMS:
            ret = STATE_MARKING_INUMS;
            break;
        case MAGIC_STATE_UPDATING_INUMS:
            ret = STATE_UPDATING_INUMS;
            break;
        case MAGIC_STATE_UPDATING_SB:
            ret = STATE_UPDATING_SB;
            break;
        default:
            ret = STATE_UNSET;
    }
    return ret;
}

static int set_fs_fixup_state(int fd, int state)
{
    unsigned long long magic;
    struct ext4_super_block sb;
    int len;

    if (no_write) {
        no_write_fixup_state = state;
        return 0;
    }

    switch (state) {
        case STATE_MARKING_INUMS:
            magic = MAGIC_STATE_MARKING_INUMS;
            break;
        case STATE_UPDATING_INUMS:
            magic = MAGIC_STATE_UPDATING_INUMS;
            break;
        case STATE_UPDATING_SB:
            magic = MAGIC_STATE_UPDATING_SB;
            break;
        case STATE_UNSET:
        default:
            magic = 0ll;
            break;
    }

    lseek64(fd, 0, SEEK_SET);
    len = write(fd, &magic, sizeof(magic));
    if (len != sizeof(magic)) {
        critical_error("cannot write fixup_state\n");
    }

    read_sb(fd, &sb);
    if (magic) {
        /* If we are in the process of updating the filesystem, make it unmountable */
        sb.s_desc_size |= 1;
    } else {
        /* we are done, so make the filesystem mountable again */
        sb.s_desc_size &= ~1;
    }

    if (!no_write) {
        write_sb(fd, 1024, &sb);
    }

    return 0;
}

static int read_inode(int fd, unsigned int inum, struct ext4_inode *inode)
{
    unsigned int bg_num, bg_offset;
    off64_t inode_offset;
    int len;

    bg_num = (inum-1) / info.inodes_per_group;
    bg_offset = (inum-1) % info.inodes_per_group;

    inode_offset = ((unsigned long long)aux_info.bg_desc[bg_num].bg_inode_table * info.block_size) +
                    (bg_offset * info.inode_size);

    if (lseek64(fd, inode_offset, SEEK_SET) < 0) {
        critical_error_errno("failed to seek to inode %d\n", inum);
    }

    len=read(fd, inode, sizeof(*inode));
    if (len != sizeof(*inode)) {
        critical_error_errno("failed to read inode %d\n", inum);
    }

    return 0;
}

static int read_block(int fd, unsigned long long block_num, void *block)
{
    off64_t off;
    unsigned int len;

    off = block_num * info.block_size;

    if (lseek64(fd, off, SEEK_SET) , 0) {
        critical_error_errno("failed to seek to block %lld\n", block_num);
    }

    len=read(fd, block, info.block_size);
    if (len != info.block_size) {
        critical_error_errno("failed to read block %lld\n", block_num);
    }

    return 0;
}

static int write_block(int fd, unsigned long long block_num, void *block)
{
    off64_t off;
    unsigned int len;

    if (no_write) {
        return 0;
    }

    off = block_num * info.block_size;

    if (lseek64(fd, off, SEEK_SET) < 0) {
        critical_error_errno("failed to seek to block %lld\n", block_num);
    }

    len=write(fd, block, info.block_size);
    if (len != info.block_size) {
        critical_error_errno("failed to write block %lld\n", block_num);
    }

    return 0;
}

static void check_inode_bitmap(int fd, unsigned int bg_num)
{
    unsigned int inode_bitmap_block_num;
    unsigned char block[MAX_EXT4_BLOCK_SIZE];
    int i, bitmap_updated = 0;

    /* Using the bg_num, aux_info.bg_desc[], info.inodes_per_group and
     * new_inodes_per_group, retrieve the inode bitmap, and make sure
     * the bits between the old and new size are clear
     */
    inode_bitmap_block_num = aux_info.bg_desc[bg_num].bg_inode_bitmap;

    read_block(fd, inode_bitmap_block_num, block);

    for (i = info.inodes_per_group; i < new_inodes_per_group; i++) {
        if (bitmap_get_bit(block, i)) {
            bitmap_clear_bit(block, i);
            bitmap_updated = 1;
        }
    }

    if (bitmap_updated) {
        if (verbose) {
            printf("Warning: updated inode bitmap for block group %d\n", bg_num);
        }
        write_block(fd, inode_bitmap_block_num, block);
    }

    return;
}

/* Update the superblock and bgdesc of the specified block group */
static int update_superblocks_and_bg_desc(int fd, int state)
{
    off64_t ret;
    struct ext4_super_block sb;
    unsigned int num_block_groups, total_new_inodes;
    unsigned int i;


    read_sb(fd, &sb);

    /* Compute how many more inodes are now available */
    num_block_groups = DIV_ROUND_UP(aux_info.len_blocks, info.blocks_per_group);
    total_new_inodes = num_block_groups * (new_inodes_per_group - sb.s_inodes_per_group);

    if (verbose) {
        printf("created %d additional inodes\n", total_new_inodes);
    }

    /* Update the free inodes count in each block group descriptor */
    for (i = 0; i < num_block_groups; i++) {
       if (state == STATE_UPDATING_SB) {
           aux_info.bg_desc[i].bg_free_inodes_count += (new_inodes_per_group - sb.s_inodes_per_group);
       }
       check_inode_bitmap(fd, i);
    }

    /* First some sanity checks */
    if ((sb.s_inodes_count + total_new_inodes) != (new_inodes_per_group * num_block_groups)) {
        critical_error("Failed sanity check on new inode count\n");
    }
    if (new_inodes_per_group % (info.block_size/info.inode_size)) {
        critical_error("Failed sanity check on new inode per group alignment\n");
    }

    /* Update the free inodes count in the superblock */
    sb.s_inodes_count += total_new_inodes;
    sb.s_free_inodes_count += total_new_inodes;
    sb.s_inodes_per_group = new_inodes_per_group;

    for (i = 0; i < aux_info.groups; i++) {
        if (ext4_bg_has_super_block(i)) {
            unsigned int sb_offset;

            if (i == 0) {
              /* The first superblock is offset by 1K to leave room for boot sectors */
              sb_offset = 1024;
            } else {
              sb_offset = 0;
            }

            sb.s_block_group_nr = i;
            /* Don't write out the backup superblocks with the bit set in the s_desc_size
             * which prevents the filesystem from mounting.  The bit for the primary
             * superblock will be cleared on the final call to set_fs_fixup_state() */
            if (i != 0) {
                sb.s_desc_size &= ~1;
            }

            if (!no_write) {
                write_sb(fd,
                         (unsigned long long)i
                         * info.blocks_per_group * info.block_size
                         + sb_offset,
                         &sb);
            }

            ret = lseek64(fd, ((unsigned long long)i * info.blocks_per_group * info.block_size) +
                              (info.block_size * (aux_info.first_data_block + 1)), SEEK_SET);
            if (ret < 0)
                critical_error_errno("failed to seek to block group descriptors");

            if (!no_write) {
                ret = write(fd, aux_info.bg_desc, info.block_size * aux_info.bg_desc_blocks);
                if (ret < 0)
                    critical_error_errno("failed to write block group descriptors");
                if (ret != (int)info.block_size * (int)aux_info.bg_desc_blocks)
                    critical_error("failed to write all of block group descriptors");
            }
        }
        if ((bail_phase == 4) && ((unsigned int)bail_count == i)) {
            critical_error("bailing at phase 4\n");
        }
    }

    return 0;
}


static int get_direct_blocks(struct ext4_inode *inode, unsigned long long *block_list,
                                                       unsigned int *count)
{
    unsigned int i = 0;
    unsigned int ret = 0;
    unsigned int sectors_per_block;

    sectors_per_block = info.block_size / INODE_BLOCK_SIZE;
    while ((i < (inode->i_blocks_lo / sectors_per_block)) && (i < EXT4_NDIR_BLOCKS)) {
        block_list[i] = inode->i_block[i];
        i++;
    }

    *count += i;

    if ((inode->i_blocks_lo / sectors_per_block) > EXT4_NDIR_BLOCKS) {
        ret = 1;
    }

    return ret;
}

static int get_indirect_blocks(int fd, struct ext4_inode *inode,
                               unsigned long long *block_list, unsigned int *count)
{
    unsigned int i;
    unsigned int *indirect_block;
    unsigned int sectors_per_block;

    sectors_per_block = info.block_size / INODE_BLOCK_SIZE;

    indirect_block = (unsigned int *)malloc(info.block_size);
    if (indirect_block == 0) {
        critical_error("failed to allocate memory for indirect_block\n");
    }

    read_block(fd, inode->i_block[EXT4_NDIR_BLOCKS], indirect_block);

    for(i = 0; i < (inode->i_blocks_lo / sectors_per_block - EXT4_NDIR_BLOCKS); i++) {
       block_list[EXT4_NDIR_BLOCKS+i] = indirect_block[i];
    }

    *count += i;

    free(indirect_block);

    return 0;
}

static int get_block_list_indirect(int fd, struct ext4_inode *inode, unsigned long long *block_list)
{
    unsigned int count=0;

    if (get_direct_blocks(inode, block_list, &count)) {
        get_indirect_blocks(fd, inode, block_list, &count);
    }

    return count;
}

static int get_extent_ents(struct ext4_extent_header *ext_hdr, unsigned long long *block_list)
{
    int i, j;
    struct ext4_extent *extent;
    off64_t fs_block_num;

    if (ext_hdr->eh_depth != 0) {
        critical_error("get_extent_ents called with eh_depth != 0\n");
    }

    /* The extent entries immediately follow the header, so add 1 to the pointer
     * and cast it to an extent pointer.
     */
    extent = (struct ext4_extent *)(ext_hdr + 1);

    for (i = 0; i < ext_hdr->eh_entries; i++) {
         fs_block_num = ((off64_t)extent->ee_start_hi << 32) | extent->ee_start_lo;
         for (j = 0; j < extent->ee_len; j++) {
             block_list[extent->ee_block+j] = fs_block_num+j;
         }
         extent++;
    }

    return 0;
}

static int get_extent_idx(int fd, struct ext4_extent_header *ext_hdr, unsigned long long *block_list)
{
    int i;
    struct ext4_extent_idx *extent_idx;
    struct ext4_extent_header *tmp_ext_hdr;
    off64_t fs_block_num;
    unsigned char block[MAX_EXT4_BLOCK_SIZE];

    /* Sanity check */
    if (ext_hdr->eh_depth == 0) {
        critical_error("get_extent_idx called with eh_depth == 0\n");
    }

    /* The extent entries immediately follow the header, so add 1 to the pointer
     * and cast it to an extent pointer.
     */
    extent_idx = (struct ext4_extent_idx *)(ext_hdr + 1);

    for (i = 0; i < ext_hdr->eh_entries; i++) {
         fs_block_num = ((off64_t)extent_idx->ei_leaf_hi << 32) | extent_idx->ei_leaf_lo;
         read_block(fd, fs_block_num, block);
         tmp_ext_hdr = (struct ext4_extent_header *)block;

         if (tmp_ext_hdr->eh_depth == 0) {
             get_extent_ents(tmp_ext_hdr, block_list); /* leaf node, fill in block_list */
         } else {
             get_extent_idx(fd, tmp_ext_hdr, block_list); /* recurse down the tree */
         }
    }

    return 0;
}

static int get_block_list_extents(int fd, struct ext4_inode *inode, unsigned long long *block_list)
{
    struct ext4_extent_header *extent_hdr;

    extent_hdr = (struct ext4_extent_header *)inode->i_block;

    if (extent_hdr->eh_magic != EXT4_EXT_MAGIC) {
        critical_error("extent header has unexpected magic value 0x%4.4x\n",
                       extent_hdr->eh_magic);
    }

    if (extent_hdr->eh_depth == 0) {
         get_extent_ents((struct ext4_extent_header *)inode->i_block, block_list);
         return 0;
    }

    get_extent_idx(fd, (struct ext4_extent_header *)inode->i_block, block_list);

    return 0;
}

static int is_entry_dir(int fd, struct ext4_dir_entry_2 *dirp, int pass)
{
    struct ext4_inode inode;
    int ret = 0;

    if (dirp->file_type == EXT4_FT_DIR) {
        ret = 1;
    } else if (dirp->file_type == EXT4_FT_UNKNOWN) {
        /* Somebody was too lazy to fill in the dir entry,
         * so we have to go fetch it from the inode. Grrr.
         */
        /* if UPDATE_INODE_NUMS pass and the inode high bit is not
         * set return false so we don't recurse down the tree that is
         * already updated.  Otherwise, fetch inode, and return answer.
         */
        if ((pass == UPDATE_INODE_NUMS) && !(dirp->inode & 0x80000000)) {
            ret = 0;
        } else {
            read_inode(fd, (dirp->inode & 0x7fffffff), &inode);
            if (S_ISDIR(inode.i_mode)) {
                ret = 1;
            }
        }
    }

    return ret;
}

static int recurse_dir(int fd, struct ext4_inode *inode, char *dirbuf, int dirsize, int mode)
{
    unsigned long long *block_list;
    unsigned int num_blocks;
    struct ext4_dir_entry_2 *dirp, *prev_dirp = 0;
    char name[256];
    unsigned int i, leftover_space, is_dir;
    struct ext4_inode tmp_inode;
    int tmp_dirsize;
    char *tmp_dirbuf;

    switch (mode) {
        case SANITY_CHECK_PASS:
        case MARK_INODE_NUMS:
        case UPDATE_INODE_NUMS:
            break;
        default:
            critical_error("recurse_dir() called witn unknown mode!\n");
    }

    if (dirsize % info.block_size) {
        critical_error("dirsize %d not a multiple of block_size %d.  This is unexpected!\n",
                dirsize, info.block_size);
    }

    num_blocks = dirsize / info.block_size;

    block_list = malloc((num_blocks + 1) * sizeof(*block_list));
    if (block_list == 0) {
        critical_error("failed to allocate memory for block_list\n");
    }

    if (inode->i_flags & EXT4_EXTENTS_FL) {
        get_block_list_extents(fd, inode, block_list);
    } else {
        /* A directory that requires doubly or triply indirect blocks in huge indeed,
         * and will almost certainly not exist, especially since make_ext4fs only creates
         * directories with extents, and the kernel will too, but check to make sure the
         * directory is not that big and give an error if so.  Our limit is 12 direct blocks,
         * plus block_size/4 singly indirect blocks, which for a filesystem with 4K blocks
         * is a directory 1036 blocks long, or 4,243,456 bytes long!  Assuming an average
         * filename length of 20 (which I think is generous) thats 20 + 8 bytes overhead
         * per entry, or 151,552 entries in the directory!
         */
        if (num_blocks > (info.block_size / 4 + EXT4_NDIR_BLOCKS)) {
            critical_error("Non-extent based directory is too big!\n");
        }
        get_block_list_indirect(fd, inode, block_list);
    }

    /* Read in all the blocks for this directory */
    for (i = 0; i < num_blocks; i++) {
        read_block(fd, block_list[i], dirbuf + (i * info.block_size));
    }

    dirp = (struct ext4_dir_entry_2 *)dirbuf;
    while (dirp < (struct ext4_dir_entry_2 *)(dirbuf + dirsize)) {
        count++;
        leftover_space = (char *)(dirbuf + dirsize) - (char *)dirp;
        if (((mode == SANITY_CHECK_PASS) || (mode == UPDATE_INODE_NUMS)) &&
            (leftover_space <= 8) && prev_dirp) {
            /* This is a bug in an older version of make_ext4fs, where it
             * didn't properly include the rest of the block in rec_len.
             * Update rec_len on the previous entry to include the rest of
             * the block and exit the loop.
             */
            if (verbose) {
                printf("fixing up short rec_len for diretory entry for %s\n", name);
            }
            prev_dirp->rec_len += leftover_space;
            break;
        }

        if (dirp->inode == 0) {
            /* This is the last entry in the directory */
            break;
        }

        strncpy(name, dirp->name, dirp->name_len);
        name[dirp->name_len]='\0';

        /* Only recurse on pass UPDATE_INODE_NUMS if the high bit is set.
         * Otherwise, this inode entry has already been updated
         * and we'll do the wrong thing.  Also don't recurse on . or ..,
         * and certainly not on non-directories!
         */
        /* Hrm, looks like filesystems made by fastboot on stingray set the file_type
         * flag, but the lost+found directory has the type set to Unknown, which
         * seems to imply I need to read the inode and get it.
         */
        is_dir = is_entry_dir(fd, dirp, mode);
        if ( is_dir && (strcmp(name, ".") && strcmp(name, "..")) &&
            ((mode == SANITY_CHECK_PASS) || (mode == MARK_INODE_NUMS) ||
              ((mode == UPDATE_INODE_NUMS) && (dirp->inode & 0x80000000))) ) {
            /* A directory!  Recurse! */
            read_inode(fd, dirp->inode & 0x7fffffff, &tmp_inode);

            if (!S_ISDIR(tmp_inode.i_mode)) {
                critical_error("inode %d for name %s does not point to a directory\n",
                        dirp->inode & 0x7fffffff, name);
            }
            if (verbose) {
                printf("inode %d %s use extents\n", dirp->inode & 0x7fffffff,
                       (tmp_inode.i_flags & EXT4_EXTENTS_FL) ? "does" : "does not");
            }

            tmp_dirsize = tmp_inode.i_blocks_lo * INODE_BLOCK_SIZE;
            if (verbose) {
                printf("dir size = %d bytes\n", tmp_dirsize);
            }

            tmp_dirbuf = malloc(tmp_dirsize);
            if (tmp_dirbuf == 0) {
                critical_error("failed to allocate memory for tmp_dirbuf\n");
            }

            recurse_dir(fd, &tmp_inode, tmp_dirbuf, tmp_dirsize, mode);

            free(tmp_dirbuf);
        }

        if (verbose) {
            if (is_dir) {
                printf("Directory %s\n", name);
            } else {
                printf("Non-directory %s\n", name);
            }
        }

        /* Process entry based on current mode.  Either set high bit or change inode number */
        if (mode == MARK_INODE_NUMS) {
            dirp->inode |= 0x80000000;
        } else if (mode == UPDATE_INODE_NUMS) {
            if (dirp->inode & 0x80000000) {
                dirp->inode = compute_new_inum(dirp->inode & 0x7fffffff);
            }
        }

        if ((bail_phase == mode) && (bail_loc == 1) && (bail_count == count)) {
            critical_error("Bailing at phase %d, loc 1 and count %d\n", mode, count);
        }

        /* Point dirp at the next entry */
        prev_dirp = dirp;
        dirp = (struct ext4_dir_entry_2*)((char *)dirp + dirp->rec_len);
    }

    /* Write out all the blocks for this directory */
    for (i = 0; i < num_blocks; i++) {
        write_block(fd, block_list[i], dirbuf + (i * info.block_size));
        if ((bail_phase == mode) && (bail_loc == 2) && (bail_count <= count)) {
            critical_error("Bailing at phase %d, loc 2 and count %d\n", mode, count);
        }
    }

    free(block_list);

    return 0;
}

int ext4fixup(char *fsdev)
{
    return ext4fixup_internal(fsdev, 0, 0, 0, 0, 0);
}

int ext4fixup_internal(char *fsdev, int v_flag, int n_flag,
                       int stop_phase, int stop_loc, int stop_count)
{
    int fd;
    struct ext4_inode root_inode;
    unsigned int dirsize;
    char *dirbuf;

    if (setjmp(setjmp_env))
        return EXIT_FAILURE; /* Handle a call to longjmp() */

    verbose = v_flag;
    no_write = n_flag;

    bail_phase = stop_phase;
    bail_loc = stop_loc;
    bail_count = stop_count;

    fd = open(fsdev, O_RDWR);

    if (fd < 0)
        critical_error_errno("failed to open filesystem image");

    read_ext(fd, verbose);

    if (info.feat_incompat & EXT4_FEATURE_INCOMPAT_RECOVER) {
        critical_error("Filesystem needs recovery first, mount and unmount to do that\n");
    }

    /* Clear the low bit which is set while this tool is in progress.
     * If the tool crashes, it will still be set when we restart.
     * The low bit is set to make the filesystem unmountable while
     * it is being fixed up.  Also allow 0, which means the old ext2
     * size is in use.
     */
    if (((aux_info.sb->s_desc_size & ~1) != sizeof(struct ext2_group_desc)) &&
        ((aux_info.sb->s_desc_size & ~1) != 0))
        critical_error("error: bg_desc_size != sizeof(struct ext2_group_desc)\n");

    if ((info.feat_incompat & EXT4_FEATURE_INCOMPAT_FILETYPE) == 0) {
        critical_error("Expected filesystem to have filetype flag set\n");
    }

#if 0 // If we have to fix the directory rec_len issue, we can't use this check
    /* Check to see if the inodes/group is copacetic */
    if (info.inodes_per_blockgroup % (info.block_size/info.inode_size) == 0) {
             /* This filesystem has either already been updated, or was
              * made correctly.
              */
             if (verbose) {
                 printf("%s: filesystem correct, no work to do\n", me);
             }
             exit(0);
    }
#endif

    /* Compute what the new value of inodes_per_blockgroup will be when we're done */
    new_inodes_per_group=EXT4_ALIGN(info.inodes_per_group,(info.block_size/info.inode_size));

    read_inode(fd, EXT4_ROOT_INO, &root_inode);

    if (!S_ISDIR(root_inode.i_mode)) {
        critical_error("root inode %d does not point to a directory\n", EXT4_ROOT_INO);
    }
    if (verbose) {
        printf("inode %d %s use extents\n", EXT4_ROOT_INO,
               (root_inode.i_flags & EXT4_EXTENTS_FL) ? "does" : "does not");
    }

    dirsize = root_inode.i_blocks_lo * INODE_BLOCK_SIZE;
    if (verbose) {
        printf("root dir size = %d bytes\n", dirsize);
    }

    dirbuf = malloc(dirsize);
    if (dirbuf == 0) {
        critical_error("failed to allocate memory for dirbuf\n");
    }

    /* Perform a sanity check pass first, try to catch any errors that will occur
     * before we actually change anything, so we don't leave a filesystem in a
     * corrupted, unrecoverable state.  Set no_write, make it quiet, and do a recurse
     * pass and a update_superblock pass.  Set flags back to requested state when done.
     * Only perform sanity check if the state is unset.  If the state is _NOT_ unset,
     * then the tool has already been run and interrupted, and it presumably ran and
     * passed sanity checked before it got interrupted.  It is _NOT_ safe to run sanity
     * check if state is unset because it assumes inodes are to be computed using the
     * old inodes/group, but some inode numbers may be updated to the new number.
     */
    if (get_fs_fixup_state(fd) == STATE_UNSET) {
        verbose = 0;
        no_write = 1;
        recurse_dir(fd, &root_inode, dirbuf, dirsize, SANITY_CHECK_PASS);
        update_superblocks_and_bg_desc(fd, STATE_UNSET);
        verbose = v_flag;
        no_write = n_flag;

        set_fs_fixup_state(fd, STATE_MARKING_INUMS);
    }

    if (get_fs_fixup_state(fd) == STATE_MARKING_INUMS) {
        count = 0; /* Reset debugging counter */
        if (!recurse_dir(fd, &root_inode, dirbuf, dirsize, MARK_INODE_NUMS)) {
            set_fs_fixup_state(fd, STATE_UPDATING_INUMS);
        }
    }

    if (get_fs_fixup_state(fd) == STATE_UPDATING_INUMS) {
        count = 0; /* Reset debugging counter */
        if (!recurse_dir(fd, &root_inode, dirbuf, dirsize, UPDATE_INODE_NUMS)) {
            set_fs_fixup_state(fd, STATE_UPDATING_SB);
        }
    }

    if (get_fs_fixup_state(fd) == STATE_UPDATING_SB) {
        /* set the new inodes/blockgroup number,
         * and sets the state back to 0.
         */
        if (!update_superblocks_and_bg_desc(fd, STATE_UPDATING_SB)) {
            set_fs_fixup_state(fd, STATE_UNSET);
        }
    }

    close(fd);
    free(dirbuf);

    return 0;
}
