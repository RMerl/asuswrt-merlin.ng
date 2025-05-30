/*
 *
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom
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

#ifndef __GBPM_H_INCLUDED__
#define __GBPM_H_INCLUDED__

/*
 *******************************************************************************
 * File Name : gbpm.h
 *
 *******************************************************************************
 */
#define GBPM_VERSION	"v0.1"
#define GBPM_VER_STR	GBPM_VERSION
#define GBPM_MODNAME	"Broadcom GBPM"

#define GBPM_ERROR	(-1)
#define GBPM_SUCCESS	 0

#define GBPM_HIGH_PRIO_ALLOC	1
#define GBPM_LOW_PRIO_ALLOC     0

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#define GBPM_XTM_SUPPORT
#endif

typedef enum {
	GBPM_PORT_ETH,
	GBPM_PORT_XTM,
	GBPM_PORT_FWD,
	GBPM_PORT_WLAN,
	GBPM_PORT_USB,
	GBPM_PORT_MAX
} gbpm_port_t;

struct gbpm_hw_buffer_mngr {
	const char *name;
	int (*pool_init)(void);
	void (*pool_exit)(void);
};

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
typedef enum {
	GBPM_REF_BUFF,
	GBPM_REF_FKB,
	GBPM_REF_SKB
} gbpm_reftype_t;

typedef enum {
	GBPM_DRV_BPM,
	GBPM_DRV_ETH,
	GBPM_DRV_XTM,
	GBPM_DRV_KERN,
	GBPM_DRV_BDMF,
	GBPM_DRV_ARCHER,
	GBPM_DRV_MAX
} gbpm_driver_t;

typedef enum {
	GBPM_VAL_UNMARKED,
	GBPM_VAL_ALLOC,
	GBPM_VAL_CLONE,
	GBPM_VAL_RECYCLE,
	GBPM_VAL_FREE,
	GBPM_VAL_RX,
	GBPM_VAL_TX,
	GBPM_VAL_ENTER,
	GBPM_VAL_EXIT,
	GBPM_VAL_INFO,
	GBPM_VAL_INIT,
	GBPM_VAL_COPY_SRC,
	GBPM_VAL_COPY_DST,
	GBPM_VAL_XLATE,
	GBPM_VAL_MAX
} gbpm_value_t;

typedef struct {
	size_t addr;
	union {
		uint16_t word;
		struct {
			uint16_t driver:4;
			uint16_t info:4;
			uint16_t reftype:2;
			uint16_t value:6;
		};
	};
} gbpm_mark_t;

typedef struct {
	atomic_t ref_cnt;
	atomic_t idle_cnt;
	uint32_t write;
	gbpm_mark_t *mbuf_p;
} gbpm_trail_t;

typedef void (*gbpm_mark_buf_hook_t) (void *, void *, int, int, int, int);
typedef void (*gbpm_add_ref_hook_t) (void *, int);
#endif /* bpm tracking */

/*
 *-----------------------------------------------------------------------------
 * GBPM callbacks are managed in a single global instantiation of gbpm_t gbpm_g
 * GBPM Hooks may be viewed as "BPM" callbacks and "User" callbacks.
 * - GBPM_BIND() lists all BPM callbacks
 * - GBPM_USER() lists all USER callbacks (bind per user driver module).
 *-----------------------------------------------------------------------------
 */

