/*
 * mkcramfs - make a cramfs file system
 *
 * Copyright (C) 1999-2002 Transmeta Corporation
 *
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * If you change the disk format of cramfs, please update fs/cramfs/README.
 */

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <linux/cramfs_fs.h>
#include <zlib.h>

/* BRCM Modification start */
#include "7z.h"
/* BRCM Modification end */

/* Exit codes used by mkfs-type programs */
#define MKFS_OK          0	/* No errors */
#define MKFS_ERROR       8	/* Operational error */
#define MKFS_USAGE       16	/* Usage or syntax error */

/* The kernel only supports PAD_SIZE of 0 and 512. */
#define PAD_SIZE 512

/* The kernel assumes PAGE_CACHE_SIZE as block size. */
#define PAGE_CACHE_SIZE (4096)

/*
 * The longest filename component to allow for in the input directory tree.
 * ext2fs (and many others) allow up to 255 bytes.  A couple of filesystems
 * allow longer (e.g. smbfs 1024), but there isn't much use in supporting
 * >255-byte names in the input directory tree given that such names get
 * truncated to CRAMFS_MAXPATHLEN (252 bytes) when written to cramfs.
 *
 * Old versions of mkcramfs generated corrupted filesystems if any input
 * filenames exceeded CRAMFS_MAXPATHLEN (252 bytes), however old
 * versions of cramfsck seem to have been able to detect the corruption.
 */
#define MAX_INPUT_NAMELEN 255

/*
 * Maximum size fs you can create is roughly 256MB.  (The last file's
 * data must begin within 256MB boundary but can extend beyond that.)
 *
 * Note that if you want it to fit in a ROM then you're limited to what the
 * hardware and kernel can support.
 */
#define MAXFSLEN ((((1 << CRAMFS_OFFSET_WIDTH) - 1) << 2) /* offset */ \
		  + (1 << CRAMFS_SIZE_WIDTH) - 1 /* filesize */ \
		  + (1 << CRAMFS_SIZE_WIDTH) * 4 / PAGE_CACHE_SIZE /* block pointers */ )

static const char *progname = "mkcramfs";
static unsigned int blksize = PAGE_CACHE_SIZE;
static long total_blocks = 0, total_nodes = 1; /* pre-count the root node */
static int image_length = 0;

/*
 * If opt_holes is set, then mkcramfs can create explicit holes in the
 * data, which saves 26 bytes per hole (which is a lot smaller a
 * saving than most most filesystems).
 *
 * Note that kernels up to at least 2.3.39 don't support cramfs holes,
 * which is why this is turned off by default.
 *
 * If opt_verbose is 1, be verbose.  If it is higher, be even more verbose.
 */
static u32 opt_edition = 0;
static int opt_errors = 0;
static int opt_holes = 0;
static int opt_pad = 0;
static int opt_verbose = 0;
static char *opt_image = NULL;
static char *opt_name = NULL;
static int swap_endian = 0;
/* BRCM Modification start */
static int opt_gzip = 0;
static int opt_compression_level = 1;
/* BRCM Modification end */


static int warn_dev, warn_gid, warn_namelen, warn_skip, warn_size, warn_uid;

/* In-core version of inode / directory entry. */
struct entry {
	/* stats */
	unsigned char *name;
	unsigned int mode, size, uid, gid;

	/* these are only used for non-empty files */
	char *path;		/* always null except non-empty files */
	int fd;			/* temporarily open files while mmapped */

	/* FS data */
	void *uncompressed;
	/* points to other identical file */
	struct entry *same;
	unsigned int offset;		/* pointer to compressed data in archive */
	unsigned int dir_offset;	/* Where in the archive is the directory entry? */

	/* organization */
	struct entry *child; /* null for non-directories and empty directories */
	struct entry *next;
};

/* Input status of 0 to print help and exit without an error. */
static void usage(int status)
{
	FILE *stream = status ? stderr : stdout;

	fprintf(stream, "usage: %s [-h] [-e edition] [-i file] [-n name] dirname outfile\n"
		" -h         print this help\n"
		" -E         make all warnings errors (non-zero exit status)\n"
		" -e edition set edition number (part of fsid)\n"
		" -g         use gzip compression\n"
		" -i file    insert a file image into the filesystem (requires >= 2.4.0)\n"
                " -l level   set compression level\n"
		" -n name    set name of cramfs filesystem\n"
		" -p         pad by %d bytes for boot code\n"
		" -r         reverse endian-ness of filesystem\n"
		" -s         sort directory entries (old option, ignored)\n"
		" -v         be more verbose\n"
		" -z         make explicit holes (requires >= 2.3.39)\n"
		" dirname    root of the directory tree to be compressed\n"
		" outfile    output file\n", progname, PAD_SIZE);

	exit(status);
}

