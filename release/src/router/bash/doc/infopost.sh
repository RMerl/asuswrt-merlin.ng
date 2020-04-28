#! /bin/sh
#
# Some of these should really be done by options to makeinfo or by
# using @setfilename, but this way we can have both bashref.info and
# bash.info (for installing)
#

sed -e 's|bashref.info|bash.info|g'
