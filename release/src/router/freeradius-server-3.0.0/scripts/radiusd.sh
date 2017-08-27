#!/bin/sh
#
#  The purpose of this script is to forcibly load the *correct* version
#  of OpenSSL for FreeRADIUS, when you have more than one version of OpenSSL
#  installed on your system.
#
#  You'll have to edit the directories to the correct location
#  for your local system.
#
#	$Id$
#

LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/ssl/lib:/usr/local/radius/lib
LD_PRELOAD=/usr/local/ssl/lib/libcrypto.so

export LD_LIBRARY_PATH LD_PRELOAD
exec /usr/local/radius/sbin/radiusd $@
