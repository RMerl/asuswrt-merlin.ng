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

#ifndef __CMS_PSP_H__
#define __CMS_PSP_H__


/*!\file cms_psp.h
 * \brief Header file for the CMS Persistent Scratch Pad API.
 *  This is in the cms_util library.
 *
 * Persistent storage can be reserved and used by applications on a
 * first-come-first-served basis, meaning if applications request
 * more scratch pad area then available on the system, the later
 * requests will be denied.  
 *
 * On "top-boot" systems, there is 8KB of persistent scratch pad area
 * available.  On "bottom-boot" systems, there is no scratch pad area
 * available.  
 */

#include "cms.h"




/** Write data to persistent scratch pad.
 *
 * This function can be called multiple times for the same key.  If
 * a subsequent call with the same key has a larger data buffer,
 * the region in the scratch pad is grown if there is enough space.
 * If a subsequent call with the same key has a smaller data buffer,
 * the region in the scratch pad is shrunk and the extra data from
 * the previous, larger set is freed.  To delete the key and its
 * storage area from the scratch pad, call this function with the
 * key name with len set to 0.
 *
 * @param key (IN)  A string identifying the scratch pad area.  The key can
 *                  have a maximum of 15 characters.
 * @param buf (IN)  The data to write to the scratch pad.
 * @param bufLen (IN)  Length of Data.
 *
 * @return CmsRet.
 */
CmsRet cmsPsp_set(const char *key, const void *buf, UINT32 bufLen);


/** Read data from persistent scratch pad.
 *
 * @param key (IN)  A string identifying the scratch pad area.  The key can
 *                  have a maximum of 15 characters.
 * @param buf (IN/OUT) User allocates a buffer to hold the read results and
 *                     passes it into this function.  On successful return,
 *                     the buffer contains data that was read.
 * @param bufLen (IN)  Length of given buffer.
 *
 * @return On success, the number of bytes returned to caller.
 *         If the key was not found or there is some other erro, 0 will be returned.
 *         If the key was found but the user provided
 *         buffer is not big enough to hold the data, a negative number will
 *         be returned and the caller's buffer is not modified at all;
 *         the absolute value of the negative number is the 
 *         number of bytes needed to hold the data.
 */
SINT32 cmsPsp_get(const char *key, void *buf, UINT32 bufLen);


/** Get a list of all keys in the persistent scratch pad.
 *
 * @param buf (IN/OUT) User allocates a buffer to hold the read results and
 *                     passes it into this function.  On successful return,
 *                     the buffer contains a list of all keys in the 
 *                     persistent scratch pad.  Each key is terminated by
 *                     a NULL byte.
 *                     
 * @param bufLen (IN)  Length of given buffer.
 *
 * @return On success, the number of bytes returned to caller.
 *         If the persistent scratch pad is empty, 0 will be returned.
 *         If the user provided buffer is not big enough to hold all the keys,
 *         a negative number will be returned and the caller's buffer is not modified at all;
 *         the absolute value of the negative number is the 
 *         number of bytes needed to hold the data.
 */
SINT32 cmsPsp_list(char *buf, UINT32 bufLen);


/** Zeroize all keys from scratch pad area.
 *
 * This has the effect of clearing the entire scratch pad.
 * Use with extreme caution.
 *
 * @return CmsRet enum.
 */
CmsRet cmsPsp_clearAll(void);


#endif /* __CMS_PSP_H__ */
