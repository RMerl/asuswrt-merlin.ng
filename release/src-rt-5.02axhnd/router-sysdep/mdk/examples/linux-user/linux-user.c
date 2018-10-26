/******************************************************************************
 * $Id: linux-user.c,v 1.7 Broadcom SDK $
 * $Copyright: Copyright 2009 Broadcom Corporation.
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
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef BRCM_CMS_BUILD
#include "cms.h"
#include "cms_eid.h"
#include "cms_util.h"
#include "cms_core.h"
#include "cms_msg.h"
#include "cms_log.h"
#endif

/* CDK Package Headers */
#include <cdk_config.h>
#include <cdk/cdk_device.h>
#include <cdk/cdk_readline.h>
#include <cdk/cdk_shell.h>
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

#include "ethswctl_api.h"

#ifdef BRCM_CMS_BUILD
void *msgHandle=NULL;
#endif

typedef struct spi_dev_ids {
    int spi_id;
  int chip_id;
} spi_device;

void link_poll_function(void *);
void poll_mdkshell(void *);

int linux_user_spi_read(void *dvc, uint32_t addr, uint8_t *data, uint32_t len)
{
    spi_device *spi = (spi_device *)dvc;
    return bcm_spi_read(spi->spi_id, spi->chip_id, addr, (char *)data, (int)len);
}

int linux_user_spi_write(void *dvc, uint32_t addr, const uint8_t *data, uint32_t len)
{
    spi_device *spi = (spi_device *)dvc;
    return bcm_spi_write(spi->spi_id, spi->chip_id, addr, (char *)data, (int)len);
}

int linux_user_mdio_read(void *dvc, uint32_t addr, uint8_t *data, uint32_t len)
{
    return bcm_pseudo_mdio_read(addr, (char *)data, (int)len);
}

int linux_user_mdio_write(void *dvc, uint32_t addr, const uint8_t *data, uint32_t len)
{
    return bcm_pseudo_mdio_write(addr, (char *)data, (int)len);
}

int linux_user_ubus_read(void *dvc, uint32_t addr, uint8_t *data, uint32_t len)
{
    return bcm_reg_read(addr, (char *)data, (int)len);
}

int linux_user_ubus_write(void *dvc, uint32_t addr, const uint8_t *data, uint32_t len)
{
    return bcm_reg_write(addr, (char *)data, (int)len);
}

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

FILE *fd1, *fd2;

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
    if (strcmp(buf, "quit") == 0) {
        fclose(fd1);
        fclose(fd2);
        remove("/var/mdksh");
    }
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
_bus_flags(uint32_t bus_type)
{
    uint32_t flags = 0;

    if (bus_type == MBUS_SPI) {
        flags = CDK_DEV_MBUS_SPI;
    } else if (bus_type == MBUS_MDIO) {
        flags = CDK_DEV_MBUS_MII;
    } else {
        flags = CDK_DEV_MBUS_PCI;
    }

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
   uint32_t base_addr, uint32_t bus_type, void *dvc, uint32_t pbmp, uint32_t phypbmp)
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
    id.config_pbmp = pbmp;
    id.phy_pbmp = phypbmp;


    /*
     * Setup the device vectors structure
     */
    memset(&dv, 0, sizeof(dv));

    if (bus_type == MBUS_SPI) {
      dv.dvc = dvc;
        dv.read = &linux_user_spi_read;
        dv.write = &linux_user_spi_write;
    } else if (bus_type == MBUS_MDIO) {
        dv.read = &linux_user_mdio_read;
        dv.write = &linux_user_mdio_write;
    } else if (bus_type == MBUS_MMAP){
        dv.read = &linux_user_mmap_read;
        dv.write = &linux_user_mmap_write;
    } else {
        dv.read = &linux_user_ubus_read;
        dv.write = &linux_user_ubus_write;
        /* mmap the physical address into our virtual address space */
        /* and provide this as the base address for the device */
//        dv.base_addr = _mmap(base_addr, 64*1024);
    }

    /*
     * Create the CDK Device Context
     */
    rc = cdk_dev_create(&id, &dv, _bus_flags(bus_type));
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
 * Including Internal and External PHY Support in your system.
 * This enables PHY programming in the BMD and requires the PHY package.
 */

#if BMD_CONFIG_INCLUDE_PHY == 1

#include <phy/phy_drvlist.h>

