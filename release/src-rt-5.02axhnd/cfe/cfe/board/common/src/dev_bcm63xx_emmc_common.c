/***************************************************************************
 <:copyright-BRCM:2016:DUAL/GPL:standard
 
    Copyright (c) 2016 Broadcom 
    All Rights Reserved
 
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
#include "lib_crc.h"
#include "bcm_otp.h"
#include "bcmTag.h"
#include "bcm_hwdefs.h"
#include "lib_byteorder.h"
#include "dev_bcm63xx_emmc_common.h"
#include "rom_parms.h"
#include "btrm_if.h"
#include "bcm63xx_util.h"

/* This struct describes the fixed header of a bootfs entry */
typedef struct
{
    unsigned int nextEntryOffset;   // Relative to this field
    unsigned int dataOffset;        // Relative to this field
    unsigned int dataLength;
    unsigned int hdrCrc;
} BOOTFS_ENTRY_FIXED_HDR;

/* This struct stores all information regarding a bootfs entry */
typedef struct
{          
    BOOTFS_ENTRY_FIXED_HDR fixedHdr;
    unsigned int relPartOffset;      // Offset to this entry from start of partition
    unsigned int relNextEntryOffset; // Relative to start of bootfs Entry
    unsigned int relDataOffset;      // Relative to start of bootfs Entry
    unsigned int relFileNameOffset;  // Relative to start of bootfs Entry 
    unsigned int fileNameLength;
    unsigned int relPadHdrOffset;    // Relative to start of bootfs Entry
    unsigned int padHdrLength;
    unsigned int dataCrc;
    unsigned int relFtrOffset;       // Relative to start of bootfs Entry
    unsigned int ftrLength;
} BOOTFS_ENTRY;

#define PRIMARY_IMG     0
#define SECONDARY_IMG   1
#define EMMC_COMMON_USE_MULTI_BLOCK_TRANSFER    1

#define DEBUG_BOOTFS_PARSER     0
#define EMMC_COMMON_DEBUG_CRC   0

/* Externs */
extern BOOT_INFO bootInfo;

/************************************************************
 *                      Functions                           *
 ************************************************************/
int get_seqnum_from_emmc_mdata(  int image_num, int * sequence_num, int * committed_flag, 
                                 int (*bootfs_parsefunc)( char * , char * , char * , int * ) )
{
    int ret = 0;
    char fname[] = NAND_CFE_RAM_NAME;
    char seq_num[4] = {0};
    char cm_flag = 0;
    char part_name[EMMC_MAX_PART_NAME];
    int i;
    /* Search for sequence numbers and committed flags */
    for ( i=EMMC_IMG_IDX_START; i<=EMMC_NUM_IMGS; i++ )
    {
        sprintf(part_name, EMMC_CFE_PNAME_FMT_STR_MDATA, image_num, i);
        xprintf("...Searching %s for seq_num\n", part_name);
        if ( (ret = bootfs_parsefunc(part_name, fname, seq_num, NULL)) == 0 )
            if ( (ret = bootfs_parsefunc(part_name, "committed", &cm_flag, NULL)) == 0 )
                break;
    }

    if( ret == 0 )
    {
        xprintf("Image %d has Sequence number %s, committed: %c\n", image_num, (strlen(seq_num)?seq_num:"0"), cm_flag);
        *sequence_num = atoi(seq_num);
        *committed_flag = cm_flag - '0';
    }
    else
    {
        xprintf("Sequence number not found for image %d\n", image_num);
        *sequence_num = -1;
        *committed_flag = 0;
    }
    return ret;
}

