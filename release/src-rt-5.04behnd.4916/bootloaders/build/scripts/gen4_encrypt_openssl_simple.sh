#!/bin/bash

#
# Usage:
#   ./gen4_encrypt_openssl_simple.sh <key_map> <options: -aes-128-cbc|-aes-256-cbc [-salt]> <ek_name_hint> <iv_name_hint> <file_in> <file_out>
#
# Description:
#	This a reference Bash shell script, designed to obtain from user input for various parameters required to perform AES encryption using OpenSSL.
#	User should provide:
#		1. Path to Key Map Configuration File (Key Name Hint to Key Material )
#		2. AES encryption options, for example: -aes-128-cbc or -aes-256-cbc and optionaly -salt
#		3. Key Name Hint for the encryption key (ek) and initialization vector (iv),
#			These hints used to retrieve the ek and iv from a secure location or to provide additional context to the user about the key being used.
#		4. Path to the file that the user wishes to encrypt.
#		5. Path to the file where the resulting encrypted data should be stored.
#

#for arg in "$@"; do
#	echo "Argument: $arg"
#done

source $1
aes_options=$2
ek_hint=$3
iv_hint=$4
file_in=$5
file_out=$6

if [[ $# != 6 ]]; then
	exit -1
fi

if [ -f "$file_out" ]; then
	rm $file_out
fi

ek_key_file="undefined"
for hint in "${!KEYS_MAP_AES_EK[@]}"; do
  if [ "$hint" == "$ek_hint" ]; then
  	ek_key_file="${KEYS_MAP_AES_EK[$hint]}"
	break
  fi
done

iv_key_file="undefined"
for hint in "${!KEYS_MAP_AES_IV[@]}"; do
  if [ "$hint" == "$iv_hint" ]; then
  	iv_key_file="${KEYS_MAP_AES_IV[$hint]}"
	break
  fi
done

if [ "$iv_key_file" == "undefined" ] || [ "$ek_key_file" == "undefined" ]; then
	exit -1
fi

echo "gen4 encryptor - fin:$file_in, fout:$file_out, ek:$ek_key_file, iv:$iv_key_file, options:$aes_options"
openssl enc $aes_options -in $file_in -out $file_out -K $(cat $ek_key_file) -iv $(cat $iv_key_file) -e

if [ $? -ne 0 ]; then
	exit -1
fi

exit 0
