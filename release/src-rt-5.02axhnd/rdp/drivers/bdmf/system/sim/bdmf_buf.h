/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


/*******************************************************************
 * bdmf_buf.h
 *
 * BDMF - user-space sk_buff emulation
 *
 * This file is derived from linux skbuff.h
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *******************************************************************/

#ifndef _BL_BUF_H_
#define _BL_BUF_H_

#define SKB_ALLOC_LEN    2048
#define SKB_ALLOC_ALIGN  256
#define SKB_RESERVE      64
#define SKB_MAGIC        ((' '<<24) | ('s'<<16) | ('k'<<8) | ('b'))

struct sk_buff {
        struct sk_buff *next;
        uint32_t magic;
        uint32_t len;
        uint8_t *data;
        uint8_t *tail;
        uint8_t *end;
        uint8_t cb[16];  /* control buffer - for driver use */
        int priority;
};

struct sk_buff_head {
        struct sk_buff *head;
        struct sk_buff *tail;
};

void dev_kfree_skb(struct sk_buff *skb);

#define dev_kfree_skb_irq dev_kfree_skb

/* returns skb pointer data belongs to or NULL if data doesn't belong to skb */
struct sk_buff *data_to_skb(void *data);

static inline void skb_queue_head_init(struct sk_buff_head *q)
{
        q->head = q->tail = NULL;
}

static inline int skb_queue_empty(const struct sk_buff_head *q)
{
    return (q->head == NULL);
}

static inline void __skb_queue_tail(struct sk_buff_head *q, struct sk_buff *skb)
{
        skb->next = NULL;
        if (q->head)
                q->tail->next = skb;
        else
                q->head = skb;
        q->tail = skb;
}

static inline struct sk_buff *__skb_dequeue(struct sk_buff_head *q)
{
        struct sk_buff *skb = q->head;
        if (!skb)
                return NULL;
        q->head = skb->next;
        return skb;
}


struct sk_buff *dev_alloc_skb(uint32_t length);

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
       skb->tail = skb->data + offset;
}

static inline uint8_t *skb_put(struct sk_buff *skb, uint32_t size)
{
        uint8_t *data=skb->tail;
        assert(data+size <= skb->end);
        skb->tail += size;
        skb->len += size;
        return data;
}

/*
 * Buffer data pointer accessor
 */
static inline  uint8_t *skb_data(struct sk_buff *skb)
{
        return skb->data;
}


/*
 * Buffer length accessor
 */
static inline  uint32_t skb_length(struct sk_buff *skb)
{
        return skb->len;
}


static inline void skb_trim(struct sk_buff *skb, uint32_t size)
{
        assert(skb->len >= size);
        skb->len = size;
}


/**
 *  skb_reserve - adjust headroom
 *  @skb: buffer to alter
 *  @len: bytes to move
 *
 *  Increase the headroom of an empty &sk_buff by reducing the tail
 *  room. This is only allowed for an empty buffer.
 */
static inline void skb_reserve(struct sk_buff *skb, unsigned int len)
{
    skb->data += len;
    skb->tail += len;
}

/**
*    skb_push - add data to the start of a buffer
*    @skb: buffer to use
*    @len: amount of data to add
*
*    This function extends the used data area of the buffer at the buffer
*    start. If this would exceed the total buffer headroom the kernel will
*    panic. A pointer to the first byte of the extra data is returned.
*/
static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
   skb->data -= len;
   skb->len  += len;
   assert(skb->data>=(uint8_t *)skb + sizeof(struct sk_buff));
   return skb->data;
}


/**
*    dev_kfree_skb_data - release skb given the data pointer
*    the data pointer should not be farer than 256 bytes from
*    the beginning of skb buffer it is part of
*    @data: buffer pointer. originally skb_data(skb)
*           where skb is allocated by dev_alloc_skb
*           possibly modified later, but not more that 64 bytes in either direction
*/
void dev_kfree_skb_data(uint8_t *data);



/**
*    skb_make - fill in skb header given data pointer and length
*    the data pointer should not be farer than 256 bytes from
*    the beginning of skb buffer it is part of
*    @data: buffer pointer. originally skb_data(skb)
*           where skb is allocated by dev_alloc_skb
*           possibly modified later, but not more that 64 bytes in either direction
*/
struct sk_buff *skb_make(uint8_t *data, uint32_t len);



/**
*    skb_stat - get skb pool statistics
*    @alloc_count: number of outstanding skb allocations
*    @free_count: number of buffers on skb free list
*/
void skb_stat(uint32_t *alloc_count, uint32_t *free_count);

#endif /* _BL_BUF_H_ */
