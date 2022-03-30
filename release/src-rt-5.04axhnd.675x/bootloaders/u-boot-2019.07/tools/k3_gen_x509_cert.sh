#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Script to add K3 specific x509 cetificate to a binary.
#

# Variables
OUTPUT=tiboot3.bin
TEMP_X509=x509-temp.cert
CERT=certificate.bin
RAND_KEY=eckey.pem
LOADADDR=0x41c00000
BOOTCORE_OPTS=0
BOOTCORE=16

gen_degen_template() {
cat << 'EOF' > degen-template.txt

asn1=SEQUENCE:rsa_key

[rsa_key]
version=INTEGER:0
modulus=INTEGER:0xDEGEN_MODULUS
pubExp=INTEGER:1
privExp=INTEGER:1
p=INTEGER:0xDEGEN_P
q=INTEGER:0xDEGEN_Q
e1=INTEGER:1
e2=INTEGER:1
coeff=INTEGER:0xDEGEN_COEFF
EOF
}

# Generate x509 Template
gen_template() {
cat << 'EOF' > x509-template.txt
 [ req ]
 distinguished_name     = req_distinguished_name
 x509_extensions        = v3_ca
 prompt                 = no
 dirstring_type         = nobmp

 [ req_distinguished_name ]
 C                      = US
 ST                     = TX
 L                      = Dallas
 O                      = Texas Instruments Incorporated
 OU                     = Processors
 CN                     = TI support
 emailAddress           = support@ti.com

 [ v3_ca ]
 basicConstraints = CA:true
 1.3.6.1.4.1.294.1.1 = ASN1:SEQUENCE:boot_seq
 1.3.6.1.4.1.294.1.2 = ASN1:SEQUENCE:image_integrity
 1.3.6.1.4.1.294.1.3 = ASN1:SEQUENCE:swrv
# 1.3.6.1.4.1.294.1.4 = ASN1:SEQUENCE:encryption
 1.3.6.1.4.1.294.1.8 = ASN1:SEQUENCE:debug

 [ boot_seq ]
 certType = INTEGER:TEST_CERT_TYPE
 bootCore = INTEGER:TEST_BOOT_CORE
 bootCoreOpts = INTEGER:TEST_BOOT_CORE_OPTS
 destAddr = FORMAT:HEX,OCT:TEST_BOOT_ADDR
 imageSize = INTEGER:TEST_IMAGE_LENGTH

 [ image_integrity ]
 shaType = OID:2.16.840.1.101.3.4.2.3
 shaValue = FORMAT:HEX,OCT:TEST_IMAGE_SHA_VAL

 [ swrv ]
 swrv = INTEGER:0

# [ encryption ]
# initalVector = FORMAT:HEX,OCT:TEST_IMAGE_ENC_IV
# randomString = FORMAT:HEX,OCT:TEST_IMAGE_ENC_RS
# iterationCnt = INTEGER:TEST_IMAGE_KEY_DERIVE_INDEX
# salt = FORMAT:HEX,OCT:TEST_IMAGE_KEY_DERIVE_SALT

 [ debug ]
 debugUID = FORMAT:HEX,OCT:0000000000000000000000000000000000000000000000000000000000000000
 debugType = INTEGER:4
 coreDbgEn = INTEGER:0
 coreDbgSecEn = INTEGER:0
EOF
}

parse_key() {
	sed '/\ \ \ \ /s/://g' key.txt | awk  '!/\ \ \ \ / {printf("\n%s\n", $0)}; /\ \ \ \ / {printf("%s", $0)}' | sed 's/\ \ \ \ //g' | awk "/$1:/{getline; print}"
}

gen_degen_key() {
# Generate a 4096 bit RSA Key
	openssl genrsa -out key.pem 1024 >>/dev/null 2>&1
	openssl rsa -in key.pem -text -out key.txt >>/dev/null 2>&1
	DEGEN_MODULUS=$( parse_key 'modulus' )
	DEGEN_P=$( parse_key 'prime1' )
	DEGEN_Q=$( parse_key 'prime2' )
	DEGEN_COEFF=$( parse_key 'coefficient' )
	gen_degen_template

	sed -e "s/DEGEN_MODULUS/$DEGEN_MODULUS/"\
		-e "s/DEGEN_P/$DEGEN_P/" \
		-e "s/DEGEN_Q/$DEGEN_Q/" \
		-e "s/DEGEN_COEFF/$DEGEN_COEFF/" \
		 degen-template.txt > degenerateKey.txt

	openssl asn1parse -genconf degenerateKey.txt -out degenerateKey.der >>/dev/null 2>&1
	openssl rsa -in degenerateKey.der -inform DER -outform PEM -out $RAND_KEY >>/dev/null 2>&1
	KEY=$RAND_KEY
	rm key.pem key.txt degen-template.txt degenerateKey.txt degenerateKey.der
}

