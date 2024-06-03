#!/bin/bash

#
# Usage: 
#   ./gen4_sign_openssl_simple.sh <key_map> <key_name_hint> <file_to_sign> <file_to_store_signature>
#
# Description:
#	This a reference Bash shell script, designed to obtain from user input for the parameters required to perform RSA signing using OpenSSL.
#	User should provide:
#		1. Path to Key Map Configuration File (Key Name Hint to Key Material )
#		2. RSA Key Name Hint
#			This hint could be utilized to retrieve the RSA private key from a secure location or to provide additional context to the user about the key being used.
#		3. Path to the file that the user intends to sign.
#		4. Path to the file where the resulting digital signature should be stored after the signing operation.
#

#for arg in "$@"; do
#	echo "Argument: $arg"
#done

source $1
signing_key_hint=$2
file_to_sign=$3
file_to_store_signature=$4

if [[ $# != 4 ]]; then
	exit -1
fi

if [ -f "$file_to_store_signature" ]; then
	rm $file_to_store_signature
fi

key_file="undefined"
for hint in "${!KEYS_MAP_RSA[@]}"; do
  if [ "$hint" == "$signing_key_hint" ]; then
  	key_file="${KEYS_MAP_RSA[$hint]}"
	break
  fi
done

if [ "$key_file" == "undefined" ]; then
	exit -1
fi

openssl rsa -in $key_file -noout -text | grep 2048

if [ $? -ne 0 ]; then
	sha="sha512"
else
	sha="sha256"
fi

echo "gen4 signer - fin:$file_to_sign, fout:$file_to_store_signature, k:$key_file, sha:$sha"
openssl dgst -$sha -sign $key_file -keyform pem -sigopt rsa_padding_mode:pkcs1 -out $file_to_store_signature $file_to_sign

if [ $? -ne 0 ]; then
	exit -1
fi

exit 0
