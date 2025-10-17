#ifndef _MXL_BR_VLAN_H_
#define _MXL_BR_VLAN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include "hal_chipid.h"

#define FID_VLAN_MAP_ENABLE 1
#define BP_ISOLATE_ENABLE	1

#define NET_VLAN_TAG_UNSPEC 0x0fff
#define NET_ETH_PTYPE_VLAN  0x8100

/* Max Bridge ID possible is 63 */
#define MAX_BRIDGE_ID           63

/* Max Bridge ID possible is 128 */
#define MAX_BRIDGE_PORT         128

/* 128 - 18 (17 + 1) (i.e Tot ports - Port based BR ports (16 (PHY ports) + 1 (CPU port)) + 1 Reserved BRP127) */
#define TOT_TAG_BASED_BR_PORTS	(MAX_BRIDGE_PORT - (MAC_16 + 2))
#define TAG_BP_PER_SETTING	((TOT_TAG_BASED_BR_PORTS + 2) / 3)

#define EG_VLAN_BLOCK_SIZE	2

/* Max size is 32 */
#define VLAN_BLOCK_SIZE		16

/* Reserved VLAN entry for discard options */
#define VLAN_ENTRY_RESERVED	3
#define VLAN_DISCARD_UNTAG	0
#define VLAN_DISCARD_TAG(x)	(VLAN_DISCARD_UNTAG + ((x) ? 2 : 1))

#define VLAN_ENTRY_MAP_SIZE	(VLAN_BLOCK_SIZE - VLAN_ENTRY_RESERVED)
#define VLAN_TAG(x)		(VLAN_ENTRY_RESERVED + (x))

#define ENABLE_PVID_HW		0

#define INVALID_CTP         63

#if defined(RTBE82M)
#define WAN_PORT 1
#define CPU_WAN_PORT 6
#define CPU_LAN_PORT 7
#elif defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO)
#define WAN_PORT 4
#define CPU_WAN_PORT 9
#define CPU_LAN_PORT 10
#elif defined(GT7)
#define WAN_PORT 0
#define CPU_WAN_PORT 0
#define CPU_LAN_PORT 7
#else
#error "Unsupported platform"
#endif

struct __attribute__((packed)) port_vlan_cfg {
	uint32_t en: 1;
	uint32_t fid: 6;
	uint32_t discard_untag: 1;
	uint32_t discard_tag: 1;
	uint32_t res: 7;
};

/* structure to keep CTP Ext VLAN config info
 * index 1~15 is physical port 1~15
 * index 0 is physical port 16
 */
struct __attribute__((packed)) ctp_ext_vlan_t {
	uint32_t alloc : 1;
	uint32_t assigned : 1;
	uint32_t discard_untag : 1;
	uint32_t discard_tag : 1;
	uint32_t vlan_blockid : 12;
	uint32_t entry_map :
	VLAN_ENTRY_MAP_SIZE;
};

struct __attribute__((packed)) tag_vlan_cfg {
	uint32_t en: 1;
	uint32_t dbl_tag: 1;
	uint32_t ctp: 5;
	uint32_t fid: 6;
	uint32_t outer_vid: 12;
	uint32_t inner_vid: 13;	/* = 4096 is wildcard
					 * > 4096 is invalid
					 */
	uint32_t outer_tpid: 2;
	uint32_t inner_tpid: 2;
	uint32_t res: 6;
};

typedef enum {
	VLAN_TYPE_SINGLE_TAG = 0,
	VLAN_TYPE_DOUBLE_TAG = 1,
	VLAN_TYPE_MAC_BASED = 2,
	VLAN_TYPE_PROTO_BASED = 3,
	VLAN_TYPE_MAX,
} vlan_type_t;

struct __attribute__((packed)) vlan_tag {
	union {
		uint16_t vlan_tci;
		struct {
			uint16_t tag : 12;
			uint16_t dei : 1;
			uint16_t pcp : 3;
		};
	};
	uint16_t tpid;
};

struct __attribute__((packed)) tag_vlan_data {
	struct vlan_tag outer_tag;
	struct vlan_tag inner_tag;
};

struct __attribute__((packed)) mac_vlan_data {
	uint8_t addr[6];
};

struct __attribute__((packed)) bpid_vlan_data {
	uint8_t type : 2;
	uint8_t ctp  : 6; // bpid2ctp
	union {
		struct tag_vlan_data tag_vlan;
		struct mac_vlan_data mac_vlan;
	} vlan_data;
};

extern uint8_t brp_fid_map[128];
extern struct bpid_vlan_data bp_vlan_data[128];

