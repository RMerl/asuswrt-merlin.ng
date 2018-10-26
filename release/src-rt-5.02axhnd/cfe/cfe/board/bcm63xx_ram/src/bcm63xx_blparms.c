/***************************************************************************
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
 ***************************************************************************/
#include "bcm63xx_util.h"
#include "flash_api.h"
#include "bcm63xx_blparms.h"
#include "bcm63xx_auth.h"
#include "bcm63xx_nvram.h"
#include "bsp_config.h"
#if INC_EMMC_FLASH_DRIVER
#include "dev_emmcflash.h"
#endif

/* Boot Loader Parameter Functions
 *
 * These functions create a buffer of boot loader parameters that are passed
 * to Linux. The functions add new parameters to the buffer and put the buffer
 * into a location that can be accessed by Linux when it boots.  A parameter
 * has the format: <name>=<value>'\0'.  The last parameter has two NULL
 * termination characters.
 */

#define XPARMS_LEN ((BLPARMS_LEN*sizeof(char))/2)

extern unsigned long cfe_get_sdram_size(void);
static char blparms_buf[BLPARMS_LEN];
static char *xparms_buf;
static int xparms_len;
static int blparms_len;

static int xparms_init(void);

//static int blparms_add_extra_parms(char* parm, char parm_op, char parm_delimiter, char parm_data_delimiter, char escape_char);
void blparms_init(void)
{
    blparms_buf[0] = '\0';
    blparms_len = 0;
    xparms_init();
#if defined(CFG_DT)
    dtb_init(DTB_ID_CFE, (void*)CFG_DTB_ADDRESS, CFG_DTB_MAX_SIZE);
#endif
}

#define ITOA_LEN 11

void blparms_set_int(char *name, int value)
{
    if (strlen(name) + ITOA_LEN + 1 + blparms_len < (BLPARMS_LEN/2)) {
        blparms_len += sprintf(blparms_buf + blparms_len, "%s=%d",name,value) + 1;
    }
}

void blparms_set_str(char *name, const char *value)
{
    if (strlen(name) + strlen(value) + 1 + blparms_len < (BLPARMS_LEN/2)) {
        blparms_len += sprintf(blparms_buf + blparms_len, "%s=%s",name,value) + 1;     
    }
}

static int xparms_init()
{
    if (!xparms_buf) {
        xparms_buf = KMALLOC(XPARMS_LEN,0);
        if (!xparms_buf) {
            return -1;
        }
        memset(xparms_buf,'\0',XPARMS_LEN);
    }
    return 0;
}

#if XPARMS_DEINIT
static void xparms_deinit(void)
{
    if (!xparms_buf) {
        KFREE(xparms_buf);
        xparms_buf = NULL;
    }
}
#endif

static inline char* xparms_get(void)
{
    return xparms_buf;
}

static inline unsigned int xparms_get_len(void)
{
    return xparms_len;
}

/*
   returns total string length of the input
*/
static int xparms_input(char* buf,
                      unsigned int len,
                      char tkn1, 
                      char tkn2)
{
      unsigned int offs = 0,read_len;
      while(1) {
         read_len = console_readline("", buf+offs, len-offs);
         offs += read_len;
         if ((buf[offs-2]&buf[offs-1]) == tkn2) {
              offs -= 2;
              break;
         }
         if (offs+1 >= len) {
             break;
         }
         buf[offs++] = tkn1;
      }
      return offs;
}

/* advances to the end of the sequence of 'c' 
   in/out **p  pointer to the last char+1 of the sequence.
   0 if sequence is found 
*/
static inline int chars_seq(char** p, char* se, char c)
{
   int cnt = 0;
   char* s = *p;
   while (s < se) {
       if (*s != c) {
           break;
       }
       cnt++;
       s++; 
   }
   *p = s;
   return (cnt==0);
}
/*
   returns pointer to the next char if current is equal to 'c'
   returns 0 if matched
*/
static inline int char_ifeq(char** p, char* se, char c)
{
       char* s = *p; 
       if (s < se) { 
           if (*s == c) {
               *p = s+1;
               return 0;
           }
       }
       return 1; 
}

