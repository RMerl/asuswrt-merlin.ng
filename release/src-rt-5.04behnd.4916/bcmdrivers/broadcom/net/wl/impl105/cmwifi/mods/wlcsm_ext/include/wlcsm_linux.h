/*
 * The wlcsm kernel module
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>
 *
 * $Id: wlcsm_linux.h 832801 2023-11-13 20:14:38Z $
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
#include <linux/string.h>
#define MARK_IDX(idx) (idx|0x80)
#define REAL_IDX(idx) (idx&0x0f)
#define IS_IDX_MARKED(idx) (idx&0x80)

/* enable/disable WLCSM_DEBUG requires to change DSLCPE_WLCSM_DEBUG in
 * userspace wlcsm/Makefile
 */
//#define WLCSM_DEBUG

#define WLCSM_SET_TYPE(x, v)  (((struct wlcsm_msg_hdr *)x)->type = v)
#define WLCSM_SET_LEN(x, v)  (((struct wlcsm_msg_hdr *)x)->len = v)
#define WLCSM_SET_PID(x, v)  (((struct wlcsm_msg_hdr *)x)->pid = v)

#define TRUE 1
#define FALSE 0
#define m_MAX_BUF_SIZE		(1280 * 2)	/* Sync with MAX_NLRCV_BUF_SIZE in unfnvram */
#define WLCSM_NAMEVALUEPAIR_MAX 1280

#define WLCSM_TRACE_NONE 0
#define WLCSM_TRACE_DBG 1
#define WLCSM_TRACE_ERR 2
#define WLCSM_TRACE_GEN 4
#define WLCSM_TRACE_LOG 8
#define WLCSM_TRACE_FUNC 16
#define WLCSM_TRACE_PKT 32

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

static inline int _str_int_aligned_len(char *name) {
	int nlen=strlen(name)+1;
	int modlen=(nlen&(sizeof(int)-1));
	if(modlen) return (nlen+sizeof(int))&(~(sizeof(int)-1));
	else return nlen;
}
static inline int _get_valuepair_total_len(char *name, char *value,int len)
{
	return 2*sizeof(int)+_str_int_aligned_len(name)+(len?len:(value!=NULL?(strlen(value)+1):0));
}

static inline t_WLCSM_NAME_VALUEPAIR *_get_valuepair_value(t_WLCSM_NAME_VALUEPAIR *v)
{
    return (t_WLCSM_NAME_VALUEPAIR *)((char *)(v)+sizeof(int)+_str_int_aligned_len(VALUEPAIR_NAME(v)));
}

#define VALUEPAIR_VALUE(v) ((_get_valuepair_value(v)->len)?(_get_valuepair_value(v)->value):NULL)
#define VALUEPAIR_VALUE_LEN(v) (_get_valuepair_value(v)->len)
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
#endif // endif
    WLCSM_MSG_GETWL_BASE,
    WLCSM_MSG_GETWL_VAR,
    WLCSM_MSG_GETWL_VAR_RESP,
    WLCSM_MSG_GETWL_VAR_RESP_DONE,
    WLCSM_MSG_SETWL_VAR,
    WLCSM_MSG_SETWL_VAR_RESP,
#ifdef CMWIFI
    WLCSM_MSG_NVRAM_COMMIT_REQD,
    WLCSM_MSG_NVRAM_COMMIT_REQD_CLEAR,
    WLCSM_MSG_NVRAM_SET_ERR,		/* Error in set */
    WLCSM_MSG_NVRAM_GET_ERR,		/* Error in get */
    WLCSM_MSG_NVRAM_GETALL_ERR,		/* Error in getall */
    WLCSM_MSG_NVRAM_GETALL_BUSY,	/* getall in progress */
#endif /* CMWIFI */
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

#ifdef WLCSM_DEBUG
extern unsigned int  g_WLCSM_TRACE_LEVEL;
#endif // endif

#ifndef __KERNEL__
#include <stdlib.h>
int  wlcsm_mngr_response(t_WLCSM_MSGTYPES type,char * name,char *value,int len,unsigned int to_pid);
#ifdef WLCSM_DEBUG
void wlcsm_print(const char *fmt, ...);
#define WLCSM_TRACE(loglevel,fmt,arg...)   do { if(g_WLCSM_TRACE_LEVEL & loglevel) wlcsm_print("U:%s:%d  "fmt,__FUNCTION__,__LINE__,##arg); } while(0)
#else
#define WLCSM_TRACE(loglevel,fmt,arg...)
#endif // endif

#ifndef  _MALLOC_
#define _MALLOC_(x)             calloc(x, sizeof(char))
#endif // endif

#ifndef  _MFREE_
#define _MFREE_(buf)      free(buf)
#endif // endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)            (sizeof(a) / sizeof(a[0]))
#endif // endif

#else                                           /**< kernel definitions */

#ifdef WLCSM_DEBUG
void wlcsm_print(const char *fmt, ...);
void wlcsm_hexdump_ascii(const char *title,unsigned char c, const unsigned char *buf, unsigned int len);
void wlcsm_dump_pkt(char *prompt,void* pktdata,int pktlen,unsigned int match_srclast4,unsigned int match_destlast4);
#define WLCSM_TRACE(loglevel,fmt,arg...)   do { if(g_WLCSM_TRACE_LEVEL & loglevel) wlcsm_print("K:%s:%d  "fmt,__FUNCTION__,__LINE__,##arg); } while(0)
#else
#define WLCSM_TRACE(loglevel,fmt,arg...)
#endif // endif

#endif // endif

#define WLCSM_INLINE        inline  __attribute__ ((always_inline))
#define WLCSM_NOINSTR_FUNC  __attribute__ ((no_instrument_function))
#define WLCSM_MAC_CMP(mac1,mac2) (((*(unsigned int *)mac1)== (*(unsigned int *)mac2))&& ((*(unsigned short *)(((char *)mac1)+sizeof(unsigned int)))== (*(unsigned short *)(((char *)mac2)+sizeof(unsigned int)))))

#define MAX_ALLOWED_SIZE        (m_MAX_BUF_SIZE - sizeof(t_WLCSM_MSG_HDR))

#endif // endif
