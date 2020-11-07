#ifndef __EPON_BDMF_INTERFACE_H
#define __EPON_BDMF_INTERFACE_H

#include <bcm_epon_common.h> 
#include <PonConfigDb.h>
#include <rdpa_epon.h>

extern BOOL (*p_OntmMpcpAssignMcast) (rdpa_epon_assign_mcast_op op, LinkIndex unicast_link_id, 
    rdpa_epon_mcast_link_t *mcast_link, uint32_t *mlink_idx);

extern BOOL (*p_OntDirFecModeSet) (LinkIndex link, BOOL rx, BOOL tx);

extern BOOL (*p_OntDirFecModeGet) (LinkIndex link, BOOL *rx, BOOL *tx);
 
extern int (*p_OntDirNewLinkNumSet) (U8 links);

extern int (*p_OntDirRegLinkNumGet) (U8 *max_links, U8 *registered_links);

extern void (*p_OntDirLaserTxModeSet) (rdpa_epon_laser_tx_mode mode);

extern void (*p_OntDirLaserTxModeGet) (rdpa_epon_laser_tx_mode *mode);

extern void (*p_OntDirLaserRxPowerSet) (bdmf_boolean on);

extern void (*p_OntDirLaserRxPowerGet) (bdmf_boolean *enable);

extern void (*p_OntDirMpcpStateGet) (LinkIndex link, rdpa_epon_link_mpcp_state
    *state);

extern void (*p_OntDirHoldoverGet)(rdpa_epon_holdover_t *);

extern void (*p_OntDirHoldoverSet)(rdpa_epon_holdover_t *);

extern void (*p_OntDirEponReset)(void);

extern U16 (*p_PonLosCheckTimeGet)(void);

extern U16 (*p_GateLosCheckTimeGet)(void);

extern void (*p_PonLosCheckTimeSet)(uint16_t time);

extern void (*p_GateLosCheckTimeSet)(uint16_t time);

extern int (*p_PonMgrActToWanState)(LinkIndex link, BOOL enable);

extern void (*p_PonMgrUserTrafficGet)(LinkIndex link, BOOL* enable);

extern int (*p_PonMgrLinkFlushSet)(LinkIndex link, BOOL enable);

extern void (*p_OntDirBurstCapGet)(LinkIndex link,  U16 * bcap);

extern void (*p_OntDirBurstCapSet)(LinkIndex link, const U16 * bcap);
#endif //__EPON_BDMF_INTERFACE_H

