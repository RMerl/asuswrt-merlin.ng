#!/bin/bash

config_file=$1
blob_file=$2

if [[ $# != 2 ]]; then
	echo "Usage: ./gen4_demo_ksm_blob.sh <cfg> <out>"
	echo " "
	echo "    Note:"
	echo "        This script is an example and it may be required to customize if needed"
	echo " "
	echo "    Description:"
	echo "        This script generate Key Store Modification Box acording to specified Configuration"
	echo "        Should be executed from"
	echo "            SDK_root_directory/targets/keys/demo/GEN4/<SOC_FOLDER>/ksb"
	echo " "
	echo "    Example:"
	echo "        ../../scripts/gen4_demo_ksm_blob.sh ./ksm.cfg ../ksm.bin"
	echo " "
	echo "    Secured KSM Layout:"
	echo "        31                            0"
	echo "         ----------------------------- "
	echo "        | size                        |"
	echo "        | reserved[3]                 |"
	echo "        | [                           |"
	echo "        |      certificate            |"
	echo "        | ]                           |"
	echo "        |-----------------------------|"
	echo "        | ...                         |"
	echo "        | ...                         |"
	echo "        | ....                        |"
	echo "         ----------------------------- "
	echo "        | size                        |"
	echo "        | reserved[3]                 |"
	echo "        | [                           |"
	echo "        |      certificate            |"
	echo "        | ]                           |"
	echo "        |-----------------------------|"
	echo " "
	echo "    Results:"
	echo "        ksm.bin"
	echo " "
	echo "    Return Value:"
	echo "        0 - error"
	echo "        N - number of entries in the ksm.bin"
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
	exit 0
}

# $1 - file
function ks_entry_padding() {
	maxsize=$((16))
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
# $3 - file to store the result
function param_validate_and_store() {
	if [[ $1 == *=* ]]; then
		param=$(echo $1 | cut -d= -f1)
		value=$(echo $1 | cut -d= -f2)
		if [ "$param" == "$2" ]; then
			if [ "$param" == "materials" ]; then
				cat $value > $3
			else
				echo -n $value | fold -w2 | tac | tr -d "\n" | xxd -r -p > $3
			fi
		else
			clean_up
		fi
	else
		clean_up
	fi
}

# $1 - certificate file
# $2 - file to store the result
function certificate_get_size_and_store() {
	minsize=$((1328))
	filesize=$(stat -c "%s" $1)
	if (( filesize < minsize )); then
		echo "certificate file size $filesize less than expected $minsize"
		clean_up
	fi
	filesizehex=$(printf '%08x' "$filesize")
	echo -n $filesizehex | fold -w2 | tac | tr -d "\n" | xxd -r -p > $2
}

KSM_ENTRY_TEMP=__ksm_entry.tmp

entry_counter=0
while read line; do
	if [[ ${line::1} == "#" ]]; then
		continue
	fi
	if [[ $line == \[*\] ]]; then
		echo "ksmcfg: $line"
		read line && param_validate_and_store $line "materials" $KSM_ENTRY_TEMP.tmp
		echo "ksmcfg: $line"
		certificate_get_size_and_store $KSM_ENTRY_TEMP.tmp $KSM_ENTRY_TEMP
		ks_entry_padding $KSM_ENTRY_TEMP
		cat $KSM_ENTRY_TEMP >> $blob_file
		cat $KSM_ENTRY_TEMP.tmp >> $blob_file
		entry_counter=$((entry_counter+1))
	else
		clean_up
	fi
done < $config_file

clean_up

echo "done"
exit $entry_counter
