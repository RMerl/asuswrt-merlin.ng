#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mxl_br_vlan.h"

#include <gsw_device.h>
#include <gsw.h>
#include <gsw_api.h>

/* init during switch init, updated dynamically to indicate
 * Bridge Port (BP) in use
 */
uint64_t tag_based_brp_bmap[2] = {0};
struct ctp_ext_vlan_t ctp_ext_vlan_cfg[16] = {0};

/* All BP to vlan information */
struct bpid_vlan_data bp_vlan_data[128];

typedef struct {
	GSW_ExtendedVlan_4_Tpid_Mode_t tpid;
	GSW_ExtendedVlanTreatmentTpid_t outer_etpid;
	GSW_ExtendedVlanTreatmentTpid_t inner_etpid;
} tpid_lookup;

static const tpid_lookup tpid_lookup_matrix[4][4] = {
	[GSW_EXTENDEDVLAN_TPID_VTETYPE_1] = { // (88a8)
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_1] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_1, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_2] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_1, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_8021Q },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_3] = { 0xFF, 0xFF, 0xFF }, // Unsupported
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_4] = { 0xFF, 0xFF, 0xFF }, // Unsupported
	},
	[GSW_EXTENDEDVLAN_TPID_VTETYPE_2] = { // (8100)
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_1] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_1, GSW_EXTENDEDVLAN_TREATMENT_8021Q, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_2] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_2, GSW_EXTENDEDVLAN_TREATMENT_8021Q, GSW_EXTENDEDVLAN_TREATMENT_8021Q },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_3] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_3, GSW_EXTENDEDVLAN_TREATMENT_8021Q, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_4] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_4, GSW_EXTENDEDVLAN_TREATMENT_8021Q, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
	},
	[GSW_EXTENDEDVLAN_TPID_VTETYPE_3] = { // (9100)
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_1] = { 0xFF, 0xFF, 0xFF }, // Unsupported
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_2] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_3, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_8021Q },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_3] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_3, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_4] = { 0xFF, 0xFF, 0xFF }, // Unsupported
	},
	[GSW_EXTENDEDVLAN_TPID_VTETYPE_4] = { // (9200)
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_1] = { 0xFF, 0xFF, 0xFF }, // Unsupported
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_2] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_4, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_8021Q },
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_3] = { 0xFF, 0xFF, 0xFF }, // Unsupported
		[GSW_EXTENDEDVLAN_TPID_VTETYPE_4] = { GSW_EXTENDEDVLAN_TPID_VTETYPE_4, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE, GSW_EXTENDEDVLAN_TREATMENT_VTETYPE },
	}
};

static int tpid_to_trment_mode_type(uint8_t outer_tpid, uint8_t inner_tpid,
				    GSW_ExtendedVlan_4_Tpid_Mode_t *tpid,
				    GSW_ExtendedVlanTreatmentTpid_t *outer_etpid,
				    GSW_ExtendedVlanTreatmentTpid_t *inner_etpid)
{
	if (outer_tpid >= 4 || inner_tpid >= 4) {
		return -1; // Invalid TPID values
	}

	const tpid_lookup *entry = &tpid_lookup_matrix[outer_tpid][inner_tpid];
	if (entry->tpid == 0xFF)
		return -1;

	/* Assign the outputs */
	*tpid = entry->tpid;
	*outer_etpid = entry->outer_etpid;
	*inner_etpid = entry->inner_etpid;

	return 0; // Success
}

static int tpid_from_trment_mode_type(
	GSW_ExtendedVlan_4_Tpid_Mode_t tpid,
	GSW_ExtendedVlanTreatmentTpid_t outer_etpid,
	GSW_ExtendedVlanTreatmentTpid_t inner_etpid,
	uint8_t *outer_tpid, uint8_t *inner_tpid)
{
	for (int outer = 0; outer < 4; outer++) {
		for (int inner = 0; inner < 4; inner++) {
			const tpid_lookup *entry = &tpid_lookup_matrix[outer][inner];

			if (entry->tpid == 0xFF)
				continue;

			if (entry->tpid != tpid ||
			    entry->outer_etpid != outer_etpid ||
			    entry->inner_etpid != inner_etpid)
				continue;
			*outer_tpid = outer;
			*inner_tpid = inner;
			return 0;
		}
	}

	return -1;
}

