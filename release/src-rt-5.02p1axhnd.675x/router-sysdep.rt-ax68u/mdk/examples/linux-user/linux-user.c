/******************************************************************************
 * $Id: linux-user.c,v 1.7 Broadcom SDK $
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 ******************************************************************************
 *
 * Linux User mode CDK/BMD Application
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>

/* CDK Package Headers */
#include <cdk_config.h>
#include <cdk/cdk_device.h>
#include <cdk/cdk_readline.h>
#include <cdk/cdk_shell.h>
#include <cdk/arch/xgs_cmds.h>
#include <cdk/shell/chip_cmds.h>

#ifdef CDK_CONFIG_ARCH_ROBO_INSTALLED
#include <cdk/arch/robo_cmds.h>
#endif
#ifdef CDK_CONFIG_ARCH_XGS_INSTALLED
#include <cdk/arch/xgs_cmds.h>
#endif

/* BMD Package Headers */
#include <bmd_config.h>
#include <bmd/bmd.h>
#include <bmd/bmd_phy_ctrl.h>
#include <bmd/shell/bmd_cmds.h>

/* PHY Package Headers */
#include <phy_config.h>
#include <phy/phy_drvlist.h>


/*******************************************************************************
 *
 * Terminal support
 *
 * The functions below are used by the readline terminal
 * interface, and should work on most POSIX systems.
 *
 ******************************************************************************/

#ifdef SYS_HAS_TTY
#include <termios.h>

/*
 * This function ensures that the TTY returns typed characters
 * immediately and has character echo disabled.
 */
static int
_tty_set(int reset)
{
    static struct termios old, new;
    static int tty_initialized;

    if (reset) {
        /* Restore TTY settings */
        if (tty_initialized) {
            tcsetattr(0, TCSADRAIN, &old);
        }
        return 0;
    }

    if (tcgetattr(0, &old) < 0) {
        perror("tcgetattr");
    } else {
        /* Save terminal settings */
        new = old;
        /* Disable echo and buffering */
        new.c_lflag &= ~(ECHO | ICANON | ISIG);
        new.c_iflag &= ~(ISTRIP | INPCK);
        new.c_cc[VMIN] = 1;
        new.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSADRAIN, &new) < 0) {
            perror("tcsetattr");
        } else {
            tty_initialized = 1;
        }
    }
    return 0;
}

/* Read character from TTY */
static int
_tty_getchar(void)
{
    return getchar();
}

/* Send character to TTY */
static int
_tty_putchar(int ch)
{
    return putchar(ch);
}
#endif



/*
 * Function: _usleep
 *
 * Purpose:
 *   Function used by the BMD package and PHY packages for sleeps. 
 *   Specifed as the value of BMD_SYS_USLEEP and PHY_SYS_USLEEP. 
 *
 */
int
_usleep(uint32_t usecs)
{
    return usleep(usecs); 
}

/*
 * Function: _mmap
 *
 * Purpose:
 *    Helper function for address mmapping. 
 * Parameters:
 *    p - physical address start
 *    size - size of region
 * Returns:
 *    Pointer to mapped region, or NULL on failure. 
 */
