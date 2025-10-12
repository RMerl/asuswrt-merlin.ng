/*
 * The wlcsm kernel module
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>
 *
 * $Id: wlcsm_nvram.c 838017 2024-03-19 22:03:08Z $
 */

#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/rbtree.h>
#include <wlcsm_linux.h>
#include <wlcsm_nvram.h>
#include <linux/mutex.h>

#ifdef WLCSM_DEBUG
//unsigned int g_WLCSM_TRACE_LEVEL=WLCSM_TRACE_DBG|WLCSM_TRACE_PKT;           /**< Debug time trace level setting value */
unsigned int g_WLCSM_TRACE_LEVEL=0;           /**< Debug time trace level setting value */
#endif // endif

static DEFINE_SPINLOCK(nvram_lock);
#define WLCSM_LOCK(_lock)       spin_lock_bh(_lock)
#define WLCSM_UNLOCK(_lock)     spin_unlock_bh(_lock)

static int _setcount = 0;			/* Protected by nvram_lock */
static int total_pairs = 0;			/* Protected by nvram_lock */
static int64_t total_bytes = 0;			/* Protected by nvram_lock */

struct rb_root wlcsm_nvram_tree= RB_ROOT;       /**< redblack binary tree root for nvram index */

static void _valuepair_set(t_WLCSM_NAME_VALUEPAIR *v,char *name,char *value,int len)
{
    t_WLCSM_NAME_VALUEPAIR *vp=v;
    /*  init to 0 */
    memset((void *)vp,0,_get_valuepair_total_len(name,value,len));
    /*  set name first */
    vp->len=strlen(name)+1;
    strcpy(vp->value,name);

    /*  move pointer to  */
    vp = _get_valuepair_value(v);
    if(value) {
        vp->len=(len?len:(strlen(value)+1));
        memcpy(&(vp->value),value,vp->len);
    } else
        vp->len=0;
}

static t_WLCSM_NVRAM_TUPLE *_wlcsm_nvram_tuple_search(char *name)
{
    struct rb_node *node = wlcsm_nvram_tree.rb_node;
    int result;

    while (node) {
        t_WLCSM_NVRAM_TUPLE *data = container_of(node, t_WLCSM_NVRAM_TUPLE, node);

        result = strcmp(name, data->tuple->value);

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return data;
    }
    return NULL;
}

static int _wlcsm_nvram_tuple_insert(char *buf, int len)
{
    struct rb_node **new = &(wlcsm_nvram_tree.rb_node), *parent = NULL;
    int result=0;
    t_WLCSM_NVRAM_TUPLE *data=(t_WLCSM_NVRAM_TUPLE *)kmalloc(sizeof(t_WLCSM_NVRAM_TUPLE),GFP_KERNEL);
    if (data == NULL) {
        printk("%s,%d kmalloc sizeof %zu err\n", __FUNCTION__, __LINE__, sizeof(t_WLCSM_NVRAM_TUPLE));
        return WLCSM_GEN_ERR;
    }

    data->tuple=(t_WLCSM_NAME_VALUEPAIR *)kmalloc(len,GFP_KERNEL);
    if (data->tuple == NULL) {
        kfree(data);
        printk("%s,%d kmalloc sizeof %d err\n", __FUNCTION__, __LINE__, len);
        return WLCSM_GEN_ERR;
    }

    memcpy(data->tuple,buf,len);

    /*  Figure out where to put new node */
    while (*new) {
        t_WLCSM_NVRAM_TUPLE  *this = container_of(*new, t_WLCSM_NVRAM_TUPLE, node);
        result = strcmp(data->tuple->value, this->tuple->value);
        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else {
            kfree(data->tuple);
            kfree(data);
            return WLCSM_GEN_ERR;
        }
    }

    /*  Add new node and rebalance tree. */
    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, &wlcsm_nvram_tree);
    return WLCSM_SUCCESS;
}

