/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
 *
 ************************************************************************/


#ifndef __BCM_GENERIC_HAL_UTILS_H__
#define __BCM_GENERIC_HAL_UTILS_H__

/*!\file bcm_generic_hal_utils.h
 * \brief Utility functions associated with the BCM Generic HAL.  These
 *        are so useful that some code which do not use the Generic HAL
 *        will still want to use these functions.
 *
 */

#include "number_defs.h"
#include "bcm_generic_hal_defs.h"

/** Copy an array of BcmGenericParamInfo structs.
 *
 *  It is better to use this function than to manually copy the entry by
 *  doing dst[i] = src[i] because this function allows the caller to 
 *  control whether the pointers are duplicated (deep copy) or transfered
 *  from src to dest (not deep copy).  A manual copy requires additional
 *  fixup code at the calling site and is harder to maintain if the 
 *  BcmGenericParamInfo struct changes.
 *
 *  @param destination entry
 *  @param source entry
 *  @param number of elements to copy
 *  @param deepCopy TRUE: make a copy of the pointers; FALSE: transfer ownership of pointers.
 */
void cmsUtl_copyParamInfoArray(BcmGenericParamInfo *dst,
                               BcmGenericParamInfo *src,
                               UINT32 numParamInfos,
                               UBOOL8 deepCopy);

/** Free an array of BcmGenericParamInfo.
 *
 *  The array and its internal pointers are allocated with cmsMem_alloc,
 *  so it is important that code use this function to free instead of trying
 *  to free with the standard libc free.  This function will also null out
 *  the pointer to the array to prevent accidental use after free.
 *
 *  @param address of the array pointer.
 *  @param number of elements in the array.
 */
void cmsUtl_freeParamInfoArray(BcmGenericParamInfo  **paramInfoArray,
                               UINT32                 numParamInfos);


/** Copy an array of BcmGenericParamAttry structs.
 *  Same comments as cmsUtl_copyParamInfoArray.
 */
void cmsUtl_copyParamAttrArray(BcmGenericParamAttr *dst,
                               BcmGenericParamAttr *src,
                               UINT32 numParamAttrs,
                               UBOOL8 deepCopy);

/** Free an array of BcmGenericParamAttr.
 *  Same comments as cmsUtl_freeParamInfos.
 */
void cmsUtl_freeParamAttrArray(BcmGenericParamAttr  **paramAttrArray,
                               UINT32                 numParamAttrs);


/** Free all strings in the array, and then free the array itself.
 *  Set the pointer to NULL (that is why ***).  Note that the array and the
 *  string buffers are allocated with cmsMem_alloc, so it is important that
 *  code use this function to free instead of trying to free with the standard
 *  libc free.
 */
void cmsUtl_freeArrayOfStrings(char ***array, UINT32 len);


#endif /* __BCM_GENERIC_HAL_UTILS__ */