int get_egr_brp_ext_vlan_dets(uint8_t br_port, uint8_t *en,
			      uint8_t *vtag_type, uint8_t *ctp,
			      uint8_t *bridge_id, uint16_t *ev_block_id,
			      uint16_t *ovlan_id, uint16_t *ivlan_id,
			      int8_t *ctp_vidx, uint8_t *outer_tpid,
			      uint8_t *inner_tpid)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	GSW_BRIDGE_portConfig_t brpCfg;
	GSW_EXTENDEDVLAN_config_t evcfg;
	struct ctp_ext_vlan_t *pcfg;
	GSW_ExtendedVlan_4_Tpid_Mode_t tpid = GSW_EXTENDEDVLAN_TPID_VTETYPE_1;
	GSW_ExtendedVlanTreatmentTpid_t outer_etpid = GSW_EXTENDEDVLAN_TREATMENT_INNER_TPID;
	GSW_ExtendedVlanTreatmentTpid_t inner_etpid = GSW_EXTENDEDVLAN_TREATMENT_INNER_TPID;

	/* all default values if bridge port not in use */
	if (!bpid_in_use(br_port)) {
		LOG_DBG("BR port %u not in use", br_port);
		return 0;
	}

	memset(&brpCfg, 0, sizeof(brpCfg));
	brpCfg.nBridgePortId = br_port;
	brpCfg.eMask = GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_ID;
	brpCfg.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN;
	brpCfg.eMask |= GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING;

	if (GSW_BridgePortConfigGet(dev, &brpCfg)) {
		LOG_ERR("BR port %u not allocated", brpCfg.nBridgePortId);
		return -EIO;
	}

	*ctp = brpCfg.nDestLogicalPortId + brpCfg.nDestSubIfIdGroup;
	*bridge_id = brpCfg.nBridgeId;

	if (!brpCfg.bEgressExtendedVlanEnable)
		return 0;

	memset(&evcfg, 0, sizeof(evcfg));

	evcfg.nExtendedVlanBlockId = brpCfg.nEgressExtendedVlanBlockId;
	if (ev_block_id)
		*ev_block_id = evcfg.nExtendedVlanBlockId;
	if (GSW_ExtendedVlanGet(dev, &evcfg)) {
		LOG_ERR("Get EVLAN block %d failed for BR port %u",
			evcfg.nExtendedVlanBlockId,
			brpCfg.nBridgePortId);
		return -EIO;
	}

	if (evcfg.sTreatment.bAddOuterVlan) {
		*en = 1; /* at least add outer VLAN tag */
		*vtag_type = 0;
		*ovlan_id = evcfg.sTreatment.sOuterVlan.eVidVal;
		tpid = evcfg.sTreatment.eTreatment_4_Tpid_Mode;
		outer_etpid = evcfg.sTreatment.sOuterVlan.eTpid;
		inner_etpid = GSW_EXTENDEDVLAN_TREATMENT_8021Q; /* default inner etpid */
	}

	if (evcfg.sTreatment.bAddInnerVlan) {
		*vtag_type = 1;
		*ivlan_id = evcfg.sTreatment.sInnerVlan.eVidVal;
		inner_etpid = evcfg.sTreatment.sInnerVlan.eTpid;
	}

	/**
	  *If both outer etpid and inner etpid are GSW_EXTENDEDVLAN_TREATMENT_8021Q,
	  * hardware doesn't care about tpid mode.
	  * Then make eTreatment_4_Tpid_Mode to GSW_EXTENDEDVLAN_TPID_VTETYPE_2.
	 **/
	if (outer_etpid == GSW_EXTENDEDVLAN_TREATMENT_8021Q && inner_etpid == GSW_EXTENDEDVLAN_TREATMENT_8021Q)
		tpid = GSW_EXTENDEDVLAN_TPID_VTETYPE_2;

	if (tpid_from_trment_mode_type(tpid, outer_etpid, inner_etpid,
				       outer_tpid, inner_tpid)) {
		LOG_ERR("cannot convert treatment to outer/inner tpid");
		return -EIO;
	}

	/* fixed inner VID, no need update for wildcard */
	if (evcfg.sTreatment.bAddInnerVlan && !ctp_vidx)
		return 0;

	pcfg = &ctp_ext_vlan_cfg[brpCfg.nDestLogicalPortId & 15];
	assert(pcfg->alloc);

	/* search for CTP ingress VLAN entry to
	 * determine wildcard of inner VID
	 */
	for (uint32_t map = pcfg->entry_map, i = 0; map != 0; map >>= 1, i++) {
		memset(&evcfg, 0, sizeof(evcfg));

		evcfg.nExtendedVlanBlockId = pcfg->vlan_blockid;
		evcfg.nEntryIndex = VLAN_TAG(i);
		if (GSW_ExtendedVlanGet(dev, &evcfg)) {
			LOG_ERR("Get ing EVLAN index %d in block %d failed on CTP %u",
				evcfg.nEntryIndex, evcfg.nExtendedVlanBlockId,
				*ctp);
			continue;
		}

		if (!evcfg.sTreatment.bReassignBridgePort ||
		    evcfg.sTreatment.nNewBridgePortId != br_port)
			continue;

		if (evcfg.sFilter.sOuterVlan.nVidVal != *ovlan_id ||
		    evcfg.sFilter.sOuterVlan.eType != GSW_EXTENDEDVLAN_FILTER_TYPE_NORMAL)
			continue;

		if (*vtag_type) {
			/* VLAN entry has no inner tag or inner VID mismatch */
			if (evcfg.sFilter.sInnerVlan.eType != GSW_EXTENDEDVLAN_FILTER_TYPE_NORMAL ||
			    evcfg.sFilter.sInnerVlan.nVidVal != *ivlan_id)
				continue;
		} else if (evcfg.sFilter.sInnerVlan.eType == GSW_EXTENDEDVLAN_FILTER_TYPE_NO_FILTER) {
			/* inner VLAN tag is wildcard */
			*vtag_type = 1;
			*ivlan_id = 4096;
		} else {
			/* VLAN entry has inner tag */
			if (evcfg.sFilter.sInnerVlan.eType != GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG)
				continue;
		}

		/* found the CTP ingress VLAN entry */
		if (ctp_vidx)
			*ctp_vidx = i;

		break;
	}

	return 0;
}

