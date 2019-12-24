/***************************************************************************
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
 ***************************************************************************/

#ifndef _CFE_GPT_H
#define _CFE_GPT_H


#include <emmc_base_defs.h>
/*********************************************************************
 *  Constants
 ********************************************************************* */

/***************************************************************************
 *PMBR - Protective MBR(Master Boot Recorder)
 ***************************************************************************/
/* CFE :: PMBR :: ?? [31:16] */
#define GPT_PMBR_BOOTCODE_OFFSET				0
#define GPT_PMBR_BOOTCODE_LENGTH				424

#define GPT_PMBR_UNIQUEMBRDISKSIGNATURE_OFFSET	440
#define GPT_PMBR_UNIQUEMBRDISKSIGNATURE_LENGTH	4

#define GPT_PMBR_UNKNOWN_OFFSET					444
#define GPT_PMBR_UNKNOWN_LENGTH					2

#define GPT_PMBR_PARTITIONRECORD_OFFSET			446
#define GPT_PMBR_PARTITIONRECORD_LENGTH			64	//16*4

#define GPT_PMBR_SIGNATURE_OFFSET				510
#define GPT_PMBR_SIGNATURE_LENGTH				2

#define GPT_PMBR_RESERVED_OFFSET				512
#define GPT_PMBR_RESERVED_LENGTH				0	//Logical BlockSize - 512

#define GPT_PMBR_PARTRCD_BOOTINDICATOR_OFFSET	0
#define GPT_PMBR_PARTRCD_BOOTINDICATOR_LENGTH	1

#define GPT_PMBR_PARTRCD_STARTINGCHS_OFFSET		1
#define GPT_PMBR_PARTRCD_STARTINGCHS_LENGTH		3

#define GPT_PMBR_PARTRCD_OSTYPE_OFFSET			4
#define GPT_PMBR_PARTRCD_OSTYPE_LENGTH			1

#define GPT_PMBR_PARTRCD_ENDINGCHS_OFFSET		5
#define GPT_PMBR_PARTRCD_ENDINGCHS_LENGTH		3

#define GPT_PMBR_PARTRCD_STARTINGLBA_OFFSET		8
#define GPT_PMBR_PARTRCD_STARTINGLBA_LENGTH		4

#define GPT_PMBR_PARTRCD_SIZEINLBA_OFFSET		12
#define GPT_PMBR_PARTRCD_SIZEINLBA_LENGTH		4

#define GPT_MAX_MBR_PARTITIONS					4
#define GPT_PMBR_GPT_PARTITION					0xEE


/***************************************************************************
 *GPT_HEADER - Block Reset and Count Register
 ***************************************************************************/
/* CFE :: GPT_HEADER :: ?? [31:16] */
/*#define CFE_GPT_HEADER_SIGNATURE_MASK           0xffff0000*/
#define GPT_HDR_SIGNATURE_OFFSET			0
#define GPT_HDR_SIGNATURE_LENGTH			8

#define GPT_HDR_REVISION_OFFSET				8
#define GPT_HDR_REVISION_LENGTH				4

#define GPT_HDR_HEADERSIZE_OFFSET			12
#define GPT_HDR_HEADERSIZE_LENGTH			4

#define GPT_HDR_HEADERCRC32_OFFSET			16
#define GPT_HDR_HEADERCRC32_LENGTH			4

#define GPT_HDR_RESERVED_OFFSET				20
#define GPT_HDR_RESERVED_LENGTH				4

#define GPT_HDR_MYLBA_OFFSET				24
#define GPT_HDR_MYLBA_LENGTH				8

#define GPT_HDR_ALTERNATELBA_OFFSET			32
#define GPT_HDR_ALTERNATELBA_LENGTH			8

#define GPT_HDR_FIRSTUSABLELBA_OFFSET		40
#define GPT_HDR_FIRSTUSABLELBA_LENGTH		8

#define GPT_HDR_LASTUSABLELBA_OFFSET		48
#define GPT_HDR_LASTUSABLELBA_LENGTH		8

#define GPT_HDR_DISKGUID_OFFSET				56
#define GPT_HDR_DISKGUID_LENGTH				16
#define GPT_HDR_DISKGUID_OFFSET_L1			56
#define GPT_HDR_DISKGUID_LENGTH_L1			4
#define GPT_HDR_DISKGUID_OFFSET_L2			60
#define GPT_HDR_DISKGUID_LENGTH_L2			2
#define GPT_HDR_DISKGUID_OFFSET_L3			62
#define GPT_HDR_DISKGUID_LENGTH_L3			2
#define GPT_HDR_DISKGUID_OFFSET_H1			64
#define GPT_HDR_DISKGUID_LENGTH_H1			2
#define GPT_HDR_DISKGUID_OFFSET_H2			66
#define GPT_HDR_DISKGUID_LENGTH_H2			6

#define GPT_HDR_PARTITIONENTRYLBA_OFFSET			72
#define GPT_HDR_PARTITIONENTRYLBA_LENGTH			8

#define GPT_HDR_NUMBEROFPARTITIONENTRIES_OFFSET		80
#define GPT_HDR_NUMBEROFPARTITIONENTRIES_LENGTH		4

