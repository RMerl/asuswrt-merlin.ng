/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#ifndef __ACCESS_MACROS_H_INCLUDED

#define __ACCESS_MACROS_H_INCLUDED

#define RDP_BLOCK_SIZE          0x1000000
#define WAN_BLOCK_ADDRESS_MASK  0xFFFF

#if defined(__ARMEL__) || defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
#define _BYTE_ORDER_LITTLE_ENDIAN_
#endif
#ifndef FIRMWARE_LITTLE_ENDIAN
#define FIRMWARE_LITTLE_ENDIAN
#endif
#endif

#if defined(__ARMEL__) || defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define SOC_BASE_ADDRESS        0x00000000
#else
#define SOC_BASE_ADDRESS		0xA0000000
#endif

#if !defined(FIRMWARE_INIT) && !defined(USE_SOC_BASE_ADDR)
#define DEVICE_ADDRESS(_a) ( (SOC_BASE_ADDRESS) | ((uintptr_t)_a))
#else /*FIRMWARE_INIT || USE_SOC_BASE_ADDR:*/
extern uint8_t *soc_base_address;
#define DEVICE_ADDRESS(_a) ( (volatile uint8_t * const) (soc_base_address + ((uint32_t)(_a) & 0xFFFFF)) )
#endif

#ifndef FSSIM

#if defined LINUX_KERNEL || __KERNEL__
 #include "linux/types.h"
 #include <asm/barrier.h>
 #define WMB()	wmb()
#else
 #define WMB()	/* */
#endif

/* This is a temporary fix and must be removed once table manager is fixed */
#define FIELD_MREAD_I_32(p, x, y, i, r)

static inline uint16_t __swap2bytes(uint16_t a)
{
    return ( a << 8 ) | ( a >> 8 );
}
static inline uint32_t __swap4bytes(uint32_t a)
{
    return ( ( a << 24 ) |
             ( ( a & 0xFF00 ) << 8 ) |
             ( ( a & 0xFF0000 ) >> 8 ) |
             ( a >> 24 ) );
}

#if defined(FIRMWARE_LITTLE_ENDIAN)
#if defined(__ARMEL__) || defined(__AARCH64EL__)
static inline uint16_t swap2bytes(uint16_t a)
{
    __asm__("rev16 %0, %1" : "=r" (a) : "r" (a));
    return a;
}

#if defined(__ARMEL__)
static inline uint32_t swap4bytes(uint32_t a)
{
    __asm__("rev %0, %1" : "=r" (a) : "r" (a));
    return a;
}

#define READ_RX_DESC(_desc_ptr, _w0, _w1, _w2) \
	__asm__("ldm	%0, {%1, %2, %3}" \
		: "=r" (_desc_ptr), "=r" (_w0), "=r" (_w1), "=r" (_w2) \
		: "0" (_desc_ptr))
#else /* defined(__AARCH64EL__) */
static inline uint32_t swap4bytes(uint32_t a)
{
    __asm__("rev32 %0, %1" : "=r" (a) : "r" (a));
    return a;
}

/* reverses the 4 bytes in each 32-bit element of Xm*/
static inline uint64_t swap4bytes64(uint64_t a)
{
    __asm__("rev32 %0, %1" : "=r" (a) : "r" (a));
    return a;
}

#define READ_RX_DESC(_desc_ptr, _dw0, _dw1) \
	__asm__("ldnp	%1, %2, [%0]" \
		: "=r" (_desc_ptr), "=r" (_dw0), "=r" (_dw1) \
		: "0" (_desc_ptr))
#endif

#else
#define swap2bytes(a) __swap2bytes(a)
#define swap4bytes(a) __swap4bytes(a)
#endif /* __ARMEL__ */

#else /* FIRMWARE_LITTLE_ENDIAN */
#define swap2bytes(a)           ( a )
#define swap4bytes(a)           ( a )
#endif /* FIRMWARE_LITTLE_ENDIAN */

#ifdef FIRMWARE_INIT
#define cpu_to_le32(a)  __swap4bytes(a)
#define cpu_to_le16(a)  __swap2bytes(a)
#endif


/*
 * Endian swapping macros that work on any CPU.
 * Swap between CPU byte order and Big Endian byte order
 */
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_) || \
    (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(__BYTE_ORDER__) && defined(__LITTLE_ENDIAN__) && __BYTE_ORDER__ == __LITTLE_ENDIAN__)

