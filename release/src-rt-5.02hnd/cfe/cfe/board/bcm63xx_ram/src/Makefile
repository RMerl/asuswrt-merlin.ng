BSPOBJS += dev_bcm63xx_uart.o

BSPOBJS +=	bcmSpiRes.o         \
		bcmLegSpi.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_6858_),)
ifeq ($(strip $(NONETWORK)),0)
ifeq ($(strip $(BRCM_CHIP)),6838)
CFLAGS += -DMAC_UNIMAC
CFLAGS += -DPHY_6848_EGPHY -DPHY_EXT1
BSPOBJS +=					\
	bcm_misc_hw_init_impl3.o\
	unimac_drv_impl1.o		\
    mac_drv_unimac.o        \
	bus_drv_6838.o          \
	phy_drv_6848_egphy.o    \
	phy_bp_parsing_6838.o   \
	phy_drv_ext1.o          \
	mdio_drv_impl1.o
endif
ifeq ($(strip $(BRCM_CHIP)),6848)
CFLAGS += -DMAC_UNIMAC
CFLAGS += -DPHY_6848_EGPHY -DPHY_6848_EPHY -DPHY_6848_SGMII
BSPOBJS +=					\
	bcm_misc_hw_init_impl5.o\
	unimac_drv_impl1.o		\
    mac_drv_unimac.o        \
	bus_drv_6848.o          \
	phy_bp_parsing_6848.o   \
	phy_drv_6848_egphy.o    \
	phy_drv_6848_ephy.o     \
	phy_drv_6848_sgmii.o    \
	mdio_drv_impl2.o
endif
ifeq ($(strip $(BRCM_CHIP)),6858)
CFLAGS += -DMAC_LPORT
CFLAGS += -DPHY_6858_EGPHY -DPHY_EXT2 -DPHY_EXT3 -DPHY_LPORT_SERDES
BSPOBJS +=					\
    mac_drv_lport.o         \
	bus_drv_6858.o          \
	phy_bp_parsing_6858.o   \
    phy_drv_6858_egphy.o    \
    phy_drv_ext2.o          \
    BCM8486-A0-v00-00-12-TCM.o \
    phy_drv_ext3.o          \
    phy_drv_lport_serdes.o
endif
BSPOBJS +=					\
	bus_drv.o               \
	mac_drv.o               \
	phy_drv.o               \
	phy_drv_mii.o           \
	phy_drv_brcm.o          \
	phy_bp_parsing.o

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_6848_),)
BSPOBJS +=					\
	phys_common_drv.o       \
	clk_rst.o	            \
	bcm_otp_impl1.o		\
	pmc_drv.o               \
	rdp_bbh_arrays.o		\
	rdp_dma_arrays.o		\
	rdp_drv_ih.o			\
	rdp_drv_bpm.o			\
	rdp_drv_sbpm.o			\
	rdp_drv_bbh.o			\
	rdp_cpu_ring.o			\
	rdd_cpu.o				\
    rdd_init.o				\
    rdd_tm.o				\
    rdd_common.o			\
	dev_bcm63xx_eth_impl2.o \
	data_path_init.o	    \
    runner_fw_a.o			\
    runner_fw_b.o			\
    runner_fw_c.o			\
    runner_fw_d.o			\
    predict_runner_fw_a.o 	\
    predict_runner_fw_b.o 	\
    predict_runner_fw_c.o 	\
    predict_runner_fw_d.o 
endif		
endif		
endif

ifeq ($(strip $(NONETWORK)),0)
ifeq ($(strip $(BRCM_CHIP)),6858)

#LPORT
BSPOBJS +=              \
    dev_bcm63xx_eth_impl3.o	\
    BCM6858_A0_LPORT_AG.o \
    bcm6858_lport_led_ag.o \
    bcm6858_lport_mdio_ag.o \
    BCM6858_A0LPORT_MDIO_AG.o \
    bcm6858_lport_mib_ag.o  \
    BCM6858_A0LPORT_MIB_AG.o \
    bcm6858_mib_conf_ag.o \
    BCM6858_A0MIB_CONF_AG.o \
    bcm6858_lport_rgmii_ag.o \
    bcm6858_xlmac_conf_ag.o \
    BCM6858_A0LPORT_XLMAC_AG.o \
    BCM6858_A0XLMAC_CONF_AG.o \
    bcm6858_lport_xlmac_ag.o \
    bcm6858_lport_mab_ag.o \
    BCM6858_A0LPORT_MAB_AG.o \
    BCM6858_A0LPORT_INTR_AG.o \
    BCM6858_A0LPORT_CTRL_AG.o \
    BCM6858_A0LPORT_SRDS_AG.o \
    BCM6858_A0LPORT_LED_AG.o \
    BCM6858_A0LPORT_RGMII_AG.o \
    bcm6858_lport_srds_ag.o \
    bcm6858_lport_intr_ag.o \
    bcm6858_lport_ctrl_ag.o \
    lport_drv.o \
    lport_stats.o \
    lport_mdio.o \
    serdes_access.o \
    pmc_xrdp.o \
    pmc_lport.o

ifneq ($(strip $(CONFIG_MERLIN_SDK)),)
BSPOBJS += merlin_serdes_api.o\
    merlin_lport_drv.o
else
BSPOBJS += merlin_direct_access.o
endif
endif
endif

# meaning if ((BRCM_CHIP == 63138) || (BRCM_CHIP == 63148))
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_),)
BSPOBJS +=               \
	clk_rst.o		\
	pmc_drv.o               \
	bcm_pinmux_impl1.o      \
	bcm_misc_hw_init_impl1.o \
	bcm_gpio_impl1.o \
	bcm_otp_impl1.o \
	bcm_led_impl1.o

