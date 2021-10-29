/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
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
 *
 ************************************************************************/

#ifndef __CMS_MATH_H__
#define __CMS_MATH_H__

/*!\file cms_math.h
 * \brief Public header file for arithmetic operations.
 */

#include "cms.h"

SINT32 pointOneMicroWattsTodBm(UINT32 puw);
SINT32 convertPointOneMicroWattsToOmcidB(const char *dir, UINT32 puw,
  SINT32 min, SINT32 max, SINT32 inc, UBOOL8 isdBmW);


#endif /* __CMS_MATH_H__ */
