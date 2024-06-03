#!/bin/sh
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

# This script creates the configuration whitelist file. This file contains
# all the config options which are allowed to be used outside Kconfig.
# Please do not add things to the whitelist. Instead, add your new option
# to Kconfig.
#
export LC_ALL=C LC_COLLATE=C

# There are two independent greps. The first pulls out the component parts
# of CONFIG_SYS_EXTRA_OPTIONS. An example is:
#
#	SUN7I_GMAC,AHCI,SATAPWR=SUNXI_GPB(8)
#
# We want this to produce:
#	CONFIG_SUN7I_GMAC
#	CONFIG_AHCI
#	CONFIG_SATAPWR
#
# The second looks for the rest of the CONFIG options, but excludes those in
# Kconfig and defconfig files.
#
(
git grep CONFIG_SYS_EXTRA_OPTIONS |sed -n \
	's/.*CONFIG_SYS_EXTRA_OPTIONS="\(.*\)"/\1/ p' \
	| tr , '\n' \
	| sed 's/ *\([A-Za-z0-9_]*\).*/CONFIG_\1/'

git grep CONFIG_ | \
	egrep -vi "(Kconfig:|defconfig:|README|\.py|\.pl:)" \
	| tr ' \t' '\n\n' \
	| sed -n 's/^\(CONFIG_[A-Za-z0-9_]*\).*/\1/p'
) \
	|sort |uniq >scripts/config_whitelist.txt.tmp1;

# Finally, we need a list of the valid Kconfig options to exclude these from
# the whitelist.
cat `find . -name "Kconfig*"` |sed -n \
	-e 's/^\s*config *\([A-Za-z0-9_]*\).*$/CONFIG_\1/p' \
	-e 's/^\s*menuconfig *\([A-Za-z0-9_]*\).*$/CONFIG_\1/p' \
	|sort |uniq >scripts/config_whitelist.txt.tmp2

# Use only the options that are present in the first file but not the second.
comm -23 scripts/config_whitelist.txt.tmp1 scripts/config_whitelist.txt.tmp2 \
	|sort |uniq >scripts/config_whitelist.txt.tmp3

# If scripts/config_whitelist.txt already exists, take the intersection of the
# current list and the new one.  We do not want to increase whitelist options.
if [ -r scripts/config_whitelist.txt ]; then
	comm -12 scripts/config_whitelist.txt.tmp3 scripts/config_whitelist.txt \
		> scripts/config_whitelist.txt.tmp4
	mv scripts/config_whitelist.txt.tmp4 scripts/config_whitelist.txt
else
	mv scripts/config_whitelist.txt.tmp3 scripts/config_whitelist.txt
fi

rm scripts/config_whitelist.txt.tmp*

unset LC_ALL LC_COLLATE
