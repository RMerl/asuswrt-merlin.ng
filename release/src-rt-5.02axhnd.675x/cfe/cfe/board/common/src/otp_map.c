/* 
    Copyright 2000-2019 Broadcom Corporation

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
/**********************************************************************
 *	
 * otp_map.c			 
 * simple otp call map 
 * Includes both secure and non-secure otp operations 
 *
 *	
 *********************************************************************	
 */

#include "rom_main.h"
#include "bcm_otp.h"
#include "lib_byteorder.h"
#include "lib_crc.h"
#include "initdata.h"
#include "bcm63xx_storage_dev.h"
#include "bcm63xx_keystore.h"
#include "bcm63xx_sec.h"
#include "bcm63xx_sotp.h"
#include "bcm63xx_potp_sotp_util.h"

#define COMMIT_OTP
/*#define DEBUG_OTP_MAP*/

typedef struct __otp_obj_map_{
                  uint32_t row;
                  uint32_t shift;
                  uint32_t msk;
                  uint32_t sec;
                  uint32_t len;
                  uint32_t ecc;
                  void*    opt;
              } otp_obj_map_t;
#if defined (_BCM96878_)
/* SKO 0 only support for now*/
typedef enum _SKO {
    SKO_0 = 0,
    SKO_1,
    SKO_MAX
}sko_t;
typedef enum _SKO_TYPE {
    SKO_DESC = 0,
    SKO_CTL,
    SKO_TYPE_MAX
}sko_row_type_t;
/* Descriptor fields for  SKO objects 
Row:42 SKO_0 desc=0x9B009000 :(ecc which calcalated at run-time) 
7-bit_ECC=0x46, VALID=1, Rsvd=2'b0, WLOCK=1, RLOCK=1, SEND_ECC=0, ECC_EN=1, FOUT=1, 12-bit_LAST_PTR=0x9, 12-bit_FIRST_PTR=0x0
Row 46 CRC
*/
otp_obj_map_t sko_obj_desc[SKO_MAX][SKO_TYPE_MAX] = {{{42, 0x0, 0xffffffff, 0, 1, 0, (void*)0x9b009000}, 
                                           {46, 0x0, 0xffffffff, 0, 1, 0, (void*)0x80000000}},
                                           {{0},{0}}};
/*
 * This a debug SKO_1 if needed for the test
Row:43 SKO_1 desc=0x9B01300A : 7-bit_ECC=0x4E, VALID=1, Rsvd=2'b0, WLOCK=1, RLOCK=1, SEND_ECC=0, ECC_EN=1, FOUT=1, 12-bit_LAST_PTR=0x13,12-bit_FIRST_PTR=0xA
 *
*
    {{43, 0x0, 0xffffffff, 0, 1, 0, (void*)0x8101300A}, 
                                           {56, 0x0, 0xffffffff, 0, 1, 0, (void*)0x80000000}}
*/
/* Testing/debug
 * sko_obj_t sko_obj_desc[SKO_MAX][2] = {{{42, 0x99009000, 0x1e}, {46, 0x80000000, 0x0}}, {{0},{0}} };
 data ecc is on key_obj_desc_t sko_obj_desc[SKO_MAX][2] = {{{42, 0x9B009000, 0x46}, {46, 0x80000000, 0x0}}, {0} };*/


#endif