static void die(int status, int syserr, const char *fmt, ...)
{
	va_list arg_ptr;
	int save = errno;

	fflush(0);
	va_start(arg_ptr, fmt);
	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, arg_ptr);
	if (syserr) {
		fprintf(stderr, ": %s", strerror(save));
	}
	fprintf(stderr, "\n");
	va_end(arg_ptr);
	exit(status);
}

static void map_entry(struct entry *entry)
{
	if (entry->path) {
		entry->fd = open(entry->path, O_RDONLY);
		if (entry->fd < 0) {
			die(MKFS_ERROR, 1, "open failed: %s", entry->path);
		}
		entry->uncompressed = mmap(NULL, entry->size, PROT_READ, MAP_PRIVATE, entry->fd, 0);
		if (entry->uncompressed == MAP_FAILED) {
			die(MKFS_ERROR, 1, "mmap failed: %s", entry->path);
		}
	}
}

static void unmap_entry(struct entry *entry)
{
	if (entry->path) {
		if (munmap(entry->uncompressed, entry->size) < 0) {
			die(MKFS_ERROR, 1, "munmap failed: %s", entry->path);
		}
		close(entry->fd);
	}
}

static int find_identical_file(struct entry *orig, struct entry *newfile)
{
	if (orig == newfile)
		return 1;
	if (!orig)
		return 0;
	if (orig->size == newfile->size && (orig->path || orig->uncompressed))
	{
		map_entry(orig);
		map_entry(newfile);
		if (!memcmp(orig->uncompressed, newfile->uncompressed, orig->size))
		{
			newfile->same = orig;
			unmap_entry(newfile);
			unmap_entry(orig);
			return 1;
		}
		unmap_entry(newfile);
		unmap_entry(orig);
	}
	return (find_identical_file(orig->child, newfile) ||
		find_identical_file(orig->next, newfile));
}

static void eliminate_doubles(struct entry *root, struct entry *orig) {
	if (orig) {
		if (orig->size && (orig->path || orig->uncompressed))
			find_identical_file(root, orig);
		eliminate_doubles(root, orig->child);
		eliminate_doubles(root, orig->next);
	}
}

/*
 * We define our own sorting function instead of using alphasort which
 * uses strcoll and changes ordering based on locale information.
 */
static int cramsort (const void *a, const void *b)
{
	return strcmp ((*(const struct dirent **) a)->d_name,
		       (*(const struct dirent **) b)->d_name);
}

