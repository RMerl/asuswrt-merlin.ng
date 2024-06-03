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
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <bcmtypes.h>
#if defined(CONFIG_COMPAT)
#include "compat_board.h"
#endif
#include <board.h>

/* brcmboard device driver related variables */
static struct cdev brcmboard_cdev;
static struct device *brcmboard_device = NULL;
static struct class *brcmboard_cl     = NULL;
static dev_t brcmboard_devId;

static int board_open( struct inode *inode, struct file *filp )
{
    return 0;
}

static int board_release(struct inode *inode, struct file *filp)
{
    return 0;
}

int board_ioctl_mem_access(BOARD_MEMACCESS_IOCTL_PARMS* parms, char* kbuf, int len)
{
    int i, j;
    unsigned char *cp,*bcp;
    unsigned short *sp,*bsp;
    unsigned int *ip,*bip;
    void *va;

    bcp = (unsigned char *)kbuf;
    bsp = (unsigned short *)bcp;
    bip = (unsigned int *)bcp;

    switch(parms->space) {
        case BOARD_MEMACCESS_IOCTL_SPACE_REG:
            va = ioremap((long)parms->address, len);
            break;
        case BOARD_MEMACCESS_IOCTL_SPACE_KERN:
            va = (void*)(uintptr_t)parms->address;
            break;
        default:
            va = NULL;
            return EFAULT;
    }
    //printk("memacecssioctl address started %08x mapped to %08x size is %d count is %d\n",(int)parms->address, (int)va, parms->size, parms->count);
    cp = (unsigned char *)va;
    sp = (unsigned short *)((long)va & ~1);
    ip = (unsigned int *)((long)va & ~3);
    for (i=0; i < parms->count; i++) {
        if ((parms->op == BOARD_MEMACCESS_IOCTL_OP_WRITE) 
            || (parms->op == BOARD_MEMACCESS_IOCTL_OP_FILL)) {
            j = 0;
            if (parms->op == BOARD_MEMACCESS_IOCTL_OP_WRITE) 
            {
                j = i;
            }
            switch(parms->size) {
                case 1:
                    cp[i] = bcp[j];
                    break;
                case 2:
                    sp[i] = bsp[j];
                    break;
                case 4:
                    ip[i] = bip[j];
                    break;
            }
        } else {
                switch(parms->size) {
                case 1:
                    bcp[i] = cp[i];
                    break;
                case 2:
                    bsp[i] = sp[i];
                    break;
                case 4:
                    bip[i] = ip[i];
                    break;
            }
        }
    }
    
    if (va != (void*)(uintptr_t)parms->address)
    {
        iounmap(va);
    }
    return 0;
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

    if (__copy_from_user((void*)kbuf, (void*)parms->buf, blen)) {
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

int board_ioctl( struct inode *inode, struct file *flip,
                       unsigned int command, unsigned long arg )
{
    int ret = 0;

    switch (command) {
	case BOARD_IOCTL_MEM_ACCESS:
	    {
		BOARD_MEMACCESS_IOCTL_PARMS parms;
		int blen;

		/* Here we are overloading BOARD_IOCTL_PARMS with BOARD_MEMACCESS_IOCTL_PARMS
		* so we need to copy it manually
		*/
		if (copy_from_user((void*)&parms, (void*)arg, sizeof(parms))) 
		{
		    ret = -EFAULT;
		    break;
		}

		if (parms.op == BOARD_MEMACCESS_IOCTL_OP_FILL) {
		    blen = parms.size;
		} else {
		    blen = parms.size * parms.count;
		}

		ret = memaccess(&parms, blen);

		if (copy_to_user((void *)(arg), (void*)&parms, sizeof(parms))) {
			printk("copy_to_user failed %s:%d \n", __FILE__, __LINE__);
	 		ret=-EFAULT;
		}
		}
	    break;
	default:
	    ret = -EINVAL;
	    printk("board_ioctl: invalid command %x, cmd %d .\n",command,_IOC_NR(command));
	    break;

    } /* switch */

    return ret;
} /* board_ioctl */

#if defined(HAVE_UNLOCKED_IOCTL)
static DEFINE_MUTEX(ioctlMutex);

long board_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    long rt;
    
    inode = file_inode(filep);

    mutex_lock(&ioctlMutex);
    rt = board_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&ioctlMutex);
    return rt;
    
}
#endif

static struct file_operations board_fops =
{
    open:       board_open,
#if defined(HAVE_UNLOCKED_IOCTL)
    unlocked_ioctl: board_unlocked_ioctl,
#else
    ioctl:      board_ioctl,
#endif
#if defined(CONFIG_COMPAT)
    compat_ioctl: compat_board_ioctl,
#endif    
    release:    board_release,
};

static int __init brcm_board_init( void )
{
    int ret;
  
    alloc_chrdev_region(&brcmboard_devId, 0, 2, "brcmboard");
    
    /* Create class and device ( /sys entries ) */
    brcmboard_cl = class_create(THIS_MODULE, "brcmboard");
    if(brcmboard_cl == NULL)
    {
       printk(KERN_ERR "Error creating device class\n");
       goto err_cdev_cleanup;
    }
    
    brcmboard_device = device_create(brcmboard_cl, NULL, brcmboard_devId, NULL, "brcmboard");
    if(brcmboard_device == NULL)
    {
       printk(KERN_ERR "Error creating device\n");
       goto err_class_cleanup;
    }
    
    /* Set the DMA masks for this device */
    dma_coerce_mask_and_coherent(brcmboard_device, DMA_BIT_MASK(32));
        
    /* Init the character device */
    cdev_init(&brcmboard_cdev, &board_fops);
    brcmboard_cdev.owner = THIS_MODULE;
    ret = cdev_add(&brcmboard_cdev, brcmboard_devId, 1);
    if(ret != 0)
    {
       printk(KERN_ERR "Error %d adding brcmboard driver", ret);
       goto err_device_cleanup;
    }
    else
    {
	printk("brcm_board_init done\n");
        return 0;
    }

err_device_cleanup:
    device_destroy(brcmboard_cl, brcmboard_devId);
err_class_cleanup:
    class_destroy(brcmboard_cl);
err_cdev_cleanup:
    cdev_del(&brcmboard_cdev);

    return -1;
}

module_init(brcm_board_init);
