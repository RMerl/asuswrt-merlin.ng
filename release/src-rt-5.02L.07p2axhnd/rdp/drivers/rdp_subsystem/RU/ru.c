/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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
/**
 * \brief Register Utilities functional implementation
 */
#include "ru.h"
#include "ru_config.h"
#ifdef UNDEF
#if !SIMULATION_BUILD
#include "bdmf_system.h"
#endif
#endif
#ifdef RU_TO_BBS
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#elif defined(RU_EMULATION)
#include <limits.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <byteswap.h>
#include <errno.h>
#endif


#ifdef RU_TO_BBS
#define MAKE_IP(a,b,c,d)    ((a << 0) | (b << 8) | (c << 16) | (d << 24))
#define SERVER MAKE_IP(10, 64, 10, 75)
#define PORT 8888   //The port on which to send data

uint32_t bbs_send(bbs_op_type op, uint32_t address, uint32_t value)
{
    struct sockaddr_in si_other;
    int s, slen = sizeof(si_other);
    char req[18];
    char resp[8];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        bcmos_printf("socket");
        return 0;
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    si_other.sin_addr.s_addr = SERVER;

    switch (op)
    {
        case BBS_READ:
            req[0] = 'R';
            break;
        case BBS_WRITE:
            req[0] = 'W';
            break;
        case BBS_TERMINATE:
            req[0] = 'T';
            break;
        default:
            bcmos_printf("invalid op\n");
            close(s);
            return 0;
    }
    sprintf(&req[1], "%08X", address);
    sprintf(&req[9], "%08X", value);
    req[17] = '\0';

    //send the message
    if (sendto(s, req, sizeof(req), 0, (struct sockaddr *)&si_other, slen)== -1)
    {
        bcmos_printf("sendto()");
        close(s);
        return 0;
    }

    //try to receive some data, this is a blocking call
    if (recvfrom(s, &resp, sizeof(resp), 0, (struct sockaddr *)&si_other, (socklen_t *)&slen) == -1)
    {
        bcmos_printf("recvfrom()");
        close(s);
        return 0;
    }

    value = strtoul(resp, NULL, 16);
    close(s);
    return value;
}

void *memset_write32 (void *block, int c, uint32_t size)
{
    uint32_t val;
    uint32_t addr;
    val = (c & 0xFF) | ((c & 0xFF) << 8) | ((c & 0xFF) << 16) | ((c & 0xFF) << 24);
    /* round up to the nearest multiple of 4, only until removed when we have the board, then we can use memset() */
    for (addr = (((uint32_t)block) + 3) & (~3) ; addr < (((uint32_t)block) + size); addr += 4)
    {
        WRITE_32(addr, val);
    }
    return NULL;
}
#elif defined(RU_EMULATION)

#define RU_EMULATION_IP_ADDRESS "0.0.0.0"
#define RU_EMULATION_PORT 33333

#define INV_END(x) bswap_32(x)

