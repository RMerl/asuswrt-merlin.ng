#!/bin/bash
#
# healthd -- 	This is a simple daemon which can be used to alert you in the
#		event of a hardware health monitoring alarm by sending an 
#		email to the value of ADMIN_EMAIL (defined below).
#
# To Use  --	Simply start the daemon from a shell (may be backgrounded)
#
# Other details -- Checks status every 15 seconds.  Sends warning emails every
#		   ten minutes during alarm until the alarm is cleared.
#		   It won't start up if there is a pending alarm on startup.
#		   Very low loading on the machine (sleeps almost all the time).
#		   This is just an example.  It works, but hopefully we can
#		   get something better written. :')
#
# Requirements -- mail, sensors, bash, sleep
#
# Written & Copyrighten by Philip Edelbrock, 1999.
#
# Version: 1.1
#

ADMIN_EMAIL="root@localhost"

# Try loading the built-in sleep implementation to avoid spawning a
# new process every 15 seconds
enable -f sleep.so sleep >/dev/null 2>&1

sensors_state=$(sensors)
if [[ "$sensors_state" =~ 'ALARM' ]]
then
        echo "Pending Alarms on start up!  Exiting!"
        exit
fi

while true
do
 sleep 15
 sensors_state=$(sensors)
 if [[ "$sensors_state" =~ 'ALARM' ]]
 then
        echo "$sensors_state" | mail -s '**** Hardware Health Warning ****' $ADMIN_EMAIL
        sleep 600
 fi
done
