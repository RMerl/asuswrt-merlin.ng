#!/bin/sh


################################################################
#              Delegate package generation script              #
#                                                              #
# NOTE: THIS SCRIPT IS AN EXAMPLE AND IT MUST BE CUSTOMIZED    #
#       BEFORE IT IS RUN                                       #
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

OUTDIR=$1
################ Customize The Following ######################

# Legacy loader support - Legacy SDR Format 
# Set this to 0 when generating delegations for devices  which have the latest loader binaries
# installed (i.e with TPLs that have been upgraded to min_tpl_compatibility of 0x3 and higher)
LEGACY_SDR_SUPPORT=1

# Unique delegation ID
DELEGATE_ID=12345

# This delegation is valid on all devices where DEVICE_ANTIROLLBACK_LVL <= ROLLBACK
ROLLBACK=3

KEY_PATH=targets/keys/demo
# Key-owner's ROE Encryption key
ROE_AES_EK=$KEY_PATH/GEN3/Kroe-fld-ek.bin
ROE_AES_IV=$KEY_PATH/GEN3/Kroe-fld-iv.bin

# Key-owner's ROT Signing Key 
ROT_RSA_PRV=$KEY_PATH/GEN3/Krot-fld.pem

# Delegate's RSA key pair (Private key in .pem and a binary public key modulus )
DELG_RSA_PUB_MOD_BIN=$KEY_PATH/DELG/Krsa-delg-pub.bin
DBG_RSA_PUB_MOD_BIN=$KEY_PATH/DELG/Krsa-delg-dbg-pub.bin
DELG_RSA_PRV=$KEY_PATH/DELG/Krsa-delg.pem

# Delegate's plain text and encrypted AES key 
DELG_AES_EK=$KEY_PATH/DELG/Kaes-delg-ek.bin
DELG_AES_IV=$KEY_PATH/DELG/Kaes-delg-iv.bin
DELG_ENC_AES=$KEY_PATH/DELG/Kaes-delg.enc

# Key-owner's Security Policy 
DELG_SEC_POL=bootloaders/build/configs/sec_policy_template.its

# ROE Encoded Keyring. 
# Each line in keyring has an encoded key listed as <KEY_NAME>:<ENCODED KEY VALUE>
# For each KEY_NAME listed, there should be a corresponding 'data = [%KEY_NAME%]'
# entry under a 'Keyx' node under the 'encoded_keys' section of the security policy
# Keyring can be predefined, or can be generated during script invocation
DELG_KEY_RING=${OUTDIR}/enc_keyring.txt

# Controls whether keyring is generated. 
# Enable to force keyring generation on every invocation
FORCE_KEY_RING_GEN=0

# If not using precompiled encoded keyring, generate the keyring
if [ -n $DELG_KEY_RING ] && [ ! -f $DELG_KEY_RING ]; then
	# Enable keyring generation
	FORCE_KEY_RING_GEN=1
fi

# If generating keyring, specify keys
if [ -n $DELG_KEY_RING ] && [ $FORCE_KEY_RING_GEN -eq 1 ]; then
	# Specify keys to be encoded
	DELG_KEY1_NAME="ENC_KEY1"
	DELG_KEY1_SRC=${OUTDIR}/enc_key1_src.bin
	DELG_KEY2_NAME="ENC_KEY2"
	DELG_KEY2_SRC=${OUTDIR}/enc_key2_src.bin

	# Generate keys. 
	# Comment this section out if using pregenerated secret keys
	if [ ! -f $DELG_KEY1_SRC ]; then
		dd if=/dev/random of=$DELG_KEY1_SRC count=32 bs=1 status=none
	fi
	if [ ! -f $DELG_KEY2_SRC ]; then
		dd if=/dev/random of=$DELG_KEY2_SRC count=32 bs=1 status=none
	fi
fi
###############################################################

# Output file names
SEC_POL_ITB=${DELEGATE_ID}_sec_policy.itb
SEC_POL_ITS=${DELEGATE_ID}_sec_policy.its
SDR_BIN=${DELEGATE_ID}_sec_sdr.bin
SEC_POL_ITB_SIG=${DELEGATE_ID}_sec_policy.itb.sig
SDR_BIN_SIG=${DELEGATE_ID}_sec_sdr.bin.sig

