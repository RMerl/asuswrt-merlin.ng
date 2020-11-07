/*
* <:copyright-BRCM:2018:DUAL/GPL:standard
*
*    Copyright (c) 2018 Broadcom
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
:>
*/
#if defined(BCM_PKTFWD)

#include <linux/netdevice.h>
#include <linux/crc16.h>
#include <bcm_pktfwd.h>


/**
 * =============================================================================
 * Section: BCM_PKTFWD Global System Object(s)
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Singleton global object.
 *
 * - lock             : Global system lock
 * - d3lut            : Lookup table for 802.3 MacAddresses for LAN and WLAN.
 * - initialized      : Flag for initialization state of singleton global
 * - pktqueue_context : Per PKTQUEUE domain, consumer pktqueue_context.
 *
 * -----------------------------------------------------------------------------
 */

typedef struct bcm_pktfwd                /* Global System State */
{
    spinlock_t          lock;           /* system lock - NOT USED */

    d3lut_t           * d3lut;          /* 802.3 MAC Address LUT */
    bool                initialized;    /* global system initialized */
    pktqueue_context_t  pktqueue_context[PKTQUEUE_DOMAINS_TOT]; /* Upstream */

} bcm_pktfwd_t;


/** Static initialization of singleton system global object */
bcm_pktfwd_t bcm_pktfwd_g =
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    .lock               = __SPIN_LOCK_UNLOCKED(bcm_pktfwd_g.lock),
#endif
    .d3lut              = D3LUT_NULL,
    .initialized        = false,
    .pktqueue_context   = { },
};


/**
 * -----------------------------------------------------------------------------
 * Function : Construct all bcm_pktfwd subsystems. Function is not re-entrant.
 *            Invoked in bcmlibs_module_init (see bcm_libs_module.c).
 *
 * Initialize the D3LUT system with a pool of D3LUT_ELEM_TOT elements.
 * By default, all pools will use by-freelist.
 *
 * -----------------------------------------------------------------------------
 */
int
bcm_pktfwd_sys_init(void)
{

    uint32_t                domain;
    d3lut_t               * d3lut;
    pktqueue_context_t    * pktqueue_context;
    bcm_pktfwd_t          * bcm_pktfwd = &bcm_pktfwd_g;

    PKTFWD_FUNC();

    if (bcm_pktfwd->initialized == true) /* global system already setup, bail */
    {
        PKTFWD_ERROR("already initialized");
        return BCM_PKTFWD_SUCCESS;
    }

    PKTFWD_TRACE("D3LUT_ELEM %d %d\n", D3LUT_ELEM_TOT, D3LUT_ELEM_MAX);

    //spin_lock_init(&bcm_pktfwd->lock);

    /* Reset pktqueue_contexts pool */
    for (domain = 0U; domain < PKTQUEUE_DOMAINS_TOT; ++domain)
    {
        pktqueue_context = &bcm_pktfwd->pktqueue_context[domain];
        pktqueue_context_reset(pktqueue_context);
    }

    /* Initialize the global D3 Lookup Table */
    d3lut = d3lut_init(D3LUT_ELEM_TOT);
    if (d3lut == D3LUT_NULL) {
        PKTFWD_ERROR("d3lut init failure");
        goto bcm_pktfwd_sys_init_failure;
    }

    bcm_pktfwd->d3lut = d3lut;

    PKTFWD_TRACE(" System Constructed");

    return BCM_PKTFWD_SUCCESS;

bcm_pktfwd_sys_init_failure:

    PKTFWD_ERROR("System Construction Failure");

    bcm_pktfwd_sys_fini();

    return BCM_PKTFWD_FAILURE;

}   /* bcm_pktfwd_sys_init() */

/**
 * -----------------------------------------------------------------------------
 * Function : Destructor for the PKTFWD global system.
 *            Invoked by bcmlibs_module_exit() and
 *            when bcm_pktfwd_sys_init() fails.
 * -----------------------------------------------------------------------------
 */
void
bcm_pktfwd_sys_fini(void)
{
    d3lut_t       * d3lut;
    bcm_pktfwd_t  * bcm_pktfwd = &bcm_pktfwd_g;

    PKTFWD_FUNC();

    bcm_pktfwd->initialized = false;

    d3lut = bcm_pktfwd->d3lut;              /* D3 lookup table */
    bcm_pktfwd->d3lut = D3LUT_NULL;

    if (d3lut != D3LUT_NULL)                /* Destruct the D3 lookup table */
        d3lut_fini(d3lut);

    PKTFWD_TRACE(" System Destructed");

}   /* bcm_pktfwd_sys_fini() */


#if defined(BCM_PKTLIST)

/**
 * -----------------------------------------------------------------------------
 * Section: BCM_PKTLIST Abstract Data Type
 * -----------------------------------------------------------------------------
 */

/**
 * =============================================================================
 * Packet List management library between an ingress and egress network device.
 * Used in the WFD to WLAN transfer.
 * =============================================================================
 */

/**
 * List of all instantiated pktlist_context.
 * When a pktlist_context (producer or consumer) is instantiate, it's instance
 * is linked into this global pktlist_context_instances_g dll. The global
 * list of all instance may be used in debugging.
 */
static dll_t pktlist_context_instances_g =
{
    .next_p = &pktlist_context_instances_g,
    .prev_p = &pktlist_context_instances_g
};


/**
 * -----------------------------------------------------------------------------
 * Function   : pktlist_context_init
 * Description: Initialize a pktlist_context_t and attach a dispatch handler.
 *
 *              Allocate a pktlist_table_t.
 *              Initialize all lists (mcast and by ucast[prio]) and free list.
 *              Initialize all pktlist_elem_t in the pktlist_table_t and place
 *              then in the free work list (i.e. all pktlists are empty).
 *              Attach peer context and "xfer" handler.
 *              Bind parent driver context.
 *
 * Impl Caveat: Runtime kmalloc of pktlist_table_t, and size is dependent
 *              on a number of destinations supported.
 *
 * -----------------------------------------------------------------------------
 */

pktlist_context_t *
pktlist_context_init(
    pktlist_context_t * pktlist_context_peer,
    pktlist_context_xfer_fn_t pktlist_context_xfer_fn,
    pktlist_context_keymap_fn_t pktlist_context_keymap_fn,
    void * driver, const char * driver_name, uint32_t unit)
{
    uint32_t prio, dest;

    pktlist_t         * pktlist;
    pktlist_elem_t    * pktlist_elem;
    pktlist_table_t   * pktlist_table   = PKTLIST_TABLE_NULL;
    pktlist_context_t * pktlist_context = PKTLIST_CONTEXT_NULL;

    PKTLIST_TRACE("%s pktlist_context_init", driver_name);

    /* Assert Initialization Params */
    PKTLIST_ASSERT(driver != (void *)NULL);
    PKTLIST_ASSERT(driver_name != (const char *)NULL);
    /* pktlist_context_peer can be NULL */

    if (pktlist_context_xfer_fn != (pktlist_context_xfer_fn_t)NULL) {
        PKTLIST_ASSERT(pktlist_context_peer != PKTLIST_CONTEXT_NULL);
    }

    PKTLIST_TRACE("%s pktlist_context_t size %u",
        driver_name, (uint32_t)sizeof(pktlist_context_t));

    pktlist_context = kmalloc(sizeof(pktlist_context_t), GFP_ATOMIC);
    if (pktlist_context == PKTLIST_CONTEXT_NULL) {
        PKTLIST_ERROR("%s pktlist_context_t kmalloc %u failure",
            driver_name, (uint32_t)sizeof(pktlist_context_t));
        goto pktlist_context_init_done;
    }

    memset(pktlist_context, 0, sizeof(pktlist_context_t));

    /* pktlist_table_t is dynamically allocated */
    PKTLIST_TRACE("%s pktlist_table_t size %u",
        driver_name, (uint32_t) sizeof(pktlist_table_t));

    pktlist_table = kmalloc(sizeof(pktlist_table_t), GFP_ATOMIC);
    if (pktlist_table == PKTLIST_TABLE_NULL) {
        kfree(pktlist_context);
        pktlist_context = PKTLIST_CONTEXT_NULL;

        PKTLIST_ERROR("%s pktlist_table_t kmalloc %u failure",
            driver_name, (uint32_t)sizeof(pktlist_table_t));

        goto pktlist_context_init_done;
    }

    /* Any fields statically initialized will be zeroed in memset above */

    spin_lock_init(&pktlist_context->lock);
    pktlist_context->dispatches = 0U;

    /* Attach table to pktlist_context */
    pktlist_context->table = pktlist_table;

    /* Initialize all elements in table and place in free list */
    dll_init(&pktlist_context->free);

    dll_init(&pktlist_context->mcast); /* initialize mcast list */
    for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        dll_init(&pktlist_context->ucast[prio]); /* initialize ucast list */

        for (dest = 0U; dest < PKTLIST_DEST_MAX; ++dest)
        {
            pktlist_elem = &pktlist_table->elem[prio][dest];
            dll_init(&pktlist_elem->node);
            dll_append(&pktlist_context->free, &pktlist_elem->node);

            pktlist = &pktlist_elem->pktlist;

            pktlist->dest  = dest; /* never reset */
            pktlist->prio  = prio; /* never reset */

            PKTLIST_RESET(pktlist); /* only len MUST be reset */
        }
    }

    /* Setup peer's transfer configuration */
    pktlist_context->peer    = pktlist_context_peer;
    pktlist_context->xfer_fn = pktlist_context_xfer_fn;
    pktlist_context->keymap_fn = pktlist_context_keymap_fn;

    /* Setup parent driver context owning this pktlist_context */
    pktlist_context->unit   = unit;
    pktlist_context->driver = driver;
    strncpy(pktlist_context->driver_name, driver_name, PKTLIST_CTX_NAME_SZ - 1);
    pktlist_context->driver_name[PKTLIST_CTX_NAME_SZ-1] = '\0';

    /* Add new instance of pktlist context to list of instances (for show) */
    dll_init(&pktlist_context->instance);
    dll_append(&pktlist_context_instances_g, &pktlist_context->instance);


    PKTLIST_TRACE(PKTFWD_VRP_FMT " Constructed %p for %s",
                  PKTFWD_VRP_VAL(PKTLIST, PKTLIST_VERSIONCODE),
                  pktlist_context, pktlist_context->driver_name);

