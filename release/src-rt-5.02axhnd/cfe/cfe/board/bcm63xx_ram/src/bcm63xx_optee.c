/*  *********************************************************************
    *
    <:copyright-BRCM:2018:proprietary:standard
    
       Copyright (c) 2018 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>
    ********************************************************************* */

#include "bcm63xx_dtb.h"
#include "bcm63xx_util.h"

#define MAX_IMAGE_LEN 0x800000

extern void cfe_save_context(void);
extern void cfe_restore_context(void);
void *memcpy(void *dest, const void *src, size_t n);
// XXX can't include "cfe_iocb.h" due to unused cfe_iocb_t typedef
#if defined(CONFIG_ARM64)
unsigned int g_atf_addr = 0;
#endif
unsigned int g_optee_addr = 0;

// load optee image(s) from boot filesystem
int optee_init(void)
{
#if defined(CONFIG_ARM64)
    static const char binpath[] = "armtf.lz";
#endif
    static const char binpath_next[] = "optee.lz";
    static const char dtbpath[] = "optee.dtb";
    unsigned int dtb_len = CFG_DTB_MAX_SIZE;
    unsigned int img_len = MAX_IMAGE_LEN;  /* hard coded for now. should get from optee dts */
    void* optee_dtb;
    unsigned int img_option = BOOT_FILE_LOAD_OPT_DEFAULT|BOOT_FILE_LOAD_OPT_FLUSHCACHE;

    /* Allocate memory to load optee dtb */
    optee_dtb = KMALLOC(dtb_len, 0);
    /* Load uncompressed optee device tree blob */
    if (optee_dtb ) {
       char node[64];
       int nodeoffset = 0;
       const char *propdata;
       int proplen = 0;

       dtb_init(DTB_ID_OPTEE, optee_dtb, dtb_len);
       if (dtb_prepare(DTB_ID_OPTEE, DTB_PREPARE_FDT_DEF, dtbpath, NULL, 0) == 0 ) {

           /* the following code must be fixed to use the standard dtb api such as dtb_getprop_reg
           to get any node's reg property(address, len). The optee dtb also need to fix atf, optee
           memory reg property using the standard dtb  address, len, addr_cell_size, len_cell_size */ 

           /* Look for optee memory node in the dtb */
           sprintf(node,"%s%s",DT_ROOT_NODE,DT_MEMORY_NODE);
           nodeoffset = fdt_path_offset(optee_dtb, node);
           if (nodeoffset == -FDT_ERR_NOTFOUND) {
              printf ("ERROR: Failed to obtain OPTEE memory node\n");
              dtb_done(DTB_ID_OPTEE);
              goto err_out;
           }
           /* Extract memory region information from device tree */
           propdata = fdt_getprop(optee_dtb, nodeoffset, "reg", &proplen);
#if defined(CONFIG_ARM64)
           g_atf_addr = fdt32_to_cpu(*(uint32_t*)propdata);
#endif
           g_optee_addr = fdt32_to_cpu(*(uint32_t*)(propdata + sizeof(uint32_t)));
        }
        dtb_done(DTB_ID_OPTEE);
    }
    else {
        goto err_out;
    }

#if defined(CONFIG_ARM64)
    /* Load compressed ATF image */
    if (!g_atf_addr || cfe_load_boot_file(binpath, (void*)(long)g_atf_addr, &img_len, img_option|BOOT_FILE_LOAD_OPT_COMPRESS) != 0)
      goto err_out;
#endif
    img_len = MAX_IMAGE_LEN;
    /* Load compressed OPTEE image */
    if (!g_optee_addr || cfe_load_boot_file(binpath_next, (void*)(long)g_optee_addr, &img_len, img_option|BOOT_FILE_LOAD_OPT_COMPRESS) != 0)
      goto err_out;

    /* Run OPTEE */
    cfe_secureos();

    KFREE(optee_dtb);
    return 0;

 err_out:
    printf ("ERROR: Failed to load secure os \n");
    KFREE(optee_dtb);
    return -1;
}
