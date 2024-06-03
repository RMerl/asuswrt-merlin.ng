/*
 *
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom
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

/*
 *******************************************************************************
 * File Name  : gbpm.c
 *******************************************************************************
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/gbpm.h>
#include <linux/bcm_colors.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#include <linux/bcm_log_mod.h>
#include <linux/bcm_log.h>
#include <linux/bcm_realtime.h>

/* Global Buffer Pool Manager (BPM) */

#undef GBPM_DECL
#define GBPM_DECL(HOOKNAME) .HOOKNAME = (gbpm_ ## HOOKNAME ## _hook_t)NULL,
gbpm_t gbpm_g = {
	GBPM_BIND() /* Static initialization to NULL of BPM "BIND" hooks */
	GBPM_USER() /* Static initialization to NULL of BPM "USER" hooks */
	.debug = 0U,
	.is_hw_buffer_mngr_registered = false
}; /* End of static initialization of gbpm globals */
EXPORT_SYMBOL(gbpm_g);

/* Debug macros */

#if defined(CC_GBPM_SUPPORT_DEBUG)
#define gbpm_print(fmt, arg...) \
	do { \
		if (gbpm_g.debug) \
			pr_debug(CLRc "GBPM %s :" fmt CLRnl, __func__, ##arg) \
	} while (0)

#define gbpm_assertv(cond) \
	do { \
		if (!cond) { \
			pr_debug(CLRerr "GBPM ASSERT %s : " #cond CLRnl, \
				__func__); \
			return; \
		} \
	} while (0)

#define gbpm_assertr(cond, rtn) \
	do { \
		if (!cond) { \
			pr_debug(CLRerr "GBPM ASSERT %s : " #cond CLRnl, \
				__func__); \
			return rtn; \
		} \
	} while (0)

#define GBPM_DBG(debug_code)	do {debug_code} while (0)
#else
#define gbpm_print(fmt, arg...)	NULL_STMT
#define gbpm_assertv(cond)	NULL_STMT
#define gbpm_assertr(cond, rtn)	NULL_STMT
#define GBPM_DBG(debug_code)	NULL_STMT
#endif

#define gbpm_error(fmt, arg...) \
	pr_err(CLRerr "GBPM ERROR %s :" fmt CLRnl, __func__, ##arg)


/*
 * ----------------------------------------------------------------------------
 * STUBS: Default stubs, until BPM binds its service handlers.
 * ----------------------------------------------------------------------------
 */
/* --- BPM BUF POOL --- */
int gbpm_alloc_mult_buf_stub(uint32_t num, void **buf_p, uint32_t prio)
{
	return GBPM_ERROR;
}

void gbpm_free_mult_buf_stub(uint32_t num, void **buf_p)
{
}

void *gbpm_alloc_buf_stub(void)
{
	return NULL;
}

void gbpm_free_buf_stub(void *buf_p)
{
}

/* --- BPM SKB POOL --- */
uint32_t gbpm_total_skb_stub(void)
{
	return 0U;
}

uint32_t gbpm_avail_skb_stub(void)
{
	return 0U;
}

void gbpm_attach_skb_stub(void *skbp, void *data, uint32_t datalen)
{
}

void *gbpm_alloc_skb_stub(void)
{
	return NULL;
}

void *gbpm_alloc_buf_skb_attach_stub(uint32_t datalen)
{
	return NULL;
}

void *gbpm_alloc_mult_skb_stub(uint32_t num)
{
	return NULL;
}

void gbpm_free_skb_stub(void *skbp)
{
}

void gbpm_free_skblist_stub(void *head, void *tail, uint32_t num,
			    void **bufp_arr)
{
}

void *gbpm_invalidate_dirtyp_stub(void *skb)
{
	return NULL;
}

void gbpm_recycle_skb_stub(void *skbp, unsigned long context,
			   uint32_t recycle_action)
{
}

/* --- BPM pNBuff --- */
void gbpm_recycle_pNBuff_stub(void *pNBuff, unsigned long context,
			      uint32_t recycle_action)
{
}

/* --- BPM Get Accessors --- */
int gbpm_get_dyn_buf_lvl_stub(void)
{
	return 1;
}

uint32_t gbpm_get_total_bufs_stub(void)
{
	return 0;
}

uint32_t gbpm_get_avail_bufs_stub(void)
{
	return 0;
}

uint32_t gbpm_get_max_dyn_bufs_stub(void)
{
	return 0;
}

/* --- BPM HW Buffer Manager ---*/
bool gbpm_is_buf_hw_recycle_capable_stub(void *buf_p)
{
	return false;
}

void gbpm_recycle_hw_buf_stub(void *buf_p, uint32_t recycle_context)
{
}

int gbpm_register_hw_pool_api_stub(void *ops_p, void *id_p)
{
	return 0;
}

#if defined(CONFIG_BCM_BPM_BUF_TRACKING)

gbpm_mark_buf_hook_t gbpm_mark_buf_hook_g = (gbpm_mark_buf_hook_t)NULL;
EXPORT_SYMBOL(gbpm_mark_buf_hook_g);

gbpm_add_ref_hook_t gbpm_add_ref_hook_g = (gbpm_add_ref_hook_t)NULL;
EXPORT_SYMBOL(gbpm_add_ref_hook_g);

void gbpm_mark_buf(void *buf_p, void *addr, int reftype, int driver, int value,
		   int info)
{
	if (gbpm_mark_buf_hook_g != NULL)
		return gbpm_mark_buf_hook_g(buf_p, addr, reftype, driver,
					    value, info);
}
EXPORT_SYMBOL(gbpm_mark_buf);

void gbpm_add_ref(void *buf_p, int i)
{
	if (gbpm_mark_buf_hook_g != NULL)
		return gbpm_add_ref_hook_g(buf_p, i);
}
EXPORT_SYMBOL(gbpm_add_ref);
#endif

/*
 * function   : gbpm_register_hw_buffer_manager
 * description: for HW buffer manager to register to GBPM
 */
int gbpm_register_hw_buffer_manager(struct gbpm_hw_buffer_mngr *new_mngr)
{
	int rc;
	struct module *bpm_mod;

	if ((new_mngr == NULL) || (new_mngr->pool_init == NULL))
		return -EINVAL;

	memcpy(&gbpm_g.hw_buffer_mngr, new_mngr,
		sizeof(struct gbpm_hw_buffer_mngr));
	gbpm_g.is_hw_buffer_mngr_registered = true;

	bpm_mod = find_module("bcm_bpm");
	if (bpm_mod != NULL) {
		pr_info("BPM framework has been inserted! Pool init starts!\n");

		rc = gbpm_g.hw_buffer_mngr.pool_init();
		if (rc) {
			pr_err("HW Buffer Manager Pool init failed!\n");
			return rc;
		}
	}

	pr_info("HW Buffer Manager %s has registered to GBPM!\n", gbpm_g.hw_buffer_mngr.name);

	return 0;
}
EXPORT_SYMBOL(gbpm_register_hw_buffer_manager);

/*
 *------------------------------------------------------------------------------
 * Function	 : gbpm_bind
 * Description  : Override default hooks.
 *------------------------------------------------------------------------------
 */
#undef  GBPM_DECL
#define GBPM_DECL(HOOKNAME) gbpm_ ## HOOKNAME ## _hook_t HOOKNAME,

void gbpm_bind(GBPM_BIND() uint32_t debug)
{
	uint32_t saved_debug = gbpm_g.debug;

	gbpm_g.debug = debug; /* Debug print */

#undef  GBPM_DECL
#define GBPM_DECL(HOOKNAME) gbpm_print("Hook:%s[<%px>]", #HOOKNAME, HOOKNAME);
	GBPM_BIND()

	/* --- Bind BPM hooks --- */
#undef  GBPM_DECL
#define GBPM_DECL(HOOKNAME) gbpm_g.HOOKNAME = HOOKNAME;
	GBPM_BIND()

	gbpm_g.debug = saved_debug; /* restore gbpm debug level */
}
EXPORT_SYMBOL(gbpm_bind);

/*
 *------------------------------------------------------------------------------
 * Function	 : gbpm_unbind
 * Description  : use default stub hooks.
 *------------------------------------------------------------------------------
 */
void gbpm_unbind(void)
{
#undef GBPM_DECL
#define GBPM_DECL(HOOKNAME) gbpm_ ## HOOKNAME ## _stub,
	gbpm_bind(GBPM_BIND() gbpm_g.debug);
}
EXPORT_SYMBOL(gbpm_unbind);

/*
 *------------------------------------------------------------------------------
 * Function	 : __init_gbpm
 * Description  : Static construction of global buffer pool manager subsystem.
 *------------------------------------------------------------------------------
 */
static int __init __init_gbpm(void)
{
	gbpm_unbind();

	pr_info(GBPM_MODNAME GBPM_VER_STR " initialized\n");
	return 0;
}

subsys_initcall(__init_gbpm);

