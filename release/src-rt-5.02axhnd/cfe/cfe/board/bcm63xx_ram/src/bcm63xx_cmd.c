/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  
    *  bcm63xx board specific routines and commands.
    *  
    *  by:  seanl
    *
    *       April 1, 2002
    *
    *********************************************************************  
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
    ********************************************************************* */


#include "bcm63xx_util.h"
#include "bcm63xx_auth.h"
#include "flash_api.h"
#include "shared_utils.h"
#include "jffs2.h"
#include "lib_math.h"
#if (INC_NAND_FLASH_DRIVER == 1) || (INC_SPI_PROG_NAND == 1) || (INC_SPI_NAND_DRIVER == 1)
#include "bcm_ubi.h"
#endif
#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "BPCM.h"
#endif
#if defined(_BCM94908_)|| defined(_BCM963158_)
#include "command.h"
#endif
#if defined (_BCM963268_) || defined (_BCM963381_)
#include "mii_shared.h"
#include "robosw_reg.h"
#elif (defined (_BCM96838_) || defined (_BCM96848_)) && (NONETWORK==0)
#include "phys_common_drv.h"
#elif defined (_BCM960333_) || defined(_BCM947189_)
#else
#if (NONETWORK == 0)
#include "bcm_ethsw.h"
#endif
#endif

#include "bcm_otp.h"
#if (INC_KERMIT==1) && (NONETWORK==1)
#include "cfe_kermit.h"
#endif
#include "bcm63xx_blparms.h"
#include "btrm_if.h"
#if (defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_) || defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
#include "rdp_cpu_ring.h"
#endif
#include "lib_byteorder.h"
#include "lib_malloc.h"
#include "rom_parms.h"
#include "bcm63xx_nvram.h"
#if INC_EMMC_FLASH_DRIVER
#include "dev_emmcflash.h"
#endif

#if defined (_BCM96858_) && (NONETWORK == 0)
#include "lport_drv.h"
#include "lport_stats.h"
#include "serdes_access.h"
#include "bcm6858_lport_xlmac_ag.h"
#define LPORT_MAX_PHY_ID (31)
#endif

#if (defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
#include "mdio_drv_impl5.h"
#include "mac_drv.h"
#endif

#if defined(BOARD_SEC_ARCH)
#include "bcm63xx_sec.h"
#endif

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

extern int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);
extern unsigned long mem_totalsize;

#ifdef USE_LZ4_DECOMPRESSOR
extern int LZ4_decompress_fast (const char* source, char* dest, int originalSize);
#endif

extern int send_rescueack(unsigned short no, unsigned short lo);

#if defined(CONFIG_ARM64)
#define LA_DEFAULT_FLAGS    LOADFLG_64BITIMG
#else
#define LA_DEFAULT_FLAGS    0x0
#endif

// global 
int g_processing_cmd = 0;
unsigned short g_force_mode = 0;
unsigned short g_raw_flash_write = 0;
int g_invalid_img_rev = 0; /* Used when CFE determines that current 
                            * running image is invalid for this cip rev */
int g_in_cfe = 0;
#if defined(CONFIG_ARM64)
int g_switch2aarch32 = 0;
#endif

/*image loading helper indices*/
typedef enum {id_img_boot,  id_img_ramdisk, id_img_dtb, id_img_max} id_img_index_t;

typedef struct run_info_s {
    char* ip; 
    char boot_opt; 
    char* img_id[id_img_max];
    unsigned long  laddr;
} run_info_t;

char fakeConsole[2] = " ";

#if (defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_) || defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
static char *vrpd_usage_str = "Usage: vrpd <ring index> <pd index>";
#endif

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);

#if defined (_BCM96858_) && (NONETWORK == 0)
static char *lport_rate_usage = "Usage: lport_rate <port id (0-6)> [10M|100M|1G|2.5G|10G]";
static char *lport_status_usage = "Usage: lport_status <port id (0-6)>";
static char *lport_stats_usage = "Usage: lport_stats <port id (0-7)> [\"clean\"]";
static char *lport_rgmii_usage = "Usage: lport_rgmii <port id (4-6)> [<valid 0|1> <ib_status_override 0|1> <phy_attached 0|1> <phy_id (0-31)>]";
static char *lport_serdes_reg_usage = "Usage: lport_serdes_reg <serdes id 0|1> <addr (hex)> [mask (hex)] [value (hex)]";
static char *lport_phy_usage = "Usage: phy <phy_id (0-31)> <register (hex)> [value (hex)]";
static char *send_buffer_usage = "Usage sbuf <port_id (0-6)> <number_of_packets> <is_random_size (0-1)> <packet_size (1-1536)>";
static char *serdes_set_loopback_usage = "Usage ssl <port_id(0-6)> <loopback_mode (0-4)> <enable (0-1)>";
static char *serdes_diag_status_usage = "Usage sds <port_id(0-6)>";
static char *serdes_diag_stats_usage = "Usage sdt <port_id(0-6)>";
static char *serdes_read_reg_usage = "Usage sdw <port_id(0-6)> <device_type> <reg_addr(offset)>";
static char *serdes_write_reg_usage = "Usage ssw <port_id(0-6)> <device_type> <reg_addr(offset)> <value(inHex)>";
static char *set_port_usage = "Usage set_port <port_id(0-6)>";
static char *set_compare_enable_usage = "Usage set_compare <val(0-1)>";
#endif
#if (defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)) && (NONETWORK == 0)
static char *network_stats_usage = "Usage net_stat [\"clean\"]";
static char *set_dump_enable_usage = "Usage set_dump <val(0-1)>";
#endif
#if  (defined (_BCM96846_) || defined(_BCM96856_)) && (NONETWORK == 0)
static char *unimac_stats_usage = "Usage: unimac_stats <port id (0-5)> [\"clean\"]";
#endif
extern int optee_init(void);
static int ui_cmd_set_board_param(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int ret = 0;
    int i = 0;
    int reset = 0;

    if(!argc)
    {
        reset += setBoardParam();
    }

    while(argc && !ret)
    {
        if(!strcmp(argv[i], "g"))
        {
            reset += setGponBoardParam();
        }
        else {
            /*Setup WPS Device Pin*/
            if(!strcmp(argv[i], "w"))
            {
                 reset += setWpsDevicePinBoardParam();
            }
            else
           {
               ret = -1;
           }
       }

        argc--;
        i++;
    }

    if(reset)
        softReset(0);

    return ret;
}

#if defined(INC_DHCLIENT)

static int ui_cmd_dhcp(ui_cmdline_t *cmd,int argc,char *argv[])
{
    dhcpreply_t *reply = NULL;
    int err = 0;
    // cfe_sleep((2*CFE_HZ));
    net_setparam(NET_IPADDR,(uint8_t *)"\0\0\0\0");
    err = dhcp_bootrequest(&reply);
    if (err == 0) {
        xprintf("address %d.%d.%d.%d\n",
           reply->dr_ipaddr[0],
           reply->dr_ipaddr[1],
           reply->dr_ipaddr[2],
           reply->dr_ipaddr[3]);
        net_setparam(NET_IPADDR,(uint8_t *)reply->dr_ipaddr);
        net_setparam(NET_GATEWAY,(uint8_t *)reply->dr_gateway);
        net_setparam(NET_NETMASK,(uint8_t *)reply->dr_netmask);
    } else {
        xprintf("DHCP failed %d\n",err);
    }
    return err;
}
#endif

static int ui_cmd_reset(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char* pszDelay = NULL;
    int delay = 0;

    pszDelay = cmd_getarg(cmd,0);
    if( pszDelay )
        delay = atoi(pszDelay);

    softReset(delay);
    return 0;
}

int actual_total_memsize=-1;

static int ui_cmd_set_memsize(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char* memsize_str = NULL;

    if(actual_total_memsize == -1)
        actual_total_memsize = mem_totalsize;
    memsize_str = cmd_getarg(cmd,0);
    if( memsize_str )
        mem_totalsize = atoi(memsize_str)*1024;

    if(actual_total_memsize < mem_totalsize)
    {
        printf("cannot set memory size to beyond actually available %dMB > %dMB\n", mem_totalsize/1024, actual_total_memsize/1024);
        mem_totalsize=actual_total_memsize;
    }

    printf("Forcing memory size to %d[0x%03xMB]\n", mem_totalsize, mem_totalsize/1024);

    return 0;
}

// return 0 if 'y'
int yesno(void)
{
    char ans[5];

    printf(" (y/n):");
    console_readline ("", ans, sizeof (ans));
    if (ans[0] != 'y')
        return -1;

    return 0;
}

// erase Persistent sector

int erase_psi = 0;

static int ui_cmd_run_program(ui_cmdline_t *cmd,int argc,char *argv[]);
static int ui_cmd_erase_psi(ui_cmdline_t *cmd,int argc,char *argv[])
{
    if (g_force_mode || (flash_get_flash_type() == FLASH_IFC_SPI))
    {
        printf("Erase persistent storage data?");
        if (yesno())
            return -1;

        printf("Erasing persistent storage data\n");
#if INC_EMMC_FLASH_DRIVER
    if ( flash_get_flash_type() ==  FLASH_IFC_UNSUP_EMMC ) 
        emmc_erase_psi();
    else
#endif        
        kerSysErasePsi();
    }
    else
    {
#if 0
        printf("Please PAY ATTENTION !\nPersistent storage will be marked for erase\n");
        printf("and continue boot to Linux.\n");
        printf("Should we continue ?");
#else
	printf("Erase persistent storage data?");
#endif
        if (yesno())
            return -1;

	kerSysErasePsi();
#if 0
        erase_psi = 1;
        ui_cmd_run_program(cmd, 0, NULL);
#endif
    }
    return 0;
}


#if (INC_SPI_NAND_DRIVER==1)
extern int spi_nand_page_read(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len);
#endif

static int ui_cmd_erase_nand(ui_cmdline_t *cmd,int argc,char *argv[])
{

#if (INC_NAND_FLASH_DRIVER==1)
    char *flag;
#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
    unsigned char *pBuf = NULL;
#endif
    int i, blk_end, blkSize = 0;
    unsigned char *buf = NULL;

    flag = cmd_getarg(cmd,0);

    if (!flag) {
        printf("'e b' to reinitialize NAND flash or 'e a' to erase kernel\n");
        return 0;
    }

    blkSize = flash_get_sector_size(0);
    buf = KMALLOC(blkSize + 2048, sizeof(void*));
    if (!buf) {
         goto end; 
    }

    switch (*flag)
    {
    case 'b':
    {
        char *badBlockInit = cmd_getarg(cmd, 1);

        printf("Reinitialize NAND flash?");
        if (yesno()) {
            goto end;
        }
        printf("\nNow think carefully.  Do you really,\n"
            "really want to reinitialize the NAND flash?");
        if (yesno())
            goto end;


#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
        /* preserve the pmc code in first 64KB */
        if ((pBuf = (unsigned char *) KMALLOC(IMAGE_OFFSET, sizeof(long))) != NULL)
            flash_read_buf(0, 0, pBuf, IMAGE_OFFSET);
        StallPmc();
#endif

        if (badBlockInit && (badBlockInit[0] == '1')) {
            printf("Initializing BAD BLOCKS also.\n");
            flash_dev_specific_cmd(NAND_REINIT_FLASH_BAD, NULL, NULL);
        } else {
            printf("Initializing GOOD BLOCKS only.\n");
            flash_dev_specific_cmd(NAND_REINIT_FLASH, NULL, NULL);
        }

#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
        if( pBuf ) {
            flash_write_buf(0, 0, pBuf, IMAGE_OFFSET);
            KFREE(pBuf);
        }
        UnstallPmc();
#endif

    }
        break;
    case 'a':
        {
        printf("Erase NAND flash? The modem will not be able to boot from "
            "flash");
        if (yesno())
            goto end;

        validateNandPartTbl(0, 0);
        blk_end = NVRAM.ulNandPartOfsKb[NP_BBT] /
            (flash_get_sector_size(0) / 1024);

        for (i = 1; i < blk_end; i++) {
            printf(".");
            flash_sector_erase_int(i);
        }
        printf("\n");
	
        }
        break;
    case 'n':
        printf("Erase nvram?");
        if (yesno())
            goto end;

        NVRAM_ERASE();
        softReset(0);
        break;
    case 'p':
        ui_cmd_erase_psi(cmd,argc,argv);
        break;
    case 's':
        {
#if (INC_SPI_NAND_DRIVER==1)
            extern void dump_spi_spare(void);

            if (flash_get_flash_type() == FLASH_IFC_SPINAND)
                dump_spi_spare();
#endif
            extern void dump_spare(void);

            if (flash_get_flash_type() == FLASH_IFC_NAND)
                dump_spare();

        }
        break;

    case 'r':
    case 'i':
    case 'W':
        {
        extern int read_spare_data(int blk, int offset, unsigned char *buf, int bufsize, int read_all_oob);

        void ui_dumpaddr( unsigned char *pAddr, int nLen, int offset );
        unsigned char spare[64];
        char *pszLen = cmd_getarg(cmd, 2);
        int len = (pszLen) ? atoi(pszLen) : 64;
        char *pszBlk = cmd_getarg(cmd, 1);
        int blk = atoi(pszBlk);
        int status = 0;

        if (*flag == 'i')
            if( flash_sector_erase_int(blk) < 0 )
                printf("Error erasing block %d\n", blk);

        if( flash_read_buf(blk, 0, buf, blkSize) < 0 )
            printf("Error reading block %d\n", blk);

#if (INC_SPI_NAND_DRIVER==1)
        if (flash_get_flash_type() == FLASH_IFC_SPINAND)
            status = spi_nand_page_read(blk * flash_get_sector_size(0), flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL), spare, sizeof(spare));
#endif
        if (flash_get_flash_type() == FLASH_IFC_NAND)
            status = read_spare_data(blk, 0, spare, sizeof(spare), 1);

        if (status > 0)
        {
            printf("spare area for blk %d\n", blk);
            ui_dumpaddr(spare, sizeof(spare), 0);
        }

        /* Use dm command to view entire block contents. */
        printf("block read into buffer at 0x%8.8lx\n", (unsigned long)buf);
        ui_dumpaddr(buf, len, 0);

        if (*flag == 'W')
        {
            flash_sector_erase_int(atoi(pszBlk));        
            if( flash_write_buf(blk, 0, buf, blkSize) > 0 )
                printf("block rewritten\n");
            else
                printf("block NOT rewritten\n");

        }

        }
        break;
    case 'w':
        {
        char *pszBlk = cmd_getarg(cmd, 1);
        char szData[] = "The quick brown fox jumped over the lazy dog. ";
        int dataSize = sizeof(szData) - 1;
        int i;
        char *p;

        for(i = 0, p = (char *) buf; i < blkSize / dataSize; i++, p += dataSize)
            strcpy(p, szData);
        
        flash_sector_erase_int(atoi(pszBlk));        
        if( flash_write_buf(atoi(pszBlk), 0, buf, blkSize) > 0 )
        {
            printf("block written\n");
        }
        else
            printf("block NOT written\n");
        /* Can break into JTAG now to view entire block contents. */
        }
        break;
    default:
        printf("Erase [n]vram, [p]ersistent storage or [a]ll flash except bootrom\nusage: e [n/p/a]\n");
        goto end;
    }
end:
    KFREE(buf);
#elif (INC_SPI_PROG_NAND==1)
    char *flag;
#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM96856_)
    unsigned char *pBuf = NULL;
#endif
    unsigned char *buf = NULL;
    int i, blk_end, blkSize = 0;


    flash_change_flash_type(FLASH_IFC_NAND);

    flag = cmd_getarg(cmd,0);

    if (!flag) 
    {
        printf("'n b' to reinitialize NAND flash or 'n a' to erase kernel on NAND\n");
        goto finish;
    }

    blkSize = flash_get_sector_size(0);
    buf = KMALLOC(blkSize + 2048, sizeof(void*));
    if (!buf) {
         goto end; 
    }
    switch (*flag)
    {
    case 'b':
    {
        int opt ;
        char *badBlockInit = cmd_getarg(cmd, 1);

        if (badBlockInit && (badBlockInit[0] == '1'))
        {
            printf("Initializing BAD BLOCKS also.\n");
            opt = NAND_REINIT_FLASH_BAD;
        }
        else
        {
            printf("Initializing GOOD BLOCKS only.\n");
            opt = NAND_REINIT_FLASH;            
        }
        
        printf("Reinitialize NAND flash?");
        if (yesno())
            goto end;

        printf("\nNow think carefully.  Do you really,\n"
            "really want to reinitialize the NAND flash?");
        if (yesno())
            goto end;


#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
        /* preserve the pmc code in first 64KB */
        if ((pBuf = (unsigned char *) KMALLOC(IMAGE_OFFSET, sizeof(long))) != NULL)
             flash_read_buf(0, 0, pBuf, IMAGE_OFFSET);
        StallPmc();
#endif

        flash_dev_specific_cmd(opt, NULL, NULL);

#if defined(_BCM963138_) || defined(_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_)
        if( pBuf )
        {
            flash_write_buf(0, 0, pBuf, IMAGE_OFFSET);
            KFREE(pBuf);
        }
        UnstallPmc();
#endif

    }
        break;

    case 'a':
        printf("Erase NAND flash? The modem will not be able to boot from "
            "flash");
        if (yesno())
            goto finish;

        blk_end = flash_get_numsectors();
        for (i = 1; i < blk_end; i++)
        {
            printf(".");
            flash_sector_erase_int(i);
        }
        printf("\n");
        break;
    case 'n':
        printf("Erase nvram?");
        if (yesno())
            goto end;

        NVRAM_ERASE();
        softReset(0);
        break;
    case 'p':
        ui_cmd_erase_psi(cmd,argc,argv);
        break;
    case 's':
        {
#if (INC_SPI_NAND_DRIVER==1)
            extern void dump_spi_spare(void);

            if (flash_get_flash_type() == FLASH_IFC_SPINAND)
                dump_spi_spare();
#endif
            extern void dump_spare(void);

            if (flash_get_flash_type() == FLASH_IFC_NAND)
                dump_spare();
        }
        break;

    case 'r':
    case 'i':
        {
        void ui_dumpaddr( unsigned char *pAddr, int nLen, int offset );
        extern int read_spare_data(int blk, int offset, unsigned char *buf, int bufsize, int read_all_oob);
        unsigned char spare[64];
        char *pszLen = cmd_getarg(cmd, 2);
        int len = (pszLen) ? atoi(pszLen) : 64;
        char *pszBlk = cmd_getarg(cmd, 1);
        int blk = atoi(pszBlk);
        int status = 0;

        if (*flag == 'i')
            if( flash_sector_erase_int(blk) < 0 )
                printf("Error erasing block %d\n", blk);

        if( flash_read_buf(blk, 0, buf, blkSize) < 0 )
            printf("Error reading block %d\n", blk);

#if (INC_SPI_NAND_DRIVER==1)
        if (flash_get_flash_type() == FLASH_IFC_SPINAND)
            status = spi_nand_page_read(blk * flash_get_sector_size(0), flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL), spare, sizeof(spare));
#endif
        if (flash_get_flash_type() == FLASH_IFC_NAND)
            status = read_spare_data(blk, 0, spare, sizeof(spare), 1);

        if( status > 0 )
        {
            printf("spare area for blk %d\n", blk);
            ui_dumpaddr(spare, sizeof(spare), 0);
        }

        /* Use dm command to view entire block contents. */
        printf("block read into buffer at 0x%8.8lx\n", (unsigned long)buf);
        ui_dumpaddr(buf, len, 0);
        }
        break;
    case 'w':
        {
        char *pszBlk = cmd_getarg(cmd, 1);
        char szData[] = "The quick brown fox jumped over the lazy dog. ";
        int dataSize = sizeof(szData) - 1;
        int i;
        char *p;

        for(i = 0, p = (char *) buf; i < blkSize / dataSize; i++, p += dataSize)
            strcpy(p, szData);

        flash_sector_erase_int(atoi(pszBlk));
        if( flash_write_buf(atoi(pszBlk), 0, buf, blkSize) > 0 )
        {
            printf("block written\n");
        }
        else
            printf("block NOT written\n");
        /* Can break into JTAG now to view entire block contents. */
        }
        break;
    default:
        printf("Erase [n]vram, [p]ersistent storage or [a]ll flash except bootrom on NAND\nusage: n [n/p/a]\n");
    }
finish:
    flash_change_flash_type(FLASH_IFC_SPI);
end:
    KFREE(buf);
#endif
    return 0;
}

