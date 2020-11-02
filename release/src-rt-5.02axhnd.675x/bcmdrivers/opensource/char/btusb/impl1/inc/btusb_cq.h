/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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
 
#ifndef BTUSB_CQ_H
#define BTUSB_CQ_H

/* for smp_* */
#include <linux/spinlock.h>

struct __btusb_cq {
    unsigned int in;
    unsigned int out;
    unsigned int mask;
};

#define __STRUCT_BTUSB_CQ_COMMON(datatype) \
    union { \
        struct __btusb_cq  cq; \
        datatype          *type; \
        const datatype    *const_type; \
    }

/* contains the power of 2 size check when declaring the buf array */
#define STRUCT_BTUSB_CQ(type, size) \
struct { \
    __STRUCT_BTUSB_CQ_COMMON(type); \
    type buf[((size < 2) || (size & (size - 1))) ? -1 : size]; \
}

#define DECLARE_BTUSB_CQ(queue, type, size) STRUCT_BTUSB_CQ(type, size) queue

#define INIT_BTUSB_CQ(queue) \
(void)({ \
    typeof(&(queue)) __tmp = &(queue); \
    struct __btusb_cq *__cq = &__tmp->cq; \
    __cq->in = 0; \
    __cq->out = 0; \
    __cq->mask = ARRAY_SIZE(__tmp->buf) - 1;\
})

#define btusb_cq_len(queue) \
({ \
    typeof((queue) + 1) __tmpl = (queue); \
    __tmpl->cq.in - __tmpl->cq.out; \
})

#define btusb_cq_is_empty(queue) \
({ \
    typeof((queue) + 1) __tmpq = (queue); \
    __tmpq->cq.in == __tmpq->cq.out; \
})

#define btusb_cq_is_full(queue) \
({ \
        typeof((queue) + 1) __tmpq = (queue); \
        btusb_cq_len(__tmpq) > __tmpq->cq.mask; \
})


#define btusb_cq_put(queue, val) \
({ \
    typeof((queue) + 1) __tmp = (queue); \
    typeof(*__tmp->const_type) __val = (val); \
    unsigned int __ret; \
    struct __btusb_cq *__cq = &__tmp->cq; \
    __ret = !btusb_cq_is_full(__tmp); \
    if (__ret) { \
        __tmp->buf[__cq->in & __cq->mask] = \
                (typeof(*__tmp->type))__val; \
        smp_wmb(); \
        __cq->in++; \
    } \
    __ret; \
})

#define btusb_cq_get(queue, val) \
({ \
    typeof((queue) + 1) __tmp = (queue); \
    typeof(__tmp->type) __val = (val); \
    unsigned int __ret; \
    struct __btusb_cq *__cq = &__tmp->cq; \
    __ret = !btusb_cq_is_empty(__tmp); \
    if (__ret) { \
        *(typeof(__tmp->type))__val = \
            __tmp->buf[__cq->out & __cq->mask]; \
        smp_wmb(); \
        __cq->out++; \
    } \
    __ret; \
})

#endif
