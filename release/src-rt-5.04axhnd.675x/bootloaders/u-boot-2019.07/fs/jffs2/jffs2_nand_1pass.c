#include <common.h>

#include <malloc.h>
#include <linux/stat.h>
#include <linux/time.h>

#include <jffs2/jffs2.h>
#include <jffs2/jffs2_1pass.h>
#include <nand.h>

#include "jffs2_nand_private.h"

#define	NODE_CHUNK	1024	/* size of memory allocation chunk in b_nodes */

/* Debugging switches */
#undef	DEBUG_DIRENTS		/* print directory entry list after scan */
#undef	DEBUG_FRAGMENTS		/* print fragment list after scan */
#undef	DEBUG			/* enable debugging messages */

#ifdef  DEBUG
# define DEBUGF(fmt,args...)	printf(fmt ,##args)
#else
# define DEBUGF(fmt,args...)
#endif

static struct mtd_info *mtd;

/* Compression names */
static char *compr_names[] = {
	"NONE",
	"ZERO",
	"RTIME",
	"RUBINMIPS",
	"COPY",
	"DYNRUBIN",
	"ZLIB",
#if defined(CONFIG_JFFS2_LZO)
	"LZO",
#endif
};

/* Spinning wheel */
static char spinner[] = { '|', '/', '-', '\\' };

/* Memory management */
struct mem_block {
	unsigned index;
	struct mem_block *next;
	char nodes[0];
};

static void
free_nodes(struct b_list *list)
{
	while (list->listMemBase != NULL) {
		struct mem_block *next = list->listMemBase->next;
		free(list->listMemBase);
		list->listMemBase = next;
	}
}

static struct b_node *
add_node(struct b_list *list, int size)
{
	u32 index = 0;
	struct mem_block *memBase;
	struct b_node *b;

	memBase = list->listMemBase;
	if (memBase != NULL)
		index = memBase->index;

	if (memBase == NULL || index >= NODE_CHUNK) {
		/* we need more space before we continue */
		memBase = mmalloc(sizeof(struct mem_block) + NODE_CHUNK * size);
		if (memBase == NULL) {
			putstr("add_node: malloc failed\n");
			return NULL;
		}
		memBase->next = list->listMemBase;
		index = 0;
	}
	/* now we have room to add it. */
	b = (struct b_node *)&memBase->nodes[size * index];
	index ++;

	memBase->index = index;
	list->listMemBase = memBase;
	list->listCount++;
	return b;
}

static struct b_node *
insert_node(struct b_list *list, struct b_node *new)
{
#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
	struct b_node *b, *prev;

	if (list->listTail != NULL && list->listCompare(new, list->listTail))
		prev = list->listTail;
	else if (list->listLast != NULL && list->listCompare(new, list->listLast))
		prev = list->listLast;
	else
		prev = NULL;

	for (b = (prev ? prev->next : list->listHead);
	     b != NULL && list->listCompare(new, b);
	     prev = b, b = b->next) {
		list->listLoops++;
	}
	if (b != NULL)
		list->listLast = prev;

	if (b != NULL) {
		new->next = b;
		if (prev != NULL)
			prev->next = new;
		else
			list->listHead = new;
	} else
#endif
	{
		new->next = (struct b_node *) NULL;
		if (list->listTail != NULL) {
			list->listTail->next = new;
			list->listTail = new;
		} else {
			list->listTail = list->listHead = new;
		}
	}

	return new;
}

static struct b_node *
insert_inode(struct b_list *list, struct jffs2_raw_inode *node, u32 offset)
{
	struct b_inode *new;

	if (!(new = (struct b_inode *)add_node(list, sizeof(struct b_inode)))) {
		putstr("add_node failed!\r\n");
		return NULL;
	}
	new->offset = offset;
	new->version = node->version;
	new->ino = node->ino;
	new->isize = node->isize;
	new->csize = node->csize;

	return insert_node(list, (struct b_node *)new);
}

