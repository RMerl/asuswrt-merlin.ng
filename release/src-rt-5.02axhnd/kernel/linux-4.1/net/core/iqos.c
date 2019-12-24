#if defined(CONFIG_BCM_KF_NBUFF)
/*
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
 * File Name  : iqos.c
 *******************************************************************************
 */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/iqos.h>
#include <linux/bcm_colors.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/ip.h>
#if defined(CONFIG_BCM_KF_LOG)
#include <linux/bcm_log.h>
#endif


/* Debug macros */
#if defined(CC_IQOS_SUPPORT_DEBUG)
#define iqos_print(fmt, arg...)	\
	if (iqos_debug_g)	\
		printk(CLRc "IQOS %s :" fmt CLRnl, __func__, ##arg)	\
#define iqos_assertv(cond)	\
	if (!cond) {		\
		printk(CLRerr "IQOS ASSERT %s : " #cond CLRnl, __func__);	\
		return;		\
	}
#define iqos_assertr(cond, rtn)	\
	if (!cond) {		\
		printk(CLRerr "IQOS ASSERT %s : " #cond CLRnl, __func__);	\
		return rtn;	\
	}
#define IQOS_DBG(debug_code)	do { debug_code } while(0)
#else
#ifndef NULL_STMT
#define NULL_STMT		do { /* NULL BODY */ } while (0)
#endif
#define iqos_print(fmt, arg...)	NULL_STMT
#define iqos_assertv(cond)	NULL_STMT
#define iqos_assertr(cond, rtn)	NULL_STMT
#define IQOS_DBG(debug_code)	NULL_STMT
#endif

#define iqos_error(fmt, arg...)	\
	printk(CLRerr "IQOS ERROR %s :" fmt CLRnl, __func__, ##arg)

#undef IQOS_DECL
#define IQOS_DECL(x) #x,	/* string declaration */


/*--- globals ---*/
uint32_t iqos_enable_g = 0;	/* Enable Ingress QoS feature */
uint32_t iqos_cpu_cong_g = 0;
uint32_t iqos_debug_g = 0;

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
DEFINE_SPINLOCK(iqos_lock_g);
EXPORT_SYMBOL(iqos_lock_g);
DEFINE_SPINLOCK(iqos_cong_lock_g);
EXPORT_SYMBOL(iqos_cong_lock_g);
#endif

/*
 *------------------------------------------------------------------------------
 * Default Ingress QoS hooks.
 *------------------------------------------------------------------------------
 */
static iqos_common_hook_t iqos_add_keymask_hook_g = NULL;
static iqos_common_hook_t iqos_rem_keymask_hook_g = NULL;
static iqos_common_hook_t iqos_add_key_hook_g = NULL;
static iqos_common_hook_t iqos_rem_key_hook_g = NULL;
static iqos_common_hook_t iqos_get_key_hook_g = NULL;
static iqos_int_hook_t iqos_set_status_hook_g = NULL;
static iqos_void_hook_t iqos_flush_hook_g = NULL;

/*
 *------------------------------------------------------------------------------
 * Get the Ingress QoS priority for L4 Dest port (layer4 UDP or TCP)
 *------------------------------------------------------------------------------
 */
int iqos_prio_L4port(uint8_t ipProto, uint16_t destPort)
{
	iqos_param_t param;
	int rc;

	iqos_key_param_start(&param);
	iqos_key_param_field_set(&param, IQOS_FIELD_IP_PROTO, (uint32_t *)&ipProto, 1);
	iqos_key_param_field_set(&param, IQOS_FIELD_DST_PORT, (uint32_t *)&destPort, 2);

	rc = iqos_key_commit_and_get(&param);
	if (rc)
		return IQOS_PRIO_LOW;

	if (param.action != IQOS_ACTION_PRIO)
		return IQOS_PRIO_LOW;

	return param.action_value;
}


/*
 *------------------------------------------------------------------------------
 * Add the Ingress QoS priority, and type of entry for L4 Dest port.
 *------------------------------------------------------------------------------
 */
int iqos_add_L4port(uint8_t ipProto, uint16_t destPort, iqos_ent_t ent,
		    iqos_prio_t prio)
{
	iqos_param_t param;
	int rc;

#if defined(CONFIG_BCM_KF_LOG)
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ,
		      "AddPort ent<%d> ipProto<%d> dport<%d> prio<%d> ",
		      ent, ipProto, destPort, prio);
#endif

	iqos_key_param_start(&param);
	iqos_key_param_field_set(&param, IQOS_FIELD_IP_PROTO, (uint32_t *)&ipProto, 1);
	iqos_key_param_field_set(&param, IQOS_FIELD_DST_PORT, (uint32_t *)&destPort, 2);
	iqos_key_param_action_set(&param, IQOS_ACTION_PRIO, (uint32_t)prio);

	rc = iqos_key_commit_and_add(&param, ent);
#if defined(CONFIG_BCM_KF_LOG)
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "rc<%d>", rc);
#endif
	return rc;
}


/*
 *------------------------------------------------------------------------------
 * Remove the L4 dest port from the Ingress QoS priority table
 *------------------------------------------------------------------------------
 */
int iqos_rem_L4port(uint8_t ipProto, uint16_t destPort, iqos_ent_t ent)
{
	iqos_param_t param;
	int rc;

#if defined(CONFIG_BCM_KF_LOG)
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "RemPort ent<%d> ipProto<%d> dport<%d> ",
		      ent, ipProto, destPort);
#endif

	iqos_key_param_start(&param);
	iqos_key_param_field_set(&param, IQOS_FIELD_IP_PROTO, (uint32_t *)&ipProto, 1);
	iqos_key_param_field_set(&param, IQOS_FIELD_DST_PORT, (uint32_t *)&destPort, 2);

	rc = iqos_key_commit_and_delete(&param, ent);
#if defined(CONFIG_BCM_KF_LOG)
	BCM_LOG_DEBUG(BCM_LOG_ID_IQ, "rc<%d> ", rc);
#endif
	return rc;
}

int iqos_keymask_param_start(iqos_param_t *param)
{
	if (param == NULL)
		return -EINVAL;

	memset(param, 0x0, sizeof(iqos_param_t));
	param->param_type = IQOS_PARAM_TYPE_KEYMASK;

	return 0;
}

int iqos_keymask_param_field_set(iqos_param_t *param, uint32_t field, uint32_t *val_ptr)
{
	uint32_t offset_type;

	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEYMASK)
		return -EINVAL;

	if (field >= IQOS_FIELD_MAX)
		return -EINVAL;
	
	param->field_mask |= 1 << field;

	/* for offset type of keymask, it requires more parameter */
	switch (field) {
	case IQOS_FIELD_OFFSET_0_TYPE:
		offset_type = *(uint32_t *)val_ptr;
		if (offset_type >= IQOS_OFFSET_TYPE_MAX)
			return -EINVAL;
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_0;
		param->offset0.type = offset_type;
		break;
	case IQOS_FIELD_OFFSET_0_START:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_0;
		param->offset0.start = *(uint32_t *)val_ptr;
		break;
	case IQOS_FIELD_OFFSET_0_SIZE:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_0;
		param->offset0.size = *(uint32_t *)val_ptr;
		break;
	case IQOS_FIELD_OFFSET_0_MASK:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_0;
		param->offset0.mask = *(uint32_t *)val_ptr;
		break;
	case IQOS_FIELD_OFFSET_1_TYPE:
		offset_type = *(uint32_t *)val_ptr;
		if (offset_type >= IQOS_OFFSET_TYPE_MAX)
			return -EINVAL;
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_1;
		param->offset1.type = offset_type;
		break;
	case IQOS_FIELD_OFFSET_1_START:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_1;
		param->offset1.start = *(uint32_t *)val_ptr;
		break;
	case IQOS_FIELD_OFFSET_1_SIZE:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_1;
		param->offset1.size = *(uint32_t *)val_ptr;
		break;
	case IQOS_FIELD_OFFSET_1_MASK:
		param->field_mask |= 1 << IQOS_FIELD_OFFSET_1;
		param->offset1.mask = *(uint32_t *)val_ptr;
		break;
	default:
		break;
	}

	return 0;
}

