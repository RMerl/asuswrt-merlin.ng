#ifndef BCM_MEMC_H__
#define BCM_MEMC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef attr_unused
#define attr_unused	__attribute__((unused))
#endif

int bcm_memc_get_of_memcfg(unsigned int *memcfg);
int bcm_memc_get_spd_mhz(unsigned int *spd_mhz);

#ifdef CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE
int bcm_memc_init_self_refresh(void *pdev);
#if defined(CONFIG_BRCM_SMC_BOOT)
int enable_ddr_auto_self_refresh(unsigned int thre);
int disable_ddr_auto_self_refresh(void);
int ddr_auto_self_refresh_thres_get(void);
#endif
#else
static inline int bcm_memc_init_self_refresh(void *pdev attr_unused) { return 0; }
#if defined(CONFIG_BRCM_SMC_BOOT)
static inline int enable_ddr_auto_self_refresh(unsigned int thre) { return 0; }
static inline int disable_ddr_auto_self_refresh(void) { return 0;}
static inline int ddr_auto_self_refresh_thres_get(void) {return 0;}
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif

