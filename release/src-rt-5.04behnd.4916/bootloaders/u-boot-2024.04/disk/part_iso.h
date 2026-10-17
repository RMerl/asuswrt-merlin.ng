/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch.
 */
#ifndef _PART_CD_H
#define _PART_CD_H

#define BRVD	0x11
#define PVD_OFFSET 0x10


typedef struct iso_boot_rec {
	unsigned char desctype;			/* type of Volume descriptor: 0 = boot record, 1 = primary, 2 = Supplement, 3 = volume part 0xff trminator */
	unsigned char stand_ident[5]; /* "CD001" */
	unsigned char vers;					/* Version */
	char					ident_str[0x20]; /* Ident String "EL TORITO SPECIFICATION" */
	unsigned char unused[0x20];		/* unused */
	unsigned char pointer[4];		/* absolute pointer to Boot Catalog */
} iso_boot_rec_t;


typedef struct iso_pri_rec {
	unsigned char desctype;			/* type of Volume descriptor: 0 = boot record, 1 = primary, 2 = Supplement, 3 = volume part 0xff trminator */
	unsigned char stand_ident[5]; /* "CD001" */
	unsigned char vers;					/* Version */
	unsigned char unused;
	char					sysid[32];		/* system Identifier */
	char					volid[32];		/* volume Identifier */
	unsigned char zeros1[8];		/* unused */
	unsigned int volsiz_LE;		/* volume size Little Endian */
	unsigned int volsiz_BE;		/* volume size Big Endian */
	unsigned char zeros2[32];		/* unused */
	unsigned short setsize_LE;	/* volume set size LE */
	unsigned short setsize_BE;	/* volume set size BE */
	unsigned short seqnum_LE;		/* volume sequence number LE */
	unsigned short seqnum_BE;		/* volume sequence number BE */
	unsigned short secsize_LE;	/* sector size LE */
	unsigned short secsize_BE;	/* sector size BE */
	unsigned int pathtablen_LE;/* Path Table size LE */
	unsigned int pathtablen_BE;/* Path Table size BE */
	unsigned int firstsek_LEpathtab1_LE; /* location of first occurrence of little endian type path table */
	unsigned int firstsek_LEpathtab2_LE; /* location of optional occurrence of little endian type path table */
	unsigned int firstsek_BEpathtab1_BE; /* location of first occurrence of big endian type path table */
	unsigned int firstsek_BEpathtab2_BE; /* location of optional occurrence of big endian type path table */
	unsigned char rootdir[34];	/* directory record for root dir */
	char					volsetid[128];/* Volume set identifier */
	char					pubid[128];		/* Publisher identifier */
	char					dataprepid[128]; /* data preparer identifier */
	char					appid[128];		/* application identifier */
	char					copyr[37];		/* copyright string */
	char					abstractfileid[37]; /* abstract file identifier */
	char					bibliofileid[37]; /* bibliographic file identifier */
	unsigned char creationdate[17]; /* creation date */
	unsigned char modify[17];		/* modification date */
	unsigned char expire[17];		/* expiring date */
	unsigned char effective[17];/* effective date */
	unsigned char filestruc_ver;	/* file structur version */
} iso_pri_rec_t;