/* GBPM_DECL may be undef/define, for GBPM_BIND and GBPM_USER template usage */
#define GBPM_BIND()				\
   /* --- BPM BUF POOL --- */		\
   GBPM_DECL(alloc_mult_buf)		\
   GBPM_DECL(free_mult_buf)		\
   GBPM_DECL(alloc_buf)			\
   GBPM_DECL(free_buf)			\
   /* --- BPM SKB POOL --- */		\
   GBPM_DECL(total_skb)			\
   GBPM_DECL(avail_skb)			\
   GBPM_DECL(attach_skb)			\
   GBPM_DECL(attach_skb_no_shinfo_reset)	\
   GBPM_DECL(alloc_skb)			\
   GBPM_DECL(alloc_buf_skb_attach)		\
   GBPM_DECL(alloc_mult_buf_skb_attach)		\
   GBPM_DECL(alloc_mult_skb)		\
   GBPM_DECL(free_skb)			\
   GBPM_DECL(free_skblist)			\
   GBPM_DECL(invalidate_dirtyp)		\
   GBPM_DECL(recycle_skb)			\
   GBPM_DECL(attach_skb_hw_buf)			\
   GBPM_DECL(recycle_skb_hw_buf)			\
   GBPM_DECL(free_mult_skb_and_buf)    \
   GBPM_DECL(is_skb_with_hw_buf)    \
   /* --- BPM pNBuff --- */		\
   GBPM_DECL(recycle_pNBuff)		\
   /* --- BPM Get Accessors --- */		\
   GBPM_DECL(get_dyn_buf_lvl)		\
   GBPM_DECL(get_total_bufs)		\
   GBPM_DECL(get_avail_bufs)		\
   GBPM_DECL(get_max_dyn_bufs)		\
   /* --- BPM Runtime --- */		\
   GBPM_DECL(is_buf_hw_recycle_capable)	\
   GBPM_DECL(recycle_hw_buf)		\
   GBPM_DECL(register_hw_pool_api)


	/* --- BPM Users --- */
#define GBPM_ENET()	GBPM_DECL(enet_status)

#if defined(GBPM_XTM_SUPPORT)
#define GBPM_XTM()		\
	GBPM_DECL(xtm_status)	\
	GBPM_DECL(xtm_thresh)
#else
#define GBPM_XTM()
#endif /* GBPM_XTM_SUPPORT */

#define GBPM_USER()	 \
	GBPM_ENET()	 \
	GBPM_XTM()

/*
 * typedefs for callbacks managed by GBPM.
 */

/* --- BPM BUF POOL --- */
/* buffer allocation/free */
typedef int (*gbpm_alloc_mult_buf_hook_t)(uint32_t, void **, uint32_t);
typedef void (*gbpm_free_mult_buf_hook_t)(uint32_t, void **);
typedef void *(*gbpm_alloc_buf_hook_t)(void);
typedef void (*gbpm_free_buf_hook_t)(void *);
typedef void (*gbpm_recycle_pNBuff_hook_t)(void *, unsigned long, uint32_t);

/* --- BPM SKB POOL --- */
typedef uint32_t (*gbpm_total_skb_hook_t)(void);
typedef uint32_t (*gbpm_avail_skb_hook_t)(void);
typedef void (*gbpm_attach_skb_hook_t)(void *, void *, uint32_t);
typedef void (*gbpm_attach_skb_no_shinfo_reset_hook_t)(void *, void *, uint32_t);
typedef void *(*gbpm_alloc_skb_hook_t)(void);
typedef void *(*gbpm_alloc_buf_skb_attach_hook_t)(uint32_t);
typedef void *(*gbpm_alloc_mult_buf_skb_attach_hook_t)(uint32_t, uint32_t, uint32_t);
typedef void *(*gbpm_alloc_mult_skb_hook_t)(uint32_t);
typedef void (*gbpm_free_skb_hook_t)(void *);
typedef void (*gbpm_free_skblist_hook_t)(void *, void *, uint32_t, uint32_t, void **);
typedef void *(*gbpm_invalidate_dirtyp_hook_t)(void *);
typedef void (*gbpm_recycle_skb_hook_t)(void *, unsigned long, uint32_t);
typedef void (*gbpm_attach_skb_hw_buf_hook_t)(void *, void *, uint32_t, uint32_t, bool);
typedef void (*gbpm_recycle_skb_hw_buf_hook_t)(void *, unsigned long, uint32_t);
typedef void (*gbpm_free_mult_skb_and_buf_hook_t)(void *, uint32_t);
typedef bool (*gbpm_is_skb_with_hw_buf_hook_t)(void *);

/* --- BPM Get Accessors --- */
typedef int (*gbpm_get_dyn_buf_lvl_hook_t)(void);
typedef uint32_t (*gbpm_get_total_bufs_hook_t)(void);
typedef uint32_t (*gbpm_get_avail_bufs_hook_t)(void);
typedef uint32_t (*gbpm_get_max_dyn_bufs_hook_t)(void);

/* --- BPM Set Accessors --- */
typedef void (*gbpm_upd_buf_lvl_hook_t)(int);

