
# NOTE: Once more than about 3 modules exist here, add a single check
# for check_all_installed that can bypass all of the individual checks
#

all: check_digest_crc

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /hostTools, /hostTools,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))
include $(BUILD_DIR)/make.common

export LINUX_VER_STR TOOLCHAIN_PREFIX

PERL5LIB +=:$(HOSTTOOLS_PERL_LIB)
PATH :=$(PATH):$(HOSTTOOLS_DIR)/local_install/Perl/bin/
export PERL5LIB


check_digest_crc:
	if perl -MDigest::CRC -e "exit;" ;\
	then \
		echo "Digest::CRC Found" ; \
	else \
		tar xf  Digest-CRC-0.21.tar.gz ; \
		cd Digest-CRC-0.21/ ; \
		perl Makefile.PL PREFIX=$(HOSTTOOLS_DIR)/local_install/Perl LIB=$(HOSTTOOLS_DIR)/local_install/Perl ; \
		make ; \
		make install ; \
		cd .. ; \
		rm -rf Digest-CRC-0.21/ ; \
	fi 


shell:
	bash