#if 0
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
#endif

static phy_driver_t *phy_drv_list[] = {
#if BRCM_EXT_SWITCH_TYPE == 53101
    &bcm53101_drv,
#endif
#if BRCM_EXT_SWITCH_TYPE == 53115
    &bcm53115_drv,
#endif
#if defined BCM_PHY_54616
    &bcm54616_drv,
#endif
    &bcm_generic_drv,
    NULL
};
#endif /* BMD_CONFIG_INCLUDE_PHY */

/* TBD: to use the defines in config.h files.  */
#define MAX_SWITCH_PORTS 8
int thread_lock = 1;
#if BMD_CONFIG_INCLUDE_PHY == 1
typedef struct link_poll_info_s {
    int unit;
    unsigned int phypbmp;
    unsigned int vendor_id;
} link_poll_info_t;
#endif

static int num_switches = 0;
static unsigned int vendor_id, device_id, rev_id;
int main(int argc, char* argv[])
{
    int rv, unit, port;
    spi_device spi_dev;
    int bus_type;
    unsigned int base_addr = 0;
    unsigned int spi_id, spi_cid;
    unsigned int pbmp, phypbmp;
#if BMD_CONFIG_INCLUDE_PHY == 1
    pthread_t linkpoll_thread[BMD_CONFIG_MAX_UNITS];
    int retval;
    link_poll_info_t poll_info;
#endif

#ifdef BRCM_CMS_BUILD
    SINT32 c, logLevelNum;
    CmsRet ret;
    CmsLogLevel logLevel=DEFAULT_LOG_LEVEL;

    cmsLog_initWithName(EID_SWMDK, argv[0]);

    while ((c = getopt(argc, argv, "v:m:")) != -1)
    {
       switch(c)
       {
          case 'v':
          logLevelNum = atoi(optarg);
          if (logLevelNum == 0) logLevel = LOG_LEVEL_ERR;
          else if (logLevelNum == 1) logLevel = LOG_LEVEL_NOTICE;
          else logLevel = LOG_LEVEL_DEBUG;
          cmsLog_setLevel(logLevel);
          break;

          default:
          break;
       }
    }

    if ((ret = cmsMsg_initWithFlags(EID_SWMDK, 0, &msgHandle)) != CMSRET_SUCCESS)
    {
        cmsLog_cleanup();
        exit(-1);
    }
#endif

    for(unit = 0; unit < BMD_CONFIG_MAX_UNITS; unit++) {
        bcm_get_switch_info(unit, &vendor_id, &device_id, &rev_id, &bus_type,
            &spi_id, &spi_cid, &pbmp, &phypbmp);
        if (rev_id != 0) {
            fprintf(stderr, "Note: forcing rev_id to zero for now \n");
            rev_id = 0;
        }
        if ((device_id & 0xfff0) == 0x6810) {
            fprintf(stderr, "Note: Loading 6816 MDK driver for %X chip \n", device_id);
            device_id = 0x6816;
        }
        if (device_id == 0x6369) {
            fprintf(stderr, "Note: Loading 6368 MDK driver for 6369 chip \n");
            device_id = 0x6368;
        }
        if (bus_type != MBUS_NONE) {
            spi_dev.spi_id = spi_id;
            spi_dev.chip_id = spi_cid;
            /* Create the specified cdk device */
            rv = _create_cdk_device(vendor_id, device_id, rev_id, base_addr,
                bus_type, (void *)&spi_dev, pbmp, phypbmp);
            if (rv < 0)
                return -1;
            num_switches++;
        }
    }


#if BMD_CONFIG_INCLUDE_PHY == 1
    bmd_phy_probe_init(bmd_phy_probe_default, phy_drv_list);
#endif

    printf("Switch MDK: num_switches = %d\n", num_switches);
    for(unit = 0; unit < num_switches; unit++) {
        if(CDK_DEV_EXISTS(unit)) {
            bmd_attach(unit);
            bmd_init(unit);
        } else {
            fprintf(stderr, "CDK Device was created but CDK_DEV_EXISTS failed \n");
        }
    }

    if (num_switches <= 0)
       goto cleanup_n_exit;

#if BMD_CONFIG_INCLUDE_PHY == 1
    for(unit = 0; unit < num_switches; unit++) {
        if(CDK_DEV_EXISTS(unit)) {
            pbmp = CDK_DEV_CONFIG_PBMP(unit);
            phypbmp = CDK_DEV_PHY_PBMP(unit);
            for (port = 0; port < MAX_SWITCH_PORTS; port++) {
                if (phypbmp & (1 << port))
                    bmd_phy_probe(unit, port);
            }
            if (phypbmp) {
                poll_info.unit = unit;
                poll_info.phypbmp = phypbmp;
                poll_info.vendor_id = CDK_DEV_VENDOR_ID(unit);
                retval = pthread_create(&linkpoll_thread[unit], NULL,
                   (void *)&link_poll_function, (void *)&poll_info);
            }
        }
    }
#endif

    if (num_switches > 1) {
        for(unit = 0; unit < num_switches-1; unit++) {
            printf("Initializing unit %d in unmanaged mode \n", unit);
            bmd_switching_init(unit);
        }
    }

#if 0
    retval = pthread_create(&vportstatuspoll_thread, NULL,
                (void *)&vportstatus_poll_function, NULL);
#endif

    poll_mdkshell(NULL);

#if BMD_CONFIG_INCLUDE_PHY == 1
    for(unit = 0; unit < num_switches; unit++) {
      if(CDK_DEV_EXISTS(unit)) {
        phypbmp = CDK_DEV_PHY_PBMP(unit);
        if (phypbmp)
            pthread_join(linkpoll_thread[unit], NULL);
      }
    }
#endif

cleanup_n_exit:
#ifdef BRCM_CMS_BUILD
    cmsMsg_cleanup(&msgHandle);
    cmsLog_cleanup();
#endif
    return 0;
}

