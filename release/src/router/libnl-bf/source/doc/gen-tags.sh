#!/bin/bash

#
# Based on a script found on the englinemtn-devel mailinglist
# written by Carsten Haitzler <ras...@rasterman.com>
#

echo '<libnltags>'
for f in api/group__*.html
do
	bf=$(basename $f)

	grep -oE '<!-- doxytag.* -->' $f |
		sed 's/<!-- doxytag:/<libnltag/' |
		sed "s/-->/file=\"$bf\" \/>/" |
		sed "s/ ref=\"/ href=\"$bf#/" |
		sed 's/ member="\([^:]*::\)\([^"]*\)"/ member="\2"/' |
		sed 's/ member="\([^"]*\)"/ short="\1"/'
done
echo '</libnltags>'
