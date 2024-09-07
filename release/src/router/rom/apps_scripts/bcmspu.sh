#!/bin/sh

echo
echo "###### /debug/bcmspu/stats ######"
cat /debug/bcmspu/stats
echo

echo
echo "###### /debug/bcmspu/offload_sessions ######"
cat /debug/bcmspu/offload_sessions
echo

echo
echo "###### ip xfrm stat ######"
ip xfrm stat
echo

if [ -e /proc/fcache ]; then
	echo "###### fcctl status ######"
	fc status
	echo
	echo "###### /proc/fcache/* ######"
	cat /proc/fcache/*
	echo
	echo "###### /proc/fcache/nflist ######"
	cat /proc/fcache/nflist
	echo
	echo "###### /proc/fcache/misc/* ######"
	cat /proc/fcache/misc/*
	echo
	echo "###### /proc/fcache/stats/* ######"
	cat /proc/fcache/stats/*
fi