// erase some sectors
static int ui_cmd_erase(ui_cmdline_t *cmd,int argc,char *argv[])
{

    //FILE_TAG cfeTag;
    PFILE_TAG pTag;
    char *flag;
    int i, blk_start, blk_end;

    flag = cmd_getarg(cmd,0);

    if (!flag) 
    {
        printf("Erase [n]vram, [p]ersistent storage or [a]ll flash except bootrom\nusage: e [n/p/a]\n");
        return 0;
    }

    switch (*flag)
    {
    case 'b':
        printf("Erase boot loader?");
        if (yesno())
            return 0;
        printf("\nNow think carefully.  Do you really,\n"
            "really want to erase the boot loader?");
        if (yesno())
            return 0;
        blk_start = flash_get_blk(IMAGE_OFFSET);
        flash_sector_erase_int(blk_start);
        break;
    case 'n':
        printf("Erase nvram?");
        if (yesno())
            return 0;
        NVRAM_ERASE();
        softReset(0);
        break;
    case 'a':

        printf("Erase all flash (except bootrom)?");
        if (yesno())
            return 0;

        blk_end = flash_get_numsectors();
        if ((pTag = getTagFromPartition(1)) != NULL)
            blk_start = flash_get_blk(atoi(pTag->rootfsAddress) + BOOT_OFFSET + IMAGE_OFFSET);
        else  // just erase all after cfe
        {
            FLASH_ADDR_INFO flash_info;

            kerSysFlashAddrInfoGet(&flash_info);
            for( blk_start = 0, i = 0; i<flash_info.flash_rootfs_start_offset &&
                 blk_start < blk_end; blk_start++ )
            {
                i += flash_get_sector_size(blk_start);
            }
            printf("No image tag found.  Erase the blocks start at [%d]\n",
                blk_start);
        }

        if( blk_start > 0 )
        {
            for (i = blk_start; i < blk_end; i++)
            {
                printf(".");
                flash_sector_erase_int(i);
            }
            printf("\n");
        }

        /* Preserve the NVRAM fields that are used in the 'b' command. */
        softReset(0);
        break;
    case 'p':
        ui_cmd_erase_psi(cmd,argc,argv);
        break;
    default:
        printf("Erase [n]vram, [p]ersistent storage or [a]ll flash except bootrom\nusage: e [n/p/a]\n");
        return 0;
    }

    return 0;
}


static int loadRaw(char *hostImageName, uint8_t *ptr)
{
    cfe_loadargs_t la;
    int res;

    printf("Loading %s ...\n", hostImageName);

    // tftp only
    la.la_filesys = "tftp";
    la.la_filename = hostImageName;
    la.la_device = NULL;
    la.la_address = (unsigned long)ptr;
    la.la_options = NULL;
    la.la_maxsize = cfe_get_mempool_size();
    la.la_flags =  LOADFLG_SPECADDR;

    res = bcm63xx_cfe_rawload(&la);
    if (res < 0)
    {
        ui_showerror(res, "Loading failed.");
        return res;
    }
    printf("Finished loading %d bytes\n", res);

    return res;
}

// flash the image 
static int ui_cmd_flash_image(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char hostImageName[BOOT_FILENAME_LEN + BOOT_IP_LEN];
    char *imageName;
    int res = -1;
    uint8_t *ptr = (uint8_t*) cfe_get_mempool_ptr();

    g_processing_cmd = 1;

    imageName = cmd_getarg(cmd, 0);
    
    if (imageName)
    {
        if (strchr(imageName, ':'))
            strcpy(hostImageName, imageName);
        else
        {
            strcpy(hostImageName, bootInfo.hostIp);
            strcat(hostImageName, ":");
            strcat(hostImageName, imageName);
        }
    }
    else  // use default flash file name
    {
        strcpy(hostImageName, bootInfo.hostIp);
        strcat(hostImageName, ":");
        strcat(hostImageName, bootInfo.flashFileName);
    }

    if ((res = loadRaw(hostImageName, ptr)) < 0)
    {
        g_processing_cmd = 0;
        return res;
    }

    // check and flash image
    res = flashImage(ptr);

    if( res == 0 ) {
        char *p;
	char* bootline = KMALLOC(sizeof(NVRAM.szBootline), sizeof(void*));
        if (!bootline) {
	    return -1;
	}
        strncpy(bootline, NVRAM.szBootline, sizeof(NVRAM.szBootline)/sizeof(char));
        for( p = bootline; p[2] != '\0'; p++ ) {
            if( p[0] == 'r' && p[1] == '=' && p[2] == 'h' ) {
                /* Change boot source to "boot from flash". */
                p[2] = 'f';
		NVRAM_UPDATE_FIELD(szBootline, bootline, sizeof(NVRAM.szBootline));
                break;
            }
        }
        KFREE(bootline);
        softReset(0);
    }

    g_processing_cmd = 0;
    return( res );
}

#if defined(CFG_DT)
static int ui_cmd_load_dtb(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char hostDtbName[BOOT_FILENAME_LEN + BOOT_IP_LEN];
    char *dtbName;
    int len;
    uint8_t *ptr = (uint8_t*) cfe_get_mempool_ptr();

    dtbName = cmd_getarg(cmd, 0);
    
    if (dtbName)
    {
        if (strchr(dtbName, ':'))
            strcpy(hostDtbName, dtbName);
        else
        {
            strcpy(hostDtbName, bootInfo.hostIp);
            strcat(hostDtbName, ":");
            strcat(hostDtbName, dtbName);
        }
    }
    else
    {
      printf("no dtb host and file name found!\r\n");
        return -1;
    }

    if ((len = loadRaw(hostDtbName, ptr)) < 0)
        return -1;
 
    return dtb_prepare(DTB_ID_CFE, DTB_PREPARE_FDT, NULL, ptr, len);
}
#endif

// write the whole image
static int ui_cmd_write_whole_image(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char hostImageName[BOOT_FILENAME_LEN + BOOT_IP_LEN];
    char *imageName;
#if defined (_BCM963138_) || (_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM96856_)
    char *pmcName = NULL;
#endif
    uint8_t *ptr = (uint8_t *) cfe_get_mempool_ptr();
    int res;
    int i;

    g_processing_cmd = 1;

    imageName = cmd_getarg(cmd, 0);
    if (!imageName) 
        return ui_showusage(cmd);

    if (strchr(imageName, ':'))
        strcpy(hostImageName, imageName);
    else
    {
        strcpy(hostImageName, bootInfo.hostIp);
        strcat(hostImageName, ":");
        strcat(hostImageName, imageName);
    } 

    if ((res = loadRaw(hostImageName, ptr)) < 0)
    {
        g_processing_cmd = 0;
        return res;
    }
#if defined (_BCM963138_) || (_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM96856_)
    /* check if it is for pmc image */
    pmcName = cmd_getarg(cmd, 1);
    if( pmcName && (strncmp(pmcName, "pmc", 3) == 0) && (flash_get_flash_type() != FLASH_IFC_NAND ) && ( flash_get_flash_type() != FLASH_IFC_SPINAND )) 
    {
        if( res <= IMAGE_OFFSET && res < flash_get_sector_size(0) )
        {
            printf("flash PMC ROM image %d bytes.\n", res);
            flash_sector_erase_int(0);
            if( flash_write_buf(0, 0, ptr, res) != res) 
            {
                 printf("Failed to write pmc\n");
                 res = -1;
            }
            else 
            {
                printf("Finished flashing image.\n");
                res = 0;
            }
        }
        g_processing_cmd = 0;
        return( res );
    }
    else
#endif

#if INC_NAND_FLASH_DRIVER == 1 || INC_SPI_PROG_NAND == 1 || INC_SPI_NAND_DRIVER == 1
    if(argc == 2)
    {
        res = writeWholeImageDataPartition(ptr, res, cmd->argv[2]);
    }
    else if ( g_raw_flash_write == 1)
    {
        res = writeWholeImageRaw(ptr, res, 0);
    }
    else
#endif
    {
        res = writeWholeImage(ptr, res);
	for (i=0; i<6; i++)
		send_rescueack(0x0006, (res == 0) ? 0x0001 : 0x0000);

    }

    printf("Finished flashing image.\n");

    if (res == 0)
    {
        softReset(0);
    }

    g_processing_cmd = 0;
    return( res );
}


/*  *********************************************************************
    *  cfe_go(la)
    *  
    *  Starts a previously loaded program.  cfe_loadargs.la_entrypt
    *  must be set to the entry point of the program to be started
    *  
    *  Input parameters: 
    *      la - loader args
    *     
    *  Return value:
    *      does not return
    ********************************************************************* */

void cfe_go(cfe_loadargs_t *la)
{
    if (la->la_entrypt == 0) {
        xprintf("No program has been loaded.\n");
        return;
    }

    if (net_getparam(NET_DEVNAME)) {
        xprintf("Closing network.\n");
        net_uninit();
    }

    xprintf("Starting program at 0x%p\n",la->la_entrypt);

    blparms_install((unsigned long *) la->la_address);

    setPowerOnLedOn();
#if defined(CONFIG_ARM64)
    if( (la->la_flags&LOADFLG_64BITIMG) == 0 )
    {
        xprintf("Switching to AArch32 mode\n");
        g_switch2aarch32 = 1;
    }
#endif

#if INC_EMMC_FLASH_DRIVER
    /* Reset eMMC - Note: Do this after all CFE based eMMC accesses are done */
    if (emmc_go_idle() != 0) 
    {
        printf("Failed to set eMMC to Idle mode!\n");
        return;
    }
#endif    

#if defined(BOARD_SEC_ARCH)
    cfe_sec_set_sotp_access_permissions();
#endif

    cfe_start(la->la_entrypt);
}

static int bootImage(char *fileSys, char *device, int zflag, char *imageName)
{
    cfe_loadargs_t la;
    int res;

    // elf only
    la.la_filesys = fileSys;
    la.la_filename = imageName;
    la.la_device = device;
    la.la_options = 0;
    la.la_maxsize = 0;
    la.la_address = 0;
    la.la_flags =  zflag;

    res = bcm63xx_cfe_elfload(&la); 
    if (res != 0)
        return res;

    if (la.la_flags & LOADFLG_NOISY) 
        xprintf("Entry at 0x%p\n",la.la_entrypt);
    if ((la.la_flags & LOADFLG_EXECUTE) && (la.la_entrypt != 0)) {
        cfe_go(&la);
    }

    return res;
}

#if (INC_NAND_FLASH_DRIVER==0)
// Compressed image head format in Big Endian:
// 1) Text Start address:    4 bytes
// 2) Program Entry point:   4 bytes
// 3) Compress image Length: 4 bytes
// 4) Compress data starts:  compressed data
static int bootCompressedImage(uint32_t *puiCmpImage, int retry)
{
    unsigned char *pucSrc;
    char          brcmMagic[] = {'B','R','C','M'};
    int ret = 0;
    cfe_loadargs_t la;
    bcm_image_hdr_t image_hdr;
    uint32_t image_hdr_size;

    memset((unsigned char *) &la, 0x00, sizeof(la));
    if( (uintptr_t) puiCmpImage > FLASH_BASE )
    {
        /* Boot compressed image from flash. */
        unsigned int *puiOrigCmpImage = (unsigned int*)((unsigned char*)puiCmpImage + IMAGE_OFFSET);
        unsigned int *puiNewCmpImage = NULL;
        unsigned int *puiOldCmpImage = NULL;
        unsigned int *puiFs = NULL;
        PFILE_TAG pTag1 = getTagFromPartition(1);
        PFILE_TAG pTag2 = getTagFromPartition(2);
        PFILE_TAG pCurTag = NULL;
        unsigned int *puiImg= NULL;
        int nImgLen = 0;
        unsigned int ulCrc, ulImgCrc;
        int bootImg = BOOTED_NEW_IMAGE;

        if( pTag1 && pTag2 )
        {
            /* Two images are on flash.  Determine which one is being booted. */
            PFILE_TAG pNewTag = NULL;
            PFILE_TAG pOldTag = NULL;
            int seq1 = atoi(pTag1->imageSequence);
            int seq2 = atoi(pTag2->imageSequence);

            if( seq1 > seq2 )
            {
                pNewTag = pTag1;
                pOldTag = pTag2;
            }
            else
            {
                pNewTag = pTag2;
                pOldTag = pTag1;
            }

            puiNewCmpImage = (unsigned int *)(uintptr_t)
                (atoi(pNewTag->kernelAddress) + BOOT_OFFSET + IMAGE_OFFSET);
            puiOldCmpImage = (unsigned int *)(uintptr_t)
                (atoi(pOldTag->kernelAddress) + BOOT_OFFSET + IMAGE_OFFSET);

            if( puiOrigCmpImage == puiOldCmpImage )
            {
                printf("Booting from previous image (0x%8.8lx) ...\n",
                    (unsigned long) atoi(pOldTag->rootfsAddress) + IMAGE_OFFSET +
                    BOOT_OFFSET - TAG_LEN);
                pCurTag = pOldTag;
                bootImg = BOOTED_OLD_IMAGE;
            }
            else
            {
                printf("Booting from latest image (0x%8.8lx) ...\n",
                    (unsigned long) atoi(pNewTag->rootfsAddress) + IMAGE_OFFSET +
                    BOOT_OFFSET - TAG_LEN);
                pCurTag = pNewTag;
                bootImg = BOOTED_NEW_IMAGE;
            }
        }
        else
            if( pTag1 || pTag2 )
            {
                /* Only one image on flash. */
                pCurTag = (pTag1) ? pTag1 : pTag2;
                printf("Booting from only image (0x%8.8lx) ...\n",
                    (unsigned long) atoi(pCurTag->rootfsAddress) + IMAGE_OFFSET +
                    BOOT_OFFSET - TAG_LEN);
                bootImg = BOOTED_ONLY_IMAGE;
            }
            else
            {
                /* No image on flash. */
                printf("No valid boot image\n");
                ret = -1;
            }

        if( ret == 0 )
        {
            /* Copy compressed image to SDRAM. */
            unsigned char* pDestBndry = (unsigned char*) cfe_get_mempool_ptr(), *pDest = NULL;
	    pDest = pDestBndry + 1024;

            kerSysReadFromFlash( pDest, (uintptr_t) (puiCmpImage) + IMAGE_OFFSET,
                atoi(pCurTag->kernelLen));

#if defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)
            /* If secure boot, put the signature in front of the kernel */
            if (bcm_otp_is_boot_secure())
	       memcpy((void *)(pDest - SEC_S_SIGNATURE), (void *)(pDest + atoi(pCurTag->kernelLen) - SEC_S_SIGNATURE), SEC_S_SIGNATURE);
#endif

            puiCmpImage = (uint32_t *) pDest;

            /* Copy file system to SDRAM. */
            pDest += atoi(pCurTag->kernelLen) + 1024;
#if defined(_BCM947189_)
            /* Align destination buffer to a word boundary */
            pDest = (unsigned char *)((uintptr_t)pDest & 0xfffffff0);
#endif
            kerSysReadFromFlash( pDest, (uintptr_t)
                (atoi(pCurTag->rootfsAddress) + BOOT_OFFSET + IMAGE_OFFSET),
                atoi(pCurTag->rootfsLen));

            puiFs = (unsigned int *) pDest;

            memcpy(&image_hdr, (void*)puiCmpImage, sizeof(image_hdr)); 
            /* leagacy adjustments:
                new image format contains broadcom signature and uncompressed length.*/
            image_hdr_size = (image_hdr.magic == *(uint32_t*)brcmMagic)? sizeof(image_hdr) : sizeof(image_hdr)-(2*sizeof(uint32_t));
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
            /* ARM linux kernel compiled for virtual address at 0xc0008000,
                     convert to physical address for CFE */
            image_hdr.la = (image_hdr.la&0xfffffff);
            image_hdr.entrypt = (image_hdr.entrypt&0xfffffff);
#endif
            pucSrc = (unsigned char*)puiCmpImage + image_hdr_size;
            printf("Code Address: 0x%08X, Entry Address: 0x%08x\n",
                (uintptr_t) image_hdr.la, (uintptr_t) image_hdr.entrypt);

            /* Check Linux file system CRC */
            ulImgCrc = *(unsigned int *) (pCurTag->imageValidationToken +
                CRC_LEN);
            if( ulImgCrc ) {
                if( puiFs )
                    puiImg = puiFs;
                else {
                    puiImg = (unsigned int *) ((uintptr_t)(atoi(pCurTag->rootfsAddress) +
                        BOOT_OFFSET + IMAGE_OFFSET));
                }
                nImgLen = atoi(pCurTag->rootfsLen);

                ulCrc = CRC32_INIT_VALUE;
                ulCrc = getCrc32((unsigned char *) puiImg, (UINT32) nImgLen, ulCrc);      
                if( ulCrc != ulImgCrc)
                {
                    printf("Linux file system CRC error.  Corrupted image?\n");
                    ret = -1;
                }
            }

            /* Check Linux kernel CRC */
            ulImgCrc = *(unsigned int *) (pCurTag->imageValidationToken +
                (CRC_LEN * 2));
            if( ulImgCrc ) {
                puiImg = (unsigned int *) puiCmpImage;
                nImgLen = atoi(pCurTag->kernelLen);

                ulCrc = CRC32_INIT_VALUE;
                ulCrc = getCrc32((unsigned char *) puiImg, (UINT32) nImgLen, ulCrc);      
                if( ulCrc != ulImgCrc) {
                    printf("Linux kernel CRC error.  Corrupted image?\n");
                    ret = -1;
                }
            }

#if defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)
            if (bcm_otp_is_boot_secure()) {
	       /* Authenticate the vmlinux.lz image */
               Booter1AuthArgs authArgs;
               extern unsigned char _ftext;
#if defined(_BCM963138_) || defined(_BCM963148_)
	       void *authCreds = (void *)(&_ftext - (16*1024) - sizeof(Booter1AuthArgs));
#else
	       void *authCreds = (void *)(&_ftext - sizeof(Booter1AuthArgs));
#endif
	       memcpy((void *)&authArgs, authCreds, sizeof(Booter1AuthArgs));
               printf("%s", "Authenticating vmlinux.lz ... "); 
               authenticate((uint8_t *)(((uintptr_t)pucSrc) - (image_hdr_size + SEC_S_SIGNATURE)), image_hdr.len + image_hdr_size + SEC_S_SIGNATURE, authArgs.manu);
               printf("%s\n", "pass"); 
            }
#endif

            if( ret == 0 ) {
#ifdef USE_LZ4_DECOMPRESSOR
                if( image_hdr.len_uncomp ) {
                    ret = LZ4_decompress_fast((const char *)pucSrc, (char *)image_hdr.la, image_hdr.len_uncomp) != image_hdr.len;
                } else
#endif
                {
                    ret = decompressLZMA(pucSrc, image_hdr.len, (unsigned char*)(unsigned long)image_hdr.la, 23*1024*1024);
                }
                if (ret != 0) 
                    printf("Failed to decompress image.  Corrupted image?\n");
            }

            if (ret != 0) {
                /* Try to boot from the other flash image, if one exists. */
                if( retry == TRUE && pTag1 && pTag2 ) {
                    int blk = 0;
                    unsigned char *pBase = flash_get_memptr(0);
                    uint32_t *flash_addr_kernel;
                    FLASH_ADDR_INFO flash_info;

                    /* The boot image is bad.  Erase the sector with the tag so
                     * the image is not tried in subsequent boots.
                     */
                    kerSysFlashAddrInfoGet(&flash_info);
                    if( pCurTag == pTag1 ) {
                        blk = flash_get_blk((int)((uintptr_t)pBase +
                            flash_info.flash_rootfs_start_offset));
                    } else if( pCurTag == pTag2 ) {
         	            blk = flash_get_blk((int) ((uintptr_t)pBase +
                                (flash_get_total_size()/2) + IMAGE_OFFSET));
                    }

                    if( blk )
                        flash_sector_erase_int(blk);

                    /* Boot from the other flash image. */
                    if( puiOrigCmpImage == puiOldCmpImage )
                        flash_addr_kernel = puiNewCmpImage - IMAGE_OFFSET;
                    else
                        flash_addr_kernel = puiOldCmpImage - IMAGE_OFFSET;

                    /* Don't try this at home*/ 
                    ret = bootCompressedImage( flash_addr_kernel, FALSE );
                }
            } else {
                blparms_set_int(BOOTED_IMAGE_ID_NAME, bootImg);
                printf("Decompression %s image OK!\n",image_hdr.len_uncomp ? "LZ4":"LZMA");
                la.la_address = (long)image_hdr.la;
                la.la_entrypt = (long)image_hdr.entrypt;
                la.la_flags = LA_DEFAULT_FLAGS;
                printf("Entry at 0x%p\n",la.la_entrypt);
                cfe_go(&la);  // never return...
            }
        }
    }
    else
    {
        /* Boot compressed image that was downloaded to RAM. */

        memcpy(&image_hdr, (void*)puiCmpImage, sizeof(image_hdr)); 
        /* leagacy adjustments:
           new image format contains broadcom signature and uncompressed length.*/
        image_hdr_size = (image_hdr.magic == *(uint32_t*)brcmMagic)? sizeof(image_hdr) : sizeof(image_hdr)-(2*sizeof(uint32_t));
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        /* ARM linux kernel compiled for virtual address at 0xc0008000,
                     convert to physical address for CFE */
        image_hdr.la = (image_hdr.la&0xfffffff);
        image_hdr.entrypt = (image_hdr.entrypt&0xfffffff);
#endif
        pucSrc = (unsigned char*)puiCmpImage + image_hdr_size; 
        printf("Code Address: 0x%08X, Entry Address: 0x%08x\n", image_hdr.la, image_hdr.entrypt);
#ifdef USE_LZ4_DECOMPRESSOR
        if( image_hdr.len_uncomp ) 
        {
            ret = LZ4_decompress_fast((const char *)pucSrc, (char *)image_hdr.la, image_hdr.len_uncomp) != image_hdr.len;
        }
        else
#endif 
       {
            ret = decompressLZMA(pucSrc, image_hdr.len, (unsigned char*)(unsigned long)image_hdr.la, 23*1024*1024);
       } 
      
        if (ret == 0) 
        {
            printf("Decompression %s image OK!\n",image_hdr.len_uncomp ? "LZ4":"LZMA");
            la.la_entrypt = (long) image_hdr.entrypt;
            la.la_flags = LA_DEFAULT_FLAGS;
            printf("Entry at 0x%p\n",la.la_entrypt);
            cfe_go(&la);  // never return...
        }
        else
            printf("Failed on decompression.  Corrupted image?\n");
    }

    return ret;
}

