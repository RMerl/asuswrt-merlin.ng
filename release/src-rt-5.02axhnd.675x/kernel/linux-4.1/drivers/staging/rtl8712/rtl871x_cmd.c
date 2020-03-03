/******************************************************************************
 * rtl871x_cmd.c
 *
 * Copyright(c) 2007 - 2010 Realtek Corporation. All rights reserved.
 * Linux device driver for RTL8192SU
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * Modifications for inclusion into the Linux staging tree are
 * Copyright(c) 2010 Larry Finger. All rights reserved.
 *
 * Contact information:
 * WLAN FAE <wlanfae@realtek.com>
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 ******************************************************************************/

#define _RTL871X_CMD_C_

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/usb.h>
#include <linux/usb/ch9.h>
#include <linux/circ_buf.h>
#include <linux/uaccess.h>
#include <asm/byteorder.h>
#include <linux/atomic.h>
#include <linux/semaphore.h>
#include <linux/rtnetlink.h>

#include "osdep_service.h"
#include "drv_types.h"
#include "recv_osdep.h"
#include "mlme_osdep.h"

/*
Caller and the r8712_cmd_thread can protect cmd_q by spin_lock.
No irqsave is necessary.
*/

static sint _init_cmd_priv(struct cmd_priv *pcmdpriv)
{
	sema_init(&(pcmdpriv->cmd_queue_sema), 0);
	sema_init(&(pcmdpriv->terminate_cmdthread_sema), 0);

	_init_queue(&(pcmdpriv->cmd_queue));

	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */
	pcmdpriv->cmd_seq = 1;
	pcmdpriv->cmd_allocated_buf = kmalloc(MAX_CMDSZ + CMDBUFF_ALIGN_SZ,
					      GFP_ATOMIC);
	if (pcmdpriv->cmd_allocated_buf == NULL)
		return _FAIL;
	pcmdpriv->cmd_buf = pcmdpriv->cmd_allocated_buf  +  CMDBUFF_ALIGN_SZ -
			    ((addr_t)(pcmdpriv->cmd_allocated_buf) &
			    (CMDBUFF_ALIGN_SZ-1));
	pcmdpriv->rsp_allocated_buf = kmalloc(MAX_RSPSZ + 4, GFP_ATOMIC);
	if (pcmdpriv->rsp_allocated_buf == NULL)
		return _FAIL;
	pcmdpriv->rsp_buf = pcmdpriv->rsp_allocated_buf  +  4 -
			    ((addr_t)(pcmdpriv->rsp_allocated_buf) & 3);
	pcmdpriv->cmd_issued_cnt = 0;
	pcmdpriv->cmd_done_cnt = 0;
	pcmdpriv->rsp_cnt = 0;
	return _SUCCESS;
}

static sint _init_evt_priv(struct evt_priv *pevtpriv)
{
	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */
	pevtpriv->event_seq = 0;
	pevtpriv->evt_allocated_buf = kmalloc(MAX_EVTSZ + 4, GFP_ATOMIC);

	if (pevtpriv->evt_allocated_buf == NULL)
		return _FAIL;
	pevtpriv->evt_buf = pevtpriv->evt_allocated_buf  +  4 -
			    ((addr_t)(pevtpriv->evt_allocated_buf) & 3);
	pevtpriv->evt_done_cnt = 0;
	return _SUCCESS;
}

static void _free_evt_priv(struct evt_priv *pevtpriv)
{
	kfree(pevtpriv->evt_allocated_buf);
}

static void _free_cmd_priv(struct cmd_priv *pcmdpriv)
{
	if (pcmdpriv) {
		kfree(pcmdpriv->cmd_allocated_buf);
		kfree(pcmdpriv->rsp_allocated_buf);
	}
}

/*
Calling Context:

_enqueue_cmd can only be called between kernel thread,
since only spin_lock is used.

ISR/Call-Back functions can't call this sub-function.

*/

static sint _enqueue_cmd(struct  __queue *queue, struct cmd_obj *obj)
{
	unsigned long irqL;

	if (obj == NULL)
		return _SUCCESS;
	spin_lock_irqsave(&queue->lock, irqL);
	list_add_tail(&obj->list, &queue->queue);
	spin_unlock_irqrestore(&queue->lock, irqL);
	return _SUCCESS;
}

static struct cmd_obj *_dequeue_cmd(struct  __queue *queue)
{
	unsigned long irqL;
	struct cmd_obj *obj;

