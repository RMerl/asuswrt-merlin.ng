#!/bin/bash

config_file=$1
blob_file=$2

if [[ $# != 2 ]]; then
	echo "Usage: ./gen4_demo_ksb_blob.sh <cfg> <out>"
	echo " "
	echo "    Example:"
	echo "        ./gen4_demo_ksb_blob.sh ./ksb.cfg ../ksb.bin"
	echo " "
	echo "    Key Store Blob Layout:"
	echo "        31                            0"
	echo "         ----------------------------- "
	echo "        | id                          |"
	echo "        | paired_id                   |"
	echo "        | type                        |"
	echo "        | properties                  |"
	echo "        | permissions                 |"
	echo "        | secret                      |"
	echo "        | reserved[2]                 |"
	echo "        | [                           |"
	echo "        |      key's materials        |"
	echo "        | ]                           |"
	echo "        | padding up to 544B          |"
	echo "        |-----------------------------|"
	echo "        | ...                         |"
	echo "        | ...                         |"
	echo "        | ....                        |"
	echo "        |-----------------------------|"
	echo "        | id                          |"
	echo "        | paired_id                   |"
	echo "        | type                        |"
	echo "        | properties                  |"
	echo "        | permissions                 |"
	echo "        | secret                      |"
	echo "        | reserved[2]                 |"
	echo "        | [                           |"
	echo "        |      key's materials        |"
	echo "        | ]                           |"
	echo "        | padding up to 544B          |"
	echo "         ----------------------------- "
	echo " "
	echo "    Results:"
	echo "        ksb.bin"
	echo " "
	echo "    PAY ATTENTION !!!"
	echo "         This utility does not check key's validity and usage."
	echo " "
	exit
fi

if [ -f "$blob_file" ]; then
	rm $blob_file
fi

function clean_up() {
	rm *.tmp*
	exit
}

declare -A KEYS_TYPE
KEYS_TYPE["TYPE_AES_128"]="BIT0"
KEYS_TYPE["TYPE_AES_256"]="BIT1"
KEYS_TYPE["TYPE_RSA_2048"]="BIT2"
KEYS_TYPE["TYPE_RSA_4096"]="BIT3"

declare -A KEYS_PROP
KEYS_PROP["PROP_OTP"]="BIT0"
KEYS_PROP["PROP_SKS"]="BIT1"
KEYS_PROP["PROP_ENCRYPT"]="BIT2"
KEYS_PROP["PROP_DECRYPT"]="BIT3"
KEYS_PROP["PROP_SIGN"]="BIT4"
KEYS_PROP["PROP_VERIFY"]="BIT5"

declare -A KEYS_PERM
KEYS_PERM["PERM_DESTROYABLE"]="BIT0"
KEYS_PERM["PERM_AUTOEXPORTABLE"]="BIT1"
KEYS_PERM["PERM_EXPORTABLE"]="BIT2"
KEYS_PERM["PERM_UPDATEABLE"]="BIT3"
KEYS_PERM["PERM_ROOT"]="BIT4"
KEYS_PERM["PERM_UNSEC_WORLD"]="BIT5"
KEYS_PERM["PERM_TRUSTED_WORLD"]="BIT6"

# $1 - string of settings separated by '|'
# $2 - name of map
function get_bitmask() {
	bitmask_by_settings=$1
	map_name=$2
	bitmask=0

	IFS='|' read -ra settings <<< "$bitmask_by_settings"
	for setting in "${settings[@]}"; do
		bit="undefined"
		if [ "$map_name" == "KEYS_TYPE" ]; then
			for setting_in_map in "${!KEYS_TYPE[@]}"; do
				if [ "$setting_in_map" == "$setting" ]; then
					bit="${KEYS_TYPE[$setting_in_map]}"
					break
				fi
			done
		elif [ "$map_name" == "KEYS_PROP" ]; then
			for setting_in_map in "${!KEYS_PROP[@]}"; do
				if [ "$setting_in_map" == "$setting" ]; then
					bit="${KEYS_PROP[$setting_in_map]}"
					break
				fi
			done
		elif [ "$map_name" == "KEYS_PERM" ]; then
			for setting_in_map in "${!KEYS_PERM[@]}"; do
				if [ "$setting_in_map" == "$setting" ]; then
					bit="${KEYS_PERM[$setting_in_map]}"
					break
				fi
			done
		fi
		if [ "$bit" == "undefined" ]; then
			echo "undefined setting $setting"
			clean_up
		fi
		bit_number=$(echo "$bit" | sed 's/BIT//')
		bit_mask=$((1 << bit_number))
		bitmask=$((bitmask | bit_mask))
	done
	printf "0x%08x\n" "$bitmask"
}

# $1 - file
function ks_entry_padding() {
	maxsize=$((544))
	filesize=$(stat -c "%s" $1)
	if (( filesize > maxsize )); then
		echo "ks entry size $filesize exceed limit $maxsize"
		clean_up
	fi
	padcount=$((maxsize - filesize))
	dd if=/dev/zero ibs=1 count="$padcount" >> $1
	if [ $? -ne 0 ]; then
		clean_up
	fi
}

# $1 - parameter line
# $2 - expected param
# $3 - ks file
function param_validate_and_store() {
	if [[ $1 == *=* ]]; then
		param=$(echo $1 | cut -d= -f1)
		value=$(echo $1 | cut -d= -f2)
		if [ "$param" == "$2" ]; then
			if [ "$param" == "materials" ]; then
				cat $value >> $3
			else
				if [ "$param" == "id" ]; then
					value="0x$(echo -n "$value" | xxd -ps -c 8 | tr -d '\n')"
				elif [ "$param" == "type" ]; then
					value=$(get_bitmask $value "KEYS_TYPE")
				elif [ "$param" == "properties" ]; then
					value=$(get_bitmask $value "KEYS_PROP")
				elif [ "$param" == "permissions" ]; then
					value=$(get_bitmask $value "KEYS_PERM")
				fi
				echo -n $value | fold -w2 | tac | tr -d "\n" | xxd -r -p >> $3
			fi
		else
			clean_up
		fi
	else
		clean_up
	fi
}

KS_ENTRY_TEMP=__ks_entry.tmp

while read line; do
	if [[ ${line::1} == "#" ]]; then
		continue
	fi
	if [[ $line == \[*\] ]]; then
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "id" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "paired_id" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "type" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "properties" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "permissions" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "secret" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "reserved" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "reserved" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		read line && param_validate_and_store $line "materials" $KS_ENTRY_TEMP
		echo "ksbcfg: $line"
		ks_entry_padding $KS_ENTRY_TEMP
		cat $KS_ENTRY_TEMP >> $blob_file
		rm $KS_ENTRY_TEMP
	else
		clean_up
	fi
done < $config_file

echo "done"
