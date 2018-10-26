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
#include "bcm_otp.h"
#include "lib_byteorder.h"
#include "initdata.h"
#include "bcm63xx_storage_dev.h"
#include "bcm63xx_keystore.h"
#include "bcm63xx_sec.h"
#include "bcm63xx_sotp.h"
#include "bcm63xx_potp_sotp_util.h"

#define COMMITSOTP

extern void* _get_buf(unsigned int size);
extern void _resize_buf(void* buf,unsigned int size);
extern int board_puts(const char* s);
static inline void _putc(char c)
{
          char s[2]={c,'\0'};
          board_puts(s);
}

static inline int cfe_sec_check_abort(int sec)
{
    char str[8],ch;
    sec *= 4;
    while (sec > 0) {
	ch = board_getc();
        if (ch == 'c' || ch == 'C') {
            break;
        }
        sprintf(str,"%3d",sec/4);
        board_puts(str);
        _putc('\r');
        memset(str,0,8);
        cfe_usleep(250000);
        sec--;
    }
    return sec;
}

#if  (BOARD_SEC_ARCH==SEC_ARCH_GEN2)
static inline cfe_sec_err_t _transit_to_(key_store_req_state_t req_state, 
                                sec_state_t cur_state)
{
        switch(cur_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KEY_STORE_REQ_TRANSIT_GEN2_OP || 
                                req_state == KEY_STORE_REQ_TRANSIT_GEN2_MFG) {
                                uint16_t  mid[2];
                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_MID, 
                                        KEY_STORE_DATA_STATE_RAW, &mid[0]) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                PRINT4CH(MFG);
				/*mid[0] = ntohs(mid[0]); needs to be for v7 generation of potp
                                 */
#ifdef COMMITSOTP 
                                {
                                    int res;
                                    res = potp_set_brom_mode();
                                    if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                    }
                                    res = burn_potp_mid(mid[0]);
                                    if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                    }
                                }
                                if (read_potp_mid(&mid[1])) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                }
                                if (memcmp(&mid[0],&mid[1],sizeof(uint16_t))) {
                                        xprintf("mid %x \n",mid[1]);
                                        PRINT4CH(EMID);
                                        goto err;
                                }
#else
                                xprintf("Fusing mid %x \n",mid[0]);
#endif
                        }
                        break;
                case SEC_STATE_GEN2_MFG: {
                        if (req_state == KEY_STORE_REQ_TRANSIT_GEN2_OP) {
                                uint16_t  oid[2];
                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_OID, 
                                        KEY_STORE_DATA_STATE_RAW, &oid[0]) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                PRINT4CH(OPER);
				/*oid[0] = ntohs(oid[0]); be*/
#ifdef COMMITSOTP 
                                {
                                    int res = potp_set_brom_opmode();
                                    if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                    }
                                    res = potp_set_oid(oid[0]);
                                    if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                    }
                                    if (read_potp_oid(&oid[1])) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                    }
                                    if (memcmp(&oid[0], &oid[1],sizeof(uint16_t))) {
                                        xprintf("Fusing oid 0x%x 0x%x\n",oid[0], oid[1]);
                                        PRINT4CH(EOID);
                                        goto err; 
                                    }
                                }
#else
                                xprintf("Fusing oid %x \n",oid[0]);
#endif
                        }
                }
                break;
                default:
                        goto err;
        }
        key_store_data_release();
        return CFE_SEC_ERR_OK;
err:    
        key_store_data_release();
        return CFE_SEC_ERR_FAIL;
}

#elif  (BOARD_SEC_ARCH==SEC_ARCH_GEN3)
static inline void _array2ntohl_(unsigned int* ptr, unsigned int length) 
{
    int i;
    for (i = 0;i < length; i++)  {
         ptr[i] = ntohl(ptr[i]);
    }
}