static int ctp_ext_vlan_update(uint32_t ctp,
			       struct ctp_ext_vlan_t *pcfg)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint32_t new_en = pcfg->discard_untag ||
			  pcfg->discard_tag ||
			  pcfg->entry_map;
	GSW_CTP_portConfig_t ctpcfg = {0};

#ifdef ENABLE_CTP_REPLACE_DEBUG
	printk("%s:%u: port %u, discard_untag %u, discard_tag %u, entry_map 0x%08x, assigned %u, new_en %u\n",
	       __func__, __LINE__, ctp, pcfg->discard_untag,
	       pcfg->discard_tag, pcfg->entry_map, pcfg->assigned, new_en);
#endif

	/* no change */
	if (pcfg->assigned == new_en)
		return 0;

	ctpcfg.nLogicalPortId = ctp & 0x0F;
	ctpcfg.nSubIfIdGroup = ctp & ~0x0F;
	ctpcfg.eMask = GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN;
	ctpcfg.bIngressExtendedVlanEnable = new_en ? GSW_TRUE : GSW_FALSE;
	ctpcfg.nIngressExtendedVlanBlockId = pcfg->vlan_blockid;
	if (GSW_CtpPortConfigSet(dev, &ctpcfg)) {
		LOG_ERR("Failed in Extended VLAN block %u %s CTP %u",
			pcfg->vlan_blockid,
			new_en ? "assignment to" : "removal from", ctp);
		return -EIO;
	}

	pcfg->assigned = new_en;
	return 0;
}