int load_file_from_next_bootfs( char * fname, char * buf , int * boot_img_id, int * img_idx,
                                       int (*bootfs_parsefunc)( char * , char * , char * , int * ) )
{
    int i;
    int fail = -1;
    char * msg_img;
    char emmcImgPartName[EMMC_MAX_PART_NAME];
    char msg[] = "Loaded %s (%d bytes) from %s image in %s ...\n";
    int file_length = 0;

    /* Variables to hold image squence numbers */
    int seq1 = -1;
    int seq2 = -1;
    int sel_seqnum = -1;

    /* Variables to hold image committed flags */
    int cm_flag1 = 0;
    int cm_flag2 = 0;
    int sel_cm_flag = 0;

    /* Retrieve seq_nums to help us determine which image to load
     * only when img_idx is not specified by the caller
     */
    if( !img_idx || (img_idx && (*img_idx == 0)) )
    {
        get_seqnum_from_emmc_mdata(1, &seq1, &cm_flag1, bootfs_parsefunc);
        get_seqnum_from_emmc_mdata(2, &seq2, &cm_flag2, bootfs_parsefunc);

        /* Deal with wrap around case */
        if (seq1 == 0 && seq2 == 999)
            seq1 = 1000;
        if (seq2 == 0 && seq1 == 999)
            seq2 = 1000;
    }
    else
    {
        /* Otherwise just get the sequence number and commit state of specified image */
        get_seqnum_from_emmc_mdata(*img_idx, &sel_seqnum, &sel_cm_flag, bootfs_parsefunc);
    }

    /* Try loading our file from both images, using the 
     * bootPartition variable to pick the first image to search in 
     */  
    for( i=PRIMARY_IMG; i<=SECONDARY_IMG; i++ )
    {
        /* Determine img index based on seq_num and bootInfo */
        if( !img_idx || (img_idx && (*img_idx == 0)) )
        {
            /* Determine the sequence number and committed flags of 
             * selected image 
             */
             
            if(PRIMARY_IMG  == i){
                sel_seqnum   = (seq2 > seq1) ? seq2 : seq1;           
                sel_cm_flag  = (seq2 > seq1) ? cm_flag2 : cm_flag1;
                if(cm_flag1 ^ cm_flag2){//If just one is committed - take it
                    if(!sel_cm_flag){ 
                        sel_seqnum   = (sel_seqnum == seq2) ? seq1 : seq2;           
						sel_cm_flag = (sel_cm_flag == cm_flag2) ? cm_flag1 : cm_flag2;  
                    }						
                }
                if( bootInfo.bootPartition == BOOT_SET_OLD_IMAGE) /* Whatever was regulary selected by def. - use the other option */ 
                {
                    sel_seqnum =  (sel_seqnum == seq2) ? seq1 : seq2;
                    sel_cm_flag = (sel_cm_flag == cm_flag2) ? cm_flag1 : cm_flag2;					
                }			
            }else{//bootfs_parsefunc failed on the 1st image. Try the other	image	
                 sel_seqnum   = (sel_seqnum == seq2) ? seq1 : seq2;           
                 sel_cm_flag = (sel_cm_flag == cm_flag2) ? cm_flag1 : cm_flag2;  
			}
								
            
            /*ensure that we never even attempt to parse if the seq number is invalid */
            if(sel_seqnum < 0)
                continue;
            /* Determine the selected image index */
            *img_idx = (sel_seqnum == seq1) ? NP_ROOTFS_1 : NP_ROOTFS_2;			 
        }
        else
        {
            /* Increment i to breakout of for loop in the case when img_idx
             * has been specified by the calling function. So in this case
             * if our selected img_idx fails, we simply quit the function 
             */
            i=SECONDARY_IMG;
        }
		
        sprintf(emmcImgPartName, EMMC_CFE_PNAME_FMT_STR_BOOTFS, *img_idx);
        fail = bootfs_parsefunc( emmcImgPartName, fname, (char *)buf, &file_length );

        if ( !fail )
            break;

        /* reset loop variables */
        *img_idx = 0;
    }
       
    /* Based on which valid image was found, update print msg
     * and the boot_img_id
     * */
    if( !fail )
    {
        if( seq1 < 0 || seq2 < 0 )
        {
            if( seq1 < 0 && seq2 < 0 )
                msg_img = "specified";
            else
                msg_img = "only";
            if( boot_img_id )
                *boot_img_id = BOOTED_ONLY_IMAGE;

        }
        else if(  ((bootInfo.bootPartition == BOOT_SET_NEW_IMAGE) && (i == PRIMARY_IMG  )) 
               || ((bootInfo.bootPartition != BOOT_SET_NEW_IMAGE) && (i == SECONDARY_IMG)) )
        {
            msg_img = "latest";
            if( boot_img_id )
                *boot_img_id = BOOTED_NEW_IMAGE;
        }    
        else
        {  
            msg_img = "previous";
            if( boot_img_id )
                *boot_img_id = BOOTED_OLD_IMAGE;
        }

        xprintf(msg, fname, file_length, msg_img, emmcImgPartName);   
    }

    return (fail<0?fail:file_length);
}

