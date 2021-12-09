/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

#ifndef _BRCM_RDB_UTIL_H_
#define _BRCM_RDB_UTIL_H_

// Register Access Macros:

typedef unsigned long  UInt32;
typedef unsigned short UInt16;
typedef unsigned char  UInt8;

/*
** The following two macros create a "combine" routine which can be nested.
** Without the nesting you can't call BRCM_CONCAT(BRCM_CONCAT(x,y),z)
*/

#define BRCM_CONCAT(a,b) BRCM_CONCATX(a,b)
#define BRCM_CONCATX(a,b) a ## b

/*
** HELPER MACROS
** These are used to combine names for the actual read and write macros.
*/

#define BRCM_FIELDNAME(r,f)   BRCM_CONCAT(BRCM_CONCAT(r,_),f)

#define BRCM_REGOFS(r)      BRCM_CONCAT(r,_OFFSET)

#define BRCM_REGTYPE(r)     BRCM_CONCAT(r,_TYPE)

#define BRCM_REGADDR(b,r)   ( (b) + ( BRCM_REGOFS(r) ) )

#define BRCM_FIELDMASK(r,f) (BRCM_CONCAT(BRCM_FIELDNAME(r,f),_MASK))

#define BRCM_FIELDSHIFT(r,f)  (BRCM_CONCAT(BRCM_FIELDNAME(r,f),_SHIFT))

/*
** The following macros read and write registers or bit fields.
**
** b is the base address name (NAME Must be generated from RDB rdb2h.pl)
** r is the register name     (NAME Must be generated from RDB rdb2h.pl)
** f is the field name        (NAME Must be generated from RDB rdb2h.pl)
** d is the data to write.
**
** Reserved bits handling is enforced by the macros:
** 1.  Reserved bit must be written to 0.
** 2.  Reserved bits are undefined when read, so masked off to zero.
**
** Note: Compiler optimizes away AND operation when reserved mask is 0.
*/

#define BRCM_READ_REG(b,r)  ( ( *(volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r) &         \
                                ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_WRITE_REG(b,r,d) ( ( *(volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r) ) =      \
                                ( (d) & ~( BRCM_CONCAT( r, _RESERVED_MASK)) ) )

#define BRCM_READ_REG_IDX(b,r,i)  ( ( ((volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r))[i] &  \
                                    ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_WRITE_REG_IDX(b,r,i,d) ( (((volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r))[i] ) \
                                = ( (d) & ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_READ_REG_FIELD(b,r,f)   ( ( BRCM_READ_REG(b,r) & BRCM_FIELDMASK(r,f) ) >> \
                                       BRCM_FIELDSHIFT(r,f) )

#define BRCM_WRITE_REG_FIELD(b,r,f,d)  (BRCM_WRITE_REG(b,r,                            \
                             (( ((d) << BRCM_FIELDSHIFT(r,f)) & BRCM_FIELDMASK(r,f)) | \
                                (BRCM_READ_REG(b,r) & (~BRCM_FIELDMASK(r,f))  ))))

#endif // _BRCM_RDB_UTIL_H_

