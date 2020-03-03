/*
 * Unsquash a squashfs filesystem.  This is a highly compressed read only filesystem.
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 * Phillip Lougher <phillip@lougher.demon.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * unsquash-4.c
 */

#include "unsquashfs.h"
#include "squashfs_swap.h"
#include "read_fs.h"

static struct squashfs_fragment_entry *fragment_table;
static unsigned int *id_table;

int read_fragment_table_4()
{
	int res, i, indexes = SQUASHFS_FRAGMENT_INDEXES(sBlk.fragments);
	squashfs_fragment_index fragment_table_index[indexes];

	TRACE("read_fragment_table: %d fragments, reading %d fragment indexes "
		"from 0x%llx\n", sBlk.fragments, indexes,
		sBlk.fragment_table_start);

	if(sBlk.fragments == 0)
		return;

	if((fragment_table = malloc(sBlk.fragments *
			sizeof(squashfs_fragment_entry))) == NULL)
		EXIT_UNSQUASH("read_fragment_table: failed to allocate "
			"fragment table\n");

	res = read_bytes(sBlk.fragment_table_start,
		SQUASHFS_FRAGMENT_INDEX_BYTES(sBlk.fragments),
		(char *) fragment_table_index);
	if(res == FALSE) {
		ERROR("read_fragment_table: failed to read fragment table "
			"index\n");
		return FALSE;
	}
	SQUASHFS_INSWAP_FRAGMENT_INDEXES(fragment_table_index, indexes);

	for(i = 0; i < indexes; i++) {
		int length = read_block(fragment_table_index[i], NULL,
			((char *) fragment_table) + (i *
			SQUASHFS_METADATA_SIZE));
		TRACE("Read fragment table block %d, from 0x%llx, length %d\n",
			i, fragment_table_index[i], length);
		if(length == FALSE) {
			ERROR("read_fragment_table: failed to read fragment "
				"table index\n");
			return FALSE;
		}
	}

	for(i = 0; i < sBlk.fragments; i++) 
		SQUASHFS_INSWAP_FRAGMENT_ENTRY(&fragment_table[i]);

	return TRUE;
}


void read_fragment_4(unsigned int fragment, long long *start_block, int *size)
{
	TRACE("read_fragment: reading fragment %d\n", fragment);

	squashfs_fragment_entry *fragment_entry = &fragment_table[fragment];
	*start_block = fragment_entry->start_block;
	*size = fragment_entry->size;
}


struct inode *read_inode_4(unsigned int start_block, unsigned int offset)
{
	static squashfs_inode_header header;
	long long start = sBlk.inode_table_start + start_block;
	int bytes = lookup_entry(inode_table_hash, start);
	char *block_ptr = inode_table + bytes + offset;
	static struct inode i;

	TRACE("read_inode: reading inode [%d:%d]\n", start_block,  offset);

	if(bytes == -1) {
		ERROR("read_inode: inode table block %lld not found\n", start); 		
		return NULL;
	}

	SQUASHFS_SWAP_BASE_INODE_HEADER(&header.base,
		(squashfs_base_inode_header *) block_ptr);

	i.uid = (uid_t) id_table[header.base.uid];
	i.gid = (uid_t) id_table[header.base.guid];
	i.mode = lookup_type[header.base.inode_type] | header.base.mode;
	i.type = header.base.inode_type;
	i.time = header.base.mtime;
	i.inode_number = header.base.inode_number;

