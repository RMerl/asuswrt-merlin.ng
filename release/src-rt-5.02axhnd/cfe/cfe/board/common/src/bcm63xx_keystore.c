/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */

#include "rom_main.h"
#include "rom_parms.h"
#include "lib_crc.h"
#include "bcm_memory.h"
#include "bcm63xx_storage_dev.h"
#include "bcm63xx_keystore.h"
#include "bcm_sec.h"
#if !defined (CFG_RAMAPP)
#define _VA(arg) ((void*)arg)
#define GET_BUF MEMORY_AREA_HEAP_ADDR
#define GET_BUF_SIZE MEMORY_AREA_HEAP_SIZE 
#define RELEASE_BUF(arg)
#else
#define GET_BUF KMALLOC(SZ_16K,sizeof(void*))
#define GET_BUF_SIZE SZ_16K 
#define RELEASE_BUF(arg)  KFREE(arg)
#endif


enum key_store_obj_state {
        KEY_STORE_OBJ_INACTIVE = 0,
        KEY_STORE_OBJ_INVALID = -1,
        KEY_STORE_OBJ_OK=1
};

static struct _key_store_obj {
        key_store_t key_store;
        unsigned int key_store_sz;
        unsigned int sec_state;
        enum key_store_obj_state state;
        /* data buffer allocated/set in gen_info call*/
        void* mem;
        void* _mptr;
        void* _mem; 
        Booter1Args* auth_args; 
        cfe_storage_dev_t *fs;
} _obj;
/* trivial memory manager list*/
void* _get_buf(unsigned int size);
void* _get_buf(unsigned int size)
{
       _obj._mem = _obj._mptr;
       _obj._mptr = (void*)ALIGN(sizeof(uintptr_t),((uintptr_t)_obj._mptr+size));
      return _obj._mem;
}

void _resize_buf(void* buf,unsigned int size);
void _resize_buf(void* buf,unsigned int size)
{
      if (buf !=_obj._mem) {
          return;
      }
      _obj._mptr = (void*)ALIGN(sizeof(uintptr_t),((uintptr_t)_obj._mem+size));
}

static inline void _release_buf(void* buf)
{
      if (buf == _obj._mem) {
          _obj._mptr = _obj._mem;
      }
}

static inline key_store_err_t _init_buf(void)
{
        if (GET_BUF_SIZE < KEY_STORE_MAX_SZ) {
            return KEY_STORE_ERR_INVALID; 
        }
        _obj._mptr = GET_BUF;
        _obj._mem = GET_BUF;
        _obj.mem = NULL;
        return KEY_STORE_ERR_SUCC; 
}

static inline key_store_err_t _type2size(key_store_data_type_t data_type, unsigned int *size)
{
        unsigned int sz;
        switch(data_type) {
                case KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_IV:
                case KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_EK:
                        sz = KEY_STORE_AES_128_CBC_SZ;
                        break;  
                case KEY_STORE_DATA_TYPE_KEY_AES_CBC_256_EK:
                case KEY_STORE_DATA_TYPE_KEY_AES_CBC_256_IV:
                        sz = KEY_STORE_AES_256_CBC_SZ;
                        break;  
                case KEY_STORE_DATA_TYPE_RSA_PUB:
                case KEY_STORE_DATA_TYPE_HASH:
                        sz = KEY_STORE_HASH_SZ;
                        break;  
                case KEY_STORE_DATA_TYPE_MID:
                case KEY_STORE_DATA_TYPE_OID:
                        sz = KEY_STORE_MID_SZ;
                        break;  
                default:
                        return  KEY_STORE_ERR_INVALID;
        }
        *size = sz;
        return  KEY_STORE_ERR_SUCC;
}