static int
_wlcsm_nvram_tuple_del(char *name)
{
	t_WLCSM_NVRAM_TUPLE *data = _wlcsm_nvram_tuple_search(name);
	if (data == NULL)
		return WLCSM_GEN_ERR;

	rb_erase(&data->node,&wlcsm_nvram_tree);
	if (data->tuple) {
		int nlen, vlen; /* strlen + 1 of name/value */

		nlen = VALUEPAIR_NAME_LEN(data->tuple);
		vlen = VALUEPAIR_VALUE_LEN(data->tuple);
		total_bytes -= (nlen + vlen);
		total_pairs--;
		kfree(data->tuple);
	}
	kfree(data);
	return WLCSM_SUCCESS;
}

int
wlcsm_nvram_set(char *buf, int len)
{
	int ret = 0;
	int nnlen, nvlen, cvlen; /* strlen + 1 of new name/value and the current value */
	char *name, *new_value;
	t_WLCSM_NVRAM_TUPLE *data = NULL;
	t_WLCSM_NAME_VALUEPAIR *newpair = (t_WLCSM_NAME_VALUEPAIR *)buf;

	if (VALUEPAIR_LEN(newpair) >= WLCSM_NAMEVALUEPAIR_MAX)
		return WLCSM_GEN_ERR;

	name = VALUEPAIR_NAME(newpair);
	new_value = VALUEPAIR_VALUE(newpair);
	nnlen = VALUEPAIR_NAME_LEN(newpair);
	nvlen = VALUEPAIR_VALUE_LEN(newpair);
	WLCSM_LOCK(&nvram_lock); /* aquire the lock before searching */

	data = _wlcsm_nvram_tuple_search(name);
	if (data) {
		char *cur_value = VALUEPAIR_VALUE(data->tuple);

		if (new_value && cur_value && strcmp(new_value, cur_value) == 0) {
			WLCSM_UNLOCK(&nvram_lock);
			return WLCSM_SUCCESS;
		}

		cvlen = VALUEPAIR_VALUE_LEN(data->tuple);
		if (nvlen > cvlen) {
			/* New vlaue len is larger than current value len */
			total_bytes += (nvlen - cvlen);
		} else
			total_bytes -= (cvlen - nvlen);
		if (ksize(data->tuple) >= len) {
			memcpy(data->tuple, buf, len);
			_setcount++;
			WLCSM_UNLOCK(&nvram_lock);
			return WLCSM_SUCCESS;
		}

		rb_erase(&data->node, &wlcsm_nvram_tree);
		kfree(data->tuple);
		kfree(data);
		total_bytes -= (nnlen + cvlen);
		total_pairs--;
	}
	ret = _wlcsm_nvram_tuple_insert(buf, len);
	if (ret == WLCSM_SUCCESS) {
		_setcount++;
		total_bytes += (nnlen + nvlen);
		total_pairs++;
	}
	WLCSM_UNLOCK(&nvram_lock);

	return ret;
}

int
wlcsm_nvram_k_set(char *name, char *value)
{
	int ret, len = _get_valuepair_total_len(name, value, 0);
	char *buf;

	if (len >= WLCSM_NAMEVALUEPAIR_MAX || (buf = kmalloc(len, GFP_ATOMIC)) == NULL) {
		return WLCSM_GEN_ERR;
	}

	_valuepair_set((t_WLCSM_NAME_VALUEPAIR *)buf, name, value, 0);
	ret = wlcsm_nvram_set(buf, len);
	kfree(buf);

	return ret;
}

void
wlcsm_nvram_unset(char *name)
{
	WLCSM_LOCK(&nvram_lock);

	if (_wlcsm_nvram_tuple_del(name) == WLCSM_SUCCESS)
		_setcount++;

	WLCSM_UNLOCK(&nvram_lock);

	return;
}

int wlcsm_nvram_get(char *name,char *result)
{
    int len=0;
    t_WLCSM_NVRAM_TUPLE *data=NULL;

    WLCSM_LOCK(&nvram_lock);
    data=_wlcsm_nvram_tuple_search(name);
    if (data) {
        /* result is copied to result to make sure no correuption after unlock
         * and value is get released by del from other process
         */
        len=VALUEPAIR_LEN(data->tuple);
        memcpy(result,data->tuple,len);
    } else {
        //VALUEPAIR_SET(result,name,NULL);
        len=0;
    }
    WLCSM_UNLOCK(&nvram_lock);
    return len;
}