ifeq ($(strip $(NONETWORK)),0)
BSPOBJS +=				\
	unimac_drv_impl1.o		\
	rdp_bbh_arrays.o		\
	rdp_dma_arrays.o		\
	rdp_drv_ih.o			\
	rdp_drv_bpm.o			\
	rdp_drv_sbpm.o			\
	rdp_drv_bbh.o			\
	rdp_cpu_ring.o			\
	rdd_cpu.o			\
	rdd_init.o			\
	rdd_tm.o			\
	rdd_common.o			\
	dev_bcm63xx_eth_impl2.o		\
	data_path_init.o	\
	runner_fw_b.o			\
	runner_fw_c.o			\
	runner_fw_d.o			\
	predict_runner_fw_b.o		\
	predict_runner_fw_c.o		\
	predict_runner_fw_d.o		\
	pmc_switch.o			\
	pmc_rdp.o			\
	bcm_ethsw_impl1.o
endif
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_68360_),)
BSPOBJS +=               \
    bcm_pinmux_impl1.o       \
    bcm_gpio_impl1.o \
    bcm_led_impl1.o  \
    bcm_otp_impl1.o  \
    bcm_misc_hw_init_impl6.o
# FIXMET: add pmc driver when adjusted for 68360
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_6858_),)
BSPOBJS +=               \
	bcm_pinmux_impl1.o       \
	bcm_misc_hw_init_impl6.o \
	bcm_gpio_impl1.o \
	bcm_led_impl1.o \
	bcm_otp_impl1.o \
	pmc_drv.o
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_),)
BSPOBJS +=              \
    clk_rst.o
endif
ifeq ($(strip $(NONETWORK)),0)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_),)
BSPOBJS +=				\
	dev_bcm63xx_eth.o \
	bcm_ethsw_impl1.o \
	pmc_switch.o	
endif
endif
endif

ifeq ($(strip $(BRCM_CHIP)),63381)
BSPOBJS +=				\
	pmc_switch.o			\
	pmc_drv.o			\
	bcm_pinmux_impl1.o		\
	bcm_led_impl1.o                 \
	bcm_otp_impl1.o 		\
	bcm_gpio_impl1.o \
	bcm_misc_hw_init_impl2.o
endif

ifeq ($(strip $(BRCM_CHIP)),6848)
BSPOBJS +=					\
	bcm_pinmux_impl1.o		\
	bcm_led_impl1.o			\
	bcm_gpio_impl1.o
endif

ifeq ($(strip $(BRCM_CHIP)),63268)
BSPOBJS +=				\
	bcm_otp_impl1.o
endif

ifeq ($(strip $(BRCM_CHIP)),47189)
ifeq ($(strip $(NONETWORK)),0)
BSPOBJS += dev_bcm47189_eth.o
endif
endif

BSPOBJS +=              \
	dev_bcm63xx_flash.o \
	flash_api.o         \
	flash_common.o      \
	bcm63xx_devs.o      \
	bcm63xx_board.o     \
	bcm63xx_cmd.o       \
	bcm63xx_blparms.o   \
	boardparms.o        \
	bp_funcs.o          \
	boardparms_voice.o  \
	shared_utils.o      \
	bcm63xx_util.o      \
	bcm63xx_ldr_raw.o   \
	bcm63xx_ldr_elf.o   \
	bcm63xx_main.o      \
	bcm63xx_env_subr.o  \
	bcm63xx_httpd.o     \
	bcm63xx_nvram.o		\
	bcm63xx_net_icmp.o

BSPOBJS += \
	bcm_ubi.o
ifeq ($(strip $(BRCM_CHIP)),47189)
BSPOBJS += \
	bcm_misc_hw_init_impl7.o \
	bcm_gpio_impl3.o
endif

# the following chips DO NOT support HsSPI: 63680
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6368_),)
BSPOBJS +=               \
	bcmHsSpi.o        
endif

# the following chips DO NOT support robosw: 6838, 63138, 60333, 63148, 6848, 4908, 6858
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6838_63138_60333_63148_6848_4908_6858_47189_68360_),)
BSPOBJS +=			\
	bcm_ethsw_impl2.o	\
	dev_bcm63xx_eth.o
endif

ifeq ($(strip $(BRCM_CHIP)),60333)
BSPOBJS += dev_bcm63xx_eth.o \
  dev_bcm63xx_unimac_gphy.o \
  bcm_led_impl2.o \
  bcm_gpio_impl2.o
endif

ifeq ($(strip ${INC_CFI_FLASH_DRIVER}),1)
BSPOBJS +=               \
	cfiflash.o
endif

ifeq ($(strip ${INC_SPI_FLASH_DRIVER}),1)
ifeq ($(strip $(BRCM_CHIP)),47189)
BSPOBJS +=               \
	spiflash_impl2.o
else
BSPOBJS +=               \
	spiflash.o
endif
endif

ifeq ($(strip ${INC_SPI_PROG_NAND}),1)
BSPOBJS +=               \
	nandflash.o
endif

ifeq ($(strip ${INC_NAND_FLASH_DRIVER}),1)
BSPOBJS +=               \
	nandflash.o
ifeq ($(strip ${INC_SPI_NAND_DRIVER}),1)
BSPOBJS +=               \
	spinandflash.o
endif
endif

ifeq ($(strip ${CFG_WEB_SERVER}),1)
BSPOBJS +=               \
	ul.o				\
	ulinfo.o
endif

ifeq ($(strip ${CFG_LDR_SREC}),1)
BSPOBJS +=                  \
	cfe_ldr_srec.o      \
	cfe_loader.o
endif

ifeq ($(strip ${CFG_DT}),1)
BSPOBJS +=                  \
	bcm63xx_dtb.o
endif