static inline cfe_sec_err_t _transit_to_(key_store_req_state_t req_state, 
                                sec_state_t cur_state)
{
#ifdef COMMITSOTP
        int res;
#endif
        switch(cur_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KEY_STORE_REQ_TRANSIT_FLD || 
                                req_state == KEY_STORE_REQ_TRANSIT_MFG){
                                PRINT4CH(MFG);
#ifdef COMMITSOTP 
                                res = potp_set_brom_mode();
                                if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                }
#endif
                        }
                        break;
                case SEC_STATE_GEN3_MFG:
                        if (req_state == KEY_STORE_REQ_TRANSIT_FLD) {
#ifdef COMMITSOTP 
                                int res;
                                uint32_t *mem; 
#endif
                        /*program otp bits accordingly */
                                uint8_t ek_iv[KEY_STORE_AES_128_CBC_SZ+KEY_STORE_AES_128_CBC_SZ],
                                                hash[KEY_STORE_HASH_SZ]; 
                                uint16_t mid[2];

                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_EK, 
                                        KEY_STORE_DATA_STATE_FLD_ENCR, ek_iv) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_KEY_AES_CBC_128_IV, 
                                        KEY_STORE_DATA_STATE_FLD_ENCR, ek_iv+KEY_STORE_AES_128_CBC_SZ) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_HASH, 
                                                KEY_STORE_DATA_STATE_RAW, hash) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                if (key_store_get_data_info(KEY_STORE_DATA_TYPE_MID, 
                                        KEY_STORE_DATA_STATE_RAW, mid) != KEY_STORE_ERR_SUCC) {
                                        goto err;
                                }
                                PRINT4CH(FLD);
				/*
                                 since sotp interface dealing with 32 bit integers - assumption is
                                 that raw byte data from keystore are always honoring big endian 
                                */
				mid[0] = ntohs(mid[0]);
                                _array2ntohl_((unsigned int*)ek_iv,(KEY_STORE_AES_128_CBC_SZ*2)/sizeof(unsigned int)); 
                                _array2ntohl_((unsigned int*)hash,(KEY_STORE_HASH_SZ)/sizeof(unsigned int)); 
#ifdef COMMITSOTP 
                                res = burn_potp_mid(mid[0]);
                                if (res) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                }
                                /*cfe_usleep(25000);*/
                                if (read_potp_mid(&mid[1])) {
                                        PRINT4CH(EOTP);
                                        goto err;
                                }
                                if (memcmp(&mid[0],&mid[1],sizeof(uint16_t))) {
                                        PRINT4CH(EMID);
                                        goto err;
                                }
                                res = sotpWriteSecKey(8, (uint32_t*)ek_iv, sizeof(ek_iv)/sizeof(uint32_t));
                                if (res != SOTP_S_KEY_SUCCESS) {
                                        PRINT4CH(ESOT);
                                        goto err;
                                }
                                /*cfe_usleep(25000);*/
                                mem = (uint32_t*)_get_buf(sizeof(ek_iv)); 
                                res = sotpReadSecKey(8, mem, sizeof(ek_iv)/sizeof(uint32_t));
                                if (res != SOTP_S_KEY_SUCCESS) {
                                        PRINT4CH(ESOT);
                                        goto err;
                                }
                                if (memcmp(mem, ek_iv,sizeof(ek_iv))) {
                                        PRINT4CH(EKEY);
                                        goto err;
                                }
                                res = sotpWriteSecKey( 9, (uint32_t*)hash, sizeof(hash)/sizeof(uint32_t));
                                if (res != SOTP_S_KEY_SUCCESS) {
                                        PRINT4CH(ESOT);
                                        goto err;
                                }
                                /*cfe_usleep(25000);*/
                                _resize_buf((void*)mem,sizeof(hash));
                                res = sotpReadSecKey(9, mem, sizeof(hash)/sizeof(uint32_t));
                                if (res != SOTP_S_KEY_SUCCESS) {
                                        PRINT4CH(ESOT);
                                        goto err;
                                }
                                if (memcmp(mem, hash, sizeof(hash))) {
                                        PRINT4CH(EHSH);
                                        goto err;
                                }
#else
                                {
                                        int i;
                                        xprintf("mid %x \n",mid[0]);
                                        xprintf("ek : ");
                                        for (i = 0;i < KEY_STORE_AES_128_CBC_SZ/sizeof(uint32_t); i++)  {
                                                xprintf("%x ",((uint32_t*)ek_iv)[i]);
                                        } 
                                        xprintf("\n");
                                        xprintf("iv : ");
                                        for (i = 0;i < KEY_STORE_AES_128_CBC_SZ/sizeof(uint32_t); i++)  {
                                                xprintf("%x ",((uint32_t*)ek_iv)[i+KEY_STORE_AES_128_CBC_SZ/sizeof(uint32_t)]);
                                        } 
                                        xprintf("\n");
                                        xprintf("hash : "); 
                                        for (i=0;i < KEY_STORE_HASH_SZ/sizeof(uint32_t); i++)  {
                                                xprintf("%x ",((uint32_t*)hash)[i]);
                                        } 
                                        xprintf("\n");
                                }
#endif
                        }
                        break;
                default:
                        goto err;
        }
        key_store_data_release();
        return CFE_SEC_ERR_OK;
