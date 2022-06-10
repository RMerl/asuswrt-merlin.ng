include ../common.mak

ifeq ($(RTCONFIG_QTN),y)
	QTN = _qtn
else
	QTN =
endif

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuild/$(BUILD_NAME)/*),)

ifeq ($(RTCONFIG_LIBASUSLOG),y)
CFLAGS += -I$(TOP)/libasuslog
LIBS += -L$(TOP)/libasuslog -lasuslog
endif

CFLAGS += -I$(TOP)/json-c

# BCM6750 : seperate bin / header / modules from bcm675x
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX58U TUF-AX3000 TUF-AX5400 RT-AX82U RT-AX82_XD6 RT_AX82_XD6S GS-AX3000 GS-AX5400 DSL-AX82U))
export BWDPI_BCM6750 := y
endif

# BCM4912 : seperate bin / header / modules from bcm6756
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),ET12 XT12 GT-AX6000 GT-AX11000_PRO GT-AXE16000))
export BWDPI_BCM4912 := y
endif

# BCM6753 : seperate bin / header / modules from bcm6756
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),GT10))
export BWDPI_BCM6753 := y
endif

# MTK7621A + RTCONFIG_WLMODULE_MT7915D_AP
ifeq ($(RTCONFIG_WLMODULE_MT7915D_AP),y)
export BWDPI_MTK7621_NEW := y
endif

# signature version
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX56_XD4 CT-AX56_XD4 RT-AX55 XD4PRO))
export LITE_VERSION := y
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
export LITE_VERSION := y
else ifeq ($(or $(BWDPI_MTK7621_NEW)),y)
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),TUF-AX4200 TX-AX4200))
export FULL_VERSION := y
else
export LITE_VERSION := y
endif
else ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AC85U RT-AC65U RT-AC56S RT-N18U))
export PART_VERSION := y
else
export FULL_VERSION := y
endif

pre_build:
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cp include/tdts/sysdeps/ipq806x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq806x
	@cp include/udb/sysdeps/ipq806x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq806x
	@cp bin/ipq806x/* bin/ -rf                                                           # ipq806x
else ifeq ($(RTCONFIG_SOC_IPQ8074),y)
	@cp include/tdts/sysdeps/ipq807x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq807x
	@cp include/udb/sysdeps/ipq807x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq807x
	@cp bin/ipq807x/* bin/ -rf                                                           # ipq807x
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp include/tdts/sysdeps/dakota-tmcfg.h include/tdts/tmcfg.h -f                      # dakota
	@cp include/udb/sysdeps/dakota-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # dakota
	@cp bin/dakota/* bin/ -rf                                                            # dakota
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
	@cp include/tdts/sysdeps/qca956x-tmcfg.h include/tdts/tmcfg.h -f                     # qca956x / qnc550x
	@cp include/udb/sysdeps/qca956x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # qca956x / qnc550x
	@cp bin/qca956x/* bin/ -rf                                                           # qca956x / qnc550x
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
ifeq ($(RTCONFIG_QCA_PLC2),y)
	@cp include/tdts/sysdeps/ipq60xx-plc-tmcfg.h include/tdts/tmcfg.h -f                 # ipq60xx-plc
	@cp include/udb/sysdeps/ipq60xx-plc-tmcfg_udb.h include/udb/tmcfg_udb.h -f           # ipq60xx-plc
	@cp bin/ipq60xx-plc/* bin/ -rf                                                       # ipq60xx-plc
else
	@cp include/tdts/sysdeps/ipq60xx-tmcfg.h include/tdts/tmcfg.h -f                     # ipq60xx
	@cp include/udb/sysdeps/ipq60xx-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq60xx
	@cp bin/ipq60xx/* bin/ -rf                                                           # ipq60xx
endif
else ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(RTCONFIG_RALINK_MT7622),y)
	@cp include/tdts/sysdeps/mtk7622-tmcfg.h include/tdts/tmcfg.h -f                     # mtk7622
	@cp include/udb/sysdeps/mtk7622-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # mtk7622
	@cp bin/mtk7622/* bin/ -rf                                                           # mtk7622
else
ifeq ($(BWDPI_MTK7621_NEW),y)
	@cp include/tdts/sysdeps/mtk7621-tmcfg.h include/tdts/tmcfg.h -f                     # mtk7621 new
	@cp include/udb/sysdeps/mtk7621-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # mtk7621 new
	@cp bin/mtk7621/* bin/ -rf                                                           # mtk7621 new
else
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
	@cp bin/mtk/* bin/ -rf                                                               # mtk
endif # BWDPI_MTK7621_NEW
endif
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp include/tdts/sysdeps/lantiq-tmcfg.h include/tdts/tmcfg.h -f                      # lantiq
	@cp include/udb/sysdeps/lantiq-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # lantiq
	@cp bin/lantiq/* bin/ -rf                                                            # lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp include/tdts/sysdeps/bcm7-tmcfg.h include/tdts/tmcfg.h -f                        # bcm7
	@cp include/udb/sysdeps/bcm7-tmcfg_udb.h include/udb/tmcfg_udb.h -f                  # bcm7
	@cp bin/bcm7/* bin/ -rf                                                              # bcm7
else ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6756),y)
ifeq ($(BWDPI_BCM4912), y)
	@cp include/tdts/sysdeps/bcm4912-tmcfg.h include/tdts/tmcfg.h -f                     # bcm4912
	@cp include/udb/sysdeps/bcm4912-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm4912
	@cp bin/bcm4912/* bin/ -rf                                                           # bcm4912
else ifeq ($(BWDPI_BCM6753), y)
	@cp include/tdts/sysdeps/bcm6753-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6753
	@cp include/udb/sysdeps/bcm6753-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6753
	@cp bin/bcm6753/* bin/ -rf                                                           # bcm6753
else
	@cp include/tdts/sysdeps/bcm6756-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6756
	@cp include/udb/sysdeps/bcm6756-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6756
	@cp bin/bcm6756/* bin/ -rf                                                           # bcm6756
endif
else ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp include/tdts/sysdeps/bcm6710-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6710
	@cp include/udb/sysdeps/bcm6710-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6710
	@cp bin/bcm6710/* bin/ -rf                                                           # bcm6710
else ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(BWDPI_BCM6750), y)
	@cp include/tdts/sysdeps/bcm6750-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6750
	@cp include/udb/sysdeps/bcm6750-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6750
	@cp bin/bcm6750/* bin/ -rf                                                           # bcm6750
else
	@cp include/tdts/sysdeps/bcm675x-tmcfg.h include/tdts/tmcfg.h -f                     # bcm675x
	@cp include/udb/sysdeps/bcm675x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm675x
	@cp bin/bcm675x/* bin/ -rf                                                           # bcm675x
endif # BWDPI_BCM6750
else
	@cp include/tdts/sysdeps/axhnd-tmcfg.h include/tdts/tmcfg.h -f                       # ax-hnd
	@cp include/udb/sysdeps/axhnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                 # ax-hnd
	@cp bin/axhnd/* bin/ -rf                                                             # ax-hnd
endif # RTCONFIG_HND_ROUTER_AX_675X
else
	@cp include/tdts/sysdeps/hnd-tmcfg.h include/tdts/tmcfg.h -f                         # hnd
	@cp include/udb/sysdeps/hnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # hnd
	@cp bin/hnd/* bin/ -rf                                                               # hnd
endif # HND_ROUTER_AX
else ifeq ($(RTCONFIG_BCM_7114),y)
	@cp include/tdts/sysdeps/bcm7114-tmcfg.h include/tdts/tmcfg.h -f                     # bcm7114
	@cp include/udb/sysdeps/bcm7114-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm7114
	@cp bin/bcm7114/* bin/ -rf                                                           # bcm7114
else
ifeq ($(RTCONFIG_BCMSMP),y)
	@cp include/tdts/sysdeps/bcm6_smp-tmcfg.h include/tdts/tmcfg.h -f                    # bcm6_smp
	@cp include/udb/sysdeps/bcm6_smp-tmcfg_udb.h include/udb/tmcfg_udb.h -f              # bcm6_smp
	@cp bin/bcm6_smp/* bin/ -rf                                                          # bcm6_smp
else
	@cp include/tdts/sysdeps/bcm6_up-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6_up
	@cp include/udb/sysdeps/bcm6_up-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6_up
	@cp bin/bcm6_up/* bin/ -rf                                                           # bcm6_up
endif
endif
endif
else  # ($(wildcard $(TOP)/bwdpi_source/prebuild/*),)
pre_build:
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cp include/tdts/sysdeps/ipq806x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq806x
	@cp include/udb/sysdeps/ipq806x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq806x
else ifeq ($(RTCONFIG_SOC_IPQ8074),y)
	@cp include/tdts/sysdeps/ipq807x-tmcfg.h include/tdts/tmcfg.h -f                     # ipq807x
	@cp include/udb/sysdeps/ipq807x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq807x
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp include/tdts/sysdeps/dakota-tmcfg.h include/tdts/tmcfg.h -f                      # dakota
	@cp include/udb/sysdeps/dakota-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # dakota
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
	@cp include/tdts/sysdeps/qca956x-tmcfg.h include/tdts/tmcfg.h -f                     # qca956x / qnc550x
	@cp include/udb/sysdeps/qca956x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # qca956x / qnc550x
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
ifeq ($(RTCONFIG_QCA_PLC2),y)
	@cp include/tdts/sysdeps/ipq60xx-plc-tmcfg.h include/tdts/tmcfg.h -f                 # ipq60xx-plc
	@cp include/udb/sysdeps/ipq60xx-plc-tmcfg_udb.h include/udb/tmcfg_udb.h -f           # ipq60xx-plc
else
	@cp include/tdts/sysdeps/ipq60xx-tmcfg.h include/tdts/tmcfg.h -f                     # ipq60xx
	@cp include/udb/sysdeps/ipq60xx-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq60xx
endif
else ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(RTCONFIG_RALINK_MT7622),y)
	@cp include/tdts/sysdeps/mtk7622-tmcfg.h include/tdts/tmcfg.h -f                     # mtk7622
	@cp include/udb/sysdeps/mtk7622-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # mtk7622
else
ifeq ($(BWDPI_MTK7621_NEW),y)
	@cp include/tdts/sysdeps/mtk7621-tmcfg.h include/tdts/tmcfg.h -f                     # mtk7621 new
	@cp include/udb/sysdeps/mtk7621-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # mtk7621 new
else
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
endif # BWDPI_MTK7621_NEW
endif
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp include/tdts/sysdeps/lantiq-tmcfg.h include/tdts/tmcfg.h -f                      # lantiq
	@cp include/udb/sysdeps/lantiq-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp include/tdts/sysdeps/bcm7-tmcfg.h include/tdts/tmcfg.h -f                        # bcm7
	@cp include/udb/sysdeps/bcm7-tmcfg_udb.h include/udb/tmcfg_udb.h -f                  # bcm7
else ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6756),y)
ifeq ($(BWDPI_BCM4912), y)
	@cp include/tdts/sysdeps/bcm4912-tmcfg.h include/tdts/tmcfg.h -f                     # bcm4912
	@cp include/udb/sysdeps/bcm4912-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm4912
else ifeq ($(BWDPI_BCM6753), y)
	@cp include/tdts/sysdeps/bcm6753-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6753
	@cp include/udb/sysdeps/bcm6753-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6753
else
	@cp include/tdts/sysdeps/bcm6756-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6756
	@cp include/udb/sysdeps/bcm6756-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6756
endif
else ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp include/tdts/sysdeps/bcm6710-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6710
	@cp include/udb/sysdeps/bcm6710-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6710
else ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(BWDPI_BCM6750), y)
	@cp include/tdts/sysdeps/bcm6750-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6750
	@cp include/udb/sysdeps/bcm6750-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6750
else
	@cp include/tdts/sysdeps/bcm675x-tmcfg.h include/tdts/tmcfg.h -f                     # bcm675x
	@cp include/udb/sysdeps/bcm675x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm675x
endif # BWDPI_BCM6750
else
	@cp include/tdts/sysdeps/axhnd-tmcfg.h include/tdts/tmcfg.h -f                       # ax-hnd
	@cp include/udb/sysdeps/axhnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                 # ax-hnd
endif # RTCONFIG_HND_ROUTER_AX_675X
else
	@cp include/tdts/sysdeps/hnd-tmcfg.h include/tdts/tmcfg.h -f                         # hnd
	@cp include/udb/sysdeps/hnd-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # hnd
endif # HND_ROUTER_AX
else ifeq ($(RTCONFIG_BCM_7114),y)
	@cp include/tdts/sysdeps/bcm7114-tmcfg.h include/tdts/tmcfg.h -f                     # bcm7114
	@cp include/udb/sysdeps/bcm7114-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm7114
else
ifeq ($(RTCONFIG_BCMSMP),y)
	@cp include/tdts/sysdeps/bcm6_smp-tmcfg.h include/tdts/tmcfg.h -f                    # bcm6_smp
	@cp include/udb/sysdeps/bcm6_smp-tmcfg_udb.h include/udb/tmcfg_udb.h -f              # bcm6_smp
else
	@cp include/tdts/sysdeps/bcm6_up-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6_up
	@cp include/udb/sysdeps/bcm6_up-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6_up
endif
endif
endif
endif # ($(wildcard $(TOP)/bwdpi_source/prebuild/*),)

asus_build:
	@cd asus/ && $(MAKE)

asus_sql_build:
	@cd asus_sql/ && $(MAKE)

all: pre_build asus_build asus_sql_build

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuild/$(BUILD_NAME)/*),)
install: all
	# asus function here
	@cd asus/ && $(MAKE) install
	@cd asus_sql/ && $(MAKE) install
	# [INSTALL] PREPARE bwdpi_source/RC_INDEP/
	@rm -rf RC_INDEP
	@mkdir RC_INDEP
	@tar -xf bin/dcd.tar -C RC_INDEP/.
	@tar -xf bin/shn_ctrl.tar -C RC_INDEP/.
	@tar -xf bin/tdts_wred.tar -C RC_INDEP/.
	@cp -f bin/libshn_*.so RC_INDEP
	@cp -f bin/rule.trf RC_INDEP
	@cp -f bin/tdts_rule_agent RC_INDEP
	-@cp -f bin/tcd RC_INDEP
	-@cp -f bin/*enc RC_INDEP
	-@cp -f bin/*pem RC_INDEP
	@rm -f RC_INDEP/*.sh
	@rm -f RC_INDEP/*.conf
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	@cp -f modules/ipq806x/* RC_INDEP/
else ifeq ($(RTCONFIG_SOC_IPQ8074),y)
	@$(if $(RTCONFIG_SPF8_QSDK),cp -f modules/ipq807x/* RC_INDEP/)				# SPF8
	@$(if $(RTCONFIG_SPF11_QSDK),cp -f modules/ipq807x_spf11/* RC_INDEP/)			# SPF11.0
	@$(if $(RTCONFIG_SPF11_1_QSDK),cp -f modules/ipq807x_spf11.1/* RC_INDEP/)		# SPF11.1
	@$(if $(RTCONFIG_SPF11_4_QSDK),cp -f modules/ipq807x_spf11.4/* RC_INDEP/)		# SPF11.4
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp -f modules/dakota/* RC_INDEP/
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
	@cp -f modules/qca956x/* RC_INDEP/
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
ifeq ($(RTCONFIG_QCA_PLC2),y)
	@cp -f modules/ipq60xx-plc/* RC_INDEP/
else
	@cp -f modules/ipq60xx/* RC_INDEP/
endif
else ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(RTCONFIG_RALINK_MT7622),y)
	@cp -f modules/mtk7622/* RC_INDEP/
else
ifeq ($(BWDPI_MTK7621_NEW),y)
	@cp -f modules/mtk7621/* RC_INDEP/
else
	@cp -f modules/mtk/* RC_INDEP/
endif # BWDPI_MTK7621_NEW
endif
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp -f modules/lantiq/* RC_INDEP/
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp -f modules/bcm7/* RC_INDEP/
else ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6756),y)
ifeq ($(BWDPI_BCM4912), y)
	@cp -f modules/bcm4912/* RC_INDEP/
else ifeq ($(BWDPI_BCM6753), y)
	@cp -f modules/bcm6753/* RC_INDEP/
else
	@cp -f modules/bcm6756/* RC_INDEP/
endif
else ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp -f modules/bcm6710/* RC_INDEP/
else ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(BWDPI_BCM6750), y)
	@cp -f modules/bcm6750/* RC_INDEP/
else
	@cp -f modules/bcm675x/* RC_INDEP/
endif # BWDPI_BCM6750
else
	@cp -f modules/axhnd/* RC_INDEP/
endif # RTCONFIG_HND_ROUTER_AX_675X
else
	@cp -f modules/hnd/* RC_INDEP/
endif
else ifeq ($(RTCONFIG_BCM_7114),y)
	@cp -f modules/bcm7114/* RC_INDEP/
else
ifeq ($(RTCONFIG_BCMSMP),y)
	@cp -f modules/bcm6_smp/* RC_INDEP/
else
	@cp -f modules/bcm6_up/* RC_INDEP/
endif
endif
endif

# signature version
ifeq ($(LITE_VERSION),y)
	@cp -f bin/wrs/rule.trf RC_INDEP/
else ifeq ($(PART_VERSION),y)
	@cp -f bin/partial/rule.trf RC_INDEP/
else
	@cp -f bin/rule.trf RC_INDEP/
endif

ifeq ($(HND_ROUTER),y)
	@echo "start to install bwdpi_source into 94908HND"
	@install -d $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.so $(INSTALLDIR)/usr/lib/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.ko $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.trf $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.cert $(INSTALLDIR)/usr/bwdpi/
	-@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.enc $(INSTALLDIR)/usr/bwdpi/             # enc / pem / libshn_pctrl.so needs in the same folder
	-@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.pem $(INSTALLDIR)/usr/bwdpi/             # enc / pem / libshn_pctrl.so needs in the same folder
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred $(INSTALLDIR)/usr/sbin/wred
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_conf $(INSTALLDIR)/usr/sbin/wred_set_conf
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_wbl $(INSTALLDIR)/usr/sbin/wred_set_wbl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/dcd $(INSTALLDIR)/usr/sbin/dcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tcd $(INSTALLDIR)/usr/sbin/tcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/shn_ctrl $(INSTALLDIR)/usr/sbin/shn_ctrl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tdts_rule_agent $(INSTALLDIR)/usr/sbin/tdts_rule_agent
	@install -D $(TOP)/bwdpi_source/RC_INDEP/sample.bin $(INSTALLDIR)/usr/sbin/sample.bin
endif
else
install:
	# asus function here
	@cd asus/ && $(MAKE) install
	@cd asus_sql/ && $(MAKE) install
	# [INSTALL] prebuild into RC_INDEP
	-@mkdir RC_INDEP
	-@cp -f prebuild/$(BUILD_NAME)/* RC_INDEP/
ifeq ($(HND_ROUTER),y)
	@echo "start to install bwdpi_source into 94908HND"
	@install -d $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.so $(INSTALLDIR)/usr/lib/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.ko $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.trf $(INSTALLDIR)/usr/bwdpi/
	@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.cert $(INSTALLDIR)/usr/bwdpi/
	-@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.enc $(INSTALLDIR)/usr/bwdpi/             # enc / pem / libshn_pctrl.so needs in the same folder
	-@cp -f $(TOP)/bwdpi_source/RC_INDEP/*.pem $(INSTALLDIR)/usr/bwdpi/             # enc / pem / libshn_pctrl.so needs in the same folder
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred $(INSTALLDIR)/usr/sbin/wred
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_conf $(INSTALLDIR)/usr/sbin/wred_set_conf
	@install -D $(TOP)/bwdpi_source/RC_INDEP/wred_set_wbl $(INSTALLDIR)/usr/sbin/wred_set_wbl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/dcd $(INSTALLDIR)/usr/sbin/dcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tcd $(INSTALLDIR)/usr/sbin/tcd
	@install -D $(TOP)/bwdpi_source/RC_INDEP/shn_ctrl $(INSTALLDIR)/usr/sbin/shn_ctrl
	@install -D $(TOP)/bwdpi_source/RC_INDEP/tdts_rule_agent $(INSTALLDIR)/usr/sbin/tdts_rule_agent
	@install -D $(TOP)/bwdpi_source/RC_INDEP/sample.bin $(INSTALLDIR)/usr/sbin/sample.bin
endif
endif

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuild/$(BUILD_NAME)/*),)
clean:
	# asus function here
	@cd asus && $(MAKE) clean
	@cd asus_sql && $(MAKE) clean
else
clean:
endif