pktlist_context_init_done:
    return pktlist_context;

}   /* pktlist_context_init() */


/**
 * -----------------------------------------------------------------------------
 * Function   : pktlist_context_fini
 * Description: Free the pktlist_table_t
 *
 * Impl Caveat: No check is done whether any pktlist has pending packets.
 *              pktlist_context_fini() mayonly be invoked on an initialized
 *              pktlist_context.
 * -----------------------------------------------------------------------------
 */

pktlist_context_t *
pktlist_context_fini(pktlist_context_t * pktlist_context)
{
    PKTLIST_FUNC();

    if (pktlist_context == PKTLIST_CONTEXT_NULL)
        return PKTLIST_CONTEXT_NULL;

    /* Assumes pktlist_context was initialized, apriori */
    PKTLIST_ASSERT(pktlist_context->table != PKTLIST_TABLE_NULL);

    PKTLIST_TRACE("Destructing %s ...", pktlist_context->driver_name);

    /* Delete from global pktlist_context instances dll */
    dll_delete(&pktlist_context->instance);

    kfree(pktlist_context->table);
    memset(pktlist_context, 0xff, sizeof(pktlist_context_t)); /* scribble */
    kfree(pktlist_context);

    return PKTLIST_CONTEXT_NULL;

}   /* pktlist_context_fini() */


/**
 * -----------------------------------------------------------------------------
 * Function   : pktlist_context_dump
 * Description: Dump a pktlist_context_t object, with verbose pktlist_t dump
 *
 *              No dll may be traversed, as no spinlock is taken.
 *              Also, the tail packet in a pktlist is not terminated by NULL.
 * -----------------------------------------------------------------------------
 */
static inline void
__pktlist_context_dump(pktlist_context_t * pktlist_context)
{
    printk("%s pktlist%u <%u:%u> dispatch<%u> list<%u> pkts<%u> peer<%p,%pS>\n",
        pktlist_context->driver_name, pktlist_context->unit,
        PKTLIST_PRIO_MAX, PKTLIST_DEST_MAX,
        pktlist_context->dispatches, pktlist_context->list_stats,
        pktlist_context->pkts_stats,
        pktlist_context->peer, pktlist_context->xfer_fn);

}   /* __pktlist_context_dump() */


void
pktlist_context_dump(pktlist_context_t * pktlist_context,
    bool dump_peer, bool dump_verbose)
{
    int32_t credits = ~0;
    uint32_t prio, dest;
    uint32_t total_len   = 0U;

    pktlist_t * pktlist;

    PKTLIST_ASSERT(pktlist_context != PKTLIST_CONTEXT_NULL);

    __pktlist_context_dump(pktlist_context);

    if (dump_peer == true)
    {
        if (pktlist_context->peer != PKTLIST_CONTEXT_NULL)
            __pktlist_context_dump(pktlist_context->peer);
        else
            printk("\t Peer: is not initialized\n");
    }

    if (dump_verbose == false)
        return;

    printk(" DEST PRIO KEY  PACKETS    CREDITS\n");
    for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        for (dest = 0U; dest < PKTLIST_DEST_MAX; ++dest)
        {
            pktlist = &pktlist_context->table->elem[prio][dest].pktlist;
            if (pktlist->len != 0)
            {
#if defined(BCM_PKTFWD_FLCTL)
                if (pktlist_context->fctable != PKTLIST_FCTABLE_NULL)
                {
                    credits = __pktlist_fctable_get_credits(pktlist_context,
                                                            prio, dest);
                }
#endif /* BCM_PKTFWD_FLCTL */

                printk(PKTLIST_FMT "    %d\n", PKTLIST_VAL(pktlist), credits);
                total_len   += pktlist->len;
            }
        }
    }
    printk("                =======\n           %8u\n\n", total_len);

}   /* pktlist_context_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function   : pktlist_context_dump_all
 * Description: Dump all initialized pktlist_context_t objects
 * Impl Caveat: Walking dll pktlist_context_instances_g without lock.
 * -----------------------------------------------------------------------------
 */

void /* Caution No lock is taken */
pktlist_context_dump_all(void)
{
    dll_t             * instance_p;
    pktlist_context_t * pktlist_context;
    bool dump_peer    = false;
    bool dump_verbose = false;

#if defined(CC_PKTLIST_DEBUG)
    dump_verbose = true;
#endif

    dll_for_each(instance_p, &pktlist_context_instances_g)
    {
        pktlist_context = _envelope_of(instance_p, pktlist_context_t, instance);
        pktlist_context_dump(pktlist_context, dump_peer, dump_verbose);
    }

}   /* pktlist_context_dump_all() */

EXPORT_SYMBOL(pktlist_context_init);
EXPORT_SYMBOL(pktlist_context_fini);
EXPORT_SYMBOL(pktlist_context_dump);
EXPORT_SYMBOL(pktlist_context_dump_all);


#endif /* BCM_PKTLIST */


#if defined(BCM_PKTQUEUE)

/**
 * -----------------------------------------------------------------------------
 * Section: BCM_PKTQUEUE Abstract Data Type
 * -----------------------------------------------------------------------------
 */

/**
 * =============================================================================
 * Packet Queue management library between an ingress and egress network device.
 * Used in the WLAN to WFD/LAN transfer.
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function   : pktqueue_context_register
 * Description: Initialize a pktqueue_context_t
 *              Register egress driver "flush" and "flush_complete" handlers.
 *              Bind egress network device context.
 * -----------------------------------------------------------------------------
 */

pktqueue_context_t *
pktqueue_context_register(
        pktqueue_flush_pkts_fn_t        flush_pkts_fn,
        pktqueue_flush_complete_fn_t    flush_complete_fn,
        void * driver, uint32_t domain)
{
    pktqueue_context_t * pktqueue_context;
    bcm_pktfwd_t       * bcm_pktfwd = &bcm_pktfwd_g;

    /* Assert Initialization Params */
    PKTQUEUE_ASSERT(driver != (void *)NULL);
    PKTQUEUE_ASSERT(domain < PKTQUEUE_DOMAINS_TOT);
    PKTQUEUE_ASSERT(flush_pkts_fn != PKTQUEUE_FLUSH_PKTS_NULL);
    PKTQUEUE_ASSERT(flush_complete_fn != PKTQUEUE_FLUSH_COMPLETE_NULL);

    PKTQUEUE_TRACE("Domain %d pktqueue_context_register", domain);

    pktqueue_context = &bcm_pktfwd->pktqueue_context[domain];

    if (pktqueue_context->initialized == true)
    {
        PKTFWD_ERROR("already registered");
        return pktqueue_context;
    }

    pktqueue_context->queue_stats           = 0;
    pktqueue_context->pkts_stats            = 0;
    pktqueue_context->domain                = domain;
    pktqueue_context->driver                = driver;
    pktqueue_context->flush_pkts_fn         = flush_pkts_fn;
    pktqueue_context->flush_complete_fn     = flush_complete_fn;
    pktqueue_context->initialized           = true;

    PKTQUEUE_TRACE(PKTFWD_VRP_FMT " Constructed %p for %d",
                   PKTFWD_VRP_VAL(PKTQUEUE, PKTQUEUE_VERSIONCODE),
                   pktqueue_context, pktqueue_context->domain);

    return pktqueue_context;

}   /* pktqueue_context_register() */


/**
 * -----------------------------------------------------------------------------
 * Function: Reset a pktqueue_context
 * -----------------------------------------------------------------------------
 */

void
pktqueue_context_reset(pktqueue_context_t * pktqueue_context)
{
    pktqueue_context->queue_stats           = 0;
    pktqueue_context->pkts_stats            = 0;
    pktqueue_context->domain                = ~0;
    pktqueue_context->driver                = NULL;
    pktqueue_context->flush_pkts_fn         = NULL;
    pktqueue_context->flush_complete_fn     = NULL;
    pktqueue_context->initialized           = false;

}   /* pktqueue_context_reset() */


 /**
 * -----------------------------------------------------------------------------
 * Function   : pktqueue_context_unregister
 * Description: Destruct a pktqueue_context_t
 * -----------------------------------------------------------------------------
 */

pktqueue_context_t *
pktqueue_context_unregister(pktqueue_context_t * pktqueue_context)
{
    PKTQUEUE_FUNC();

    if (pktqueue_context == PKTQUEUE_CONTEXT_NULL)
        return PKTQUEUE_CONTEXT_NULL;

    PKTQUEUE_ASSERT(pktqueue_context ==
                &(bcm_pktfwd_g.pktqueue_context[pktqueue_context->domain]));

    PKTQUEUE_TRACE("Destructing pktqueue_context[%d]...",
                    pktqueue_context->domain);

    pktqueue_context_reset(pktqueue_context);

    return PKTQUEUE_CONTEXT_NULL;

}   /* pktqueue_context_unregister() */


/**
 * -----------------------------------------------------------------------------
 *  Function    : pktqueue_get_domain_pktqueue_context
 *  Description : Get egress driver pktqueue_context identified by domain idx
 * -----------------------------------------------------------------------------
 */

pktqueue_context_t *
pktqueue_get_domain_pktqueue_context(uint32_t domain)
{
    pktqueue_context_t * pktqueue_context;

    PKTQUEUE_ASSERT(domain < PKTQUEUE_DOMAINS_TOT);

    pktqueue_context = &(bcm_pktfwd_g.pktqueue_context[domain]);

    if (pktqueue_context->initialized)
        return pktqueue_context;

    return PKTQUEUE_CONTEXT_NULL;

}   /* pktqueue_get_domain_pktqueue_context() */


