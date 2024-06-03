#!/bin/sh
# usage: encrypt_openssl_simple.sh <key_len - optional default 128 > <image to encrypt> <path to key and iv separated by space>  >  <encrypted_file> 
#echo "Running $0"
AES_LEN=128
set -- `getopt l: $*`
for i in $*
do
	case $i in
		-l) AES_LEN=$2; shift; shift;;
		--) shift; break;;
	esac
done
IMG=$1
KEY="$2 $3"
 
if [ -z "$KEY" ] || [ -z "$IMG" ]
then
	echo "ERROR: - missing arguments in $0"
	exit 1
fi
#echo "Got - im: $IMG ek: $KEY len: $AES_LEN"
IV=`echo $KEY|sed -r "s/^ *([^ ]*) +([^ ]*)/xxd -ps -c 256 \2/g"`
EK=`echo $KEY|sed -r "s/^ *([^ ]*) +([^ ]*)/xxd -ps -c 256 \1/g"`
#echo "$EK $IV"
#echo "-- openssl enc -aes-${AES_LEN}-cbc -K `$EK` -iv `$IV`"
cat $IMG|openssl enc -aes-${AES_LEN}-cbc -K `$EK` -iv `$IV`
# Verify this
#echo "cat $1.enc|openssl enc -d -aes-128-cbc -K `$EK` -iv `$IV` > $1.dec" 
#cat $1.enc|openssl enc -d -aes-128-cbc -K `$EK` -iv `$IV` > $1.dec 
#cmp -s $1 $1.dec
#[ ! $? -eq 0 ] && exit 1