#define REPORT_ERROR(error,message,args...) printf ( "ERROR [%s:%u]| (error=%d) " message "\n", __FUNCTION__, __LINE__, error, ##args )
#define CHECK_ERROR_AND_RETURN(condition,error) if(condition)\
{\
    REPORT_ERROR(error, "'" #condition "'" );\
    return error;\
}

uint32_t ru_emulation_client_send(reg_io_type io_type, uint32_t address, uint32_t value)
{
    static int sock = -1;
    static int first_time_flag = 1 ;
    struct sockaddr_in sa;
    int err;
    int n;
    reg_io_t reg_io;
#ifdef RU_EMULATION_VERBOSE
    struct timeval tv1;
    struct timeval tv2;
    uint32_t optime;
#endif

    if(io_type >= TYPE_NUM_OF)
        return 0;

    reg_io.type = io_type;
    reg_io.address = address;
    reg_io.value = value;

    if(sock == -1 && first_time_flag)
    {
        first_time_flag = 0;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET; /* Socket type is TCP */
        sa.sin_port = htons(RU_EMULATION_PORT);
        sa.sin_addr.s_addr = inet_addr(RU_EMULATION_IP_ADDRESS);
        CHECK_ERROR_AND_RETURN(sa.sin_addr.s_addr == UINT_MAX, 1);

        sock = socket(AF_INET, SOCK_STREAM, 0);
        CHECK_ERROR_AND_RETURN(sock == -1, errno);

#ifdef RU_EMULATION_VERBOSE
        printf("connecting...\n");
#endif
        err = connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
        if(err)
        {
            REPORT_ERROR(1, "can't connect to simulation server\n" );
            sock = -1;
        }
    }
    if(sock == -1)
        return 0;

    reg_io.value = INV_END(reg_io.value);
#ifdef RU_EMULATION_VERBOSE
    printf("SEND: type=%u, address=0x%x, value=0x%x (%u)\n", reg_io.type, reg_io.address, reg_io.value, reg_io.value);
    gettimeofday(&tv1,NULL);
#endif
    n = send(sock, (void*)&reg_io, sizeof(reg_io), 0);
    CHECK_ERROR_AND_RETURN(n != sizeof(reg_io), errno);

    if(io_type==TYPE_EXIT)
        return 0;

    n = recv(sock, (void*)&reg_io, sizeof(reg_io), MSG_WAITALL);
    CHECK_ERROR_AND_RETURN(n != sizeof(reg_io), errno);

    reg_io.value = INV_END(reg_io.value);
#ifdef RU_EMULATION_VERBOSE
    gettimeofday(&tv2,NULL);
    optime = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
    printf("RECV: type=%u, address=0x%x, value=0x%x (%u) [time=%uus]\n", reg_io.type, reg_io.address, reg_io.value, reg_io.value, optime);
#endif

    if(reg_io.type == TYPE_READ)
        return reg_io.value;
    return 0;
}

void *memset_write32 (void *block, int c, uint32_t size)
{
    uint32_t val;
    uint32_t addr;
    val = (c & 0xFF) | ((c & 0xFF) << 8) | ((c & 0xFF) << 16) | ((c & 0xFF) << 24);
    /* round up to the nearest multiple of 4, only until removed when we have the board, then we can use memset() */
    for (addr = (((uint32_t)block) + 3) & (~3) ; addr < (((uint32_t)block) + size); addr += 4)
    {
        WRITE_32(addr, val);
    }
    return NULL;
}
#else
void *memset_write32 (void *block, int c, uint32_t size)
{
    return memset(block, c, size);
}
#endif



extern const ru_block_rec *RU_ALL_BLOCKS[];

/******************************************************************************
 * Find by name utilities
 ******************************************************************************/
const ru_block_rec *ru_block_name_find(const char *name)
{
    int blk_idx = 0;
    const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];

    while (blk)
    {
        if (strcmp(blk->name, name) == 0)
            break;
        blk = RU_ALL_BLOCKS[++blk_idx];
    }
    return blk;
}


const ru_reg_rec *ru_reg_name_find(ru_block_inst blk_inst,
                                   const ru_block_rec *block,
                                   const char *name)
{
    int i;
    const ru_reg_rec *reg = NULL;

    for (i = 0; i < block->reg_count; i++)
    {
        if (strcmp(block->regs[i]->name, name) == 0)
        {
            reg = block->regs[i];
            break;
        }
    }
    return reg;
}

#if RU_INCLUDE_FIELD_DB
const ru_field_rec *ru_field_name_find(const ru_reg_rec *reg, const char *name)
{
    int i;
    const ru_field_rec *fld = NULL;

    for (i = 0; i < reg->field_count; i++)
    {
        if (strcmp(reg->fields[i]->name, name) == 0)
        {
            fld = reg->fields[i];
            break;
        }
    }
    return fld;
}
#endif


/******************************************************************************
 * Find by register address
 ******************************************************************************/
int ru_block_addr_find(uint32_t addr,
                       ru_block_inst *blk_inst,
                       const ru_block_rec **block)
{
    int rc = -1;
    ru_block_inst blk_idx = 0;
    const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];
    uint32_t reg_idx;

    while (blk)
    {
        for (reg_idx = 0; reg_idx < blk->reg_count; reg_idx++)
        {
            for (*blk_inst = 0;
                 *blk_inst < blk->addr_count;
                 (*blk_inst)++)
            {
                for (reg_idx = 0; reg_idx < blk->reg_count; ++reg_idx)
                {
                    if (blk->regs[reg_idx]->addr + blk->addr[*blk_inst] == addr)
                    {
                        *block = blk;
                        return 0;
                    }
                }
            }
        }
        blk = RU_ALL_BLOCKS[++blk_idx];
    }

    return rc;
}