/**
 * -----------------------------------------------------------------------------
 * Function   : pktqueue_context_dump
 * Description: Dump a pktqueue_context_t object
 * -----------------------------------------------------------------------------
 */

void
pktqueue_context_dump(pktqueue_context_t * pktqueue_context)
{
    PKTQUEUE_ASSERT(pktqueue_context != PKTQUEUE_CONTEXT_NULL);

    printk("pktqueue domian %u <tot:%u> queue<%u> pkts<%u>\n",
        pktqueue_context->domain, PKTQUEUE_DOMAINS_TOT,
        pktqueue_context->queue_stats,
        pktqueue_context->pkts_stats);

}   /* pktqueue_context_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function   : pktqueue_context_dump_all
 * Description: Dump all initialized pktqueue_context_t objects
 * -----------------------------------------------------------------------------
 */

void
pktqueue_context_dump_all(void)
{
    uint32_t                domain;
    pktqueue_context_t    * pktqueue_context;
    bcm_pktfwd_t          * bcm_pktfwd = &bcm_pktfwd_g;

    for (domain = 0U; domain < PKTQUEUE_DOMAINS_TOT; ++domain)
    {
        pktqueue_context = &bcm_pktfwd->pktqueue_context[domain];

        if (pktqueue_context->initialized)
            pktqueue_context_dump(pktqueue_context);
    }

}   /* pktqueue_context_dump_all() */

EXPORT_SYMBOL(pktqueue_context_register);
EXPORT_SYMBOL(pktqueue_context_unregister);
EXPORT_SYMBOL(pktqueue_get_domain_pktqueue_context);
EXPORT_SYMBOL(pktqueue_context_dump);
EXPORT_SYMBOL(pktqueue_context_dump_all);


#endif /* BCM_PKTQUEUE */


#if defined(BCM_D3FWD)

/**
 * -----------------------------------------------------------------------------
 * Function: Debug dump a D3FWD WlIf statistics
 * -----------------------------------------------------------------------------
 */

void
d3fwd_wlif_stats_dump(d3fwd_wlif_stats_t * d3fwd_wlif_stats, bool clear_on_read)
{
    uint32_t prio;
    d3fwd_wlif_stats_t * stats;

    /* Dump TX stats */
    printk("\t\t %4s %10s %10s", "prio", "tot_pkts", "tx_drops");
    printk(" %10s %10s", "cfp_pkts", "cfp_fwds");
    printk(" %10s %10s", "chn_pkts", "chn_fwds");
    printk(" %10s", "slow_pkts");
    printk(" %10s %10s\n", "schedule", "complete");

    for (prio = 0U; prio < D3FWD_PRIO_MAX; ++prio)
    {
        stats = d3fwd_wlif_stats + prio;
        printk("\t\t %4u %10u %10u", prio, stats->tot_pkts, stats->tx_drops);
        printk(" %10u %10u", stats->cfp_pkts, stats->cfp_fwds);
        printk(" %10u %10u", stats->chn_pkts, stats->chn_fwds);
	printk(" %10u", stats->slow_pkts);
        printk(" %10u %10u\n", stats->schedule, stats->complete);
    }

    /* Dump RX stats */
    printk("\t\t %4s %10s %10s", "prio", "rx_tot_pkts", "rx_drops");
    printk(" %10s %10s\n", "rx_fast_pkts", "rx_slow_pkts");

    for (prio = 0U; prio < D3FWD_PRIO_MAX; ++prio)
    {
        stats = d3fwd_wlif_stats + prio;
        printk("\t\t %4u %10u %10u", prio, stats->rx_tot_pkts, stats->rx_drops);
        printk(" %10u %10u\n", stats->rx_fast_pkts, stats->rx_slow_pkts);
    }

    if (clear_on_read == true)
        memset(d3fwd_wlif_stats, 0, D3FWD_PRIO_MAX * sizeof(d3fwd_wlif_stats));

}   /* d3fwd_wlif_stats_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function: Debug dump the D3FWD WlIf state. (also d3fwd_wlif stats)
 * -----------------------------------------------------------------------------
 */

void
d3fwd_wlif_dump(d3fwd_wlif_t *d3fwd_wlif)
{
    struct net_device * net_device;

    if (d3fwd_wlif == D3FWD_WLIF_NULL)
        return;

    net_device = d3fwd_wlif->net_device; /* no lock is taken: read name */
    if (net_device == (struct net_device *)NULL) {
        printk("d3fwd_wlif net_device NULL\n");
        return;
    }

    printk("\t\t d3fwd_wlif <%s:%p,%p,%p> unit %u wfd_idx %u stations %u\n",
            net_device->name, net_device, d3fwd_wlif->wlif, d3fwd_wlif,
            d3fwd_wlif->unit, d3fwd_wlif->wfd_idx, d3fwd_wlif->stations);

    D3FWD_STATS_EXPR( /* stats cleared on read */
        d3fwd_wlif_stats_dump(d3fwd_wlif->stats, true);
    )

}   /* d3fwd_wlif_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function: Clear a D3FWD WlIf statistics
 * -----------------------------------------------------------------------------
 */

void
d3fwd_wlif_clr(d3fwd_wlif_t * d3fwd_wlif)
{
    D3FWD_STATS_EXPR(
        memset(d3fwd_wlif->stats, 0,
               sizeof(d3fwd_wlif_stats_t) * D3FWD_PRIO_MAX);
    )
}   /* d3fwd_wlif_clr() */


/**
 * -----------------------------------------------------------------------------
 * Function: Debug dump the D3FWD Extension to a D3LUT Element
 * -----------------------------------------------------------------------------
 */

void
d3fwd_ext_dump(d3fwd_ext_t * d3fwd_ext)
{
    printk("\t ext %p flow_key " D3LUT_KEY_FMT
        "ucast 0x%2x wlan %u assoc %u inuse %u hit %u\n",
        d3fwd_ext->d3fwd_wlif, D3LUT_KEY_VAL(d3fwd_ext->flow_key),
        d3fwd_ext->ucast_bmap, d3fwd_ext->wlan, d3fwd_ext->assoc,
        d3fwd_ext->inuse, d3fwd_ext->hit);
}   /* d3fwd_ext_dump() */

EXPORT_SYMBOL(d3fwd_wlif_stats_dump);
EXPORT_SYMBOL(d3fwd_wlif_dump);
EXPORT_SYMBOL(d3fwd_wlif_clr);
EXPORT_SYMBOL(d3fwd_ext_dump);

#endif /* BCM_D3FWD */


#if defined(BCM_D3LUT)

/**
 * =============================================================================
 * Section: D3LUT Functional Interface
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Helper Functions
 * -----------------------------------------------------------------------------
 */


/**
 * -----------------------------------------------------------------------------
 * Function: d3lut_hash() is used to compute a 16 bit hash on a 6 Byte symbol.
 * Returns a 16 bit hash value, using either an XOR or CRC16 algorithm.
 * Inputs: sym needs to be 16 bit aligned.
 * -----------------------------------------------------------------------------
 */
static inline uint16_t
__d3lut_hash(const uint8_t * sym)
{
    uint16_t hash;
#if defined(CC_D3LUT_XORH)
    const uint16_t * sym16 = (uint16_t *)sym;   /* used in 16bit xor */
    D3LUT_ASSERT_SYM(sym16);                    /* assert 16bit aligned */
    hash = (*(sym16 + 0)) ^ (*(sym16 + 1)) ^ (*(sym16 + 2));
#else  /* ! CC_D3LUT_XORH */
    hash = crc16(0xFFFF, sym, 6);               /* CRC16 computation */
#endif /* ! CC_D3LUT_XORH */

    return hash;

}   /* __d3lut_hash() */

uint16_t d3lut_hash(const uint8_t * sym) { return __d3lut_hash(sym); }


/**
 * -----------------------------------------------------------------------------
 * Function: Compares to 6 Byte symbols sym1 and sym2.
 * Returns 0x0000 if both are equal and a non-zero 16bit value if not equal.
 * Inputs: both sym1 and sym2 need to be 16 bit aligned.
 * -----------------------------------------------------------------------------
 */
static inline uint16_t
__d3lut_cmp(const uint16_t * sym1, const uint16_t * sym2)
{
    return ((*(sym1 + 0) ^ *(sym2 + 0)) |
            (*(sym1 + 1) ^ *(sym2 + 1)) |
            (*(sym1 + 2) ^ *(sym2 + 2)));

}   /* __d3lut_cmp() */


/**
 * -----------------------------------------------------------------------------
 * Function: Sets a dictionary element's 6 Byte symbol dst_sym with src_sym.
 * Inputs: both dst_sym and src_sym need to be 16 bit aligned.
 * -----------------------------------------------------------------------------
 */
static inline void
__d3lut_set(uint16_t * sym_dst, const uint16_t * sym_src)
{
    *(sym_dst + 0) = *(sym_src + 0);
    *(sym_dst + 1) = *(sym_src + 1);
    *(sym_dst + 2) = *(sym_src + 2);

}   /* __d3lut_set() */


#if defined(CC_D3LUT_BFLT)

/**
 * =============================================================================
 * Section: D3LUT Bloom Filter
 *
 * A single hash Bloom Filter is instantiated.
 * Bloom filter comprises of a 64 Kbits, using 2 K x 32 bit words.
 * A 16 bit hash is used synonymously as a bit position in the 64 Kbits bitmap.
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * Function: Determine whether a 16 bit hash exists in the bloom filter by using
 * the 16 bit hash value as a bit position in the 64 Kbit bitmap.
 * Returns a non-zero value if the 16 bit hash exists.
 * -----------------------------------------------------------------------------
 */

static inline uint32_t
__d3lut_bflt_get(const uint16_t bitpos, const uint32_t * bitmap)
{
    return ((*(bitmap + (bitpos >> 5))) & (1U << (bitpos & 31)));
}