#if (BOARD_SEC_ARCH == SEC_ARCH_GEN2)
static key_store_err_t key_store_verify(key_store_t* key_store,
                                        unsigned int sec_state,
                                        /*Sizeof of the object without signature*/
                                        Booter1AuthArgs* auth)
{
        key_store_req_state_t sec_state_req = key_store->hdr.req_info.state;
                        /*crc verification of data and keys with signature */
        switch(sec_state_req) {
                case KEY_STORE_REQ_TRANSIT_GEN2_MFG:
                case KEY_STORE_REQ_TRANSIT_GEN2_OP:
                        switch (sec_state) {
                                case SEC_STATE_UNSEC:
                                case SEC_STATE_GEN2_BTRM:
                                case SEC_STATE_GEN2_MFG:
                                        if (key_store->hdr.info_size > sizeof(uint32_t) && key_store->key.crc == 
                                                        lib_get_crc32((uint8_t*)(&key_store->key.crc)+sizeof(key_store->key.crc),
                                                        key_store->hdr.info_size-sizeof(key_store->key.crc), CRC32_INIT_VALUE)) {
                                                goto sccs;
                                        }
                                        PRINT4CH(KCRC);
                                default:
                                        break;
                        }
                default:
                        break;
        }
        PRINT4CH(EVER);
        return  KEY_STORE_ERR_INVALID;
sccs:
        return  KEY_STORE_ERR_SUCC;
}

#elif (BOARD_SEC_ARCH == SEC_ARCH_GEN3)
static key_store_err_t key_store_verify(key_store_t* key_store,
                                        unsigned int curr_sec_state,
                                        /*Sizeof of the object without signature*/
                                        Booter1AuthArgs* auth)
{
        key_store_req_state_t sec_state_req = key_store->hdr.req_info.state;
        switch(sec_state_req) {
                case KEY_STORE_REQ_TRANSIT_FLD: 
                        if (curr_sec_state == SEC_STATE_GEN3_MFG) {
                        /*signature followed by key_info_t - authenticated */ 
                                unsigned char* mem = (unsigned char*)_get_buf(key_store->hdr.info_size-sizeof(key_store->key.crc));
                                memcpy(mem, key_store->key.sig, key_store->hdr.info_size-sizeof(key_store->key.crc));
                                authenticate(mem, key_store->hdr.info_size-sizeof(key_store->key.crc), auth->manu);
                                _release_buf(mem);
                                PRINT4CH(KATH);
                        } else if (curr_sec_state == SEC_STATE_UNSEC) {
                                if (key_store->hdr.info_size < sizeof(uint32_t) || 
                                key_store->key.crc != lib_get_crc32(((uint8_t*)&key_store->key.crc)+sizeof(key_store->key.crc), 
                                        key_store->hdr.info_size-sizeof(key_store->key.crc), CRC32_INIT_VALUE)) {
                                                                        goto err;
                                }
                        }
                        break;  
                case KEY_STORE_REQ_TRANSIT_MFG:
                        if (key_store->hdr.info_size != 0 || curr_sec_state != SEC_STATE_UNSEC) {
                                goto err;
                        }
                        break;
                default:
                        goto err;
        }
        return  KEY_STORE_ERR_SUCC;
err:
        PRINT4CH(EVER);
        return  KEY_STORE_ERR_INVALID;
}