	spin_lock_irqsave(&(queue->lock), irqL);
	if (list_empty(&(queue->queue)))
		obj = NULL;
	else {
		obj = LIST_CONTAINOR(queue->queue.next,
				     struct cmd_obj, list);
		list_del_init(&obj->list);
	}
	spin_unlock_irqrestore(&(queue->lock), irqL);
	return obj;
}

u32 r8712_init_cmd_priv(struct cmd_priv *pcmdpriv)
{
	return _init_cmd_priv(pcmdpriv);
}

u32 r8712_init_evt_priv(struct evt_priv *pevtpriv)
{
	return _init_evt_priv(pevtpriv);
}

void r8712_free_evt_priv(struct evt_priv *pevtpriv)
{
	_free_evt_priv(pevtpriv);
}

void r8712_free_cmd_priv(struct cmd_priv *pcmdpriv)
{
	_free_cmd_priv(pcmdpriv);
}

u32 r8712_enqueue_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *obj)
{
	int res;

	if (pcmdpriv->padapter->eeprompriv.bautoload_fail_flag == true)
		return _FAIL;
	res = _enqueue_cmd(&pcmdpriv->cmd_queue, obj);
	up(&pcmdpriv->cmd_queue_sema);
	return res;
}

u32 r8712_enqueue_cmd_ex(struct cmd_priv *pcmdpriv, struct cmd_obj *obj)
{
	unsigned long irqL;
	struct  __queue *queue;

	if (obj == NULL)
		return _SUCCESS;
	if (pcmdpriv->padapter->eeprompriv.bautoload_fail_flag == true)
		return _FAIL;
	queue = &pcmdpriv->cmd_queue;
	spin_lock_irqsave(&queue->lock, irqL);
	list_add_tail(&obj->list, &queue->queue);
	spin_unlock_irqrestore(&queue->lock, irqL);
	up(&pcmdpriv->cmd_queue_sema);
	return _SUCCESS;
}

struct cmd_obj *r8712_dequeue_cmd(struct  __queue *queue)
{
	return _dequeue_cmd(queue);
}

void r8712_free_cmd_obj(struct cmd_obj *pcmd)
{
	if ((pcmd->cmdcode != _JoinBss_CMD_) &&
	    (pcmd->cmdcode != _CreateBss_CMD_))
		kfree(pcmd->parmbuf);
	if (pcmd->rsp != NULL) {
		if (pcmd->rspsz != 0)
			kfree(pcmd->rsp);
	}
	kfree(pcmd);
}

/*
r8712_sitesurvey_cmd(~)
	### NOTE:#### (!!!!)
	MUST TAKE CARE THAT BEFORE CALLING THIS FUNC,
	 YOU SHOULD HAVE LOCKED pmlmepriv->lock
*/
u8 r8712_sitesurvey_cmd(struct _adapter *padapter,
			struct ndis_802_11_ssid *pssid)
{
	struct cmd_obj	*ph2c;
	struct sitesurvey_parm	*psurveyPara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psurveyPara = kmalloc(sizeof(*psurveyPara), GFP_ATOMIC);
	if (psurveyPara == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psurveyPara,
				   GEN_CMD_CODE(_SiteSurvey));
	psurveyPara->bsslimit = cpu_to_le32(48);
	psurveyPara->passive_mode = cpu_to_le32(pmlmepriv->passive_mode);
	psurveyPara->ss_ssidlen = 0;
	memset(psurveyPara->ss_ssid, 0, IW_ESSID_MAX_SIZE + 1);
	if ((pssid != NULL) && (pssid->SsidLength)) {
		memcpy(psurveyPara->ss_ssid, pssid->Ssid, pssid->SsidLength);
		psurveyPara->ss_ssidlen = cpu_to_le32(pssid->SsidLength);
	}
	set_fwstate(pmlmepriv, _FW_UNDER_SURVEY);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	mod_timer(&pmlmepriv->scan_to_timer,
		  jiffies + msecs_to_jiffies(SCANNING_TIMEOUT));
	padapter->ledpriv.LedControlHandler(padapter, LED_CTL_SITE_SURVEY);
	padapter->blnEnableRxFF0Filter = 0;
	return _SUCCESS;
}

