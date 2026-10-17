#!/bin/sh


################################################################
#              Delegate package generation script              #
#                                                              #
# NOTE: THIS SCRIPT IS AN EXAMPLE AND IT MUST BE CUSTOMIZED    #
#       BEFORE IT IS RUN                                       #
#                                                              #
# This script generates a delegate package. Please change      #
# config fields to customise your choice of ID, rollback,      #
# credentials and policy_template. Look for 'CUSTOMIZE' tag    #
# to see which items CAN be customized IF NEEDED. Defaults     #
# for all customizable items will ONLY work with unmodified    #
# standard SDK code.                                           #
#                                                              #
# ./gen_delg_package.sh <output folder> [gen3 version]         #
#                                                              #
# [gen3 version] is required when building delegations for     #
# devices which have been transitioned to secure boot using    #
# larger ROT/ROE keys. The version corresponds to key sizes as #
# follows:                                                     #
#    default - Supports 128 bit AES and 2048 bit RSA keys      #
#    v2      - Supports 192 bit AES and 3072 bit RSA keys      #
#    v3      - Supports 256 bit AES and 4096 bit RSA keys      #
# Gen3 version specified MUST match version deployed on chip   #
#                                                              #
# NOTE: Script must be run from top-level build dir            #
#                                                              #
# eg:                                                          #
# ./gen_delg_package.sh bootloaders/build/work/delg_package    #
# ./gen_delg_package.sh bootloaders/build/work/delg_package v2 #
# ./gen_delg_package.sh bootloaders/build/work/delg_package v3 #
#                                                              #
################################################################

if [ -z "$1" ] || [ ! -d $1 ]
then
	echo "Error: Invalid output folder: $1"
	echo "Usage: ./gen_delg_package.sh <output folder> [secure boot version]"
	exit 1;
else
	echo ""
	echo -e "######## Delegate package generation script ########\n"
fi

OUTDIR=$1
GEN3_VER=$2
################# Customize The Following ######################

# CUSTOMIZE: Legacy loader support - Legacy SDR Format 
# Set this to 0 when generating delegations for devices which have the latest loader binaries
# installed (i.e with TPLs that have been upgraded to min_tpl_compatibility of 0x3 and higher)
LEGACY_SDR_SUPPORT=1
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

# CUSTOMIZE: Enable using offline pre-encrypted delegate aes key
USE_PREENC_DELG_AES=0

# CUSTOMIZE: Enable using offline generated SDR and Security policy signatures
USE_OFFLINE_SIG=0

# CUSTOMIZE: Enable using an offline key-owner keyring containing encoded secret keys
USE_OFFLINE_ENC_KEYRING=0

# CUSTOMIZE: Enable encryption of image binaries and generation of an encryption record
USE_IMAGE_ENCRYPTION=0

# CUSTOMIZE: Regenerate image encryption keys everytime delegate package is built
USE_REGEN_IMG_ENC_KEYS=1

# CUSTOMIZE: Unique delegation ID
DELEGATE_ID=12345

# CUSTOMIZE: This delegation is valid on all devices where DEVICE_ANTIROLLBACK_LVL <= ROLLBACK
ROLLBACK=3

# CUSTOMIZE: Path to credentials
KEY_PATH=targets/keys/demo
DELG_KEY_PATH=targets/keys/demo/DELG

# Set GEN3 version based parameters
if [ "$GEN3_VER" == "v3" ]; then
	ROE_AES_LEN=256
	ROT_RSA_LEN=4096
elif [ "$GEN3_VER" == "v2" ]; then
	ROE_AES_LEN=192
	ROT_RSA_LEN=3072
else
	GEN3_VER=
	ROE_AES_LEN=128
	ROT_RSA_LEN=2048
fi

# CUSTOMIZE: Key-owner's ROE Encryption key
ROE_AES_EK=$KEY_PATH/GEN3/${GEN3_VER}/Kroe-fld-ek.bin
ROE_AES_IV=$KEY_PATH/GEN3/${GEN3_VER}/Kroe-fld-iv.bin