/**
 * -----------------------------------------------------------------------------
 * Function: Set a bit in the 64 Kbit bitmap. The 16 bit hash is bit position.
 * -----------------------------------------------------------------------------
 */

static inline void
__d3lut_bflt_set(const uint16_t bitpos, uint32_t * bitmap)
{
    *(bitmap + (bitpos >> 5)) |= (1U << (bitpos & 31));
}


/**
 * -----------------------------------------------------------------------------
 * Function: Clear a bit in the 64 Kbit bitmap. The 16 bit hash is bit position.
 * -----------------------------------------------------------------------------
 */

static inline void
__d3lut_bflt_clr(const uint16_t bitpos, uint32_t * bitmap)
{
    *(bitmap + (bitpos >> 5)) &= ~(1U << (bitpos & 31));
}


/**
 * -----------------------------------------------------------------------------
 * Function: Insert a 16 bit hash into the bloom filter. Record the 16 bit hash
 * for the element identified by the element index, for use in delete algorithm.
 * -----------------------------------------------------------------------------
 */

static inline void
__d3lut_bflt_ins(d3lut_bflt_t * d3lut_bflt,
                 uint16_t bitpos, uint32_t elem_index)
{
    D3LUT_PFUNC();
    D3LUT_ASSERT(d3lut_bflt != D3LUT_BFLT_NULL);
    D3LUT_ASSERT(elem_index < d3lut_bflt->elem_total);

    /* An element with hash value of 0x0000 implies it is NOT active */
    if (bitpos == D3LUT_BFLT_HASH_INVALID)  /* re-adjust for invalid bitpos */
        bitpos = ~D3LUT_BFLT_HASH_INVALID;

    d3lut_bflt->hash[elem_index] = bitpos;  /* save 16 bit hash for element */

    __d3lut_bflt_set(bitpos, d3lut_bflt->b64K); /* set bit position in bitmap */

}   /* __d3lut_bflt_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function: Delete a 16 bit hash from the bloom filter. If no other elements
 * have the same 16 bit hash key, then clear the corresponding bit in bitmap.
 * -----------------------------------------------------------------------------
 */

static inline void
__d3lut_bflt_del(d3lut_bflt_t * d3lut_bflt,
                 uint16_t bitpos, uint32_t elem_index)
{
    D3LUT_PFUNC();
    D3LUT_ASSERT(d3lut_bflt != D3LUT_BFLT_NULL);
    D3LUT_ASSERT(elem_index < d3lut_bflt->elem_total);

    /* An element with hash value of 0x0000 implies it is NOT active */
    if (bitpos == D3LUT_BFLT_HASH_INVALID)  /* re-adjust for invalid bitpos */
        bitpos = ~D3LUT_BFLT_HASH_INVALID;

    /* Mark element as not active */
    d3lut_bflt->hash[elem_index] = D3LUT_BFLT_HASH_INVALID;

    /* Determine whether any other element has a hash for bitpos */
    for (elem_index = 0; elem_index < d3lut_bflt->elem_total; ++elem_index)
    {
        if (d3lut_bflt->hash[elem_index] == bitpos)
        {
            return; /* found at least one element with a matching 16 bit hash */
        }
    }

    /* Did not find a single element with a matching 16 bit hash */
    __d3lut_bflt_clr(bitpos, d3lut_bflt->b64K); /* clear bit at bitpos */

}   /* __d3lut_bflt_del() */


/**
 * -----------------------------------------------------------------------------
 * Function: Test whether a 16 bit hash exists in the bloom filter.
 * Returns maybe or no, as per bloom filter axiom of "No False Negatives".
 * -----------------------------------------------------------------------------
 */

uint32_t
d3lut_bflt_exist(d3lut_bflt_t * d3lut_bflt, uint16_t hash)
{
    uint32_t exist;

    D3LUT_ASSERT(d3lut_bflt != D3LUT_BFLT_NULL);

    if (__d3lut_bflt_get(hash, d3lut_bflt->b64K))
        exist = D3LUT_BFLT_EXIST_MAYBE; /* possible false positives */
    else
        exist = D3LUT_BFLT_EXIST_NO;    /* no false negatives */

    return exist;

}   /* d3lut_bflt_exist() */


/**
 * -----------------------------------------------------------------------------
 * Function: Allocate and initialize a bloom filter subsystem in D3LUT object.
 * -----------------------------------------------------------------------------
 */