#else

static int bootCompressedImage(uint32_t *puiCmpImage, int retry)
{
    return(-1);
}
#endif

/* only support uncompressed kernel elf image + uncompressed ramdisk image */
static int bootElfRamfsImage(char* img_addr[id_img_max], 
                             unsigned int rd_addr)
{
    uint8_t *ptr;
    int size;
    
    ptr = (uint8_t *)((uintptr_t)KERNADDR(rd_addr));
    printf("Loading %s image at address 0x%08x..\n", img_addr[id_img_dtb],ptr);
    size = loadRaw(img_addr[id_img_dtb], ptr); 
    if (size < 0) {
        return -1;
    } 
    if (dtb_prepare(DTB_ID_CFE, DTB_PREPARE_FDT, NULL, ptr, size)) {
        return -1;
    }
    /* 1. load ramfsImage to FLASH_STAGING_BUFFER */
    printf("Loading image %s at address 0x%08x..\n",img_addr[id_img_ramdisk], rd_addr);
    size = loadRaw(img_addr[id_img_ramdisk], ptr);
    if (size < 0) {
        return size;
    }

    if (dtb_set_chosen_initrd(DTB_ID_CFE, rd_addr, size)) {
        return -1; 
    }
    /* 2. load and boot uncompressed kernel elf image */
    size = bootImage("tftp", "eth0",  LOADFLG_EXECUTE | LOADFLG_NOISY, img_addr[id_img_boot]);
    if (size) {
        printf("Error: Failed to load/execute %s\n",img_addr[id_img_boot]);
    }
    return size;
}

#if (INC_NAND_FLASH_DRIVER==1)
static int bootNandImageFromRootfs(int start_blk, int end_blk, int bootImg)
{
    char brcmMagic[] = {'B','R','C','M'};
    uint8_t *image = NULL;
    uint32_t image_hdr_size = 0, image_size = 0, 
           image_sig_size = 0;
    int sector_size, ret = 0;
    bcm_image_hdr_t image_hdr;
    cfe_loadargs_t la;

    memset((unsigned char *) &la, 0x00, sizeof(la));
    la.la_flags = LA_DEFAULT_FLAGS;
    sector_size = flash_get_sector_size(0);

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (load_hash_block(start_blk, end_blk) != 0)
        die();
#endif // CONFIG_CFE_SUPPORT_HASH_BLOCK

#if !defined(_BCM96848_) && !defined(_BCM947189_)
     unsigned char* image_sig = NULL;
#ifndef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (bcm_otp_is_boot_secure()) {
        if ( cfe_fs_find_file(NAND_FLASH_BOOT_SIG_NAME,
                              C_CSTRLEN(NAND_FLASH_BOOT_SIG_NAME),
                              start_blk, end_blk - start_blk,
                              &image_sig, &image_sig_size, 0) ) {
             return -1;
        }
        /* Assumption is here that SEC_S_SIGNATURE is 4 byte aligned*/
#if (SEC_S_SIGNATURE&3) != 0
#error "SEC_S_SIGNATURE must be 4 byte aligned"
#endif
        if (SEC_S_SIGNATURE != image_sig_size) {
            printf("Image is corrupt %s\n",NAND_FLASH_BOOT_SIG_NAME);
            return -1;
        }
    }
#endif // !CONFIG_CFE_SUPPORT_HASH_BLOCK
#endif // devices that have secure boot otp
    if (
#ifdef USE_LZ4_DECOMPRESSOR 
        cfe_fs_find_file(NAND_FLASH_BOOT_IMAGE_LZ4, C_CSTRLEN(NAND_FLASH_BOOT_IMAGE_LZ4), start_blk, 
                               end_blk - start_blk, &image, &image_size, image_sig_size) && 
#endif
        cfe_fs_find_file(NAND_FLASH_BOOT_IMAGE_LZ, C_CSTRLEN(NAND_FLASH_BOOT_IMAGE_LZ), start_blk, 
                          end_blk - start_blk, &image, &image_size, image_sig_size) 
      ) 
    {
         printf("ERROR: file %s is not found.\n",NAND_FLASH_BOOT_IMAGE_LZ);
         return -1; 
    }

#if !defined(_BCM96848_) && !defined(_BCM947189_)
    if (image_sig_size ) {
        if (image != image_sig) {
            memcpy((void*)image, image_sig, image_sig_size);
	}
        image += image_sig_size;
    }
#endif
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (hash_block_start) {
        unsigned char hash[SHA256_S_DIGEST8];
        int ret;
        unsigned int content_len;
        // printf("look for hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
        // printf("start of hash block %x %x %x %x\n",hash_block_start[0],hash_block_start[1],hash_block_start[2],hash_block_start[3]);
        ret = find_boot_hash(&content_len, hash, hash_block_start, NAND_FLASH_BOOT_IMAGE_LZ); // FIXME -- LZ4 not supported
        if (ret == 0)  {
            printf("failed to find hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
            die();
        } else {
            printf("got hash for %s\n",NAND_FLASH_BOOT_IMAGE_LZ);
           if (sec_verify_sha256((uint8_t const*)image, image_size, (const uint8_t *)hash)) {
               printf("Kernel Digest failed\n");
               die();
           } else {
               printf("Kernel Digest OK\n");
           }

        }

    }
#endif // CONFIG_CFE_SUPPORT_HASH_BLOCK
    /* Suffice to mention that there's no way 
       to tell what endianness was used for the image header...  */
    memcpy(&image_hdr, (void*)image, sizeof(image_hdr));
    /* leagacy adjustments:
       new image format contains broadcom signature and uncompressed length.*/
    image_hdr_size = (image_hdr.magic==*(uint32_t*)brcmMagic)? sizeof(image_hdr) : sizeof(image_hdr)-sizeof(uint32_t);
    image += image_hdr_size;

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    /* ARM linux kernel compiled for virtual address at 0xc0008000, 
       convert to physical address for CFE */
    la.la_address =(long)(((uintptr_t)image_hdr.la)&0xfffffff);
    la.la_entrypt = (long)(((uintptr_t)image_hdr.entrypt)&0xfffffff);
#else
    la.la_address = (long)image_hdr.la;
    la.la_entrypt = (long)image_hdr.entrypt;
#endif

#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || \
              defined(_BCM963138_) || defined(_BCM963148_) || \
              defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM963158_) || defined (_BCM96856_)
    // only use this secure boot authentication if hash_block was not already checked
    if (!hash_block_start && (bcm_otp_is_boot_secure())) {
       /* Get the top of the cferam */
       if ( (CFE_RAM_ROM_PARMS_GET_SEQ_P1 & ~0xffff) == NAND_SEQ_MAGIC &&
            (CFE_RAM_ROM_PARMS_GET_SEQ_P2 & ~0xffff) == NAND_SEQ_MAGIC ) {
           /* cferom found and ran cferam... cferam was not somehow loaded by jtag
                 Security credentials should be available. */
          Booter1AuthArgs authArgs;
          CFE_RAM_ROM_PARMS_AUTH_PARM_GETM(&authArgs);
          /* Authenticate the vmlinux.lz image */
          printf("%s", "Authenticating vmlinux.lz ... "); 
          /* header size on vmlinux is 12 bytes on aarch32 and 20 bytes on aarch64 */	
          authenticate((uint8_t *)((uintptr_t)image - image_sig_size - image_hdr_size), 
                 image_hdr.len + image_sig_size + image_hdr_size,  authArgs.manu);
          printf("%s\n", "pass"); 
       } else {
        /* cferam has been loaded by jtag, but we are in full-secure mode. cferam cannot authenticate linux.*/
              /* while(1) loop is good enough because AES keys were cleaned up after cferam was decrypted. */ 
          printf("Image vmlinux.lz cannot be authenticated. Stoppping\n"); 
          while(1);
       }
    }
#endif

#ifdef USE_LZ4_DECOMPRESSOR
    if (image_hdr.len_uncomp) {           
        ret = LZ4_decompress_fast((const char *)image, (char *)(void*)la.la_address, image_hdr.len_uncomp) != image_hdr.len;
    } else
#endif
    {
        ret = decompressLZMA((unsigned char*)image, image_hdr.len, (unsigned char*)la.la_address, (RAMAPP_TEXT - la.la_address)) < 0;
    }

    if (ret) {
        printf("Failed to decompress %s image.  ret = %d Corrupted image?\n",image_hdr.len_uncomp? "LZ4":"LZMA",ret);
    } else {
            /* Save the rootfs offset of the rootfs that the Linux image
             * is loaded from at the memory location before the Linux load
             * address.  The Linux image uses this value to determine the
             * rootfs to use.
             */
        blparms_set_int(NAND_RFS_OFS_NAME, (start_blk * sector_size) / 1024);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
        /* Continue to save the rootfs offset the word before the 
         * destination (load) address for backwards compatibility.
        */
        *(unsigned int *) (la.la_address - 4) = (start_blk * sector_size) / 1024;
#endif
        /* Pass the age of the Linux image that is being booted to Linux.
         * BOOTED_OLD_IMAGE, BOOTED_NEW_IMAGE or BOOTED_ONLY_IMAGE
        */
        blparms_set_int(BOOTED_IMAGE_ID_NAME, bootImg);
        /* Pass board parameters to be loaded in Linux by function kerSysEarlyFlashInit
         * in file bcm63xx_flash.c, bypassing requirement for early driver support
         */
        blparms_set_str(BOARD_ID_NAME, NVRAM.szBoardId);
        blparms_set_str(VOICE_BOARD_ID_NAME, NVRAM.szVoiceBoardId);
        blparms_set_int(BOARD_STUFF_NAME, NVRAM.ulBoardStuffOption);

        printf("Decompression %s Image OK!\n",image_hdr.len_uncomp ? "LZ4":"LZMA");
        printf("Entry at 0x%p\n",la.la_entrypt);
        cfe_go(&la);  /* never return...  */
    }
    return( ret );
}

static int bootNandImage(void)
{
    int ret = -1;
    int bootImgA, bootImgB;
    char *msgA, *msgB;
    PFILE_TAG pTag1 = getTagFromPartition(1);
    PFILE_TAG pTag2 = getTagFromPartition(2);
    int seq1 = (pTag1) ? atoi(pTag1->imageSequence) : -1;
    int seq2 = (pTag2) ? atoi(pTag2->imageSequence) : -1;
    int start_blk, end_blk, rootfsA, rootfsB;
    int len = flash_get_sector_size(0) / 1024;
    int state = kerSysGetBootImageState();

    validateNandPartTbl(0, 0);

    if( pTag1 && pTag2 ) {
        /* Deal with wrap around case */
        if (seq1 == 0 && seq2 == 999)
            seq1 = 1000;
        if (seq2 == 0 && seq1 == 999)
            seq2 = 1000;

        if (state == BOOT_SET_NEW_IMAGE_ONCE)
        { // at this point this state can only exist for JFFS2 as the BOOT_SET_*_IMAGE_ONCE states have been cleared in CFEROM
            state = BOOT_SET_NEW_IMAGE;
            kerSysSetBootImageState(BOOT_SET_OLD_IMAGE);
        }
        else if (state == BOOT_SET_OLD_IMAGE_ONCE)
        {
            state = BOOT_SET_OLD_IMAGE;
            kerSysSetBootImageState(BOOT_SET_NEW_IMAGE);
        }
        else if (bootInfo.bootPartition == BOOT_SET_OLD_IMAGE)
        { // swap the booted image if we have the override set unless the state was JFFS2/UBI BOOT_SET_*_IMAGE_ONCE
            if (state == BOOT_SET_NEW_IMAGE)
                state = BOOT_SET_OLD_IMAGE;
            else
                state = BOOT_SET_NEW_IMAGE;
        }

        bootInfo.bootPartition = state; 

        if( state == BOOT_SET_NEW_IMAGE )
        {
            bootImgA = BOOTED_NEW_IMAGE;
            bootImgB = BOOTED_OLD_IMAGE;
            msgA = "Booting from latest image (address 0x%8.8lx, flash offset 0x%8.8lx) ...\n";
            msgB = "Booting from previous image (address 0x%8.8lx, flash offset 0x%8.8lx) ...\n";
            rootfsA = (seq2 > seq1) ? NP_ROOTFS_2 : NP_ROOTFS_1;
        }
        else /* Boot from the previous image. */
        {
            bootImgA = BOOTED_OLD_IMAGE;
            bootImgB = BOOTED_NEW_IMAGE;
            msgA = "Booting from previous image (address 0x%8.8lx, flash offset 0x%8.8lx) ...\n";
            msgB = "Booting from latest image (address 0x%8.8lx, flash offset 0x%8.8lx) ...\n";
            rootfsA = (seq2 <= seq1) ? NP_ROOTFS_2 : NP_ROOTFS_1;
        }

        rootfsB = (rootfsA == NP_ROOTFS_2) ? NP_ROOTFS_1 : NP_ROOTFS_2;
        start_blk = NVRAM.ulNandPartOfsKb[rootfsA] / len;
        end_blk = start_blk +
            (NVRAM.ulNandPartSizeKb[rootfsA] / len);
        printf(msgA, FLASH_BASE + (NVRAM.ulNandPartOfsKb[rootfsA] * 1024), NVRAM.ulNandPartOfsKb[rootfsA] * 1024);
        if((ret = bootNandImageFromRootfs(start_blk, end_blk, bootImgA)) != 0)
        {
            start_blk = NVRAM.ulNandPartOfsKb[rootfsB] / len;
            end_blk = start_blk +
                (NVRAM.ulNandPartSizeKb[rootfsB] / len);
            printf(msgB, FLASH_BASE + (NVRAM.ulNandPartOfsKb[rootfsB]*1024), NVRAM.ulNandPartOfsKb[rootfsB]*1024);
            bootInfo.bootPartition = (bootImgB == BOOTED_OLD_IMAGE) ? BOOT_SET_OLD_IMAGE : BOOT_SET_NEW_IMAGE; // set booted image partition so correct dtb is loaded via cfe_fs_fetch_file
            if((ret = bootNandImageFromRootfs(start_blk, end_blk,bootImgB))!= 0)
                printf("Unable to boot image.\n");
        }
    }
    else
    {
        if( pTag1 )
            rootfsA = NP_ROOTFS_1;
        else
            if( pTag2 )
                rootfsA = NP_ROOTFS_2;

        if( pTag1 || pTag2 )
        {
            start_blk = NVRAM.ulNandPartOfsKb[rootfsA] / len;
            end_blk = start_blk +
                (NVRAM.ulNandPartSizeKb[rootfsA] / len);
            printf("Booting from only image (address 0x%8.8lx, flash offset 0x%8.8lx) ...\n",
                    FLASH_BASE + (NVRAM.ulNandPartOfsKb[rootfsA] * 1024), NVRAM.ulNandPartOfsKb[rootfsA] * 1024);
            if( (ret = bootNandImageFromRootfs(start_blk, end_blk,
                BOOTED_ONLY_IMAGE)) != 0 )
            {
                printf("Unable to boot image.\n");
            }
        }
        else
            printf("No image found.\n");
    }
    return( ret );
}

#else

static int bootNandImage(void)
{
    return(-1);
}
#endif

static int boot_ramfs(char *host_ip,
                      char* img_id[id_img_max],
                      unsigned long rd_addr)
{
   int res  = -1;
   int name_sz = strlen(host_ip)+sizeof(":") + 
                       MAX3(strlen(img_id[id_img_boot]), strlen(img_id[id_img_ramdisk]), strlen(img_id[id_img_dtb])); 
   char *img_names = KMALLOC(name_sz*id_img_max,0);
   if (!img_names) {
        return res;  
   }
   sprintf((img_names+name_sz*id_img_boot),"%s:%s", host_ip, img_id[id_img_boot]);
   sprintf((img_names+name_sz*id_img_ramdisk),"%s:%s", host_ip, img_id[id_img_ramdisk]);
   sprintf((img_names+name_sz*id_img_dtb),"%s:%s", host_ip,img_id[id_img_dtb]);
   /* try to download and boot from the kernel elf and ramdisk image*/
   img_id[id_img_boot] = img_names + name_sz*id_img_boot;
   img_id[id_img_ramdisk] = img_names + name_sz*id_img_ramdisk;
   img_id[id_img_dtb] = img_names + name_sz*id_img_dtb;
   res = bootElfRamfsImage(img_id, rd_addr);
            /*In case of failure we drop here*/
   KFREE(img_names);
   return res;
}


static int auto_run(run_info_t* ar)
{
    int ret;

    if (g_invalid_img_rev) {
        printf("Cannot run --- Chip rev does not match image rev\n");
        return CFE_ERR_INV_COMMAND;
    }
#if ( defined (_BCM963138_) || defined (_BCM963148_) ) && (INC_SPI_PROG_NAND==1)
    if (ar->boot_opt == 'n') {
        /* Chain to NAND CFEROM*/
        printf("Launching NAND\n");
        cfe_sleep(CFE_HZ);
        NAND->NandCsNandXor = 1;
        cfe_launch(0xffe10000);
    }
#endif
    if (ar->boot_opt == 'f'/* && !ar->img_id[id_img_boot]*/) {
         if(( flash_get_flash_type() ==  FLASH_IFC_SPI )  || ( flash_get_flash_type() ==  FLASH_IFC_HS_SPI  )) {
             PFILE_TAG pTag = getBootImageTag();
             uintptr_t flash_addr_kernel = (uintptr_t)(atoi(pTag->kernelAddress) + BOOT_OFFSET);
             ret = bootCompressedImage((uint32_t *)flash_addr_kernel, TRUE);
         } else if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND )) {
             ret = bootNandImage();
         } 
#if INC_EMMC_FLASH_DRIVER
	   else if ( flash_get_flash_type() ==  FLASH_IFC_UNSUP_EMMC ) {
             ret = emmc_boot_os_image(0);
         }
#endif             
    } else { 
      /* loading from host  */
#if (NONETWORK==0)
        if (g_in_cfe == 0) {
            int i;
            board_netdevice_init();
            enet_init();
             /* trying to POLL devices */
            for (i = 0; i < 10; i++) {
                POLL();
                cfe_sleep(CFE_HZ/4);
            }
            g_in_cfe = 1;
        }
#endif
        if (ar->boot_opt == 'c') {
            ret = boot_ramfs(ar->ip, ar->img_id, ar->laddr);
		if (ret < 0) {
			printf("loading ramfs failed. Try to boot from flash.\n");
			bootNandImage();
		}
        } else if (ar->boot_opt =='h') {
            char img[BOOT_FILENAME_LEN + BOOT_IP_LEN];
            if (strchr(ar->img_id[id_img_boot], ':'))
                strcpy(img, ar->img_id[id_img_boot]);
            else {
                sprintf(img,"%s:%s",ar->ip, ar->img_id[id_img_boot]);  
            }
            /* try uncompressed image first */
            ret = bootImage("tftp", "eth0",  LOADFLG_EXECUTE | LOADFLG_NOISY, img);
            if( ret == CFE_ERR_NOTELF ) {
                uint8_t *ptr = (uint8_t *) cfe_get_mempool_ptr();
                // next try as a compressed image
                printf("Retry loading it as a compressed image.\n");
                if ((ret = loadRaw(img, ptr)) > 0)
                    bootCompressedImage((uint32_t *) ptr, TRUE); 
            }
        }
    }
    return ret;
}


// run program from compressed image in flash or from tftped program from host
static int ui_cmd_run_program(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int ret;
    char* arg = cmd_getarg(cmd, 0);
    run_info_t ar = {0};
    if (argc == 0) {
        ar.ip = bootInfo.hostIp;
        ar.boot_opt = bootInfo.runFrom;
        if (ar.boot_opt == 'c') {
            ar.img_id[id_img_boot] = bootInfo.hostFileName;
            ar.img_id[id_img_ramdisk] = bootInfo.ramfsFileName;
            ar.img_id[id_img_dtb] = bootInfo.dtbFileName;
            ar.laddr = bootInfo.rdAddr;
        } else if (ar.boot_opt == 'h')  {
            ar.img_id[id_img_boot] = bootInfo.hostFileName;
        }
    } else if (argc == 1) {
        ar.ip = bootInfo.hostIp;
        ar.img_id[id_img_boot] = arg;
        ar.boot_opt = 'h';
    } else if (argc == id_img_max+3 && strlen(arg) == 1 && *arg == 'n') {
        ar.ip = cmd_getarg(cmd, 1);
        ar.img_id[id_img_boot] = cmd_getarg(cmd, 2);
        ar.img_id[id_img_ramdisk] = cmd_getarg(cmd, 3);
        ar.img_id[id_img_dtb] = cmd_getarg(cmd, 4);
        ar.laddr = atoi(cmd_getarg(cmd, 5));
        ar.boot_opt = 'c';
    } else {
        printf("Invalid command option\n");
        return CFE_ERR_INV_COMMAND;
    }


    if( getBootImageTag() || ar.boot_opt == 'c' 
              || ar.boot_opt == 'h' ||  ar.boot_opt == 'n') {
        g_processing_cmd = 1;
        ret = auto_run(&ar);
    } else {
        printf("ERROR: There is not a valid image to boot from.\n");
        ret = CFE_ERR_FILENOTFOUND;
    }

    return ( ret );
}


