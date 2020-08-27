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
 * unsquashfs.c
 */

#include <sys/sysmacros.h>
#include "unsquashfs.h"
#include "squashfs_swap.h"
#include "squashfs_compat.h"
#include "read_fs.h"

struct cache *fragment_cache, *data_cache;
struct queue *to_reader, *to_deflate, *to_writer, *from_writer;
pthread_t *thread, *deflator_thread;
pthread_mutex_t	fragment_mutex;

/* user options that control parallelisation */
int processors = -1;

struct super_block sBlk;
squashfs_operations s_ops;

int bytes = 0, swap, file_count = 0, dir_count = 0, sym_count = 0,
	dev_count = 0, fifo_count = 0;
char *inode_table = NULL, *directory_table = NULL;
struct hash_table_entry *inode_table_hash[65536], *directory_table_hash[65536];
int fd;
unsigned int *uid_table, *guid_table;
unsigned int cached_frag = SQUASHFS_INVALID_FRAG;
char *fragment_data;
char *file_data;
char *data;
unsigned int block_size;
unsigned int block_log;
int lsonly = FALSE, info = FALSE, force = FALSE, short_ls = TRUE;
int use_regex = FALSE;
char **created_inode;
int root_process;
#ifdef SQUASHFS_LZMA_ENABLE
struct sqlzma_un un;
#endif
int columns;
int rotate = 0;
pthread_mutex_t	screen_mutex;
pthread_cond_t progress_wait;
int progress = TRUE, progress_enabled = FALSE;
unsigned int total_blocks = 0, total_files = 0, total_inodes = 0;
unsigned int cur_blocks = 0;
int inode_number = 1;

int lookup_type[] = {
	0,
	S_IFDIR,
	S_IFREG,
	S_IFLNK,
	S_IFBLK,
	S_IFCHR,
	S_IFIFO,
	S_IFSOCK,
	S_IFDIR,
	S_IFREG,
	S_IFLNK,
	S_IFBLK,
	S_IFCHR,
	S_IFIFO,
	S_IFSOCK
};

struct test table[] = {
	{ S_IFMT, S_IFSOCK, 0, 's' },
	{ S_IFMT, S_IFLNK, 0, 'l' },
	{ S_IFMT, S_IFBLK, 0, 'b' },
	{ S_IFMT, S_IFDIR, 0, 'd' },
	{ S_IFMT, S_IFCHR, 0, 'c' },
	{ S_IFMT, S_IFIFO, 0, 'p' },
	{ S_IRUSR, S_IRUSR, 1, 'r' },
	{ S_IWUSR, S_IWUSR, 2, 'w' },
	{ S_IRGRP, S_IRGRP, 4, 'r' },
	{ S_IWGRP, S_IWGRP, 5, 'w' },
	{ S_IROTH, S_IROTH, 7, 'r' },
	{ S_IWOTH, S_IWOTH, 8, 'w' },
	{ S_IXUSR | S_ISUID, S_IXUSR | S_ISUID, 3, 's' },
	{ S_IXUSR | S_ISUID, S_ISUID, 3, 'S' },
	{ S_IXUSR | S_ISUID, S_IXUSR, 3, 'x' },
	{ S_IXGRP | S_ISGID, S_IXGRP | S_ISGID, 6, 's' },
	{ S_IXGRP | S_ISGID, S_ISGID, 6, 'S' },
	{ S_IXGRP | S_ISGID, S_IXGRP, 6, 'x' },
	{ S_IXOTH | S_ISVTX, S_IXOTH | S_ISVTX, 9, 't' },
	{ S_IXOTH | S_ISVTX, S_ISVTX, 9, 'T' },
	{ S_IXOTH | S_ISVTX, S_IXOTH, 9, 'x' },
	{ 0, 0, 0, 0}
};

void progress_bar(long long current, long long max, int columns);
void update_progress_bar();

void sigwinch_handler()
{
	struct winsize winsize;

	if(ioctl(1, TIOCGWINSZ, &winsize) == -1) {
		if(isatty(STDOUT_FILENO))
			ERROR("TIOCGWINSZ ioctl failed, defaulting to 80 "
				"columns\n");
		columns = 80;
	} else
		columns = winsize.ws_col;
}


void sigalrm_handler()
{
	rotate = (rotate + 1) % 4;
}


struct queue *queue_init(int size)
{
	struct queue *queue = malloc(sizeof(struct queue));

	if(queue == NULL)
		return NULL;

	if((queue->data = malloc(sizeof(void *) * (size + 1))) == NULL) {
		free(queue);
		return NULL;
	}

	queue->size = size + 1;
	queue->readp = queue->writep = 0;
	pthread_mutex_init(&queue->mutex, NULL);
	pthread_cond_init(&queue->empty, NULL);
	pthread_cond_init(&queue->full, NULL);

	return queue;
}


void queue_put(struct queue *queue, void *data)
{
	int nextp;

	pthread_mutex_lock(&queue->mutex);

	while((nextp = (queue->writep + 1) % queue->size) == queue->readp)
		pthread_cond_wait(&queue->full, &queue->mutex);

	queue->data[queue->writep] = data;
	queue->writep = nextp;
	pthread_cond_signal(&queue->empty);
	pthread_mutex_unlock(&queue->mutex);
}


void *queue_get(struct queue *queue)
{
	void *data;
	pthread_mutex_lock(&queue->mutex);

	while(queue->readp == queue->writep)
		pthread_cond_wait(&queue->empty, &queue->mutex);

	data = queue->data[queue->readp];
	queue->readp = (queue->readp + 1) % queue->size;
	pthread_cond_signal(&queue->full);
	pthread_mutex_unlock(&queue->mutex);

	return data;
}


/* Called with the cache mutex held */
void insert_hash_table(struct cache *cache, struct cache_entry *entry)
{
	int hash = CALCULATE_HASH(entry->block);

	entry->hash_next = cache->hash_table[hash];
	cache->hash_table[hash] = entry;
	entry->hash_prev = NULL;
	if(entry->hash_next)
		entry->hash_next->hash_prev = entry;
}


/* Called with the cache mutex held */
void remove_hash_table(struct cache *cache, struct cache_entry *entry)
{
	if(entry->hash_prev)
		entry->hash_prev->hash_next = entry->hash_next;
	else
		cache->hash_table[CALCULATE_HASH(entry->block)] =
			entry->hash_next;
	if(entry->hash_next)
		entry->hash_next->hash_prev = entry->hash_prev;

	entry->hash_prev = entry->hash_next = NULL;
}


/* Called with the cache mutex held */
void insert_free_list(struct cache *cache, struct cache_entry *entry)
{
	if(cache->free_list) {
		entry->free_next = cache->free_list;
		entry->free_prev = cache->free_list->free_prev;
		cache->free_list->free_prev->free_next = entry;
		cache->free_list->free_prev = entry;
	} else {
		cache->free_list = entry;
		entry->free_prev = entry->free_next = entry;
	}
}


/* Called with the cache mutex held */
void remove_free_list(struct cache *cache, struct cache_entry *entry)
{
	if(entry->free_prev == NULL && entry->free_next == NULL)
		/* not in free list */
		return;
	else if(entry->free_prev == entry && entry->free_next == entry) {
		/* only this entry in the free list */
		cache->free_list = NULL;
	} else {
		/* more than one entry in the free list */
		entry->free_next->free_prev = entry->free_prev;
		entry->free_prev->free_next = entry->free_next;
		if(cache->free_list == entry)
			cache->free_list = entry->free_next;
	}

	entry->free_prev = entry->free_next = NULL;
}


struct cache *cache_init(int buffer_size, int max_buffers)
{
	struct cache *cache = malloc(sizeof(struct cache));

	if(cache == NULL)
		return NULL;

	cache->max_buffers = max_buffers;
	cache->buffer_size = buffer_size;
	cache->count = 0;
	cache->free_list = NULL;
	memset(cache->hash_table, 0, sizeof(struct cache_entry *) * 65536);
	cache->wait_free = FALSE;
	cache->wait_pending = FALSE;
	pthread_mutex_init(&cache->mutex, NULL);
	pthread_cond_init(&cache->wait_for_free, NULL);
	pthread_cond_init(&cache->wait_for_pending, NULL);

	return cache;
}


struct cache_entry *cache_get(struct cache *cache, long long block, int size)
{
	/*
	 * Get a block out of the cache.  If the block isn't in the cache
 	 * it is added and queued to the reader() and deflate() threads for
 	 * reading off disk and decompression.  The cache grows until max_blocks
 	 * is reached, once this occurs existing discarded blocks on the free
 	 * list are reused
 	 */
	int hash = CALCULATE_HASH(block);
	struct cache_entry *entry;

	pthread_mutex_lock(&cache->mutex);

	for(entry = cache->hash_table[hash]; entry; entry = entry->hash_next)
		if(entry->block == block)
			break;

