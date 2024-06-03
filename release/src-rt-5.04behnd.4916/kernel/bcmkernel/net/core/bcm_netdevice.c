#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include "bcmnet.h"
#include <net/gre.h>
#include <linux/bcm_version_compat.h>

int (*bcm_vlan_handle_frame_hook)(struct sk_buff **) = NULL;
EXPORT_SYMBOL(bcm_vlan_handle_frame_hook);

#ifdef CONFIG_BCM_TMS_MODULE
int (*bcm_1ag_handle_frame_check_hook)(struct sk_buff *) = NULL;
int (*bcm_3ah_handle_frame_check_hook)(void *dev, unsigned short protocol, unsigned char *datPtr) = NULL;
EXPORT_SYMBOL(bcm_1ag_handle_frame_check_hook);
EXPORT_SYMBOL(bcm_3ah_handle_frame_check_hook);
#endif

#define MAC_LIMIT(dev) dev->bcm_nd_ext.mac_limit
DEFINE_SPINLOCK(mac_limit_spinlock);
EXPORT_SYMBOL(mac_limit_spinlock);

int (*bcm_netdev_gen_hwaccel_notfier_cb)(struct net_device *dev,
		int event, int group) = NULL;
EXPORT_SYMBOL(bcm_netdev_gen_hwaccel_notfier_cb);

#if defined(CONFIG_BRIDGE)
extern void bcm_mac_limit_enable(uint32_t enable);
#endif

/*------------------------------------------------------------------------------------
 *BCM Netdevice devid map table and its related functions
 *-----------------------------------------------------------------------------------*/
#define BCM_NETDEV_DEVID_IDX_MASK      (BCM_NETDEV_DEVID_MAX_ENTRIES - 1)
#define BCM_NETDEV_DEVID_INCARN_BITS   (4)
#define AVAIL_BITMAP_NUM_WORDS         (BCM_NETDEV_DEVID_MAX_ENTRIES / (sizeof(unsigned long) * 8))
#define BCM_NETDEV_DEVID_RESERVE_BITS  ((sizeof(uint16_t) * 8) - (BCM_NETDEV_DEVID_MAX_BITS + BCM_NETDEV_DEVID_INCARN_BITS))

#define bcm_netdev_id_get_devid(x)     (x.devid)
#define bcm_netdev_id_get_idx(x)       (x.bmp.idx)
#define bcm_netdev_id_get_incarn(x)    (x.bmp.incarn)

typedef struct {
    union {
       struct {
           uint16_t idx:BCM_NETDEV_DEVID_MAX_BITS;
           uint16_t incarn:BCM_NETDEV_DEVID_INCARN_BITS;
           uint16_t reserved:BCM_NETDEV_DEVID_RESERVE_BITS;
       }bmp;
       uint16_t devid;
    };
}bcm_netdev_id_t;

typedef struct {
    atomic_t user_count;
    bcm_netdev_id_t devid;
    struct net_device *dev;
}bcm_netdev_devid_map_ent_t;

typedef struct {
    spinlock_t lock;
    // bitmap for availability
    unsigned long free_idx_bitmap[AVAIL_BITMAP_NUM_WORDS];
    bcm_netdev_devid_map_ent_t ent[BCM_NETDEV_DEVID_MAX_ENTRIES];
}bcm_netdev_devid_tbl_t;

static bcm_netdev_devid_tbl_t netdev_devid_tbl;
static bcm_netdev_devid_tbl_t *netdev_devid_tbl_p = NULL;
static char bcm_invalid_netdev_name[] = "INVALID_NETDEV";

static struct class *bcm_netdev_class = NULL;
static struct device *bcm_netdev_device = NULL;
static struct cdev bcm_netdev_cdev;
static int bcm_netdev_major;

static inline int get_idx(uint16_t *pdx)
{
    static int offset = 1;
    uint16_t idx = 0;
    volatile unsigned long *addr;
    addr = &netdev_devid_tbl_p->free_idx_bitmap[0];
    idx = find_next_bit((const unsigned long *)addr, BCM_NETDEV_DEVID_MAX_ENTRIES, offset);

    if ((idx == BCM_NETDEV_DEVID_MAX_ENTRIES) && (offset != 1))
        idx = find_next_bit((const unsigned long *)addr, BCM_NETDEV_DEVID_MAX_ENTRIES, 1);
        
    if ((idx > 0) && (idx < BCM_NETDEV_DEVID_MAX_ENTRIES))
    {
        *pdx = idx;
        clear_bit(*pdx, addr);
        if (idx == (BCM_NETDEV_DEVID_MAX_ENTRIES -1))
            offset = 1;
        else
            offset = idx + 1;
        return 0;
    }
    else
    {
        offset = 1;
    }
    return -1;
}

static void put_idx(uint16_t *pdx)
{
    volatile unsigned long *addr;
    if (*pdx < BCM_NETDEV_DEVID_MAX_ENTRIES)
    {
        addr = &netdev_devid_tbl_p->free_idx_bitmap[0];
        set_bit(*pdx, addr);
    }
}

static inline void free_netdev_entry(bcm_netdev_devid_map_ent_t *pentry, uint16_t idx)
{
    unsigned long flags;
    spin_lock_irqsave(&netdev_devid_tbl_p->lock,flags);
    put_idx(&idx);
    dev_put(pentry->dev);
    pentry->dev = NULL;
    spin_unlock_irqrestore(&netdev_devid_tbl_p->lock,flags);
}

