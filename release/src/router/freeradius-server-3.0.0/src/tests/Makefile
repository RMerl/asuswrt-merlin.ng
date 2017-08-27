# -*- makefile -*-
##
## Makefile -- Build and run tests for the server.
##
##	http://www.freeradius.org/
##	$Id$
##
#
include ../../Make.inc

BUILD_PATH := $(top_builddir)/build
TEST_PATH := $(top_builddir)/src/tests
BIN_PATH := $(BUILD_PATH)/bin/local
LIB_PATH := $(BUILD_PATH)/lib/.libs/
DYLD_LIBRARY_PATH := $(DYLD_LIBRARY_PATH):$(LIB_PATH)

export DYLD_LIBRARY_PATH

RADDB_PATH := $(top_builddir)/raddb

TESTS	= user_password vpiter chap mschapv1 digest-01/digest* \
	test.example.com wimax

PORT	 = 12340
#PORT	 = 1812
ACCTPORT = $(shell expr $(PORT) + 1)

#	example.com stripped.example.com

EAPOL_TEST = eapol_test

EAP_TLS_TESTS = eap-ttls-pap.conf eap-mschapv2.conf \
	      eap-ttls-mschapv2.conf peap-mschapv2.conf

SECRET	= testing123

.PHONY: all eap dictionary clean

#
#	Build the directory for testing the server
#
all: parse tests

clean:
	@rm -f $(RADDB_PATH)/test.conf test.conf dictionary

dictionary:
	@echo "# test dictionary not install.  Delete at any time." > dictionary
	@echo '$$INCLUDE ' $(top_builddir)/share/dictionary >> dictionary
	@echo '$$INCLUDE ' $(top_builddir)/src/tests/dictionary.test >> dictionary
	@echo '$$INCLUDE ' $(top_builddir)/share/dictionary.dhcp >> dictionary

test.conf: dictionary
	@echo "# test configuration file.  Do not install.  Delete at any time." > test.conf
	@echo "libdir =" $(LIB_PATH) >> test.conf
	@echo "testdir =" $(TEST_PATH) >> test.conf
	@echo 'logdir = $${testdir}' >> test.conf
	@echo 'radacctdir = $${testdir}' >> test.conf
	@echo 'pidfile = $${testdir}/radiusd.pid' >> test.conf
	@echo '$$INCLUDE radiusd.conf' >> test.conf
	@echo '$$INCLUDE $${testdir}/config/' >> test.conf

radiusd.pid: $(RADDB_PATH)/test.conf test.conf
	@rm -f $(TEST_PATH)/gdb.log $(TEST_PATH)/radius.log
	@gdb -silent -x $(TEST_PATH)/gdb.conf --args \
	$(BIN_PATH)/radiusd -fPtxxxxml $(TEST_PATH)/radius.log -d $(RADDB_PATH) -n test -i 127.0.0.1 -p $(PORT) -D $(TEST_PATH) > $(TEST_PATH)/gdb.log 2>&1 &

# We can't make this depend on radiusd.pid, because then make will create
# radiusd.pid when we make radiusd.kill, which we don't want.
.PHONY: radiusd.kill
radiusd.kill:
	@if [ -f radiusd.pid ]; then \
	    ret=0; \
	    if ! ps `cat $(TEST_PATH)/radiusd.pid` >/dev/null 2>&1; then \
		rm -f radiusd.pid; \
	        echo "FreeRADIUS terminated during test"; \
	        echo "GDB output was:"; \
	        cat "$(TEST_PATH)/gdb.log"; \
	        echo "Last log entries were:"; \
	        tail -n 20 $(TEST_PATH)/radius.log; \
	        ret=1; \
	    fi; \
		if ! kill -TERM `cat $(TEST_PATH)/radiusd.pid` >/dev/null 2>&1; then \
		    ret=1; \
		fi; \
		exit $$ret; \
	fi
	@rm -f radiusd.pid

#  Link from the main database directory to here
$(RADDB_PATH)/test.conf: test.conf
	@[ -f $(RADDB_PATH)/test.conf ] || ln -s ../src/tests/test.conf $(RADDB_PATH)/

# kill the server (if it's running)
# start the server
# run the tests (ignoring any failures)
# kill the server
# remove the changes to raddb/
tests: $(RADDB_PATH)/test.conf radiusd.kill
	@chmod a+x runtests.sh
	@$(MAKE) radiusd.pid
	@echo "Waiting for server to start..."; \
	for i in 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0; do \
		printf "$$i "; \
		if [ -f '$(TEST_PATH)/radiusd.pid' ]; then \
			if ps `cat $(TEST_PATH)/radiusd.pid > /dev/null` > /dev/null 2>&1; then \
				echo; echo "Server started with PID" `cat $(TEST_PATH)/radiusd.pid`; \
				exit 0; \
			else \
				break; \
			fi; \
		fi; \
		sleep 1; \
	done; echo; \
	echo "FreeRADIUS daemon failed to start"; \
	echo "Last log entries were:"; \
	tail -n 20 "$(TEST_PATH)/radius.log"; \
	echo "GDB output was:"; \
	cat "$(TEST_PATH)/gdb.log"; \
	exit 1;
	@BIN_PATH="$(BIN_PATH)" PORT="$(PORT)" ./runtests.sh $(TESTS)
	@$(MAKE) radiusd.kill
	@rm -f $(RADDB_PATH)/test.conf

tests.eap: $(RADDB_PATH)/test.conf radiusd.kill
	@chmod a+x runtests.sh
	@rm -f $(TEST_PATH)/radius.log $(TEST_PATH)/gdb.log
	@$(MAKE) radiusd.pid
	@$(MAKE) eap
	@$(MAKE) radiusd.kill
	@rm -f $(RADDB_PATH)/test.conf

eap: $(EAP_TLS_TESTS)
	for x in $(EAP_TLS_TESTS); do \
		$(EAPOL_TEST) -c $$x -p $(PORT) -s $(SECRET); \
	done

md5:
	$(EAPOL_TEST) -c eap-md5.conf -s $(SECRET) 

tls:
	$(EAPOL_TEST) -c eap-ttls-tls.conf -s $(SECRET)

ttls:
	$(EAPOL_TEST) -c eap-ttls-pap.conf -s $(SECRET)

peap:
	$(EAPOL_TEST) -c peap-mschapv2.conf -s $(SECRET)

leap:
	$(EAPOL_TEST) -c leap.conf -s $(SECRET)

ATTRS	:= rfc.txt errors.txt extended.txt lucent.txt wimax.txt \
	condition.txt xlat.txt

parse: $(ATTRS) $(BIN_PATH)/radattr
	$(BIN_PATH)/radattr -d $(top_srcdir)/share $(ATTRS)