u8 r8712_setdatarate_cmd(struct _adapter *padapter, u8 *rateset)
{
	struct cmd_obj		*ph2c;
	struct setdatarate_parm	*pbsetdataratepara;
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pbsetdataratepara = kmalloc(sizeof(*pbsetdataratepara), GFP_ATOMIC);
	if (pbsetdataratepara == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pbsetdataratepara,
				   GEN_CMD_CODE(_SetDataRate));
	pbsetdataratepara->mac_id = 5;
	memcpy(pbsetdataratepara->datarates, rateset, NumRates);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_set_chplan_cmd(struct _adapter *padapter, int chplan)
{
	struct cmd_obj *ph2c;
	struct SetChannelPlan_param *psetchplanpara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetchplanpara = kmalloc(sizeof(*psetchplanpara), GFP_ATOMIC);
	if (psetchplanpara == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetchplanpara,
				GEN_CMD_CODE(_SetChannelPlan));
	psetchplanpara->ChannelPlan = chplan;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setbasicrate_cmd(struct _adapter *padapter, u8 *rateset)
{
	struct cmd_obj *ph2c;
	struct setbasicrate_parm *pssetbasicratepara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pssetbasicratepara = kmalloc(sizeof(*pssetbasicratepara), GFP_ATOMIC);
	if (pssetbasicratepara == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pssetbasicratepara,
		_SetBasicRate_CMD_);
	memcpy(pssetbasicratepara->basicrates, rateset, NumRates);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

/* power tracking mechanism setting */
u8 r8712_setptm_cmd(struct _adapter *padapter, u8 type)
{
	struct cmd_obj		*ph2c;
	struct writePTM_parm	*pwriteptmparm;
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pwriteptmparm = kmalloc(sizeof(*pwriteptmparm), GFP_ATOMIC);
	if (pwriteptmparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pwriteptmparm, GEN_CMD_CODE(_SetPT));
	pwriteptmparm->type = type;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setfwdig_cmd(struct _adapter *padapter, u8 type)
{
	struct cmd_obj *ph2c;
	struct writePTM_parm *pwriteptmparm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pwriteptmparm = kmalloc(sizeof(*pwriteptmparm), GFP_ATOMIC);
	if (pwriteptmparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pwriteptmparm, GEN_CMD_CODE(_SetDIG));
	pwriteptmparm->type = type;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setfwra_cmd(struct _adapter *padapter, u8 type)
{
	struct cmd_obj *ph2c;
	struct writePTM_parm *pwriteptmparm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pwriteptmparm = kmalloc(sizeof(*pwriteptmparm), GFP_ATOMIC);
	if (pwriteptmparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pwriteptmparm, GEN_CMD_CODE(_SetRA));
	pwriteptmparm->type = type;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setrfreg_cmd(struct _adapter  *padapter, u8 offset, u32 val)
{
	struct cmd_obj *ph2c;
	struct writeRF_parm *pwriterfparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pwriterfparm = kmalloc(sizeof(*pwriterfparm), GFP_ATOMIC);
	if (pwriterfparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, pwriterfparm, GEN_CMD_CODE(_SetRFReg));
	pwriterfparm->offset = offset;
	pwriterfparm->value = val;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_getrfreg_cmd(struct _adapter *padapter, u8 offset, u8 *pval)
{
	struct cmd_obj *ph2c;
	struct readRF_parm *prdrfparm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	prdrfparm = kmalloc(sizeof(*prdrfparm), GFP_ATOMIC);
	if (prdrfparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	INIT_LIST_HEAD(&ph2c->list);
	ph2c->cmdcode = GEN_CMD_CODE(_GetRFReg);
	ph2c->parmbuf = (unsigned char *)prdrfparm;
	ph2c->cmdsz =  sizeof(struct readRF_parm);
	ph2c->rsp = pval;
	ph2c->rspsz = sizeof(struct readRF_rsp);
	prdrfparm->offset = offset;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

void r8712_getbbrfreg_cmdrsp_callback(struct _adapter *padapter,
				      struct cmd_obj *pcmd)
{
	kfree(pcmd->parmbuf);
	kfree(pcmd);
	padapter->mppriv.workparam.bcompleted = true;
}

void r8712_readtssi_cmdrsp_callback(struct _adapter *padapter,
				struct cmd_obj *pcmd)
{
	kfree(pcmd->parmbuf);
	kfree(pcmd);

	padapter->mppriv.workparam.bcompleted = true;
}

u8 r8712_createbss_cmd(struct _adapter *padapter)
{
	struct cmd_obj *pcmd;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct wlan_bssid_ex *pdev_network =
				 &padapter->registrypriv.dev_network;

	padapter->ledpriv.LedControlHandler(padapter, LED_CTL_START_TO_LINK);
	pcmd = kmalloc(sizeof(*pcmd), GFP_ATOMIC);
	if (pcmd == NULL)
		return _FAIL;
	INIT_LIST_HEAD(&pcmd->list);
	pcmd->cmdcode = _CreateBss_CMD_;
	pcmd->parmbuf = (unsigned char *)pdev_network;
	pcmd->cmdsz = r8712_get_ndis_wlan_bssid_ex_sz((
			struct ndis_wlan_bssid_ex *)
			pdev_network);
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;
	/* notes: translate IELength & Length after assign to cmdsz; */
	pdev_network->Length = pcmd->cmdsz;
	pdev_network->IELength = pdev_network->IELength;
	pdev_network->Ssid.SsidLength =	pdev_network->Ssid.SsidLength;
	r8712_enqueue_cmd(pcmdpriv, pcmd);
	return _SUCCESS;
}

u8 r8712_joinbss_cmd(struct _adapter  *padapter, struct wlan_network *pnetwork)
{
	uint t_len = 0;
	struct ndis_wlan_bssid_ex *psecnetwork;
	struct cmd_obj		*pcmd;
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct qos_priv		*pqospriv = &pmlmepriv->qospriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	enum NDIS_802_11_NETWORK_INFRASTRUCTURE ndis_network_mode = pnetwork->
						network.InfrastructureMode;

	padapter->ledpriv.LedControlHandler(padapter, LED_CTL_START_TO_LINK);
	pcmd = kmalloc(sizeof(*pcmd), GFP_ATOMIC);
	if (pcmd == NULL)
		return _FAIL;
	t_len = sizeof(u32) + 6 * sizeof(unsigned char) + 2 +
			sizeof(struct ndis_802_11_ssid) + sizeof(u32) +
			sizeof(s32) +
			sizeof(enum NDIS_802_11_NETWORK_TYPE) +
			sizeof(struct NDIS_802_11_CONFIGURATION) +
			sizeof(enum NDIS_802_11_NETWORK_INFRASTRUCTURE) +
			sizeof(NDIS_802_11_RATES_EX) +
			sizeof(u32) + MAX_IE_SZ;

	/* for hidden ap to set fw_state here */
	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE) !=
	    true) {
		switch (ndis_network_mode) {
		case Ndis802_11IBSS:
			pmlmepriv->fw_state |= WIFI_ADHOC_STATE;
			break;
		case Ndis802_11Infrastructure:
			pmlmepriv->fw_state |= WIFI_STATION_STATE;
			break;
		case Ndis802_11APMode:
		case Ndis802_11AutoUnknown:
		case Ndis802_11InfrastructureMax:
			break;
		}
	}
	psecnetwork = (struct ndis_wlan_bssid_ex *)&psecuritypriv->sec_bss;
	if (psecnetwork == NULL) {
		kfree(pcmd);
		return _FAIL;
	}
	memcpy(psecnetwork, &pnetwork->network, t_len);
	psecuritypriv->authenticator_ie[0] = (unsigned char)
					     psecnetwork->IELength;
	if ((psecnetwork->IELength-12) < (256 - 1))
		memcpy(&psecuritypriv->authenticator_ie[1],
			&psecnetwork->IEs[12], psecnetwork->IELength-12);
	else
		memcpy(&psecuritypriv->authenticator_ie[1],
			&psecnetwork->IEs[12], (256-1));
	psecnetwork->IELength = 0;
	/* If the driver wants to use the bssid to create the connection.
	 * If not,  we copy the connecting AP's MAC address to it so that
	 * the driver just has the bssid information for PMKIDList searching.
	 */
	if (pmlmepriv->assoc_by_bssid == false)
		ether_addr_copy(&pmlmepriv->assoc_bssid[0],
				&pnetwork->network.MacAddress[0]);
	psecnetwork->IELength = r8712_restruct_sec_ie(padapter,
						&pnetwork->network.IEs[0],
						&psecnetwork->IEs[0],
						pnetwork->network.IELength);
	pqospriv->qos_option = 0;
	if (pregistrypriv->wmm_enable) {
		u32 tmp_len;

		tmp_len = r8712_restruct_wmm_ie(padapter,
					  &pnetwork->network.IEs[0],
					  &psecnetwork->IEs[0],
					  pnetwork->network.IELength,
					  psecnetwork->IELength);
		if (psecnetwork->IELength != tmp_len) {
			psecnetwork->IELength = tmp_len;
			pqospriv->qos_option = 1; /* WMM IE in beacon */
		} else
			pqospriv->qos_option = 0; /* no WMM IE in beacon */
	}
	if (pregistrypriv->ht_enable) {
		/* For WEP mode, we will use the bg mode to do the connection
		 * to avoid some IOT issues, especially for Realtek 8192u
		 * SoftAP.
		 */
		if ((padapter->securitypriv.PrivacyAlgrthm != _WEP40_) &&
		    (padapter->securitypriv.PrivacyAlgrthm != _WEP104_)) {
			/* restructure_ht_ie */
			r8712_restructure_ht_ie(padapter,
						&pnetwork->network.IEs[0],
						&psecnetwork->IEs[0],
						pnetwork->network.IELength,
						&psecnetwork->IELength);
		}
	}
	psecuritypriv->supplicant_ie[0] = (u8)psecnetwork->IELength;
	if (psecnetwork->IELength < 255)
		memcpy(&psecuritypriv->supplicant_ie[1], &psecnetwork->IEs[0],
			psecnetwork->IELength);
	else
		memcpy(&psecuritypriv->supplicant_ie[1], &psecnetwork->IEs[0],
			255);
	/* get cmdsz before endian conversion */
	pcmd->cmdsz = r8712_get_ndis_wlan_bssid_ex_sz(psecnetwork);
#ifdef __BIG_ENDIAN
	/* wlan_network endian conversion */
	psecnetwork->Length = cpu_to_le32(psecnetwork->Length);
	psecnetwork->Ssid.SsidLength = cpu_to_le32(
				       psecnetwork->Ssid.SsidLength);
	psecnetwork->Privacy = cpu_to_le32(psecnetwork->Privacy);
	psecnetwork->Rssi = cpu_to_le32(psecnetwork->Rssi);
	psecnetwork->NetworkTypeInUse = cpu_to_le32(
					psecnetwork->NetworkTypeInUse);
	psecnetwork->Configuration.ATIMWindow = cpu_to_le32(
				psecnetwork->Configuration.ATIMWindow);
	psecnetwork->Configuration.BeaconPeriod = cpu_to_le32(
				 psecnetwork->Configuration.BeaconPeriod);
	psecnetwork->Configuration.DSConfig = cpu_to_le32(
				psecnetwork->Configuration.DSConfig);
	psecnetwork->Configuration.FHConfig.DwellTime = cpu_to_le32(
				psecnetwork->Configuration.FHConfig.DwellTime);
	psecnetwork->Configuration.FHConfig.HopPattern = cpu_to_le32(
				psecnetwork->Configuration.FHConfig.HopPattern);
	psecnetwork->Configuration.FHConfig.HopSet = cpu_to_le32(
				psecnetwork->Configuration.FHConfig.HopSet);
	psecnetwork->Configuration.FHConfig.Length = cpu_to_le32(
				psecnetwork->Configuration.FHConfig.Length);
	psecnetwork->Configuration.Length = cpu_to_le32(
				psecnetwork->Configuration.Length);
	psecnetwork->InfrastructureMode = cpu_to_le32(
				psecnetwork->InfrastructureMode);
	psecnetwork->IELength = cpu_to_le32(psecnetwork->IELength);
#endif
	INIT_LIST_HEAD(&pcmd->list);
	pcmd->cmdcode = _JoinBss_CMD_;
	pcmd->parmbuf = (unsigned char *)psecnetwork;
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;
	r8712_enqueue_cmd(pcmdpriv, pcmd);
	return _SUCCESS;
}

u8 r8712_disassoc_cmd(struct _adapter *padapter) /* for sta_mode */
{
	struct cmd_obj *pdisconnect_cmd;
	struct disconnect_parm *pdisconnect;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	pdisconnect_cmd = kmalloc(sizeof(*pdisconnect_cmd), GFP_ATOMIC);
	if (pdisconnect_cmd == NULL)
		return _FAIL;
	pdisconnect = kmalloc(sizeof(*pdisconnect), GFP_ATOMIC);
	if (pdisconnect == NULL) {
		kfree(pdisconnect_cmd);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(pdisconnect_cmd, pdisconnect,
				   _DisConnect_CMD_);
	r8712_enqueue_cmd(pcmdpriv, pdisconnect_cmd);
	return _SUCCESS;
}

u8 r8712_setopmode_cmd(struct _adapter *padapter,
		 enum NDIS_802_11_NETWORK_INFRASTRUCTURE networktype)
{
	struct cmd_obj *ph2c;
	struct setopmode_parm *psetop;

	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetop = kmalloc(sizeof(*psetop), GFP_ATOMIC);
	if (psetop == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetop, _SetOpMode_CMD_);
	psetop->mode = (u8)networktype;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setstakey_cmd(struct _adapter *padapter, u8 *psta, u8 unicast_key)
{
	struct cmd_obj *ph2c;
	struct set_stakey_parm *psetstakey_para;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct set_stakey_rsp *psetstakey_rsp = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct sta_info *sta = (struct sta_info *)psta;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetstakey_para = kmalloc(sizeof(*psetstakey_para), GFP_ATOMIC);
	if (psetstakey_para == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	psetstakey_rsp = kmalloc(sizeof(*psetstakey_rsp), GFP_ATOMIC);
	if (psetstakey_rsp == NULL) {
		kfree(ph2c);
		kfree(psetstakey_para);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);
	ph2c->rsp = (u8 *) psetstakey_rsp;
	ph2c->rspsz = sizeof(struct set_stakey_rsp);
	ether_addr_copy(psetstakey_para->addr, sta->hwaddr);
	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
		psetstakey_para->algorithm = (unsigned char)
					    psecuritypriv->PrivacyAlgrthm;
	else
		GET_ENCRY_ALGO(psecuritypriv, sta,
			       psetstakey_para->algorithm, false);
	if (unicast_key == true)
		memcpy(&psetstakey_para->key, &sta->x_UncstKey, 16);
	else
		memcpy(&psetstakey_para->key,
			&psecuritypriv->XGrpKey[
			psecuritypriv->XGrpKeyid - 1]. skey, 16);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setrfintfs_cmd(struct _adapter *padapter, u8 mode)
{
	struct cmd_obj *ph2c;
	struct setrfintfs_parm *psetrfintfsparm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetrfintfsparm = kmalloc(sizeof(*psetrfintfsparm), GFP_ATOMIC);
	if (psetrfintfsparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetrfintfsparm,
				   GEN_CMD_CODE(_SetRFIntFs));
	psetrfintfsparm->rfintfs = mode;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setrttbl_cmd(struct _adapter *padapter,
		      struct setratable_parm *prate_table)
{
	struct cmd_obj *ph2c;
	struct setratable_parm *psetrttblparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetrttblparm = kmalloc(sizeof(*psetrttblparm), GFP_ATOMIC);
	if (psetrttblparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetrttblparm,
				   GEN_CMD_CODE(_SetRaTable));
	memcpy(psetrttblparm, prate_table, sizeof(struct setratable_parm));
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_gettssi_cmd(struct _adapter *padapter, u8 offset, u8 *pval)
{
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj *ph2c;
	struct readTSSI_parm *prdtssiparm;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	prdtssiparm = kmalloc(sizeof(*prdtssiparm), GFP_ATOMIC);
	if (prdtssiparm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	INIT_LIST_HEAD(&ph2c->list);
	ph2c->cmdcode = GEN_CMD_CODE(_ReadTSSI);
	ph2c->parmbuf = (unsigned char *)prdtssiparm;
	ph2c->cmdsz = sizeof(struct readTSSI_parm);
	ph2c->rsp = pval;
	ph2c->rspsz = sizeof(struct readTSSI_rsp);

	prdtssiparm->offset = offset;
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setMacAddr_cmd(struct _adapter *padapter, u8 *mac_addr)
{
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj *ph2c;
	struct SetMacAddr_param	*psetMacAddr_para;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetMacAddr_para = kmalloc(sizeof(*psetMacAddr_para), GFP_ATOMIC);
	if (psetMacAddr_para == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetMacAddr_para,
				   _SetMacAddress_CMD_);
	ether_addr_copy(psetMacAddr_para->MacAddr, mac_addr);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_setassocsta_cmd(struct _adapter *padapter, u8 *mac_addr)
{
	struct cmd_priv			*pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj			*ph2c;
	struct set_assocsta_parm	*psetassocsta_para;
	struct set_assocsta_rsp		*psetassocsta_rsp = NULL;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	psetassocsta_para = kmalloc(sizeof(*psetassocsta_para), GFP_ATOMIC);
	if (psetassocsta_para == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	psetassocsta_rsp = kmalloc(sizeof(*psetassocsta_rsp), GFP_ATOMIC);
	if (psetassocsta_rsp == NULL) {
		kfree(ph2c);
		kfree(psetassocsta_para);
		return _FAIL;
	}
	init_h2fwcmd_w_parm_no_rsp(ph2c, psetassocsta_para, _SetAssocSta_CMD_);
	ph2c->rsp = (u8 *) psetassocsta_rsp;
	ph2c->rspsz = sizeof(struct set_assocsta_rsp);
	ether_addr_copy(psetassocsta_para->addr, mac_addr);
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_addbareq_cmd(struct _adapter *padapter, u8 tid)
{
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj		*ph2c;
	struct addBaReq_parm	*paddbareq_parm;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	paddbareq_parm = kmalloc(sizeof(*paddbareq_parm), GFP_ATOMIC);
	if (paddbareq_parm == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	paddbareq_parm->tid = tid;
	init_h2fwcmd_w_parm_no_rsp(ph2c, paddbareq_parm,
				   GEN_CMD_CODE(_AddBAReq));
	r8712_enqueue_cmd_ex(pcmdpriv, ph2c);
	return _SUCCESS;
}

u8 r8712_wdg_wk_cmd(struct _adapter *padapter)
{
	struct cmd_obj *ph2c;
	struct drvint_cmd_parm  *pdrvintcmd_param;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	pdrvintcmd_param = kmalloc(sizeof(*pdrvintcmd_param), GFP_ATOMIC);
	if (pdrvintcmd_param == NULL) {
		kfree(ph2c);
		return _FAIL;
	}
	pdrvintcmd_param->i_cid = WDG_WK_CID;
	pdrvintcmd_param->sz = 0;
	pdrvintcmd_param->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvintcmd_param, _DRV_INT_CMD_);
	r8712_enqueue_cmd_ex(pcmdpriv, ph2c);
	return _SUCCESS;
}

void r8712_survey_cmd_callback(struct _adapter *padapter, struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (pcmd->res != H2C_SUCCESS)
		clr_fwstate(pmlmepriv, _FW_UNDER_SURVEY);
	r8712_free_cmd_obj(pcmd);
}

void r8712_disassoc_cmd_callback(struct _adapter *padapter,
				 struct cmd_obj *pcmd)
{
	unsigned long irqL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (pcmd->res != H2C_SUCCESS) {
		spin_lock_irqsave(&pmlmepriv->lock, irqL);
		set_fwstate(pmlmepriv, _FW_LINKED);
		spin_unlock_irqrestore(&pmlmepriv->lock, irqL);
		return;
	}
	r8712_free_cmd_obj(pcmd);
}

void r8712_joinbss_cmd_callback(struct _adapter *padapter, struct cmd_obj *pcmd)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (pcmd->res != H2C_SUCCESS)
		mod_timer(&pmlmepriv->assoc_timer,
			  jiffies + msecs_to_jiffies(1));
	r8712_free_cmd_obj(pcmd);
}

void r8712_createbss_cmd_callback(struct _adapter *padapter,
				  struct cmd_obj *pcmd)
{
	unsigned long irqL;
	struct sta_info *psta = NULL;
	struct wlan_network *pwlan = NULL;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct ndis_wlan_bssid_ex *pnetwork = (struct ndis_wlan_bssid_ex *)
					      pcmd->parmbuf;
	struct wlan_network *tgt_network = &(pmlmepriv->cur_network);

	if (pcmd->res != H2C_SUCCESS)
		mod_timer(&pmlmepriv->assoc_timer,
			  jiffies + msecs_to_jiffies(1));
	del_timer(&pmlmepriv->assoc_timer);
#ifdef __BIG_ENDIAN
	/* endian_convert */
	pnetwork->Length = le32_to_cpu(pnetwork->Length);
	pnetwork->Ssid.SsidLength = le32_to_cpu(pnetwork->Ssid.SsidLength);
	pnetwork->Privacy = le32_to_cpu(pnetwork->Privacy);
	pnetwork->Rssi = le32_to_cpu(pnetwork->Rssi);
	pnetwork->NetworkTypeInUse = le32_to_cpu(pnetwork->NetworkTypeInUse);
	pnetwork->Configuration.ATIMWindow = le32_to_cpu(pnetwork->
					Configuration.ATIMWindow);
	pnetwork->Configuration.DSConfig = le32_to_cpu(pnetwork->
					Configuration.DSConfig);
	pnetwork->Configuration.FHConfig.DwellTime = le32_to_cpu(pnetwork->
					Configuration.FHConfig.DwellTime);
	pnetwork->Configuration.FHConfig.HopPattern = le32_to_cpu(pnetwork->
					Configuration.FHConfig.HopPattern);
	pnetwork->Configuration.FHConfig.HopSet = le32_to_cpu(pnetwork->
					Configuration.FHConfig.HopSet);
	pnetwork->Configuration.FHConfig.Length = le32_to_cpu(pnetwork->
					Configuration.FHConfig.Length);
	pnetwork->Configuration.Length = le32_to_cpu(pnetwork->
					Configuration.Length);
	pnetwork->InfrastructureMode = le32_to_cpu(pnetwork->
					   InfrastructureMode);
	pnetwork->IELength = le32_to_cpu(pnetwork->IELength);
#endif
	spin_lock_irqsave(&pmlmepriv->lock, irqL);
	if ((pmlmepriv->fw_state) & WIFI_AP_STATE) {
		psta = r8712_get_stainfo(&padapter->stapriv,
					 pnetwork->MacAddress);
		if (!psta) {
			psta = r8712_alloc_stainfo(&padapter->stapriv,
						   pnetwork->MacAddress);
			if (psta == NULL)
				goto createbss_cmd_fail;
		}
		r8712_indicate_connect(padapter);
	} else {
		pwlan = _r8712_alloc_network(pmlmepriv);
		if (pwlan == NULL) {
			pwlan = r8712_get_oldest_wlan_network(
				&pmlmepriv->scanned_queue);
			if (pwlan == NULL)
				goto createbss_cmd_fail;
			pwlan->last_scanned = jiffies;
		} else
			list_add_tail(&(pwlan->list),
					 &pmlmepriv->scanned_queue.queue);
		pnetwork->Length = r8712_get_ndis_wlan_bssid_ex_sz(pnetwork);
		memcpy(&(pwlan->network), pnetwork, pnetwork->Length);
		pwlan->fixed = true;
		memcpy(&tgt_network->network, pnetwork,
			(r8712_get_ndis_wlan_bssid_ex_sz(pnetwork)));
		if (pmlmepriv->fw_state & _FW_UNDER_LINKING)
			pmlmepriv->fw_state ^= _FW_UNDER_LINKING;
		/* we will set _FW_LINKED when there is one more sat to
		 * join us (stassoc_event_callback) */
	}
createbss_cmd_fail:
	spin_unlock_irqrestore(&pmlmepriv->lock, irqL);
	r8712_free_cmd_obj(pcmd);
}

void r8712_setstaKey_cmdrsp_callback(struct _adapter *padapter,
				     struct cmd_obj *pcmd)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct set_stakey_rsp *psetstakey_rsp = (struct set_stakey_rsp *)
						(pcmd->rsp);
	struct sta_info *psta = r8712_get_stainfo(pstapriv,
						  psetstakey_rsp->addr);

	if (psta == NULL)
		goto exit;
	psta->aid = psta->mac_id = psetstakey_rsp->keyid; /*CAM_ID(CAM_ENTRY)*/
exit:
	r8712_free_cmd_obj(pcmd);
}

void r8712_setassocsta_cmdrsp_callback(struct _adapter *padapter,
				       struct cmd_obj *pcmd)
{
	unsigned long	irqL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct set_assocsta_parm *passocsta_parm =
				(struct set_assocsta_parm *)(pcmd->parmbuf);
	struct set_assocsta_rsp *passocsta_rsp =
				(struct set_assocsta_rsp *) (pcmd->rsp);
	struct sta_info *psta = r8712_get_stainfo(pstapriv,
						  passocsta_parm->addr);

	if (psta == NULL)
		return;
	psta->aid = psta->mac_id = passocsta_rsp->cam_id;
	spin_lock_irqsave(&pmlmepriv->lock, irqL);
	if ((check_fwstate(pmlmepriv, WIFI_MP_STATE)) &&
	    (check_fwstate(pmlmepriv, _FW_UNDER_LINKING)))
		pmlmepriv->fw_state ^= _FW_UNDER_LINKING;
	set_fwstate(pmlmepriv, _FW_LINKED);
	spin_unlock_irqrestore(&pmlmepriv->lock, irqL);
	r8712_free_cmd_obj(pcmd);
}

u8 r8712_disconnectCtrlEx_cmd(struct _adapter *adapter, u32 enableDrvCtrl,
			u32 tryPktCnt, u32 tryPktInterval, u32 firstStageTO)
{
	struct cmd_obj *ph2c;
	struct DisconnectCtrlEx_param *param;
	struct cmd_priv *pcmdpriv = &adapter->cmdpriv;

	ph2c = kmalloc(sizeof(*ph2c), GFP_ATOMIC);
	if (ph2c == NULL)
		return _FAIL;
	param = kzalloc(sizeof(*param), GFP_ATOMIC);
	if (param == NULL) {
		kfree(ph2c);
		return _FAIL;
	}

	param->EnableDrvCtrl = (unsigned char)enableDrvCtrl;
	param->TryPktCnt = (unsigned char)tryPktCnt;
	param->TryPktInterval = (unsigned char)tryPktInterval;
	param->FirstStageTO = (unsigned int)firstStageTO;

	init_h2fwcmd_w_parm_no_rsp(ph2c, param,
				GEN_CMD_CODE(_DisconnectCtrlEx));
	r8712_enqueue_cmd(pcmdpriv, ph2c);
	return _SUCCESS;
}
