#ifndef _GSW_BR_VLAN_H_
#define _GSW_BR_VLAN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mxl_br_vlan.h"

#if FID_VLAN_MAP_ENABLE
int get_fid_vlan(uint16_t vid);
int fid_vlan_cfg(uint8_t en, uint16_t vid);
int get_port_fid_tag_bp(uint8_t port, uint8_t fid);
int port_fid_tag_bp_cfg(uint8_t en, uint8_t port, uint8_t fid);
#endif

int get_port_based_vlan_cfg(uint8_t port, uint16_t *fid, uint8_t *discard_untag, uint8_t *discard_tag);
int set_port_based_vlan_cfg(uint8_t port, uint16_t fid, uint8_t discard_untag, uint8_t discard_tag);
int set_port_based_vlan_fid(uint8_t port, uint16_t fid);
int set_port_based_discard_cfg(uint8_t port, uint8_t discard_untag, uint8_t discard_tag);

int get_tag_based_vlan_cfg(uint8_t tag_port, uint8_t *en, uint8_t *vtype, uint8_t *port, uint8_t *fid,
		    uint16_t *outer_vlan, uint16_t *inner_vlan, uint8_t *outer_etype, uint8_t *inner_etype);
int reset_tag_based_vlan_cfg(uint8_t tag_port);
int set_tag_based_vlan_cfg(uint8_t tag_port, uint8_t vtype, uint8_t port, uint8_t fid,
		    uint16_t outer_vlan, uint16_t inner_vlan, uint8_t outer_etype, uint8_t inner_etype);
int set_tag_based_vlan_egress_priority(uint8_t tag_port, uint8_t outer_pri_type, uint8_t inner_pri_type,
		    uint8_t outer_pri, uint8_t inner_pri, uint8_t outer_dei, uint8_t inner_dei);
int set_port_fid_isolation(uint8_t port, uint8_t fid, uint8_t enable, uint8_t isolate_port);
void get_port_fid_isolation(uint8_t port, uint8_t fid, uint8_t *isolate_port);

int get_bpid_list(uint8_t fid);

#ifdef __cplusplus
}
#endif

#endif