static struct b_node *
insert_dirent(struct b_list *list, struct jffs2_raw_dirent *node, u32 offset)
{
	struct b_dirent *new;

	if (!(new = (struct b_dirent *)add_node(list, sizeof(struct b_dirent)))) {
		putstr("add_node failed!\r\n");
		return NULL;
	}
	new->offset = offset;
	new->version = node->version;
	new->pino = node->pino;
	new->ino = node->ino;
	new->nhash = full_name_hash(node->name, node->nsize);
	new->nsize = node->nsize;
	new->type = node->type;

	return insert_node(list, (struct b_node *)new);
}

#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
/* Sort data entries with the latest version last, so that if there
 * is overlapping data the latest version will be used.
 */
static int compare_inodes(struct b_node *new, struct b_node *old)
{
	struct jffs2_raw_inode ojNew;
	struct jffs2_raw_inode ojOld;
	struct jffs2_raw_inode *jNew =
		(struct jffs2_raw_inode *)get_fl_mem(new->offset, sizeof(ojNew), &ojNew);
	struct jffs2_raw_inode *jOld =
		(struct jffs2_raw_inode *)get_fl_mem(old->offset, sizeof(ojOld), &ojOld);

	return jNew->version > jOld->version;
}

/* Sort directory entries so all entries in the same directory
 * with the same name are grouped together, with the latest version
 * last. This makes it easy to eliminate all but the latest version
 * by marking the previous version dead by setting the inode to 0.
 */
static int compare_dirents(struct b_node *new, struct b_node *old)
{
	struct jffs2_raw_dirent ojNew;
	struct jffs2_raw_dirent ojOld;
	struct jffs2_raw_dirent *jNew =
		(struct jffs2_raw_dirent *)get_fl_mem(new->offset, sizeof(ojNew), &ojNew);
	struct jffs2_raw_dirent *jOld =
		(struct jffs2_raw_dirent *)get_fl_mem(old->offset, sizeof(ojOld), &ojOld);
	int cmp;

	/* ascending sort by pino */
	if (jNew->pino != jOld->pino)
		return jNew->pino > jOld->pino;

	/* pino is the same, so use ascending sort by nsize, so
	 * we don't do strncmp unless we really must.
	 */
	if (jNew->nsize != jOld->nsize)
		return jNew->nsize > jOld->nsize;

	/* length is also the same, so use ascending sort by name
	 */
	cmp = strncmp(jNew->name, jOld->name, jNew->nsize);
	if (cmp != 0)
		return cmp > 0;

	/* we have duplicate names in this directory, so use ascending
	 * sort by version
	 */
	if (jNew->version > jOld->version) {
		/* since jNew is newer, we know jOld is not valid, so
		 * mark it with inode 0 and it will not be used
		 */
		jOld->ino = 0;
		return 1;
	}

	return 0;
}
#endif

static u32
jffs_init_1pass_list(struct part_info *part)
{
	struct b_lists *pL;

	if (part->jffs2_priv != NULL) {
		pL = (struct b_lists *)part->jffs2_priv;
		free_nodes(&pL->frag);
		free_nodes(&pL->dir);
		free(pL);
	}
	if (NULL != (part->jffs2_priv = malloc(sizeof(struct b_lists)))) {
		pL = (struct b_lists *)part->jffs2_priv;

		memset(pL, 0, sizeof(*pL));
#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
		pL->dir.listCompare = compare_dirents;
		pL->frag.listCompare = compare_inodes;
#endif
	}
	return 0;
}

/* find the inode from the slashless name given a parent */
static long
jffs2_1pass_read_inode(struct b_lists *pL, u32 ino, char *dest,
		       struct stat *stat)
{
	struct b_inode *jNode;
	u32 totalSize = 0;
	u32 latestVersion = 0;
	long ret;

#ifdef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
	/* Find file size before loading any data, so fragments that
	 * start past the end of file can be ignored. A fragment
	 * that is partially in the file is loaded, so extra data may
	 * be loaded up to the next 4K boundary above the file size.
	 * This shouldn't cause trouble when loading kernel images, so
	 * we will live with it.
	 */
	for (jNode = (struct b_inode *)pL->frag.listHead; jNode; jNode = jNode->next) {
		if ((ino == jNode->ino)) {
			/* get actual file length from the newest node */
			if (jNode->version >= latestVersion) {
				totalSize = jNode->isize;
				latestVersion = jNode->version;
			}
		}
	}
#endif