static unsigned int parse_directory(struct entry *root_entry, const char *name, struct entry **prev, loff_t *fslen_ub)
{
	struct dirent **dirlist;
	int totalsize = 0, dircount, dirindex;
	char *path, *endpath;
	size_t len = strlen(name);

	/* Set up the path. */
	/* TODO: Reuse the parent's buffer to save memcpy'ing and duplication. */
	path = (char *)malloc(len + 1 + MAX_INPUT_NAMELEN + 1);
	if (!path) {
		die(MKFS_ERROR, 1, "malloc failed");
	}
	memcpy(path, name, len);
	endpath = path + len;
	*endpath = '/';
	endpath++;

	/* read in the directory and sort */
	dircount = scandir(name, &dirlist, 0, cramsort);

	if (dircount < 0) {
		die(MKFS_ERROR, 1, "scandir failed: %s", name);
	}

	/* process directory */
	for (dirindex = 0; dirindex < dircount; dirindex++) {
		struct dirent *dirent;
		struct entry *entry;
		struct stat st;
		int size;
		size_t namelen;

		dirent = dirlist[dirindex];

		/* Ignore "." and ".." - we won't be adding them to the archive */
		if (dirent->d_name[0] == '.') {
			if (dirent->d_name[1] == '\0')
				continue;
			if (dirent->d_name[1] == '.') {
				if (dirent->d_name[2] == '\0')
					continue;
			}
		}
		namelen = strlen(dirent->d_name);
		if (namelen > MAX_INPUT_NAMELEN) {
			die(MKFS_ERROR, 0,
				"very long (%u bytes) filename found: %s\n"
				"please increase MAX_INPUT_NAMELEN in mkcramfs.c and recompile",
				namelen, dirent->d_name);
		}
		memcpy(endpath, dirent->d_name, namelen + 1);

		if (lstat(path, &st) < 0) {
			warn_skip = 1;
			continue;
		}
		entry = (struct entry *)calloc(1, sizeof(struct entry));
		if (!entry) {
			die(MKFS_ERROR, 1, "calloc failed");
		}
		entry->name = (unsigned char *)strdup(dirent->d_name);
		if (!entry->name) {
			die(MKFS_ERROR, 1, "strdup failed");
		}
		/* truncate multi-byte UTF-8 filenames on character boundary */
		if (namelen > CRAMFS_MAXPATHLEN) {
			namelen = CRAMFS_MAXPATHLEN;
			warn_namelen = 1;
			/* the first lost byte must not be a trail byte */
			while ((entry->name[namelen] & 0xc0) == 0x80) {
				namelen--;
				/* are we reasonably certain it was UTF-8 ? */
				if (entry->name[namelen] < 0x80 || !namelen) {
					die(MKFS_ERROR, 0, "cannot truncate filenames not encoded in UTF-8");
				}
			}
			entry->name[namelen] = '\0';
		}
		entry->mode = st.st_mode;
		entry->size = st.st_size;
		entry->uid = st.st_uid;
		if (entry->uid >= 1 << CRAMFS_UID_WIDTH)
			warn_uid = 1;
		entry->gid = st.st_gid;
		if (entry->gid >= 1 << CRAMFS_GID_WIDTH)
			/* TODO: We ought to replace with a default
			   gid instead of truncating; otherwise there
			   are security problems.  Maybe mode should
			   be &= ~070.  Same goes for uid once Linux
			   supports >16-bit uids. */
			warn_gid = 1;
		size = sizeof(struct cramfs_inode) + ((namelen + 3) & ~3);
		*fslen_ub += size;
		if (S_ISDIR(st.st_mode)) {
			entry->size = parse_directory(root_entry, path, &entry->child, fslen_ub);
		} else if (S_ISREG(st.st_mode)) {
			if (entry->size) {
				if (access(path, R_OK) < 0) {
					warn_skip = 1;
					continue;
				}
				entry->path = strdup(path);
				if (!entry->path) {
					die(MKFS_ERROR, 1, "strdup failed");
				}
				if ((entry->size >= 1 << CRAMFS_SIZE_WIDTH)) {
					warn_size = 1;
					entry->size = (1 << CRAMFS_SIZE_WIDTH) - 1;
				}
			}
		} else if (S_ISLNK(st.st_mode)) {
			entry->uncompressed = malloc(entry->size);
			if (!entry->uncompressed) {
				die(MKFS_ERROR, 1, "malloc failed");
			}
			if (readlink(path, (char *)entry->uncompressed, entry->size) < 0) {
				warn_skip = 1;
				continue;
			}
		} else if (S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) {
			/* maybe we should skip sockets */
			entry->size = 0;
		} else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) {
			entry->size = st.st_rdev;
			if (entry->size & -(1<<CRAMFS_SIZE_WIDTH))
				warn_dev = 1;
		} else {
			die(MKFS_ERROR, 0, "bogus file type: %s", entry->name);
		}

		if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
			int blocks = ((entry->size - 1) / blksize + 1);

			/* block pointers & data expansion allowance + data */
			if (entry->size)
				*fslen_ub += (4+26)*blocks + entry->size + 3;
		}

		/* Link it into the list */
		*prev = entry;
		prev = &entry->next;
		totalsize += size;
	}
	free(path);
	free(dirlist);		/* allocated by scandir() with malloc() */
	return totalsize;
}

/* routines to swap endianness/bitfields in inode/superblock block data */
static void fix_inode(struct cramfs_inode *inode)
{
#define wswap(x)    (((x)>>24) | (((x)>>8)&0xff00) | (((x)&0xff00)<<8) | (((x)&0xff)<<24))
	/* attempt #2 */
	inode->mode = (inode->mode >> 8) | ((inode->mode&0xff)<<8);
	inode->uid = (inode->uid >> 8) | ((inode->uid&0xff)<<8);
	inode->size = (inode->size >> 16) | (inode->size&0xff00) |
		((inode->size&0xff)<<16);
	((u32*)inode)[2] = wswap(inode->offset | (inode->namelen<<26));
}

