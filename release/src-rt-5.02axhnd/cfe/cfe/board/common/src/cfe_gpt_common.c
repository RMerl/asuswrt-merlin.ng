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

#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "lib_malloc.h"
#include "cfe_error.h"
#include "cfe_timer.h"
#include "cfe_gpt_common.h"

/*  *********************************************************************
    *  Definition
    ********************************************************************* */
/* For GPT Standard Implementation */
/* Protective MBR */
#define GPT_PMBR_SIGNATURE      0xAA55
#define GPT_PMBR_BOOTINDICATOR      0x00
#define GPT_PMBR_STARTING_CHS       0x000200
#define GPT_PMBR_OSTYPE         0xEE
#define GPT_PMBR_ENDING_CHS     0xFFFFFF
#define GPT_PMBR_MAXPARTITION       4       // 1 is for GPT. Max is 4.
/* GPT Header */
#define GPT_HDR_SIGNATURE       0x5452415020494645
#define GPT_HDR_REVISION_INIT       0x00010000      // ver 1.0
#define GPT_HDR_SIZE            92      // GPT_HDR_RESERVED_REST_OFFSET
#define GPT_HDR_PRIMARY_LBA     1
#define GPT_HDR_FIRSTUSABLELBA512   64      // 32KB for GPT. Block address at 512byte/block
#define GPT_HDR_FIRSTUSABLELBA4K    8       // 32KB for GPT. Block address at 4Kbyte/block
/* Partition Entry */
#define GPT_PART_NUM_ENTRY      128
#define GPT_PART_ENTRY_SIZE     128
#define GPT_ENTRY_ATTR_BIT0     0x01
#define GPT_ENTRY_ATTR_BIT1     0x02
#define GPT_ENTRY_ATTR_BIT2     0x04

// Basic data partition : EBD0A0A2-B9E5-4433-87C0-68B6B72699C7
#define GPT_PART_GUID_LINUX_L1      0xEBD0A0A2 
#define GPT_PART_GUID_LINUX_L2      0xB9E5
#define GPT_PART_GUID_LINUX_L3      0x4433
#define GPT_PART_GUID_LINUX_H1      0x87C0
#define GPT_PART_GUID_LINUX_H2      0x68B6B72699C7

#define CFE_PRIMARY_RECOVERY        1
#define CFE_BACKUP_RECOVERY     1

#if 0
#define CFE_GPT_PRIM_NUM_BLOCK_512      34  /* 1(PMBR) + 1(HDR) + 32(ENTRY:128*128/512) */
#define CFE_GPT_PRIM_NUM_BLOCK_4K       6   /* 1(PMBR) + 1(HDR) + 4(ENTRY:128*128/4096) */
#define CFE_GPT_BACK_NUM_BLOCK_512      33  /* 1(HDR) + 32(ENTRY:128*128/512) */
#define CFE_GPT_BACK_NUM_BLOCK_4K       5   /* 1(HDR) + 4(ENTRY:128*128/4096) */
#endif

#define CFE_GPT_BIG_ENDIAN_ON       1
#define CFE_GPT_BIG_ENDIAN_OFF      0

#define GPT_OK              0x00
#define GPT_ERR             -1
#define GPT_ERR_HEADER          0x01
#define GPT_ERR_PARTENTRY       0x02

#ifdef CFG_RUNFROMKSEG0
#define CFE_GPT_BUF_ADDR(x)     (uint8_t *) KERNADDR(x) 
#else
#define CFE_GPT_BUF_ADDR(x)     (uint8_t *) UNCADDR(x)
#endif



/*  *********************************************************************
    The GUIDs in this table are written assuming a little-endian byte order. 
    For example, the GUID for an EFI System partition is written as {C12A7328-F81F-11D2-BA4B-00A0C93EC93B} here, 
    which corresponds to the 16 byte sequence 28h 73h 2Ah C1h 1Fh F8h D2h 11h BAh 4Bh 00h A0h C9h 3Eh C9h 3Bh ? 
    only the first three blocks are byte-swapped.
********************************************************************* */

/*  *********************************************************************
    *  Externs
    ********************************************************************* */

/*  *********************************************************************
    *  Global
    ********************************************************************* */
/* crc_tab[] -- this crcTable is being build by chksum_crc32GenTab().
 *      so make sure, you call it before using the other functions!
 *      From gdisk 6.10!
 */
uint32_t crc_tab[256];


/*  *********************************************************************
    *  Function Declaration
    ********************************************************************* */
int gpt_get_GptFromDisk( cfe_gpt_probe_t *cfe_gpt_probe, int gpt_hdr_idx);
int gpt_set_GptToDisk( cfe_gpt_probe_t *cfe_gpt_probe);
void gpt_get_PMBR( uint8_t *ptr_gpt_prim, gpt_ProtectiveMBR_t *pPMBR );
void gpt_set_PMBR( uint8_t *ptr_gpt_prim, gpt_ProtectiveMBR_t *pPMBR );
void gpt_get_PMBR_PartRcd( uint8_t *ptr_gpt_prim, gpt_PartitionRecord_t *pPartRcd, uint16_t PartRcdOffset );
void gpt_set_PMBR_PartRcd( uint8_t *ptr_gpt_prim, gpt_PartitionRecord_t *pPartRcd, uint16_t PartRcdOffset );
void gpt_get_HDR( uint8_t *ptr_gpt_hdr, gpt_GptHeader_t *pGptHdr );
void gpt_set_HDR( uint8_t *ptr_gpt_hdr, gpt_GptHeader_t *pGptHdr );
int gpt_get_PartitionEntry( uint8_t *ptr_partentry, gpt_PartitionEntry_t *pPartEntry );
int gpt_set_PartitionEntry( uint8_t *ptr_partentry, gpt_PartitionEntry_t *pPartEntry );
uint64_t gpt_get_TableValue( uint8_t *ptr_mem, uint16_t ByteOffset, uint8_t ByteLength, int BigEndian );
void gpt_set_TableValue( uint8_t *ptr_mem, uint16_t ByteOffset, uint8_t ByteLength, uint64_t value, int BigEndian );
// Main functions
int gpt_valid_GptTable( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_hdr_mem, uint8_t *ptr_pentry_mem, uint64_t startLBA, gpt_GptHeader_t *pGptHdr );
int gpt_recover_Header( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_bad, gpt_GptHeader_t *pGptHdrBad, gpt_GptHeader_t *pGptHdr );
int gpt_recover_PartEntry( uint8_t *ptr_bad_partentry, uint8_t *ptr_good_partentry );
int gpt_check_PartEntry( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_pentry_mem, gpt_GptHeader_t *pGptHdr, cfe_gpt_PartEntryStatus_t *pPEntryStatus );
int gpt_check_HeaderCRC( uint32_t MaxSize, uint8_t *ptr_hdr_mem, gpt_GptHeader_t *pGptHdr );
int gpt_check_PartEntryArrayCRC( uint8_t *ptr_pentry_mem, gpt_GptHeader_t *pGptHdr );
uint32_t gpt_set_HeaderCRC( uint8_t *ptr_hdr_mem, gpt_GptHeader_t *pGptHdr );
uint32_t gpt_set_PartEntryArrayCRC( uint8_t *ptr_partentry_mem, gpt_GptHeader_t *pGptHdr );
// From dgisk 6.10
uint32_t chksum_crc32( unsigned char *block, unsigned int length );
void chksum_crc32gentab( void );
// GUID
//static int guid_generate( gpt_GUID_t *newGUID );
//static uint32_t rand( uint32_t seed );
int guid_generate( gpt_GUID_t *newGUID );
uint32_t rand( uint32_t seed );
// Print
#if DEBUG_GPT_PRINT
int gpt_print_table( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_prim, uint8_t *ptr_gpt_back, cfe_gpt_t *pGPT, int usedPrimPartEntry, int usedBackPartEntry );
void gpt_print_PMBR( cfe_gpt_t *pGPT );
void gpt_print_GptHeader( cfe_gpt_t *pGPT );
void gpt_print_PartEntries( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_prim, uint8_t *ptr_gpt_back, cfe_gpt_t *pGPT, int usedPrimPartEntry, int usedBackPartEntry );
#endif



/*  *********************************************************************
    *  File system list
    ********************************************************************* */

/*  *********************************************************************
    *  cfe_gpt_run( cfe_gpt_probe )
    *  
    *  Run GPT partition
    *  
    *  Input parameters: 
    *      cfe_gpt_probe - CFE GPT probe
    *      
    *  Return value:
    *      0 if ok
    *      else error code
    ********************************************************************* */