static int ui_cmd_print_system_info(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return printSysInfo();
}

static int ui_cmd_change_bootline(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return changeBootLine();
}

static int ui_cmd_set_afe_id(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return changeAfeId();
}

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_)  || defined(_BCM96848_) ||\
    defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
static int ui_cmd_set_ddr_config(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return changeDDRConfig();
}
#endif

#if defined(_BCM94908_)|| defined(_BCM963158_)
static int ui_cmd_set_avs_config(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int PMCcmd(int[4]), pmc[4];
    static const char *disables[] = { "0","disable","false","off" };
    int i, on = 1;

    if (argc == 0)
        return getAVSConfig();
    else if (strcmp(argv[0], "show") == 0){
        int res;
        memset((void*)pmc, 0, sizeof(pmc));
        /* Try to know the AVS disable state */
        pmc[0] = cmdGetAvsDisableState;
        if (!(res = PMCcmd(pmc)))
            printf("AVS %s\n", pmc[2] ? "Disabled" : "Enabled");
        return res;
    }
    else {
        for (i = 0; i < sizeof disables / sizeof disables[0]; i++)
            if (strcmp(argv[0], disables[i]) == 0) {
                on = 0;
                break;
            }
        setAVSConfig(on);
    }
    return on;
}
#endif

void ui_dumphex( unsigned char *pAddr, int nLen, int sz );
void ui_dumphex( unsigned char *pAddr, int nLen, int sz )
{
    int a;
    unsigned char *j;
    unsigned char *crow;
    unsigned short *hrow;
    unsigned int wrow[4];
    int i;
#ifdef CONFIG_ARM64
    uint64_t *dwrow;
    dwrow = (uint64_t *)wrow;
#endif
    crow = (unsigned char *)wrow;
    hrow = (unsigned short *)wrow;
    a = 0;
    pAddr = (unsigned char *)((uintptr_t)pAddr & (~(sz-1)));
    do {
        if ((a & 15) == 0) {
            printf("%08p: ", pAddr +a);
        }
        if (a < nLen * sz) {
            switch (sz) {
            case 1:
                printf("%02x ",crow[a & 15] = *(unsigned char *)(pAddr+a));
                break;
            case 2:
                printf("%04x ",hrow[(a/2)&7] = *(unsigned short *)(pAddr+a));
                break;
            case 4:
                printf("%08x ",wrow[(a/4)&3] = *(unsigned int *)(pAddr+a));
                break;
#ifdef CONFIG_ARM64
            case 8:
                printf("%016llx ",dwrow[(a/8)&1] = *(uint64_t *)(pAddr+a));
                break;
#endif
            }
        } else {
            for (i = 0; i < sz*2 + 1 ; i++) {
                printf(" ");
            }
        }
        if (((a + sz) & 15) == 0) {
            for (i = 0, j = pAddr +(a & ~15) ; (j < pAddr + nLen*sz) && (j <= pAddr +a ) ; j++, i++) {
                printf("%c", (crow[i] >= ' ' && crow[i] <= '~') ? crow[i] : '.');
            }
            printf("\n");
        }
        a = a + sz;
    } while (a < ((nLen*sz+15) & ~15));
}

void ui_dumpaddr( unsigned char *pAddr, int nLen, int offset );
void ui_dumpaddr( unsigned char *pAddr, int nLen, int offset )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i = 0, j, size = 0;
    unsigned int ul;
    unsigned short us;
    unsigned int cmp;

#if !defined (_BCM96838_) && !defined (_BCM96848_)
	cmp = 0xff000000;
#else
	cmp = 0xf000000;
#endif
    if( ((uintptr_t) pAddr & 0xff000000) == 0xff000000 ||
        ((uintptr_t) pAddr & cmp) == 0xb0000000 )
    {
        if (nLen == 2) {
            pAddr = (unsigned char *) ((uintptr_t) pAddr & ~0x01);
        } else if (nLen != 1) {
            /* keeping the old logic as is. */
            if( ((unsigned long) pAddr & 0x03) != 0 )
                nLen += 4;
            pAddr = (unsigned char *) ((uintptr_t) pAddr & ~0x03);
        }
    }
    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (uintptr_t) (pAddr + offset));
        p = szLine + strlen(szLine);

        if( ((uintptr_t) pAddr & 0xff000000) == 0xff000000 ||
#if defined (_BCM963138_) || defined (_BCM963148_) || defined(_BCM94908_) || defined(_BCM96858_) || \
    defined (_BCM963158_) || defined (_BCM96846_) || defined (_BCM96856_)
            ((uintptr_t) pAddr & 0xf0000000) == 0x80000000 ||
#endif
            ((uintptr_t) pAddr & cmp) == 0xb0000000 )
        {
            for(i = 0; i < 16 && nLen > 0; i += sizeof(int), nLen -= sizeof(int))
            {
                if (nLen == 1) {
                    q = pAddr;
                    size = 1;
                } else if (nLen == 2) {
                    us = *(unsigned short *)pAddr;
                    q = (unsigned char *) &us;
                    size = 2;
                } else {
                    ul = *(unsigned int *) &pAddr[i];
                    q = (unsigned char *) &ul;
                    size = sizeof(int);
                }
                for( j = 0; j < size; j++ )
                {
                    *p++ = szHexChars[q[j] >> 4];
                    *p++ = szHexChars[q[j] & 0x0f];
                }
                *p++ = ' ';
            }
        }
        else
        {
            for(i = 0; i < 16 && nLen > 0; i++, nLen-- )
            {
                ch =  pAddr[i];

                *p++ = szHexChars[ch >> 4];
                *p++ = szHexChars[ch & 0x0f];
                if( (i & 0x03) == 3 )
                    *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pAddr[j];
            *p++ = (ch >= ' ' && ch <= '~') ? ch : '.';
        }

        *p++ = '\0';
        printf( "%s\r\n", szLine );

        pAddr += i;
    }
    printf( "\r\n" );
} /* ui_dumpaddr */

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static int ui_cmd_dump_mem(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszLen = cmd_getarg(cmd, 1);
    if( pszAddr && pszLen )
        ui_dumpaddr((unsigned char *) xtoul(pszAddr), atoi(pszLen), 0);
    else
        printf("dm address_in_hex length_in_decimal\n");

    return( 0 );
}
#endif


#ifdef CONFIG_ARM64
static int ui_cmd_dump_dwords(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszLen = cmd_getarg(cmd, 1);

    if( pszAddr && pszLen )
        ui_dumphex((unsigned char*)xtoul(pszAddr), atoi(pszLen),8);
    else
        printf("ddw address_in_hex length_in_decimal\n");

    return( 0 );
}
#endif

static int ui_cmd_dump_words(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszLen = cmd_getarg(cmd, 1);

    if( pszAddr && pszLen )
      ui_dumphex((unsigned char*)xtoul(pszAddr), atoi(pszLen),4);
    else
        printf("dw address_in_hex length_in_decimal\n");

    return( 0 );
}

static int ui_cmd_dump_halfwords(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszLen = cmd_getarg(cmd, 1);
    if( pszAddr && pszLen )
        ui_dumphex((unsigned char *)xtoul(pszAddr), atoi(pszLen),2);
    else
        printf("dh address_in_hex length_in_decimal\n");

    return( 0 );
}

static int ui_cmd_dump_bytes(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszLen = cmd_getarg(cmd, 1);
    if( pszAddr && pszLen )
        ui_dumphex((unsigned char *)xtoul(pszAddr), atoi(pszLen),1);
    else
        printf("dw address_in_hex length_in_decimal\n");

    return( 0 );
}

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)

extern int read_spare_data(int blk, int offset, unsigned char *buf, int bufsize, int read_all_oob);

int dump_nand(unsigned int addr, unsigned int end);
int dump_nand(unsigned int addr, unsigned int end)
{
    int blkSize = flash_get_sector_size(0);
    int pageSize = flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL);
    int spareSize = flash_dev_specific_cmd(GET_SPARE_SIZE, NULL, NULL);
    int status;
    unsigned char *buf =  KMALLOC(blkSize + 2048, sizeof(void*));
    #define BLOCK addr/blkSize
    #define PAGEADDR (addr%blkSize)
    #define PAGE PAGEADDR/pageSize
    if (!buf) {
        return -1;
    }
    for (; addr < end; addr += pageSize)
    {
        printf("------------------ block: %d, page: %d ------------------\n", BLOCK, PAGE);
        if( flash_read_buf(BLOCK, PAGEADDR, buf, pageSize) < 0 )
            printf("Error reading block %d\n", addr/blkSize);
        {
	    ui_dumpaddr(buf, pageSize, (int)(addr - (uintptr_t)buf));

#if (INC_SPI_NAND_DRIVER==1)
            if (flash_get_flash_type() == FLASH_IFC_SPINAND)
                status = spi_nand_page_read(addr, pageSize, buf, spareSize);
#endif

            if (flash_get_flash_type() == FLASH_IFC_NAND)
                status = read_spare_data(BLOCK, PAGEADDR, buf, spareSize, 1);

            {
                printf("----------- spare area for block %d, page %d -----------\n", BLOCK, PAGE);
                ui_dumpaddr(buf, spareSize, (int)(pageSize - (uintptr_t) buf));
            }

            if(status != FLASH_API_OK)
                printf("Error reading block %d\n", BLOCK);

        }
    }

    KFREE(buf);
    return(status);
}

static int ui_cmd_dump_nand(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszFirst = cmd_getarg(cmd, 0);
    char *pszSecond = cmd_getarg(cmd, 1);
    char *pszThird = cmd_getarg(cmd, 2);
    int blkSize = flash_get_sector_size(0);
    int pageSize = flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL);
    static int addr = 0;
    int end, status;

    if (argc == 3)
    {
        addr = (atoi(pszFirst) * blkSize) + (atoi(pszSecond) * pageSize);
        end = addr + (atoi(pszThird) * pageSize);
    }
    else if (argc == 2)
    {
        addr = atoi(pszFirst) & (~(pageSize - 1));
        end = addr + (atoi(pszSecond) * pageSize);
    }
    else
        end = addr + pageSize;

    status = dump_nand(addr, end);
    addr = end;
    return(status);
}


static int ui_cmd_compare_blocks(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszFirst = cmd_getarg(cmd, 0);
    char *pszSecond = cmd_getarg(cmd, 1);
    char *pszThird = cmd_getarg(cmd, 2);
    int blkSize = flash_get_sector_size(0);
    int pageSize = flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL);
    int spareSize = flash_dev_specific_cmd(GET_SPARE_SIZE, NULL, NULL);
    int first, second, amount, block, page, status;
    unsigned char **buf = (unsigned char**)(void*)KMALLOC(ALIGN(sizeof(void*),(pageSize+spareSize))*sizeof(unsigned char)*2,sizeof(void*));
    if (!buf) {
        return 0;  
    } 
    #define PAGES blkSize/pageSize
    #define ADDR1 (block*blkSize)+(page*pageSize)
    #define ADDR2 ((block+OFFSET)*blkSize)+(page*pageSize)
    #define PADDR (page*pageSize)
    #define OFFSET (second>first?second-first:first-second)

    if (3 != argc)
    {
        printf("wrong number of arguments, need to provide source block number, destination block number, number of blocks to compare\n");
        KFREE(buf);
        return(0);
    }

    first  = atoi(pszFirst);
    second = atoi(pszSecond);
    amount = atoi(pszThird);

    for (block = first; block < first + amount; block++)
    {
        for (page = 0; page < PAGES; page++)
        {
            printf("checking page 0x%x of block 0x%x against block 0x%x\n", page, block, block + OFFSET);

            if( flash_read_buf(block, PADDR, buf[0], pageSize) < 0 )
                printf("Error reading block %d\n", block);

            if( flash_read_buf(block + OFFSET, PADDR, buf[1], pageSize) < 0 )
                printf("Error reading block %d\n", block + OFFSET);

#if (INC_SPI_NAND_DRIVER==1)
            if (flash_get_flash_type() == FLASH_IFC_SPINAND)
            {
                status = spi_nand_page_read(ADDR1, pageSize, buf[0] + pageSize, spareSize);
                status = spi_nand_page_read(ADDR2, pageSize, buf[1] + pageSize, spareSize);
            }
#endif

            if (flash_get_flash_type() == FLASH_IFC_NAND)
            {
                status = read_spare_data(block,          PADDR, buf[0] + pageSize, spareSize, 1);
                status = read_spare_data(block + OFFSET, PADDR, buf[1] + pageSize, spareSize, 1);
            }            

            if (memcmp(buf[0], buf[1], pageSize + spareSize))
            {
                printf("------------------ block: %d, page: %d ------------------\n", block, page);
                ui_dumpaddr(buf[0], pageSize, (int)(ADDR1 - (uintptr_t) buf[0]));

                printf("----------- spare area for block %d, page %d -----------\n", block, page);
                ui_dumpaddr(buf[0] + pageSize, spareSize, (int)(pageSize - ((uintptr_t)buf[0] + pageSize)));

                printf("------------------ block: %d, page: %d ------------------\n", block + OFFSET, page);
                ui_dumpaddr(buf[1], pageSize, (int)(ADDR2 - (uintptr_t) buf[1]));

                printf("----------- spare area for block %d, page %d -----------\n", block + OFFSET, page);
                ui_dumpaddr(buf[1] + pageSize, spareSize, (int)(pageSize - ((uintptr_t)buf[1] + pageSize)));
            }

            if(status != FLASH_API_OK)
                printf("Error reading block %d\n", block);
        }
    }
    KFREE(buf);

    return(status);
}

static int ui_cmd_find_string(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszFirst = cmd_getarg(cmd, 0);
    char *pszSecond = cmd_getarg(cmd, 1);
    char *pszThird = cmd_getarg(cmd, 2);
    int blkSize = flash_get_sector_size(0);
    int pageSize = flash_dev_specific_cmd(GET_PAGE_SIZE, NULL, NULL);
    int first, last, len, block, i;
    char *buf = KMALLOC(blkSize, sizeof(void*));
    if (!buf) {
	    return -1;
    }
    if (3 != argc)
    {
        printf("wrong number of arguments, start block number, end block number to search, string to search for\n");
        return(0);
    }

    first = atoi(pszFirst);
    last = atoi(pszSecond);

    len = strlen(pszThird);

    for (block = first; block <= last; block++)
    {
        printf("checking block 0x%x\n", block);

        if( flash_read_buf(block, 0, (unsigned char *)buf, blkSize) < 0 )
            printf("Error reading block %d\n", block);

        for (i = 0; i <= (blkSize - len); i++)
        {
            if (!(strncmp(pszThird, &buf[i], len)))
                printf("found match at block 0x%x, page 0x%x, offset 0x%x\n", block, i/pageSize, i%pageSize);
        }
    }
    KFREE(buf);
    return(FLASH_API_OK);
}


#define MAX_READ_VALUES 256

static int ui_cmd_nand_cmd(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char * command;
    int item = 0, read = 0;
    uint16_t value;
    char data[MAX_READ_VALUES];

    if (!argc)
    {
        printf("Generic NAND command, a=address then address, c=command then command, w=write data then values to write, r=read data then # bytes to read\n"
               "ex. to get feature drive strength: nc c 0xEE a 0x80 r 4\n"
               "    to set feature drive strength: nc c 0xEF a 0x80 w 1 0 0 0\n"
               "    to read device ID:             nc c 0x90 a 0 r 4\n"
               "    to read parameter page:        nc c 0xEC a 0 r 8\n");
        return(0);
    }

    while(argc--)
    {
        if ( (argv[item][0] == 'c') || (argv[item][0] == 'a') || (argv[item][0] == 'r') || (argv[item][0] == 'w') ||
             (argv[item][0] == 'C') || (argv[item][0] == 'A') || (argv[item][0] == 'R') || (argv[item][0] == 'W') )
            command = argv[item];
        else
        {
            value = atoi(argv[item]);

            if (command[0] == 'r')
            {
                if (value > MAX_READ_VALUES)
                    value = MAX_READ_VALUES;

                while(value--)
                    data[read++] = nandCommand(command[0], value);
            }
            else
                nandCommand(command[0], value);
        }

        item++;
    }

    nandCommand('i', 0); // idle the NAND/controller

    item = 0;
    while(read--)
        printf("read 0x%x\n", data[item++]);

    return(0);
}


static int ui_cmd_find_bad_blocks(ui_cmdline_t *cmd, int argc, char *argv[])
{
    unsigned int i;
    int blocks = flash_get_numsectors();

    printf(">>>>>>>> FIND BAD BLOCKS from block 0 to block 0x%x <<<<<<<<\n", blocks - 1);

    for (i = 0; i < blocks; i++)
    {
        if (flash_dev_specific_cmd(CHECK_BAD_BLOCK, &i, NULL))
            printf("Block 0x%x is marked bad\n", i);
    }
    return(0);
}

static int ui_cmd_mark_block_bad(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *pszBlock = cmd_getarg(cmd, 0);
    unsigned int block;

    if (NULL == pszBlock)
    {
        printf("wrong number of arguments, need to provide block number to mark bad\n");
        return(0);
    }

    block = atoi(pszBlock);

    printf("\nNow think carefully.  Do you really,\n"
        "really want to mark block 0x%x bad?", block);
    if (yesno())
        return 0;

    printf("Marking block 0x%x bad\n", block);

    flash_dev_specific_cmd(MARK_BLOCK_BAD, &block, NULL);

    return(0);
}


static int ui_cmd_erase_nand_blk(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *pszStart = cmd_getarg(cmd, 0);
    char *pszEnd = cmd_getarg(cmd, 1);
    unsigned int start, end, i;

    if (pszStart == NULL)
    {
        printf("wrong number of arguments, need to provide block start and optionally block end to erase\n");
        return(0);
    }

    start = atoi(pszStart);

    if (pszEnd != NULL)
        end = atoi(pszEnd);
    else // if end not specified, user hopefully just wants to erase a single block
        end = start;

    printf("Forceably Erase FLASH block? (which includes clearing the bad block marker)");
    if (yesno())
        return 0;
    printf("\nNow think carefully.  Do you really,\n"
            "really want to erase FLASH blocks 0x%x to 0x%x?", start, end);
    if (yesno())
        return 0;

    for (i = start; i <= end; i++)
    {
        printf(".");
        flash_dev_specific_cmd(FORCE_ERASE, &i, NULL);
    }
    printf("\n");

    return(0);
}


static int ui_cmd_write_data(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *pszBlock = cmd_getarg(cmd, 0);
    char *pszPage = cmd_getarg(cmd, 1);
    char *pszOffset = cmd_getarg(cmd, 2);
    char *pszData = cmd_getarg(cmd, 3);
    unsigned char data;
    flash_write_data_t fwd;

    if ((NULL == pszBlock) || (NULL == pszPage) || (NULL == pszOffset) || (NULL == pszData))
    {
        printf("wrong number of arguments, need to provide block, page, offset and data to change\n");
        return(0);
    }

    fwd.block = atoi(pszBlock);
    fwd.page = atoi(pszPage);
    fwd.offset = atoi(pszOffset);
    fwd.amount = 1;
    data = atoi(pszData);
    fwd.data = &data;

    printf("\nNow think carefully.  Do you really,\n"
        "really want to write block 0x%x, page 0x%x, offset 0x%x with data 0x%x?", fwd.block, fwd.page, fwd.offset, *fwd.data);
    if (yesno())
        return 0;

    printf("Writing block 0x%x, page 0x%x, offset 0x%x with data 0x%x\n", fwd.block, fwd.page, fwd.offset, *fwd.data);

    flash_dev_specific_cmd(WRITE_WITHOUT_ECC, &fwd, NULL);

    return(0);
}


#if (INC_SPI_NAND_DRIVER==1)
static int ui_cmd_force_spi_nand(ui_cmdline_t *cmd, int argc, char *argv[])
{
    printf("Forcing SPI NAND read/write\n");

    flash_init(); // do flash init again, but with g_force_mode on will now default to SPI NAND

    return(0);
}


static int ui_cmd_get_feature(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *pszAddress = cmd_getarg(cmd, 0);
    unsigned char address;

    if (1 != argc)
    {
        printf("Get feature wrong number of arguments, gf [byte address]\n");
        return(0);
    }

    address = atoi(pszAddress);

    printf("Get feature address 0x%x = 0x%x\n", address, flash_dev_specific_cmd(GET_FEATURE, &address, NULL));

    return(0);
}


static int ui_cmd_set_feature(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *pszAddress = cmd_getarg(cmd, 0);
    char *pszData = cmd_getarg(cmd, 1);
    unsigned char address, data;

    if (2 != argc)
    {
        printf("Set feature wrong number of arguments, sf [byte address] [byte data to write]\n");
        return(0);
    }

    address = atoi(pszAddress);
    data = atoi(pszData);

    flash_dev_specific_cmd(SET_FEATURE, &address, &data);

    printf("Set feature address 0x%x (0x%x)\n", address, data);

    return(0);
}


#endif /* INC_SPI_NAND_DRIVER==1 */

static int ui_cmd_enable_raw_flash_write(ui_cmdline_t *cmd, int argc, char *argv[])
{
    g_raw_flash_write=1;
    return(0);
}

