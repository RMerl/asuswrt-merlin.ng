#!/bin/sh

########################################
# define parameters
########################################

CONFDIR=/etc

########################################
# sanity check
########################################

########################################
# common api
########################################

getValue()
{
	export IFS=";"
	idx=0
	word=
	for word in $1; do
		if [ $2 -eq $idx ]; then
			break
		else
			word=
		fi
		idx=`expr $idx + 1`
	done
	export IFS=" "

}

########################################
# api for generating config
########################################

#$1: 2860/rtdev $2: ra0
genConfig()
{

	ctrl_alid=`nvram_get $1 map_controller_alid`
	agnt_alid=`nvram_get $1 map_agent_alid`
	map_controller=`nvram_get $1 map_controller`
	map_agent=`nvram_get $1 map_agent`
	map_root=`nvram_get $1 map_root`
	bh_type=`nvram_get $1 bh_type`
	ra_band=`nvram_get $1 radio_band`
	br_inf=`nvram_get $1 br_inf_name`
	lan_inf=`nvram_get $1 lan_inf_name`
	map_bss_config_priority=`nvram_get $1 bss_config_priority`

	echo "##Multi-AP configuration file##


# Has MAP controller on this device
map_controller=$map_controller

# Controller's ALID
map_controller_alid=$ctrl_alid

# Has MAP agent on this device
map_agent=$map_agent

# This device is a MAP root
map_root=$map_root

# Agent's ALID
map_agent_alid=$agnt_alid

# Default Backhault Type
bh_type=$bh_type

#Config Band setting of each Radio
radio_band=$ra_band

#bridge interface
br_inf=$br_inf

#lan interface
lan=$lan_inf

#bss config
bss_config_priority=$map_bss_config_priority
" > $CONFDIR/map_cfg.txt

}

genConfig