/* Advances to the end sequence formed as of not equal to any 2 chars in 'ch' until is the one is met */
static inline int not2chars_seq(char** p, char* se, char ch[2])
{
      int cnt = 0;
      char* s = *p;
      while (s < se) {
             if (*s == ch[0] || *s == ch[1]) {
                 break;
             } else {
                 cnt++;
             }
             s++; 
      }
      *p = s;
      return (cnt==0);
}
/* return number of tuples encountered*/
static int xparms_chek_tuples(char* s, char* se, char tkn1, char tkn2)
{
    char t[ ] = {tkn1, tkn2};
    int touples = 0; 
    /* searches for token0 sequence until first occurence of non token0 is met*/
    chars_seq(&s, se, t[0]); 
    while (s < se) {
       /* searches for not token1 & not token2 until either of tokens met*/ 
       if (not2chars_seq(&s, se, t)) {
           return 0;
       }
       chars_seq(&s, se, t[0]); 
       /* move to the next char if current is equal to 'c'*/ 
       if (char_ifeq(&s, se, t[1])) {
           return 0; 
       }
       chars_seq(&s, se, t[0]);

       if (not2chars_seq(&s, se, t)) {
           return 0;
       }
       chars_seq(&s, se, t[0]); 
       touples++;
   }
   return touples; 
}

/*
Enforces the following relationship of pairs [pair]
<BoS><opt tkn2>[<lval valid chars><tkn1><rval valid chars><tkn2|EoS>]
returns 0 for properly formed pairs
*/
static int xparms_verify(char* s,int len,
                         char tkn1, char tkn2)
{
     return xparms_chek_tuples(s, s+len, tkn2, tkn1)? 0:1;
}
/*
Forms the following string from user input:
<BLKERNPARM>="<payload>"
payload must be triples of the following form:
        <space or begin of the string><lvalue>=<rvalue><space><lvalue>=<rvalue>...<space><lvalue>=<rvalue>
rvalue = can be any char except '=' although,  processing of non-alpha and non-digit numbers 
may not be usable in procfs entries
lvalue = only consequitive input chars except spaces and '=' are interpreted as consistent lvalue 

*/
int blparms_add_extra_parms(char* prefix, char parm_op, char parm_delimiter, 
                            char parm_data_delimiter, char escape_char,
                            char* parm)
{
      unsigned int offs;
      unsigned int prefix_len, parm_len=0;
      char* xbuf = xparms_get();
      
      if (!xbuf || !prefix) {
          return -1;
      }
      if (parm) {
          parm_len = strlen(parm);
      }
      
      prefix_len = strlen(prefix);
      
      if (parm_len + sizeof(parm_op) + sizeof(parm_delimiter) + 
          sizeof(parm_data_delimiter)*2 + parm_len + sizeof((char)'\0')+64 > XPARMS_LEN-1) {
          return -1;
      }
      offs = prefix_len + sizeof(parm_op) + sizeof(parm_data_delimiter);
      strcpy(xbuf, prefix);
      xbuf[prefix_len] = parm_op;
      xbuf[prefix_len + sizeof(parm_op)] = parm_data_delimiter;
      if (parm_len) {
          strcpy(xbuf+offs, parm);
      } else {   
          parm_len = xparms_input(xbuf+offs, XPARMS_LEN-(offs+2),
                                  parm_delimiter,escape_char);
      }
      if (xparms_verify(xbuf+offs, parm_len, parm_op, parm_delimiter)) {
          *(xbuf) = '\0';
          return -1;
      }
      offs += parm_len;
      xbuf[offs++] = parm_data_delimiter;
      xparms_len = offs;
      
      return 0;
}


#if defined(CFG_DT)

void set_reserved_memory(void)
{
    uint64_t mem_end = cfe_get_sdram_size();
    char dt_node_name[64];
#if !defined (_BCM960333_)
    uint64_t rsrv_mem_required = 8*SZ_1M;
    uint64_t adsl_rsv_size = 0, adsl_base_addr = 0; 
    unsigned long sdram_size = cfe_get_sdram_size();

#if !defined(_BCM963381_) && !defined(_BCM963268_)
    int rdp_size[2], dhd_size[3], i, rdp_def_size, chk_dt;
    uint8_t bp_dhd_size;
    uint64_t dt_rdp_size = 0, dt_dhd_size = 0, dt_rdp_addr, dt_dhd_addr;
    char* rdp_str[] = {PARAM1_BASE_ADDR_STR, PARAM2_BASE_ADDR_STR};
    char* dhd_str[] = {DHD_BASE_ADDR_STR, DHD_BASE_ADDR_STR_1,DHD_BASE_ADDR_STR_2};
#endif

#if !defined(_BCM96858_)
    if( mem_end > 256*SZ_1M )
        mem_end = 256*SZ_1M;
#endif

    sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, ADSL_BASE_ADDR_STR);
    if (!dtb_getprop_reg(DTB_ID_CFE, DT_RSVD_NODE_STR, dt_node_name,
                          &adsl_base_addr,  &adsl_rsv_size)) {
        if (adsl_rsv_size && adsl_rsv_size < mem_end) {
              rsrv_mem_required += adsl_rsv_size;
              mem_end -= adsl_rsv_size;
              dtb_set_reserved_memory(DTB_ID_CFE, dt_node_name, mem_end, adsl_rsv_size);
        } else {
              dtb_del_reserved_memory(DTB_ID_CFE, dt_node_name);        
        } 
    }