static int update_vlan(const GSW_Device_t *dev,
		       uint8_t ctp, int entry, uint8_t bp, uint8_t en,
		       uint8_t vtype, uint16_t outer, uint16_t inner,
		       uint8_t tpid_mode, uint8_t outer_tpid, uint8_t inner_tpid)
{
	struct ctp_ext_vlan_t *pcfg = &ctp_ext_vlan_cfg[ctp & 15];
	GSW_EXTENDEDVLAN_config_t cfg = {0};
	int ret;

	assert(pcfg->alloc);

	/* no entry to disable */
	if (!en && entry < 0)
		return 0;

	/* search for free entry */
	if (entry < 0) {
		for (entry = 0;
		     entry < VLAN_ENTRY_MAP_SIZE &&
		     (pcfg->entry_map & BIT(entry)) != 0;
		     entry++);
		if (entry == VLAN_ENTRY_MAP_SIZE) {
			LOG_ERR("Can't find free VLAN entry on CTP %u", ctp);
			return -ENOENT;
		}
	}

	cfg.nExtendedVlanBlockId = pcfg->vlan_blockid;
	cfg.nEntryIndex = VLAN_TAG(entry);
	if (!en) {
		/* no action etnry */
		cfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
		cfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
		cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_NOT_REMOVE_TAG;
		cfg.sTreatment.bReassignBridgePort = GSW_FALSE;
	} else {
		cfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NORMAL;
		cfg.sFilter.sOuterVlan.bVidEnable = GSW_TRUE;
		cfg.sFilter.sOuterVlan.nVidVal = outer;
		cfg.sFilter.eFilter_4_Tpid_Mode = tpid_mode;
		if (vtype == 0) {
			/* signle tag (outer tag only) */
			cfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
			cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_REMOVE_1_TAG;
			if (outer_tpid == GSW_EXTENDEDVLAN_TPID_VTETYPE_2)
				cfg.sFilter.sOuterVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_8021Q;
			else
				cfg.sFilter.sOuterVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_VTETYPE;
		} else if (inner == 4096) {
			/* double tag with wildcard in inner tag
			 * do not remove inner tag
			 */
			cfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_FILTER;
			cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_REMOVE_1_TAG;
		} else {
			/* double tag */
			cfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NORMAL;
			cfg.sFilter.sInnerVlan.bVidEnable = GSW_TRUE;
			cfg.sFilter.sInnerVlan.nVidVal = inner;
			cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_REMOVE_2_TAG;

			/* outer tag filter */
			if (outer_tpid == GSW_EXTENDEDVLAN_TPID_VTETYPE_2)
				cfg.sFilter.sOuterVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_8021Q;
			else
				cfg.sFilter.sOuterVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_VTETYPE;

			/* inner tag filter */
			if (inner_tpid == GSW_EXTENDEDVLAN_TPID_VTETYPE_2)
				cfg.sFilter.sInnerVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_8021Q;
			else
				cfg.sFilter.sInnerVlan.eTpid = GSW_EXTENDEDVLAN_FILTER_TPID_VTETYPE;
		}
		cfg.sTreatment.bReassignBridgePort = GSW_TRUE;
		cfg.sTreatment.nNewBridgePortId = bp;
	}
	ret = GSW_ExtendedVlanSet(dev, &cfg);
	if (ret) {
		LOG_ERR("Can't config extended vlan %u+%u: %d\n",
			cfg.nExtendedVlanBlockId, cfg.nEntryIndex, ret);
		return -EIO;
	}

	if (en)
		pcfg->entry_map |= BIT(entry);
	else
		pcfg->entry_map &= ~BIT(entry);

	return ctp_ext_vlan_update(ctp, pcfg);
}

int rm_tag_vlan(uint8_t bp)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint8_t en = 0, vtag_type = 0, ctp = 0, bridge_id = 0;
	uint16_t ev_block_id = 0, ovlan_id = 0, ivlan_id = 0;
	int8_t ctp_vidx = -1;
	int ret;
	GSW_BRIDGE_portAlloc_t bpfree = {0};
	GSW_EXTENDEDVLAN_alloc_t evlanfree = {0};
	struct bpid_vlan_data *data;

	if (!is_vlan_bpid(bp))
		return -EINVAL;

	ret = get_egr_brp_ext_vlan_dets(bp, &en, &vtag_type, &ctp,
					&bridge_id, &ev_block_id, &ovlan_id,
					&ivlan_id, &ctp_vidx, NULL, NULL);
	if (ret) {
		LOG_ERR("API get_egr_brp_ext_vlan_dets failed for BR port %u",
			bp);
		return ret;
	}

	/* return if bridge port was not enabled */
	if (!en)
		return 0;

	/* disable this bridge port */
	bp_disable(bp);

	/* search ingress extended VLAN in CTP and free it */
	ret = update_vlan(dev, ctp, ctp_vidx, 0, 0, 0, 0, 0, 0, 0, 0);
	if (ret) {
		LOG_ERR("remove VLAN fail: %d\n", ret);
		return ret;
	}

	/* free bridge port */
	bpfree.nBridgePortId = bp;
	ret = GSW_BridgePortFree(dev, &bpfree);
	if (ret) {
		LOG_ERR("BRP %u free failed: %d", bp, ret);
		return -EIO;
	}

	/* free egress extended VLAN of bridge port */
	evlanfree.nExtendedVlanBlockId = ev_block_id;
	ret = GSW_ExtendedVlanFree(dev, &evlanfree);
	if (ret) {
		LOG_ERR("Freeing extended VLAN block %u assigned to BR port %u failed: %d",
			ev_block_id, bp, ret);
		return -EIO;
	}

	data = get_vlan_data_from_bpid(bp);
	data->ctp = INVALID_CTP;
	if (data->type == VLAN_TYPE_SINGLE_TAG) {
		data->vlan_data.tag_vlan.outer_tag.tag = NET_VLAN_TAG_UNSPEC;
	} else if (data->type == VLAN_TYPE_DOUBLE_TAG) {
		data->vlan_data.tag_vlan.outer_tag.tag = NET_VLAN_TAG_UNSPEC;
		data->vlan_data.tag_vlan.inner_tag.tag = NET_VLAN_TAG_UNSPEC;
	}

	return 0;
}

static uint16_t tpid_to_uint16(uint8_t tpid)
{
	switch (tpid) {
	case 0:
		return 0x88A8;
	case 1:
		return 0x8100;
	case 2:
		return 0x9100;
	case 3:
		return 0x9200;
	default:
		return 0x8100;
	}
}

