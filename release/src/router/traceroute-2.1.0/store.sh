#!/bin/sh
#
#   Copyright (c)  2000, 2001		Dmitry Butskoy
#					<buc@citadel.stu.neva.ru>
#   License:  GPL v2 or any later
#
#   See COPYING for the status of this software.
#

#
#   Store package script.
#   Normally invoked by a Makefile.
#
#   Stores package with an appropriate version info 
#   as the main directory postfix (i.e., name-0.1.2/*),
#   even if the source dir don`t have it.
#


[ $# -lt 2 ] && {
	echo "Usage: $0 target store_dir" >&2
	exit 2
}


target=$1
store_dir=$2
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


targ_vers=${target}-$version

cd ..

[ "$main_dir" != "$targ_vers" ] && {
    ln -s "$main_dir" "${main_dir}~" || exit 1	#  paranoia and paranoia
    mv -f "$main_dir" "$targ_vers" || exit 1
}

tar -cvhf - "$targ_vers" | gzip -c -9 > $store_dir/${targ_vers}.tar.gz

[ "$main_dir" != "$targ_vers" ] && {
    mv -f "$targ_vers" "$main_dir"
    rm -f "${main_dir}~"
}

exit 0