int cfe_gpt_run( cfe_gpt_probe_t *cfe_gpt_probe )
{
    int i;
    int res=CFE_OK, res_prim=GPT_OK, res_back=GPT_OK;
#if CFG_RAMAPP  
    int res_recover=GPT_OK;
    int truncated_gpt = 0;
#endif  
    cfe_gpt_t   cfe_gpt;
    uint8_t     *ptr_prim_hdr, *ptr_back_hdr;
    uint8_t     *ptr_prim_partentry, *ptr_back_partentry;
    uint32_t    partEntrySize;
    uint64_t    LastBlock;
    cfe_gpt_PartEntryStatus_t PartEntryStatus[CFE_MAX_GPT_PARTITIONS];
#if DEBUG_GPT_PRINT
    int     usedPrimPartEntry=0, usedBackPartEntry=0;
#endif

    //------------------------------------------------------
    //[Step 0] Initialize CRC
    chksum_crc32gentab( );
    
    //------------------------------------------------------
    //[Step 1] Read Primary GPT table data from disk to memory.
    if( gpt_get_GptFromDisk(cfe_gpt_probe, GPT_HDR_IDX_PRIMARY) == CFE_ERR )
    {
        xprintf("---> GPT Primary Reading Error from eMMC(disk)!\n");
        return CFE_ERR_ENVNOTFOUND;
    }
    
    //------------------------------------------------------
    //[Step 2] Get PMBR table & Verify the valid Protective MBR.
#if DEBUG_GPT_VALID
    xprintf("[Step 2] ");
#endif
    gpt_get_PMBR( cfe_gpt_probe->ptr_gpt_prim, &cfe_gpt.PMBR );
    
    for( i=0; i<GPT_MAX_MBR_PARTITIONS; i++)
    {
        if( cfe_gpt.PMBR.PartRcd[i].BootIndicator == 0x00 &&
            cfe_gpt.PMBR.PartRcd[i].OSType == GPT_PMBR_GPT_PARTITION &&
            cfe_gpt.PMBR.PartRcd[i].StartingLBA == 1 )
        {
#if DEBUG_GPT_VALID
            xprintf("Valid Protective MBR!\n");
#endif

#if CFG_RAMAPP
            /* If we know device size, check if GPT covers entire device */
            if( cfe_gpt_probe->lba_size )
            {
                if( cfe_gpt_probe->lba_size != cfe_gpt.PMBR.PartRcd[i].SizeInLBA + 1 )
                {
                    /* Detected a truncated GPT */
                    xprintf("Truncated GPT detected, available_lba:%d, current_lba:%d!\n", cfe_gpt_probe->lba_size, cfe_gpt.PMBR.PartRcd[i].SizeInLBA + 1);
                    truncated_gpt = 1;
                }
            }
            else
#endif    
            {
                /* Update gpt_probe with proper device size in LBA */
                cfe_gpt_probe->lba_size = cfe_gpt.PMBR.PartRcd[i].SizeInLBA + 1;
            }
            break;
        }
        
        if( i == GPT_MAX_MBR_PARTITIONS-1 )
        {
#if DEBUG_GPT_VALID
            xprintf("Not valid Protective MBR!\n");
#endif
            return CFE_ERR_ENVNOTFOUND;

        }
    }

    /* This is a special case which occurs only in cferom
     * when we dont know the full size of the device before
     * calling cfe_gpt_run. In this case fillout the info
     * for the backup GPT header from PMBR data
     */
    if( (cfe_gpt_probe->fd_prim == cfe_gpt_probe->fd_back)
        && (cfe_gpt_probe->offset_prim == cfe_gpt_probe->offset_back) )
    {
        cfe_gpt_probe->offset_back = (cfe_gpt_probe->lba_size * cfe_gpt_probe->block_size) - CFE_GPT_TABLE_SIZE; 
    }

    //------------------------------------------------------
    //[Step 2.5] Read Backup GPT table data from disk to memory.
    if( gpt_get_GptFromDisk(cfe_gpt_probe, GPT_HDR_IDX_BACKUP) == CFE_ERR )
    {
        xprintf("---> GPT Backup Reading Error from eMMC(disk)!\n");
        return CFE_ERR_ENVNOTFOUND;
    }

    //------------------------------------------------------
    //[Step 3] Get primary/backup GPT Header & check validation.
    ptr_prim_hdr        = (uint8_t *)(cfe_gpt_probe->ptr_gpt_prim + cfe_gpt_probe->block_size );
    ptr_back_hdr        = (uint8_t *)(cfe_gpt_probe->ptr_gpt_back + (CFE_GPT_TABLE_SIZE - cfe_gpt_probe->block_size) );
    
    /* Copy GPT header information into our staging cfe_gpt.xxxHDR structs */
    gpt_get_HDR( ptr_prim_hdr, &cfe_gpt.primHDR );
    gpt_get_HDR( ptr_back_hdr, &cfe_gpt.backHDR );

    /* Get partition entry pointers */
    ptr_prim_partentry  = (uint8_t *)( ptr_prim_hdr   + cfe_gpt_probe->block_size ); 
    partEntrySize       = (cfe_gpt.primHDR.NumberOfPartitionEntries * cfe_gpt.primHDR.SizeOfPartitionEntry); 
    ptr_back_partentry  = (uint8_t *)( ptr_back_hdr - partEntrySize );

#if CFG_RAMAPP
    /* If truncated GPT, rewrite PMBR and Prim/2nd GPT to cover whole flash */
    if( truncated_gpt )
    {
        /* Adjust flash size in PMBR */
        cfe_gpt.PMBR.PartRcd[i].SizeInLBA = cfe_gpt_probe->lba_size - 1;
        gpt_set_PMBR( cfe_gpt_probe->ptr_gpt_prim, &cfe_gpt.PMBR );
        
        /* Extend last usable LBA to right before GPT in primary GPT */
        cfe_gpt.primHDR.LastUsableLBA = cfe_gpt_probe->lba_size - (uint64_t)(CFE_GPT_BACKUP_SIZE / cfe_gpt_probe->block_size);
        cfe_gpt.primHDR.AlternateLBA  = cfe_gpt_probe->lba_size - 1;
        gpt_set_HDR( ptr_prim_hdr, &cfe_gpt.primHDR );
        cfe_gpt.primHDR.HeaderCRC32 = gpt_set_HeaderCRC( ptr_prim_hdr, &cfe_gpt.primHDR );

        /* Rewrite backup GPT based on new GPT */
        res_prim = gpt_recover_PartEntry( ptr_back_partentry, ptr_prim_partentry );
        res_prim = gpt_recover_Header(cfe_gpt_probe, ptr_back_hdr, &cfe_gpt.backHDR, &cfe_gpt.primHDR);

        /* Write new headers to flash */
        res_prim = gpt_set_GptToDisk( cfe_gpt_probe );

        if( res_prim != GPT_OK )
        {
            xprintf("!!! GPT expansion failed!!!\n");
            return res_prim;
        }
        else
            xprintf("Expanding GPT to cover entire flash\n");

    }
#endif    

#if DEBUG_GPT_PRINT
    gpt_print_GptHeader( &cfe_gpt );
#endif
    
#if DEBUG_GPT_VALID
    xprintf("[Step 3] Get primary/backup GPT Header!\n");
#endif
    LastBlock = cfe_gpt_probe->lba_size - 1;
    res_prim = gpt_valid_GptTable(cfe_gpt_probe, ptr_prim_hdr, ptr_prim_partentry, GPT_HDR_PRIMARY_LBA, &cfe_gpt.primHDR);
    res_back = gpt_valid_GptTable(cfe_gpt_probe, ptr_back_hdr, ptr_back_partentry, LastBlock, &cfe_gpt.backHDR);

    /* cferom case when we dont want to write anything to eMMC, so we are ok if any table is good */
    if( (res_prim == GPT_OK || res_back == GPT_OK)  && (!cfe_gpt_probe->write_func))
    {
#if DEBUG_GPT_VALID
        xprintf(" Found Valid partition tables\n");
#endif
    }
#if CFG_RAMAPP  
        /* In cfe ram, we will attempt to recover bad GPT tables */
    else if( res_prim == GPT_OK && res_back == GPT_OK )
    {
        // < Case 1 > Primary OK, Backup OK
#if DEBUG_GPT_VALID
        xprintf(" < Case 1 > Valid primary & backup partition table!\n");
        xprintf("            Header validation success!\n");
#endif
    }
    else if( res_prim == GPT_OK && res_back != GPT_OK )
    {
        // < Case 2 > Primary OK, Backup NG -> Recover Backup Header.
#if DEBUG_GPT_VALID
        xprintf(" < Case 2 > Valid primary and not valid backup partition table!\n");
#endif
        xprintf("Backup GPT invalid, recovering from Primary GPT!\n");
        res_recover = GPT_OK;
        if( res_back & GPT_ERR_PARTENTRY )
        {
            res_recover = gpt_recover_PartEntry( ptr_back_partentry, ptr_prim_partentry );
        }
        res_recover += gpt_recover_Header(cfe_gpt_probe, ptr_back_hdr, &cfe_gpt.backHDR, &cfe_gpt.primHDR);
        if( res_recover == GPT_OK )
        {
            if( gpt_valid_GptTable(cfe_gpt_probe, ptr_back_hdr, ptr_back_partentry, LastBlock, &cfe_gpt.backHDR) == CFE_OK ) 
            {
#if DEBUG_GPT_VALID
                xprintf("  Recover backup partition table success!\n");
#endif
                /* Write the recovered partition tables back to the media */
                res_back = gpt_set_GptToDisk( cfe_gpt_probe );
            }
            else
            {
                xprintf("Failed to recover backup GPT!\n");
                return CFE_ERR_ENVNOTFOUND;
            }
        }
        else
        {
#if DEBUG_GPT_VALID
            xprintf("  Recover backup partition table failed!\n");
#endif
            return CFE_ERR_ENVNOTFOUND;
        }

    }
    else if( res_prim != GPT_OK && res_back == GPT_OK )
    {
        // < Case 3 > Primary NG, Backup OK -> Recover Primary Header.
#if DEBUG_GPT_VALID
        xprintf(" < Case 3 > Not valid primary & valid backup partition table!!!\n");
#endif      
        xprintf("Primary GPT invalid, recovering from Backup GPT!\n");
        res_recover = GPT_OK;
        if( res_prim & GPT_ERR_PARTENTRY )
        {
            res_recover = gpt_recover_PartEntry( ptr_prim_partentry, ptr_back_partentry );
        }
        res_recover += gpt_recover_Header(cfe_gpt_probe, ptr_prim_hdr, &cfe_gpt.primHDR, &cfe_gpt.backHDR);
        if( res_recover == GPT_OK )
        {
            if( gpt_valid_GptTable(cfe_gpt_probe, ptr_prim_hdr, ptr_prim_partentry, GPT_HDR_PRIMARY_LBA, &cfe_gpt.primHDR) == CFE_OK ) 
            {
#if DEBUG_GPT_VALID
                xprintf("  Recover primary partition table success!\n");
#endif
                /* Write the recovered partition tables back to the media */
                res_prim = gpt_set_GptToDisk( cfe_gpt_probe );
            }
            else
            {
                xprintf("Failed to recover Primary GPT!\n");
                return CFE_ERR_ENVNOTFOUND;
            }
        }
        else
        {
#if DEBUG_GPT_VALID
            xprintf("  Recover primary partition table failed!\n");
#endif
            return CFE_ERR_ENVNOTFOUND;
        }
        
    }
#endif /* CFG_RAMAPP */
    else    //( res_prim != GPT_OK && res_back != GPT_OK )
    {
        // < Case 4 > Primary NG, Backup NG -> Init.
#if DEBUG_GPT_VALID
        xprintf(" < Case 4 > Not Valid primary & backup partition table!\n");
        xprintf("            GPT totally broken OR No GPT! Setup new valid GPT!\n");
#endif
        return CFE_ERR_ENVNOTFOUND;
    }

#if DEBUG_GPT_PRINT
    //------------------------------------------------------
    //[Step 4] Get Primary Partition Entries
    if( res_prim == GPT_OK )
    {
        xprintf("[Step 4-1.Debug] Get Primary Partition Entries!\n");
        for( i=0; i<cfe_gpt.primHDR.NumberOfPartitionEntries; i++ )
        {   
            ptr_prim_partentry = (uint8_t *)( cfe_gpt_probe->ptr_gpt_prim + ( 2*cfe_gpt_probe->block_size + i*cfe_gpt.primHDR.SizeOfPartitionEntry) ); 
            usedPrimPartEntry += gpt_get_PartitionEntry( ptr_prim_partentry, &cfe_gpt.primPartEntry );
            cfe_gpt.primPartEntry.PartSizeLBA = ( cfe_gpt.primPartEntry.EndingLBA - cfe_gpt.primPartEntry.StartingLBA + 1 );
        }
    }
    
    // Backup Partition Entris
    if( res_back == GPT_OK )
    {
        xprintf("[Step 4-2.Debug] Get Backup Partition Entries!\n");
        for( i=0; i<cfe_gpt.backHDR.NumberOfPartitionEntries; i++ )
        {
            ptr_back_partentry = (uint8_t *)( cfe_gpt_probe->ptr_gpt_back
                                    + ( CFE_GPT_TABLE_SIZE - cfe_gpt_probe->block_size 
                                    - (cfe_gpt.backHDR.NumberOfPartitionEntries - i)*cfe_gpt.backHDR.SizeOfPartitionEntry) ); 
            usedBackPartEntry += gpt_get_PartitionEntry( ptr_back_partentry, &cfe_gpt.backPartEntry );
            cfe_gpt.backPartEntry.PartSizeLBA = ( cfe_gpt.backPartEntry.EndingLBA - cfe_gpt.backPartEntry.StartingLBA + 1 );
        }
    }
    
    res = gpt_print_table( cfe_gpt_probe, 
                ((res_prim == GPT_OK)?cfe_gpt_probe->ptr_gpt_prim:NULL), 
                ((res_back == GPT_OK)?cfe_gpt_probe->ptr_gpt_back:NULL), 
                &cfe_gpt, 
                usedPrimPartEntry, 
                usedBackPartEntry );
#endif

    //------------------------------------------------------
    //[Step 5] Check the integrity of partition entries
#if DEBUG_GPT_VALID
    xprintf("[Step 5] Check the integrity of partition entries!\n");
#endif
    if( res_prim == GPT_OK )
    {
        ptr_prim_partentry = (uint8_t *)( cfe_gpt_probe->ptr_gpt_prim + ( 2*cfe_gpt_probe->block_size ) );
        res = gpt_check_PartEntry( cfe_gpt_probe, ptr_prim_partentry, &cfe_gpt.primHDR, PartEntryStatus );
    }
    else if( res_back == GPT_OK )
    {
        ptr_back_partentry = (uint8_t *)( cfe_gpt_probe->ptr_gpt_back
                            + ( CFE_GPT_TABLE_SIZE - cfe_gpt_probe->block_size 
                            - (cfe_gpt.backHDR.NumberOfPartitionEntries * cfe_gpt.backHDR.SizeOfPartitionEntry)) ); 
        res = gpt_check_PartEntry( cfe_gpt_probe, ptr_back_partentry, &cfe_gpt.backHDR, PartEntryStatus );
    }

    return res;
    
}