#if defined(_BCM963381_) || defined(_BCM963268_)
   
    if ( sdram_size < rsrv_mem_required) {
           printf("Not enough memory to reserve 0x%x bytes!\n", rsrv_mem_required);
    }

#else
    /* reseved memory have to be within 256MB */
    rdp_size[0] = NVRAM.allocs.alloc_rdp.param1_size;
    rdp_size[1] = NVRAM.allocs.alloc_rdp.param2_size;
    dhd_size[0] = NVRAM.alloc_dhd.dhd_size[0];
    dhd_size[1] = NVRAM.alloc_dhd.dhd_size[1];
    dhd_size[2] = NVRAM.alloc_dhd.dhd_size[2];

    /* The priority for rdp reserved memory is as following:
       1. Use nvram value if sticky bit is set and value is not 0xff (for rdp param2 only since the max for param1 is over 127)
       2. if DT value present
         2.1 Use nvram value if it is not 0xff and larger than DT value
         2.2 otherwise use DT value
       3. otherwise use default value if nvarm value is 0xff 
       4. otherwise use nvram value */
    for( i = 0; i < 2; i++ ) {
    	chk_dt = 1;
        if( i == 0 )
            rdp_def_size = DEFAULT_NVRAM_RDP_PARAM1;
        else
            rdp_def_size = DEFAULT_NVRAM_RDP_PARAM2;

        /* check param1 and param2 value */
        if( rdp_size[i] == 0xff )
            rdp_size[i] = rdp_def_size;
        /* if sticky bits, use nvram setting */
        else if ( (i == 1) && (rdp_size[i] & 0x80)) {
            rdp_size[i] = rdp_size[i] & 0x7f;
            chk_dt = 0;
	}

        rdp_size[i] = ALIGN(SZ_2M, rdp_size[i]*SZ_1M);

	if (chk_dt) {
            /* check the device tree value, use that if it is large than nvram value */
            sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, rdp_str[i]);
            if (!dtb_getprop_reg(DTB_ID_CFE, DT_RSVD_NODE_STR, dt_node_name,
                          &dt_rdp_addr,  &dt_rdp_size)) {
                if (dt_rdp_size > rdp_size[i]) {
                    printf("rdp param%d value 0x%x in device tree larger than nvram value 0x%x. Use device tree value!\n", i+1, (uint32_t)dt_rdp_size, rdp_size[i]);
                    rdp_size[i] = dt_rdp_size;
                }
            }
        }
    }
    rsrv_mem_required += rdp_size[0] + rdp_size[1];

    /* Select dhd value. The order of selection is as following:
       - if nvram setting has stick bit set, use nvram setting always
       - if board parameter has specifiy dhd value, use board parameter setting
       - otherwise use the larger value of nvram and device tree setting */
    for( i = 0; i < 3; i++ ) {
        chk_dt = 0;
        /* if sticky bits, alway use nvram setting */
        if( dhd_size[i] != 0xff && (dhd_size[i]&0x80) == 0x80) {
            dhd_size[i] = dhd_size[i]&0x7f;
	}
        /* use board parameter value if it is specified in bp */
        else if( BpGetDHDMemReserve(i, &bp_dhd_size) == BP_SUCCESS ) {
            dhd_size[i] = bp_dhd_size;
	}
        /* use regular nvram or device tree setting, whichever is bigger */
        else {
            chk_dt = 1;
            /* if any one is never set, set to zero to disable dhd reserve */
            if( dhd_size[i] == 0xff )
                dhd_size[i] = 0x0;
        }

        dhd_size[i] = dhd_size[i]*SZ_1M;

        /* check the device tree value, use that if it is large than nvram value */
        if( chk_dt ) {
            sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, dhd_str[i]);
            if (!dtb_getprop_reg(DTB_ID_CFE, DT_RSVD_NODE_STR, dt_node_name,
                &dt_dhd_addr,  &dt_dhd_size)) {
                if( dt_dhd_size > dhd_size[i] ) {
                    printf("dhd param%d value 0x%x in device tree larger than nvram value 0x%x. Use device tree value!\n", i+1, (uint32_t)dt_dhd_size, dhd_size[i]);
                    dhd_size[i] = dt_dhd_size;
                }
            }
        }

        rsrv_mem_required += dhd_size[i];
    }

    /* check if we have enough memory */
    if ( sdram_size < rsrv_mem_required)
    {
        rsrv_mem_required -= (dhd_size[0] + dhd_size[1] + dhd_size[2]);
        dhd_size[0] = dhd_size[1] = dhd_size[2] = 0x0;
       /* If RDP is enabled, try to use the default
        * RDP reserved memory size and try again */
        rsrv_mem_required -= (rdp_size[0] + rdp_size[1]);
        rdp_size[0] = RDP_PARAM1_DEF_DDR_SIZE;
        rdp_size[1] = RDP_PARAM2_DEF_DDR_SIZE;
        rsrv_mem_required += rdp_size[0] + rdp_size[1];
        if (sdram_size < rsrv_mem_required) {
            printf("Not enough memory to reserve 0x%x bytes!\n", rsrv_mem_required);
            return;
        }
    }

    for( i = 0; i < 2; i++ ) {
        sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, rdp_str[i]);
        if (!dtb_set_reserved_memory(DTB_ID_CFE, dt_node_name, ALIGN_FLR(SZ_2M, mem_end - rdp_size[i]), rdp_size[i])) {
            /* RDP reserved memory has to be 2MB-aligned */
            mem_end = ALIGN_FLR(SZ_2M, mem_end - rdp_size[i]);
        }
    }

    for( i = 0; i < 3; i++ ) {
        sprintf(dt_node_name, "%s%s%d", DT_RSVD_PREFIX_STR, "dhd", i);
        if( dhd_size[i] != 0 ) {
            sprintf(dt_node_name, "%s%s%d", DT_RSVD_PREFIX_STR, "dhd", i);
            if (!dtb_set_reserved_memory(DTB_ID_CFE, dt_node_name, ALIGN_FLR(SZ_2M, mem_end - dhd_size[i]), dhd_size[i])) {
      	        /* DHD reserved memory has to be 2MB-aligned */
                mem_end = ALIGN_FLR(SZ_2M, mem_end - dhd_size[i]);
            }
        }
        else
            dtb_del_reserved_memory(DTB_ID_CFE, dt_node_name);        
    }
