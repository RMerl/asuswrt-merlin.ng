/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
*/

/** Includes. **/
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map_part.h"
#define printk  printf
#else       // linux
#include <linux/version.h>
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/hardirq.h>
#include <bcm_map_part.h>
#endif

#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"
#include "bcmSpiRes.h"

#ifndef INC_BTRM_BUILD
#define INC_BTRM_BUILD 0
#endif


/** Defines. **/

/* Flash types in the chipcommon capabilities register */
#define FLASH_NONE              0x000           /* No flash */
#define SFLASH_ST               0x100           /* ST serial flash */
#define SFLASH_AT               0x200           /* Atmel serial flash */
#define NFLASH                  0x300
#define PFLASH                  0x700           /* Parallel flash */
#define QSPIFLASH_ST            0x800
#define QSPIFLASH_AT            0x900

#define SI_NANDFLASH            0x30000000      /* 47189 NAND flash base */
#define SI_NORFLASH             0x1c000000      /* 47189 NOR flash base */

/* Only support 16MB direct access for 3-byte address modes in spi flash */
#define SI_NORFLASH_WINDOW      0x01000000

/* Start/busy bit in flashcontrol */
#define SFLASH_OPCODE           0x000000ff
#define SFLASH_ACTION           0x00000700
#define SFLASH_CS_ACTIVE        0x00001000      /* Chip Select Active, rev >= 20 */
#define SFLASH_START            0x80000000
#define SFLASH_BUSY             SFLASH_START

/* flashcontrol action codes */
#define SFLASH_ACT_OPONLY       0x0000          /* Issue opcode only */
#define SFLASH_ACT_OP1D         0x0100          /* opcode + 1 data byte */
#define SFLASH_ACT_OP3A         0x0200          /* opcode + 3 addr bytes */
#define SFLASH_ACT_OP3A1D       0x0300          /* opcode + 3 addr & 1 data bytes */
#define SFLASH_ACT_OP3A4D       0x0400          /* opcode + 3 addr & 4 data bytes */
#define SFLASH_ACT_OP3A4X4D     0x0500          /* opcode + 3 addr, 4 don't care & 4 data bytes */
#define SFLASH_ACT_OP3A1X4D     0x0700          /* opcode + 3 addr, 1 don't care & 4 data bytes */

/* flashcontrol action+opcodes for ST flashes */
#define SFLASH_ST_WREN          0x0006          /* Write Enable */
#define SFLASH_ST_WRDIS         0x0004          /* Write Disable */
#define SFLASH_ST_RDSR          0x0105          /* Read Status Register */
#define SFLASH_ST_WRSR          0x0101          /* Write Status Register */
#define SFLASH_ST_READ          0x0303          /* Read Data Bytes */
#define SFLASH_ST_PP            0x0302          /* Page Program */
#define SFLASH_ST_SE            0x02d8          /* Sector Erase */
#define SFLASH_ST_BE            0x00c7          /* Bulk Erase */
#define SFLASH_ST_DP            0x00b9          /* Deep Power-down */
#define SFLASH_ST_RES           0x03ab          /* Read Electronic Signature */
#define SFLASH_ST_CSA           0x1000          /* Keep chip select asserted */
#define SFLASH_ST_SSE           0x0220          /* Sub-sector Erase */

#define SFLASH_ST_READ4B        0x6313          /* Read Data Bytes in 4Byte address */
#define SFLASH_ST_PP4B          0x6312          /* Page Program in 4Byte address */
#define SFLASH_ST_SE4B          0x62dc          /* Sector Erase in 4Byte address */
#define SFLASH_ST_SSE4B         0x6221          /* Sub-sector Erase */

#define SFLASH_MXIC_RDID        0x0390          /* Read Manufacture ID */
#define SFLASH_MXIC_MFID        0xc2            /* MXIC Manufacture ID */

