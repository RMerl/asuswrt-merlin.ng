#ifndef RDP_DRV_ARCH_H
#define RDP_DRV_ARCH_H

#if defined(RDP_ARCH_SIM)
#define sim_ag_drv_cnpl_cnpl_stat_rsrv_thr_set ag_drv_cnpl_cnpl_stat_rsrv_thr_set
#elif defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_QEMU_SIM)
#define sim_ag_drv_cnpl_cnpl_stat_rsrv_thr_set(X, Y) 0
#else /* no RDP arch */
#error "no arch defined"
#endif /* RDP_ARCH_SIM */

#endif /* RDP_DRV_ARCH_H */
