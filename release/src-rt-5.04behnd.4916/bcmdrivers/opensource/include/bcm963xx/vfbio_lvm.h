/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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
*/
/*******************************************************************************
 *
 * vfbio_lvm.h    VFBIO LVM user interface
 * June 7, 2022
 * Igor Ternovsky
 *
 *******************************************************************************/
#ifndef _VFBIO_LVM_H_
#define _VFBIO_LVM_H_

#define VFBIO_LVM_ALLOC_BLOCK_SIZE      4096    /* lun size is always defined in these units. DO NOT CHANGE! */
#define VFBIO_LVM_MAX_NAME_SIZE         16      /* Max lun name size, excluding 0-terminatoir. DO NOT CHANGE! */
#define VFBIO_LVM_LUN_ID_AUTOASSIGN     (-1)    /* Constant to be used in vfbuio_lun_create for automatic lun id assignment */
#define VFBIO_LVM_MAX_ID_NAME_PAIRS     16      /* Max number of luns that can be renamed simultaneously */

#define RPMB_BLOCK_SIZE                 (256)   /* Size of RPMB data */
typedef enum
{ 
    VFBIO_RPMB_DMA_FROM_DEVICE,
    VFBIO_RPMB_DMA_TO_DEVICE
}vfbio_rpmb_dma_op; 

typedef enum  {
    VFBIO_RPMB_WRITE_KEY = 0x01,
    VFBIO_RPMB_READ_CNT  = 0x02,
    VFBIO_RPMB_WRITE     = 0x03,
    VFBIO_RPMB_READ      = 0x04,
}rpmb_cmd_type;


/*
 * This structure is shared with OP-TEE and the MMC ioctl layer.
 * It is the "data frame for RPMB access" defined by JEDEC, minus the
 * start and stop bits.
 */
typedef struct  
{
    uint8_t stuff_bytes[196];
    uint8_t key_mac[32];

    uint8_t data[RPMB_BLOCK_SIZE];

    uint8_t nonce[16];
    uint32_t write_counter;
    uint16_t address;
    uint16_t block_count;

    uint16_t op_result;
#define RPMB_RESULT_OK                              0x00
#define RPMB_RESULT_GENERAL_FAILURE                 0x01
#define RPMB_RESULT_AUTH_FAILURE                    0x02
#define RPMB_RESULT_ADDRESS_FAILURE                 0x04
#define RPMB_RESULT_AUTH_KEY_NOT_PROGRAMMED         0x07

    uint16_t msg_type;
#define VFBIO_RPMB_WRITE_KEY                        0x01
#define VFBIO_RPMB_READ_CNT                         0x02
#define VFBIO_RPMB_WRITE                            0x03
#define VFBIO_RPMB_READ                             0x04
#define VFBIO_RPMB_RESP                             0x05
#define RPMB_MSG_TYPE_RESP_AUTH_KEY_PROGRAM         0x0100
#define RPMB_MSG_TYPE_RESP_WRITE_COUNTER_VAL_READ   0x0200
#define RPMB_MSG_TYPE_RESP_AUTH_DATA_WRITE          0x0300
#define RPMB_MSG_TYPE_RESP_AUTH_DATA_READ           0x0400
}rpmb_data_frame_t;

/* vFlash errors */
typedef enum
{
    VFBIO_ERROR_OK = 0,
    VFBIO_ERROR_PARMS = -100,               /* Error in parameters */
    VFBIO_ERROR__FIRST = VFBIO_ERROR_PARMS,
    VFBIO_ERROR_LUN_INVALID = -101,         /* Invalid lun index */
    VFBIO_ERROR_LUN_NOT_OPENED = -102,      /* lun is not opened */
    VFBIO_ERROR_LUN_IDX_EXISTS = -103,      /* lun with such index already exists */
    VFBIO_ERROR_LUN_NAME_EXISTS = -104,     /* lun with such name already exists */
    VFBIO_ERROR_LUN_IS_NOT_DYNAMIC =-105,   /* Attempt to change a static lun */
    VFBIO_ERROR_NO_MORE_LUNS = -106,        /* No more luns (for an iterator) */
    VFBIO_ERROR_TRANSLATION = -107,         /* Address translation error */
    VFBIO_ERROR_NO_MEM = -108,              /* Dynamic memory allocation failed */
    VFBIO_ERROR_NO_ROOM = -109,             /* No room on flash */
    VFBIO_ERROR_NO_WRITE_PERMISISON = -110, /* No write permission */
    VFBIO_ERROR_NO_READ_PERMISISON = -111,  /* No read permission */
    VFBIO_ERROR_INTERNAL = -112,            /* Internal error */
    VFBIO_ERROR_INVALID_DEVICE = -113,      /* Invalid device index */
    VFBIO_ERROR_INVALID_DEVICE_PAGE = -114, /* Invalid device page */
    VFBIO_ERROR_NO_LVM_LUN = -115,       /* No lun for for saving dynamic configuration */
    VFBIO_ERROR_CANT_SAVE_CONFIG = -116,    /* Can't save dynamic lun configuration */
    VFBIO_ERROR_CANT_LOAD_CONFIG = -117,    /* Can't load dynamic lun configuration */
    VFBIO_ERROR_LVM_IS_NOT_SUPPORTED= -118, /* LVM is not supported */
} vfbio_error;