#if CFG_RAMAPP
/*  *********************************************************************
    *  cfe_gpt_init( cfe_gpt_probe )
    *  
    *  Initialize a GPT tables using CFE partitions
    *  
    *  Input parameters: 
    *      cfe_gpt_probe - CFE GPT probe
    *      
    *  Return value:
    *      0 if ok
    *      else error code
    ********************************************************************* */
int cfe_gpt_init( cfe_gpt_probe_t *cfe_gpt_probe )
{
    uint8_t                 i, j;
    gpt_PartitionEntry_t    PartEntry;
    gpt_GptHeader_t         PrimHeader;
    gpt_GptHeader_t         BackHeader;
    gpt_ProtectiveMBR_t     ProtMBR;
    //gpt_PartitionRecord_t PartRecord;
    
    int         res;
    uint64_t    EntryStartLBA, EntryEndLBA;
    
    uint8_t     *ptr_prim_hdr, *ptr_back_hdr;
    uint8_t     *ptr_prim_partentry, *ptr_back_partentry;
    uint8_t     *ptr_partenty;
    uint32_t    PartEntrySize;
    
    // Memory Initialization to Zero
    memset(cfe_gpt_probe->ptr_gpt_prim, 0, CFE_GPT_PRIMRY_SIZE);
    memset(cfe_gpt_probe->ptr_gpt_back, 0, CFE_GPT_BACKUP_SIZE);

    ptr_prim_hdr        = (uint8_t *)( cfe_gpt_probe->ptr_gpt_prim + cfe_gpt_probe->block_size );
    ptr_prim_partentry  = (uint8_t *)( ptr_prim_hdr   + cfe_gpt_probe->block_size ); 
    ptr_back_hdr        = (uint8_t *)( cfe_gpt_probe->ptr_gpt_back + (CFE_GPT_BACKUP_SIZE - cfe_gpt_probe->block_size) );
    PartEntrySize       = GPT_PART_NUM_ENTRY*GPT_PART_ENTRY_SIZE;
    ptr_back_partentry  = (uint8_t *)( ptr_back_hdr - PartEntrySize );
    
    chksum_crc32gentab( );
    
    //------------------------------------------------------
    // [Step 1] Partition Entry
    EntryStartLBA   = cfe_gpt_probe->cfe_FirstUsableLba;
    EntryEndLBA     = cfe_gpt_probe->cfe_LastUsableLba;
    ptr_partenty    = ptr_prim_partentry;
    
    // Exclusive cfe gpt0/gpt1 patitions
    for( i=1; i<cfe_gpt_probe->num_parts-1; i++ )
    {
        memset( &PartEntry, 0, sizeof(gpt_PartitionEntry_t) );
        PartEntry.PartitionNumber = i-1;    
        PartEntry.PartSizeLBA = (uint64_t)( cfe_gpt_probe->cfe_parts[i].fp_size >> cfe_gpt_probe->block_size_bit );
        //xprintf(" >>> PartEntry.PartSizeLBA = 0x%016llX!\n", PartEntry.PartSizeLBA);
        // PartitionTypeGUID : Linux/Window basic data partition
        PartEntry.PartitionTypeGUID.l1 = GPT_PART_GUID_LINUX_L1;
        PartEntry.PartitionTypeGUID.l2 = GPT_PART_GUID_LINUX_L2;
        PartEntry.PartitionTypeGUID.l3 = GPT_PART_GUID_LINUX_L3;
        PartEntry.PartitionTypeGUID.h1 = GPT_PART_GUID_LINUX_H1;
        PartEntry.PartitionTypeGUID.h2 = GPT_PART_GUID_LINUX_H2;
        // UniquePartitionGUID
        guid_generate( &PartEntry.UniquePartitionGUID );
        // StartingLBA, EndingLBA, Attributes
        PartEntry.StartingLBA   = cfe_gpt_probe->cfe_parts[i].fp_offset_bytes >> cfe_gpt_probe->block_size_bit;
        PartEntry.EndingLBA     = PartEntry.StartingLBA + PartEntry.PartSizeLBA - 1;

        if( PartEntry.StartingLBA < EntryStartLBA || PartEntry.EndingLBA > EntryEndLBA )
        {
            xprintf("Invalid GPT partition offsets!\n");
            return GPT_ERR;
        }

        PartEntry.Attributes    = 0x00;
        
        // Copy over partition names in UTF-16 format
        memset(PartEntry.PartitionName, 0, GPT_PART_PARITIONNAME_LENGTH);
        j = 0;
        do
        {
            //xprintf("%c", tmp_char);
            PartEntry.PartitionName[j*2]    = cfe_gpt_probe->cfe_parts[i].fp_name[j];
            PartEntry.PartitionName[j*2+1]  = 0x00;
            j++;
        } while( cfe_gpt_probe->cfe_parts[i].fp_name[j] != '\0' && j < CFE_GPT_MAX_PART_NAME);
        //xprintf("\n");
        
        res = gpt_set_PartitionEntry( ptr_partenty, &PartEntry );
        ptr_partenty = (uint8_t *)(ptr_partenty + GPT_PART_ENTRY_SIZE);
        
    }
    // Copy primary partition entries to backup partition entries.
    memcpy( ptr_back_partentry, ptr_prim_partentry, PartEntrySize );
    
    //------------------------------------------------------
    // [Step 2] GPT Header
    memset( &PrimHeader, 0, sizeof(gpt_GptHeader_t) );
    PrimHeader.Signature    = GPT_HDR_SIGNATURE;
    PrimHeader.Revision     = GPT_HDR_REVISION_INIT;
    PrimHeader.HeaderSize   = GPT_HDR_SIZE;
    PrimHeader.HeaderCRC32  = 0x00000000;   // Initial value to get CRC
    PrimHeader.Reserved     = 0x00000000;
    // LBA
    PrimHeader.MyLBA            = 1;
    PrimHeader.AlternateLBA     = cfe_gpt_probe->lba_size - 1;
    PrimHeader.FirstUsableLBA   = (uint64_t)(CFE_GPT_PRIMRY_SIZE / cfe_gpt_probe->block_size);
    PrimHeader.LastUsableLBA    = cfe_gpt_probe->lba_size - (uint64_t)(CFE_GPT_BACKUP_SIZE / cfe_gpt_probe->block_size);
    guid_generate( &PrimHeader.DiskGUID );
    PrimHeader.PartitionEntryLBA        = GPT_HDR_PRIMARY_LBA + 1;
    PrimHeader.NumberOfPartitionEntries = GPT_PART_NUM_ENTRY;
    PrimHeader.SizeOfPartitionEntry     = GPT_PART_ENTRY_SIZE;
    PrimHeader.PartitionEntryArrayCRC32 =  gpt_set_PartEntryArrayCRC( ptr_prim_partentry, &PrimHeader );
    // Get & save CRC for primary header
    gpt_set_HDR( ptr_prim_hdr, &PrimHeader );
    PrimHeader.HeaderCRC32 = gpt_set_HeaderCRC( ptr_prim_hdr, &PrimHeader );
    gpt_set_TableValue( ptr_prim_hdr, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)PrimHeader.HeaderCRC32, CFE_GPT_BIG_ENDIAN_OFF);
    
    //------------------------------------------------------
    // [Step 3] Backup header of GPT
    //memcpy( ptr_back_hdr, ptr_prim_hdr, GPT_HDR_SIZE );
    memcpy( &BackHeader, &PrimHeader, sizeof(gpt_GptHeader_t) );
    BackHeader.HeaderCRC32  = 0x00000000;   // Initial value to get CRC
    BackHeader.MyLBA        = PrimHeader.AlternateLBA;
    BackHeader.AlternateLBA = PrimHeader.MyLBA;
    BackHeader.PartitionEntryLBA = cfe_gpt_probe->lba_size - ((uint64_t)PartEntrySize>>cfe_gpt_probe->block_size_bit) - 1;  // - header_size
    //BackHeader.PartitionEntryLBA = PrimHeader.LastUsableLBA + 1;
    // Save data into memory
    gpt_set_HDR( ptr_back_hdr, &BackHeader );
    // Get & save CRC for backup header
    BackHeader.HeaderCRC32 = gpt_set_HeaderCRC( ptr_back_hdr, &BackHeader );
    gpt_set_TableValue( ptr_back_hdr, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)BackHeader.HeaderCRC32, CFE_GPT_BIG_ENDIAN_OFF);
    
    //------------------------------------------------------
    // [Step 4] Protective MBR
    memset( &ProtMBR, 0, sizeof(gpt_ProtectiveMBR_t) );
    ProtMBR.Signature = GPT_PMBR_SIGNATURE;
    ProtMBR.PartRcd[0].BootIndicator    = GPT_PMBR_BOOTINDICATOR;
    ProtMBR.PartRcd[0].StartingCHS      = GPT_PMBR_STARTING_CHS;
    ProtMBR.PartRcd[0].OSType           = GPT_PMBR_OSTYPE;
    ProtMBR.PartRcd[0].EndingCHS        = GPT_PMBR_ENDING_CHS;  //cfe_gpt_probe->lba_size - 1;
    ProtMBR.PartRcd[0].StartingLBA      = GPT_HDR_PRIMARY_LBA;
    ProtMBR.PartRcd[0].SizeInLBA        = cfe_gpt_probe->lba_size - 1;
    if( (cfe_gpt_probe->lba_size - 1) > 0xFFFFFFFF )
        ProtMBR.PartRcd[0].SizeInLBA = 0xFFFFFFFF;
    for( i=1; i<4; i++ )
    {
        ProtMBR.PartRcd[i].BootIndicator    = 0;
        ProtMBR.PartRcd[i].StartingCHS      = 0;
        ProtMBR.PartRcd[i].OSType           = 0;
        ProtMBR.PartRcd[i].EndingCHS        = 0;
        ProtMBR.PartRcd[i].StartingLBA      = 0;
        ProtMBR.PartRcd[i].SizeInLBA        = 0;
    }   
    // Write to memory
    gpt_set_PMBR( cfe_gpt_probe->ptr_gpt_prim, &ProtMBR );
    
    //------------------------------------------------------
    // [Step 4] Copy to eMMC/disk from memory
    res = gpt_set_GptToDisk( cfe_gpt_probe );

    (void)EntryEndLBA;
    (void)res;
        
    return GPT_OK;
    
}
#endif