	for (jNode = (struct b_inode *)pL->frag.listHead; jNode; jNode = jNode->next) {
		if ((ino != jNode->ino))
			continue;
#ifndef CONFIG_SYS_JFFS2_SORT_FRAGMENTS
		/* get actual file length from the newest node */
		if (jNode->version >= latestVersion) {
			totalSize = jNode->isize;
			latestVersion = jNode->version;
		}
#endif
		if (dest || stat) {
			char *src, *dst;
			char data[4096 + sizeof(struct jffs2_raw_inode)];
			struct jffs2_raw_inode *inode;
			size_t len;

			inode = (struct jffs2_raw_inode *)&data;
			len = sizeof(struct jffs2_raw_inode);
			if (dest)
				len += jNode->csize;
			nand_read(mtd, jNode->offset, &len, inode);
			/* ignore data behind latest known EOF */
			if (inode->offset > totalSize)
				continue;

			if (stat) {
				stat->st_mtime = inode->mtime;
				stat->st_mode = inode->mode;
				stat->st_ino = inode->ino;
				stat->st_size = totalSize;
			}

			if (!dest)
				continue;

			src = ((char *) inode) + sizeof(struct jffs2_raw_inode);
			dst = (char *) (dest + inode->offset);

			switch (inode->compr) {
			case JFFS2_COMPR_NONE:
				ret = 0;
				memcpy(dst, src, inode->dsize);
				break;
			case JFFS2_COMPR_ZERO:
				ret = 0;
				memset(dst, 0, inode->dsize);
				break;
			case JFFS2_COMPR_RTIME:
				ret = 0;
				rtime_decompress(src, dst, inode->csize, inode->dsize);
				break;
			case JFFS2_COMPR_DYNRUBIN:
				/* this is slow but it works */
				ret = 0;
				dynrubin_decompress(src, dst, inode->csize, inode->dsize);
				break;
			case JFFS2_COMPR_ZLIB:
				ret = zlib_decompress(src, dst, inode->csize, inode->dsize);
				break;
#if defined(CONFIG_JFFS2_LZO)
			case JFFS2_COMPR_LZO:
				ret = lzo_decompress(src, dst, inode->csize, inode->dsize);
				break;
#endif
			default:
				/* unknown */
				putLabeledWord("UNKNOWN COMPRESSION METHOD = ", inode->compr);
				return -1;
			}
		}
	}

	return totalSize;
}

/* find the inode from the slashless name given a parent */
static u32
jffs2_1pass_find_inode(struct b_lists * pL, const char *name, u32 pino)
{
	struct b_dirent *jDir;
	int len = strlen(name);	/* name is assumed slash free */
	unsigned int nhash = full_name_hash(name, len);
	u32 version = 0;
	u32 inode = 0;

	/* we need to search all and return the inode with the highest version */
	for (jDir = (struct b_dirent *)pL->dir.listHead; jDir; jDir = jDir->next) {
		if ((pino == jDir->pino) && (jDir->ino) &&	/* 0 for unlink */
		    (len == jDir->nsize) && (nhash == jDir->nhash)) {
			/* TODO: compare name */
			if (jDir->version < version)
				continue;

			if (jDir->version == version && inode != 0) {
				/* I'm pretty sure this isn't legal */
				putstr(" ** ERROR ** ");
/*				putnstr(jDir->name, jDir->nsize); */
/*				putLabeledWord(" has dup version =", version); */
			}
			inode = jDir->ino;
			version = jDir->version;
		}
	}
	return inode;
}