/* Status register bits for ST flashes */
#define SFLASH_ST_WIP           0x01            /* Write In Progress */
#define SFLASH_ST_WEL           0x02            /* Write Enable Latch */
#define SFLASH_ST_BP_MASK       0x1c            /* Block Protect */
#define SFLASH_ST_BP_SHIFT      2
#define SFLASH_ST_SRWD          0x80            /* Status Register Write Disable */

/* flashcontrol action+opcodes for Atmel flashes */
#define SFLASH_AT_READ                          0x07e8
#define SFLASH_AT_PAGE_READ                     0x07d2
#define SFLASH_AT_BUF1_READ
#define SFLASH_AT_BUF2_READ
#define SFLASH_AT_STATUS                        0x01d7
#define SFLASH_AT_BUF1_WRITE                    0x0384
#define SFLASH_AT_BUF2_WRITE                    0x0387
#define SFLASH_AT_BUF1_ERASE_PROGRAM            0x0283
#define SFLASH_AT_BUF2_ERASE_PROGRAM            0x0286
#define SFLASH_AT_BUF1_PROGRAM                  0x0288
#define SFLASH_AT_BUF2_PROGRAM                  0x0289
#define SFLASH_AT_PAGE_ERASE                    0x0281
#define SFLASH_AT_BLOCK_ERASE                   0x0250
#define SFLASH_AT_BUF1_WRITE_ERASE_PROGRAM      0x0382
#define SFLASH_AT_BUF2_WRITE_ERASE_PROGRAM      0x0385
#define SFLASH_AT_BUF1_LOAD                     0x0253
#define SFLASH_AT_BUF2_LOAD                     0x0255
#define SFLASH_AT_BUF1_COMPARE                  0x0260
#define SFLASH_AT_BUF2_COMPARE                  0x0261
#define SFLASH_AT_BUF1_REPROGRAM                0x0258
#define SFLASH_AT_BUF2_REPROGRAM                0x0259

/* Status register bits for Atmel flashes */
#define SFLASH_AT_READY                         0x80
#define SFLASH_AT_MISMATCH                      0x40
#define SFLASH_AT_ID_MASK                       0x38
#define SFLASH_AT_ID_SHIFT                      3



#define OSL_DELAY(X)                        \
    do { { int i; for( i = 0; i < (X) * 500; i++ ) ; } } while(0)

/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#ifndef SPINWAIT_POLL_PERIOD
#define SPINWAIT_POLL_PERIOD	10
#endif

#define SPINWAIT(exp, us) {                                     \
    unsigned int countdown = (us) + (SPINWAIT_POLL_PERIOD - 1); \
    while ((exp) && (countdown >= SPINWAIT_POLL_PERIOD)) {      \
        OSL_DELAY(SPINWAIT_POLL_PERIOD);                        \
        countdown -= SPINWAIT_POLL_PERIOD;                      \
    }                                                           \
}


#define MAX_RETRY           3

#ifndef NULL
#define NULL 0
#endif

#if (INC_BTRM_BUILD==1)
/* reduce the memory usage for BTRM which only need to access up to 512KB */
#define MAXSECTORS          128
#else
#define MAXSECTORS          8192    /* maximum number of sectors supported */
#endif

#define FLASH_PAGE_256      256
#define SECTOR_SIZE_4K      (4 * 1024)
#define SECTOR_SIZE_64K     (64 * 1024)

/* Standard Boolean declarations */
#define TRUE                1
#define FALSE               0

/* flash memory type, the second byte returned from the READ ID instruction*/
#define FLASH_MEMTYPE_NULL  0x00
#define FLASH_MEMTYPE_20    0x20
#define FLASH_MEMTYPE_BA    0xBA

/* Return codes from flash_status */
#define STATUS_READY        0       /* ready for action */
#define STATUS_BUSY         1       /* operation in progress */
#define STATUS_TIMEOUT      2       /* operation timed out */
#define STATUS_ERROR        3       /* unclassified but unhappy status */