/*
 * Initializes the bcm_netdev_devid_tbl.  
 */
static int netdev_devid_tbl_init(void)
{
    int idx;
    netdev_devid_tbl_p = &netdev_devid_tbl;
    memset(netdev_devid_tbl_p, 0, sizeof(bcm_netdev_devid_tbl_t));
    spin_lock_init(&netdev_devid_tbl_p->lock);
    for (idx=0; idx < BCM_NETDEV_DEVID_MAX_ENTRIES; idx++)
    {
        netdev_devid_tbl_p->ent[idx].devid.bmp.idx     = idx;
        netdev_devid_tbl_p->ent[idx].devid.bmp.incarn = 1;
        netdev_devid_tbl_p->ent[idx].dev    = (struct net_device *)NULL;
        atomic_set(&netdev_devid_tbl_p->ent[idx].user_count,0);
        //Mark this ent to be available for mapping
        set_bit(idx,&netdev_devid_tbl_p->free_idx_bitmap[0]);
    }
    // Mark the very first index with devid as 0 to be non-available
    clear_bit(0,&netdev_devid_tbl_p->free_idx_bitmap[0]);
    return 0;
}

/*
 * Allocates an entry in netdev_devid_tbl_t to create a netdev devid mapping.
 * Called from NETDEV_REGISTER notification
 */
static int alloc_devid(struct net_device *dev, uint16_t *devid)
{
    unsigned long flags;
    int status = 0;
    uint16_t idx = 0;
    bcm_netdev_devid_map_ent_t *pentry;
    spin_lock_irqsave(&netdev_devid_tbl_p->lock,flags);
    if (get_idx(&idx))
    {
        status = -1;
        printk(KERN_ERR "%s:invalid idx for netdev devid mapping [%d]\n",__FUNCTION__,idx);
        goto assign_exit;
    }
    pentry = &netdev_devid_tbl_p->ent[idx];
    if (atomic_read(&pentry->user_count) == 0)
    {
        pentry->dev = dev;
        atomic_inc(&pentry->user_count);
        *devid = pentry->devid.devid;
        dev_hold(dev);
    }
    else
    {
        status = -1;
        printk(KERN_ERR "user count is not zero something is wrong\n");
    }
    printk(KERN_INFO "Assigning idx[%d] for %s\n",pentry->devid.bmp.idx,dev->name);
assign_exit:
    spin_unlock_irqrestore(&netdev_devid_tbl_p->lock,flags);
    return status;
}

/*
 *  Frees the entry associated to the devid once the user_count reaches zero.
 */
static int free_devid(uint16_t devid)
{
    int status = 0;
    bcm_netdev_devid_map_ent_t *pentry;
    uint16_t idx    = (devid & BCM_NETDEV_DEVID_IDX_MASK);
    if (unlikely((idx == 0) || (idx >= BCM_NETDEV_DEVID_MAX_ENTRIES)))
    {
        status = -1;
        printk(KERN_ERR "%s:invalid idx for netdev devid mapping [%d]\n",__FUNCTION__,idx);
        goto free_exit;
    }
    pentry = &netdev_devid_tbl_p->ent[idx];
    if (unlikely(devid != pentry->devid.devid))
    {
        status = -1;
        printk(KERN_ERR "%s:devid[%d] doesn't match with devid in table[%d]\n",__FUNCTION__,
                                                                     devid,
                                                                     pentry->devid.devid);
        goto free_exit;
    }
    if (atomic_read(&pentry->user_count) > 0)
    {
        if (atomic_dec_and_test(&pentry->user_count))
        {
            pentry->devid.bmp.incarn++;
            if (pentry->devid.bmp.incarn == 0)
                pentry->devid.bmp.incarn = 1;
            //users is zero now delete this entry
            printk(KERN_INFO "Freeing devid[%d] for %s\n",pentry->devid.devid,pentry->dev->name);
            free_netdev_entry(pentry, idx);
        }
    }
    else
    {
        printk(KERN_ERR "%s:usercount is invalid for %s\n",__FUNCTION__,
                                                     pentry->dev->name);
    }
free_exit:
    return status;
}

/*
 *  Given a devid returns netdev pointer.
 */
struct net_device *bcm_get_netdev_by_id_nohold(uint16_t udevid)
{
    bcm_netdev_devid_map_ent_t *pentry;
    uint16_t idx    = (udevid & BCM_NETDEV_DEVID_IDX_MASK);
    pentry = &netdev_devid_tbl_p->ent[idx];
    if (likely((pentry->devid.devid == udevid) && (atomic_read(&pentry->user_count) > 0)))
    {
        return pentry->dev;
    }
    else 
    {
        printk(KERN_ERR "%s:devid[%d] match fails entry.devid[%d]\n",__FUNCTION__,
                                                            udevid,
                                                            pentry->devid.devid);
    }
    return NULL;
}
EXPORT_SYMBOL(bcm_get_netdev_by_id_nohold);

/*
 *  Given a devid locates mapped netdev pointer and also increases its
 *  user_count. Caller of this API should call bcm_put_netdev_by_id once done.
 */