int ru_reg_addr_find(uint32_t addr,
                     ru_block_inst *blk_inst,
                     const ru_block_rec **block,
                     const ru_reg_rec **reg)
{
    int rc = -1;
    ru_block_inst blk_idx = 0;
    const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];
    uint32_t reg_idx;

    while (blk)
    {
        for (reg_idx = 0; reg_idx < blk->reg_count; reg_idx++)
        {
            for (*blk_inst = 0;
                 *blk_inst < blk->addr_count;
                 (*blk_inst)++)
            {
                for (reg_idx = 0; reg_idx < blk->reg_count; ++reg_idx)
                {
                    if (blk->regs[reg_idx]->addr + blk->addr[*blk_inst] == addr)
                    {
                        *block = blk;
                        *reg = blk->regs[reg_idx];
                        return 0;
                    }
                }
            }
        }
        blk = RU_ALL_BLOCKS[++blk_idx];
    }

    return rc;
}


/******************************************************************************
 * Print parsed register value
 ******************************************************************************/
int ru_reg_print(ru_block_inst blk_inst,
                 const ru_block_rec *block,
                 const ru_reg_rec *reg,
                 uint32_t value)
{
#if RU_INCLUDE_FIELD_DB
    int i;
#endif

    RU_PRINT("%s.%s[%d]@0x%lX+%lX: 0x%08X\n",
             block->name,
             reg->name,
             blk_inst,
             block->addr[blk_inst],
             reg->addr,
             value);
#if RU_INCLUDE_FIELD_DB
    for (i = 0; i < reg->field_count; i++)
    {
        RU_PRINT("    %s[%d:%d]: 0x%X\n",
                 reg->fields[i]->name,
                 reg->fields[i]->shift +
                 reg->fields[i]->bits - 1,
                 reg->fields[i]->shift,
                 ru_field_get(blk_inst, block, reg, reg->fields[i], value));
    }
#endif
    return 0;
}


int ru_reg_addr_print(uint32_t addr, uint32_t value)
{
    ru_block_inst blk_inst;
    const ru_block_rec *block;
    const ru_reg_rec *reg;
    int rc;

    if (!(rc = ru_reg_addr_find(addr, &blk_inst, &block, &reg)))
        ru_reg_print(blk_inst, block, reg, value);

    return rc;
}


int ru_reg_name_print(ru_block_inst blk_inst,
                      const char *bname,
                      const char *rname,
                      uint32_t value)
{
    const ru_block_rec *block;
    const ru_reg_rec *reg;
    int rc = -1;

    block = ru_block_name_find(bname);
    if (block)
        {
        reg = ru_reg_name_find(blk_inst, block, rname);
        if (reg)
            rc = ru_reg_print(blk_inst, block, reg, value);
        }

    return rc;
}

#if RU_FIELD_CHECK_ENABLE
static uint8_t ru_field_check_enable;
#endif

#if RU_OFFLINE_TEST
static uint32_t ru_reg_space[RU_REG_COUNT];
static uint32_t **ru_mem_reg_space[RU_BLK_COUNT];
#endif

void ru_field_bounds_check_enable(int enable)
{
#if RU_FIELD_CHECK_ENABLE
    ru_field_check_enable = enable;
#endif /* RU_FIELD_CHECK_ENABLE */
}


/******************************************************************************
 * Register access functions
 ******************************************************************************/