int iqos_keymask_commit_and_add(iqos_param_t *param, uint8_t prio)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEYMASK)
		return -EINVAL;

	if (unlikely(iqos_add_keymask_hook_g == NULL))
		return -EPERM;

	param->prio = prio;

	return iqos_add_keymask_hook_g(param);
}

int iqos_keymask_commit_and_delete(iqos_param_t *param)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEYMASK)
		return -EINVAL;

	if (unlikely(iqos_rem_keymask_hook_g == NULL))
		return -EPERM;

	return iqos_rem_keymask_hook_g(param);
}

int iqos_key_param_start(iqos_param_t *param)
{
	if (param == NULL)
		return -EINVAL;

	memset(param, 0x0, sizeof(iqos_param_t));
	param->param_type = IQOS_PARAM_TYPE_KEY;

	return 0;
}

int iqos_key_param_field_set(iqos_param_t *param, uint32_t field,
			     uint32_t *val_ptr, uint32_t val_size)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEY)
		return -EINVAL;

	if (field >= IQOS_FIELD_MAX)
		return -EINVAL;
	
	param->field_mask |= 1 << field;

	switch (field) {
	case IQOS_FIELD_INGRESS_DEVICE:
		/* FIXME!! implement this */
		if (val_size != 4)
			return -EINVAL;
		param->data.ingress_device = *val_ptr;
		break;
	case IQOS_FIELD_SRC_MAC:
		if (val_size != ETH_ALEN)
			return -EINVAL;
		memcpy(param->data.src_mac, val_ptr, val_size);
		break;
	case IQOS_FIELD_DST_MAC:
		if (val_size != ETH_ALEN)
			return -EINVAL;
		memcpy(param->data.dst_mac, val_ptr, val_size);
		break;
	case IQOS_FIELD_ETHER_TYPE:
		if (val_size != 2)
			return -EINVAL;
		param->data.eth_type = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_OUTER_VID:
		if (val_size != 2)
			return -EINVAL;
		param->data.outer_vid = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_OUTER_PBIT:
		if (val_size != 1)
			return -EINVAL;
		param->data.outer_pbit = *((uint8_t *)val_ptr);
		break;
	case IQOS_FIELD_INNER_VID:
		if (val_size != 2)
			return -EINVAL;
		param->data.inner_vid = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_INNER_PBIT:
		if (val_size != 1)
			return -EINVAL;
		param->data.inner_pbit = *((uint8_t *)val_ptr);
		break;
	case IQOS_FIELD_L2_PROTO:
		if (val_size != 2)
			return -EINVAL;
		param->data.l2_proto = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_L3_PROTO:
		if (val_size != 2)
			return -EINVAL;
		param->data.l3_proto = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_IP_PROTO:
		if (val_size != 1)
			return -EINVAL;
		param->data.ip_proto = *((uint8_t *)val_ptr);
		break;
	case IQOS_FIELD_SRC_IP:
		if (val_size == 4) {	/* IPV4 case */
			/* someone has set the IQOS to do IPv6 */
			if (param->data.is_ipv6 == 1)
				return -EINVAL;
			param->data.src_ip[0] = *val_ptr;
		} else if (val_size == 16) { /* IPV6 case */
			memcpy(param->data.src_ip, val_ptr, val_size);
			param->data.is_ipv6 = 1;
		} else
			return -EINVAL;
		break;
	case IQOS_FIELD_DST_IP:
		if (val_size == 4) {	/* IPV4 case */
			/* someone has set the IQOS to do IPv6 */
			if (param->data.is_ipv6 == 1)
				return -EINVAL;
			param->data.dst_ip[0] = *val_ptr;
		} else if (val_size == 16) { /* IPV6 case */
			memcpy(param->data.dst_ip, val_ptr, val_size);
			param->data.is_ipv6 = 1;
		} else
			return -EINVAL;
		break;
	case IQOS_FIELD_DSCP:
		if (val_size != 1)
			return -EINVAL;

		param->data.dscp = *((uint8_t *)val_ptr);
		break;
	case IQOS_FIELD_IPV6_FLOW_LABEL:
		if (val_size != 4)
			return -EINVAL;

		param->data.is_ipv6 = 1;
		param->data.flow_label = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_SRC_PORT:
		if (val_size != 2)
			return -EINVAL;
		param->data.l4_src_port = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_DST_PORT:
		if (val_size != 2)
			return -EINVAL;
		param->data.l4_dst_port = *((uint16_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_0_TYPE:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.type = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_0_START:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.type = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_0_SIZE:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.size = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_0_MASK:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.mask = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_1_TYPE:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.type = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_1_START:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.type = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_1_SIZE:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.size = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_1_MASK:
		if (val_size != 4)
			return -EINVAL;
		param->offset0.mask = *((uint32_t *)val_ptr);
		break;
	case IQOS_FIELD_OFFSET_0:
		/* FIXME!! implement getting some offset-used value */
	case IQOS_FIELD_OFFSET_1:
		/* FIXME!! implement getting some offset-used value */
	default:
		break;
	}

	return 0;
}

int iqos_key_param_action_set(iqos_param_t *param, uint32_t action,
			      uint32_t value)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEY)
		return -EINVAL;

	if (action >= IQOS_ACTION_MAX)
		return -EINVAL;

	param->action = action;
	param->action_value = value;

	return 0;
}

