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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <bcmnetlink.h>

#include <bcmtypes.h>
#include <board.h>
#include <bcm_dgasp.h>

#include <shared_utils.h>
#include <flash_api.h>
#include <flash_common.h>

#include "bcm_otp.h"
#include "board_ioctl.h"
#include "board_util.h"
#include "board_image.h"
#include "board_wl.h"
#include <linux/of.h>

static DEFINE_MUTEX(ioctlMutex);

extern int compat_kerSysBcmSpiSlaveInit(int dev);
extern int compat_kerSysBcmSpiSlaveRead(int dev, unsigned int addr, unsigned int *data, unsigned int len);
extern int compat_kerSysBcmSpiSlaveWrite(int dev, unsigned int addr, unsigned int data, unsigned int len);
extern int compat_kerSysBcmSpiSlaveWriteBuf(int dev, unsigned int addr, unsigned int *data, unsigned int len, unsigned int unitSize);

void kerSysNvRamGetBoardId(char *boardId, int len)
{
    if( eNvramGet(NVRAM_SZBOARDID,boardId, len) < 0)
    {
        memset(boardId, '\0', len);
    } 
}
EXPORT_SYMBOL(kerSysNvRamGetBoardId);
                                        
long board_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    long rt;
    
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif


    mutex_lock(&ioctlMutex);
    rt = board_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&ioctlMutex);
    return rt;
    
}

static void legacy_ioctl_alert(char * msg)
{
   printk("ERROR: IOCTL %s attempted -- should be done in library, not IOCTL\n", msg);
}