	if(entry) {
		/*
 		 *found the block in the cache, increment used count and
 		 * if necessary remove from free list so it won't disappear
 		 */
		entry->used ++;
		remove_free_list(cache, entry);
		pthread_mutex_unlock(&cache->mutex);
	} else {
		/*
 		 * not in the cache
		 *
		 * first try to allocate new block
		 */
		if(cache->count < cache->max_buffers) {
			entry = malloc(sizeof(struct cache_entry));
			if(entry == NULL)
				goto failed;
			entry->data = malloc(cache->buffer_size);
			if(entry->data == NULL) {
				free(entry);
				goto failed;
			}
			entry->cache = cache;
			entry->free_prev = entry->free_next = NULL;
			cache->count ++;
		} else {
			/*
			 * try to get from free list
			 */
			while(cache->free_list == NULL) {
				cache->wait_free = TRUE;
				pthread_cond_wait(&cache->wait_for_free,
					&cache->mutex);
			}
			entry = cache->free_list;
			remove_free_list(cache, entry);
			remove_hash_table(cache, entry);
		}

		/*
		 * initialise block and insert into the hash table
		 */
		entry->block = block;
		entry->size = size;
		entry->used = 1;
		entry->error = FALSE;
		entry->pending = TRUE;
		insert_hash_table(cache, entry);

		/*
		 * queue to read thread to read and ultimately (via the
		 * decompress threads) decompress the buffer
 		 */
		pthread_mutex_unlock(&cache->mutex);
		queue_put(to_reader, entry);
	}

	return entry;

failed:
	pthread_mutex_unlock(&cache->mutex);
	return NULL;
}

	
void cache_block_ready(struct cache_entry *entry, int error)
{
	/*
	 * mark cache entry as being complete, reading and (if necessary)
 	 * decompression has taken place, and the buffer is valid for use.
 	 * If an error occurs reading or decompressing, the buffer also 
 	 * becomes ready but with an error...
 	 */
	pthread_mutex_lock(&entry->cache->mutex);
	entry->pending = FALSE;
	entry->error = error;

	/*
	 * if the wait_pending flag is set, one or more threads may be waiting
	 * on this buffer
	 */
	if(entry->cache->wait_pending) {
		entry->cache->wait_pending = FALSE;
		pthread_cond_broadcast(&entry->cache->wait_for_pending);
	}

	pthread_mutex_unlock(&entry->cache->mutex);
}


void cache_block_wait(struct cache_entry *entry)
{
	/*
	 * wait for this cache entry to become ready, when reading and (if
	 * necessary) decompression has taken place
	 */
	pthread_mutex_lock(&entry->cache->mutex);

	while(entry->pending) {
		entry->cache->wait_pending = TRUE;
		pthread_cond_wait(&entry->cache->wait_for_pending,
			&entry->cache->mutex);
	}

	pthread_mutex_unlock(&entry->cache->mutex);
}


void cache_block_put(struct cache_entry *entry)
{
	/*
	 * finished with this cache entry, once the usage count reaches zero it
 	 * can be reused and is put onto the free list.  As it remains
 	 * accessible via the hash table it can be found getting a new lease of
 	 * life before it is reused.
 	 */
	pthread_mutex_lock(&entry->cache->mutex);

	entry->used --;
	if(entry->used == 0) {
		insert_free_list(entry->cache, entry);

		/*
		 * if the wait_free flag is set, one or more threads may be
		 * waiting on this buffer
		 */
		if(entry->cache->wait_free) {
			entry->cache->wait_free = FALSE;
			pthread_cond_broadcast(&entry->cache->wait_for_free);
		}
	}

	pthread_mutex_unlock(&entry->cache->mutex);
}


char *modestr(char *str, int mode)
{
	int i;

	strcpy(str, "----------");

	for(i = 0; table[i].mask != 0; i++) {
		if((mode & table[i].mask) == table[i].value)
			str[table[i].position] = table[i].mode;
	}

	return str;
}


#define TOTALCHARS  25
int print_filename(char *pathname, struct inode *inode)
{
	char str[11], dummy[100], dummy2[100], *userstr, *groupstr;
	int padchars;
	struct passwd *user;
	struct group *group;
	struct tm *t;

	if(short_ls) {
		printf("%s\n", pathname);
		return 1;
	}

	if((user = getpwuid(inode->uid)) == NULL) {
		sprintf(dummy, "%d", inode->uid);
		userstr = dummy;
	} else
		userstr = user->pw_name;
		 
	if((group = getgrgid(inode->gid)) == NULL) {
		sprintf(dummy2, "%d", inode->gid);
		groupstr = dummy2;
	} else
		groupstr = group->gr_name;

	printf("%s %s/%s ", modestr(str, inode->mode), userstr, groupstr);

	switch(inode->mode & S_IFMT) {
		case S_IFREG:
		case S_IFDIR:
		case S_IFSOCK:
		case S_IFIFO:
		case S_IFLNK:
			padchars = TOTALCHARS - strlen(userstr) -
				strlen(groupstr);

			printf("%*lld ", padchars > 0 ? padchars : 0,
				inode->data);
			break;
		case S_IFCHR:
		case S_IFBLK:
			padchars = TOTALCHARS - strlen(userstr) -
				strlen(groupstr) - 7; 

			printf("%*s%3d,%3d ", padchars > 0 ? padchars : 0, " ",
				(int) inode->data >> 8, (int) inode->data &
				0xff);
			break;
	}

	t = localtime(&inode->time);

	printf("%d-%02d-%02d %02d:%02d %s", t->tm_year + 1900, t->tm_mon + 1,
		t->tm_mday, t->tm_hour, t->tm_min, pathname);
	if((inode->mode & S_IFMT) == S_IFLNK)
		printf(" -> %s", inode->symlink);
	printf("\n");
		
	return 1;
}
	

int add_entry(struct hash_table_entry *hash_table[], long long start, int bytes)
{
	int hash = CALCULATE_HASH(start);
	struct hash_table_entry *hash_table_entry;

	if((hash_table_entry = malloc(sizeof(struct hash_table_entry))) == NULL) {
		ERROR("add_hash: out of memory in malloc\n");
		return FALSE;
	}

	hash_table_entry->start = start;
	hash_table_entry->bytes = bytes;
	hash_table_entry->next = hash_table[hash];
	hash_table[hash] = hash_table_entry;

	return TRUE;
}


int lookup_entry(struct hash_table_entry *hash_table[], long long start)
{
	int hash = CALCULATE_HASH(start);
	struct hash_table_entry *hash_table_entry;

	for(hash_table_entry = hash_table[hash]; hash_table_entry;
				hash_table_entry = hash_table_entry->next)

		if(hash_table_entry->start == start)
			return hash_table_entry->bytes;

	return -1;
}


int read_bytes(long long byte, int bytes, char *buff)
{
	off_t off = byte;
	int res, count;

	TRACE("read_bytes: reading from position 0x%llx, bytes %d\n", byte,
		bytes);

	if(lseek(fd, off, SEEK_SET) == -1) {
		ERROR("Lseek failed because %s\n", strerror(errno));
		return FALSE;
	}

	for(count = 0; count < bytes; count += res) {
		res = read(fd, buff + count, bytes - count);
		if(res < 1) {
			if(res == 0) {
				ERROR("Read on filesystem failed because EOF\n");
				return FALSE;
			} else if(errno != EINTR) {
				ERROR("Read on filesystem failed because %s\n",
						strerror(errno));
				return FALSE;
			} else
				res = 0;
		}
	}

	return TRUE;
}


int read_block(long long start, long long *next, char *block)
{
	unsigned short c_byte;
	int offset = 2;
	
	if(swap) {
		if(read_bytes(start, 2, block) == FALSE)
			goto failed;
		((unsigned char *) &c_byte)[1] = block[0];
		((unsigned char *) &c_byte)[0] = block[1]; 
	} else 
		if(read_bytes(start, 2, (char *)&c_byte) == FALSE)
			goto failed;

	TRACE("read_block: block @0x%llx, %d %s bytes\n", start,
		SQUASHFS_COMPRESSED_SIZE(c_byte), SQUASHFS_COMPRESSED(c_byte) ?
		"compressed" : "uncompressed");

	if(SQUASHFS_CHECK_DATA(sBlk.flags))
		offset = 3;
	if(SQUASHFS_COMPRESSED(c_byte)) {
		char buffer[SQUASHFS_METADATA_SIZE];
		int res;
		unsigned long bytes = SQUASHFS_METADATA_SIZE;
		
#ifdef SQUASHFS_LZMA_ENABLE
		enum {Src, Dst};
		struct sized_buf sbuf[] = {
			{.buf = buffer},
			{.buf = block, .sz = bytes}
		};
#endif

		c_byte = SQUASHFS_COMPRESSED_SIZE(c_byte);
		if(read_bytes(start + offset, c_byte, buffer) == FALSE)
			goto failed;

#ifdef SQUASHFS_LZMA_ENABLE
		sbuf[Src].sz = c_byte;
		res = sqlzma_un(&un, sbuf + Src, sbuf + Dst);
		if (res)
			abort();
		bytes = un.un_reslen;
#else
		res = uncompress((unsigned char *) block, &bytes,
			(const unsigned char *) buffer, c_byte);

		if(res != Z_OK) {
			if(res == Z_MEM_ERROR)
				ERROR("zlib::uncompress failed, not enough "
					"memory\n");
			else if(res == Z_BUF_ERROR)
				ERROR("zlib::uncompress failed, not enough "
					"room in output buffer\n");
			else
				ERROR("zlib::uncompress failed, unknown error "
					"%d\n", res);
			goto failed;
		}
#endif		
		if(next)
			*next = start + offset + c_byte;
		return bytes;
	} else {
		c_byte = SQUASHFS_COMPRESSED_SIZE(c_byte);
		if(read_bytes(start + offset, c_byte, block) == FALSE)
			goto failed;
		if(next)
			*next = start + offset + c_byte;
		return c_byte;
	}

failed:
	ERROR("read_block: failed to read block @0x%llx\n", start);
	return FALSE;
}


