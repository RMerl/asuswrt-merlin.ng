include ../common.mak

ifeq ($(RTCONFIG_QTN),y)
	QTN = _qtn
else
	QTN =
endif

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuild/$(BUILD_NAME)/*),)
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
	@cp include/tdts/sysdeps/ipq60xx-tmcfg.h include/tdts/tmcfg.h -f                     # ipq60xx
	@cp include/udb/sysdeps/ipq60xx-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq60xx
	@cp bin/ipq60xx/* bin/ -rf                                                           # ipq60xx
else ifeq ($(RTCONFIG_RALINK),y)
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
	@cp bin/mtk/* bin/ -rf                                                               # mtk
else ifeq ($(RTCONFIG_ALPINE),y)
	@cp include/tdts/sysdeps/alpine-tmcfg.h include/tdts/tmcfg.h -f                      # gt-ac9600
	@cp include/udb/sysdeps/alpine-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # gt-ac9600
	@cp bin/alpine/* bin/ -rf                                                            # gt-ac9600
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
ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp include/tdts/sysdeps/bcm6710-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6710
	@cp include/udb/sysdeps/bcm6710-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6710
	@cp bin/bcm6710/* bin/ -rf                                                           # bcm6710
else
	@cp include/tdts/sysdeps/bcm675x-tmcfg.h include/tdts/tmcfg.h -f                     # bcm675x
	@cp include/udb/sysdeps/bcm675x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm675x
	@cp bin/bcm675x/* bin/ -rf                                                           # bcm675x
endif # RTCONFIG_HND_ROUTER_AX_6710
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
else ifeq ($(RTCONFIG_QCN550X),y)
	@cp include/tdts/sysdeps/qcn550x-tmcfg.h include/tdts/tmcfg.h -f                     # qcn550x
	@cp include/udb/sysdeps/qcn550x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # qcn550x
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
	@cp include/tdts/sysdeps/ipq60xx-tmcfg.h include/tdts/tmcfg.h -f                     # ipq60xx
	@cp include/udb/sysdeps/ipq60xx-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # ipq60xx
else ifeq ($(RTCONFIG_RALINK),y)
	@cp include/tdts/sysdeps/mtk-tmcfg.h include/tdts/tmcfg.h -f                         # mtk
	@cp include/udb/sysdeps/mtk-tmcfg_udb.h include/udb/tmcfg_udb.h -f                   # mtk
else ifeq ($(RTCONFIG_ALPINE),y)
	@cp include/tdts/sysdeps/alpine-tmcfg.h include/tdts/tmcfg.h -f                      # gt-ac9600
	@cp include/udb/sysdeps/alpine-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # gt-ac9600
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp include/tdts/sysdeps/lantiq-tmcfg.h include/tdts/tmcfg.h -f                      # lantiq
	@cp include/udb/sysdeps/lantiq-tmcfg_udb.h include/udb/tmcfg_udb.h -f                # lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp include/tdts/sysdeps/bcm7-tmcfg.h include/tdts/tmcfg.h -f                        # bcm7
	@cp include/udb/sysdeps/bcm7-tmcfg_udb.h include/udb/tmcfg_udb.h -f                  # bcm7
else ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp include/tdts/sysdeps/bcm6710-tmcfg.h include/tdts/tmcfg.h -f                     # bcm6710
	@cp include/udb/sysdeps/bcm6710-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm6710
else
	@cp include/tdts/sysdeps/bcm675x-tmcfg.h include/tdts/tmcfg.h -f                     # bcm675x
	@cp include/udb/sysdeps/bcm675x-tmcfg_udb.h include/udb/tmcfg_udb.h -f               # bcm675x
endif # RTCONFIG_HND_ROUTER_AX_6710
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
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	@cp -f modules/dakota/* RC_INDEP/
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
	@cp -f modules/qca956x/* RC_INDEP/
	@cp -f bin/wrs/rule.trf RC_INDEP/ # wrs / lite version
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
	@cp -f modules/ipq60xx/* RC_INDEP/
else ifeq ($(RTCONFIG_RALINK),y)
	@cp -f modules/mtk/* RC_INDEP/
	@cp -f bin/partial/rule.trf RC_INDEP/
else ifeq ($(RTCONFIG_ALPINE),y)
	@cp -f modules/alpine/* RC_INDEP/
else ifeq ($(RTCONFIG_LANTIQ),y)
	@cp -f modules/lantiq/* RC_INDEP/
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	@cp -f modules/bcm7/* RC_INDEP/
else ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX_6710),y)
	@cp -f modules/bcm6710/* RC_INDEP/
else
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX56_XD4 CT-AX56_XD4 RT-AX55))
	@cp -f bin/wrs/rule.trf RC_INDEP/ # wrs / lite version
endif
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-AX58U TUF-AX3000 RT-AX82U RT-AX82_XD6)) # model flag
	@cp -f modules/bcm6750/* RC_INDEP/
else
	@cp -f modules/bcm675x/* RC_INDEP/
endif # model flag
endif # RTCONFIG_HND_ROUTER_AX_6710
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
	@cp -f bin/partial/rule.trf RC_INDEP/
endif
endif
endif
ifeq ($(or $(HND_ROUTER),$(HND_ROUTER_AX),$(RTCONFIG_HND_ROUTER_AX_675X),$(RTCONFIG_HND_ROUTER_AX_6710)),y)
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
ifneq ($(wildcard ~/amng.private/tm/$(BUILD_NAME)/*),)
	-@cp ~/amng.private/tm/$(BUILD_NAME)/* RC_INDEP/
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
endif

ifeq ($(wildcard $(TOP)/bwdpi_source/prebuild/$(BUILD_NAME)/*),)
clean:
	# asus function here
	@cd asus && $(MAKE) clean
	@cd asus_sql && $(MAKE) clean
else
clean:
endif
