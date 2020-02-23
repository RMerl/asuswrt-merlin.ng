
all: check_config check_cbc

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

export LINUX_VER_STR TOOLCHAIN_PREFIX

PERL5LIB +=:$(HOSTTOOLS_PERL_LIB)
PATH :=$(PATH):$(HOSTTOOLS_DIR)/local_install/Perl/bin/
export PERL5LIB


check_cbc:
	if perl -MConvert::Binary::C -e "exit;" 2>/dev/null  && ccconfig --version  ;\
	then \
		echo "CBC Found" ; \
	else \
		tar xf Convert-Binary-C-0.77.tar.gz ; \
		cd Convert-Binary-C-0.77 ; \
		perl Makefile.PL PREFIX=$(HOSTTOOLS_DIR)/local_install/Perl LIB=$(HOSTTOOLS_DIR)/local_install/Perl ; \
		make ; \
		make install ; \
		cd .. ; \
		rm -rf Convert-Binary-C-0.77 ; \
	fi 


check_config: $(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf

$(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf: | check_cbc
	mkdir -p $(HOSTTOOLS_DIR)/local_install/conf
	if ccconfig -c $(CC) > $(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf; \
	then \
		echo "config file generation success";\
	else \
		rm $(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf;\
		cp $(HOSTTOOLS_DIR)/conf/$(TOOLCHAIN_PREFIX).conf $(HOSTTOOLS_DIR)/local_install/conf/$(TOOLCHAIN_PREFIX).conf; \
		echo "Using the default $(TOOLCHAIN_PREFIX).conf file\n"; \
	fi

shell:
	bash