static int dt_number_of_emacs_get(void)
{
    struct device_node *np, *child, *childp;
    int count = -1;

    if ((np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
    {
        count = 0;
        for_each_available_child_of_node(np, child)
        {
            for_each_available_child_of_node(child, childp)
            {
                if (of_get_property(childp, "phy-handle", NULL))
                    count++;
            }
        }
    }

    if (np)
        of_node_put(np);

    return count;
}

int memaccess(BOARD_MEMACCESS_IOCTL_PARMS *parms, int blen)
{

    unsigned char* kbuf=NULL;
    int ret=0;

    kbuf = (unsigned char *)kmalloc(blen, GFP_KERNEL);
    if (kbuf == NULL) {
        ret = -EFAULT;
        goto err;
    }

    /* Here we are overloading BOARD_IOCTL_PARMS with BOARD_MEMACCESS_IOCTL_PARMS
     * so we need to copy it manually
     */
    if (copy_from_user((void*)kbuf, (void*)parms->buf, blen)) {
        printk("copy_from_user failed %s:%d\n", __FILE__, __LINE__);
        ret = -EFAULT;
        goto ext;
    }

    ret = board_ioctl_mem_access(parms, kbuf, blen);

   /* Here we are overloading BOARD_IOCTL_PARMS with BOARD_MEMACCESS_IOCTL_PARMS
    * so we need to copy it manually
    */

   if(__copy_to_user((void *)parms->buf, (void*)kbuf, blen))
       printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);

ext:
    kfree(kbuf);
err:
    return ret;

}

int board_ioctl_cmd_handler(struct file *flip, unsigned int command, unsigned long arg, BOARD_IOCTL_PARMS *ctrlParms)
{

    int ret = 0;
    char *param=NULL, *value=NULL;
    unsigned char ucaMacAddr[NVRAM_MAC_ADDRESS_LEN];


    if(!ctrlParms)
        return -EINVAL;

    switch (command) {
    case BOARD_IOCTL_FLASH_WRITE:
        legacy_ioctl_alert("BOARD_IOCTL_FLASH_WRITE");
        switch (ctrlParms->action) {
        case SCRATCH_PAD:
            if (ctrlParms->offset == -1)
                ret =  kerSysScratchPadClearAll();
            else
            {
                /* ctrlParms->offset is the ctrlparms.buf length here so alloc a buffer for it */
                char * temp_scratchp_bufp = kmalloc(ctrlParms->offset, GFP_KERNEL);
                ret = -EFAULT;
                if ( temp_scratchp_bufp )
                {
                    memset(temp_scratchp_bufp, 0, ctrlParms->offset);
                    /* Since ctlrParms.buf has user defined behaviour, explicitly copy it here */
                    if (copy_from_user((void*)temp_scratchp_bufp, (void*)ctrlParms->buf, ctrlParms->offset) == 0) 
                        ret = kerSysScratchPadSet(ctrlParms->string, temp_scratchp_bufp, ctrlParms->offset);

                    kfree(temp_scratchp_bufp);
                }
            }
            break;

        case PERSISTENT:
            ret = kerSysPersistentSet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case BACKUP_PSI:
            ret = kerSysBackupPsiSet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case SYSLOG:
            ret = kerSysSyslogSet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case NVRAM:
        {
            printk("Invalid IOCTL NVRAM\n"); 
            break;
        }
        case ENVRAM:
        {
            /* Note that ctrlParms->param is a userspace string */
            param = (unsigned char *)kmalloc(strnlen_user(ctrlParms->param, 100)+1, GFP_KERNEL);
            if (param == NULL)
            {
                ret = -EFAULT;
                break;
            }
            memset(param, '\0', strnlen_user(ctrlParms->param, 100)+1); 
            value = ctrlParms->value;
            if (value == NULL)
            {
                kfree(param);
                ret = -EFAULT;
                break;
            }
            memset(value, '\0', ctrlParms->value_length+1); 
            ret = -EFAULT;
            if(copy_from_user(param, ctrlParms->param, strnlen_user(ctrlParms->param, 100)) == 0)
            {
               if(envram_sync_set_locked(param, value) > 0)
               {
                   ret=0;
               }
            }
            else
                printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
            kfree(param);
            break;
        }

        default:
            ret = -EINVAL;
            printk("flash_ioctl_command: invalid command %d\n", ctrlParms->action);
            break;
        }
        ctrlParms->result = ret;
        break;

    case BOARD_IOCTL_FLASH_READ:
        legacy_ioctl_alert("BOARD_IOCTL_FLASH_READ");
        switch (ctrlParms->action) {
        case SCRATCH_PAD:
            {
                /* ctrlParms->offset is the ctrlparms.buf length here so alloc a buffer for it */
                char * temp_scratchp_bufp = kmalloc(ctrlParms->offset, GFP_KERNEL);
                ret = -EFAULT;
                if (!temp_scratchp_bufp)
                    break;
                
                ret = kerSysScratchPadGet(ctrlParms->string, temp_scratchp_bufp, ctrlParms->offset);
                if (ret)
                    goto end_scratch_pad;

                /* Since ctlrParms.buf has user defined behaviour, explicitly copy it back here */
                ret = __copy_to_user((void*)ctrlParms->buf, temp_scratchp_bufp, ctrlParms->offset);
                if (ret)
                    printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
end_scratch_pad:
                kfree(temp_scratchp_bufp);
            }

            break;

        case PERSISTENT:
            ret = kerSysPersistentGet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case BACKUP_PSI:
            ret = kerSysBackupPsiGet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case SYSLOG:
            ret = kerSysSyslogGet(ctrlParms->string, ctrlParms->strLen, ctrlParms->offset);
            break;

        case NVRAM:
            printk("NVRAM IOCTL IS DEPRECATED\n");
            printk("please use /proc/environment/<env> to read the u-boot environment variable\n");
            ret = -1;
            break;
        case ENVRAM:
            /* Note that ctrlParms->param is a userspace string */
            param = (unsigned char *)kmalloc(strnlen_user(ctrlParms->param, 100)+1, GFP_KERNEL);
            if (param == NULL)
            {
                ret = -EFAULT;
                break;
            }
            memset(param, '\0', strnlen_user(ctrlParms->param, 100)+1);  
            value = ctrlParms->value;
            if (value == NULL)
            {
                kfree(param);
                ret = -EFAULT;
                break;
            }
            memset(value, '\0', ctrlParms->value_length);

            ret=-EFAULT;
            if(copy_from_user(param, ctrlParms->param, strnlen_user(ctrlParms->param, 100)) == 0)
            {
                ret=envram_get_locked(param, value, ctrlParms->value_length);
                if(ret)
                {
                    ret=-EFAULT;
                    memcpy(ctrlParms->value, value, ctrlParms->value_length);
                    ret=0;
                }
                else
                    ret=-1;
            }
            else
                printk("copy_from_user failed %s:%d \n", __FILE__, __LINE__);
            kfree(param);
            break;

        case FLASH_SIZE:
            ret = kerSysFlashSizeGet();
            break;

        default:
            ret = -EINVAL;
            printk("Not supported.  invalid command %d\n", ctrlParms->action);
            break;
        }
        ctrlParms->result = ret;
        break;

    case BOARD_IOCTL_FLASH_LIST:
        legacy_ioctl_alert("BOARD_IOCTL_FLASH_LIST");
        switch (ctrlParms->action) {
        case SCRATCH_PAD:
            {
                /* ctrlParms->offset is the buflength here so alloc a buffer for it */
                char * temp_scratchp_bufp = kmalloc(ctrlParms->offset, GFP_KERNEL);
                ret = -EFAULT;
                if ( temp_scratchp_bufp )
                {
                    ret = kerSysScratchPadList(temp_scratchp_bufp, ctrlParms->offset);
                    /* Since ctlrParms.buf has user defined behaviour, explicitly copy it back here */
                    if(__copy_to_user((void*)ctrlParms->buf, temp_scratchp_bufp, ctrlParms->offset))
                        printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
                    kfree(temp_scratchp_bufp);
                }
            }
            break;

        default:
            ret = -EINVAL;
            printk("Not supported.  invalid command %d\n", ctrlParms->action);
            break;
        }
        ctrlParms->result = ret;
        break;

    case BOARD_IOCTL_DUMP_ADDR:
        dumpaddr( (unsigned char *) ctrlParms->string, ctrlParms->strLen );
        ctrlParms->result = 0;
        ret = 0;
        break;

    case BOARD_IOCTL_SET_MEMORY:
        {
            unsigned int *pul = (unsigned int *)  ctrlParms->string;
            unsigned short *pus = (unsigned short *) ctrlParms->string;
            unsigned char *puc = (unsigned char *)  ctrlParms->string;
            switch( ctrlParms->strLen ) {
            case 4:
                *pul = (unsigned int) ctrlParms->offset;
                break;
            case 2:
                *pus = (unsigned short) ctrlParms->offset;
                break;
            case 1:
                *puc = (unsigned char) ctrlParms->offset;
                break;
            }
            dumpaddr( (unsigned char *) ctrlParms->string, sizeof(int) );
            ctrlParms->result = 0;
            ret = 0;
        }
        break;

    case BOARD_IOCTL_MIPS_SOFT_RESET:
        kernel_restart(NULL);
        break;

    case BOARD_IOCTL_LED_CTRL:
        kerSysLedCtrl((BOARD_LED_NAME)ctrlParms->strLen, (BOARD_LED_STATE)ctrlParms->offset);
        ret = 0;
        break;

    case BOARD_IOCTL_GET_ID:
        if( ctrlParms->string )
        {
                memset(ctrlParms->string, '\0', ctrlParms->strLen);
                ret = 0;
                if(eNvramGet(NVRAM_SZBOARDID, ctrlParms->string, ctrlParms->strLen) < 0)
                    ret=-1;
        }
        ctrlParms->result = 0;
        break;
    
    case BOARD_IOCTL_GET_MAC_ADDRESS:
        ctrlParms->result = kerSysGetMacAddress( ucaMacAddr,
            ctrlParms->offset );

        if( ctrlParms->result == 0 )
        {
            if(memcpy(ctrlParms->string, ucaMacAddr,
                sizeof(ucaMacAddr)) == NULL)
                  break;
        }

        ret = ctrlParms->result;
        break;

    case BOARD_IOCTL_ALLOC_MAC_ADDRESSES:
        /* ctrlParms->buf is used to pass an int indicating number of mac addresses */
        {
            UINT32 num_addr;
            ctrlParms->result = -EFAULT;
            if (copy_from_user((void*)&num_addr, (void*)ctrlParms->buf, sizeof(UINT32)) == 0)
            {
                ctrlParms->result = kerSysGetMacAddresses( ucaMacAddr, num_addr, ctrlParms->offset );
            }
            else
            {
                printk("board_ioctl: command %x failed copying string/value from userspace , cmd %d .\n",command,_IOC_NR(command));
            }

            if( ctrlParms->result == 0 )
            {
                if(memcpy(ctrlParms->string, ucaMacAddr, sizeof(ucaMacAddr)))
                    ctrlParms->strLen = sizeof(ucaMacAddr);
            }
            ret = ctrlParms->result;
        }
        break;

    case BOARD_IOCTL_RELEASE_MAC_ADDRESSES:
        if (ctrlParms->strLen)
        {
            UINT32 num_addr;
            ctrlParms->result = -EFAULT;
            memcpy(ucaMacAddr, ctrlParms->string, sizeof(ucaMacAddr));
            /* ctrlParms->buf is used to pass an int indicating number of mac addresses */
            if (copy_from_user((void*)&num_addr, (void*)ctrlParms->buf, sizeof(UINT32)) == 0)
            {
                ctrlParms->result = kerSysReleaseMacAddresses( ucaMacAddr, num_addr );
            }
            else
            {
                printk("board_ioctl: command %x failed copying string/value from userspace , cmd %d .\n",command,_IOC_NR(command));
            }
        }
        else
        {
            ctrlParms->result = -EACCES;
        }

        ret = ctrlParms->result;
        break;

    case BOARD_IOCTL_RELEASE_MAC_ADDRESS:
        if (ctrlParms->strLen)
        {
            memcpy(ucaMacAddr, ctrlParms->string, sizeof(ucaMacAddr));
            ctrlParms->result = kerSysReleaseMacAddress( ucaMacAddr );
        }
        else
        {
            ctrlParms->result = -EACCES;
        }

        ret = ctrlParms->result;
        break;

    case BOARD_IOCTL_GET_PSI_SIZE:
        {
            legacy_ioctl_alert("BOARD_IOCTL_GET_PSI_SIZE");
            ret = -1;
        }
        break;

    case BOARD_IOCTL_GET_BACKUP_PSI_SIZE:
        {
            legacy_ioctl_alert("BOARD_IOCTL_GET_BACKUP_PSI_SIZE");
            // if number_blks > 0, that means there is a backup psi, but length is the same
            // as the primary psi (persistent).

            ret = -1;
        }
        break;

    case BOARD_IOCTL_GET_SYSLOG_SIZE:
        {
            legacy_ioctl_alert("BOARD_IOCTL_GET_SYSLOG_SIZE");
            ret = -1;
        }
        break;

    case BOARD_IOCTL_GET_SDRAM_SIZE:
        /* return size in MB to avoid overflow */
        ctrlParms->result = (int) (kerSysGetSdramSize()>>20);
        ret = 0;
        break;

    case BOARD_IOCTL_GET_BASE_MAC_ADDRESS:
        {
            PMAC_INFO pMacInfo = get_mac_info();
            if (!pMacInfo)
            {
               printk("WARNING : pMacInfo == NULL\n");
               ctrlParms->string[0] = 0;
               ctrlParms->string[1] = 1;
               ctrlParms->string[2] = 2;
               ctrlParms->result = 0;
               ret = 0;
            }
            else if(memcpy(ctrlParms->string, pMacInfo->ucaBaseMacAddr, NVRAM_MAC_ADDRESS_LEN) == NULL)
                ret=-EFAULT;
            else {
                ctrlParms->result = 0;
                ret = 0;
            }
        }
        break;

    case BOARD_IOCTL_GET_CHIP_ID:
        ctrlParms->result = kerSysGetChipId();
        ret = 0;
        break;

    case BOARD_IOCTL_GET_CHIP_REV:
        ctrlParms->result = UtilGetChipRev();
        ret = 0;
        break;

    case BOARD_IOCTL_GET_NUM_ENET_MACS:
    case BOARD_IOCTL_GET_NUM_ENET_PORTS:
        ctrlParms->result = dt_number_of_emacs_get();
        if (ctrlParms->result >= 0)
        {
            ret = 0;
            break;
        }

        break;

    case BOARD_IOCTL_GET_NUM_MAC_ADDRESSES:
        {
            PMAC_INFO pMacInfo = get_mac_info();
            if (!pMacInfo)
            {
               printk("WARNING : pMacInfo == NULL\n");
               ctrlParms->result = 1;
            }
            else {
               ret = -EFAULT;			
               ctrlParms->result = (int)pMacInfo->ulNumMacAddrs;
            }
            if (ctrlParms->result >= 0)
                ret = 0;
    	}
        break;

    case BOARD_IOCTL_GET_CFE_VER:
        ret = -EFAULT;
        break;

#if defined (WIRELESS)
    case BOARD_IOCTL_GET_WLAN_ANT_INUSE:
        {
                ret = -EFAULT;
            
        }
        break;
#endif
    case BOARD_IOCTL_SET_TRIGGER_EVENT:
        {
            BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
            ctrlParms->result = -EFAULT;
            ret = -EFAULT;
            if (ctrlParms->strLen == sizeof(unsigned int)) {
                int * eventmask = (int*)ctrlParms->string;
                board_ioc->eventmask |= *eventmask;
#if defined (WIRELESS)
                if((board_ioc->eventmask & SES_EVENTS)) {
                    ctrlParms->result = 0;
                    ret = 0;
                }
#endif
            }
        }
        break;

    case BOARD_IOCTL_GET_TRIGGER_EVENT:
        {
            BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
            if (ctrlParms->strLen == sizeof(unsigned int)) {
                memcpy(ctrlParms->string, &board_ioc->eventmask, sizeof(unsigned int));
                ctrlParms->result = 0;
                ret = 0;
            } else
                ret = -EFAULT;
        }

        break;

    case BOARD_IOCTL_UNSET_TRIGGER_EVENT:
        if (ctrlParms->strLen == sizeof(unsigned int)) {
            BOARD_IOC *board_ioc = (BOARD_IOC *)flip->private_data;
            int * eventmask = (int*)ctrlParms->string;
            board_ioc->eventmask &= (~(*eventmask));
            ctrlParms->result = 0;
            ret = 0;
        } else
            ret = -EFAULT;

        break;
#if defined (WIRELESS)
    case BOARD_IOCTL_SET_SES_LED:
        if (ctrlParms->strLen == sizeof(int)) {
            int * action = (int*)ctrlParms->string;
            sesLed_ctrl(*action);
            ctrlParms->result = 0;
            ret = 0;
        } else
            ret = -EFAULT;

        break;
#endif

    case BOARD_IOCTL_SET_MONITOR_FD:
        kerSysSetMonitorTaskPid(ctrlParms->offset);
        printk(KERN_INFO "monitor task is initialized pid= %d\n",ctrlParms->offset);
        ret=0;
        break;

    case BOARD_IOCTL_WAKEUP_MONITOR_TASK:
        kerSysSendtoMonitorTask(MSG_NETLINK_BRCM_WAKEUP_MONITOR_TASK, NULL, 0);
        break;

    case BOARD_IOCTL_SET_CS_PAR:
        ret = ConfigCs(ctrlParms);
        break;

    case BOARD_IOCTL_BOOT_IMAGE_OPERATION:
        legacy_ioctl_alert("BOARD_IOCTL_BOOT_IMAGE_OPERATION");
        switch(ctrlParms->offset)
        {
            default:
                ctrlParms->result = -EFAULT;
                break;
        }
        ret = 0;
        break;

    case BOARD_IOCTL_GET_TIMEMS:
        ret = jiffies_to_msecs(jiffies - INITIAL_JIFFIES);
        break;

#ifdef CONFIG_BCM_FTTDP_G9991
    case BOARD_IOCTL_SPI_SLAVE_INIT:  
        ret = 0;
        if (compat_kerSysBcmSpiSlaveInit(ctrlParms->result) == SPI_STATUS_OK)
            break;

        ret = -EFAULT;
        break;
    case BOARD_IOCTL_SPI_SLAVE_READ:  
        {
            /* ctrlParms->strLen is the ctrlParms->buf length here so alloc a buffer for it */
            char * temp_spislave_bufp = kmalloc(ctrlParms->strLen, GFP_KERNEL);
            ret = 0;
            if ( temp_spislave_bufp )
            {
                if (compat_kerSysBcmSpiSlaveRead(ctrlParms->result, ctrlParms->offset, (unsigned int *)temp_spislave_bufp, ctrlParms->strLen) != SPI_STATUS_OK)
                    ret = -EFAULT;

                /* Since ctlrParms.buf has user defined behaviour, explicitly copy it back here */
                if(__copy_to_user((void*)ctrlParms->buf, temp_spislave_bufp, ctrlParms->strLen))
                        printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
                kfree(temp_spislave_bufp);
            }
            else
                ret = -EFAULT;
        }
        break;    
    case BOARD_IOCTL_SPI_SLAVE_WRITE_BUF:
        {
            /* ctrlParms->strLen is the ctrlParms->buf length here so alloc a buffer for it */
            char * temp_spislave_bufp = kmalloc(ctrlParms->strLen, GFP_KERNEL);
            ret = 0;
            if ( temp_spislave_bufp )
            {
                /* Since ctlrParms.buf has user defined behaviour, explicitly copy it here */
                if (copy_from_user((void*)temp_spislave_bufp, (void*)ctrlParms->buf, ctrlParms->strLen) == 0) 
                {
                    if (compat_kerSysBcmSpiSlaveWriteBuf(ctrlParms->result, ctrlParms->offset, (unsigned int *)temp_spislave_bufp, ctrlParms->strLen, 4) != SPI_STATUS_OK)
                    {
                        ret = -EFAULT;
                    }
                }
                else
                {
                    printk("copy_from_user failed %s:%d \n", __FILE__, __LINE__);
                    ret = -EFAULT;
                }

                kfree(temp_spislave_bufp);
            }
            else
                ret = -EFAULT;
        }
        break;

    case BOARD_IOCTL_GET_GPIO:
       if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0) {
           ctrlParms.offset = kerSysGetGpioValue(ctrlParms.strLen);
            __copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(BOARD_IOCTL_PARMS));
            ret = 0;
        }
        else {
            ret = -EFAULT;
        }
        break;
#endif

    case BOARD_IOCTL_GET_BATTERY_ENABLE:
        ret = bcm_bmu_is_battery_enabled();
        break;

    case BOARD_IOCTL_MEM_ACCESS:
        {
            BOARD_MEMACCESS_IOCTL_PARMS parms;
            int blen;

            /* Here we are overloading BOARD_IOCTL_PARMS with BOARD_MEMACCESS_IOCTL_PARMS
             * so we need to copy it manually
             */
            if (copy_from_user((void*)&parms, (void*)arg, sizeof(parms))) 
            {
		printk("copy_from_user failed %s:%d %lx\n", __FILE__, __LINE__, arg);
                ret = -EFAULT;
                break;
            }

            if (parms.op == BOARD_MEMACCESS_IOCTL_OP_FILL) {
                blen = parms.size;
            } else {
                blen = parms.size * parms.count;
            }
            ret=memaccess(&parms, blen);

            if(copy_to_user((void *)(arg), (void*)&parms, sizeof(parms))) {
                printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
		ret=-EFAULT;
            }

        }
        break;

    case BOARD_IOCTL_SET_DYING_GASP_INTERRUPT:
#if defined(CONFIG_BCM_DGASP_DRV)
        kerSysDyingGaspIoctl(ctrlParms->offset);
#endif
        break;

    case BOARD_IOCTL_GET_BOOT_SECURE:
        ctrlParms->result = 0;
#if defined(CONFIG_BCM_OTP_DRV)
        ctrlParms->result = bcm_otp_is_boot_secure();
#endif
        ret = 0;
        break;

    case BOARD_IOCTL_GET_BTRM_BOOT:
        ctrlParms->result = 0;
#if defined(CONFIG_BCM_OTP_DRV)
        ctrlParms->result = bcm_is_btrm_boot();
#endif
        ret = 0;
        break;

    case BOARD_IOCTL_GET_BOOT_MFG_SECURE:
#if defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96855)
        ctrlParms->result = bcm_otp_is_boot_mfg_secure();
#else
        ctrlParms->result = 0;
#endif
        ret = 0;
        break;

#if defined(CONFIG_BCM963158)
    case BOARD_IOCTL_BT_GPIO: // bluetooth gpio for reset / wake
        {
            // get or set bt gpio
            switch (ctrlParms->strLen) {
            case BOARD_IOCTL_BT_GPIO_RESET:
               printk("BpGetBtResetGpio is no longer supported, Add code to read this from device tree\n");
               ret = -ENOSYS;
               break;
            case BOARD_IOCTL_BT_GPIO_WAKE:
               printk("BpGetBtWakeGpio is no longer supported, Add code to read this from device tree\n");
               ret = -ENOSYS;
               break;
            default:
               ret = -EFAULT;
            }
        }
        break;
#endif

    case BOARD_IOCTL_GET_FLASH_TYPE:
        ctrlParms->result = flash_get_flash_type();
        ret = 0;
        break;

    default:
        ret = -EINVAL;
        ctrlParms->result = 0;
        printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
        break;

    } /* switch */

    return ret;
}