static d3lut_bflt_t *
d3lut_bflt_init(uint32_t d3lut_bflt_elem_total)
{
    uint32_t mem_bytes;
    d3lut_bflt_t * d3lut_bflt;

    D3LUT_FUNC();

    mem_bytes = (uint32_t) sizeof(d3lut_bflt_t);

    d3lut_bflt = (d3lut_bflt_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (d3lut_bflt == D3LUT_BFLT_NULL)
        goto d3lut_bflt_init_failure;

    memset(d3lut_bflt, 0x0, mem_bytes);
    d3lut_bflt->elem_total = d3lut_bflt_elem_total;

    D3LUT_TRACE("mem %p elem_total %u", d3lut_bflt, d3lut_bflt->elem_total);

    return d3lut_bflt;

d3lut_bflt_init_failure:

    D3LUT_ERROR("kmalloc %u failure", mem_bytes);
    return D3LUT_BFLT_NULL;

}   /* d3lut_bflt_init() */


/**
 * -----------------------------------------------------------------------------
 * Function: Deallocate all memory resources used by bloom filter.
 * -----------------------------------------------------------------------------
 */

static void
d3lut_bflt_fini(d3lut_bflt_t * d3lut_bflt)
{
    D3LUT_FUNC();

    if (d3lut_bflt == D3LUT_BFLT_NULL)
        return;

    D3LUT_TRACE("mem %p elem_total %u", d3lut_bflt, d3lut_bflt->elem_total);

    d3lut_bflt->elem_total = 0U;
    kfree(d3lut_bflt);

    return; /* avoid accidental access */

}   /* d3lut_bflt_fini() */

#endif /* CC_D3LUT_BFLT */


/**
 * =============================================================================
 * Section: D3LUT Element
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function: Allocate a free pool of d3lut elements. Element key is set at init
 * time. Key's incarnation is incremented on deallocation. see d3lut_elem_put().
 * Pool of elements are managed in a sll free list.
 * -----------------------------------------------------------------------------
 */

static d3lut_elem_t *
d3lut_pool_init(d3lut_pool_t * d3lut_pool, uint32_t d3lut_elem_tot)
{
    uint32_t index, pool, mem_bytes;
    d3lut_pool_t * d3lut_pool_base;
    d3lut_elem_t * d3lut_elem_base;
    d3lut_elem_t * d3lut_elem, * d3lut_elem_next;

    D3LUT_PFUNC();
    D3LUT_ASSERT(d3lut_elem_tot == (D3LUT_POOL_TOT * D3LUT_ELEM_MAX));

    /* Allocate pool of d3lut elements (includes a d3fwd_ext_t extension) */
    mem_bytes = D3LUT_ELEM_TOT * (uint32_t) sizeof(d3lut_elem_t);
    d3lut_elem_base = (d3lut_elem_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (d3lut_elem_base == D3LUT_ELEM_NULL)
        goto d3lut_pool_init_failure;
    D3LUT_TRACE("elem mem %p", d3lut_elem_base);

    /* Initialize with 0xFF instead of 0x0 */
    memset(d3lut_elem_base, 0xff, mem_bytes);

    d3lut_elem = d3lut_elem_base;
    d3lut_elem_next = D3LUT_ELEM_NULL;

    d3lut_pool_base = d3lut_pool;

    /* Setup d3lut::pool[0] */
    pool = 0U;
    d3lut_pool->elem_head = d3lut_elem;
    d3lut_pool->elem_free = D3LUT_ELEM_MAX;
    d3lut_pool->elem_max  = D3LUT_ELEM_MAX;
    d3lut_pool->elem_base = d3lut_elem;
    d3lut_pool->policy.pool_freelist = D3LUT_POLICY_POOL_FREELIST;

    for (index = 0U; index < D3LUT_ELEM_TOT; d3lut_elem = d3lut_elem_next)
    {
        D3LUT_ASSERT_SYM(d3lut_elem->sym.v8);

        d3lut_elem_next        = (d3lut_elem + 1);

        /* NOTE: index and domain are persistent until pool is destructed */
        /* Element Key's index, is a system wide index */
        d3lut_elem->key.index  = index;         /* index set at init time */
        d3lut_elem->key.incarn = 0x0;           /* incarn incremented per put */
        d3lut_elem->key.domain = pool;          /* pool aka domain */

	dll_init(&d3lut_elem->sta_list);        /* initialize sta list */

        D3FWD_EXT_PUT(d3lut_elem->ext);         /* clobber the elem extension */

        if (++index % D3LUT_ELEM_MAX)           /* pool not full */
        {
            d3lut_elem->next = d3lut_elem_next; /* continue current pool */
        }
        else                                    /* pool is full */
        {
            d3lut_elem->next = D3LUT_ELEM_NULL; /* terminate pool free list */
            d3lut_pool->elem_tail = d3lut_elem; /* tail is current elem */

            /* Free elements into next domain's pool */
            if (++pool < D3LUT_POOL_TOT)
            {
                ++d3lut_pool;                   /* setup next pool */
                d3lut_pool->elem_head = d3lut_elem_next;
                d3lut_pool->elem_free = D3LUT_ELEM_MAX;
                d3lut_pool->elem_max  = D3LUT_ELEM_MAX;
                d3lut_pool->elem_base = d3lut_elem_next;
                d3lut_pool->policy.pool_freelist = D3LUT_POLICY_POOL_FREELIST;
            }
            else
                break; /* all pool's elements are intialized */
        }
    }

    D3LUT_ASSERT(index == D3LUT_ELEM_TOT);

    D3LUT_TRACE("pool %u elem total %u size %u",
        pool, index, (uint32_t) sizeof(d3lut_elem_t));

    for (pool = 0U; pool < D3LUT_POOL_TOT; ++pool)
    {
        d3lut_pool = d3lut_pool_base + pool;

        D3LUT_TRACE("pool %u head %p tail %p free %u base %p %s",
            pool, d3lut_pool->elem_head, d3lut_pool->elem_tail,
            d3lut_pool->elem_free, d3lut_pool->elem_base,
            (d3lut_pool->policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST) ?
            "Freelist" : "ByIndex");
    }

    D3LUT_TRACE("Constructed Elem base %p mem_bytes %u pools %u elems max %u",
        d3lut_elem_base, mem_bytes, D3LUT_POOL_TOT, D3LUT_ELEM_MAX);

    return d3lut_elem_base;

d3lut_pool_init_failure:

    D3LUT_ERROR("kmalloc %u failure", mem_bytes);
    return d3lut_elem_base;

}   /* d3lut_pool_init() */


/**
 * -----------------------------------------------------------------------------
 * Function: Release all memory used by WAR element pool
 * -----------------------------------------------------------------------------
 */

static void
d3lut_pool_fini(d3lut_elem_t * d3lut_elem_base)
{
    D3LUT_FUNC();

    if (d3lut_elem_base == D3LUT_ELEM_NULL)
        return;

    D3LUT_TRACE("Destructing Elem base %p ...", d3lut_elem_base);
    memset(d3lut_elem_base, 0xff, D3LUT_ELEM_TOT * sizeof(d3lut_elem_t));
    kfree((void *) d3lut_elem_base);

}   /* d3lut_pool_fini() */


/**
 * -----------------------------------------------------------------------------
 * Function: Configure the pool allocation policy
 * -----------------------------------------------------------------------------
 */

void
d3lut_policy_set(d3lut_t * d3lut, uint32_t pool, uint32_t d3lut_policy)
{
    d3lut_pool_t * d3lut_pool;

    D3LUT_FUNC();
    D3LUT_ASSERT(d3lut != D3LUT_NULL);
    D3LUT_ASSERT(pool < D3LUT_POOL_TOT);

    d3lut_pool = &d3lut->pool[pool];
    d3lut_pool->policy.v32 = d3lut_policy;

}   /* d3lut_policy_set() */


/**
 * -----------------------------------------------------------------------------
 * Function: Allocate a D3LUT element from the pool using the configured policy.
 * If pool policy is by_index, then allocate d3lut_elem selected by index, else
 * use the free pool.
 * -----------------------------------------------------------------------------
 */

static inline d3lut_elem_t *
__d3lut_elem_get(d3lut_t * d3lut, uint32_t pool, d3lut_policy_t d3lut_policy)
{
    d3lut_elem_t * d3lut_elem;
    d3lut_pool_t * d3lut_pool;

    D3LUT_PFUNC();

    d3lut_pool = &d3lut->pool[pool];

    /* User explicitly selects a d3lut_elem from the pool by index */
    if (d3lut_policy.pool_freelist != D3LUT_POLICY_POOL_FREELIST)
    {
        D3LUT_ASSERT(d3lut_pool->policy.pool_by_index
                     == D3LUT_POLICY_POOL_BY_INDEX);
        D3LUT_ASSERT(d3lut_policy.pool_by_index < d3lut_pool->elem_max);

        d3lut_elem = /* Caller selects an element to allocate */
            D3LUT_TABLE_ELEM(d3lut_pool->elem_base, d3lut_policy.pool_by_index);

        d3lut_pool->elem_free--;
        d3lut_elem->next = D3LUT_ELEM_NULL;     /* prepare element */

        D3FWD_EXT_GET(d3lut_elem->ext);

        goto __d3lut_elem_get_done;
    }

    /* User does not specify an explicit element, hence use the freelist */
    D3LUT_ASSERT(d3lut_pool->policy.pool_freelist
                 == d3lut_policy.pool_freelist);

    /* Allocation policy is by freelist */
    bcm_prefetch(d3lut_pool->elem_head);

    if (d3lut_pool->elem_free > 0)
    {
        D3LUT_ASSERT(d3lut_pool->elem_head != D3LUT_ELEM_NULL);

        d3lut_elem = d3lut_pool->elem_head;     /* freelist head element */
        D3LUT_ASSERT(d3lut_elem->key.index ==   /* Do not change key.index */
            D3LUT_TABLE_IDX(d3lut->elem_base, d3lut_elem));

        d3lut_pool->elem_head = d3lut_elem->next;   /* pop head element */
        if (d3lut_pool->elem_head == D3LUT_ELEM_NULL)
        {
            D3LUT_ASSERT(d3lut_pool->elem_tail == d3lut_elem);
            D3LUT_ASSERT(d3lut_pool->elem_free == 1);
            d3lut_pool->elem_tail = D3LUT_ELEM_NULL;
        }

        d3lut_pool->elem_free--;
        d3lut_elem->next = D3LUT_ELEM_NULL;     /* prepare element */

        D3LUT_STATS_EXPR( ++d3lut->stats.elem_gets; )
        D3LUT_PTRACE("elem %p " D3LUT_KEY_FMT "free %u",
            d3lut_elem, D3LUT_KEY_VAL(d3lut_elem->key), d3lut_pool->elem_free);

        D3FWD_EXT_GET(d3lut_elem->ext);
    }
    else
    {
        D3LUT_ASSERT(d3lut_pool->elem_tail == D3LUT_ELEM_NULL);
        D3LUT_STATS_EXPR( ++d3lut->stats.elem_errs; )
        D3LUT_WARN("depleted pool warning");
        d3lut_elem = D3LUT_ELEM_NULL;
    }

__d3lut_elem_get_done:

    return d3lut_elem;

}   /* __d3lut_elem_get() */


d3lut_elem_t *  /* Allocate element from free pool */
d3lut_get(d3lut_t * d3lut, uint32_t pool, d3lut_policy_t d3lut_policy)
{
    d3lut_elem_t * d3lut_elem;

    D3LUT_ASSERT(d3lut != D3LUT_NULL);
    D3LUT_ASSERT(pool < D3LUT_POOL_TOT);
    D3LUT_ASSERT((d3lut_policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST) ||
                 (d3lut_policy.pool_by_index <  D3LUT_ELEM_MAX));

    D3LUT_LOCK(d3lut); // +++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (d3lut->elem_base == D3LUT_ELEM_NULL) {
        D3LUT_WARN("elem_base not initialized failure");
        d3lut_elem = D3LUT_ELEM_NULL;
    }
    else
    {
        d3lut_elem = __d3lut_elem_get(d3lut, pool, d3lut_policy);
    }

    D3LUT_UNLK(d3lut); // -----------------------------------------------------

    return d3lut_elem;

}   /* d3lut_get() */


/**
 * -----------------------------------------------------------------------------
 * Function: Deallocate a d3lut element by adding it to the freelist tail.
 * The element's key incarnation is incremented to avoid latent access. Symbol
 * is invalidated by setting all 6 Bytes to 0xFF.
 *
 * If elements were explicitly allocated, then free list is meaningless.
 * -----------------------------------------------------------------------------
 */

static inline void
__d3lut_elem_put(d3lut_t * d3lut, d3lut_elem_t * d3lut_elem)
{
    uint32_t pool;
    d3lut_pool_t * d3lut_pool;

    D3LUT_ASSERT(d3lut_elem != D3LUT_ELEM_NULL);

    D3LUT_PTRACE("elem %p " D3LUT_KEY_FMT,
        d3lut_elem, D3LUT_KEY_VAL(d3lut_elem->key));

    D3LUT_ASSERT(d3lut_elem->key.index ==
        D3LUT_TABLE_IDX(d3lut->elem_base, d3lut_elem));

    pool = D3LUT_POOL_IDX(d3lut_elem->key.index);
    D3LUT_ASSERT(pool == d3lut_elem->key.domain);

    d3lut_pool = &d3lut->pool[pool];
    bcm_prefetch(d3lut_pool->elem_tail);

    /* Invalidate element and place into free list */
    ++d3lut_elem->key.incarn;                       /* increment incarnation */
    /* index and domain are never modified */
    *((uint32_t *)(d3lut_elem->sym.v8 + 0)) = 0xFFFFFFFF;
    *((uint16_t *)(d3lut_elem->sym.v8 + 4)) = 0xFFFF;   /* reset symbol to ~0 */

    /* Invalidate the d3lut_elem's extension ... as not in use */
    D3FWD_EXT_PUT(d3lut_elem->ext);

    if (d3lut_pool->policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST)
    {
        /* Now place into free list at tail */
        if (d3lut_pool->elem_head != D3LUT_ELEM_NULL)
        {
            d3lut_pool->elem_tail->next = d3lut_elem;
            d3lut_pool->elem_tail       = d3lut_elem;
        }
        else
        {
            d3lut_pool->elem_head = d3lut_pool->elem_tail = d3lut_elem;
        }
        d3lut_elem->next = D3LUT_ELEM_NULL;
    }

    ++d3lut_pool->elem_free;

    D3LUT_STATS_EXPR( ++d3lut->stats.elem_puts; )

}   /* __d3lut_elem_put() */

void /* Free an element back to free pool: with lock */
d3lut_put(d3lut_t * d3lut, d3lut_elem_t * d3lut_elem)
{
    D3LUT_LOCK(d3lut); // +++++++++++++++++++++++++++++++++++++++++++++++++++++

    D3LUT_ASSERT(d3lut->elem_base != D3LUT_ELEM_NULL);
    __d3lut_elem_put(d3lut, d3lut_elem);

    D3LUT_UNLK(d3lut); // -----------------------------------------------------

}   /* d3lut_put() */


/**
 * =============================================================================
 * Section: D3LUT Dictionary
 *
 * Dictionary is implemented as a two stage lookup.
 * Stage 1. Use the last hit element for a match if the saved hash matches.
 * Stage 2. Use a hash with 4 bins per bucket.
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function: Allocate and initialize the last hit lookup and hash table.
 * -----------------------------------------------------------------------------
 */

static d3lut_dict_t *
_d3lut_dict_init(void)
{
    uint32_t slot, bkt, bin, mem_bytes;
    d3lut_dict_t * d3lut_dict;

    D3LUT_FUNC();

    mem_bytes = (uint32_t) sizeof(d3lut_dict_t);

    d3lut_dict = (d3lut_dict_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (d3lut_dict == D3LUT_DICT_NULL)
        goto d3lut_dict_init_failure;

    memset(d3lut_dict, 0, mem_bytes);    /* below is redundant */

    for (slot = 0; slot < D3LUT_LAST_SLOTS_MAX; ++slot)
        d3lut_dict->last[slot].v32 = D3LUT_LAST_INVALID;

    for (bkt = 0; bkt < D3LUT_DICT_BKTS_MAX; ++bkt)
        for (bin = 0; bin < D3LUT_DICT_BINS_MAX; ++bin)
            d3lut_dict->htable.bkts[bkt].bins[bin] = D3LUT_ELEM_NULL;

    d3lut_dict->lookups = 0U;
    d3lut_dict->entries = 0U;

    D3LUT_STATS_EXPR(
        d3lut_dict->stats.last = 0U; d3lut_dict->stats.hits = 0U;
        d3lut_dict->stats.inss = 0U; d3lut_dict->stats.dels = 0U;
        d3lut_dict->stats.dups = 0U; d3lut_dict->stats.errs = 0U;
    )

    D3LUT_TRACE("Constructed Dict mem %p last %u bkts %u bins %u", d3lut_dict,
        D3LUT_LAST_SLOTS_MAX, D3LUT_DICT_BKTS_MAX, D3LUT_DICT_BINS_MAX);

    return d3lut_dict;

d3lut_dict_init_failure:

    D3LUT_ERROR("kmalloc %u failure", mem_bytes);
    return d3lut_dict;

}   /* _d3lut_dict_init() */


/**
 * -----------------------------------------------------------------------------
 * Function: Deallocate all memory associated with dictionary.
 * -----------------------------------------------------------------------------
 */

static void
_d3lut_dict_fini(d3lut_dict_t * d3lut_dict)
{
    D3LUT_FUNC();

    if (d3lut_dict != D3LUT_DICT_NULL)
    {
        D3LUT_TRACE("Destructing Dict mem %p ...", d3lut_dict);

        memset(d3lut_dict, 0xff, sizeof(d3lut_dict_t));
        d3lut_dict->entries = 0U;
        kfree((void *) d3lut_dict);
    }

}   /* _d3lut_dict_fini() */


/**
 * =============================================================================
 * Section: D3LUT system
 * =============================================================================
 */

/** If multiple instance of D3LUT are needed, remove use of global d3lut_gp */
d3lut_t * d3lut_gp = D3LUT_NULL;

/**
 * -----------------------------------------------------------------------------
 * Function: Construct D3LUT subsystems: element pool, bloomfilter, dictionary.
 * -----------------------------------------------------------------------------
 */


d3lut_t * /* Initialize all d3lut subsystems */
d3lut_init(uint32_t d3lut_elem_tot)
{
    uint32_t mem_bytes;
    d3lut_t * d3lut;

    D3LUT_FUNC();
    D3LUT_ASSERT(d3lut_elem_tot == D3LUT_ELEM_TOT);

    if (d3lut_gp != D3LUT_NULL) {
        D3LUT_ERROR("already initialized");
        d3lut = d3lut_gp;
        goto d3lut_init_success;
    }

    mem_bytes = (uint32_t) sizeof(d3lut_t);
    d3lut = (d3lut_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (d3lut == D3LUT_NULL) {
        D3LUT_ERROR("kmalloc %u failure", mem_bytes);
        goto d3lut_init_failure;
    }

    memset(d3lut, 0, sizeof(d3lut_t));
    spin_lock_init(&d3lut->lock);       /* initialize lock */

    /* Setup preallocated pool of elements */
    d3lut->elem_base = d3lut_pool_init(d3lut->pool, d3lut_elem_tot);
    if (d3lut->elem_base == D3LUT_ELEM_NULL) {
        goto d3lut_init_failure;
    }

    D3LUT_BFLT_EXPR(
    {   /* Setup bloom filter */
        d3lut->bflt = d3lut_bflt_init(d3lut_elem_tot);
        if (d3lut->bflt == D3LUT_BFLT_NULL)
            goto d3lut_init_failure;
    })

    /* Setup dictionary: Buckets and Bins are compile constants */
    d3lut->dict = _d3lut_dict_init();
    if (d3lut->dict == D3LUT_DICT_NULL) {
        goto d3lut_init_failure;
    }

    d3lut_gp = d3lut; /* save in global - used in singleton logic and debug */

d3lut_init_success:

    D3LUT_TRACE(PKTFWD_VRP_FMT " Constructed LUT %p",
           PKTFWD_VRP_VAL(D3LUT, D3LUT_VERSIONCODE), d3lut_gp);

    return d3lut;

d3lut_init_failure:

    d3lut_gp = d3lut;
    d3lut_fini(d3lut);

    return D3LUT_NULL;

}   /* d3lut_init() */


/**
 * -----------------------------------------------------------------------------
 * Function: Destruct D3LUT subsystems: element pool, bloomfilter, dictionary.
 * -----------------------------------------------------------------------------
 */
void
d3lut_fini(d3lut_t * d3lut)
{
    D3LUT_FUNC();

    if (d3lut == D3LUT_NULL)
        return;

    D3LUT_TRACE("Desctructing LUT %p ...", d3lut);

    D3LUT_ASSERT(d3lut_gp == d3lut);
    _d3lut_dict_fini(d3lut->dict);
    d3lut->dict = D3LUT_DICT_NULL; /* accidental access */

    D3LUT_BFLT_EXPR(
    {
        d3lut_bflt_fini(d3lut->bflt);
        d3lut->bflt = D3LUT_BFLT_NULL;
    })

    d3lut_pool_fini(d3lut->elem_base);
    d3lut->elem_base = D3LUT_ELEM_NULL; /* accidental access */

    memset(d3lut, 0xff, sizeof(d3lut_t)); /* scribble */
    kfree(d3lut);

    d3lut_gp = D3LUT_NULL;

}   /* d3lut_fini() */


/**
 * -----------------------------------------------------------------------------
 * Function: D3LUT subsystems debug dump
 * CAUTION : No lock is taken
 * -----------------------------------------------------------------------------
 */

void
d3fwd_sta_dump(dll_t *sta_list)
{
    int i = 0;
    dll_t *item;
    d3lut_sta_t *sta;

    if (dll_empty(sta_list))
	return;

    printk("\t STAs: [addr] \t\t\t[d3lut_elem] \t\t[flag]\n");
    dll_for_each(item, sta_list)
    {
	sta = (d3lut_sta_t *)item;
	printk("\t\t[%02x:%02x:%02x:%02x:%02x:%02x] \t[%px] \t[0x%x]\n",
		sta->mac[0], sta->mac[1], sta->mac[2], sta->mac[3], sta->mac[4], sta->mac[5],
		sta->d3lut_elem, sta->flag);
	if (i++ > PKTFWD_ENDPOINTS_MAX) {
	    printk("%s: ERROR sta num is greater than %d!\n", __FUNCTION__, PKTFWD_ENDPOINTS_MAX);
	    break;
	}
    }
}

void
d3fwd_elem_dump(d3lut_elem_t * d3lut_elem)
{
    printk("\t Elem %p pool %u " D3LUT_ELEM_FMT "\n",
        d3lut_elem, D3LUT_POOL_IDX(d3lut_elem->key.index),
        D3LUT_ELEM_VAL(d3lut_elem));
#if defined(BCM_D3FWD)
    d3fwd_ext_dump(&d3lut_elem->ext);
#endif
    d3fwd_sta_dump(&d3lut_elem->sta_list);
}   /* d3fwd_elem_dump() */


void
d3lut_dump(d3lut_t * d3lut)
{
    uint32_t pool, elem_total, elem_free;
    d3lut_dict_t * d3lut_dict;
    d3lut_pool_t * d3lut_pool;
    bool dump_verbose = false;

#if (CC_D3LUT_DEBUG >= 1)
    dump_verbose = true;
#endif

    if (d3lut == D3LUT_NULL) {
        printk("D3LUT is not yet initialized\n");
        return;
    }

    D3LUT_ASSERT(d3lut_gp == d3lut);
    D3LUT_ASSERT(d3lut->dict != D3LUT_DICT_NULL);
    D3LUT_ASSERT(d3lut->elem_base != D3LUT_ELEM_NULL);

    elem_total = 0U;
    elem_free  = 0U;

    printk("D3LUT DUMP\n\t STATE:: elem: pool %u elem %u\n",
        D3LUT_POOL_TOT, D3LUT_ELEM_MAX);

    if (d3lut->elem_base == D3LUT_ELEM_NULL)
        return;

    for (pool = 0U; pool < D3LUT_POOL_TOT; ++pool)
    {
        d3lut_pool = d3lut->pool + pool;
        printk("\t\t Pool max %u free %u policy %s\n",
            d3lut_pool->elem_max, d3lut_pool->elem_free,
            (d3lut_pool->policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST) ?
            "Freelist" : "By_Index");
        elem_total += d3lut_pool->elem_max;
        elem_free  += d3lut_pool->elem_free;
    }
    printk("\t\t Elem total %u free %u\n", elem_total, elem_free);

    d3lut_dict = d3lut->dict;
    if (d3lut_dict == D3LUT_DICT_NULL)
        return;

    printk("\t\t Dict lookups %u entries %u failures %u\n",
        d3lut_dict->lookups, d3lut_dict->entries, d3lut_dict->failures);
    D3LUT_STATS_EXPR(
        printk("\t\t\t last %u hits %u miss %u coll %u "
            "adds %u dels %u dups %u errs %u\n",
            d3lut_dict->stats.last, d3lut_dict->stats.hits,
            d3lut_dict->stats.miss, d3lut_dict->stats.coll,
            d3lut_dict->stats.inss, d3lut_dict->stats.dels,
            d3lut_dict->stats.dups, d3lut_dict->stats.errs);
    )

    if (dump_verbose == true)
    {
        uint32_t bkt, bin;
        d3lut_elem_t * d3lut_elem;
        for (bkt = 0U; bkt < D3LUT_DICT_BKTS_MAX; ++bkt)
        {
            for (bin = 0U; bin < D3LUT_DICT_BINS_MAX; ++bin)
            {
                d3lut_elem = d3lut_dict->htable.bkts[bkt].bins[bin];
                if (d3lut_elem != D3LUT_ELEM_NULL)
                    d3fwd_elem_dump(d3lut_elem);
            }
        }
    }   /* dump_verbose */

    D3LUT_STATS_EXPR(
        printk("\t STATS:: elem: gets %u puts %u errs %u\n",
            d3lut->stats.elem_gets, d3lut->stats.elem_puts,
            d3lut->stats.elem_errs);
    )

}   /* d3lut_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function: Conversion functions
 * -----------------------------------------------------------------------------
 */

d3lut_elem_t * /* Convert key to element */
d3lut_k2e(d3lut_t * d3lut, d3lut_key_t key)
{
	d3lut_elem_t * d3lut_elem;
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));
    if (d3lut == D3LUT_NULL) {
        return D3LUT_ELEM_NULL;
    }

    if (key.v16 == D3LUT_KEY_INVALID) {
        D3LUT_WARN("invalid key");
        return D3LUT_ELEM_NULL;
    }

    D3LUT_ASSERT(key.index < D3LUT_ELEM_TOT);

    d3lut_elem = D3LUT_TABLE_ELEM(d3lut->elem_base, key.index);

	/* compare the retrieved key with the supplied key to ensure the provided
	 * entry is not stale
	 */
	if (d3lut_elem->key.v16 == key.v16) {
		return d3lut_elem;
	} else {
        D3LUT_WARN("Stale d3lut_elem key in packet");
		return D3LUT_ELEM_NULL;
	}
}   /* d3lut_k2e() */


d3lut_key_t /* Convert element to key */
d3lut_e2k(d3lut_t * d3lut, d3lut_elem_t * elem)
{
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));

    if (d3lut == D3LUT_NULL) {
        d3lut_key_t d3lut_key_invalid;
        d3lut_key_invalid.v16 = D3LUT_KEY_INVALID;
        return d3lut_key_invalid;
    }

    D3LUT_ASSERT(elem != D3LUT_ELEM_NULL);

    return elem->key;

}   /* d3lut_e2k() */