#define cpu_be_swap_16(x)   swap2bytes(x)
#define cpu_be_swap_32(x)   swap4bytes(x)

#else

#define cpu_be_swap_16(x)   (x)
#define cpu_be_swap_32(x)   (x)

#endif


/* The following group of macros are for register access only.
   Please don't use them to read/write memory - they are unsafe
*/
#if defined(FIRMWARE_LITTLE_ENDIAN)

#define VAL32(_a)               ( *(volatile uint32_t*)(DEVICE_ADDRESS(_a)) )
#define READ_8(a, r)            ( *(volatile uint8_t*) &(r) = *(volatile uint8_t* ) DEVICE_ADDRESS(a) )
#define READ_16(a, r)           do { \
                                       uint16_t u16 = *(volatile uint16_t*) DEVICE_ADDRESS(a); \
                                       *(volatile uint16_t*)&(r) = swap2bytes(u16); \
                                } while(0)
#define READ_32(a, r)           do { \
                                       uint32_t u32 = *(volatile uint32_t*) DEVICE_ADDRESS(a); \
                                       *(volatile uint32_t*)&(r) = swap4bytes(u32); \
                                } while(0)

#define WRITE_8( a, r)			( *(volatile uint8_t* )DEVICE_ADDRESS(a) = *(uint8_t* )&(r) )
#define WRITE_16(a, r)			( *(volatile uint16_t*)DEVICE_ADDRESS(a) = swap2bytes(*(uint16_t*)&(r) ))
#define WRITE_32(a, r)			( *(volatile uint32_t*)DEVICE_ADDRESS(a) = swap4bytes(*(uint32_t*)&(r) ))

#define READ_I_8(a, i, r)		( *(volatile uint8_t* )&(r) = *((volatile uint8_t* ) DEVICE_ADDRESS(a) + (i)) )
#define READ_I_16(a, i, r)		do { \
                                    uint16_t u16 = *((volatile uint16_t*) DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint16_t*)&(r) = swap2bytes(u16); \
                                } while(0)
#define READ_I_32(a, i, r)		do { \
                                    uint32_t u32 = *((volatile uint32_t*) DEVICE_ADDRESS(a) + (i)); \
                                    *(volatile uint32_t*)&(r) = swap4bytes(u32); \
                                } while(0)

#define WRITE_I_8( a, i, r )    ( *((volatile uint8_t* ) DEVICE_ADDRESS(a) + (i)) = *(uint8_t*) &(r) )
#define WRITE_I_16( a, i, r )	( *((volatile uint16_t*) DEVICE_ADDRESS(a) + (i)) = swap2bytes(*(uint16_t*)&(r) ))
#define WRITE_I_32( a, i, r )	( *((volatile uint32_t*) DEVICE_ADDRESS(a) + (i)) = swap4bytes(*(uint32_t*)&(r) ))

#else

#define VAL32(_a) 				( *(volatile uint32_t*)(DEVICE_ADDRESS(_a)) )
#define READ_8(a, r)			( *(volatile uint8_t*) &(r) = *(volatile uint8_t* ) DEVICE_ADDRESS(a) )
#define READ_16(a, r)			( *(volatile uint16_t*)&(r) = *(volatile uint16_t*) DEVICE_ADDRESS(a) )
#define READ_32(a, r)			( *(volatile uint32_t*)&(r) = *(volatile uint32_t*) DEVICE_ADDRESS(a) )

#define WRITE_8( a, r)			( *(volatile uint8_t* )DEVICE_ADDRESS(a) = *(uint8_t* )&(r) )
#define WRITE_16(a, r)			( *(volatile uint16_t*)DEVICE_ADDRESS(a) = *(uint16_t*)&(r) )
#define WRITE_32(a, r)			( *(volatile uint32_t*)DEVICE_ADDRESS(a) = *(uint32_t*)&(r) )

#define READ_I_8(a, i, r)		( *(volatile uint8_t* )&(r) = *((volatile uint8_t* ) DEVICE_ADDRESS(a) + (i)) )
#define READ_I_16(a, i, r)		( *(volatile uint16_t*)&(r) = *((volatile uint16_t*) DEVICE_ADDRESS(a) + (i)) )
#define READ_I_32(a, i, r)		( *(volatile uint32_t*)&(r) = *((volatile uint32_t*) DEVICE_ADDRESS(a) + (i)) )