static uint32_t *
_mmap(off_t p, int size) 
{  
    static int _memfd = -1; 
    uint32_t* va = NULL; 

    if(p == 0 && _memfd != -1) {
        /* Close the descriptor */
        close(_memfd); 
        _memfd = -1; 
        return NULL; 
    }

    if(_memfd == -1) {
        /* Open the descriptor */
        if ((_memfd = open("/dev/mem", 
                           O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {
            perror("could not open /dev/mem: ");
            exit(1); 
        }
    }

    /* Map the address */
    va = mmap(NULL, size, PROT_READ|PROT_WRITE, 
             MAP_SHARED, _memfd, p);

    if(va == 0 || va == MAP_FAILED) {
        fprintf(stderr, "could not mmap physical address 0x%x: ", (unsigned int)p); 
        perror(""); 
        exit(1); 
    }

    return va; 
}

/*
 * Function:
 *      _readline
 * Purpose:
 *      Retrieve an input line from the user. 
 * Returns:
 *      Pointer to the readline buffer
 * Notes:
 *
 *      If SYS_HAS_TTY is defined:
 *          The built-in readline library is used.
 *
 *      If SYS_HAS_TTY is not defined:
 *          A normal read on stdio is used. 
 *
 */

char *
_readline(const char *prompt, char *buf, int max)
{
#ifdef SYS_HAS_TTY
    _tty_set(0);
    cdk_readline(_tty_getchar, _tty_putchar, prompt, buf, max);
    _tty_set(1);
#else
    int len;

    write(0, prompt, strlen(prompt));
    if ((len = read(0, buf, max)) <= 0) {
        buf[0] = 0;
    } else {
        buf[len-1] = 0;
    }
#endif
    return buf;
}


/*
 * Function:
 *      _bus_flags
 * Purpose:
 *      Get bus endian settings
 * Parameters:
 *      None
 * Returns:
 *      CDK bus endian device flags.
 * Notes:
 *      These settings are platform dependent and are specified by the build
 *      This data is passed to cdk_dev_create(). 
 */

/* These defines must be specified for this sample to work in a real system */
#if !defined(SYS_BE_PIO) || !defined(SYS_BE_PACKET) || !defined(SYS_BE_OTHER)
#error PCI bus endian flags SYS_BE_PIO, SYS_BE_PACKET and SYS_BE_OTHER not defined
#endif

static uint32_t 
_bus_flags(void)
{
    uint32_t flags = CDK_DEV_MBUS_PCI;

#if SYS_BE_PIO == 1
    flags |= CDK_DEV_BE_PIO;
#endif
#if SYS_BE_PACKET == 1
    flags |= CDK_DEV_BE_PACKET;
#endif
#if SYS_BE_OTHER == 1
    flags |= CDK_DEV_BE_OTHER;
#endif
    return flags;
}



/*
 * Function:
 *      _create_cdk_device
 * Purpose:
 *      This function creates a CDK device context
 * Parameters:
 *      vendor_id:      Vendor Id
 *      device_id:      Device Id
 *      rev_id:         Revision Id
 *      base_addr:      Physical Base Address of the device
 * Returns:
 *      Unit number >= 0 on success, 
 *      -1 on failure. 
 */

static int
_create_cdk_device(uint32_t vendor_id, uint32_t device_id, uint32_t rev_id, 
                   uint32_t base_addr)
{
    cdk_dev_id_t id; 
    cdk_dev_vectors_t dv; 
    int rc; 

    /* 
     * Setup the device identification structure 
     */
    memset(&id, 0, sizeof(id)); 
    id.vendor_id = vendor_id; 
    id.device_id = device_id; 
    id.revision = rev_id; 
    

    /*
     * Setup the device vectors structure 
     */
    memset(&dv, 0, sizeof(dv)); 
    
    /* mmap the physical address into our virtual address space */
    /* and provide this as the base address for the device */
    dv.base_addr = _mmap(base_addr, 64*1024); 
    
    /*
     * Create the CDK Device Context
     */
    rc = cdk_dev_create(&id, &dv, _bus_flags()); 
    if(rc < 0) {
        fprintf(stderr, "cdk_dev_create: could not create device 0x%x:0x%x:0x%x @ 0x%x: %s (%d)\n", 
                vendor_id, device_id, rev_id, base_addr, 
                CDK_ERRMSG(rc), rc); 
        exit(1); 
    }

    /* 
     * This unit is ready to use
     */
    return rc; 
}
    
/*
 * Function:
 *      _help
 * Purpose:
 *      Generate a help message for this example
 * Parameters:
 *      None
 * Returns:
 *      Nothing
 */
static void
_help(FILE* fp)
{
    fprintf(fp, "\nLinux Usermode Sample MDK Application\n\n"); 
    fprintf(fp, "usage:\n\n"); 
    fprintf(fp, "[-u | -unit | --unit] <vendor:device:revision@base_address>\n"); 
    fprintf(fp, "    Specifies a device to be created. All values should be in hexadecimal.\n"); 
    fprintf(fp, "    Multiple '-unit' options will create multiple devices\n\n"); 
    fprintf(fp, "    Example:\n"); 
    fprintf(fp, "        --unit 0x14e4:0xb504:0x10@0xbfff0000\n\n"); 
    fprintf(fp, "-n | -nounits | --nounits\n"); 
    fprintf(fp, "    Startup without creating any devices\n\n"); 
    fprintf(fp, "[-h | -help | --help]\n"); 
    fprintf(fp, "    This help message\n\n"); 
}




/*
 * Including Internal and External PHY Support in your system. 
 * This enables PHY programming in the BMD and requires the PHY package. 
 */

#if BMD_CONFIG_INCLUDE_PHY == 1

#include <phy/phy_drvlist.h>

/*
 * Supported PHY drivers
 */
static phy_driver_t *phy_drv_list[] = {
    &bcmi_fusioncore_xgxs_drv,
    &bcmi_fusioncore12g_xgxs_drv,
    &bcmi_unicore_xgxs_drv,
    &bcmi_unicore16g_xgxs_drv,
    &bcmi_hypercore_xgxs_drv,
    &bcmi_hyperlite_xgxs_drv,
    &bcmi_xgs_serdes_drv,
    &bcmi_nextgen_serdes_drv,
    &bcmi_nextgen65_serdes_drv,
    &bcmi_combo_serdes_drv,
    &bcmi_combo65_serdes_drv,
    &bcmi_hyperlite_serdes_drv,
    &bcmi_unicore16g_serdes_drv,
    &bcm5228_drv,
    &bcm5238_drv,
    &bcm5248_drv,
    &bcm53314_drv,
    &bcm5395_drv,
    &bcm5421_drv,
    &bcm5461_drv,
    &bcm5464_drv,
    &bcm54684_drv,
    &bcm5482_drv,
    &bcm5488_drv,
    &bcm54980_drv,
    &bcm8705_drv,
    &bcm8706_drv,
    NULL
};

#endif /* BMD_CONFIG_INCLUDE_PHY */



int     
main(int argc, char* argv[])
{
    
    /*
     * Parse given arguments for unit information
     */
    int i; 
    int unit; 

    if(argc == 1) {
        _help(stderr); 
        exit(1); 
    }

    for(i = 1; i < argc; i++) {

        if(!strcmp(argv[i], "-h") ||
           !strcmp(argv[i], "-help") ||
           !strcmp(argv[i], "--help")) {
            _help(stdout); 
            continue;
        }
        else if(!strcmp(argv[i], "-u") ||
                !strcmp(argv[1], "-unit") ||
                !strcmp(argv[i], "--unit")) {
            
            uint32_t vendor_id; 
            uint32_t device_id; 
            uint32_t revision_id; 
            uint32_t base_addr; 

            if(argv[i+1]) {
                if(sscanf(argv[i+1], "0x%x:0x%x:0x%x@0x%x", 
                          &vendor_id, &device_id, &revision_id, &base_addr) != 4) {
                    fprintf(stderr, "error: malformed option: %s %s\n", argv[i], argv[i+1]); 
                    exit(1); 
                }
            }   
            else {
                printf("error: device specified with %s\n", argv[i]); 
            }

            /* Create the specified cdk device */
            _create_cdk_device(vendor_id, device_id, revision_id, base_addr); 

            /* Skip the device argument */
            i++; 
        }
        else if(!strcmp(argv[i], "-n") ||
                !strcmp(argv[i], "-nounits") ||
                !strcmp(argv[i], "--nounits")) {
            /* Just startup with no units attached */
            break; 
        }
        else {
            fprintf(stderr, "unrecognized option '%s'\n", argv[i]); 
            exit(1);
        }
    }
                

    /*
     * All specified CDK devices have been created. 
     *
     * Attach the BMD driver library to each unit. 
     */

#if BMD_CONFIG_INCLUDE_PHY == 1
    bmd_phy_probe_init(bmd_phy_probe_default, phy_drv_list);
#endif

    for(unit = 0; unit < BMD_CONFIG_MAX_UNITS; unit++) {
        if(CDK_DEV_EXISTS(unit)) {
            bmd_attach(unit); 
        }       
    }   
    
    /* Initialize CDK Shell */
    cdk_shell_init(); 

    /* Add RoboSwitch architecture commands if installed */
#ifdef CDK_CONFIG_ARCH_ROBO_INSTALLED
    cdk_shell_add_robo_core_cmds(); 
#endif

    /* Add XGS architecture commands if installed */
#ifdef CDK_CONFIG_ARCH_XGS_INSTALLED
    cdk_shell_add_xgs_core_cmds(); 
#endif

    /* Add BMD commands */
    bmd_shell_add_bmd_cmds(); 

    cdk_shell("MDK", _readline); 

    return 0;
}
    


    