#if FID_VLAN_MAP_ENABLE
extern uint16_t fid_vlanid_map[MAX_BRIDGE_ID+1];
#endif
extern uint64_t tag_based_brp_bmap[2];
extern struct ctp_ext_vlan_t ctp_ext_vlan_cfg[16];
extern uint64_t fid_alloc;

enum DISCARD_TYPE {
	DISCARD_UNTAG,
	DISCARD_TAG
};

/* update Bridge Port
 * remove it from current Bridge
 * add to new Bridge (new_fid)
 */
int bp_update(uint32_t bpid, uint32_t new_fid);


/* disable Bridge Port */
static inline void bp_disable(uint32_t bpid)
{
	uint32_t bits = sizeof(tag_based_brp_bmap[0]) * 8;

	tag_based_brp_bmap[bpid / bits] &= ~BIT64(bpid % bits);

	/* default Bridge 0 (FID 0) for Bridge Port (BP) not in use */
	bp_update(bpid, 0);
}

/* enable Bridge Port and add to new Bridge (new_fid) */
static inline void bp_enable(uint32_t bpid, uint32_t new_fid)
{
	uint32_t bits = sizeof(tag_based_brp_bmap[0]) * 8;

	tag_based_brp_bmap[bpid / bits] |= BIT64(bpid % bits);

	bp_update(bpid, new_fid);
}

int get_discard_cfg(enum DISCARD_TYPE type, uint32_t ctp);
int set_discard_cfg(enum DISCARD_TYPE type, uint32_t ctp, bool enable);
int tag_vlan_init(void);
int tag_vlan_uninit(void);
void ctp_ext_vlan_print(void);

int get_egr_brp_ext_vlan_dets(uint8_t br_port, uint8_t *en,
			      uint8_t *vtag_type, uint8_t *phy_port,
			      uint8_t *bridge_id, uint16_t *ev_block_id,
			      uint16_t *ovlan_id, uint16_t *ivlan_id,
			      int8_t *ctp_vidx, uint8_t *outer_tpid,
			      uint8_t *inner_tpid);
int rm_tag_vlan(uint8_t bp);
int update_tag_vlan(uint8_t bp, uint8_t vtype, uint8_t ctp,
		    uint8_t fid, uint16_t outer, uint16_t inner, uint8_t outer_tpid, uint8_t inner_tpid);
int update_tag_vlan_egress_priority(uint8_t bp, uint8_t outer_pri_type, uint8_t inner_pri_type,
		    uint8_t outer_pri, uint8_t inner_pri, uint8_t outer_dei, uint8_t inner_dei);

#if FID_VLAN_MAP_ENABLE
/* return VLAN ID associated with Bridge ID (FID) */
static inline uint16_t fid2vid(uint8_t fid)
{
	assert(fid < ARRAY_SIZE(fid_vlanid_map));
	return fid_vlanid_map[fid];
}
#endif

#if BP_ISOLATE_ENABLE
void bp_isolate_update(uint8_t bpid, uint8_t enable, uint8_t isolate_bp);
int get_bp_isolate(uint8_t bpid);
#endif

/* return 0xFF if not found, otherwise, return Bridge ID (FID) of VLAN ID */
static inline uint8_t vid2fid(uint16_t vid)
{
#if FID_VLAN_MAP_ENABLE
	uint8_t i;

	for (i = 0;
	     i < ARRAY_SIZE(fid_vlanid_map) && fid_vlanid_map[i] != vid;
	     i++);

	/* default is 0 if not found */
	return i < ARRAY_SIZE(fid_vlanid_map) ? i : 0xFF;
#else
	return vid;
#endif
}

/* return Bridge ID (FID) which includes this Bridge Port (BP) */
static inline uint8_t bpid2fid(uint8_t bpid)
{
	assert(bpid < ARRAY_SIZE(brp_fid_map));
	return brp_fid_map[bpid];
}

/* return VLAN ID which includes this Bridge Port (BP) */
static inline uint16_t bpid2vid(uint8_t bpid)
{
#if FID_VLAN_MAP_ENABLE
	return fid2vid(bpid2fid(bpid));
#else
	return bpid2fid(bpid);
#endif
}

/* return CTP destination logical portid which includes this Bridge Port (BP) */
static inline uint8_t bpid2ctp(uint8_t bpid)
{
	assert(bpid < ARRAY_SIZE(bp_vlan_data));
	return bp_vlan_data[bpid].ctp;
}

/* return true if Bridge Port (BP) is in use */
static inline bool bpid_in_use(uint8_t bpid)
{
	uint32_t bits = sizeof(tag_based_brp_bmap[0]) * 8;

	return (tag_based_brp_bmap[bpid / bits] & BIT64(bpid % bits)) ? true : false;
}