static void fix_offset(struct cramfs_inode *inode, u32 offset)
{
	u32 tmp = wswap(((u32*)inode)[2]);
	((u32*)inode)[2] = wswap((offset >> 2) | (tmp&0xfc000000));
}

static void fix_block_pointer(u32 *p)
{
	*p = wswap(*p);
}

static void fix_super(struct cramfs_super *super)
{
	u32 *p = (u32*)super;

	/* fix superblock fields */
	p[0] = wswap(p[0]);	/* magic */
	p[1] = wswap(p[1]);	/* size */
	p[2] = wswap(p[2]);	/* flags */
	p[3] = wswap(p[3]);	/* future */

	/* fix filesystem info fields */
	p = (u32*)&super->fsid;
	p[0] = wswap(p[0]);	/* crc */
	p[1] = wswap(p[1]);	/* edition */
	p[2] = wswap(p[2]);	/* blocks */
	p[3] = wswap(p[3]);	/* files */

	fix_inode(&super->root);
#undef wswap
}

/* Returns sizeof(struct cramfs_super), which includes the root inode. */
static unsigned int write_superblock(struct entry *root, char *base, int size)
{
	struct cramfs_super *super = (struct cramfs_super *) base;
	unsigned int offset = sizeof(struct cramfs_super) + image_length;

	offset += opt_pad;	/* 0 if no padding */

	super->magic = CRAMFS_MAGIC;
	super->flags = CRAMFS_FLAG_FSID_VERSION_2 | CRAMFS_FLAG_SORTED_DIRS;
	if (opt_holes)
		super->flags |= CRAMFS_FLAG_HOLES;
	if (image_length > 0)
		super->flags |= CRAMFS_FLAG_SHIFTED_ROOT_OFFSET;
	super->size = size;
	memcpy(super->signature, CRAMFS_SIGNATURE, sizeof(super->signature));

	super->fsid.crc = crc32(0L, Z_NULL, 0);
	super->fsid.edition = opt_edition;
	super->fsid.blocks = total_blocks;
	super->fsid.files = total_nodes;

	memset(super->name, 0x00, sizeof(super->name));
	if (opt_name)
		strncpy((char *)super->name, opt_name, sizeof(super->name));
	else
		strncpy((char *)super->name, "Compressed", sizeof(super->name));

	super->root.mode = root->mode;
	super->root.uid = root->uid;
	super->root.gid = root->gid;
	super->root.size = root->size;
	super->root.offset = offset >> 2;
	if (swap_endian) fix_super(super);

	return offset;
}

static void set_data_offset(struct entry *entry, char *base, unsigned long offset)
{
	struct cramfs_inode *inode = (struct cramfs_inode *) (base + entry->dir_offset);

	if ((offset & 3) != 0) {
		die(MKFS_ERROR, 0, "illegal offset of %lu bytes", offset);
	}
	if (offset >= (1 << (2 + CRAMFS_OFFSET_WIDTH))) {
		die(MKFS_ERROR, 0, "filesystem too big");
	}
	if (swap_endian)
		fix_offset(inode, offset);
	else
		inode->offset = (offset >> 2);
}

/*
 * TODO: Does this work for chars >= 0x80?  Most filesystems use UTF-8
 * encoding for filenames, whereas the console is a single-byte
 * character set like iso-latin-1.
 */
static void print_node(struct entry *e)
{
	char info[10];
	char type = '?';

	if (S_ISREG(e->mode)) type = 'f';
	else if (S_ISDIR(e->mode)) type = 'd';
	else if (S_ISLNK(e->mode)) type = 'l';
	else if (S_ISCHR(e->mode)) type = 'c';
	else if (S_ISBLK(e->mode)) type = 'b';
	else if (S_ISFIFO(e->mode)) type = 'p';
	else if (S_ISSOCK(e->mode)) type = 's';

	if (S_ISCHR(e->mode) || (S_ISBLK(e->mode))) {
		/* major/minor numbers can be as high as 2^12 or 4096 */
		snprintf(info, 10, "%4d,%4d", major(e->size), minor(e->size));
	}
	else {
		/* size be as high as 2^24 or 16777216 */
		snprintf(info, 10, "%9d", e->size);
	}

	printf("%c %04o %s %5d:%-3d %s\n",
	       type, e->mode & ~S_IFMT, info, e->uid, e->gid, e->name);
}