int read_data_block(long long start, unsigned int size, char *block)
{
	int res;
	unsigned long bytes = block_size;
	int c_byte = SQUASHFS_COMPRESSED_SIZE_BLOCK(size);

	TRACE("read_data_block: block @0x%llx, %d %s bytes\n", start,
		SQUASHFS_COMPRESSED_SIZE_BLOCK(c_byte),
		SQUASHFS_COMPRESSED_BLOCK(c_byte) ? "compressed" :
		"uncompressed");

	if(SQUASHFS_COMPRESSED_BLOCK(size)) {
#ifdef SQUASHFS_LZMA_ENABLE
		enum {Src, Dst};
		struct sized_buf sbuf[] = {
			{.buf = data},
			{.buf = block, .sz = bytes}
		};
#endif
		
		if(read_bytes(start, c_byte, data) == FALSE)
			goto failed;
#ifdef SQUASHFS_LZMA_ENABLE
		res = sqlzma_un(&un, sbuf + Src, sbuf + Dst);
		if (res)
			abort();
		bytes = un.un_reslen;
#else
		res = uncompress((unsigned char *) block, &bytes,
			(const unsigned char *) data, c_byte);

		if(res != Z_OK) {
			if(res == Z_MEM_ERROR)
				ERROR("zlib::uncompress failed, not enough "
					"memory\n");
			else if(res == Z_BUF_ERROR)
				ERROR("zlib::uncompress failed, not enough "
					"room in output buffer\n");
			else
				ERROR("zlib::uncompress failed, unknown error "
					"%d\n", res);
			goto failed;
		}
#endif
		return bytes;
	} else {
		if(read_bytes(start, c_byte, block) == FALSE)
			goto failed;

		return c_byte;
	}

failed:
	ERROR("read_data_block: failed to read block @0x%llx, size %d\n", start,
		size);
	return FALSE;
}


void uncompress_inode_table(long long start, long long end)
{
	int size = 0, bytes = 0, res;

	TRACE("uncompress_inode_table: start %lld, end %lld\n", start, end);
	while(start < end) {
		if((size - bytes < SQUASHFS_METADATA_SIZE) &&
				((inode_table = realloc(inode_table, size +=
				SQUASHFS_METADATA_SIZE)) == NULL))
			EXIT_UNSQUASH("uncompress_inode_table: out of memory "
				"in realloc\n");
		TRACE("uncompress_inode_table: reading block 0x%llx\n", start);
		add_entry(inode_table_hash, start, bytes);
		res = read_block(start, &start, inode_table + bytes);
		if(res == 0) {
			free(inode_table);
			EXIT_UNSQUASH("uncompress_inode_table: failed to read "
				"block \n");
		}
		bytes += res;
	}
}


int set_attributes(char *pathname, int mode, uid_t uid, gid_t guid, time_t time,
	unsigned int set_mode)
{
	struct utimbuf times = { time, time };

	if(utime(pathname, &times) == -1) {
		ERROR("set_attributes: failed to set time on %s, because %s\n",
			pathname, strerror(errno));
		return FALSE;
	}

	if(root_process) {
		if(chown(pathname, uid, guid) == -1) {
			ERROR("set_attributes: failed to change uid and gids "
				"on %s, because %s\n", pathname,
				strerror(errno));
			return FALSE;
		}
	} else
		mode &= ~07000;

	if((set_mode || (mode & 07000)) && chmod(pathname, (mode_t) mode) == -1) {
		ERROR("set_attributes: failed to change mode %s, because %s\n",
			pathname, strerror(errno));
		return FALSE;
	}

	return TRUE;
}


int write_bytes(int fd, char *buff, int bytes)
{
	int res, count;

	for(count = 0; count < bytes; count += res) {
		res = write(fd, buff + count, bytes - count);
		if(res == -1) {
			if(errno != EINTR) {
				ERROR("Write on output file failed because "
					"%s\n", strerror(errno));
				return -1;
			}
			res = 0;
		}
	}

	return 0;
}


int lseek_broken = FALSE;
char *zero_data = NULL;

int write_block(int file_fd, char *buffer, int size, int hole, int sparse)
{
	off_t off = hole;

	if(hole) {
		if(sparse && lseek_broken == FALSE) {
			 int error = lseek(file_fd, off, SEEK_CUR);
			 if(error == -1)
				/* failed to seek beyond end of file */
				lseek_broken = TRUE;
		}

		if((sparse == FALSE || lseek_broken) && zero_data == NULL) {
			if((zero_data = malloc(block_size)) == NULL)
				EXIT_UNSQUASH("write_block: failed to alloc "
					"zero data block\n");
			memset(zero_data, 0, block_size);
		}

		if(sparse == FALSE || lseek_broken) {
			int blocks = (hole + block_size -1) / block_size;
			int avail_bytes, i;
			for(i = 0; i < blocks; i++, hole -= avail_bytes) {
				avail_bytes = hole > block_size ? block_size :
					hole;
				if(write_bytes(file_fd, zero_data, avail_bytes)
						== -1)
					goto failure;
			}
		}
	}

	if(write_bytes(file_fd, buffer, size) == -1)
		goto failure;

	return TRUE;

failure:
	return FALSE;
}


int write_file(struct inode *inode, char *pathname)
{
	unsigned int file_fd, i;
	unsigned int *block_list;
	int file_end = inode->data / block_size;
	long long start = inode->start;
	struct squashfs_file *file;

	TRACE("write_file: regular file, blocks %d\n", inode->blocks);

	file_fd = open(pathname, O_CREAT | O_WRONLY | (force ? O_TRUNC : 0),
		(mode_t) inode->mode & 0777);
	if(file_fd == -1) {
		ERROR("write_file: failed to create file %s, because %s\n",
			pathname, strerror(errno));
		return FALSE;
	}

	if((block_list = malloc(inode->blocks * sizeof(unsigned int))) == NULL)
		EXIT_UNSQUASH("write_file: unable to malloc block list\n");

	s_ops.read_block_list(block_list, inode->block_ptr, inode->blocks);

	if((file = malloc(sizeof(struct squashfs_file))) == NULL)
		EXIT_UNSQUASH("write_file: unable to malloc file\n");

	/*
	 * the writer thread is queued a squashfs_file structure describing the
 	 * file.  If the file has one or more blocks or a fragments they are
 	 * queued separately (references to blocks in the cache).
 	 */
	file->fd = file_fd;
	file->file_size = inode->data;
	file->mode = inode->mode;
	file->gid = inode->gid;
	file->uid = inode->uid;
	file->time = inode->time;
	file->pathname = strdup(pathname);
	file->blocks = inode->blocks + (inode->frag_bytes > 0);
	file->sparse = inode->sparse;
	queue_put(to_writer, file);

	for(i = 0; i < inode->blocks; i++) {
		int c_byte = SQUASHFS_COMPRESSED_SIZE_BLOCK(block_list[i]);
		struct file_entry *block = malloc(sizeof(struct file_entry));

		if(block == NULL)
			EXIT_UNSQUASH("write_file: unable to malloc file\n");
		block->offset = 0;
		block->size = i == file_end ? inode->data & (block_size - 1) :
			block_size;
		if(block_list[i] == 0) /* sparse file */
			block->buffer = NULL;
		else {
			block->buffer = cache_get(data_cache, start,
				block_list[i]);
			if(block->buffer == NULL)
				EXIT_UNSQUASH("write_file: cache_get failed\n");
			start += c_byte;
		}
		queue_put(to_writer, block);
	}

	if(inode->frag_bytes) {
		int size;
		long long start;
		struct file_entry *block = malloc(sizeof(struct file_entry));

		if(block == NULL)
			EXIT_UNSQUASH("write_file: unable to malloc file\n");
		s_ops.read_fragment(inode->fragment, &start, &size);
		block->buffer = cache_get(fragment_cache, start, size);
		if(block->buffer == NULL)
			EXIT_UNSQUASH("write_file: cache_get failed\n");
		block->offset = inode->offset;
		block->size = inode->frag_bytes;
		queue_put(to_writer, block);
	}

	free(block_list);
	return TRUE;
}


