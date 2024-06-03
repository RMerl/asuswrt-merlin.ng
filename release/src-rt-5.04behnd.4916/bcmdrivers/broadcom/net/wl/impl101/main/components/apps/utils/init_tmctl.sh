#!/bin/sh
#
# This scripts configure the runner egress_tm buffer according
# to the interface physical speed (1G, 2.5G or 10G)
#
# Copyright (C) 2023, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual>>
#
# $Id$

#
# To convert an x_MB of memory, using 1536B packet size,  the holding capacity of a queue:
# qsize = ((x_MB - 1024B) / 1536B)
#

size10GE_tid0=2730      # drop_threshold 4193280
size10GE_tid1=2730      # drop_threshold 4193280
size10GE_tid2=2730      # drop_threshold 4193280
size10GE_tid3=2730      # drop_threshold 4193280
size10GE_tid4=1364      # drop_threshold 2096128
size10GE_tid5=1364      # drop_threshold 2096128
size10GE_tid6=682       # drop_threshold 1047552
size10GE_tid7=682       # drop_threshold 1047552

size5GE_tid0=2730      # drop_threshold 4193280
size5GE_tid1=2730      # drop_threshold 4193280
size5GE_tid2=2730      # drop_threshold 4193280
size5GE_tid3=2730      # drop_threshold 4193280
size5GE_tid4=1364      # drop_threshold 2096128
size5GE_tid5=1364      # drop_threshold 2096128
size5GE_tid6=682       # drop_threshold 1047552
size5GE_tid7=682       # drop_threshold 1047552

size2GE_tid0=2730      # drop_threshold 4193280
size2GE_tid1=1364      # drop_threshold 2096128
size2GE_tid2=1364      # drop_threshold 2096128
size2GE_tid3=1364      # drop_threshold 2096128
size2GE_tid4=1364      # drop_threshold 2096128
size2GE_tid5=1364      # drop_threshold 2096128
size2GE_tid6=682       # drop_threshold 1047552
size2GE_tid7=682       # drop_threshold 1047552

size1GE_tid0=1364      # drop_threshold 2096128
size1GE_tid1=1364      # drop_threshold 2096128
size1GE_tid2=1364      # drop_threshold 2096128
size1GE_tid3=1364      # drop_threshold 2096128
size1GE_tid4=682       # drop_threshold 1047552
size1GE_tid5=682       # drop_threshold 1047552
size1GE_tid6=682       # drop_threshold 1047552
size1GE_tid7=682       # drop_threshold 1047552

tid=_tid

if [ -f /tmp/init_tmctl ]; then
  exit 0
fi

echo "init_tmctl.sh: Configuring runner egress queue sizes."
eth_list=`ls -d /sys/class/net/eth? | cut -d/ -f5-`

for ethx in $eth_list
do
  is_10G=`ethctl $ethx media-type 2>&1 | grep -c 10G`
  is_5G=`ethctl $ethx media-type 2>&1 | grep -c -e \ 5G -e \|5G`
  is_2G=`ethctl $ethx media-type 2>&1 | grep -c 2.5G`
  is_1G=`ethctl $ethx media-type 2>&1 | grep -c 1G`
  if [ $is_10G -gt 0 ]; then
    speed=10GE
  elif [ $is_5G -gt 0 ]; then
    speed=5GE
  elif [ $is_2G -gt 0 ]; then
    speed=2GE
  else
    speed=1GE
  fi

  echo >> /tmp/init_tmctl
  echo Checking $ethx: $is_10G $is_2G $is_1G $speed >> /tmp/init_tmctl

  for prio in 0 1 2 3 4 5 6 7
  do
  eval queue_size="\$size$speed$tid$prio"
    echo "tmctl setqcfg --devtype 0 --if $ethx --qid $prio --qsize $queue_size" >> /tmp/init_tmctl
    tmctl setqcfg --devtype 0 --if $ethx --qid $prio --qsize $queue_size >> /tmp/init_tmctl
  done
done

# Display all output at once:
echo "init_tmctl.sh: cat /tmp/init_tmctl to see the results"
