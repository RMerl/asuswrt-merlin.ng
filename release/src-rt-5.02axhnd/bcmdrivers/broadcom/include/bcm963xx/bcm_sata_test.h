#ifndef _BCM_SATA_TEST_H_ 
#define _BCM_SATA_TEST_H_

struct sata_test_params{
    /*pass to kernel*/
    uint16_t    test_mode;
    union{
        uint16_t    test_type;
        uint16_t    param_type;
    };
    uint32_t    param1;
    uint32_t    param2;
    uint32_t    param3;

    /* read from kernel */    
    uint16_t    ssc_enabled;
    uint16_t    sata_mode;
    uint32_t    tx_amp_mvolts;
    uint32_t    reserved[2];
};

enum SATA_TEST_MODE{
    SATA_TESTMODE_START=0,
    SATA_TESTMODE_CONFIGURATION,
    SATA_TESTMODE_PHY_TSG_MOI,
    SATA_TESTMODE_RX,
    SATA_TESTMODE_RW_PHY_REGS,
    SATA_TESTMODE_STOP,
    SATA_TESTMODE_MAX
};

enum SATA_PHY_TSG_MOI_TEST_TYPE{
    SATA_PHY_TSG_MOI_TEST_LFTP =1,
    SATA_PHY_TSG_MOI_TEST_LBP,
    SATA_PHY_TSG_MOI_TEST_HFTP,
    SATA_PHY_TSG_MOI_TEST_MFTP,
    SATA_PHY_TSG_MOI_TEST_PRBS,
    SATA_PHY_TSG_MOI_TEST_MAX
};

enum SATA_RX_TEST{
    SATA_RX_TEST_BIST_L_GEN1=1,
    SATA_RX_TEST_BIST_L_GEN2,
    SATA_RX_TEST_BIST_L_GEN3,
    SATA_RX_TEST_MAX
};

enum SATA_PHY_REG_MODE{
    SATA_PHY_REG_READ=1,
    SATA_PHY_REG_WRITE,
    SATA_PHY_REG_MAX
};

enum SATA_TEST_PARAM_CONFIG{
    SATA_TEST_CONFIG_SSC=1,
    SATA_TEST_CONFIG_SATAMODE,
    SATA_TEST_CONFIG_TXAMP,
    SATA_TEST_CONFIG_MAX
};

enum SATA_MODE{
    SATA_MODE_GEN1i=1,
    SATA_MODE_GEN1m,
    SATA_MODE_GEN2i,
    SATA_MODE_GEN2m,
    SATA_MODE_GEN3i,
    SATA_MODE_MAX
};


#endif/*_BCM_SATA_TEST_H_ */