int create_inode(char *pathname, struct inode *i)
{
	TRACE("create_inode: pathname %s\n", pathname);

	if(created_inode[i->inode_number - 1]) {
		TRACE("create_inode: hard link\n");
		if(force)
			unlink(pathname);

		if(link(created_inode[i->inode_number - 1], pathname) == -1) {
			ERROR("create_inode: failed to create hardlink, "
				"because %s\n", strerror(errno));
			return FALSE;
		}

		return TRUE;
	}

	switch(i->type) {
		case SQUASHFS_FILE_TYPE:
		case SQUASHFS_LREG_TYPE:
			TRACE("create_inode: regular file, file_size %lld, "
				"blocks %d\n", i->data, i->blocks);

			if(write_file(i, pathname))
				file_count ++;
			break;
		case SQUASHFS_SYMLINK_TYPE:
			TRACE("create_inode: symlink, symlink_size %lld\n",
				i->data);

			if(force)
				unlink(pathname);

			if(symlink(i->symlink, pathname) == -1) {
				ERROR("create_inode: failed to create symlink "
					"%s, because %s\n", pathname,
					strerror(errno));
				break;
			}

			if(root_process) {
				if(lchown(pathname, i->uid, i->gid) == -1)
					ERROR("create_inode: failed to change "
						"uid and gids on %s, because "
						"%s\n", pathname,
						strerror(errno));
			}

			sym_count ++;
			break;
 		case SQUASHFS_BLKDEV_TYPE:
	 	case SQUASHFS_CHRDEV_TYPE: {
			int chrdev = i->type == SQUASHFS_CHRDEV_TYPE;
			TRACE("create_inode: dev, rdev 0x%llx\n", i->data);

			if(root_process) {
				if(force)
					unlink(pathname);

				if(mknod(pathname, chrdev ? S_IFCHR : S_IFBLK,
						makedev((i->data >> 8) & 0xff,
						i->data & 0xff)) == -1) {
					ERROR("create_inode: failed to create "
						"%s device %s, because %s\n",
						chrdev ? "character" : "block",
						pathname, strerror(errno));
					break;
				}
				set_attributes(pathname, i->mode, i->uid,
					i->gid, i->time, TRUE);
				dev_count ++;
			} else
				ERROR("create_inode: could not create %s "
					"device %s, because you're not "
					"superuser!\n", chrdev ? "character" :
					"block", pathname);
			break;
		}
		case SQUASHFS_FIFO_TYPE:
			TRACE("create_inode: fifo\n");

			if(force)
				unlink(pathname);

			if(mknod(pathname, S_IFIFO, 0) == -1) {
				ERROR("create_inode: failed to create fifo %s, "
					"because %s\n", pathname,
					strerror(errno));
				break;
			}
			set_attributes(pathname, i->mode, i->uid, i->gid,
				i->time, TRUE);
			fifo_count ++;
			break;
		case SQUASHFS_SOCKET_TYPE:
			TRACE("create_inode: socket\n");
			ERROR("create_inode: socket %s ignored\n", pathname);
			break;
		default:
			ERROR("Unknown inode type %d in create_inode_table!\n",
				i->type);
			return FALSE;
	}

	created_inode[i->inode_number - 1] = strdup(pathname);

	return TRUE;
}


void uncompress_directory_table(long long start, long long end)
{
	int bytes = 0, size = 0, res;

	TRACE("uncompress_directory_table: start %lld, end %lld\n", start, end);

	while(start < end) {
		if(size - bytes < SQUASHFS_METADATA_SIZE && (directory_table =
				realloc(directory_table, size +=
				SQUASHFS_METADATA_SIZE)) == NULL)
			EXIT_UNSQUASH("uncompress_directory_table: out of "
				"memory in realloc\n");
		TRACE("uncompress_directory_table: reading block 0x%llx\n",
				start);
		add_entry(directory_table_hash, start, bytes);
		res = read_block(start, &start, directory_table + bytes);
		if(res == 0)
			EXIT_UNSQUASH("uncompress_directory_table: failed to "
				"read block\n");
		bytes += res;
	}
}


int squashfs_readdir(struct dir *dir, char **name, unsigned int *start_block,
unsigned int *offset, unsigned int *type)
{
	if(dir->cur_entry == dir->dir_count)
		return FALSE;

	*name = dir->dirs[dir->cur_entry].name;
	*start_block = dir->dirs[dir->cur_entry].start_block;
	*offset = dir->dirs[dir->cur_entry].offset;
	*type = dir->dirs[dir->cur_entry].type;
	dir->cur_entry ++;

	return TRUE;
}


void squashfs_closedir(struct dir *dir)
{
	free(dir->dirs);
	free(dir);
}


char *get_component(char *target, char *targname)
{
	while(*target == '/')
		target ++;

	while(*target != '/' && *target!= '\0')
		*targname ++ = *target ++;

	*targname = '\0';

	return target;
}


void free_path(struct pathname *paths)
{
	int i;

	for(i = 0; i < paths->names; i++) {
		if(paths->name[i].paths)
			free_path(paths->name[i].paths);
		free(paths->name[i].name);
		if(paths->name[i].preg) {
			regfree(paths->name[i].preg);
			free(paths->name[i].preg);
		}
	}

	free(paths);
}


struct pathname *add_path(struct pathname *paths, char *target, char *alltarget)
{
	char targname[1024];
	int i, error;

	target = get_component(target, targname);

	if(paths == NULL) {
		if((paths = malloc(sizeof(struct pathname))) == NULL)
			EXIT_UNSQUASH("failed to allocate paths\n");

		paths->names = 0;
		paths->name = NULL;
	}

	for(i = 0; i < paths->names; i++)
		if(strcmp(paths->name[i].name, targname) == 0)
			break;

	if(i == paths->names) {
		/*
		 * allocate new name entry
		 */
		paths->names ++;
		paths->name = realloc(paths->name, (i + 1) *
			sizeof(struct path_entry));
		paths->name[i].name = strdup(targname);
		paths->name[i].paths = NULL;
		if(use_regex) {
			paths->name[i].preg = malloc(sizeof(regex_t));
			error = regcomp(paths->name[i].preg, targname,
				REG_EXTENDED|REG_NOSUB);
			if(error) {
				char str[1024];

				regerror(error, paths->name[i].preg, str, 1024);
				EXIT_UNSQUASH("invalid regex %s in export %s, "
					"because %s\n", targname, alltarget,
					str);
			}
		} else
			paths->name[i].preg = NULL;

		if(target[0] == '\0')
			/*
			 * at leaf pathname component
			*/
			paths->name[i].paths = NULL;
		else
			/*
			 * recurse adding child components
			 */
			paths->name[i].paths = add_path(NULL, target, alltarget);
	} else {
		/*
		 * existing matching entry
		 */
		if(paths->name[i].paths == NULL) {
			/*
			 * No sub-directory which means this is the leaf
			 * component of a pre-existing extract which subsumes
			 * the extract currently being added, in which case stop
			 * adding components
			 */
		} else if(target[0] == '\0') {
			/*
			 * at leaf pathname component and child components exist
			 * from more specific extracts, delete as they're
			 * subsumed by this extract
			 */
			free_path(paths->name[i].paths);
			paths->name[i].paths = NULL;
		} else
			/*
			 * recurse adding child components
			 */
			add_path(paths->name[i].paths, target, alltarget);
	}

	return paths;
}


struct pathnames *init_subdir()
{
	struct pathnames *new = malloc(sizeof(struct pathnames));
	new->count = 0;
	return new;
}


struct pathnames *add_subdir(struct pathnames *paths, struct pathname *path)
{
	if(paths->count % PATHS_ALLOC_SIZE == 0)
		paths = realloc(paths, sizeof(struct pathnames *) +
		(paths->count + PATHS_ALLOC_SIZE) * sizeof(struct pathname *));

	paths->path[paths->count++] = path;
	return paths;
}


void free_subdir(struct pathnames *paths)
{
	free(paths);
}


int matches(struct pathnames *paths, char *name, struct pathnames **new)
{
	int i, n;

	if(paths == NULL) {
		*new = NULL;
		return TRUE;
	}

	*new = init_subdir();

	for(n = 0; n < paths->count; n++) {
		struct pathname *path = paths->path[n];
		for(i = 0; i < path->names; i++) {
			int match = use_regex ?
				regexec(path->name[i].preg, name, (size_t) 0,
				NULL, 0) == 0 : fnmatch(path->name[i].name,
				name, FNM_PATHNAME|FNM_PERIOD|FNM_EXTMATCH) ==
				0;
			if(match && path->name[i].paths == NULL)
				/*
				 * match on a leaf component, any subdirectories
				 * will implicitly match, therefore return an
				 * empty new search set
				 */
				goto empty_set;

			if(match)
				/*
				 * match on a non-leaf component, add any
				 * subdirectories to the new set of
				 * subdirectories to scan for this name
				 */
				*new = add_subdir(*new, path->name[i].paths);
		}
	}

	if((*new)->count == 0) {
		/*
		 * no matching names found, delete empty search set, and return
		 * FALSE
		 */
		free_subdir(*new);
		*new = NULL;
		return FALSE;
	}

	/*
	* one or more matches with sub-directories found (no leaf matches),
	* return new search set and return TRUE
	*/
	return TRUE;

empty_set:
	/*
	 * found matching leaf exclude, return empty search set and return TRUE
	 */
	free_subdir(*new);
	*new = NULL;
	return TRUE;
}


