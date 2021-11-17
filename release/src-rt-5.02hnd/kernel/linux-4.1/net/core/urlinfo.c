#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI)
/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#include <linux/netdevice.h>
#include <linux/export.h>
#include <linux/urlinfo.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/bcm_colors.h>

typedef struct {
    UrlInfo_t      * htable[ URLINFO_HTABLE_SIZE ];
    UrlInfo_t        etable[ URLINFO_MAX_ENTRIES ];

    Dll_t         usedlist;         /* List of used urlinfo entries */
    Dll_t         frlist;           /* List of free urlinfo entries */
} __attribute__((aligned(16))) HttpInfo_t;

HttpInfo_t httpInfo;    /* Global URL info context */

#if defined(CC_URLINFO_SUPPORT_DEBUG)
#define urlinfo_print(fmt, arg...)                                           \
    if ( urlinfo_dbg )                                                       \
        printk( CLRc "URLINFO %s :" fmt CLRnl, __FUNCTION__, ##arg )
#define urlinfo_assertv(cond)                                                \
    if ( !cond ) {                                                           \
        printk( CLRerr "URLINFO ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return;                                                              \
    }
#define urlinfo_assertr(cond, rtn)                                           \
    if ( !cond ) {                                                           \
        printk( CLRerr "URLINFO ASSERT %s : " #cond CLRnl, __FUNCTION__ );   \
        return rtn;                                                          \
    }
#define URLINFO_DBG(debug_code)    do { debug_code } while(0)
#else
#define urlinfo_print(fmt, arg...) URLINFO_NULL_STMT
#define urlinfo_assertv(cond) URLINFO_NULL_STMT
#define urlinfo_assertr(cond, rtn) URLINFO_NULL_STMT
#define URLINFO_DBG(debug_code) URLINFO_NULL_STMT
#endif

int urlinfo_dbg = 0;
static struct proc_dir_entry *url_info_entry = NULL;

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_alloc
 * Description  : Allocate a URL info entry
 *------------------------------------------------------------------------------
 */
static UrlInfo_t * urlinfo_alloc( void )
{
    UrlInfo_t * ptr = URLINFO_NULL;

    if (unlikely(dll_empty(&httpInfo.frlist)))
    {
        urlinfo_print("no free entry! No collect now");
        return ptr;
    }

    if (likely(!dll_empty(&httpInfo.frlist)))
    {
        ptr = (UrlInfo_t*)dll_head_p(&httpInfo.frlist);
        dll_delete(&ptr->node);
    }

    urlinfo_print("idx<%u>", ptr->entry.idx);

    return ptr;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _hash
 * Description  : Computes a simple hash from a 32bit value.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _hash( uint32_t hash_val )
{
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
}

/*
 *------------------------------------------------------------------------------
 * Function     : _urlinfo_hash
 * Description  : Compute the hash of a URL
 *------------------------------------------------------------------------------
 */
static inline uint32_t _urlinfo_hash( const UrlInfoEntry_t *url )
{
    uint32_t hashix;

    /* 
     * if url length > 8, take first 8 characters with lenght for hash
     * otherwise, take first 4 characters with length for hash
     */
    if (url->hostlen > 8)
    {
        hashix = _hash( (*((uint32_t *) (&(url->host[0])))) +
                        (*((uint32_t *) (&(url->host[4])))) +
                        url->hostlen );
    }
    else
    {
        hashix = _hash( (*((uint32_t *) (&(url->host[0])))) +
                        url->hostlen );
    }

    return hashix % URLINFO_HTABLE_SIZE;
}

/*
 *------------------------------------------------------------------------------
 * Function     : _urlinfo_match
 * Description  : Checks whether the URL matches.
 *------------------------------------------------------------------------------
 */
static inline uint32_t _urlinfo_match( const UrlInfo_t *ptr,
                                       const UrlInfoEntry_t *url )
{
    return ( (ptr->entry.hostlen == url->hostlen) && 
             !memcmp(ptr->entry.host, url->host, url->hostlen) );
}

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_hashin
 * Description  : Insert a new entry into the urlinfo at a given hash index.
 *------------------------------------------------------------------------------
 */
static void urlinfo_hashin( UrlInfo_t * ptr, uint32_t hashix )
{
    urlinfo_print("enter");

    dll_prepend(&httpInfo.usedlist, &ptr->node);
    ptr->chain_p = httpInfo.htable[ hashix ];  /* Insert into hash table */
    httpInfo.htable[ hashix ] = ptr;
}

static uint32_t urlinfo_new( const UrlInfoEntry_t *url, uint32_t hashix )
{
    UrlInfo_t * ptr;

    urlinfo_print("enter");

    ptr = urlinfo_alloc();
    if ( unlikely(ptr == URLINFO_NULL) )
    {
        urlinfo_print("failed urlinfo_alloc");
        return URLINFO_IX_INVALID;              /* Element table depletion */
    }

    ptr->entry.hostlen = url->hostlen;
    strncpy(ptr->entry.host, url->host, url->hostlen);

    urlinfo_hashin(ptr, hashix);              /* Insert into hash table */

    urlinfo_print("idx<%u>", ptr->entry.idx);

    return ptr->entry.idx;
}

#if 0
/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_free
 * Description  : Free a device info entry
 *------------------------------------------------------------------------------
 */
void urlinfo_free( UrlInfo_t * dev_p )
{
    dev_p->entry.flags = 0;
    dev_p->entry.vendor_id = 0;
    dev_p->entry.os_id = 0;
    dev_p->entry.class_id = 0;
    dev_p->entry.type_id = 0;

    memset(dev_p->mac, 0, ETH_ALEN);

    dll_prepend(&httpInfo.frlist, &dev_p->node);
}

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_unhash
 * Description  : Remove a urlinfo from the device info at a given hash index.
 *------------------------------------------------------------------------------
 */
static void urlinfo_unhash(UrlInfo_t * dev_p, uint32_t hashix)
{
    register UrlInfo_t * hDev_p = httpInfo.htable[hashix];

    if ( unlikely(hDev_p == URLINFO_NULL) )
    {
        urlinfo_print( "ERROR: httpInfo.htable[%u] is NULL", hashix );
        goto urlinfo_notfound;
    }

    if ( likely(hDev_p == dev_p) )                /* At head */
    {
        httpInfo.htable[ hashix ] = dev_p->chain_p;  /* Delete at head */
    }
    else
    {
        uint32_t found = 0;

        /* Traverse the single linked hash collision chain */
        for ( hDev_p = httpInfo.htable[ hashix ];
              likely(hDev_p->chain_p != URLINFO_NULL);
              hDev_p = hDev_p->chain_p )
        {
            if ( hDev_p->chain_p == dev_p )
            {
                hDev_p->chain_p = dev_p->chain_p;
                found = 1;
                break;
            }
        }

        if ( unlikely(found == 0) )
        {
            urlinfo_print( "ERROR:httpInfo.htable[%u] find failure", hashix );
            goto urlinfo_notfound;
        }
    }

    return; /* SUCCESS */

urlinfo_notfound:
    urlinfo_print( "not found: hash<%u>", hashix );
}
#endif

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_lookup
 * Description  : Given a mac, lookup device info.
 *------------------------------------------------------------------------------
 */
uint16_t urlinfo_lookup( const UrlInfoEntry_t *url )
{
    UrlInfo_t * ptr;
    uint16_t idx;
    uint32_t hashix;

    hashix = _urlinfo_hash(url);

    urlinfo_print("hashix<%u> url<%s>", hashix, url->host);

    for ( ptr = httpInfo.htable[ hashix ]; ptr != URLINFO_NULL;
          ptr = ptr->chain_p)
    {
        urlinfo_print("elem: idx<%u> URL<%s>",
                      ptr->entry.idx, ptr->entry.host);

        if (likely( _urlinfo_match(ptr, url) ))
        {
            urlinfo_print("idx<%u>", ptr->entry.idx);
            return ptr->entry.idx;
        }
    }

    /* New URL found, alloc an entry */
    idx = urlinfo_new(url, hashix);

    urlinfo_print("idx<%u>", idx);

    return idx;
}

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_get
 * Description  : Given urlinfo index, return the UrlInfoEntry_t.
 *------------------------------------------------------------------------------
 */
void urlinfo_get( uint16_t idx, UrlInfoEntry_t *entry )
{
    UrlInfo_t * ptr;

    memset(entry, 0, sizeof(UrlInfoEntry_t));

    ptr = &httpInfo.etable[idx];
    entry->idx = ptr->entry.idx;
    strncpy(entry->host, ptr->entry.host, ptr->entry.hostlen);

    urlinfo_print("idx<%u> host<%s>", entry->idx, entry->host);

    return;
}

/*
 *------------------------------------------------------------------------------
 * Function     : urlinfo_set
 * Description  : Given urlinfo index, set the urlinfo_entry.
 *------------------------------------------------------------------------------
 */
void urlinfo_set( const UrlInfoEntry_t *entry )
{
    UrlInfo_t * ptr;

    urlinfo_print("idx<%u> host<%s>", entry->idx, entry->host);

    ptr = &httpInfo.etable[entry->idx];
    ptr->entry.hostlen = entry->hostlen;
    strncpy(ptr->entry.host, entry->host, entry->hostlen);

    return;
}


static void *url_seq_start(struct seq_file *seq, loff_t *pos)
{
    static unsigned long counter = 0;

    rcu_read_lock();
    if (*pos == 0)
        return &counter;
    else
    {
        *pos = 0;
        return NULL;
    }
}

static void *url_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	return NULL;
}

static int url_seq_show(struct seq_file *seq, void *v)
{
    Dll_t  *tmp_p;
    Dll_t  *list_p;
    UrlInfo_t *elem_p;
	int ret = 0;

    urlinfo_print("enter");

   	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "URL list\n");
		return ret;
	}

    list_p = &httpInfo.usedlist;

    if (!dll_empty(list_p))
    {
        dll_for_each(tmp_p, list_p) 
        {
            elem_p = (UrlInfo_t *)tmp_p;
            seq_printf(seq, "%s\n", elem_p->entry.host);
        }
    }

	return 0;
}

static void url_seq_stop(struct seq_file *seq, void *v)
{
	rcu_read_unlock();
}

static struct seq_operations url_seq_ops = {
	.start = url_seq_start,
	.next  = url_seq_next,
	.stop  = url_seq_stop,
	.show  = url_seq_show,
};

static int url_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &url_seq_ops);
}