char *mkmodestr(unsigned long mode, char *str)
{
	static const char *l = "xwr";
	int mask = 1, i;
	char c;

	switch (mode & S_IFMT) {
		case S_IFDIR:    str[0] = 'd'; break;
		case S_IFBLK:    str[0] = 'b'; break;
		case S_IFCHR:    str[0] = 'c'; break;
		case S_IFIFO:    str[0] = 'f'; break;
		case S_IFLNK:    str[0] = 'l'; break;
		case S_IFSOCK:   str[0] = 's'; break;
		case S_IFREG:    str[0] = '-'; break;
		default:         str[0] = '?';
	}

	for(i = 0; i < 9; i++) {
		c = l[i%3];
		str[9-i] = (mode & mask)?c:'-';
		mask = mask<<1;
	}

	if(mode & S_ISUID) str[3] = (mode & S_IXUSR)?'s':'S';
	if(mode & S_ISGID) str[6] = (mode & S_IXGRP)?'s':'S';
	if(mode & S_ISVTX) str[9] = (mode & S_IXOTH)?'t':'T';
	str[10] = '\0';
	return str;
}

static inline void dump_stat(struct stat *st, const char *name)
{
	char str[20];
	char s[64], *p;

	if (st->st_mtime == (time_t)(-1)) /* some ctimes really hate -1 */
		st->st_mtime = 1;

	ctime_r(&st->st_mtime, s/*,64*/); /* newlib ctime doesn't have buflen */

	if ((p = strchr(s,'\n')) != NULL) *p = '\0';
	if ((p = strchr(s,'\r')) != NULL) *p = '\0';

/*
	printf("%6lo %s %8ld %s %s\n", st->st_mode, mkmodestr(st->st_mode, str),
		st->st_size, s, name);
*/

	printf(" %s %8ld %s %s", mkmodestr(st->st_mode,str), st->st_size, s, name);
}

static inline int
dump_inode(struct b_lists *pL, struct b_dirent *d, struct b_inode *i)
{
	char fname[JFFS2_MAX_NAME_LEN + 1];
	struct stat st;
	size_t len;

	if(!d || !i) return -1;
	len = d->nsize;
	nand_read(mtd, d->offset + sizeof(struct jffs2_raw_dirent),
		  &len, &fname);
	fname[d->nsize] = '\0';

	memset(&st, 0, sizeof(st));

	jffs2_1pass_read_inode(pL, i->ino, NULL, &st);

	dump_stat(&st, fname);
/* FIXME
	if (d->type == DT_LNK) {
		unsigned char *src = (unsigned char *) (&i[1]);
	        putstr(" -> ");
		putnstr(src, (int)i->dsize);
	}
*/
	putstr("\r\n");

	return 0;
}

/* list inodes with the given pino */
static u32
jffs2_1pass_list_inodes(struct b_lists * pL, u32 pino)
{
	struct b_dirent *jDir;
	u32 i_version = 0;

	for (jDir = (struct b_dirent *)pL->dir.listHead; jDir; jDir = jDir->next) {
		if ((pino == jDir->pino) && (jDir->ino)) { /* ino=0 -> unlink */
			struct b_inode *jNode = (struct b_inode *)pL->frag.listHead;
			struct b_inode *i = NULL;

			while (jNode) {
				if (jNode->ino == jDir->ino && jNode->version >= i_version) {
					i_version = jNode->version;
					i = jNode;
				}
				jNode = jNode->next;
			}
			dump_inode(pL, jDir, i);
		}
	}
	return pino;
}

static u32
jffs2_1pass_search_inode(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);

	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
#if 0
		putstr("search_inode: tmp = ");
		putstr(tmp);
		putstr("\r\n");
		putstr("search_inode: wtmp = ");
		putstr(working_tmp);
		putstr("\r\n");
		putstr("search_inode: c = ");
		putstr(c);
		putstr("\r\n");
#endif
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
#if 0
		putstr("search_inode: post tmp = ");
		putstr(tmp);
		putstr("\r\n");
#endif

		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino))) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