/*  *********************************************************************
    *  gpt_get_GptFromDisk(fsctx,name)
    *  
    *  Copy whole GPT partition(32K) from disk(eMMC) to memory
    *  
    *  Input parameters: 
    *      fsctx - result from a previous fs_init 
    *      name - name of fs to hook onto the beginning
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

int gpt_get_GptFromDisk( cfe_gpt_probe_t *cfe_gpt_probe, int gpt_hdr_idx)
{
    int res = -1;

    if( gpt_hdr_idx == GPT_HDR_IDX_PRIMARY )
    {
        if (cfe_gpt_probe->fd_prim < 0) 
        {
        
            xprintf("Cannot open primary gpt!\n");
            return GPT_ERR;
        }
        else
        {
                res = cfe_gpt_probe->read_func( cfe_gpt_probe->fd_prim, 
                            cfe_gpt_probe->offset_prim, 
                            (unsigned char*)cfe_gpt_probe->ptr_gpt_prim, 
                            CFE_GPT_TABLE_SIZE );   //CFE_GPT_PRIM_NUM_BLOCK_4K );
        }
    }
    else
    {
        if (cfe_gpt_probe->fd_back < 0) 
        {
            xprintf("Cannot open backup gpt!\n");
            return GPT_ERR;
        }
        else
        {
            res = cfe_gpt_probe->read_func( cfe_gpt_probe->fd_back, 
                            cfe_gpt_probe->offset_back, 
                            (unsigned char*)cfe_gpt_probe->ptr_gpt_back, 
                            CFE_GPT_TABLE_SIZE );   //CFE_GPT_BACK_NUM_BLOCK_4K );
        }
    }
    
    if( res < 0 )
        return GPT_ERR;
    else
        return GPT_OK;
    
}

/*  *********************************************************************
    *  gpt_set_GptToDisk( cfe_gpt_probe, ptr_gpt_prim, ptr_gpt_back )
    *  
    *  Set GPT table to the disk(eMMC) a filesystem to attach a filter, like a decompression
    *  or decryption engine.
    *  
    *  Input parameters: 
    *      fsctx - result from a previous fs_init 
    *      name - name of fs to hook onto the beginning
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

int gpt_set_GptToDisk( cfe_gpt_probe_t *cfe_gpt_probe )
{
    int res;

    if (cfe_gpt_probe->fd_prim < 0) 
    {
    
        xprintf("Cannot open primary gpt!\n");
        return GPT_ERR;
    }
    else
    {
            res = cfe_gpt_probe->write_func( cfe_gpt_probe->fd_prim, 
                        cfe_gpt_probe->offset_prim, 
                        (unsigned char*)cfe_gpt_probe->ptr_gpt_prim, 
                        CFE_GPT_TABLE_SIZE );   //CFE_GPT_PRIM_NUM_BLOCK_4K );
    }
    
    if (cfe_gpt_probe->fd_back < 0) 
    {
        xprintf("Cannot open backup gpt!\n");
        return GPT_ERR;
    }
    else
    {
        res = cfe_gpt_probe->write_func( cfe_gpt_probe->fd_back, 
                        cfe_gpt_probe->offset_back, 
                        (unsigned char*)cfe_gpt_probe->ptr_gpt_back, 
                        CFE_GPT_TABLE_SIZE );   //CFE_GPT_BACK_NUM_BLOCK_4K );
    }
    
    if( res < 0 )
        return GPT_ERR;
    else
        return GPT_OK;
    
}

/*  *********************************************************************
    *  gpt_get_PMBR( ptr_mem, pPMBR )
    *  
    *  Get Protective MBR Values.
    *  
    *  Input parameters: 
    *      ptr_gpt_prim - staging memory pointer of primary GPT
    *      pPMBR        - Protective MBR tables
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

void gpt_get_PMBR( uint8_t *ptr_gpt_prim, gpt_ProtectiveMBR_t *pPMBR )
{
    uint16_t PartRcdOffset;
    
    pPMBR->Signature = (uint64_t)gpt_get_TableValue( ptr_gpt_prim, GPT_PMBR_SIGNATURE_OFFSET, GPT_PMBR_SIGNATURE_LENGTH, CFE_GPT_BIG_ENDIAN_OFF );
    
    PartRcdOffset = GPT_PMBR_PARTITIONRECORD_OFFSET;
    gpt_get_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[0], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_get_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[1], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_get_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[2], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_get_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[3], PartRcdOffset );
    
}

void gpt_get_PMBR_PartRcd( uint8_t *ptr_gpt_prim, gpt_PartitionRecord_t *pPartRcd, uint16_t PartRcdOffset )
{
    pPartRcd->BootIndicator = (uint8_t )gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_BOOTINDICATOR_OFFSET   , GPT_PMBR_PARTRCD_BOOTINDICATOR_LENGTH , CFE_GPT_BIG_ENDIAN_OFF);
    pPartRcd->StartingCHS   = (uint32_t)gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_STARTINGCHS_OFFSET     , GPT_PMBR_PARTRCD_STARTINGCHS_LENGTH   , CFE_GPT_BIG_ENDIAN_OFF);
    pPartRcd->OSType        = (uint8_t )gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_OSTYPE_OFFSET          , GPT_PMBR_PARTRCD_OSTYPE_LENGTH        , CFE_GPT_BIG_ENDIAN_OFF);
    pPartRcd->EndingCHS     = (uint32_t)gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_ENDINGCHS_OFFSET       , GPT_PMBR_PARTRCD_ENDINGCHS_LENGTH     , CFE_GPT_BIG_ENDIAN_OFF);
    pPartRcd->StartingLBA   = (uint32_t)gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_STARTINGLBA_OFFSET     , GPT_PMBR_PARTRCD_STARTINGLBA_LENGTH   , CFE_GPT_BIG_ENDIAN_OFF);
    pPartRcd->SizeInLBA     = (uint32_t)gpt_get_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_SIZEINLBA_OFFSET       , GPT_PMBR_PARTRCD_SIZEINLBA_LENGTH     , CFE_GPT_BIG_ENDIAN_OFF);
    
}

#if CFG_RAMAPP
void gpt_set_PMBR( uint8_t *ptr_gpt_prim, gpt_ProtectiveMBR_t *pPMBR )
{
    uint16_t PartRcdOffset;
    
    gpt_set_TableValue( ptr_gpt_prim, GPT_PMBR_SIGNATURE_OFFSET, GPT_PMBR_SIGNATURE_LENGTH, (uint64_t)pPMBR->Signature, CFE_GPT_BIG_ENDIAN_OFF );
    
    PartRcdOffset = GPT_PMBR_PARTITIONRECORD_OFFSET;
    gpt_set_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[0], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_set_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[1], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_set_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[2], PartRcdOffset );
    
    PartRcdOffset += 16;
    gpt_set_PMBR_PartRcd( ptr_gpt_prim, &pPMBR->PartRcd[3], PartRcdOffset );
    
}


void gpt_set_PMBR_PartRcd( uint8_t *ptr_gpt_prim, gpt_PartitionRecord_t *pPartRcd, uint16_t PartRcdOffset )
{
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_BOOTINDICATOR_OFFSET   , GPT_PMBR_PARTRCD_BOOTINDICATOR_LENGTH , (uint64_t)pPartRcd->BootIndicator, CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_STARTINGCHS_OFFSET     , GPT_PMBR_PARTRCD_STARTINGCHS_LENGTH   , (uint64_t)pPartRcd->StartingCHS  , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_OSTYPE_OFFSET          , GPT_PMBR_PARTRCD_OSTYPE_LENGTH        , (uint64_t)pPartRcd->OSType       , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_ENDINGCHS_OFFSET       , GPT_PMBR_PARTRCD_ENDINGCHS_LENGTH     , (uint64_t)pPartRcd->EndingCHS    , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_STARTINGLBA_OFFSET     , GPT_PMBR_PARTRCD_STARTINGLBA_LENGTH   , (uint64_t)pPartRcd->StartingLBA  , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_prim, PartRcdOffset+GPT_PMBR_PARTRCD_SIZEINLBA_OFFSET       , GPT_PMBR_PARTRCD_SIZEINLBA_LENGTH     , (uint64_t)pPartRcd->SizeInLBA    , CFE_GPT_BIG_ENDIAN_OFF);
    
}
#endif

/*  *********************************************************************
    *  gpt_get_HDR( ptr_gpt_prim, ptr_gpt_back, pGPT )
    *  
    *  Get Header Values.
    *  
    *  Input parameters: 
    *      ptr_prim_hdr     - staging memory pointer of primary GPT header
    *      ptr_back_hdr     - staging memory pointer of backup GPT header
    *      pGPT             - GPT tables
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

void gpt_get_HDR( uint8_t *ptr_gpt_hdr, gpt_GptHeader_t *pGptHdr )
{
    pGptHdr->Signature                  = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_SIGNATURE_OFFSET               , GPT_HDR_SIGNATURE_LENGTH               , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->Revision                   = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_REVISION_OFFSET                , GPT_HDR_REVISION_LENGTH                , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->HeaderSize                 = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_HEADERSIZE_OFFSET              , GPT_HDR_REVISION_LENGTH                , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->HeaderCRC32                = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_HEADERCRC32_OFFSET             , GPT_HDR_HEADERCRC32_LENGTH             , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->Reserved                   = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_RESERVED_OFFSET                , GPT_HDR_RESERVED_LENGTH                , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->MyLBA                      = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_MYLBA_OFFSET                   , GPT_HDR_MYLBA_LENGTH                   , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->AlternateLBA               = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_ALTERNATELBA_OFFSET            , GPT_HDR_ALTERNATELBA_LENGTH            , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->FirstUsableLBA             = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_FIRSTUSABLELBA_OFFSET          , GPT_HDR_FIRSTUSABLELBA_LENGTH          , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->LastUsableLBA              = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_LASTUSABLELBA_OFFSET           , GPT_HDR_LASTUSABLELBA_LENGTH           , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->DiskGUID.l1                = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L1             , GPT_HDR_DISKGUID_LENGTH_L1             , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->DiskGUID.l2                = (uint16_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L2             , GPT_HDR_DISKGUID_LENGTH_L2             , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->DiskGUID.l3                = (uint16_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L3             , GPT_HDR_DISKGUID_LENGTH_L3             , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->DiskGUID.h1                = (uint16_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_H1             , GPT_HDR_DISKGUID_LENGTH_H1             , CFE_GPT_BIG_ENDIAN_ON  );
    pGptHdr->DiskGUID.h2                = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_H2             , GPT_HDR_DISKGUID_LENGTH_H2             , CFE_GPT_BIG_ENDIAN_ON  );
    pGptHdr->PartitionEntryLBA          = (uint64_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_PARTITIONENTRYLBA_OFFSET       , GPT_HDR_PARTITIONENTRYLBA_LENGTH       , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->NumberOfPartitionEntries   = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_NUMBEROFPARTITIONENTRIES_OFFSET, GPT_HDR_NUMBEROFPARTITIONENTRIES_LENGTH, CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->SizeOfPartitionEntry       = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_SIZEOFPARTITIONENTRY_OFFSET    , GPT_HDR_SIZEOFPARTITIONENTRY_LENGTH    , CFE_GPT_BIG_ENDIAN_OFF );
    pGptHdr->PartitionEntryArrayCRC32   = (uint32_t)gpt_get_TableValue( ptr_gpt_hdr, GPT_HDR_PARTITIONENTRYARRAYCRC32_OFFSET, GPT_HDR_PARTITIONENTRYARRAYCRC32_LENGTH, CFE_GPT_BIG_ENDIAN_OFF );
    
}

#if CFG_RAMAPP
void gpt_set_HDR( uint8_t *ptr_gpt_hdr, gpt_GptHeader_t *pGptHdr )
{
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_SIGNATURE_OFFSET               , GPT_HDR_SIGNATURE_LENGTH               , (uint64_t)pGptHdr->Signature               , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_REVISION_OFFSET                , GPT_HDR_REVISION_LENGTH                , (uint64_t)pGptHdr->Revision                , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_HEADERSIZE_OFFSET              , GPT_HDR_REVISION_LENGTH                , (uint64_t)pGptHdr->HeaderSize              , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_HEADERCRC32_OFFSET             , GPT_HDR_HEADERCRC32_LENGTH             , (uint64_t)pGptHdr->HeaderCRC32             , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_RESERVED_OFFSET                , GPT_HDR_RESERVED_LENGTH                , (uint64_t)pGptHdr->Reserved                , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_MYLBA_OFFSET                   , GPT_HDR_MYLBA_LENGTH                   , (uint64_t)pGptHdr->MyLBA                   , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_ALTERNATELBA_OFFSET            , GPT_HDR_ALTERNATELBA_LENGTH            , (uint64_t)pGptHdr->AlternateLBA            , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_FIRSTUSABLELBA_OFFSET          , GPT_HDR_FIRSTUSABLELBA_LENGTH          , (uint64_t)pGptHdr->FirstUsableLBA          , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_LASTUSABLELBA_OFFSET           , GPT_HDR_LASTUSABLELBA_LENGTH           , (uint64_t)pGptHdr->LastUsableLBA           , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L1             , GPT_HDR_DISKGUID_LENGTH_L1             , (uint64_t)pGptHdr->DiskGUID.l1             , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L2             , GPT_HDR_DISKGUID_LENGTH_L2             , (uint64_t)pGptHdr->DiskGUID.l2             , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_L3             , GPT_HDR_DISKGUID_LENGTH_L3             , (uint64_t)pGptHdr->DiskGUID.l3             , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_H1             , GPT_HDR_DISKGUID_LENGTH_H1             , (uint64_t)pGptHdr->DiskGUID.h1             , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_DISKGUID_OFFSET_H2             , GPT_HDR_DISKGUID_LENGTH_H2             , (uint64_t)pGptHdr->DiskGUID.h2             , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_PARTITIONENTRYLBA_OFFSET       , GPT_HDR_PARTITIONENTRYLBA_LENGTH       , (uint64_t)pGptHdr->PartitionEntryLBA       , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_NUMBEROFPARTITIONENTRIES_OFFSET, GPT_HDR_NUMBEROFPARTITIONENTRIES_LENGTH, (uint64_t)pGptHdr->NumberOfPartitionEntries, CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_SIZEOFPARTITIONENTRY_OFFSET    , GPT_HDR_SIZEOFPARTITIONENTRY_LENGTH    , (uint64_t)pGptHdr->SizeOfPartitionEntry    , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_gpt_hdr, GPT_HDR_PARTITIONENTRYARRAYCRC32_OFFSET, GPT_HDR_PARTITIONENTRYARRAYCRC32_LENGTH, (uint64_t)pGptHdr->PartitionEntryArrayCRC32, CFE_GPT_BIG_ENDIAN_OFF);
    
}
#endif

/*  *********************************************************************
    *  gpt_get_PartitionEntry( ptr_gpt, pGPT )
    *  
    *  Get Partition Entry Values.
    *  
    *  Input parameters: 
    *      ptr_partentry    - staging memory pointer of GPT Partition Enty
    *      pGPT             - GPT tables
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

int gpt_get_PartitionEntry( uint8_t *ptr_partentry, gpt_PartitionEntry_t *pPartEntry )
{
    uint16_t i;
    
    pPartEntry->PartitionTypeGUID.l1    = (uint32_t)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L1      , GPT_PART_PARITIONTYPEGUID_LENGTH_L1    , CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->PartitionTypeGUID.l2    = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L2      , GPT_PART_PARITIONTYPEGUID_LENGTH_L2    , CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->PartitionTypeGUID.l3    = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L3      , GPT_PART_PARITIONTYPEGUID_LENGTH_L3    , CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->PartitionTypeGUID.h1    = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_H1      , GPT_PART_PARITIONTYPEGUID_LENGTH_H1    , CFE_GPT_BIG_ENDIAN_ON );
    pPartEntry->PartitionTypeGUID.h2    = (uint64_t)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_H2      , GPT_PART_PARITIONTYPEGUID_LENGTH_H2    , CFE_GPT_BIG_ENDIAN_ON );
    pPartEntry->UniquePartitionGUID.l1  = (uint32_t)gpt_get_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L1  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L1, CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->UniquePartitionGUID.l2  = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L2  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L2, CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->UniquePartitionGUID.l3  = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L3  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L3, CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->UniquePartitionGUID.h1  = (uint16_t)gpt_get_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H1  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H1, CFE_GPT_BIG_ENDIAN_ON );
    pPartEntry->UniquePartitionGUID.h2  = (uint64_t)gpt_get_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H2  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H2, CFE_GPT_BIG_ENDIAN_ON );
    pPartEntry->StartingLBA             = (uint64_t)gpt_get_TableValue( ptr_partentry, GPT_PART_STARTINGLBA_OFFSET              , GPT_PART_STARTINGLBA_LENGTH            , CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->EndingLBA               = (uint64_t)gpt_get_TableValue( ptr_partentry, GPT_PART_ENDINGLBA_OFFSET                , GPT_PART_ENDINGLBA_LENGTH              , CFE_GPT_BIG_ENDIAN_OFF);
    pPartEntry->Attributes              = (uint64_t)gpt_get_TableValue( ptr_partentry, GPT_PART_ATTRIBUTES_OFFSET               , GPT_PART_ATTRIBUTES_LENGTH             , CFE_GPT_BIG_ENDIAN_OFF);
    
    for( i=0; i<GPT_PART_PARITIONNAME_LENGTH; i++ )
    {
        pPartEntry->PartitionName[i] = (char)gpt_get_TableValue( ptr_partentry, GPT_PART_PARITIONNAME_OFFSET+i, 1, CFE_GPT_BIG_ENDIAN_OFF );
    }
    
    if( pPartEntry->PartitionTypeGUID.l1 != 0x00000000 )
    {
        return 1;
    }
    else
    {
        return GPT_OK;
    }
        
}

#if CFG_RAMAPP
int gpt_set_PartitionEntry( uint8_t *ptr_partentry, gpt_PartitionEntry_t *pPartEntry )
{
    uint16_t i;
    
    gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L1      , GPT_PART_PARITIONTYPEGUID_LENGTH_L1    , (uint64_t)pPartEntry->PartitionTypeGUID.l1   , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L2      , GPT_PART_PARITIONTYPEGUID_LENGTH_L2    , (uint64_t)pPartEntry->PartitionTypeGUID.l2   , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_L3      , GPT_PART_PARITIONTYPEGUID_LENGTH_L3    , (uint64_t)pPartEntry->PartitionTypeGUID.l3   , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_H1      , GPT_PART_PARITIONTYPEGUID_LENGTH_H1    , (uint64_t)pPartEntry->PartitionTypeGUID.h1   , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONTYPEGUID_OFFSET_H2      , GPT_PART_PARITIONTYPEGUID_LENGTH_H2    , (uint64_t)pPartEntry->PartitionTypeGUID.h2   , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L1  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L1, (uint64_t)pPartEntry->UniquePartitionGUID.l1 , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L2  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L2, (uint64_t)pPartEntry->UniquePartitionGUID.l2 , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_L3  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_L3, (uint64_t)pPartEntry->UniquePartitionGUID.l3 , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H1  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H1, (uint64_t)pPartEntry->UniquePartitionGUID.h1 , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_partentry, GPT_PART_UNIQUEPARTITIONSGUID_OFFSET_H2  , GPT_PART_UNIQUEPARTITIONSGUID_LENGTH_H2, (uint64_t)pPartEntry->UniquePartitionGUID.h2 , CFE_GPT_BIG_ENDIAN_ON );
    gpt_set_TableValue( ptr_partentry, GPT_PART_STARTINGLBA_OFFSET              , GPT_PART_STARTINGLBA_LENGTH            , (uint64_t)pPartEntry->StartingLBA            , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_ENDINGLBA_OFFSET                , GPT_PART_ENDINGLBA_LENGTH              , (uint64_t)pPartEntry->EndingLBA              , CFE_GPT_BIG_ENDIAN_OFF);
    gpt_set_TableValue( ptr_partentry, GPT_PART_ATTRIBUTES_OFFSET               , GPT_PART_ATTRIBUTES_LENGTH             , (uint64_t)pPartEntry->Attributes             , CFE_GPT_BIG_ENDIAN_OFF);
    
    for( i=0; i<GPT_PART_PARITIONNAME_LENGTH; i++ )
    {
         gpt_set_TableValue( ptr_partentry, GPT_PART_PARITIONNAME_OFFSET+i, 1, (uint64_t)pPartEntry->PartitionName[i], CFE_GPT_BIG_ENDIAN_OFF );
    }

    return GPT_OK;
    
}
#endif


/*  *********************************************************************
    *  gpt_get_TableValue/gpt_set_TableValue( ptr_mem, ByteOffset, ByteLength )
    *  
    *  Decode GPT table.
    *
    *  Input parameters: 
    *      ptr_mem     - staging memory pointer of table
    *      ByteOffset  - offset for mnemonic
    *      ByteLength  - offset for mnemonic
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

uint64_t gpt_get_TableValue( uint8_t *ptr_mem, uint16_t ByteOffset, uint8_t ByteLength, int BigEndian  )
{
    uint64_t value=0;
    int      i, j;
    
#if !defined(CFG_BIENDIAN)
    if( BigEndian == CFE_GPT_BIG_ENDIAN_OFF )
    {
        for( i=0; i<ByteLength; i++ )
        {
            value += (uint64_t)(*(ptr_mem+ByteOffset+i)) << (8*i);
        }
    }
    else
    {
        for( i=0; i<ByteLength; i++ )
        {
            j = (ByteLength - 1) - i;
            value += (uint64_t)(*(ptr_mem+ByteOffset+j)) << (8*i);
        }
    }       
#else   // Big Endian
    for( i=0; i<ByteLength; i++ )
    {
        j = (ByteLength - 1) - i;
        value += (uint64_t)(*(ptr_mem+ByteOffset+j)) << (8*i);
    }
#endif
        
    return value;
    
}


void gpt_set_TableValue( uint8_t *ptr_mem, uint16_t ByteOffset, uint8_t ByteLength, uint64_t value, int BigEndian )
{
    uint8_t i=0, j=0;
    
#if !defined(CFG_BIENDIAN)
    if( BigEndian == CFE_GPT_BIG_ENDIAN_OFF )
    {
        for( i=0; i<ByteLength; i++ )
        {
            *(ptr_mem+ByteOffset+i) = (uint8_t)( (value >> (8*i)) & 0xFF );
        }   
    }
    else
    {
        for( i=0; i<ByteLength; i++ )
        {
            j = (ByteLength - 1) - i;
            *(ptr_mem+ByteOffset+j) = (uint8_t)( (value >> (8*i)) & 0xFF );
        }
    }
#else   // Big Endian
    for( i=0; i<ByteLength; i++ )
    {
        j = (ByteLength - 1) - i;
        *(ptr_mem+ByteOffset+j) = (uint8_t)( (value >> (8*i)) & 0xFF );
    }   
#endif
}


/*  *********************************************************************
    *  gpt_valid_GptTable( block_io, disk_io, lba, part_header )
    *  
    *  Install child handles if the Handle supports GPT partition structure.
    *  Caution: This function may receive untrusted input.
    *   The GPT partition table header is external input, so this routine
    *   will do basic validation for GPT partition table header before return.
    *  
    *  Input parameters: 
    *      cfe_gpt_probe    - CFE GPT probe
    *      ptr_gpt_mem      - Staging memory pointer of GPT header
    *      startLBA         - The starting Lba of the Partition Table
    *      pGptHdr          - Partition table header structure.
    *      
    *  Return value:
    *      if ok      - the partition table is valid
    *      else error - the partition table is invalid
    ********************************************************************* */