#endif

#else
    uint64_t plc_rsv_size = 0, plc_base_addr = 0;

    if( mem_end > 256*SZ_1M )
        mem_end = 256*SZ_1M;

    sprintf(dt_node_name, "%s%s", DT_RSVD_PREFIX_STR, PLC_BASE_ADDR_STR);
    if (!dtb_getprop_reg(DTB_ID_CFE, DT_RSVD_NODE_STR, dt_node_name,
                          &plc_base_addr,  &plc_rsv_size)) {
        if (plc_rsv_size && plc_rsv_size < mem_end) {
              mem_end -= plc_rsv_size;
              dtb_set_reserved_memory(DTB_ID_CFE, dt_node_name, mem_end, plc_rsv_size);
        } else {
              dtb_del_reserved_memory(DTB_ID_CFE, dt_node_name);        
        }
    }
#endif
}

extern int erase_psi;

void dtb_install(void)
{
    uint64_t rel_addr, *target_addr;
    NVRAM_DATA nvramData;
    int len;
    char *bootCfeVersion=NULL;
    int cfe_ver_size=0;


    /* defaults are pointing to statically defined dtb array*/

    dtb_set_memory(DTB_ID_CFE, cfe_get_sdram_size());
    
    set_reserved_memory();

    /* all the secondary cpus has the same release address */

#if defined (_BCM96858_) || defined (_BCM94908_) || defined(_BCM963158_) || defined(_BCM96856_)
    if( dtb_get_cpu_rel_addr(DTB_ID_CFE, &rel_addr, 1) == 0 )
    {
        target_addr = (uint64_t*)(CFG_BOOT_AREA_ADDR + 8);
        *target_addr = rel_addr;
    }
#else
    (void)rel_addr; (void)target_addr;
#endif
#if defined (_BCM96858_) || defined (_BCM94908_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM963138_)
    dtb_set_nr_cpus(DTB_ID_CFE);
#endif
#if defined(_BCM963138_)
    unsigned int nr_cpus=0;
    extern int cpu_limited;
    if(get_nr_cpus(&nr_cpus) == 0 )
    {
        if(nr_cpus == 1 )
            dtb_preappend_bootargs(DTB_ID_CFE, "nosmp ");
    }
    if(cpu_limited == 1)
        dtb_preappend_bootargs(DTB_ID_CFE, "nosmp ");
    if(nr_cpus==1 || cpu_limited == 1)
        cfe_set_cpu_freq(666);
#endif

    if (dtb_set_blparms(DTB_ID_CFE, blparms_buf, blparms_len)) {
        printf("ERROR: Failed to add blparms to DTB\n");
    }

    /* Retrieve NVRAM parameters */
    NVRAM_COPY_TO(&nvramData);


#if INC_EMMC_FLASH_DRIVER
    dtb_preappend_bootargs(DTB_ID_CFE, "rootwait ");
    dtb_set_chosen_root(DTB_ID_CFE, get_emmc_chosen_root());
#endif

#if INC_EMMC_FLASH_DRIVER
    get_emmc_boot_cfe_version(&bootCfeVersion, &cfe_ver_size);
#else
    get_flash_boot_cfe_version(&bootCfeVersion, &cfe_ver_size);
#endif

    if(bootCfeVersion != NULL)
    {
        /* Pass the boot cfe version in dtb */
        printf("Appending CFE version to dtb, ret:%d\n",
              dtb_append_data_blob(DTB_ID_CFE, BRCM_CFEVER_PROP,
               (char*)bootCfeVersion, cfe_ver_size));
    }

 /* Pass NVRAM in dtb to kernel in binary blob format */ 
    if(dtb_get_prop(DTB_ID_CFE, "/", BRCM_NVRAM_PROP, &len) == NULL)
    {
        printf("Appending NVRAM to dtb, ret:%d\n",
            dtb_append_data_blob(DTB_ID_CFE, BRCM_NVRAM_PROP,
            (char*)&nvramData, sizeof(NVRAM_DATA)));
    }
    else
    {
       printf("dtb already has NVRAM appended\n");
    }

    if (erase_psi)
        dtb_preappend_bootargs(DTB_ID_CFE, "erase_psi ");

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (hash_block_start) {
        unsigned char hash[SHA256_S_DIGEST8];
        int ret;
        unsigned int content_len;
        printf("look for hash for %s\n",HASH_BLOCK_ROOTFS_ENTRY_NAME);
        // printf("start of hash block %x %x %x %x\n",hash_block_start[0],hash_block_start[1],hash_block_start[2],hash_block_start[3]);
        ret = find_boot_hash(&content_len, hash, hash_block_start, HASH_BLOCK_ROOTFS_ENTRY_NAME); 
        if (ret == 0)  {
            printf("failed to find hash for %s\n",HASH_BLOCK_ROOTFS_ENTRY_NAME);
            die();
        } else {
            printf("got hash for %s\n",HASH_BLOCK_ROOTFS_ENTRY_NAME);
            dtb_append_data_blob(DTB_ID_CFE, BRCM_ROOTFS_SHA256_PROP,
                (char*)hash, SHA256_S_DIGEST8);
            dtb_append_data_blob(DTB_ID_CFE, BRCM_ROOTFS_IMGLEN_PROP,
                (char*)&content_len, sizeof(content_len));

        }

    }
#endif // CONFIG_CFE_SUPPORT_HASH_BLOCK

    dtb_done(DTB_ID_CFE);

    return;
}
#elif defined(CFG_ATAG)
static void atag_install(void)
{
    struct tag *atag_ptr = (struct tag *)ARM_ATAG_LOC;
    /* add_coretag has to be the first command */
    atag_ptr = add_coretag(atag_ptr, 1, ATAG_CORE_DEF_PAGESIZE, 0xff);

    atag_ptr = add_memtag(atag_ptr, 0x0, mem_totalsize*1024);

    /* for cmdline, kernel option should be set to "CONFIG_CMDLINE_EXTEND",
     * which means whatever we type here is an extension of what we already
     * have in kernel's CONFIG_CMDLINE */
//    atag_ptr = add_cmdlinetag(atag_ptr, "console=ttyS0,115200 earlyprintk debug",
//    39);

    if (bootInfo.runFrom == 'f')
    {
        // for flash, we don't need to add in any tag, because the root fs
        // mounting is done in our kernel
        //atag_ptr = add_cmdlinetag(atag_ptr, "root=/dev/mtdblock2 rw", 23);
    }
    else if (bootInfo.runFrom == 'c')
    {
        atag_ptr = add_initrdtag(atag_ptr, bootInfo.rdAddr, 0x1000000,
                bootInfo.rdAddr & 0x80000000);
    }
    else /* if ((bootInfo.runFrom == 'h') || (bootInfo.runFrom == 'r')) */
    {
        // FIXME!! the following is just a dummy
        //atag_ptr = add_cmdlinetag(atag_ptr, "root=/dev/nfs nfsroot=h:/ "
        //"ip=192.168.1.1:192.168.1.2::255.255.255.0::eth0:0ff "
        //"rw", 12);
    }

#if defined (_BCM963138_) || defined (_BCM963148_)
    blparms_buf[blparms_len++] = '\0';
    atag_ptr = add_blparmtag(atag_ptr, blparms_buf, blparms_len);
    atag_ptr = add_rdpsizetag(atag_ptr, NVRAM.allocs.alloc_rdp.param1_size,
            NVRAM.allocs.alloc_rdp.param2_size);

    atag_ptr = add_dhdsizetag(atag_ptr, NVRAM.alloc_dhd.dhd_size);
#endif
    /* complete_tag has to be the last command */
    atag_ptr = complete_tag(atag_ptr);
}


