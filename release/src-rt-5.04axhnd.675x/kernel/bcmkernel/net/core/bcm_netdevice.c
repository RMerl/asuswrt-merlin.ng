#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>

int (*bcm_vlan_handle_frame_hook)(struct sk_buff **) = NULL;
EXPORT_SYMBOL(bcm_vlan_handle_frame_hook);

#ifdef CONFIG_BCM_TMS_MODULE
int (*bcm_1ag_handle_frame_check_hook)(struct sk_buff *) = NULL;
int (*bcm_3ah_handle_frame_check_hook)(void *dev, unsigned short protocol, unsigned char *datPtr) = NULL;
EXPORT_SYMBOL(bcm_1ag_handle_frame_check_hook);
EXPORT_SYMBOL(bcm_3ah_handle_frame_check_hook);
#endif

extern int bcm_mac_limit_en;
#define MAC_LIMIT(dev) dev->bcm_nd_ext.mac_limit
DEFINE_SPINLOCK(mac_limit_spinlock);
EXPORT_SYMBOL(mac_limit_spinlock);

/*-----------------------------------------------------------------------------------*/
/* bcmnet character device ioctl handler to process get/set netdev BRCM private info */
/*-----------------------------------------------------------------------------------*/
#include <linux/fs.h>
#include <linux/rtnetlink.h>
#include "bcmnet.h"

static int bcmnet_open(struct inode *inode, struct file *filep)
{
    return 0;
}

static int mac_limit_reserve(struct net_device *dev, void * data)
{
    int reserve = *(int *)data;
    
    MAC_LIMIT(dev).reserve += reserve;
    return 0;
}

static int mac_limit_learn(struct net_device *dev, void * data)
{
    int learn = *(int *)data;
    
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
                            netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &rsv);
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
                bcm_mac_limit_en = val;  /* dev is null, global enable */
                if (bcm_mac_limit_en)
                    printk("mac limit enabled\n");
                else
                    printk("mac limit disabled\n");
            }
            else
            {
                int rsv, learn;
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
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_reserve, &rsv);
                    netdev_walk_all_upper_dev_rcu(dev, mac_limit_learn, &learn);
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
            info->st_mac_limit.mac_limit = (void *)compat_info.st_mac_limit.mac_limit;
            break;
        }
        default:
            break;
    }
    return 0;
}
#endif

static long bcmnet_ioctl(struct file *filep, unsigned int command, unsigned long arg)
{
    bcmnet_info_t info;
    struct net_device *dev;

    if (command >= BCMNET_IOCTL_MAX)
        return -1;

    copy_from_user((void*) &info, (void*) arg, sizeof(info));
#if defined(CONFIG_COMPAT)
    if (is_compat_task())
    {
        if (compat_bcmnet_info_get(&info, command, arg))
            return -1;
    }
#endif
    
    rtnl_lock();
    dev = __dev_get_by_name(&init_net, info.if_name);
    if (!dev) {
        if (command != BCMNET_IOCTL_MAC_LIMIT)    //mac limit decide later
            goto IOCTL_FAILURE;
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
		copy_to_user((void*) arg, (void*)&info, sizeof(info));
        break;
    case BCMNET_IOCTL_GET_LAST_CHANGE:
        info.st_get_last_change.last_change =  dev_trans_start(dev);
        copy_to_user((void*) arg, (void*)&info, sizeof(info));
        break;
    case BCMNET_IOCTL_ADD_NETDEV_PATH:
        {
            struct net_device *next_dev;
            int err;
            next_dev = dev_get_by_name(&init_net, info.st_add_netdev_path.next_if_name);
            if (!next_dev) {
                printk("BCMNET_IOCTL_ADD_NETDEV_PATH: Inavlid Next Device Name: %s\n", info.st_add_netdev_path.next_if_name);
                goto IOCTL_FAILURE;
            }
            err = netdev_path_add(dev, next_dev);
            if (err) {
                printk("BCMNET_IOCTL_ADD_NETDEV_PATH: Failed to add %s to Interface path (%d)\n", info.if_name, err);
                dev_put(next_dev);
                goto IOCTL_FAILURE;
            } else
                netdev_path_dump(dev);
        }
        break;
    case BCMNET_IOCTL_MAC_LIMIT:
         if (mac_limit_ioctl(dev, (void*)&info.st_mac_limit))
             goto IOCTL_FAILURE;
         break;
    case BCMNET_IOCTL_CLR_STATS:  // clear stat is not supported in 4.19
    default:
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
        *
        */
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
	if (is_ip6gretap_dev(dev) || is_gretap_dev(dev))
	{
		bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
		return;
	}

    
    if((dev->dev.type) && (dev->dev.type->name) && (strcmp(dev->dev.type->name, "vxlan")==0)) 
    {
        bcm_netdev_ext_field_set(dev, blog_stats_flags, BLOG_DEV_STAT_FLAG_INCLUDE_ALL);
    }

#if defined(CONFIG_BCM_NAT46) || defined(CONFIG_BCM_NAT46_MODULE)
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

static void bcm_netdev_update_mtu_setting(struct net_device *dev)
{
    if (is_netdev_bcm_dev(dev))
    {
        /* Set the max_mtu to zero so each device driver will check the MTU update */
        dev->max_mtu = 0;
    }
}

static int netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    struct net_device *next_dev;

    switch (event)
    {
	case NETDEV_REGISTER:
#ifdef CONFIG_BLOG
		bcm_init_blog_stats_flags(dev);
        bcm_netdev_update_mtu_setting(dev);
#endif
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
                atomic_add(MAC_LIMIT(dev).min, (atomic_t *)&MAC_LIMIT(upper_dev).reserve);
            else
                atomic_sub(MAC_LIMIT(dev).min, (atomic_t *)&MAC_LIMIT(upper_dev).reserve);
            break;
        }
    }

    return NOTIFY_DONE;
}

static struct notifier_block netdev_notifier = {
    .notifier_call = netdev_event,
};

static int __init bcmnet_ioctl_init(void)
{
    
    if (register_chrdev(BCMNET_DRV_MAJOR, BCMNET_DRV_NAME, &bcmnet_fops))
    {
        printk("unable to get major number %d\n", BCMNET_DRV_MAJOR);
        return -1;
    }
    printk("%s Char device registered\n", BCMNET_DRV_DEVICE_NAME);

    register_netdevice_notifier(&netdev_notifier);
    return 0;
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


subsys_initcall(bcmnet_ioctl_init);