int update_tag_vlan(uint8_t bp, uint8_t vtype, uint8_t ctp,
		    uint8_t fid, uint16_t outer, uint16_t inner, uint8_t outer_tpid, uint8_t inner_tpid)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint8_t en = 0, vtag_type = 0, phy_port = 0, bridge_id = 0, c_outer_tpid = 0, c_inner_tpid = 0;
	uint16_t ev_block_id = 0, ovlan_id = 0, ivlan_id = 0;
	int8_t ctp_vidx = -1;
	int ret;
	GSW_BRIDGE_portAlloc_t bpnew = {0};
	GSW_EXTENDEDVLAN_alloc_t vlannew = {0};
	GSW_BRIDGE_portConfig_t bpcfg = {0};
	GSW_EXTENDEDVLAN_config_t evlancfg = {0};
	struct bpid_vlan_data *data;
	GSW_ExtendedVlan_4_Tpid_Mode_t tpid = 0;
	GSW_ExtendedVlanTreatmentTpid_t outer_etpid = 0;
	GSW_ExtendedVlanTreatmentTpid_t inner_etpid = 0;

	if (!is_vlan_bpid(bp))
		return -EINVAL;

	ret = get_egr_brp_ext_vlan_dets(bp, &en, &vtag_type, &phy_port,
					&bridge_id, &ev_block_id, &ovlan_id,
					&ivlan_id, &ctp_vidx, &c_outer_tpid, &c_inner_tpid);
	if (ret) {
		LOG_ERR("API get_egr_brp_ext_vlan_dets failed for BR port %u",
			bp);
		return ret;
	}

	/* nothing to change */
	if (en && vtype == vtag_type && ctp == phy_port && fid == bridge_id &&
	    outer == ovlan_id && (vtype == 0 || inner == ivlan_id) &&
	    outer_tpid == c_outer_tpid && inner_tpid == c_inner_tpid)
		return 0;

	/* single tag, inner tpid default 8100*/
	if (vtype == 0)
		inner_tpid = 1;

	ret = tpid_to_trment_mode_type(outer_tpid, inner_tpid,
				       &tpid, &outer_etpid, &inner_etpid);
	if (ret) {
		LOG_ERR("Cannot convert outer/inner tpid to treatment: %d, %d", outer_tpid, inner_tpid);
		return -EIO;
	}

	if (!en) {
		/* allocate new bridge port if it was disabled */
		bpnew.nBridgePortId = bp;
		ret = GSW_BridgePortAlloc(dev, &bpnew);
		if (ret) {
			LOG_ERR("BRP %u alloc fail: %d", bp, ret);
			return -EIO;
		}

		/* update ingress extended VLAN of CTP */
		ret = update_vlan(dev, ctp, -1, bp, 1, vtype,
				  outer, inner, tpid, outer_tpid, inner_tpid);
		if (ret) {
			LOG_ERR("config VLAN fail: %d", ret);
			return ret;
		}

		/* allocate egress extended VLAN of bridge port */
		vlannew.nNumberOfEntries = 3;
		ret = GSW_ExtendedVlanAlloc(dev, &vlannew);
		if (ret) {
			LOG_ERR("BRP alloc failed: %d", ret);
			return -EIO;
		}

		/* configure egress bridge port */
		bpcfg.nBridgePortId = bp;
		bpcfg.eMask = GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_VLAN |
			      GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING |
			      GSW_BRIDGE_PORT_CONFIG_MASK_MC_DEST_IP_LOOKUP |
			      GSW_BRIDGE_PORT_CONFIG_MASK_MC_SRC_IP_LOOKUP;
		bpcfg.bEgressExtendedVlanEnable = GSW_TRUE;
		bpcfg.nEgressExtendedVlanBlockId = vlannew.nExtendedVlanBlockId;
		bpcfg.nDestLogicalPortId = ctp & 15;
		bpcfg.nDestSubIfIdGroup = ctp & ~15;
		bpcfg.bMcDestIpLookupDisable = 0;
		bpcfg.bMcSrcIpLookupEnable = 0;

		ret = GSW_BridgePortConfigSet(dev, &bpcfg);
		if (ret) {
			LOG_ERR("Set egr CTP map failed for BR port %u",
				bpcfg.nBridgePortId);
			return -EIO;
		}

		ev_block_id = vlannew.nExtendedVlanBlockId;
	} else {
		/* search ingress extended VLAN in CTP and update it */
#ifdef ENABLE_CTP_REPLACE_DEBUG
		printk("%s:search: port %u, vtype %u, outer %u, inner %u, ctp_vidx %d\n",
		       __func__, phy_port, vtag_type, ovlan_id, ivlan_id,
		       ctp_vidx);
#endif
		if (phy_port != ctp) {
			/* remove from old CTP if change to another CTP */
			ret = update_vlan(dev, phy_port, ctp_vidx,
					  0, 0, 0, 0, 0, 0, 0, 0);
			if (ret) {
				LOG_ERR("remove VLAN fail: %d\n", ret);
				return ret;
			}

			/* configure egress bridge port to CTP map */
			bpcfg.nBridgePortId = bp;
			bpcfg.eMask = GSW_BRIDGE_PORT_CONFIG_MASK_EGRESS_CTP_MAPPING;
			bpcfg.nDestLogicalPortId = ctp & 15;
			bpcfg.nDestSubIfIdGroup = ctp & ~15;
			ret = GSW_BridgePortConfigSet(dev, &bpcfg);
			if (ret) {
				LOG_ERR("Set egr CTP map failed for BR port %u",
					bpcfg.nBridgePortId);
				return -EIO;
			}

			ctp_vidx = -1;
		}
#ifdef ENABLE_CTP_REPLACE_DEBUG
		printk("%s:update: port %u, entry %d, bp %u, vtype %u, outer %u, inner %u\n",
		       __func__, ctp, ret, bp, vtype, outer, inner);
#endif
		ret = update_vlan(dev, ctp, ctp_vidx, bp, 1, vtype,
				  outer, inner, tpid, outer_tpid, inner_tpid);
#ifdef ENABLE_CTP_REPLACE_DEBUG
		printk("update: ret %d\n", ret);
#endif
		if (ret) {
			LOG_ERR("update VLAN fail: %d\n", ret);
			return ret;
		}
	}

	/* config egress extended VLAN */
	evlancfg.nExtendedVlanBlockId = ev_block_id;
	evlancfg.nEntryIndex = 0;
	evlancfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
	evlancfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
	evlancfg.sTreatment.eTreatment_4_Tpid_Mode = tpid;
	evlancfg.sTreatment.bAddOuterVlan = GSW_TRUE;
	evlancfg.sTreatment.sOuterVlan.eTpid = outer_etpid;
	evlancfg.sTreatment.sOuterVlan.eVidVal = outer;
	evlancfg.sTreatment.sOuterVlan.ePriorityMode = GSW_EXTENDEDVLAN_TREATMENT_OUTER_PRORITY;
	evlancfg.sTreatment.sOuterVlan.eDei = GSW_EXTENDEDVLAN_TREATMENT_OUTER_DEI;
	if (vtype && inner < 4096) {
		/* double VLAN */
		evlancfg.sTreatment.bAddInnerVlan = GSW_TRUE;
		evlancfg.sTreatment.sInnerVlan.eTpid = inner_etpid;
		evlancfg.sTreatment.sInnerVlan.eVidVal = inner;
		evlancfg.sTreatment.sInnerVlan.ePriorityMode = GSW_EXTENDEDVLAN_TREATMENT_INNER_PRORITY;
		evlancfg.sTreatment.sInnerVlan.eDei = GSW_EXTENDEDVLAN_TREATMENT_INNER_DEI;
	}
	ret = GSW_ExtendedVlanSet(dev, &evlancfg);
	if (ret) {
		LOG_ERR("Set egr EVLAN index %d in block %d failed for BR port %u",
			evlancfg.nEntryIndex, evlancfg.nExtendedVlanBlockId, bp);
		return -EIO;
	}

