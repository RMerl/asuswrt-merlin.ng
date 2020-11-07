# File: bcmdrivers/broadcom/char/fap/impl1/Makefile
#
# Makefile for the fap host driver.
#
#The structure of this Makefile requires that it is called with -j1
MAKEFLAGS += -j1

override obj := $(abspath $(obj))
override src := $(abspath $(src))

# $(src) is a kbuild generated variable containing the directory from which this Makefile is run
FAP4KE_DIR = $(src)/4ke
FAP4KE_OBJ_DIR = $(src)/4ke/objs
FAP4KE_OBJ = $(FAP4KE_OBJ_DIR)/bcmfap4ke.elf
FAP4KE_DEP = $(FAP4KE_OBJ_DIR)/bcmfap4ke.d
FAP4KE_RNAMES_FILE = $(FAP4KE_DIR)/releaseNames.txt
FAP4KE_MKOPTS = $(FAP4KE_DIR)/make4keOpts
FAP4KE_REL_DIR = $(src)/releases
FAP4KE_RNAMES_CHECK_FILE = $(FAP4KE_REL_DIR)/releaseNamesCheck.txt

SCRIPTS_DIR = $(HOSTTOOLS_DIR)/scripts
FAP4KE_DEFAULT_RELEASE_NAME := $(shell head -n 1 $(FAP4KE_RNAMES_FILE))

-include $(FAP4KE_MKOPTS)
FAP4KE_H_SUFFIX := $(shell $(SCRIPTS_DIR)/metafilename.pl -m $(FAP4KE_DEFAULT_RELEASE_NAME) "$(MAKE_4KE_OPTS)")

FAP4KE_RELEASE_HEADER = $(FAP4KE_REL_DIR)/bcmfap4ke_Release.$(FAP4KE_H_SUFFIX).h
FAP4KE_RELEASE_BINARY_HEADER = $(FAP4KE_REL_DIR)/bcmfap4keBinary_Release.$(FAP4KE_H_SUFFIX).h
FAP4KE_HEADER = $(FAP4KE_OBJ_DIR)/bcmfap4ke.h
FAP4KE_BINARY_HEADER = $(FAP4KE_OBJ_DIR)/bcmfap4keBinary.h

$(info FAP4KE_RELEASE_HEADER is $(FAP4KE_RELEASE_HEADER))

obj-$(CONFIG_BCM_FAP) += bcmfap.o

# The following variable is where build-time variables are passed from the host
# to the 4ke Make.  Note: configurable parameters should be passed using
# releaseNames.txt, and metafilename.pl.  
MAKE_4KE_CMD = 	make -f $(FAP4KE_DIR)/Makefile \
		FAP4KE_OBJ=$(FAP4KE_OBJ) \
		DEP=$(FAP4KE_DEP) \
		DEPSRC=$(src) \
		DIR_4KE=$(FAP4KE_DIR) \
		BRCMDRIVERS_DIR=$(BRCMDRIVERS_DIR) \
		ROOT_DIR=$(BRCMDRIVERS_DIR)/.. \
		INC_BRCMDRIVER_PUB_PATH=$(INC_BRCMDRIVER_PUB_PATH) \
		INC_BRCMDRIVER_PRIV_PATH=$(INC_BRCMDRIVER_PRIV_PATH) \
		INC_BRCMBOARDPARMS_PATH=$(INC_BRCMBOARDPARMS_PATH) \
		INC_BRCMSHARED_PUB_PATH=$(INC_BRCMSHARED_PUB_PATH) \
		INC_BRCMSHARED_PRIV_PATH=$(INC_BRCMSHARED_PRIV_PATH) \
		BUILD_VERBOSE=$(KBUILD_VERBOSE) \
		DIR_PKTDMA=$(BRCMDRIVERS_DIR)/opensource/net/lib/pktdma/impl$(CONFIG_BCM_PKTDMA_IMPL) \
		DIR_XTMRT=$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL) \
		FAP4KE_RNAMES_FILE=$(FAP4KE_RNAMES_FILE)