int gpt_valid_GptTable( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_hdr_mem, uint8_t *ptr_pentry_mem, uint64_t startLBA, gpt_GptHeader_t *pGptHdr )
{
    int res=GPT_OK;
    
    //[Step 1] Check Signature, CRC, MyLBA in Header
    if( (pGptHdr->Signature != GPT_HDR_SIGNATURE)
        || (gpt_check_HeaderCRC(cfe_gpt_probe->block_size, ptr_hdr_mem, pGptHdr)!=GPT_OK)
        || pGptHdr->MyLBA != startLBA )
    {
#if DEBUG_GPT_VALID
        xprintf("   Invalid EFI partition table header!\n");
#endif
        res = GPT_ERR_HEADER;
    }
    
    //[Step 2] Check CRC in Partition Entry
    if( gpt_check_PartEntryArrayCRC(ptr_pentry_mem, pGptHdr) != GPT_OK )
    {
#if DEBUG_GPT_VALID
        xprintf("   Invalid EFI partition entry!\n");
#endif
        res = GPT_ERR_PARTENTRY;
    }

    if( res != GPT_OK )
    {
        return res;
    }
    else
    {
#if DEBUG_GPT_VALID
        xprintf("   Valid EFI partition table header & entry!!!\n");
#endif
        return GPT_OK;
    }   
    
}