/* --- BPM HW Buffer Manager ---*/
typedef bool (*gbpm_is_buf_hw_recycle_capable_hook_t)(void *);
typedef void (*gbpm_recycle_hw_buf_hook_t)(void *, uint32_t);
typedef int (*gbpm_register_hw_pool_api_hook_t)(void *, void *);

/* --- BPM User --- */
typedef void (*gbpm_thresh_hook_t)(void);
typedef void (*gbpm_status_hook_t)(void);


/* --- BPM User instantiations --- */
typedef gbpm_status_hook_t gbpm_enet_status_hook_t;
typedef gbpm_thresh_hook_t gbpm_enet_thresh_hook_t;

typedef gbpm_status_hook_t gbpm_xtm_status_hook_t;
typedef gbpm_thresh_hook_t gbpm_xtm_thresh_hook_t;


/* Typedef of the Global BPM hook manager */
#undef GBPM_DECL
#define GBPM_DECL(HOOKNAME)	gbpm_ ## HOOKNAME ## _hook_t HOOKNAME;

typedef struct gbpm {
	GBPM_BIND() /* List of BPM "BIND" hooks */
	GBPM_USER() /* List of DRV "USER" hooks */
	uint32_t debug;
	bool is_hw_buffer_mngr_registered;
	struct gbpm_hw_buffer_mngr hw_buffer_mngr;
} gbpm_t;

extern gbpm_t gbpm_g; /* exported global */

int gbpm_register_hw_buffer_manager(struct gbpm_hw_buffer_mngr *new_mngr);

/* BPM registering callbacks into GBPM */
#undef GBPM_DECL
#define GBPM_DECL(HOOKNAME)	 gbpm_ ## HOOKNAME ## _hook_t HOOKNAME,

void gbpm_bind(GBPM_BIND() uint32_t debug);
void gbpm_unbind(void);

void gbpm_queue_work(void);

/*
 * Wrappers for GBPM callbacks
 */

/* --- BPM BUF POOL --- */
/* combine gbpm_g.alloc_mult_buf and gbpm_g.alloc_mult_buf_ex, they
 * are really the same functions
 */
static inline int gbpm_alloc_mult_buf(uint32_t num, void **buf_p)
{
	return gbpm_g.alloc_mult_buf(num, buf_p, GBPM_LOW_PRIO_ALLOC);
}

static inline int gbpm_alloc_mult_buf_ex(uint32_t num, void **buf_p,
					 uint32_t prio)
{
	return gbpm_g.alloc_mult_buf(num, buf_p, prio);
}

static inline void gbpm_free_mult_buf(uint32_t num, void **buf_p)
{
	gbpm_g.free_mult_buf(num, buf_p);
}

static inline void *gbpm_alloc_buf(void)
{
	return gbpm_g.alloc_buf();
}

static inline void gbpm_free_buf(void *buf_p)
{
	return gbpm_g.free_buf(buf_p);
}

/* --- BPM SKB --- */
static inline uint32_t gbpm_total_skb(void)
{
	return gbpm_g.total_skb();
}

static inline uint32_t gbpm_avail_skb(void)
{
	return gbpm_g.avail_skb();
}

static inline void gbpm_attach_skb(void *skbp, void *data, uint32_t datalen)
{
	gbpm_g.attach_skb(skbp, data, datalen);
}

static inline void gbpm_attach_skb_no_shinfo_reset(void *skbp, void *data, uint32_t datalen)
{
	gbpm_g.attach_skb_no_shinfo_reset(skbp, data, datalen);
}


static inline void *gbpm_alloc_skb(void)
{
	return gbpm_g.alloc_skb();
}

static inline void *gbpm_alloc_buf_skb_attach(uint32_t datalen)
{
	return gbpm_g.alloc_buf_skb_attach(datalen);
}

static inline void *gbpm_alloc_mult_buf_skb_attach(uint32_t num, uint32_t datalen, uint32_t prio)
{
	return gbpm_g.alloc_mult_buf_skb_attach(num, datalen, prio);
}


static inline void *gbpm_alloc_mult_skb(uint32_t num)
{
	return gbpm_g.alloc_mult_skb(num);
}

static inline void gbpm_free_skb(void *skbp)
{
	gbpm_g.free_skb(skbp);
}