/** Structs. **/
/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {
    char *name;         /* "AT25F512", etc. */
    unsigned long addr; /* physical address, once translated */
    int nsect;          /* # of sectors */
    struct {
        long size;      /* # of bytes in this sector */
        long base;      /* offset from beginning of device */
    } sec[MAXSECTORS];  /* per-sector info */
};

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT 0
#endif

/** Prototypes. **/
int spi_flash_init(flash_device_info_t **flash_info);
static int spi_flash_sector_erase_int(unsigned short sector);
static int ccsflash_poll(void);

#if (INC_BTRM_BOOT==0) && defined(_CFE_) && defined(CFG_RAMAPP)
static
#endif
int spi_flash_read_buf(unsigned short sector, int offset, unsigned char *buffer, int nbytes);

static int spi_flash_write_buf(unsigned short sector, int offset,
                               unsigned char *buffer, int nbytes);
static int spi_flash_get_numsectors(void);

#if (INC_BTRM_BOOT==0) && defined(_CFE_) && defined(CFG_RAMAPP)
static
#endif
int spi_flash_get_sector_size(unsigned short sector);

static unsigned char *spi_get_flash_memptr(unsigned short sector);
static unsigned char *spi_flash_get_memptr(unsigned short sector);
static unsigned char *spi_flash_get_memptr_phys(unsigned short sector);
static int spi_flash_get_blk(int addr);
static int spi_flash_get_total_size(void);

/** Variables. **/
static flash_device_info_t flash_spi_dev =
    {
        0xffff,
        FLASH_IFC_SPI,
        "",
        spi_flash_sector_erase_int,
        spi_flash_read_buf,
        spi_flash_write_buf,
        spi_flash_get_numsectors,
        spi_flash_get_sector_size,
        spi_flash_get_memptr_phys,
        spi_flash_get_blk,
        spi_flash_get_total_size
    };


#ifndef _CFE_
static DEFINE_SEMAPHORE(spi_flash_lock);
static bool bSpiFlashSlaveRes = FALSE;
#endif

static struct flashinfo meminfo; /* Flash information structure */
static int totalSize = 0;

/* Issue a serial flash command */
static inline void ccsflash_cmd(unsigned int opcode)
{
    SERIAL_FLASH->flashcontrol = SFLASH_START | opcode;
    while (SERIAL_FLASH->flashcontrol & SFLASH_BUSY);
}

/* Poll for command completion. Returns zero when complete. */
static int ccsflash_poll(void)
{
    switch(MISC->capabilities & CC_CAP_FLASH_MASK) {
    case SFLASH_ST:
        /* Check for ST Write in Progress bit */
        ccsflash_cmd(SFLASH_ST_RDSR);
        return (SERIAL_FLASH->flashdata & SFLASH_ST_WIP);
    case SFLASH_AT:
        /* Check for Atmel Ready bit */
        ccsflash_cmd(SFLASH_AT_STATUS);
        return !(SERIAL_FLASH->flashdata & SFLASH_AT_READY);
    }
    return 0;
}


