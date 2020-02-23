ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6878_),)
ifeq ($(strip $(CFG_RAMAPP)),1)
BSPOBJS += bcm63xx_potp_sotp_util.o

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63148_63138_63381_),)
BSPOBJS += bcm63xx_sotp_if.o
BSPOBJS += sotp_base_funcs.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63381_),)
BSPOBJS +=           \
	bcm63xx_potp_sotp_cmd_impl1.o
else
BSPOBJS +=           \
	bcm63xx_potp_sotp_cmd.o
endif

else
# CFE_ROM build; gating with respect to the board type & BLD_BTRM_SEC option

ifeq ($(strip $(BLD_SEC_TK)),1)
BSPOBJS += bcm63xx_potp_sotp_util.o

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63148_63138_63381_),)
BSPOBJS += bcm63xx_sotp_if.o
BSPOBJS += sotp_base_funcs.o
endif

endif

endif

endif
