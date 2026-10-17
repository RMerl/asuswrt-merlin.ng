############################################################################
#
# A central place to check the hostTools versions.
#
############################################################################
all default: prebuild_checks

############################################################################
#
# A lot of the stuff in the original Makefile has been moved over
# to make.common.
#
############################################################################
BUILD_DIR = $(CURDIR)
include $(BUILD_DIR)/make.common

REQUIRED_MAKE_VERSION := 4.2.1
CMAKE := $(if $(shell which cmake3),cmake3,cmake)
ifneq ($(strip $(BUILD_BCM_BSTORM)),)
REQUIRED_CMAKE_VERSION := 3.26.3
else
REQUIRED_CMAKE_VERSION := 3.14.0
endif
REQUIRED_HOST_KERNEL_VERSION := 2.6
REQUIRED_AUTOMAKE_VERSION := 1.13.4
REQUIRED_AUTOCONF_VERSION := 2.65
REQUIRED_LIBTOOLIZE_VERSION := 1.4.0
REQUIRED_BCMTOOL_VERSION := Rel1.8

REQUIRED_BISON_VERSION := 3.0.4
EXCLUDE_BISON_VERSIONS := 3.7.9,3.7.91,3.8 
REQUIRED_GIT_VERSION := 2.32.0

BCMTOOL_VERSION := `echo "BCM_TOOLCHAIN_VERSION" | gcc -E -P -o- -include $${TOOLCHAIN_TOP}/${TOOLCHAIN_USR_DIR}/include/bcm_toolver.h -xc - | sed 's/^"\(.*\)"$$/\1/'`

HOST_KERNEL_VERSION=$(shell uname -r)

prebuild_checks:
	@echo "shell is $(SHELL).  Bash version is $(shell echo $$BASH_VERSION)"
	@echo "BCM_MODULAR_BUILD = $(BCM_MODULAR_BUILD)"
	@echo "BUILD_DIR         = $(BUILD_DIR)"
	@echo "EXT_BUILD_DIR     = $(EXT_BUILD_DIR)"
	@echo "EXT_DEVICEFS_DIR  = $(EXT_DEVICEFS_DIR)"
	@if [ -z "$(shell echo $$BASH_VERSION)" ]; then \
		echo "***************************************************"; \
		echo "ERROR: $(SHELL) does not invoke bash shell"; \
		echo "***************************************************"; \
		exit 1; \
	fi
	@if [ $(shell whoami) == root ]; then \
		echo "***************************************************"; \
		echo "ERROR: Attempting to build as root."; \
		echo "***************************************************"; \
		exit 1; \
	fi
	@if [ ! -d $(TOOLCHAIN_TOP) ]; then \
		echo "****************************************************"; \
		echo "ERROR: could not find Toolchain"; \
		echo "$(TOOLCHAIN_TOP)"; \
		echo "****************************************************"; \
		exit 1; \
	fi
	@if [ ! -d $(KERNEL_DIR) ]; then \
		echo "****************************************************"; \
		echo "ERROR: Could not find kernel directory:             "; \
		echo "$(KERNEL_DIR) "; \
		echo "****************************************************"; \
		exit 1; \
	fi
	@if [[ $(BUILD_DIR) == *" "* ]]; then \
		echo "****************************************************"; \
		echo "ERROR: BUILD_DIR may not contain spaces             "; \
		echo "$(BUILD_DIR) "; \
		echo "****************************************************"; \
		exit 1; \
	fi
ifneq ($(BCM_MODULAR_BUILD)),)
	@if [[ "$(EXT_BUILD_DIR)" == "" ]]; then \
		echo "****************************************************"; \
		echo "ERROR: in BCM_MODULAR_BUILD, EXT_BUILD_DIR must be set"; \
		echo "****************************************************"; \
		exit 1; \
	fi
	@if [[ "$(EXT_DEVICEFS_DIR)" == "" ]]; then \
		echo "****************************************************"; \
		echo "ERROR: in BCM_MODULAR_BUILD, EXT_DEVICEFS_DIR must be set"; \
		echo "****************************************************"; \
		exit 1; \
	fi
