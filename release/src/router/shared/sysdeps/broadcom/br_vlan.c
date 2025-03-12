
#include "br_vlan.h"
#include "mxl_br_vlan.h"

/*
int main(void) {
	int ret;

	ret = tag_vlan_uninit();
	ret |= tag_vlan_init();
	if (ret) {
		LOG_ERR("Failed to re-init tag vlan: %d", ret);
	}

    return 0;
}
*/

/*****************************************************************
 * Get port based VLAN
 * para: port, input, range [1, get_chip_port_num()]
 *       fid, output, range [0, 63]
 *       discard_untag, output, range [0, 1]
 *       discard_tag, output, range [0, 1]
 * return: 0: success; non-0: fail.
 *****************************************************************/
int get_port_based_vlan_cfg(uint8_t port, uint16_t *fid, uint8_t *discard_untag, uint8_t *discard_tag)
{
	uint8_t hw_pnum = 0;

	if (!fid || !discard_untag || !discard_tag)
		return -1;

	if (port == 0 || port > get_chip_port_num()) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	hw_pnum = map_port_idx(port);
	*fid = bpid2vid(hw_pnum);
	*discard_untag = get_discard_cfg(DISCARD_UNTAG, hw_pnum);
	*discard_tag = get_discard_cfg(DISCARD_TAG, hw_pnum);
	LOG_INF("BRP VLAN id %d for port %d, discard untag %d, discard tag %d", *fid, port, *discard_untag, *discard_tag);

	return 0;
}

/*****************************************************************
 * Set port based VLAN
 * para: port, input, range [1, get_chip_port_num()]
 *       fid, input, range [0, 63], default FID = 0
 *       discard_untag, input, range [0, 1]
 *       discard_tag, input, range [0, 1]
 * return: 0: success; non-0: fail.
 *****************************************************************/
int set_port_based_vlan_cfg(uint8_t port, uint16_t fid, uint8_t discard_untag, uint8_t discard_tag)
{
	uint8_t hw_pnum = 0;

	if (port == 0 || port > get_chip_port_num() || fid > 63 ||
		discard_untag > 1 || discard_tag > 1) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	hw_pnum = map_port_idx(port);
	bp_update(hw_pnum, fid);
	set_discard_cfg(DISCARD_TAG, hw_pnum, discard_tag);
	set_discard_cfg(DISCARD_UNTAG, hw_pnum, discard_untag);

#if 0 //Debug
	for (int loopi = 0; loopi < 8; loopi++) {
		for (int loopj = 0; loopj < 16; loopj++) {
			if (brp_fid_map[(loopi * 16) + loopj] == 0)
				continue;
			LOG_DBG("brp_fid_map[%d]=0x%x\n", (loopi * 16) + loopj, brp_fid_map[(loopi * 16) + loopj]);
		}
	}
#if FID_VLAN_MAP_ENABLE
	for (int loopi = 0; loopi < 8; loopi++) {
		for (int loopj = 0; loopj < 8; loopj++) {
			if (fid_vlanid_map[(loopi * 8) + loopj] == 0)
				continue;
			LOG_DBG("fid_vlanid_map[%d]=0x%x\n", (loopi * 8) + loopj, fid_vlanid_map[(loopi * 8) + loopj]);
		}
	}
#endif
#endif

	return 0;
}

/*****************************************************************
 * Get bridge port list
 * para: fid, input, range [0, 63]
 *       count, output, range [1, 126]
 *       bpmap[128], output, range [bp1, bp126]
 * return: 0: success; non-0: fail.
 *****************************************************************/
int get_bpid_list(uint8_t fid)
{
	uint8_t bpmap[128] = {0}, count = 0;

	if (fid > 63) {
		LOG_ERR("Input data is out of range");
		return -1;
	}
	get_bpmap_from_fid(fid, bpmap, &count);
	return 0;
}