#endif /* INC_NAND_FLASH_DRIVER */

#if defined (_BCM963138_)
int cpu_limited=0;
static int ui_cmd_set_cpu_limit(ui_cmdline_t *cmd, int argc, char *argv[])
{
	cpu_limited=1;
	return (0);
}
#endif

static int ui_cmd_set_kernp(ui_cmdline_t *cmd, int argc, char *argv[])
{ char *batch_parm = NULL;
    int res = -1; 
    if (argc > 0) {
       int i, size = 0;
       for (i = 0; i < argc; i++) {
          char* s = cmd_getarg(cmd, i);
          /*accounting for space, string length and  null terminator*/
          int ssz = strlen(s)*sizeof(char)+sizeof(char);
          batch_parm = (char*)KREALLOC(batch_parm, size + ssz +sizeof(char), 0);
          if (!batch_parm) {
              goto err_out;
          }
          *(batch_parm+size) = ' ';
          memcpy(batch_parm+size+sizeof(char), s, ssz-sizeof(char));
          size += ssz;
       }
       *(batch_parm+size) = '\0'; 
    }
    if (blparms_add_extra_parms(BLKERNPARM, '=', ' ', '"', '/', batch_parm)) {
       printf("ERROR: Invalid paramter. Input <lval> = <rval> pairs.\n");
        
       goto err_out;
    }
     
    res = 0;
err_out:
    if (batch_parm) {
        KFREE(batch_parm);
    }
    return res;
}

#if defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM96858_)
/* SWREG*/

#if defined(_BCM96846_) || defined(_BCM96856_)
#define SWR_CTRL 		( (volatile unsigned int*) 0xffb20060)
#define SWR_WR   		( (volatile unsigned int*) 0xffb20064)
#define SWR_RD   		( (volatile unsigned int*) 0xffb20068)
#define PMC_GPIN		( (volatile unsigned int*) 0xffb01140)
#define SWR_FIRST 0
#define SWR_LAST 4
#elif defined(_BCM96858_)
#define SWR_CTRL 		( (volatile unsigned int*) 0x80280060)
#define SWR_WR   		( (volatile unsigned int*) 0x80280064)
#define SWR_RD   		( (volatile unsigned int*) 0x80280068)
#define SWR_FIRST 1
#define SWR_LAST 3
#endif

#define SWR_READ_CMD_P 0xB800
#define SWR_WR_CMD_P   0xB400
#define SWR_EN         0x1000
#define SET_ADDR(ps, reg)  (((ps) << 5 | ((reg) & 0x1f)) & 0x2ff)  

static int swrp(ui_cmdline_t *cmd,int argc,char *argv[])
{
#if !defined(_BCM96858_)
	printf("0 - 1.0D\n");
#endif
	printf("1 - 1.8\n");
	printf("2 - 1.5\n");
#if !defined(_BCM96858_)
	printf("3 - 1.0A\n");
#endif
	return 0;
}

#define TEST(x)  {\
					int num;\
					for(num=1000;(((*SWR_CTRL) & 0x8000) && (num > 0)) ; num--) ;\
						if(!num) \
						{\
							printf("Error num %d timeout num = %d!!!", (x),  num);\
							goto error;\
						}\
				}

static void  swr_read( unsigned int ps, unsigned int reg)
{
		unsigned int cmd = SWR_READ_CMD_P | SET_ADDR( ps, reg);	
		
		*SWR_CTRL = SWR_EN;
		*SWR_CTRL = cmd;
		TEST(22);
		printf("%s, reg=0x%02x, val=0x%04x\n", 
				(ps == 0 ? "1.0D" : (ps == 1 ? "1.8 " :(ps == 2 ? "1.5 " : "1.0A"))),
				reg,
				*SWR_RD);
		return;
error:	printf("power_supply_in_hex, reg_in_hex");
}	
				
static int swrr(ui_cmdline_t *cmd, int argc, char *argv[])
{
	char *pss   = cmd_getarg(cmd, 0);
    char *pregs = cmd_getarg(cmd, 1);
	
	if(pss && pregs)
	{
		unsigned int ps  = (unsigned int) atoi(pss);
	    unsigned int reg = (unsigned int) atoi(pregs);
		if(reg > 9 || ps > 3)
		{
			printf("reg %d, ps %d\n", reg, ps);
			goto error;
		}
		swr_read(ps, reg);
	}
	else
error:		printf("power_supply_in_hex, reg_in_hex\n");
	
	return 0;
}    

	
static int swrw(ui_cmdline_t *cmd, int argc, char *argv[])
{
	char *pss   = cmd_getarg(cmd, 0);
    char *pregs = cmd_getarg(cmd, 1);
	char *vals  = cmd_getarg(cmd, 2);
	
	if(pss && pregs && vals )
	{
		unsigned int ps   = (unsigned int) atoi(pss);
	    unsigned int reg  = (unsigned int) atoi(pregs);
	    unsigned int val  = (unsigned int) atoi(vals);
		unsigned int cmd  = 0; 
		unsigned int cmd1 = 0;  
        unsigned int reg0 = 0;
		
		if(reg > 9 || ps > 3)
		{
			printf("reg %d, ps %d\n", reg,ps);
			goto error;
		}

#if !defined(_BCM96858_)
		printf("PMC GPIN 0x%x\n", *PMC_GPIN);
#endif
		
		*SWR_CTRL = SWR_EN;
		if(reg == 0)
		{
			/* no need read reg0 in case that we write to it , we know walv :)*/
			reg0 = val;
		}
		else
		{
			/* read reg0*/
			cmd1  = SWR_READ_CMD_P | SET_ADDR( ps, 0);
			*SWR_CTRL = cmd1;
			TEST(1)
			reg0 = *SWR_RD;
		}
		/* write reg */
		*SWR_WR = val;
		cmd  = SWR_WR_CMD_P | SET_ADDR( ps, reg);
		*SWR_CTRL = cmd;
		TEST(2);
		/*toggele bit 1 reg0 this load the new regs value */
		cmd1  = SWR_WR_CMD_P | SET_ADDR( ps, 0);
		*SWR_WR = reg0 & ~0x2;
		*SWR_CTRL = cmd1;
		TEST(3);
		*SWR_WR = reg0 | 0x2;
		*SWR_CTRL = cmd1;
		TEST(4);
		*SWR_WR = reg0 & ~0x2;
		*SWR_CTRL = cmd1;
		TEST(5);
		
		printf("write done, read back....\n");
		swr_read(ps, reg);
	}
	else
error:		printf("power_supply_in_hex, reg_in_hex, val_in_hex\n");
	
	return 0;

}    

static int swrd(ui_cmdline_t *cmd, int argc, char *argv[])
{
	char *pss   = cmd_getarg(cmd, 0);
	if(pss )
	{
		int i,j;
		unsigned int ps   = (unsigned int) atoi(pss);
		if(ps < 4 )
		{
			i= ps;
			ps++;
		}
		else
		{
			i= SWR_FIRST;
			ps = SWR_LAST;
		}
#if !defined(_BCM96858_)
		printf("PMC GPIN 0x%x\n", *PMC_GPIN);
#endif
		for( ; i < ps ; i++)
			for(j=0; j <10; j++)
				 swr_read(i,j);
	}
	else
		printf("power_supply_in_hex or 4_for_all\n");
	
	return 0;
}
#endif

static int ui_cmd_set_mem(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszValue = cmd_getarg(cmd, 1);
    char *pszSize = cmd_getarg(cmd, 2);
    if( pszAddr && pszValue && pszSize )
    {
        unsigned long ulAddr = xtoul(pszAddr);
        unsigned int ulValue = (unsigned int) atoi(pszValue);
        int nSize = atoi(pszSize);
#ifdef CONFIG_ARM64
        uint64_t *puint64 = (uint64_t*) ulAddr;
#endif
        unsigned int *pul  = (unsigned int *)  ulAddr;
        unsigned short *pus = (unsigned short *) ulAddr;
        unsigned char *puc  = (unsigned char *)  ulAddr;
        switch( nSize )
        {
#ifdef CONFIG_ARM64
        case 8:
            *puint64 = (uint64_t)lib_xtoul(pszValue);
            break;
#endif

        case 4:
            *pul = (unsigned int) ulValue;
            break;

        case 2:
            *pus = (unsigned short) ulValue;
            break;

        case 1:
            *puc = (unsigned char) ulValue;
            break;

        default:
            printf("sm address_in_hex value_in_hex size_4_2_or_1");
            break;
        }

        ui_dumphex((unsigned char *) ulAddr, 1, nSize);
    }
    else
        printf("sm address_in_hex value_in_hex size_4_2_or_1");

    return( 0 );
}

#if (NONETWORK==0)
static int ui_cmd_phy(ui_cmdline_t *cmd,int argc,char *argv[])
{
#if !defined (_BCM96838_) && !defined (_BCM960333_) && !defined (_BCM96848_) && \
    !defined (_BCM947189_) && !defined (_BCM96846_) && !defined (_BCM96856_)
    if (argc < 2) {
        printf("phy address_in_hex reg_in_hex value_in_hex");
        return 0;
    }

    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszReg = cmd_getarg(cmd, 1);

    if( pszAddr )
    {
        unsigned short usValue;
        unsigned long ulAddr = (unsigned long) xtoi(pszAddr);
        unsigned short usReg = (unsigned short) atoi(pszReg);

        /* specify address_in_hex = phy id|PHY_INTEGRATED_VALID|PHY_EXTERNAL for external phy id
         * that is less than 0x10 in the cmd line
         */
        if (argv[2]) {
            char *pszValue = cmd_getarg(cmd, 2);
            usValue = (unsigned short) atoi(pszValue);
            mii_write(ulAddr, usReg & 0x1F, usValue);
        } else {
            printf("Phy Reg 0x%02x = 0x%04x \n", usReg & 0x1F, mii_read(ulAddr, usReg & 0x1F));
        }
    }
    else
        printf("phy address_in_hex reg_in_hex value_in_hex");
#elif defined (_BCM96838_) || defined (_BCM96848_)
    if (argc < 2) {
          printf("phy address_in_hex reg_in_hex value_in_hex");
          return 0;
      }

      char *pszAddr = cmd_getarg(cmd, 0);
      char *pszReg = cmd_getarg(cmd, 1);

      if( pszAddr )
      {
          unsigned short usValue;
          unsigned long ulAddr = (unsigned long) xtoi(pszAddr);
          unsigned short usReg = (unsigned short) atoi(pszReg);

          /* specify address_in_hex = phy id|PHY_INTEGRATED_VALID|PHY_EXTERNAL for external phy id
           * that is less than 0x10 in the cmd line
           */
          if (argv[2]) {
              char *pszValue = cmd_getarg(cmd, 2);
              usValue = (unsigned short) atoi(pszValue);
              if ((ulAddr >0) && (ulAddr < 8))
              {
                  phy_write_register(ulAddr, usReg, usValue);
                  printf("Write Phy Reg 0x%02x = 0x%04x \n", usReg, phy_read_register(ulAddr, usReg));
              }
              else
              {
                  printf("Illegal Phy number\n");
              }
          } else {
              if ((ulAddr >0) && (ulAddr < 8))
                  printf("Read Phy Reg 0x%02x = 0x%04x \n", usReg, phy_read_register(ulAddr, usReg));
              else
              {
                  printf("Illegal Phy number\n");
              }
          }
      }
      else
          printf("phy address_in_hex reg_in_hex value_in_hex");
#elif defined (_BCM96858_)
      char *pszPhyId = cmd_getarg(cmd, 0);
      char *pszReg = cmd_getarg(cmd, 1);
      char *pszVal = cmd_getarg(cmd, 2);
      int phyid;
      uint16_t reg, val;

      if (!pszPhyId || !pszReg)
          goto WrongParams;

      phyid = atoi(pszPhyId);
      if (phyid < 0 || phyid > LPORT_MAX_PHY_ID)
          goto WrongParams;

      reg = xtoi(pszReg);
      if (pszVal)
      {
          val = xtoi(pszVal);
          if (lport_mdio22_wr(phyid, reg, val))
          {
              printf("Failed to write register 0x%x\n", reg);
              return -1;
          }
      }
      if (lport_mdio22_rd(phyid, reg, &val))
      {
          printf("Failed to read register 0x%x\n", reg);
          return -1;
      }
      printf("%s Phy Reg 0x%02x = 0x%04x\n", pszVal ? "Write" : "Read", reg, val);
      return 0;

WrongParams:
      printf("%s\n", lport_phy_usage);
      return -1;
#elif defined (_BCM96846_) || defined (_BCM96856_)
      char *pszPhyId = cmd_getarg(cmd, 0);
      char *pszReg = cmd_getarg(cmd, 1);
      char *pszVal = cmd_getarg(cmd, 2);
      int phyid;
      uint16_t reg, val;

      if (!pszPhyId || !pszReg)
          goto WrongParams;

      phyid = atoi(pszPhyId);

      reg = xtoi(pszReg);
      if (pszVal)
      {
          val = xtoi(pszVal);
          if (mdio_write_c22_register(MDIO_INT, phyid, reg, val))
          {
              printf("Failed to write register 0x%x\n", reg);
              return -1;
          }
      }
      if (mdio_read_c22_register(MDIO_INT, phyid, reg, &val))
      {
          printf("Failed to read register 0x%x\n", reg);
          return -1;
      }
      printf("%s Phy Reg 0x%02x = 0x%04x\n", pszVal ? "Write" : "Read", reg, val);
      return 0;
WrongParams:
      printf("%s\n", "Wrong params");
      return -1;
#endif
    return( 0 );
}
#endif /* (NONETWORK==0) */