endif

	@echo "prebuild_checks.mk: checking for required host tools before building:"
	@echo "Checking python3 version"
	@if ! python3 --version 2>/dev/null; then \
		echo "ERROR: python3 is required "; \
		exit 1; \
	else \
		pyver=`python3 --version | cut -d " " -f2 | sed 's/[^0-9]*//g'`; \
		if [[ $$pyver -lt 370 ]]; then \
			echo "ERROR: python3 version must be 3.7.0 or greater "; \
			exit 1; \
		fi; \
		if [ "" != "$(findstring OPTEE,$(PROFILE))" ]; then \
			if ! python3 -c "from cryptography.hazmat.primitives import hashes"; then \
				echo "***************************************************"; \
				echo "ERROR: Install cryptography for python"; \
				echo "***************************************************"; \
				exit 1; \
			fi; \
		fi; \
		if [ "" != "$(findstring BUILD_OPENSYNC,$(PROFILE))" ]; then \
			if ! python3 -c "import kconfiglib" || \
			   ! python3 -c "import pydot" || \
			   ! python3 -c "import jinja2"; then \
				echo "***************************************************"; \
				echo "ERROR: opensync needs python3 modules: kconfiglib, pydot, jinja2"; \
				echo "***************************************************"; \
				exit 1; \
			fi; \
			if ! quilt --version 2>/dev/null; then \
				echo "ERROR: quilt is required for opensync build "; \
	 			exit 1; \
			fi; \
		fi; \
		if [ "" != "$(BUILD_MESON_NINJA)" ]; then \
			if ! python3 -c "import packaging"; then \
				echo "***************************************************"; \
				echo "ERROR: Need python3-packaging"; \
				echo "***************************************************"; \
				exit 1; \
			fi; \
		fi; \
	fi
	@if ! xxd -v 2>/dev/null; then \
		echo "ERROR: xxd is required for build                   "; \
		exit 1; \
	fi
	@if ! realpath --version 2>/dev/null; then \
		echo "ERROR: realpath is required for build                   "; \
		exit 1; \
	fi
	@if ! gawk --version 2>/dev/null; then \
		echo "ERROR: gawk is required for build quagga           "; \
		exit 1; \
	fi
	@if ! rsync --version 2>/dev/null; then \
		echo "ERROR: rsync is required for build "; \
		exit 1; \
	fi
ifneq ($(strip $(BUILD_NINJA_GN)),)
	@echo "Check clang is installed"
	@if ! clang --version 2>/dev/null; then \
		echo "ERROR: clang is required for build "; \
		exit 1; \
	else \
		clangver=`clang --version | grep version | sed 's/^.*version //g' | cut -d " " -f1 | cut -d "-" -f1 | sed 's/[^0-9]*//g'`; \
		if [[ $$clangver -lt 900 ]]; then \
			echo "ERROR: clang version must be 9.0.0 or greater "; \
			exit 1; \
		fi; \
	fi
endif
	@echo "Check git is installed"
	@if ! git --version 2>/dev/null; then \
		echo "ERROR: git is required for build "; \
		exit 1; \
	else \
		if [ ! "x"`env |grep AUTOFETCH |grep -m 1 _git` == "x" ]; then \
			$(HOSTTOOLS_DIR)/scripts/checkver.pl -e git -r $(REQUIRED_GIT_VERSION) || $(COND_FAIL); \
		fi \
	fi
	@echo "Checking openssl version"
	@if ! openssl version 2>/dev/null; then \
		echo "ERROR: openssl is required "; \
		exit 1; \
	else \
		sslver=`openssl version | cut -d " " -f2 | sed 's/[^0-9]*//g'`; \
		if [[ $$sslver -lt 100 ]]; then \
			echo "ERROR: openssl version must be 1.0.0a or greater "; \
			exit 1; \
		fi; \
	fi
	@echo "Checking openssl devel headers"
	@if ! echo "#include <openssl/ssl.h>" | gcc -E - >/dev/null; then \
		echo "ERROR: ssl development library is required for build"; \
		echo "       usually, this is provided by the libssl-dev library"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@echo "Checking sed version"
	@if ! echo bcrm|sed -E 's/brcm/BRCM/' 2> /dev/null > /dev/null; then\
		echo "ERROR: sed version 4.2.1 or greater is required for the 4.19 kernel build"; \
		exit 1; \
	fi;
	@echo "Checking make version"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -s $(MAKE_VERSION) -r $(REQUIRED_MAKE_VERSION) || $(COND_FAIL);
	@echo "Checking cmake version"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e $(CMAKE) -r $(REQUIRED_CMAKE_VERSION) || $(COND_FAIL);

	@echo "Checking host kernel version"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -s $(HOST_KERNEL_VERSION) -r $(REQUIRED_HOST_KERNEL_VERSION) || $(COND_FAIL);

ifeq ($(strip $(DESKTOP_LINUX)),)
ifeq ($(strip $(BRCM_USE_ALT_TOOLCHAIN)),)
	@echo "Checking bcm tools version"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -s $(BCMTOOL_VERSION) -r $(REQUIRED_BCMTOOL_VERSION) --quiet; \
	 if [ $$? != 0 ]; then \
	 	echo "****************************************************"; \
	 	echo "* WARNING:  "; \
	 	echo "* A newer version of the bcm toolchain is available."; \
	 	echo "* It is recommended you upgrade to $(REQUIRED_BCMTOOL_VERSION)."; \
	 	echo "****************************************************"; \
	 fi; \
	 echo "$(BCMTOOL_VERSION)";
else
	@echo "Skipping	bcm tools version check because alternate toolchain is being used!"