/* LUN flags */
#define VFBIO_LUN_CREATE_FLAG_NONE       0x00
#define VFBIO_LUN_CREATE_FLAG_READ_ONLY  0x01
#define VFBIO_LUN_CREATE_FLAG_ENCRYPTED  0x04
#define VFBIO_LUN_CREATE_FLAG_HIDDEN     0x08

/* Get error text */
const char *vfbio_error_str(vfbio_error err);

/* Create a new dynamic lun
 * @param[in]  name         0-terminated name up to 16 bytes long, must be unique
 * @param[in]  size         size bytes
 * @param[in]  lun_flags    LUN flags. A combination of VFBIO_LUN_CREATE_FLAG_XX constants
 * @param[out] lun_id       LUN id
 * @return 0 if successful or error code
 */
int vfbio_lun_create(const char *name, uint64_t size, uint32_t lun_flags, int *lun_id);

/* Delete dynamic lun created by vfbio_lun_create
 * @param[out] lun_id       LUN id
 * @return 0 if successful or error code
 */
int vfbio_lun_delete(int lun_id);

/* Resize dynamic lun
 * @param[out] lun_id       LUN id
 * @param[in]  size         new size bytes
 * @return 0 if successful or error code
 */
int vfbio_lun_resize(int lun_id, uint64_t size);

/* lun id-name pair */
typedef struct vfbio_lun_id_name
{
    int lun_id;
    char lun_name[VFBIO_LVM_MAX_NAME_SIZE];
} vfbio_lun_id_name;

/* Rename 1 or multiple luns atomically (all or nothing)
 * @param[in]  num_lunss  number of entries in id_name array
 * @param[in]  id_name    array of { lun id, lun name } pairs
 * @return 0 if successful or error code
 */
int vfbio_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[]);

/* Set lun access type (read-write or read-only)
 * @param[out] lun_id    LUN id
 * @param[in]  read_only    0=read-write, !=0=read-only
 * @return 0 if successful or error code
 */
int vfbio_lun_chmod(int lun_id, int read_only);

/* lun information record */
typedef struct vfbio_lun_descr {
    int lun_id;
    char lun_name[VFBIO_LVM_MAX_NAME_SIZE];
    uint32_t block_size;
    uint32_t size_in_blocks;
    int read_only;
    int dynamic;
    int encrypted;
} vfbio_lun_descr;

/* Get lun info
 * @param[in]  lun_id       lun id
 * @param[out] lun_info
 * @return 0 if successful or error code
 */
int vfbio_lun_get_info(int lun_id, struct vfbio_lun_descr *lun_descr);

/* Get device info
 * @param[out] total_size   Total number of 4K blocks
 * @param[out] free_size    Number of free 4K blocks
 * @return 0 if successful or error code
 */
int vfbio_device_get_info(uint64_t *total_size, uint64_t *free_size);

/* Get lun id by name
 * @param[in]  lun_name  LUN name
 * @param[out] id           LUN id
 * @return 0 if successful or error code
 */
int vfbio_lun_get_id(const char *name, int *id);

#define VFBIO_LUN_ID_GET_FIRST (-1)

/* LUN iterator.
 * Get next lun id
 * @param[in]  id_prev      Previous lun id. Use -1 to get first
 * @param[out] id           LUN id
 * @return 0 if successful, -ENODEV if prev is the last or error code
 */
int vfbio_lun_get_next(int prev, int *lun_id);

/* Write block into lun using a syncronous RPC.
 * It is caller's responsibility to query the block size for the LUN in advance.
 * @param[in]  lun_id       LUN id
 * @param[in]  data
 * @param[in]  size         size in bytes
 * @return 0 if successful or error < 0
 */
int vfbio_lun_write(int lun_id, void *data, uint32_t size);


/* Write RPMB block into lun using a syncronous RPC.
 * It is caller's responsibility to query the block size for the LUN in advance.
 * @param[in]  data                 RPMB data
 * @param[in]  data_size            size in bytes
 * @param[in]  cmd                  RPMB command
 * @param[in]  OP                   from/to LUN
 * @param[in]  nfrm                 Number of the frame  
 * @return 0 if successful or error < 0
 */
int vfbio_rpmb_transfer_data(void *data,  uint32_t data_size, rpmb_cmd_type cmd, vfbio_rpmb_dma_op dma_op, unsigned int nfrm);


#endif

