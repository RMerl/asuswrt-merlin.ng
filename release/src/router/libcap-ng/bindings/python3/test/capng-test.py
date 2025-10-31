#!/usr/bin/env python3

import os
import sys
import time
load_path = '../'
if False:
    sys.path.insert(0, load_path)

import capng

last = capng.CAP_LAST_CAP
try:
	with open('/proc/sys/kernel/cap_last_cap', 'r') as f:
		last = int(f.readline())
except IOError as e:
	print("Error opening /proc/sys/kernel/cap_last_cap: {0}".format(e.strerror))

print("Doing basic bit tests...")
capng.capng_clear(capng.CAPNG_SELECT_BOTH)
if capng.capng_have_capabilities(capng.CAPNG_SELECT_BOTH) != capng.CAPNG_NONE:
	print("Failed clearing capabilities\n")
	sys.exit(1)

capng.capng_fill(capng.CAPNG_SELECT_BOTH)
if capng.capng_have_capabilities(capng.CAPNG_SELECT_BOTH) != capng.CAPNG_FULL:
	print("Failed filling capabilities")
	sys.exit(1)

text = capng.capng_print_caps_numeric(capng.CAPNG_PRINT_BUFFER, capng.CAPNG_SELECT_CAPS)
len = len(text)
if len < 80 and last > 30:
	last = 30

print("Doing advanced bit tests for %d capabilities...\n" % (last))
for i in range(last+1):
	capng.capng_clear(capng.CAPNG_SELECT_BOTH)
	rc = capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE, i)
	if rc:
		print("Failed update test 1")
		sys.exit(1)

	rc = capng.capng_have_capability(capng.CAPNG_EFFECTIVE, int(i))
	if rc <= capng.CAPNG_NONE:
		print("Failed have capability test 1")
		capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT, capng.CAPNG_SELECT_CAPS)
		sys.exit(1)

	if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) != capng.CAPNG_PARTIAL:
		print("Failed have capabilities test 1")
		sys.exit(1)

	capng.capng_fill(capng.CAPNG_SELECT_BOTH)
	rc = capng.capng_update(capng.CAPNG_DROP, capng.CAPNG_EFFECTIVE, i)
	if rc:
		print("Failed update test 3")
		sys.exit(1)

	if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS)!=capng.CAPNG_PARTIAL:
		print("Failed have capabilities test 3")
		capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT, capng.CAPNG_SELECT_CAPS)
		sys.exit(1)

	rc = capng.capng_update(capng.CAPNG_ADD, capng.CAPNG_EFFECTIVE, i)
	if rc:
		print("Failed update test 4")
		sys.exit(1)

	if capng.capng_have_capabilities(capng.CAPNG_SELECT_CAPS) != capng.CAPNG_FULL:
		print("Failed have capabilities test 4")
		capng.capng_print_caps_numeric(capng.CAPNG_PRINT_STDOUT, capng.CAPNG_SELECT_CAPS)
		sys.exit(1)

sys.exit(0)
