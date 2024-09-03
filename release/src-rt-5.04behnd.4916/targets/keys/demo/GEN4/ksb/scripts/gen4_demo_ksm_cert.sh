#!/bin/bash

cert_epoch=$1
cert_action=$2
cert_approver=$3
cert_approver_key=$4
cert_wrapper=$5
cert_wrapper_ek=$6
cert_wrapper_iv=$7
cert_cfg=$8
cert_blobster=$9

if [[ $# != 9 ]]; then
	echo "Usage: ./gen4_demo_ksm_cert.sh <epoch> <action: add|remove|generate> <approver key name hint> <approver key> <wrapper key name hint> <wrapper ek> <wrapper iv> <cfg> <blobster>"
	echo " "
	echo "    Note:"
	echo "        This script is an example and it may be required to customize if needed"
	echo " "
	echo "    Description:"
	echo "        This script generate Key Store Modification Certificate acording to specified Configuration"
	echo "        Should be executed from"
	echo "            SDK_root_directory/targets/keys/demo/GEN4/<SOC_FOLDER>/ksb"
	echo " "
	echo "    Example:"
	echo "        ../../scripts/gen4_demo_ksm_cert.sh 0001 add r2k2 ./r2k2.pem a1k1 ./a1k1-ek.hex ./a1k1-iv.hex ./key.cfg ../../scripts/gen4_demo_ksb_blob.sh"
	echo " "
	echo "    Certificate Layout:"
	echo "         ---------------------------------- "
	echo "        | certificate:>>KSM-CERT<<         |"
	echo "        | param:value'\n'                  |"
	echo "        | param:value'\n'                  |"
	echo "        | ...                              |"
	echo "        | param:value'\n'                  |"
	echo "        | ...                              |"
	echo "        | padding                          |"
	echo "        | room size 512B (null terminated) |"
	echo "        |----------------------------------|"
	echo "        | Subject:                         |"
	echo "        | Crypto Materials Entrie(s)       |"
	echo "        | ...                              |"
	echo "        | no padding                       |"
	echo "        | room size upto 544B X 8 entries  |"
	echo "        |----------------------------------|"
	echo "        | Signature:                       |"
	echo "        | RSA2048 over SHA256              |"
	echo "        |  or                              |"
	echo "        | RSA4096 over SHA512              |"
	echo "        | ...                              |"
	echo "        | no padding                       |"
	echo "        | room size 256B for RSA2048       |"
	echo "        | room size 512B for RSA4096       |"
	echo "         ---------------------------------- "
	echo " "
	echo "    Results:"
	echo "        ./certificate-ksm-sec-0001.bin"
	echo " "
	echo "            where X"
	echo "                  1 - certificate for single crypto material entry"
	echo "                  N - certificate for X crypto material entries"
	echo " "
	echo "    PAY ATTENTION !!!"
	echo "         This utility does not check key's validity and usage."
	echo " "
	exit
fi

cert_magic=">>KSM-CERT<<"
cert_version="1.1"
cert_date=$(date)
cert_id="0000"
cert_description="Broadcom Demo Key Store Modification Certificate"
cert_file="./__certificate-ksm.tmp"

function clean_up() {
	rm *.tmp*
	exit
}

# Prepare Parameters List
echo "certificate:$cert_magic" > $cert_file
echo "version:$cert_version" >> $cert_file
echo "id:$cert_epoch" >> $cert_file
echo "description:$cert_description" >> $cert_file
echo "type:ksm" >> $cert_file
echo "cfg:" >> $cert_cfg
echo "date:$cert_date" >> $cert_file
echo "epoch:$cert_epoch" >> $cert_file
echo "approver:$cert_approver" >> $cert_file
echo "wrapper:$cert_wrapper" >> $cert_file
echo "action:$cert_action" >> $cert_file

maxsize=$((512))
filesize=$(stat -c "%s" $cert_file)
if (( filesize > maxsize )); then
	echo "certificate header size $filesize exceed limit $maxsize"
	clean_up
fi
padcount=$((maxsize - filesize))
dd if=/dev/zero ibs=1 count="$padcount" >> $cert_file
if [ $? -ne 0 ]; then
	clean_up
fi

# Prepare Crypto Material BLOB
$cert_blobster $cert_cfg ./__crypto_material.tmp
entry_counter=$?
if [ $entry_counter == "0" ]; then
	clean_up
fi

# Encrypt Crypto Material BLOB
filesize=$(stat -c "%s" $cert_wrapper_ek)
if (( filesize == 32 )); then
	algo="-aes-128-cbc"
else
	algo="-aes-256-cbc"
fi

openssl enc $algo -salt -in ./__crypto_material.tmp -out ./__crypto_material.tmp.enc -K $(cat $cert_wrapper_ek) -iv $(cat $cert_wrapper_iv) -e
if [ $? -ne 0 ]; then
	clean_up
fi

# Attach Crypto Material BLOB to Certificate
cat ./__crypto_material.tmp.enc >> $cert_file

# Sign the Certificate
openssl rsa -in $cert_approver_key -noout -text | grep 2048
if [ $? -ne 0 ]; then
	sha="sha512"
else
	sha="sha256"
fi

openssl dgst -$sha -sign $cert_approver_key -keyform pem -sigopt rsa_padding_mode:pkcs1 -out ./__signature.tmp $cert_file
if [ $? -ne 0 ]; then
	clean_up
fi

# Attach Signature to Certificate
cat ./__signature.tmp >> $cert_file

# Certificate
mv $cert_file ./certificate-ksm-sec-$cert_epoch.bin

echo "done"
clean_up