declare -A options_help
usage() {
	if [ -n "$*" ]; then
		echo "ERROR: $*"
	fi
	echo -n "Usage: $0 "
	for option in "${!options_help[@]}"
	do
		arg=`echo ${options_help[$option]}|cut -d ':' -f1`
		if [ -n "$arg" ]; then
			arg=" $arg"
		fi
		echo -n "[-$option$arg] "
	done
	echo
	echo -e "\nWhere:"
	for option in "${!options_help[@]}"
	do
		arg=`echo ${options_help[$option]}|cut -d ':' -f1`
		txt=`echo ${options_help[$option]}|cut -d ':' -f2`
		tb="\t\t\t"
		if [ -n "$arg" ]; then
			arg=" $arg"
			tb="\t"
		fi
		echo -e "   -$option$arg:$tb$txt"
	done
	echo
	echo "Examples of usage:-"
	echo "# Example of signing the SYSFW binary with rsa degenerate key"
	echo "    $0 -c 0 -b ti-sci-firmware-am6x.bin -o sysfw.bin -l 0x40000"
	echo "# Example of signing the SPL binary with rsa degenerate key"
	echo "    $0 -c 16 -b spl/u-boot-spl.bin -o tiboot3.bin -l 0x41c00000"
}

options_help[b]="bin_file:Bin file that needs to be signed"
options_help[k]="key_file:file with key inside it. If not provided script generates a rsa degenerate key."
options_help[o]="output_file:Name of the final output file. default to $OUTPUT"
options_help[c]="core_id:target core id on which the image would be running. Default to $BOOTCORE"
options_help[l]="loadaddr: Target load address of the binary in hex. Default to $LOADADDR"

while getopts "b:k:o:c:l:h" opt
do
	case $opt in
	b)
		BIN=$OPTARG
	;;
	k)
		KEY=$OPTARG
	;;
	o)
		OUTPUT=$OPTARG
	;;
	l)
		LOADADDR=$OPTARG
	;;
	c)
		BOOTCORE=$OPTARG
	;;
	h)
		usage
		exit 0
	;;
	\?)
		usage "Invalid Option '-$OPTARG'"
		exit 1
	;;
	:)
		usage "Option '-$OPTARG' Needs an argument."
		exit 1
	;;
	esac
done

if [ "$#" -eq 0 ]; then
	usage "Arguments missing"
	exit 1
fi

if [ -z "$BIN" ]; then
	usage "Bin file missing in arguments"
	exit 1
fi

# Generate rsa degenerate key if user doesn't provide a key
if [ -z "$KEY" ]; then
	gen_degen_key
fi

if [ $BOOTCORE == 0 ]; then	# BOOTCORE M3, loaded by ROM
	CERTTYPE=2
elif [ $BOOTCORE == 16 ]; then	# BOOTCORE R5, loaded by ROM
	CERTTYPE=1
else				# Non BOOTCORE, loaded by SYSFW
	BOOTCORE_OPTS_VER=$(printf "%01x" 1)
	# Add input args option for SET and CLR flags.
	BOOTCORE_OPTS_SETFLAG=$(printf "%08x" 0)
	BOOTCORE_OPTS_CLRFLAG=$(printf "%08x" 0x100) # Clear FLAG_ARMV8_AARCH32
	BOOTCORE_OPTS="0x$BOOTCORE_OPTS_VER$BOOTCORE_OPTS_SETFLAG$BOOTCORE_OPTS_CLRFLAG"
	# Set the cert type to zero.
	# We are not using public/private key store now
	CERTTYPE=$(printf "0x%08x" 0)
fi

SHA_VAL=`openssl dgst -sha512 -hex $BIN | sed -e "s/^.*= //g"`
BIN_SIZE=`cat $BIN | wc -c`
ADDR=`printf "%08x" $LOADADDR`

gen_cert() {
	#echo "Certificate being generated :"
	#echo "	LOADADDR = 0x$ADDR"
	#echo "	IMAGE_SIZE = $BIN_SIZE"
	#echo "	CERT_TYPE = $CERTTYPE"
	sed -e "s/TEST_IMAGE_LENGTH/$BIN_SIZE/"	\
		-e "s/TEST_IMAGE_SHA_VAL/$SHA_VAL/" \
		-e "s/TEST_CERT_TYPE/$CERTTYPE/" \
		-e "s/TEST_BOOT_CORE_OPTS/$BOOTCORE_OPTS/" \
		-e "s/TEST_BOOT_CORE/$BOOTCORE/" \
		-e "s/TEST_BOOT_ADDR/$ADDR/" x509-template.txt > $TEMP_X509
	openssl req -new -x509 -key $KEY -nodes -outform DER -out $CERT -config $TEMP_X509 -sha512
}

gen_template
gen_cert
cat $CERT $BIN > $OUTPUT

# Remove all intermediate files
rm $TEMP_X509 $CERT x509-template.txt
if [ "$KEY" == "$RAND_KEY" ]; then
	rm $RAND_KEY
fi
