#!/bin/sh
if [ -f config-local.sh ]; then
    . ./config-local.sh 
else
    [ -f /etc/chilli/defaults ] && . /etc/chilli/defaults
    [ -f /etc/chilli/config ]   && . /etc/chilli/config
fi