endif
	@echo "Checking automake version:"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e automake -r $(REQUIRED_AUTOMAKE_VERSION) || $(COND_FAIL);
	@if [ "`which automake`" -ef "$(TOOLCHAIN_TOP)/$(TOOLCHAIN_USR_DIR)/bin/automake" ]; then \
		echo "----------------------------------------------------------------"; \
		echo "| ERROR: automake is being run from the cross-compile directory."; \
		echo "| Please install a local copy on your build machine."; \
		echo "----------------------------------------------------------------"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@echo "Checking autoconf version:"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e autoconf -r $(REQUIRED_AUTOCONF_VERSION) || $(COND_FAIL);
	@if [ "`which autoconf`" -ef "$(TOOLCHAIN_TOP)/$(TOOLCHAIN_USR_DIR)/bin/autoconf" ]; then \
		echo "----------------------------------------------------------------"; \
		echo "| ERROR: autoconf is being run from the cross-compile directory."; \
		echo "| Please install a local copy on your build machine."; \
		echo "----------------------------------------------------------------"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@echo "Checking libtoolize version:"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e libtoolize -r $(REQUIRED_LIBTOOLIZE_VERSION) || $(COND_FAIL);
	@if [ "`which libtoolize`" -ef "$(TOOLCHAIN_TOP)/$(TOOLCHAIN_USR_DIR)/bin/libtoolize" ]; then \
		echo "----------------------------------------------------------------"; \
		echo "| ERROR: libtoolize is being run from the cross-compile directory."; \
		echo "| Please install a local copy on your build machine."; \
		echo "----------------------------------------------------------------"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
endif  # DESKTOP_LINUX eq empty

	@echo "Checking bison version:"
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e bison -r $(REQUIRED_BISON_VERSION) -x $(EXCLUDE_BISON_VERSIONS) || $(COND_FAIL);
	@echo "Check flex is installed:"
	@if ! flex --version 2>/dev/null; then \
		echo "ERROR: flex is required for build "; \
		exit 1; \
	fi
	@echo "Checking tar version:"
	@tarVer=`$(HOSTTOOLS_DIR)/scripts/checkver.pl -e tar`; \
	echo "$$tarVer"; \
	if [ $$tarVer == "1.23" ]; then \
		echo "----------------------------------------------------------------"; \
		echo "| There is a known bug in tar 1.23 which causes build failures."; \
		echo "| 1.22 and 1.24 are known to be OK.  Please upgrade your tar to "; \
		echo "| another version"; \
		echo "----------------------------------------------------------------"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! echo "#include <popt.h>" | gcc -E - >/dev/null; then \
		echo "ERROR: popt development library is required for build"; \
		echo "       usually, this is provided by the popt-devel (CentOS) or"; \
		echo "       libpopt-dev (Ubuntu) library"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! echo "#include <lzo/lzo1x.h>" | gcc -E - >/dev/null; then \
		echo "ERROR: lzo/lzo1x.h development library is required for build"; \
		echo "       usually, this is provided by the liblzo2-dev library"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! echo "#include <uuid/uuid.h>" | gcc -E - >/dev/null; then \
		echo "ERROR: libuuid development library is required for build"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! echo "#include <ncurses.h>" | gcc -E - >/dev/null; then \
		echo "ERROR: ncurses development library is required for build"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! pkg-config --exists --print-errors   zlib ; then \
		echo "ERROR: pkg-config zlib failed"; \
		echo "Your distribution may require you to use a special setting of PKG_CONFIG_PATH"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! pkg-config --exists --print-errors   uuid ; then \
		echo "ERROR: pkg-config uuid failed"; \
		echo "Your distribution may require you to use a special setting of PKG_CONFIG_PATH"; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@if ! perl -MCPAN -MExtUtils::MakeMaker -e exit 2>/dev/null ; then \
		echo "ERROR: you are missing a basic functioning perl installation"; \
		echo "       please be sure that you have perl installed including"; \
		echo "       the CPAN and ExtUtils::MakeMaker components"; \
		echo "       These components are standard built-in components in the perl core"; \
		echo "       but some distributions package them seperately anyway."; \
		if [ -z "$(FORCE)" ]; then exit 1; fi \
	fi
	@echo "Checking patch version:";
	@$(HOSTTOOLS_DIR)/scripts/checkver.pl -e patch || $(COND_FAIL);
	@pkg_c=`PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=/foo pkg-config --list-all | wc -l`; \
	if [ ! $$pkg_c == "0" ];then \
		echo "ERROR: wrong pkg-config, please check PATH, should not use pkg-config from /tools/bin "; \
		echo which pkg-config; \
		echo $(shell which pkg-config); \
		exit 1; \
	fi
ifneq ($(strip $(BUILD_PROCPS)),)
	@echo "Check autopoint is installed:"
	@if ! autopoint --version 2>/dev/null; then \
		echo "ERROR: autopoint is required for build "; \
		exit 1; \
	fi
endif
