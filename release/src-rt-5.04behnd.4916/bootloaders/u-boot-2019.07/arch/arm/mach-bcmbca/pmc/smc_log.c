// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019
 * Broadcom Corp
 */
#include <linux/types.h>
#include <linux/compat.h>
#include <asm/io.h>
#include <api_public.h>
#include <string.h>
#include <stdio.h>
#include "itc_rpc.h"
#include <command.h>
#include <common.h>

#define	RAMLOG_ALIGNED(x)		    ALIGN((x), 8)

#define	RAMLOG_FIFO_COUNT		    (32 * 1024)
#define	RAMLOG_MSG_BUFFER_SIZE		RAMLOG_ALIGNED(32 * 1024 * 128)

#define	INVALID_RAMLOG_START_INDEX	0xffffffff
#define	RAMLOG_HEAD_START_INDEX		0xfffffffe
#define	RAMLOG_TAIL_START_INDEX		0xfffffffd
#define	MAX_RAMLOG_LINE_LENGTH		256
#define	RAMLOG_PAGE_SIZE		    4096

#define	CRYPTO_AES_IV_SIZE		    16

#define	BYTES_PER_LINE			32

/**
 * This is an encrypted log buffer header
 */
struct smc_ramlog_hdr {
    uint32_t magic;
    uint32_t length;
    uint8_t aes_iv[CRYPTO_AES_IV_SIZE];
    uint16_t aes_log_saltindex;
    uint8_t aes_log_keyindex;
    uint8_t hdr_ver;
    uint32_t reserved;
};



enum
{
    RPC_FUNCTION_GET_ENCRYPTED_RAMLOG = 9,
    RPC_FUNCTION_ADD_TO_RAMLOG = 10,
    RPC_FUNCTION_CLEAR_RAMLOG = 11,
    RPC_FUNCTION_SET_RAMLOG_LEVEL = 12,
    RPC_FUNCTION_SET_RAMLOG_UART_FLAG = 13,
    RPC_FUNCTION_GET_RAMLOG_COUNT = 14,
    RPC_FUNCTION_GET_RAMLOG_LAST_UPDATE = 15
};

#define RPC_SERVICE_VER_RAMLOG_REQUEST_TIMEOUT     (10) /* sec */

static inline int pmc_svc_request(rpc_msg *msg)
{
    int ret = 0;

    ret = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS, msg, RPC_SERVICE_VER_RAMLOG_REQUEST_TIMEOUT);
#ifdef  DEBUG
    rpc_dump_msg(msg);
#endif  
    if (ret) 
    {
        printf("%s:%d : ERROR: ramlog svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    return ret;
}

uint32_t get_encrypted_ramlog_entries(uint64_t buff_addr, uint32_t maxbufsize)
{
    rpc_msg msg;
    int status;
    uint32_t start_index = 0;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS, RPC_FUNCTION_GET_ENCRYPTED_RAMLOG, 0,
        (uint32_t) (((unsigned long) buff_addr) & 0xffffffff),
        ((maxbufsize & 0xffffff) | (uint32_t) ((((uint64_t) buff_addr) >> 8) & 0xff000000)),
        (start_index & 0xffffff));

    status = pmc_svc_request(&msg);
    if (status < 0)
        return status;

    start_index = (msg.data[0] & 0xffffff);
    if ((start_index & 0x800000) != 0)
        start_index |= 0xff000000;

    return start_index;
}

uint32_t get_ramlog_count(void)
{
    rpc_msg msg;
    int status;

    memset(&msg, 0, sizeof(msg));
    rpc_msg_init(&msg, RPC_SERVICE_SYS, RPC_FUNCTION_GET_RAMLOG_COUNT, 0, 0, 0, 0);
    status = pmc_svc_request(&msg);
    if (status < 0)
        return 0;
    return (uint32_t) (msg.data[0] & 0xffffff);
}

int do_pmc_smclog(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
    uint32_t record_count, buf_size;
    struct smc_ramlog_hdr *header;
    uint8_t *buf;
    uint32_t text_length;
    uint8_t *text_start;
    int i, j;

    record_count = get_ramlog_count();
    buf_size = ALIGN((MAX_RAMLOG_LINE_LENGTH * record_count +
            RAMLOG_PAGE_SIZE), RAMLOG_PAGE_SIZE);
    if (buf_size > RAMLOG_MSG_BUFFER_SIZE)
        buf_size = RAMLOG_MSG_BUFFER_SIZE;
    buf = kmalloc(buf_size, GFP_KERNEL);

    if (!buf)
        return CMD_RET_FAILURE;
    
    header = (struct smc_ramlog_hdr *)buf;

    memset(header, 0, sizeof(struct smc_ramlog_hdr));
    
    get_encrypted_ramlog_entries((uint64_t)buf, buf_size);
    
    text_length = header->length + sizeof(struct smc_ramlog_hdr);
    text_start = buf;

    printf("\n==================== Copy from below line ======================\n");

    for(i = 0, j=1; i < text_length; i++, j++)
    {
        printf("%02x", text_start[i]);
        if(j == BYTES_PER_LINE)
        {
            j = 0;
            printf("\n");
        }
    }

    printf("\n==================== Copy up to above line =====================\n");
    return CMD_RET_SUCCESS;
}