static inline uint32_t _bit(uint32_t v, uint32_t no) {
		return ((v >> no)&0x1);
}
static inline uint8_t ecc7(uint32_t v) 
{
		return (uint8_t) ((_bit(v,0)  ^ _bit(v,1)  ^ _bit(v,2)  ^ _bit(v,3)  ^ _bit(v,4)  ^ _bit(v,5)  ^ _bit(v,6)  ^ _bit(v,7)  ^ _bit(v,14) ^ _bit(v,19) ^ _bit(v,22) ^ _bit(v,24) ^ _bit(v,30) ^ _bit(v,31) ) |
          ((_bit(v,4)  ^ _bit(v,7)  ^ _bit(v,8)  ^ _bit(v,9)  ^ _bit(v,10) ^ _bit(v,11) ^ _bit(v,12) ^ _bit(v,13) ^ _bit(v,14) ^ _bit(v,15) ^ _bit(v,18) ^ _bit(v,21) ^ _bit(v,24) ^ _bit(v,29))<<0x1) |
          ((_bit(v,3)  ^ _bit(v,11) ^ _bit(v,16) ^ _bit(v,17) ^ _bit(v,18) ^ _bit(v,19) ^ _bit(v,20) ^ _bit(v,21) ^ _bit(v,22) ^ _bit(v,23) ^ _bit(v,26) ^ _bit(v,27) ^ _bit(v,29) ^ _bit(v,30))<<0x2) |
          ((_bit(v,2)  ^ _bit(v,6)  ^ _bit(v,10) ^ _bit(v,13) ^ _bit(v,15) ^ _bit(v,16) ^ _bit(v,24) ^ _bit(v,25) ^ _bit(v,26) ^ _bit(v,27) ^ _bit(v,28) ^ _bit(v,29) ^ _bit(v,30) ^ _bit(v,31))<<0x3) |
          ((_bit(v,1)  ^ _bit(v,2)  ^ _bit(v,5)  ^ _bit(v,7)  ^ _bit(v,9)  ^ _bit(v,12) ^ _bit(v,15) ^ _bit(v,20) ^ _bit(v,21) ^ _bit(v,22) ^ _bit(v,23) ^ _bit(v,25) ^ _bit(v,26) ^ _bit(v,28))<<0x4) |
          ((_bit(v,0)  ^ _bit(v,5)  ^ _bit(v,6)  ^ _bit(v,8)  ^ _bit(v,12) ^ _bit(v,13) ^ _bit(v,14) ^ _bit(v,16) ^ _bit(v,17) ^ _bit(v,18) ^ _bit(v,19) ^ _bit(v,20) ^ _bit(v,28))<<0x5) |
          ((_bit(v,0)  ^ _bit(v,1)  ^ _bit(v,3)  ^ _bit(v,4)  ^ _bit(v,8)  ^ _bit(v,9)  ^ _bit(v,10) ^ _bit(v,11) ^ _bit(v,17) ^ _bit(v,23) ^ _bit(v,25) ^ _bit(v,27) ^ _bit(v,31))<<0x6));
};
static otp_obj_map_t _otp_obj_map[KEY_STORE_DATA_TYPE_MAX] = {
#if  (BOARD_SEC_ARCH==SEC_ARCH_GEN3)
#if defined (_BCM96878_)
          {47, 0x0, 0xffffffff, 0, 8, 0, &sko_obj_desc[SKO_0]},
          {0},
          {0},
          {0},
          {0},
          {0},
          {OTP_BOOT_SW_0, 0x0, 0xffffffff, 0, 8},
          {OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_SHIFT, OTP_CUST_MFG_MRKTID_MASK, 0, 1}, 
          {0},
          {OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, 0, 1},
          {OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, OTP_CUST_BTRM_BOOT_ENABLE_MASK, 0, 1}
#else
          {8, 0, 0, 1, 8}, 
          {0},
          {0}, 
          {0}, 
          {0}, 
          {0},
          {9, 0, 0, 1, 8},
          {OTP_CUST_MFG_MRKTID_ROW, OTP_CUST_MFG_MRKTID_SHIFT, OTP_CUST_MFG_MRKTID_MASK, 0, 1}, 
          {0}, 
          {OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT,OTP_BRCM_BTRM_BOOT_ENABLE_MASK, 0, 1},
          {OTP_CUST_BTRM_BOOT_ENABLE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT,OTP_CUST_BTRM_BOOT_ENABLE_MASK, 0, 1}
#endif

#else
          {0}, 
          {0},
          {0}, 
          {0}, 
          {0}, 
          {0},
          {0},
          {OTP_CUST_MFG_MRKTID_ROW,OTP_CUST_MFG_MRKTID_SHIFT,OTP_CUST_MFG_MRKTID_MASK, 0, 1}, 
          {OTP_CUST_OP_MRKTID_ROW, OTP_CUST_OP_MRKTID_SHIFT, OTP_CUST_OP_MRKTID_MASK, 0, 1}, 
          {0},
          {OTP_BRCM_BTRM_BOOT_ENABLE_ROW, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT, OTP_BRCM_BTRM_BOOT_ENABLE_MASK, 0, 1},
          {OTP_CUST_BTRM_BOOT_ENABLE_FUSE_ROW, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT, OTP_CUST_BTRM_BOOT_ENABLE_MASK, 0, 1}

#endif
};

int __sotp_op(uint32_t* obj, otp_obj_map_t *map, int dir)
{
#if !defined (_BCM96878_)
    return dir? sotpWriteSecKey(map->row, obj, map->len): sotpReadSecKey(map->row, obj, map->len);
#else
    return -1;
#endif
}

static inline int __get_row(uint32_t* val, otp_obj_map_t *map, int row)
{
        uint32_t data = 0; 
	int res = bcm_otp_get_row(row, &data);
        if (!res) {
            *val = ((data & map->msk) >> map->shift);
        }
        return res; 
}

