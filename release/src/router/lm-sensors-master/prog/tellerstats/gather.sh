#!/bin/bash
#
#    gather.sh
#	gather the data.
#	run from cron every 5 minutes.
#	Don't run manually as root or else files in data/ will get
#	root ownership then your (non-root) cron daemon won't work
#
#    Copyright (C) 2001  Philip Edelbrock
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#    MA 02110-1301 USA.
#

# generic tellerstats init BEGIN

# get config information from /etc/tellerstats.conf or wherever we are pointed

if [ -z "$TELLERSTATS_CONF" ]
then
   TELLERSTATS_CONF=/etc/tellerstats.conf
fi   

export TELLERSTATS_CONF

if [ ! -r $TELLERSTATS_CONF ]
then
   echo "$0: Could not find config file $TELLERSTATS_CONF"
   exit 1
fi   

. $TELLERSTATS_CONF

if [ ! -d $DBPATH ]
then
   echo "$0: data directory $DBPATH does not exist"
   exit 1
fi

if [ ! -d $SENSORPATH ]
then
   echo "$0: sensor information directory $SENSORPATH does not exist."
   exit 1
fi

if [ ! -d $HTMLROOT ]
then
   echo "$0: The root of your webserver - $HTMLROOT - does not exist..bailing out"
   exit 1
fi

if [ ! -d $HTMLPATH ]
then
   echo "$0: The place where we keep HTML files and pictures - $HTMLPATH - does not exist..bailing out"
   exit 1
fi

if [ ! -r $GNUPLOTSCRIPT_TMPL ]
then
   echo "$0: The gnuplot script template $GNUPLOTSCRIPT_TMPL does not exist..bailing out"
   exit 1
fi

export DBPATH SENSORPATH TEMPPATH HTMLROOT HTMLPATH GNUPLOTSCRIPT_TMPL

if [ -n "$DEBUG" ]
then
   echo "DBPATH = $DBPATH"
   echo "SENSORPATH = $SENSORPATH"
   echo "TEMPPATH = $TEMPPATH"
   echo "HTMLROOT = $HTMLROOT"
   echo "HTMLPATH = $HTMLPATH"
   echo "GNUPLOTSCRIPT_TMPL = $GNUPLOTSCRIPT_TMPL"
fi

# generic tellerstats init END

# From /etc/sensors.conf for the W83781D:
#
#    compute in3 ((6.8/10)+1)*@ ,  @/((6.8/10)+1)
#    compute in4 ((28/10)+1)*@  ,  @/((28/10)+1)
#    compute in5 -(210/60.4)*@  ,  -@/(210/60.4)
#    compute in6 -(90.9/60.4)*@ ,  -@/(90.9/60.4)
#

#date=yyyyMMddHHmmss, the same format gnuplot expects for the x-axis
DATE=`date +%Y%m%d%H%M%S`

T=`cat $SENSORPATH/in0   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/cpu1V

T=`cat $SENSORPATH/in1   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/cpu2V

T=`cat $SENSORPATH/in2   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/ThreeVOLT

T=`cat $SENSORPATH/in3   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
T=`echo $T \* 1.68 | bc`
echo $DATE $T >> $DBPATH/FiveVOLT

T=`cat $SENSORPATH/in4   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
T=`echo $T \* 3.8 | bc`
echo $DATE $T >> $DBPATH/TwelveVOLT

T=`cat $SENSORPATH/in5   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
T=`echo $T \* -3.477 | bc`
echo $DATE $T >> $DBPATH/NegTwelveVOLT

T=`cat $SENSORPATH/in6   | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
T=`echo $T \* -1.505 | bc`
echo $DATE $T >> $DBPATH/NegFiveVOLT

T=`cat $SENSORPATH/temp1 | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/mb_temp

T=`cat $SENSORPATH/temp2 | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/cpu_temp

T=`cat $SENSORPATH/fan1  | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/fanone

T=`cat $SENSORPATH/fan2  | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/fantwo

T=`cat $SENSORPATH/fan3  | perl -p -e 's/^.+ ([^ ]+)$/$1/'`
echo $DATE $T >> $DBPATH/fanthree

T=`cat /proc/loadavg     | perl -p -e 's/^([^ ]+) .+$/$1/'`
echo $DATE $T >> $DBPATH/load

exit 0
