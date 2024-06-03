/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
*
*    Copyright (c) 2021 Broadcom
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

/* Create /proc for sync FDB for PKTC/PKTFWD from userspace */

#if defined(CONFIG_BCM_WLAN_MODULE)

#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/etherdevice.h>
#include <linux/bcm_log.h>

#undef DEBUG

static int Block_DEL =  0;

static struct proc_dir_entry *proc_fdbsync_dir = NULL;       /* /proc/fdbsync */
static struct proc_dir_entry *proc_fdbsync_ops_file = NULL;  /* /proc/fdbsync/operate */

static ssize_t fdbsync_operate_file_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos);
static const struct file_operations fdbsync_fops = {
       .owner  = THIS_MODULE,
       .write   = fdbsync_operate_file_write,
};

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
int fdbsync_proc_init(void)
{
    if (!(proc_fdbsync_dir = proc_mkdir("fdbsync", NULL)))
        goto fail;

    if (!(proc_fdbsync_ops_file = proc_create("fdbsync/operate", 0644, NULL, &fdbsync_fops)))
        goto fail;

    return 0;

fail:
    printk("%s %s: Failed to create proc /fdbsync\n", __FILE__, __FUNCTION__);
    remove_proc_entry("fdbsync" ,NULL);
    return (-1);
}
EXPORT_SYMBOL(fdbsync_proc_init);
subsys_initcall(fdbsync_proc_init);

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
void fdbsync_proc_fini(void)
{
    remove_proc_entry("operate", proc_fdbsync_dir);
    remove_proc_entry("fdbsync", NULL);
}
EXPORT_SYMBOL(fdbsync_proc_fini);

static int enet_aton(unsigned char *str,unsigned char *macbyte)
{
    int values[ETH_ALEN];
    int i;
    int ret = 0;
    if( ETH_ALEN == sscanf( str, "%x:%x:%x:%x:%x:%x%*c",
        &values[0], &values[1], &values[2],
        &values[3], &values[4], &values[5] ) )
    {
        /* convert to uint8_t */
        for( i = 0; i < ETH_ALEN; ++i )
            macbyte[i] = (unsigned char) values[i];
        ret = 1;
    }
    else
    {  /* invalid mac */

        printk("%s: err str:%s\n",__FUNCTION__,str);

    }
    return ret;
}

int parse_dev_name(unsigned char *str,unsigned char **devname)
{
    int ret = 0;
    if(str && devname)
    {
        *devname = strstr(str,"/");
        if((*devname))
        {
            (*devname)++;
            ret = 1;
        }
    }
    return ret;
}

#ifdef DEBUG
static int probe_wl_dhd_type(struct net_device * dev_p)
{
    int ret=-1;
    if(dev_p)
    {
        unsigned char ioctl_func[64]= "";

        /* tricky way by check ioctl func name to detect wl/dhd */
        sprintf(ioctl_func,"%pS",dev_p->netdev_ops->ndo_do_ioctl);

        if(!strncmp("dhd_",ioctl_func,4))
            ret = 1;
        else if(!strncmp("wl_",ioctl_func,3))
            ret = 0;
    }

    return ret;
}
#endif

static ssize_t fdbsync_operate_file_write(struct file *file, const char __user *buf, size_t cnt, loff_t *ppos)
{
    char input[64]="";
    unsigned char *macstr = NULL;
    char ACT=' ';

    macstr = NULL;

    if (cnt > 64)
        cnt = 64;

    if (copy_from_user(input, buf, cnt) != 0)
        return -EFAULT;

    input[cnt-1] = '\0';

    /* Command format :
     Toogle   :  T
        Del   :  D=<XX:XX:XX:XX:XX:XX:XX>
    */

    ACT = input[0];

#ifdef DEBUG
    printk("#### %s:%d #### recv:\"%s\" \n",__FUNCTION__,__LINE__,input);
#endif

    switch(ACT) 
    {
        case 'T': //Toggle DEL behavior Enable/Disable
            Block_DEL = !(Block_DEL);
            printk("#### %s:%d #### Block_DEL=%d\n",__FUNCTION__,__LINE__,Block_DEL);
        break;

        case 'D':
            {
                //Create Pseudo FDB entry to simulation linux bridge FDB del.

                unsigned char* mac_addr  = NULL;
                unsigned char buf[32]={0};
                mac_addr  = (unsigned char*)buf;
                macstr = &(input[2]);

                if(enet_aton(macstr,mac_addr))
                {
                    if(is_valid_ether_addr(mac_addr) )
                    {

#ifdef DEBUG
                        printk("#### %s:%d #### %s del %02X:%02X:%02X:%02X:%02X:%02X\n",__FUNCTION__,__LINE__
                            ,(Block_DEL == 1) ? "BLOCK" : ""
                            ,mac_addr[0]
                            ,mac_addr[1]
                            ,mac_addr[2]
                            ,mac_addr[3]
                            ,mac_addr[4]
                            ,mac_addr[5]
                            );
#endif
                         if (Block_DEL == 0)
                         {
                             bcmFun_t *bcm_br_wl_pktc_del_by_mac_cb = bcmFun_get(BCM_FUN_ID_WLAN_PKTC_DEL_BY_MAC);
                             if (bcm_br_wl_pktc_del_by_mac_cb != NULL)
                                 bcm_br_wl_pktc_del_by_mac_cb((void *)mac_addr);

                         }
                    }
                }

            }
            break;

    }
    return cnt;
}

#endif /* CONFIG_BCM_KF_WL */
