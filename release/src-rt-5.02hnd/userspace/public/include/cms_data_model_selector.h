/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
:>
 *
 ************************************************************************/

#ifndef __CMS_DATA_MODEL_SELECTOR_H__
#define __CMS_DATA_MODEL_SELECTOR_H__


/*!\file cms_data_model_selector.h
 * \brief This API allows proprietary, public, and GPL apps to select
 *        the data model to use.  These functions should only be called
 *        when the code has been compiled in Data Model Detection mode.
 *        These functions do not take effect until after a reboot.
 *        These functions are in the cms_util library.
 *
 */

#include "cms.h"


/** change the data model mode to the other mode */
void cmsUtil_toggleDataModel(void);


/** set the data model mode to PURE181 mode */
void cmsUtil_setDataModelDevice2(void);


/** set the data model mode to Hybrid98+181 mode */
void cmsUtil_clearDataModelDevice2(void);


/** Return TRUE if the data model mode in the Persistent Scratch Pad (PSP)
 *  is currently set to 1.  This function is useful for apps which are
 *  not attached to the MDM.  If the app is already attached to the MDM,
 *  they should call cmsMdm_isDataModelDevice2.
 *
 *  Note this function reads from the PSP.  The setting in the PSP may not
 *  have taken affect yet (may require a reboot first).
 */
UBOOL8 cmsUtil_isDataModelDevice2(void);


#endif /* __CMS_DATA_MODEL_SELECTOR_H__ */