err:    
        key_store_data_release();
        return CFE_SEC_ERR_FAIL;
}

#endif
static inline cfe_sec_err_t _implement_state_req(boot_status_t  *boot_info,
                                key_store_req_info_t *req_info)
{

/*
        if (key_store_get_req_info(&req_info) != KEY_STORE_ERR_SUCC) {
                goto err;
        }*/
        
        switch(boot_info->sec_arch) {
                case SEC_ARCH_GEN2:
                case SEC_ARCH_GEN3:
                        _transit_to_(req_info->state, boot_info->sec_state);
                        break;
                case SEC_ARCH_NONE:
                case SEC_ARCH_GEN1:
                default:
                        goto err;
        }
        return CFE_SEC_ERR_OK;
err:
        PRINT4CH(EREQ);
        return CFE_SEC_ERR_FAIL;
}

static int _allow_transit(sec_state_t sec_state, 
                        key_store_req_state_t req_state)
{
#if  (BOARD_SEC_ARCH==SEC_ARCH_GEN2)
        switch (sec_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KEY_STORE_REQ_TRANSIT_GEN2_BTRM ||
                                req_state == KEY_STORE_REQ_TRANSIT_GEN2_MFG ||
                                req_state == KEY_STORE_REQ_TRANSIT_GEN2_OP)  {
                                return 1;       
                        }
                        break;
                case SEC_STATE_GEN2_BTRM:
                        if ( req_state == KEY_STORE_REQ_TRANSIT_GEN2_MFG ||
                                req_state == KEY_STORE_REQ_TRANSIT_GEN2_OP)  {
                                return 1;
                        }
                        break;
                case SEC_STATE_GEN2_MFG:
                        if (req_state == KEY_STORE_REQ_TRANSIT_GEN2_OP)  {
                                return 1;
                        }
                        break;
                default:
                        break;
        }
#elif (BOARD_SEC_ARCH== SEC_ARCH_GEN3)
        switch (sec_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KEY_STORE_REQ_TRANSIT_MFG ||
                                req_state == KEY_STORE_REQ_TRANSIT_FLD) {
                                return 1;       
                        }
                        break;
                case SEC_STATE_GEN3_MFG:
                        if (req_state == KEY_STORE_REQ_TRANSIT_FLD) {
                                return 1;       
                        }
                        break;
                default:
                        break;
        }
#endif
        return 0; 
}
cfe_sec_err_t cfe_sec_tk_implement(cfe_storage_dev_t* fs);
cfe_sec_err_t cfe_sec_tk_implement(cfe_storage_dev_t* fs)
{
        key_store_req_info_t req_info;
        boot_status_t info;
        Booter1Args* sec_args;
        unsigned int _arch_prt;
        if( cfe_sec_get_boot_status(&info)) {
                goto err;
        }

        sec_args = cfe_sec_get_bootrom_args();
        if (!sec_args) {
                goto err;
        } 
        PRINT4CH(SECE);
#if (BOARD_SEC_ARCH==SEC_ARCH_GEN2)
        PRINT4CH(GEN2);
#else
        PRINT4CH(GEN3);
#endif
        if (key_store_init(info.sec_state,
                                info.sec_arch,
                                sec_args, fs) != KEY_STORE_ERR_SUCC) {
                goto err;
        }
        if (key_store_get_req_info(&req_info) != KEY_STORE_ERR_SUCC) {
                goto err;
        }

        if (!_allow_transit(info.sec_state, req_info.state)) {
                PRINT4CH(FUSD);
                goto done;
        }
        switch (info.sec_state) {
                case SEC_STATE_UNSEC:
                        _arch_prt = CH4_TO_U32(NSEC);
                        break;
                case SEC_STATE_GEN2_BTRM:
                        _arch_prt = CH4_TO_U32(BTRM);
                        break;
                case SEC_STATE_GEN2_MFG:
                        _arch_prt = CH4_TO_U32(MFG);
                        break;
                case SEC_STATE_GEN3_MFG:
                        _arch_prt = CH4_TO_U32(MFG);
                        break;
                default:
                        goto err;
        }
        PRINTU32(_arch_prt);
        PRINT4CH(ABT?);
        if (cfe_sec_check_abort(req_info.abort_delay)) {
                
                PRINT4CH(CNCL);
                goto done;
        }
        if (_implement_state_req(&info, &req_info) == CFE_SEC_ERR_OK) {
                PRINT4CH(POR!);
                for(;;);
        }
done:
        return CFE_SEC_ERR_OK;
err:
        PRINT4CH(SERR); 
        return CFE_SEC_ERR_FAIL;
}

