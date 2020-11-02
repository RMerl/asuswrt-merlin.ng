/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#if defined(BCM_NBUFF)

#include <dngl_stats.h>
#include <dhd.h>
#include <linux_osl.h>
#include <linux/mm.h>
#if defined(BCM_NBUFF_PKT_BPM)
#include <bcm_prefetch.h>
#include <linux/gbpm.h>
#endif /* BCM_NBUFF_PKT_BPM */

#include <dhd_dbg.h>

/* PKTTAG POOL */

#define DHD_PKTTAGS_POOL_SIZE	(1280)
#define DHD_PKTTAG_POOL_MAX_SIZE (2560)
#define  DHD_PKTTAG_ENTRY_SIZE	((uint)sizeof(dhd_pkttag_fd_t))
uint  g_dhd_pkttags_pool_size = (FKBC_POOL_SIZE_ENGG+FKBC_EXTEND_SIZE_ENGG*FKBC_EXTEND_MAX_ENGG);

/* for fkb/multicast pool use */
spinlock_t g_dhd_pkttags_pool_lock;
DEFINE_SPINLOCK(g_dhd_pkttags_pool_lock);
static uint g_dhd_pkttags_pool_free = 0;
static dhd_pkttag_fd_t *g_dhd_pkttags_pool_free_p = NULL;
static dhd_pkttag_fd_t *g_dhd_pkttags_pool_p = NULL;
uint g_dhd_fkbpool_size = DHD_FKBPOOL_DEFAULT_SIZE;

DEFINE_SPINLOCK(g_dhd_fkb_pool_lock);
uint g_dhd_fkb_pool_free = 0;
#ifdef WLCSM_DEBUG
uint g_dhd_fkb_pool_max_usage = 0;
#endif
FkBuff_t *g_dhd_fkb_pool_free_p = NULL;
int g_dhd_pool_available=0;
extern char *wlcsm_nvram_k_get(char* name);

#if defined(BCM_NBUFF_PKT_BPM)
#define NBUFF_DATABUFF_CACHE(osh) \
	(((osl_pubinfo_nbuff_t *)(osh))->databuf_cache)
#define NBUFF_DATABUFF_CACHE_IDX(osh) \
	(((osl_pubinfo_nbuff_t *)(osh))->databuf_cache_idx)
#endif /* BCM_NBUFF_PKT_BPM */

#define OSH_NBUFF_COUNTERS(osh) \
   	(((osl_pubinfo_nbuff_t *)(osh))->nbuff_counters)
#define NBUFF_PKTTAG_NOFBUF(osh) \
		(OSH_NBUFF_COUNTERS(osh).tx_tag_nobuf)
#define NBUFF_FKBPOOL_NOFBUF(osh) \
		(OSH_NBUFF_COUNTERS(osh).tx_fkb_pool_nobuf)
#define NBUFF_DUP_NOBUF(osh) \
		(OSH_NBUFF_COUNTERS(osh).tx_dup_nobuf)

void inline dhd_fkb_clear_tag(FkBuff_t *fkb)
{
    void *tag=PFKBUFF_TO_PHEAD(fkb);
    memset(tag,0,sizeof(dhd_pkttag_fd_t));
}

void
dhd_pkttags_pool_put(void *tag)
{
    dhd_pkttag_fd_t *tag_p = (dhd_pkttag_fd_t *)tag;
    spin_lock_bh(&g_dhd_pkttags_pool_lock);
    tag_p->list = g_dhd_pkttags_pool_free_p;
    g_dhd_pkttags_pool_free_p = tag_p;
    g_dhd_pkttags_pool_free++;
#ifdef WLCSM_DEBUG
	wlcsm_dbg_inc(9,1);
#endif
    spin_unlock_bh(&g_dhd_pkttags_pool_lock);
}

void *dhd_pkttags_pool_get(void)
{
    dhd_pkttag_fd_t *ret_p = NULL;
    spin_lock_bh(&g_dhd_pkttags_pool_lock);
    if ((g_dhd_pkttags_pool_free_p != NULL) && g_dhd_pool_available)
    {
        ret_p = g_dhd_pkttags_pool_free_p;
        g_dhd_pkttags_pool_free_p = g_dhd_pkttags_pool_free_p->list;
#ifdef WLCSM_DEBUG
	    wlcsm_dbg_inc(8,1);
#endif
        g_dhd_pkttags_pool_free--;
        ret_p->list = NULL;
    } else {
        printk("NO PKTTAG AVAILABLE,increase nvram entry dhd_tagpool_size's value\n");
    }
    spin_unlock_bh(&g_dhd_pkttags_pool_lock);
    return ret_p;
}

int dhd_pkttag_pool_init(void)
{
    long i = 0 ;
    char *poolsize=wlcsm_nvram_k_get("dhd_tagpool_size");
    if(poolsize && (kstrtol(poolsize,0,&i)==0) && (i <= DHD_PKTTAG_POOL_MAX_SIZE))
        g_dhd_pkttags_pool_size = (uint)i;
    printk("DHD_PKTTAG POOL size is:%u and entry size:%u\n",g_dhd_pkttags_pool_size ,DHD_PKTTAG_ENTRY_SIZE);
    if(g_dhd_pkttags_pool_size ==0) return 0;


    g_dhd_pkttags_pool_p = kmalloc(g_dhd_pkttags_pool_size * DHD_PKTTAG_ENTRY_SIZE, GFP_ATOMIC);
    if(!g_dhd_pkttags_pool_p) {
        printk("..Could not allocate mem for PKTTAG pool\r\n");
        return 1;
    }
    for (i = 0; i < g_dhd_pkttags_pool_size; ++i) {
        dhd_pkttags_pool_put(&(g_dhd_pkttags_pool_p[i]));
    }
    return 0;
}