/**
 * =============================================================================
 * D3LUT dictionary operations
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * Function: Insert an element in the hash table, given a symbol. Update bloom
 * filter. Check for duplicates.
 * Returns the inserted symbol's key, or D3LUT_KEY_INVALID on failure.
 * -----------------------------------------------------------------------------
 */

d3lut_elem_t *
d3lut_ins(d3lut_t * d3lut, uint8_t * sym, uint32_t pool,
          d3lut_policy_t d3lut_policy)
{
    uint16_t hash;
    uint32_t bin;
    d3lut_bins_t * d3lut_bkt;
    d3lut_elem_t * d3lut_elem;
    d3lut_dict_t * d3lut_dict;

    D3LUT_ASSERT_SYM(sym);
    D3LUT_PTRACE(D3LUT_SYM_FMT "pool %u policy %u",
        D3LUT_SYM_VAL(sym), pool, d3lut_policy.v32);
    D3LUT_ASSERT(pool < D3LUT_POOL_TOT);
    D3LUT_ASSERT((d3lut_policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST) ||
                 (d3lut_policy.pool_by_index <  D3LUT_ELEM_MAX));
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));

    hash = __d3lut_hash(sym);                   /* compute lkup hash */
    d3lut_dict = d3lut->dict;                   /* locate bucket of bins */
    d3lut_bkt = d3lut_dict->htable.bkts + D3LUT_DICT_BKT_IDX(hash);

    for (bin = 0U; bin < D3LUT_DICT_BINS_MAX; ++bin) /* loop over bins */
    {
        d3lut_elem = d3lut_bkt->bins[bin];      /* fetch bin */
        if (d3lut_elem == D3LUT_ELEM_NULL)      /* empty slot */
        {
            d3lut_elem = __d3lut_elem_get(d3lut, pool, d3lut_policy);

            if (d3lut_elem != D3LUT_ELEM_NULL)  /* insert new element */
            {
                __d3lut_set(d3lut_elem->sym.v16, (uint16_t *)sym); /* set sym */
                d3lut_bkt->bins[bin] = d3lut_elem; /* place into htable */
                                                /* no change to last hit */

                /* element not cached into last hit table. caching on lkup */

                D3LUT_BFLT_EXPR(
                    __d3lut_bflt_ins(d3lut->bflt, hash, d3lut_elem->key.index);)
                D3LUT_STATS_EXPR( ++d3lut_dict->stats.inss; )
                ++d3lut_dict->entries;          /* update statistics */
                goto d3lut_ins_success;         /* BREAK */
            }
            else                                /* pool depleted */
            {
                D3LUT_WARN("depleted element pool warning");
                goto d3lut_ins_failure;          /* BREAK */
            }
        }   /* else check whether it is a duplicate */
        else if (__d3lut_cmp(d3lut_elem->sym.v16, (uint16_t *)sym) == 0)
        {
            /* permit duplicates: no change to incarnation */
            goto d3lut_ins_found_duplicate;
        }

        D3LUT_STATS_EXPR( ++d3lut_dict->stats.coll; )

    }   /* loop over bins */

    /* no empty bins for insert */
    D3LUT_WARN(D3LUT_SYM_FMT "no empty bins warning", D3LUT_SYM_VAL(sym));
    d3lut_elem = D3LUT_ELEM_NULL; /* set null element to return */

