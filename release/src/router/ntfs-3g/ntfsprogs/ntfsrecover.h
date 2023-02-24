/*
 *		Declarations for processing log data
 *
 * Copyright (c) 2000-2005 Anton Altaparmakov
 * Copyright (c) 2014-2016 Jean-Pierre Andre
 */

/*
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
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define getle16(p,x) le16_to_cpu(*(const le16*)((const char*)(p) + (x)))
#define getle32(p,x) le32_to_cpu(*(const le32*)((const char*)(p) + (x)))
#define getle64(p,x) le64_to_cpu(*(const le64*)((const char*)(p) + (x)))

#define feedle16(p,x) (*(const le16*)((const char*)(p) + (x)))
#define feedle32(p,x) (*(const le32*)((const char*)(p) + (x)))
#define feedle64(p,x) (*(const le64*)((const char*)(p) + (x)))

enum ACTIONS {
	Noop,					/* 0 */
	CompensationlogRecord,			/* 1 */
	InitializeFileRecordSegment,		/* 2 */
	DeallocateFileRecordSegment,		/* 3 */
	WriteEndofFileRecordSegment,		/* 4 */
	CreateAttribute,			/* 5 */
	DeleteAttribute,			/* 6 */
	UpdateResidentValue,			/* 7 */
	UpdateNonResidentValue,			/* 8 */
	UpdateMappingPairs,			/* 9 */
	DeleteDirtyClusters,			/* 10 */
	SetNewAttributeSizes,			/* 11 */
	AddIndexEntryRoot,			/* 12 */
	DeleteIndexEntryRoot,			/* 13 */
	AddIndexEntryAllocation,		/* 14 */
	DeleteIndexEntryAllocation,		/* 15 */
	WriteEndOfIndexBuffer,			/* 16 */
	SetIndexEntryVcnRoot,			/* 17 */
	SetIndexEntryVcnAllocation,		/* 18 */
	UpdateFileNameRoot,			/* 19 */
	UpdateFileNameAllocation,		/* 20 */
	SetBitsInNonResidentBitMap,		/* 21 */
	ClearBitsInNonResidentBitMap,		/* 22 */
	HotFix,					/* 23 */
	EndTopLevelAction,			/* 24 */
	PrepareTransaction,			/* 25 */
	CommitTransaction,			/* 26 */
	ForgetTransaction,			/* 27 */
	OpenNonResidentAttribute,		/* 28 */
	OpenAttributeTableDump,			/* 29 */
	AttributeNamesDump,			/* 30 */
	DirtyPageTableDump,			/* 31 */
	TransactionTableDump,			/* 32 */
	UpdateRecordDataRoot,			/* 33 */
	UpdateRecordDataAllocation,		/* 34 */
	Win10Action35,				/* 35 */
	Win10Action36,				/* 36 */
	Win10Action37,				/* 37 */
	LastAction				/* 38 */
} ;

struct BUFFER {
	unsigned int num;
	unsigned int rnum;
	unsigned int size;
	unsigned int headsz;
	BOOL safe;
	union {
		u64 alignment;
		RESTART_PAGE_HEADER restart;
		RECORD_PAGE_HEADER record;
		char data[1];
	} block;  /* variable length, keep at the end */
} ;

struct ACTION_RECORD {
	struct ACTION_RECORD *next;
	struct ACTION_RECORD *prev;
	int num;
	unsigned int flags;
	LOG_RECORD record; /* variable length, keep at the end */
} ;

enum {		/* Flag values for ACTION_RECORD */
	ACTION_TO_REDO = 1	/* Committed, possibly not synced */
	} ;

struct ATTR {
	u64 inode;
	u64 lsn;
	le32 type;
	u16 key;
	u16 namelen;
	le16 name[1];
} ;

extern u32 clustersz;
extern int clusterbits;
extern u32 blocksz;
extern int blockbits;
extern u16 bytespersect;
extern u64 mftlcn;
extern u32 mftrecsz;
extern int mftrecbits;
extern u32 mftcnt; /* number of entries */
extern BOOL optc;
extern BOOL optn;
extern int opts;
extern int optv;
extern unsigned int redocount;
extern unsigned int undocount;
extern ntfs_inode *log_ni;
extern ntfs_attr *log_na;
extern u64 logfilelcn;
extern u32 logfilesz; /* bytes */
extern u64 redos_met;
extern u64 committed_lsn;
extern u64 synced_lsn;
extern u64 latest_lsn;
extern u64 restart_lsn;

extern RESTART_AREA restart;
extern LOG_CLIENT_RECORD client;

const char *actionname(int op);
const char *mftattrname(ATTR_TYPES attr);
void showname(const char *prefix, const char *name, int cnt);
int fixnamelen(const char *name, int len);
BOOL within_lcn_range(const LOG_RECORD *logr);
struct ATTR *getattrentry(unsigned int key, unsigned int lth);
void copy_attribute(struct ATTR *pa, const char *buf, int length);
u32 get_undo_offset(const LOG_RECORD *logr);
u32 get_redo_offset(const LOG_RECORD *logr);
u32 get_extra_offset(const LOG_RECORD *logr);
BOOL exception(int num);

struct STORE;
extern int play_undos(ntfs_volume *vol, const struct ACTION_RECORD *firstundo);
extern int play_redos(ntfs_volume *vol, const struct ACTION_RECORD *firstredo);
extern void show_redos(void);
extern void freeclusterentry(struct STORE*);
void hexdump(const char *buf, unsigned int lth);