static u32
jffs2_1pass_resolve_inode(struct b_lists * pL, u32 ino)
{
	struct b_dirent *jDir;
	struct b_inode *jNode;
	u8 jDirFoundType = 0;
	u32 jDirFoundIno = 0;
	u32 jDirFoundPino = 0;
	char tmp[JFFS2_MAX_NAME_LEN + 1];
	u32 version = 0;
	u32 pino;

	/* we need to search all and return the inode with the highest version */
	for (jDir = (struct b_dirent *)pL->dir.listHead; jDir; jDir = jDir->next) {
		if (ino == jDir->ino) {
			if (jDir->version < version)
				continue;

			if (jDir->version == version && jDirFoundType) {
				/* I'm pretty sure this isn't legal */
				putstr(" ** ERROR ** ");
/*				putnstr(jDir->name, jDir->nsize); */
/*				putLabeledWord(" has dup version (resolve) = ", */
/*					version); */
			}

			jDirFoundType = jDir->type;
			jDirFoundIno = jDir->ino;
			jDirFoundPino = jDir->pino;
			version = jDir->version;
		}
	}
	/* now we found the right entry again. (shoulda returned inode*) */
	if (jDirFoundType != DT_LNK)
		return jDirFoundIno;

	/* it's a soft link so we follow it again. */
	for (jNode = (struct b_inode *)pL->frag.listHead; jNode; jNode = jNode->next) {
		if (jNode->ino == jDirFoundIno) {
			size_t len = jNode->csize;
			nand_read(mtd,
				  jNode->offset + sizeof(struct jffs2_raw_inode),
				  &len, &tmp);
			tmp[jNode->csize] = '\0';
			break;
		}
	}
	/* ok so the name of the new file to find is in tmp */
	/* if it starts with a slash it is root based else shared dirs */
	if (tmp[0] == '/')
		pino = 1;
	else
		pino = jDirFoundPino;

	return jffs2_1pass_search_inode(pL, tmp, pino);
}