void dhd_pkttag_pool_free( void )
{
    if (g_dhd_pkttags_pool_free == -1)
        return; /* has been free'd */

    /* wait until all tags are freed*/
    while(g_dhd_pkttags_pool_free != g_dhd_pkttags_pool_size)
        yield();
    /* no lock needed as the pool is locked for now,nobody will be
     * able to access this pool 
     */
    if(g_dhd_pkttags_pool_p) {
        kfree(g_dhd_pkttags_pool_p);
        g_dhd_pkttags_pool_p = NULL;
        g_dhd_pkttags_pool_free = -1;
    }
}
/* ENDOF PKTTAG POOL */

/* GENERIC NBUFF */
INLINE BCMFASTPATH int dhd_nbuff_free(void *p)
{
#ifdef WLCSM_DEBUG
	if(IS_FKBUFF_PTR(p)) {
#endif
		FkBuff_t *fkb_p=PNBUFF_2_FKBUFF(p);
		/* if fkb is from system FKB pool, release it to system pool and
		 * release pkttag to pkttag pool if it is cloned fkb */
		if (_is_fkb_cloned_pool_(fkb_p) && (fkb_p->dhd_pkttag_info_p)) {
			DHD_PKTTAGS_POOL_PUT((void *)fkb_p->dhd_pkttag_info_p);
			fkb_p->dhd_pkttag_info_p=NULL;
		}
		else if(_is_in_skb_tag_(fkb_p->ptr, fkb_p->flags))
		{ /* free fkbinSkb , convert back to skb and free */
			fkb_p->dirty_p = 0x0; /* blog_p is reset here */
			p = (pNBuff_t)(struct sk_buff *)
			    ((uintptr_t)fkb_p - BLOG_OFFSETOF(sk_buff,fkbInSkb));
		}

		nbuff_free((pNBuff_t)p);
        	return 0;
#ifdef WLCSM_DEBUG
    	} else {
		printk( "%s:%d: WHY SKB COMING TO HERE???	\n",__FUNCTION__,__LINE__);
		return 1;
	}
#endif
}


void *osl_nbuff_dup(osl_t *osh,void *skb) {

    dhd_pkttag_fd_t *tag = (dhd_pkttag_fd_t *)DHD_PKTTAGS_POOL_GET();
    FkBuff_t *fkb_dup, *fkb_p = PNBUFF_2_FKBUFF(skb);
    void *fkb;

    if (tag) {
        if ((fkb_dup = fkb_clone(fkb_p))) {

            tag->flags = DHD_PKTTAG_FD(skb)->flags;
            fkb_dup->dhd_pkttag_info_p = tag;
            fkb= FKBUFF_2_PNBUFF(fkb_dup);
            DHD_PKT_CLR_FKBPOOL(fkb);
            return fkb;
        } else {
            /* Free the tag back to its pool */
            DHD_PKTTAGS_POOL_PUT(tag);
        }
    }
    else
        NBUFF_PKTTAG_NOFBUF(osh)++;
    NBUFF_DUP_NOBUF(osh)++;
    return NULL;
}

int dhd_nbuff_dup(osl_t *osh, void *skb, void **ret_p) {
    if (IS_FKBUFF_PTR(skb)) {
        *ret_p=osl_nbuff_dup(osh,skb);
        return 0;
    } else
        return 1;
}
/* END of GENERIC NBUFF */


/* FKBPOOL */

#ifdef DHD_DEBUG
extern uint dhd_console_ms;
void dhd_set_dconpoll(void) {
    long i = 0 ;
    char *poolsize = wlcsm_nvram_k_get("dhd_dconpoll");
    if(poolsize && (kstrtol(poolsize,0,&i)==0))
        dhd_console_ms = (uint)i;

}
#endif

/*  convert a fkb to skb and convert DA,only needed for DHD_RUNNER enabled case  */
static void *dhd_fkb_2_skb_unicast(osl_t *osh,void *txp,char *unicast_mac)
{
    /* first to restore tag and mac address from orignal FKB as txp itself
     * has chance to be recycled in the API calls, do it at beginning */
#ifdef BCM_NBUFF_PKT
    void *nbuf_tag = nbuff_pkt_get_tag(txp);
#else
    void *nbuf_tag = osl_pkt_get_tag(txp);
#endif
    void* skb_p,*skb_fkbp;
    FkBuff_t *fkb_p=(FkBuff_t *)PNBUFF_2_PBUF(txp);
    int fkb_is_clone= _is_fkb_cloned_pool_(fkb_p);
    skb_fkbp = nbuff_xlate(FKBUFF_2_PNBUFF(fkb_p));
    if (skb_fkbp) {

        //skb_headerinit(BCM_PKT_HEADROOM,BCM_MAX_PKT_LEN,(struct sk_buff *)skb_fkbp,((struct sk_buff *)skb_fkbp)->data,NULL,0,NULL);
        skb_p=PKTDUP_CPY(osh,skb_fkbp);
        if(skb_p) {
            if(unicast_mac)
                bcopy(unicast_mac,(((struct sk_buff*)skb_p)->data),ETHER_ADDR_LEN);
            /* if fkb_p is cloned FKB, it will be recycled in nbuff_xlate,
             *  then only tag to be released,  otherwize,leave it to system to
             *  take care all*/
            if(fkb_is_clone) {
                DHD_PKTTAGS_POOL_PUT(nbuf_tag);
            }
#ifdef WLCSM_DEBUG
            wlcsm_dbg_inc(5,1);
#endif
            PKTFREE(osh,skb_fkbp,FALSE);
            return skb_p;
        }
    }
    return NULL;
}