#if CFG_RAMAPP
/*  *********************************************************************
    *  gpt_recover_Header( block_io, disk_io, part_header )
    *  
    *  Restore Partition Table to its alternate place
    *  Caution: This function may receive untrusted input.
    *   (Primary -> Backup or Backup -> Primary).
    *  
    *  Input parameters: 
    *      cfe_gpt_probe    - Parent BlockIo interface.
    *      ptr_gpt_bad      - Staging memory pointer of not valide GPT header to recover.
    *      pGptHdrBad       - Not valid partition table header structure to recover.
    *      pGptHdr          - Partition table header structure.
    *      
    *  Return value:
    *      0 if ok    - Restoring succeeds
    *      else error - Restoring failed
    ********************************************************************* */

int gpt_recover_Header( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_bad, gpt_GptHeader_t *pGptHdrBad, gpt_GptHeader_t *pGptHdr )
{
    uint64_t    partEntryLBA;
    uint64_t    PartEntrySizeLBA;

    
    
    //[Step 1] Detect primary or backup table and ready new partEntryLBA
    // Backup Recovery  : Primary table -> Backup table
    PartEntrySizeLBA = (uint64_t)(pGptHdr->NumberOfPartitionEntries*pGptHdr->SizeOfPartitionEntry) >> cfe_gpt_probe->block_size_bit;
    if( pGptHdr->MyLBA == GPT_HDR_PRIMARY_LBA )
    {
        //partEntryLBA = pGptHdr->LastUsableLBA + 1;
        partEntryLBA = cfe_gpt_probe->lba_size - PartEntrySizeLBA - 1;  // - header_size
    }
    // Primany Recovery : Backup table  -> Primary table
    else
    {
        partEntryLBA = GPT_HDR_PRIMARY_LBA + 1;
    }
    
    //[Step 2] Copy good header to bad header & set some values
    memcpy( pGptHdrBad, pGptHdr, sizeof(gpt_GptHeader_t) );
    // Set proper data
    pGptHdrBad->MyLBA               = pGptHdr->AlternateLBA;
    pGptHdrBad->AlternateLBA        = pGptHdr->MyLBA;
    pGptHdrBad->PartitionEntryLBA   = partEntryLBA;
    
    //[Step 3] Save data to staging memory buffer
    gpt_set_HDR( ptr_gpt_bad, pGptHdrBad );
    
    //[Step 4] Get & Set new CRC
    pGptHdrBad->HeaderCRC32 = gpt_set_HeaderCRC( ptr_gpt_bad, pGptHdrBad );

    return GPT_OK;
    
}

int gpt_recover_PartEntry( uint8_t *ptr_bad_partentry, uint8_t *ptr_good_partentry )
{

    memcpy( ptr_bad_partentry, ptr_good_partentry, GPT_PART_NUM_ENTRY*GPT_PART_ENTRY_SIZE );
    
    return GPT_OK;
    
}
#endif

/*  *********************************************************************
    *  gpt_check_PartEntry( part_header, part_entry, p_entrystatus )
    *  
    *  This routine will check GPT partition entry and return entry status.
    *  Caution: This function may receive untrusted input.
    *   The GPT partition entry is external input, so this routine
    *   will do basic validation for GPT partition entry and report status.
    *  
    *  Input parameters: 
    *      pGptHdr          - Partition table header structure
    *      ptr_partentry    - The partition entry array
    *  Output parameters: 
    *      pEntryStatus  - the partition entry status array 
    *                         recording the status of each partition
    *      
    *  Return value:
    *      0 if ok    - CRC Valid
    *      else error - CRC Invalid
    ********************************************************************* */

int gpt_check_PartEntry( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_pentry_mem, gpt_GptHeader_t *pGptHdr, cfe_gpt_PartEntryStatus_t *pPEntryStatus )
{
    uint32_t    i, j;
    uint8_t     *ptr_partentry;
    uint64_t    StartingLBA, EndingLBA;
    uint8_t     usedPartEntry=0;
    uint8_t     sOutOfRange=0, sOsSpecific=0, sOverlap=0;
    gpt_PartitionEntry_t    PartEntry;

#if DEBUG_GPT_VALID
    for(j=0; j<cfe_gpt_probe->num_parts ; j++ )
    {
        printf("original %s: %s 0x%08x\n", __FUNCTION__,  cfe_gpt_probe->cfe_parts[j].fp_name, cfe_gpt_probe->cfe_parts[j].fp_size);
    }
#endif    
        
    /* Update gpt probe entry which refers to primary GPT header*/
    cfe_gpt_probe->num_parts = 0;   
    cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_size = CFE_GPT_PRIMRY_SIZE;
    cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_offset_bytes = 0;
    strcpy(cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_name,PRIMARY_GPT_HDR_PART_NAME);
    cfe_gpt_probe->num_parts++; 
    
    for( i=0; i < pGptHdr->NumberOfPartitionEntries; i++ ) 
    {
        ptr_partentry = (uint8_t *)( ptr_pentry_mem + i * pGptHdr->SizeOfPartitionEntry );
        if( !gpt_get_PartitionEntry(ptr_partentry, &PartEntry) )
        {
            continue;
        }
        else
        {
            usedPartEntry++;
        }
        
        StartingLBA = PartEntry.StartingLBA;
        EndingLBA   = PartEntry.EndingLBA;
                    
        if( StartingLBA > EndingLBA 
            || ( StartingLBA < pGptHdr->FirstUsableLBA || StartingLBA > pGptHdr->LastUsableLBA )
            || ( EndingLBA < pGptHdr->FirstUsableLBA || EndingLBA > pGptHdr->LastUsableLBA ) )
        {
            pPEntryStatus[i].OutOfRange = TRUE;
            sOutOfRange++;
            continue;
        }
        
        /* Update gpt probe */
        cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_size = (PartEntry.EndingLBA - PartEntry.StartingLBA + 1) * cfe_gpt_probe->block_size;
        cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_offset_bytes = PartEntry.StartingLBA * cfe_gpt_probe->block_size;
        for( j=0; j<CFE_GPT_MAX_PART_NAME-1; j++ )
        {
            cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_name[j]= PartEntry.PartitionName[2*j];       
        }
        cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_name[j]= '\0';
        cfe_gpt_probe->num_parts++;
                
        if( (PartEntry.Attributes & GPT_ENTRY_ATTR_BIT1) != 0 ) 
        {
            // If Bit 1 is set, this indicate that this is an OS specific GUID partition. 
            pPEntryStatus[i].OsSpecific = TRUE;
            sOsSpecific++;
        }
        
        for( j = i + 1; j < pGptHdr->NumberOfPartitionEntries; j++ ) 
        {
            ptr_partentry = (uint8_t *)( ptr_pentry_mem + j * pGptHdr->SizeOfPartitionEntry ); 
            if( !gpt_get_PartitionEntry(ptr_partentry, &PartEntry) )
            {
                continue;
            }
                        
            if( PartEntry.EndingLBA >= StartingLBA && PartEntry.StartingLBA <= EndingLBA )
            {
                // This region overlaps with the Index1'th region
                pPEntryStatus[i].Overlap  = TRUE;
                pPEntryStatus[j].Overlap  = TRUE;
                sOverlap++;
                continue;
            }
        }
    }
    
    /* Update gpt probe entry which refers to backup GPT header*/
    cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_size = CFE_GPT_BACKUP_SIZE;
    cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_offset_bytes =  pGptHdr->LastUsableLBA * cfe_gpt_probe->block_size;
    strcpy(cfe_gpt_probe->cfe_parts[cfe_gpt_probe->num_parts].fp_name, BACKUP_GPT_HDR_PART_NAME);
    cfe_gpt_probe->num_parts++;
    
#if DEBUG_GPT_VALID    
    for(j=0; j<cfe_gpt_probe->num_parts ; j++ )
    {
        printf("readback %s: %s 0x%08x\n", __FUNCTION__,  cfe_gpt_probe->cfe_parts[j].fp_name, cfe_gpt_probe->cfe_parts[j].fp_size);
    }
#endif    
    
#if DEBUG_GPT_VALID
    xprintf(" [Checked Partition Entries] : %d ( Used : %d )\n", pGptHdr->NumberOfPartitionEntries, usedPartEntry);
    xprintf("   OutOfRange : %d, OsSpecific : %d, Overlap : %d\n", sOutOfRange, sOsSpecific, sOverlap);
#endif
    return GPT_OK;

}


/*  *********************************************************************
    *  gpt_check_HeaderCRC( MaxSize, ptr_hdr_mem, pGptHdr )
    *  
    *  Checks the CRC32 value in the table header.
    *  
    *  Input parameters: 
    *      Maxsize      - Max Size limit
    *      ptr_hdr_mem  - Header pointer in memory
    *      pGptHdr      - Table to check
    *      
    *  Return value:
    *      if   ok    - CRC Valid
    *      else error - CRC Invalid
    ********************************************************************* */

int gpt_check_HeaderCRC( uint32_t MaxSize, uint8_t *ptr_hdr_mem, gpt_GptHeader_t *pGptHdr )
{
    uint32_t Crc, OrgCrc, Size;
    
    Size = pGptHdr->HeaderSize;
    
    // Check valid size
    if( Size == 0 )
    {
        // If header size is 0 CRC will pass so return CFE_ERR.
        return GPT_ERR;
    }
    
    if( (MaxSize != 0) && (Size > MaxSize) )
    {
        xprintf("CheckCrc32: Size > MaxSize\n");
        return GPT_ERR;
    }
    
    // Save original CRC and clear old crc from header
    OrgCrc = pGptHdr->HeaderCRC32;
    Crc = 0;
    //xprintf(" [gpt_check_HeaderCRC] Crc = 0x%08X \n", Crc);
    gpt_set_TableValue( ptr_hdr_mem, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)Crc, CFE_GPT_BIG_ENDIAN_OFF );
    
    // Calculate new CRC
    Crc = chksum_crc32( ptr_hdr_mem, Size );
    
    // Save new CRC
    pGptHdr->HeaderCRC32 = Crc;
    //xprintf(" [gpt_check_HeaderCRC] Crc = 0x%08X \n", Crc);
    gpt_set_TableValue( ptr_hdr_mem, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)Crc, CFE_GPT_BIG_ENDIAN_OFF );
    
    if( OrgCrc == Crc )
    {
#if DEBUG_GPT_VALID
        xprintf("   -> HEADERCRC32   - orgCRC:CRC  0x%08X == 0x%08X : same CRC\n", OrgCrc, Crc);
#endif
        return GPT_OK;
    }
    else
    {
#if DEBUG_GPT_VALID
        xprintf("   -> HEADERCRC32   - orgCRC:CRC  0x%08X != 0x%08X \n", OrgCrc, Crc);
#endif
        return GPT_ERR;
    }
    
}


