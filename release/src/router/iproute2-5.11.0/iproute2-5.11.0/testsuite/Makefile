# SPDX-License-Identifier: GPL-2.0
## -- Config --
DEV := lo
PREFIX := sudo -E unshare -n
RESULTS_DIR := results
## -- End Config --

HAVE_UNSHARED_UTIL := $(shell unshare --version 2> /dev/null)

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

TESTS := $(patsubst tests/%,%,$(call rwildcard,tests/,*.t))
TESTS_DIR := $(dir $(TESTS))

IPVERS := $(filter-out iproute2/Makefile,$(wildcard iproute2/*))

KENVFN := $(shell mktemp /tmp/tc_testkenv.XXXXXX)
ifneq (,$(wildcard /proc/config.gz))
	KCPATH := /proc/config.gz
else
	KVER := $(shell uname -r)
	KCPATHS := /lib/modules/$(KVER)/config /boot/config-$(KVER)
	KCPATH := $(firstword $(wildcard $(KCPATHS)))
endif

.PHONY: compile listtests alltests configure $(TESTS)

configure:
	$(MAKE) -C iproute2 configure

compile: configure generate_nlmsg
	$(MAKE) -C iproute2

listtests:
	@for t in $(TESTS); do \
		echo "$$t"; \
	done

generate_nlmsg:
	$(MAKE) -C tools

alltests: generate_nlmsg $(TESTS)

testclean:
	@echo "Removing $(RESULTS_DIR) dir ..."
	@rm -rf $(RESULTS_DIR)

clean: testclean
	@rm -f iproute2/iproute2-this
	@rm -f tests/ip/link/dev_wo_vf_rate.nl
	$(MAKE) -C tools clean

distclean: clean
	$(MAKE) -C iproute2 distclean

$(TESTS): generate_nlmsg testclean
ifeq (,$(IPVERS))
	$(error Please run make first)
endif
ifeq (,$(HAVE_UNSHARED_UTIL))
	$(error Please install util-linux tools to run tests in separated network namespace)
endif
	@./tools/generate_nlmsg

	@mkdir -p $(RESULTS_DIR)

	@for d in $(TESTS_DIR); do \
	    mkdir -p $(RESULTS_DIR)/$$d; \
	done

	@if [ "$(KCPATH)" = "/proc/config.gz" ]; then \
		gunzip -c $(KCPATH) >$(KENVFN); \
	elif [ "$(KCPATH)" != "" ]; then \
               cat $(KCPATH) >$(KENVFN); \
	fi
	@sed -i -e 's/^CONFIG_/export CONFIG_/' $(KENVFN)

	@for i in $(IPVERS); do \
		o=`echo $$i | sed -e 's/iproute2\///'`; \
		echo -n "Running $@ [$$o/`uname -r`]: "; \
		TMP_ERR=`mktemp /tmp/tc_testsuite.XXXXXX`; \
		TMP_OUT=`mktemp /tmp/tc_testsuite.XXXXXX`; \
		. $(KENVFN); \
		STD_ERR="$$TMP_ERR" STD_OUT="$$TMP_OUT" \
		TC="$$i/tc/tc" IP="$$i/ip/ip" SS=$$i/misc/ss BRIDGE="$$i/bridge/bridge" \
		DEV="$(DEV)" IPVER="$@" SNAME="$$i" \
		ERRF="$(RESULTS_DIR)/$@.$$o.err" $(PREFIX) tests/$@ > $(RESULTS_DIR)/$@.$$o.out; \
		if [ "$$?" = "127" ]; then \
			printf "\033[1;35mSKIPPED\033[0m\n"; \
		elif [ -e "$(RESULTS_DIR)/$@.$$o.err" ]; then \
			printf "\033[0;31mFAILED\033[0m\n"; \
		else \
			printf "\033[0;32mPASS\033[0m\n"; \
		fi; \
		rm "$$TMP_ERR" "$$TMP_OUT"; \
		sudo dmesg > $(RESULTS_DIR)/$@.$$o.dmesg; \
	done