static inline void gbpm_free_skblist(void *head, void *tail, uint32_t skbnum, uint32_t bufnum,
				     void **bufp_arr)
{
	gbpm_g.free_skblist(head, tail, skbnum, bufnum, bufp_arr);
}

static inline void *gbpm_invalidate_dirtyp(void *skb)
{
	return gbpm_g.invalidate_dirtyp(skb);
}

static inline void gbpm_recycle_skb(void *skbp, unsigned long context,
				    uint32_t recycle_action)
{
	gbpm_g.recycle_skb(skbp, context, recycle_action);
}

/* --- BPM pNBuff --- */
static inline void gbpm_recycle_pNBuff(void *pNBuff, unsigned long context,
				       uint32_t recycle_action)
{
	gbpm_g.recycle_pNBuff(pNBuff, context, recycle_action);
}


static inline void gbpm_attach_skb_hw_buf(void *skbp, void *data, uint32_t datalen, uint32_t dataoffset, bool good_sharedinfo)
{
   gbpm_g.attach_skb_hw_buf(skbp, data, datalen, dataoffset, good_sharedinfo);
}

static inline void gbpm_recycle_skb_hw_buf(void *skbp, unsigned long context,
				    uint32_t recycle_action)
{
   gbpm_g.recycle_skb_hw_buf(skbp, context, recycle_action);
}

static inline void gbpm_free_mult_skb_and_buf(void *skbp, uint32_t list_len)
{
   gbpm_g.free_mult_skb_and_buf(skbp, list_len);
}

static inline bool gbpm_is_skb_with_hw_buf(void *skbp)
{
   return gbpm_g.is_skb_with_hw_buf(skbp);
}

/* --- BPM Get Accessors --- */
static inline int gbpm_get_dyn_buf_lvl(void)
{
	return gbpm_g.get_dyn_buf_lvl();
}

static inline uint32_t gbpm_get_total_bufs(void)
{
	return gbpm_g.get_total_bufs();
}

static inline uint32_t gbpm_get_avail_bufs(void)
{
	return gbpm_g.get_avail_bufs();
}

static inline uint32_t gbpm_get_max_dyn_bufs(void)
{
	return gbpm_g.get_max_dyn_bufs();
}

/* --- BPM HW Buffer Manager ---*/
static inline bool gbpm_is_buf_hw_recycle_capable(void *buf_p)
{
	return gbpm_g.is_buf_hw_recycle_capable(buf_p);
}

static inline void gbpm_recycle_hw_buf(void *buf_p, uint32_t recycle_context)
{
	return gbpm_g.recycle_hw_buf(buf_p, recycle_context);
}

static inline int gbpm_register_hw_pool_api(void *ops_p, void *id_p)
{
	return gbpm_g.register_hw_pool_api(ops_p, id_p);
}

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
void gbpm_mark_buf(void *buf_p, void *addr, int reftype, int driver, int value,
		   int info);
void gbpm_add_ref(void *buf_p, int i);

#define GBPM_TRACK_BUF(buf, drv, value, info) \
	gbpm_mark_buf((void *)(buf), (void *)0, GBPM_REF_BUFF, (drv), (value), \
		      (info))

#define GBPM_TRACK_SKB(skb, drv, value, info) \
	gbpm_mark_buf((void *)((skb)->data), (void *)(skb), GBPM_REF_SKB, \
		      (drv), (value), (info))

#define GBPM_TRACK_FKB(fkb, drv, value, info) \
	gbpm_mark_buf((void *)((fkb)->data), (void *)(fkb), GBPM_REF_FKB, \
		      (drv), (value), (info)) \

#define GBPM_INC_REF(buf)	gbpm_add_ref((buf), 1)
#define GBPM_DEC_REF(buf)	gbpm_add_ref((buf), -1)
#else
#define GBPM_TRACK_BUF(buf, drv, value, info)	do {} while (0)
#define GBPM_TRACK_SKB(skb, drv, value, info)	do {} while (0)
#define GBPM_TRACK_FKB(fkb, drv, value, info)	do {} while (0)
#define GBPM_INC_REF(buf)			do {} while (0)
#define GBPM_DEC_REF(buf)			do {} while (0)
#endif

#endif /* defined(__GBPM_H_INCLUDED__) */