static inline  FkBuff_t *
_dhd_fkb_pool_get_(osl_t *osh)
{
    FkBuff_t *ret_p = NULL;
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    unsigned long lock_flags;
#endif
    DHD_FKB_POOL_LOCK();
    if ((g_dhd_fkb_pool_free_p != NULL) && g_dhd_pool_available)
    {
        ret_p = g_dhd_fkb_pool_free_p;

        g_dhd_fkb_pool_free_p =g_dhd_fkb_pool_free_p->list;
        g_dhd_fkb_pool_free--;
#ifdef WLCSM_DEBUG
        wlcsm_dbg_inc(2,1);
        if(g_dhd_fkb_pool_max_usage<g_dhd_fkbpool_size-g_dhd_fkb_pool_free)
            g_dhd_fkb_pool_max_usage=g_dhd_fkbpool_size-g_dhd_fkb_pool_free;
#endif
    } else {
        NBUFF_FKBPOOL_NOFBUF(osh)++;
    }
    DHD_FKB_POOL_UNLOCK();
    return ret_p;
}

int dhd_fkb_pool_init( void ) {
    long i = 0 ;
    char *poolsize=wlcsm_nvram_k_get("dhd_fkbpool_size");
    struct sysinfo sys_info;
    FkBuff_t *fkb_p;
    if(poolsize && (kstrtol(poolsize,0,&i)==0) && (i <= DHD_FKBPOOL_MAX_SIZE))
        g_dhd_fkbpool_size= (uint)i;
    printk("DHD_FKB_POOL size is:%u and entry size:%zu\n",g_dhd_fkbpool_size,DHD_FKBPOOL_ENTRY_SIZE);
    if(g_dhd_fkbpool_size==0) return 0;
    si_meminfo(&sys_info);
    if((g_dhd_fkbpool_size*DHD_FKBPOOL_ENTRY_SIZE)>=(sys_info.freeram*PAGE_SIZE)) {
        printk( "%s:%d:	FREE memem:%lu and fkbpool rquest more than it. \n",__FUNCTION__,__LINE__,sys_info.freeram*PAGE_SIZE);
        return 0;
    }
    for (i = 0; i < g_dhd_fkbpool_size; ++i) {
        fkb_p=(FkBuff_t *)(kmalloc(DHD_FKBPOOL_ENTRY_SIZE, GFP_ATOMIC));
        if(!fkb_p) {
            g_dhd_fkbpool_size=i;
            printk("..Could not allocate mem for DHD FKB pool,allocated:%d\r\n",g_dhd_fkbpool_size);
            break;
        }
        _fkb_set_ref(fkb_p,0);
        dhd_fkb_pool_put(fkb_p,0,0);
    }
    return 0;
}

void
dhd_fkb_pool_free( void )
{
    FkBuff_t *fkb_free_p;

    if (g_dhd_fkb_pool_free == -1)
        return; /* has been free'd */

    /* wait until all fkb freed to pool */
    while(g_dhd_fkb_pool_free != g_dhd_fkbpool_size)
        yield();
    /* no lock needed as pool is not availabe to get now */
    while ((fkb_free_p = g_dhd_fkb_pool_free_p)) {
        g_dhd_fkb_pool_free_p = fkb_free_p->list;
        kfree(fkb_free_p);
    }
    g_dhd_fkb_pool_free_p = NULL;
    g_dhd_fkb_pool_free = -1;
}

/* fkb recycle hook format */
void
dhd_fkb_pool_put(void *fkb, unsigned long context,uint32_t flags)
{
    FkBuff_t *fkb_p;
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    unsigned long lock_flags;
#endif
    DHD_FKB_POOL_LOCK();

    if(flags!=0 && context) {
        fkb_p=(FkBuff_t *)context;
    } else
        fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(fkb);

    dhd_fkb_clear_tag(fkb_p);

    fkb_p->list = (void *) g_dhd_fkb_pool_free_p;
    g_dhd_fkb_pool_free_p = fkb_p;
    g_dhd_fkb_pool_free++;

#ifdef WLCSM_DEBUG
    WLCSM_TRACE(WLCSM_TRACE_DBG, "fkb:%p to pool ok\n",fkb_p);
    wlcsm_dbg_inc(3,1);
#endif

    DHD_FKB_POOL_UNLOCK();
    return ;
}


