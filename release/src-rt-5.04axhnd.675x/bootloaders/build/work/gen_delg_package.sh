#!/bin/sh


################################################################
#              Delegate package generation script              #
#                                                              #
# This script generates a delegate package. Please change      #
# config fields to customise your choice of ID, rollback,      #
# credentials and policy_template.                             #
#                                                              #
# NOTE: Script must be run from top-level build dir            #
#                                                              #
# ./gen_delg_package.sh <output folder>                        #
#                                                              #
# eg:                                                          #
# ./gen_delg_package.sh bootloaders/build/work/delg_package    #
#                                                              #
################################################################

if [ -z "$1" ] || [ ! -d $1 ]       
then
  echo "Error: Invalid output folder: $1"
  echo "Usage: ./gen_delg_package.sh <output folder>"
  exit 1;
fi

################ Customize The Following ######################
DELEGATE_ID=12345
ROLLBACK=2
KEY_PATH=targets/keys/demo
# Key-owner's ROE Encryption key
ROE_AES_EK=$KEY_PATH/GEN3/Kroe-fld-ek.bin
ROE_AES_IV=$KEY_PATH/GEN3/Kroe-fld-iv.bin

# Key-owner's ROT Signing Key 
ROT_RSA_PRV=$KEY_PATH/GEN3/Krot-fld.pem

# Delegate's RSA key pair (Private key in .pem and a binary public key modulus )
DELG_RSA_PUB_MOD_BIN=$KEY_PATH/DELG/Krsa-delg-pub.bin
DELG_RSA_PRV=$KEY_PATH/DELG/Krsa-delg.pem

# Delegate's plain text and encrypted AES key 
DELG_AES_EK=$KEY_PATH/DELG/Kaes-delg-ek.bin
DELG_AES_IV=$KEY_PATH/DELG/Kaes-delg-iv.bin
DELG_ENC_AES=$KEY_PATH/DELG/Kaes-delg.enc

# Key-owner's Security Policy 
DELG_SEC_POL=bootloaders/build/configs/sec_policy_template.its
###############################################################

# Output file names
SEC_POL_ITB=${DELEGATE_ID}_sec_policy.itb
SEC_POL_ITS=${DELEGATE_ID}_sec_policy.its
SDR_BIN=${DELEGATE_ID}_sec_sdr.bin
SEC_POL_ITB_SIG=${DELEGATE_ID}_sec_policy.itb.sig
SDR_BIN_SIG=${DELEGATE_ID}_sec_sdr.bin.sig

# Clear output directory
OUTDIR=$1
\rm -rf ${OUTDIR}/${DELG_RSA_PRV##*/}
\rm -rf ${OUTDIR}/${DELG_AES_EK##*/} 
\rm -rf ${OUTDIR}/${DELG_AES_IV##*/} 
\rm -rf ${OUTDIR}/${SEC_POL_ITS}
\rm -rf ${OUTDIR}/${SEC_POL_ITB}
\rm -rf ${OUTDIR}/${SDR_BIN}
\rm -rf ${OUTDIR}/${SEC_POL_ITB_SIG}
\rm -rf ${OUTDIR}/${SDR_BIN_SIG}

# Copy over delegate's credentials 
cp --no-preserve=mode $DELG_RSA_PRV $OUTDIR
cp --no-preserve=mode $DELG_RSA_PUB_MOD_BIN $OUTDIR
cp --no-preserve=mode $DELG_AES_EK $OUTDIR
cp --no-preserve=mode $DELG_AES_IV $OUTDIR
cp --no-preserve=mode $DELG_ENC_AES $OUTDIR

### Comment this section out if encryption is done offline ###
# Encrypt delegate's aes key with ROE 
#\rm -rf $DELG_ENC_AES ${DELG_ENC_AES}_tmp 
#ROE_AES_EK_HEX=`hexdump -v -e '1/1 "%02X"' $ROE_AES_EK`
#ROE_AES_IV_HEX=`hexdump -v -e '1/1 "%02X"' $ROE_AES_IV`
#cat $DELG_AES_EK $DELG_AES_IV > ${DELG_ENC_AES}_tmp
#cat ${DELG_ENC_AES}_tmp |openssl enc -aes-128-cbc -K $ROE_AES_EK_HEX -iv $ROE_AES_IV_HEX -out $DELG_ENC_AES
#\rm -rf ${DELG_ENC_AES}_tmp

# Generate SDR and Security Policy ITB
./bootloaders/build/work/gen_sec_delg_obj --delg_id=$DELEGATE_ID --rollback=$ROLLBACK --secits=$DELG_SEC_POL --kencaesdelg=$DELG_ENC_AES --krsadelgpub=$DELG_RSA_PUB_MOD_BIN --out_path=$OUTDIR

### Comment this section out if signing is done offline ###
# Sign SDR and Security Policy ITB with ROT
echo "Signing delegate's SDR and Security Policy ITB"
./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SEC_POL_ITB} ${OUTDIR}/${SEC_POL_ITB_SIG}  ${ROT_RSA_PRV}
./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SDR_BIN} ${OUTDIR}/${SDR_BIN_SIG}  ${ROT_RSA_PRV}