struct net_device *bcm_get_netdev_by_id(uint16_t udevid)
{
    bcm_netdev_devid_map_ent_t *pentry;
    uint16_t idx    = (udevid & BCM_NETDEV_DEVID_IDX_MASK);
    pentry = &netdev_devid_tbl_p->ent[idx];
    if (likely(pentry->devid.devid == udevid))
    {
        if (atomic_inc_not_zero(&pentry->user_count))
            return pentry->dev;
        else
            printk(KERN_ERR "%s:Trying to get netdev pointer for idx[%d] devid[%d] but zero user_count\n",__FUNCTION__,
                                                                                                 idx,
                                                                                                 udevid);
    }
    else
    {
        printk(KERN_ERR "%s:devid[%d] match fails entry.devid[%d]\n",__FUNCTION__,
                                                            udevid,
                                                            pentry->devid.devid);
    }
    return NULL;
}
EXPORT_SYMBOL(bcm_get_netdev_by_id);

/*
 *  Given a devid locates mapped netdev mapping entry and frees it.
 */
void bcm_put_netdev_by_id(uint16_t udevid)
{
    free_devid(udevid);
}
EXPORT_SYMBOL(bcm_put_netdev_by_id);

/*
 *  Given a devid returns netdev name.
 */
char * bcm_get_netdev_name_by_id(uint16_t udevid)
{
    struct net_device *dev_p = bcm_get_netdev_by_id_nohold(udevid);
    if (dev_p != NULL)
        return dev_p->name;
    return bcm_invalid_netdev_name;
}
EXPORT_SYMBOL(bcm_get_netdev_name_by_id);

/*
 *  Given a devid returns index field.
 *  Returns -1 when index is not an valid index.
 *  Caller has to validate the udevid before calling this API.
 */
int16_t bcm_get_idx_from_id(uint16_t udevid)
{
    uint16_t idx = (udevid & BCM_NETDEV_DEVID_IDX_MASK);
    if ((idx > 0) && (idx < BCM_NETDEV_DEVID_MAX_ENTRIES))
        return (int16_t)idx;
    return -1;
}
EXPORT_SYMBOL(bcm_get_idx_from_id);

/*
 *  Given a devid checks if its valid or not.
 */
bool bcm_is_devid_valid(uint16_t udevid)
{
    bcm_netdev_devid_map_ent_t *pentry;
    uint16_t idx    = (udevid & BCM_NETDEV_DEVID_IDX_MASK);
    pentry = &netdev_devid_tbl_p->ent[idx];
    if (likely((pentry->devid.devid == udevid) && (atomic_read(&pentry->user_count) > 0)))
        return true;
    else
        return false;
}
EXPORT_SYMBOL(bcm_is_devid_valid);
/*
 * Function to dump mapping table. 
 */
static void devid_dump(void)
{
    int idx =0;
    bcm_netdev_devid_map_ent_t *pentry;
    for (idx=0;idx < BCM_NETDEV_DEVID_MAX_ENTRIES; idx++)
    {
        pentry = &netdev_devid_tbl_p->ent[idx];
        if (atomic_read(&pentry->user_count) > 0)
        {
            printk("devid:[%d] idx:[%d] net_device:[%s]\n",pentry->devid.devid,
                                                           pentry->devid.bmp.idx,
                                                           pentry->dev->name);
        }
    }
    printk("\n");
    return;
}

static int devid_register_netdev(struct net_device *dev, uint16_t *devid)
{
    int ret = 0;
    ret = alloc_devid(dev, devid);
    return ret;
}

static int devid_unregister_netdev(uint16_t devid)
{
    int ret = 0;
    ret = free_devid(devid);
    return ret;
}

/*-----------------------------------------------------------------------------------*/
/* bcmnet character device ioctl handler to process get/set netdev BRCM private info */
/*-----------------------------------------------------------------------------------*/



