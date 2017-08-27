/***************************************************************************
 <:copyright-BRCM:2012:DUAL/GPL:standard
 
    Copyright (c) 2012 Broadcom 
    All Rights Reserved
 
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
 ***************************************************************************
 * File Name  : bcm63xx_nvram.h 
 *
 *   
 ***************************************************************************/

#if !defined(_BCM63XX_NVRAM_H_)
#define _BCM63XX_NVRAM_H_

#define WARN_MISMATCH(mtype,m,vtype) \
        do { \
        mtype* __p1 = NULL; vtype *__p2 = NULL; \
        __p2?(__p2 = &(__p1->m)) : (void)__p2; } while(0)
      
#define STRONG_ASSIGN(mtype, m, vtype, v) \
        mtype* __mtyp = NULL;\
        unsigned char __v[sizeof((*__mtyp).m)]; \
        do { \
        WARN_MISMATCH(mtype,m,vtype); \
        *((vtype*)__v) = v; } while(0)

const NVRAM_DATA* const cfe_nvram_get(void);
int cfe_nvram_init(NVRAM_DATA* nv);
void cfe_nvram_deinit(void);
int cfe_nvram_read_verify(NVRAM_DATA* nv);
void cfe_nvram_copy_to(NVRAM_DATA* dst);
int cfe_nvram_update(NVRAM_DATA* nv);
int cfe_nvram_update_member(const void* m_addr,
			    void* val,
			    unsigned int size, 
                            unsigned int max_size);
int cfe_nvram_set(const void* m_addr,
		  void* val,
		  unsigned int size, 
                  unsigned int max_size);

int cfe_nvram_sync(void);
int cfe_nvram_erase(void);
//#define DEBUG_NVRAM


#define NVRAM (*cfe_nvram_get())
#define NVRAM_RP (cfe_nvram_get())
#define NVRAM_COPY_FIELD(m, v, s) cfe_nvram_set(&NVRAM.m, v, s, sizeof(NVRAM.m))

#define NVRAM_SET(m, vtype, v)        \
	do { STRONG_ASSIGN(NVRAM_DATA, m, vtype, (v)); \
	NVRAM_COPY_FIELD(m, &__v, sizeof(__v));} while(0);
#define NVRAM_COPY_TO(p) cfe_nvram_copy_to(p)
#define NVRAM_SYNC() cfe_nvram_sync()
#define NVRAM_UPDATE(nv) cfe_nvram_update(nv)
#define NVRAM_UPDATE_FIELD(m, v, s) cfe_nvram_update_member(&NVRAM.m, v, s, sizeof(NVRAM.m));
#define NVRAM_READ_VERIFY(nv) cfe_nvram_read_verify(nv)
#define NVRAM_ERASE() cfe_nvram_erase()
#define NVRAM_INIT() cfe_nvram_init(NULL)

#endif // _BCM63XX_UTIL_H_