static inline int __fuse_row(uint32_t* val, otp_obj_map_t *map, int row)
{
#ifdef DEBUG_OTP_MAP
        xprintf("row: %d data: 0x%x \n", 
             (row), ((*val << map->shift) & map->msk));
#endif
#ifdef COMMIT_OTP
        return bcm_otp_fuse_row(row, ((*val << map->shift) & map->msk));
#else
        return 0;
#endif
}
static inline int __fuse_row_ecc(uint32_t* val, otp_obj_map_t *map, int row, uint32_t ecc)
{
        ecc = ecc7(ecc);
#ifdef DEBUG_OTP_MAP
        xprintf("row: %d data: 0x%x ecc: 0x%x\n", 
             (row), ((*val << map->shift) & map->msk), ecc);
#endif
#ifdef COMMIT_OTP
        return bcm_otp_fuse_row_ecc(row, ((*val << map->shift) & map->msk), ecc);
#else
        return 0;
#endif
}

static inline uint32_t __get_crc32(uint32_t v, uint32_t crc)
{
        v = ntohl(v);
	return lib_get_crc32((uint8_t*)&v, 4, crc);
}
 
#if defined (_BCM96878_)
static inline int __fuse_sko_rows_ecc(uint32_t* obj, otp_obj_map_t *map)
{
        int i;
        otp_obj_map_t *sko_ctl =  &((otp_obj_map_t*)map->opt)[SKO_CTL],
                     *sko_desc = &((otp_obj_map_t*)map->opt)[SKO_DESC];
	uint32_t crc = CRC32_INIT_VALUE;
        /* order is important; */
        /* 1. Fuse control data for an SKO */
        if (__fuse_row_ecc((uint32_t*)&sko_ctl->opt, 
            sko_ctl, sko_ctl->row, (uint32_t)sko_ctl->opt)) {
            goto err; 
        }
        crc = __get_crc32((uint32_t)sko_ctl->opt, crc);
        /* otp data*/
        for (i = 0; i < map->len; i++) {
            if (__fuse_row_ecc(&obj[i], map, map->row + i, obj[i])) {
               goto err;
            }
            crc = __get_crc32(obj[i], crc);
        }
        crc = (~crc);
        if (__fuse_row_ecc(&crc, map, i, crc)) {
           goto err;
        }
#ifdef DEBUG_OTP_MAP
        xprintf ("final crc 0x%x\n",crc);
#endif
        /* descriptor;  must be written at last; it may enable data ecc check  and WR LOCK  */
        if (__fuse_row_ecc((uint32_t*)&sko_desc->opt, sko_desc, 
             sko_desc->row, (uint32_t)sko_desc->opt)) {
            goto err; 
        }
        return 0; 
err:
        return -1;
}
#endif
static inline int __otp_op(uint32_t* obj, otp_obj_map_t *map, int dir)
{   
    int i;
    if (map->opt && dir) {
#if defined (_BCM96878_)
        if (__fuse_sko_rows_ecc(obj, map)) {
               goto err;
        }
#else
        goto err;
#endif
    } else {
        int (*__f)(uint32_t*, otp_obj_map_t* ,int ) = 
              dir? __fuse_row : __get_row;
        for (i = 0; i < map->len; i++) {
            if (__f(&obj[i], map, map->row + i)) {
               goto err;
            }
        }
    }
    return 0; 
err:
    return -1;
}

static int _otp_op(uint32_t* obj, otp_obj_map_t *map, int dir)
{
    return map->sec? __sotp_op(obj, map, dir) : __otp_op(obj, map, dir);
}



static inline int __otp_map_op(void* data, key_store_data_type_t data_type, int dir)
{
    switch (data_type) {
        case  KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_EK:
        case  KEY_STORE_DATA_TYPE_HASH:
        case  KEY_STORE_DATA_TYPE_MID:
        case  KEY_STORE_DATA_TYPE_OID:
        case  KEY_STORE_DATA_TYPE_CUST_BROM_MODE:
        case  KEY_STORE_DATA_TYPE_BROM_MODE:
            break;
        default:
            goto err;
    }
    if (_otp_op((uint32_t*)data, &_otp_obj_map[data_type], dir)) {
        goto err;
    }
    return 0;
err:
    return -1;
}

int otp_map_write(void* data, key_store_data_type_t data_type)
{
    return __otp_map_op((uint32_t*)data, data_type, 1);
}

int otp_map_read(void* data, key_store_data_type_t data_type)
{
    return __otp_map_op((uint32_t*)data, data_type, 0);
}
