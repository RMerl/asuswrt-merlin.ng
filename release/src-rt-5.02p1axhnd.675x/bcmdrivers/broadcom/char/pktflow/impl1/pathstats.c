
/*
<:copyright-BRCM:2017:proprietary:standard

   Copyright (c) 2017 Broadcom
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
/*
 *****************************************************************************
 * File Name  : pathstats.c
 *
 * Description: This file contains the Flow Cache Path Stats related functions.

 * Binding with fcache:
 * -------------------------------------
 * See fc_bind_pathstat() *
 * Flow cache defines following hooks to which path stat attaches:
 *      add_flow_fn         <-- path_add_flow()
 *      evict_flow_fn       <-- path_evict_flow()
 *      activate_fhw_fn     <-- path_activate_fhw()
 *      deactivate_fhw_fn   <-- path_deactivate_fhw()
 *      update_stat_fn      <-- path_update_sw_pathstat()
 *      query_dev_stat_fn   <-- path_query_dev_stat()
 *      support_fn          <-- path_support()
 *
 *****************************************************************************
 */


/*----- Includes -----*/

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/blog_net.h>
#include <bcmtypes.h>
#include "fcache.h"
#include "pathstats.h"
#include <linux/jiffies.h>


/*
 *------------------------------------------------------------------------------
 * Path design debugging.
 *------------------------------------------------------------------------------
 */

/* Override global debug and/or assert compilation for Pathstat layer */
#undef PKT_DBG_SUPPORTED
#define CLRsys              CLRb
#define DBGsys              "[PATHSTAT]"
#if defined(CC_CONFIG_FCACHE_PATHSTAT_DBGLVL)
#define PKT_DBG_SUPPORTED
#define PKT_ASSERT_SUPPORTED
static int pktDbgLvl = CC_CONFIG_FCACHE_PATHSTAT_DBGLVL;
#endif
#if defined(CC_CONFIG_PATHSTAT_COLOR)
#define PKT_DBG_COLOR_SUPPORTED
#endif

#include "pktDbg.h"
#if defined(CC_CONFIG_FCACHE_DEBUG)    /* Runtime debug level setting */
int fcachePathstatDebug(int lvl) { dbg_config( lvl ); return lvl; }
#endif

/*----- helper functions in fcachedrv.c -----*/
extern void *fc_kmalloc(size_t size, unsigned int flags);
extern void *fc_kfree(const void *p);


/*----- Defines -----*/
#define MAX_HW_PATH_ENT_NUM     64
#define MAX_SW_PATH_ENT_NUM     128
#define MAX_PATH_BITMAP_NUM     (MAX_SW_PATH_ENT_NUM / 64)
#define MAX_PATH_HASH_DEPTH     8
#define MAX_VT_DEVICE_NUM       128         /* Active vt_device supported at runtime */
#define MAX_PATH_DEV            (MAX_VIRT_DEV + 2)

#define PATH_AGE_REFRESH_INTERVAL   10      /* 10ms (jiffy) refresh */


typedef struct {
    uint64_t    pkt_count;
    uint64_t    pkt_bytes;
    uint64_t    octet_count;
    int         valid;

}  PathStatsEnt_t;


typedef struct {
    Dll_t       node;                   /* First element implements dll  */
    uint64_t    path_key;               /* Hash of connection combo, virt_dev + delta */
    unsigned long hw_pathstat_age;      /* HWACC_Pathstat query timestamp */

    uint16_t    sw_pathstat_idx;        /* path index in SW table */
    uint8_t     hw_pathstat_idx;        /* path index in HWACC table */
    uint8_t     reserv_0;
    uint32_t    ref_count;              /* Number of live flows go thru this path */

    void      * path_dev_p[MAX_PATH_DEV] _FCALIGN_;   /* path connection info */
    int8_t      delta[MAX_PATH_DEV] _FCALIGN_;        /* octet delta info */

}  Path_t;


typedef struct {
    Dll_t       node;                   /* First element implements dll  */
    struct net_device * dev_p;          /* Pointer to network device */

    uint16_t     num_rxpath;             /* # of path under device's Rx dir */
    uint16_t     num_txpath;             /* # of path under device's Tx dir */

    uint64_t    dev_rxpath_bitmap[MAX_PATH_BITMAP_NUM]; /* bitmap of path traversed under device's Rx dir */
    uint64_t    dev_txpath_bitmap[MAX_PATH_BITMAP_NUM]; /* bitmap of path traversed under device's Tx dir */

}  VtDev_t;

#define DEV_BITMAP_ENT_SZ        (sizeof(uint64_t) * 8)
#define PATHSTAT_HW_CNTR_REFRESH_INTERVAL_MSEC    2000    /* 2 second refresh interval */ 

typedef struct {
    int           status;             /* Path learning status */
    int           active_vtdev_num;   /* monitoring active vtdev number */
    int           active_sw_path_num; /* monitoring active path number in SW table */
    int           active_hw_path_num; /* monitoring active path number in HWACC table */

    int           max_sw_path_ent;       /* Max # of configured SW Path entries */
    int           max_hw_path_ent;       /* Max # of configured HW Path entries */
    int           max_dev_ent;           /* Max # of configured Path entries */
    int           max_hwacc_counter;     /* Max # of configured HWACC counter for pathstat */

    Path_t      * path_table _FCALIGN_;            /* Path table */
    PathStatsEnt_t * sw_pathstat_tbl _FCALIGN_;    /* Path stats table for SW */
    PathStatsEnt_t * hw_pathstat_tbl _FCALIGN_;    /* Path stats table for HW accelerator */
    VtDev_t     * xtable _FCALIGN_;                /* Virtual Device management table */

    Dll_t         path_frlist _FCALIGN_;        /* List of free Path entries */
    Dll_t         path_alist;         /* List of active Path entries */

    Dll_t         vtdev_frlist;     /* List of free vtDev entries */
    Dll_t         vtdev_alist;      /* List of vtDev in active */

    uint32_t      cur_refresh_idx;
    uint16_t      slice_idx_step;
    uint16_t      path_num_per_slice;

} __attribute__((aligned(16))) DevPathDb_t;

DevPathDb_t devpathdb; /* Global Device Path Stats context */;


static void _dev_audit( char * fmt );
static int path_flush_hw_stats( uint8_t hw_pathstat_idx );
static void path_push_stats_clear(Path_t * path_p);
int path_exclude_dev(void * net_p);
void path_bind_fc(int enable_flag);

/*
 *------------------------------------------------------------------------------
 * Function     : _path_fhw_live
 * Description  : Test if fhw resource available
 * Design Note  : 
 *------------------------------------------------------------------------------
 */