#ifdef WLCSM_DEBUG
FkBuff_t *
dhd_fkb_pool_clone2unicast(osl_t *osh,FkBuff_t *fkb_p,void *mac,const char *func, int line) {
#else
FkBuff_t *
dhd_fkb_pool_clone2unicast(osl_t *osh,FkBuff_t *fkb_p,void *mac) {
#endif
    FkBuff_t *fkbm_p=(FkBuff_t *)PNBUFF_2_PBUF(fkb_p);
    FkBuff_t *clone_fkb_p= NULL;
    dhd_pkttag_fd_t *pkttag_p;
    uint32 flowid=0;
    bool is_clone_fkb=FALSE;
    if(fkbm_p->len <= DHD_FKB_DATA_MAXLEN && (clone_fkb_p=_dhd_fkb_pool_get_(osh))) {
        is_clone_fkb= _is_fkb_cloned_pool_(fkbm_p);
        if (is_clone_fkb)   {
            pkttag_p=fkbm_p->dhd_pkttag_info_p;
            flowid= fkbm_p->flowid;
            fkbm_p=(FkBuff_t *)PNBUFF_2_PBUF(fkbm_p->master_p);
        }
        memcpy(clone_fkb_p,fkbm_p,(fkbm_p->data+fkbm_p->len-(uint8_t *)fkbm_p));
        clone_fkb_p->recycle_hook = dhd_fkb_pool_put;
        clone_fkb_p->recycle_context = (unsigned long)clone_fkb_p;
        fkb_set_ref(clone_fkb_p, 1);
        clone_fkb_p->data= (uint8_t *)clone_fkb_p+((uintptr_t)fkbm_p->data -(uintptr_t)fkbm_p);
        clone_fkb_p->dirty_p=_to_dptr_from_kptr_(clone_fkb_p->data+clone_fkb_p->len);
        /* if the master has DHDHDR already moved */

        if(dhd_get_fkb_dhdhdr_flag(FKBUFF_2_PNBUFF(fkbm_p))) {
            memcpy(clone_fkb_p->data,fkbm_p->data - DOT11_LLC_SNAP_HDR_LEN,ETHER_HDR_LEN);
#ifdef WLCSM_DEBUG
            wlcsm_dbg_inc(7,1);
            wlcsm_dump_pkt("dhdhdr",clone_fkb_p->data,clone_fkb_p->len,0,0);
#endif
        }
        if(mac)
            memcpy(clone_fkb_p->data,mac,ETHER_ADD_LEN);
        if(is_clone_fkb)  {
            /* for cloned_fkb copy its pkttag to the cloned pkttag */
            memcpy((void *)(PFKBUFF_TO_PHEAD(clone_fkb_p)),pkttag_p,sizeof(dhd_pkttag_fd_t));
            clone_fkb_p->flowid=flowid;
        }
        clone_fkb_p=FKBUFF_2_PNBUFF(clone_fkb_p);

        DHD_PKT_CLR_DATA_DHDHDR(clone_fkb_p);
        DHD_PKT_SET_FKBPOOL(clone_fkb_p);
        return clone_fkb_p;
    } else {
#ifdef WLCSM_DEBUG
        wlcsm_dbg_inc(1,1);
        WLCSM_TRACE(WLCSM_TRACE_LOG, "Failure to allocate from FKBPOOL");
#endif
        return dhd_fkb_2_skb_unicast(osh,fkb_p,mac);
    }
}

/* --END of FKBPOOL -- */

/* PKTLIST */

INLINE BCMFASTPATH 
void dhd_pkt_queue_head_init(DHD_PKT_LIST *list)
{
    dll_init(list);
}

INLINE BCMFASTPATH 
void dhd_pkt_queue_head(DHD_PKT_LIST *list,	void *newpkt)
{
    (DHD_PKTTAG_FD(newpkt))->pkt = newpkt;
    dll_append(list, &(DHD_PKTTAG_FD(newpkt)->node));
}

INLINE BCMFASTPATH 
void dhd_pkt_unlink(DHD_PKT_LIST *list, void *pkt)
{
    dll_delete(&(DHD_PKTTAG_FD(pkt)->node));
}

INLINE BCMFASTPATH 
void *dhd_pkt_dequeue(DHD_PKT_LIST *list)
{
    void *pkt = NULL;
    dll_t *item;

    item = dll_head_p(list);
    if (item) {
        pkt = ((dhd_pkttag_fd_t*)item)->pkt;
        dll_delete(dll_head_p(list));
    }
    return pkt;
}

void dhd_pkt_queue_purge(osl_t *osh, DHD_PKT_LIST *list)
{
    dll_t *item, *next;
    void *pkt;

    for (item = dll_head_p(list); !dll_end(list, item); item = next) {
        next = dll_next_p(item);
        pkt = ((dhd_pkttag_fd_t*)item)->pkt;
        PKTFREE(osh, pkt, FALSE);
    }
}

#if defined(DSLCPE) || defined(BCM_NBUFF_WLMCAST)
/* Add nbuff dump output to a buffer */
void
dhd_nbuff_dump(void *dhd, void *strb)
{
	dhd_pub_t  *dhdp= (dhd_pub_t *)dhd;

	struct bcmstrbuf *strbuf=(struct bcmstrbuf *)strb;
	bcm_bprintf(strbuf, "tx_nodup:		%lu\n", NBUFF_DUP_NOBUF(dhdp->osh));
	bcm_bprintf(strbuf, "tx_tag_nobuf:	%lu	,free tags in the pool:	%lu	,used:%lu \n",
		NBUFF_PKTTAG_NOFBUF(dhdp->osh), g_dhd_pkttags_pool_free,g_dhd_pkttags_pool_size-g_dhd_pkttags_pool_free);
	bcm_bprintf(strbuf, "fkbpool_nobuf:	%lu	,free fkbs in the pool: %lu	,used:%lu\n",
		NBUFF_FKBPOOL_NOFBUF(dhdp->osh), g_dhd_fkb_pool_free,g_dhd_fkbpool_size-g_dhd_fkb_pool_free);

	return;
}
#endif

BCMFASTPATH 
#ifdef BCM_NBUFF_PKT
void inline *nbuff_pkt_get_tag(void *pkt)
#else
void inline *osl_pkt_get_tag(void *pkt)
#endif
{
	if (IS_SKBUFF_PTR(pkt)) {
		return  ((void *)(((struct sk_buff*)(pkt))->cb));
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		if (_is_fkb_cloned_pool_(fkb_p))
			return (void *)fkb_p->dhd_pkttag_info_p;
		else
			return ((void *)(PFKBUFF_TO_PHEAD(fkb_p)));
	}
}

#ifdef BCM_NBUFF_PKT
void inline nbuff_pkt_clear_tag(void *pkt)
#else
void inline osl_pkt_clear_tag(void *pkt)
#endif
{
#ifdef BCM_NBUFF_PKT
	void *tag = nbuff_pkt_get_tag(pkt);
#else
	void *tag = osl_pkt_get_tag(pkt);
#endif

	*(uint32 *)(tag) = 0;
	*(uint32 *)(tag+4) = 0;
	*(uint32 *)(tag+8) = 0;
	*(uint32 *)(tag+12) = 0;
	*(uint32 *)(tag+16) = 0;
	*(uint32 *)(tag+20) = 0;
	*(uint32 *)(tag+24) = 0;
	*(uint32 *)(tag+28) = 0;
}

INLINE BCMFASTPATH 
uint
#ifdef BCM_NBUFF_PKT
nbuff_pktprio(void *pkt)
#else
osl_pktprio(void *pkt)
#endif
{
	uint32 prio = 0;
	if (IS_SKBUFF_PTR(pkt)) {
		prio = ((struct sk_buff*)pkt)->mark>>PRIO_LOC_NFMARK & 0x7;
	} else { /* manuplated in dhd, 3bit prio + 10bit flowid */
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		prio = fkb_p->wl.ucast.dhd.wl_prio; /* Same bit pos used for mcast */
	}

	if (prio > 7) {
		prio = 0;
	}

	return prio;
}

INLINE BCMFASTPATH 
void
#ifdef BCM_NBUFF_PKT
nbuff_pktsetprio(void *pkt, uint x)
#else
osl_pktsetprio(void *pkt, uint x)
#endif
{
    if (IS_SKBUFF_PTR(pkt)) {
        ((struct sk_buff*)pkt)->mark &= ~(0x7 << PRIO_LOC_NFMARK);
        ((struct sk_buff*)pkt)->mark |= (x & 0x7) << PRIO_LOC_NFMARK;
        ((struct sk_buff*)pkt)->wl.ucast.dhd.wl_prio = x;
    }
    else
    {
        FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
        fkb_p->wl.ucast.dhd.wl_prio = x; /* Same bit pos used for mcast */
    }
}

INLINE BCMFASTPATH 
uint
#ifdef BCM_NBUFF_PKT
nbuff_pktflowid(void *pkt)
#else
osl_pktflowid(void *pkt)
#endif
{
	uint fid = -1;

	if (IS_SKBUFF_PTR(pkt)) {
		fid = ((struct sk_buff *)pkt)->wl.ucast.dhd.flowring_idx;
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		/* for both ucast and mcast, flowring_idx is at the same position */
		fid = fkb_p->wl.ucast.dhd.flowring_idx;
	}
	return fid;
}

INLINE BCMFASTPATH 
void
#ifdef BCM_NBUFF_PKT
nbuff_pktsetflowid(void *pkt, uint x)
#else
osl_pktsetflowid(void *pkt, uint x)
#endif
{
	if (IS_SKBUFF_PTR(pkt)) {
		((struct sk_buff *)pkt)->wl.ucast.dhd.flowring_idx = x;
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		/* for both ucast and mcast, flowring_idx is at the same position */
		fkb_p->wl.ucast.dhd.flowring_idx = x;
	}
	return;
}

uchar*
#ifdef BCM_NBUFF_PKT
nbuff_pktdata(osl_t *osh, void *pkt)
#else
osl_pktdata(osl_t *osh, void *pkt)
#endif
{
	BCM_REFERENCE(osh);
	return nbuff_get_data((pNBuff_t)pkt);
}

uint
#ifdef BCM_NBUFF_PKT
nbuff_pktlen(osl_t *osh, void *pkt)
#else
osl_pktlen(osl_t *osh, void *pkt)
#endif
{
	BCM_REFERENCE(osh);
	return nbuff_get_len((pNBuff_t)pkt);
}

INLINE BCMFASTPATH 
void
#ifdef BCM_NBUFF_PKT
nbuff_pktsetlen(osl_t *osh, void *pkt, uint len)
#else
osl_pktsetlen(osl_t *osh, void *pkt, uint len)
#endif
{
	BCM_REFERENCE(osh);
	if (IS_SKBUFF_PTR((pNBuff_t)pkt))
		__pskb_trim((struct sk_buff*)pkt, len);
	/* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
	else
		nbuff_set_len((pNBuff_t)pkt, len);
}

INLINE BCMFASTPATH 
uint
nbuff_pktheadroom(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt))
		return (uint) fkb_headroom((FkBuff_t *)PNBUFF_2_PBUF(pkt));
	else
		return (uint) skb_headroom((struct sk_buff *) pkt);
}

INLINE BCMFASTPATH 
uint
nbuff_pkttailroom(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return 0;
	} else {
		return (uint) skb_tailroom((struct sk_buff *) pkt);
	}
}

INLINE BCMFASTPATH 
uchar*
nbuff_pktpush(osl_t *osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_push(pkt, bytes));
}

INLINE BCMFASTPATH 
uchar*
nbuff_pktpull(osl_t *osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_pull(pkt, bytes));
}

INLINE BCMFASTPATH 
bool
nbuff_pktshared(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (((struct sk_buff*)pkt)->cloned);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return FALSE;
	}
}

INLINE BCMFASTPATH 
void *
#ifdef BCM_NBUFF_PKT
nbuff_pktlink(void *pkt)
#else
osl_pktlink(void *pkt)
#endif
{
	if (IS_FKBUFF_PTR(pkt))
		return nbuff_get_queue(pkt);
	else
		return (((struct sk_buff*)(pkt))->prev);
}

INLINE BCMFASTPATH 
void
#ifdef BCM_NBUFF_PKT
nbuff_pktsetlink(void *pkt, void *x)
#else
osl_pktsetlink(void *pkt, void *x)
#endif
{
	if (IS_FKBUFF_PTR(pkt))
		nbuff_set_queue(pkt, x);
	else
		(((struct sk_buff*)(pkt))->prev = (struct sk_buff*)(x));
}

INLINE BCMFASTPATH 
void
nbuff_pktsetnext(osl_t *osh, void *pkt, void *x)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt))
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	else
		(((struct sk_buff*)(pkt))->next = (struct sk_buff*)(x));
}

INLINE BCMFASTPATH 
void *
nbuff_pktnext(osl_t *osh, void *pkt)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);

		return NULL;
	} else {
		return (((struct sk_buff*)(pkt))->next);
	}
}


/*  for cloned FKB, attach tag info to it */
INLINE BCMFASTPATH 
#ifdef BCM_NBUFF_PKT
int nbuff_pkttag_attach(void *osh, void *pkt)
#else
int osl_pkttag_attach(void *osh, void *pkt)
#endif
{
	if (IS_FKBUFF_PTR(pkt) && _is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(pkt))) {
		FkBuff_t *fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pkt);
		dhd_pkttag_fd_t *tag = DHD_PKTTAGS_POOL_GET();
		if (!tag) {
			/* if not attached ok, set it to NULL, then pktfree will
			 * not release the tag.
			 */
			NBUFF_PKTTAG_NOFBUF(osh)++;
			fkb_p->dhd_pkttag_info_p = NULL;
			return 1;
		}
		//tag->flags = 0;
		fkb_p->dhd_pkttag_info_p = tag;
	}
#if 1
	DHD_PKT_CLR_WFD_BUF(pkt);
	DHD_PKT_CLR_WMF_FKB_UCAST(pkt);
	DHD_PKT_CLR_WAN_MCAST(pkt);
#endif
	return 0;
}