bcmfap-objs += fap_driver.o fap_hw.o fap_hwInit.o fap_mailBox.o fap_interrupt.o fap_dqm.o fap_packet.o fap_protocol.o fap_mcast.o fap_l2flow.o fap_tm.o fap_swq.o
# note: the following line is not recomended kbuild practice, but will work.  Because this file is shared
# by the 4ke, we have to do this.
bcmfap-objs += 4ke/fap4ke_iopDma.o
bcmfap-objs += fap_slob.o

ifneq ("$(CONFIG_BCM_FAP_GSO)","")
bcmfap-objs += fap_gso.o 
endif
ifneq ("$(CONFIG_BCM_SPDSVC)","")
bcmfap-objs += fap_perf.o 
endif


clean-files := $(obj)/4ke/fap4ke_iopDma.o $(FAP4KE_HEADER:$(src)/%=%) $(FAP4KE_BINARY_HEADER:$(src)/%=%) $(FAP4KE_MKOPTS:$(src)/%=%) $(FAP4KE_RNAMES_CHECK_FILE:$(src)/%=%) core
clean-dirs := $(FAP4KE_OBJ_DIR:$(src)/%=%) 

ifneq ($(wildcard $(FAP4KE_DIR)/Makefile),)
ifeq ("$(RELEASE_BUILD)","")
# 4ke Makefile exists and not doing release build
clean-dirs += $(FAP4KE_REL_DIR:$(src)/%=%)
endif
endif

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(FAP4KE_DIR)
EXTRA_CFLAGS += -I$(FAP4KE_OBJ_DIR)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/broadcom/char/fap/impl$(CONFIG_BCM_FAP_IMPL)/ffe
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/enet/shared
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/xtmrt/impl$(CONFIG_BCM_XTMRT_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/net/lib/pktdma/impl$(CONFIG_BCM_PKTDMA_IMPL)
EXTRA_CFLAGS += -I$(KERNEL_DIR)/net/core

EXTRA_CFLAGS += -Werror -Wall -Wfatal-errors

EXTRA_CFLAGS += -D_MIPSREGS32_

EXTRA_CFLAGS += -g

-include $(TOPDIR)/Rules.make

$(obj)/fap_hwInit.o: $(FAP4KE_BINARY_HEADER) $(FAP4KE_HEADER)

$(obj)/fap_mailBox.o: $(FAP4KE_HEADER)

$(FAP4KE_OBJ_DIR):
	mkdir $(FAP4KE_OBJ_DIR)

$(FAP4KE_HEADER): $(FAP4KE_RELEASE_HEADER) $(FAP4KE_OBJ_DIR)
	cp -f $(FAP4KE_RELEASE_HEADER) $(FAP4KE_HEADER)

$(FAP4KE_BINARY_HEADER): $(FAP4KE_RELEASE_BINARY_HEADER) $(FAP4KE_OBJ_DIR)
	cp -f $(FAP4KE_RELEASE_BINARY_HEADER) $(FAP4KE_BINARY_HEADER)

ifeq ("$(RELEASE_BUILD)","")

# Doing a local developer build here:
$(FAP4KE_RELEASE_HEADER) $(FAP4KE_RELEASE_BINARY_HEADER): $(FAP4KE_RNAMES_CHECK_FILE) $(FAP4KE_REL_DIR) $(FAP4KE_RNAMES_FILE)
	@if [ ! -f $(FAP4KE_DIR)/Makefile ]; then \
	    if [ -f $(FAP4KE_RELEASE_HEADER) -a -f $(FAP4KE_RELEASE_BINARY_HEADER) ]; then \
	        echo "$(notdir $(FAP4KE_RELEASE_HEADER)), $(notdir $(FAP4KE_RELEASE_BINARY_HEADER)) and $(notdir $(FAP4KE_RELEASE_IOPDMA_OBJ)) exist"; \
	    else \
	        echo "Error: -- No prebuilt FAP 4KE exists for configuration $(FAP4KE_H_SUFFIX) " ; \
	        false; \
	    fi \
	else \
	    if [ `grep -c "^$(FAP4KE_H_SUFFIX)$$" $(FAP4KE_RNAMES_FILE)` == 0 ]; then \
	        echo "========================================================" >&2; \
	        echo "| WARNING: $(FAP4KE_H_SUFFIX) not in $(FAP4KE_RNAMES_FILE)" >&2; \
	        echo "|   see fap_binary_release.doc for details" >&2; \
	        if [ -n "$(FORCE)" ]; then \
	           echo "|   ignorning, as FORCE flag is set" >&2; \
	           echo "========================================================" >&2; \
	        else \
	           echo "|   or use FORCE=1 on the command line to ignore" >&2; \
	           echo "========================================================" >&2; \
	           exit 1; \
	        fi \
	    fi; \
	    echo "===================="; \
	    echo "= Making $(notdir $(FAP4KE_RELEASE_HEADER)) and $(notdir $(FAP4KE_RELEASE_BINARY_HEADER))"; \
	    echo "cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $(MAKE_4KE_OPTS) FAP4KE_RELEASE_HEADER=$(FAP4KE_RELEASE_HEADER) FAP4KE_RELEASE_BINARY_HEADER=$(FAP4KE_RELEASE_BINARY_HEADER)"; \
	    cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $(MAKE_4KE_OPTS) FAP4KE_RELEASE_HEADER=$(FAP4KE_RELEASE_HEADER) FAP4KE_RELEASE_BINARY_HEADER=$(FAP4KE_RELEASE_BINARY_HEADER); \
	    if [ $$? == 0 ]; then \
	        echo "= Done Making $(notdir $(FAP4KE_RELEASE_HEADER))"; \
	        echo "===================="; \
	    else \
	        echo "Fap4ke build failed" >&2; \
	        exit 1; \
	    fi; \
	fi


$(FAP4KE_RNAMES_CHECK_FILE): $(FAP4KE_REL_DIR) $(FAP4KE_RNAMES_FILE)
	@echo "--------------------------"
	@echo "Checking releaseNames.txt:"
	@failure=; \
	toBeChecked=`sed -e "1,/^.*$$/ d" -e "s|#.*$$||g" -e "/^$$/ d" $(FAP4KE_RNAMES_FILE)`; \
	for suffix in $$toBeChecked; do \
	   echo $$suffix; \
	   releaseOpts=$$( $(SCRIPTS_DIR)/metafilename.pl -f $(FAP4KE_DEFAULT_RELEASE_NAME) release.$$suffix ); \
	   if [[ $$releaseOpts =~ ^[\ \n\t]*$$ ]]; then \
	       failure="y"; \
	   fi; \
	done; \
	if [ $$failure ]; then \
	   echo "   ...failed"; \
	   # have to delete check file to force rule to be run again next time: \
	   rm -f $(FAP4KE_RNAMES_CHECK_FILE); \
	   exit 1; \
	else \
	   echo "Validation passed\n" > $(FAP4KE_RNAMES_CHECK_FILE); \
	   echo "  ...success"; \
	fi;

else

# This will automagically fail the build if a profile's configuration is not
# represented in the list of configurations to build
$(FAP4KE_RELEASE_HEADER) $(FAP4KE_RELEASE_BINARY_HEADER): RELEASES

# This is a bit of ugliness -- we're building 4ke images here which do not match
# the configuration of the host.  The alternatives aren't any better though, so sticking
# with this 
RELEASES: | $(FAP4KE_REL_DIR)
	@echo "========================"
	@echo "= MAKING ALL RELEASES FOR $(BRCM_CHIP)"
	@toBeBuilt=`sed -e "1,/^.*$$/ d" -e "s|#.*$$||g" -e "/^$$/ d" $(FAP4KE_RNAMES_FILE) | grep "C_$(BRCM_CHIP)"`; \
	for suffix in $$toBeBuilt; do \
	   echo $$suffix; \
	   releaseHeader=$(FAP4KE_REL_DIR)/bcmfap4ke_Release.$$suffix.h; \
	   releaseBinaryHeader=$(FAP4KE_REL_DIR)/bcmfap4keBinary_Release.$$suffix.h; \
	   if [ ! -f  $$releaseHeader ]; then \
	       echo "-------------------------------------"; \
	       echo "= Making $$releaseBinaryHeader from release"; \
	       releaseOpts=$$( $(SCRIPTS_DIR)/metafilename.pl -f $(FAP4KE_DEFAULT_RELEASE_NAME) release.$$suffix ); \
	       echo "cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $$releaseOpts FAP4KE_RELEASE_HEADER=$$releaseHeader FAP4KE_RELEASE_BINARY_HEADER=$$releaseBinaryHeader"; \
	       if [[ $$releaseOpts =~ ^[\ \n\t]*$$ ]]; then \
	           echo "Fap4ke build failed for $$releaseBinaryHeader "; \
	           echo "(error with metafilename.pl -- releaseNames.txt is likely corrupt)"; \
	           echo "-------------------------------------"; \
	           exit 1; \
	       fi; \
	       rm -rf $(FAP4KE_OBJ_DIR); \
	       rm -f $(FAP4KE_MKOPTS); \
	       cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $$releaseOpts FAP4KE_RELEASE_HEADER=$$releaseHeader FAP4KE_RELEASE_BINARY_HEADER=$$releaseBinaryHeader BUILD_VERBOSE=1; \
	       if [ $$? == 0 ]; then \
	           echo "= Done making $$releaseBinaryHeader"; \
	           echo "-------------------------------------"; \
	       else \
	           echo "Fap4ke build failed for $$releaseBinaryHeader"; \
	           echo "-------------------------------------"; \
	           exit 1; \
	       fi; \
	   fi; \
	done
	@echo "= DONE MAKING ALL RELEASES FOR $(BRCM_CHIP)"
	@echo "========================"
	@if [ ! -f $(FAP4KE_RELEASE_HEADER) ]; then \
	   echo "-----------------------"; \
	   echo "- WARNING: $(notdir $(FAP4KE_RNAMES_FILE)) does not provision for $(notdir $(FAP4KE_H_SUFFIX)) from release"; \
	   echo "= Making $(notdir $(FAP4KE_RELEASE_HEADER)) and $(notdir $(FAP4KE_RELEASE_BINARY_HEADER))"; \
	   echo "cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $(MAKE_4KE_OPTS) FAP4KE_RELEASE_HEADER=$(FAP4KE_RELEASE_HEADER) FAP4KE_RELEASE_BINARY_HEADER=$(FAP4KE_RELEASE_BINARY_HEADER)"; \
	   rm -rf $(FAP4KE_OBJ_DIR); \
	   rm -f $(FAP4KE_MKOPTS); \
	   cd $(FAP4KE_DIR); $(MAKE_4KE_CMD) $(MAKE_4KE_OPTS) FAP4KE_RELEASE_HEADER=$(FAP4KE_RELEASE_HEADER) FAP4KE_RELEASE_BINARY_HEADER=$(FAP4KE_RELEASE_BINARY_HEADER) BUILD_VERBOSE=1; \
	   if [ $$? == 0 ]; then \
	       echo "= Done making $(notdir $(FAP4KE_RELEASE_HEADER))"; \
	       echo "-------------------------------------"; \
	   else \
	       echo "Fap4ke build failed for $(notdir $(FAP4KE_RELEASE_HEADER))"; \
	       echo "-------------------------------------"; \
	       exit 1; \
	   fi \
	fi


endif

$(FAP4KE_REL_DIR) :
	@echo "Making fap 4ke release directory $(FAP4KE_REL_DIR)"
	$(Q)mkdir $(FAP4KE_REL_DIR)


$(FAP4KE_MKOPTS) : $(src)/Makefile $(FAP4KE_RNAMES_FILE)
	@echo "-----------------------"
	@echo "- Making $(notdir $@)"
	echo -n "MAKE_4KE_OPTS = " > $@
	$(SCRIPTS_DIR)/metafilename.pl -d $(FAP4KE_DEFAULT_RELEASE_NAME) >> $@


-include $(FAP4KE_DEP)

