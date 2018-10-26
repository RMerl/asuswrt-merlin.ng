#!/bin/sh

# This script creates a new target directory that will contains the target-specific security credentials 
# This script should only be run once when a new SoC type (eg. 6836) is being introduced.
# Dont forget to add the newly created Kroe2-mfg-priv.enc and mfgRoeData.sig into source control!
# The script inputs Kroe1-mfg-ek and Kroe1-mfg-iv are defined within the bootrom code file bcm63xx_encr3_clr.c

if test $# -lt 3
then
   echo "error: A minimum of three arguments is required"
   echo "usage: make_new_target.sh <target_dir eg. 6836> <Kroe1-mfg-ek .ex 12345678123456781234567812345678> <Kroe1-mfg-iv>"
   exit 1
fi

Kroe1_mfg_ek=$2 
Kroe1_mfg_iv=$3

cur_dir=`pwd`
demo_creds_dir=$cur_dir/../demo_creds
gen3_common_dir=$cur_dir/../..
target_dir=$cur_dir/../../../$1

# Create target dir if it does not exist
mkdir -p $target_dir

# Decrypt RSA key Krot-mfg
echo "Commencing the decryption of the file Krot-mfg-encrypted.pem ... \n"
openssl enc -d -aes-128-cbc -in ./Krot-mfg-encrypted.pem -out ./Krot-mfg.pem
# if [ $$? == 0 ]; then
#   echo Correct password. Decryption successful. Continuing ...
# else 
#    echo Wrong password. Decryption failed. Exiting ...
#    rm -f ./Krot-mfg.pem
#    exit 1
# fi 

# Create Kroe2-mfg-priv.enc within the target directory
cat $demo_creds_dir/Kroe2-mfg-priv-p.bin $demo_creds_dir/Kroe2-mfg-priv-q.bin $demo_creds_dir/Kroe2-mfg-priv-dmp1.bin $demo_creds_dir/Kroe2-mfg-priv-dmq1.bin $demo_creds_dir/Kroe2-mfg-priv-iqmp.bin > ./Kroe2-mfg-priv.bin
openssl enc -aes-128-cbc -in ./Kroe2-mfg-priv.bin -out $target_dir/Kroe2-mfg-priv.enc -K $Kroe1_mfg_ek -iv $Kroe1_mfg_iv
rm ./Kroe2-mfg-priv.bin

# Create a signature across the targets ver. 2 mfg RoE CoT
cat $demo_creds_dir/Kroe2-mfg-pub.bin $target_dir/Kroe2-mfg-priv.enc $gen3_common_dir/mid.bin > ./mfgRoeCot.bin
openssl dgst -sign ./Krot-mfg.pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $target_dir/mfgRoeData.sig ./mfgRoeCot.bin
rm ./mfgRoeCot.bin ./Krot-mfg.pem