INLINE BCMFASTPATH 
void *
osl_pkt_get_dirtyp(osl_t *osh, void *pkt)
{
	if (pkt) {
		if(IS_FKBUFF_PTR(pkt)) {
			return _to_kptr_from_dptr_(((FkBuff_t *)PNBUFF_2_PBUF(pkt))->dirty_p);
		} else
			return skb_shinfo((struct sk_buff*)(pkt))->dirty_p;
	}
	return NULL;
}

INLINE BCMFASTPATH 
void
osl_pkt_set_dirtyp(osl_t *osh, void *pkt, void *addr)
{
	if (pkt) {
		if (IS_FKBUFF_PTR(pkt)) {
			((FkBuff_t *)PNBUFF_2_PBUF(pkt))->dirty_p = _to_dptr_from_kptr_(addr);
		} else {
			skb_shinfo((struct sk_buff*)(pkt))->dirty_p = addr;
		}
	}
}



BCMFASTPATH 
void inline
osl_pkt_set_dirtyp_len(osl_t *osh, void *p, int len)
{
	if (p) {
		if (IS_FKBUFF_PTR(p)) {
			FkBuff_t *fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(p);
			len = fkb_p->len > len ? len : fkb_p->len;
			if (fkb_p->dirty_p < (uint8 *)(fkb_p->data+len))
				fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data+len);
		} else {
			if (skb_shinfo((struct sk_buff*)(p))->dirty_p < ((uint8 *)(((struct sk_buff*)p)->data)+len))
				skb_shinfo((struct sk_buff*)(p))->dirty_p = (((struct sk_buff*)p)->data)+len;
		}
	}
}