static u32
jffs2_1pass_search_list_inodes(struct b_lists * pL, const char *fname, u32 pino)
{
	int i;
	char tmp[256];
	char working_tmp[256];
	char *c;

	/* discard any leading slash */
	i = 0;
	while (fname[i] == '/')
		i++;
	strcpy(tmp, &fname[i]);
	working_tmp[0] = '\0';
	while ((c = (char *) strchr(tmp, '/')))	/* we are still dired searching */
	{
		strncpy(working_tmp, tmp, c - tmp);
		working_tmp[c - tmp] = '\0';
		for (i = 0; i < strlen(c) - 1; i++)
			tmp[i] = c[i + 1];
		tmp[i] = '\0';
		/* only a failure if we arent looking at top level */
		if (!(pino = jffs2_1pass_find_inode(pL, working_tmp, pino)) &&
		    (working_tmp[0])) {
			putstr("find_inode failed for name=");
			putstr(working_tmp);
			putstr("\r\n");
			return 0;
		}
	}

	if (tmp[0] && !(pino = jffs2_1pass_find_inode(pL, tmp, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	/* this is for the bare filename, directories have already been mapped */
	if (!(pino = jffs2_1pass_list_inodes(pL, pino))) {
		putstr("find_inode failed for name=");
		putstr(tmp);
		putstr("\r\n");
		return 0;
	}
	return pino;

}

unsigned char
jffs2_1pass_rescan_needed(struct part_info *part)
{
	struct b_node *b;
	struct jffs2_unknown_node onode;
	struct jffs2_unknown_node *node;
	struct b_lists *pL = (struct b_lists *)part->jffs2_priv;

	if (part->jffs2_priv == 0){
		DEBUGF ("rescan: First time in use\n");
		return 1;
	}
	/* if we have no list, we need to rescan */
	if (pL->frag.listCount == 0) {
		DEBUGF ("rescan: fraglist zero\n");
		return 1;
	}

	/* or if we are scanning a new partition */
	if (pL->partOffset != part->offset) {
		DEBUGF ("rescan: different partition\n");
		return 1;
	}

	/* FIXME */
#if 0
	/* but suppose someone reflashed a partition at the same offset... */
	b = pL->dir.listHead;
	while (b) {
		node = (struct jffs2_unknown_node *) get_fl_mem(b->offset,
			sizeof(onode), &onode);
		if (node->nodetype != JFFS2_NODETYPE_DIRENT) {
			DEBUGF ("rescan: fs changed beneath me? (%lx)\n",
					(unsigned long) b->offset);
			return 1;
		}
		b = b->next;
	}
#endif
	return 0;
}

#ifdef DEBUG_FRAGMENTS
static void
dump_fragments(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;

	putstr("\r\n\r\n******The fragment Entries******\r\n");
	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
		putLabeledWord("\r\n\tbuild_list: FLASH_OFFSET = ", b->offset);
		putLabeledWord("\tbuild_list: totlen = ", jNode->totlen);
		putLabeledWord("\tbuild_list: inode = ", jNode->ino);
		putLabeledWord("\tbuild_list: version = ", jNode->version);
		putLabeledWord("\tbuild_list: isize = ", jNode->isize);
		putLabeledWord("\tbuild_list: atime = ", jNode->atime);
		putLabeledWord("\tbuild_list: offset = ", jNode->offset);
		putLabeledWord("\tbuild_list: csize = ", jNode->csize);
		putLabeledWord("\tbuild_list: dsize = ", jNode->dsize);
		putLabeledWord("\tbuild_list: compr = ", jNode->compr);
		putLabeledWord("\tbuild_list: usercompr = ", jNode->usercompr);
		putLabeledWord("\tbuild_list: flags = ", jNode->flags);
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
	}
}
#endif

#ifdef DEBUG_DIRENTS
static void
dump_dirents(struct b_lists *pL)
{
	struct b_node *b;
	struct jffs2_raw_dirent *jDir;

	putstr("\r\n\r\n******The directory Entries******\r\n");
	b = pL->dir.listHead;
	while (b) {
		jDir = (struct jffs2_raw_dirent *) get_node_mem(b->offset);
		putstr("\r\n");
		putnstr(jDir->name, jDir->nsize);
		putLabeledWord("\r\n\tbuild_list: magic = ", jDir->magic);
		putLabeledWord("\tbuild_list: nodetype = ", jDir->nodetype);
		putLabeledWord("\tbuild_list: hdr_crc = ", jDir->hdr_crc);
		putLabeledWord("\tbuild_list: pino = ", jDir->pino);
		putLabeledWord("\tbuild_list: version = ", jDir->version);
		putLabeledWord("\tbuild_list: ino = ", jDir->ino);
		putLabeledWord("\tbuild_list: mctime = ", jDir->mctime);
		putLabeledWord("\tbuild_list: nsize = ", jDir->nsize);
		putLabeledWord("\tbuild_list: type = ", jDir->type);
		putLabeledWord("\tbuild_list: node_crc = ", jDir->node_crc);
		putLabeledWord("\tbuild_list: name_crc = ", jDir->name_crc);
		putLabeledWord("\tbuild_list: offset = ", b->offset);	/* FIXME: ? [RS] */
		b = b->next;
		put_fl_mem(jDir);
	}
}
#endif

static int
jffs2_fill_scan_buf(struct mtd_info *mtd, unsigned char *buf,
		    unsigned ofs, unsigned len)
{
	int ret;
	unsigned olen;

	olen = len;
	ret = nand_read(mtd, ofs, &olen, buf);
	if (ret) {
		printf("nand_read(0x%x bytes from 0x%x) returned %d\n", len, ofs, ret);
		return ret;
	}
	if (olen < len) {
		printf("Read at 0x%x gave only 0x%x bytes\n", ofs, olen);
		return -1;
	}
	return 0;
}

#define	EMPTY_SCAN_SIZE	1024
static u32
jffs2_1pass_build_lists(struct part_info * part)
{
	struct b_lists *pL;
	struct jffs2_unknown_node *node;
	unsigned nr_blocks, sectorsize, ofs, offset;
	char *buf;
	int i;
	u32 counter = 0;
	u32 counter4 = 0;
	u32 counterF = 0;
	u32 counterN = 0;

	struct mtdids *id = part->dev->id;
	mtd = get_nand_dev_by_index(id->num);
	if (!mtd) {
		pr_err("\nno NAND devices available\n");
		return 0;
	}

	/* if we are building a list we need to refresh the cache. */
	jffs_init_1pass_list(part);
	pL = (struct b_lists *)part->jffs2_priv;
	pL->partOffset = part->offset;
	puts ("Scanning JFFS2 FS:   ");

	sectorsize = mtd->erasesize;
	nr_blocks = part->size / sectorsize;
	buf = malloc(sectorsize);
	if (!buf)
		return 0;

	for (i = 0; i < nr_blocks; i++) {
		printf("\b\b%c ", spinner[counter++ % sizeof(spinner)]);

		offset = part->offset + i * sectorsize;

		if (nand_block_isbad(mtd, offset))
			continue;

		if (jffs2_fill_scan_buf(mtd, buf, offset, EMPTY_SCAN_SIZE))
			return 0;

		ofs = 0;
		/* Scan only 4KiB of 0xFF before declaring it's empty */
		while (ofs < EMPTY_SCAN_SIZE && *(uint32_t *)(&buf[ofs]) == 0xFFFFFFFF)
			ofs += 4;
		if (ofs == EMPTY_SCAN_SIZE)
			continue;

		if (jffs2_fill_scan_buf(mtd, buf + EMPTY_SCAN_SIZE, offset + EMPTY_SCAN_SIZE, sectorsize - EMPTY_SCAN_SIZE))
			return 0;
		offset += ofs;

		while (ofs < sectorsize - sizeof(struct jffs2_unknown_node)) {
			node = (struct jffs2_unknown_node *)&buf[ofs];
			if (node->magic != JFFS2_MAGIC_BITMASK || !hdr_crc(node)) {
				offset += 4;
				ofs += 4;
				counter4++;
				continue;
			}
			/* if its a fragment add it */
			if (node->nodetype == JFFS2_NODETYPE_INODE &&
				    inode_crc((struct jffs2_raw_inode *) node)) {
				if (insert_inode(&pL->frag, (struct jffs2_raw_inode *) node,
						 offset) == NULL) {
					return 0;
				}
			} else if (node->nodetype == JFFS2_NODETYPE_DIRENT &&
				   dirent_crc((struct jffs2_raw_dirent *) node)  &&
				   dirent_name_crc((struct jffs2_raw_dirent *) node)) {
				if (! (counterN%100))
					puts ("\b\b.  ");
				if (insert_dirent(&pL->dir, (struct jffs2_raw_dirent *) node,
						  offset) == NULL) {
					return 0;
				}
				counterN++;
			} else if (node->nodetype == JFFS2_NODETYPE_CLEANMARKER) {
				if (node->totlen != sizeof(struct jffs2_unknown_node))
					printf("OOPS Cleanmarker has bad size "
						"%d != %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
			} else if (node->nodetype == JFFS2_NODETYPE_PADDING) {
				if (node->totlen < sizeof(struct jffs2_unknown_node))
					printf("OOPS Padding has bad size "
						"%d < %zu\n",
						node->totlen,
						sizeof(struct jffs2_unknown_node));
			} else {
				printf("Unknown node type: %x len %d offset 0x%x\n",
					node->nodetype,
					node->totlen, offset);
			}
			offset += ((node->totlen + 3) & ~3);
			ofs += ((node->totlen + 3) & ~3);
			counterF++;
		}
	}

	putstr("\b\b done.\r\n");		/* close off the dots */

#if 0
	putLabeledWord("dir entries = ", pL->dir.listCount);
	putLabeledWord("frag entries = ", pL->frag.listCount);
	putLabeledWord("+4 increments = ", counter4);
	putLabeledWord("+file_offset increments = ", counterF);
#endif

#ifdef DEBUG_DIRENTS
	dump_dirents(pL);
#endif

#ifdef DEBUG_FRAGMENTS
	dump_fragments(pL);
#endif

	/* give visual feedback that we are done scanning the flash */
	led_blink(0x0, 0x0, 0x1, 0x1);	/* off, forever, on 100ms, off 100ms */
	free(buf);

	return 1;
}


static u32
jffs2_1pass_fill_info(struct b_lists * pL, struct b_jffs2_info * piL)
{
	struct b_node *b;
	struct jffs2_raw_inode ojNode;
	struct jffs2_raw_inode *jNode;
	int i;

	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		piL->compr_info[i].num_frags = 0;
		piL->compr_info[i].compr_sum = 0;
		piL->compr_info[i].decompr_sum = 0;
	}
/*	FIXME
	b = pL->frag.listHead;
	while (b) {
		jNode = (struct jffs2_raw_inode *) get_fl_mem(b->offset,
			sizeof(ojNode), &ojNode);
		if (jNode->compr < JFFS2_NUM_COMPR) {
			piL->compr_info[jNode->compr].num_frags++;
			piL->compr_info[jNode->compr].compr_sum += jNode->csize;
			piL->compr_info[jNode->compr].decompr_sum += jNode->dsize;
		}
		b = b->next;
	}
*/
	return 0;
}


static struct b_lists *
jffs2_get_list(struct part_info * part, const char *who)
{
	if (jffs2_1pass_rescan_needed(part)) {
		if (!jffs2_1pass_build_lists(part)) {
			printf("%s: Failed to scan JFFSv2 file structure\n", who);
			return NULL;
		}
	}
	return (struct b_lists *)part->jffs2_priv;
}


/* Print directory / file contents */
u32
jffs2_1pass_ls(struct part_info * part, const char *fname)
{
	struct b_lists *pl;
	long ret = 0;
	u32 inode;

	if (! (pl = jffs2_get_list(part, "ls")))
		return 0;

	if (! (inode = jffs2_1pass_search_list_inodes(pl, fname, 1))) {
		putstr("ls: Failed to scan jffs2 file structure\r\n");
		return 0;
	}

#if 0
	putLabeledWord("found file at inode = ", inode);
	putLabeledWord("read_inode returns = ", ret);
#endif

	return ret;
}


/* Load a file from flash into memory. fname can be a full path */
u32
jffs2_1pass_load(char *dest, struct part_info * part, const char *fname)
{

	struct b_lists *pl;
	long ret = 0;
	u32 inode;

	if (! (pl = jffs2_get_list(part, "load")))
		return 0;

	if (! (inode = jffs2_1pass_search_inode(pl, fname, 1))) {
		putstr("load: Failed to find inode\r\n");
		return 0;
	}

	/* Resolve symlinks */
	if (! (inode = jffs2_1pass_resolve_inode(pl, inode))) {
		putstr("load: Failed to resolve inode structure\r\n");
		return 0;
	}

	if ((ret = jffs2_1pass_read_inode(pl, inode, dest, NULL)) < 0) {
		putstr("load: Failed to read inode\r\n");
		return 0;
	}

	DEBUGF ("load: loaded '%s' to 0x%lx (%ld bytes)\n", fname,
				(unsigned long) dest, ret);
	return ret;
}

/* Return information about the fs on this partition */
u32
jffs2_1pass_info(struct part_info * part)
{
	struct b_jffs2_info info;
	struct b_lists *pl;
	int i;

	if (! (pl = jffs2_get_list(part, "info")))
		return 0;

	jffs2_1pass_fill_info(pl, &info);
	for (i = 0; i < JFFS2_NUM_COMPR; i++) {
		printf ("Compression: %s\n"
			"\tfrag count: %d\n"
			"\tcompressed sum: %d\n"
			"\tuncompressed sum: %d\n",
			compr_names[i],
			info.compr_info[i].num_frags,
			info.compr_info[i].compr_sum,
			info.compr_info[i].decompr_sum);
	}
	return 1;
}