static inline uint32_t _path_fhw_live(void)
{
    return (devpathdb.max_hwacc_counter > 0) && (devpathdb.hw_pathstat_tbl != NULL);
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_pathstat_print
 * Description  : Dump the path status to a proc fs file.
 * Design Note  : 
 * CAUTION      : Do not use the nflist or brlist global double linked list !
 *------------------------------------------------------------------------------
 */
int path_pathstat_print(char * p, int * index_p, int * linesbudget_p)
{
    int bytes = 0;

    if (*index_p == 0)
    {
        bytes += sprintf( p + bytes, "\nMax # of Path = %d\n", devpathdb.max_sw_path_ent);
        bytes += sprintf( p + bytes, "Max # of HWACC cntr = %d\n", devpathdb.max_hwacc_counter);
        bytes += sprintf( p + bytes, "Active # of Path = %d\n", devpathdb.active_sw_path_num);        
        bytes += sprintf( p + bytes, "Active # of vt device = %d\n", devpathdb.active_vtdev_num);
        bytes += sprintf( p + bytes, "Auto refresh path HW slice_idx_step = %u\n", devpathdb.slice_idx_step);
        bytes += sprintf( p + bytes, "Auto refresh path HW path_num_per_slice = %u\n", devpathdb.path_num_per_slice);
        bytes += sprintf( p + bytes, "Auto refresh cur_refresh_idx = %u\n\n", devpathdb.cur_refresh_idx);
    }

    *index_p += 1;
    *linesbudget_p -= *linesbudget_p;
    return bytes;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_vtdev_stats_print
 * Description  : Dump the vt device status to a proc fs file.
 * Design Note  : 
 * CAUTION      : Do not use the nflist or brlist global double linked list !
 *------------------------------------------------------------------------------
 */
int path_vtdev_stats_print(char * p, int * index_p, int * linesbudget_p)
{
    int bytes = 0, j;
    Dll_t  * list_p;
    VtDev_t * vtdev_p;
    struct net_device * disp_dev_p;

    if (*index_p == 0)
    {
        bytes += sprintf( p + bytes, "\nMax # of vt device = %d\n", devpathdb.max_dev_ent);
        bytes += sprintf( p + bytes, "Active # vt device = %d\n", devpathdb.active_vtdev_num);

        list_p = &devpathdb.vtdev_alist;
        for ( vtdev_p = (VtDev_t*) dll_head_p( list_p );
              !dll_end( list_p, &vtdev_p->node );
              vtdev_p = (VtDev_t*) dll_next_p( &vtdev_p->node ) )
        {
            disp_dev_p = vtdev_p->dev_p;
            bytes += sprintf( p + bytes, "\n\n<Device %s>\n\t RX_path_num=%d, rx_bitmap= ",
                                    disp_dev_p->name, vtdev_p->num_rxpath );

            for ( j=0; j<MAX_PATH_BITMAP_NUM; j++ )
                bytes += sprintf( p + bytes, "0x%016llx, ",
                                    vtdev_p->dev_rxpath_bitmap[j] );

            bytes += sprintf( p + bytes, "\n\t TX_path_num=%d, tx_bitmap= ",
                                    vtdev_p->num_txpath );

            for ( j=0; j<MAX_PATH_BITMAP_NUM; j++ )
                bytes += sprintf( p + bytes, "0x%016llx, ",
                                    vtdev_p->dev_txpath_bitmap[j] );
        }
        bytes += sprintf( p+bytes, "\n" );
    }

    *index_p += 1;
    *linesbudget_p -= *linesbudget_p;
    return bytes;
}

/*
 *----------------------------------------------------------------------------
 * Function     : pathstat_print_by_idx
 * Description  : Dump the contents of the pathstat table to a proc fs file.
 * Design Note  : 
 *----------------------------------------------------------------------------
 */
int pathstat_print_by_idx(char * p, int index)
{
    int bytes = 0, dev_idx, path_idx;
    Path_t * path_p;
    PathStatsEnt_t *sw_stat_p, *hw_stat_p;

    path_p = &devpathdb.path_table[index];
    /* only print non_zero entry */
    if ( path_p->path_key == 0 )
        return bytes;

    bytes += sprintf( p + bytes, "\n<Path_%d> key=0x%llx, active_flows=%d, sw_pathstat_idx=%d, hw_pathstat_idx=%d, hw_pathstat_age=%lu\n",
                    index, path_p->path_key, path_p->ref_count, path_p->sw_pathstat_idx, path_p->hw_pathstat_idx, path_p->hw_pathstat_age );

    path_idx = path_p->sw_pathstat_idx;
    sw_stat_p = &devpathdb.sw_pathstat_tbl[path_idx];
    bytes += sprintf( p + bytes, "\t sw_pkt_count=%llu, sw_pkt_bytes=%llu \n",
                    sw_stat_p->pkt_count, sw_stat_p->pkt_bytes );

    if ( _path_fhw_live() )
    {
        path_flush_hw_stats( path_p->hw_pathstat_idx );
        hw_stat_p = &devpathdb.hw_pathstat_tbl[path_p->hw_pathstat_idx];
        bytes += sprintf( p + bytes, "\t hw_counter_valid=%d, hw_pkt_count=%llu, hw_pkt_bytes=%llu \n",
                        hw_stat_p->valid, hw_stat_p->pkt_count, hw_stat_p->pkt_bytes );
    }
    bytes += sprintf( p + bytes, "\t ");

    for (dev_idx=0; dev_idx<MAX_PATH_DEV; dev_idx++)
    {
        void * detach_dev_p;
        struct net_device * disp_dev_p;
        BlogDir_t dir;

        detach_dev_p = DEVP_DETACH_DIR( path_p->path_dev_p[dev_idx] );
        if ( detach_dev_p == (void *)NULL )
            break;

        dir = IS_RX_DIR( path_p->path_dev_p[dev_idx] ) ? DIR_RX : DIR_TX;
        disp_dev_p = (struct net_device *)detach_dev_p;
        bytes += sprintf( p + bytes, "%s_%s [%d] -> ",
                    disp_dev_p->name, (dir == DIR_RX) ? "rx" : "tx",  path_p->delta[dev_idx] );
    }
    bytes += sprintf( p + bytes, "End\n\n");

    return bytes;
}

int path_idx = 0;
int prev_path_idx = 0;

/*
 *------------------------------------------------------------------------------
 * Function Name: pathstatDrv_seq_xxx
 * Description  : Handler to print the pathstat in seq
 *------------------------------------------------------------------------------
 */
static void *pathstatDrv_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos == 0) 
    {
       path_idx = PATH_IX_INVALID;
    }

    if (path_idx >= devpathdb.max_sw_path_ent) 
    {
        return NULL;
    }

    return &path_idx;
}

static void *pathstatDrv_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    path_idx = *(int *)v;
    path_idx = (path_idx < devpathdb.max_sw_path_ent) ? path_idx + 1 : devpathdb.max_sw_path_ent;

    (*pos)++;

    if (path_idx >= devpathdb.max_sw_path_ent) 
    {
       return NULL;
    }
    return v;
}

static void pathstatDrv_seq_stop(struct seq_file *s, void *v)
{
    return;
}

static int pathstatDrv_seq_show(struct seq_file *s, void *v)
{
    char buffer[900]={0};
    int bytes = 0;
    int ret = 0;

    path_idx = *(int *)v;

    blog_lock();

    bytes = pathstat_print_by_idx(buffer, path_idx);

    blog_unlock();

    // NOTE:- proc calls seq_start() and seq_stop() multiple times with the flw_index
    // at the PAGE boundary.
    // PATH_IX_INVALID is to print headers
    if ((prev_path_idx != path_idx) || (path_idx == PATH_IX_INVALID)) {
        if (bytes) {
            // seq_puts() returns error when the page (PAGE=4KB) is full.
            ret = seq_puts(s, buffer);
        }
    }

    if (!ret)
        prev_path_idx = path_idx;

    return 0;
}


static struct seq_operations pathstatDrv_seq_ops = {
    .start = pathstatDrv_seq_start,
    .next  = pathstatDrv_seq_next,
    .stop  = pathstatDrv_seq_stop,
    .show  = pathstatDrv_seq_show,
};