static int ui_cmd_set_force_mode(ui_cmdline_t *cmd,int argc,char *argv[]) {

    if (g_force_mode == 0)
    {
        brcm_cmd_addcmd("memsize",
           ui_cmd_set_memsize,
           NULL,
           "override memsize",
           "e.g. memsize <size in MB>",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)
        brcm_cmd_addcmd("ef",
                   ui_cmd_erase_nand_blk,
                   NULL,
                   "Erase NAND contents along with spare area",
                   "eg. ef [block start] [block end]",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("mb",
                   ui_cmd_mark_block_bad,
                   NULL,
                   "Mark NAND block bad",
                   "eg. mb [block]",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("wd",
                   ui_cmd_write_data,
                   NULL,
                   "Write data to SPI NAND",
                   "eg. wd [block] [page] [offset]",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("raw_flash_write",
                   ui_cmd_enable_raw_flash_write,
                   NULL,
                   "Enable raw flash image write",
                   "",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("nc",
                   ui_cmd_nand_cmd,
                   NULL,
                   "Generic NAND command, a=address, c=command, w=write data, r=read data",
                   "eg. to get feature drive strength: nc c 0xEE a 0x80 r 4, to set feature drive strength: nc c 0xEF a 0x80 w 1 0 0 0",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

#if (INC_SPI_NAND_DRIVER==1)
        brcm_cmd_addcmd("sn",
                   ui_cmd_force_spi_nand,
                   NULL,
                   "Force read/writes to SPI NAND",
                   "eg. sn",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("gf",
                   ui_cmd_get_feature,
                   NULL,
                   "SPI NAND get feature",
                   "eg. gf address",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

        brcm_cmd_addcmd("sf",
                   ui_cmd_set_feature,
                   NULL,
                   "SPI NAND set feature",
                   "eg. sf address data",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);

#endif
#endif
#if defined (_BCM963138_)
        brcm_cmd_addcmd("cpu_limit",
                   ui_cmd_set_cpu_limit,
                   NULL,
                   "set cpu limit",
                   "",
                   "",
                   BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

        g_force_mode = 1;
    }

    return( 0 );
}

static int ui_cmd_spi_access(ui_cmdline_t *cmd,int argc,char *argv[])
{
#if defined (_BCM963268_) || defined (_BCM963381_) 
    uint8 data[8];
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszSize = cmd_getarg(cmd, 1);
    char *pszValue = cmd_getarg(cmd, 2);

    if( pszAddr && pszSize )
    {
        unsigned long ulAddr = (unsigned long) xtoi(pszAddr);
        int i = 0, nSize = atoi(pszSize);

        if (nSize > 8) {
            printf("Size should be less than 8\n");
            return -1;
        }

        if (pszValue) {
            for (i = 0; i < nSize; i++) {
                data[i] = *(pszValue+i);
                printf("data[i] = %02x\n",data[i]);
            }
            ethsw_wreg_ext(SPI_BUS, (ulAddr >> 8) & 0xFF, ulAddr & 0xFF, data, nSize);
        } else {
            ethsw_rreg_ext(SPI_BUS, (ulAddr >> 8) & 0xFF, ulAddr & 0xFF, data, nSize);
            printf("Data: 0x");
            for (i = 0; i < nSize; i++)
                printf("%02x", data[i]);
            printf("\n");
        }
    }
    else
        printf("spi address_in_hex size_4_2_or_1 [value_in_hex]");
#endif
    return( 0 );
}

static int ui_cmd_pmdio_access(ui_cmdline_t *cmd,int argc,char *argv[])
{
#if defined (_BCM963268_) || defined (_BCM963381_) 
    uint8 data[8];
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszSize = cmd_getarg(cmd, 1);
    char *pszValue = cmd_getarg(cmd, 2);

    if( pszAddr && pszSize )
    {
        unsigned long ulAddr = (unsigned long) xtoi(pszAddr);
        int i = 0, nSize = atoi(pszSize);

        if (nSize > 8) {
            printf("Size should be less than 8\n");
            return -1;
        }

        if (pszValue) {
            ethsw_wreg_ext(MDIO_BUS, (ulAddr >> 8) & 0xFF, ulAddr & 0xFF, (uint8 *)pszValue, nSize);
        } else {
            ethsw_rreg_ext(MDIO_BUS, (ulAddr >> 8) & 0xFF, ulAddr & 0xFF, data, nSize);
            printf("Data: 0x");
            for (i = 0; i < nSize; i++)
                printf("%02x", data[i]);
            printf("\n");
        }
    }
    else
        printf("pmdio address_in_hex size_4_2_or_1 [value_in_hex]");
#endif
    return( 0 );
}

#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_) || defined (_BCM963158_)

static int  ui_cmd_set_cpufreq(ui_cmdline_t *cmd,int argc,char *argv[]) {

    int freq = 0;
    char *pszFreq = cmd_getarg(cmd, 0);
    int res;

    if( pszFreq )
        freq = atoi(pszFreq);

    if (freq)
        res = cfe_set_cpu_freq(freq);
    else
    {
        printf("cpu frequency is not specified\n");
        res = -1;
    }

    return res;
}

#endif
#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_) || defined (_BCM96858_) || defined (_BCM96856_)
/* write PMC image in the first sector/block */
static int ui_cmd_write_loaded_pmc(ui_cmdline_t *cmd,int argc,char *argv[])
{
    unsigned long address = 0, size = 0;
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszSize = cmd_getarg(cmd, 1);
    int res;
    
    if( pszAddr )
        address = xtoul(pszAddr);

    if( pszSize )
        size = (unsigned long) atoi(pszSize);

    if (address && size)
    {
         flash_sector_erase_int(0);
         if( flash_write_buf(0, 0, (unsigned char*)address, size) != size) 
         {
             printf("Failed to write flash\n");
             res = -1;
         }
    }
    else
    {
        printf("Failed - address or size not spicified\n");
        res = -1;
    }

    return res;
}
#endif

#if (BPCM_CFE_CMD==1)
static int  ui_cmd_read_bpcm_reg(ui_cmdline_t *cmd,int argc,char *argv[]) {

    uint8_t bus_id = 0, addr_id = 0, offset = 0;
    uint32_t val = 0;
    char *pszBusId = cmd_getarg(cmd, 0);
    char *pszAddrId = cmd_getarg(cmd, 1);
    char *pszOffset = cmd_getarg(cmd, 2);
    int res;

    if (!pszBusId || !pszAddrId || !pszOffset) {
        printf("did not specify enough parameter\n");
        return -1;
    }

    bus_id = (uint8_t)atoi(pszBusId);
    addr_id = (uint8_t)atoi(pszAddrId);
    offset = (uint8_t)atoi(pszOffset);

    res = cfe_read_bpcm(bus_id, addr_id, offset, &val);
    if (res == 0) {
        printf("val = 0x%08x\n", (unsigned int)val);
    } else {
        printf("fail:read_bpcm\n");
        res = -1;
    }

    return res;
}

static int  ui_cmd_write_bpcm_reg(ui_cmdline_t *cmd,int argc,char *argv[]) {

    uint8_t bus_id = 0, addr_id = 0, offset = 0;
    uint32_t val = 0;
    char *pszBusId = cmd_getarg(cmd, 0);
    char *pszAddrId = cmd_getarg(cmd, 1);
    char *pszOffset = cmd_getarg(cmd, 2);
    char *pszVal = cmd_getarg(cmd, 3);
    int res;

    if (!pszBusId || !pszAddrId || !pszOffset || !pszVal) {
        printf("did not specify enough parameter\n");
        return -1;
    }

    bus_id = (uint8_t)atoi(pszBusId);
    addr_id = (uint8_t)atoi(pszAddrId);
    offset = (uint8_t)atoi(pszOffset);
    val = (uint32_t)atoi(pszVal);

    res = cfe_write_bpcm(bus_id, addr_id, offset, val);
    if (res == 0) {
        printf("val = 0x%08x, write done\n", (unsigned int)val);
    } else {
        printf("fail:write_bpcm\n");
        res = -1;
    }

    return res;
}
#endif

#if defined _BCM963138_ || defined _BCM963148_ || defined _BCM963381_ || \
    defined _BCM94908_ || defined _BCM963158_
static int  ui_cmd_pmc_close_avs(ui_cmdline_t *cmd,int argc,char *argv[]) 
{
    char *pszIsland = NULL;
    char *pszMargin1 = NULL;
    char *pszMargin2 = NULL;
    int res;
    uint32_t val0 = 0;
    uint32_t val1 = 0;
    uint32_t val2  = 0;

    switch(argc){
    case 2:
      pszMargin1 = cmd_getarg(cmd, 0);
      pszMargin2 = cmd_getarg(cmd, 1);
      break;
    case 3:
      pszIsland = cmd_getarg(cmd, 0);
      pszMargin1 = cmd_getarg(cmd, 1);
      pszMargin2 = cmd_getarg(cmd, 2);
      val0 = (uint32_t)atoi(pszIsland);
      break;
    default:
      printf ("Invalid parameters\n");
      return 0;
    }

    val1 = (uint32_t)atoi(pszMargin1);
    val2  = (uint32_t)atoi(pszMargin2);

    res = CloseAVS(val0, val1, val2);
    if( res == 0 )
        printf("close avs with margin %dmV %dmVsucceeded\n", val1, val2);
    else
        printf("close avs failed\n");

    return res;
}

static int  ui_cmd_pmc_cmd(ui_cmdline_t *cmd,int argc,char *argv[])
{
    #define IS_DIGIT(ch) (ch >= '0' && ch <= '9')
    int PMCcmd(int[4]), pmc[4], res = 0;

    /* Check if pmc is taking numerical command ID */
    if ( IS_DIGIT(cmd_getarg(cmd, 0)[0]) ) {
        pmc[0] = atoi(cmd_getarg(cmd, 0));
        pmc[1] = atoi(cmd_getarg(cmd, 1));
        pmc[2] = atoi(cmd_getarg(cmd, 2));
        pmc[3] = atoi(cmd_getarg(cmd, 3));
        res = PMCcmd(pmc);
        printf("%04x %04x %04x %04x\n", pmc[0], pmc[1], pmc[2], pmc[3]);
    }
    else {
#if (INC_PMC_DRIVER==1)
        if (strcmp(cmd_getarg(cmd, 0), "on") == 0) 
            pmc_init();
        else if  (strcmp(cmd_getarg(cmd, 0), "off") == 0)
            pmc_reset();
        else
#endif
        {
            printf ("Unknown pmc command\n");
            res = -1;
        }
    }
    return res;
}

static int  ui_cmd_pmc_log(ui_cmdline_t *cmd,int argc,char *argv[])
{
#if defined(PMC_SHARED_MEMORY)
    pmc_log( atoi(cmd_getarg(cmd, 0)) );
#endif
    return 0;
}
#endif

static unsigned long loaded_file_size = 0;
static unsigned long loaded_file_address = 0;

static int ui_cmd_write_loaded_imge(ui_cmdline_t *cmd,int argc,char *argv[])
{
    unsigned long address, size;
    char *pszAddr = cmd_getarg(cmd, 0);
    char *pszSize = cmd_getarg(cmd, 1);
    int res;
    
    if( pszAddr )
        address = xtoul(pszAddr);
    else
        address = loaded_file_address;

    if( pszSize )
        size = (unsigned long) atoi(pszSize);
    else
        size = loaded_file_size;

    if (address && size)
         res = writeWholeImage((uint8_t*)address, size);
    else
    {
        printf("Failed - address or size not spicified\n");
        res = -1;
    }

    return res;
}

static int ui_cmd_loadb(ui_cmdline_t *cmd,int argc,char *argv[])
{
    unsigned long address;
    int res = 0;
    int serial = 0, network = 0, fs = 0;
    unsigned int filelen = 0, option = 0;
    char hostFileName[BOOT_FILENAME_LEN + BOOT_IP_LEN];
    char *pszOpt = cmd_getarg(cmd, 0);
    char *pszAddr = cmd_getarg(cmd, 1);
    char *pszFileName = cmd_getarg(cmd, 2);
    char *pszFileOption = cmd_getarg(cmd, 3);

    if( pszOpt == NULL )
    {
        printf("please specifiy load option\n");
        return -1;
    }
  
    if ( strcmp(pszOpt, "s") == 0 )
        serial = 1;
    else if ( strcmp(pszOpt, "n") == 0 )
    {
        network = 1;
        if( pszFileName )
        {
            if (strchr(pszFileName, ':'))
                strcpy(hostFileName, pszFileName);
            else
            {
                strcpy(hostFileName, bootInfo.hostIp);
                strcat(hostFileName, ":");
                strcat(hostFileName, pszFileName);
            }
        }
        else
            return -1;
    }
    else if ( strcmp(pszOpt, "f") == 0 )
    {
        /* load from bootfs */
        fs = 1;
        filelen = 0;
        option = atoi(pszFileOption);
    }
    else
        return -1;

#if (INC_KERMIT==0)
    if( serial == 1 )
    {
        printf("please enable KERMIT build for serial port load\n");
        return -1;
    }
#endif

#if (NONETWORKT==1)
    if( network == 1 )
    {
        printf("please enable NETWORK build for network load\n");
        return -1;
    }
#endif

    if( pszAddr )
        address = xtoul(pszAddr); 
    else
        address = cfe_get_mempool_ptr();

    loaded_file_address = address;
#if (INC_KERMIT==1)
    if( serial )
    {
        res = do_load_serial_bin(address, &loaded_file_size);
    }
#endif

#if (NONETWORKT==0)
    if( network == 1 ) 
    {
        loaded_file_size = loadRaw(hostFileName, (uint8_t*)address);
        if( (int)loaded_file_size < 0 )
            return -1;
    }
#endif

    if( fs == 1 )
    {
        res = -1;
        if( cfe_load_boot_file(pszFileName, (void*)NULL, &filelen, option) == CFE_ERR_NOMEM )
        {
            unsigned* tempbuf = KMALLOC(filelen, 0);
            printf("allocate file size  buffer %d\n", filelen);
            if( tempbuf == NULL ) 
            {
                printf("failed to allocated %d byte buffer!\n");
                return -1;
            } 
 
            if( (res = cfe_load_boot_file(pszFileName, (void*)tempbuf, &filelen, option)) == CFE_OK ) 
            {
                memcpy((void*)address, tempbuf, filelen); 
                if (option&BOOT_FILE_LOAD_OPT_FLUSHCACHE)
                    _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,  (uint8_t *)address,  ((uint8_t *)address) + filelen - 1);
                loaded_file_size = filelen;    	       
            }
            KFREE(tempbuf);
        }
    }
    
    if( res == 0 )
        printf("load %d bytes binary at 0x%lx successfully.\n", loaded_file_size, loaded_file_address);

    return res;
}

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) || defined (_BCM96848_) || ((INC_KERMIT==1) && !defined (_BCM960333_) && (NONETWORK==1))
static int ui_cmd_go(ui_cmdline_t *cmd,int argc,char *argv[])
{
    unsigned long address;

    char *pszAddr = cmd_getarg(cmd, 0);

    if( pszAddr )
    {
        address = xtoul(pszAddr);
        blparms_install(NULL);

#if ( defined(_BCM963138_) || defined(_BCM963148_) ) && (INC_SPI_PROG_NAND==1)
        NAND->NandCsNandXor = 1;
#endif

        cfe_launch(address);
    }

    return 0;
}
#endif

static int ui_cmd_set_partition_size(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int ret = 0;
    int reset = 0;

    if(!argc)
    {
        reset += setPartitionSizes();
    }

    if(reset)
        softReset(0);

    return ret;
}

static int ui_cmd_erase_misc_partition(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *str1 = cmd_getarg(cmd, 0);
    if (str1)
    {
        if(str1[0]-'1' >= 0 && str1[0]-'1'  < BCM_MAX_EXTRA_PARTITIONS )
        {
            erase_misc_partition(str1[0]-'1', NVRAM_RP);
        }
    }
return 0;

}

#if (defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_) || defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
static int ui_cmd_runner_show_rings(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int start_from = -1;
    char *str1 = cmd_getarg(cmd, 0);

    if (str1)
    {
        start_from = atoi(str1);

        if (start_from < 0 || start_from >= D_NUM_OF_RING_DESCRIPTORS)
        {
            xprintf("wrong ring id, max is %d\n", D_NUM_OF_RING_DESCRIPTORS - 1);
            return -1;
        }
    }

    return cpu_ring_shell_list_rings(NULL, start_from);
}

static int ui_cmd_runner_print_pd(ui_cmdline_t *cmd, int argc, char *argv[])
{
    uint32_t ring_id;
	uint32_t pd_index;
    char *str1, *str2;

    if (argc != 2)
    {
        xprintf("Wrong parameters number\n%s\n", vrpd_usage_str);
        return -1;
    }

    str1 = cmd_getarg(cmd, 0);
    str2 = cmd_getarg(cmd, 1);
	ring_id = atoi(str1);
	pd_index = atoi(str2);

    if (ring_id >= D_NUM_OF_RING_DESCRIPTORS)
    {
        xprintf("Wrong params values\nMax ring index %d\n", D_NUM_OF_RING_DESCRIPTORS - 1);
        return -1;
    }

    if (pd_index >= rdp_cpu_ring_get_queue_size(ring_id))
    {
        xprintf("Wrong params values\nMax pd index %d\n", rdp_cpu_ring_get_queue_size(ring_id) - 1);
        return -1;
    }

    return cpu_ring_shell_print_pd(NULL, ring_id, pd_index);
}

#endif

#if defined(_BCM96858_) && (NONETWORK == 0)

static int lport_port_validate(ui_cmdline_t *cmd, const char *usage, int min, int max)
{
    char *port_str;
    int port;

    if (!(port_str = cmd_getarg(cmd, 0)))
    {
        xprintf("%s\n", usage);
        return -1;
    }

    port = atoi(port_str);
    if (port < min || port > max)
    {
        xprintf("%s\n", usage);
        return -1;
    }
    return port;
}

static int ui_get_mandatory_params(ui_cmdline_t *cmd, int start, char *strs[], int count)
{
    int i;

    for (i = 0; i < count; i++)
        if (!(strs[i] = cmd_getarg(cmd, start + i)))
            return -1;

    return 0;
}

static char *lport_duplex_to_str(LPORT_PORT_DUPLEX duplex)
{
    return duplex == LPORT_HALF_DUPLEX ? "Half duplex" : "Full duplex";
}

static int ui_cmd_lport_rate(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int port;
    char *rate_str;
    lport_port_cfg_s port_conf;
    LPORT_PORT_RATE rate; 

    if ((port = lport_port_validate(cmd, lport_rate_usage, 0, 6)) == -1)
        return -1;

    lport_get_port_configuration(port, &port_conf);

    rate_str = cmd_getarg(cmd, 1);
    if (!rate_str)
    {
        xprintf("Port %d rate: %s\n", port, lport_rate_to_str(port_conf.speed));
        return 0;
    }

    if ((rate = lport_str_to_rate(rate_str)) == LPORT_RATE_UNKNOWN)
    {
        xprintf("%s\n",  lport_rate_usage);
        return -1;
    }

    port_conf.speed = rate;
    lport_set_port_configuration(port, &port_conf);

    return 0;
}

static int ui_cmd_lport_status(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int port;
    lport_port_status_s port_status = {};

    if ((port = lport_port_validate(cmd, lport_status_usage, 0, 6)) == -1)
        return -1;

    lport_get_port_status(port, &port_status);
    xprintf("Port %d status:\n", port);
    xprintf("\tautoneg_en: %d\n", port_status.autoneg_en);
    xprintf("\tport_up: %d\n", port_status.port_up);
    xprintf("\trate: %s\n", lport_rate_to_str(port_status.rate));
    xprintf("\tduplex: %s\n", lport_duplex_to_str(port_status.duplex));
    xprintf("\trx_pause_en: %d\n", port_status.rx_pause_en);
    xprintf("\ttx_pause_en: %d\n", port_status.rx_pause_en);
	
	lport_xlmac_fifo_status fifo_status;
	ag_drv_lport_xlmac_fifo_status_get(port, &fifo_status);
	xprintf("\txlmac_status: %d\n", fifo_status.link_status);
	xprintf("\trx_pkt_overflow: %d\n", fifo_status.rx_pkt_overflow);
	xprintf("\ttx_ts_fifo_overflow: %d\n", fifo_status.tx_ts_fifo_overflow);
	xprintf("\ttx_llfc_msg_overflow: %d\n", fifo_status.tx_llfc_msg_overflow);
	xprintf("\ttx_pkt_overflow: %d\n", fifo_status.tx_pkt_overflow);
	xprintf("\ttx_pkt_underflow: %d\n", fifo_status.tx_pkt_underflow);
	xprintf("\trx_msg_overflow: %d\n", fifo_status.rx_msg_overflow);

	xprintf("\ttx_pause_en: %d\n", port_status.rx_pause_en);

    return 0;
}

static int ui_cmd_lport_stats(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int port;
    lport_rx_stats_s rx_stats;
    lport_tx_stats_s tx_stats;
    char *clean;

    if ((port = lport_port_validate(cmd, lport_stats_usage, 0, 7)) == -1)
        return -1;

    if ((clean = cmd_getarg(cmd, 1)))
    {
        if (strcmp(clean, "clean"))
        {
            xprintf("%s\n", lport_stats_usage);
            return -1;
        }
    }

    lport_stats_get_rx(port, &rx_stats);
    lport_stats_get_tx(port, &tx_stats);
    xprintf("\x1B[31m \t\tLPORT %d Statistics\x1B[0m\n", port);
    xprintf("+----------------+---------++----------------+---------+\n");
    xprintf("| Counter        | Value   || Counter        | Value   |\n");
    xprintf("+----------------+---------++----------------+---------+\n");
    xprintf("| Rx Packet OK   |%9u|| Tx Packet OK   |%9u|\n", rx_stats.GRXPOK, tx_stats.GTXPOK);
    xprintf("| Rx 64 Byte     |%9u|| Tx 64 Byte     |%9u|\n", rx_stats.GRX64, tx_stats.GTX64);
    xprintf("| Rx 65 to 127   |%9u|| Tx 65 to 127   |%9u|\n", rx_stats.GRX127, tx_stats.GTX127);
    xprintf("| Rx 128 to 255  |%9u|| Tx 128 to 255  |%9u|\n", rx_stats.GRX255, tx_stats.GTX255);
    xprintf("| Rx 256 to 511  |%9u|| Tx 256 to 511  |%9u|\n", rx_stats.GRX511, tx_stats.GTX511);
    xprintf("| Rx 512 to 1023 |%9u|| Tx 512 to 1023 |%9u|\n", rx_stats.GRX1023, tx_stats.GTX1023);
    xprintf("| Rx 1024 to 1518|%9u|| Tx 1024 to 1518|%9u|\n", rx_stats.GRX1518, tx_stats.GTX1518);
    xprintf("| Rx 1519 to 2047|%9u|| Tx 1519 to 2047|%9u|\n", rx_stats.GRX2047, tx_stats.GTX2047);
    xprintf("| Rx frame       |%9u|| Tx frame       |%9u|\n", rx_stats.GRXPKT, tx_stats.GTXPKT);
    xprintf("| Rx Bytes       |%9u|| Tx Bytes       |%9u|\n", rx_stats.GRXBYT, tx_stats.GTXBYT);
    xprintf("| Rx Unicast     |%9u|| Tx Unicast     |%9u|\n", rx_stats.GRXUCA, tx_stats.GTXUCA);
    xprintf("| Rx Multicast   |%9u|| Tx Multicast   |%9u|\n", rx_stats.GRXMCA, tx_stats.GTXUCA);
    xprintf("| Rx Broadcast   |%9u|| Tx Broadcast   |%9u|\n", rx_stats.GRXBCA, tx_stats.GTXBCA);
    xprintf("| Rx FCS Error   |%9u|| Tx FCS Error   |%9u|\n", rx_stats.GRXFCS, tx_stats.GTXFCS);
    xprintf("| Rx Control     |%9u|| Tx Control     |%9u|\n", rx_stats.GRXCF, tx_stats.GTXCF);
    xprintf("| Rx PAUSE       |%9u|| Tx PAUSE       |%9u|\n", rx_stats.GRXPF, tx_stats.GTXCF);
    xprintf("| Rx PFC         |%9u|| Tx PFC         |%9u|\n", rx_stats.GRXPP, tx_stats.GTXPFC);
    xprintf("| Rx Unsup Op    |%9u|| Tx Oversized   |%9u|\n", rx_stats.GRXUO, tx_stats.GTXOVR);
    xprintf("| Rx Unsup DA    |%9u||                |         |\n", rx_stats.GRXUDA);
    xprintf("| Rx Wrong SA    |%9u||                |         |\n", rx_stats.GRXWSA);
    xprintf("| Rx Align Error |%9u||                |         |\n", rx_stats.GRXALN);
    xprintf("| Rx Length OOR  |%9u||                |         |\n", rx_stats.GRXFLR);
    xprintf("| Rx Code Error  |%9u||                |         |\n", rx_stats.GRXFRERR);
    xprintf("| Rx False Carri |%9u||                |         |\n", rx_stats.GRXFCR);
    xprintf("| Rx Oversized   |%9u||                |         |\n", rx_stats.GRXOVR);
    xprintf("| Rx Jabber      |%9u||                |         |\n", rx_stats.GRXJBR);
    xprintf("| Rx MTU ERR     |%9u||                |         |\n", rx_stats.GRXMTUE);
    xprintf("| Rx Truncated   |%9u||                |         |\n", rx_stats.GRXTRFU);
    xprintf("| Rx SCH CRC     |%9u||                |         |\n", rx_stats.GRXSCHCRC);
    xprintf("| Rx RUNT        |%9u||                |         |\n", rx_stats.GRXRPKT);
    xprintf("| Rx Undersize   |%9u||                |         |\n", rx_stats.GRXUND);
    xprintf("| Rx Fragment    |%9u||                |         |\n", rx_stats.GRXFRG);
    xprintf("+----------------+---------++----------------+---------+\n");
    

    if (clean)
        lport_stats_reset(port);

    return 0;
}

static int ui_cmd_lport_rgmii(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int port, i, param[4];
    lport_rgmii_cfg_s rgmii_cfg;
    char *param_str[4];

    if ((port = lport_port_validate(cmd, lport_rgmii_usage, 4, 6)) == -1)
        return -1;

    lport_get_rgmii_cfg(port, &rgmii_cfg);

    if (ui_get_mandatory_params(cmd, 1, param_str, 4))
    {
        xprintf("RGMII port %d configuration:\n", port);
        xprintf("\tvalid: %d\n", rgmii_cfg.valid);
        xprintf("\tib_status_override: %d\n", rgmii_cfg.ib_status_overide);
        xprintf("\tphy_attached: %d\n", rgmii_cfg.phy_attached);
        xprintf("\tphyid: %d\n", rgmii_cfg.phyid);
        return 0;
    }

    for (i = 0; i < 3; i++)
        if ((param[i] = atoi(param_str[i])) != 0 && param[i] != 1)
        {
            xprintf("%s\n", lport_rgmii_usage);
            return -1;
        }

    param[3] = atoi(param_str[3]);

    rgmii_cfg.valid = param[0];
    rgmii_cfg.ib_status_overide = param[1];
    rgmii_cfg.phy_attached = param[2];

    if (param[3] > LPORT_MAX_PHY_ID)
    {
        xprintf("%s\n", lport_rgmii_usage);
        return -1;
    }
    rgmii_cfg.phyid = param[3];

    rgmii_cfg.portmode = 3; /* RGMII */

    lport_set_rgmii_cfg(port, &rgmii_cfg);
    
    return 0;
}

static int ui_cmd_lport_serdes_reg(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *id_str, *reg_addr_str, *mask_str, *value_str;
    E_MERLIN_ID id;
    uint16_t reg_addr, mask = 0, value;

    if (!(id_str = cmd_getarg(cmd, 0)))
        goto WrongParams;

    switch (id_str[0])
    {
        case '0': id = MERLIN_ID_0; break;
        case '1': id = MERLIN_ID_1; break;
        default: goto WrongParams;
    }

    if (!(reg_addr_str = cmd_getarg(cmd, 1)))
        goto WrongParams;
    reg_addr = xtoi(reg_addr_str);
 
    if ((mask_str = cmd_getarg(cmd, 2)))
        mask = xtoi(mask_str);
    value_str = cmd_getarg(cmd, 3);

    if (!value_str)
    {
        /* Register read was requested */
        read_serdes_reg(id, reg_addr, mask, &value);
        xprintf("0x%x\n", value);
        return 0;
    }

    /* Register write was requested */
    value = xtoi(value_str);
    write_serdes_reg(id, reg_addr, mask, value);

    return 0;

WrongParams:
    xprintf("%s\n", lport_serdes_reg_usage);
    return -1;
}

extern int send_dummy_packet(uint8_t port_id, uint32_t packet_size, int number_of_packets,
    uint8_t is_random);
extern int lport_serdes_set_loopback(uint32_t port, uint32_t loopback_mode, uint32_t enable);
extern int lport_serdes_diag(uint32_t port, uint32_t cmd);
extern int lport_serdes_read_reg(uint32_t port, uint16_t device_type, uint16_t reg_address);
extern int lport_serdes_write_reg(uint32_t port, uint16_t device_type, uint16_t reg_address, uint32_t value);

int ui_cmd_send_buffer(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str, *packet_size_str, *number_of_packets_str, *is_random_str;
    uint8_t port_id;
    uint32_t packet_size;
    int number_of_packets;
    uint8_t is_random;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    if(!(number_of_packets_str = cmd_getarg(cmd, 1)))
        goto wrong_params;

    number_of_packets = atoi(number_of_packets_str);

    if(!(is_random_str = cmd_getarg(cmd, 2)))
        goto wrong_params;

    is_random = atoi(is_random_str);

    if (is_random != 0 && is_random != 1)
        goto wrong_params;

    if(!(packet_size_str = cmd_getarg(cmd, 3)))
        goto wrong_params;

    packet_size = atoi(packet_size_str);

    if (packet_size <= 0 || packet_size > 1536)
        goto wrong_params;

    return send_dummy_packet(port_id, packet_size, number_of_packets, is_random);

wrong_params:
    xprintf("%s\n", send_buffer_usage);
    return -1;
}

int ui_cmd_serdes_set_loopback(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str, *loopback_mode_str, *is_enabled_str;
    uint8_t port_id;
    uint8_t loopback_mode;
    uint8_t is_enabled;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    if(!(loopback_mode_str = cmd_getarg(cmd, 1)))
        goto wrong_params;

    loopback_mode = atoi(loopback_mode_str);
    if (loopback_mode > 4)
        goto wrong_params;

    if(!(is_enabled_str = cmd_getarg(cmd, 2)))
        goto wrong_params;

    is_enabled = atoi(is_enabled_str);

    if (is_enabled != 0 && is_enabled != 1)
        goto wrong_params;

    return lport_serdes_set_loopback(port_id, loopback_mode, is_enabled);

wrong_params:
    xprintf("%s\n", serdes_set_loopback_usage);
    xprintf("Loopback modes: \n 0 - NONE\n 1 - PCS Local loopback\n 2 - PCS Remote loopback\n"
        " 3 - PMD Local Loopback\n 4 - PMD Remote Loopback\n"); 
    return -1;
}

int ui_cmd_serdes_diag_status(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str;
    uint8_t port_id;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    return lport_serdes_diag(port_id, 1);

wrong_params:
    xprintf("%s\n", serdes_diag_status_usage);
    return -1;
}

int ui_cmd_serdes_diag_stats(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str;
    uint8_t port_id;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    return lport_serdes_diag(port_id, 2);

wrong_params:
    xprintf("%s\n", serdes_diag_stats_usage);
    return -1;
}

int ui_cmd_serdes_read_reg(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str, *device_type_str, *reg_addr_str;
    uint8_t port_id;
    uint16_t device_type, reg_addr;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    if(!(device_type_str = cmd_getarg(cmd, 1)))
        goto wrong_params;
    device_type = atoi(device_type_str);

    if (!(reg_addr_str = cmd_getarg(cmd, 2)))
        goto wrong_params;
    reg_addr = xtoi(reg_addr_str);
 
    return lport_serdes_read_reg(port_id, device_type, reg_addr);

wrong_params:
    xprintf("%s\n", serdes_read_reg_usage);
    return -1;
}

int ui_cmd_serdes_write_reg(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str, *device_type_str, *reg_addr_str, *value_str;
    uint8_t port_id;
    uint16_t device_type, reg_addr;
    uint16_t value;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    if(!(device_type_str = cmd_getarg(cmd, 1)))
        goto wrong_params;
    device_type = atoi(device_type_str);

    if (!(reg_addr_str = cmd_getarg(cmd, 2)))
        goto wrong_params;
    reg_addr = xtoi(reg_addr_str);
 
    if (!(value_str = cmd_getarg(cmd, 3)))
        goto wrong_params;
    value = xtoi(value_str);

    return lport_serdes_write_reg(port_id, device_type, reg_addr, value);

wrong_params:
    xprintf("%s\n", serdes_write_reg_usage);
    return -1;
}

extern int cfe_eth_set_port(uint8_t port);
int ui_cmd_set_port(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *port_id_str;
    uint8_t port_id;

    if(!(port_id_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    port_id = atoi(port_id_str);
    if (port_id < 0 || port_id > 6)
        goto wrong_params;

    return cfe_eth_set_port(port_id);

wrong_params:
    xprintf("%s\n", set_port_usage);
    return -1;
}

extern int cfe_set_compare_enable(int is_enable);
int ui_cmd_set_compare(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *is_enable_str;
    int is_enable;

    if(!(is_enable_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    is_enable = atoi(is_enable_str);
    if (is_enable < 0 || is_enable > 1)
        goto wrong_params;

    return cfe_set_compare_enable(is_enable);

wrong_params:
    xprintf("%s\n", set_compare_enable_usage);
    return -1;
}
#endif

#if (defined(_BCM96858_) || defined(_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
extern int get_cfe_net_stats(int clean);
int ui_cmd_net_stats(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *clean_str;
    int do_clean = 0;
    if ((clean_str = cmd_getarg(cmd, 0)))
    {
        if (strcmp(clean_str, "clean"))
            goto wrong_params;
        do_clean = 1;
    }
 
    return get_cfe_net_stats(do_clean); 

wrong_params:
    xprintf("%s\n", network_stats_usage);
    return -1;
}

extern int cfe_set_dump_enable(int is_enable);
int ui_cmd_set_dump(ui_cmdline_t *cmd, int argc, char *argv[])
{
    char *is_enable_str;
    int is_enable;

    if(!(is_enable_str = cmd_getarg(cmd, 0)))
        goto wrong_params;

    is_enable = atoi(is_enable_str);
    if (is_enable < 0 || is_enable > 1)
        goto wrong_params;

    return cfe_set_dump_enable(is_enable);

wrong_params:
    xprintf("%s\n", set_dump_enable_usage);
    return -1;
}
#endif
#if ( defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)

static int unimac_port_validate(ui_cmdline_t *cmd, const char *usage, int min, int max)
{
    char *port_str;
    int port;

    if (!(port_str = cmd_getarg(cmd, 0)))
    {
        xprintf("%s\n", usage);
        return -1;
    }

    port = atoi(port_str);
    if (port < min || port > max)
    {
        xprintf("%s\n", usage);
        return -1;
    }
    return port;
}

int mac_stats_get(uint32_t port, mac_stats_t *stats);
void mac_stats_clear(uint32_t port);

static int ui_cmd_unimac_stats(ui_cmdline_t *cmd, int argc, char *argv[])
{
    int port;
    mac_stats_t stats;
    char *clean;

    if ((port = unimac_port_validate(cmd, unimac_stats_usage, 0, 5)) == -1)
        return -1;

    if ((clean = cmd_getarg(cmd, 1)))
    {
        if (strcmp(clean, "clean"))
        {
            xprintf("%s\n", unimac_stats_usage);
            return -1;
        }
    }

    mac_stats_get(port, &stats);

    xprintf("\x1B[31m \t\tUnimac %d Statistics\x1B[0m\n", port);
    xprintf("+----------------+---------++----------------+---------+\n");
    xprintf("| Counter        | Value   || Counter        | Value   |\n");
    xprintf("+----------------+---------++----------------+---------+\n");
    xprintf("| Rx Bytes       |%9u|| Tx Bytes       |%9u|\n", stats.rx_byte, stats.tx_byte);
    xprintf("| Rx Packet      |%9u|| Tx Packet      |%9u|\n", stats.rx_packet, stats.tx_packet);
    xprintf("| Rx 64 Byte     |%9u|| Tx 64 Byte     |%9u|\n", stats.rx_frame_64, stats.tx_frame_64);
    xprintf("| Rx 65 to 127   |%9u|| Tx 65 to 127   |%9u|\n", stats.rx_frame_65_127, stats.tx_frame_65_127);
    xprintf("| Rx 128 to 255  |%9u|| Tx 128 to 255  |%9u|\n", stats.rx_frame_128_255, stats.tx_frame_128_255);
    xprintf("| Rx 256 to 511  |%9u|| Tx 256 to 511  |%9u|\n", stats.rx_frame_256_511, stats.tx_frame_256_511);
    xprintf("| Rx 512 to 1023 |%9u|| Tx 512 to 1023 |%9u|\n", stats.rx_frame_512_1023, stats.tx_frame_512_1023);
    xprintf("| Rx 1024 to 1518|%9u|| Tx 1024 to 1518|%9u|\n", stats.rx_frame_1024_1518, stats.tx_frame_1024_1518);
    xprintf("| Rx 1519 to MTU |%9u|| Tx 1519 to MTU |%9u|\n", stats.rx_frame_1519_mtu, stats.tx_frame_1519_mtu);
    xprintf("| Rx Unicast     |%9u|| Tx Unicast     |%9u|\n", stats.rx_unicast_packet, stats.tx_unicast_packet);
    xprintf("| Rx Multicast   |%9u|| Tx Multicast   |%9u|\n", stats.rx_multicast_packet, stats.tx_multicast_packet);
    xprintf("| Rx Broadcast   |%9u|| Tx Broadcast   |%9u|\n", stats.rx_broadcast_packet, stats.tx_broadcast_packet);
    xprintf("| Rx FCS Error   |%9u|| Tx FCS Error   |%9u|\n", stats.rx_fcs_error, stats.tx_fcs_error);
    xprintf("| Rx Control     |%9u|| Tx Control     |%9u|\n", stats.rx_control_frame, stats.tx_control_frame);
    xprintf("| Rx PAUSE       |%9u|| Tx PAUSE       |%9u|\n", stats.rx_pause_control_frame, stats.tx_pause_control_frame);
    xprintf("| Rx Oversized   |%9u|| Tx Oversized   |%9u|\n", stats.rx_oversize_packet, stats.tx_oversize_frame);
    xprintf("| Rx Undersize   |%9u|| Tx Undersize   |%9u|\n", stats.rx_undersize_packet, stats.tx_undersize_frame);
    xprintf("| Rx FIFO Err    |%9u|| Tx FIFO Err    |%9u|\n", stats.rx_fifo_errors, stats.tx_fifo_errors);
    xprintf("| Rx Fragments   |%9u|| Tx Fragment    |%9u|\n", stats.rx_fragments, stats.tx_fragments_frame);
    xprintf("| Rx Jabber      |%9u|| Tx Jabber      |         |\n", stats.rx_jabber, stats.tx_jabber_frame);
    xprintf("| Tx Error       |%9u||                |         |\n", stats.tx_error);
    xprintf("| Tx Underrun    |%9u||                |         |\n", stats.tx_underrun);
    xprintf("| Tx Undersize   |%9u||                |         |\n", stats.tx_undersize_frame);
    xprintf("| Tx Total Coll  |%9u||                |         |\n", stats.tx_total_collision);
    xprintf("| Tx Excess Coll |%9u||                |         |\n", stats.tx_excessive_collision);
    xprintf("| Tx Late Coll   |%9u||                |         |\n", stats.tx_late_collision);
    xprintf("| Tx Single Coll |%9u||                |         |\n", stats.tx_single_collision);
    xprintf("| Tx Mult Coll   |%9u||                |         |\n", stats.tx_multiple_collision);
    xprintf("| Tx Defferal    |%9u||                |         |\n", stats.tx_deferral_packet); 
    xprintf("| Tx Excess Deffe|%9u||                |         |\n", stats.tx_excessive_deferral_packet); 
    xprintf("| Rx Unsup Opcode|%9u||                |         |\n", stats.rx_unknown_opcode);
    xprintf("| Rx Align Error |%9u||                |         |\n", stats.rx_alignment_error);
    xprintf("| Rx Length Err  |%9u||                |         |\n", stats.rx_frame_length_error);
    xprintf("| Rx Code Error  |%9u||                |         |\n", stats.rx_code_error);
    xprintf("| Rx Carri Sense |%9u||                |         |\n", stats.rx_carrier_sense_error);
    xprintf("| Rx Overflow    |%9u||                |         |\n", stats.rx_overflow);
    xprintf("+----------------+---------++----------------+---------+\n");

    if (clean)
        mac_stats_clear(port);

    return 0;
}
#endif

#if INC_EMMC_FLASH_DRIVER
extern queue_t cfe_devices;

static int ui_cmd_emmc_gpt_fmt(ui_cmdline_t *cmd,int argc,char *argv[])
{
    unsigned int bootfs_size, rootfs_size;
    unsigned int misc1_size, misc2_size, misc3_size, misc4_size;
    unsigned int data_size;

    if( argc != 7 )
    {
       printf("Invalid number of arguments\n");
       return 0;
    }

    bootfs_size = (unsigned int)atoi(cmd_getarg(cmd,0));    
    rootfs_size = (unsigned int)atoi(cmd_getarg(cmd,1));
    data_size  =  (unsigned int)atoi(cmd_getarg(cmd,2));
    misc1_size  = (unsigned int)atoi(cmd_getarg(cmd,3));
    misc2_size  = (unsigned int)atoi(cmd_getarg(cmd,4));
    misc3_size  = (unsigned int)atoi(cmd_getarg(cmd,5));
    misc4_size  = (unsigned int)atoi(cmd_getarg(cmd,6));

    if(bootfs_size <= 0)
    {
       printf("Invalid bootfs size %d\n", bootfs_size);
       return 0;
    }
    if(rootfs_size <= 0)
    {
       printf("Invalid bootfs size %d\n", rootfs_size);
       return 0;
    }
    if(data_size <= 0)
    {
       printf("Invalid bootfs size %d\n", data_size);
       return 0;
    }

    printf("Modify eMMC GPT partitions? This will erase all data on existing GPT partitions!");
    if (yesno())
        return -1;

    return (emmc_format_gpt_dataPhysPart(bootfs_size, rootfs_size, data_size,
                        misc1_size, misc2_size, misc3_size, misc4_size)); 
}

static int ui_cmd_emmc_gpt_dmp(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return (emmc_dump_gpt_dataPhysPart());
}
static int ui_cmd_dump_emmc_bootfs(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char * flashdev;
    char * filename = 0;
    
    flashdev = cmd_getarg(cmd,0);   
    filename = cmd_getarg(cmd,1);    
               
    return (emmc_dump_bootfs(flashdev, filename));
}

static int ui_cmd_get_emmc_write_part_words(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char * flashdev;
    unsigned int offset = 0;
    unsigned int write_val = 0;
    
    flashdev = cmd_getarg(cmd,0);

    if(cmd_getarg(cmd,1))     
    {
        offset = xtoul(cmd_getarg(cmd,1));    
    }
            
    if(cmd_getarg(cmd,2))             
    {
        write_val = xtoul(cmd_getarg(cmd,2));    
    }
    
    return(emmc_write_part_words( flashdev, offset, write_val ));
}

static int ui_cmd_get_emmc_read_part_words(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char * flashdev;
    unsigned int offset;
    int num_words;
    
    flashdev = cmd_getarg(cmd,0);
    offset = xtoul(cmd_getarg(cmd,1));    
            
    num_words = atoi(cmd_getarg(cmd,2));    
    num_words = (num_words ? num_words: 64);
    
    return( emmc_read_part_words(flashdev, offset, num_words));
}


static int ui_cmd_get_emmc_erase_part(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char * flashdev;
    
    flashdev = cmd_getarg(cmd,0);
    
    if( !flashdev )
    {
       printf("Invalid number of arguments\n");
       return 0;
    }
    
    return( emmc_erase_partition(flashdev));
}

static int ui_cmd_get_emmc_erase_img(ui_cmdline_t *cmd,int argc,char *argv[])
{
        
    int img_num = atoi(cmd_getarg(cmd,0));
    int erase_cferom_nvram = atoi(cmd_getarg(cmd,1));
    
    if( !img_num )
    {
       printf("Invalid number of arguments\n");
       return 0;
    }
    
    return( emmc_erase_img(img_num, erase_cferom_nvram));
}

static int ui_cmd_get_emmc_erase_all(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int leave_blank = atoi(cmd_getarg(cmd,0));

    printf("Erase all partitions on eMMC? This will wipeout all data from the device!");
    if (yesno())
        return -1;

    emmc_erase_all(leave_blank);

    return( 0 );
}

int ui_cmd_devnames(ui_cmdline_t *cmd,int argc,char *argv[])
{
    queue_t *qb;
    cfe_device_t *dev;

    xprintf("Device Name          Description\n");
    xprintf("-------------------  ---------------------------------------------------------\n");

    for (qb = cfe_devices.q_next; qb != &cfe_devices; qb = qb->q_next) {
        dev = (cfe_device_t *) qb;

        printf("%19s  %s\n",dev->dev_fullname,
                dev->dev_description);

        }
    return 0;
}

static int ui_cmd_go_emmc_idle(ui_cmdline_t *cmd,int argc,char *argv[])
{    
    return (emmc_go_idle());
}

static int ui_cmd_emmc_allow_img_update_repartition(ui_cmdline_t *cmd,int argc,char *argv[])
{    
    int optVal = atoi(cmd_getarg(cmd,0));

    if( optVal )
    {
        printf("Allow next image update to repartition eMMC? This will erase all data on existing GPT partitions!");
        if( yesno() != 0 ) 
        {
            return 0;
        }
        else
        printf("Setting imgupdate flash repartitioning flag\n");
    }
    else
        printf("Clearing imgupdate flash repartitioning flag\n");

    emmc_allow_img_update_repartitions(optVal);
    return 0;
}

static int ui_cmd_emmc_boot_img(ui_cmdline_t *cmd,int argc,char *argv[])
{
    int imageNum = atoi(cmd_getarg(cmd,0));    
    
    if( imageNum <= 2)   
        return (emmc_boot_os_image(imageNum));
    else
        return CFE_ERR_IOERR;        
}

static int ui_cmd_get_emmc_info(ui_cmdline_t *cmd,int argc,char *argv[])
{
    return (emmc_get_info());
}
#endif /* INC_EMMC_FLASH_DRIVER */

extern int ui_init_netcmds(void);
#if (defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM963138_) || defined(_BCM963381_) || defined (_BCM96856_) || defined(_BCM96846_)) && ( (INC_NAND_FLASH_DRIVER==1) || (INC_EMMC_FLASH_DRIVER==1) )
extern int ui_init_otp_cmds(void);
#endif

static int ui_cmd_meminfo(ui_cmdline_t *cmd,int argc,char *argv[])
{
    cfe_mem_info();
    cfe_stack_info();
    cfe_heap_info();
    return 0;
}


static int ui_init_bcm63xx_cmds(void)
{
    console_name = fakeConsole;     // for cfe_rawfs strcmp
    // Used to flash an image that does not contain a FILE_TAG record.

    brcm_cmd_addcmd("force",
           ui_cmd_set_force_mode,
           NULL,
           "override chipid check for images.",
           "",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_) || defined(_BCM963158_)

    brcm_cmd_addcmd("cpufreq",
           ui_cmd_set_cpufreq,
           NULL,
           "set CPU frequency",
           "eg. cpufreq freq_in_MHz",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

#if (BPCM_CFE_CMD==1)
    brcm_cmd_addcmd("readbpcm",
           ui_cmd_read_bpcm_reg,
           NULL,
           "read BPCM register",
           "eg. readbpcm bus_id addr_id word_offset (for B15PLL, bus_id = 1, add_id = 12)",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("writebpcm",
           ui_cmd_write_bpcm_reg,
           NULL,
           "write BPCM register",
           "eg. writebpcm bus_id addr_id word_offset val (for B15PLL, bus_id = 1, addr_id = 12)",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

#if defined _BCM963138_ || defined _BCM963148_ || defined _BCM963381_ || \
    defined _BCM94908_ || defined _BCM963158_
    brcm_cmd_addcmd("closeavs",
           ui_cmd_pmc_close_avs,
           NULL,
           "pmc close avs cmd",
           "eg. closeavs [island] margin1 margin2",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
    brcm_cmd_addcmd("pmc",
           ui_cmd_pmc_cmd,
           NULL,
           "pmc cmd",
           "eg. pmc on/off\neg. pmc cmd# param1 param2 param3",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
    brcm_cmd_addcmd("pmclog",
           ui_cmd_pmc_log,
           NULL,
           "pmclog",
           "eg. pmclog",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

    brcm_cmd_addcmd("spi ",
           ui_cmd_spi_access,
           NULL,
           "Legacy SPI access of external switch.",
           "eg. spi address_in_hex value_in_hex size_4_2_or_1",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("pmdio",
           ui_cmd_pmdio_access,
           NULL,
           "Pseudo MDIO access for external switches.",
           "eg. pmdio address_in_hex value_in_hex size_4_2_or_1",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

#if defined(_BCM94908_)
    brcm_cmd_addcmd("reboot",
               ui_cmd_reset,
               NULL,
               "",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);
#endif

    brcm_cmd_addcmd("reset",
               ui_cmd_reset,
               NULL,
               "Reset the board",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);

    brcm_cmd_addcmd("b",
               ui_cmd_set_board_param,
               NULL,
               "Change board parameters",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);

    brcm_cmd_addcmd("a",
               ui_cmd_set_afe_id,
               NULL,
               "Change board AFE ID",
               "",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM96848_) || \
    defined(_BCM963158_) || defined(_BCM96858_) || defined(_BCM96846_) || defined(_BCM96856_)
    brcm_cmd_addcmd("ddr",
               ui_cmd_set_ddr_config,
               NULL,
               "Change board DDR config",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);
#endif

#if defined(_BCM94908_) || defined(_BCM963158_)
    brcm_cmd_addcmd("avs",
               ui_cmd_set_avs_config,
               NULL,
               "Change AVS config",
               "eg. avs enable/disable\neg. avs show",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);
#endif

#if defined(INC_DHCLIENT)
    brcm_cmd_addcmd("dhcp",
               ui_cmd_dhcp,
               NULL,
               "get DHCP address",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);
#endif

    brcm_cmd_addcmd("i",
                ui_cmd_erase_psi,
                NULL,
                "Erase persistent storage data",
                "",
                "",
               BRCM_UI_CMD_TYPE_NORMAL);

    if(( flash_get_flash_type() !=  FLASH_IFC_NAND ) && ( flash_get_flash_type() !=  FLASH_IFC_SPINAND ))
    {
        brcm_cmd_addcmd("f",
            ui_cmd_flash_image,
            NULL,
            "Write image to the flash ",
            "eg. f [[hostip:]filename]<cr> -- if no filename, tftped from host with file name in 'Default host flash file name'",
            "",
            BRCM_UI_CMD_TYPE_NORMAL);
    }

    brcm_cmd_addcmd("c",
               ui_cmd_change_bootline,
               NULL,
               "Change booline parameters",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);

    brcm_cmd_addcmd("p",
               ui_cmd_print_system_info,
               NULL,
               "Print boot line and board parameter info",
               "",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);

    brcm_cmd_addcmd("r",
               ui_cmd_run_program,
               NULL,
               "Run program from flash image or from host depending on [f/h/c] flags",
               "eg. r [[hostip:]filename]<cr> if no filename, use the file name in 'Default host run file name';from network(tftp):'r n <host ip> <rdimage> <ramdisk> <dtb> <ramdisk load phy address>'\n\n",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("loadb",
               ui_cmd_loadb,
               NULL,
               "load binary via network or kermit protocol.",
               "loadb op address [[hostip:]filename]\n op n for network, s for serial port, f for bootfs.\n\n"
               "",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
    
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64) || defined (_BCM96848_) || ((INC_KERMIT==1) && !defined (_BCM960333_) && (NONETWORK==1))
    brcm_cmd_addcmd("go",
               ui_cmd_go,
               NULL,
               "goto and execute from specefic address.",
               "go [address] \n\n"
               "",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

#endif

#if defined(CFG_DT)
    brcm_cmd_addcmd("ldt",
               ui_cmd_load_dtb,
               NULL,
               "load device tree blob from tftp server.",
               "eg. ldt [hostip:]device_tree_blob_name \n\n"
               "",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

    if( flash_get_flash_type() !=  FLASH_IFC_UNSUP_EMMC )
    {
        brcm_cmd_addcmd("ws",
                ui_cmd_write_loaded_imge,
                NULL,
                "Write whole image (priviously loaded by kermit or JTAG) to flash .",
               "ws [address] [size] : if no address - get address from priviously loaded by kermit \n\n"
                "",
                "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#if defined (_BCM963138_) || defined (_BCM963148_) || defined(_BCM94908_) || defined(_BCM96858_) || defined (_BCM96856_)
        brcm_cmd_addcmd("wp",
                ui_cmd_write_loaded_pmc,
                NULL,
                "Write pmc (previously loaded through JTAG to flash block 0.",
                "wp address size \n\n"
                "",
                "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

#if (INC_SPI_PROG_NAND==1)
        brcm_cmd_addcmd("n",
                ui_cmd_erase_nand,
                NULL,
                "Erase NAND flash",
                "e [a]",
                "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#else
        if(( flash_get_flash_type() ==  FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND ))
        {
            brcm_cmd_addcmd("e",
                    ui_cmd_erase_nand,
                    NULL,
                    "Erase NAND flash",
                    "e [a]",
                    "",
                    BRCM_UI_CMD_TYPE_RESTRICTED);
        }
        else
#endif
        {
            brcm_cmd_addcmd("e",
                    ui_cmd_erase,
                    NULL,
                    "Erase [n]vram or [a]ll flash except bootrom",
                    "e [n/a]",
                    "",
                    BRCM_UI_CMD_TYPE_RESTRICTED);
        }

        brcm_cmd_addcmd("w",
                    ui_cmd_write_whole_image,
                    NULL,
                    "Write the whole image start from beginning of the flash",
                    "eg. w [hostip:]whole_image_file_name [misc partition] ",
                    "",
                    BRCM_UI_CMD_TYPE_NORMAL);
    }

#ifdef CONFIG_ARM64
    brcm_cmd_addcmd("ddw",
           ui_cmd_dump_dwords,
           NULL,
           "Dump dwords.",
           "eg. ddw address_in_hex length_in_decimal",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

    brcm_cmd_addcmd("dw",
           ui_cmd_dump_words,
           NULL,
           "Dump words.",
           "eg. dw address_in_hex length_in_decimal",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("dh",
           ui_cmd_dump_halfwords,
           NULL,
           "Dump half-words.",
           "eg. dh address_in_hex length_in_decimal",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("db",
           ui_cmd_dump_bytes,
           NULL,
           "Dump bytes.",
           "eg. db address_in_hex length_in_decimal",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    brcm_cmd_addcmd("dm",
           ui_cmd_dump_mem,
           NULL,
           "Dump memory or registers.",
           "eg. dm address_in_hex length_in_decimal",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

#if defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM96858_)
    brcm_cmd_addcmd("swrr",
           swrr,
           NULL,
           "Read SWREG",
           "eg. power_supply_in_hex, reg_in_hex",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("swrw",
           swrw,
           NULL,
           "Write SWREG",
           "eg. power_supply_in_hex, reg_in_hex val_in_hex",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
   
    brcm_cmd_addcmd("swrd",
           swrd,
           NULL,
           "Dump SWREG",
           "eg. power_supply_in_hex of 4_for_all",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
   
    brcm_cmd_addcmd("swrp",
           swrp,
           NULL,
           "Enumerate  power supplies ",
           "no params",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
#endif
   
    brcm_cmd_addcmd("sm",
           ui_cmd_set_mem,
           NULL,
           "Set memory or registers.",
           "eg. sm address_in_hex value_in_hex size_4_2_or_1",
           "",
           BRCM_UI_CMD_TYPE_RESTRICTED);
    brcm_cmd_addcmd("kernp",
           ui_cmd_set_kernp,
           NULL,
           "Updates extra bootloader parameter for kernel. To end input enter // consecutively at any time then press <enter>",
           "Parameters must have the following form: <space (optional for first param)><lval> = <rval> <space><lval>=<rval> ... <space><lval>=<rval> where lval is contiguous set of chars",
           " kernp  ",
           BRCM_UI_CMD_TYPE_RESTRICTED);
    brcm_cmd_addcmd("meminfo",
           ui_cmd_meminfo,
           NULL,
           "Display CFE System Memory",
           "",
           "",
           BRCM_UI_CMD_TYPE_NORMAL);
#if (NONETWORK==0)
    brcm_cmd_addcmd("phy",
           ui_cmd_phy,
           NULL,
           "Set memory or registers.",
#ifndef _BCM96858_
           "eg. phy address_in_hex reg_in_hex [value_in_hex]",
#else
           lport_phy_usage,
#endif
           "",
#endif
           BRCM_UI_CMD_TYPE_RESTRICTED);

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)

    brcm_cmd_addcmd("dn",
               ui_cmd_dump_nand,
               NULL,
               "Dump NAND contents along with spare area",
               "eg. dn [block] [page] [total pages to view] or dn [address] [total pages to view]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("fb",
               ui_cmd_find_bad_blocks,
               NULL,
               "Find NAND bad blocks",
               "eg. fb",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("comp",
               ui_cmd_compare_blocks,
               NULL,
               "Compare NAND blocks",
               "eg. comp [source block 1] [source block 2] [number of blocks to compare]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("find",
               ui_cmd_find_string,
               NULL,
               "Find string in NAND",
               "eg. find [start block] [end block] [string]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

    if( flash_get_flash_type() !=  FLASH_IFC_UNSUP_EMMC )
    {
        brcm_cmd_addcmd("x",
                ui_cmd_set_partition_size,
                NULL,
                "Change extra partitions size",
                "",
                "",
                BRCM_UI_CMD_TYPE_RESTRICTED);
        brcm_cmd_addcmd("erase_misc_parti",
                ui_cmd_erase_misc_partition,
                NULL,
                "Erase misc partition",
                "e.g. erase_misc <partition_number>\n partition_number - 1,2 or 3",
                "",
                BRCM_UI_CMD_TYPE_RESTRICTED);
    }
#if INC_EMMC_FLASH_DRIVER
    brcm_cmd_addcmd("emmcr",
                ui_cmd_emmc_boot_img,
                NULL,
                "run linux image from emmc partition",
                "e.g. emmcbootimg <image number = 1|2, or 0 to auto select img>",
                "",
                BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("emmci",
               ui_cmd_get_emmc_info,
               NULL,
               "Get emmc info",
               "e.g. emmcinfo",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);

    brcm_cmd_addcmd("emmcgi",
               ui_cmd_go_emmc_idle,
               NULL,
               "force emmc to idle state",
               "e.g. emmcgoidle [emmcflash<dev num>.<logical partition name>]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
                              
    brcm_cmd_addcmd("emmcsetimgrepart",
               ui_cmd_emmc_allow_img_update_repartition,
               NULL,
               "Allow next image update to repartition flash if necessary",
               "e.g. emmcsetimgrepart <0|1>, 0-clears the repart flag, 1-sets the repart flag",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

    brcm_cmd_addcmd("emmcdpw",
               ui_cmd_get_emmc_read_part_words,
               NULL,
               "Read words from emmc logical partition",
               "e.g. emmcdpw [emmcflash<dev num>.<logical partition name>] [hex offset] [num words]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);               

    brcm_cmd_addcmd("emmcspw",
               ui_cmd_get_emmc_write_part_words,
               NULL,
               "Write a word to emmc logical partition",
               "e.g. emmcspw [emmcflash<dev num>.<logical partition name>] [hex offset] [hex value]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);                              

    brcm_cmd_addcmd("emmcep",
               ui_cmd_get_emmc_erase_part,
               NULL,
               "erase an emmc logical partition",
               "e.g. emmcerase [emmcflash<dev num>.<logical partition name>]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);                              

    brcm_cmd_addcmd("emmcei",
               ui_cmd_get_emmc_erase_img,
               NULL,
               "erase all partitions belonging to an emmc image",
               "e.g. emmceraseimg <image num> <flag: 1 - erase_cferom_nvram as well>",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);                              

    brcm_cmd_addcmd("emmcea",
               ui_cmd_get_emmc_erase_all,
               NULL,
               "erase all partitions on emmc, and set partition settings to default",
               "e.g. emmceraseall <flag: 1 - do not set default partition settings>",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);                              

    brcm_cmd_addcmd("emmcdbfs",
               ui_cmd_dump_emmc_bootfs,
               NULL,
               "dump the bootfs entries",
               "e.g. emmcdumpbootfs [emmcflash<dev num>.<logical partition name>] [filename-leave blank for all files]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
               
    brcm_cmd_addcmd("emmcfmtgpt",
               ui_cmd_emmc_gpt_fmt,
               NULL,
               "modify emmc GPT partitions",
               "e.g. emmcfmtgpt <bootfsKB rootfsKB dataKB misc1KB misc2KB misc3KB misc4KB>.\n Sizes in KB, size 0 will disable misc partitions",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);               

    brcm_cmd_addcmd("emmcdmpgpt",
               ui_cmd_emmc_gpt_dmp,
               NULL,
               "dump emmc GPT partition config",
               "e.g. emmcfmtgpt",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);               
                              
    brcm_cmd_addcmd("showdevs", 
               ui_cmd_devnames, 
               NULL,
               "Display information about the installed cfe devices.",               
               "e.g. showdevs",
               "",
               BRCM_UI_CMD_TYPE_NORMAL);                                         
#endif

#if (defined (_BCM96838_) || defined (_BCM96848_) || defined (_BCM96858_) || defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
  brcm_cmd_addcmd("sar",
               ui_cmd_runner_show_rings,
               NULL,
               "Show available Runner to CPU rings",
               "eg. sar [start_from]",
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("vrpd",
               ui_cmd_runner_print_pd, 
               NULL,
               "View Ring packet descriptor",
               vrpd_usage_str,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif

#if defined(_BCM96858_) && (NONETWORK == 0)
  brcm_cmd_addcmd("lport_rate",
               ui_cmd_lport_rate,
               NULL,
               "Show/Configure LPORT port rate",
               lport_rate_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("lport_status",
               ui_cmd_lport_status,
               NULL,
               "Show LPORT port status",
               lport_status_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("lport_stats",
               ui_cmd_lport_stats,
               NULL,
               "Show LPORT port statistics",
               lport_stats_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("lport_rgmii",
               ui_cmd_lport_rgmii,
               NULL,
               "Set/Get LPORT RGMII port configuration",
               lport_rgmii_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("lport_serdes_reg",
               ui_cmd_lport_serdes_reg,
               NULL,
               "Read/Write LPORT serders register",
               lport_serdes_reg_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("sbuf",
               ui_cmd_send_buffer,
               NULL,
               "Send Packet from CFE to LAN",
               send_buffer_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("ssl",
               ui_cmd_serdes_set_loopback,
               NULL,
               "Set Serdes loopback mode",
               serdes_set_loopback_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("sds",
               ui_cmd_serdes_diag_status,
               NULL,
               "Get Serdes port status",
               serdes_diag_status_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("sdt",
               ui_cmd_serdes_diag_stats,
               NULL,
               "Get Serdes port stats",
               serdes_diag_stats_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("sdw",
               ui_cmd_serdes_read_reg,
               NULL,
               "Display Serdes register",
               serdes_read_reg_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("ssw",
               ui_cmd_serdes_write_reg,
               NULL,
               "Store Serdes register",
               serdes_write_reg_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("set_port",
               ui_cmd_set_port,
               NULL,
               "Set active network port",
               set_port_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("set_compare",
               ui_cmd_set_compare,
               NULL,
               "Compare all recieved packets on active port",
               set_compare_enable_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif
#if (defined(_BCM96858_) || defined(_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
  brcm_cmd_addcmd("net_stats",
               ui_cmd_net_stats,
               NULL,
               "Get network ports status",
               network_stats_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);

  brcm_cmd_addcmd("set_dump",
               ui_cmd_set_dump,
               NULL,
               "Dump all recieved packets on active port",
               set_dump_enable_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif
#if (defined (_BCM96846_) || defined (_BCM96856_)) && (NONETWORK == 0)
  brcm_cmd_addcmd("unimac_stats",
               ui_cmd_unimac_stats,
               NULL,
               "Show UNIMAC port statistics",
               unimac_stats_usage,
               "",
               BRCM_UI_CMD_TYPE_RESTRICTED);
#endif
        return 0;
}


static int runDelay(int delayCount)
{
    int goAuto = 0;
    
    if (delayCount == 0)
        return goAuto;

    printf("*** Press any key to stop auto run (%d seconds) ***\n", delayCount);
    printf("Auto run second count down: %d", delayCount);

    cfe_sleep(CFE_HZ/8);        // about 1/4 second

    while (1)
    {
        printf("\b%d", delayCount);
        cfe_sleep(CFE_HZ);        // about 1 second
        if (console_status())
            break; 
        if (--delayCount == 0)
        {
            goAuto = 1;
            break;
        }
    }
    printf("\b%d\n", delayCount);

    return goAuto;
}

static void preboot_init(void)
{
#if defined (_BCM96838_)
    if (NULL != BpGetVoipDspConfig(0)) 
    {
        //Ensure the HVG and SLIC are in IDLE state
        HVG->reg_hvg_zar_if_xmt_data = 0x00000000; /* Set SLIC to FWD-ACTIVE state */
        APM->reg_cha_hybal_config &= ~YFLTR_EN; /* Disable Y-filt */
        APM->reg_chb_hybal_config &= ~YFLTR_EN; /* Disable Y-filt */
        APM->reg_ring_config_1 |= (RING_STOP_IMMED_A | RING_STOP_IMMED_B); /* Stop ringing ref signal */    
        HVG->reg_hvg_cha_const_volt_goal = 0x0578; /* Set proper hvg voltage goal */
        HVG->reg_hvg_chb_const_volt_goal = 0x0578; /* Set proper hvg voltage goal */
        HVG->reg_hvg_cha_reg_2 |= HVG_MODE_ONHOOK_FIXED; /* HVG to fixed mode */
        HVG->reg_hvg_chb_reg_2 |= HVG_MODE_ONHOOK_FIXED; /* HVG to fixed mode */
    }
#endif
    return;
}

void bcm63xx_run_ex(int breakIntoCfe, int skip_check_memcfg, int autorun)
{
    int ret = 0, i = 0;
    unsigned short gpio;

    printSysInfo();
#if defined(_BCM963138_) || defined(_BCM963148_) || defined(_BCM94908_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM947189_)
    {
       unsigned int memcfg;
       if (skip_check_memcfg == 0 && validateMemoryConfig(NVRAM_RP, &memcfg)) {
           NVRAM_UPDATE_FIELD(ulMemoryConfig, &memcfg, sizeof(memcfg));
           printf("\nMEMORY CONFIG UPDATED...  REBOOTING\n");
           cfe_sleep(2*CFE_HZ);
           softReset(0);
        }
    }
#endif
/* Exception case handling to detect A0 image on B0 and vice versa */
#if defined(_BCM963138_)
    {
        unsigned int chipRev = UtilGetChipRev();
        if (chipRev == 0xA0)
        {
            ui_init_bcm63xx_cmds();
            g_invalid_img_rev = 1;
            printf("\nERROR !! Chip Rev <0x%X> does not match Image Rev; Please boot from previous image and load correct rev image\n",chipRev);
            return;
        }
    }
#endif /* _BCM963138_ */

#if defined(_BCM963158_)
    {
        unsigned int chipRev = UtilGetChipRev();
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
        if (chipRev != 0xA0)
#else
        if (chipRev == 0xA0)
#endif
        {
            printf("\nERROR !! Chip Rev <0x%X> does not match Image Rev <0x%X>; Please use CFE Early Abort Menu to boot previous image and load correct rev image !!\n\n\n",chipRev, CONFIG_BRCM_CHIP_REV);
            breakIntoCfe = 1;
        }
    }
#endif /* _BCM963158_ */

#if (NONETWORK==0) && !defined (_BCM96838_) && !defined (_BCM960333_) && !defined (_BCM96848_) && \
    !defined(_BCM96858_) && !defined (_BCM96846_) && !defined(_BCM947189_) && !defined(_BCM96856_)
    bcm_ethsw_init();
#endif

#ifdef BCM_OPTEE
    /* Run OPTEE, drop to interactive console if failed */
    if (optee_init() == -1) {
       breakIntoCfe = 1;
    }
#endif

	if (!breakIntoCfe && autorun && runDelay(bootInfo.bootDelay)) {
        run_info_t ar = { 0 };
        preboot_init();

        ar.boot_opt = bootInfo.runFrom;
        ar.ip = bootInfo.hostIp;
        if (ar.boot_opt == 'c') {
            ar.img_id[id_img_boot] = bootInfo.hostFileName;
            ar.img_id[id_img_ramdisk] = bootInfo.ramfsFileName;
            ar.img_id[id_img_dtb] = bootInfo.dtbFileName;
            ar.laddr = bootInfo.rdAddr;
        } else if (ar.boot_opt == 'h') {
            ar.img_id[id_img_boot] = bootInfo.hostFileName;
        } 
        ret = auto_run(&ar);        // never returns
    }

    /* We are here because of a user key press OR if *
     * auto_run failed to find a valid linux image.  */
    
    /* Enable console commands */
    ui_init_bcm63xx_cmds();
#if (defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM963381_)|| defined(_BCM963138_) \
     || defined(_BCM96856_) || defined(_BCM96846_)) && ( (INC_NAND_FLASH_DRIVER==1) || (INC_EMMC_FLASH_DRIVER==1) )
    ui_init_otp_cmds();
#endif

    /* Enable Network */
#if (NONETWORK==0)
    if (g_in_cfe == 0) {
        board_netdevice_init();
        enet_init();
        g_in_cfe = 1;
    }
#endif

    g_in_cfe = 1;

   if (!breakIntoCfe && autorun && (ret != 0)
   ) {
       printf("\nEnter Rescue Mode ...\n\n");

       if (BpGetBootloaderPowerOnLedGpio(&gpio) != BP_SUCCESS)
           gpio = 20 | BP_ACTIVE_LOW | BP_LED_USE_GPIO;  // use WPS LED instead
       /* Wait forever for an image */
       while ((ret = ui_docommand("w 255.255.255.255:ASUSSPACELINK")) == CFE_ERR_TIMEOUT || ret == -1) {
           if (i%2 == 0)
               setLed(gpio, LED_OFF);
           else
               setLed(gpio, LED_ON);
           i++;
           if (i==0xffffff)
               i = 0;
       }
   }
}
void bcm63xx_run(int breakIntoCfe, int autorun)
{
	bcm63xx_run_ex(breakIntoCfe, 0, autorun);
}
