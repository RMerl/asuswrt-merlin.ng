/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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

#include "rdp_common.h"
#include "rdd_data_structures_auto.h"
#include "access_macros.h"

#ifdef USE_BDMF_SHELL

struct bdmfmon_enum_val tbl_idx_enum_table[] = {
    {"TBL_0", NATC_TBL0_ID},
    {"TBL_1", NATC_TBL1_ID},
    {"TBL_2", NATC_TBL2_ID},
    {"TBL_3", NATC_TBL3_ID},
    {"TBL_4", NATC_TBL4_ID},
    {"TBL_5", NATC_TBL5_ID},
    {"TBL_6", NATC_TBL6_ID},
    {"TBL_7", NATC_TBL7_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val eng_idx_enum_table[] = {
    {"ENG_0", NATC_ENG0_ID},
    {"ENG_1", NATC_ENG1_ID},
    {"ENG_2", NATC_ENG2_ID},
    {"ENG_3", NATC_ENG3_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val ubus_mstr_id_enum_table[] = {
    {"UBUS_MSTR_0", UBUS_MSTR0_ID},
    {"UBUS_MSTR_1", UBUS_MSTR1_ID},
    {NULL, 0},
};

struct bdmfmon_enum_val channel_id_enum_table[] = {
    {"CHANNEL_0", CHANNEL0_ID},
    {"CHANNEL_1", CHANNEL1_ID},
    {"CHANNEL_2", CHANNEL2_ID},
    {"CHANNEL_3", CHANNEL3_ID},
    {"CHANNEL_4", CHANNEL4_ID},
    {"CHANNEL_5", CHANNEL5_ID},
    {"CHANNEL_6", CHANNEL6_ID},
    {"CHANNEL_7", CHANNEL7_ID},
    {NULL, 0},
};

#if !defined(_CFE_) && !defined(RDP_SIM) && defined(DUAL_ISSUE)

typedef union memcpy_buf_u
{
    uint64_t u64;
    struct
    {
        uint32_t u32l;
        uint32_t u32h;
    };
} memcpy_buf_t;


static const uintptr_t U16_ALIGNMENT_MASK = (uintptr_t)(sizeof(uint16_t) - 1);
static const uintptr_t U32_ALIGNMENT_MASK = (uintptr_t)(sizeof(uint32_t) - 1);
static const uintptr_t U64_ALIGNMENT_MASK = (uintptr_t)(sizeof(uint64_t) - 1);
static const uint BITS__PER__BYTE = 8;


/* _xrdp__memcpy optimized for the common case where alignment of source and destination addresses is the same */
static inline void _xrdp__memcpy_equal_alignment(volatile void *dst, const volatile void *src, size_t len)
{
    volatile uint8_t *u8dst_ptr = (volatile uint8_t *)dst;
    const volatile uint8_t *u8src_ptr = (const volatile uint8_t *)src;

    if (((uintptr_t)u8src_ptr & U16_ALIGNMENT_MASK) && (len > 0))
    {
        *u8dst_ptr = *u8src_ptr;
        ++u8dst_ptr;
        ++u8src_ptr;
        len--;
    }

    if (((uintptr_t)u8src_ptr & U32_ALIGNMENT_MASK) && (len >= sizeof(uint16_t))) 
    {
        *(uint16_t *)u8dst_ptr = *(uint16_t *)u8src_ptr;
        u8dst_ptr += sizeof(uint16_t);
        u8src_ptr += sizeof(uint16_t);
        len -= sizeof(uint16_t);
    }

    {
        uint32_t       *u32dst_ptr = (uint32_t *)u8dst_ptr;
        uint32_t const *u32src_ptr = (uint32_t const *)u8src_ptr;

        while (len >= sizeof(uint32_t)) 
        {
            *u32dst_ptr++ = *u32src_ptr++;
            len -= sizeof(uint32_t);
        }
        u8dst_ptr = (uint8_t *)u32dst_ptr;
        u8src_ptr = (uint8_t const *)u32src_ptr;
    }

    if (len >= sizeof(uint16_t)) 
    {
        *(uint16_t *)u8dst_ptr = *(uint16_t *)u8src_ptr;
        u8dst_ptr += sizeof(uint16_t);
        u8src_ptr += sizeof(uint16_t);
        len -= sizeof(uint16_t);
    }

    if (len > 0)
    {
        *u8dst_ptr = *u8src_ptr;
        ++u8dst_ptr;
        ++u8src_ptr;
        len--;
    }
}

/* Writes naturally aligned transactions (1,2,or 4 bytes) to *p_dst_ptr. Returns numbers of bytes written. */
static inline int _xrdp__memcpy_helper_aligned_write(volatile uint8_t **p_dst_ptr, size_t max_len, uint32_t data)
{
    int bytes_written = 0;
    uint dst_alignment = (uint)((uintptr_t)(*p_dst_ptr) & U32_ALIGNMENT_MASK);

    switch (dst_alignment)
    {
    case 0: /* Aligned on uint32 */
        {
            if (max_len >= sizeof(uint32_t))
            {
                *((volatile uint32_t *)(*p_dst_ptr)) = (uint32_t)data;
                *p_dst_ptr += sizeof(uint32_t);
                bytes_written += sizeof(uint32_t);
            }
            else
            {
                if (max_len >= sizeof(uint16_t))
                {
                    *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                    *p_dst_ptr += sizeof(uint16_t);
                    max_len -= sizeof(uint16_t);                                 
                    data >>= sizeof(uint16_t)*BITS__PER__BYTE;      
                    bytes_written += sizeof(uint16_t);
                }
                if (max_len == sizeof(uint8_t)) /* here max_len is either 1 or 0 */
                {
                    *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                    bytes_written += sizeof(uint8_t);
                }
            }
        }
        break;
    case 2: /* p_dst_ptr points to byte 2 in the 32bit word */
        {
            if (max_len >= sizeof(uint16_t))
            {
                *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                *p_dst_ptr += sizeof(uint16_t);
                max_len -= sizeof(uint16_t);                                 
                data >>= sizeof(uint16_t)*BITS__PER__BYTE;      
                bytes_written += sizeof(uint16_t);
    
                if (max_len == sizeof(uint16_t)) /* last two bytes of the memcpy operation. */
                {
                    *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                    bytes_written += sizeof(uint16_t);
                }
                else
                if (max_len == sizeof(uint8_t)) /* last byte of the memcpy operation. */
                {
                    *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                    bytes_written += sizeof(uint8_t);
                }
            }
            else
            if (max_len == sizeof(uint8_t))     /* here max_len is either 1 or 0 */
            {
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                *p_dst_ptr += sizeof(uint8_t);
                max_len -= sizeof(uint8_t);                                 
                data >>= sizeof(uint8_t)*BITS__PER__BYTE;      
                bytes_written += sizeof(uint8_t);
            }
        }
        break;
    case 1: /* p_dst_ptr points to byte 1 in the 32bit word */
        {
            if (max_len >= sizeof(uint8_t))
            {
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                *p_dst_ptr += sizeof(uint8_t);
                max_len -= sizeof(uint8_t);                                 
                data >>= sizeof(uint8_t)*BITS__PER__BYTE;      
                bytes_written += sizeof(uint8_t);
            }
            
            if (max_len >= sizeof(uint16_t))
            {
                *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                *p_dst_ptr += sizeof(uint16_t);
                max_len -= sizeof(uint16_t);                                 
                data >>= sizeof(uint16_t)*BITS__PER__BYTE;      
                bytes_written += sizeof(uint16_t);
            }
    
            if (max_len == sizeof(uint8_t)) /* last byte of the memcpy operation. */
            {
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                bytes_written += sizeof(uint8_t);
            }
        }
        break;
    case 3: /* p_dst_ptr points to byte 3 in the 32bit word */
        {
            if (max_len >= sizeof(uint8_t))
            {
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                *p_dst_ptr += sizeof(uint8_t);
                max_len -= sizeof(uint8_t);                                 
                data >>= sizeof(uint8_t)*BITS__PER__BYTE;      
                bytes_written += sizeof(uint8_t);
            }
    
            if (max_len == (sizeof(uint16_t)+sizeof(uint8_t))) /* last three bytes of the memcpy operation. */
            {
                *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                *p_dst_ptr += sizeof(uint16_t);                     
                max_len -= sizeof(uint16_t);                                 
                data >>= sizeof(uint16_t)*BITS__PER__BYTE;      
    
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                bytes_written += (sizeof(uint16_t) + sizeof(uint8_t));
            }
            else
            if (max_len == sizeof(uint16_t)) /* last two bytes of the memcpy operation. */
            {
                *((volatile uint16_t *)(*p_dst_ptr)) = (uint16_t)data;
                bytes_written += sizeof(uint16_t);
            }
            else
            if (max_len == sizeof(uint8_t)) /* last byte of the memcpy operation. */
            {
                *((volatile uint8_t *)(*p_dst_ptr)) = (uint8_t)data;
                bytes_written += sizeof(uint8_t);
            }
        }
        break;
    }
    
    return bytes_written;
}

static void _xrdp__memcpy_helper_unaligned_32_write(volatile uint8_t **p_dst_ptr, size_t *p_len, memcpy_buf_t *p_wr_mem_buf, int *p_wr_mem_buf_sz, uint32_t data)
{
    int bytes_written;

    /* push data into *p_wr_mem_buf */
    p_wr_mem_buf->u32h = data;
    p_wr_mem_buf->u64 >>= (sizeof(uint32_t) - *p_wr_mem_buf_sz)*BITS__PER__BYTE;
    *p_wr_mem_buf_sz += sizeof(uint32_t);
    /* at this point p_wr_mem_buf->u32l is filled with data. p_wr_mem_buf->u32h may be partially filled with data */

    bytes_written = _xrdp__memcpy_helper_aligned_write(p_dst_ptr, *p_len, p_wr_mem_buf->u32l);
    
    if (*p_wr_mem_buf_sz > sizeof(uint32_t))
    {
        p_wr_mem_buf->u64 >>= (*p_wr_mem_buf_sz - sizeof(uint32_t))*BITS__PER__BYTE;
    }

    *p_wr_mem_buf_sz -= bytes_written; /* wr_mem_buf states. VV represents valid byte value. xx represents empty byte.
                                          xx xx xx xx  xx xx xx xx    *p_wr_mem_buf_sz ==0
                                          xx xx xx xx  VV xx xx xx    *p_wr_mem_buf_sz ==1
                                          xx xx xx xx  VV VV xx xx    *p_wr_mem_buf_sz ==2
                                          xx xx xx xx  VV VV VV xx    *p_wr_mem_buf_sz ==3
                                          
                                          xx xx xx xx  VV VV VV VV    *p_wr_mem_buf_sz ==4
                                          xx xx xx VV  VV VV VV VV    *p_wr_mem_buf_sz ==5
                                          xx xx VV VV  VV VV VV VV    *p_wr_mem_buf_sz ==6
                                          xx VV VV VV  VV VV VV VV    *p_wr_mem_buf_sz ==7
                                          VV VV VV VV  VV VV VV VV    *p_wr_mem_buf_sz ==8
                                        */
    *p_len -= bytes_written;
}


volatile void *_xrdp__memcpy(volatile void *dst, const volatile void *src, size_t len) 
{
    uint src_alignment = (uint)((uintptr_t)src & U32_ALIGNMENT_MASK); 
    uint dst_alignment = (uint)((uintptr_t)dst & U32_ALIGNMENT_MASK);
 
    /* In case of equal alignment, call the optimized function. The general algorithm will work but take longer to execute. */
    if (src_alignment == dst_alignment) 
    {
        _xrdp__memcpy_equal_alignment(dst, src, len);
    }
    else
    if (len) /*this algorithm does not work if len==0*/
    {
        volatile uint32_t const *u32_aligned_src_ptr = (volatile uint32_t const *)((uintptr_t)src & ~U32_ALIGNMENT_MASK);
        uint32_t u32word_rd_cnt = (len+src_alignment+sizeof(uint32_t)-1)/sizeof(uint32_t); /* number of uint32 words to be read from *src memory */
        memcpy_buf_t rd_mem_buf = {0};  

        volatile uint8_t *u8dst_ptr = (volatile uint8_t *)dst; /* keep original dst for return value */
        memcpy_buf_t wr_mem_buf = {0};  
        int wr_mem_buf_sz = 0; /* number of valid bytes in wr_mem_buf. If wr_mem_buf_sz>len, only len bytes are valid. */

        rd_mem_buf.u32l = *u32_aligned_src_ptr; 

        ++u32_aligned_src_ptr;
        --u32word_rd_cnt;       
        while (u32word_rd_cnt)
        {
            rd_mem_buf.u32h = *u32_aligned_src_ptr;
            rd_mem_buf.u64 >>= src_alignment*BITS__PER__BYTE;

            _xrdp__memcpy_helper_unaligned_32_write(&u8dst_ptr, &len, &wr_mem_buf, &wr_mem_buf_sz, rd_mem_buf.u32l);

            rd_mem_buf.u64 >>= (sizeof(uint32_t) - src_alignment)*BITS__PER__BYTE;
            ++u32_aligned_src_ptr;
            --u32word_rd_cnt;
        }

        rd_mem_buf.u64 >>= src_alignment*BITS__PER__BYTE;

        _xrdp__memcpy_helper_unaligned_32_write(&u8dst_ptr, &len, &wr_mem_buf, &wr_mem_buf_sz, rd_mem_buf.u32l);

        if (len) 
        {
            /* flush wr_mem_buf */
            _xrdp__memcpy_helper_unaligned_32_write(&u8dst_ptr, &len, &wr_mem_buf, &wr_mem_buf_sz, 0);
        }
    }

    return dst;
}

volatile void *_xrdp__memset(volatile void *dst, int val, size_t len)
{
    uint8_t  *u8dst_ptr = (uint8_t *)dst;
    uint8_t  c_val = (uint8_t)val;
    uint16_t u16_val = (uint16_t)c_val<<sizeof(uint8_t)*BITS__PER__BYTE    | (uint16_t)c_val;
    uint32_t u32_val = (uint32_t)u16_val<<sizeof(uint16_t)*BITS__PER__BYTE | (uint32_t)u16_val;

    if (((uintptr_t)u8dst_ptr & U16_ALIGNMENT_MASK) && (len > 0))
    {
        *u8dst_ptr = c_val;
        u8dst_ptr += sizeof(uint8_t);
        len -= sizeof(uint8_t);
    }

    if (((uintptr_t)u8dst_ptr & U32_ALIGNMENT_MASK) && (len >= sizeof(uint16_t))) 
    {
        *(uint16_t *)u8dst_ptr = u16_val;
        u8dst_ptr += sizeof(uint16_t);
        len -= sizeof(uint16_t);
    }

    {
        uint32_t *u32dst_ptr = (uint32_t *)u8dst_ptr;
        while (len >= sizeof(uint32_t)) 
        {
            *u32dst_ptr++ = u32_val;
            u8dst_ptr += sizeof(uint32_t);
            len -= sizeof(uint32_t);
        }
    }

    if (len >= sizeof(uint16_t)) 
    {
        *(uint16_t *)u8dst_ptr = u16_val;
        u8dst_ptr += sizeof(uint16_t);
        len -= sizeof(uint16_t);
    }
    
    if (len > 0)
    {
        *u8dst_ptr = c_val;
        u8dst_ptr += sizeof(uint8_t);
        len -= sizeof(uint8_t);
    }

    return dst;
}


#ifdef XRDP__MEMCPY_MEMSET_UNIT_TEST

/*
to run this unit test, call _xrdp__memcpy_memset_unit_test((). For example, add the following line before 
returning from rdp_drv_policer.c::drv_cnpl_policer_set()
 
_xrdp__memcpy_memset_unit_test((uint32_t *)DEVICE_ADDRESS(RU_BLK(CNPL).addr[0])); 
 
and send these commands from the shell:
echo '0' > /proc/sys/kernel/nmi_watchdog\n
bs /Driver/rNr_regs/Bkpt/re 0 0\nbs /Driver/rNr_regs/Bkpt/re 1 0\nbs /Driver/rNr_regs/Bkpt/re 2 0\n
echo RunnerStopped\n
bs /b/n policer/index=10,dir=ds,cfg={type=dual_bucket_tr,cr=100000,cbs=200,pr=13089000,pbs=72000,dei_mode=no} \n 
*/


typedef int boolean;
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

static void unit_test_memcpy_io_32bit_aligned(volatile uint32_t *dest, volatile uint32_t *src, size_t n_words)
{
    while (n_words)
    {
        *dest = *src;
        ++dest;
        ++src;
        --n_words;
    }
}


#define TEST_BUF_U64_SZ (8)
#define TEST_BUF_U32_SZ (TEST_BUF_U64_SZ*2)
#define TEST_BUF_U8_SZ (sizeof(uint64_t)*TEST_BUF_U64_SZ)
static uint64_t ddr_inp_mem[TEST_BUF_U64_SZ];
static uint64_t ddr_out_mem_undertest[TEST_BUF_U64_SZ];
static uint64_t ddr_out_mem_reference[TEST_BUF_U64_SZ];
static uint64_t ddr_xrdp_mem_simulated[TEST_BUF_U64_SZ];
static uint8_t glob_seed_val;

/* returns next recommented seed value */
static inline uint8_t memcpy_unit_test__init_mem(uint8_t seed, volatile uint32_t *xrdp_test_mem)
{
    int cnt;
    for (cnt = 0; cnt < sizeof(ddr_inp_mem); ++cnt)
    {
        ((uint8_t *)ddr_inp_mem)[cnt] = seed;
        ++seed;
    }
    
    memset(ddr_out_mem_undertest, 0xAA, sizeof(ddr_out_mem_undertest));
    memset(ddr_out_mem_reference, 0xAA, sizeof(ddr_out_mem_reference));
    memset(ddr_xrdp_mem_simulated, 0xA0, sizeof(ddr_xrdp_mem_simulated));
    unit_test_memcpy_io_32bit_aligned(xrdp_test_mem, (volatile uint32_t *)ddr_xrdp_mem_simulated, TEST_BUF_U32_SZ);
    return seed;
}

static inline boolean memcpy_unit_test__compare_out_mem(uint64_t test_num, 
                                                        volatile uint32_t *xrdp_test_mem, 
                                                        size_t length, 
                                                        volatile uint8_t *xrdp_prt, 
                                                        uint8_t *ddr_inp_prt, 
                                                        uint8_t *ddr_out_prt_undertest)
{
    int cnt;
    boolean b_error = FALSE;
    
    for (cnt = 0; cnt < sizeof(ddr_out_mem_undertest); ++cnt)
    {
        if (((uint8_t *)ddr_out_mem_undertest)[cnt] != ((uint8_t *)ddr_out_mem_reference)[cnt])
        {
            b_error = TRUE;
        }
    }

    if (b_error)
    {
        bdmf_trace("test_num: 0x%08X %08X length 0x%02X, xrdp_mem/prt %p/%p, ddr_inp_mem/prt %p/%p, ddr_out_mem/prt %p/%p\n"
                   , (uint32_t)(test_num >> 32), (uint32_t)test_num, length, xrdp_test_mem, xrdp_prt, ddr_inp_mem, ddr_inp_prt
                   , ddr_out_mem_undertest, ddr_out_prt_undertest);
    }

    if (b_error)
    {               
        bdmf_trace("ddr_inp_mem:           ");
        for (cnt = 0; cnt < sizeof(ddr_inp_mem); ++cnt)
        {
            bdmf_trace("%02X ", ((uint8_t *)ddr_inp_mem)[cnt]);
        }
        bdmf_trace("\nddr_inp_mem:           ");
        {
            uint8_t *lcl_prt = (uint8_t *)ddr_inp_mem;
            for (cnt = 0; cnt < sizeof(ddr_inp_mem); ++cnt, ++lcl_prt)
            {
                bdmf_trace("%s ", (ddr_inp_prt <= lcl_prt && lcl_prt < (ddr_inp_prt+length)) ? "^^" : "  ");
            }
        }
        bdmf_trace("\nddr_out_mem_undertest: ");
        for (cnt = 0; cnt < sizeof(ddr_out_mem_undertest); ++cnt)
        {
            bdmf_trace("%02X ", ((uint8_t *)ddr_out_mem_undertest)[cnt]);
        }
        bdmf_trace("\nddr_out_mem_reference: ");
        for (cnt = 0; cnt < sizeof(ddr_out_mem_reference); ++cnt)
        {
            bdmf_trace("%02X ", ((uint8_t *)ddr_out_mem_reference)[cnt]);
        }
        
        bdmf_trace("\ndiff:                  ");
        for (cnt = 0; cnt < sizeof(ddr_out_mem_undertest); ++cnt)
        {
            if (((uint8_t *)ddr_out_mem_undertest)[cnt] == ((uint8_t *)ddr_out_mem_reference)[cnt])
            {
                bdmf_trace("   ");
            }
            else
            {
                bdmf_trace("!! ");
            }
        }
    
        bdmf_trace("\nxrdp_test_mem:         ");
        for (cnt = 0; cnt < TEST_BUF_U32_SZ; ++cnt)
        {
            bdmf_trace("%02X %02X %02X %02X "
                       , ((xrdp_test_mem)[cnt]&(uint32_t)0x000000FF) >> 0 , ((xrdp_test_mem)[cnt]&(uint32_t)0x0000FF00) >> 8 
                       , ((xrdp_test_mem)[cnt]&(uint32_t)0x00FF0000) >> 16, ((xrdp_test_mem)[cnt]&(uint32_t)0xFF000000) >> 24);
        }
        bdmf_trace("\nxrdp_test_mem:         ");
        {
            uint8_t *lcl_prt = (uint8_t *)xrdp_test_mem;
            for (cnt = 0; cnt < TEST_BUF_U8_SZ; ++cnt, ++lcl_prt)
            {
                bdmf_trace("%s ", (xrdp_prt <= lcl_prt && lcl_prt < (xrdp_prt+length)) ? "^^" : "  ");
            }
        }

        bdmf_trace("\nxrdp_test_mem:         ");
        for (cnt = 0; cnt < TEST_BUF_U32_SZ; ++cnt)
        {
            bdmf_trace("%08X ", (xrdp_test_mem)[cnt]);
        }
        bdmf_trace("\n\n");
    }

    return b_error;
}

/* xrdp_test_mem points to an XRDP registers memory mapped space of at least TEST_BUF_U8_SZ bytes (ex phys addr 0x82E4AAF8) */
void _xrdp__memcpy_memset_unit_test(volatile uint32_t *xrdp_test_mem)
{
    uint8_t *ddr_inp_prt;
    volatile uint8_t *xrdp_prt;
    uint8_t *ddr_out_prt_undertest;
    uint8_t *ddr_out_prt_reference;
    size_t length;
    uint64_t test_num = 0;
    boolean b_error = FALSE;

    int test_repetition_cnt = 0;
    
    bdmf_trace("_xrdp__memcpy_memset_unit_test. xrdp_test_mem %p=>\n", xrdp_test_mem);
    
    for (test_repetition_cnt = 0; test_repetition_cnt < 100; ++test_repetition_cnt)
    {
        bdmf_trace("_xrdp__memcpy_memset_unit_test test_repetition_cnt %d\n", test_repetition_cnt);

        for (ddr_inp_prt = (uint8_t *)(ddr_inp_mem+1); (uint8_t *)ddr_inp_prt < (uint8_t *)(ddr_inp_mem+2); ++ddr_inp_prt)
        {
            for (ddr_out_prt_undertest = (uint8_t *)(ddr_out_mem_undertest+1), ddr_out_prt_reference = (uint8_t *)(ddr_out_mem_reference+1); 
                  ddr_out_prt_undertest < (uint8_t *)(ddr_out_mem_undertest+2); 
                  ++ddr_out_prt_undertest, ++ddr_out_prt_reference  
                )
            {
                for (xrdp_prt = (volatile uint8_t *)(xrdp_test_mem)+sizeof(uint64_t); xrdp_prt < (volatile uint8_t *)(xrdp_test_mem)+2*sizeof(uint64_t); ++xrdp_prt)
                {
                    for (length = 0; length < sizeof(uint64_t)*2; ++length)
                    {
                        /* memset DDR buffer */
                        glob_seed_val = memcpy_unit_test__init_mem(glob_seed_val, xrdp_test_mem);
                        _xrdp__memset(ddr_out_prt_undertest, 0x10, length);
                               memset(ddr_out_prt_reference, 0x10, length);
                        if (memcpy_unit_test__compare_out_mem(test_num + 0x00, xrdp_test_mem, length, xrdp_prt, ddr_inp_prt, ddr_out_prt_undertest))
                        {
                            b_error = TRUE;
                            goto xrdp__memcpy_memset_unit_test_exit_on_error;
                        }
    
                        /* memcpy DDR to DDR */
                        glob_seed_val = memcpy_unit_test__init_mem(glob_seed_val, xrdp_test_mem);
                        _xrdp__memcpy(ddr_out_prt_undertest, ddr_inp_prt, length);
                        memcpy(ddr_out_prt_reference, ddr_inp_prt, length);
                        if (memcpy_unit_test__compare_out_mem(test_num + 0x01, xrdp_test_mem, length, xrdp_prt, ddr_inp_prt, ddr_out_prt_undertest))
                        {
                            b_error = TRUE;
                            goto xrdp__memcpy_memset_unit_test_exit_on_error;
                        }
    
                        if (0 == (length & U32_ALIGNMENT_MASK)  &&  0 == ((uintptr_t)xrdp_prt & U32_ALIGNMENT_MASK)) /* can't memset unaligned XRDP memory */
                        {
                            /* memset XRDP buffer */
                            glob_seed_val = memcpy_unit_test__init_mem(glob_seed_val, xrdp_test_mem); 
                            _xrdp__memset(xrdp_prt, 0x12, length);
                            unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)ddr_out_mem_undertest, xrdp_test_mem, TEST_BUF_U32_SZ);
                            memset((uint8_t *)ddr_xrdp_mem_simulated + ((uint8_t *)xrdp_prt - (uint8_t *)xrdp_test_mem), 0x12, length);
                            unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)ddr_out_mem_reference, (volatile uint32_t *)ddr_xrdp_mem_simulated, TEST_BUF_U32_SZ);
                            if (memcpy_unit_test__compare_out_mem(test_num + 0x02, xrdp_test_mem, length, xrdp_prt, ddr_inp_prt, ddr_out_prt_undertest))
                            {
                                b_error = TRUE;
                                goto xrdp__memcpy_memset_unit_test_exit_on_error;
                            }
                        }
                        
                        if (0 == (length & U32_ALIGNMENT_MASK)  &&  0 == ((uintptr_t)xrdp_prt & U32_ALIGNMENT_MASK)) /* can't write to unaligned XRDP memory */
                        {
                            /* memcpy DDR to XRDP */
                            glob_seed_val = memcpy_unit_test__init_mem(glob_seed_val, xrdp_test_mem); 
                            _xrdp__memcpy(xrdp_prt, ddr_inp_prt, length);
                            unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)ddr_out_mem_undertest, xrdp_test_mem, TEST_BUF_U32_SZ);
                            memcpy((uint8_t *)ddr_xrdp_mem_simulated + ((uint8_t *)xrdp_prt - (uint8_t *)xrdp_test_mem), ddr_inp_prt, length);
                            unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)ddr_out_mem_reference, (volatile uint32_t *)ddr_xrdp_mem_simulated, TEST_BUF_U32_SZ);
                            if (memcpy_unit_test__compare_out_mem(test_num + 0x03, xrdp_test_mem, length, xrdp_prt, ddr_inp_prt, ddr_out_prt_undertest))
                            {
                                b_error = TRUE;
                                goto xrdp__memcpy_memset_unit_test_exit_on_error;
                            }
                        }

                        /* memcpy XRDP to DDR */
                        glob_seed_val = memcpy_unit_test__init_mem(glob_seed_val, xrdp_test_mem); 
                        unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)xrdp_test_mem, (volatile uint32_t *)ddr_inp_mem, TEST_BUF_U32_SZ);
                        unit_test_memcpy_io_32bit_aligned((volatile uint32_t *)ddr_xrdp_mem_simulated, (volatile uint32_t *)ddr_inp_mem, TEST_BUF_U32_SZ);
                        _xrdp__memcpy(ddr_out_prt_undertest, xrdp_prt, length);
                        memcpy(ddr_out_prt_reference, (uint8_t *)ddr_xrdp_mem_simulated + ((uint8_t *)xrdp_prt - (uint8_t *)xrdp_test_mem),  length);
                        if (memcpy_unit_test__compare_out_mem(test_num + 0x04, xrdp_test_mem, length, xrdp_prt, ddr_inp_prt, ddr_out_prt_undertest))
                        {
                            b_error = TRUE;
                            goto xrdp__memcpy_memset_unit_test_exit_on_error;
                        }

                        test_num += 0x0000000001000000L;
                    }
                    
                    test_num += 0x0000000100000000L;
                }
    
                test_num += 0x0000010000000000L;
            }
            
            test_num += 0x0001000000000000L;
        }
        
        test_num += 0x0100000000000000L;
    }

xrdp__memcpy_memset_unit_test_exit_on_error:

    bdmf_trace("_xrdp__memcpy_memset_unit_test test_repetition_cnt %d  %s\n", test_repetition_cnt, b_error ? "!!! FAILED !!!" : "Passed");
}

#endif /* XRDP__MEMCPY_MEMSET_UNIT_TEST */

#endif /* !defined(_CFE_) && !defined(RDP_SIM) && defined(DUAL_ISSUE) */
                                                                        
#endif /* USE_BDMF_SHELL */

#ifdef XRDP_EMULATION
/* used for ACCESS_MACORS, defined at emulation env. */
write32_p write32;
write16_p write16;
write8_p write8;
read32_p read32;
read16_p read16;
read8_p read8;
#endif
