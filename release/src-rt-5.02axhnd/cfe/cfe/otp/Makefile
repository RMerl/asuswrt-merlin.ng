BSPOBJS += bcm63xx_potp_sotp_util.o

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63148_63381_),)
BSPOBJS += bcm63xx_sotp_if.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_63381_),)
BSPOBJS +=           \
	bcm63xx_potp_sotp_cmd_impl1.o
else
BSPOBJS +=           \
	bcm63xx_potp_sotp_cmd.o
endif

# CFE_ROM build; gating with respect to the board type & BLD_BTRM_SEC option

ifeq ($(strip $(BLD_SEC_TK)),1)
BSPOBJS += bcm63xx_potp_sotp_util.o

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63148_63138_63381_),)
BSPOBJS += bcm63xx_sotp_if.o
endif

endif
