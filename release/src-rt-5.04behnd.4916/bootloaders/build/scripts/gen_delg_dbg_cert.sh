#!/bin/sh

#########################################################################################
#       Delegate debug certificate generation script                                    #
#                                                                                       #
# This script generates a signed debug certificate for key-owner to deploy to their     # 
# devices. Customers should copy and modify this script to point to their own debug     #
# certificate source, signing key and signing mechanism                                 #
#                                                                                       #
# NOTE: Script must be run from top-level build dir                                     #
#                                                                                       #
#./gen_delg_package.sh [-i source .its debug certificate] [-k signing key] <output file>#
# eg:                                                                                   #
# ./bootloaders/build/work gen_delg_dbg_cert.sh \                                       #
#                       -i bootloaders/build/work/delg_debug/debug_cert.its \           #
#                       -k targets/keys/demo/DELG/Krsa-delg-dbg.pem \                   #
#                       debug_signed.cert                                               #
#########################################################################################

##############################################################################
# Default certificate and signing key                                        #
# NOTE: Modify these to point to custom certificate source and signing key   #
#       OR pass userdefined source and key using commandline switches        #
##############################################################################
INPUT_CERT=bootloaders/build/work/delg_package/debug_cert.its            
CERT_SIGN_KEY=targets/keys/demo/DELG/Krsa-delg-dbg.pem

#!/bin/bash
while [ -n "$1" ]
do
case "$1" in
  -i) INPUT_CERT="$2"
    shift ;;
  -k) CERT_SIGN_KEY="$2"
    shift ;;
   *) OUTPUT_CERT="$1";;
esac
shift
done

if [ -z "$INPUT_CERT" ] || [ ! -f $INPUT_CERT ]
then
  echo "Error: Invalid certificate source .its file $INPUT_CERT "
  err=1;
elif [ -z "$CERT_SIGN_KEY" ] || [ ! -f $CERT_SIGN_KEY ] 
then
  echo "Error: Invalid signing key $CERT_SIGN_KEY "
  err=1;
elif [ -z "$OUTPUT_CERT" ]       
then
  echo "Error: Invalid output signed certificate name $OUTPUT_CERT"
  err=1;
fi

if [ ! -z "$err" ]       
then
  echo "Usage: ./gen_delg_dbg_cert [-i cert_src.its] [-k private_key.pem] <ouput_signed_dbg_cert>"
  exit 1;
fi

echo ""
echo "###################################################"
echo "Generating debug certificate with following inputs:"
echo "Input Certificate  : $INPUT_CERT"
echo "Signing Key        : $CERT_SIGN_KEY"
echo "Output Certificate : $OUTPUT_CERT"
echo "###################################################"

# Clean files
\rm -rf tmp_dbg_cert.itb tmp_dbg_cert.sig $OUTPUT_CERT

# Generate binary certificate
dtc -I dts -O dtb $INPUT_CERT > tmp_dbg_cert.itb

###############################################################################
# Generate signature                                                          #
# NOTE: Please add calls to appropriate scripts which can perform the signing #
#       via HSM. The object to be signed is tmp_dbg_cert.itb, the output      #
#       signature is tmp_dbg_cert.sig                                         #
###############################################################################
./bootloaders/build/scripts/sign_openssl_simple.sh tmp_dbg_cert.itb tmp_dbg_cert.sig $CERT_SIGN_KEY

# Align signature to 4byte boundary
cert_size=$(stat -c %s tmp_dbg_cert.itb)
cert_size=$(( $cert_size + 3 ))
cert_size=$(( $cert_size & (~3) )) 
sig_size=$(stat -c %s tmp_dbg_cert.sig)
signed_cert_size=$(( $cert_size + $sig_size ))

# Generate signed debug certificate
echo "Signed Debug Certificate Size: $signed_cert_size"
dd if=/dev/zero of=$OUTPUT_CERT bs=$signed_cert_size count=1
dd if=tmp_dbg_cert.itb of=$OUTPUT_CERT conv=notrunc
dd if=tmp_dbg_cert.sig of=$OUTPUT_CERT seek=$cert_size bs=1 conv=notrunc