#if EG_VLAN_BLOCK_SIZE > 1
	evlancfg.nEntryIndex = 1;
	evlancfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT;
	evlancfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
	ret = GSW_ExtendedVlanSet(dev, &evlancfg);
	if (ret) {
		LOG_ERR("Set egr EVLAN index %d +1 in block %d failed for BR port %u",
			evlancfg.nEntryIndex, evlancfg.nExtendedVlanBlockId, bp);
		return -EIO;
	}
#endif

#if EG_VLAN_BLOCK_SIZE > 2
	evlancfg.nEntryIndex = 2;
	evlancfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT;
	evlancfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT;
	ret = GSW_ExtendedVlanSet(dev, &evlancfg);
	if (ret) {
		LOG_ERR("Set egr EVLAN index %d +2 in block %d failed for BR port %u",
			evlancfg.nEntryIndex, evlancfg.nExtendedVlanBlockId, bp);
		return -EIO;
	}
#endif

	/* enable bridge port */
	bp_enable(bp, fid);

	data = get_vlan_data_from_bpid(bp);
	data->ctp = ctp;
	data->type = vtype;
	if (data->type == VLAN_TYPE_SINGLE_TAG) {
		data->vlan_data.tag_vlan.outer_tag.tag = outer;
		data->vlan_data.tag_vlan.outer_tag.tpid = tpid_to_uint16(outer_tpid);
		data->vlan_data.tag_vlan.inner_tag.tpid = tpid_to_uint16(inner_tpid);
	} else if (data->type == VLAN_TYPE_DOUBLE_TAG) {
		data->vlan_data.tag_vlan.outer_tag.tag = outer;
		data->vlan_data.tag_vlan.inner_tag.tag = inner;
		data->vlan_data.tag_vlan.outer_tag.tpid = tpid_to_uint16(outer_tpid);
		data->vlan_data.tag_vlan.inner_tag.tpid = tpid_to_uint16(inner_tpid);
	}

	return 0;
}

