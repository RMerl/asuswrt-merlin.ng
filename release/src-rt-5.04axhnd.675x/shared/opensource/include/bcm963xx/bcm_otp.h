
#if !defined(_BCM_OTP_H_)
#define _BCM_OTP_H_
#if !defined(CONFIG_BCM947189) && !defined(_BCM947189_)

/* OTP implementation defines */
#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908) || \
    defined(_BCM963148_) || defined(_BCM963138_) || defined(_BCM96858_) || defined(_BCM94908_)
#define OTP_TYPE_V1
#else
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96856) || \
    defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM96756) || \
    defined(_BCM963158_) || defined(_BCM963178_) || defined(_BCM947622_) || defined(_BCM96856_) || defined(_BCM96846_) || \
    defined(_BCM96878_) || defined(_BCM96855_) || defined(_BCM96756_) 
#define OTP_TYPE_V2
#else
#if  defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(_BCM963146_) || \
     defined(CONFIG_BCM96813)
#define OTP_TYPE_V3
#else	 
#error "OTP TYPE NOT SUPPORTED!!"
#endif
#endif
#endif

#ifdef OTP_TYPE_V1
#define OTP_JTAG_SER_NUM_ROW_1          19          // Row19[25:20] = CSEC_CHIPID[5:0]
#define OTP_JTAG_SER_NUM_MASK_1         0x0000003F
#define OTP_JTAG_SER_NUM_REG_SHIFT_1    20
#define OTP_JTAG_SER_NUM_ROW_2          20          // Row20[25:0]  = CSEC_CHIPID[31:6]
#define OTP_JTAG_SER_NUM_MASK_2         0x03FFFFFF
#define OTP_JTAG_SER_NUM_SHIFT_2        6
#define OTP_JTAG_PWD_ROW_1              21          // Row21[25:0]  = CSEC_PWD[25:0]   
#define OTP_JTAG_PWD_ROW_2              22          // Row22[25:0]  = CSEC_PWD[51:26]
#define OTP_JTAG_PWD_ROW_3              23          // Row23[11:0]  = CSEC_PWD[63:52]
#define OTP_JTAG_PWD_MASK_1             0x03FFFFFF
#define OTP_JTAG_PWD_MASK_2             0x03FFFFFF
#define OTP_JTAG_PWD_MASK_3             0x00000FFF
#define OTP_JTAG_PWD_SHIFT_1            0
#define OTP_JTAG_PWD_SHIFT_2            26
#define OTP_JTAG_PWD_SHIFT_3            52
#define OTP_JTAG_PWD_RDLOCK_ROW         23
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT   25          // Row23[25]
#define OTP_JTAG_MODE_NUM_BITS          6
#define OTP_JTAG_MODE_MAJORITY_BIT_CNT  2
#define OTP_JTAG_MODE_ROW               18
#define OTP_JTAG_MODE_REG_SHIFT         9         
#define OTP_JTAG_MODE_LOCK              0x38
#define OTP_JTAG_MODE_PERMALOCK         0x3F
#define OTP_JTAG_MODE_MASK              0x3F
#define OTP_JTAG_CUST_LOCK_ROW          6
#define OTP_JTAG_CUST_LOCK_VAL          0x1F
#define OTP_JTAG_CUST_LOCK_REG_SHIFT    25
#endif

#ifdef OTP_TYPE_V2
#define OTP_JTAG_SER_NUM_ROW_1          20          // Row20 = CSEC_CHIPID
#define OTP_JTAG_PWD_ROW_1              21          // Row21 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_ROW_2              22          // ROW22 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_MASK_1             0xFFFFFFFF
#define OTP_JTAG_PWD_MASK_2             0xFFFFFFFF
#define OTP_JTAG_PWD_SHIFT_1            0
#define OTP_JTAG_PWD_SHIFT_2            32
#define OTP_JTAG_PWD_RDLOCK_ROW         19          // ROW19[31]
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT   31          
#define OTP_JTAG_MODE_NUM_BITS          6
#define OTP_JTAG_MODE_MAJORITY_BIT_CNT  2
#define OTP_JTAG_MODE_ROW               18
#define OTP_JTAG_MODE_REG_SHIFT         9         
#define OTP_JTAG_MODE_LOCK              0x38
#define OTP_JTAG_MODE_PERMALOCK         0x3F
#define OTP_JTAG_MODE_MASK              0x3F
#define OTP_JTAG_CUST_LOCK_ROW          6
#define OTP_JTAG_CUST_LOCK_VAL          0x1F
#define OTP_JTAG_CUST_LOCK_REG_SHIFT    25
#endif