char *wlcsm_nvram_k_get(char*name)
{
    t_WLCSM_NVRAM_TUPLE *data=NULL;

    WLCSM_LOCK(&nvram_lock);
    data=_wlcsm_nvram_tuple_search(name);
    WLCSM_UNLOCK(&nvram_lock);
    if (data)  {
        return (char *)VALUEPAIR_VALUE(data->tuple);
    } else {
        WLCSM_TRACE(WLCSM_TRACE_DBG," %s get null?\r\n",name);
        return NULL;
    }
}

/* XXX the new wlcsm_nvram_getall with code merged from wlcsm_nvram_getall_pos() from 504L02
 * bcmdrivers/broadcom/char/wlcsm_ext/impl1/src/wlcsm_nvram.c:

int wlcsm_nvram_getall_pos(char *buf,int count,int pos)
{
    struct rb_node *node;
    t_WLCSM_NAME_VALUEPAIR *data;
    char *name,*value;
    int len=0,tcount=0,first=1;
    char tbuf[WLCSM_NAMEVALUEPAIR_MAX];

    WLCSM_NVRAM_LOCK();

    for(node=rb_first(&wlcsm_nvram_tree); node; node=rb_next(node)) {
        data = rb_entry(node,t_WLCSM_NVRAM_TUPLE, node)->tuple;
        name=VALUEPAIR_NAME(data);
        value=VALUEPAIR_VALUE(data);
        len+=sprintf(tbuf,"%s=%s",name,value)+1;
        if(len>pos && tcount<count) {
            if(first)  {
                tcount+=sprintf(buf+tcount,"%s",tbuf+strlen(tbuf)+1-(len-pos))+1;
                first=0;
            }
            else
                tcount+=sprintf(buf+tcount,"%s",tbuf)+1;
        }
        if(tcount>count) break;
    }

    WLCSM_NVRAM_UNLOCK();
    return tcount;
}

 * The last_pos is added to improve the efficiency.
 */
static struct _node_off_info {
	struct rb_node *node;
	int offset;
}	last_pos;

int wlcsm_nvram_getall(char *buf, int count, int pos)
{
	struct rb_node *node;
	t_WLCSM_NAME_VALUEPAIR *data;
	int tcount = 0, off, tbuf_len;
	char *name, *value, *tbuf = kmalloc(WLCSM_NAMEVALUEPAIR_MAX, GFP_KERNEL);

	if (tbuf == NULL)
		return -1;

	WLCSM_LOCK(&nvram_lock);
	if (pos == 0) {
		memset(&last_pos, 0, sizeof(last_pos));
		node = rb_first(&wlcsm_nvram_tree);
		off = 0;
	} else {
		node = last_pos.node;
		off = last_pos.offset;
	}
	for (; node; node = rb_next(node)) {
		data = rb_entry(node, t_WLCSM_NVRAM_TUPLE, node)->tuple;
		name = VALUEPAIR_NAME(data);
		value = VALUEPAIR_VALUE(data);
		tbuf_len = snprintf(tbuf, WLCSM_NAMEVALUEPAIR_MAX, "%s=%s", name, value) + 1;
		if (tcount + tbuf_len <= count) {
			if (tcount == 0) {
				tcount += sprintf(buf, "%s", tbuf + off) + 1;
			} else {
				tcount += sprintf(buf + tcount, "%s", tbuf) + 1;
			}
		} else {
			last_pos.node = node;
			last_pos.offset = off = count - tcount;
			memcpy(buf + tcount, tbuf, off);
			tcount += off;
			break;
		}
	}

	kfree(tbuf);
	WLCSM_UNLOCK(&nvram_lock);
	return tcount;
}

int
wlcsm_nvram_setcount_get(void)
{
	return _setcount;
}

void
wlcsm_nvram_setcount_clear(void)
{
	WLCSM_LOCK(&nvram_lock);
	_setcount = 0;
	WLCSM_UNLOCK(&nvram_lock);
}