/*********************************************************************/
/* Init_flash is used to build a sector table. This information is   */
/* translated from erase_block information to base:offset information*/
/* for each individual sector. This information is then stored       */
/* in the meminfo structure, and used throughout the driver to access*/
/* sector information.                                               */
/*                                                                   */
/* This is more efficient than deriving the sector base:offset       */
/* information every time the memory map switches (since on the      */
/* development platform can only map 64k at a time).  If the entire  */
/* flash memory array can be mapped in, then the addition static     */
/* allocation for the meminfo structure can be eliminated, but the   */
/* drivers will have to be re-written.                               */
/*                                                                   */
/* The meminfo struct occupies 44 bytes of heap space, depending     */
/* on the value of the define MAXSECTORS.  Adjust to suit            */
/* application                                                       */
/*********************************************************************/
int spi_flash_init(flash_device_info_t **flash_info)
{
    int sectorsize = 0;
    int numsector = 0;
    unsigned int id = 0;
    unsigned int id2 = 0;
    int basecount = 0;
    int i;
    int count = 0;

    *flash_info = &flash_spi_dev;

    switch(MISC->capabilities & CC_CAP_FLASH_MASK) {
    case SFLASH_ST:
        /* Probe for ST chips */
        meminfo.name = "ST compatible";
        ccsflash_cmd(SFLASH_ST_DP);
        SERIAL_FLASH->flashaddress = 0;
        ccsflash_cmd(SFLASH_ST_RES);
        id = SERIAL_FLASH->flashdata;
        sectorsize = 64 * 1024;

        switch(id) {
        case 0x11:
            /* ST M25P20 2 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P20");
            numsector = 4;
            break;
        case 0x12:
            /* ST M25P40 4 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P40");
            numsector = 8;
            break;
        case 0x13:
            ccsflash_cmd(SFLASH_MXIC_RDID);
            id = SERIAL_FLASH->flashdata;
            if (id == SFLASH_MXIC_MFID) {
                /* MXIC MX25L8006E 8 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "MX25L8006E");
                sectorsize = 4 * 1024;
                numsector = 16 * 6;
            } else {
                /* ST M25P80 8 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "STM25P80");
                numsector = 16;
            }
            break;
        case 0x14:
            /* ST M25P16 16 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P16");
            numsector = 32;
            break;
        case 0x15:
            /* ST M25P32 32 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P32");
            numsector = 64;
            break;
        case 0x16:
            /* ST M25P64 64 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P64");
            numsector = 128;
            break;
        case 0x17:
            /* ST M25FL128 128 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "STM25P128");
            numsector = 256;
            break;
        case 0x18:
            /* MXIC MX25L25635F 256 Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "MX25L25635F");
            numsector = 512;
            break;
        case 0xbf:
            /* All of the following flashes are SST with 4KB subsectors. Others
             * should be added but We'll have to revamp the way we identify them
             * since RES is not eough to disambiguate them.
             */
            meminfo.name = "SST";
            sectorsize = 4 * 1024;
            SERIAL_FLASH->flashaddress = 1;
            ccsflash_cmd(SFLASH_ST_RES);
            id2 = SERIAL_FLASH->flashdata;

            switch(id2) {
            case 1:
                /* SST25WF512 512 Kbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25WF512");
                numsector = 16;
                break;
            case 0x48:
		/* SST25VF512 512 Kbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF512");
                numsector = 16;
                break;
            case 2:
                /* SST25WF010 1 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25WF010");
                numsector = 32;
                break;
            case 0x49:
                /* SST25VF010 1 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF010");
                numsector = 32;
                break;
            case 3:
                /* SST25WF020 2 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25WF020");
                numsector = 64;
                break;
            case 0x43:
                /* SST25VF020 2 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF020");
                numsector = 64;
                break;
            case 4:
                /* SST25WF040 4 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25WF040");
                numsector = 128;
                break;
            case 0x44:
                /* SST25VF040 4 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF040");
                numsector = 128;
                break;
            case 0x8d:
                /* SST25VF040B 4 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF040B");
                numsector = 128;
                break;
            case 5:
                /* SST25WF080 8 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25WF080");
                numsector = 256;
                break;
            case 0x8e:
                /* SST25VF080B 8 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF080B");
                numsector = 256;
                break;
            case 0x41:
                /* SST25VF016 16 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF016");
                numsector = 512;
                break;
            case 0x4a:
                /* SST25VF032 32 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF032");
                numsector = 1024;
                break;
            case 0x4b:
                /* SST25VF064 64 Mbit Serial Flash */
                strcpy(flash_spi_dev.flash_device_name, "SST25VF064");
                numsector = 2048;
                break;
            }
            break;
        }
        break;

    case SFLASH_AT:
        /* Probe for Atmel chips */
        meminfo.name = "Atmel";
        ccsflash_cmd(SFLASH_AT_STATUS);
        id = SERIAL_FLASH->flashdata & 0x3c;

        switch (id) {
        case 0xc:
            /* Atmel AT45DB011 1Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB011");
            sectorsize = 256;
            numsector = 512;
            break;
        case 0x14:
            /* Atmel AT45DB021 2Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB021");
            sectorsize = 256;
            numsector = 1024;
            break;
        case 0x1c:
            /* Atmel AT45DB041 4Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB041");
            sectorsize = 256;
            numsector = 2048;
            break;
        case 0x24:
            /* Atmel AT45DB081 8Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB081");
            sectorsize = 256;
            numsector = 4096;
            break;
        case 0x2c:
            /* Atmel AT45DB161 16Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB161");
            sectorsize = 512;
            numsector = 4096;
            break;
        case 0x34:
            /* Atmel AT45DB321 32Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB321");
            sectorsize = 512;
            numsector = 8192;
            break;
        case 0x3c:
            /* Atmel AT45DB642 64Mbit Serial Flash */
            strcpy(flash_spi_dev.flash_device_name, "AT45DB642");
            sectorsize = 1024;
            numsector = 8192;
            break;
        }
        break;
    }

    if (numsector > MAXSECTORS)
        numsector = MAXSECTORS;

    if (sectorsize * numsector > SI_NORFLASH_WINDOW) {
        printk("NOR flash size %dMB is bigger than %dMB, limiting it to %dMB\n",
               sectorsize * numsector / 0x100000, SI_NORFLASH_WINDOW / 0x100000,
               SI_NORFLASH_WINDOW / 0x100000);
        numsector = SI_NORFLASH_WINDOW / sectorsize;
    }

    meminfo.addr = SI_NORFLASH;
    meminfo.nsect = numsector;
    for (i = 0; i < numsector; i++) {
        meminfo.sec[i].size = sectorsize;
        meminfo.sec[i].base = basecount;
        basecount += meminfo.sec[i].size;
        count++;
    }
    flash_spi_dev.flash_device_id = id;
    totalSize = meminfo.sec[count - 1].base + meminfo.sec[count - 1].size;

    return FLASH_API_OK;
}