int get_discard_cfg(enum DISCARD_TYPE type, uint32_t ctp)
{
	uint32_t idx = ctp & 0x0F;

	if (type == DISCARD_UNTAG) {
		return ctp_ext_vlan_cfg[idx].discard_untag;
	} else {
		return ctp_ext_vlan_cfg[idx].discard_tag;
	}
}

static int set_discard_untag(uint32_t ctp, struct ctp_ext_vlan_t *pcfg,
			     int enable)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	GSW_EXTENDEDVLAN_config_t cfg = {0};

	ARG_UNUSED(ctp);

	/* no change */
	if (pcfg->discard_untag == enable)
		return 0;

	cfg.nExtendedVlanBlockId = pcfg->vlan_blockid;
	cfg.nEntryIndex = VLAN_DISCARD_UNTAG;
	cfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
	cfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
	if (enable) {
		cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_DISCARD_UPSTREAM;
	} else {
		cfg.sTreatment.eRemoveTag = GSW_EXTENDEDVLAN_TREATMENT_NOT_REMOVE_TAG;
	}
	if (GSW_ExtendedVlanSet(dev, &cfg)) {
		LOG_ERR("Failed in Extended VLAN block %u entry %u config",
			cfg.nExtendedVlanBlockId, cfg.nEntryIndex);
		return -EIO;
	}

	pcfg->discard_untag = enable;
	return 0;
}

static int set_discard_tag(uint32_t ctp, struct ctp_ext_vlan_t *pcfg,
			   int enable)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint32_t outer[] = {
		GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT,
		GSW_EXTENDEDVLAN_FILTER_TYPE_NO_FILTER
	};
	uint32_t inner[] = {
		GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG,
		GSW_EXTENDEDVLAN_FILTER_TYPE_DEFAULT
	};
	uint32_t act;

	ARG_UNUSED(ctp);

	/* no change */
	if (pcfg->discard_tag == enable)
		return 0;

	if (enable) {
		act = GSW_EXTENDEDVLAN_TREATMENT_DISCARD_UPSTREAM;
	} else {
		act = GSW_EXTENDEDVLAN_TREATMENT_NOT_REMOVE_TAG;
	}
	for (uint32_t i = 0; i < 2; i++) {
		GSW_EXTENDEDVLAN_config_t cfg = {0};

		cfg.nExtendedVlanBlockId = pcfg->vlan_blockid;
		cfg.nEntryIndex = VLAN_DISCARD_TAG(i);
		cfg.sFilter.sOuterVlan.eType = outer[i];
		cfg.sFilter.sInnerVlan.eType = inner[i];
		cfg.sTreatment.eRemoveTag = act;
		if (GSW_ExtendedVlanSet(dev, &cfg)) {
			LOG_ERR("Failed in Extended VLAN block %u entry %u config",
				cfg.nExtendedVlanBlockId, cfg.nEntryIndex);
			return -EIO;
		}
	}

	pcfg->discard_tag = enable;
	return 0;
}

int set_discard_cfg(enum DISCARD_TYPE type, uint32_t ctp, bool enable)
{
	struct ctp_ext_vlan_t *pcfg = &ctp_ext_vlan_cfg[ctp & 0x0F];
	int new_en = enable ? 1 : 0;
	int ret;

	assert(ctp > 0 && ctp <= 16);
	assert(pcfg->alloc);

	if (type == DISCARD_UNTAG) {
		ret = set_discard_untag(ctp, pcfg, new_en);
	} else {
		ret = set_discard_tag(ctp, pcfg, new_en);
	}

	if (ret)
		return ret;

	return ctp_ext_vlan_update(ctp, pcfg);
}