#ifdef DSLCPE_CACHE_SMARTFLUSH
INLINE BCMFASTPATH 
uint32 osl_dirtyp_is_valid(osl_t *osh, void *p)
{
    uint8_t *dirty_p;
    static int dirtyp_invalid_cnt=0;
    if(IS_FKBUFF_PTR(p)) {

        FkBuff_t *fkb_p=(FkBuff_t *)PNBUFF_2_PBUF(p);
        dirty_p=_to_kptr_from_dptr_(fkb_p->dirty_p);
        if (dirty_p && dirty_p >=fkb_p->data && dirty_p <= fkb_p->data+fkb_p->len)
            return 1;
        else if (dirty_p == NULL)
            return 0;

        /*
         * Something is wrong.  dirty_p is not NULL, but also not pointing to
         * the inside of the data buffer region.  This is bad, must be fixed.
         */
        dirtyp_invalid_cnt++;

        printk("dirtyp_is_valid(%d) dirty_p:%p  data[%p] len: %d]\n",
               dirtyp_invalid_cnt, dirty_p,fkb_p->data, fkb_p->len);
        fkb_p->dirty_p=fkb_p->data+fkb_p->len;
        return 1;


    } else {
        struct sk_buff *skb = (struct sk_buff *)p;

        dirty_p = PKTGETDIRTYP(osh, skb);
        if (dirty_p && dirty_p >= skb->data && dirty_p <= skb_tail_pointer(skb))
            return 1;

        if (dirty_p == NULL)
            return 0;

        /*
         * Something is wrong.  dirty_p is not NULL, but also not pointing to
         * the inside of the data buffer region.  This is bad, must be fixed.
         */
        dirtyp_invalid_cnt++;
        printk("dirtyp_is_valid(%d) %p skb %p data[%p %p]\n",
               dirtyp_invalid_cnt, dirty_p, skb, skb->data, skb_tail_pointer(skb));
        return 0;
    }
}
#endif /* DSLCPE_CACHE_SMARTFLUSH */

