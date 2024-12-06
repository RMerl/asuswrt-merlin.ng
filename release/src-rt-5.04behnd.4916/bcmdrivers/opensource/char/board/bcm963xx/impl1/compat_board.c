/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
*
*    Copyright (c) 2016 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/

/***************************************************************************
* File Name  : compat_board.c
*
* Description: The compat layer for the board driver.
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/compat.h>
#include "compat_board.h"
#include <board.h>
#include <linux/slab.h>

#include "board_ioctl.h"

typedef struct compatBoardIoctParms
{
    compat_uptr_t string;
    compat_uptr_t buf;
    compat_int_t strLen;
    compat_int_t offset;
    compat_int_t action;
    compat_int_t result;
} COMPAT_BOARD_IOCTL_PARMS;

typedef struct compatBoardMemaccessIoctlParms
{
    compat_int_t op;
    compat_int_t space;
    compat_u64 address;
    compat_int_t size; // 1, 2, or 4
    compat_int_t count; // number of items
    compat_uptr_t buf;
} COMPAT_BOARD_MEMACCESS_IOCTL_PARMS;

#define COMPAT_BOARD_IOCTL_FLASH_WRITE                 _IOWR(BOARD_IOCTL_MAGIC, 0, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_FLASH_READ                  _IOWR(BOARD_IOCTL_MAGIC, 1, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_DUMP_ADDR                   _IOWR(BOARD_IOCTL_MAGIC, 2, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_MEMORY                  _IOWR(BOARD_IOCTL_MAGIC, 3, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_MIPS_SOFT_RESET             _IOWR(BOARD_IOCTL_MAGIC, 4, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_LED_CTRL                    _IOWR(BOARD_IOCTL_MAGIC, 5, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_PSI_SIZE                _IOWR(BOARD_IOCTL_MAGIC, 6, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_SDRAM_SIZE              _IOWR(BOARD_IOCTL_MAGIC, 7, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_ID                      _IOWR(BOARD_IOCTL_MAGIC, 8, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_CHIP_ID                 _IOWR(BOARD_IOCTL_MAGIC, 9, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_CHIP_REV                _IOWR(BOARD_IOCTL_MAGIC, 10, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_CFE_VER                 _IOWR(BOARD_IOCTL_MAGIC, 11, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BASE_MAC_ADDRESS        _IOWR(BOARD_IOCTL_MAGIC, 12, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_MAC_ADDRESS             _IOWR(BOARD_IOCTL_MAGIC, 13, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_RELEASE_MAC_ADDRESS         _IOWR(BOARD_IOCTL_MAGIC, 14, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_NUM_ENET_MACS           _IOWR(BOARD_IOCTL_MAGIC, 15, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_NUM_ENET_PORTS          _IOWR(BOARD_IOCTL_MAGIC, 16, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_MONITOR_FD              _IOWR(BOARD_IOCTL_MAGIC, 17, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_WAKEUP_MONITOR_TASK         _IOWR(BOARD_IOCTL_MAGIC, 18, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 19, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 20, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_UNSET_TRIGGER_EVENT         _IOWR(BOARD_IOCTL_MAGIC, 21, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_WLAN_ANT_INUSE          _IOWR(BOARD_IOCTL_MAGIC, 22, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_SES_LED                 _IOWR(BOARD_IOCTL_MAGIC, 23, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_CS_PAR                  _IOWR(BOARD_IOCTL_MAGIC, 25, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_GPIO                    _IOWR(BOARD_IOCTL_MAGIC, 26, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_FLASH_LIST                  _IOWR(BOARD_IOCTL_MAGIC, 27, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BACKUP_PSI_SIZE         _IOWR(BOARD_IOCTL_MAGIC, 28, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_SYSLOG_SIZE             _IOWR(BOARD_IOCTL_MAGIC, 29, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_SHUTDOWN_MODE           _IOWR(BOARD_IOCTL_MAGIC, 30, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_STANDBY_TIMER           _IOWR(BOARD_IOCTL_MAGIC, 31, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_BOOT_IMAGE_OPERATION        _IOWR(BOARD_IOCTL_MAGIC, 32, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_TIMEMS                  _IOWR(BOARD_IOCTL_MAGIC, 33, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_ALLOC_MAC_ADDRESSES         _IOWR(BOARD_IOCTL_MAGIC, 36, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_RELEASE_MAC_ADDRESSES       _IOWR(BOARD_IOCTL_MAGIC, 37, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_INIT              _IOWR(BOARD_IOCTL_MAGIC, 38, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_READ              _IOWR(BOARD_IOCTL_MAGIC, 39, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_WRITE             _IOWR(BOARD_IOCTL_MAGIC, 40, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_NUM_FE_PORTS            _IOWR(BOARD_IOCTL_MAGIC, 41, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_NUM_GE_PORTS            _IOWR(BOARD_IOCTL_MAGIC, 42, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_NUM_VOIP_PORTS          _IOWR(BOARD_IOCTL_MAGIC, 43, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_SWITCH_PORT_MAP         _IOWR(BOARD_IOCTL_MAGIC, 44, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_EPON_GPIOS              _IOWR(BOARD_IOCTL_MAGIC, 46, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_SET_BITS          _IOWR(BOARD_IOCTL_MAGIC, 47, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_CLEAR_BITS        _IOWR(BOARD_IOCTL_MAGIC, 48, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SPI_SLAVE_WRITE_BUF         _IOWR(BOARD_IOCTL_MAGIC, 49, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_GPIOVERLAYS             _IOWR(BOARD_IOCTL_MAGIC, 50, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BATTERY_ENABLE          _IOWR(BOARD_IOCTL_MAGIC, 51, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_FTTDP_DSP_BOOTER            _IOWR(BOARD_IOCTL_MAGIC, 52, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_MEM_ACCESS                  _IOWR(BOARD_IOCTL_MAGIC, 53, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SET_DYING_GASP_INTERRUPT    _IOWR(BOARD_IOCTL_MAGIC, 54, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BTRM_BOOT               _IOWR(BOARD_IOCTL_MAGIC, 55, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BOOT_SECURE             _IOWR(BOARD_IOCTL_MAGIC, 56, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_SATA_TEST                   _IOWR(BOARD_IOCTL_MAGIC, 57, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_BT_GPIO                     _IOWR(BOARD_IOCTL_MAGIC, 58, COMPAT_BOARD_IOCTL_PARMS)
#define COMPAT_BOARD_IOCTL_GET_BOOT_MFG_SECURE         _IOWR(BOARD_IOCTL_MAGIC, 59, COMPAT_BOARD_IOCTL_PARMS)

#define COMPAT_TO_REG_IOCTL(cmd) ( ((cmd) & (~(IOCSIZE_MASK))) | ((BOARD_IOCTL_FLASH_WRITE) & (IOCSIZE_MASK)) )

#if !defined(CONFIG_BRCM_IKOS)
static int compat_put_board_ioctl_parms( COMPAT_BOARD_IOCTL_PARMS __user *ioctlParms32,
                                         BOARD_IOCTL_PARMS __user *ioctlParms)
{
    int err = 0;
    COMPAT_BOARD_IOCTL_PARMS temp_ioctl_parms;

    temp_ioctl_parms.strLen=ioctlParms->strLen;
    temp_ioctl_parms.offset=ioctlParms->offset;
    temp_ioctl_parms.action=ioctlParms->action;
    temp_ioctl_parms.result=ioctlParms->result;

    err=copy_to_user(ioctlParms32, &temp_ioctl_parms, sizeof(temp_ioctl_parms));


    return err;  
}
#endif

static int compat_get_board_ioctl_parms( COMPAT_BOARD_IOCTL_PARMS __user *ioctlParms32,
                                         BOARD_IOCTL_PARMS __user *ioctlParms)
{
    int err = 0;
    COMPAT_BOARD_IOCTL_PARMS temp_ioctl_parms;


    if( (err=copy_from_user(&temp_ioctl_parms, ioctlParms32, sizeof(temp_ioctl_parms))) == 0)
    {
        ioctlParms->string=compat_ptr(temp_ioctl_parms.string);
        ioctlParms->buf=compat_ptr(temp_ioctl_parms.buf);
        ioctlParms->strLen=temp_ioctl_parms.strLen;
        ioctlParms->offset=temp_ioctl_parms.offset;
        ioctlParms->action=temp_ioctl_parms.action;
        ioctlParms->result=temp_ioctl_parms.result;
    }

    return err;    
}

static int compat_get_board_ioctl_mem_access_parms( COMPAT_BOARD_MEMACCESS_IOCTL_PARMS __user *memAccIoctlParms32,
                                                    BOARD_MEMACCESS_IOCTL_PARMS __user *memAccIoctlParms)
{

    int err = 0;
    COMPAT_BOARD_MEMACCESS_IOCTL_PARMS temp_parms;

    if(! (err=copy_from_user(&temp_parms, memAccIoctlParms32, sizeof(temp_parms)))) {
        memAccIoctlParms->address=temp_parms.address;
        memAccIoctlParms->buf=compat_ptr(temp_parms.buf);
        memAccIoctlParms->op=temp_parms.op;
        memAccIoctlParms->space=temp_parms.space;
        memAccIoctlParms->size=temp_parms.size;
        memAccIoctlParms->count=temp_parms.count;
    }

    return err;    
}

long compat_board_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret;
    int err;
    void *user_val_string_bufp = NULL;
    void *temp_string_bufp = NULL;

    if (!filp->f_op->unlocked_ioctl)
        return -ENOENT;

    switch (cmd) {
        case COMPAT_BOARD_IOCTL_MEM_ACCESS:
        {
            COMPAT_BOARD_MEMACCESS_IOCTL_PARMS __user *memAccIoctlParms32;
            BOARD_MEMACCESS_IOCTL_PARMS memAccIoctlParms;

            memAccIoctlParms32 = compat_ptr(arg);

            err = compat_get_board_ioctl_mem_access_parms(memAccIoctlParms32, &memAccIoctlParms);
            if (err)
                return err;
            if (memAccIoctlParms.op == BOARD_MEMACCESS_IOCTL_OP_FILL)
                memAccIoctlParms.count=1;

            ret = memaccess(&memAccIoctlParms, memAccIoctlParms.size * memAccIoctlParms.count);
            return ret ? ret : err;
        }
        break;

        default:
        {
            COMPAT_BOARD_IOCTL_PARMS __user *ioctlParms32;
            BOARD_IOCTL_PARMS ioctlParms;

            ioctlParms32 = compat_ptr(arg);
            err = compat_get_board_ioctl_parms(ioctlParms32, &ioctlParms);
            if (err)
                return err;

            /* Copy over data from union ioctlParms.string OR ioctlParms.value */
            if( ioctlParms.string && ioctlParms.strLen )
            {
                /* allocate one extra byte due to legacy reasons */
                temp_string_bufp = kmalloc(ioctlParms.strLen+1, GFP_KERNEL);
                if( temp_string_bufp )
                {
                    memset(temp_string_bufp, 0, ioctlParms.strLen+1);
                    if (copy_from_user((void*)temp_string_bufp, (void*)(ioctlParms.string), ioctlParms.strLen) == 0) {
                        user_val_string_bufp = (void*)ioctlParms.string;
                        ioctlParms.string = temp_string_bufp;
                    }
                    else
                    {
                        kfree(temp_string_bufp);
                        printk("board_compat_ioctl: command %x failed copying string/value from userspace , cmd %d .\n",cmd,_IOC_NR(cmd));
                        ret = -EFAULT;
                    }
                }
                else
                {
                    printk("board_compat_ioctl: command %x failed to allocate memory for string/value , cmd %d .\n",cmd,_IOC_NR(cmd));
                    ret = -EFAULT;
                }
            }

#if !defined(CONFIG_BRCM_IKOS)
            ret=board_ioctl_cmd_handler(filp, COMPAT_TO_REG_IOCTL(cmd), arg, &ioctlParms);

            /* Copy to user ioctlParms and memory pointed to by ioctlParms.string OR ioctlParms.value */
            if( ioctlParms.string && ioctlParms.strLen )
            {
                temp_string_bufp = ioctlParms.string;
                ioctlParms.string = user_val_string_bufp;
                if(copy_to_user((void*)ioctlParms.string, temp_string_bufp, ioctlParms.strLen))
  	            printk("__copy_to_user failed %s:%d\n", __FILE__, __LINE__);
                kfree(temp_string_bufp);
            }
            err = compat_put_board_ioctl_parms(ioctlParms32, &ioctlParms);
            return ret ? ret : err;
#else
            return -ENOENT;
#endif
        }
        break;
    }
}