typedef struct iso_sup_rec {
	unsigned char desctype;			/* type of Volume descriptor: 0 = boot record, 1 = primary, 2 = Supplement, 3 = volume part 0xff trminator */
	unsigned char stand_ident[5]; /* "CD001" */
	unsigned char vers;					/* Version */
	unsigned char volumeflags;	/* if bit 0 = 0 => all escape sequences are according ISO 2375 */
	char					sysid[32];		/* system Identifier */
	char					volid[32];		/* volume Identifier */
	unsigned char zeros1[8];		/* unused */
	unsigned int volsiz_LE;		/* volume size Little Endian */
	unsigned int volsiz_BE;		/* volume size Big Endian */
	unsigned char escapeseq[32];/* Escape sequences */
	unsigned short setsize_LE;	/* volume set size LE */
	unsigned short setsize_BE;	/* volume set size BE */
	unsigned short seqnum_LE;		/* volume sequence number LE */
	unsigned short seqnum_BE;		/* volume sequence number BE */
	unsigned short secsize_LE;	/* sector size LE */
	unsigned short secsize_BE;	/* sector size BE */
	unsigned int pathtablen_LE;/* Path Table size LE */
	unsigned int pathtablen_BE;/* Path Table size BE */
	unsigned int firstsek_LEpathtab1_LE; /* location of first occurrence of little endian type path table */
	unsigned int firstsek_LEpathtab2_LE; /* location of optional occurrence of little endian type path table */
	unsigned int firstsek_BEpathtab1_BE; /* location of first occurrence of big endian type path table */
	unsigned int firstsek_BEpathtab2_BE; /* location of optional occurrence of big endian type path table */
	unsigned char rootdir[34];	/* directory record for root dir */
	char					volsetid[128];/* Volume set identifier */
	char					pubid[128];		/* Publisher identifier */
	char					dataprepid[128]; /* data preparer identifier */
	char					appid[128];		/* application identifier */
	char					copyr[37];		/* copyright string */
	char					abstractfileid[37]; /* abstract file identifier */
	char					bibliofileid[37]; /* bibliographic file identifier */
	unsigned char creationdate[17]; /* creation date */
	unsigned char modify[17];		/* modification date */
	unsigned char expire[17];		/* expiring date */
	unsigned char effective[17];/* effective date */
	unsigned char filestruc_ver;	/* file structur version */
}iso_sup_rec_t;

typedef struct iso_part_rec {
	unsigned char desctype;			/* type of Volume descriptor: 0 = boot record, 1 = primary, 2 = Supplement, 3 = volume part 0xff trminator */
	unsigned char stand_ident[5]; /* "CD001" */
	unsigned char vers;					/* Version */
	unsigned char unused;
	char					sysid[32];		 /* system Identifier */
	char					volid[32];		/* volume partition Identifier */
	unsigned int partloc_LE;		/* volume partition location LE */
	unsigned int partloc_BE;		/* volume partition location BE */
	unsigned int partsiz_LE;		/* volume partition size LE */
	unsigned int partsiz_BE;		/* volume partition size BE */
}iso_part_rec_t;


typedef struct iso_val_entry {
	unsigned char	header_id;		/* Header ID must be 0x01 */
	unsigned char	platform;			/* Platform: 0=x86, 1=PowerPC, 2=MAC */
	unsigned char res[2];				/* reserved */
	char					manu_str[0x18]; /* Ident String of manufacturer/developer */
	unsigned char chk_sum[2];	/* Check sum (all words must be zero)  */
	unsigned char key[2];				/* key[0]=55, key[1]=0xAA */
} iso_val_entry_t;

typedef struct iso_header_entry {
	unsigned char	header_id;		/* Header ID must be 0x90 or 0x91 */
	unsigned char	platform;			/* Platform: 0=x86, 1=PowerPC, 2=MAC */
	unsigned char numentry[2];	/* number of entries */
	char					id_str[0x1C]; /* Ident String of sectionr */
} iso_header_entry_t;


typedef struct iso_init_def_entry {
	unsigned char	boot_ind;			/* Boot indicator 0x88=bootable 0=not bootable */
	unsigned char	boot_media;		/* boot Media Type: 0=no Emulation, 1=1.2MB floppy, 2=1.44MB floppy, 3=2.88MB floppy 4=hd (0x80) */
	unsigned char ld_seg[2];		/* Load segment (flat model=addr/10) */
	unsigned char systype;			/* System Type copy of byte5 of part table */
	unsigned char res;					/* reserved */
	unsigned char sec_cnt[2];		/* sector count in VIRTUAL Blocks (0x200) */
	unsigned char rel_block_addr[4];	/* relative Block address */
} iso_init_def_entry_t;


void print_partition_cd(int dev);

#endif /* _PART_CD_H */
