#!/bin/bash
#
#    tellerstats.sh                  3
#	generate graphs from the data
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

if [ -z "$LINEWIDTH" ]
then
   LINEWIDTH=5
fi
export LINEWIDTH   

if [ -z "$PLOTFORMAT" ]
then
   PLOTFORMAT=ps
fi
export PLOTFORMAT

if [ -z "$PLOTTERMINAL" ]
then
   PLOTTERMINAL="postscript eps enhanced color \"Helvetica\" 22"
fi
export PLOTTERMINAL

if [ -n "$DEBUG" ]
then
   echo "LINEWIDTH = $LINEWIDTH"
   echo "PLOTFORMAT = $PLOTFORMAT"
   echo "PLOTTERMINAL = $PLOTTERMINAL"
fi

# Trim files to 48 hour window

cd $DBPATH
files="`echo *`"

for this in $files
do
   tail $this -n576 > ${this}.tmp
   mv ${this}.tmp $this
done

###############################################

rm -rf $TEMPPATH
mkdir -p $TEMPPATH

cd $TEMPPATH

# Update primary plots
GNUPLOTSCRIPT="$TEMPPATH/gnuplotscript"
cat $GNUPLOTSCRIPT_TMPL | perl -p -e's/\$(\w+)/$ENV{$1}/g' > $GNUPLOTSCRIPT
gnuplot < $GNUPLOTSCRIPT
rm $GNUPLOTSCRIPT

files="`echo *`"

CONVERT_OPTS_A="-interlace none -scale 320x240 -quality 100"
CONVERT_OPTS_B="-interlace none -scale 800x600 -quality 100"

for this in $files
do
   prefix=`echo $this|perl -p -e's/\.\w+$//'`
   convert $CONVERT_OPTS_A $TEMPPATH/$this $HTMLPATH/${prefix}.png
   convert $CONVERT_OPTS_B $TEMPPATH/$this $HTMLPATH/${prefix}B.png
   touch $HTMLPATH/${prefix}.png $HTMLPATH/${prefix}B.png
done

# Update timestamp

touch $HTMLPATH/index.shtml

# if this was called as a cgi script, it should redirect to the index.shtml file
if [ -n "$REMOTE_HOST" ]
then
   REL_HTML=${HTMLPATH#$HTMLROOT}
   echo "Location: $REL_HTML/index.shtml"
   echo
fi

if [ -z "$DEBUG" ]
then
   rm -rf $TEMPPATH
fi   

exit 0
