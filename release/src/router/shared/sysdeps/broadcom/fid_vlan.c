
#include "br_vlan.h"
#include "mxl_br_vlan.h"
#include "hal_chipid.h"

#if FID_VLAN_MAP_ENABLE

#define TOT_FWD_BRIDGES 64
uint16_t fid_vlanid_map[TOT_FWD_BRIDGES] = {0};

struct __attribute__((packed)) fid_bp_data {
	uint8_t bp;
};
struct __attribute__((packed)) port_fid_data {
	struct fid_bp_data fid_bp[TOT_FWD_BRIDGES];
};

struct port_fid_data port_fid_bp[7] = {0}; //5+2

int get_fid_vlan(uint16_t vid)
{
	return vid2fid(vid); /* default is 0xff if not found */
}

int fid_vlan_cfg(uint8_t en, uint16_t vid)
{
	uint8_t fid;

	if (en)
	{
		for (fid = 1;
			fid < TOT_FWD_BRIDGES && fid_vlanid_map[fid] != 0;
			fid++);
		if (fid == TOT_FWD_BRIDGES) {
			LOG_ERR("Can't find free fid");
			return -1;
		}
		fid_vlanid_map[fid] = vid;
	}
	else
	{
		fid = vid2fid(vid);
		if (fid == 0xFF) {
			LOG_ERR("Can't find fid by vlan %u", vid);
			return -1;
		}
		fid_vlanid_map[fid] = 0;
	}

	return fid;
}

int get_port_fid_tag_bp(uint8_t port, uint8_t fid)
{
	return (port_fid_bp[port].fid_bp[fid].bp); /* default is 0 if not assigned */
}

int port_fid_tag_bp_cfg(uint8_t en, uint8_t port, uint8_t fid)
{
	uint8_t tag_bp, max_bp = TOT_TAG_BASED_BR_PORTS + 17;

	if (port == 0 || port > get_chip_port_num() || fid > 63) {
		LOG_ERR("Input data is out of range");
		return -1;
	}

	if (en)
	{
		/* search for free entry */
		for (tag_bp = 17; tag_bp < max_bp && 
			bpid_in_use(tag_bp) != false;
			tag_bp++);
		if (tag_bp == max_bp) {
			LOG_ERR("Can't find free bp entry for port %u fid %u", port, fid);
			return -1;
		}
		port_fid_bp[port].fid_bp[fid].bp = tag_bp;
	}
	else
	{
		tag_bp = port_fid_bp[port].fid_bp[fid].bp;
		port_fid_bp[port].fid_bp[fid].bp = 0;
	}

	return tag_bp;
}

#endif