/*
 * We do a width-first printout of the directory
 * entries, using a stack to remember the directories
 * we've seen.
 */
static unsigned int write_directory_structure(struct entry *entry, char *base, unsigned int offset)
{
	int stack_entries = 0;
	int stack_size = 64;
	struct entry **entry_stack;

	entry_stack = (struct entry **)malloc(stack_size * sizeof(struct entry *));
	if (!entry_stack) {
		die(MKFS_ERROR, 1, "malloc failed");
	}

	if (opt_verbose) {
		printf("root:\n");
	}

	for (;;) {
		int dir_start = stack_entries;
		while (entry) {
			struct cramfs_inode *inode = (struct cramfs_inode *) (base + offset);
			size_t len = strlen((const char *)entry->name);

			entry->dir_offset = offset;

			inode->mode = entry->mode;
			inode->uid = entry->uid;
			inode->gid = entry->gid;
			inode->size = entry->size;
			inode->offset = 0;
			/* Non-empty directories, regfiles and symlinks will
			   write over inode->offset later. */

			offset += sizeof(struct cramfs_inode);
			total_nodes++;	/* another node */
			memcpy(base + offset, entry->name, len);
			/* Pad up the name to a 4-byte boundary */
			while (len & 3) {
				*(base + offset + len) = '\0';
				len++;
			}
			inode->namelen = len >> 2;
			offset += len;

			if (opt_verbose)
				print_node(entry);

			if (entry->child) {
				if (stack_entries >= stack_size) {
					stack_size *= 2;
					entry_stack = (struct entry **)realloc(entry_stack, stack_size * sizeof(struct entry *));
					if (!entry_stack) {
						die(MKFS_ERROR, 1, "realloc failed");
					}
				}
				entry_stack[stack_entries] = entry;
				stack_entries++;
			}
			entry = entry->next;
			if (swap_endian) fix_inode(inode);
		}

		/*
		 * Reverse the order the stack entries pushed during
		 * this directory, for a small optimization of disk
		 * access in the created fs.  This change makes things
		 * `ls -UR' order.
		 */
		{
			struct entry **lo = entry_stack + dir_start;
			struct entry **hi = entry_stack + stack_entries;
			struct entry *tmp;

			while (lo < --hi) {
				tmp = *lo;
				*lo++ = *hi;
				*hi = tmp;
			}
		}

		/* Pop a subdirectory entry from the stack, and recurse. */
		if (!stack_entries)
			break;
		stack_entries--;
		entry = entry_stack[stack_entries];

		set_data_offset(entry, base, offset);
		if (opt_verbose) {
			printf("%s:\n", entry->name);
		}
		entry = entry->child;
	}
	free(entry_stack);
	return offset;
}

static int is_zero(char const *begin, unsigned len)
{
	/* Returns non-zero iff the first LEN bytes from BEGIN are all NULs. */
	return (len-- == 0 ||
		(begin[0] == '\0' &&
		 (len-- == 0 ||
		  (begin[1] == '\0' &&
		   (len-- == 0 ||
		    (begin[2] == '\0' &&
		     (len-- == 0 ||
		      (begin[3] == '\0' &&
		       memcmp(begin, begin + 4, len) == 0))))))));
}

/* BRCM Modification start */
static void dumpHex(unsigned char *start, int len)
{
    unsigned char *ptr = start,
    *end = start + len;
    int i;

    while (ptr < end) {
    for (i=0;i<16 && ptr < end;i++) 
              printf("%4x", *ptr++);
    printf("\n");      
    }
}
/* BRCM Modification end */

/*
 * One 4-byte pointer per block and then the actual blocked
 * output. The first block does not need an offset pointer,
 * as it will start immediately after the pointer block;
 * so the i'th pointer points to the end of the i'th block
 * (i.e. the start of the (i+1)'th block or past EOF).
 *
 * Note that size > 0, as a zero-sized file wouldn't ever
 * have gotten here in the first place.
 */
