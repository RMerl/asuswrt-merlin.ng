#!/bin/sh
SCHEDULE_TIME=$(nvram get schedule_time)

sleep $SCHEDULE_TIME ; startservice run_rc_shutdown; /sbin/reboot &