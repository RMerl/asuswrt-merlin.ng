/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
*/

/**  @mainpage Broadcom Configureation Statics Module
 *
 *   @section Introduction
 *
 *   This document covers Broadcom WLAN Configuration and Statics
 *   Module(WCSM).
 *
 *  @file wlcsm_linux.h
 *  @brief shared wlcsm header file for userspace and kernel space
 *
 *  this header file includes all general functions and structure definations
 *  used for this wlcsm module.
 *
 * */

#ifndef __WLCSM_LINUX_H_
#define  __WLCSM_LINUX_H_
#ifndef __cplusplus
#include <linux/string.h>
#include "wl_common_defs.h"
#define MARK_IDX(idx) (idx|0x80)
#define REAL_IDX(idx) (idx&0x0f)
#define IS_IDX_MARKED(idx) (idx&0x80)

#define WLCSM_SET_TYPE(x, v)  (((struct wlcsm_msg_hdr *)x)->type = v)
#define WLCSM_SET_LEN(x, v)  (((struct wlcsm_msg_hdr *)x)->len = v)
#define WLCSM_SET_PID(x, v)  (((struct wlcsm_msg_hdr *)x)->pid = v)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define WLCSM_NAMEVALUEPAIR_MAX 1024
#define NL_PACKET_SIZE  WLCSM_NAMEVALUEPAIR_MAX
#define MAX_NLRCV_BUF_SIZE   1280 /*MAX BUF NEEDS TO BIGGER THAN NL_PACKET_SIZE */

#define WLCSM_TRACE_NONE 0
#define WLCSM_TRACE_ERR 1
#define WLCSM_TRACE_DBG 2
#define WLCSM_TRACE_GEN 4
#define WLCSM_TRACE_LOG 8
#define WLCSM_TRACE_FUNC 16
#define WLCSM_TRACE_PKT 32
#define WLCSM_TRACE_DM 64
#define WLCSM_TRACE_NVRAM 128

typedef struct t_wlcsm_name_valuepair {
    int len;                                /**< total length of this structure name:value */
    char value[1];                          /**< acting as pointer to the name:value pair string */
} t_WLCSM_NAME_VALUEPAIR;

typedef struct t_wlcsm_mngr_varhdr {
    unsigned int	radio_idx;	/**< radio index */
    unsigned int  	sub_idx;	/**< level two index */
    unsigned char 	extra_idx[4];	/**< extra index for deeper level in data model */
    unsigned int 	dm_oid;		/**< in case dm oid to be carried */
    unsigned int 	offset;		/**< dm variable offset */
} t_WLCSM_MNGR_VARHDR;

typedef struct t_wlcsm_mngr_var {
    t_WLCSM_MNGR_VARHDR hdr;
    t_WLCSM_NAME_VALUEPAIR pair;
} t_WLCSM_MNGR_VAR;

#define VALUEPAIR_NAME(v) (((t_WLCSM_NAME_VALUEPAIR *)(v))->value)
#define VALUEPAIR_NAME_LEN(v) (((t_WLCSM_NAME_VALUEPAIR *)(v))->len)

static inline int _str_int_aligned_len(char *name)
{
    int nlen=strlen(name)+1;
    int modlen=(nlen&(sizeof(int)-1));
    if(modlen) return (nlen+sizeof(int))&(~(sizeof(int)-1));
    else return nlen;
}
static inline int _get_valuepair_total_len(char *name, char *value,int len)
{
    return 2*sizeof(int)+_str_int_aligned_len(name)+(len?len:(value!=NULL?((int)strlen(value)+1):0));
}

static inline t_WLCSM_NAME_VALUEPAIR *_get_valuepair_value(t_WLCSM_NAME_VALUEPAIR *v)
{
    return (t_WLCSM_NAME_VALUEPAIR *)((char *)(v)+sizeof(int)+_str_int_aligned_len(VALUEPAIR_NAME(v)));
}

#define VALUEPAIR_VALUE(v) ((_get_valuepair_value((t_WLCSM_NAME_VALUEPAIR *)(v))->len)?\
			    (_get_valuepair_value((t_WLCSM_NAME_VALUEPAIR *)(v))->value):NULL)
#define VALUEPAIR_VALUE_LEN(v) (_get_valuepair_value((t_WLCSM_NAME_VALUEPAIR *)(v))->len)
#define VALUEPAIR_LEN(v) (_str_int_aligned_len(VALUEPAIR_NAME(v))+VALUEPAIR_VALUE_LEN(v)+2*sizeof(int))

typedef struct wlcsm_msg_hdr {
    unsigned short type;
    unsigned short len;
    unsigned int pid;
} t_WLCSM_MSG_HDR;

typedef struct wlcsm_msg_register {
    unsigned int pid;
    unsigned int len;
    char process[1];
} t_WLCSM_MSG_REGISTER;

