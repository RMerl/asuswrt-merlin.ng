/*  *********************************************************************
    *
    <:copyright-BRCM:2012:DUAL/GPL:standard
    
       Copyright (c) 2012 Broadcom 
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
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  bcm63xx_nvram.c NVRAM accessor functions
    *  
    * 
    *
    *********************************************************************
*/
    
#include "bcm_memory.h"
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "jffs2.h"
#include "boardparms.h"
#include "rom_parms.h"
#include "bcm63xx_ipc.h"
#include "lib_byteorder.h"
#include "lib_malloc.h"
#include "bcm63xx_nvram.h"
#if INC_EMMC_FLASH_DRIVER
#include "dev_emmcflash.h"
#endif

#ifdef DEBUG_NVRAM
#define _dbg_trace_ do { printf("%s:%d\n",__func__,__LINE__); } while(0);
#else
#define _dbg_trace_
#endif

static NVRAM_DATA *nvram;
#ifndef NVRAM__DYNAMIC_ALLOC
static NVRAM_DATA s_nvram;
#endif

static int nvram_write_dev(NVRAM_DATA* nv);
static int nvram_read_dev(NVRAM_DATA* nv);
static NVRAM_DATA* nvram_alloc(void);
static void nvram_free(void);
static int nvram_set(const void* m_addr,
		     void* val,
		     unsigned int size,
                     unsigned int max_size);

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
#include "bcm_ubi.h"
static BOOT_BLOCK_MIRROR_INFO *nv_mirrors;
#ifndef NVRAM__DYNAMIC_ALLOC
static BOOT_BLOCK_MIRROR_INFO s_nv_mirros;
#endif


static BOOT_BLOCK_MIRROR_INFO* nv_mirror_alloc(void)
{
      _dbg_trace_;
#ifdef NVRAM_DYNAMIC_ALLOC
      return KMALLOC(sizeof(BOOT_BLOCK_MIRROR_INFO), sizeof(void*));
#else
      return &s_nv_mirros; 
#endif

}

static void populate_nv_mirror_info(void)
{

    int blk_size=0, blk=0, offset=0, nv_mirrors_idx=0;
    unsigned int addr=0, temp_crc;
    int reached_cferam_bootfs=0;
    unsigned char buff[sizeof(NVRAM_DATA)+strlen(NVRAM_DATA_SIGN)];
    PNVRAM_DATA temp_nv_data;

    if (!nv_mirrors) {
         nv_mirrors = nv_mirror_alloc();
         if (!nv_mirrors){
             return;
         }

        _dbg_trace_;
        blk_size=flash_get_sector_size(0);

        for(addr=0;reached_cferam_bootfs == 0 && addr <= MAX_BOOT_BLOCK_MIRROR_LOOKUP_OFFSET && nv_mirrors_idx < MAX_MIRRORS;addr += 2048) {
            offset=addr%blk_size;
            blk=addr/blk_size;
            memset(buff, '\0', sizeof(NVRAM_DATA)+strlen(NVRAM_DATA_SIGN));
            if(flash_read_buf(blk, offset,
                buff,
                sizeof(NVRAM_DATA)+strlen(NVRAM_DATA_SIGN)) == sizeof(NVRAM_DATA)+strlen(NVRAM_DATA_SIGN)) {
                //check if we have reached the CFERAM BOOTFS
                if(offset == 0) {
                    if(check_jffs_ubi_magic(buff) == 1) {
                        printf("NVRAM_MIRROR SCAN: OFFSET blk %d %d addr %x\n", blk, offset, addr);
                        reached_cferam_bootfs = 1;
                        continue;
                    } 
                } 
                //check nvram data signature
                if(strncmp((const char *)buff, NVRAM_DATA_SIGN, strlen(NVRAM_DATA_SIGN)) == 0) {
                    //check nvram crc
                    printf("NVRAM_MIRROR SCAN: NVRAM back up found at add %x\n", addr);
                    temp_nv_data=(PNVRAM_DATA)(buff+strlen(NVRAM_DATA_SIGN));
                    temp_crc=temp_nv_data->ulCheckSum;
                    temp_nv_data->ulCheckSum = 0;  
                    if (temp_crc == getCrc32((unsigned char *)temp_nv_data, sizeof(NVRAM_DATA),
                                CRC32_INIT_VALUE)) {
                            //increament mirror index
                            nv_mirrors->offset[nv_mirrors_idx]=addr;
                            nv_mirrors->image_type[nv_mirrors_idx]=IMG_NVRAM;
                            nv_mirrors_idx++;
                        }
                    temp_nv_data->ulCheckSum=temp_crc;
                }
            }
        }
    }
}