#define WRITE_I_8( a, i, r )		( *((volatile uint8_t* ) DEVICE_ADDRESS(a) + (i)) = *(uint8_t*) &(r) )
#define WRITE_I_16( a, i, r )	( *((volatile uint16_t*) DEVICE_ADDRESS(a) + (i)) = *(uint16_t*)&(r) )
#define WRITE_I_32( a, i, r )	( *((volatile uint32_t*) DEVICE_ADDRESS(a) + (i)) = *(uint32_t*)&(r) )

#endif

#define BL_READ_32(a,r) READ_32(a,r)
#define BL_WRITE_32(a,r) WRITE_32(a,r)
#define BL_WRITE_I_32( a, i, r ) WRITE_I_32( a, i, r )
#define BL_READ_I_32(a, i, r) READ_I_32(a, i, r)

/* The following group of macros are intended for shared/io memory access
*/

#define MGET_8(a )              ( *(volatile uint8_t* )(a) )
#define MGET_16(a)              swap2bytes( *(volatile uint16_t*)(a) )
#define MGET_32(a)              swap4bytes( *(volatile uint32_t*)(a) )

#define MREAD_8( a, r)			( (r) = MGET_8( a ) )
#define MREAD_16(a, r)			( (r) = MGET_16( a ) )
#define MREAD_32(a, r)			( (r) = MGET_32( a ) )

#define MWRITE_8( a, r )        ( *(volatile uint8_t *)(a) = (uint8_t) (r))
#define MWRITE_16( a, r )       ( *(volatile uint16_t*)(a) = swap2bytes((uint16_t)(r)))
#define MWRITE_32( a, r )       ( *(volatile uint32_t*)(a) = swap4bytes((uint32_t)(r)))

#define MGET_I_8( a, i)         ( *((volatile uint8_t *)(a) + (i)) )
#define MGET_I_16(a, i)         swap2bytes( *((volatile uint16_t*)(a) + (i)) )
#define MGET_I_32(a, i)         swap4bytes( *((volatile uint32_t*)(a) + (i)) )

#define MREAD_I_8( a, i, r)		( (r) = MGET_I_8( (a),(i)) )
#define MREAD_I_16(a, i, r)		( (r) = MGET_I_16((a),(i)) )
#define MREAD_I_32(a, i, r)		( (r) = MGET_I_32((a),(i)) )

#define MWRITE_I_8( a, i, r)    ( *((volatile uint8_t *)(a) + (i)) = (uint8_t)(r) )
#define MWRITE_I_16(a, i, r)    ( *((volatile uint16_t*)(a) + (i)) = swap2bytes((uint16_t)(r)) )
#define MWRITE_I_32(a, i, r)    ( *((volatile uint32_t*)(a) + (i)) = swap4bytes((uint32_t)(r)) )

/* Set block of shared memory to the specified value */
#ifdef _CFE_
#include "lib_types.h"
#include "lib_string.h"
#define MEMSET(a, v, sz)                lib_memset(a, v, sz)
#else
#define MEMSET(a, v, sz)				memset(a, v, sz)
#endif
/* Copy memory block local memory --> shared memory */
#define MWRITE_BLK_8(d, s, sz )      memcpy(d, s, sz)
#define MWRITE_BLK_16(d, s, sz)      { uint32_t i, val; for ( i = 0; i < ( sz / 2 ); i++ ){ val = *((volatile uint16_t*)(s) + (i)); MWRITE_I_16( d, i, val ); } }
#define MWRITE_BLK_32(d, s, sz)      { uint32_t i, val; for ( i = 0; i < ( sz / 4 ); i++ ){ val = *((volatile uint32_t*)(s) + (i)); MWRITE_I_32( d, i, val ); } }

/* Copy memory block shared memory --> local memory */
#define MREAD_BLK_8(d, s, sz )   	    memcpy(d, s, sz)
#define MREAD_BLK_16(d, s, sz)  		memcpy(d, s, sz)
#define MREAD_BLK_32(d, s, sz)  		memcpy(d, s, sz)

#else
 #define WMB() 	/* */
 /* Simulation environment */
 #include <access_macros_fssim.h>

#endif /* #ifdef FSSIM */