int tag_vlan_init(void)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint8_t tot_ports = get_chip_port_num();
	GSW_register_t param = {0};

	for (uint8_t pnum = 1; pnum <= tot_ports; pnum++) {
		uint8_t hw_pnum = map_port_idx(pnum);
		uint8_t idx = hw_pnum & 15;
		GSW_EXTENDEDVLAN_alloc_t evlanAlloc = {0};

		evlanAlloc.nNumberOfEntries = VLAN_BLOCK_SIZE;
		if (GSW_ExtendedVlanAlloc(dev, &evlanAlloc)) {
			LOG_ERR("Extended VLAN alloc failed for CT port %u", hw_pnum);
			return -1;
		}

		ctp_ext_vlan_cfg[idx].alloc = 1;
		ctp_ext_vlan_cfg[idx].vlan_blockid = evlanAlloc.nExtendedVlanBlockId;

		/* Init EVLAN table block */
		for (size_t i = 0; i < evlanAlloc.nNumberOfEntries; i++) {
			GSW_EXTENDEDVLAN_config_t ingEvlanCfg;

			memset(&ingEvlanCfg, 0, sizeof(ingEvlanCfg));

			ingEvlanCfg.nExtendedVlanBlockId = evlanAlloc.nExtendedVlanBlockId;
			ingEvlanCfg.nEntryIndex = i;
			ingEvlanCfg.sFilter.sOuterVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;
			ingEvlanCfg.sFilter.sInnerVlan.eType = GSW_EXTENDEDVLAN_FILTER_TYPE_NO_TAG;

			if (GSW_ExtendedVlanSet(dev, &ingEvlanCfg)) {
				LOG_ERR("Set ing EVLAN index %d in block %d failed for CT port %u",
					ingEvlanCfg.nEntryIndex, ingEvlanCfg.nExtendedVlanBlockId, hw_pnum);
			}
		}
	}

	for (uint8_t bpid = 0; bpid < ARRAY_SIZE(bp_vlan_data); bpid++) {

		memset(&bp_vlan_data[bpid], 0, sizeof(struct bpid_vlan_data));
		if (!is_vlan_bpid(bpid))
			bp_vlan_data[bpid].ctp = bpid;
		else
			bp_vlan_data[bpid].ctp = INVALID_CTP;

		bp_vlan_data[bpid].vlan_data.tag_vlan.outer_tag.tag = NET_VLAN_TAG_UNSPEC;
		bp_vlan_data[bpid].vlan_data.tag_vlan.inner_tag.tag = NET_VLAN_TAG_UNSPEC;
		bp_vlan_data[bpid].vlan_data.tag_vlan.outer_tag.tpid = NET_ETH_PTYPE_VLAN;
		bp_vlan_data[bpid].vlan_data.tag_vlan.inner_tag.tpid = NET_ETH_PTYPE_VLAN;
	}

	return 0;
}

int tag_vlan_uninit(void)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	uint8_t tot_ports = get_chip_port_num();

	for (uint8_t pnum = 1; pnum <= tot_ports; pnum++) {
		uint8_t hw_pnum = map_port_idx(pnum);
		uint8_t idx = hw_pnum & 15;
		GSW_CTP_portConfig_t ctpcfg = {0};
		GSW_EXTENDEDVLAN_alloc_t evlanfree = {0};

		/* free ingress extended VLAN of ctp port */
		ctpcfg.nLogicalPortId = hw_pnum & 0x0F;
		ctpcfg.nSubIfIdGroup = hw_pnum & ~0x0F;
		ctpcfg.eMask = GSW_CTP_PORT_CONFIG_MASK_INGRESS_VLAN;
		ctpcfg.bIngressExtendedVlanEnable = GSW_FALSE;
		ctpcfg.nIngressExtendedVlanBlockId = ctp_ext_vlan_cfg[idx].vlan_blockid;
		if (GSW_CtpPortConfigSet(dev, &ctpcfg)) {
			LOG_ERR("Failed in Extended VLAN block %u removal from CTP %u",
				ctp_ext_vlan_cfg[idx].vlan_blockid, hw_pnum);
			return -EIO;
		}

		evlanfree.nExtendedVlanBlockId = ctp_ext_vlan_cfg[idx].vlan_blockid;
		if (GSW_ExtendedVlanFree(dev, &evlanfree)) {
			LOG_ERR("Freeing extended VLAN block %u assigned to ctp port %u failed",
				evlanfree.nExtendedVlanBlockId, idx);
			return -EIO;
		}
		memset(&ctp_ext_vlan_cfg[idx], 0, sizeof(struct ctp_ext_vlan_t));
	}

	for (uint8_t bpid = 0; bpid < ARRAY_SIZE(bp_vlan_data); bpid++) {
		memset(&bp_vlan_data[bpid], 0, sizeof(struct bpid_vlan_data));
		bp_vlan_data[bpid].ctp = INVALID_CTP;
	}

	return 0;
}