static struct file_operations url_info_proc_fops = {
	.owner = THIS_MODULE,
	.open  = url_seq_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};


int urlinfo_init( void )
{
    register int id;
    UrlInfo_t * ptr;

    memset( (void*)&httpInfo, 0, sizeof(HttpInfo_t) );

    /* Initialize list */
    dll_init( &httpInfo.usedlist );
    dll_init( &httpInfo.frlist );

    /* Initialize each urlinfo entry and insert into free list */
    for ( id=URLINFO_IX_INVALID; id < URLINFO_MAX_ENTRIES; id++ )
    {
        ptr = &httpInfo.etable[id];
        ptr->entry.idx = id;

        if ( unlikely(id == URLINFO_IX_INVALID) )
            continue;           /* Exclude this entry from the free list */

        dll_append(&httpInfo.frlist, &ptr->node);/* Insert into free list */
    }

    url_info_entry = proc_create("url_info", 0, init_net.proc_net,
			   &url_info_proc_fops);

    URLINFO_DBG( printk( "URLINFO urlinfo_dbg<0x%08x> = %d\n"
                         "%d Available entries\n",
                         (int)&urlinfo_dbg, urlinfo_dbg,
                         URLINFO_MAX_ENTRIES-1 ); );
    
    return 0;
}

EXPORT_SYMBOL(urlinfo_init);
EXPORT_SYMBOL(urlinfo_lookup);
EXPORT_SYMBOL(urlinfo_get);
EXPORT_SYMBOL(urlinfo_set);
#endif /* if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BRCM_DPI) */