/* Bit-field access macros
: v		-  value
: lsbn	- ls_bit_number
: fw	- field_width
: a     - address
: rv	- read_value
 */
#define FIELD_GET(v, lsbn, fw)			( ((v)>>(lsbn)) & ((unsigned)(1 << (fw)) - 1) )

#define FIELD_MGET_32(a, lsbn, fw)		( FIELD_GET( MGET_32(a), (lsbn), (fw)) )
#define FIELD_MGET_16(a, lsbn, fw)      ( FIELD_GET( MGET_16(a), (lsbn), (fw)) )
#define FIELD_MGET_8( a, lsbn, fw)		( FIELD_GET( MGET_8(a) , (lsbn), (fw)) )

#define FIELD_MREAD_8( a, lsbn, fw, rv)	( rv = FIELD_MGET_8( (a),   (lsbn), (fw)) )
#define FIELD_MREAD_16(a, lsbn, fw, rv)	( rv = FIELD_MGET_16((a),   (lsbn), (fw)) )
#define FIELD_MREAD_32(a, lsbn, fw, rv)	( rv = FIELD_MGET_32((a),   (lsbn), (fw)) )

#define SWAPBYTES(buffer,len) do {uint8_t _i; for(_i = 0; _i < len; _i += sizeof(uint32_t)) *((uint32_t *)(&(buffer[_i]))) = swap4bytes(*((uint32_t *)(&(buffer[_i]))));} while(0)

#define FIELD_SET( value, ls_bit_number, field_width, write_value )     \
        do {                                                           \
            uint32_t  mask;                                             \
            mask = ( ( 1U << (field_width) ) - 1 ) << (ls_bit_number);   \
            value &=  ~mask;                                            \
            value |= (write_value) << (ls_bit_number);                  \
        } while(0)

#define FIELD_MWRITE_32( address, ls_bit_number, field_width, write_value )     \
        do {                                                                    \
            uint32_t  current_value = MGET_32(address);            			    \
            FIELD_SET(current_value, ls_bit_number, field_width, write_value ); \
            MWRITE_32(address, current_value);                                  \
        } while(0)

#define FIELD_MWRITE_16( address, ls_bit_number, field_width, write_value)      \
        do{                                                                     \
            uint16_t  current_value = MGET_16(address);                         \
            FIELD_SET(current_value, ls_bit_number, field_width, write_value);  \
            MWRITE_16(address, current_value);                                  \
        } while(0)

#define FIELD_MWRITE_8( address, ls_bit_number, field_width, write_value )      \
        do{                                                                     \
            uint8_t  current_value = MGET_8(address);                           \
            FIELD_SET(current_value, ls_bit_number, field_width, write_value);  \
            MWRITE_8(address, current_value);                                   \
        } while(0)


#define GROUP_MREAD_I_8(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_8))
#define GROUP_MREAD_I_16(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_16))
#define GROUP_MREAD_I_32(group, addr, i, ret) (ret = _rdd_i_read(group, (addr), i, rdd_size_32))

#define GROUP_MREAD_8(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_8))
#define GROUP_MREAD_16(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_16))
#define GROUP_MREAD_32(group, addr, ret) (ret = _rdd_i_read(group, (addr), 0, rdd_size_32))

#define GROUP_FIELD_MREAD_8(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, rdd_size_8))
#define GROUP_FIELD_MREAD_16(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, rdd_size_16))
#define GROUP_FIELD_MREAD_32(group, addr, lsb, width, ret) (ret = _rdd_field_read(group, (addr), lsb, rdd_size_32))

#define GROUP_MWRITE_I_8(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_8)
#define GROUP_MWRITE_I_16(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_16)
#define GROUP_MWRITE_I_32(group, addr, i, val) _rdd_i_write(group, (addr), val, i, rdd_size_32)

#define GROUP_MWRITE_8(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_8)
#define GROUP_MWRITE_16(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_16)
#define GROUP_MWRITE_32(group, addr, val) _rdd_i_write(group, (addr), val, 0, rdd_size_32)

#define GROUP_FIELD_MWRITE_8(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_8)
#define GROUP_FIELD_MWRITE_16(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_16)
#define GROUP_FIELD_MWRITE_32(group, addr, lsb, width, val) _rdd_field_write(group, (addr), val, lsb, width, rdd_size_32)

#endif /* __ACCESS_MACROS_H_INCLUDED */