/*****************************************************************
 * Get tag based VLAN
 * para: tag_port, input, range [0, TOT_TAG_BASED_BR_PORTS]
 *       en, output, range [0, 1]
 *       vtype, output, range [0, 1], 0=single tag, 1=double tag
 *       port, output, range [1, get_chip_port_num()]
 *       fid, output, range [0, 63]
 *       outer_vlan, output, range [1, 4095]
 *       inner_vlan, output, range [1, 4095]
 *       outer_etype, output, range [3], 0x8100
 *       inner_etype, output, range [3], 0x8100
 * return: 0: success; non-0: fail.
 *****************************************************************/
int get_tag_based_vlan_cfg(uint8_t tag_port, uint8_t *en, uint8_t *vtype, uint8_t *port, uint8_t *fid,
		       uint16_t *outer_vlan, uint16_t *inner_vlan, uint8_t *outer_etype, uint8_t *inner_etype)
{
	uint8_t phy_port = 0;
	int ret;

	if (!en || !vtype || !port || !fid || !outer_vlan ||
		!inner_vlan || !outer_etype || !inner_etype)
		return -1;

	if (tag_port > TOT_TAG_BASED_BR_PORTS) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	ret = get_egr_brp_ext_vlan_dets(tag_port+17, en, vtype,
						&phy_port, fid,
						NULL, outer_vlan,
						inner_vlan, NULL, outer_etype, inner_etype);
	if (ret) {
		LOG_ERR("API get_egr_brp_ext_vlan_dets failed for BR port %u",
			tag_port);
		return ret;
	}

	*port = map_drv_port_idx(phy_port);

	LOG_DBG("BRP %u En %u vtType %u port %u Bridge id %u oVLAN id %u iVLAN id %u oType %u iType %u",
		tag_port, *en, *vtype, *port, *fid,
		*outer_vlan, *inner_vlan, *outer_etype, *inner_etype);

	return ret;
}

/*****************************************************************
 * Reset tag based VLAN
 * para: tag_port, input, range [0, TOT_TAG_BASED_BR_PORTS]
 * return: 0: success; non-0: fail.
 *****************************************************************/
int reset_tag_based_vlan_cfg(uint8_t tag_port)
{
	int ret;

	if (tag_port > TOT_TAG_BASED_BR_PORTS) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	ret = rm_tag_vlan(tag_port+17);
	if (ret) {
		LOG_ERR("Failed in removing tag VLAN entry: %d",
			ret);
	}
	return ret;
}

/*****************************************************************
 * Set tag based VLAN
 * para: tag_port, input, range [0, TOT_TAG_BASED_BR_PORTS]
 *       en, input, range [0, 1]
 *       vtype, input, range [0, 1], 0=single tag, 1=double tag
 *       port, input, range [1, get_chip_port_num()]
 *       fid, input, range [0, 63]
 *       outer_vlan, input, range [1, 4095]
 *       inner_vlan, input, range [1, 4095]
 *       outer_etype, input, range [3], 0x8100
 *       inner_etype, input, range [3], 0x8100
 * return: 0: success; non-0: fail.
 *****************************************************************/
int set_tag_based_vlan_cfg(uint8_t tag_port, uint8_t vtype, uint8_t port, uint8_t fid,
		       uint16_t outer_vlan, uint16_t inner_vlan, uint8_t outer_etype, uint8_t inner_etype)
{
	uint8_t hw_pnum = 0;
	int ret;

	if (tag_port > TOT_TAG_BASED_BR_PORTS ||
		vtype > 1 || port == 0 || port > get_chip_port_num() ||
		fid > 63 || outer_vlan > 4095 || (vtype && inner_vlan > 4096)) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	hw_pnum = map_port_idx(port);
	ret = update_tag_vlan(tag_port+17, vtype, hw_pnum, fid, outer_vlan, inner_vlan, outer_etype, inner_etype);
	if (ret) {
		LOG_ERR("Failed in adding/updating tag VLAN entry: %d",
			ret);
	}

	return ret;
}