int iqos_key_commit_and_add(iqos_param_t *param, uint8_t type)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEY)
		return -EINVAL;

	if (unlikely(iqos_add_key_hook_g == NULL))
		return -EPERM;

	if (type >= IQOS_ENT_MAX)
		return -EINVAL;
	param->type = type;

	return iqos_add_key_hook_g(param);
}

int iqos_key_commit_and_delete(iqos_param_t *param, uint8_t type)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEY)
		return -EINVAL;

	if (unlikely(iqos_rem_key_hook_g == NULL))
		return -EPERM;

	if (type >= IQOS_ENT_MAX)
		return -EINVAL;
	param->type = type;

	return iqos_rem_key_hook_g(param);
}

int iqos_key_commit_and_get(iqos_param_t *param)
{
	if (param == NULL)
		return -EINVAL;

	if (param->param_type != IQOS_PARAM_TYPE_KEY)
		return -EINVAL;

	if (unlikely(iqos_get_key_hook_g == NULL))
		return -EPERM;

	return iqos_get_key_hook_g(param);
}

/* flush out all the existing dynamic entry, also delete keymask if they are not used
 * by static entries */
void iqos_flush(void)
{
	if (likely(iqos_flush_hook_g != NULL))
		iqos_flush_hook_g();
}

int iqos_set_status(uint32_t status)
{
	if (likely(iqos_set_status_hook_g != NULL))
		return iqos_set_status_hook_g(status);
	return -EPERM;
}