# Clear output directory
\rm -rf ${OUTDIR}/${DELG_ENC_AES##*/}
\rm -rf ${OUTDIR}/${DELG_RSA_PUB_MOD_BIN##*/}
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

### Encrypt delegate's aes key with ROE ###
# Comment this section out if encryption is done offline
#\rm -rf $DELG_ENC_AES ${DELG_ENC_AES}_tmp 
#ROE_AES_EK_HEX=`hexdump -v -e '1/1 "%02X"' $ROE_AES_EK`
#ROE_AES_IV_HEX=`hexdump -v -e '1/1 "%02X"' $ROE_AES_IV`
#cat $DELG_AES_EK $DELG_AES_IV > ${DELG_ENC_AES}_tmp
#cat ${DELG_ENC_AES}_tmp |openssl enc -aes-128-cbc -K $ROE_AES_EK_HEX -iv $ROE_AES_IV_HEX -out $DELG_ENC_AES
#\rm -rf ${DELG_ENC_AES}_tmp

### Legacy SDR support check ###
# Comment this section out if legacy SDR support is not needed
if [ $LEGACY_SDR_SUPPORT -eq 1 ]; then
	echo "WARNING - This delegation is built with Legacy SDR format to work with TPLs older than August 2022."
	echo "          That skips some security protections while maintaining compatibility with old LOADER images"
	LEG_SDR_OPT=--legacy_sdr
	echo "Do you want to continue? YES/[NO]"
	read legacy_prompt
	if [ "$legacy_prompt" != "YES" ]; then
		exit;
	fi
else
	echo "WARNING - This delegation is built with NON-Legacy SDR format. It has current improvements"
	echo "          but will not be compatible with LOADERS that use TPL older then August 2022."
fi

### Generate encoded keyring ###
# Encrypt keys using ROE and insert encoded keys into keyring
if [ -n $DELG_KEY_RING ]; then
	if [ $FORCE_KEY_RING_GEN -eq 1 ]; then
		echo "Generating encoded keyring"

		# Generate empty keyring 
		\rm -rf $DELG_KEY_RING; echo -n > $DELG_KEY_RING

		# Encode secret keys 
		# Comment this out if using pre-encoded secret keys
		./bootloaders/build/scripts/encrypt_openssl_simple.sh $DELG_KEY1_SRC "$ROE_AES_EK $ROE_AES_IV" > ${DELG_KEY1_SRC}.enc
		DELG_ENCKEY1_HEX=`hexdump -v -e '1/1 "%02X"' ${DELG_KEY1_SRC}.enc`
		./bootloaders/build/scripts/encrypt_openssl_simple.sh $DELG_KEY2_SRC "$ROE_AES_EK $ROE_AES_IV" > ${DELG_KEY2_SRC}.enc
		DELG_ENCKEY2_HEX=`hexdump -v -e '1/1 "%02X"' ${DELG_KEY2_SRC}.enc`

		# Add encoded keys to keyring
		echo "$DELG_KEY1_NAME:${DELG_ENCKEY1_HEX}" >> $DELG_KEY_RING
		echo "$DELG_KEY2_NAME:${DELG_ENCKEY2_HEX}" >> $DELG_KEY_RING
	fi
	KEY_RING_OPT="--keyring=${DELG_KEY_RING}"
fi

### Generate SDR and Security Policy ITB ###
./bootloaders/build/work/gen_sec_delg_obj --delg_id=$DELEGATE_ID --rollback=$ROLLBACK --secits=$DELG_SEC_POL --kencaesdelg=$DELG_ENC_AES --krsadelgpub=$DELG_RSA_PUB_MOD_BIN --krsadbgpub=$DBG_RSA_PUB_MOD_BIN --out_path=$OUTDIR $LEG_SDR_OPT $KEY_RING_OPT

### Sign SDR and Security Policy ITB with ROT ###
# Comment this section out if signing is done offline ###
echo "Signing delegate's SDR and Security Policy ITB"
./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SEC_POL_ITB} ${OUTDIR}/${SEC_POL_ITB_SIG}  ${ROT_RSA_PRV}
./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SDR_BIN} ${OUTDIR}/${SDR_BIN_SIG}  ${ROT_RSA_PRV}
