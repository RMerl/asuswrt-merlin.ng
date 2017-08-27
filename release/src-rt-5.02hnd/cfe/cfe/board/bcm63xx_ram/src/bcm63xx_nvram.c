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
    return nvram_write_dev(nv);
#else
    printf("writeNvramData skipped with SKIP_FLASH=1 build!!!\n");
    return 0;
#endif
}

/* read the nvramData struct from nvram 
   return -1:  crc fail, 0 ok
*/
static int nvram_read(NVRAM_DATA* nv)
{
    UINT32 crc;
    _dbg_trace_;
    if (nvram_read_dev(nv)) {
        return -1;
    }
    crc = nv->ulCheckSum;
    nv->ulCheckSum = 0;  
    if (crc != getCrc32((unsigned char *)nv, sizeof(NVRAM_DATA),
			CRC32_INIT_VALUE)) {
        if (*(unsigned int *)nv != NVRAM_DATA_ID)
            memset(nv, 0xff, sizeof(NVRAM_DATA));
        return -1;
    }

    return 0;
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