/*
 *------------------------------------------------------------------------------
 * Function     : iqos_bind
 * Description  : Override default hooks.
 *------------------------------------------------------------------------------
 */
void iqos_bind(iqos_common_hook_t iqos_add_keymask,
	       iqos_common_hook_t iqos_rem_keymask,
	       iqos_common_hook_t iqos_add_key,
	       iqos_common_hook_t iqos_rem_key,
	       iqos_common_hook_t iqos_get_key,
	       iqos_int_hook_t iqos_set_status,
	       iqos_void_hook_t iqos_flush)
{
	iqos_print("Bind add_keymask[<%p>] rem_keymask[<%p>] add_key[<%p>] "
		   "rem_key[<%p>] get_key[<%p>] flush[<%p>\n",
		   iqos_add_keymask, iqos_rem_keymask, iqos_add_key,
		   iqos_rem_key, iqos_get_key, iqos_flush);

	iqos_add_keymask_hook_g = iqos_add_keymask;
	iqos_rem_keymask_hook_g = iqos_rem_keymask;
	iqos_add_key_hook_g = iqos_add_key;
	iqos_rem_key_hook_g = iqos_rem_key;
	iqos_get_key_hook_g = iqos_get_key;
	iqos_set_status_hook_g = iqos_set_status;
	iqos_flush_hook_g = iqos_flush;
}

