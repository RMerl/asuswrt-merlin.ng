#if defined(CONFIG_BCM_KF_NBUFF)
/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

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



/*
 *******************************************************************************
 * File Name  : gbpm.c
 *******************************************************************************
 */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/gbpm.h>
#include <linux/bcm_colors.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <linux/bcm_realtime.h>

/* Debug macros */
int gbpm_debug_g = 0;

#if defined(CC_GBPM_SUPPORT_DEBUG)
#define gbpm_print(fmt, arg...)                                         \
    if ( gbpm_debug_g )                                                     \
    printk( CLRc "GBPM %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define gbpm_assertv(cond)                                              \
    if ( !cond ) {                                                      \
        printk( CLRerr "GBPM ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return;                                                         \
    }
#define gbpm_assertr(cond, rtn)                                         \
    if ( !cond ) {                                                      \
        printk( CLRerr "GBPM ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return rtn;                                                     \
    }
#define GBPM_DBG(debug_code)    do { debug_code } while(0)
#else
#define gbpm_print(fmt, arg...) NULL_STMT
#define gbpm_assertv(cond)      NULL_STMT
#define gbpm_assertr(cond, rtn) NULL_STMT
#define GBPM_DBG(debug_code)    NULL_STMT
#endif

#define gbpm_error(fmt, arg...)                                         \
    printk( CLRerr "GBPM ERROR %s :" fmt CLRnl, __FUNCTION__, ##arg)

#undef  GBPM_DECL
#define GBPM_DECL(x)        #x,         /* string declaration */


/* global Buffer Pool Manager (BPM) */
gbpm_status_hook_t gbpm_enet_status_hook_g = (gbpm_status_hook_t)NULL;
EXPORT_SYMBOL(gbpm_enet_status_hook_g);


#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
gbpm_status_hook_t gbpm_fap_status_hook_g = (gbpm_status_hook_t)NULL;

gbpm_thresh_hook_t gbpm_fap_thresh_hook_g = (gbpm_thresh_hook_t)NULL;
gbpm_thresh_hook_t gbpm_fap_enet_thresh_hook_g = (gbpm_thresh_hook_t)NULL;
gbpm_thresh_hook_t gbpm_enet_thresh_hook_g = (gbpm_thresh_hook_t)NULL;
gbpm_upd_buf_lvl_hook_t gbpm_fap_upd_buf_lvl_hook_g = (gbpm_upd_buf_lvl_hook_t)NULL;

EXPORT_SYMBOL(gbpm_fap_status_hook_g);
EXPORT_SYMBOL(gbpm_fap_thresh_hook_g);
EXPORT_SYMBOL(gbpm_fap_enet_thresh_hook_g);
EXPORT_SYMBOL(gbpm_enet_thresh_hook_g);
EXPORT_SYMBOL(gbpm_fap_upd_buf_lvl_hook_g);
#endif



#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
gbpm_status_hook_t gbpm_xtm_status_hook_g = (gbpm_status_hook_t)NULL;
gbpm_thresh_hook_t gbpm_xtm_thresh_hook_g = (gbpm_thresh_hook_t)NULL;

EXPORT_SYMBOL(gbpm_xtm_status_hook_g);
EXPORT_SYMBOL(gbpm_xtm_thresh_hook_g);
#endif


/*
 *------------------------------------------------------------------------------
 * Default hooks.
 * FIXME: Group these hooks into a structure and change gbpm_bind to use
 *        a structure.
 *------------------------------------------------------------------------------
 */
static gbpm_dyn_buf_lvl_hook_t gbpm_dyn_buf_lvl_hook_g = 
                                    (gbpm_dyn_buf_lvl_hook_t ) NULL;
static gbpm_alloc_mult_hook_t gbpm_alloc_mult_hook_g = (gbpm_alloc_mult_hook_t) NULL;
static gbpm_free_mult_hook_t gbpm_free_mult_hook_g=(gbpm_free_mult_hook_t) NULL;
static gbpm_alloc_hook_t gbpm_alloc_hook_g = (gbpm_alloc_hook_t) NULL;
static gbpm_free_hook_t gbpm_free_hook_g = (gbpm_free_hook_t) NULL;
static gbpm_resv_rx_hook_t gbpm_resv_rx_hook_g = (gbpm_resv_rx_hook_t) NULL;
static gbpm_unresv_rx_hook_t gbpm_unresv_rx_hook_g=(gbpm_unresv_rx_hook_t) NULL;
static gbpm_get_total_bufs_hook_t gbpm_get_total_bufs_hook_g=(gbpm_get_total_bufs_hook_t) NULL;
static gbpm_get_avail_bufs_hook_t gbpm_get_avail_bufs_hook_g=(gbpm_get_avail_bufs_hook_t) NULL;
static gbpm_get_max_dyn_bufs_hook_t gbpm_get_max_dyn_bufs_hook_g=(gbpm_get_max_dyn_bufs_hook_t) NULL;


#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
static void gbpm_do_work(struct work_struct *);
static DECLARE_WORK(gbpm_work, gbpm_do_work);
static struct workqueue_struct *gbpm_workqueue;

extern gbpm_evt_hook_t gbpm_fap_evt_hook_g;

/* Do the BPM work */
void gbpm_do_work(struct work_struct *work_unused)
{
    /* process BPM pending events */
    if ( likely(gbpm_fap_evt_hook_g != (gbpm_evt_hook_t)NULL) )
        gbpm_fap_evt_hook_g();
}
EXPORT_SYMBOL(gbpm_do_work);

/* Add the BPM work */
void gbpm_queue_work(void)
{
	queue_work(gbpm_workqueue, &gbpm_work);
}

EXPORT_SYMBOL(gbpm_queue_work);
#endif


/* Stub functions */
int gbpm_dyn_buf_lvl_stub( void ) 
{
    return 1;
}

int gbpm_alloc_mult_stub( uint32_t num, void **buf_p )
{
    return GBPM_ERROR;
}

void gbpm_free_mult_stub( uint32_t num, void **buf_p )
{
    return;
}

void * gbpm_alloc_stub( void )
{
    return NULL;
}

void gbpm_free_stub( void * buf_p )
{
    return;
}

int gbpm_resv_rx_stub( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t bulk_alloc_cnt ) 
{
    return GBPM_ERROR;
}

int gbpm_unresv_rx_stub( gbpm_port_t port, uint32_t chnl ) 
{
    return GBPM_ERROR;
}

uint32_t gbpm_get_total_bufs_stub( void )
{
    return 0;
}

uint32_t gbpm_get_avail_bufs_stub( void )
{
    return 0;
}

uint32_t gbpm_get_max_dyn_bufs_stub( void )
{
    return 0;
}


/* BQoS BPM APIs invoked through hooks */
int gbpm_get_dyn_buf_level(void) 
{
    return gbpm_dyn_buf_lvl_hook_g();
}

int gbpm_alloc_mult_buf( uint32_t num, void **buf_p )
{
    return gbpm_alloc_mult_hook_g( num, buf_p );
}

void gbpm_free_mult_buf( uint32_t num, void **buf_p )
{
    gbpm_free_mult_hook_g( num, buf_p );
}

void * gbpm_alloc_buf( void )
{
    return gbpm_alloc_hook_g();
}

void gbpm_free_buf( void * buf_p )
{
    return gbpm_free_hook_g( buf_p );
}

int gbpm_resv_rx_buf( gbpm_port_t port, uint32_t chnl,
        uint32_t num_rx_buf, uint32_t bulk_alloc_cnt ) 
{
    return gbpm_resv_rx_hook_g(port, chnl, num_rx_buf, bulk_alloc_cnt);
}

int gbpm_unresv_rx_buf( gbpm_port_t port, uint32_t chnl ) 
{
    return gbpm_unresv_rx_hook_g( port, chnl );
}

uint32_t gbpm_get_total_bufs( void )
{
    return gbpm_get_total_bufs_hook_g();
}

uint32_t gbpm_get_avail_bufs( void )
{
    return gbpm_get_avail_bufs_hook_g();
}

uint32_t gbpm_get_max_dyn_bufs( void )
{
    return gbpm_get_max_dyn_bufs_hook_g();
}

/*
 *------------------------------------------------------------------------------
 * Function     : gbpm_bind
 * Description  : Override default hooks.
 *  gbpm_dyn_buf_lvl: Function pointer to get the buffer level in BPM
 *  gbpm_alloc_mult: Function pointer to allocate multiple buffers
 *  gbpm_free_mult : Function pointer to free multiple buffers
 *  gbpm_alloc     : Function pointer to allocate one buffer
 *  gbpm_free      : Function pointer to free one buffer
 *  gbpm_resv_rx   : Function pointer to reserve buffers
 *  gbpm_uresv_rx  : Function pointer to unreserve buffers
 *  gbpm_get_total_bufs : get total number of bufs BPM manages
 *  gbpm_get_avail_bufs : get current number of free bufs in BPM pool
 *  gbpm_get_max_dyn_bufs : get number of free bufs in BPM pool at init time.
 *------------------------------------------------------------------------------
 */
void gbpm_bind( gbpm_dyn_buf_lvl_hook_t gbpm_dyn_buf_lvl, 
                gbpm_alloc_mult_hook_t gbpm_alloc_mult,
                gbpm_free_mult_hook_t gbpm_free_mult,
                gbpm_alloc_hook_t gbpm_alloc,
                gbpm_free_hook_t gbpm_free,
                gbpm_resv_rx_hook_t gbpm_resv_rx, 
                gbpm_unresv_rx_hook_t gbpm_unresv_rx,
                gbpm_get_total_bufs_hook_t gbpm_get_total_bufs,
                gbpm_get_avail_bufs_hook_t gbpm_get_avail_bufs,
                gbpm_get_max_dyn_bufs_hook_t gbpm_get_max_dyn_bufs )
{
    gbpm_print( "Bind dyn[<%08x>] "
                "mult_alloc[<%08x>] mult_free[<%08x>]" 
                "alloc[<%08x>] free[<%08x>]" 
                "resv_rx[<%08x>] unresv_rx[<%08x>]" 
                "get_total[<%08x>] get_avail[<%08x>]" 
                "get_max_dyn[<%08x>]", 
                (int)gbpm_dyn_buf_lvl, 
                (int)gbpm_alloc_mult, (int)gbpm_free_mult,
                (int)gbpm_alloc, (int)gbpm_free,
                (int)gbpm_resv_rx, (int)gbpm_unresv_rx,
                (int)gbpm_get_total_bufs, (int)gbpm_get_avail_bufs,
                (int)gbpm_get_max_dyn_bufs
                );

    gbpm_dyn_buf_lvl_hook_g = gbpm_dyn_buf_lvl; 
    gbpm_alloc_mult_hook_g = gbpm_alloc_mult;
    gbpm_free_mult_hook_g  = gbpm_free_mult;
    gbpm_alloc_hook_g  = gbpm_alloc;
    gbpm_free_hook_g   = gbpm_free;
    gbpm_resv_rx_hook_g = gbpm_resv_rx; 
    gbpm_unresv_rx_hook_g = gbpm_unresv_rx; 
    gbpm_get_total_bufs_hook_g = gbpm_get_total_bufs;
    gbpm_get_avail_bufs_hook_g = gbpm_get_avail_bufs;
    gbpm_get_max_dyn_bufs_hook_g = gbpm_get_max_dyn_bufs;
}


/*
 *------------------------------------------------------------------------------
 * Function     : gbpm_unbind
 * Description  : use default hooks.
 *------------------------------------------------------------------------------
 */
void gbpm_unbind( void )
{
    gbpm_bind( gbpm_dyn_buf_lvl_stub, 
        gbpm_alloc_mult_stub, gbpm_free_mult_stub, 
        gbpm_alloc_stub, gbpm_free_stub, 
        gbpm_resv_rx_stub, gbpm_unresv_rx_stub,
        gbpm_get_total_bufs_stub, gbpm_get_avail_bufs_stub,
        gbpm_get_max_dyn_bufs_stub
        );
}


/*
 *------------------------------------------------------------------------------
 * Function     : __init_gbpm
 * Description  : Static construction of global buffer pool manager subsystem.
 *------------------------------------------------------------------------------
 */
static int __init __init_gbpm( void )
{
#if defined(CONFIG_BCM_KF_FAP) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* Set up  BPM workqueue - single threaded/high priority */
    gbpm_workqueue = alloc_workqueue("bpm", WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI, 1);
#endif

    gbpm_unbind();

    printk( GBPM_MODNAME GBPM_VER_STR " initialized\n" );
    return 0;
}

subsys_initcall(__init_gbpm);

EXPORT_SYMBOL(gbpm_get_dyn_buf_level); 
EXPORT_SYMBOL(gbpm_alloc_mult_buf);
EXPORT_SYMBOL(gbpm_free_mult_buf);
EXPORT_SYMBOL(gbpm_alloc_buf);
EXPORT_SYMBOL(gbpm_free_buf);
EXPORT_SYMBOL(gbpm_resv_rx_buf);
EXPORT_SYMBOL(gbpm_unresv_rx_buf);
EXPORT_SYMBOL(gbpm_get_total_bufs);
EXPORT_SYMBOL(gbpm_get_avail_bufs);
EXPORT_SYMBOL(gbpm_get_max_dyn_bufs);

EXPORT_SYMBOL(gbpm_bind);
EXPORT_SYMBOL(gbpm_unbind);
#endif /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

#endif