int pre_scan(char *parent_name, unsigned int start_block, unsigned int offset,
	struct pathnames *paths)
{
	unsigned int type;
	char *name, pathname[1024];
	struct pathnames *new;
	struct inode *i;
	struct dir *dir = s_ops.squashfs_opendir(start_block, offset, &i);

	if(dir == NULL) {
		ERROR("pre_scan: Failed to read directory %s (%x:%x)\n",
			parent_name, start_block, offset);
		return FALSE;
	}

	while(squashfs_readdir(dir, &name, &start_block, &offset, &type)) {
		struct inode *i;

		TRACE("pre_scan: name %s, start_block %d, offset %d, type %d\n",
			name, start_block, offset, type);

		if(!matches(paths, name, &new))
			continue;

		strcat(strcat(strcpy(pathname, parent_name), "/"), name);

		if(type == SQUASHFS_DIR_TYPE)
			pre_scan(parent_name, start_block, offset, new);
		else if(new == NULL) {
			if(type == SQUASHFS_FILE_TYPE ||
					type == SQUASHFS_LREG_TYPE) {
				if((i = s_ops.read_inode(start_block, offset))
						== NULL) {
					ERROR("failed to read header\n");
					continue;
				}
				if(created_inode[i->inode_number - 1] == NULL) {
					created_inode[i->inode_number - 1] =
						(char *) i;
					total_blocks += (i->data +
						(block_size - 1)) >> block_log;
				}
				total_files ++;
			}
			total_inodes ++;
		}

		free_subdir(new);
	}

	squashfs_closedir(dir);

	return TRUE;
}


int dir_scan(char *parent_name, unsigned int start_block, unsigned int offset,
	struct pathnames *paths)
{
	unsigned int type;
	char *name, pathname[1024];
	struct pathnames *new;
	struct inode *i;
	struct dir *dir = s_ops.squashfs_opendir(start_block, offset, &i);

	if(dir == NULL) {
		ERROR("dir_scan: Failed to read directory %s (%x:%x)\n",
			parent_name, start_block, offset);
		return FALSE;
	}

	if(lsonly || info)
		print_filename(parent_name, i);

	if(!lsonly && mkdir(parent_name, (mode_t) dir->mode) == -1 &&
			(!force || errno != EEXIST)) {
		ERROR("dir_scan: failed to open directory %s, because %s\n",
			parent_name, strerror(errno));
		return FALSE;
	}

	while(squashfs_readdir(dir, &name, &start_block, &offset, &type)) {
		TRACE("dir_scan: name %s, start_block %d, offset %d, type %d\n",
			name, start_block, offset, type);


		if(!matches(paths, name, &new))
			continue;

		strcat(strcat(strcpy(pathname, parent_name), "/"), name);

		if(type == SQUASHFS_DIR_TYPE)
			dir_scan(pathname, start_block, offset, new);
		else if(new == NULL) {
			if((i = s_ops.read_inode(start_block, offset)) == NULL) {
				ERROR("failed to read header\n");
				continue;
			}

			if(lsonly || info)
				print_filename(pathname, i);

			if(!lsonly) {
				create_inode(pathname, i);
				update_progress_bar();
				}

			if(i->type == SQUASHFS_SYMLINK_TYPE ||
					i->type == SQUASHFS_LSYMLINK_TYPE)
				free(i->symlink);
		}

		free_subdir(new);
	}

	if(!lsonly)
		set_attributes(parent_name, dir->mode, dir->uid, dir->guid,
			dir->mtime, force);

	squashfs_closedir(dir);
	dir_count ++;

	return TRUE;
}


void squashfs_stat(char *source)
{
	time_t mkfs_time = (time_t) sBlk.mkfs_time;
	char *mkfs_str = ctime(&mkfs_time);

#if __BYTE_ORDER == __BIG_ENDIAN
	printf("Found a valid %sSQUASHFS %d:%d superblock on %s.\n",
		sBlk.s_major == 4 ? "" : swap ? "little endian " :
		"big endian ", sBlk.s_major, sBlk.s_minor, source);
#else
	printf("Found a valid %sSQUASHFS %d:%d superblock on %s.\n",
		sBlk.s_major == 4 ? "" : swap ? "big endian " :
		"little endian ", sBlk.s_major, sBlk.s_minor, source);
#endif
	printf("Creation or last append time %s", mkfs_str ? mkfs_str :
		"failed to get time\n");
	printf("Filesystem is %sexportable via NFS\n",
		SQUASHFS_EXPORTABLE(sBlk.flags) ? "" : "not ");

	printf("Inodes are %scompressed\n",
		SQUASHFS_UNCOMPRESSED_INODES(sBlk.flags) ? "un" : "");
	printf("Data is %scompressed\n",
		SQUASHFS_UNCOMPRESSED_DATA(sBlk.flags) ? "un" : "");
	if(sBlk.s_major > 1 && !SQUASHFS_NO_FRAGMENTS(sBlk.flags))
		printf("Fragments are %scompressed\n",
			SQUASHFS_UNCOMPRESSED_FRAGMENTS(sBlk.flags) ? "un" :
			"");
	printf("Check data is %spresent in the filesystem\n",
		SQUASHFS_CHECK_DATA(sBlk.flags) ? "" : "not ");
	if(sBlk.s_major > 1) {
		printf("Fragments are %spresent in the filesystem\n",
			SQUASHFS_NO_FRAGMENTS(sBlk.flags) ? "not " : "");
		printf("Always_use_fragments option is %sspecified\n",
			SQUASHFS_ALWAYS_FRAGMENTS(sBlk.flags) ? "" : "not ");
	} else
		printf("Fragments are not supported by the filesystem\n");

	if(sBlk.s_major > 1)
		printf("Duplicates are %sremoved\n",
			SQUASHFS_DUPLICATES(sBlk.flags) ? "" : "not ");
	else
		printf("Duplicates are removed\n");
	printf("Filesystem size %.2f Kbytes (%.2f Mbytes)\n",
		sBlk.bytes_used / 1024.0, sBlk.bytes_used / (1024.0 * 1024.0));
	printf("Block size %d\n", sBlk.block_size);
	if(sBlk.s_major > 1)
		printf("Number of fragments %d\n", sBlk.fragments);
	printf("Number of inodes %d\n", sBlk.inodes);
	if(sBlk.s_major == 4)
		printf("Number of ids %d\n", sBlk.no_ids);
	else {
		printf("Number of uids %d\n", sBlk.no_uids);
		printf("Number of gids %d\n", sBlk.no_guids);
	}

	TRACE("sBlk.inode_table_start 0x%llx\n", sBlk.inode_table_start);
	TRACE("sBlk.directory_table_start 0x%llx\n",
		sBlk.directory_table_start);
	if(sBlk.s_major == 4)
		TRACE("sBlk.id_table_start 0x%llx\n", sBlk.id_table_start);
	else {
		TRACE("sBlk.uid_start 0x%llx\n", sBlk.uid_start);
		TRACE("sBlk.guid_start 0x%llx\n", sBlk.guid_start);
	}
	if(sBlk.s_major > 1)
		TRACE("sBlk.fragment_table_start 0x%llx\n\n",
			sBlk.fragment_table_start);
}