	switch(header.base.inode_type) {
		case SQUASHFS_DIR_TYPE: {
			squashfs_dir_inode_header *inode = &header.dir;

			SQUASHFS_SWAP_DIR_INODE_HEADER(inode,
				(squashfs_dir_inode_header *) block_ptr);

			i.data = inode->file_size;
			i.offset = inode->offset;
			i.start = inode->start_block;
			break;
		}
		case SQUASHFS_LDIR_TYPE: {
			squashfs_ldir_inode_header *inode = &header.ldir;

			SQUASHFS_SWAP_LDIR_INODE_HEADER(inode,
				(squashfs_ldir_inode_header *) block_ptr);

			i.data = inode->file_size;
			i.offset = inode->offset;
			i.start = inode->start_block;
			break;
		}
		case SQUASHFS_FILE_TYPE: {
			squashfs_reg_inode_header *inode = &header.reg;

			SQUASHFS_SWAP_REG_INODE_HEADER(inode,
				(squashfs_reg_inode_header *) block_ptr);

			i.data = inode->file_size;
			i.frag_bytes = inode->fragment == SQUASHFS_INVALID_FRAG
				?  0 : inode->file_size % sBlk.block_size;
			i.fragment = inode->fragment;
			i.offset = inode->offset;
			i.blocks = inode->fragment == SQUASHFS_INVALID_FRAG ?
				(inode->file_size + sBlk.block_size - 1) >>
				sBlk.block_log :
				inode->file_size >> sBlk.block_log;
			i.start = inode->start_block;
			i.sparse = 0;
			i.block_ptr = block_ptr + sizeof(*inode);
			break;
		}	
		case SQUASHFS_LREG_TYPE: {
			squashfs_lreg_inode_header *inode = &header.lreg;

			SQUASHFS_SWAP_LREG_INODE_HEADER(inode,
				(squashfs_lreg_inode_header *) block_ptr);

			i.data = inode->file_size;
			i.frag_bytes = inode->fragment == SQUASHFS_INVALID_FRAG
				?  0 : inode->file_size % sBlk.block_size;
			i.fragment = inode->fragment;
			i.offset = inode->offset;
			i.blocks = inode->fragment == SQUASHFS_INVALID_FRAG ?
				(inode->file_size + sBlk.block_size - 1) >>
				sBlk.block_log :
				inode->file_size >> sBlk.block_log;
			i.start = inode->start_block;
			i.sparse = inode->sparse != 0;
			i.block_ptr = block_ptr + sizeof(*inode);
			break;
		}	
		case SQUASHFS_SYMLINK_TYPE:
		case SQUASHFS_LSYMLINK_TYPE: {
			squashfs_symlink_inode_header *inode = &header.symlink;

			SQUASHFS_SWAP_SYMLINK_INODE_HEADER(inode,
				(squashfs_symlink_inode_header *) block_ptr);

			i.symlink = malloc(inode->symlink_size + 1);
			if(i.symlink == NULL)
				EXIT_UNSQUASH("read_inode: failed to malloc "
					"symlink data\n");
			strncpy(i.symlink, block_ptr +
				sizeof(squashfs_symlink_inode_header),
				inode->symlink_size);
			i.symlink[inode->symlink_size] = '\0';
			i.data = inode->symlink_size;
			break;
		}
 		case SQUASHFS_BLKDEV_TYPE:
	 	case SQUASHFS_CHRDEV_TYPE:
 		case SQUASHFS_LBLKDEV_TYPE:
	 	case SQUASHFS_LCHRDEV_TYPE: {
			squashfs_dev_inode_header *inode = &header.dev;

			SQUASHFS_SWAP_DEV_INODE_HEADER(inode,
				(squashfs_dev_inode_header *) block_ptr);

			i.data = inode->rdev;
			break;
		}
		case SQUASHFS_FIFO_TYPE:
		case SQUASHFS_SOCKET_TYPE:
		case SQUASHFS_LFIFO_TYPE:
		case SQUASHFS_LSOCKET_TYPE:
			i.data = 0;
			break;
		default:
			ERROR("Unknown inode type %d in read_inode!\n",
				header.base.inode_type);
			return NULL;
	}
	return &i;
}


struct dir *squashfs_opendir_4(unsigned int block_start, unsigned int offset,
	struct inode **i)
{
	squashfs_dir_header dirh;
	char buffer[sizeof(squashfs_dir_entry) + SQUASHFS_NAME_LEN + 1];
	squashfs_dir_entry *dire = (squashfs_dir_entry *) buffer;
	long long start;
	int bytes;
	int dir_count, size;
	struct dir_ent *new_dir;
	struct dir *dir;