/*  *********************************************************************
    *  gpt_check_PartEntryArrayCRC( ptr_partentry_mem, pGptHdr )
    *  
    *  Check if the CRC field in the Partition table header is valid
    *  for Partition entry array.
    *  
    *  Input parameters: 
    *      ptr_partentry_mem    - Patitiion Entry Pointer in Memory
    *      pGptHdr              - Partition table header structure.
    *      
    *  Return value:
    *      if ok      - the CRC is valid
    *      else error - the CRC is invalid
    ********************************************************************* */

int gpt_check_PartEntryArrayCRC( uint8_t *ptr_partentry_mem, gpt_GptHeader_t *pGptHdr )
{
    uint32_t Crc, OrgCrc;
    uint32_t Size;
    
    OrgCrc  = pGptHdr->PartitionEntryArrayCRC32;
    Size    = pGptHdr->NumberOfPartitionEntries * pGptHdr->SizeOfPartitionEntry;
    /* To protect software from broken case */
    if( Size > CFE_GPT_TABLE_SIZE )
    {
        return GPT_ERR;
    }

    Crc = 0;
    Crc = chksum_crc32( ptr_partentry_mem, Size );
    //pGptHdr->PartitionEntryArrayCRC32 = Crc;
    //xprintf("   &ptr_partentry_mem : 0x%08X [0x%08X]\n", &ptr_partentry_mem, ptr_partentry_mem);
        
    if( OrgCrc == Crc )
    {
#if DEBUG_GPT_VALID
        xprintf("   -> PartitionEntryArrayCRC32 - orgCRC:CRC  0x%08X == 0x%08X : same CRC\n", OrgCrc, Crc);
#endif
        return GPT_OK;
    }
    else
    {
#if DEBUG_GPT_VALID
        xprintf("   -> PartitionEntryArrayCRC32 - orgCRC:CRC  0x%08X != 0x%08X \n", OrgCrc, Crc);
#endif
        return GPT_ERR;
    }
    
}

#if CFG_RAMAPP
/*  *********************************************************************
    *  gpt_set_HeaderCRC( ptr_hdr_mem, pGptHdr )
    *  
    *  Updates the CRC32 value in the table header.
    *  
    *  Input parameters: 
    *      ptr_hdr_mem  - Header pointer in memory
    *      pGptHdr      - GPT Header
    *
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

uint32_t gpt_set_HeaderCRC( uint8_t *ptr_hdr_mem, gpt_GptHeader_t *pGptHdr )
{
    uint32_t Crc;
    
    // Clear CRC to get new CRC
    Crc = 0;
    pGptHdr->HeaderCRC32 = Crc;
    gpt_set_TableValue( ptr_hdr_mem, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)Crc, CFE_GPT_BIG_ENDIAN_OFF );
    
    // Calculate new CRC
    Crc = chksum_crc32( ptr_hdr_mem, pGptHdr->HeaderSize );
    
    // Save new value
    pGptHdr->HeaderCRC32 = Crc;
    gpt_set_TableValue( ptr_hdr_mem, GPT_HDR_HEADERCRC32_OFFSET, GPT_HDR_HEADERCRC32_LENGTH, (uint64_t)Crc, CFE_GPT_BIG_ENDIAN_OFF );
    
    return Crc;
}


uint32_t gpt_set_PartEntryArrayCRC( uint8_t *ptr_partentry_mem, gpt_GptHeader_t *pGptHdr )
{
    // Calculate new CRC
    return ( chksum_crc32( ptr_partentry_mem, (uint32_t)(pGptHdr->NumberOfPartitionEntries*pGptHdr->SizeOfPartitionEntry) ) );
}
#endif

/* chksum_crc() -- to a given block, this one calculates the
 *              crc32-checksum until the length is
 *              reached. the crc32-checksum will be
 *              the result.
 */
uint32_t chksum_crc32( unsigned char *block, unsigned int length )
{
    register unsigned long crc;
    unsigned long i;
    
    crc = 0xFFFFFFFF;
    for( i=0; i<length; i++ )
    {
        crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
    }
    return( crc ^ 0xFFFFFFFF );
}


/* chksum_crc32gentab() -- to a global crc_tab[256], this one will
 *              calculate the crcTable for crc32-checksums.
 *              it is generated to the polynom [..]
 */
void chksum_crc32gentab( void )
{
    unsigned long crc, poly;
    int i, j;

    poly = 0xEDB88320L;
    
    for( i=0; i<256; i++ )
    {
        crc = i;
        for( j=8; j>0; j-- )
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ poly;
            }
            else
            {
                crc >>= 1;
            }
        }
        
        crc_tab[i] = crc;
    }
    
}

#if CFG_RAMAPP
/* guid_generate( uint32_t seed, gpt_GUID_t *newGUID ) -- to generate GUID for CFE paritions
 */
//static int guid_generate( gpt_GUID_t *newGUID ) 
int guid_generate( gpt_GUID_t *newGUID )
{
    int         i;
    //static uint32_t seed;
    uint32_t    seed;
    uint32_t    lowGUID=0, highGUID=0;
    
    seed = (uint32_t)_getticks( );
    //seed = (uint32_t)cfe_getticks( ); // Too slow
    
    for( i=0; i<5; i++ )
    {
        if( i==4 )
        {
            lowGUID  = rand( seed );
            seed     = lowGUID;
            highGUID = rand( seed );
        }
        else
        {
            lowGUID = rand( seed );
            seed    = lowGUID;
        }
        
        switch( i )
        {
            case 0: newGUID->l1 = (uint32_t)lowGUID;        break;
            case 1: newGUID->l2 = (uint16_t)lowGUID;        break;
            case 2: newGUID->l3 = (uint16_t)lowGUID;        break;
            case 3: newGUID->h1 = (uint16_t)lowGUID;        break;
            case 4: newGUID->h2 = (uint64_t)lowGUID + (((uint64_t)highGUID)<<32);       break;
        }
    }
#if GUID_DEBUG
    xprintf(" New GUID %08X-%04X-%04X-%04X-%012llu \n", newGUID->l1, newGUID->l2, newGUID->l3, newGUID->h1, newGUID->h2);
#endif
    
    return GPT_OK;  
}
#endif

/* rand( ) -- to get random number
 */
//static uint32_t rand( uint32_t seed )
uint32_t rand( uint32_t seed )
{
    //static uint32_t seed;
    long x, hi, lo, t;
    
    x = seed;
    hi = x / 127773;
    lo = x % 127773;
    t = 16807 * lo - 2836 * hi;
    if (t <= 0) t += 0x7fffffff;
    //seed = t;

    return t;
}


/*  *********************************************************************
    *  gpt_print_table( cfe_gpt_probe, ptr_gpt_prim, ptr_gpt_back, pGPT, usedPrimPartEntry, usedBackPartEntry )
    *  
    *  Decode GPT table.
    *
    *  Input parameters: 
    *      cfe_gpt_probe - 
    *      ptr_gpt_prim  - 
    *      ptr_gpt_back  - 
    *      pGPT          - 
    *      usedPrimPartEntry    - 
    *      usedBackPartEntry    - 
    *      
    *  Return value:
    *      0 if ok
    *      else error
    ********************************************************************* */

#if DEBUG_GPT_PRINT
int gpt_print_table( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_prim, uint8_t *ptr_gpt_back, cfe_gpt_t *pGPT, 
                    int usedPrimPartEntry, int usedBackPartEntry )
{
    int i, j, k, startBackupTable=0, maxByte;
    
    // PMBR, Primary GPT Header, Parition Entries
    if( ptr_gpt_prim )
    {
        xprintf(" [Hex Dump : ptr_gpt_prim - Used Partition Entry = %d[EA]]\n", usedPrimPartEntry);
        xprintf("            F E  D C  B A  9 8  7 6  5 4  3 2  1 0\n");
        maxByte = 2 * cfe_gpt_probe->block_size + GPT_PART_ENTRY_SIZE * usedPrimPartEntry;
        for( i=0; i<maxByte/16; i++ )
        {   
            xprintf(" %08X: ", i*16);
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {   
                k = 16*i + j-1;
                xprintf("%02X", ptr_gpt_prim[k]);
                if( j==15 || j==13 || j==11 || j==9 || j==7 || j==5 || j==3 )
                    xprintf(" ");
            }
            xprintf("  ");
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {
                k = 16*i + j-1;
                if( 0x20<=ptr_gpt_prim[k] && ptr_gpt_prim[k]<=0x7E )
                    xprintf("%c", ptr_gpt_prim[k]);
                else
                    xprintf(".");
            }
            xprintf("\n");
        }
    }
    
    if( ptr_gpt_back )
    {
        xprintf(" [Hex Dump : ptr_gpt_back - Used Partition Entry = %d[EA]]\n", usedBackPartEntry);
        xprintf("            F E  D C  B A  9 8  7 6  5 4  3 2  1 0\n");
        // Backup GPT Header
        maxByte = cfe_gpt_probe->block_size;
        for( i=0; i<maxByte/16; i++ )
        {   
            k = (CFE_GPT_TABLE_SIZE - maxByte) + i*16;
            xprintf(" %08X: ", k);
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {
                xprintf("%02X", ptr_gpt_back[k + j-1]);
                if( j==15 || j==13 || j==11 || j==9 || j==7 || j==5 || j==3 )
                    xprintf(" ");
            }
            xprintf("  ");
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {
                if( 0x20<=ptr_gpt_back[k + j-1] && ptr_gpt_back[k + j-1]<=0x7E )
                    xprintf("%c", ptr_gpt_back[k + j-1]);
                else
                    xprintf(".");
            }
            xprintf("\n");
        }
        // Patition Entries
        startBackupTable = cfe_gpt_probe->block_size + GPT_PART_ENTRY_SIZE * pGPT->backHDR.NumberOfPartitionEntries;
        maxByte = GPT_PART_ENTRY_SIZE * usedBackPartEntry;
        for( i=0; i<maxByte/16; i++ )
        {   
            k = (CFE_GPT_TABLE_SIZE - startBackupTable) + i*16;
            xprintf(" %08X: ", k);
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {
                xprintf("%02X", ptr_gpt_back[k + j-1]);
                if( j==15 || j==13 || j==11 || j==9 || j==7 || j==5 || j==3 )
                    xprintf(" ");
            }
            xprintf("  ");
            for( j=16; j>0; j-- )   // Little Endian : F E D C ... 3 2 1 0
            {
                if( 0x20<=ptr_gpt_back[k + j-1] && ptr_gpt_back[k + j-1]<=0x7E )
                    xprintf("%c", ptr_gpt_back[k + j-1]);
                else
                    xprintf(".");
            }
            xprintf("\n");
        }
    }

    // PMBR
    gpt_print_PMBR( pGPT );

    // Primary/Backup GPT Header
    gpt_print_GptHeader( pGPT );
    
    // Primary/Backup Partition Entries
    
    gpt_print_PartEntries( cfe_gpt_probe, ptr_gpt_prim, ptr_gpt_back, pGPT, usedPrimPartEntry, usedBackPartEntry );
    
    return GPT_OK;
    
}