#else
#error "Not implemented"
#endif
/*
        Reads and verifies key store
*/
key_store_err_t key_store_init(sec_state_t sec_state, 
                unsigned int sec_arch, 
                Booter1Args *auth_args,
                cfe_storage_dev_t* fs)
{
        /*
          Read flash block at offset 724*1024  
          Verify if valid header is valid 
        */
        int res;
        key_store_hdr_t *hdr = NULL;
        unsigned char *mem = NULL;
        unsigned int err_code = CH4_TO_U32(EINI);
        
        if (_obj.state  == KEY_STORE_OBJ_OK) {
                return KEY_STORE_ERR_SUCC; 
        }
        if (_obj.state == KEY_STORE_OBJ_INVALID) {      
                return KEY_STORE_ERR_INVALID; 
        }
        if (_init_buf()) {
            goto err;
        }
        mem = (unsigned char*)_get_buf(sizeof(key_store_hdr_t));
        res = fs->read_raw(KEY_STORE_OFFSET, sizeof(key_store_hdr_t), mem);

        if (res < 0) {
                goto err;
        }
        hdr = (key_store_hdr_t*)mem;
        if (hdr->crc != lib_get_crc32(mem, 
                        sizeof(key_store_hdr_t)-sizeof((*hdr).crc),
                        CRC32_INIT_VALUE)) {
                err_code = CH4_TO_U32(EHCR);
                goto err;
        }

        if (hdr->info_size + sizeof(key_store_hdr_t) > KEY_STORE_MAX_SZ) {
                err_code = CH4_TO_U32(ESIZ);
                goto err;
        }

        if (memcmp(hdr->magic, KEY_STORE_MAGIC, KEY_STORE_MAGIC_SIZE)) {
                err_code = CH4_TO_U32(EMGC);
                goto err;
        }


        /* we've compiled in with predefined sec arch support
        - GEN1
        - GEN2
        */
        if (sec_arch != hdr->sec_arch) {
                err_code = CH4_TO_U32(EARC);
                goto err;
        }
        /*Reading an entire keystore */
        _resize_buf(mem, hdr->info_size+sizeof(key_store_hdr_t));
        res = fs->read_raw(KEY_STORE_OFFSET, 
                        hdr->info_size+sizeof(key_store_hdr_t), mem);
        if (res < 0) {
                goto err;
        }
        /* if booted in non-sec mode - verify data crc */
        if (key_store_verify((key_store_t*)mem, sec_state, &auth_args->authArgs)) {
                goto err;
        }
        memcpy(&_obj.key_store.hdr, mem, sizeof(key_store_hdr_t));
        _obj.sec_state = sec_state;
        _obj.auth_args = auth_args;
        _obj.fs = fs;
        _obj.state = KEY_STORE_OBJ_OK;
        _release_buf(mem);
        return KEY_STORE_ERR_SUCC; 
err:
        if (mem) {
                _release_buf(mem);
        }
        PRINTU32(err_code);
        _obj.state = KEY_STORE_OBJ_INVALID;
        return KEY_STORE_ERR_INVALID;
}

