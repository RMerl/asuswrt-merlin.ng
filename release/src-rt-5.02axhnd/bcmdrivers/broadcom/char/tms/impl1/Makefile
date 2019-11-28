# File: bcmdrivers/char/tms/bcm963xx/Makefile
#
# Makefile for the BCM63xx TMS module
#
BCMARCH=$(subst ",,$(CONFIG_BCM_CPU_ARCH_NAME))
# " #(matching quote so some editors don't complain)
export BCMARCH

ifneq ($(strip $(CONFIG_SMP)),)
SMPOPT = SMP
else
SMPOPT = UNI
endif

BCM_TMS_DIR	= $(BRCMDRIVERS_DIR)/broadcom/char/tms/$(LN_NAME)

HAVESRC = $(wildcard $(src)/nci/common/*.c)

ifeq ($(strip $(HAVESRC)),)
ifneq ("$(CONFIG_DEBUG_SPINLOCK)","")
$(error "TMS binary is not compatible with CONFIG_DEBUG_SPINLOCK.")
endif
endif

obj-$(CONFIG_BCM_TMS) += nciTMSkmod.o

ifeq ($(strip $(HAVESRC)),)
nciTMSkmod-objs += nciTMSkmod_dep.o
else
nciTMSkmod-objs	:=	ethPackage.o eth2App.o eth2Drv.o ethOAM.o ethMUX.o ethTStamp.o 1ag.o 1agTLV.o 1agCCM.o	\
					1agLBM.o 1agLTM.o 1agMIPdb.o y1731.o y1731CCM.o y1731AIS.o y1731LCK.o	\
					y1731CSF.o y1731MISC.o y1731TST.o y1731LMM.o y1731DMM.o	y1731EDM.o y1731GNM.o \
					y1731ALC.o y1731SLM.o y1731ENG.o 3ah.o 3ahDSC.o 3ahPKT.o 3ahLBK.o 3ahTLV.o	\
					3ahEVT.o 3ahVAR.o 3ahMIB.o nciLINUX.o nciTimer.o nciPool.o nciVFifo.o	\
					nciVer.o nciTINT.o nciCRC.o nciMEM.o nciContext.o nciBrd.o nciDebug.o	\
					tmsXlateKrnl.o linuxeth1AG.o linuxeth3AH.o board.o tmsLINUXkmod.o
endif

#This Makefile structure requires that it will be called with -j1
#Sub-makes of this Makefile can be called with -j
MAKEFLAGS += -j1 --no-print-directory


.DEFAULT:
ifeq ($(strip $(HAVESRC)),)
	@cp $(obj)/nciTMSkmod.$(BCMARCH).o_$(SMPOPT)_save $(obj)/nciTMSkmod_dep.o
else
	@$(MAKE) -C $(BCM_TMS_DIR) -f nci/tms.mak $(MAKECMDGOALS)
endif

tms:
ifeq ($(strip $(HAVESRC)),)
	@cp $(obj)/nciTMSkmod.$(BCMARCH).o_$(SMPOPT)_save $(obj)/nciTMSkmod_dep.o
else
	@$(MAKE) -C $(BCM_TMS_DIR) -f nci/tms.mak tms 
endif

clean:
	@$(MAKE) -C $(BCM_TMS_DIR) -f nci/tms.mak clean


