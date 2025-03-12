#include <assert.h>
#include <string.h>

#include "mxl_br_vlan.h"
#include <gsw_device.h>
#include <gsw.h>
#include <gsw_api.h>

/* Bridge Port (BP) to Bridge (FID) mapping.
 * Bridge 0 (FID 0) is default bridge for all Bridge Ports (BPs).
 * Maximum 128 BPs with following allocation:
 *      0   - fixed allocation, WSP CPU port
 *    1~8   - fixed allocation, GMAC ports
 *    9~16  - fixed allocation, XGMAC ports
 *   17~127 - dynamic allocation, BP created for tag based VLAN interface
 */
uint8_t brp_fid_map[128] = {0};
uint64_t fid_alloc = 1UL;

void bp_update(uint32_t bpid, uint32_t new_fid)
{
	const GSW_Device_t *dev = gsw_get_struc(0, 0);
	GSW_BRIDGE_portConfig_t bpcfg;
	uint32_t old_fid;
	uint16_t portmap[8] = {0}, idx, set, clr;
	uint16_t use_cnt;

	assert(bpid < ARRAY_SIZE(brp_fid_map));
#if FID_VLAN_MAP_ENABLE
	assert(new_fid < ARRAY_SIZE(fid_vlanid_map));
#else
	assert(new_fid < 64);
#endif

	old_fid = bpid2fid(bpid);

	/* if FID not change and it was allocated
	 * no change is required
	 */
	if (new_fid && new_fid == old_fid &&
	    (fid_alloc & BIT64(new_fid)))
		return;

	LOG_DBG("bp_update: bpid=%d new_fid=%d\n", bpid, new_fid);

	brp_fid_map[bpid] = new_fid;

	/* allocate bridge if it's not allocated */
	if ((fid_alloc & BIT64(new_fid)) == 0) {
		GSW_BRIDGE_alloc_t cfg = {.nBridgeId = new_fid};

		GSW_BridgeAlloc(dev, &cfg);
		fid_alloc |= BIT64(new_fid);
	}

	idx = bpid / 16;
	if (bpid_in_use(bpid)) {
		/* if BP in use, setup set BIT and clear MASK
		 * even old_fid == new_fid, the BIT will be set in portmap
		 */
		set = BIT(bpid % 16);
		clr = ~set;
	} else {
		/* if BP not in use, setup clear MASK and leave set to be 0
		 * the BIT is always cleared
		 */
		set = 0;
		clr = ~BIT(bpid % 16);
	}

	use_cnt = ARRAY_SIZE(brp_fid_map);
	for (size_t i = 0; i < ARRAY_SIZE(brp_fid_map); i++) {
		uint16_t cur_val, new_val;

		/* this bridge port is not in use */
		if (!bpid_in_use(i)) {
			/* old_fid not used by this bridge port */
			use_cnt--;
			continue;
		}

		/* this bridge port is updated after all ports scanned */
		if (i == bpid) {
			/* old_fid will be replaced by new_fid on this
			 * bridge port
			 */
			if (old_fid != new_fid)
				use_cnt--;
			continue;
		}

		/* old_fid not used by this bridge port */
		if (brp_fid_map[i] != old_fid)
			use_cnt--;

		/* skip if this bridge port is neither in old bridge nor
		 * in new bridge
		 */
		if (brp_fid_map[i] != old_fid &&
		    brp_fid_map[i] != new_fid)
			continue;

		memset(&bpcfg, 0, sizeof(bpcfg));
		bpcfg.nBridgePortId = i;
		bpcfg.eMask = GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_PORT_MAP;
		if (GSW_BridgePortConfigGet(dev, &bpcfg)) {
			LOG_ERR("Get BP config failed for port %lu", i);
		}
		new_val = cur_val = bpcfg.nBridgePortMap[idx];
		if (brp_fid_map[i] == old_fid) {
			/* remove from target bridge port from port map
			 * if this bridge port
			 */
			new_val &= clr;
		}
		if (brp_fid_map[i] == new_fid) {
			/* add this bridge port to port map of target
			 * bridge port
			 */
			new_val |= set;
			portmap[i / 16] |= BIT(i % 16);
		}

		/* skip if no change */
		if (cur_val == new_val)
			continue;

		bpcfg.nBridgePortMap[idx] = new_val;

		// TODO: Temporary work around to add Port 0 to Bridge Portmap
		// Final Solution need to use PCE rule to send Control packet to CPU
		if (is_vlan_bpid(bpcfg.nBridgePortId)) {
			if (!bpcfg.nBridgePortMap[0] & 0x1)
				bpcfg.nBridgePortMap[0] |= 0x1;
		}

		LOG_DBG("BridgePort_ConfigSet 1\n");
		LOG_DBG("bpcfg.nBridgePortId=%d \n", bpcfg.nBridgePortId);
		LOG_DBG("bpcfg.eMask=0x%x \n", bpcfg.eMask);
		LOG_DBG("bpcfg.nBridgeId=%d \n", bpcfg.nBridgeId);
		LOG_DBG("bpcfg.nBridgePortMap[0]=0x%x \n", bpcfg.nBridgePortMap[0]);
		LOG_DBG("bpcfg.nBridgePortMap[1]=0x%x \n", bpcfg.nBridgePortMap[1]);

		if (GSW_BridgePortConfigSet(dev, &bpcfg)) {
			LOG_ERR("Set BP config failed for port %lu", i);
		}
	}

	memset(&bpcfg, 0, sizeof(bpcfg));
	bpcfg.nBridgePortId = bpid;
	bpcfg.eMask = GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_PORT_MAP | GSW_BRIDGE_PORT_CONFIG_MASK_BRIDGE_ID;
	bpcfg.nBridgeId = new_fid;
	if (bpid_in_use(bpid)) {
		/* set portmap if port in use
		 * otherwise clear portmap
		 */
		memcpy(bpcfg.nBridgePortMap, portmap, sizeof(portmap));
	}

	LOG_DBG("BridgePort_ConfigSet 2\n");
	LOG_DBG("bpcfg.nBridgePortId=%d \n", bpcfg.nBridgePortId);
	LOG_DBG("bpcfg.eMask=0x%x \n", bpcfg.eMask);
	LOG_DBG("bpcfg.nBridgeId=%d \n", bpcfg.nBridgeId);
	LOG_DBG("bpcfg.nBridgePortMap[0]=0x%x \n", bpcfg.nBridgePortMap[0]);
	LOG_DBG("bpcfg.nBridgePortMap[1]=0x%x \n", bpcfg.nBridgePortMap[1]);

	if (GSW_BridgePortConfigSet(dev, &bpcfg)) {
		LOG_ERR("Set BP config failed for port %u", bpid);
	}

	/* free bridge if it's not in use anymore */
	if (use_cnt == 0 && (fid_alloc & BIT64(old_fid))) {
		GSW_BRIDGE_alloc_t cfg = {.nBridgeId = old_fid};

		GSW_BridgeFree(dev, &cfg);
		fid_alloc &= ~BIT64(old_fid);
	}
}