#ifndef FWDER_MAX_RADIO
#define FWDER_MAX_RADIO         (4)
#endif
dhd_pub_t *g_dhd_info[FWDER_MAX_RADIO];  /* for each dongle adapter */


int dhd_nbuff_attach(void)
{
    if (dhd_fkb_pool_init())
        return -1;
    if (dhd_pkttag_pool_init()) {
        printk("..Could not allocate mem for PKTTAG pool\r\n");
        dhd_fkb_pool_free();
        return -1;
    }
#if defined(BCM_WFD)
    /* initialize global variables */
    memset(&g_dhd_info[0], 0, sizeof(dhd_pub_t *) * FWDER_MAX_RADIO);
#endif /* BCM_WFD */
#ifdef WLCSM_DEBUG
    wlcsm_dbg_reg(1,"duplicated FKB failure:");
    wlcsm_dbg_reg(2,"alloc from fkbpool:");
    wlcsm_dbg_reg(3,"free  to   fkbpool:");
    wlcsm_dbg_reg(4,"dhdup from fkbpool:");
    wlcsm_dbg_reg(5,"allocated FKB->SKB:");
    wlcsm_dbg_reg(6,"wrong prio:");
    wlcsm_dbg_reg(7,"has dhdhdr:");
    wlcsm_dbg_reg(8,"get tag from tag pool:");
    wlcsm_dbg_reg(9,"put tag to tag pool:");
    wlcsm_dbg_reg(10,"wfd mcast packet in");
    wlcsm_dbg_reg(11,"wfd ucast packet in");
    wlcsm_dbg_reg(12,"slow path packets");
    wlcsm_dbg_reg_item("Max fkb item used  :",&g_dhd_fkb_pool_max_usage);
    wlcsm_dbg_reg_item("freed fkb in fkbpool:",&g_dhd_fkb_pool_free);
    wlcsm_dbg_reg_item("freed tag in tagpool:",&g_dhd_pkttags_pool_free);
#endif
#ifdef DHD_DEBUG
    dhd_set_dconpoll();
#endif

    g_dhd_pool_available=1;
    return 0;
}
void dhd_nbuff_detach(void) {

    g_dhd_pool_available=0;
    dhd_fkb_pool_free();
    dhd_pkttag_pool_free();

}

void dhd_set_fkb_dhdhdr_flag(void *pkt)
{
    FkBuff_t *fkb_master = NULL, *fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pkt);
    dhd_pkttag_fd_t  *pkttag=NULL;
    fkb_master=_is_fkb_cloned_pool_(fkb_p)?fkb_p->master_p:fkb_p;
    pkttag=(dhd_pkttag_fd_t *)(PFKBUFF_TO_PHEAD(fkb_master));
    pkttag->flags|=DHD_PKTTAG_DATA_DHDHDR;
    return;
}

bool dhd_get_fkb_dhdhdr_flag(void *pkt)
{
    FkBuff_t *fkb_master = NULL, *fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pkt);
    dhd_pkttag_fd_t  *pkttag=NULL;
    fkb_master=_is_fkb_cloned_pool_(fkb_p)?fkb_p->master_p:fkb_p;
    pkttag=(dhd_pkttag_fd_t *)(PFKBUFF_TO_PHEAD(fkb_master));
    return (pkttag->flags & DHD_PKTTAG_DATA_DHDHDR)? TRUE:FALSE;
}

void dhd_clr_fkb_dhdhdr_flag(void *pkt)
{
    FkBuff_t *fkb_master = NULL, *fkb_p = (FkBuff_t *)PNBUFF_2_PBUF(pkt);
    dhd_pkttag_fd_t  *pkttag=NULL;
    fkb_master=_is_fkb_cloned_pool_(fkb_p)?fkb_p->master_p:fkb_p;
    pkttag=(dhd_pkttag_fd_t *)(PFKBUFF_TO_PHEAD(fkb_master));
    pkttag->flags &= ~ DHD_PKTTAG_DATA_DHDHDR;
    return;
}
#ifdef WLCSM_DEBUG
void  dhd_pkt_set_fkbclone(void *pkt)
{
    ((DHD_PKTTAG_FD(pkt))->flags |= DHD_PKTTAG_FKBPOOL);
}
#endif

#ifdef BCM_NBUFF_PKT
void *
nbuff_pktdup(osl_t *osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
		p = osl_pktdup(osh, pkt);
	} else {
		p = osl_nbuff_dup(osh, pkt);
	}
	return p;
}

void *
nbuff_pktdup_cpy(osl_t *osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
		p = osl_pktdup_cpy(osh, pkt);
	} else {
		p = DHD_FKB_CLONE2UNICAST(osh, pkt, NULL);
	}
	return p;
}

void BCMFASTPATH
nbuff_pktfree(osl_t *osh, void *pkt, bool send)
{
	if (IS_SKBUFF_PTR(pkt)) {
		linux_pktfree(osh, pkt, send);
	} else {
		dhd_nbuff_free(pkt);
	}
	return;
}

