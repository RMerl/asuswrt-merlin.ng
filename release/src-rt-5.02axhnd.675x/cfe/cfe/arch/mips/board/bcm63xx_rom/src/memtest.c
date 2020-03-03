/*
 *----------------------------------------------------------------------------*
 *  Collection of Memory Tests.
 *----------------------------------------------------------------------------*
 */

//
// NOTE: Blatant inlining ... (not sure whether caller supports EABI calls).
// WARNING CFE: Must NOT use function calls !!!
//
#define _ALWAYS_INLINE_     __attribute__((always_inline))
#define _INLINE_            inline static

typedef enum memTestResult {
    MEMTEST_FAILURE = 0,
    MEMTEST_SUCCESS,
    MEMTEST_ERROR,
} MemTestResult_t;

/* ------------------------------------------------------------------------- */

#undef PATTERN
#define PATTERN(x)      PATTERN_##x,

/*
 * For each pattern listed, the inverse pattern is also automatically used.
 * E.g. 0x55555555, the inverse of defined 0xAAAAAAAA is covered.
 */
typedef enum pattern {
    PATTERN(0x00000000)
    PATTERN(0xAAAAAAAA)
    PATTERN(0xCCCCCCCC)
    PATTERN(0x77777777)
    PATTERN(0xF0F0F0F0)
    PATTERN(0xFF00FF00)
    PATTERN(0xFFFF0000)
    PATTERN(0x01234567)
    PATTERN(0x89ABCDEF)
    PATTERN_MAX,
} Pattern_t;

#undef PATTERN
#define PATTERN(x)      x,
const uint32_t pattern[] = {
    PATTERN(0x00000000)
    PATTERN(0xAAAAAAAA)
    PATTERN(0xCCCCCCCC)
    PATTERN(0x77777777)
    PATTERN(0xF0F0F0F0)
    PATTERN(0xFF00FF00)
    PATTERN(0xFFFF0000)
    PATTERN(0x01234567)
    PATTERN(0x89ABCDEF)
    PATTERN_MAX,
};

/* ------------------------------------------------------------------------- */

#ifndef NBBY
#define NBBY            8   /* FreeBSD style: Number Bits per BYte */
#endif

/* ------------------------------------------------------------------------- */

#define NBITS(type)             (sizeof(type) * NBBY)
#define NBITVAL(nbits)          (1 << (nbits))
#define MAXBITVAL(nbits)        ( NBITVAL(nbits) - 1)
#define NBITMASK(nbits)         MAXBITVAL(nbits)
#define MAXNBVAL(nbyte)         MAXBITVAL((nbyte) * NBBY)

#define DIVBY(val32,by)         ((val32)>>(by)) 
#define MODBY(val32,by)         ((val32) & ((1 <<(by)) - 1) )

#define IS_POWEROF2(val32)      ( (((val32)-1) & (val32)) == 0 )

#define ROUNDDN(addr, align)    ( (addr) & ~((align) - 1) )
#define ROUNDUP(addr, align)    ( ((addr) + (align) - 1) & ~((align) - 1) )
//#define ROUNDUP(x,y)          ((((x)+((y)-1))/(y))*(y))
#define ALIGN_ADDR(addr, bytes) (void *)( ((uint32_t *)(addr) + (bytes) - 1) \
                                          & ~((bytes) - 1) )
#define IS_ALIGNED(addr, bytes) (((uint32_t)(addr) & ((bytes)-1)) == 0)

#define OFFSET_OF(stype,member) ((uint32_t) &((struct stype *)0)->member)
#define RELOC(base,stype,member)  ((base) + OFFSET_OF(stype, member))

#define RROTATE32(val32)        (((val32) << 31) | ((val32) >>  1))
#define LROTATE32(val32)        (((val32) <<  1) | ((val32) >> 31))

/* ------------------------------------------------------------------------- */