EXPORT_SYMBOL(iqos_cpu_cong_g);
EXPORT_SYMBOL(iqos_enable_g);
EXPORT_SYMBOL(iqos_debug_g);
EXPORT_SYMBOL(iqos_add_L4port);
EXPORT_SYMBOL(iqos_rem_L4port);
EXPORT_SYMBOL(iqos_prio_L4port);
EXPORT_SYMBOL(iqos_bind);
EXPORT_SYMBOL(iqos_flush);
EXPORT_SYMBOL(iqos_set_status);

EXPORT_SYMBOL(iqos_keymask_param_start);
EXPORT_SYMBOL(iqos_keymask_param_field_set);
EXPORT_SYMBOL(iqos_keymask_commit_and_add);
EXPORT_SYMBOL(iqos_keymask_commit_and_delete);

EXPORT_SYMBOL(iqos_key_param_start);
EXPORT_SYMBOL(iqos_key_param_field_set);
EXPORT_SYMBOL(iqos_key_param_action_set);
EXPORT_SYMBOL(iqos_key_commit_and_add);
EXPORT_SYMBOL(iqos_key_commit_and_delete);
EXPORT_SYMBOL(iqos_key_commit_and_get);

/*
 *------------------------------------------------------------------------------
 * Function     : __init_iqos
 * Description  : Static construction of ingress QoS subsystem.
 *------------------------------------------------------------------------------
 */
static int __init __init_iqos(void)
{
	printk( IQOS_MODNAME IQOS_VER_STR " initialized\n" );
	return 0;
}

subsys_initcall(__init_iqos);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

/* get the congestion status for system */
iqos_cong_status_t iqos_get_sys_cong_status(void)
{
	return ((iqos_cpu_cong_g)? IQOS_CONG_STATUS_HI : IQOS_CONG_STATUS_LO);
}


/* get the congestion status for an RX channel of an interface */
iqos_cong_status_t iqos_get_cong_status(iqos_if_t iface, uint32_t chnl)
{
	return ((iqos_cpu_cong_g & (1 << (iface + chnl))) ?
			IQOS_CONG_STATUS_HI : IQOS_CONG_STATUS_LO);
}


/* set/reset the congestion status for an RX channel of an interface */
uint32_t iqos_set_cong_status(iqos_if_t iface, uint32_t chnl,
			      iqos_cong_status_t status)
{
	unsigned long flags;

	IQOS_LOCK_IRQSAVE();

	if (status == IQOS_CONG_STATUS_HI)
		iqos_cpu_cong_g |= (1 << (iface + chnl));
	else
		iqos_cpu_cong_g &= ~(1 << (iface + chnl));

	IQOS_UNLOCK_IRQRESTORE();

	return iqos_cpu_cong_g;
}

EXPORT_SYMBOL(iqos_get_cong_status);
EXPORT_SYMBOL(iqos_set_cong_status);

#endif /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)) */
#endif

