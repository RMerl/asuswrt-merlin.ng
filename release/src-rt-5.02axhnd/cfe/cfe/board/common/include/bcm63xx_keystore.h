/*
    Copyright 2000-2017 Broadcom Corporation

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#ifndef __BCM63XX_KEY_STORE_H_
#define __BCM63XX_KEY_STORE_H_
#include "lib_byteorder.h"
#include "bcm63xx_sec.h"
//#define CH4_TO_U32(CH4) be32_to_cpu(*((uint32_t*)((char*)#CH4)))
//#define PRINT4CH(CH4) do { board_setleds(CH4_TO_U32(CH4)); }while(0);
//#define PRINTU32(ui) do { board_setleds(ui); }while(0);
 
#define KEY_STORE_AES_256_CBC_SZ 32
#define KEY_STORE_AES_128_CBC_SZ 16
#define KEY_STORE_HASH_SZ 32 
#define KEY_STORE_MID_SZ 2

#define KEY_STORE_MAGIC "BRCMKEYSTORE"
/*Adjusting for \0*/
#define KEY_STORE_MAGIC_SIZE (sizeof(KEY_STORE_MAGIC)-sizeof(char))
#define KEY_STORE_MAGIC_LEN (KEY_STORE_MAGIC_SIZE/sizeof(char))

#define KEY_STORE_SIG_SIZE  (256*sizeof(char))          
#define KEY_STORE_DEF_ABORT_DELAY 1



#if (BOARD_SEC_ARCH == SEC_ARCH_GEN3)

#define KEY_STORE_MAX_KEYS 3 
#define KEY_STORE_MAX_SZ ALIGN(4,(sizeof(key_store_t)+KEY_STORE_MAX_KEYS*(sizeof(key_store_key_info_t)+KEY_STORE_AES_128_CBC_SZ*4+KEY_STORE_HASH_SZ*2+KEY_STORE_MID_SZ*2)))

#elif (BOARD_SEC_ARCH == SEC_ARCH_GEN2)

#define KEY_STORE_MAX_KEYS 2 
#define KEY_STORE_MAX_SZ ALIGN(4,(sizeof(key_store_t)+KEY_STORE_MAX_KEYS*(sizeof(key_store_key_info_t)+KEY_STORE_MID_SZ*2)))
#else
#error Unsupported key store capacity
#endif

/* needs to be in bcm63xx_hwdefs*/
#define KEY_STORE_OFFSET  0xFE000 /* 1016*1024*/      

typedef enum _key_store_err {
        KEY_STORE_ERR_SUCC,
        KEY_STORE_ERR_INVALID,
        KEY_STORE_ERR_KEY_NOT_FOUND
} key_store_err_t;

typedef enum _key_store_req_state {
        KEY_STORE_REQ_NONE=0x0,
        KEY_STORE_REQ_TRANSIT_GEN2_BTRM,
        KEY_STORE_REQ_TRANSIT_GEN2_MFG,
        KEY_STORE_REQ_TRANSIT_GEN2_OP,
        KEY_STORE_REQ_TRANSIT_MFG,
        KEY_STORE_REQ_TRANSIT_FLD,
} key_store_req_state_t;

typedef enum _key_store_data_type {
        KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_EK=0x0,
        KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_IV=0x1,
        KEY_STORE_DATA_TYPE_KEY_AES_CBC_256_EK=0x2,
        KEY_STORE_DATA_TYPE_KEY_AES_CBC_256_IV=0x3,
        KEY_STORE_DATA_TYPE_RSA_PUB=0x4,
        KEY_STORE_DATA_TYPE_RSA_PRIV=0x5,
        KEY_STORE_DATA_TYPE_HASH=0x6,
        KEY_STORE_DATA_TYPE_MID=0x7,
        KEY_STORE_DATA_TYPE_OID=0x8,
        KEY_STORE_DATA_TYPE_MAX=0x9
} key_store_data_type_t;

typedef enum _key_store_data_state {
        KEY_STORE_DATA_STATE_RAW=0x0,
        KEY_STORE_DATA_STATE_MFG_ENCR=0x1,
        KEY_STORE_DATA_STATE_MFG_OEM_ENCR=0x2,
        KEY_STORE_DATA_STATE_FLD_ENCR=0x3,
        KEY_STORE_DATA_STATE_FLD_OEM_ENCR=0x4,
        KEY_STORE_DATA_STATE_MAX=0x5
} key_store_data_state_t;

#define KEY_STORE_STAT_MSK 0xff
#define KEY_STORE_DATA_GET_TYPE(d) (d&KEY_STORE_STAT_MSK)
#define KEY_STORE_DATA_GET_STATE(d) ((d>>8)&KEY_STORE_STAT_MSK)

struct __attribute__((packed)) _key_store_req_info {
        /*This what was requested by build*/
        /*key_store_req_state_t*/
        unsigned char state;
        unsigned char abort_delay;
        /*sec_arch_info_t */
        //unsigned char sec_arch;
};

typedef struct  _key_store_req_info key_store_req_info_t;

struct __attribute__((packed)) _key_store_key_info {
        unsigned short size;
        unsigned short type_state;
        unsigned char data[0];
};

typedef struct _key_store_key_info key_store_key_info_t;

typedef struct __attribute__((packed)) _key_store_key {
        /*crc of the data, sig and the key_info*/
        unsigned int crc;
#if (BOARD_SEC_ARCH == SEC_ARCH_GEN3)
        unsigned char sig[KEY_STORE_SIG_SIZE];
#endif
        key_store_key_info_t info;
} key_store_key_t;

/* Dual purpose:
1. In non-secure mode (verifying via otp) if not-requested to enter to SEC mode
   proceed to default cfe_rom path
   -if requiested an MFG_FLD proceed to MFG enter
   -if requiested an FLD proceed to FLD via MFG
2. If in secure MFG but requested MFG_FLD proceed to FLD
3. If in FLD mode proceed to secure boot  
*/
struct __attribute__((packed)) _key_store_hdr {
        unsigned char magic[KEY_STORE_MAGIC_LEN];
        unsigned char sec_arch;
        key_store_req_info_t req_info;
        unsigned short info_size; 
        /*header crc */
        unsigned int crc; 
};
typedef struct _key_store_hdr key_store_hdr_t; 

typedef struct __attribute__((packed)) _key_store {
        key_store_hdr_t hdr;
        key_store_key_t key;
} key_store_t;

/*
        Reads and verifies key store
*/
key_store_err_t key_store_init(unsigned int sec_state, unsigned int sec_arch, Booter1Args *auth_args, cfe_storage_dev_t* fs);
/*
        Erases keystore block from the flash
*/
key_store_err_t key_store_get_data_info(key_store_data_type_t type, 
                                key_store_data_state_t state,
                                void* data);
/*
key_store_err_t key_store_get_data(void* key_mem, int ref, 
                                void* data);
*/

key_store_err_t key_store_get_req_info(key_store_req_info_t* req_info);
/*
        Erases keystore block from the flash
*/
key_store_err_t key_store_erase(void);

key_store_err_t key_store_data_release(void);

key_store_err_t key_store_reset(void);


#endif