static int bcmnet_open(struct inode *inode, struct file *filep)
{
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
static int mac_limit_reserve(struct net_device *dev, struct netdev_nested_priv* priv)
#else
static int mac_limit_reserve(struct net_device *dev, void * data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
    int reserve = *(int *)priv->data;
#else
    int reserve = *(int *)data;
#endif
    
    MAC_LIMIT(dev).reserve += reserve;
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
static int mac_limit_learn(struct net_device *dev, struct netdev_nested_priv* priv)
#else
static int mac_limit_learn(struct net_device *dev, void * data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
    int learn = *(int *)priv->data;
#else
    int learn = *(int *)data;
#endif
    
    /* coverity[missing_lock] caller already hold mac_limit_spinlock */
    MAC_LIMIT(dev).learning_count += learn;
    return 0;
}

static long mac_limit_ioctl(struct net_device *dev, void *arg)
{
    long ret = 0;
    uint32_t cmd, val;
    struct mac_limit mac_limit, *mac_limit_user;
    mac_limit_arg_t * limit_arg = (mac_limit_arg_t*) arg;
    
    mac_limit_user = (struct mac_limit *)limit_arg->mac_limit;
    cmd = limit_arg->cmd;
    val = limit_arg->val;

    if (!dev && (cmd!=MAC_LIMIT_IOCTL_EN))
        return -EPERM;
    
    switch (cmd)
    {
        case MAC_LIMIT_IOCTL_GET:
        {          
            if (copy_to_user((void *)(mac_limit_user), (void*)&MAC_LIMIT(dev), sizeof(mac_limit)))
            {
                printk(KERN_ERR "%s: failed copy data to user!\n", __FUNCTION__);
                ret = -1;
            }
            break;
        }
        case MAC_LIMIT_IOCTL_SET:
        {
            if (copy_from_user((void *)&mac_limit, (void*)mac_limit_user, sizeof(mac_limit)))
            {
                printk(KERN_ERR "%s: failed copy data from user!\n", __FUNCTION__);
                ret = -1;
            }
            switch (val)
            {
                case MAC_LIMIT_SET_MAX:
                    MAC_LIMIT(dev).max = mac_limit.max;
                    MAC_LIMIT(dev).max_zero_drop = mac_limit.max_zero_drop;
                    break;
                case MAC_LIMIT_SET_MIN:
                    {
                        int rsv_new, rsv_now, rsv;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
			struct netdev_nested_priv priv;
			priv.data=&rsv;
#endif
                        spin_lock(&mac_limit_spinlock);
                        rsv_new = (int)mac_limit.min - (int)MAC_LIMIT(dev).learning_count;
                        if (rsv_new < 0)
                            rsv_new = 0;
                        rsv_now = (int)MAC_LIMIT(dev).min - (int)MAC_LIMIT(dev).learning_count;
                        if (rsv_now < 0)
                            rsv_now = 0;
                        
                        rsv = rsv_new - rsv_now;
                        if (MAC_LIMIT(dev).enable)
                        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
                            netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &priv);
#else
                            netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &rsv);
#endif
                        }
                        MAC_LIMIT(dev).min = mac_limit.min;
                        spin_unlock(&mac_limit_spinlock);
                    }
                    break;
            }
            break;
        }
        case MAC_LIMIT_IOCTL_CLR:
        {
            memset((void *)&MAC_LIMIT(dev), 0, sizeof(mac_limit));
            break;
        }
        case MAC_LIMIT_IOCTL_EN:
        {
            if (dev == NULL)
            {
                /* dev is null, global enable */
#if defined(CONFIG_BRIDGE)
                bcm_mac_limit_enable(val);
#endif
            }
            else
            {
                int rsv, learn;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
			struct netdev_nested_priv priv, priv_1;
			priv.data=&rsv;
			priv_1.data=&learn;
#endif
                spin_lock(&mac_limit_spinlock);
                if (!MAC_LIMIT(dev).enable ^ !val)
                {
                    learn = (int)MAC_LIMIT(dev).learning_count;
                    rsv = (int)MAC_LIMIT(dev).min - (int)MAC_LIMIT(dev).learning_count;
                    if (rsv < 0)
                        rsv = 0;
                    
                    if (val)
                       MAC_LIMIT(dev).enable = 1;
                    else
                    {
                       MAC_LIMIT(dev).enable = 0;
                       rsv = -rsv;  /* decrease */
                       learn = -learn;
                    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &priv);
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_learn, &priv_1);
#else
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &rsv);
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_learn, &learn);
#endif
                }
                spin_unlock(&mac_limit_spinlock);
            }
            break;
        }
        default:
        {
           printk("\n Operation not supported\n" );
           ret = -EPERM;
           break;
        }
    }

    return ret;
}
#if defined(CONFIG_COMPAT)
static int compat_bcmnet_info_get(bcmnet_info_t *info, unsigned int command, unsigned long arg)
{
    compat_bcmnet_info_t compat_info;
    
    if (copy_from_user((void*)&compat_info, (void*)arg, sizeof(compat_bcmnet_info_t)))
    {
        printk(KERN_ERR "%s: failed copy data from user!\n", __FUNCTION__);
        return -1;
    }
    switch (command)
    {
        case BCMNET_IOCTL_MAC_LIMIT:
        {
            info->st_mac_limit.cmd = compat_info.st_mac_limit.cmd;
            info->st_mac_limit.val = compat_info.st_mac_limit.val;
            info->st_mac_limit.mac_limit = (void *)(uintptr_t)compat_info.st_mac_limit.mac_limit;
            break;
        }
        default:
            break;
    }
    return 0;
}
#endif


#ifdef CONFIG_BLOG
static void bcmnet_flush_dev(struct net_device *dev_p)
{
	BlogFlushParams_t blogFlushParams = {};

	blogFlushParams.flush_dev = 1;
	blogFlushParams.devid = dev_p->ifindex;
	blog_notify_async_wait(FLUSH, (void *)&blogFlushParams, (unsigned long)&blogFlushParams, 0);
}
#else
static void bcmnet_flush_dev(struct net_device *dev_p) {}
#endif

int bcmnet_configure_gdx_accel(struct net_device *dev, bcmnet_accel_t *accel_p)
{
    int is_old_accel_hw = is_netdev_hw_accel_gdx(dev);

#if !defined(CONFIG_BCM_GDX_HW)
    if (accel_p->gdx_hw)
    {
        printk("GDX HW Acceleration not supported\n");
        return -1;
    }
#endif

    if(accel_p->gdx_rx){
        netdev_accel_gdx_rx_set(dev);

        /* disable GRO */
        dev->features &= ~NETIF_F_GRO;
        printk(KERN_INFO "GDX Acceleration: Disabling GRO on dev=%s \n", dev->name);
    }
    else
        netdev_accel_gdx_rx_unset(dev);

    if(accel_p->gdx_tx)
        netdev_accel_gdx_tx_set(dev);
    else
        netdev_accel_gdx_tx_unset(dev);

    if(accel_p->gdx_hw) {
        if(!is_old_accel_hw){
            if (bcm_netdev_gen_hwaccel_notfier(dev, 1, 0) == 0)
                netdev_hw_accel_gdx_set(dev);
        }
    } else if(is_old_accel_hw) {
        netdev_hw_accel_gdx_unset(dev);
        bcm_netdev_gen_hwaccel_notfier(dev, 0, 0);
    }

    /* debug code */
    if(accel_p->gdx_debug)
        netdev_accel_gdx_debug_set(dev);
    else
        netdev_accel_gdx_debug_unset(dev);

    /* flush all the flows on this netdev */
    bcmnet_flush_dev(dev);
    return 0;
}
EXPORT_SYMBOL(bcmnet_configure_gdx_accel);