struct sk_buff *
nbuff_pkt_tonative(osl_t *osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = osl_pkt_tonative(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}

void *
nbuff_pkt_frmnative(osl_t *osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = osl_pkt_frmnative(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}


#if defined(BCM_NBUFF_PKT_BPM)

/**
 * =============================================================================
 * Section: BPM based data buffers
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function    : dhd_databuf_alloc
 * Description : Allocate a BPM data buffer from local cached pool and refill
 *               the pool if falling short. Packets allocated from BPM have
 *               their data buffers pre-cache-invalidated
 * -----------------------------------------------------------------------------
 */

void *
dhd_databuf_alloc(dhd_pub_t * dhdp)
{
    void * databuf = NULL;

    if (NBUFF_DATABUFF_CACHE_IDX(dhdp->osh) >= 0)
    {
        databuf = NBUFF_DATABUFF_CACHE(dhdp->osh)[NBUFF_DATABUFF_CACHE_IDX(dhdp->osh)--];
        bcm_prefetch(NBUFF_DATABUFF_CACHE(dhdp->osh)[NBUFF_DATABUFF_CACHE_IDX(dhdp->osh)]);
    }
    else
    {
        /* Refill the local cache from global pool */
        if (gbpm_alloc_mult_buf(DHD_DATABUF_CACHE_MAXBUFS,
                    NBUFF_DATABUFF_CACHE(dhdp->osh)) != GBPM_ERROR)
        {
            NBUFF_DATABUFF_CACHE_IDX(dhdp->osh) = DHD_DATABUF_CACHE_MAXBUFS - 1;
            databuf = NBUFF_DATABUFF_CACHE(dhdp->osh)[NBUFF_DATABUFF_CACHE_IDX(dhdp->osh)--];
            bcm_prefetch(NBUFF_DATABUFF_CACHE(dhdp->osh)[NBUFF_DATABUFF_CACHE_IDX(dhdp->osh)]);
        }
    }

    return databuf;
}   /* dhd_databuf_alloc() */


/**
 * -----------------------------------------------------------------------------
 * Function : Free all BPM buffers in local cache pool
 * -----------------------------------------------------------------------------
 */

void
dhd_databuf_cache_clear(dhd_pub_t * dhdp)
{
    void * databuf;

    while (NBUFF_DATABUFF_CACHE_IDX(dhdp->osh) >= 0)
    {
        databuf = NBUFF_DATABUFF_CACHE(dhdp->osh)[NBUFF_DATABUFF_CACHE_IDX(dhdp->osh)--];
        cache_invalidate_len(databuf, BCM_MAX_PKT_LEN);
        gbpm_free_buf(databuf);
    }
}   /* dhd_databuf_cache_clear() */


/**
 * -----------------------------------------------------------------------------
 * Function : Allocate a SKB and attach to BPM buffer
 * -----------------------------------------------------------------------------
 */

struct sk_buff *
dhd_xlate_to_skb(dhd_pub_t * dhd_pub, pNBuff_t pNBuff)
{
    struct sk_buff * skb;

    skb = nbuff_xlate(pNBuff);

    if (unlikely(skb == (struct sk_buff *) NULL))
    {
        return NULL;
	}
    SKB_BPM_TAINTED(skb);

    return skb;
}   /* dhd_xlate_to_skb() */


int dhd_nbuff_bpm_init(dhd_pub_t *dhdp)
{
	uint32 mem_bytes = 0;

	NBUFF_DATABUFF_CACHE_IDX(dhdp->osh) = -1;

	/* Allocate databuf_cache which helps to reduce the overhead when posting RX buffers */
	mem_bytes = sizeof(void *) * DHD_DATABUF_CACHE_MAXBUFS;
	NBUFF_DATABUFF_CACHE(dhdp->osh) = (void **)kmalloc(mem_bytes, GFP_ATOMIC);
	if (NBUFF_DATABUFF_CACHE(dhdp->osh) == NULL)
	{
		DHD_ERROR(("%s: Failed to allocate memory for databuf cache\n",  __FUNCTION__));
		return BCME_ERROR;
	}
	memset(NBUFF_DATABUFF_CACHE(dhdp->osh), 0, mem_bytes);

	return BCME_OK;
}


void dhd_nbuff_bpm_deinit(dhd_pub_t *dhdp)
{
    if (NBUFF_DATABUFF_CACHE(dhdp->osh) == NULL)
        return;

    dhd_databuf_cache_clear(dhdp);

    memset(NBUFF_DATABUFF_CACHE(dhdp->osh), 0xFF, /* scribble */
            (sizeof(void *) * DHD_DATABUF_CACHE_MAXBUFS));
    kfree(NBUFF_DATABUFF_CACHE(dhdp->osh));

    NBUFF_DATABUFF_CACHE_IDX(dhdp->osh) = -1;
    NBUFF_DATABUFF_CACHE(dhdp->osh) = NULL;
}

#endif /* BCM_NBUFF_PKT_BPM */

#endif /* BCM_NBUFF_PKT */

#if defined(BCM_NBUFF_PKT_BPM_SKB)
struct sk_buff *dhd_nbuff_bpm_skb_get(dhd_pub_t *dhdp, int len) {
    struct sk_buff *pkt_skb = gbpm_alloc_buf_skb_attach(len);

    if (pkt_skb) {
        SKB_BPM_TAINTED(pkt_skb);
        PKTACCOUNT(dhdp->osh, 1, TRUE);
        pkt_skb->blog_p = NULL;
    }

    return pkt_skb;
}
#endif /* BCM_NBUFF_PKT_BPM_SKB */

#endif /* BCM_NBUFF */