d3lut_ins_failure:

    ++d3lut_dict->failures;
    D3LUT_STATS_EXPR( ++d3lut_dict->stats.errs; )

    return d3lut_elem;

d3lut_ins_found_duplicate:

    ++d3lut_dict->failures;

    D3LUT_STATS_EXPR( ++d3lut_dict->stats.dups; )
    D3LUT_PTRACE("found duplicate " D3LUT_ELEM_FMT "warning",
               D3LUT_ELEM_VAL(d3lut_elem));

    /* permit duplicates, return duplicate */

d3lut_ins_success:
    return d3lut_elem;

}   /* d3lut_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function: Delete an element from the dictionary with a matching symbol.
 * Recompute bloom filter after hash bitposition has been deleted.
 * Delete all entries in last hit table with matching hash.
 * Returns key of deleted element or D3LUT_KEY_INVALID (if not found).
 * -----------------------------------------------------------------------------
 */

d3lut_elem_t *
d3lut_del(d3lut_t * d3lut, uint8_t * sym, uint32_t pool)
{
    uint8_t  last_sym_byte;
    uint16_t hash;
    uint32_t bin, bin_next;
    d3lut_bins_t * d3lut_bkt;
    d3lut_elem_t * d3lut_elem;
    d3lut_dict_t * d3lut_dict;

    D3LUT_ASSERT_SYM(sym);
    D3LUT_PTRACE(D3LUT_SYM_FMT, D3LUT_SYM_VAL(sym));
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));
    D3LUT_ASSERT(pool != D3LUT_LKUP_GLOBAL_POOL);

    d3lut_dict = d3lut->dict;

    /* Invalidate last hit cached element */
    last_sym_byte = *(sym + D3LUT_LAST_SYM_BYTE_IX);
    d3lut_dict->last[last_sym_byte].v32 = D3LUT_LAST_INVALID;

    hash = __d3lut_hash(sym);                   /* compute lkup hash */
    d3lut_bkt = d3lut_dict->htable.bkts + D3LUT_DICT_BKT_IDX(hash);

    bcm_prefetch(d3lut_bkt->bins[0]);

    for (bin = 0U; bin < D3LUT_DICT_BINS_MAX; ++bin) /* loop over bins */
    {
        bcm_prefetch(d3lut_bkt->bins[bin + 1]);

        d3lut_elem = d3lut_bkt->bins[bin];
        if (d3lut_elem == D3LUT_ELEM_NULL)
            break; /* no holes permitted in bin array */

        if ((__d3lut_cmp(d3lut_elem->sym.v16, (uint16_t *)sym) == 0) &&
            (pool == d3lut_elem->key.domain))
        {
            D3LUT_ASSERT(d3lut_elem->key.v16 != D3LUT_KEY_INVALID);

            D3LUT_BFLT_EXPR(
                __d3lut_bflt_del(d3lut->bflt, hash, d3lut_elem->key.index); )
            D3LUT_STATS_EXPR( ++d3lut_dict->stats.dels; )
            --d3lut_dict->entries;

            __d3lut_elem_put(d3lut, d3lut_elem);

            bin_next = bin + 1;                     /* fill holes */
            while (bin_next < D3LUT_DICT_BINS_MAX)   /* pack bins */
                d3lut_bkt->bins[bin++] = d3lut_bkt->bins[bin_next++];
            d3lut_bkt->bins[bin] = D3LUT_ELEM_NULL;

            goto d3lut_dict_del_success;
        }

        D3LUT_STATS_EXPR( ++d3lut_dict->stats.coll; )

    }   /* loop over bins */

    d3lut_elem = D3LUT_ELEM_NULL;

    ++d3lut_dict->failures;
    D3LUT_STATS_EXPR( ++d3lut_dict->stats.errs; )
    D3LUT_WARN(D3LUT_SYM_FMT "not found warning", D3LUT_SYM_VAL(sym));