static unsigned int do_compress(char *base, unsigned int offset, char const *name, char *uncompressed, unsigned int size)
{
	unsigned long original_size = size;
	unsigned long original_offset = offset;
	unsigned long new_size;
	unsigned long blocks = (size - 1) / blksize + 1;
	unsigned long curr = offset + 4 * blocks;
	int change;
        static int one = 0;

	total_blocks += blocks;

	do {
		unsigned long len = 2 * blksize;
		unsigned int input = size;
		int err;

		if (input > blksize)
			input = blksize;
		size -= input;
		if (!(opt_holes && is_zero (uncompressed, input))) {
                /* BRCM Modification start */
		        if (opt_gzip) {
			        err = compress2((Bytef *)base + curr, &len, (const Bytef *)uncompressed, input, Z_BEST_COMPRESSION);
			        if (err != Z_OK) {
				        die(MKFS_ERROR, 0, "compression error: %s", zError(err));
			        }
			} else {
			        unsigned lzma_algo;
                                unsigned lzma_dictsize;
                                unsigned lzma_fastbytes;                        
                                switch (opt_compression_level) {
                                        case 1 :
                                                        lzma_algo = 1;
                                                        lzma_dictsize = 1 << 20;
                                                        lzma_fastbytes = 64;
                                                        break;
                                        case 2 :
                                                        lzma_algo = 2;
                                                        lzma_dictsize = 1 << 22;
                                                        lzma_fastbytes = 128;
                                                        break;
                                        case 3 :
                                                        lzma_algo = 2;
                                                        lzma_dictsize = 1 << 24;
                                                        lzma_fastbytes = 255;
                                                        break;
                                        default :
                                                        die(MKFS_ERROR, 0, "Invalid LZMA compression level. Must be 1,2,3.");
                                }
                                len = 8*PAGE_CACHE_SIZE;
#if 0                                
                                        if (one < 2) {
                                                 printf("Input data, size =%d\n",input);
                                                 dumpHex((unsigned char *)uncompressed,input);
                                        }
#endif                                
                                err = compress_lzma_7z((const unsigned char*) uncompressed, 
                                                                     (unsigned) input, 
                                                                     (unsigned char*) (base+curr), 
                                                                     (unsigned &) len, 
                                                                     lzma_algo, 
                                                                     lzma_dictsize, 
                                                                     lzma_fastbytes);
                                if (!err) {
                                        /* this should NEVER happen */
                                        die(MKFS_ERROR, 0, "Internal error - LZMA compression failed.\n");
                                 }
#if 0                                 
                                 if (one < 2 ) {
                                        printf("Output data, size =%d\n",len);
                                        dumpHex((unsigned char *)(base+curr),len);
                                 }       
                                 if (one < 2) {
                                        printf("Input data, size =%d\n",input);
                                        dumpHex((unsigned char *)uncompressed,input);
                                 }
                     
                                 one++;
#endif                                
#if 0
				 printf("verify...\n");
				 unsigned char verify[4096];
				 memset((void *)verify,0,4096);
				 err = decompress_lzma_7z((unsigned char*) (base+curr), len, verify, 23*1024*1024);
				 if (err != 0) {
					die(MKFS_ERROR, 0, "Internal error - LZMA decompression failed.\n");
				 }
				 printf("Verifying...input=%d,",input);
				 if (memcmp(verify,uncompressed,input) != 0) {
					die(MKFS_ERROR, 0, "Internal error - LZMA compression/decompression not matching.\n");
				 }
#endif				 
                        }					
                        if (opt_verbose) {
			    printf("(%ld) ",len);
                        } else {
                            printf(".");
                        }
                        /* BRCM Modification end*/   
			curr += len;
		}
		uncompressed += input;

		if (len > blksize*2) {
			/* (I don't think this can happen with zlib.) */
			die(MKFS_ERROR, 0, "AIEEE: block \"compressed\" to > 2*blocklength (%ld)", len);
		}

		*(u32 *) (base + offset) = curr;
		if (swap_endian) fix_block_pointer((u32*)(base + offset));
		offset += 4;
	} while (size);

	curr = (curr + 3) & ~3;
	new_size = curr - original_offset;
	/* TODO: Arguably, original_size in these 2 lines should be
	   st_blocks * 512.  But if you say that then perhaps
	   administrative data should also be included in both. */
	change = new_size - original_size;
	if (opt_verbose > 1) {
		printf("%6.2f%% (%+d bytes)\t%s\n",
		       (change * 100) / (double) original_size, change, name);
	}

	return curr;
}


/*
 * Traverse the entry tree, writing data for every item that has
 * non-null entry->path (i.e. every non-empty regfile) and non-null
 * entry->uncompressed (i.e. every symlink).
 */
