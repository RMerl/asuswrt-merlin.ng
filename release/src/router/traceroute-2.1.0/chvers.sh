#!/bin/sh
#
#   Copyright (c)  2000, 2001		Dmitry Butskoy
#					<buc@citadel.stu.neva.ru>
#   License:  GPL v2 or any later
#
#   See COPYING for the status of this software.
#

#
#   Change version script.
#   Normally invoked by a Makefile.
#
#   Changes version info in directory name, in VERSION file
#   and rpm spec file (if some of these are used).
#
#   `release3' changes:		0.2.7 --> 0.2.8
#   `release2' changes:		0.2.7 --> 0.3.0
#   `release1' changes:		0.2.7 --> 1.0.0
#   etc.
#   `release' without a digit increment the last number used.
#
#


[ $# -lt 1 ] && {
	echo "Usage: $0 release[123...0]" >&2
	exit 2
}

level=`expr $1 : '.*\([0-9]\)$'`
[ -z "$level" ] && level=0

main_dir=`basename \`pwd\``


#  Find current version info.

dir_v=""
file_v=""

dir_v=`expr $main_dir : '.*-\([0-9.]*\)$'`
[ -r VERSION ] && file_v=`awk '/^ *#/ { print $3 ; exit; }
			/^ *VERSION *=/ {split ($0, a, "="); print a[2]; exit; }
			{ print $0; exit; }' < VERSION `
[ -n "$file_v" ] && file_v=`echo $file_v `	#  to strip possible spaces

[ -z "$file_v" -a -z "$dir_v" ] && {
	echo "$0: Cannot determine version (use dirname postfix or VERSION file)" >&2
	exit 2
}

[ -n "$file_v" -a -n "$dir_v" -a "$dir_v" != "$file_v" ] && {
	echo "$0: Different version from dirname postfix and VERSION file" >&2
	exit 2
}

version="$dir_v"
[ -z "$version" ] && version="$file_v"


#  Increment current version, as specified by level.

new_version=`echo $version | awk '{ 
	level = '"$level"';
	n = split ($0, a, ".");
	if (level == 0)  level = n;
	for (i = 1; i <= n; i++) {
	    if (i == level)  a[i] = a[i] + 1 ;
	    else if (i > level)  a[i] = 0 ;
	}
	str = a[1]
	for (i = 2; i <= n; i++)  str = str "." a[i]
	print str
}' 2>/dev/null `


#  Adjust VERSION file, if any.

#  it is ugly, because $version contains dots...
[ -n "$file_v" ] && {
	sed "s/$version/$new_version/" < VERSION > VERSION.new && mv -f VERSION.new VERSION
}

#  Adjust rpm .spec file, if any.
for spec in *.spec
do
    [ -f $spec ] || continue

    grep '^Version:[ 	]*'"$version" $spec >/dev/null 2>&1 || continue
    sed '/^Version:[ 	]*'"$version/ s/$version/$new_version/" < $spec > ${spec}.new && mv -f ${spec}.new $spec
done
 

#  Adjust dirname postfix, if any.

[ -n "$dir_v" ] && {
    base=`expr \`pwd\` : '^\(.*\)-'"$version"'$' `
    mv -f ${base}-$version ${base}-$new_version
}

exit 0