static long bcmnet_ioctl(struct file *filep, unsigned int command, unsigned long arg)
{
    bcmnet_info_t info;
    struct net_device *dev;

    if (command >= BCMNET_IOCTL_MAX)
        return -1;

    if ((arg != 0) && (copy_from_user((void*) &info, (void*) arg, sizeof(info)) != 0))
        return -1;

#if defined(CONFIG_COMPAT)
    if (is_compat_task())
    {
        if ((arg != 0) && (compat_bcmnet_info_get(&info, command, arg) != 0))
            return -1;
    }
#endif
    
    rtnl_lock();
    if (arg != 0)
    {
        dev = __dev_get_by_name(&init_net, info.if_name);
        if (!dev) {
            if (command != BCMNET_IOCTL_MAC_LIMIT)    //mac limit decide later
            {
                printk(KERN_ERR "%s: failure, device %s not found\n", __FUNCTION__, info.if_name);
                goto IOCTL_FAILURE;
            }
        }
    }

    switch (command)
    {
    case BCMNET_IOCTL_GET_EXT_FLAGS:
        info.st_get_ext_flags.ret_val.is_wan = (is_netdev_wan(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_vlan = (is_netdev_vlan(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_ppp = (is_netdev_ppp(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_hw_fdb = (is_netdev_hw_fdb(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_hw_switch = (is_netdev_hw_switch(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_wlan = (is_netdev_wlan(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_bcm_dev = (is_netdev_bcm_dev(dev) != 0) ? 1 : 0;
        info.st_get_ext_flags.ret_val.is_sdn = (is_netdev_sdn_ignore(dev) != 0) ? 1 : 0;
        if( copy_to_user((void*) arg, (void*)&info, sizeof(info)))
                printk(KERN_ERR "%s: failed copy data to user (BCMNET_IOCTL_GET_EXT_FLAGS)!\n", __FUNCTION__);
        break;
    case BCMNET_IOCTL_GET_LAST_CHANGE:
        info.st_get_last_change.last_change =  dev_trans_start(dev);
        if(copy_to_user((void*) arg, (void*)&info, sizeof(info)))
                printk(KERN_ERR "%s: failed copy data to user (BCMNET_IOCTL_GET_LAST_CHANGE)!\n", __FUNCTION__);
        break;
    case BCMNET_IOCTL_ADD_NETDEV_PATH:
        {
            struct net_device *next_dev;
            int err;
            next_dev = dev_get_by_name(&init_net, info.st_add_netdev_path.next_if_name);
            if (!next_dev) {
                printk(KERN_ERR "%s: BCMNET_IOCTL_ADD_NETDEV_PATH: Inavlid Next Device Name: %s\n", 
                       __FUNCTION__, info.st_add_netdev_path.next_if_name);
                goto IOCTL_FAILURE;
            }
            err = netdev_path_add(dev, next_dev);
            if (err) {
                printk(KERN_ERR "%s: BCMNET_IOCTL_ADD_NETDEV_PATH: Failed to add %s to Interface path (%d)\n", 
                       __FUNCTION__, info.if_name, err);
                dev_put(next_dev);
                goto IOCTL_FAILURE;
            } else
                netdev_path_dump(dev);
        }
        break;
    case BCMNET_IOCTL_MAC_LIMIT:
        if (mac_limit_ioctl(dev, (void*)&info.st_mac_limit))
        {
            printk(KERN_ERR "%s: BCMNET_IOCTL_MAC_LIMIT: ioctl failure\n", __FUNCTION__);
            goto IOCTL_FAILURE;
        }
        break;
    case BCMNET_IOCTL_SET_ACCEL_GDX: 
        if (bcmnet_configure_gdx_accel(dev, &info.accel) != 0)
        {
            printk(KERN_ERR "%s: BCMNET_IOCTL_SET_ACCEL_GDX: ioctl failure\n", __FUNCTION__);
            goto IOCTL_FAILURE;
        }
        break;
    case BCMNET_IOCTL_SET_ACCEL_TC_EGRESS:
        {
            /*tc egress shaping support */

             if(info.accel.tc_egress)
                 netdev_accel_tc_egress_set(dev);
            else
                 netdev_accel_tc_egress_unset(dev);

             bcmnet_flush_dev(dev);
        }
        break;
    case BCMNET_IOCTL_SET_ACCEL_FC_TX_THREAD:
        {
            /*fcache Tx Thread enable/disable */

#if defined(CONFIG_FCACHE_TX_THREAD)
             if(info.accel.fc_tx_thread)
                 netdev_accel_fc_tx_thread_set(dev);
            else
                 netdev_accel_fc_tx_thread_unset(dev);
#else
            printk(KERN_ERR "%s: This device does not support FC_TX_THREAD feature\n", __FUNCTION__);
            goto IOCTL_FAILURE;
#endif
        }
        break;
    case BCMNET_IOCTL_GET_ACCEL:
         {
             info.accel.gdx_rx = is_netdev_accel_gdx_rx(dev) ? 1 : 0 ;
             info.accel.gdx_tx = is_netdev_accel_gdx_tx(dev) ? 1 : 0 ;
             info.accel.gdx_hw = is_netdev_hw_accel_gdx(dev) ? 1 : 0 ;
             info.accel.gdx_debug = is_netdev_accel_gdx_debug(dev) ? 1 : 0 ;
             info.accel.tc_egress = is_netdev_accel_tc_egress(dev) ? 1 : 0 ;
#if defined(CONFIG_FCACHE_TX_THREAD)
             info.accel.fc_tx_thread = is_netdev_accel_fc_tx_thread(dev) ? 1 : 0 ;
#else
             info.accel.fc_tx_thread = 0 ;
#endif

             if(copy_to_user((void*) arg, (void*)&info, sizeof(info)))
                printk(KERN_ERR "%s: failed copy data to user (BCMNET_IOCTL_GET_ACCEL)!\n", __FUNCTION__);
         }
        break;
    case BCMNET_IOCTL_DUMP_NETDEV_DEVID:
      /* Dump netdev devid map table */
      devid_dump();
      break;
    case BCMNET_IOCTL_SET_SDN_IGNORE:
        {
            if (info.st_get_ext_flags.ret_val.is_sdn) {
                printk(KERN_DEBUG "dev(%s) netdev_sdn_ignore_set()\n", info.if_name);
                netdev_sdn_ignore_set(dev);
            }
            else {
                printk(KERN_DEBUG "dev(%s) netdev_sdn_ignore_unset()\n", info.if_name);
                netdev_sdn_ignore_unset(dev);
            }
        }
        break;
    case BCMNET_IOCTL_CLR_STATS:  // clear stat is not supported in 4.19
    default:
        printk(KERN_ERR "%s: ioctl failure\n", __FUNCTION__);
        goto IOCTL_FAILURE;
    }
    rtnl_unlock();
    return 0;
IOCTL_FAILURE:
    rtnl_unlock();
    return -1;
}

static const struct file_operations bcmnet_fops =
{
    .unlocked_ioctl = bcmnet_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = bcmnet_ioctl,
#endif
    .open = bcmnet_open
};

#include <linux/if_arp.h>
static void netdev_ppp_setup(struct net_device *dev)
{
    int unit;

    netdev_wan_set(dev);
    netdev_ppp_set(dev);

    sscanf(dev->name, "ppp%d", &unit);
#if 0
    if (unit >= 0)
    {
        unsigned num[3]={0,0,0};
        unsigned u=unit;
         
#define FIELD0    4        /* ppp device number ppp0, ppp1, the third digit (max 16) */
#define FIELD1    8        /* if 0, default mode, 1 vlan mux, 2 msc */    
#define FIELD2    19       /* if FILED1 is 0, add no extension, 1 add vlan id, 2 add conId for msc */

        /* req_name will beused as ifname and  for
         * num[1] == 0:  default connection mode: ppp0, ppp1...
         * num[1] == 1:  vlanMux mode: ppp0.100, ppp1.200...  
         * num[1] == 2:  msc (multiple service mode) ppp0_1, ppp1_3...
         * num[1] == 3:  pppoa0, pppoa1...
         */

        if(u < (1<<FIELD2)) /* Not BRCM PPP naming rule, return directly */ 
            return;

        num[0] = u<<(32-(FIELD2+FIELD1+FIELD0))>>(32-FIELD0);
        num[1] = u<<(32-(FIELD2+FIELD1))>>(32-FIELD1);
        num[2] = u<<(32-(FIELD2))>>(32-FIELD2);
        if (num[1] == 0)
        {
           sprintf(dev->name, "ppp%d", num[0]);
        }
        else if (num[1] == 1) /* vlan mux */
        {
           sprintf(dev->name, "ppp%d.%d", num[0], num[2]);
        }
        else if (num[1] == 2) /* msc */
        {
           sprintf(dev->name, "ppp%d_%d", num[0], num[2]);
        }
        else if (num[1] == 3) /* pppoa */
        {
           sprintf(dev->name, "pppoa%d", num[0]);
        }
    }
#endif
}

#ifdef CONFIG_BLOG

#if IS_ENABLED(CONFIG_NET_IPGRE)
extern bool is_gretap_dev(const struct net_device *dev);
#else
 bool is_gretap_dev(const struct net_device *dev)
 {
	return false;
 }
#endif

#if IS_ENABLED(CONFIG_IPV6_GRE)
extern bool is_ip6gretap_dev(const struct net_device *dev);
#else
bool is_ip6gretap_dev(const struct net_device *dev)
{
	return false;
}
#endif

static void bcm_init_blog_stats_flags(struct net_device *dev)
{
    unsigned int chk_flags = 0;

	/* if stats are already intialized dont override */
	if(bcm_netdev_ext_field_get(dev, blog_stats_flags))
		return;
    
	/* Bridge */
    chk_flags |= IFF_EBRIDGE;

	/* OVS */
    chk_flags |= IFF_OPENVSWITCH;
    
    /*VLAN */
    chk_flags |= IFF_802_1Q_VLAN;

	/* MAC VLAN */
    chk_flags |= (IFF_MACVLAN | IFF_MACVLAN_PORT );


	if(dev->priv_flags & chk_flags)
	{
        bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
        return;
	}

	/* identify GRE TAP devices */
	if ( netif_is_ip6gretap(dev) || netif_is_gretap(dev))
	{
		bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
		return;
	}

    
    if((dev->dev.type) && (dev->dev.type->name) && (strcmp(dev->dev.type->name, "vxlan")==0)) 
    {
        bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
    }

#if defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE)
    if(dev->type == ARPHRD_NONE)
    {
        uint32_t sig = *((uint32_t *)netdev_priv(dev));

        if (sig == NAT46_DEVICE_SIGNATURE)
        {
            bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
        }
    }
#endif

    /* identify device based on device type */
	/* PPP, tunnels (IPGRE , IPGRE6 etc..) */
    switch(dev->type)
    {
        case ARPHRD_PPP:
        case ARPHRD_SIT:
        case ARPHRD_TUNNEL6:
        case ARPHRD_IPGRE:
        case ARPHRD_IP6GRE:
            bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
            return;

        default:
            return;
    }
}
#endif

#ifdef CONFIG_BLOG
static void bcm_netdev_update_mtu_setting(struct net_device *dev)
{
    if (is_netdev_bcm_dev(dev))
    {
        /* Set the max_mtu to zero so each device driver will check the MTU update */
        dev->max_mtu = 0;
    }
}
#endif

static void inherit_flags(struct net_device *lowerdev, struct net_device *dev)
{
    /* Inherit the realDev flags as long as the real device is not a bridge */
    if(!(lowerdev->priv_flags & IFF_EBRIDGE))
    {
        /*IFF_BRIDGE_PORT/IFF_MACVLAN_PORT should be set when the real action happens,
          not inherited from the lowerdev*/
        dev->priv_flags |= (lowerdev->priv_flags & ~IFF_BRIDGE_PORT & ~IFF_MACVLAN_PORT);
    }

    bcm_netdev_ext_inherit(lowerdev, dev);

    /* Clear the bonding flags for the newly created device */
    dev->priv_flags &= ~(IFF_BONDING);
    dev->flags &= ~(IFF_MASTER | IFF_SLAVE);
    dev->state  = (lowerdev->state & ((1<<__LINK_STATE_NOCARRIER) |
                        (1<<__LINK_STATE_DORMANT))) | (1<<__LINK_STATE_PRESENT);
}


static int netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    struct net_device *next_dev;
    int ret = NOTIFY_DONE;

    switch (event)
    {
    case NETDEV_REGISTER:
#ifdef CONFIG_BLOG
        bcm_init_blog_stats_flags(dev);
        bcm_netdev_update_mtu_setting(dev);
#endif
        if (devid_register_netdev(dev,bcm_netdev_ext_field_get_ptr(dev, devid)))
        {
            printk(KERN_ERR "Failed to create netdev devid mapping for %s netdevice\n",dev->name);
        }
        break;

    case NETDEV_UNREGISTER:
        if (dev->type == ARPHRD_PPP)
        {
            next_dev = netdev_path_next_dev(dev);
            if (netdev_path_remove(dev))
            {
                printk("NETDEV_UNREGISTER: Failed to remove %s from Interface path\n", dev->name);
                netdev_path_dump(dev);
            }
            if (next_dev != NULL)
                dev_put(next_dev);
        }
        if (devid_unregister_netdev(bcm_netdev_ext_field_get(dev,devid)))
        {
            printk(KERN_ERR "Failed to delete netdev devid mapping for %s netdevice\n",dev->name);
        }
        break;

    case NETDEV_POST_INIT:
        if (dev->type == ARPHRD_PPP)
            netdev_ppp_setup(dev);
        break;
        
    case NETDEV_CHANGEUPPER:
        {
            struct netdev_notifier_changeupper_info *upper_info = container_of(ptr, struct netdev_notifier_changeupper_info, info);
            struct net_device *upper_dev = upper_info->upper_dev;
            
            if (upper_info->linking)
            {
                atomic_add(MAC_LIMIT(dev).min, (atomic_t *)&MAC_LIMIT(upper_dev).reserve);
                
                /* Currently only three interfaces will do inherit_flags here
                  (1)vfrwd dev created by "ip link add link" 
                  (2)macvlan dev created by "ip link add link"  
                  (3)vlan type dev created by "ip link add link" 
                */
                if (is_netdev_vfrwd(upper_dev) ||
                    (upper_dev->priv_flags & IFF_MACVLAN) ||
                    (upper_dev->priv_flags & IFF_802_1Q_VLAN))
                {
                    inherit_flags(dev, upper_dev);
                    netdev_path_dump(upper_dev);
                }
            }
            else
                atomic_sub(MAC_LIMIT(dev).min, (atomic_t *)&MAC_LIMIT(upper_dev).reserve);
            break;
        }
    }

    return ret;
}

static struct notifier_block netdev_notifier = {
    .notifier_call = netdev_event,
};

static void bcmnet_ioctl_deinit(void)
{
	if (!IS_ERR(bcm_netdev_device)) {
		device_destroy(bcm_netdev_class, MKDEV(bcm_netdev_major, 0));
		cdev_del(&bcm_netdev_cdev);
	}

	if (!IS_ERR(bcm_netdev_class))
		class_destroy(bcm_netdev_class);

	if (bcm_netdev_major)
		unregister_chrdev_region(MKDEV(bcm_netdev_major, 0), 1);
}

static int __init bcmnet_ioctl_init(void)
{
	dev_t dev = 0;
	dev_t devno;
	int rc;

	rc = alloc_chrdev_region(&dev, 0, 1, BCMNET_DRV_NAME);
	if (rc < 0) {
		pr_err("%s:alloc_chrdev_region() failed\n", __func__);
		return -ENODEV;
	}
	bcm_netdev_major = MAJOR(dev);

	/* create device and class */
	bcm_netdev_class = class_create(THIS_MODULE, BCMNET_DRV_NAME);
	if (IS_ERR(bcm_netdev_class)) {
		rc = PTR_ERR(bcm_netdev_class);
		pr_err("%s:Fail to create class %s, rc = %d\n", __func__,
		       BCMNET_DRV_NAME, rc);
		goto fail;
	}

	devno = MKDEV(bcm_netdev_major, 0);
	cdev_init(&bcm_netdev_cdev, &bcmnet_fops);
	bcm_netdev_cdev.owner = THIS_MODULE;

	rc = cdev_add(&bcm_netdev_cdev, devno, 1);
	if (rc) {
		pr_err("%s:Fail to add cdev %s, rc = %d\n", __func__,
		       BCMNET_DRV_NAME, rc);
		goto fail;
	}

	bcm_netdev_device = device_create(bcm_netdev_class, NULL, devno, NULL,
					  BCMNET_DRV_NAME);
	if (IS_ERR(bcm_netdev_device)) {
		rc = PTR_ERR(bcm_netdev_device);
		pr_err("%s:Fail to create device %s, rc = %d\n", __func__,
		       BCMNET_DRV_NAME, rc);
		goto fail;
	}

	pr_info("Char device /dev/%s(%d) registered\n", BCMNET_DRV_NAME,
		bcm_netdev_major);

	/* Initialize netdev_devid_tbl */
	netdev_devid_tbl_init();

	register_netdevice_notifier(&netdev_notifier);

	return 0;

fail:
	bcmnet_ioctl_deinit();
	return rc;
}

static rx_handler_result_t vlan_hook_adapter(struct sk_buff **pskb)
{

#ifdef CONFIG_BCM_TMS_MODULE
    struct sk_buff *skb = *pskb;
    /* Check if 802.1ag service is started. */
    if (bcm_1ag_handle_frame_check_hook) {
        /* Skip vlan handler for 1ag packet. */
        if (bcm_1ag_handle_frame_check_hook(skb))
            return RX_HANDLER_PASS;
    }
    /* Check if 802.3ah service is started. */
    if (bcm_3ah_handle_frame_check_hook) {
        /* Skip vlan handler for 3ah packet, or for any packet
           when 3ah loopback mode was enabled. */
        if ((bcm_3ah_handle_frame_check_hook(skb->dev, ntohs(skb->protocol), skb->data)))
            return RX_HANDLER_PASS;
    }
#endif

    if (bcm_vlan_handle_frame_hook(pskb))
        return RX_HANDLER_CONSUMED;
    else
        return RX_HANDLER_ANOTHER;
}

int bcm_attach_vlan_hook(struct net_device *dev)
{
    rx_handler_func_t *rx_handler = rcu_dereference(dev->rx_handler);
    if (rx_handler)
    {
        if (rx_handler == vlan_hook_adapter)
        {
            int *rc = rcu_dereference(dev->rx_handler_data);
            rc[0]++;
            return 0;
        }
        else
            return -EBUSY;
    }
    else
    {
        int *rc = kmalloc(sizeof(*rc), GFP_KERNEL);
        int ret = netdev_rx_handler_register(dev, vlan_hook_adapter, rc);
        if (ret)
            kfree(rc);
        else
            rc[0] = 1;
        return ret;
    }
}
EXPORT_SYMBOL(bcm_attach_vlan_hook);

void bcm_detach_vlan_hook(struct net_device *dev)
{
    rx_handler_func_t *rx_handler = rcu_dereference(dev->rx_handler);
    int *rc;

    if (rx_handler != vlan_hook_adapter)
        return;

    rc = rcu_dereference(dev->rx_handler_data);
    rc[0]--;
    if (!rc[0])
    {
        kfree(rc);
        netdev_rx_handler_unregister(dev);
    }
}
EXPORT_SYMBOL(bcm_detach_vlan_hook);

void bcm_netdev_ext_inherit(struct net_device *parent, struct net_device * child)
{
    // inherit iff_flags
    if (is_netdev_wan(parent))      netdev_wan_set(child);
    if (is_netdev_wlan(parent))     netdev_wlan_set(child);
        
    // inherit other bcm_netdev_ext fields

    // also conditional inheritance can be done, for example if child device is vlan
    // if (is_netdev_vlan(child)) ....
}
EXPORT_SYMBOL(bcm_netdev_ext_inherit);

void bcm_dev_put(struct net_device *dev)
{
    dev_put(dev);
}
EXPORT_SYMBOL(bcm_dev_put);

void bcm_dev_hold(struct net_device *dev)
{
    dev_hold(dev);
}
EXPORT_SYMBOL(bcm_dev_hold);

subsys_initcall(bcmnet_ioctl_init);