int parse_emmc_bootfs_common( int fh, uint64_t offset, char * filename, char * dev_buffer, char * data_buffer, int * data_length,
                                     int block_size, int (*read_func)(int, unsigned long long, unsigned char *, int ) )
{
    int ret = 0;
    BOOTFS_ENTRY bootfs_entry;
    unsigned int lastRetreivedBlkDevBuffOffset=0;
    unsigned int bootFsEntryHdrDevBufOffset=0;
    unsigned int bootFsEntryFtrDevBufOffset=0;
    unsigned int currentBlk=0;
    unsigned int hdrCrc=0;
    unsigned int dataCrc=0;
    unsigned int numFullDataBlocks=0;  
    unsigned int remPartialDataBytes=0;
    unsigned int remDataSize=0;
    unsigned int i;
    unsigned int crcdBytes = 0;        
    unsigned int emmcBlkSize = block_size;
    char * pData = data_buffer;

                
    /* Start at first block */
    currentBlk=0;
    ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[0], emmcBlkSize);  
    lastRetreivedBlkDevBuffOffset = 0;
    do             
    {
        /* Calculate relative offsets (from start of partition) for current bootfs entry */  
        bootfs_entry.relPartOffset = currentBlk*emmcBlkSize + bootFsEntryHdrDevBufOffset;
#if DEBUG_BOOTFS_PARSER        
        printf("currentBlk                : %d\n", currentBlk                );
        printf("bootFsEntryHdrDevBufOffset: %d\n", bootFsEntryHdrDevBufOffset); 
        printf("bootfs_entry.relPartOffset: 0x%.08x\n", bootfs_entry.relPartOffset); 
#endif        
        
        /* Check if fixed header spans 2 blocks */
        if( bootFsEntryHdrDevBufOffset + sizeof(bootfs_entry.fixedHdr) > emmcBlkSize )
        {
            /* Retrieve next block to get full header - This will also gurantee that some data is also retrieved */
            currentBlk++;
#if DEBUG_BOOTFS_PARSER        
            printf("currentBlk-hdr spans blks : %d\n", currentBlk                );
#endif         
            ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[emmcBlkSize], emmcBlkSize);  
            lastRetreivedBlkDevBuffOffset = emmcBlkSize;
        }
                
        /* Retrieve fixed header information - copy to avoid misaligned access */
        memcpy((char *)(&bootfs_entry.fixedHdr), (char *)(&dev_buffer[bootFsEntryHdrDevBufOffset]), sizeof(bootfs_entry.fixedHdr) );
        
        /* Run CRC on raw header */
        hdrCrc = CRC32_INIT_VALUE;
        hdrCrc = getCrc32((unsigned char *) &bootfs_entry.fixedHdr, offsetof(BOOTFS_ENTRY_FIXED_HDR,hdrCrc), hdrCrc); 
        
        /* Fix endianess */
        bootfs_entry.fixedHdr.nextEntryOffset   = be32_to_cpu(bootfs_entry.fixedHdr.nextEntryOffset);
        bootfs_entry.fixedHdr.dataOffset        = be32_to_cpu(bootfs_entry.fixedHdr.dataOffset     );
        bootfs_entry.fixedHdr.dataLength        = be32_to_cpu(bootfs_entry.fixedHdr.dataLength     );
        bootfs_entry.fixedHdr.hdrCrc            = be32_to_cpu(bootfs_entry.fixedHdr.hdrCrc         );
        
        if( !bootfs_entry.fixedHdr.nextEntryOffset || !bootfs_entry.fixedHdr.dataOffset 
          ||!bootfs_entry.fixedHdr.dataLength      || !bootfs_entry.fixedHdr.hdrCrc )
        {
            if(filename)
            {
                printf("Cannot find file %s in partition\n", filename);
                ret = -1;
                goto END;
            }
            else
            {
                printf("End of Bootfs\n");
                ret = 0;
            }
            break;
        }
                        
        /* Check header CRC */
        if( hdrCrc != bootfs_entry.fixedHdr.hdrCrc )
        {
            printf("Bootfs Entry Hdr CRC chck Fail. Exp:0x%08x Calc:0x%08x!!\n", bootfs_entry.fixedHdr.hdrCrc, hdrCrc);
#if EMMC_COMMON_DEBUG_CRC         
            printf("bootFsEntryHdrDevBufOffset: %d\n", bootFsEntryHdrDevBufOffset); 
            printf("bootfs_entry.relPartOffset: 0x%.08x\n", bootfs_entry.relPartOffset); 
            printf("nextEntryOffset     : 0x%08x\n", bootfs_entry.fixedHdr.nextEntryOffset);
            printf("dataOffset          : 0x%08x\n", bootfs_entry.fixedHdr.dataOffset     );
            printf("dataLength          : 0x%08x\n", bootfs_entry.fixedHdr.dataLength     );
            printf("hdrCrc              : 0x%08x calc=0x%08x\n", bootfs_entry.fixedHdr.hdrCrc, hdrCrc);
#endif        
            ret = -1;
            goto END;
        }                    
        
        /* Calculate relative offsets (from start of entry) for nextentry and data */
        bootfs_entry.relNextEntryOffset = bootfs_entry.fixedHdr.nextEntryOffset + offsetof(BOOTFS_ENTRY_FIXED_HDR,dataOffset) ;        
        bootfs_entry.relDataOffset      = bootfs_entry.fixedHdr.dataOffset      + offsetof(BOOTFS_ENTRY_FIXED_HDR,dataLength);        
        
        /* If full fixed header is in block, check where data starts */ 
        if( bootFsEntryHdrDevBufOffset + sizeof(bootfs_entry.fixedHdr) <= emmcBlkSize )
        {
            /* Check if Start of data is in current block */ 
            if( bootFsEntryHdrDevBufOffset + bootfs_entry.relDataOffset >= emmcBlkSize  )
            {
                /* Retrieve next block to get first block containing data */
                currentBlk++;
#if DEBUG_BOOTFS_PARSER        
                printf("currentBlk-data in nxt blk: %d\n", currentBlk                );
#endif        
                ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[emmcBlkSize], emmcBlkSize);              
                lastRetreivedBlkDevBuffOffset = emmcBlkSize;
            }
        }
        
        /* Calculate remaining relative offsets and lengths */
        bootfs_entry.relFileNameOffset  = sizeof(bootfs_entry.fixedHdr);
        bootfs_entry.fileNameLength     = strlen((char*)&dev_buffer[bootfs_entry.relFileNameOffset + bootFsEntryHdrDevBufOffset]) + 1;        
        bootfs_entry.relPadHdrOffset    = bootfs_entry.relFileNameOffset + bootfs_entry.fileNameLength;                        
        bootfs_entry.padHdrLength       = bootfs_entry.relDataOffset - bootfs_entry.relPadHdrOffset;        
        bootfs_entry.relFtrOffset       = bootfs_entry.relDataOffset + bootfs_entry.fixedHdr.dataLength;
        bootfs_entry.ftrLength          = bootfs_entry.relNextEntryOffset - bootfs_entry.relFtrOffset;  
        
        /* Check if filename matches */
        if( filename && strcmp(filename, (const char*)&dev_buffer[bootfs_entry.relFileNameOffset + bootFsEntryHdrDevBufOffset]) )
        {
            
            /* determine which block new entry is in, and update if different */
            if( currentBlk != (bootfs_entry.relPartOffset + bootfs_entry.relNextEntryOffset) / emmcBlkSize ) 
            {
                currentBlk = (bootfs_entry.relPartOffset + bootfs_entry.relNextEntryOffset) / emmcBlkSize;
                ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[0], emmcBlkSize);                                  
                lastRetreivedBlkDevBuffOffset = 0;
#if DEBUG_BOOTFS_PARSER
                printf("Reading blocks for next entry\n");
#endif        
            }
