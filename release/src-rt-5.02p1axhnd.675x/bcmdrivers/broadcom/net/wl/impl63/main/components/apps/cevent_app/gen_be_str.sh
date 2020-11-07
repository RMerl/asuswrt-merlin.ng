#!/bin/sh
#
# Script to generate event define strings from bcmevent.h
#
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Open:>>
#
#
# $Id:$
#

if [ $# -lt 2 ]; then echo "Usage error, $0 <path/to/bcmevent.h> <some output.c>"; exit; fi
TMP_FILE=${2}.tmp
sed -n '/^#define.*WLC_E_SET_SSID.*$/,/^#define.*WLC_E_LAST.*$/p;/^#define.*WLC_E_LAST.*$/q' $1 > $TMP_FILE

llast=`grep '^#define.*WLC_E_LAST' $1 | tail -1`
nlast=`echo $llast | cut -d' ' -f3`

# redirect output from this line onwards to filename passed as $2
exec 1> $2
echo -e "/*\n * Auto-generated using $0\n *\n *\n * \$Id:\$\n */\n"
echo "/* string array of bcm_event_t types defined in "`basename $1`" */"
echo "char *ca_bcm_event_str[${nlast} + 1] = {"
echo -e "\t/* fillers from 0 to ${nlast} */"
for x in `seq 0 $nlast`; do echo -e "\t\"E_${x}\","; done
echo -e "\t/* explicit overrides for available defines (has gaps) */"
# sed -n -e 's/^#define[ \t]*\bWLC_\(E_[^ \t]\+\)[ \t]\+\([0-9]\+\)\b\([ \t]*\/\*[ ]*\(.*\)\*\/\)\?.*/\t\[\2\] \=\t"\1",\t\t\/\* \4\*\//p' $TMP_FILE
# same as above, but with comment tabs aligned with {range limited match}
sed -n	-e 's/^#define[ \t]*\bWLC_\(E_[^ \t]\{19,\}\)[ \t]\+\([0-9]\+\)\b\([ \t]*\/\*[ ]*\(.*\)\*\/\)\?.*/\t\[\2\] \=\t"\1", \/\* \4\*\//p' \
	-e 's/^#define[ \t]*\bWLC_\(E_[^ \t]\{11,18\}\)[ \t]\+\([0-9]\+\)\b\([ \t]*\/\*[ ]*\(.*\)\*\/\)\?.*/\t\[\2\] \=\t"\1",\t\/\* \4\*\//p' \
	-e 's/^#define[ \t]*\bWLC_\(E_[^ \t]\{3,10\}\)[ \t]\+\([0-9]\+\)\b\([ \t]*\/\*[ ]*\(.*\)\*\/\)\?.*/\t\[\2\] \=\t"\1",\t\t\/\* \4\*\//p' \
	-e 's/^#define[ \t]*\bWLC_\(E_[^ \t]\{1,2\}\)[ \t]\+\([0-9]\+\)\b\([ \t]*\/\*[ ]*\(.*\)\*\/\)\?.*/\t\[\2\] \=\t"\1",\t\t\t\/\* \4\*\//p' $TMP_FILE

echo '};'
rm $TMP_FILE
