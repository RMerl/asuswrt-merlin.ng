#!/bin/sh
# usage: encrypt_openssl_simple.sh <image to encrypt> <path to key and iv separated by space>  >  <encrypted_file> 
EK=`echo $2|sed -r "s/^ *([^ ]*) +([^ ]*)/xxd -ps \1/g"`
IV=`echo $2|sed -r "s/^ *([^ ]*) +([^ ]*)/xxd -ps \2/g"`
#echo "cat $1|openssl enc -aes-128-cbc -K `$EK` -iv `$IV`" 
cat $1|openssl enc -aes-128-cbc -K `$EK` -iv `$IV`
# Verify this
#echo "cat $1.enc|openssl enc -d -aes-128-cbc -K `$EK` -iv `$IV` > $1.dec" 
#cat $1.enc|openssl enc -d -aes-128-cbc -K `$EK` -iv `$IV` > $1.dec 
#cmp -s $1 $1.dec
#[ ! $? -eq 0 ] && exit 1