static unsigned int write_data(struct entry *entry, char *base, unsigned int offset)
{
	do {
		if (entry->path || entry->uncompressed) {
			if (entry->same) {
				set_data_offset(entry, base, entry->same->offset);
				entry->offset = entry->same->offset;
			}
			else {
				set_data_offset(entry, base, offset);
				entry->offset = offset;
				map_entry(entry);
				offset = do_compress(base, offset, (const char *)entry->name, (char *)entry->uncompressed, entry->size);
				unmap_entry(entry);
			}
		}
		else if (entry->child)
			offset = write_data(entry->child, base, offset);
		entry=entry->next;
	} while (entry);
	return offset;
}

static unsigned int write_file(char *file, char *base, unsigned int offset)
{
	int fd;
	char *buf;

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		die(MKFS_ERROR, 1, "open failed: %s", file);
	}
	buf = (char *)mmap(NULL, image_length, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == MAP_FAILED) {
		die(MKFS_ERROR, 1, "mmap failed");
	}
	memcpy(base + offset, buf, image_length);
	munmap(buf, image_length);
	close (fd);
	/* Pad up the image_length to a 4-byte boundary */
	while (image_length & 3) {
		*(base + offset + image_length) = '\0';
		image_length++;
	}
	return (offset + image_length);
}