int read_super(char *source)
{
	squashfs_super_block_3 sBlk_3;
	squashfs_super_block sBlk_4;

#ifdef SQUASHFS_LZMA_ENABLE
	un.un_lzma = 1;
#endif

	/*
	 * Try to read a Squashfs 4 superblock
	 */
	read_bytes(SQUASHFS_START, sizeof(squashfs_super_block),
		(char *) &sBlk_4);

	
	
#ifdef SQUASHFS_LZMA_ENABLE
	if(sBlk_4.s_major == 4 && sBlk_4.s_minor == 0)
	{
		swap = 0;
		
		switch (sBlk_4.s_magic) {
			squashfs_super_block sblk;
		case SQUASHFS_MAGIC:
			un.un_lzma = 0;
			/*FALLTHROUGH*/
		case SQUASHFS_MAGIC_LZMA:
			s_ops.squashfs_opendir = squashfs_opendir_4;
			s_ops.read_fragment = read_fragment_4;
			s_ops.read_fragment_table = read_fragment_table_4;
			s_ops.read_block_list = read_block_list_2;
			s_ops.read_inode = read_inode_4;
			s_ops.read_uids_guids = read_uids_guids_4;
			memcpy(&sBlk, &sBlk_4, sizeof(sBlk_4));
			return TRUE;
		case SQUASHFS_MAGIC_SWAP:
			un.un_lzma = 0;
			/*FALLTHROUGH*/
		case SQUASHFS_MAGIC_LZMA_SWAP:
			printf("\nReading a different endian SQUASHFS filesystem on %s\n", source);
			SQUASHFS_SWAP_SUPER_BLOCK(&sblk, &sBlk_4);
			memcpy(&sBlk_4, &sblk, sizeof(squashfs_super_block));

			s_ops.squashfs_opendir = squashfs_opendir_4;
			s_ops.read_fragment = read_fragment_4;
			s_ops.read_fragment_table = read_fragment_table_4;
			s_ops.read_block_list = read_block_list_2;
			s_ops.read_inode = read_inode_4;
			s_ops.read_uids_guids = read_uids_guids_4;
			
			swap = 1;
			return TRUE;
		default:
			ERROR("Can't find a SQUASHFS superblock on %s\n", source);
			goto failed_mount;
		}
	}
	else
	{//previous squashfs
#else
	swap = sBlk_4.s_magic != SQUASHFS_MAGIC;
	SQUASHFS_INSWAP_SUPER_BLOCK(&sBlk_4);
	if(sBlk_4.s_magic == SQUASHFS_MAGIC && sBlk_4.s_major == 4 &&
			sBlk_4.s_minor == 0) {
		s_ops.squashfs_opendir = squashfs_opendir_4;
		s_ops.read_fragment = read_fragment_4;
		s_ops.read_fragment_table = read_fragment_table_4;
		s_ops.read_block_list = read_block_list_2;
		s_ops.read_inode = read_inode_4;
		s_ops.read_uids_guids = read_uids_guids_4;
		memcpy(&sBlk, &sBlk_4, sizeof(sBlk_4));
		return TRUE;
	}
#endif	

 	/*
 	 * Not a Squashfs 4 superblock, try to read a squashfs 3 superblock
 	 * (compatible with 1 and 2 filesystems)
 	 */
	read_bytes(SQUASHFS_START, sizeof(squashfs_super_block_3),
		(char *) &sBlk_3);

	/*
	 * Check it is a SQUASHFS superblock
	 */
	swap = 0;
	

	if(sBlk_3.s_magic != SQUASHFS_MAGIC) {
		if(sBlk_3.s_magic == SQUASHFS_MAGIC_SWAP) {
			squashfs_super_block_3 sblk;
			ERROR("Reading a different endian SQUASHFS filesystem "
				"on %s\n", source);
			SQUASHFS_SWAP_SUPER_BLOCK_3(&sblk, &sBlk_3);
			memcpy(&sBlk_3, &sblk, sizeof(squashfs_super_block_3));
			swap = 1;
		} else  {
			ERROR("Can't find a SQUASHFS superblock on %s\n",
				source);
			goto failed_mount;
		}
	}

	sBlk.s_magic = sBlk_3.s_magic;
	sBlk.inodes = sBlk_3.inodes;
	sBlk.mkfs_time = sBlk_3.mkfs_time;
	sBlk.block_size = sBlk_3.block_size;
	sBlk.fragments = sBlk_3.fragments;
	sBlk.block_log = sBlk_3.block_log;
	sBlk.flags = sBlk_3.flags;
	sBlk.s_major = sBlk_3.s_major;
	sBlk.s_minor = sBlk_3.s_minor;
	sBlk.root_inode = sBlk_3.root_inode;
	sBlk.bytes_used = sBlk_3.bytes_used;
	sBlk.inode_table_start = sBlk_3.inode_table_start;
	sBlk.directory_table_start = sBlk_3.directory_table_start;
	sBlk.fragment_table_start = sBlk_3.fragment_table_start;
	sBlk.lookup_table_start = sBlk_3.lookup_table_start;
	sBlk.no_uids = sBlk_3.no_uids;
	sBlk.no_guids = sBlk_3.no_guids;
	sBlk.uid_start = sBlk_3.uid_start;
	sBlk.guid_start = sBlk_3.guid_start;

	/* Check the MAJOR & MINOR versions */
	if(sBlk.s_major == 1 || sBlk.s_major == 2) {
		sBlk.bytes_used = sBlk_3.bytes_used_2;
		sBlk.uid_start = sBlk_3.uid_start_2;
		sBlk.guid_start = sBlk_3.guid_start_2;
		sBlk.inode_table_start = sBlk_3.inode_table_start_2;
		sBlk.directory_table_start = sBlk_3.directory_table_start_2;
		
		if(sBlk.s_major == 1) {
			sBlk.block_size = sBlk_3.block_size_1;
			sBlk.fragment_table_start = sBlk.uid_start;
			s_ops.squashfs_opendir = squashfs_opendir_1;
			s_ops.read_fragment_table = read_fragment_table_1;
			s_ops.read_block_list = read_block_list_1;
			s_ops.read_inode = read_inode_1;
			s_ops.read_uids_guids = read_uids_guids_1;
		} else {
			sBlk.fragment_table_start =
				sBlk_3.fragment_table_start_2;
			s_ops.squashfs_opendir = squashfs_opendir_1;
			s_ops.read_fragment = read_fragment_2;
			s_ops.read_fragment_table = read_fragment_table_2;
			s_ops.read_block_list = read_block_list_2;
			s_ops.read_inode = read_inode_2;
			s_ops.read_uids_guids = read_uids_guids_1;
		}
	} else if(sBlk.s_major == 3) {
		s_ops.squashfs_opendir = squashfs_opendir_3;
		s_ops.read_fragment = read_fragment_3;
		s_ops.read_fragment_table = read_fragment_table_3;
		s_ops.read_block_list = read_block_list_2;
		s_ops.read_inode = read_inode_3;
		s_ops.read_uids_guids = read_uids_guids_1;
	} else {
		ERROR("Filesystem on %s is (%d:%d), ", source, sBlk.s_major,
			sBlk.s_minor);
		ERROR("which is a later filesystem version than I support!\n");
		goto failed_mount;
	}
#ifdef SQUASHFS_LZMA_ENABLE
	} //end of previous squashfs
#endif
	return TRUE;

failed_mount:
	return FALSE;
}


struct pathname *process_extract_files(struct pathname *path, char *filename)
{
	FILE *fd;
	char name[16384];

	if((fd = fopen(filename, "r")) == NULL)
		EXIT_UNSQUASH("Could not open %s, because %s\n", filename,
			strerror(errno));

	while(fscanf(fd, "%16384[^\n]\n", name) != EOF)
		path = add_path(path, name, name);

	fclose(fd);
	return path;
}
		

/*
 * reader thread.  This thread processes read requests queued by the
 * cache_get() routine.
 */
void *reader(void *arg)
{
	while(1) {
		struct cache_entry *entry = queue_get(to_reader);
		int res = read_bytes(entry->block,
			SQUASHFS_COMPRESSED_SIZE_BLOCK(entry->size),
			entry->data);

		if(res && SQUASHFS_COMPRESSED_BLOCK(entry->size))
			/*
			 * queue successfully read block to the deflate
			 * thread(s) for further processing
 			 */
			queue_put(to_deflate, entry);
		else
			/*
			 * block has either been successfully read and is
			 * uncompressed, or an error has occurred, clear pending
			 * flag, set error appropriately, and wake up any
			 * threads waiting on this buffer
			 */
			cache_block_ready(entry, !res);
	}
}


/*
 * writer thread.  This processes file write requests queued by the
 * write_file() routine.
 */
void *writer(void *arg)
{
	int i;

	while(1) {
		struct squashfs_file *file = queue_get(to_writer);
		int file_fd;
		int hole = 0;
		int failed = FALSE;
		int error;

		if(file == NULL) {
			queue_put(from_writer, NULL);
			continue;
		}

		TRACE("writer: regular file, blocks %d\n", file->blocks);

		file_fd = file->fd;

		for(i = 0; i < file->blocks; i++, cur_blocks ++) {
			struct file_entry *block = queue_get(to_writer);

			if(block->buffer == 0) { /* sparse file */
				hole += block->size;
				free(block);
				continue;
			}

			cache_block_wait(block->buffer);

			if(block->buffer->error)
				failed = TRUE;

			if(failed)
				continue;

			error = write_block(file_fd, block->buffer->data +
				block->offset, block->size, hole, file->sparse);

			if(error == FALSE) {
				ERROR("writer: failed to write data block %d\n",
					i);
				failed = TRUE;
			}

			hole = 0;
			cache_block_put(block->buffer);
			free(block);
		}

		if(hole && failed == FALSE) {
			/*
			 * corner case for hole extending to end of file
			 */
			if(file->sparse == FALSE ||
					lseek(file_fd, hole, SEEK_CUR) == -1) {
				/*
				 * for files which we don't want to write
				 * sparsely, or for broken lseeks which cannot
				 * seek beyond end of file, write_block will do
				 * the right thing
				 */
				hole --;
				if(write_block(file_fd, "\0", 1, hole,
						file->sparse) == FALSE) {
					ERROR("writer: failed to write sparse "
						"data block\n");
					failed = TRUE;
				}
			} else if(ftruncate(file_fd, file->file_size) == -1) {
				ERROR("writer: failed to write sparse data "
					"block\n");
				failed = TRUE;
			}
		}

		close(file_fd);
		if(failed == FALSE)
			set_attributes(file->pathname, file->mode, file->uid,
				file->gid, file->time, force);
		else {
			ERROR("Failed to write %s, skipping\n", file->pathname);
			unlink(file->pathname);
		}
		free(file->pathname);
		free(file);

	}
}


/*
 * decompress thread.  This decompresses buffers queued by the read thread
 */
void *deflator(void *arg)
{
	char tmp[block_size];
#ifdef SQUASHFS_LZMA_ENABLE	
	struct sqlzma_un *thread_un = (struct sqlzma_un *) arg;
#endif

	while(1) {
		struct cache_entry *entry = queue_get(to_deflate);
		int res;
		unsigned long bytes = block_size;
#ifdef SQUASHFS_LZMA_ENABLE
		enum {Src, Dst};
		struct sized_buf sbuf[] = {
			{.buf = (void *)entry->data, .sz = SQUASHFS_COMPRESSED_SIZE_BLOCK(entry->size)},
			{.buf = (void *)tmp, .sz = bytes}
		};

		res = sqlzma_un(thread_un, sbuf + Src, sbuf + Dst);
		if(res)
			abort();
		bytes = thread_un->un_reslen;
		memcpy(entry->data, tmp, bytes);

#else
		res = uncompress((unsigned char *) tmp, &bytes,
			(const unsigned char *) entry->data,
			SQUASHFS_COMPRESSED_SIZE_BLOCK(entry->size));

		if(res != Z_OK) {
			if(res == Z_MEM_ERROR)
				ERROR("zlib::uncompress failed, not enough"
					"memory\n");
			else if(res == Z_BUF_ERROR)
				ERROR("zlib::uncompress failed, not enough "
					"room in output buffer\n");
			else
				ERROR("zlib::uncompress failed, unknown error "
					"%d\n", res);
		} else
			memcpy(entry->data, tmp, bytes);
#endif
		/*
		 * block has been either successfully decompressed, or an error
 		 * occurred, clear pending flag, set error appropriately and
 		 * wake up any threads waiting on this block
 		 */ 
		cache_block_ready(entry, res != Z_OK);
	}
}


void *progress_thread(void *arg)
{
	struct timeval timeval;
	struct timespec timespec;
	struct itimerval itimerval;
	struct winsize winsize;

	if(ioctl(1, TIOCGWINSZ, &winsize) == -1) {
		if(isatty(STDOUT_FILENO))
			ERROR("TIOCGWINSZ ioctl failed, defaulting to 80 "
				"columns\n");
		columns = 80;
	} else
		columns = winsize.ws_col;
	signal(SIGWINCH, sigwinch_handler);
	signal(SIGALRM, sigalrm_handler);

	itimerval.it_value.tv_sec = 0;
	itimerval.it_value.tv_usec = 250000;
	itimerval.it_interval.tv_sec = 0;
	itimerval.it_interval.tv_usec = 250000;
	setitimer(ITIMER_REAL, &itimerval, NULL);

	pthread_cond_init(&progress_wait, NULL);

	pthread_mutex_lock(&screen_mutex);
	while(1) {
		gettimeofday(&timeval, NULL);
		timespec.tv_sec = timeval.tv_sec;
		if(timeval.tv_usec + 250000 > 999999)
			timespec.tv_sec++;
		timespec.tv_nsec = ((timeval.tv_usec + 250000) % 1000000) *
			1000;
		pthread_cond_timedwait(&progress_wait, &screen_mutex,
			&timespec);
		if(progress_enabled)
			progress_bar(sym_count + dev_count +
				fifo_count + cur_blocks, total_inodes -
				total_files + total_blocks, columns);
	}
}


void initialise_threads(int fragment_buffer_size, int data_buffer_size)
{
	int i;
	sigset_t sigmask, old_mask;
	int all_buffers_size = fragment_buffer_size + data_buffer_size;

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, SIGQUIT);
	if(sigprocmask(SIG_BLOCK, &sigmask, &old_mask) == -1)
		EXIT_UNSQUASH("Failed to set signal mask in intialise_threads"
			"\n");

	if(processors == -1) {
#ifndef linux
		int mib[2];
		size_t len = sizeof(processors);

		mib[0] = CTL_HW;
#ifdef HW_AVAILCPU
		mib[1] = HW_AVAILCPU;
#else
		mib[1] = HW_NCPU;
#endif

		if(sysctl(mib, 2, &processors, &len, NULL, 0) == -1) {
			ERROR("Failed to get number of available processors.  "
				"Defaulting to 1\n");
			processors = 1;
		}
#else
		processors = get_nprocs();
#endif
	}

	if((thread = malloc((3 + processors) * sizeof(pthread_t))) == NULL)
		EXIT_UNSQUASH("Out of memory allocating thread descriptors\n");
	deflator_thread = &thread[3];

	to_reader = queue_init(all_buffers_size);
	to_deflate = queue_init(all_buffers_size);
	to_writer = queue_init(1000);
	from_writer = queue_init(1);
	fragment_cache = cache_init(block_size, fragment_buffer_size);
	data_cache = cache_init(block_size, data_buffer_size);
	pthread_create(&thread[0], NULL, reader, NULL);
	pthread_create(&thread[1], NULL, writer, NULL);
	pthread_create(&thread[2], NULL, progress_thread, NULL);
	pthread_mutex_init(&fragment_mutex, NULL);

	for(i = 0; i < processors; i++) {
#ifdef SQUASHFS_LZMA_ENABLE
		struct sqlzma_un *thread_un = malloc(sizeof(struct sqlzma_un));
		if(thread_un == NULL)
			EXIT_UNSQUASH("Failed to allocate memory for sqlzma_un\n");
		if(sqlzma_init(thread_un, un.un_lzma, 0) != Z_OK)
			EXIT_UNSQUASH("Failed to initialize: sqlzma_init\n");
		if(pthread_create(&deflator_thread[i], NULL, deflator, (void *)thread_un) != 0 )
#else	
		if(pthread_create(&deflator_thread[i], NULL, deflator, NULL) != 0)
#endif			
			
			EXIT_UNSQUASH("Failed to create thread\n");
	}

	printf("Parallel unsquashfs: Using %d processor%s\n", processors,
			processors == 1 ? "" : "s");

	if(sigprocmask(SIG_SETMASK, &old_mask, NULL) == -1)
		EXIT_UNSQUASH("Failed to set signal mask in intialise_threads"
			"\n");
}


void enable_progress_bar()
{
	pthread_mutex_lock(&screen_mutex);
	progress_enabled = TRUE;
	pthread_mutex_unlock(&screen_mutex);
}


void disable_progress_bar()
{
	pthread_mutex_lock(&screen_mutex);
	progress_enabled = FALSE;
	pthread_mutex_unlock(&screen_mutex);
}


void update_progress_bar()
{
	pthread_mutex_lock(&screen_mutex);
	pthread_cond_signal(&progress_wait);
	pthread_mutex_unlock(&screen_mutex);
}


void progress_bar(long long current, long long max, int columns)
{
	char rotate_list[] = { '|', '/', '-', '\\' };
	int max_digits = floor(log10(max)) + 1;
	int used = max_digits * 2 + 11;
	int hashes = (current * (columns - used)) / max;
	int spaces = columns - used - hashes;
	static int tty = -1;

	if((current > max) || (columns - used < 0))
		return;

	if(tty == -1)
		tty = isatty(STDOUT_FILENO);
	if(!tty) {
		static long long previous = -1;

		/* Updating much more frequently than this results in huge
		 * log files. */
		if((current % 100) != 0 && current != max)
			return;
		/* Don't update just to rotate the spinner. */
		if(current == previous)
			return;
		previous = current;
	}

	printf("\r[");

	while (hashes --)
		putchar('=');

	putchar(rotate_list[rotate]);

	while(spaces --)
		putchar(' ');

	printf("] %*lld/%*lld", max_digits, current, max_digits, max);
	printf(" %3lld%%", current * 100 / max);
	fflush(stdout);
}


#define VERSION() \
	printf("unsquashfs version 4.0 (2009/04/05)\n");\
	printf("copyright (C) 2009 Phillip Lougher <phillip@lougher.demon.co.uk>"\
		"\n\n");\
    	printf("This program is free software; you can redistribute it and/or\n");\
	printf("modify it under the terms of the GNU General Public License\n");\
	printf("as published by the Free Software Foundation; either version 2,"\
		"\n");\
	printf("or (at your option) any later version.\n\n");\
	printf("This program is distributed in the hope that it will be useful,"\
		"\n");\
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of"\
		"\n");\
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"\
		"\n");\
	printf("GNU General Public License for more details.\n");