#if DEBUG_BOOTFS_PARSER
            printf("Continuing to next entry\n");
#endif        
            
            /* Determing offset from start of block for new entry */
            bootFsEntryHdrDevBufOffset = (bootfs_entry.relPartOffset + bootfs_entry.relNextEntryOffset) % emmcBlkSize;           
            continue;
        }
        
        /* Dump bootfs entry info is no data buffer provided */
        if( !data_buffer )
        {
            /* Print filename */
            printf("<BOOTFS ENTRY INFO-Filename>\n");
            printf("FileName            : %s\n",      &dev_buffer[bootfs_entry.relFileNameOffset + bootFsEntryHdrDevBufOffset]); 
                       
            /* Dump Fixed Header */
            printf("<BOOTFS ENTRY INFO-Fixed Header>\n");
            printf("nextEntryOffset     : 0x%08x\n", bootfs_entry.fixedHdr.nextEntryOffset);
            printf("dataOffset          : 0x%08x\n", bootfs_entry.fixedHdr.dataOffset     );
            printf("dataLength          : 0x%08x\n", bootfs_entry.fixedHdr.dataLength     );
            printf("hdrCrc              : 0x%08x calc=0x%08x\n", bootfs_entry.fixedHdr.hdrCrc, hdrCrc);
        }
        else
        {
            if( data_length )
                *data_length = bootfs_entry.fixedHdr.dataLength;
        }
        
        /* Do CRC on filename and padding */
        dataCrc = CRC32_INIT_VALUE;
        remDataSize = bootfs_entry.fileNameLength + bootfs_entry.padHdrLength;
        dataCrc = getCrc32((unsigned char *) &dev_buffer[bootfs_entry.relFileNameOffset + bootFsEntryHdrDevBufOffset],
            remDataSize, dataCrc); 
        crcdBytes = remDataSize;    
                
        /* Do CRC on partial first data block - get size of data present in the first few retrieved blocks */
        remDataSize = emmcBlkSize - (bootfs_entry.relDataOffset + bootFsEntryHdrDevBufOffset)%emmcBlkSize;
        if( bootfs_entry.fixedHdr.dataLength < remDataSize )
        {
            /* adjust remDataSize to match datalength */
            remDataSize = bootfs_entry.fixedHdr.dataLength;
        }
        dataCrc = getCrc32((unsigned char *) &dev_buffer[bootfs_entry.relDataOffset + bootFsEntryHdrDevBufOffset],
            remDataSize, dataCrc); 
        crcdBytes += remDataSize;  
        
        /* Copy over data */
        if( data_buffer )
        {
            memcpy(pData, &dev_buffer[bootfs_entry.relDataOffset + bootFsEntryHdrDevBufOffset], remDataSize);
            pData += remDataSize;               
        }
            
        /* Check if we need to retrieve more blocks for data */
        if( bootfs_entry.fixedHdr.dataLength - remDataSize )
        {
            numFullDataBlocks   = (bootfs_entry.fixedHdr.dataLength - remDataSize) / emmcBlkSize;
            remPartialDataBytes = (bootfs_entry.fixedHdr.dataLength - remDataSize) % emmcBlkSize;
#if EMMC_COMMON_USE_MULTI_BLOCK_TRANSFER
            /* If data_buffer is not defined, we cannot use multi-block transfers */
            if( data_buffer )
            {
                /* Do a multi-block read of all full blocks */
                (void)i;
                remDataSize = remPartialDataBytes;
                currentBlk++;
                ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char *)pData, numFullDataBlocks*emmcBlkSize);                      
                dataCrc = getCrc32((unsigned char *)pData, numFullDataBlocks*emmcBlkSize, dataCrc);         
                crcdBytes +=  numFullDataBlocks*emmcBlkSize;       
                pData += numFullDataBlocks*emmcBlkSize;                                    
                currentBlk += numFullDataBlocks-1;
            }
            else
