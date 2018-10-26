
#if !defined(_BCM_OTP_H_)
#define _BCM_OTP_H_

int bcm_otp_get_row(int row, unsigned int* val);
int bcm_otp_is_btrm_boot(void);
int bcm_otp_is_boot_secure(void);
#if defined (CONFIG_BCM96848) || defined(_BCM96848_)
int bcm_otp_get_revId(void);
int bcm_otp_get_max_clksel(void);
#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
int bcm_otp_is_sgmii_disabled(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
int bcm_otp_get_chipid(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96836) || defined(_BCM96836_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_)
int bcm_otp_get_cpu_clk(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM94908) || defined(_BCM94908_) || \
    defined(CONFIG_BCM96836) || defined(_BCM96836_) || \
    defined(CONFIG_BCM963158) || defined(_BCM963158_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_)

int bcm_otp_get_nr_cpus(unsigned int* val);
int bcm_otp_get_pmc_boot_sts(unsigned int* val);
int bcm_otp_is_boot_mfg_secure(void);
int bcm_otp_is_pcm_disabled(unsigned int* val);
#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846)
int bcm_otp_is_sata_disabled(unsigned int* val);
#endif
#endif

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
int bcm_otp_get_usb_port_disabled(int port, unsigned int* val);
#endif

#endif  /* _BCM_OTP_H_ */