#define GPT_HDR_SIZEOFPARTITIONENTRY_OFFSET			84
#define GPT_HDR_SIZEOFPARTITIONENTRY_LENGTH			4

#define GPT_HDR_PARTITIONENTRYARRAYCRC32_OFFSET		88
#define GPT_HDR_PARTITIONENTRYARRAYCRC32_LENGTH		4

#define GPT_HDR_RESERVED_REST_OFFSET				92
#define GPT_HDR_RESERVED_REST_LENGTH				36	// Block Size - 92

#define GPT_HDR_BLOCK_SIZE					92


/***************************************************************************
 *GPT_PART - GPT Partition Entry
 ***************************************************************************/
/* CFE :: PART :: ?? [31:16] */
/*#define GPT_HEADER_SIGNATURE_MASK           0xffff0000*/
#define GPT_PART_PARITIONTYPEGUID_OFFSET		0	// [Byte]
#define GPT_PART_PARITIONTYPEGUID_LENGTH		16	// [Byte]
#define GPT_PART_PARITIONTYPEGUID_OFFSET_L1		0
#define GPT_PART_PARITIONTYPEGUID_LENGTH_L1		4
#define GPT_PART_PARITIONTYPEGUID_OFFSET_L2		4
#define GPT_PART_PARITIONTYPEGUID_LENGTH_L2		2
#define GPT_PART_PARITIONTYPEGUID_OFFSET_L3		6
#define GPT_PART_PARITIONTYPEGUID_LENGTH_L3		2
#define GPT_PART_PARITIONTYPEGUID_OFFSET_H1		8
#define GPT_PART_PARITIONTYPEGUID_LENGTH_H1		2
#define GPT_PART_PARITIONTYPEGUID_OFFSET_H2		10
#define GPT_PART_PARITIONTYPEGUID_LENGTH_H2		6

#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET	16
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH	16
#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L1	16	// Endianess : Native
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L1	4
#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L2	20	// Endianess : Native
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L2	2
#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L3	22	// Endianess : Native
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L3	2
#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H1	24	// Big Endian, byte-swapped
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H1	2
#define GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H2	26	// Big Endian, byte-swapped
#define GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H2	6

#define GPT_PART_STARTINGLBA_OFFSET				32
#define GPT_PART_STARTINGLBA_LENGTH				8

#define GPT_PART_ENDINGLBA_OFFSET				40
#define GPT_PART_ENDINGLBA_LENGTH				8

#define GPT_PART_ATTRIBUTES_OFFSET				48
#define GPT_PART_ATTRIBUTES_LENGTH				8

#define GPT_PART_PARITIONNAME_OFFSET				56
#define GPT_PART_PARITIONNAME_LENGTH				72		// Name is encoded in UTF-16 format

#define GPT_PART_RESERVED_OFFSET				128
#define GPT_PART_RESERVED_LENGTH				0		// Size Of Partition on Entry - 128

#define CFE_GPT_SIZE_OF_PARTITION(x)			(128*x)


/***************************************************************************
 *GPT INFORMATION TO IMPLEMENT
 ***************************************************************************/
#define CFE_MAX_GPT_PARTITIONS			EMMC_FLASH_MAX_LOGICAL_PARTS		/* For GPT partition */
#define CFE_GPT_MAX_PART_NAME			(GPT_PART_PARITIONNAME_LENGTH/2)	/* For GPT partition */
#define CFE_GPT_TABLE_SIZE			32768	/* 32KB, CFE partition size for GPT Partition entries + GPT HDRS + extra bytes(PMBR) */
#define CFE_GPT_PRIMRY_SIZE			CFE_GPT_TABLE_SIZE	/* Actual size : 34LBA=34*512=17KB, CFE partition size for primary GPT table */
#define CFE_GPT_BACKUP_SIZE			CFE_GPT_TABLE_SIZE	/* Actual size : 33LBA=33*512=16.5KB, CFE partition size for backup GPT table */
#define PRIMARY_GPT_HDR_PART_NAME 		"gpt0"
#define BACKUP_GPT_HDR_PART_NAME  		"gpt1"
#define GPT_HDR_IDX_PRIMARY			0
#define GPT_HDR_IDX_BACKUP			1

#define GPT_OFF			0
#define GPT_ON			1
#define DEBUG_GPT		GPT_OFF
#define DEBUG_GPT_VALID	GPT_OFF
#define DEBUG_GPT_PRINT	GPT_OFF


/*  *********************************************************************
    *  Prototypes
    ********************************************************************* */
/*
 * Partition structure - use this to define a gpt table.
 * The partitions are assigned in order from the beginning of the gpt.
 * CFE Partitions can be on byte boundaries.
 */
typedef struct gpt_memaddr_s {
	uint64_t	MemTopOfGpt;
	uint64_t	MemBotOfGpt;
} gpt_memaddr_t;