d3lut_dict_del_success:

    return d3lut_elem;

}   /* d3lut_del() */


/**
 * -----------------------------------------------------------------------------
 * Function: Find a symbol in the dictionary, using a 2 stage lookup, first
 * using the last match table, and then using the bloom filter.
 * Returns the found symbol elem or D3LUT_ELEM_NULL, if symbol does not exist.
 * -----------------------------------------------------------------------------
 */

d3lut_elem_t * /* Find a symbol in the dictionary */
d3lut_lkup(d3lut_t *d3lut, uint8_t *sym, uint32_t pool)
{
    uint8_t last_sym_byte;
    uint16_t hash;
    uint32_t bin;
    d3lut_bins_t * d3lut_bkt;
    d3lut_elem_t * d3lut_elem;
    d3lut_dict_t * d3lut_dict;

    D3LUT_ASSERT(d3lut != D3LUT_NULL);
    D3LUT_PTRACE(D3LUT_SYM_FMT, D3LUT_SYM_VAL(sym));
    D3LUT_ASSERT_SYM(sym);

    d3lut_dict = d3lut->dict;

    ++d3lut_dict->lookups;

    /* Peek into last hit cached element */
    last_sym_byte = *(sym + D3LUT_LAST_SYM_BYTE_IX);

    bcm_prefetch(&d3lut_dict->last[last_sym_byte]);
    hash = __d3lut_hash(sym);

    /* last hit element matches hash */
    if (hash == d3lut_dict->last[last_sym_byte].hash)
    {
        /* Get the saved last hit element for an exact symbol match test */
        d3lut_elem = D3LUT_TABLE_ELEM(d3lut->elem_base,
                                      d3lut_dict->last[last_sym_byte].elem);
        if ((__d3lut_cmp(d3lut_elem->sym.v16, (uint16_t *)sym) == 0) &&
            ((pool == D3LUT_LKUP_GLOBAL_POOL) ||
             (pool == d3lut_elem->key.domain)))
        {
            D3LUT_STATS_EXPR( ++d3lut_dict->stats.last; )
            D3LUT_ASSERT(d3lut_elem->key.v16 != D3LUT_KEY_INVALID);

            goto d3lut_lkup_found_symbol;
        }
    }

    /* DO WE NEED BLOOM FILTER LOOKUP? ... not for pktfwd ... yet. */

    /* Search the dictionary */
    d3lut_bkt = d3lut_dict->htable.bkts + D3LUT_DICT_BKT_IDX(hash);
    bcm_prefetch(d3lut_bkt->bins[0]);

    for (bin = 0U; bin < D3LUT_DICT_BINS_MAX; ++bin) /* loop over bins */
    {
        bcm_prefetch(d3lut_bkt->bins[bin + 1]);

        d3lut_elem = d3lut_bkt->bins[bin];
        if (d3lut_elem == D3LUT_ELEM_NULL) /* no holes */
            break;

        if ((__d3lut_cmp(d3lut_elem->sym.v16, (uint16_t *)sym) == 0) &&
            ((pool == D3LUT_LKUP_GLOBAL_POOL) ||
             (pool == d3lut_elem->key.domain)))
        {
            /* Cache last hit into slot identified by the symbol (6'th byte) */
            d3lut_dict->last[last_sym_byte].hash = hash;
            d3lut_dict->last[last_sym_byte].elem =
                D3LUT_TABLE_IDX(d3lut->elem_base, d3lut_elem);

            D3LUT_STATS_EXPR( ++d3lut_dict->stats.hits; )
            D3LUT_ASSERT(d3lut_elem->key.v16 != D3LUT_KEY_INVALID);

            goto d3lut_lkup_found_symbol;
        }

        D3LUT_STATS_EXPR( ++d3lut_dict->stats.coll; )

    } /* loop over bins */

    ++d3lut_dict->failures;

    D3LUT_STATS_EXPR( ++d3lut_dict->stats.miss; )
    D3LUT_PTRACE(D3LUT_SYM_FMT "not found warning", D3LUT_SYM_VAL(sym));

    return D3LUT_ELEM_NULL;

d3lut_lkup_found_symbol:

    D3LUT_PTRACE(D3LUT_KEY_FMT, D3LUT_KEY_VAL(d3lut_elem->key));

    return d3lut_elem;

}   /* d3lut_lkup() */


/**
 * -----------------------------------------------------------------------------
 * Function: Locate all entires in the Dictionary that has a matching extension.
 * Caution: Caller will be holding lock while traversing the table.
 * -----------------------------------------------------------------------------
 */

void /* Clear all elements in dictionary with a matching d3fwd_wlif extension */
d3lut_clr(d3lut_t *d3lut, void * ext, bool ignore_ext_match)
{
    uint16_t hash;
    uint32_t bin, bkt;
    d3lut_bins_t * d3lut_bkt;
    d3lut_elem_t * d3lut_elem;
    d3lut_dict_t * d3lut_dict;

    D3LUT_FUNC();
    D3LUT_ASSERT(ext != (void *) NULL);
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));

    d3lut_dict = d3lut->dict;                   /* locate bucket of bins */
    d3lut_bkt  = d3lut_dict->htable.bkts;

    /* Walk entire hash table, deleting symbols in dictionary as requested */
    for (bkt = 0U; bkt < D3LUT_DICT_BKTS_MAX; ++bkt, ++d3lut_bkt)
    {
        bool need_packing = false;

        /* Traverse all bins and clear bins with matching element or all bins */
        for (bin = 0U; bin < D3LUT_DICT_BINS_MAX; ++bin)
        {
            d3lut_elem = d3lut_bkt->bins[bin];

            if (d3lut_elem == D3LUT_ELEM_NULL)
                break; /* no holes permitted in bin array, so no more search */

            /*
             * Delete all symbols if entire D3LUT must be cleared, or
             * Delete symbols that belong to the matching extension (interface)
             */
            if ((ignore_ext_match == true)
                || D3FWD_EXT_CMP(d3lut_elem->ext, ext))
            {
                uint8_t last_sym_byte;

                D3LUT_ASSERT(d3lut_elem->key.v16 != D3LUT_KEY_INVALID);
                need_packing = true; /* need packing, unless ignore_ext_match */

                /* Clear last hit, using the byte in the symbol */
                last_sym_byte = d3lut_elem->sym.v8[D3LUT_LAST_SYM_BYTE_IX];
                d3lut_dict->last[last_sym_byte].v32 = D3LUT_LAST_INVALID;

                /* Delete elem from bloom filter */
                hash = __d3lut_hash(d3lut_elem->sym.v8);
                D3LUT_BFLT_EXPR(
                    __d3lut_bflt_del(d3lut->bflt, hash, d3lut_elem->key.index);)

                D3LUT_STATS_EXPR( ++d3lut_dict->stats.dels; )

                --d3lut_dict->entries;
                __d3lut_elem_put(d3lut, d3lut_elem);

                d3lut_bkt->bins[bin] = D3LUT_ELEM_NULL;

            } /* if ignore_ext_match or ext matches, delete symbol */

        } /* traverse all bins in a bucket */

        /* Check whether packing is needed. No packing if ignore_ext_match */
        if ((ignore_ext_match) || (need_packing == false))
            continue; /* no packing required */

        /* Pack the bins, as no holes are allowed */
        for (bin = 0; bin < D3LUT_DICT_BINS_MAX; ++bin)
        {
            uint32_t ne_bin; /* iterator to find next non-empty bin */

            d3lut_elem = d3lut_bkt->bins[bin];
            if (d3lut_elem != D3LUT_ELEM_NULL)
                continue; /* not a hole, so no packing for this bin */

            /* Is a hole, find next non-empty bin, if any */
            for (ne_bin = bin + 1; ne_bin < D3LUT_DICT_BINS_MAX; ++ne_bin)
            {
                if (d3lut_bkt->bins[ne_bin] != D3LUT_ELEM_NULL)
                    break;
            }

            /* Swap if a non-empty bin, is found */
            if (ne_bin >= D3LUT_DICT_BINS_MAX)
                break;
            d3lut_bkt->bins[bin] = d3lut_bkt->bins[ne_bin];
            d3lut_bkt->bins[ne_bin] = D3LUT_ELEM_NULL;

        } /* pack bins */

    } /* bkts */

}   /* d3lut_clr() */


/**
 * -----------------------------------------------------------------------------
 * Function: Clear all d3lut statistics (including dictionary)
 * Caution: Caller will be holding lock while traversing the table.
 * -----------------------------------------------------------------------------
 */
extern void
d3lut_stats_clr(d3lut_t * d3lut)
{
    D3LUT_ASSERT((d3lut != D3LUT_NULL) && (d3lut_gp == d3lut));

#if defined(CC_D3LUT_STATS)
    memset(&d3lut->stats, 0, sizeof(d3lut->stats));
    if (d3lut->dict != D3LUT_DICT_NULL)
        memset(&d3lut->dict->stats, 0, sizeof(d3lut->dict->stats));
#endif /* CC_D3LUT_STATS */

}   /* d3lut_stats_clr() */

EXPORT_SYMBOL(d3lut_gp);
EXPORT_SYMBOL(d3lut_hash);
EXPORT_SYMBOL(d3lut_policy_set);
EXPORT_SYMBOL(d3lut_put);
EXPORT_SYMBOL(d3lut_dump);
EXPORT_SYMBOL(d3lut_k2e);
EXPORT_SYMBOL(d3lut_e2k);
EXPORT_SYMBOL(d3lut_ins);
EXPORT_SYMBOL(d3lut_del);
EXPORT_SYMBOL(d3lut_lkup);
EXPORT_SYMBOL(d3lut_clr);
EXPORT_SYMBOL(d3lut_stats_clr);

#endif /* BCM_D3LUT */


#endif /* BCM_PKTFWD */
