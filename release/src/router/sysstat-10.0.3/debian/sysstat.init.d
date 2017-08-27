#! /bin/sh
# vim:ft=sh:et
### BEGIN INIT INFO
# Provides:          sysstat
# Required-Start:    $remote_fs $local_fs $syslog
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:
# Short-Description: Start/stop sysstat's sadc
# Description:       Sysstat contains system performance tools for Linux
#                    The init file runs the sadc command in order to write
#                    the "LINUX RESTART" mark to the daily data file
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/lib/sysstat/sa1
NAME=sadc
DESC="the system activity data collector"

test -f "$DAEMON" || exit 0
umask 022

# our configuration file
DEFAULT=/etc/default/sysstat

# default settings...
ENABLED="false"
SA1_OPTIONS=""

# ...overriden in the configuration file
test -r "$DEFAULT" && . "$DEFAULT"

set -e 
status=0

. /lib/lsb/init-functions

case "$1" in
  start|restart|reload|force-reload)
        if [ "$ENABLED" = "true" ] ; then
                log_daemon_msg "Starting $DESC" "$NAME"
                start-stop-daemon --start --quiet --exec $DAEMON -- --boot $SA1_OPTIONS || status=$?
                log_end_msg $status
        fi
        ;;
  stop)
        ;;
  status)
        if [ "$ENABLED" = "true" ] ; then
                log_success_msg "sadc cron jobs are enabled"
                exit 0
        else
                log_failure_msg "sadc cron jobs are disabled"
                exit 3
        fi
        ;;
  *)
        log_failure_msg "Usage: $0 {start|stop|restart|reload|force-reload|status}"
        exit 1
        ;;
esac

exit $status
