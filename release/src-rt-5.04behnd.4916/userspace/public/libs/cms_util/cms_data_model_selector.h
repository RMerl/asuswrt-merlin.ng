/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
