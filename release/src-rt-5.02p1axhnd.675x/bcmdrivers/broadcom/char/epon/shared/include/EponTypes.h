/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/



#if !defined(Types_h)
#define Types_h
////////////////////////////////////////////////////////////////////////////////
/// \file Types.h
/// \brief Arithmetic operation for custom data type
///
/// This file contains functions for performing basic arithmetic and comparison
/// operations on custom integer data type larger than 32 bits wide.
////////////////////////////////////////////////////////////////////////////////

#include "Teknovus.h"
#include "Ethernet.h"


#define MakeBitVecSize(elem)    (((elem)/8) + ((elem)%8 != 0))


//##############################################################################
// U48 Functions
//##############################################################################


////////////////////////////////////////////////////////////////////////////////
/// U48EqU32: Assign U32 value
///
 // Parameters:
/// \param u48 U48 to assign to
/// \param u32 U32 value to assign
////////////////////////////////////////////////////////////////////////////////
extern
void U48EqU32 (U48 *u48, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U48EqU48: Assign U48 value
///
 // Parameters:
/// \param a U48 to assign to
/// \param b U48 to assign
////////////////////////////////////////////////////////////////////////////////
extern
void U48EqU48 (U48 *a, const U48 *b);


////////////////////////////////////////////////////////////////////////////////
/// U48AddEqU32: Add and assign U32
///
 // Parameters:
/// \param a U48 to add and assign to
/// \param b U32 to add
////////////////////////////////////////////////////////////////////////////////
extern
void U48AddEqU32 (U48 *u48, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U48Decrement: Decrement a U48
///
 // Parameters:
/// \u48 U48 to decrement
////////////////////////////////////////////////////////////////////////////////
extern
void U48Decrement (U48 *u48);


////////////////////////////////////////////////////////////////////////////////
/// U48Increment: Increment a U48
///
 // Parameters:
/// \param u48 U48 to increment
////////////////////////////////////////////////////////////////////////////////
extern
void U48Increment (U48 *u48);


////////////////////////////////////////////////////////////////////////////////
/// U48AddEqU48: Add and assign two U48s
///
 // Parameters:
/// \param a U48 to add and assign to
/// \param b U48 to add
////////////////////////////////////////////////////////////////////////////////
extern
void U48AddEqU48 (U48 *a, U48 const *b);


////////////////////////////////////////////////////////////////////////////////
/// U48IsEqU48: Compares to U48s
///
 // Parameters:
/// \param a U48 pointer 1
/// \param b U48 pointer 2
///
/// \return TRUE if they are equal
////////////////////////////////////////////////////////////////////////////////
extern
BOOL U48IsEqU48 (U48 *a, U48 *b);


//##############################################################################
// U64 Functions
//##############################################################################


////////////////////////////////////////////////////////////////////////////////
/// U64EqU32:  Assign U32 to U64
///
 // Parameters:
/// \param u64  Pointer to destination U64
/// \param u32  U32 to add to u64
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64EqU32 (U64* u64, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U64EqU48:  Assign U48 to U64
///
 // Parameters:
/// \param u64  Pointer to destination U64
/// \param u48  U48 to add to u64
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64EqU48 (U64 *u64, const U48 *u48);


////////////////////////////////////////////////////////////////////////////////
/// U64MaxU32:  Assign U32 to U64 if it is greater than the existing value
///
 // Parameters:
/// \param u64  Pointer to destination U64
/// \param u32  U32 to test and set if applicable
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64MaxU32(U64* u64, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U64AddEqU32:  Add U32 to U64
///
 // Parameters:
/// \param u64  Pointer to destination U64
/// \param u32  U32 to add to u64
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64AddEqU32 (U64* u64, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U64AddEqU64:  add two U64s
///
 // Parameters:
/// \param a    Destination U64
/// \param b    U64 to add to a
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64AddEqU64 (U64 BULK* a, U64 const BULK* b);


////////////////////////////////////////////////////////////////////////////////
/// U64SubEqU32:  Sub U32 to U64
///
 // Parameters:
/// \param u64  Pointer to destination U64
/// \param u32  U32 to sub to u64
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64SubEqU32 (U64* u64, U32 u32);


////////////////////////////////////////////////////////////////////////////////
/// U64SubEqU64:  Sub two U64s
///
 // Parameters:
/// \param a    Destination U64
/// \param b    U64 to sub to a
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
extern
void U64SubEqU64 (U64 BULK* a, U64 const BULK* b);


//##############################################################################
// Bit Vector Functions
//##############################################################################


////////////////////////////////////////////////////////////////////////////////
/// BitVecGetVal: Get the value from a bit vector
///
/// This function returns the value of a bit vector at the given index.
///
 // Parameters:
/// \param vec Pointer to bit vector
/// \param index Index into the vector
///
/// \return Value of the vector at the given index
////////////////////////////////////////////////////////////////////////////////
extern
BOOL BitVecGetVal (U8 const BULK *vec, U16 index);


////////////////////////////////////////////////////////////////////////////////
/// BitVecSetVal: Set the value of a bit
///
/// This function sets the value of a given bit in a bit vector.
///
 // Parameters:
/// \param vec Pointer to bit vector
/// \param index Index into the vector
///
/// \return Nothing
////////////////////////////////////////////////////////////////////////////////
extern
void BitVecSetVal (U8 BULK *vec, U16 index);


////////////////////////////////////////////////////////////////////////////////
/// BitVecClrVal: Clear a bit in a bit vector
///
/// This function clears the value of a given bit in a bit vector.
///
 // Parameters:
/// \param vec Pointer to bit vector
/// \param index Index into the vector
///
/// \return Nothing
////////////////////////////////////////////////////////////////////////////////
extern
void BitVecClrVal (U8 BULK *vec, U16 index);



//##############################################################################
// Number to string formatting functions
//##############################################################################


////////////////////////////////////////////////////////////////////////////////
/// UnsignedToHexString - Convert unsigned integer into 0 padded hex string
///
/// This function takes a unsigend value and converts it to a null terminated
/// ASCII text string in hexadecimal padded to the width value characters with
/// 0.  The length of the string minus the null character is returned.
///
/// Parameters:
/// \param buf character buffer to write to
/// \param num number to write
/// \param width number of padding characters
///
/// \return
/// Number of characters written to the string
////////////////////////////////////////////////////////////////////////////////
extern
U8 UnsignedToHexString (char BULK *buf, U32 num, U8 width);


////////////////////////////////////////////////////////////////////////////////
/// UnsignedToDecString - Convert unsigned integer into decimal ASCII string
///
/// This function takes a unsigend value and converts it to a null terminated
/// ASCII text string in decimal.  The length of the string minus the null
/// character is returned.
///
/// Parameters:
/// \param buf character buffer to write to
/// \param num number to write
///
/// \return
/// Number of characters written to the string
////////////////////////////////////////////////////////////////////////////////
extern
U8 UnsignedToDecString (char BULK *buf, U32 num);


////////////////////////////////////////////////////////////////////////////////
/// MacAddrToString - Convert a MAC address into an ASCII string
///
/// This function takes a MAC address value and converts it into a null
/// terminated ASCII string.  The number of characters minus the null character
/// are returned.
///
 // Parameters:
/// \param buf character buffer to write to
/// \param mac MAC address to write
///
/// \return
/// Number of characters written to the string
////////////////////////////////////////////////////////////////////////////////
extern
U8 MacAddrToString (char BULK *buf, MacAddr const *mac);


/* Calculating the Delta between two accumalate counters */
U32 calcStatDelta(U32 base_value, U32 current_value);

#endif // Types_h