# CUSTOMIZE: Key-owner's ROT Signing Key 
ROT_RSA_PRV=$KEY_PATH/GEN3/${GEN3_VER}/Krot-fld.pem

# CUSTOMIZE: Delegate's RSA key pair (Private key in .pem and a binary public key modulus )
DELG_RSA_PUB_MOD_BIN=$DELG_KEY_PATH/Krsa-delg-pub.bin
DELG_RSA_PRV=$DELG_KEY_PATH/Krsa-delg.pem

# CUSTOMIZE: Optional Key-owner generated delegation debug public key modulus
DBG_RSA_PUB_MOD_BIN=$DELG_KEY_PATH/Krsa-delg-dbg-pub.bin

# CUSTOMIZE: Delegate's plain text and encrypted AES key 
DELG_AES_EK=$DELG_KEY_PATH/Kaes-delg-ek.bin
DELG_AES_IV=$DELG_KEY_PATH/Kaes-delg-iv.bin
DELG_ENC_AES=$DELG_KEY_PATH/Kaes-delg.enc${GEN3_VER}

# CUSTOMIZE: Key-owner's Security Policy 
DELG_SEC_POL=bootloaders/build/configs/sec_policy_template.its

# CUSTOMIZE: Key-owner's ROE Encoded Keyring options. 
# If USE_OFFLINE_ENC_KEYRING is 0, then the keyring is automatically generated during 
# script invocation by encrypting DELG_SEC_SECRET_KEYS with ROE
# If USE_OFFLINE_ENC_KEYRING is 1, then a pre-compiled keyring MUST be available
# and populated with keys that were encrypted via ROE
if [ $USE_OFFLINE_ENC_KEYRING -eq 0 ] ; then
	# CUSTOMIZE: Add key-owner's plain-text keys which need to be encoded in key ring
	# Keys are specified as follows:
	# 
	#    <KEY_IDENT>:<KEY_LOCATION>
	#
	# KEY_IDENT: %PLACEHOLDER% for key data in the security policy
	# KEY_LOCATION: Location of plain-text key binary
	DELG_ENCKEY_SECRET_KEYS="ENC_KEY1:${OUTDIR}/enc_key1_src.bin ENC_KEY2:${OUTDIR}/enc_key2_src.bin"

	# Set default key ring
	DELG_KEY_RING=${OUTDIR}/enc_keyring.txt
	DELG_DFLT_SECRET_KEY_SIZE="32"

	# Check if any demo keys need to be generated
	for file in $DELG_ENCKEY_SECRET_KEYS; do
		key=${file%%:*}
		key_file=${file##*:}
		key_filename=`basename $key_file`

		# Check if demo keys need to be generated
		if [ ! -f $key_file ]; then
			echo "Generating demo secret key $key: ${OUTDIR}/${key_filename}"
			dd if=/dev/random of=${OUTDIR}/${key_filename} count=$DELG_DFLT_SECRET_KEY_SIZE bs=1 status=none
		fi
	
	done
else
	# CUSTOMIZE: Specify pre-encoded key ring
	# Each line in keyring has an encoded key listed as <KEY_NAME>:<ENCODED KEY VALUE>
	# The ENCODED KEY VALUE must be obtained by encrypting the key using ROE. For each
	# KEY_NAME listed in the Keyring, there should be a corresponding 'data = [%KEY_NAME%]'
	# entry under a 'Keyx' node under the 'encoded_keys' section of the security policy 
	DELG_KEY_RING=${OUTDIR}/enc_keyring.txt
fi

# CUSTOMIZE: If encrypting binaries specify file location and encryption keys
if [ $USE_IMAGE_ENCRYPTION -eq 1 ]; then
	# CUSTOMIZE: Specify binaries to be encrypted in the form:
	#
	#    <img_name>:<img_bin> <img_name>:<img_bin> ...
	#
	# img_name = [atf|optee|uboot|kernel]
	# img_bin  = location of binary to be encrypted. Path relative to top-level build dir
	DELG_IMG_ENC_BINS="atf:bootloaders/obj/atf/armtf.bin optee:bootloaders/obj/optee/tee.bin"
	
	# CUSTOMIZE: Specify the predefined key-owner image encryption key.
	# The encoded encryption key must exist under the 'encoded_keys' section
	# in the security policy with a matching %placeholder% and 'key_name' as 
	# follows:
	#
	# DELG_KEY_IMG_INC                : %placeholder% for key data in security policy
	# DELG_KEY_IMG_ENC_KEYNAME_SUFFIX : suffix to be appended to <img_name> to form 
	#                                   the 'key_name' for the encoded encryption key
	# DELG_KEY_IMG_ENC_AES_LEN        : AES bit length of the key 
	# DELG_KEY_IMG_ENC_SRC            : Location of key binary
	#
	# The key must have an IV of 16 bytes and an EK of 128/192/256 bits.
	# If the key is not specified then a key is generated with default values. 
	DELG_KEY_IMG_ENC="ENC_KEY_IMG_ENC"
	DELG_KEY_IMG_ENC_KEYNAME_SUFFIX="_enc"
	DELG_KEY_IMG_ENC_AES_LEN=128
	DELG_KEY_IMG_ENC_SRC=${OUTDIR}/Kaes-delg-img-enc.bin

	# Calculate expected key size
	DELG_KEY_IMG_ENC_AES_EK_SIZE_CALC=`expr $DELG_KEY_IMG_ENC_AES_LEN / 8`
	DELG_KEY_IMG_ENC_AES_IV_SIZE_CALC=16
	DELG_KEY_IMG_ENC_AES_SIZE_CALC=`expr $DELG_KEY_IMG_ENC_AES_EK_SIZE_CALC + $DELG_KEY_IMG_ENC_AES_IV_SIZE_CALC`

	# If using predefined key, check if specified key exists
	if [ $USE_REGEN_IMG_ENC_KEYS -eq 0 ] ; then
		if [ -f $DELG_KEY_IMG_ENC_SRC ] ; then
			# If using predefined keys, confirm if keysize is correct
			DELG_KEY_IMG_ENC_AES_SIZE=`stat -c%s $DELG_KEY_IMG_ENC_SRC`
			if [ ! $DELG_KEY_IMG_ENC_AES_SIZE -eq $DELG_KEY_IMG_ENC_AES_SIZE_CALC ] ; then
				echo "ERROR: Invalid key size for AES key: $DELG_KEY_IMG_ENC_SRC"
				echo "ERROR: Expected sizes: key:$DELG_KEY_IMG_ENC_AES_SIZE_CALC"
				echo "ERROR: Actual   sizes: key:$DELG_KEY_IMG_ENC_AES_SIZE" 
				exit;
			fi
			echo "Using key-owner's pre-defined image encryption key: $DELG_KEY_IMG_ENC_SRC"
		else
			echo "WARNING: key-owner's pre-defined image encryption key doesnt exist! Generating random key!"
			USE_REGEN_IMG_ENC_KEYS=1 
		fi
	fi

	# If using NEW generated keys each time, delete older keys and regen
	if [ $USE_REGEN_IMG_ENC_KEYS -eq 1 ] ; then
		if [ $USE_OFFLINE_ENC_KEYRING -eq 1 ]; then
			echo "ERROR: Invalid configuration!"
			echo "ERROR: Cannot use offline encoded keyring if generating new image encryption keys!" 
			exit;
		fi

		# Regenerate encryption key
		\rm -rf $DELG_KEY_IMG_ENC_SRC;
		echo "Generating key-owner's image encryption key: $DELG_KEY_IMG_ENC_SRC"
		dd if=/dev/random of=$DELG_KEY_IMG_ENC_SRC count=$DELG_KEY_IMG_ENC_AES_SIZE_CALC bs=1 status=none
	fi

	# Generate individual aes ek & iv binaries
	DELG_KEY_IMG_ENC_AES_EK=${OUTDIR}/Kaes-delg-img-enc-ek.bin
	DELG_KEY_IMG_ENC_AES_IV=${OUTDIR}/Kaes-delg-img-enc-iv.bin
	\rm -rf $DELG_KEY_IMG_ENC_AES_EK
	\rm -rf $DELG_KEY_IMG_ENC_AES_IV
	dd if=$DELG_KEY_IMG_ENC_SRC of=$DELG_KEY_IMG_ENC_AES_EK bs=1 count=$DELG_KEY_IMG_ENC_AES_EK_SIZE_CALC status=none
	dd if=$DELG_KEY_IMG_ENC_SRC of=$DELG_KEY_IMG_ENC_AES_IV bs=1  skip=$DELG_KEY_IMG_ENC_AES_EK_SIZE_CALC count=$DELG_KEY_IMG_ENC_AES_IV_SIZE_CALC status=none

	# Add encryption key to list of secret keys that need to be encoded
	if [ $USE_OFFLINE_ENC_KEYRING -eq 0 ]; then
		DELG_ENCKEY_SECRET_KEYS="${DELG_ENCKEY_SECRET_KEYS} ${DELG_KEY_IMG_ENC}:${DELG_KEY_IMG_ENC_SRC}"
	fi
fi

###############################################################
# Output file names
SEC_POL_ITB=${DELEGATE_ID}_sec_policy.itb
SEC_POL_ITS=${DELEGATE_ID}_sec_policy.its
SDR_BIN=${DELEGATE_ID}_sec_sdr.bin
SEC_POL_ITB_SIG=${DELEGATE_ID}_sec_policy.itb.sig${GEN3_VER}
SDR_BIN_SIG=${DELEGATE_ID}_sec_sdr.bin.sig${GEN3_VER}
SEC_IMG_ENC_REC=${DELEGATE_ID}_sec_enc_bin.rec

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
\rm -rf ${OUTDIR}/${SEC_IMG_ENC_REC}

### Encrypt delegate's aes key with ROE ###
if [ $USE_PREENC_DELG_AES -eq 1 ]; then
	echo "Using offline pre-encrypted delegate aes key: $DELG_ENC_AES"
	if [ ! -f $DELG_ENC_AES ]; then
		echo "ERROR: Missing offline pre-encrypted delegate aes key: $DELG_ENC_AES"
		exit;
	fi
else
	echo "Generating encrypted delegate aes key: $DELG_ENC_AES"
	\rm -rf $DELG_ENC_AES ${DELG_ENC_AES}_tmp 
	cat $DELG_AES_EK $DELG_AES_IV > ${DELG_ENC_AES}_tmp
	./bootloaders/build/scripts/encrypt_openssl_simple.sh -l $ROE_AES_LEN ${DELG_ENC_AES}_tmp "$ROE_AES_EK $ROE_AES_IV" > $DELG_ENC_AES
	\rm -rf ${DELG_ENC_AES}_tmp
fi

### Generate encoded keyring ###
# Encrypt keys using ROE and insert encoded keys into keyring
if [ -n $DELG_KEY_RING ]; then
	if [ $USE_OFFLINE_ENC_KEYRING -eq 0 ]; then
		echo "Generating encoded keyring: $DELG_KEY_RING"

		# Generate empty keyring 
		\rm -rf $DELG_KEY_RING; echo -n > $DELG_KEY_RING

		# Process secret keys
		for file in $DELG_ENCKEY_SECRET_KEYS; do
			key=${file%%:*}
			key_file=${file##*:}
			key_size=`stat -c%s $key_file`
			key_filename=`basename $key_file`

			# Encode secret keys 
			\rm -rf ${OUTDIR}/${key_filename}.enc${GEN3_VER}
			echo "	Encoding key: $file"
			./bootloaders/build/scripts/encrypt_openssl_simple.sh -l $ROE_AES_LEN $key_file "$ROE_AES_EK $ROE_AES_IV" > ${OUTDIR}/${key_filename}.enc${GEN3_VER}
			key_hex=`hexdump -v -e '1/1 "%02X"' ${OUTDIR}/${key_filename}.enc${GEN3_VER}`

			# Add encoded keys to keyring
			echo "$key:${key_hex}" >> $DELG_KEY_RING
			echo "${key}_SIZE:${key_size}" >> $DELG_KEY_RING
		done
	else
		echo "Using offline generated encoded key ring: $DELG_KEY_RING"
		if [ ! -f $DELG_KEY_RING ]; then
			echo "ERROR: Missing offline generated encoded key ring: $DELG_KEY_RING"
			exit;
		fi
	fi
	KEY_RING_OPT="--keyring=${DELG_KEY_RING}"
fi

### Encrypt binaries and generate encryption record ###
if [ $USE_IMAGE_ENCRYPTION -eq 1 ]; then
	echo "Generating encryption record: ${OUTDIR}/${SEC_IMG_ENC_REC}"
	echo "# Each line in the encryption record contains information about an encrypted binary." >> ${OUTDIR}/${SEC_IMG_ENC_REC}
	echo "# Each entry is encoded as follows:" >> ${OUTDIR}/${SEC_IMG_ENC_REC}
	echo "#     fit_image_name:aes_length:unencrypted_size:encrypted_file:encryption_key" >> ${OUTDIR}/${SEC_IMG_ENC_REC}
	for file in $DELG_IMG_ENC_BINS; do
		echo "	Encrypting binary: $file"
		img=${file%%:*}
		img_file=${file##*:}
		img_size=`stat -c%s $img_file`
		img_filename=`basename $img_file`
		./bootloaders/build/scripts/encrypt_openssl_simple.sh -l $DELG_KEY_IMG_ENC_AES_LEN $img_file "$DELG_KEY_IMG_ENC_AES_EK $DELG_KEY_IMG_ENC_AES_IV" > ${OUTDIR}/enc_${img_filename}
		echo "$img:aes${DELG_KEY_IMG_ENC_AES_LEN}:$img_size:enc_${img_filename}:${img}${DELG_KEY_IMG_ENC_KEYNAME_SUFFIX}" >> ${OUTDIR}/${SEC_IMG_ENC_REC}
	done
fi

### Generate SDR and Security Policy ITB ###
echo "Generating Security Policy ITB: ${OUTDIR}/${SEC_POL_ITB}"
echo "Generating SDR: ${OUTDIR}/${SDR_BIN}"
./bootloaders/build/work/gen_sec_delg_obj --delg_id=$DELEGATE_ID --rollback=$ROLLBACK --secits=$DELG_SEC_POL --kencaesdelg=$DELG_ENC_AES --kdelgaesek=$DELG_AES_EK --kroeaeslen=$ROE_AES_LEN --krsadelgpub=$DELG_RSA_PUB_MOD_BIN --krsadbgpub=$DBG_RSA_PUB_MOD_BIN --out_path=$OUTDIR $LEG_SDR_OPT $KEY_RING_OPT

### Sign SDR and Security Policy ITB with ROT ###
if [ $USE_OFFLINE_SIG -eq 1 ]; then
	echo "WARNING! Incompleted delegate package, offline signing enabled."
	echo "  Must externally sign SDR and Security Policy ITB using ${ROT_RSA_PRV}"
	echo "  Use an external tool to sign ${OUTDIR}/${SDR_BIN} and copy the signature to ${OUTDIR}/${SDR_BIN_SIG}"
	echo "  Use an external tool to sign ${OUTDIR}/${SEC_POL_ITB} and copy the signature to ${OUTDIR}/${SEC_POL_ITB_SIG}"
else
	echo "Signing Security Policy ITB:"
	./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SEC_POL_ITB} ${OUTDIR}/${SEC_POL_ITB_SIG}  ${ROT_RSA_PRV}
	echo "Signing SDR:"
	./bootloaders/build/scripts/sign_openssl_simple.sh ${OUTDIR}/${SDR_BIN} ${OUTDIR}/${SDR_BIN_SIG}  ${ROT_RSA_PRV}
	echo -e "\n######## New complete delegate package generated in: $OUTDIR ########\n\n"
fi

# Finally, copy over all delegate credentials 
cp --no-preserve=mode $DELG_RSA_PRV $OUTDIR
cp --no-preserve=mode $DELG_RSA_PUB_MOD_BIN $OUTDIR
cp --no-preserve=mode $DELG_AES_EK $OUTDIR
cp --no-preserve=mode $DELG_AES_IV $OUTDIR
cp --no-preserve=mode $DELG_ENC_AES $OUTDIR