/* Aligned (32bit register) read/write access */
#define RD16(addr16)            (*(volatile uint32_t *)(addr16))
#define WR16(addr16,val16)      (*(volatile uint32_t *)(addr16))=(val16)
#define RD32(addr32)            (*(volatile uint32_t *)(addr32))
#define WR32(addr32,val32)      (*(volatile uint32_t *)(addr32))=(val32)

/*---------------------------------------------------------------------------*/

/* Forward declaration */
_INLINE_ void fill_memory( uint32_t * addr, uint32_t bytes, uint32_t fill32)
                                                                _ALWAYS_INLINE_;
_INLINE_ void fill_alt_memory(uint32_t * addr, uint32_t bytes,
                            uint32_t fillA32, uint32_t fillB32) _ALWAYS_INLINE_;

void fill_memory( uint32_t * addr, uint32_t bytes, uint32_t fill32)
{
    uint32_t * at, * end_p;
    uint32_t words;
    words = bytes / sizeof(uint32_t);
    if ( words == 0 ) return;

    end_p = addr + words;
    for ( at = addr; at < end_p; at++ )
        WR32( at, fill32 );
}

void fill_alt_memory( uint32_t * addr, uint32_t bytes,
                      uint32_t fillA32, uint32_t fillB32)
{
    uint32_t * at, * end_p;
    uint32_t words;
    words = bytes / sizeof(uint32_t);
    words = ROUNDDN( words, 2 );
    if ( words == 0 ) return;

    end_p = addr + words;
    for ( at = addr; at < end_p; at+=2 )
    {
        WR32( at+0, fillA32 );
        WR32( at+1, fillB32 );
    }
}

/* Forward declaration */
_INLINE_ MemTestResult_t scanWordValue( uint32_t * addr, uint32_t bytes,
                uint32_t pat32 )          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t scanBulkValue( uint32_t * addr, uint32_t bytes,
                uint32_t pat32 )          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t scanBulkAltInv( uint32_t * addr, uint32_t bytes,
                uint32_t pat32 )          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t scanWordSelf( uint32_t * addr, uint32_t bytes )
                                          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t scanBulkSelf( uint32_t * addr, uint32_t bytes )
                                          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t slidingAltInv( uint32_t * addr, uint32_t bytes,
                uint32_t pat32 )          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t slidingDiag( uint32_t * addr, uint32_t bytes,
                uint32_t pat32 )          _ALWAYS_INLINE_;
_INLINE_ MemTestResult_t memoryBulkCopy( uint32_t * saddr, uint32_t * daddr,
                uint32_t bytes )          _ALWAYS_INLINE_;

/*
 *-----------------------------------------------------------------------------
 * Function: scanWordValue
 *
 * Description:
 * 4 Passes are conducted on the memory region.
 * Pass 1. In INcreasing memory address, write a word with value and verify. 
 * Pass 2. In DEcreasing memory address, write a word with value and verify. 
 * Pass 3. In INcreasing memory address, write a word with INVERSE and verify. 
 * Pass 4. In DEcreasing memory address, write a word with INVERSE and verify. 
 * Pass 5. In INcreasing shifted memory address, write word with value verify.
 * Pass 6. In INcreasing shifted memory address, write word with INVERSE verify.
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *  pat32:  32bit pattern, e.g. 0x0U, 0xAAAAAAAA, 0xFF00FF00, 0xFFFF0000,
 *                              0xF0F0F0F0, 0xC3C3C3C3, 0x87878787
 *-----------------------------------------------------------------------------
 */