#endif    
            {
                /* Use Single block transfers to read all blocks */            
                remDataSize = emmcBlkSize;
                for( i=0; i< numFullDataBlocks; i++ )
                {            
                    currentBlk++;
                    ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[0], emmcBlkSize);                      
                    lastRetreivedBlkDevBuffOffset = 0;
                    dataCrc = getCrc32((unsigned char *) &dev_buffer[0], remDataSize, dataCrc);         
                    crcdBytes += remDataSize;       
                    /* Copy over data */
                    if( data_buffer )
                    {
                        memcpy(pData, &dev_buffer[0], remDataSize);
                        pData += remDataSize;                                    
                    }
                }
            }
                    
            /* Force retrieve last block - we either need it for partial data of for footer */    
            remDataSize = remPartialDataBytes;
            currentBlk++;
#if DEBUG_BOOTFS_PARSER        
            printf("currentBlk-last block retr: %d\n", currentBlk                );
#endif        
            ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[0], emmcBlkSize); 
            lastRetreivedBlkDevBuffOffset = 0;

            if( remDataSize )
            {
                /* Do CRC on partial last data block */
                dataCrc = getCrc32((unsigned char *) &dev_buffer[0], remDataSize, dataCrc);                 
                crcdBytes += remDataSize;    
                /* Copy over data */
                if( data_buffer )
                {
                    memcpy(pData, &dev_buffer[0], remDataSize);
                    pData += remDataSize;                                    
                }
            }        
            
            /* We copied a new block into dev_buffer to get the remPartialDataBytes,
             * Update bootFsEntryFtrDevBufOffset to point to the end of this new block.
             */
            bootFsEntryFtrDevBufOffset = remPartialDataBytes;
        }
        else
        {
            /* All data is contained within the already retrieved early blocks */
            numFullDataBlocks   = 0;           
            remPartialDataBytes = 0;
            bootFsEntryFtrDevBufOffset = bootFsEntryHdrDevBufOffset + bootfs_entry.relDataOffset + bootfs_entry.fixedHdr.dataLength; 
        }
