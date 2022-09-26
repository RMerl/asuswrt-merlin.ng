#!/bin/sh
# $Id: external-ip.sh,v 1.2 2017/11/02 15:33:09 nanard Exp $
# (c) 2010 Reuben Hawkins
upnpc -s | sed -n -e 's/^ExternalIPAddress = \([0-9.]*\)$/\1/p'