/*********************************************************************/
/* Flash_sector_erase_int() wait until the erase is completed before */
/* returning control to the calling function.  This can be used in   */
/* cases which require the program to hold until a sector is erased, */
/* without adding the wait check external to this function.          */
/*********************************************************************/
static int spi_flash_sector_erase_int(unsigned short sector)
{
    unsigned int cmd = 0;
    unsigned int addr;

#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    /* Get the sector base address */
    addr = (uintptr_t) spi_get_flash_memptr(sector);

    switch (MISC->capabilities & CC_CAP_FLASH_MASK) {
    case SFLASH_ST:
        ccsflash_cmd(SFLASH_ST_WREN);
        SERIAL_FLASH->flashaddress = addr;
        /* Newer flashes have "sub-sectors" which can be erased independently
         * with a new command: ST_SSE. The ST_SE command erases 64KB just as
         * before.
         */
        if (addr < SI_NORFLASH_WINDOW) {
            if (meminfo.sec[sector].size < (64 * 1024))
                cmd = SFLASH_ST_SSE;
            else
                cmd = SFLASH_ST_SE;
        }
        else {
            unsigned int secmd = 0;
            unsigned int ssecmd = 0;
            switch (flash_spi_dev.flash_device_id) {
            case 0x18:
                /* MXIC MX25L25635F 256Mbit Serial Flash */
                secmd = SFLASH_ST_SE4B;
                ssecmd = SFLASH_ST_SSE4B;
                break;
            default:
                printk("ERROR: Need to support 4BYTE address command\n");
                return -1;
                break;
            }
            if (meminfo.sec[sector].size < (64 * 1024))
                cmd = ssecmd;
            else
                cmd = secmd;
        }

        ccsflash_cmd(cmd);
        while (ccsflash_poll());
        return meminfo.sec[sector].size;

    case SFLASH_AT:
        SERIAL_FLASH->flashaddress = addr << 1;
        ccsflash_cmd(SFLASH_AT_PAGE_ERASE);
        return meminfo.sec[sector].size;
    }

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

    return(FLASH_API_OK);
}


