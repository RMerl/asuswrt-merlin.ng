#if defined(CONFIG_BCM_KF_NETFILTER)
#include <linux/module.h>
#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/times.h>
#include <net/net_namespace.h>
#include <asm/uaccess.h>
#include "br_private.h"

#if defined(CONFIG_BCM_KF_BRIDGE_PORT_ISOLATION)
static RAW_NOTIFIER_HEAD(bridge_event_chain);

void br_dev_notify_if_change(struct net_device *br_dev, struct net_device *dev, int isadd)
{
    struct bridge_notifier_info info;

    info.br_dev = br_dev;
    info.dev    = dev;
    info.isadd  = isadd;
	raw_notifier_call_chain(&bridge_event_chain, BREVT_IF_CHANGED, &info);
}

/* NOTE -- IMPORTANT : Caller MUST take the RCU_READ_LOCK */
void bridge_get_br_list(char *brList, const unsigned int listSize)
{
    struct net_device *dev = NULL;
    unsigned int arrayIdx=0, brNameLen;

    /* Must enable Kernel debugging features and CONFIG_DEBUG_LOCK_ALLOC to make following statement take effect */
    BUG_ON(!rcu_read_lock_held()); 

    for_each_netdev_rcu(&init_net, dev) {
        if(dev->priv_flags & IFF_EBRIDGE)
        {
            if (arrayIdx > 0 && arrayIdx+1 <= listSize)
            {
                /* Bridge names separated by comma */
                brList[arrayIdx++] = ',';
            }

            brNameLen = strlen(dev->name);
            if (arrayIdx+brNameLen+1 > listSize)
            {
                printk("bridge_get_br_list() : insufficient size; skipping <%s> <%d>\n",
                       dev->name,brNameLen);
                brList[arrayIdx-1] = '\0'; /* Remove the trailing "," if present */
                break;
            }
            strcpy(&brList[arrayIdx],dev->name);
            arrayIdx += brNameLen; /* Intentionally not accounting for NULL towards the end */
        }
    }
    brList[arrayIdx] = '\0'; /* Force Null terminated string */

}
/* NOTE -- IMPORTANT : Caller MUST take the RCU_READ_LOCK */
struct net_device *bridge_get_next_port(char *brName, unsigned int *brPort)
{
    struct net_bridge_port *cp;
    struct net_bridge_port *np;
    struct net_bridge *br;
    struct net_device *dev;
    struct net_device *prtDev;

    /* Must enable Kernel debugging features and CONFIG_DEBUG_LOCK_ALLOC to make following statement take effect */
    BUG_ON(!rcu_read_lock_held());

    dev = dev_get_by_name(&init_net, brName);
    if(!dev)
        return NULL;

    br = netdev_priv(dev);
    if (list_empty(&br->port_list))
    {
        dev_put(dev);
        return NULL;
    }

    if (*brPort == 0xFFFFFFFF)
    {
        np = list_first_entry(&br->port_list, struct net_bridge_port, list);
        *brPort = np->port_no;
        prtDev = np->dev;
    }
    else
    {
        cp = br_get_port(br, *brPort);
        if ( cp )
        {
           if (list_is_last(&cp->list, &br->port_list))
           {
               prtDev = NULL;
           }
           else
           {
              np = list_first_entry(&cp->list, struct net_bridge_port, list);
              *brPort = np->port_no;
              prtDev = np->dev;
           }
        }
        else
        {
           prtDev = NULL;
        }
    }

    dev_put(dev);
    return prtDev;
}
EXPORT_SYMBOL(bridge_get_next_port);
EXPORT_SYMBOL(bridge_get_br_list);


int register_bridge_notifier(struct notifier_block *nb)
{
    return raw_notifier_chain_register(&bridge_event_chain, nb);
}
EXPORT_SYMBOL(register_bridge_notifier);

int unregister_bridge_notifier(struct notifier_block *nb)
{
    return raw_notifier_chain_unregister(&bridge_event_chain, nb);
}
EXPORT_SYMBOL(unregister_bridge_notifier);
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_STP)
static RAW_NOTIFIER_HEAD(bridge_stp_event_chain);

void br_stp_notify_state_port(const struct net_bridge_port *p)
{
	struct stpPortInfo portInfo;

	memcpy(&portInfo.portName[0], p->dev->name, IFNAMSIZ);
	portInfo.stpState = ( BR_NO_STP == p->br->stp_enabled ) ? BR_STATE_OFF : p->state;
	raw_notifier_call_chain(&bridge_stp_event_chain, BREVT_STP_STATE_CHANGED, &portInfo);
}

void br_stp_notify_state_bridge(const struct net_bridge *br)
{
	struct net_bridge_port *p;
	struct stpPortInfo portInfo;

	rcu_read_lock();
	list_for_each_entry_rcu(p, &br->port_list, list) {
		if ( BR_NO_STP == br->stp_enabled )
		{
			portInfo.stpState = BR_STATE_OFF;
		}
		else
		{
			portInfo.stpState = p->state;
		}
		memcpy(&portInfo.portName[0], p->dev->name, IFNAMSIZ);
		raw_notifier_call_chain(&bridge_stp_event_chain, BREVT_STP_STATE_CHANGED, &portInfo);
	}
	rcu_read_unlock();

}

void call_br_stp_notifiers(unsigned long evt, struct net_device *dev, struct stpPortInfo *ctx)
{
	struct stpPortInfo portInfo;
	memcpy(&portInfo.portName[0], dev->name, IFNAMSIZ);
	portInfo.stpState = ctx->stpState;

	raw_notifier_call_chain(&bridge_stp_event_chain, evt, &portInfo);
}
EXPORT_SYMBOL(call_br_stp_notifiers);

int register_bridge_stp_notifier(struct notifier_block *nb)
{
    return raw_notifier_chain_register(&bridge_stp_event_chain, nb);
}
EXPORT_SYMBOL(register_bridge_stp_notifier);

int unregister_bridge_stp_notifier(struct notifier_block *nb)
{
    return raw_notifier_chain_unregister(&bridge_stp_event_chain, nb);
}
EXPORT_SYMBOL(unregister_bridge_stp_notifier);
#endif

#endif