#if DEBUG_BOOTFS_PARSER        
        printf("bootFsEntryFtrDevBufOffset: %d\n", bootFsEntryFtrDevBufOffset);
        printf("lastRetreivedBlkDevBuffOffset: %d\n", lastRetreivedBlkDevBuffOffset);
#endif        

        /* To get the complete footer, we need to deal with 2 cases:
         *
         * CASE 1:
         * The last bytes of file data are completely present in the 1st block of dev_buffer[].
         *
         * CASE 2:
         * The last bytes of file data are completely present in the 2nd block of dev_buffer[].
         * (this will happen if either the fixed bootfs header spans blocks, or if data starts in the 
         * block after the one containing the fixed header, In both these cases we end up retrieving
         * an extra blockis into the 2nd block of dev_buffer[]) 
         *
         * In either of these CASES the footer may be (a) fully contained in 1st/2nd block of 
         * dev_buffer[], (b) partially contained or (c) not contained in this block. If any part
         * of the footer is in the next block, we retrieve it into the 2nd block of dev_buffer[]
         */
        
        /* Check if last block retrieved was placed in 2nd block of dev_buffer[] */
        if( lastRetreivedBlkDevBuffOffset == emmcBlkSize )
        {
            /* Handle CASE 2 - Make it look like Case 1 */
            if( bootFsEntryFtrDevBufOffset + bootfs_entry.ftrLength > 2*emmcBlkSize )
            {
                /* Copy over last retrieved block from 2nd block of dev_buffer to 1st */
                memcpy(&dev_buffer[0], &dev_buffer[emmcBlkSize], emmcBlkSize);

                /* Update bootFsEntryFtrDevBufOffset accordingly */
                bootFsEntryFtrDevBufOffset -= emmcBlkSize;

                /* Update lastRetreivedBlkDevBuffOffset */
                lastRetreivedBlkDevBuffOffset = 0;
            }
        }
       
        /* Handle CASE 1 */
        if( lastRetreivedBlkDevBuffOffset == 0 )
        {
            if( bootFsEntryFtrDevBufOffset + bootfs_entry.ftrLength > emmcBlkSize )
            {
                /* Retrieve next block to get full footer - This will gurantee that hdr and some data of next entry is also retrieved */
                currentBlk++;
#if DEBUG_BOOTFS_PARSER        
                printf("currentBlk-ftr spans blks : %d\n", currentBlk                );
#endif    
                ret = read_func(fh, offset + currentBlk*emmcBlkSize, (unsigned char*)&dev_buffer[emmcBlkSize], emmcBlkSize);  
                lastRetreivedBlkDevBuffOffset = emmcBlkSize;
            }
        }

        /* Check data CRC */ 
        memcpy( (char*)(&bootfs_entry.dataCrc), (char*)(&dev_buffer[bootFsEntryFtrDevBufOffset]), sizeof(bootfs_entry.dataCrc) );
        bootfs_entry.dataCrc  = be32_to_cpu(bootfs_entry.dataCrc);
        if( dataCrc != bootfs_entry.dataCrc )
        {
            printf("Bootfs Entry Data CRC chck Fail. Exp:0x%08x Calc:0x%08x!!\n", bootfs_entry.dataCrc, dataCrc);
#if EMMC_COMMON_DEBUG_CRC         
            printf("bootFsEntryHdrDevBufOffset: %d\n", bootFsEntryHdrDevBufOffset); 
            printf("bootfs_entry.relPartOffset: 0x%.08x\n", bootfs_entry.relPartOffset); 
            printf("nextEntryOffset     : 0x%08x\n", bootfs_entry.fixedHdr.nextEntryOffset);
            printf("dataOffset          : 0x%08x\n", bootfs_entry.fixedHdr.dataOffset     );
            printf("dataLength          : 0x%08x\n", bootfs_entry.fixedHdr.dataLength     );
#endif            
            ret = -1;
            goto END;
        }                    
            
        /* Get ready for next Entry */        
        bootFsEntryHdrDevBufOffset = bootFsEntryFtrDevBufOffset + bootfs_entry.ftrLength;            

        /* Dump bootfs entry info is no data buffer provided */
        if( !data_buffer )
        {
            printf("<BOOTFS ENTRY INFO-Lengths>\n");
            printf("fileNameLength      : 0x%.08x\n", bootfs_entry.fileNameLength       );
            printf("padHdrLength        : 0x%.08x\n", bootfs_entry.padHdrLength         );
            printf("Data Length         : 0x%.08x\n", bootfs_entry.fixedHdr.dataLength  );
            printf("ftrLength           : 0x%.08x\n", bootfs_entry.ftrLength          );                
            printf("<FILE INFO-Offsets from start of entry>\n");
            printf("fileNameOffset      : 0x%.08x\n", bootfs_entry.relFileNameOffset    );
            printf("padHdrOffset        : 0x%.08x\n", bootfs_entry.relPadHdrOffset      );                                 
            printf("Data offset         : 0x%.08x\n", bootfs_entry.relDataOffset        );        
            printf("ftrOffset           : 0x%.08x\n", bootfs_entry.relFtrOffset         );        
            printf("Offset to next Entry: 0x%.08x\n", bootfs_entry.relNextEntryOffset   );             
            printf("Length for data CRC : 0x%.08x\n", bootfs_entry.fixedHdr.dataLength + bootfs_entry.padHdrLength + bootfs_entry.fileNameLength);
            printf("Data CRC            : 0x%.08x calc=0x%.08x\n", bootfs_entry.dataCrc, dataCrc); 
#if EMMC_COMMON_DEBUG_CRC         
            printf("CRC bytes sum bytes : 0x%.08x %ul 0x%.08x\n", crcdBytes, byteSum, numByteSum1);          
#endif        
            printf("<BOOTFS ENTRY-END>\n\n\n");        
        }
        
        if( filename )
        {
            break;
        }
            
    } while( bootfs_entry.fixedHdr.nextEntryOffset );
    
    /* No errors so far */
    ret = 0;
        