#ifdef OTP_TYPE_V3
#define OTP_JTAG_SER_NUM_ROW_1          26          // Row26 = CSEC_JTAGID
#define OTP_JTAG_PWD_ROW_1              27          // Row27 = CSEC_JTAGPWD[31:0]
#define OTP_JTAG_PWD_ROW_2              28          // ROW28 = CSEC_JTAGPWD[63:32]
#define OTP_JTAG_PWD_MASK_1             0xFFFFFFFF
#define OTP_JTAG_PWD_MASK_2             0xFFFFFFFF
#define OTP_JTAG_PWD_SHIFT_1            0
#define OTP_JTAG_PWD_SHIFT_2            32
#define OTP_JTAG_PWD_RDLOCK_ROW         14          // ROW14[25]
#define OTP_JTAG_PWD_RDLOCK_REG_SHIFT   25          
#define OTP_JTAG_MODE_NUM_BITS          2
#define OTP_JTAG_MODE_ROW               14
#define OTP_JTAG_MODE_REG_SHIFT         16         
#define OTP_JTAG_MODE_LOCK              0x02 
#define OTP_JTAG_MODE_PERMALOCK         0x03
#define OTP_JTAG_MODE_MASK              0x03
#define OTP_JTAG_CUST_LOCK_ROW          11	    // ROW11 = CFG_LOCK
#define OTP_JTAG_CUST_LOCK_VAL          0x1F 
#define OTP_JTAG_CUST_LOCK_REG_SHIFT    15
#endif


#endif /* !defined(CONFIG_BCM947189) && !defined(_BCM947189_) */
 
int bcm_is_btrm_boot(void);
int bcm_otp_is_boot_secure(void);
int bcm_otp_fld_secure_rows(void);

int bcm_otp_get_row(int row, unsigned int* val);
int bcm_otp_get_row_ecc(int row, unsigned int* val, unsigned int *val_hi);
int bcm_otp_fuse_row(int row, unsigned int val);
int bcm_otp_fuse_row_ecc(int row, unsigned int val, unsigned int ecc);

#if defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM94912_) || defined(CONFIG_BCM94912) || \
    defined(_BCM96756_) || defined(CONFIG_BCM96756) || defined(_BCM96813_) || defined(CONFIG_BCM96813)
int bcm_otp_is_pcie_port_disabled(unsigned int pcie_port_num, unsigned int* val);
#define BCM_OTP_IS_PCIE_PORT_DISABLED(a,b) bcm_otp_is_pcie_port_disabled(a, b)
#else
#define BCM_OTP_IS_PCIE_PORT_DISABLED(a,b) -1
#endif

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
#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96878_) && !defined(CONFIG_BCM96878) && !defined(_BCM96855_) && !defined(CONFIG_BCM96855)
int bcm_otp_is_sata_disabled(unsigned int* val);
#endif
#endif

#if defined(CONFIG_BCM94912) || defined(_BCM94912_) || defined(CONFIG_BCM96813) || defined(_BCM96813_)
int bcm_otp_get_dgasp_trim(unsigned int* val);
#endif

int bcm_otp_is_usb3_disabled(unsigned int* val);
#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
int bcm_otp_get_usb_port_disabled(int port, unsigned int* val);
#endif

#if defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM96756_)  || defined(CONFIG_BCM96756)
int bcm_otp_get_ldo_trim(unsigned int* val);
#endif

#if defined(_BCM96756_) || defined(CONFIG_BCM96756)
int bcm_otp_is_rescal_enabled(unsigned int* val);
#endif

#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
int bcm_otp_get_chipvar(unsigned int* val);
#endif

#endif  /* _BCM_OTP_H_ */