void atag_install_initrd_only(unsigned int initrd_start_addr)
{
    struct tag *atag_ptr = (struct tag *)ARM_ATAG_LOC;

    /* add_coretag has to be the first command */
    atag_ptr = add_coretag(atag_ptr, 1, ATAG_CORE_DEF_PAGESIZE, 0xff);

    atag_ptr = add_memtag(atag_ptr, 0x0, mem_totalsize*1024);

    atag_ptr = add_initrdtag(atag_ptr, initrd_start_addr, 0x800000,
                initrd_start_addr & 0x80000000);
    /* complete_tag has to be the last command */
    atag_ptr = complete_tag(atag_ptr);
}
#else

extern unsigned long fmw_arg0;
static void legacy_install(unsigned long *loadaddr)
{
    /* loadaddr is the memory address where Linux is loaded.  Copy the boot
     * loader parameter buffer before loadaddr.
     */
    if( loadaddr )
    {
        unsigned long bl_parms_addr; 
        /* Old NAND flash releases use the word at loadaddr - 1.
         * Skip for backward compatibility.
         */
        if(( flash_get_flash_type() == FLASH_IFC_NAND ) || ( flash_get_flash_type() ==  FLASH_IFC_SPINAND ))
            loadaddr--;

        blparms_buf[blparms_len++] = '\0';
        bl_parms_addr = (unsigned long)loadaddr - 8 - ((blparms_len+4) & ~3); 
        *((unsigned long*)bl_parms_addr - 1) = BLPARMS_MAGIC; 
        fmw_arg0 = (unsigned long)((unsigned long*)bl_parms_addr - 1);
        memcpy((void*)bl_parms_addr, blparms_buf, blparms_len);
    }
}
#endif

void blparms_install(unsigned long *loadaddr)
{
    char* xbuf = xparms_get();
    if (xbuf) {
        lib_strncpy(blparms_buf+blparms_len, xbuf, xparms_len);
        blparms_len += xparms_len;
    }
#if defined(CFG_DT)
extern unsigned long fmw_arg0;
#ifdef CONFIG_BRCM_IKOS
    printf("ikos run load dtb from fixed memory location directly...\n");
    dtb_prepare(DTB_ID_CFE, DTB_PREPARE_FDT, NULL, (unsigned char*)0x2000000, 4096);
#else
    dtb_prepare(DTB_ID_CFE, DTB_PREPARE_FDT_DEF, NULL, NULL, 0);
#endif
    dtb_install();
    if (loadaddr) {
        fmw_arg0 = (unsigned long)dtb_get_fdt(DTB_ID_CFE);
    }  
#elif defined(CFG_ATAG)
    atag_install();
#else
    if( loadaddr )
        legacy_install(loadaddr);
#endif

    return;
}