#if BMD_CONFIG_INCLUDE_PHY == 1
void link_poll_function(void *ptr)
{
    int unit, port, link, an_done, speed, duplex, prev_link;
    unsigned int prev_link_status_map = 0, mask, phypbmp;
    link_poll_info_t *pinfo = (link_poll_info_t *)ptr;

    unit = pinfo->unit;
    phypbmp = pinfo->phypbmp;
    printf("Switch MDK link poll thread: unit=%d; phypbmp=0x%x\n", unit, phypbmp);
    while(thread_lock) {
        for (port = 0; port < MAX_SWITCH_PORTS; port++) {
            mask = 1 << port;
            if (phypbmp & mask) {
                bmd_phy_link_get(unit, port, &link, &an_done);
                prev_link = (prev_link_status_map & mask) >> port;
                if (link != prev_link) {
                    bmd_phy_speed_get(unit, port, &speed);
                    bmd_phy_duplex_get(unit, port, &duplex);
                    bcm_set_linkstatus(unit, port, link, speed, duplex);
                    if (pinfo->vendor_id == 0x6300)
                        bmd_port_mode_update(unit, port);
                    prev_link_status_map &= ~mask;
                    prev_link_status_map |= (link << port);
                }
            }
        }
        bcm_ethsw_kernel_poll();
        sleep(1);
    }
    printf("link poll exiting... \n");
}
#endif

void poll_mdkshell(void *ptr)
{
    char msg[10];
    int fd, status, numread = 0;
    status = mkfifo("/var/mdkshell", S_IRUSR | S_IWGRP | S_IWOTH);
    fd = open("/var/mdkshell", O_RDONLY, S_IRUSR);
    memset((void *)msg, 0, sizeof(msg));

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

    while(thread_lock)
    {
        numread = read(fd, msg, sizeof(msg));
        if (numread) {
          if (strncmp(msg, "SHELL", 5) == 0) {
            fd1 = fopen("/dev/ttyS0", "r+");
            if (fd1 == 0) {
              printf("Cannot open /dev/tty\n");
              continue;
            }
            fd2 = fopen("/var/mdksh", "a+");
            if (fd2 == 0) {
                printf("error creating the /var/mdksh file \n");
                continue;
            }
            cdk_shell("MDK", _readline);
          } else if (strncmp(msg, "KILL", 4) == 0) {
            thread_lock = 0;
            break;
          } else if (strncmp(msg, "ENVPORTS", 7) == 0) {
              bmd_init(num_switches - 1);
          } else if (strncmp(msg, "DISVPORTS", 8) == 0) {
              bmd_switching_init(num_switches - 1);
          } else {
              printf("Received %s \n", msg);
          }
        }
        sleep(1);
    }

    printf("closing the fd \n");
    close(fd);
}
