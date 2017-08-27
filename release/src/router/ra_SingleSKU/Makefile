
include $(BUILD_NAME)/config

SKUDIR = $(INSTALLDIR)/ra_SKU

SKU_FILES  = $(BUILD_NAME)/$(SKU_2G_FCC) $(BUILD_NAME)/$(SKU_2G_CE) $(BUILD_NAME)/$(SKU_5G_FCC) $(BUILD_NAME)/$(SKU_5G_CE)


# usage: $(call install_file, src_file, dest_file)
define install_file
	@( \
	if [ -e $(1) ]; then \
		install $(1) $(2) ; \
	else \
		echo "File ( $(1) ) not exist" ; exit 1; \
	fi; \
	)
endef



all clean:
	@if [ ! -d $(BUILD_NAME) ]; then \
		echo "No SingleSKU for $(BUILD_NAME)"; \
		exit 1; \
	fi

install: all
	-rm -rf $(TARGETDIR)/ra_SKU
	-rm -rf $(SKUDIR)
	@install -d $(SKUDIR)
ifeq ($(RA_SKU_IN_DRV),y)
ifneq ($(SKU_2G_FCC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_FCC.dat
endif
ifneq ($(SKU_5G_FCC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_FCC.dat
endif
ifneq ($(SKU_2G_IC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_IC.dat
endif
ifneq ($(SKU_5G_IC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_IC.dat
endif
ifneq ($(SKU_2G_CE),)
	@install DUMMY_SKU  $(SKUDIR)/SingleSKU_CE.dat
endif
ifneq ($(SKU_5G_CE),)
	@install DUMMY_SKU  $(SKUDIR)/SingleSKU_5G_CE.dat
endif
ifneq ($(SKU_2G_SG),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_SG.dat
endif
ifneq ($(SKU_5G_SG),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_SG.dat
endif
ifneq ($(SKU_2G_AU),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_AU.dat
endif
ifneq ($(SKU_5G_AU),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_AU.dat
endif
ifneq ($(SKU_2G_NCC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_NCC.dat
endif
ifneq ($(SKU_5G_NCC),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_NCC.dat
endif
ifneq ($(SKU_2G_JP),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_JP.dat
endif
ifneq ($(SKU_5G_JP),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_JP.dat
endif
ifneq ($(SKU_2G_CN),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_CN.dat
endif
ifneq ($(SKU_5G_CN),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_CN.dat
endif
ifneq ($(SKU_2G_AP),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_AP.dat
endif
ifneq ($(SKU_5G_AP),)
	@install DUMMY_SKU $(SKUDIR)/SingleSKU_5G_AP.dat
endif

else	#RA_SKU==y

ifneq ($(SKU_2G_FCC),)
	@install $(BUILD_NAME)/$(SKU_2G_FCC) $(SKUDIR)/SingleSKU_FCC.dat
endif
ifneq ($(SKU_5G_FCC),)
	@install $(BUILD_NAME)/$(SKU_5G_FCC) $(SKUDIR)/SingleSKU_5G_FCC.dat
endif
ifneq ($(SKU_2G_IC),)
	@install $(BUILD_NAME)/$(SKU_2G_IC) $(SKUDIR)/SingleSKU_IC.dat
endif
ifneq ($(SKU_5G_IC),)
	@install $(BUILD_NAME)/$(SKU_5G_IC) $(SKUDIR)/SingleSKU_5G_IC.dat
endif
ifneq ($(SKU_2G_CE),)
	@install $(BUILD_NAME)/$(SKU_2G_CE)  $(SKUDIR)/SingleSKU_CE.dat
endif
ifneq ($(SKU_5G_CE),)
	@install $(BUILD_NAME)/$(SKU_5G_CE)  $(SKUDIR)/SingleSKU_5G_CE.dat
endif
ifneq ($(SKU_2G_SG),)
	@install $(BUILD_NAME)/$(SKU_2G_SG) $(SKUDIR)/SingleSKU_SG.dat
endif
ifneq ($(SKU_5G_SG),)
	@install $(BUILD_NAME)/$(SKU_5G_SG) $(SKUDIR)/SingleSKU_5G_SG.dat
endif
ifneq ($(SKU_2G_AU),)
	@install $(BUILD_NAME)/$(SKU_2G_AU) $(SKUDIR)/SingleSKU_AU.dat
endif
ifneq ($(SKU_5G_AU),)
	@install $(BUILD_NAME)/$(SKU_5G_AU) $(SKUDIR)/SingleSKU_5G_AU.dat
endif
ifneq ($(SKU_2G_NCC),)
	@install $(BUILD_NAME)/$(SKU_2G_NCC) $(SKUDIR)/SingleSKU_NCC.dat
endif
ifneq ($(SKU_5G_NCC),)
	@install $(BUILD_NAME)/$(SKU_5G_NCC) $(SKUDIR)/SingleSKU_5G_NCC.dat
endif
ifneq ($(SKU_2G_NCC2),)
	@install $(BUILD_NAME)/$(SKU_2G_NCC2) $(SKUDIR)/SingleSKU_NCC2.dat
endif
ifneq ($(SKU_5G_NCC2),)
	@install $(BUILD_NAME)/$(SKU_5G_NCC2) $(SKUDIR)/SingleSKU_5G_NCC2.dat
endif
ifneq ($(SKU_2G_JP),)
	@install $(BUILD_NAME)/$(SKU_2G_JP) $(SKUDIR)/SingleSKU_JP.dat
endif
ifneq ($(SKU_5G_JP),)
	@install $(BUILD_NAME)/$(SKU_5G_JP) $(SKUDIR)/SingleSKU_5G_JP.dat
endif
ifneq ($(SKU_2G_CN),)
	@install $(BUILD_NAME)/$(SKU_2G_CN) $(SKUDIR)/SingleSKU_CN.dat
endif
ifneq ($(SKU_5G_CN),)
	@install $(BUILD_NAME)/$(SKU_5G_CN) $(SKUDIR)/SingleSKU_5G_CN.dat
endif
ifneq ($(SKU_2G_IC),)
	@install $(BUILD_NAME)/$(SKU_2G_IC) $(SKUDIR)/SingleSKU_IC.dat
endif
ifneq ($(SKU_5G_IC),)
	@install $(BUILD_NAME)/$(SKU_5G_IC) $(SKUDIR)/SingleSKU_5G_IC.dat
endif
ifneq ($(SKU_2G_AP),)
	@install $(BUILD_NAME)/$(SKU_2G_AP) $(SKUDIR)/SingleSKU_AP.dat
endif
ifneq ($(SKU_5G_AP),)
	@install $(BUILD_NAME)/$(SKU_5G_AP) $(SKUDIR)/SingleSKU_5G_AP.dat
endif

ifeq ("$(BUILD_NAME)","RT-AC52U")
	@install $(BUILD_NAME)/$(SKU_2G_FCC_V2) $(SKUDIR)/SingleSKU_FCC_0002.dat
	@install $(BUILD_NAME)/$(SKU_5G_FCC_V2) $(SKUDIR)/SingleSKU_5G_FCC_0002.dat
	@install $(BUILD_NAME)/$(SKU_2G_CE_V2) $(SKUDIR)/SingleSKU_CE_0002.dat
	@install $(BUILD_NAME)/$(SKU_5G_CE_V2) $(SKUDIR)/SingleSKU_5G_CE_0002.dat
endif

endif	#RA_SKU_IN_DRV == y



.PHONY: all clean nstall
.PHONY: $(BUILD_NAME)