	TRACE("squashfs_opendir: inode start block %d, offset %d\n",
		block_start, offset);

	if((*i = s_ops.read_inode(block_start, offset)) == NULL) {
		ERROR("squashfs_opendir: failed to read directory inode %d\n",
			block_start);
		return NULL;
	}

	start = sBlk.directory_table_start + (*i)->start;
	bytes = lookup_entry(directory_table_hash, start);

	if(bytes == -1) {
		ERROR("squashfs_opendir: directory block %d not found!\n",
			block_start);
		return NULL;
	}

	bytes += (*i)->offset;
	size = (*i)->data + bytes - 3;

	if((dir = malloc(sizeof(struct dir))) == NULL) {
		ERROR("squashfs_opendir: malloc failed!\n");
		return NULL;
	}

	dir->dir_count = 0;
	dir->cur_entry = 0;
	dir->mode = (*i)->mode;
	dir->uid = (*i)->uid;
	dir->guid = (*i)->gid;
	dir->mtime = (*i)->time;
	dir->dirs = NULL;

	while(bytes < size) {			
		SQUASHFS_SWAP_DIR_HEADER(&dirh, (squashfs_dir_header *)
			(directory_table + bytes));
	
		dir_count = dirh.count + 1;
		TRACE("squashfs_opendir: Read directory header @ byte position "
			"%d, %d directory entries\n", bytes, dir_count);
		bytes += sizeof(dirh);

		while(dir_count--) {
			SQUASHFS_SWAP_DIR_ENTRY(dire, (squashfs_dir_entry *)
				(directory_table + bytes));

			bytes += sizeof(*dire);

			memcpy(dire->name, directory_table + bytes,
				dire->size + 1);
			dire->name[dire->size + 1] = '\0';
			TRACE("squashfs_opendir: directory entry %s, inode "
				"%d:%d, type %d\n", dire->name,
				dirh.start_block, dire->offset, dire->type);
			if((dir->dir_count % DIR_ENT_SIZE) == 0) {
				new_dir = realloc(dir->dirs, (dir->dir_count +
					DIR_ENT_SIZE) * sizeof(struct dir_ent));
				if(new_dir == NULL) {
					ERROR("squashfs_opendir: realloc "
						"failed!\n");
					free(dir->dirs);
					free(dir);
					return NULL;
				}
				dir->dirs = new_dir;
			}
			strcpy(dir->dirs[dir->dir_count].name, dire->name);
			dir->dirs[dir->dir_count].start_block =
				dirh.start_block;
			dir->dirs[dir->dir_count].offset = dire->offset;
			dir->dirs[dir->dir_count].type = dire->type;
			dir->dir_count ++;
			bytes += dire->size + 1;
		}
	}

	return dir;
}


int read_uids_guids_4()
{
	int res, i, indexes = SQUASHFS_ID_BLOCKS(sBlk.no_ids);
	long long id_index_table[indexes];

	TRACE("read_uids_guids: no_ids %d\n", sBlk.no_ids);

	id_table = malloc(SQUASHFS_ID_BYTES(sBlk.no_ids));
	if(id_table == NULL) {
		ERROR("read_uids_guids: failed to allocate id table\n");
		return FALSE;
	}

	res = read_bytes(sBlk.id_table_start,
		SQUASHFS_ID_BLOCK_BYTES(sBlk.no_ids), (char *) id_index_table);
	if(res == FALSE) {
		ERROR("read_uids_guids: failed to read id index table\n");
		return FALSE;
	}
	SQUASHFS_INSWAP_ID_BLOCKS(id_index_table, indexes);

	for(i = 0; i < indexes; i++) {
		res = read_block(id_index_table[i], NULL,
			((char *) id_table) + i * SQUASHFS_METADATA_SIZE);
		if(res == FALSE) {
			ERROR("read_uids_guids: failed to read id table block"
				"\n");
			return FALSE;
		}
	}

	SQUASHFS_INSWAP_INTS(id_table, sBlk.no_ids);

	return TRUE;
}