static int pathstatDrv_procfs_open(struct inode *inode, struct file *file)
{
    struct seq_file *s;
    int ret;

    if ((ret = seq_open(file, &pathstatDrv_seq_ops)) >= 0)
    {
        s = file->private_data;
        s->private = file;
    }
    return ret;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: pathstatDrvPathStatsProcfs
 * Description  : Handler to print the path stats
 *------------------------------------------------------------------------------
 */
static ssize_t pathstatDrvPathStatsProcfs(struct file *file, char __user *page,
        size_t len, loff_t *offset)
{
    int index=0;
    int linesbudget = 10;
    int bytes = 0;

    blog_lock();

    // MOD_INC_USE_COUNT;

    if ( *offset == 0 ) 
    {
        bytes = path_pathstat_print( page, &index, &linesbudget );
        *offset += bytes;
    }

    // MOD_DEC_USE_COUNT;

    blog_unlock();

    return bytes;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: pathstatDrvVtdevStatsProcfs
 * Description  : Handler to print the vt device stats
 *------------------------------------------------------------------------------
 */
static ssize_t pathstatDrvVtdevStatsProcfs(struct file *file, char __user *page,
        size_t len, loff_t *offset)
{
    int index=0;
    int linesbudget = 10;
    int bytes = 0;

    blog_lock();

    // MOD_INC_USE_COUNT;

    if ( *offset == 0 ) 
    {
        bytes = path_vtdev_stats_print( page, &index, &linesbudget );
        *offset += bytes;
    }

    // MOD_DEC_USE_COUNT;

    blog_unlock();

    return bytes;
}

static struct file_operations pathstatDrvProcfs_proc = {
        .open = pathstatDrv_procfs_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = seq_release,
};

static struct file_operations pathstatDrvPathStatsProcfs_proc = {
        .read = pathstatDrvPathStatsProcfs,
};
static struct file_operations pathstatDrvVtdevStatsProcfs_proc = {
        .read = pathstatDrvVtdevStatsProcfs,
};

/*
 *------------------------------------------------------------------------------
 * Function     : path_fhw_alloc
 * Description  : Allocate hw_table based on fhw resource
 * Design Note  : Invoked by fc_bind_fhw() in fcache.c
 *------------------------------------------------------------------------------
 */
int path_fhw_alloc(int hwacc_counter_num)
{
    int allocate_size;

    /* max_hwacc_counter number, read from fhw registeration */
    if ( hwacc_counter_num <= 0 )
        return FCACHE_ERROR;

    if ( hwacc_counter_num > devpathdb.max_hw_path_ent )
        devpathdb.max_hwacc_counter = devpathdb.max_hw_path_ent;
    else
        devpathdb.max_hwacc_counter = hwacc_counter_num;

    allocate_size = sizeof(PathStatsEnt_t) * devpathdb.max_hwacc_counter;
    devpathdb.hw_pathstat_tbl = (PathStatsEnt_t *)kmalloc(allocate_size, GFP_ATOMIC); 

    if (devpathdb.hw_pathstat_tbl == NULL)
    {
        fc_error("Out of memory for HW PathStat Table %d", allocate_size );
        return FCACHE_ERROR;
    }
    memset((void *)devpathdb.hw_pathstat_tbl, 0, allocate_size);

    printk( CLRbold "Pathstats allocated %u bytes" CLRnl , allocate_size);
    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_drv_construct
 * Description  : Construction of device pathstat subsystem.
 * Design Note  : Invoked by pktflow_construct() in fcachedrv.c
 *------------------------------------------------------------------------------
 */
int path_drv_construct(int procfs)
{
    register int id;
    Path_t * path_p;
    VtDev_t * vtdev_p;
    int allocate_size;

    memset( (void*)&devpathdb, 0, sizeof(DevPathDb_t) );

    devpathdb.max_dev_ent       = MAX_VT_DEVICE_NUM;
    devpathdb.max_sw_path_ent   = MAX_SW_PATH_ENT_NUM;
    devpathdb.max_hw_path_ent   = MAX_HW_PATH_ENT_NUM;
    devpathdb.cur_refresh_idx   = 0;

    allocate_size = sizeof(Path_t) * devpathdb.max_sw_path_ent;
    devpathdb.path_table = (Path_t *)fc_kmalloc(allocate_size, FCACHE_ALLOC_TYPE_ATOMIC);

    if (devpathdb.path_table == NULL)
    {
        fc_error("Out of memory for Path Table" );
        return FCACHE_ERROR;
    }
    memset((void *)devpathdb.path_table, 0, allocate_size);

    allocate_size = sizeof(PathStatsEnt_t) * devpathdb.max_sw_path_ent;
    devpathdb.sw_pathstat_tbl = (PathStatsEnt_t *)fc_kmalloc(allocate_size, FCACHE_ALLOC_TYPE_ATOMIC);

    if (devpathdb.sw_pathstat_tbl == NULL)
    {
        fc_error("Out of memory for SW PathStat Table" );
        goto path_construct_sw_path_err;
    }
    memset((void *)devpathdb.sw_pathstat_tbl, 0, allocate_size);

    allocate_size = sizeof(VtDev_t) * devpathdb.max_dev_ent;
    devpathdb.xtable = (VtDev_t *)fc_kmalloc(allocate_size, FCACHE_ALLOC_TYPE_ATOMIC);

    if (devpathdb.xtable == NULL)
    {
        fc_error("Out of memory for VT Device Table" );
        goto path_construct_xtable_err;
    }
    memset((void *)devpathdb.xtable, 0, allocate_size);

    /* Initialize Path lists */
    dll_init( &devpathdb.path_frlist );
    dll_init( &devpathdb.path_alist );

    /* Initialize each path entry and insert into free list */
    for ( id=0; id < devpathdb.max_sw_path_ent; id++ )
    {
        path_p = &devpathdb.path_table[id];
        path_p->sw_pathstat_idx = id;         /* Linear 1:1 mapping for SW pathstat table */

        dll_append( &devpathdb.path_frlist, &path_p->node ); /* Insert into free list */
    }

    /* Initialize VT device lists */
    dll_init( &devpathdb.vtdev_frlist );
    dll_init( &devpathdb.vtdev_alist );

    /* Initialize vt Device entry and insert into free list */
    for ( id=0; id < devpathdb.max_dev_ent; id++ )
    {
        vtdev_p = &devpathdb.xtable[id];
        dll_append( &devpathdb.vtdev_frlist, &vtdev_p->node ); /* Insert into free list */
    }


#if defined(CC_CONFIG_FCACHE_PROCFS)
    if ( procfs == 1 ) {
        struct proc_dir_entry *entry;

        entry = proc_create_data(FCACHE_STATS_PROCFS_DIR_PATH "/path_usage",
            S_IRUGO, NULL, &pathstatDrvPathStatsProcfs_proc, NULL);
        if (!entry)
        {
           print( CLRerr "%s Unable to create proc entry for path stats" CLRnl,
                 __FUNCTION__);
           return FCACHE_ERROR;
        }

        entry = proc_create_data(FCACHE_STATS_PROCFS_DIR_PATH "/vtdev",
            S_IRUGO, NULL, &pathstatDrvVtdevStatsProcfs_proc, NULL);
        if (!entry)
        {
           print( CLRerr "%s Unable to create proc entry for vtdev stats" CLRnl,
                 __FUNCTION__);
           return FCACHE_ERROR;
        }

        entry = proc_create_data(FCACHE_STATS_PROCFS_DIR_PATH "/path", 
            S_IRUGO, NULL, &pathstatDrvProcfs_proc, NULL);
        if (!entry)
        {
           print( CLRerr "%s Unable to create proc entry for path index table" CLRnl,
                 __FUNCTION__);
           return FCACHE_ERROR;
        }
    }
#endif

    /* Enable pathstat driver and bind with fcache */
    path_bind_fc(1);

    printk( CLRbold "Fcache Pathstats created" CLRnl );
    return FCACHE_SUCCESS;

path_construct_xtable_err:
    fc_kfree( devpathdb.sw_pathstat_tbl );

path_construct_sw_path_err:
    fc_kfree( devpathdb.path_table );

    fc_error("%s, initialization failed \n", __FUNCTION__);
    return FCACHE_ERROR;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_drv_remove_proc_entries
 * Description  : Destruction of device pathstat proc filesystem entries.
 * Design Note  : Invoked by pktflow_destruct() in fcachedrv.c
 *------------------------------------------------------------------------------
 */
int path_drv_remove_proc_entries(void)
{
#if defined(CC_CONFIG_FCACHE_PROCFS)
    remove_proc_entry( FCACHE_STATS_PROCFS_DIR_PATH "/path", NULL );
    remove_proc_entry( FCACHE_STATS_PROCFS_DIR_PATH "/path_index", NULL );
    remove_proc_entry( FCACHE_STATS_PROCFS_DIR_PATH "/vtdev", NULL );

    printk( CLRbold "Pathstats proc filesystem entries removed" CLRnl );
#endif

    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_create_path
 * Description  : Create path based on blog connection info and blog_stats_flag
 * Design Note  : follow order of rx_dev -> virt_dev[] -> tx_dev
 *------------------------------------------------------------------------------
 */
static int path_create_path(Blog_t * blog_p, void **path_dev_p, int8_t *delta)
{
    int dev_idx=0, i;
    struct net_device *tmp_dev_p;

    memset( (void *)path_dev_p, 0, sizeof(void *) * MAX_PATH_DEV );
    memset( (void *)delta, 0, sizeof(int8_t) * MAX_PATH_DEV );
    
    /* check edge rx_dev with blog_stats_flag, exclude, include */
    tmp_dev_p = (struct net_device *)blog_p->rx_dev_p;
    dbgl_print(DBG_PDATA, " Rx_edge: %s_rx flag=0x%x put_stats=%p -> ", 
               tmp_dev_p->name, tmp_dev_p->priv_flags, tmp_dev_p->put_stats);
    
    if ( path_exclude_dev(tmp_dev_p) )
        return FCACHE_ERROR;
        
    if ( tmp_dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL )
    {
        dbgl_print(DBG_CTLFL, " %s_rx, stats_included\n", tmp_dev_p->name);
        path_dev_p[0] = DEVP_APPEND_DIR(blog_p->rx_dev_p, DIR_RX);
        dev_idx = 1;
    }
    
    /* fill in the virt_dev */
    for (i = 0; i < MAX_VIRT_DEV; i++)
    {                
        /* virt_dev[] ends with NULL */
        if ( blog_p->virt_dev_p[i] == NULL )
        {
            /* Append edge tx_dev if blog_stats_flag on */
            tmp_dev_p = (struct net_device *)blog_p->tx_dev_p;
            dbgl_print(DBG_PDATA, " Tx_edge: %s_tx flag=0x%x put_stats=%p -> ", 
                       tmp_dev_p->name, tmp_dev_p->priv_flags, tmp_dev_p->put_stats);
            
            if ( path_exclude_dev(tmp_dev_p) )
                return FCACHE_ERROR;
                    
            if ( tmp_dev_p->blog_stats_flags & BLOG_DEV_STAT_FLAG_INCLUDE_ALL )
            {
                dbgl_print(DBG_CTLFL, " %s_tx, stats_included\n", tmp_dev_p->name);
                path_dev_p[dev_idx + i] = DEVP_APPEND_DIR(blog_p->tx_dev_p, DIR_TX);
            }    
            return FCACHE_SUCCESS;
        }

        tmp_dev_p = (struct net_device *)DEVP_DETACH_DIR(blog_p->virt_dev_p[i]);
        dbgl_print(DBG_PDATA, " virt_%d: %s -> ", i, tmp_dev_p->name);
        if ( path_exclude_dev(tmp_dev_p) )
            return FCACHE_ERROR;        
        
        path_dev_p[dev_idx + i] = blog_p->virt_dev_p[i];
        delta[dev_idx + i] = blog_p->delta[i];
    }
    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_build_hash_key
 * Description  : Build path_key with connection and delta info.
 * Design Note  : Invoked by path_add_flow()
 *------------------------------------------------------------------------------
 */
static int path_build_hash_key(Blog_t * blog_p, uint64_t *path_key)
{
    uint8_t dev_idx;
    void * dir_dev_p;
    void      * path_dev_p[MAX_PATH_DEV];
    int8_t      delta[MAX_PATH_DEV]; 

    /* create connection as rx_dev -> virt_dev[] -> tx_dev */
    if ( path_create_path(blog_p, &path_dev_p[0], &delta[0]) == FCACHE_ERROR )
    {
        dbgl_print(DBG_EXTIF, "\n Can't build a valid path\n");
        return FCACHE_ERROR;
    }
    
    /* build hash key */
    *path_key = 0;
    for (dev_idx = 0; dev_idx < MAX_PATH_DEV; dev_idx++)
    {
        dir_dev_p = path_dev_p[dev_idx];        
        /* path_dev_p[] terminate with NULL, break */
        if ( dir_dev_p == (void *)NULL )
            break;
        
        *path_key += (uintptr_t)dir_dev_p;
        *path_key += delta[dev_idx];
    }

    dbgl_print(DBG_CTLFL, "\n = 0x%lx\n", *path_key);
    
    if ( *path_key == PATH_IX_INVALID )
        return FCACHE_ERROR;
    else
        return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_compare_path
 * Description  : Find if connection info matches. This defines an unique path.
 * Design Note  : Invoked by path_add_flow(), path_evict_flow()
 *------------------------------------------------------------------------------
 */
static int path_compare_path(Blog_t * blog_p, Path_t * path_p)
{
    int dev_idx;
    void      * path_dev_p[MAX_PATH_DEV];
    int8_t      delta[MAX_PATH_DEV]; 

    /* follow rx_dev -> virt_dev[] -> tx_dev */
    path_create_path(blog_p, &path_dev_p[0], &delta[0]);
    
    for (dev_idx = 0; dev_idx < MAX_PATH_DEV; dev_idx++)
    {
        /* The last dev_p ends the connection */
        if ( path_dev_p[dev_idx] == NULL )
        {
            return 0;
        }

        if ( path_dev_p[dev_idx] != path_p->path_dev_p[dev_idx] )
            return -1;
        if ( delta[dev_idx] != path_p->delta[dev_idx] )
            return -1;
    }
    return 0;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_record_path
 * Description  : Store the blog connection info. This defines an unique path.
 * Design Note  : Invoked by path_add_flow(), path_evict_flow()
 *------------------------------------------------------------------------------
 */
static void path_record_path(Blog_t * blog_p, Path_t * path_p)
{
    int dev_idx;
    void      * path_dev_p[MAX_PATH_DEV];
    int8_t      delta[MAX_PATH_DEV]; 
    
    path_create_path(blog_p, &path_dev_p[0], &delta[0]);
    
    for (dev_idx = 0; dev_idx < MAX_PATH_DEV; dev_idx++)
    {
        path_p->path_dev_p[dev_idx] = path_dev_p[dev_idx];
        path_p->delta[dev_idx] = delta[dev_idx];
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_hash64shift
 * Description  : Hash function of 64bit integer key
 * Design Note  : Invoked by path_calc_hash()
 *------------------------------------------------------------------------------
 */
static uint64_t path_hash64shift(uint64_t key)
{
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_calc_hash
 * Description  : Calculate a hash_index with path_key.
 * Design Note  : Invoked by path_add_flow()
 *------------------------------------------------------------------------------
 */
static uint8_t path_calc_hash(uint64_t path_key)
{
    uint64_t path_hash;
    uint8_t  hash_idx;

    /* Generate hash with path_key */
    path_hash = path_hash64shift( path_key );
    dbgl_print(DBG_PDATA, "\n = 0x%llx\n", path_hash);

    hash_idx = path_hash % MAX_SW_PATH_ENT_NUM;
    if ( hash_idx == PATH_IX_INVALID )
        hash_idx = PATH_IX_INVALID + 1;
    dbgl_print(DBG_PDATA, "\t calc hash_idx = %d\n", hash_idx);
    return hash_idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_get_new_path_hash_idx
 * Description  : Get a new entry index.
 * Design Note  : Invoked by path_add_flow()
 *------------------------------------------------------------------------------
 */
static int path_get_new_path_hash_idx(uint64_t path_key, uint8_t *hash_idx)
{
    int i;
    Path_t * path_p;
    uint8_t hash_val;

    /* Find a new hash_idx, with depth = 8 */
    hash_val = path_calc_hash( path_key );

    for ( i=0; i<MAX_PATH_HASH_DEPTH; i++ )
    {
        *hash_idx = (hash_val + i) % MAX_SW_PATH_ENT_NUM;
        if ( *hash_idx == PATH_IX_INVALID )
            *hash_idx = PATH_IX_INVALID + 1;

        path_p = &devpathdb.path_table[*hash_idx];
        if ( path_p->path_key == 0 )
        {
            dbg_assertr( (path_p->ref_count == 0), FCACHE_ERROR );
            path_p->path_key = path_key;
            path_p->ref_count = 1;
            dbgl_print(DBG_CTLFL, "%s, key=0x%llx, hash_idx=%d\n", "Find new entry", path_key, *hash_idx);
            return 0;
        }
    }

    /* When collision is more than hash_depth, causing an overflow case. This flow will not be put into pathstat */
    dbgl_print(DBG_CTLFL, "%s, key=0x%llx, hash_idx=%d\n", "Collision and overflow", path_key, *hash_idx);
    return -1;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_hash_search
 * Description  : Search to find path hit or miss.
 * Design Note  : Invoked by path_add_flow()
 *------------------------------------------------------------------------------
 */
static int path_hash_search(Blog_t * blog_p, uint64_t path_key, uint8_t *hash_idx)
{
    int i;
    Path_t * path_p;
    uint8_t hash_val;

    /* Hash search, with depth = 8 */
    hash_val = path_calc_hash( path_key );

    for ( i=0; i<MAX_PATH_HASH_DEPTH; i++ )
    {
        *hash_idx = (hash_val + i) % MAX_SW_PATH_ENT_NUM;
        if ( *hash_idx == PATH_IX_INVALID )
            *hash_idx = PATH_IX_INVALID + 1;

        path_p = &devpathdb.path_table[*hash_idx];
        if ( path_compare_path( blog_p, path_p) == 0 )
        {
            dbgl_print(DBG_CTLFL, "%s, key=0x%llx, hash_idx=%d\n", "Path hit", path_key, *hash_idx);
            return 0;
        }
    }

    dbgl_print(DBG_CTLFL, "%s, key=0x%llx, hash_idx=%d\n", "Path miss", path_key, *hash_idx);
    return -1;
}



/*
 *------------------------------------------------------------------------------
 * Function     : path_find_dev_node
 * Description  : Find a device in the active list.
 *------------------------------------------------------------------------------
 */
static VtDev_t * path_find_dev_node(void * dev_p)
{
    Dll_t  * list_p;
    VtDev_t * vtdev_p;

    list_p = &devpathdb.vtdev_alist;
    /* Traverse the vt device alist */
    for ( vtdev_p = (VtDev_t*) dll_head_p( list_p );
          !dll_end( list_p, &vtdev_p->node );
          vtdev_p = (VtDev_t*) dll_next_p( &vtdev_p->node ) )
    {
        if ( vtdev_p->dev_p == dev_p)
            return vtdev_p;
    }

    return NULL;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_add_dev
 * Description  : Add a device to the active list.
 *------------------------------------------------------------------------------
 */
static int path_add_dev(void * dev_p)
{
    VtDev_t * vtdev_p;
    int i;

    _dev_audit("path_add_dev");

    if ( unlikely( dll_empty( &devpathdb.vtdev_frlist ) ) )
    {
        /* no more path available */
        return FCACHE_ERROR;
    }

    if ( likely( !dll_empty( &devpathdb.vtdev_frlist ) ) )
    {
        vtdev_p = (VtDev_t*) dll_head_p( &devpathdb.vtdev_frlist );
        dll_delete( &vtdev_p->node );
        dll_prepend( &devpathdb.vtdev_alist, &vtdev_p->node );

        vtdev_p->dev_p = dev_p;
        vtdev_p->num_rxpath = 0;
        vtdev_p->num_txpath = 0;
        for (i=0; i<MAX_PATH_BITMAP_NUM; i++)
        {
            vtdev_p->dev_rxpath_bitmap[i] = 0;
            vtdev_p->dev_txpath_bitmap[i] = 0;
        }

        devpathdb.active_vtdev_num += 1;
        dbgl_print(DBG_CTLFL, "%s, device=%p\n", "New VT_device_list created", vtdev_p->dev_p);
    }
    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_mark_vtdev_bitmap
 * Description  : Util function to set or clear bitmap.
 *------------------------------------------------------------------------------
 */
static void path_mark_vtdev_bitmap(VtDev_t * vt_dev_p, BlogDir_t dir, uint16_t bit_pos, uint8_t mbit)
{
    uint16_t row, col;
    row = bit_pos / DEV_BITMAP_ENT_SZ;
    col = bit_pos % DEV_BITMAP_ENT_SZ;

    dbgl_print(DBG_CTLFL, " DIR=%s, pos=%d, row=%d, col=%d, mark=%d\n", 
                     (dir==DIR_TX) ? "TX" : "RX", bit_pos, row, col, mbit);

    if ( mbit )  /* Mark to 1 */
    {
        if ( dir == DIR_TX )
            vt_dev_p->dev_txpath_bitmap[row] |= ( 1ULL << col );
        else
            vt_dev_p->dev_rxpath_bitmap[row] |= ( 1ULL << col );
    }
    else  /* Mark to 0 */
    {
        if ( dir == DIR_TX )
            vt_dev_p->dev_txpath_bitmap[row] &= ~( 1ULL << col );
        else
            vt_dev_p->dev_rxpath_bitmap[row] &= ~( 1ULL << col );
    }
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_check_vtdev_bitmap
 * Description  : Find if bit_pos is set in bitmap.
 *------------------------------------------------------------------------------
 */
static int path_check_vtdev_bitmap(VtDev_t * vt_dev_p, BlogDir_t dir, uint16_t bit_pos)
{
    uint16_t row, col;
    row = bit_pos / DEV_BITMAP_ENT_SZ;
    col = bit_pos % DEV_BITMAP_ENT_SZ;

    /*dbgl_print(DBG_CTLFL, " DIR=%s, pos=%d, r=%d, c=%d, mask=%016llx", 
                     (dir==DIR_TX) ? "TX" : "RX", bit_pos, row, col, (1ULL << col) );
    dbgl_print(DBG_CTLFL, " map_rx=%016llx, map_tx=%016llx ", 
                     vt_dev_p->dev_rxpath_bitmap[row], vt_dev_p->dev_txpath_bitmap[row] );
    */
    if ( dir == DIR_TX )
        return ( vt_dev_p->dev_txpath_bitmap[row] & (1ULL << col) ) ? 1 : 0;
    else
        return ( vt_dev_p->dev_rxpath_bitmap[row] & (1ULL << col) ) ? 1 : 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_add_flow
 * Description  : Learn a path using Blog, invoked on fc_learn.
 *                Check hit or miss. Create new path and inserts into hash table.
 *------------------------------------------------------------------------------
 */
int path_add_flow(Flow_t * flow_p)
{
    uint8_t path_hash_idx = PATH_IX_INVALID;
    uint64_t enroute_pathkey;
    Blog_t * blog_p = flow_p->blog_p;
    uint8_t dev_idx;
    BlogDir_t dir;
    Path_t  * path_p;
    VtDev_t * vt_dev_p = NULL;

    if ( path_build_hash_key(blog_p, &enroute_pathkey) == FCACHE_ERROR )
        return FCACHE_ERROR;    
    dbg_assertr( (enroute_pathkey != PATH_IX_INVALID), FCACHE_ERROR );

    /* Check if it is a new path or existing path */
    if ( path_hash_search(blog_p, enroute_pathkey, &path_hash_idx) == 0 )
    {
        /* Path hit, associate the path with flow, increment active_flow_num */
        path_p = &devpathdb.path_table[path_hash_idx];
        flow_p->sw_pathstat_idx = path_p->sw_pathstat_idx;
        path_p->ref_count += 1;
        return FCACHE_SUCCESS;
    }

    /* create new path */
    dbgl_print(DBG_CTLFL, "%s, 0x%llx\n", "Path miss, insert new entry", enroute_pathkey);
    if ( path_get_new_path_hash_idx(enroute_pathkey, &path_hash_idx) )
    {
        /* collision causing an overflow. This flow will not be put into pathstat. Use default flow based stat method */
        dbg_error("%s\n", "Path table overflow");
        return FCACHE_ERROR;
    }

    dbgl_print(DBG_CTLFL, "%s, =%d\n", "resolved hash_idx", path_hash_idx);
    path_p = &devpathdb.path_table[path_hash_idx];
    dll_delete( &path_p->node );                        /* Remove from free list */
    dll_append( &devpathdb.path_alist, &path_p->node);  /* Add to active list */
    dbgl_print(DBG_CTLFL, "%s, =%p\n", "path_p", path_p);

    /* Associate the path with flow */
    dbg_assertr( (path_p->sw_pathstat_idx < MAX_SW_PATH_ENT_NUM), FCACHE_ERROR );
    path_p->sw_pathstat_idx = path_hash_idx;
    flow_p->sw_pathstat_idx = path_p->sw_pathstat_idx;

    /* Record flow connection and delta info into path */
    path_record_path( blog_p, path_p );

    /* Insert the path into device_path_list */
    for (dev_idx = 0; dev_idx < MAX_PATH_DEV; dev_idx++)
    {
        void *dir_dev_p, *detach_dev_p;

        dir_dev_p = path_p->path_dev_p[dev_idx];

        /* Active connection end with NULL */
        if ( dir_dev_p == (void *)NULL ) {
            break;
        }

        detach_dev_p = DEVP_DETACH_DIR( dir_dev_p );
        dir = IS_RX_DIR( dir_dev_p ) ? DIR_RX : DIR_TX;

        /* Check if VT device in active list */
        vt_dev_p = path_find_dev_node( detach_dev_p );
        if ( unlikely( vt_dev_p == NULL ) )
        {
            /* Locate a new device */
            if ( unlikely( path_add_dev(detach_dev_p) != 0 ) )
                return FCACHE_ERROR;
            else
                vt_dev_p = (VtDev_t*) dll_head_p( &devpathdb.vtdev_alist );

            dbgl_print(DBG_CTLFL, "%s, vt_dev_p=%p\n", "Create new vt dev", vt_dev_p);
        }
        else
            dbgl_print(DBG_CTLFL, "%s, vt_dev_p=%p\n", "Found exist vt dev", vt_dev_p);

        /* Add path leaf under this vt device */
        if ( dir == DIR_TX )
        {
            vt_dev_p->num_txpath += 1;
            path_mark_vtdev_bitmap(vt_dev_p, DIR_TX, path_p->sw_pathstat_idx, 1);
        }
        if ( dir == DIR_RX )
        {
            vt_dev_p->num_rxpath += 1;
            path_mark_vtdev_bitmap(vt_dev_p, DIR_RX, path_p->sw_pathstat_idx, 1);
        }

    }

    devpathdb.active_sw_path_num += 1;
    dbgl_print(DBG_INTIF, "%s, idx=%d\n", "New path added", path_p->sw_pathstat_idx);
    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_activate_fhw
 * Description  : Activates pathstat counter for a flow in hardware HW.
  *------------------------------------------------------------------------------
 */
int path_activate_fhw(Flow_t * flow_p)
{
    Blog_t * blog_p = flow_p->blog_p;
    Path_t * path_p;
    PathStatsEnt_t * hw_stat_p;
    uint16_t path_idx = flow_p->sw_pathstat_idx;
    uint8_t hw_idx, i;

    path_p = &devpathdb.path_table[path_idx];
    dbg_assertr( (path_p != NULL), FCACHE_ERROR );
    dbg_assertr( _path_fhw_live(), FCACHE_ERROR );

    // Check if the sw path has associated with hwacc coutner already
    hw_idx = path_p->hw_pathstat_idx;
    if ( hw_idx != PATH_IX_INVALID )
    {
        hw_stat_p = &devpathdb.hw_pathstat_tbl[hw_idx];
        dbg_assertr( (hw_stat_p->valid != 0), FCACHE_ERROR );

        /* Assign the hwacc counter/path_idx to flow */
        flow_p->hw_pathstat_idx = hw_idx;
        blog_p->hw_pathstat_idx = hw_idx;

        dbgl_print(DBG_INTIF, "%s, idx=%d\n", "Hit and Associate HWACC counter id", flow_p->hw_pathstat_idx);
        return FCACHE_SUCCESS;
    }

    // search in HWACC resource table
    for ( i = PATH_IX_INVALID + 1; i < devpathdb.max_hwacc_counter; i++ )
    {
        hw_stat_p = &devpathdb.hw_pathstat_tbl[i];
        if ( !hw_stat_p->valid )
        {
            /* Assign the hwacc counter/path_idx to flow */
            path_p->hw_pathstat_idx = i;
            flow_p->hw_pathstat_idx = i;
            blog_p->hw_pathstat_idx = i;
            hw_stat_p->valid = 1;
            dbgl_print(DBG_INTIF, "%s, idx=%d\n", "Miss and Activate HWACC pathstat", flow_p->hw_pathstat_idx);
            return FCACHE_SUCCESS;
        }
    }

    // no more hwacc_counter
    dbgl_print(DBG_INTIF, "%s, idx=%d\n", "No HWACC counter available", i);
    return FCACHE_ERROR;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_deactivate_fhw
 * Description  : De-activates pathstat counter for a flow in hardware HW.
  *------------------------------------------------------------------------------
 */
int path_deactivate_fhw(Flow_t * flow_p)
{
    Blog_t * blog_p = flow_p->blog_p;
    PathStatsEnt_t * hw_stat_p;

    dbg_assertr( _path_fhw_live(), FCACHE_ERROR );
    // Flush FHW stats
    hw_stat_p = &devpathdb.hw_pathstat_tbl[ flow_p->hw_pathstat_idx ];
    path_flush_hw_stats( flow_p->hw_pathstat_idx );

    /* Remove hwacc counter assignment */
    flow_p->hw_pathstat_idx = PATH_IX_INVALID;
    blog_p->hw_pathstat_idx = PATH_IX_INVALID;

    dbgl_print(DBG_INTIF, "%s, idx=%d\n", "De-activate HWACC pathstat", flow_p->hw_pathstat_idx);
    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_update_sw_pathstat
 * Description  : Update SW pathstat at each packet recved in fcache, invoked on fc_recv.
 *                FC sw stats update.
 *------------------------------------------------------------------------------
 */
int path_update_sw_pathstat( unsigned long pkt_hit, unsigned long pkt_size, Flow_t * flow_p )
{
    PathStatsEnt_t * sw_stat_p;

    dbg_assertr( (flow_p->sw_pathstat_idx < devpathdb.max_sw_path_ent), FCACHE_ERROR );

    sw_stat_p = &devpathdb.sw_pathstat_tbl[flow_p->sw_pathstat_idx];
    sw_stat_p->pkt_count += pkt_hit;
    sw_stat_p->pkt_bytes += pkt_size;
    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_flush_hw_stats
 * Description  : Flush HW accelerator pathstat into DDR.
 *------------------------------------------------------------------------------
 */
static int path_flush_hw_stats( uint8_t hw_pathstat_idx )
{
    uint32_t hw_hits, hw_bytes;
    PathStatsEnt_t * hw_stat_p;

    dbg_assertr( (hw_pathstat_idx < devpathdb.max_hwacc_counter), FCACHE_ERROR );
    dbg_assertr( _path_fhw_live(), FCACHE_ERROR );

    if ( hw_pathstat_idx == PATH_IX_INVALID )
        return FCACHE_ERROR;

    fc_refresh_pathstat_fhw(hw_pathstat_idx, &hw_hits, &hw_bytes);

    hw_stat_p = &devpathdb.hw_pathstat_tbl[hw_pathstat_idx];
    hw_stat_p->pkt_count += hw_hits; /* cummulative packets */
    hw_stat_p->pkt_bytes += hw_bytes;

    dbgl_print( DBG_PDATA, "HW_PathIdx<%03u> HW_flush hw_path_pkt_hits<%u> hw_path_pkt_bytes<%u>\n",
                hw_pathstat_idx, hw_hits, hw_bytes );
    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_adjust_delta
 * Description  : Calculate adjustment of pathstat octet using delta info
 *------------------------------------------------------------------------------
 */
static int64_t path_adjust_delta( uint64_t hit_count, uint64_t octet_count, int8_t delta )
{
    int64_t  diff = delta * hit_count;

    if ( diff < 0 && octet_count < -diff )
        diff = -octet_count;
    return diff;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_get_path_stats
 * Description  : Get pathstat of a given path
 *------------------------------------------------------------------------------
 */
static void path_get_path_stats( Path_t * path_p, BlogStats_t * sw_stats_p, BlogStats_t * hw_stats_p, 
                                        BlogDir_t dir, int8_t delta )
{
    PathStatsEnt_t *sw_ent_p, *hw_ent_p;
    uint8_t  hw_pathstat_idx;

    /* SW pathstat */
    sw_ent_p   = &devpathdb.sw_pathstat_tbl[ path_p->sw_pathstat_idx ];

    /* Adjust delta on octet */
    sw_ent_p->octet_count  = sw_ent_p->pkt_bytes;
    sw_ent_p->octet_count += path_adjust_delta( sw_ent_p->pkt_count, sw_ent_p->pkt_bytes, delta );

    if ( dir == DIR_RX )
    {
        sw_stats_p->rx_packets += sw_ent_p->pkt_count;
        sw_stats_p->rx_bytes   += sw_ent_p->octet_count;
    }
    else
    {
        sw_stats_p->tx_packets += sw_ent_p->pkt_count;
        sw_stats_p->tx_bytes   += sw_ent_p->octet_count;
    }

    /* if path_fhw not alive, skip */
    if ( !_path_fhw_live() )
        return;

    /* check path age, if more than refresh interval, refresh to get latest HW stats */
    hw_pathstat_idx = path_p->hw_pathstat_idx;
    if ( (hw_pathstat_idx != PATH_IX_INVALID)
        && (jiffies - path_p->hw_pathstat_age > PATH_AGE_REFRESH_INTERVAL) )
    {
        /* Flush the HWACC to get the latest counter stat */
        path_flush_hw_stats( hw_pathstat_idx );
        path_p->hw_pathstat_age = jiffies;
    }

    /* HW pathstat */
    hw_ent_p   = &devpathdb.hw_pathstat_tbl[ hw_pathstat_idx ];

    /* Adjust delta on octet */
    hw_ent_p->octet_count  = hw_ent_p->pkt_bytes;
    hw_ent_p->octet_count += path_adjust_delta( hw_ent_p->pkt_count, hw_ent_p->pkt_bytes, delta );

    if ( dir ==  DIR_RX )
    {
        hw_stats_p->rx_packets += hw_ent_p->pkt_count;
        hw_stats_p->rx_bytes   += hw_ent_p->octet_count;
    }
    else
    {
        hw_stats_p->tx_packets += hw_ent_p->pkt_count;
        hw_stats_p->tx_bytes   += hw_ent_p->octet_count;
    }
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_collect_stats
 * Description  : collect stats on a given { path, vtdev, direction }
 *------------------------------------------------------------------------------
 */
static int path_collect_stats( void * dev_p, BlogStats_t * sw_stats_p, BlogStats_t * hw_stats_p, 
                                        BlogDir_t dir, int path_idx )
{
    int dev_idx, delta;
    Path_t * path_p;
    struct net_device * disp_dev_p;

    path_p = &devpathdb.path_table[path_idx];
    dbg_assertr( (path_p != NULL), FCACHE_ERROR );

    /* Get delta info from this path/device pair */
    delta = 0;
    for (dev_idx = 0; dev_idx < MAX_PATH_DEV; dev_idx++)
    {
        if ( path_p->path_dev_p[dev_idx] == DEVP_APPEND_DIR(dev_p, dir) )
        {
            delta = path_p->delta[dev_idx];
            disp_dev_p = (struct net_device *)dev_p;
            dbgl_print(DBG_CTLFL, "Path_%d: dev_idx=%d, adjust %s_%s delta[%d]\n",
                            path_idx, dev_idx, disp_dev_p->name, (dir==DIR_RX)?"rx":"tx", delta);
            break;
        }
    }
    dbg_assertr( (dev_idx <= MAX_PATH_DEV), FCACHE_ERROR );

    /* Retrieve this path's hw/sw stats, apply dir and delta */
    path_get_path_stats( path_p, sw_stats_p, hw_stats_p, dir, delta);

    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_collect_all_stats
 * Description  : Go through bitmap and collect all pathstat
 *------------------------------------------------------------------------------
 */
static int path_collect_all_stats( VtDev_t * vt_dev_p, BlogStats_t * sw_stats_p, BlogStats_t * hw_stats_p)
{
    int i;

    for (i = 0; i < devpathdb.max_sw_path_ent; i++ )
    {
        if ( path_check_vtdev_bitmap(vt_dev_p, DIR_RX, i) )
        {
            dbgl_print(DBG_CTLFL, "Found Path_%d: DIR_RX\n", i);
            path_collect_stats(vt_dev_p->dev_p, sw_stats_p, hw_stats_p, DIR_RX, i);
        }
        if ( path_check_vtdev_bitmap(vt_dev_p, DIR_TX, i) )
        {
            dbgl_print(DBG_CTLFL, "Found Path_%d: DIR_TX\n", i);
            path_collect_stats(vt_dev_p->dev_p, sw_stats_p, hw_stats_p, DIR_TX, i);
        }
    }
    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_query_dev_stat
 * Description  : Query pathstat of virtual device, invoked on fc_notify.
 *                fc_notify FETCH_NETIF_STATS query active bStat on virtual device.
 *------------------------------------------------------------------------------
 */
int path_query_dev_stat(void * net_p, BlogStats_t * sw_stats_p, BlogStats_t * hw_stats_p)
{
    VtDev_t * vt_dev_p = NULL;

    if ( unlikely( devpathdb.active_sw_path_num == 0 ) )
    {
        dbgl_print(DBG_BKGRD, "%s\n", "No path learned yet");
        return FCACHE_ERROR;
    }

    vt_dev_p = path_find_dev_node( net_p );
    if ( unlikely( vt_dev_p == NULL ) )
    {
        dbgl_print(DBG_BKGRD, "\n%s, %p\n", "Device not found", net_p);
        return FCACHE_ERROR;
    }

    /* Collect SW/HW table tx and rx direction */
    dbgl_print(DBG_CTLFL, "Query dev_%p: \n", net_p);
    path_collect_all_stats( vt_dev_p, sw_stats_p, hw_stats_p );

    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_clear_all_stats
 * Description  : clear pathstat of given device
 *------------------------------------------------------------------------------
 */
static int path_clear_all_stats( VtDev_t * vt_dev_p )
{
    int i;
    Path_t * path_p;

    for (i = 0; i < devpathdb.max_sw_path_ent; i++ )
    {
        if ( path_check_vtdev_bitmap(vt_dev_p, DIR_RX, i) || 
             path_check_vtdev_bitmap(vt_dev_p, DIR_TX, i) )
        {
            dbgl_print(DBG_CTLFL, "Found and Clear Path_%d stats\n", i);
            path_p = &devpathdb.path_table[i];
            dbg_assertr( (path_p != NULL), FCACHE_ERROR );

            path_push_stats_clear( path_p );
        }
    }
    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_clear_dev_stat
 * Description  : Clear pathstat of virtual device, invoked on fc_notify.
 *                fc_notify CLEAR_NETIF_STATS on virtual device.
 *------------------------------------------------------------------------------
 */
int path_clear_dev_stat(void * net_p)
{
    VtDev_t * vt_dev_p = NULL;

    if ( unlikely( devpathdb.active_sw_path_num == 0 ) )
    {
        dbgl_print(DBG_BKGRD, "%s\n", "No path learned yet");
        return FCACHE_ERROR;
    }

    vt_dev_p = path_find_dev_node( net_p );
    if ( unlikely( vt_dev_p == NULL ) )
    {
        dbgl_print(DBG_BKGRD, "\n%s, %p\n", "Device not found", net_p);
        return FCACHE_ERROR;
    }

    /* Clear SW/HW table tx and rx direction */
    dbgl_print(DBG_CTLFL, "Clear dev_%p stats\n", net_p);
    path_clear_all_stats( vt_dev_p );

    return FCACHE_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_exclude_dev
 * Description  : Check device preference with device's priv_flags. 
 *                Certain devices are opt to excluded from pathstat, for example, wlan.
 *                return 0, not_excluded; return non-0, excluded.
 *------------------------------------------------------------------------------
 */
int path_exclude_dev(void * net_p)
{
    struct net_device *dev = (struct net_device *)net_p;
    
    dbgl_print(DBG_CTLFL, " %s flag=0x%x put_stats=%p ", 
               dev->name, dev->priv_flags, dev->put_stats);
        
#if defined(CONFIG_BCM_KF_WL)
    if ( dev->put_stats )
        return -1;
#endif

    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_free_path
 * Description  : Zero out the path node, invoked on path_delete_path.
 *                Prepare to put back the path node to free_list
 *------------------------------------------------------------------------------
 */
void path_free_path(Path_t * path_p)
{
    int dev_idx;
    PathStatsEnt_t *sw_ent_p, *hw_ent_p;
    uint8_t hw_path_idx = path_p->hw_pathstat_idx;
    uint16_t sw_path_idx = path_p->sw_pathstat_idx;

    /* Free up stats table entry first */
    sw_ent_p = &devpathdb.sw_pathstat_tbl[sw_path_idx];
    memset( sw_ent_p, 0, sizeof(PathStatsEnt_t) );
    if ( _path_fhw_live() )
    {  
        hw_ent_p = &devpathdb.hw_pathstat_tbl[hw_path_idx];
        memset( hw_ent_p, 0, sizeof(PathStatsEnt_t) );
    }
    
    /* Free up path key */
    path_p->path_key         = 0;
    path_p->hw_pathstat_age  = 0;
    path_p->ref_count        = 0;
    path_p->sw_pathstat_idx  = 0;
    path_p->hw_pathstat_idx  = 0;

    for ( dev_idx=0; dev_idx<MAX_PATH_DEV; dev_idx++)
    {
        path_p->path_dev_p[dev_idx] = NULL;
        path_p->delta[dev_idx] = 0;
    }
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_delete_dev
 * Description  : Remove a device to the active list.
 *------------------------------------------------------------------------------
 */
static int path_delete_dev(VtDev_t * vtdev_p)
{
    _dev_audit("path_delete_dev");

    if ( unlikely( dll_empty( &devpathdb.vtdev_alist ) ) )
    {
        dbg_error("%s, %s, device=%p\n", __FUNCTION__, "Remove vtdev doesn't exist", vtdev_p->dev_p);
        return FCACHE_ERROR;
    }

    dbgl_print(DBG_CTLFL, "%s, device=%p\n", "Remove vt_device", vtdev_p->dev_p);
    vtdev_p->dev_p = NULL;
    dll_delete( &vtdev_p->node );
    dll_prepend( &devpathdb.vtdev_frlist, &vtdev_p->node );
    devpathdb.active_vtdev_num -= 1;

    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_push_stats_clear
 * Description  : Push the stats back to device, and clear the pathstat entry
 *                Called from eviction or clear_stat
 *------------------------------------------------------------------------------
 */
static void path_push_stats_clear(Path_t * path_p)
{
    int dev_idx;
    PathStatsEnt_t *sw_ent_p, *hw_ent_p;
    uint8_t hw_path_idx = path_p->hw_pathstat_idx;
    uint16_t sw_path_idx = path_p->sw_pathstat_idx;

    /* for each device traversed this path, refresh and push before eviction/clear */
    for (dev_idx=0; dev_idx<MAX_PATH_DEV; dev_idx++)
    {
        void *dir_dev_p, *detach_dev_p;
        struct net_device *test_dev_p;
        BlogStats_t sw_bStats = {0}, hw_bStats = {0};
        BlogDir_t dir;

        dir_dev_p = path_p->path_dev_p[dev_idx];
        if ( dir_dev_p == (void *)NULL ) break;

        detach_dev_p = DEVP_DETACH_DIR( dir_dev_p );
        dir = IS_RX_DIR( dir_dev_p ) ? DIR_RX : DIR_TX;
        test_dev_p = (struct net_device *)detach_dev_p;

        dbgl_print(DBG_CTLFL, " Path_%d evict or clear: dev<%p><%s> Dir<%u><%s> \n",
                                 sw_path_idx, detach_dev_p, test_dev_p->name, dir, (dir==DIR_RX)?"DIR_RX":"DIR_TX" );

        path_collect_stats( detach_dev_p, &sw_bStats, &hw_bStats, dir, sw_path_idx );

        /* Now push the stats into the network device */
        dbgl_print(DBG_CTLFL, " sw_tx_packets=%llu, sw_tx_bytes=%llu, sw_rx_packets=%llu, sw_rx_bytes=%llu,",
                                 sw_bStats.tx_packets, sw_bStats.tx_bytes, sw_bStats.rx_packets, sw_bStats.rx_bytes );
        dbgl_print(DBG_CTLFL, " hw_tx_packets=%llu, hw_tx_bytes=%llu, hw_rx_packets=%llu, hw_rx_bytes=%llu \n",
                                 hw_bStats.tx_packets, hw_bStats.tx_bytes, hw_bStats.rx_packets, hw_bStats.rx_bytes );
        
        blog_request( NETIF_PUT_STATS, detach_dev_p, (uintptr_t)&sw_bStats, (uintptr_t)&hw_bStats );
    }

    // Zero out the stats, leave the pathstat_idx alive
    sw_ent_p = &devpathdb.sw_pathstat_tbl[sw_path_idx];
    sw_ent_p->pkt_bytes   = 0;
    sw_ent_p->pkt_count   = 0;

    if ( _path_fhw_live() )
    {
        hw_ent_p = &devpathdb.hw_pathstat_tbl[hw_path_idx];
        path_flush_hw_stats( hw_path_idx );
        hw_ent_p->pkt_bytes   = 0;
        hw_ent_p->pkt_count   = 0;
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : path_delete_path
 * Description  : Remove a path, invoked on path_evict_flow.
 *                Update and put stat data, then remove from active list
 *------------------------------------------------------------------------------
 */
int path_delete_path(Path_t * path_p)
{
    int idx;
    VtDev_t * vt_dev_p;

    /* Push back the stats before evicition */
    path_push_stats_clear( path_p );

    /* Remove the path from bitmap */
    for (idx=0; idx<MAX_PATH_DEV; idx++)
    {
        void *dir_dev_p, *detach_dev_p;
        BlogDir_t dir;

        dir_dev_p = path_p->path_dev_p[idx];
        if ( dir_dev_p == (void *)NULL ) break;

        detach_dev_p = DEVP_DETACH_DIR( dir_dev_p );
        dir = IS_RX_DIR( dir_dev_p ) ? DIR_RX : DIR_TX;

        /* found the vtdev node, reduce the path_num, */
        vt_dev_p = path_find_dev_node( detach_dev_p );
        if ( unlikely( vt_dev_p == NULL ) )
        {
            return FCACHE_ERROR;
        }

        /* Remove path leaf under this vt device */
        if ( dir == DIR_TX )
        {
            vt_dev_p->num_txpath -= 1;
            path_mark_vtdev_bitmap(vt_dev_p, DIR_TX, path_p->sw_pathstat_idx, 0);
        }
        if ( dir == DIR_RX )
        {
            vt_dev_p->num_rxpath -= 1;
            path_mark_vtdev_bitmap(vt_dev_p, DIR_RX, path_p->sw_pathstat_idx, 0);
        }

        /* Check the vtdev list, if this is the last path, remove the vtdev */
        if ( vt_dev_p->num_txpath==0 && vt_dev_p->num_rxpath==0 )
        {
            /* add assert checking of bitmap all 0 */ 
            path_delete_dev( vt_dev_p );
        }
    }

    /* decrease the active path count */
    devpathdb.active_sw_path_num -= 1;

    /* Delete path, move back to free list */
    path_free_path( path_p );
    dll_delete( &path_p->node );                        /* Remove from active list */
    dll_append( &devpathdb.path_frlist, &path_p->node);  /* Add back to free list */

    _dev_audit("path_delete_path");
    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : path_evict_flow
 * Description  : Evict a path/flow using Blog, invoked on fc_evict.
 *                Remove a flow from path. If path reduces to zero flow, remove path.
 *------------------------------------------------------------------------------
 */
int path_evict_flow(Flow_t * flow_p)
{
    uint8_t path_hash_idx = PATH_IX_INVALID;
    uint64_t enroute_pathkey;
    Blog_t * blog_p = flow_p->blog_p;
    Path_t  * path_p;

    dbgl_print(DBG_CTLFL, " blog_p=%p\n", blog_p);
    if ( path_build_hash_key(blog_p, &enroute_pathkey) == FCACHE_ERROR )
        return FCACHE_ERROR;

    dbg_assertr( (enroute_pathkey != PATH_IX_INVALID), FCACHE_ERROR );

    if ( path_hash_search(blog_p, enroute_pathkey, &path_hash_idx) )
    {
        dbg_error("%s, %s, flow_p=%p\n", __FUNCTION__, "Can't find flow in pathDB", flow_p);
        return FCACHE_ERROR;
    }
    
    path_p = &devpathdb.path_table[path_hash_idx];
    dbg_assertr( (path_p != NULL), FCACHE_ERROR );
    dbg_assertr( (path_p->ref_count != 0), FCACHE_ERROR );
    dbgl_print(DBG_CTLFL, "%s, idx=%d, flow_num=%d\n", "Find and evict flow, path_idx", path_hash_idx, path_p->ref_count);

    path_p->ref_count -= 1;
    if ( path_p->ref_count == 0 )
    {
        /* end of path life cycle */
        if ( path_delete_path( path_p ) )
            return FCACHE_ERROR;
    }

    return FCACHE_SUCCESS;
}


/*
 *------------------------------------------------------------------------------
 * Function     : _dev_list
 * Description  : Audit a Device List
 *------------------------------------------------------------------------------
 */
static int _dev_list( Dll_t * list_p )
{

    int i=0;
    VtDev_t * vtdev_p;
    for ( vtdev_p = (VtDev_t*) dll_head_p( list_p );
          !dll_end( list_p, &vtdev_p->node );
          vtdev_p = (VtDev_t*) dll_next_p( &vtdev_p->node ) )
    {
        i++;
        if ( i > devpathdb.max_dev_ent + 1 ) return -1;
    }
    return 0;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _dev_audit
 * Description  : Audit the Dev lists
 *------------------------------------------------------------------------------
 */
static void _dev_audit( char * fmt )
{
    if (_dev_list(&devpathdb.vtdev_frlist)) fc_error( "%s: failed vt device frlist check", fmt );
    if (_dev_list(&devpathdb.vtdev_alist)) fc_error( "%s: failed vt device alist check", fmt );
}

/*
 *------------------------------------------------------------------------------
 * Function     : pathstat_calc_slice_step 
 * Description  : For certain platform, HW counter has limited range, like 32bits.
 *                To avoid wrap-around, need periodically refresh and accumulate.
 *                slice_idx_step = how many slices to skip over,
 *                path_num_per_slice = how many pathstat to refresh per slice period
 * Design Note  : Invoked by fcachedrv.c
 *                Every time user change of timer_interval triggers re-calculation
 *------------------------------------------------------------------------------
 */
void pathstat_calc_slice_step(uint32_t slice_period_jiffies)
{
    uint32_t num_slices_avail = 0;
    
    /* Set minimum slice period to avoid too frequent refresh */
    if ( slice_period_jiffies == 0 )
        slice_period_jiffies = 1;

    num_slices_avail = msecs_to_jiffies(PATHSTAT_HW_CNTR_REFRESH_INTERVAL_MSEC) / slice_period_jiffies;

    if ( devpathdb.max_sw_path_ent >= num_slices_avail )
    {
        devpathdb.slice_idx_step = 1;
        devpathdb.path_num_per_slice = devpathdb.max_sw_path_ent / num_slices_avail + 1;
        if ( devpathdb.path_num_per_slice > devpathdb.max_sw_path_ent )
            devpathdb.path_num_per_slice = devpathdb.max_sw_path_ent;
    }
    else
    {
        devpathdb.path_num_per_slice = 1;
        devpathdb.slice_idx_step = num_slices_avail / devpathdb.max_sw_path_ent;
    }
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : pathstat_slice 
 * Description  : Periodic slice timer that flushes HW pathstat entries that are active.
 *                slice_idx_step = how many slices to skip over,
 *                path_num_per_slice = how many pathstat to refresh per slice period 
 * Design Note  : Invoked by fcacheDrvTimer() in fcachedrv.c
 *------------------------------------------------------------------------------
 */
void pathstat_slice(uint32_t slice_ix)
{
    Path_t * path_p;
    uint32_t i, cur_idx = devpathdb.cur_refresh_idx;

    /* Refresh only when FHW is ON and pathstat HW available */
    if ( !_path_fhw_live() )
        return;

    /* Refresh only when enough slice_span passed */
    if ( (devpathdb.slice_idx_step > 1) && (slice_ix % devpathdb.slice_idx_step != 0) )
        return;

    for ( i = 0; i < devpathdb.path_num_per_slice; i++ )
    {
        path_p = &devpathdb.path_table[cur_idx++];
        cur_idx %= devpathdb.max_sw_path_ent;

        /* only refresh non_zero entry */
        if ( path_p->path_key )
            path_flush_hw_stats( path_p->hw_pathstat_idx );
    }

    /* Update cur_idx refreshed */
    devpathdb.cur_refresh_idx = cur_idx;

}


/*
 *------------------------------------------------------------------------------
 * Function     : path_bind_fc
 * Description  : Permits manual enabling|disabling of pathstat for FlowCache.
 * Parameter    :
 *                To enable:    enable_flag > 0
 *                To disable:   enable_flag = 0
 *------------------------------------------------------------------------------
 */
void path_bind_fc(int enable_flag)
{
    FcBindPathStatHooks_t path_hooks, *path_hooks_p = &path_hooks;

    if ( enable_flag )
    {
        /* Bind PathStats driver handlers to fc*/
        path_hooks_p->path_fhw_alloc_fn = (HOOK32) path_fhw_alloc;
        path_hooks_p->add_flow_fn       = (HOOKP) path_add_flow;
        path_hooks_p->evict_flow_fn     = (HOOKP) path_evict_flow;
        path_hooks_p->activate_fhw_fn   = (HOOKP) path_activate_fhw;
        path_hooks_p->deactivate_fhw_fn = (HOOKP) path_deactivate_fhw;
        path_hooks_p->update_stat_fn    = (HOOK3PARM) path_update_sw_pathstat;
        path_hooks_p->query_dev_stat_fn = (HOOKP3) path_query_dev_stat;
        path_hooks_p->clear_dev_stat_fn = (HOOKP) path_clear_dev_stat;
        path_hooks_p->exclude_dev_fn    = (HOOKP) path_exclude_dev;
        fc_bind_pathstat( path_hooks_p );

        dbgl_print(DBG_EXTIF, "%s\n", "PathStats driver enabled");
    }
    else
    {
        path_hooks_p->path_fhw_alloc_fn = (HOOK32) NULL;
        path_hooks_p->add_flow_fn       = (HOOKP) NULL;
        path_hooks_p->evict_flow_fn     = (HOOKP) NULL;
        path_hooks_p->activate_fhw_fn   = (HOOKP) NULL;
        path_hooks_p->deactivate_fhw_fn = (HOOKP) NULL;
        path_hooks_p->update_stat_fn    = (HOOK3PARM) NULL;
        path_hooks_p->query_dev_stat_fn = (HOOKP3) NULL;
        path_hooks_p->clear_dev_stat_fn = (HOOKP) NULL;
        path_hooks_p->exclude_dev_fn    = (HOOKP) NULL;

        /* Unbind hooks with fc */
        fc_bind_pathstat( path_hooks_p );
        dbgl_print(DBG_EXTIF, "%s\n", "PathStats driver disabled");
    }
}