static key_store_err_t key_store_get_data(key_store_t* key_store_mem,
                                        key_store_key_info_t* key_info,
                                        Booter1EncrArgs* enc,
                                        void* data)
{
        /*Copy data to the destination*/
        unsigned int key_sz, 
                        ecode=CH4_TO_U32(EKEY);
        key_store_hdr_t *hdr;
        unsigned char *mem;

        hdr = &key_store_mem->hdr;
        /*key_store_mem was copied to heap; pointing to the first byte after end*/
        mem = (unsigned char*)key_store_mem +sizeof(key_store_hdr_t) + hdr->info_size; 
        if (_type2size(KEY_STORE_DATA_GET_TYPE(key_info->type_state), &key_sz)) {
                ecode = CH4_TO_U32(EKTP);
                goto err;
        }

        if (&key_info->data[0] + key_info->size > mem) { 
                ecode = CH4_TO_U32(EKSZ);
                goto err;
        }

        switch(KEY_STORE_DATA_GET_STATE(key_info->type_state)) {
#if (BOARD_SEC_ARCH == SEC_ARCH_GEN3)
                case KEY_STORE_DATA_STATE_FLD_ENCR:
                        /*copying IV to avoid overwrite*/
                        memcpy(mem, enc->biv, CIPHER_IV_LEN);
                        /*Decrypting */
                        decryptWithEk(mem+CIPHER_IV_LEN,/*dst*/ 
                                        &key_info->data[0],/*src*/
                                        enc->bek,/*ek*/
                                        key_info->size, /*data len*/
                                        mem); /*iv*/ 
                        /*key_sz is expected size of the key per type*/
                        memcpy(data, mem+CIPHER_IV_LEN, key_sz);
                        memset(mem, 0, key_sz+CIPHER_IV_LEN);
                        break;
#endif
                case KEY_STORE_DATA_STATE_RAW:
                        memcpy(data, &key_info->data[0], key_sz);
                        break;
                default:
                        goto err;
        }
        return KEY_STORE_ERR_SUCC; 
err:
        PRINTU32(ecode);
        return KEY_STORE_ERR_INVALID; 
}
/* returned pointer must be released in non CFE_ROM mode and 
        keystore_data_release needs to be called when no more keys are not needed
*/
key_store_err_t key_store_get_data_info(key_store_data_type_t type, 
                                key_store_data_state_t state,
                                void* data)
{
        /*Copy data to the destination*/
        key_store_hdr_t *hdr;
        key_store_key_info_t *key_info;
        uint8_t *key_info_max;
        unsigned char *mem;
        if (_obj.state != KEY_STORE_OBJ_OK) {
                return KEY_STORE_ERR_INVALID; 
        }
        hdr = &_obj.key_store.hdr;
        if (!_obj.mem) {        
                mem = _get_buf(sizeof(key_store_hdr_t)+hdr->info_size);
                if (!mem) {
                        goto err;
                }
                _obj.mem = mem;
                if (_obj.fs->read_raw(KEY_STORE_OFFSET, 
                                sizeof(key_store_hdr_t)+hdr->info_size, mem) < 0) {
        
                        goto err1;
                }
        } else {
                mem = _obj.mem;
        }
        key_info = &((key_store_t*)mem)->key.info;
        key_info_max = (uint8_t*)key_info + hdr->info_size-(sizeof(unsigned int) + KEY_STORE_SIG_SIZE);
        while ((uint8_t*)key_info < key_info_max) {
                if ( KEY_STORE_DATA_GET_TYPE(key_info->type_state) == type &&
                        KEY_STORE_DATA_GET_STATE(key_info->type_state) == state) {
                        break;
                }
                key_info = (key_store_key_info_t*)((uintptr_t)key_info+sizeof(key_store_key_info_t)+key_info->size);
        }
        if ((uint8_t*)key_info == key_info_max) {
                goto err;
        }
        if (key_store_get_data((key_store_t*)mem, key_info, 
                &_obj.auth_args->encrArgs, data) != KEY_STORE_ERR_SUCC) {
                goto err;
        }
        /*
        *key_mem = mem;
        *ref = i;
        */
        return KEY_STORE_ERR_SUCC; 
err1:
        if (mem) {
                _release_buf(mem);
        }
err:
        PRINT4CH(ERNF);
        return KEY_STORE_ERR_INVALID;
}

key_store_err_t key_store_get_req_info(key_store_req_info_t* req_info)
{
        if (_obj.state != KEY_STORE_OBJ_OK) {
                goto err;
        }
        memcpy(req_info, &_obj.key_store.hdr.req_info, sizeof(key_store_req_info_t));
        return KEY_STORE_ERR_SUCC; 
err:
        return KEY_STORE_ERR_INVALID; 
}


/* Releases/frees buffer used for key store*/
key_store_err_t key_store_data_release()
{
        if (_obj.state == KEY_STORE_OBJ_OK) {
                if (_obj.mem) {
                        _obj.mem = NULL;
                }
                return KEY_STORE_ERR_SUCC; 
        }
        return KEY_STORE_ERR_INVALID; 
}

key_store_err_t key_store_reset()
{
        if (_obj.state == KEY_STORE_OBJ_OK) {
                key_store_data_release();
        }
        memset(&_obj,0,sizeof(_obj));
        return KEY_STORE_ERR_SUCC; 
}
/*
        Erases keystore block from the flash
*/
key_store_err_t key_store_erase()
{
        if (_obj.state != KEY_STORE_OBJ_OK) {
                return KEY_STORE_ERR_INVALID; 
        }
#if 0
        if (_obj.fs->erase(KEY_STORE_OFFSET) )
                return KEY_STORE_ERR_INVALID; 
        }
#endif
        return KEY_STORE_ERR_SUCC; 
}