static void nvram_write_mirrors(unsigned char *buf, PNVRAM_DATA nv)
{
    int i=0, blk, offset, blk_size;

    populate_nv_mirror_info();
    blk_size=flash_get_sector_size(0);

    if( (flash_get_flash_type() == FLASH_IFC_NAND || flash_get_flash_type() == FLASH_IFC_SPINAND ) && nv_mirrors != NULL) {
        for(i=0;i<MAX_MIRRORS;i++) {
            if(nv_mirrors->offset[i] != 0) {
                //read the full block in the buff
                //check if the signature is at the offset 
                // replace NVRAM 
                //write the nvram back
                blk=nv_mirrors->offset[i]/blk_size;
                offset=nv_mirrors->offset[i]%blk_size;

                if (flash_read_buf(blk, 0, buf, blk_size) != blk_size) {
                    printf("Error reading sector: %d\n", blk_size);
                    continue;
                }
                if(strncmp((const char*)(buf+offset), NVRAM_DATA_SIGN, strlen(NVRAM_DATA_SIGN)) == 0) {
                    flash_sector_erase_int(blk); 
                    memcpy((buf + offset+strlen(NVRAM_DATA_SIGN)), nv, sizeof(NVRAM_DATA));

                    if (flash_write_buf(blk, 0, buf, blk_size) != blk_size) {
                        printf("Error writing sector: %d\n", blk_size);
                        continue;
                    }

                }

            }
        } 
    }
}
#endif


static NVRAM_DATA* nvram_alloc()
{
      _dbg_trace_;
#ifdef NVRAM_DYNAMIC_ALLOC
      return KMALLOC(sizeof(NVRAM_DATA), sizeof(void*));
#else
      return &s_nvram; 
#endif
}

static void nvram_free()
{
     _dbg_trace_;
#ifndef _CFE_ROM_
      if (nvram) {
          KFREE(nvram);
      }
#endif
      nvram  = NULL;
}

static int nvram_write(NVRAM_DATA* nv)
{
    _dbg_trace_;
#if (SKIP_FLASH==0)
    nv->ulCheckSum = 0;
    nv->ulCheckSum = getCrc32((unsigned char *)nv, sizeof(NVRAM_DATA), CRC32_INIT_VALUE);
#if INC_EMMC_FLASH_DRIVER
    if( flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC )
        return emmc_nvram_set(nv);
    else        
#endif        

        return nvram_write_dev(nv);
#else
    printf("writeNvramData skipped with SKIP_FLASH=1 build!!!\n");
    return 0;
#endif
}

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
static int nvram_mirror_read(PNVRAM_DATA nv)
{
    int i, status=-1;
    int crc;


    //scan first 1MB and populate NV MIRROR structure
    populate_nv_mirror_info();

    for(i=0; nv_mirrors != NULL && i < MAX_MIRRORS; i++) {
        if( flash_read_buf(nv_mirrors->offset[i]/flash_get_sector_size(0), (nv_mirrors->offset[i]%flash_get_sector_size(0)) + strlen(NVRAM_DATA_SIGN),
                          (unsigned char *)nv, sizeof(NVRAM_DATA)) == sizeof(NVRAM_DATA)) {
            crc = nv->ulCheckSum;
            nv->ulCheckSum = 0;
            if (crc != getCrc32((unsigned char *)nv, sizeof(NVRAM_DATA),
                        CRC32_INIT_VALUE)) {
                if (*(unsigned int *)nv != NVRAM_DATA_ID)
                    continue;
            }
            printf("NVRAM_MIRROR SCAN: VALID NV FOUND AT %x\n", nv_mirrors->offset[i]);
            nv->ulCheckSum = crc;
            status=0;
            break;
        }
    }
return status; 
}
#endif


/* read the nvramData struct from nvram 
   return -1:  crc fail, 0 ok
*/
static int nvram_read(NVRAM_DATA* nv)
{
    UINT32 crc;
    INT ret = -1;
    _dbg_trace_;
#if INC_EMMC_FLASH_DRIVER
    if( flash_get_flash_type() == FLASH_IFC_UNSUP_EMMC )
        ret = emmc_nvram_get(nv);
    else 
#endif
        ret = nvram_read_dev(nv);
    if (!ret) {
        crc = nv->ulCheckSum;
        nv->ulCheckSum = 0;  
        if (crc != getCrc32((unsigned char *)nv, sizeof(NVRAM_DATA),
                  CRC32_INIT_VALUE)) {
            if (*(unsigned int *)nv != NVRAM_DATA_ID)
                memset(nv, 0xff, sizeof(NVRAM_DATA));
                  ret=-1;
        }
        else {
            /* Restore checksum */
            nv->ulCheckSum = crc;
            ret=0;
        }
    }

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
    if( (flash_get_flash_type() == FLASH_IFC_NAND || flash_get_flash_type() == FLASH_IFC_SPINAND) && ret == -1 ) {
        //check nvram mirrors 
        ret=nvram_mirror_read(nv);
    }
#endif

    return ret;
}