MemTestResult_t scanWordValue( uint32_t * addr, uint32_t bytes, uint32_t pat32 )
{
    volatile uint32_t * at, * end_p;
    uint32_t expected, read, words;
    uint32_t shift;
    
    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 )
        return MEMTEST_ERROR;

    expected  = pat32;  /* ORIGINAL value */

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    expected  = ~pat32; /* INVERSE value */

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* PASS 5: Shifting address walk, ORIGINAL */
    expected  = pat32;  /* ORIGINAL value */

    end_p = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, expected );
        WR32( addr, ~expected );    /* noise at base addr */
        read = RD32(at);
        if ( read != expected )
        {   
            return MEMTEST_FAILURE;
        }
    }

    expected  = ~pat32;  /* INVERSE value */

    /* PASS 6: Shifting address walk, INVERSE */
    end_p = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, expected );
        WR32( addr, ~expected );    /* noise at base addr */
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}

/*
 *-----------------------------------------------------------------------------
 * Function: scanBulkValue
 *
 * Description:
 * Pass 1. Fill entire memory in INcreasing memory address with value
 *         then in INcreasing memory address read and verify.
 * Pass 2. Fill entire memory in DEcreasing memory address with value
 *         then in DEcreasing memory address read and verify.
 * Pass 3. Fill entire memory in INcreasing memory address with inverse value
 *         then in INcreasing memory address read and verify.
 * Pass 4. Fill entire memory in DEcreasing memory address with inverse value
 *         then in DEcreasing memory address read and verify.
 * Pass 5. INcreasing shifted, ORIGINAL
 * Pass 6. INcreasing shifted, INVERSE
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *  pat32:  32bit pattern, e.g. 0x0U, 0xAAAAAAAA, 0xFF00FF00, 0xFFFF0000,
 *                              0xF0F0F0F0, 0xC3C3C3C3, 0x87878787
 *-----------------------------------------------------------------------------
 */
MemTestResult_t scanBulkValue( uint32_t * addr, uint32_t bytes, uint32_t pat32 )
{
    volatile uint32_t * at, * end_p;
    uint32_t expected, read, words;
    uint32_t shift;

    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 ) return MEMTEST_ERROR;

    expected  = pat32;  /* ORIGINAL value */

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, expected );
    }
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, expected );
    }
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    expected  = ~pat32; /* INVERSE value */

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, expected );
    }
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, expected );
    }
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* Pass 5. INCREASING Shifted traversal */
    expected  = pat32;  /* ORIGINAL value */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, expected );
        WR32( addr, ~expected );    /* noise at base addr */
    }
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    expected  = ~pat32;  /* INVERSE value */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, expected );
        WR32( addr, ~expected );    /* noise at base addr */
    }
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}

/*
 *-----------------------------------------------------------------------------
 * Function: scanBulkAltInv
 *
 * Description:
 * Pass 1. Fill entire memory in INcreasing memory address with alternating
 *         value, then in INcreasing memory address read and verify.
 * Pass 2. Fill entire memory in DEcreasing memory address with alternating
 *         value, then in DEcreasing memory address read and verify.
 * Pass 3. Same as one but with shifted address.
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *  pat32:  32bit pattern, e.g. 0x0U, 0xAAAAAAAA, 0xFF00FF00, 0xFFFF0000,
 *                              0xF0F0F0F0, 0xC3C3C3C3, 0x87878787
 *-----------------------------------------------------------------------------
 */
MemTestResult_t scanBulkAltInv( uint32_t * addr, uint32_t bytes, uint32_t pat32 )
{
    volatile uint32_t * at, * end_p;
    uint32_t read, words;
    uint32_t shift;

    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    words = ROUNDDN( words, 2 );
    if ( words == 0 ) return MEMTEST_ERROR;

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at+=2 )
    {
        WR32( at+0,  pat32 );
        WR32( at+1, ~pat32 );
    }
    for ( at = addr; at < end_p; at+=2 )
    {
        read = RD32( at+0 );
        if ( read != pat32 )
        {
            return MEMTEST_FAILURE;
        }
        read = RD32( at+1 );
        if ( read != ~pat32 )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-=2 )
    {
        WR32( at+0,  pat32 );
        WR32( at+1, ~pat32 );
    }
    for ( at = addr + words - 1; at >= end_p; at-=2 )
    {
        read = RD32( at+0 );
        if ( read != pat32 )
        {
            return MEMTEST_FAILURE;
        }
        read = RD32( at+1 );
        if ( read != ~pat32 )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING SHIFTED traversal */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at+0,  pat32 );
        WR32( addr, 0 );
        WR32( at+1, ~pat32 );
        WR32( addr, 0 );
    }
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        read = RD32( at+0 );
        if ( read != pat32 )
        {
            return MEMTEST_FAILURE;
        }
        read = RD32( at+1 );
        if ( read != ~pat32 )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}


