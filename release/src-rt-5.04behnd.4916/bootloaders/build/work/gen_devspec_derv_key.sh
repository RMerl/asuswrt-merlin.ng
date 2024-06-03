#!/bin/sh

# Generates a derived key from the device specific key, using uboot's secure export key derivation mechanism
# Usage:
# gen_devspec_derv_key.sh <binary device specific key> 0x<32-bit salt|0 if no salt> <length:32|64> <output derived key name>"

ip_key_binary=$1
salt32=$2
saltval=$(echo $(($2)))
length=$3
op_key_binary=$4

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]; then
	echo "Error: Invalid Arguments!"
	echo "Usage:"
	echo "	gen_devspec_derv_key.sh <binary device specific key> 0x<32-bit salt|0 if no salt> <length:32|64> <output derived key name>"
	echo ""
	echo "Example:"
	echo "	./gen_devspec_derv_key.sh device_specific_key.bin 0x00112233 32 derived_key.bin"
	echo ""
	exit 1
fi

salted_key1=$(mktemp)
salted_key2=$(mktemp)
derv_key1=$(mktemp)
derv_key2=$(mktemp)
be_salt=$(mktemp)
salt_bin=$(mktemp)

#echo $salted_key1 $salted_key2 $derv_key1 $derv_key2 $be_salt $salt_bin $saltval

rm -f ./${op_key_binary} 

if [ $saltval -ne 0 ]; then
	echo ${salt32} | xxd -r -p - > $be_salt ; hexdump -v -e '1/4 "%08x"' -e '"\n"' $be_salt | xxd -r -p > $salt_bin
fi

cat ${salt_bin} ${ip_key_binary} > $salted_key1
openssl dgst -sha256 -binary $salted_key1 > $derv_key1
cat $derv_key1 > ./${op_key_binary}

if [ $length == "64" ]; then 
	\rm -f ./${op_key_binary}
	cat ${ip_key_binary} ${salt_bin} > $salted_key2
	openssl dgst -sha256 -binary $salted_key2 > $derv_key2
	cat $derv_key1 $derv_key2 > ./${op_key_binary}
fi	

echo ""
echo "Device specific key ${ip_key_binary}:"
echo "-------------------------------------"
xxd -g1 ${ip_key_binary}
echo ""
if [ $saltval -ne 0 ]; then
	echo "Salt:"
	echo "-----"
	echo "$salt32"
	echo ""
fi
echo "Derived key ${op_key_binary}:"
echo "-----------------------------"
xxd -g1 ${op_key_binary}
echo ""
echo "Uboot Key programming string (8 32-bit words, little endian):"
echo "-------------------------------------------------------------"
echo -n "bca_test dev_spec_key set " ; hexdump -v -e' "0x%08x "' -e '" "' $ip_key_binary ; echo ;
echo ""
echo "Security Trust Fit entry:"
echo "-------------------------"
echo "export_itemX {                                                                 "
echo "        item_sec_id = \"SEC_ITEM_KEY_DEV_SPECIFIC\";                           "
echo "        item_name = \"key_name\";                                              "
if [ $saltval -ne 0 ]; then 
	echo "        salt = $salt32;                                                "
fi
echo "        algo = \"sha256\";                                                     "
echo "        length = <$length>;                                                    "
echo ""
