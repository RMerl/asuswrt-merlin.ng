/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _ACCESS_LOGGING_H_
#define _ACCESS_LOGGING_H_

#include <linux/types.h>

typedef enum
{
    ACCESS_LOG_OP_WRITE,
    ACCESS_LOG_OP_MWRITE,
    ACCESS_LOG_OP_MEMSET,
    ACCESS_LOG_OP_MEMSET_32,
    ACCESS_LOG_OP_SLEEP,
    ACCESS_LOG_OP_SET_CORE_ADDR,
    ACCESS_LOG_OP_STOP,
    ACCESS_LOG_OP_MEMSET_ADAPTIVE_32,
    ACCESS_LOG_OP_WRITE_6BYTE_ADDR,
} access_log_op_t;

typedef union os_size_st_union {
    uint32_t value;
    struct {
        uint32_t addr:24;
        uint32_t op_code : 4;
        uint32_t size : 4;
    };
} addr_op_size_st;

typedef struct access_log_tuple
{
    uint32_t addr_op_size;
    uint32_t value;
} access_log_tuple_t;

extern int access_log_restore(const access_log_tuple_t *entry_array);

#endif /* _ACCESS_LOGGING_H_ */