int main(int argc, char **argv)
{
	struct stat st;		/* used twice... */
	struct entry *root_entry;
	char *rom_image;
	ssize_t offset, written;
	int fd;
	/* initial guess (upper-bound) of required filesystem size */
	loff_t fslen_ub = sizeof(struct cramfs_super);
	char const *dirname, *outfile;
	u32 crc;
	int c;			/* for getopt */
	char *ep;		/* for strtoul */

	total_blocks = 0;

	if (argc)
		progname = argv[0];

	/* command line options */
	while ((c = getopt(argc, argv, "hEe:i:l:n:gprsvz")) != EOF) {
		switch (c) {
		case 'h':
			usage(MKFS_OK);
		case 'E':
			opt_errors = 1;
			break;
		case 'e':
			errno = 0;
			opt_edition = strtoul(optarg, &ep, 10);
			if (errno || optarg[0] == '\0' || *ep != '\0')
				usage(MKFS_USAGE);
			break;
		case 'g':
		        opt_gzip = 1;
			printf("Using gzip to compress instead of default lzma\n");
			break;
		case 'i':
			opt_image = optarg;
			if (lstat(opt_image, &st) < 0) {
				die(MKFS_ERROR, 1, "lstat failed: %s", opt_image);
			}
			image_length = st.st_size; /* may be padded later */
			fslen_ub += (image_length + 3); /* 3 is for padding */
			break;
                case 'l':
                        errno = 0;
                        opt_compression_level = strtoul(optarg, &ep, 10);
			if (errno || optarg[0] == '\0' || *ep != '\0')
				usage(MKFS_USAGE);                        
                        break;
		case 'n':
			opt_name = optarg;
			break;
		case 'p':
			opt_pad = PAD_SIZE;
			fslen_ub += PAD_SIZE;
			break;
		case 'r':
			swap_endian = 1;
			printf("Swapping filesystem endian-ness\n");
			break;
		case 's':
			/* old option, ignored */
			break;
		case 'v':
			opt_verbose++;
			break;
		case 'z':
			opt_holes = 1;
			break;
		}
	}

	if ((argc - optind) != 2)
		usage(MKFS_USAGE);
	dirname = argv[optind];
	outfile = argv[optind + 1];

	if (stat(dirname, &st) < 0) {
		die(MKFS_USAGE, 1, "stat failed: %s", dirname);
	}
	fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		die(MKFS_USAGE, 1, "open failed: %s", outfile);
	}

	root_entry = (struct entry *)calloc(1, sizeof(struct entry));
	if (!root_entry) {
		die(MKFS_ERROR, 1, "calloc failed");
	}
	root_entry->mode = st.st_mode;
	root_entry->uid = st.st_uid;
	root_entry->gid = st.st_gid;

	root_entry->size = parse_directory(root_entry, dirname, &root_entry->child, &fslen_ub);

	/* always allocate a multiple of blksize bytes because that's
	   what we're going to write later on */
	fslen_ub = ((fslen_ub - 1) | (blksize - 1)) + 1;

	if (fslen_ub > MAXFSLEN) {
		fprintf(stderr,
			"warning: estimate of required size (upper bound) is %LdMB, but maximum image size is %uMB, we might die prematurely\n",
			fslen_ub >> 20,
			MAXFSLEN >> 20);
		fslen_ub = MAXFSLEN;
	}

	/* find duplicate files. TODO: uses the most inefficient algorithm
	   possible. */
	eliminate_doubles(root_entry, root_entry);

	/* TODO: Why do we use a private/anonymous mapping here
	   followed by a write below, instead of just a shared mapping
	   and a couple of ftruncate calls?  Is it just to save us
	   having to deal with removing the file afterwards?  If we
	   really need this huge anonymous mapping, we ought to mmap
	   in smaller chunks, so that the user doesn't need nn MB of
	   RAM free.  If the reason is to be able to write to
	   un-mmappable block devices, then we could try shared mmap
	   and revert to anonymous mmap if the shared mmap fails. */
	rom_image = (char *)mmap(NULL, fslen_ub?fslen_ub:1, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (rom_image == MAP_FAILED) {
		die(MKFS_ERROR, 1, "mmap failed");
	}

	/* Skip the first opt_pad bytes for boot loader code */
	offset = opt_pad;
	memset(rom_image, 0x00, opt_pad);

	/* Skip the superblock and come back to write it later. */
	offset += sizeof(struct cramfs_super);

	/* Insert a file image. */
	if (opt_image) {
		printf("Including: %s\n", opt_image);
		offset = write_file(opt_image, rom_image, offset);
	}

	offset = write_directory_structure(root_entry->child, rom_image, offset);
	printf("Directory data: %d bytes\n", offset);

	printf("Compressing directory and files...\n");
	offset = write_data(root_entry, rom_image, offset);

	/* We always write a multiple of blksize bytes, so that
	   losetup works. */
	offset = ((offset - 1) | (blksize - 1)) + 1;
	printf("\nEverything: %d kilobytes\n", offset >> 10);

	/* Write the superblock now that we can fill in all of the fields. */
	write_superblock(root_entry, rom_image+opt_pad, offset);
	printf("Super block: %d bytes\n", sizeof(struct cramfs_super));

	/* Put the checksum in. */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (const Bytef *)(rom_image+opt_pad), (offset-opt_pad));
	((struct cramfs_super *) (rom_image+opt_pad))->fsid.crc = crc;
	printf("CRC: %x\n", crc);

	/* Check to make sure we allocated enough space. */
	if (fslen_ub < offset) {
		die(MKFS_ERROR, 0, "not enough space allocated for ROM image (%Ld allocated, %d used)", fslen_ub, offset);
	}

	written = write(fd, rom_image, offset);
	if (written < 0) {
		die(MKFS_ERROR, 1, "write failed");
	}
	if (offset != written) {
		die(MKFS_ERROR, 0, "ROM image write failed (wrote %d of %d bytes)", written, offset);
	}

	/* (These warnings used to come at the start, but they scroll off the
	   screen too quickly.) */
	if (warn_namelen)
		fprintf(stderr, /* bytes, not chars: think UTF-8. */
			"warning: filenames truncated to %d bytes (possibly less if multi-byte UTF-8)\n",
			CRAMFS_MAXPATHLEN);
	if (warn_skip)
		fprintf(stderr, "warning: files were skipped due to errors\n");
	if (warn_size)
		fprintf(stderr,
			"warning: file sizes truncated to %luMB (minus 1 byte)\n",
			1L << (CRAMFS_SIZE_WIDTH - 20));
	if (warn_uid) /* (not possible with current Linux versions) */
		fprintf(stderr,
			"warning: uids truncated to %u bits (this may be a security concern)\n",
			CRAMFS_UID_WIDTH);
	/*
	if (warn_gid)
		fprintf(stderr,
			"warning: gids truncated to %u bits (this may be a security concern)\n",
			CRAMFS_GID_WIDTH);
	*/
	if (warn_dev)
		fprintf(stderr,
			"WARNING: device numbers truncated to %u bits (this almost certainly means\n"
			"that some device files will be wrong)\n",
			CRAMFS_OFFSET_WIDTH);
	if (opt_errors &&
	    (warn_namelen||warn_skip||warn_size||warn_uid||warn_gid||warn_dev))
		exit(MKFS_ERROR);

	exit(MKFS_OK);
}

/*
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