#if !RU_EXTERNAL_REGISTER_ADDRESSING
int __ru_reg_write(const char *func, const int line,
                 ru_block_inst blk_inst,
                 const ru_block_rec *blk,
                 const ru_reg_rec *reg, uint32_t val)
{
    RU_DBG("RU_REG_WRITE from %s:%d, block:%s, reg:%s, val:%x\n", func, line, blk->name, reg->name, val);
#if RU_OFFLINE_TEST
    ru_reg_space[reg->log_idx] = val;
#else
    WRITE_32(blk->addr[blk_inst] + reg->addr, val);
#endif /* RU_OFFLINE_TEST */
    return 0;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


#if !RU_EXTERNAL_REGISTER_ADDRESSING
uint32_t __ru_reg_read(const char *func, const int line,
                ru_block_inst blk_inst,
                const ru_block_rec *blk,
                const ru_reg_rec *reg)
{
#if RU_OFFLINE_TEST
    return ru_reg_space[reg->log_idx];
#else
    uint32_t rv;
    
    READ_32(blk->addr[blk_inst] + reg->addr, rv);

    RU_DBG("RU_REG_READ from %s:%d, block:%s\n", func, line, blk->name);
    return rv;
#endif /* RU_OFFLINE_TEST */
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */

#if RU_OFFLINE_TEST
int ru_block_idx_find(const char *name)
{
    int blk_idx = 0;
    const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];

    while (blk)
    {
        if (strcmp(blk->name, name) == 0)
            break;
        blk = RU_ALL_BLOCKS[++blk_idx];
    }
    return blk_idx;
}

int ru_reg_idx_find(const ru_block_rec *blk, const char *name)
{
    int i;

    for (i = 0; i < blk->reg_count; i++)
    {
        if (strcmp(blk->regs[i]->name, name) == 0)
            break;
    }
    return i;
}

uint32_t *ru_reg_mem_area(const ru_block_rec *blk, const ru_reg_rec *reg)
{
    int blk_idx = ru_block_idx_find(blk->name);
    int reg_idx = ru_reg_idx_find(blk, reg->name);
    uint32_t **blk_reg_areas = ru_mem_reg_space[blk_idx]; /* Per-register pointer array */
    if (!blk_reg_areas[reg_idx])
    {
        printf("%s: %s.%s No ram. blk_idx=%d reg_idx=%d\n",
            __FUNCTION__, blk->name, reg->name, blk_idx, reg_idx);
    }
    return blk_reg_areas[reg_idx];
}
#endif

#if !RU_EXTERNAL_REGISTER_ADDRESSING
int __ru_reg_ram_write(const char *func, const int line,
                     ru_block_inst blk_inst,
                     ru_ram_addr ram_addr,
                     const ru_block_rec *blk,
                     const ru_reg_rec *reg,
                     uint32_t val)
{
    RU_DBG("RU_REG_RAM_WRITE from %s:%d, block:%s\n", func, line, blk->name);
#if RU_OFFLINE_TEST
    uint32_t *mem_area = ru_reg_mem_area(blk, reg);
    if (mem_area)
    {
        mem_area[reg->ram_count*blk_inst + ram_addr] = val;
    }
    else
    {
        printf("%s->%s: can't write %s.%s. No ram\n", func, __FUNCTION__, blk->name, reg->name);
    }
#else
    WRITE_32(blk->addr[blk_inst] +
            reg->addr +
            (reg->offset * ram_addr), val);
#endif /* RU_OFFLINE_TEST */
    return 0;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


#if !RU_EXTERNAL_REGISTER_ADDRESSING
uint32_t __ru_reg_ram_read(const char *func, const int line,
                         ru_block_inst blk_inst,
                         ru_ram_addr ram_addr,
                         const ru_block_rec *blk,
                         const ru_reg_rec *reg)
{
    uint32_t rv;
#if RU_OFFLINE_TEST
    uint32_t *mem_area = ru_reg_mem_area(blk, reg);
    if (mem_area)
    {
        rv = mem_area[reg->ram_count*blk_inst + ram_addr];
    }
    else
    {
        printf("%s->%s: can't read %s.%s. No ram\n", func, __FUNCTION__, blk->name, reg->name);
        rv = 0;
    }
#else
    READ_32(blk->addr[blk_inst] + reg->addr + (reg->offset * ram_addr), rv);

    RU_DBG("RU_REG_RAM_READ from %s:%d, block:%s\n", func, line, blk->name);
#endif /* RU_OFFLINE_TEST */
    return rv;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


uint32_t __ru_field_set(const char *func, const int line,
                      ru_block_inst blk_inst,
                      const ru_block_rec *blk,
                      const ru_reg_rec *reg,
                      const ru_field_rec *fld,
                      uint32_t reg_val,
                      uint32_t fld_val)
{
#if RU_FIELD_CHECK_ENABLE
    if (ru_field_check_enable)
    {
        if (fld_val > (fld->mask >> fld->shift))
        {
            RU_ASSERT();
            RU_PRINT("ASSERT: Field value out of range. Max %u, attempted %u\n",
                   (fld->mask >> fld->shift), fld_val);
            RU_PRINT("  In field: %s.%s.%s\n", blk->name, reg->name, fld->name);
            return reg_val;
        }
    }
#endif /* RU_FIELD_CHECK_ENABLE */
    RU_DBG("RU_FIELD_SET from %s:%d, field:%s, reg_val:%x, field_val:%x\n", func, line, fld->name, reg_val,
        fld_val);
    return FIELD_SET_(reg_val, fld->mask, fld->shift, fld_val);
}


uint32_t __ru_field_get(const char *func, const int line,
                      ru_block_inst blk_inst,
                      const ru_block_rec *blk,
                      const ru_reg_rec *reg,
                      const ru_field_rec *fld,
                      uint32_t reg_val)
{
    RU_DBG("RU_FIELD_GET from %s:%d, field:%s\n", func, line, fld->name);
    return FIELD_GET_(reg_val, fld->mask, fld->shift);
}


void __ru_field_write(const char *func, const int line,
                    ru_block_inst blk_inst,
                    const ru_block_rec *blk,
                    const ru_reg_rec *reg,
                    const ru_field_rec *fld,
                    uint32_t fld_val)
{
    uint32_t rv;

    rv = __ru_reg_read(func, line, blk_inst, blk, reg);
    rv = __ru_field_set(func, line, blk_inst, blk, reg, fld, rv, fld_val);
    __ru_reg_write(func, line, blk_inst, blk, reg, rv);
}


uint32_t __ru_field_read(const char *func, const int line,
                       ru_block_inst blk_inst,
                       const ru_block_rec *blk,
                       const ru_reg_rec *reg,
                       const ru_field_rec *fld)
{
    uint32_t rv;

    rv = __ru_reg_read(func, line, blk_inst, blk, reg);
    return __ru_field_get(func, line, blk_inst, blk, reg, fld, rv);
}


#ifdef USE_BDMF_SHELL
/*
 * RU CLI
 */

static bdmfmon_handle_t ru_cli_dir;
bdmf_session_handle ru_session;

#if RU_FIELD_CHECK_ENABLE

/* "check" handler
    BDMFMON_MAKE_ENUM( "enable", "Enable check", bdmfmon_enum_bool_table, 0));
 */
static int _ru_cli_check(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int enable = (int)parm[0].value.number;

    ru_field_bounds_check_enable(enable);

    return BDMF_ERR_OK;
}

#endif

/* "parse" handler
    BDMFMON_MAKE_PARM( "address", "Register address", BDMFMON_PARM_HEX, 0),
    BDMFMON_MAKE_PARM( "value", "Register value", BDMFMON_PARM_HEX, 0) );
 */
static int _ru_cli_parse(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t addr = parm[0].value.number;
    uint32_t value = parm[1].value.number;
    int rc;

    ru_session = session;
    rc = ru_reg_addr_print(addr, value);
    ru_session = NULL;

    return rc ? BDMF_ERR_NOENT : BDMF_ERR_OK;
}

#if RU_OFFLINE_TEST

/* Allocate memory for offline RAM register access */
static void ru_offline_ram_regs_alloc(void)
{
    const ru_block_rec *blk;
    ru_reg_rec *reg;
    int i, j;

    for (i = 0; (blk = RU_ALL_BLOCKS[i]) != NULL; i++)
    {
        uint32_t **blk_reg_areas; /* Per-register pointer array */
        blk_reg_areas = ru_mem_reg_space[i] = bdmf_calloc(sizeof(uint32_t *) * blk->reg_count);
        BUG_ON(blk_reg_areas == NULL);
        for (j = 0; j < blk->reg_count; j++)
        {
            reg = (ru_reg_rec *)(long)blk->regs[j];
            if (reg->ram_count)
            {
                blk_reg_areas[j] = bdmf_calloc(sizeof(uint32_t) * reg->ram_count * blk->addr_count);
                BUG_ON(blk_reg_areas[j] == NULL);
            }
        }
    }
}

#endif

void ru_cli_init(bdmfmon_handle_t driver_dir)
{
    if (ru_cli_dir)
        return; /* Just exit if called not the 1st time */

    ru_cli_dir = bdmfmon_dir_add(driver_dir, "ru", "Register access logging, search, etc.", BDMF_ACCESS_ADMIN, NULL);
    if (!ru_cli_dir)
    {
        bdmf_print("Can't create ru CLI directory\n");
        return;
    }

#if RU_FIELD_CHECK_ENABLE
    BDMFMON_MAKE_CMD(ru_cli_dir, "check", "Enable/disable field boundary check", _ru_cli_check,
        BDMFMON_MAKE_PARM_ENUM( "enable", "Enable check", bdmfmon_enum_bool_table, 0) );
#endif

    BDMFMON_MAKE_CMD(ru_cli_dir, "parse", "Parse register value", _ru_cli_parse,
        BDMFMON_MAKE_PARM( "address", "Register address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM( "value", "Register value", BDMFMON_PARM_HEX, 0) );

#if RU_OFFLINE_TEST
    ru_offline_ram_regs_alloc();
#endif
}

void ru_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (ru_cli_dir)
    {
        bdmfmon_token_destroy(ru_cli_dir);
        ru_cli_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

//
//#if RU_TEST_COMPILE_STUB
//int main()
//{
//
//}
//#endif /* RU_TEST_COMPILE_STUB */


/* End of file ru.c */
