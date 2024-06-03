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
#ifdef CONFIG_BCM963138
void bcm_memc_self_refresh_pre_wfi(int cpu_mask);
void bcm_memc_self_refresh_post_wfi(int cpu_mask);
#define CONFIG_BCM_PWRMNGT_DDR_SR_API
// The structure below is to be declared in ADSL PHY MIPS LMEM, if ADSL is compiled in
typedef struct _PWRMNGT_DDR_SR_CTRL_ {
  union {
   struct {
      unsigned int   phyBusy:1;
      unsigned int   phy1Busy:1;
      unsigned int   reserved0:14;
      unsigned int   tp0Busy:1;
      unsigned int   tp1Busy:1;
      unsigned int   tp2Busy:1;
      unsigned int   tp3Busy:1;
      unsigned int   reserved1:12;
    };
   struct {
      unsigned int phy:16;
      unsigned int host:16;
    };
    unsigned int     word;
  };
} PWRMNGT_DDR_SR_CTRL;

void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr);
#else // #ifdef CONFIG_BCM963138
static inline void bcm_memc_self_refresh_pre_wfi(int cpu_mask attr_unused) {}
static inline void bcm_memc_self_refresh_post_wfi(int cpu_mask attr_unused) {}
#endif // #ifdef CONFIG_BCM963138
#else // #ifdef CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE
static inline int bcm_memc_init_self_refresh(void *pdev attr_unused) { return 0; }
#endif // #ifdef CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE

#ifdef __cplusplus
}
#endif
#endif