/*********************************************************************/
/* flash_read_buf() reads buffer of data from the specified          */
/* offset from the sector parameter.                                 */
/*********************************************************************/
#if (INC_BTRM_BOOT==0) && defined(_CFE_) && defined(CFG_RAMAPP)
static
#endif
int spi_flash_read_buf(unsigned short sector, int offset,
                       unsigned char *buffer, int nbytes)
{
    unsigned int addr;
    unsigned int addr_check;
    unsigned char *from;
    unsigned char *to;
    int cnt;
    unsigned int cmd = 0;

#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    addr = (uintptr_t)spi_flash_get_memptr(sector);
    addr += offset;
    addr_check = (uintptr_t)spi_get_flash_memptr(sector);
    addr_check += offset;

    if (!nbytes)
        return 0;

    if ((addr_check + nbytes) > totalSize)
        return FLASH_API_ERROR;

    cnt = nbytes;

    from = (unsigned char*)addr;
    to = (unsigned char*)buffer;

    if ((addr_check + nbytes) <= SI_NORFLASH_WINDOW) {
        /* Direct read (memory-mapped) */
        if (((uintptr_t)from & 0x3) || ((uintptr_t)to & 0x3)) {
            /*
             * Source flash address or dest buffer not word-aligned:
             * copy byte per byte
             */
            while (cnt > 0) {
                *to = *from;
                from++;
                to++;
                cnt--;
            }
        }
        else {
            /*
             * Source flash address and dest buffer are word-aligned:
             * copy word per word
             */
            while (cnt >= 4) {
                *(unsigned int*)to = *(unsigned int*)from;
                from += 4;
                to += 4;
                cnt -= 4;
            }
            while (cnt > 0) {
                *to = *from;
                from++;
                to++;
                cnt--;
            }
        }
    }
    else {
        /* Indirect read (through sflash controller) */
        switch(flash_spi_dev.flash_device_id) {
        case 0x18:
            /* MXIC MX25L25635F 256Mbit Serial Flash */
            cmd = SFLASH_ST_READ4B;
            break;
        default:
            printk("ERROR: Need to support 4BYTE address command\n");
            return FLASH_API_ERROR;
        }

        while (cnt) {
            SERIAL_FLASH->flashaddress = offset;
            ccsflash_cmd(cmd);
            *to = SERIAL_FLASH->flashdata & 0xff;
            offset++;
            to++;
            cnt--;
        }
    }

#ifndef _CFE_
    up(&spi_flash_lock);
#endif

    return (nbytes - cnt);
}


#define	GET_BYTE(ptr)	(*(ptr))

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/
static int spi_flash_write_buf(unsigned short sector, int offset,
                               unsigned char *buffer, int nbytes)
{
    unsigned int addr;
    unsigned int cmd;
    unsigned char data;
    int ret = 0;

#ifndef _CFE_
    down(&spi_flash_lock);
#endif

    addr = (uintptr_t) spi_get_flash_memptr(sector);
    addr += offset;

    if (!nbytes)
        return 0;

    if ((addr + nbytes) > totalSize)
        return FLASH_API_ERROR;

    switch (MISC->capabilities & CC_CAP_FLASH_MASK) {
    case SFLASH_ST:
        /* Enable writes */
        ccsflash_cmd(SFLASH_ST_WREN);
        if ((addr + nbytes) <= SI_NORFLASH_WINDOW) {
            cmd = SFLASH_ST_PP;
        } else {
            switch (flash_spi_dev.flash_device_id) {
            case 0x18:
                /* MXIC MX25L25635F 256Mbit Serial Flash */
                cmd = SFLASH_ST_PP4B;
                break;
            default:
                printk("ERROR: Need to support 4BYTE address command\n");
                return FLASH_API_ERROR;
                break;
            }
        }

        SERIAL_FLASH->flashaddress = addr;
        data = GET_BYTE(buffer);
        buffer++;
        SERIAL_FLASH->flashdata = data;
        /* Issue a page program with CSA bit set */
        ccsflash_cmd(SFLASH_ST_CSA | cmd);
        ret = 1;
        addr++;
        nbytes--;

        while (nbytes > 0) {
            if ((addr & 255) == 0) {
                /* Page boundary, poll dropping cs and return */
                SERIAL_FLASH->flashcontrol = 0;
                OSL_DELAY(1);
                for (;;) {
                    int status = ccsflash_poll();
                    if (status < 0) {
                        return status;
                    } else if ((status & SFLASH_ST_WIP) != SFLASH_ST_WIP) {
                        break;
                    }
                }
                /* Enable writes */
                ccsflash_cmd(SFLASH_ST_WREN);
                SERIAL_FLASH->flashaddress = addr;
                data = GET_BYTE(buffer);
                buffer++;
                SERIAL_FLASH->flashdata = data;
                /* Issue a page program with CSA bit set */
                ccsflash_cmd(SFLASH_ST_CSA | cmd);
            } else {
                /* Write single byte */
                data = GET_BYTE(buffer);
                buffer++;
                ccsflash_cmd(SFLASH_ST_CSA | data);
            }
            ret++;
            addr++;
            nbytes--;
        }
        /* All done, drop cs & poll */
        SERIAL_FLASH->flashcontrol = 0;
        OSL_DELAY(1);
        for (;;) {
            int status = ccsflash_poll();

            if (status < 0) {
                return status;
            } else if ((status & SFLASH_ST_WIP) != SFLASH_ST_WIP) {
                break;
            }
        }
        break;
    }

#ifndef _CFE_
    up(&spi_flash_lock);
#endif
    return ret;
}


/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/
static int spi_flash_get_numsectors(void)
{
    return meminfo.nsect;
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/
#if (INC_BTRM_BOOT==0) && defined(_CFE_) && defined(CFG_RAMAPP)
static
#endif
int spi_flash_get_sector_size(unsigned short sector)
{
    return meminfo.sec[sector].size;
}

/*********************************************************************/
/* The purpose of get_flash_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer.                                  */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/
static unsigned char *spi_get_flash_memptr(unsigned short sector)
{
    unsigned char *memptr = (unsigned char*)
        (meminfo.sec[sector].base);

    return memptr;
}

static unsigned char *spi_flash_get_memptr(unsigned short sector)
{
    return ((unsigned long)SPI_FLASH_BASE + spi_get_flash_memptr(sector));
}

static unsigned char *spi_flash_get_memptr_phys(unsigned short sector)
{
    return (SPI_FLASH_PHYS_BASE + spi_get_flash_memptr(sector));
}


/*********************************************************************/
/* The purpose of flash_get_blk() is to return a the block number */
/* for a given memory address.                                       */
/*********************************************************************/
static int spi_flash_get_blk(int addr)
{
    int blk_start, i;
    int last_blk = spi_flash_get_numsectors();
    int relative_addr = addr - (int) FLASH_BASE;

    for(blk_start=0, i=0; i < relative_addr && blk_start < last_blk; blk_start++)
        i += spi_flash_get_sector_size(blk_start);

    if( (unsigned int)i > (unsigned int)relative_addr ) {
        blk_start--;        // last blk, dec by 1
    } else {
        if( blk_start == last_blk )
        {
#if (INC_BTRM_BOOT==0)
            printk("Address is too big.\n");
#endif
            blk_start = -1;
        }
    }

    return( blk_start );
}

/************************************************************************/
/* The purpose of flash_get_total_size() is to return the total size of */
/* the flash                                                            */
/************************************************************************/
static int spi_flash_get_total_size(void)
{
    return totalSize;
}


#ifndef _CFE_
static int __init BcmSpiflash_init(void)
{
    return 0;
}
module_init(BcmSpiflash_init);

static void __exit BcmSpiflash_exit(void)
{
    bSpiFlashSlaveRes = FALSE;
}
module_exit(BcmSpiflash_exit);
#endif
