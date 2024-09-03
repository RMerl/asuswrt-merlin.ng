#!/bin/bash

mcbs_68880=(1001324 1001524 1005524 1011722)
mcbs_6837=(1001524 141635)

boards_68880_1001324=(DV REF1 XSV)
boards_68880_1001524=(DV2G REF1D REF2)
boards_68880_1005524=(REF3)
boards_68880_1011722=(DV4G)

boards_6837_1001524=REF3
boards_6837_141635=OTHER

help () {
    echo "Usage: $(basename ${0%.*}).sh -p profile"
    echo "            -p          profile name as it is in make PROFILE parameter"
    exit 0
}

while getopts "hp:" options; do
    case "${options}" in
        p)
            profile=${OPTARG}
            ;;
        h)
            help
            ;;
    esac
done

cd `dirname -- "$0"`/..

if [[ $profile == *"68880"* ]]; then
    chip=68880
elif [[ $profile == *"6837"* ]]; then
    chip=6837
else
    echo "ERROR: wrong profile '$profile'"
    exit 1
fi

mcbs=$(eval echo \${mcbs_${chip}[@]})

echo Created following uboot environment LUNs for specific MCBs/Boards:
for mcb in $mcbs
do
	cp --no-preserve=mode bootloaders/build/configs/env_smc.conf targets/$profile/tmp_env
	echo vf_1_boot=`stat -c %s targets/$profile/bcm${profile}_bstrap.pkgtb` >> targets/$profile/tmp_env
	echo MCB=$mcb >> targets/$profile/tmp_env
	cat targets/$profile/tmp_env | bootloaders/obj/uboot/tools/mkenvimage --bootmagic -s 16384 -o targets/$profile/bcm${profile}_ubootenv_${mcb}_lun.bin
	rm targets/$profile/tmp_env
	chmod a+r targets/$profile/bcm${profile}_ubootenv_${mcb}_lun.bin
	echo "  targets/$profile/bcm${profile}_ubootenv_${mcb}_lun.bin"
	
	brds=$(eval echo \${boards_${chip}_${mcb}[@]})
	for brd in $brds
	do
		cp targets/$profile/bcm${profile}_ubootenv_${mcb}_lun.bin targets/$profile/bcm${profile}_ubootenv_${brd}_lun.bin
		echo "    targets/$profile/bcm${profile}_ubootenv_${brd}_lun.bin"
	done
done