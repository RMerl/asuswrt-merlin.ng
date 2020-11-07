#
# 802.1x Linux Builds Makefile
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id: Makefile 739182 2018-01-05 11:09:58Z $
#

#ifdef BCM_CEVENT
ifeq ($(CONFIG_BCM_CEVENT),y)
CFLAGS += -DBCM_CEVENT
endif
#endif // endif

# targets/builds
BUILDS = nas

# default build
.PHONY: all
all : ${BUILDS}

.PHONY: ${BUILDS}
${BUILDS} ::
	+$(MAKE) -C $@

# clean up
BUILDS-CLEAN = ${BUILDS:=-clean}

.PHONY: clean
clean : ${BUILDS-CLEAN}

.PHONY: ${BUILDS-CLEAN}
${BUILDS-CLEAN} ::
	$(MAKE) -C ${@:-clean=} clean

# install
BUILDS-INSTALL = ${BUILDS:=-install}

.PHONY: install
install : ${BUILDS-INSTALL}

.PHONY: ${BUILDS-INSTALL}
${BUILDS-INSTALL} ::
	+$(MAKE) -C ${@:-install=} install