//********************************************************************************************
// misc. ioctl calls come to here. (flash, led, reset, kernel memory access, etc.)
//********************************************************************************************
int board_ioctl( struct inode *inode, struct file *flip,
                       unsigned int command, unsigned long arg )
{
    int ret = 0;
    BOARD_IOCTL_PARMS ctrlParms;
    void *user_val_string_bufp = NULL;
    void *temp_string_bufp = NULL;

    //printk("board_ioctl: command %x cmd %d .\n",command,_IOC_NR(command));
    memset (&ctrlParms, 0, sizeof(ctrlParms));

    /* BOARD_IOCTL_MEM_ACCESS uses a different IOCTL structure for passing data */
    if( arg && (command != BOARD_IOCTL_MEM_ACCESS) )
    {
        /* Copy over ctrlParms and data for ctrlParms.string OR ctrlParms.value */
        if(copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) == 0 )
        {
            /* Copy over data from union ctrlParms.string OR ctrlParms.value */
            if( ctrlParms.string && ctrlParms.strLen ) 
            {
                /* allocate one extra byte due to legacy reasons */
                temp_string_bufp = kmalloc((size_t)ctrlParms.strLen+1, GFP_KERNEL);
                if( temp_string_bufp ) 
                {
                    memset(temp_string_bufp, 0, ctrlParms.strLen+1);
                    if (copy_from_user((void*)temp_string_bufp, (void*)(ctrlParms.string), ctrlParms.strLen) == 0) {
                        user_val_string_bufp = (void*)ctrlParms.string;
                        ctrlParms.string = temp_string_bufp;
                    }
                    else
                    {
                        kfree(temp_string_bufp);
                        printk("board_ioctl: command %x failed copying string/value from userspace , cmd %d .\n",command,_IOC_NR(command));
                        ret = -EFAULT;
                        return ret;
                    }
                }
                else
                {
                    printk("board_ioctl: command %x failed to allocate memory for string/value , cmd %d .\n",command,_IOC_NR(command));
                    ret = -EFAULT;
                    return ret;
                }
            }
        }
        else
        {
            printk("board_ioctl: command %x failed copying ctrlparms from userspace , cmd %d %px.\n",command,_IOC_NR(command), (void*)arg);
            ret = -EFAULT;
            return ret;
        }
    }

    ret=board_ioctl_cmd_handler(flip, command, arg, &ctrlParms);

    if( ret )
    {
        if( arg && (command != BOARD_IOCTL_MEM_ACCESS) )
        {
            kfree(temp_string_bufp);
        }
        return ret;
    }

    /* Copy to user ctrlParms and memory pointed to by ctrlParms.string OR ctrlParms.value */
    if( arg && (command != BOARD_IOCTL_MEM_ACCESS) )
    {
        if( ctrlParms.string && ctrlParms.strLen ) 
        {
            temp_string_bufp = ctrlParms.string;
            ctrlParms.string = user_val_string_bufp;
            if(__copy_to_user((void*)ctrlParms.string, temp_string_bufp, ctrlParms.strLen))
                printk("__copy_to_user failed\n");
            kfree(temp_string_bufp);
        }
        if(__copy_to_user((BOARD_IOCTL_PARMS*)arg, &ctrlParms, sizeof(ctrlParms)))
            printk("__copy_to_user failed\n");
    }


    return ret;

} /* board_ioctl */
