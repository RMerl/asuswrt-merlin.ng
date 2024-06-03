
#if !defined(_BCM_OTP_H_)
#define _BCM_OTP_H_

int bcm_is_btrm_boot(void);
int bcm_otp_is_boot_secure(void);
int bcm_otp_fld_secure_rows(void);

int bcm_otp_get_row(int row, unsigned int* val);
int bcm_otp_get_row_ecc(int row, unsigned int* val, unsigned int *val_hi);
int bcm_otp_fuse_row(int row, unsigned int val);
int bcm_otp_fuse_row_ecc(int row, unsigned int val, unsigned int ecc);

int bcm_otp_is_pcie_port_disabled(unsigned int pcie_port_num, unsigned int* val);



#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
int bcm_otp_is_sgmii_disabled(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_)
int bcm_otp_get_chipid(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96878) || defined(_BCM96878_) || \
    defined(CONFIG_BCM96855) || defined(_BCM96855_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_) || \
    defined(CONFIG_BCM947622) || defined(_BCM947622_)
int bcm_otp_get_cpu_clk(unsigned int* val);
#endif

#if defined(CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM94908) || defined(_BCM94908_) || \
    defined(CONFIG_BCM963158) || defined(_BCM963158_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96878) || defined(_BCM96878_) || \
    defined(CONFIG_BCM96855) || defined(_BCM96855_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_) || \
    defined(CONFIG_BCM963178) || defined(_BCM963178_) || \
    defined(CONFIG_BCM947622) || defined(_BCM947622_) || \
    defined(CONFIG_BCM96756) || defined(_BCM96756_)

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
int bcm_otp_get_pmc_boot_sts(unsigned int* val);
#endif
int bcm_otp_get_nr_cpus(unsigned int* val);
int bcm_otp_is_boot_mfg_secure(void);
int bcm_otp_is_pcm_disabled(unsigned int* val);
#endif

int bcm_otp_is_sata_disabled(unsigned int* val);

int bcm_otp_get_dgasp_trim(unsigned int* val);

int bcm_otp_is_usb3_disabled(unsigned int* val);
#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
int bcm_otp_get_usb_port_disabled(int port, unsigned int* val);
#endif

#if defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM96756_)  || defined(CONFIG_BCM96756)
int bcm_otp_get_ldo_trim(unsigned int* val);
#endif

int bcm_otp_is_rescal_enabled(unsigned int* val);
#if defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM963146) || defined(CONFIG_BCM96765)
int bcm_otp_auth_prog_mode(void);
#endif

#if defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96765)
int bcm_otp_get_chipvar(unsigned int* val);
#endif

#endif  /* _BCM_OTP_H_ */