/*
 *-----------------------------------------------------------------------------
 * Function: scanWordSelf
 *
 * Description:
 * 4 Passes are conducted on the memory region.
 * Pass 1. In INcreasing memory address, write a word with selfaddr and verify. 
 * Pass 2. In DEcreasing memory address, write a word with INVERSE and verify. 
 * Pass 3. In INcreasing memory address, write a word with INVERSE and verify. 
 * Pass 4. In DEcreasing memory address, write a word with selfaddr and verify. 
 * Pass 5. value = ORIGINAL address, INCREASING SHIFTED traversal.
 * Pass 6. value = INVERSE address, INCREASING SHIFTED traversal.
 *
 * In Pass 2 Read+Modify+Write, and in Pass 3, Read+Write is used
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *
 *-----------------------------------------------------------------------------
 */
MemTestResult_t scanWordSelf( uint32_t * addr, uint32_t bytes )
{
    volatile uint32_t * at, * end_p;
    uint32_t expected, read, words;
    uint32_t shift;
    
    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_FAILURE;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 ) return MEMTEST_ERROR;

    /* ORIGINAL value */

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        expected = (uint32_t)at;
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        expected = ~( (uint32_t)RD32(at) );
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        expected = ((uint32_t)RD32(at));
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        expected = ~((uint32_t)at);
        WR32( at, expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* value = ORIGINAL address, INCREASING SHIFTED traversal */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        expected = (uint32_t)at;    /* Not read modify write */
        WR32( at, expected );
        WR32( addr, ~expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* value = INVERSE address, INCREASING SHIFTED traversal */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        expected = ~(uint32_t)(at); /* Not read modify write */
        WR32( at, expected );
        WR32( addr, ~expected );
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}

/*
 *-----------------------------------------------------------------------------
 * Function: scanBulkSelf
 *
 * Description:
 * Pass 1. Fill entire memory in INcreasing memory address with self address
 *         then in INcreasing memory address read and verify.
 * Pass 2. Fill entire memory in DEcreasing memory address with self address
 *         then in DEcreasing memory address read and verify.
 * Pass 3. Fill entire memory in INcreasing memory address with inverse addr
 *         then in INcreasing memory address read and verify.
 * Pass 4. Fill entire memory in DEcreasing memory address with inverse addr
 *         then in DEcreasing memory address read and verify.
 * Pass 5. Same as Pass 1 but with shifted address
 * Pass 6. Same as Pass 3 but with shifted address
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *-----------------------------------------------------------------------------
 */
MemTestResult_t scanBulkSelf( uint32_t * addr, uint32_t bytes )
{
    volatile uint32_t * at, * end_p;
    uint32_t read, words;
    uint32_t shift;

    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 ) return MEMTEST_ERROR;

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, (uint32_t)at );
    }
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != (uint32_t)at )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, (uint32_t)at );
    }
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != (uint32_t)at )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING traversal */
    end_p     = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, ~((uint32_t)at) );
    }
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != ~((uint32_t)at) )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal */
    end_p     = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, ~((uint32_t)at) );
    }
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != ~((uint32_t)at) )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING traversal */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, (uint32_t)at );
        WR32( addr, ~((uint32_t)at) );
    }
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        read = RD32(at);
        if ( read != (uint32_t)at )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING traversal */
    end_p     = addr + words;
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        WR32( at, ~((uint32_t)at) );
        WR32( addr, ((uint32_t)at) );
    }
    shift = sizeof(uint32_t);
    for ( at = addr + shift; at < end_p; shift <<= 1, at = addr + shift )
    {
        read = RD32(at);
        if ( read != ~((uint32_t)at) )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}


/*
 *-----------------------------------------------------------------------------
 * Function: slidingAltInv
 *
 * Description:
 * This is the same as scanBulkAltInv, where in each invocation the value is 
 * rotated to the right. The starting value is usedefined.
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *  pat32:  pattern to slide per pass
 *-----------------------------------------------------------------------------
 */
MemTestResult_t slidingAltInv( uint32_t * addr, uint32_t bytes, uint32_t pat32 )
{
    uint32_t sliding_pat32, i;

    if ( pat32 ==  0x0 ) pat32 = 0x80000000;
    if ( pat32 == ~0x0 ) pat32 = 0x7FFFFFFF;

    sliding_pat32 = pat32;

    for ( i=0; i<32; i++ )
    {
        if ( scanBulkAltInv( addr, bytes, sliding_pat32 )
             == MEMTEST_FAILURE )
        {
            return MEMTEST_FAILURE;
        }

        sliding_pat32 = RROTATE32( sliding_pat32 );
    }

    sliding_pat32 = pat32;
    for (i=0; i<32; i++)
    {
        if ( scanBulkAltInv( addr, bytes, sliding_pat32 )
             == MEMTEST_FAILURE )
        {
            return MEMTEST_FAILURE;
        }

        sliding_pat32 = LROTATE32( sliding_pat32 );
    }

    return MEMTEST_SUCCESS;
}

/*
 *-----------------------------------------------------------------------------
 * Function: slidingDiag
 *
 * Description:
 * Pass 1. Fill entire memory in INcreasing memory address with pattern right
 *         shifted. Then read in INcreasing order and verify.
 * Pass 2. Fill entire memory in DEcreasing memory address with inverse of
 *         read value. Then read in DEcreasing order and verify.
 * Pass 3. Fill entire memory in DEcreasing memory address with pattern right
 *         shifted. Then read in DEcreasing order and verify.
 * Pass 4. Fill entire memory in INcreasing memory address with inverse of
 *         read value. Then read in INcreasing order and verify.
 *
 * Parameters:
 *  addr:   word aligned pointer to memory region
 *  bytes:  size in bytes of memory region to test
 *  pat32:  pattern to be filled shifted each write
 *-----------------------------------------------------------------------------
 */
MemTestResult_t slidingDiag( uint32_t * addr, uint32_t bytes, uint32_t pat32 )
{
    volatile uint32_t * at, * end_p;
    uint32_t expected, read = 0, words, last;

    if ( ! IS_ALIGNED(addr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 ) return MEMTEST_ERROR;


    /* INCREASING traversal */
    expected = pat32;  /* ORIGINAL value */
    end_p = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        WR32( at, expected );
        expected = RROTATE32( expected );   /* next expected */
    }
    expected = pat32;   /* ORIGINAL value */
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
        expected = RROTATE32( expected );   /* next expected */
    }

    last = ~( read );   /* Starting value for decreasing traversal, next */

    /* DECREASING traversal */
    end_p = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        expected = ~( RD32(at) );
        WR32( at, expected );
    }
    expected = last;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
        expected = LROTATE32( expected );
    }

    /* DECREASING traversal */
    expected = pat32;  /* ORIGINAL value */
    end_p = addr;
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        WR32( at, expected );
        expected = RROTATE32( expected );   /* next expected */
    }
    expected = pat32;  /* ORIGINAL value */
    for ( at = addr + words - 1; at >= end_p; at-- )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
        expected = RROTATE32( expected );   /* next expected */
    }

    last = ~( read );

    /* INCREASING traversal */
    end_p = addr + words;
    for ( at = addr; at < end_p; at++ )
    {
        expected = ~( RD32(at) );
        WR32( at, expected );
    }
    expected = last;
    for ( at = addr; at < end_p; at++ )
    {
        read = RD32(at);
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
        expected = LROTATE32( expected );
    }

    return MEMTEST_SUCCESS;
}