typedef struct gpt_GUID_s {
	uint32_t	l1;	// 32-bit, Native
	uint16_t	l2;	// 16-bit, Native
	uint16_t	l3;	// 16-bit, Native
	uint16_t	h1;	// 16-bit, Big Endian(byte-swapped)
	uint64_t	h2;	// 48-bit, Big Endian(byte-swapped)
} gpt_GUID_t;

typedef struct gpt_PartitionRecord_s {
	uint8_t		BootIndicator;
	uint32_t	StartingCHS;
	uint8_t		OSType;
	uint32_t	EndingCHS;
	uint32_t	StartingLBA;
	uint32_t	SizeInLBA;
} gpt_PartitionRecord_t;

typedef struct gpt_ProtectiveMBR_s {
	uint16_t	Signature;
	gpt_PartitionRecord_t	PartRcd[GPT_MAX_MBR_PARTITIONS];		// PartitionRecord
} gpt_ProtectiveMBR_t;

typedef struct gpt_GptHeader_s {
	uint64_t	Signature;
	uint32_t	Revision;
	uint32_t	HeaderSize;
	uint32_t	HeaderCRC32;
	uint32_t	Reserved;
	uint64_t	MyLBA;
	uint64_t	AlternateLBA;
	uint64_t	FirstUsableLBA;
	uint64_t	LastUsableLBA;
	gpt_GUID_t	DiskGUID;
	uint64_t	PartitionEntryLBA;
	uint32_t	NumberOfPartitionEntries;
	uint32_t	SizeOfPartitionEntry;
	uint32_t	PartitionEntryArrayCRC32;
	//uint8_t		Reserved_Rest[GPT_HDR_RESERVED_REST_LENGTH];		// 36 Byte
} gpt_GptHeader_t;

typedef struct gpt_PartitionEntry_s {
	gpt_GUID_t	PartitionTypeGUID;
	gpt_GUID_t	UniquePartitionGUID;
	uint64_t	StartingLBA;
	uint64_t	EndingLBA;
	uint64_t	Attributes;
	char		PartitionName[GPT_PART_PARITIONNAME_LENGTH];
	// gdisk parameters
	int			PartitionNumber;
	uint64_t	PartSizeLBA;		// [LBA]
} gpt_PartitionEntry_t;

typedef struct cfe_gpt_s {
    /* GPT config information. */
	gpt_memaddr_t			mem_addr;				/* Actual meemory in STAGING memory of CFE gpt partition */
	gpt_ProtectiveMBR_t		PMBR;
	gpt_GptHeader_t			primHDR;
	gpt_GptHeader_t			backHDR;
	gpt_PartitionEntry_t	primPartEntry;
	gpt_PartitionEntry_t	backPartEntry;
} cfe_gpt_t;

// GPT Partition Entry Status
typedef struct cfe_gpt_PartEntryStatus_s{
	int OutOfRange;
	int Overlap;
	int OsSpecific;
} cfe_gpt_PartEntryStatus_t;

typedef struct flash_part_t {
	uint64_t   fp_size;
	uint64_t   fp_offset_bytes;
	char       fp_name[CFE_GPT_MAX_PART_NAME];
	int        fp_partition;	// Partitiion attribute
} cfe_logic_part_t;

/*
 * Probe structure - this is used when we want to describe to the flash
 * driver the layout of our flash, particularly when you want to
 * manually describe the sectors.
 */
typedef struct cfe_gpt_probe_s {
    /* GPT config information. */
	cfe_logic_part_t    cfe_parts[CFE_MAX_GPT_PARTITIONS];	// CFE partition for GPT
	uint8_t				num_parts;		/* # of cfe partitions */
	uint64_t			lba_size;		/* lba size of net flash/disk. e.g. eMMC : SECTOR_COUNT */
	uint64_t			cfe_FirstUsableLba;	/* First usable LBA # except gpt0/gpt1 partition */
	uint64_t			cfe_LastUsableLba;	/* Last usable LBA # except gpt0/gpt1 partition */
	uint32_t			block_size;		/* # of Byte per LBA */
	uint32_t			block_size_bit;		/* # bit of address per LBA */
	char 				*dev_gpt0_name;		/* Primary GPT partition name to read device */
	char 				*dev_gpt1_name;		/* Backup  GPT partition name to read/write GPT */
	int (*read_func)(int handle, unsigned long long offset, unsigned char * buff_ptr, int length );
	int (*write_func)(int handle, unsigned long long offset, unsigned char * buff_ptr, int length );
	int				fd_prim;		/* File handle for primary GPT */
	int				fd_back;		/* File handle for backup GPT */
	uint64_t			offset_prim;		/* File handle/offset for primary GPT */
	uint64_t			offset_back;		/* File handle/offset for backup GPT */
	unsigned char			*ptr_gpt_prim;		/* Ptr to allocated mem for storing prim GPT */
	unsigned char			*ptr_gpt_back;		/* Ptr to allocated mem for storing prim GPT */
} cfe_gpt_probe_t;


/********************************************************************/
// Function Declaration : call by 'Bcm97xxx_devs.c'
int cfe_gpt_run( cfe_gpt_probe_t *cfe_gpt_part );
int cfe_gpt_init( cfe_gpt_probe_t *cfe_gpt_part );
/********************************************************************/


#endif