int main(int argc, char *argv[])
{
	char *dest = "squashfs-root";
	int i, stat_sys = FALSE, version = FALSE;
	int n;
	struct pathnames *paths = NULL;
	struct pathname *path = NULL;
	int fragment_buffer_size = FRAGMENT_BUFFER_DEFAULT;
	int data_buffer_size = DATA_BUFFER_DEFAULT;
	char *b;
	struct winsize winsize;

	pthread_mutex_init(&screen_mutex, NULL);
	root_process = geteuid() == 0;
	if(root_process)
		umask(0);
	
	for(i = 1; i < argc; i++) {
		if(*argv[i] != '-')
			break;
		if(strcmp(argv[i], "-version") == 0 ||
				strcmp(argv[i], "-v") == 0) {
			VERSION();
			version = TRUE;
		} else if(strcmp(argv[i], "-info") == 0 ||
				strcmp(argv[i], "-i") == 0)
			info = TRUE;
		else if(strcmp(argv[i], "-ls") == 0 ||
				strcmp(argv[i], "-l") == 0)
			lsonly = TRUE;
		else if(strcmp(argv[i], "-no-progress") == 0 ||
				strcmp(argv[i], "-n") == 0)
			progress = FALSE;
		else if(strcmp(argv[i], "-dest") == 0 ||
				strcmp(argv[i], "-d") == 0) {
			if(++i == argc) {
				fprintf(stderr, "%s: -dest missing filename\n",
					argv[0]);
				exit(1);
			}
			dest = argv[i];
		} else if(strcmp(argv[i], "-processors") == 0 ||
				strcmp(argv[i], "-p") == 0) {
			if((++i == argc) || 
					(processors = strtol(argv[i], &b, 10),
					*b != '\0')) {
				ERROR("%s: -processors missing or invalid "
					"processor number\n", argv[0]);
				exit(1);
			}
			if(processors < 1) {
				ERROR("%s: -processors should be 1 or larger\n",
					argv[0]);
				exit(1);
			}
		} else if(strcmp(argv[i], "-data-queue") == 0 ||
					 strcmp(argv[i], "-da") == 0) {
			if((++i == argc) ||
					(data_buffer_size = strtol(argv[i], &b,
					 10), *b != '\0')) {
				ERROR("%s: -data-queue missing or invalid "
					"queue size\n", argv[0]);
				exit(1);
			}
			if(data_buffer_size < 1) {
				ERROR("%s: -data-queue should be 1 Mbyte or "
					"larger\n", argv[0]);
				exit(1);
			}
		} else if(strcmp(argv[i], "-frag-queue") == 0 ||
					strcmp(argv[i], "-fr") == 0) {
			if((++i == argc) ||
					(fragment_buffer_size = strtol(argv[i],
					 &b, 10), *b != '\0')) {
				ERROR("%s: -frag-queue missing or invalid "
					"queue size\n", argv[0]);
				exit(1);
			}
			if(fragment_buffer_size < 1) {
				ERROR("%s: -frag-queue should be 1 Mbyte or "
					"larger\n", argv[0]);
				exit(1);
			}
		} else if(strcmp(argv[i], "-force") == 0 ||
				strcmp(argv[i], "-f") == 0)
			force = TRUE;
		else if(strcmp(argv[i], "-stat") == 0 ||
				strcmp(argv[i], "-s") == 0)
			stat_sys = TRUE;
		else if(strcmp(argv[i], "-lls") == 0 ||
				strcmp(argv[i], "-ll") == 0) {
			lsonly = TRUE;
			short_ls = FALSE;
		} else if(strcmp(argv[i], "-linfo") == 0 ||
				strcmp(argv[i], "-li") == 0) {
			info = TRUE;
			short_ls = FALSE;
		} else if(strcmp(argv[i], "-ef") == 0 ||
				strcmp(argv[i], "-e") == 0) {
			if(++i == argc) {
				fprintf(stderr, "%s: -ef missing filename\n",
					argv[0]);
				exit(1);
			}
			path = process_extract_files(path, argv[i]);
		} else if(strcmp(argv[i], "-regex") == 0 ||
				strcmp(argv[i], "-r") == 0)
			use_regex = TRUE;
		else
			goto options;
	}

	if(lsonly || info)
		progress = FALSE;

#ifdef SQUASHFS_TRACE
	progress = FALSE;
#endif

	if(i == argc) {
		if(!version) {
options:
			ERROR("SYNTAX: %s [options] filesystem [directories or "
				"files to extract]\n", argv[0]);
			ERROR("\t-v[ersion]\t\tprint version, licence and "
				"copyright information\n");
			ERROR("\t-d[est] <pathname>\tunsquash to <pathname>, "
				"default \"squashfs-root\"\n");
			ERROR("\t-n[o-progress]\t\tdon't display the progress "
				"bar\n");
			ERROR("\t-p[rocessors] <number>\tuse <number> "
				"processors.  By default will use\n");
			ERROR("\t\t\t\tnumber of processors available\n");
			ERROR("\t-i[nfo]\t\t\tprint files as they are "
				"unsquashed\n");
			ERROR("\t-li[nfo]\t\tprint files as they are "
				"unsquashed with file\n");
			ERROR("\t\t\t\tattributes (like ls -l output)\n");
			ERROR("\t-l[s]\t\t\tlist filesystem, but don't unsquash"
				"\n");
			ERROR("\t-ll[s]\t\t\tlist filesystem with file "
				"attributes (like\n");
			ERROR("\t\t\t\tls -l output), but don't unsquash\n");
			ERROR("\t-f[orce]\t\tif file already exists then "
				"overwrite\n");
			ERROR("\t-s[tat]\t\t\tdisplay filesystem superblock "
				"information\n");
			ERROR("\t-e[f] <extract file>\tlist of directories or "
				"files to extract.\n\t\t\t\tOne per line\n");
			ERROR("\t-da[ta-queue] <size>\tSet data queue to "
				"<size> Mbytes.  Default %d\n\t\t\t\tMbytes\n",
				DATA_BUFFER_DEFAULT);
			ERROR("\t-fr[ag-queue] <size>\tSet fagment queue to "
				"<size> Mbytes.  Default %d\n\t\t\t\t Mbytes\n",
				FRAGMENT_BUFFER_DEFAULT);
			ERROR("\t-r[egex]\t\ttreat extract names as POSIX "
				"regular expressions\n");
			ERROR("\t\t\t\trather than use the default shell "
				"wildcard\n\t\t\t\texpansion (globbing)\n");
		}
		exit(1);
	}

	for(n = i + 1; n < argc; n++)
		path = add_path(path, argv[n], argv[n]);

	if((fd = open(argv[i], O_RDONLY)) == -1) {
		ERROR("Could not open %s, because %s\n", argv[i],
			strerror(errno));
		exit(1);
	}

	if(read_super(argv[i]) == FALSE)
		exit(1);

 	if(stat_sys) {
		squashfs_stat(argv[i]);
		exit(0);
	}

	block_size = sBlk.block_size;
	block_log = sBlk.block_log;

	fragment_buffer_size <<= 20 - block_log;
	data_buffer_size <<= 20 - block_log;
	initialise_threads(fragment_buffer_size, data_buffer_size);

	if((fragment_data = malloc(block_size)) == NULL)
		EXIT_UNSQUASH("failed to allocate fragment_data\n");

	if((file_data = malloc(block_size)) == NULL)
		EXIT_UNSQUASH("failed to allocate file_data");

	if((data = malloc(block_size)) == NULL)
		EXIT_UNSQUASH("failed to allocate data\n");

	if((created_inode = malloc(sBlk.inodes * sizeof(char *))) == NULL)
		EXIT_UNSQUASH("failed to allocate created_inode\n");

	memset(created_inode, 0, sBlk.inodes * sizeof(char *));
#ifdef SQUASHFS_LZMA_ENABLE
	i = sqlzma_init(&un, un.un_lzma, 0);
	if (i != Z_OK) {
		fputs("sqlzma_init failed", stderr);
		abort();
	}
#endif	

	if(s_ops.read_uids_guids() == FALSE)
		EXIT_UNSQUASH("failed to uid/gid table\n");

	if(s_ops.read_fragment_table() == FALSE)
		EXIT_UNSQUASH("failed to read fragment table\n");

	uncompress_inode_table(sBlk.inode_table_start,
		sBlk.directory_table_start);

	uncompress_directory_table(sBlk.directory_table_start,
		sBlk.fragment_table_start);

	if(path) {
		paths = init_subdir();
		paths = add_subdir(paths, path);
	}

	pre_scan(dest, SQUASHFS_INODE_BLK(sBlk.root_inode),
		SQUASHFS_INODE_OFFSET(sBlk.root_inode), paths);

	memset(created_inode, 0, sBlk.inodes * sizeof(char *));
	inode_number = 1;

	printf("%d inodes (%d blocks) to write\n\n", total_inodes,
		total_inodes - total_files + total_blocks);

	if(progress)
		enable_progress_bar();

	dir_scan(dest, SQUASHFS_INODE_BLK(sBlk.root_inode),
		SQUASHFS_INODE_OFFSET(sBlk.root_inode), paths);

	queue_put(to_writer, NULL);
	queue_get(from_writer);

	if(progress) {
		disable_progress_bar();
		progress_bar(sym_count + dev_count + fifo_count + cur_blocks,
			total_inodes - total_files + total_blocks, columns);
	}

	if(!lsonly) {
		printf("\n");
		printf("created %d files\n", file_count);
		printf("created %d directories\n", dir_count);
		printf("created %d symlinks\n", sym_count);
		printf("created %d devices\n", dev_count);
		printf("created %d fifos\n", fifo_count);
	}

	return 0;
}