void gpt_print_PMBR( cfe_gpt_t *pGPT )
{
    int i;
    
    // PMBR 
    xprintf("[PMBR] Signature = 0x%04X\n", pGPT->PMBR.Signature);
    for( i=0; i<GPT_MAX_MBR_PARTITIONS; i++ )
    {
        xprintf("[PMBR:%d]\n", i);
        xprintf(" BootIndicator = 0x%02X\n", pGPT->PMBR.PartRcd[i].BootIndicator);
        xprintf(" StartingCHS   = 0x%06X\n", pGPT->PMBR.PartRcd[i].StartingCHS  );
        xprintf(" OSType        = 0x%02X\n", pGPT->PMBR.PartRcd[i].OSType        );
        xprintf(" EndingCHS     = 0x%06X\n", pGPT->PMBR.PartRcd[i].EndingCHS     );
        xprintf(" StartingLBA   = 0x%08X\n", pGPT->PMBR.PartRcd[i].StartingLBA  );
        xprintf(" SizeInLBA     = 0x%08X\n", pGPT->PMBR.PartRcd[i].SizeInLBA     );
    }
    
}


void gpt_print_GptHeader( cfe_gpt_t *pGPT )
{
    // Primary/Backup GPT Header
    xprintf("[primHDR] \n");
    xprintf(" Signature                = 0x%016llX\n",  pGPT->primHDR.Signature               );
    xprintf(" Revision                 = 0x%08X\n",     pGPT->primHDR.Revision                );
    xprintf(" HeaderSize               = 0x%08X\n",     pGPT->primHDR.HeaderSize              );
    xprintf(" HeaderCRC32              = 0x%08X\n",     pGPT->primHDR.HeaderCRC32             );
    xprintf(" Reserved                 = 0x%08X\n",     pGPT->primHDR.Reserved                );
    xprintf(" MyLBA                    = 0x%016llX\n",  pGPT->primHDR.MyLBA                   );
    xprintf(" AlternateLBA             = 0x%016llX\n",  pGPT->primHDR.AlternateLBA            );
    xprintf(" FirstUsableLBA           = 0x%016llX\n",  pGPT->primHDR.FirstUsableLBA          );
    xprintf(" LastUsableLBA            = 0x%016llX\n",  pGPT->primHDR.LastUsableLBA           );
    xprintf(" DiskGUID                 = ");
    xprintf("%08X-",        pGPT->primHDR.DiskGUID.l1  );
    xprintf("%04X-",        pGPT->primHDR.DiskGUID.l2  );
    xprintf("%04X-",        pGPT->primHDR.DiskGUID.l3  );
    xprintf("%04X-",        pGPT->primHDR.DiskGUID.h1  );
    xprintf("%012llX\n",    pGPT->primHDR.DiskGUID.h2  );
    xprintf(" PartitionEntryLBA        = 0x%016llX\n",  pGPT->primHDR.PartitionEntryLBA       );
    xprintf(" NumberOfPartitionEntries = 0x%08X\n",     pGPT->primHDR.NumberOfPartitionEntries);
    xprintf(" SizeOfPartitionEntry     = 0x%08X\n",     pGPT->primHDR.SizeOfPartitionEntry    );
    xprintf(" PartitionEntryArrayCRC32 = 0x%08X\n",     pGPT->primHDR.PartitionEntryArrayCRC32);
    xprintf("[backHDR] \n");
    xprintf(" Signature                = 0x%016llX\n",  pGPT->backHDR.Signature               );
    xprintf(" Revision                 = 0x%08X\n",     pGPT->backHDR.Revision                );
    xprintf(" HeaderSize               = 0x%08X\n",     pGPT->backHDR.HeaderSize              );
    xprintf(" HeaderCRC32              = 0x%08X\n",     pGPT->backHDR.HeaderCRC32             );
    xprintf(" Reserved                 = 0x%08X\n",     pGPT->backHDR.Reserved                );
    xprintf(" MyLBA                    = 0x%016llX\n",  pGPT->backHDR.MyLBA                   );
    xprintf(" AlternateLBA             = 0x%016llX\n",  pGPT->backHDR.AlternateLBA            );
    xprintf(" FirstUsableLBA           = 0x%016llX\n",  pGPT->backHDR.FirstUsableLBA          );
    xprintf(" LastUsableLBA            = 0x%016llX\n",  pGPT->backHDR.LastUsableLBA           );
    xprintf(" DiskGUID                 = ");
    xprintf("%08X-",        pGPT->backHDR.DiskGUID.l1  );
    xprintf("%04X-",        pGPT->backHDR.DiskGUID.l2  );
    xprintf("%04X-",        pGPT->backHDR.DiskGUID.l3  );
    xprintf("%04X-",        pGPT->backHDR.DiskGUID.h1  );
    xprintf("%012llX\n",    pGPT->backHDR.DiskGUID.h2  );
    xprintf(" PartitionEntryLBA        = 0x%016llX\n",  pGPT->backHDR.PartitionEntryLBA       );
    xprintf(" NumberOfPartitionEntries = 0x%08X\n",     pGPT->backHDR.NumberOfPartitionEntries);
    xprintf(" SizeOfPartitionEntry     = 0x%08X\n",     pGPT->backHDR.SizeOfPartitionEntry    );
    xprintf(" PartitionEntryArrayCRC32 = 0x%08X\n",     pGPT->backHDR.PartitionEntryArrayCRC32);
    xprintf("\n");
    
}


void gpt_print_PartEntries( cfe_gpt_probe_t *cfe_gpt_probe, uint8_t *ptr_gpt_prim, uint8_t *ptr_gpt_back, 
                            cfe_gpt_t *pGPT, int usedPrimPartEntry, int usedBackPartEntry )
{
    int         i, j, res;
    uint8_t     *ptr_part_entry;
        
    // Primary Partition Entries
    if( ptr_gpt_prim )
    {
        xprintf("[Primary Partition Entries] : %d ( Used : %d )\n", pGPT->primHDR.NumberOfPartitionEntries, usedPrimPartEntry);
        for( i=0; i<usedPrimPartEntry; i++)
        {   
            ptr_part_entry = (uint8_t *)( ptr_gpt_prim + ( 2*cfe_gpt_probe->block_size + i*GPT_PART_ENTRY_SIZE) ); 
            res = gpt_get_PartitionEntry( ptr_part_entry, &pGPT->primPartEntry );
            pGPT->primPartEntry.PartSizeLBA = ( pGPT->primPartEntry.EndingLBA - pGPT->primPartEntry.StartingLBA + 1 );
            
            xprintf(" Partition Number    = %d\n", i+1);
            xprintf(" PartitionTypeGUID   = ");
            xprintf("%08X-",        pGPT->primPartEntry.PartitionTypeGUID.l1  );
            xprintf("%04X-",        pGPT->primPartEntry.PartitionTypeGUID.l2  );
            xprintf("%04X-",        pGPT->primPartEntry.PartitionTypeGUID.l3  );
            xprintf("%04X-",        pGPT->primPartEntry.PartitionTypeGUID.h1  );
            xprintf("%012llX\n",    pGPT->primPartEntry.PartitionTypeGUID.h2  );
            xprintf(" UniquePartitionGUID = ");
            xprintf("%08X-",        pGPT->primPartEntry.UniquePartitionGUID.l1  );
            xprintf("%04X-",        pGPT->primPartEntry.UniquePartitionGUID.l2  );
            xprintf("%04X-",        pGPT->primPartEntry.UniquePartitionGUID.l3  );
            xprintf("%04X-",        pGPT->primPartEntry.UniquePartitionGUID.h1  );
            xprintf("%012llX\n",    pGPT->primPartEntry.UniquePartitionGUID.h2  );
            xprintf(" StartingLBA         = 0x%016llX\n",   pGPT->primPartEntry.StartingLBA           );
            xprintf(" EndingLBA           = 0x%016llX\n",   pGPT->primPartEntry.EndingLBA             );
            xprintf(" PartSize[LBA]       = 0x%016llX\n",   pGPT->primPartEntry.PartSizeLBA           );
            xprintf(" Attributes          = 0x%016llX\n",   pGPT->primPartEntry.Attributes            );
            //xprintf(" PartitionName          = %s\n",         pGPT->primPartEntry.PartitionName         );
            xprintf(" PartitionName       = ");
            j = 0;
            do
            {   
                xprintf("%c", pGPT->primPartEntry.PartitionName[j]);
                j += 2;
            } while( pGPT->primPartEntry.PartitionName[j] != 0x00 );
            xprintf("\n");
        }
    }
    
    // Backup Partition Entris
    if ( ptr_gpt_back )
    {
        xprintf("[Backup Partition Entries] : %d ( Used : %d )\n", pGPT->backHDR.NumberOfPartitionEntries, usedBackPartEntry);
        for( i=0; i<usedBackPartEntry; i++)
        {   
            ptr_part_entry = (uint8_t *)( ptr_gpt_back
                            + ( CFE_GPT_TABLE_SIZE - cfe_gpt_probe->block_size 
                            - (pGPT->backHDR.NumberOfPartitionEntries - i)*GPT_PART_ENTRY_SIZE) ); 
            res = gpt_get_PartitionEntry( ptr_part_entry, &pGPT->backPartEntry );
            pGPT->backPartEntry.PartSizeLBA = ( pGPT->backPartEntry.EndingLBA - pGPT->backPartEntry.StartingLBA + 1 );
            
            xprintf(" Partition Number    = %d\n", i+1);
            xprintf(" PartitionTypeGUID   = ");
            xprintf("%08X-",        pGPT->backPartEntry.PartitionTypeGUID.l1  );
            xprintf("%04X-",        pGPT->backPartEntry.PartitionTypeGUID.l2  );
            xprintf("%04X-",        pGPT->backPartEntry.PartitionTypeGUID.l3  );
            xprintf("%04X-",        pGPT->backPartEntry.PartitionTypeGUID.h1  );
            xprintf("%012llX\n",    pGPT->backPartEntry.PartitionTypeGUID.h2  );
            xprintf(" UniquePartitionGUID = ");
            xprintf("%08X-",        pGPT->backPartEntry.UniquePartitionGUID.l1  );
            xprintf("%04X-",        pGPT->backPartEntry.UniquePartitionGUID.l2  );
            xprintf("%04X-",        pGPT->backPartEntry.UniquePartitionGUID.l3  );
            xprintf("%04X-",        pGPT->backPartEntry.UniquePartitionGUID.h1  );
            xprintf("%012llX\n",    pGPT->backPartEntry.UniquePartitionGUID.h2  );
            xprintf(" StartingLBA         = 0x%016llX\n",   pGPT->backPartEntry.StartingLBA           );
            xprintf(" EndingLBA           = 0x%016llX\n",   pGPT->backPartEntry.EndingLBA             );
            xprintf(" PartSize[LBA]       = 0x%016llX\n",   pGPT->backPartEntry.PartSizeLBA           );
            xprintf(" Attributes          = 0x%016llX\n",   pGPT->backPartEntry.Attributes            );
            //xprintf(" PartitionName          = %s\n",         pGPT->backPartEntry.PartitionName        );
            xprintf(" PartitionName       = ");
            j = 0;
            do
            {   
                xprintf("%c", pGPT->backPartEntry.PartitionName[j]);
                j += 2;
            }while( pGPT->backPartEntry.PartitionName[j] != 0x00 );
            xprintf("\n");
        }
    }

    (void) res;

}
#endif