END: 
    
    if (ret < 0) 
    {
        printf("Valid bootfs entry not found!\n");
    }
    return ret;
}

int update_inmemory_metadata_data(  char * metadata_ptr, char * filename, uint8_t * data, uint32_t data_len)
{
    BOOTFS_ENTRY_FIXED_HDR bootfs_entry_hdr;
    uint8_t * bootfs_entry_data_ptr;
    uint8_t * bootfs_entry_data_crc_ptr;
    uint8_t filename_match = 0;
    uint32_t bootfs_hdr_crc;
    uint32_t updated_data_crc = 0;
    int i = 0;
    BOOTFS_ENTRY_FIXED_HDR * bootfs_entry_ptr = (BOOTFS_ENTRY_FIXED_HDR *)metadata_ptr;

    /* Search for matching bootfs entry */
    do
    {
        /* Copy over fixed header to avoid any misaligned fields */
        memcpy(&bootfs_entry_hdr, bootfs_entry_ptr, sizeof(BOOTFS_ENTRY_FIXED_HDR));

        /* check if valid entry */
        if( !be32_to_cpu(bootfs_entry_hdr.nextEntryOffset) )
        {
            printf("Bootfs entry update failed! Matching bootfs entry not found");
            return -1;
        }

        /* Check if hdr crc is correct */
        bootfs_hdr_crc = CRC32_INIT_VALUE;
        bootfs_hdr_crc = getCrc32( (void *)bootfs_entry_ptr, offsetof(BOOTFS_ENTRY_FIXED_HDR,hdrCrc), bootfs_hdr_crc );                                        
        if( be32_to_cpu(bootfs_entry_hdr.hdrCrc) != bootfs_hdr_crc )
        {
            printf("Bootfs entry update failed! Corrupt hdr_crc: 0x%08x, Expected: 0x%08x\n", bootfs_hdr_crc, be32_to_cpu(bootfs_entry_hdr.hdrCrc));
            return -1;
        }

        /* See if filename matches */
        if( strcmp( filename, (char *)bootfs_entry_ptr + sizeof(BOOTFS_ENTRY_FIXED_HDR)) == 0 )
        {
            filename_match = 1;
        }
        else
        {
            /* Move on to the next entry */
            bootfs_entry_ptr = (BOOTFS_ENTRY_FIXED_HDR *)((char *)&bootfs_entry_ptr->dataOffset + be32_to_cpu(bootfs_entry_hdr.nextEntryOffset));
        }

    } while ( !filename_match );

    /* Check if data_length matches, updates cannot change the data length */
    if( data_len != be32_to_cpu(bootfs_entry_hdr.dataLength) )
    {
        printf("Bootfs entry update failed! Incompatible data_length: %d, Required length: %d\n", data_len, be32_to_cpu(bootfs_entry_hdr.dataLength));
        return -1;
    }

    /* Update data field */
    bootfs_entry_data_ptr = (uint8_t *)&(bootfs_entry_ptr->dataLength) + be32_to_cpu(bootfs_entry_hdr.dataOffset);
    memcpy(bootfs_entry_data_ptr, data, be32_to_cpu(bootfs_entry_hdr.dataLength));

    /* Recompute data crc over filename, padding and data */
    bootfs_entry_data_crc_ptr = bootfs_entry_data_ptr + be32_to_cpu(bootfs_entry_hdr.dataLength);
    updated_data_crc = CRC32_INIT_VALUE;
    updated_data_crc = cpu_to_be32(getCrc32( (uint8_t*)bootfs_entry_ptr + sizeof(BOOTFS_ENTRY_FIXED_HDR),
                                             (uint32_t)((uint8_t *)bootfs_entry_data_crc_ptr - ( (uint8_t*)bootfs_entry_ptr + sizeof(BOOTFS_ENTRY_FIXED_HDR))), 
                                             updated_data_crc));

    /* write crc byte-by-byte to avoid alignment issues */
    for( i=0; i<sizeof(uint32_t); i++ )
    {
        *(bootfs_entry_data_crc_ptr + i) = *((uint8_t*)&(updated_data_crc) + i );
    }
    
    /* Return pointer to next bootfs entry */
    return 0; 
}