int cfe_nvram_read_verify(NVRAM_DATA* nv)
{
    _dbg_trace_; 
    return (nvram_read(nv) ||
             BpSetBoardId(nv->szBoardId) != BP_SUCCESS ||
	    BpSetVoiceBoardId(nv->szVoiceBoardId) != BP_SUCCESS);
}

int cfe_nvram_init(NVRAM_DATA* nv)
{
     int res = 0;
     _dbg_trace_;
     if (!nvram) {
         nvram = nvram_alloc();
	 if (!nvram){
	     return -1;
	 }
     }
     
     if (nv) {
        memcpy(nvram, nv, sizeof(NVRAM_DATA));
     } else {
        res = nvram_read(nvram);
     }
     return res;
}

int cfe_nvram_sync()
{
     return nvram_read(nvram);
}

void cfe_nvram_deinit()
{
      _dbg_trace_;
      nvram_free();
}

const NVRAM_DATA* const cfe_nvram_get()
{
      _dbg_trace_;
      return nvram;
}

	/* Write Pointer */
static int nvram_set(const void* m_addr,
		     void* val,
		     unsigned int size, 
		     unsigned int max_size)
{
     
     long m_offs = (unsigned long) (void*)m_addr - (unsigned long)nvram;
     _dbg_trace_;
     if ( size > max_size || (unsigned long)nvram+m_offs+size > (unsigned long)nvram+sizeof(NVRAM_DATA)) {
	  return -1;
     }
       
     memcpy((void*)((unsigned long)nvram+m_offs), val, size);
     return 0;
}

int cfe_nvram_set(const void* maddr,
		  void* val,
		  unsigned int size, 
		  unsigned int max_size)
{
     return nvram_set(maddr,val,size,max_size);
}

void cfe_nvram_copy_to(NVRAM_DATA* dst)
{
     _dbg_trace_;
     memcpy(dst, nvram, sizeof(NVRAM_DATA));
}

int cfe_nvram_update(NVRAM_DATA* nv)
{
     int res = 0;
     _dbg_trace_;
     if (nv) {
	 memcpy(nvram, nv, sizeof(NVRAM_DATA));
     }
     if (nvram_write(nvram)) {
	 return -1;
     }
     return res;
}


int cfe_nvram_update_member(const void* m_addr,
			    void* val,
			    unsigned int size, 
			    unsigned int max_size)
{
     _dbg_trace_;
     if (nvram_set(m_addr, val, size, max_size)) {
	 return -1;
     }
     return nvram_write(nvram);
}


/*******************************************************************************
 * NVRAM NAND/NOR read/write
 *******************************************************************************/
/* read nvram data from flash memory
   return:
   0 - ok
  -1 - fail
*/
static int nvram_read_dev(NVRAM_DATA* nv)
{
    _dbg_trace_;
    return flash_read_buf(NVRAM_SECTOR, NVRAM_DATA_OFFSET,
			  (unsigned char *)nv,
			  sizeof(NVRAM_DATA)) == sizeof(NVRAM_DATA) ? 0 : -1;
}


/* set nvram 
   return:
   0 - ok
   -1 - fail
*/
static int nvram_write_dev(NVRAM_DATA* nv)
{
    int res = -1;
    unsigned int size;
    unsigned char *buf;
    _dbg_trace_;
    size = flash_get_sector_size(NVRAM_SECTOR);
    
    buf = (unsigned char *) KMALLOC(size, sizeof(long));    
    if (!buf) {
	goto err_out;
    }
    if (flash_read_buf(NVRAM_SECTOR, 0, buf, size) != size) {
	printf("Error reading sector: %d\n", size);
	goto err_out;
    }
    
    flash_sector_erase_int(NVRAM_SECTOR);
    memcpy((buf + NVRAM_DATA_OFFSET), nv, sizeof(NVRAM_DATA));
    if (flash_write_buf(NVRAM_SECTOR, 0, buf, size) != size) {
	printf("Error writing sector: %d\n", size);
	goto err_out;
    }
    res = 0;

err_out:

#if (INC_NAND_FLASH_DRIVER == 1 || INC_SPI_NAND_DRIVER || INC_SPI_PROG_NAND == 1) && (INC_NVRAM_MIRRORING == 1)
    nvram_write_mirrors(buf, nv);
#endif
    if (buf) {
	KFREE(buf);
    }
    return res;
}

/***********************************************************************
 * Function Name: nvram_erase
 * Description  : Erase the NVRAM storage section of flash memory.
 * Returns      : 0 -- ok, 1 -- fail
 ***********************************************************************/
int cfe_nvram_erase(void)
{
    /* fill with 0xff (blanc state for flash) to the NVRAM flash area 
    */
    _dbg_trace_;
    memset(nvram, 0xff, sizeof(NVRAM_DATA));
    return nvram_write_dev((NVRAM_DATA*)nvram);
}
