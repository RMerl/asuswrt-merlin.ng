/***********************************************************************
 *
 *  Copyright(c) 2020 Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
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

#ifndef __CMS_VBUF_H__
#define __CMS_VBUF_H__

#include "cms.h"

/*!\file cms_vbuf.h
 * \brief Header file for serialization buffer.
 *
 */

#define CMS_VBUF_INITIAL_SIZE  2000
#define CMS_VBUF_GROW_SIZE     8000

typedef struct {
    void *data;
    UINT32 maxSize;
    UINT32 size;
    UINT32 index;
} CmsVbuf;

/** Create a new vbuf.
 */
CmsVbuf* cmsVbuf_new(void);

/** Return the current data size in a vbuf.
 */
size_t cmsVbuf_getSize(CmsVbuf *s);


/** Clean up vbuf, including its data.
 */
void cmsVbuf_destroy(CmsVbuf *s);


/** Generic function to put data into vbuf.
 * This function doesn't handle any alignment by itself.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param data    (IN) The pointer to the data to be serialized into vbuf.
 * @param size    (IN) The size of the data to be serialized in bytes.
 */
CmsRet cmsVbuf_put(CmsVbuf *vbuf, const void *data, size_t size);

/** Put a UINT16 into vbuf.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param data    (IN) SINT32 value to be serialized into vbuf.
 */
CmsRet cmsVbuf_putUINT16(CmsVbuf *vbuf, UINT16 data);

/** Put a UINT32 into vbuf.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param data    (IN) UINT32 value to be serialized into vbuf.
 */
CmsRet cmsVbuf_putUINT32(CmsVbuf *vbuf, UINT32 data);

CmsRet cmsVbuf_putUBOOL8(CmsVbuf *vbuf, UBOOL8 data);

/** Put a string into vbuf.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param data    (IN) The string to be serialized into vbuf.
 */
CmsRet cmsVbuf_putString(CmsVbuf *vbuf, const char* data);

/** Reset the index of vbuf to be 0.
 * @param vbuf    (IN) The vbuf we are working on.
 */
void cmsVbuf_resetIndex(CmsVbuf *vbuf);

/** Get a specified number of data from vbuf. The index of the vbuf will be
 *  increased by the size of the get data.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param pData   (OUT) The pointer to the buffer to hold the deserialized data.
 * @param size    (IN) The data to be de-serialized into vbuf.
 */
CmsRet cmsVbuf_get(CmsVbuf *vbuf, void *pData, size_t size);

/** Get a UINT16 from vbuf. The index of the vbuf will be
 *  increased by the size of UINT16.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param pData   (OUT) The pointer to the buffer to hold the deserialized data.
 */
CmsRet cmsVbuf_getUINT16(CmsVbuf *vbuf, UINT16 *pData);

/** Get a UINT32 from vbuf. The index of the vbuf will be
 *  increased by the size of UINT32.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param pData   (OUT) The pointer to the buffer to hold the deserialized data.
 */
CmsRet cmsVbuf_getUINT32(CmsVbuf *vbuf, UINT32 *pData);

CmsRet cmsVbuf_getUBOOL8(CmsVbuf *vbuf, UBOOL8 *pData);

/** Get a string from vbuf. The index of the vbuf will be
 *  increased by the size of UINT32. The caller is responsible to free the
 *  returned string.
 * @param vbuf    (IN) The vbuf we are working on.
 * @param data    (OUT) The pointer to the pointer of the string.
 */
CmsRet cmsVbuf_getString(CmsVbuf *vbuf, char **data);

#endif