/* Reserve Bridge Port (BP)
 * set IN_USE flag
 */
static inline void bpid_reserve(uint8_t bpid)
{
	uint32_t bits = sizeof(tag_based_brp_bmap[0]) * 8;

	tag_based_brp_bmap[bpid / bits] |= BIT64(bpid % bits);
}

/* Release Bridge Port (BP)
 * clear IN_USE flag
 */
static inline void bpid_release(uint8_t bpid)
{
	uint32_t bits = sizeof(tag_based_brp_bmap[0]) * 8;

	tag_based_brp_bmap[bpid / bits] &= ~BIT64(bpid % bits);
}

static inline bool is_fid_allocated(uint8_t fid)
{
	if ((fid_alloc & BIT64(fid)) == 0)
		return false;

	return true;
}

/* Get Bridge Port map for the given FID (bridge)
 * bpids - array of bridge ports
 * count - number of valid bridge ports available in the array
 */
static inline void get_bpmap_from_fid(uint8_t fid, uint8_t *bpids, uint8_t *count)
{
	uint8_t idx = 0;

	if (!is_fid_allocated(fid)) {
		*count = 0;
		return;
	}

	for (uint8_t bpid = 0; bpid < ARRAY_SIZE(brp_fid_map); bpid++) {

		if (!bpid_in_use(bpid))
			continue;

		if (brp_fid_map[bpid] == fid) {
			bpids[idx++] = bpid;
		}
	}

	*count = idx;
}

/* Get bpid from tag
 * outer_tag   - outer vlan tag, outer tag is a must
 * inner_tag   - inner vlan tag (set to NET_VLAN_TAG_UNSPEC if not applicable)
 * portid - Received interface where this packet is received
 * Return bpid
 */
static inline uint8_t get_bpid_from_tag(uint8_t portid, uint16_t outer_tag, uint16_t inner_tag)
{
	if (outer_tag == NET_VLAN_TAG_UNSPEC)
		return 0;

	for (uint8_t bpid = 0; bpid < ARRAY_SIZE(bp_vlan_data); bpid++)  {
		if (bp_vlan_data[bpid].vlan_data.tag_vlan.outer_tag.tag == outer_tag) {
			if (bpid2ctp(bpid) == portid) {
				if (inner_tag == NET_VLAN_TAG_UNSPEC ||
				    bp_vlan_data[bpid].vlan_data.tag_vlan.inner_tag.tag == inner_tag) {
					return bpid;
				}
			}
		}
	}

	return 0;
}

static inline uint8_t get_tag_from_bpid(uint8_t bpid, uint16_t *outer_tag, uint16_t *inner_tag)
{
	*outer_tag = bp_vlan_data[bpid].vlan_data.tag_vlan.outer_tag.tag;
	*inner_tag = bp_vlan_data[bpid].vlan_data.tag_vlan.inner_tag.tag;

	return 0;
}

static inline struct bpid_vlan_data *get_vlan_data_from_bpid(uint16_t bpid)
{
	assert(bpid < ARRAY_SIZE(bp_vlan_data));
	return &bp_vlan_data[bpid];
}

/* return TRUE - if valid TPID
 * else FALSE
 */
static inline bool is_valid_tpid(uint16_t tpid)
{
	static const uint16_t mxl_tpid[] = {
		NET_ETH_PTYPE_VLAN, 0x88A8, 0x9100, 0x9200
	};

	for (uint8_t i = 0; i < ARRAY_SIZE(mxl_tpid); i++) {
		if (tpid == mxl_tpid[i]) {
			return true;
		}
	}
	return false;
}

/* return TRUE - if vlan BPID
 * else FALSE
 */
static inline bool is_vlan_bpid(uint16_t bpid)
{
	if (bpid > MAC_16 && bpid < (MAX_BRIDGE_PORT - 1))
		return true;
	return false;
}

static inline bool is_tag_vlan(uint16_t bpid)
{
	assert(bpid < ARRAY_SIZE(bp_vlan_data));

	if ((bp_vlan_data[bpid].type == VLAN_TYPE_SINGLE_TAG) ||
	    (bp_vlan_data[bpid].type == VLAN_TYPE_DOUBLE_TAG))
		return true;
	return false;
}

static inline bool is_valid_fid(uint8_t fid)
{
	if (fid >= MAX_BRIDGE_ID)
		return false;

	return true;
}

static inline bool is_valid_bpid(uint16_t bpid)
{
	if (bpid >= MAX_BRIDGE_PORT)
		return false;

	return true;
}

#ifdef __cplusplus
}
#endif

#endif