typedef enum wlcsm_msgtype {
    WLCSM_MSG_BASE = 0,
    WLCSM_MSG_REGISTER,
    WLCSM_MSG_NVRAM_SET,
    WLCSM_MSG_NVRAM_GET,
    WLCSM_MSG_NVRAM_UNSET,
    WLCSM_MSG_NVRAM_GETALL,
    WLCSM_MSG_NVRAM_GETALL_DONE, /* for getall operation, if nvarm set is too big,more transaction may needed, so need this msg to indeciate done */
    WLCSM_MSG_NVRAM_COMMIT,
#ifdef WLCSM_DEBUG
    WLCSM_MSG_NVRAM_SETTRACE,
    WLCSM_MSG_DEBUG_LOGMESSAGE,
    WLCSM_MSG_DEBUGPID_REG,
#endif

    WLCSM_MSG_GETWL_BASE,
    WLCSM_MSG_GETWL_VAR,
    WLCSM_MSG_GETWL_VAR_RESP,
    WLCSM_MSG_GETWL_VAR_RESP_DONE,
    WLCSM_MSG_SETWL_VAR,
    WLCSM_MSG_SETWL_VAR_RESP,
    WLCSM_MSG_NVRAM_XFR,
    WLCSM_MSG_DUMP_PREV_OOPS,
} t_WLCSM_MSGTYPES;

typedef enum wlcsm_ret_codes {
    WLCSM_SUCCESS = 0,
    WLCSM_GEN_ERR = 1,
} t_WLCSM_RET_CODE;


typedef struct wlcsm_register {
    int code;
} t_WLCSM_REGISTER;


#ifdef __KERNEL__
#ifdef CONFIG_BCM_WLCSM_DEBUG
#define WLCSM_DEBUG 1
#endif
#endif

#ifdef WLCSM_DEBUG
extern unsigned int  g_WLCSM_TRACE_LEVEL;
extern char  g_WLCSM_TRACE_PROC[];
#endif


#ifndef __KERNEL__
#include <stdlib.h>
#include <wlcsm_lib_api.h>
int  wlcsm_mngr_response(t_WLCSM_MSGTYPES type,char * name,char *value,int len,unsigned int to_pid);
#ifdef WLCSM_DEBUG
void wlcsm_print(const char *fmt, ...);
#define WLCSM_TRACE(loglevel,fmt,arg...)   \
        do { \
                if(g_WLCSM_TRACE_LEVEL & loglevel) \
                  wlcsm_print("PID:%d:%s:%s:%d  "fmt,getpid(),g_WLCSM_TRACE_PROC,__FUNCTION__,__LINE__,##arg);\
        } while(0)
#else
#define WLCSM_TRACE(loglevel,fmt,arg...)
#endif

#ifndef  _MALLOC_
#define _MALLOC_(x)             calloc(x, sizeof(char))
#endif

#ifndef  _MFREE_
#define _MFREE_(buf)      free(buf)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)            (sizeof(a) / sizeof(a[0]))
#endif

#else                                           /**< kernel definitions */

extern char * wlcsm_nvram_k_get(char *name);
extern int wlcsm_nvram_k_set(char *name, char *value);
extern int wlcsm_nvram_getall(char *buf,int count);


#ifdef WLCSM_DEBUG
void wlcsm_print(const char *fmt, ...);
void wlcsm_dbg_reg_item(char *description, unsigned int *pValue) ;
void wlcsm_dbg_reg(unsigned int index,char *description) ;
void wlcsm_dbg_inc(unsigned int index,int incr) ;
void wlcsm_dbg_dec(unsigned int index,int incr) ;
void wlcsm_dbg_set(unsigned int index,int value) ;
void wlcsm_hexdump_ascii(const char *title,unsigned char c, const unsigned char *buf, unsigned int len);
void wlcsm_dump_pkt(char *prompt,void* pktdata,int pktlen,unsigned int match_srclast4,unsigned int match_destlast4);
#define WLCSM_TRACE(loglevel,fmt,arg...)   do { if(g_WLCSM_TRACE_LEVEL & loglevel) wlcsm_print("K:%s:%d  "fmt"\n",__FUNCTION__,__LINE__,##arg); } while(0)
#define WLCSM_PRINTK(condition,fmt,arg...)   \
        do { \
                if(condition  && (g_WLCSM_TRACE_LEVEL & WLCSM_TRACE_DBG)) \
                  wlcsm_print("[k]%s:%d  "fmt,__FUNCTION__,__LINE__,##arg);\
        } while(0)

#else
#define WLCSM_TRACE(loglevel,fmt,arg...)
#define WLCSM_PRINTK(condition,fmt,arg...)
#define wlcsm_dbg_reg_item(arg...)
#define wlcsm_dbg_reg(arg...)
#define wlcsm_dbg_inc(arg...)
#define wlcsm_dbg_dec(arg...)
#define wlcsm_dbg_set(arg...)
#define wlcsm_hexdump_ascii(arg...)
#define wlcsm_dump_pkt(arg...)
#endif
#endif

#define WLCSM_INLINE        inline  __attribute__ ((always_inline))
#define WLCSM_NOINSTR_FUNC  __attribute__ ((no_instrument_function))
#define WLCSM_MAC_CMP(mac1,mac2) (((*(unsigned int *)mac1)== (*(unsigned int *)mac2))&& ((*(unsigned short *)(((char *)mac1)+sizeof(unsigned int)))== (*(unsigned short *)(((char *)mac2)+sizeof(unsigned int)))))

#endif
#endif