/*
 *-----------------------------------------------------------------------------
 * Function: memoryBulkCopy
 *
 * Description:
 * Pass 1. Copy entire ORIGINAL memory in INcreasing memory address, then verify
 * Pass 2. Copy entire ORIGINAL memory in DEcreasing memory address, then verify
 * Pass 3. Copy entire INVERSE  memory in INcreasing memory address, then verify
 * Pass 4. Copy entire INVERSE  memory in DEcreasing memory address, then verify
 *-----------------------------------------------------------------------------
 */
MemTestResult_t memoryBulkCopy( uint32_t * saddr, uint32_t * daddr,
                                uint32_t bytes )
{
    volatile uint32_t * src_p, * dst_p, * end_p; 
    uint32_t expected, read, words;

    if ( ! IS_ALIGNED(saddr,4) ) return MEMTEST_ERROR;
    if ( ! IS_ALIGNED(daddr,4) ) return MEMTEST_ERROR;

    words = bytes / sizeof(uint32_t);    /* in whole words (4byte multiple) */
    if ( words == 0 ) return MEMTEST_ERROR;

    if ( (uint32_t)saddr < (uint32_t)daddr )
    {
        if ( (uint32_t)(saddr + words) > (uint32_t)daddr )
            return MEMTEST_ERROR;
    }
    else if ( (uint32_t)daddr < (uint32_t)saddr )
    {
        if ( (uint32_t)(daddr + words) > (uint32_t)saddr )
            return MEMTEST_ERROR;
    }

    /* INCREASING traversal ORIGINAL */
    end_p = saddr + words;
    for ( src_p = saddr, dst_p = daddr; src_p < end_p; src_p++, dst_p++ )
    {
        expected = RD32( dst_p );
        WR32( src_p, expected );
    }
    for ( src_p = saddr, dst_p = daddr; src_p < end_p; src_p++, dst_p++ )
    {
        expected = RD32( dst_p );
        read = RD32( src_p );
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal ORIGINAL */
    end_p = saddr;
    for ( src_p = saddr + words - 1, dst_p = daddr + words - 1;
          src_p >= end_p; src_p--, dst_p-- )
    {
        expected = RD32( dst_p );
        WR32( src_p, expected );
    }
    for ( src_p = saddr + words - 1, dst_p = daddr + words - 1;
          src_p >= end_p; src_p--, dst_p-- )
    {
        expected = RD32( dst_p );
        read = RD32( src_p );
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* INCREASING traversal INVERSE */
    end_p = saddr + words;
    for ( src_p = saddr, dst_p = daddr; src_p < end_p; src_p++, dst_p++ )
    {
        expected = ~( RD32( dst_p ) );
        WR32( src_p, expected );
    }
    for ( src_p = saddr, dst_p = daddr; src_p < end_p; src_p++, dst_p++ )
    {
        expected = ~( RD32( dst_p ) );
        read = RD32( src_p );
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    /* DECREASING traversal INVERSE */
    end_p = saddr;
    for ( src_p = saddr + words - 1, dst_p = daddr + words - 1;
          src_p >= end_p; src_p--, dst_p-- )
    {
        expected = ~( RD32( dst_p ) );
        WR32( src_p, expected );
    }
    for ( src_p = saddr + words - 1, dst_p = daddr + words - 1;
          src_p >= end_p; src_p--, dst_p-- )
    {
        expected = ~( RD32( dst_p ) );
        read = RD32( src_p );
        if ( read != expected )
        {
            return MEMTEST_FAILURE;
        }
    }

    return MEMTEST_SUCCESS;
}

/*---------------------------------------------------------------------------*/
