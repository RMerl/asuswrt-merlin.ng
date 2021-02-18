/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
* 
*    Copyright (c) 2013 Broadcom 
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
/**
 *	@file	 wlcsm_lib_dm_datatype.h
 *	@brief	 wlcsm data modual basic data type definition
 *
 *	Specifify dm required basic data structures,most of them are used in
 *	data module abstract layer and serves for wlmngr
 *
 */

/**-----------------------------------------------------------------------------
 *\addtogroup wlcsm_dm
 * @{
 *-----------------------------------------------------------------------------*/
#ifndef __WLCSM_LIB_DM_DATATYPE_H__
#define __WLCSM_LIB_DM_DATATYPE_H__
#include <stddef.h>
#include <os_defs.h>

typedef enum nwifi_security_mode {
    NWIFI_SECURITY_MODE_OPEN =0,
    NWIFI_SECURITY_MODE_WEP_64, /*  */
    NWIFI_SECURITY_MODE_WEP_128,
    NWIFI_SECURITY_MODE_WPA_Personal,
    NWIFI_SECURITY_MODE_WPA2_Personal,
    NWIFI_SECURITY_MODE_WPA_WPA2_Personal,
    NWIFI_SECURITY_MODE_WPA_Enterprise,
    NWIFI_SECURITY_MODE_WPA2_Enterprise,
    NWIFI_SECURITY_MODE_WPA_WPA2_Enterprise,
} NWIFI_SECURITY_MODE;

/** @brief Basic DM parameter data types
 *
 *  these data types are used for converting from dm type to real runtime data type
 *  being used by wlmngr global data structure.
 *
 *-----------------------------------------------------------------------------*/
typedef enum {
    WLCSM_DT_STRING, /**< string type [char *] */
    WLCSM_DT_UINT,   /**< unsigned int  */
    WLCSM_DT_SINT32, /**< int */
    WLCSM_DT_UINT64,
    WLCSM_DT_SINT64,
    WLCSM_DT_BOOL,
    WLCSM_DT_BOOLREV, /**< bool reverese, the saved value in dm is reversed than in runtime value */
    WLCSM_DT_BASE64,
    WLCSM_DT_HEXBINARY,
    WLCSM_DT_DATETIME,
    WLCSM_DT_STR2INT, /**< DM saved value is string, but runtime is using integer,use mapper to convert */
    WLCSM_DT_INT2STR, /**< DM saved value is int, but runtime is string */
    WLCSM_DT_BOOL2STR,/**< DM saved value is bool, but runtime is string */
} WLCSM_DM_TYPE;

typedef enum {
    MNGR_POS_WIFI,
    MNGR_POS_RADIO,
    MNGR_POS_BSSID,
    MNGR_POS_AP,
    MNGR_POS_AP_WPS,
    MNGR_POS_AP_SEC,
    MNGR_POS_AP_STB,
    MNGR_POS_LAST,
} WLCSM_MNGR_VAR_POS;

#define MNGR_SSID_SPECIFIC_VAR	 0
#define MNGR_SSID_SHARED_VAR	 1
#define MNGR_GENERIC_VAR	 2

typedef struct {
    char            *name;
    unsigned int    offset;
} WLCSM_NAME_OFFSET;

typedef struct {
    char            *name;
    unsigned int    offset;
    WLCSM_DM_TYPE type;
    int   mapper_index;
    char *default_value;
} WLCSM_MNGR_NAME_OFFSET;

#define WLCSM_MNGR_NAME_ENTRY_SIZE(p) (sizeof(p)/sizeof(WLCSM_MNGR_NAME_OFFSET))

typedef struct {
    int    int_value;
    char   *str_value;
    char   *nvram_str_value;
} WLCSM_MNGR_STRMAPPER_SET;

typedef struct {
    WLCSM_NAME_OFFSET dm_set;
    WLCSM_NAME_OFFSET wlmngr_set;
    WLCSM_DM_TYPE type;
    unsigned int  mapper;
} WLCSM_DM_WLMNGR_MAPPING;

#define MAPPING_ENTRY_SIZE(p) (sizeof(p)/sizeof(WLCSM_DM_WLMNGR_MAPPING))

typedef struct {
    char *nvram_var;
    WLCSM_MNGR_VAR_POS pos;
    char *mngr_var;
    int mngr_var_offset;
    WLCSM_DM_TYPE data_type;
    unsigned short type;
    unsigned short mapper;
} WLCSM_NVRAM_MNGR_MAPPING;

typedef struct {
    unsigned int oid;  /**< data model object ID */
    unsigned int mngr_oid; /**< mngr oid,the mngr oid is designed to be the same as TR181 datamodel oid   */
    WLCSM_DM_WLMNGR_MAPPING *mapper; /**< detailed dm objet and mngr object member mapper */
    unsigned int size; /**<mapper structure entries size */
} WLCSM_DM_WLMNGR_OID_MAPPING;

#define WLCSM_DM_WLMNGR_OID_MAPPING_ENTRY_SIZE(p) (sizeof(p)/sizeof(WLCSM_DM_WLMNGR_OID_MAPPING))

typedef struct {
    unsigned int oid;  /**< object ID */
    WLCSM_MNGR_NAME_OFFSET *name_offset; /**< name and offset information */
    unsigned int size; /**<structure entries size */
} WLCSM_DM_OID_MAPPING;

#define WLCSM_DM_OID_MAPPING_ENTRY_SIZE(p) (sizeof(p)/sizeof(WLCSM_DM_OID_MAPPING))
/**-----------------------------------------------------------------------------
 *  @brief a structure to hold  string list
 *
 *  wlmngr will respond to any request for a list of strings with this structure
 *  the first WL_STR_LIST#num is to tell how many strings there is.
 *
 *-----------------------------------------------------------------------------*/
typedef struct {
    int num; /**< how many strings following the number */
    char str_list[]; /**< string list */
} WL_STR_LIST;


#define WLCSM_DM_VAR_POSOFF(s,v) (offsetof(s,v))
#define NUMVAR_VALUE(p, off,type) (*((type *) (((char *)p)+(off)))) /**< number var value in a structure */
#define STRVAR_VALUE(p,off)  (*((char **)((void *)p+off))) /**< string value in a structure  */

/**-----------------------------------------------------------------------------
 * @brief Assign numerical value between source and destination
 *-----------------------------------------------------------------------------*/
#define NUM_SYNC(src,srcoff,dst,dstoff,type) NUMVAR_VALUE(dst,dstoff,type)=NUMVAR_VALUE(src,srcoff,type)

/**-----------------------------------------------------------------------------
 * @brief Assign number value to a datatype of ::WLCSM_DT_BOOLREV
 *-----------------------------------------------------------------------------*/
#define BOOL_REV(src,srcoff,dst,dstoff,type) NUMVAR_VALUE(dst,dstoff,type)=!(NUMVAR_VALUE(src,srcoff,type))


/**-----------------------------------------------------------------------------
 *  @}
 *-----------------------------------------------------------------------------*/
#endif
