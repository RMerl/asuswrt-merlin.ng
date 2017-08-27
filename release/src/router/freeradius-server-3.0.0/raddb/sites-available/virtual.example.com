# -*- text -*-
######################################################################
#
#	Sample virtual server for internally proxied requests.
#
#	See the "realm virtual.example.com" example in "proxy.conf".
#
#	$Id$
#
######################################################################

#
#  Sample contents: just do everything that the default configuration does.
#
#  You WILL want to edit this to your local needs.  We suggest copying
#  the "default" file here, and then editing it.  That way, any
#  changes to the 'default" file will not affect this virtual server,
#  and vice-versa.
#
#  When this virtual server receives the request, the original
#  attributes can be accessed as "outer.request", "outer.control", etc.
#  See "man unlang" for more details.
#
server virtual.example.com {
$INCLUDE	${confdir}/sites-available/default
}
