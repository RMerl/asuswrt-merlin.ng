#!/bin/sh

if [ -z "$1" ]; then
	echo "Usage: $0 KEY"
	exit 1
fi

TOOLS_DIR=$(dirname $0)

TEMP_X509=$(mktemp XXXXXXXX.temp)

REVISION=${2:-0}
SHA_VAL=$(openssl dgst -sha512 -hex tispl.bin | sed -e "s/^.*= //g")
BIN_SIZE=$(stat -c %s tispl.bin)

cat <<EOF >$TEMP_X509
[ req ]
distinguished_name     = req_distinguished_name
x509_extensions        = v3_ca
prompt                 = no
dirstring_type         = nobmp

[ req_distinguished_name ]
CN                     = IOT2050 Firmware Signature

[ v3_ca ]
basicConstraints       = CA:true
1.3.6.1.4.1.294.1.3    = ASN1:SEQUENCE:swrv
1.3.6.1.4.1.294.1.34   = ASN1:SEQUENCE:sysfw_image_integrity

[ swrv ]
swrv = INTEGER:$REVISION

[ sysfw_image_integrity ]
shaType                = OID:2.16.840.1.101.3.4.2.3
shaValue               = FORMAT:HEX,OCT:$SHA_VAL
imageSize              = INTEGER:$BIN_SIZE
EOF

CERT_X509=$(mktemp XXXXXXXX.crt)

openssl req -new -x509 -key $1 -nodes -outform DER -out $CERT_X509 -config $TEMP_X509 -sha512
cat $CERT_X509 tispl.bin > tispl.bin_signed
$TOOLS_DIR/binman/binman replace -i flash-pg1.bin -f tispl.bin_signed fit@180000
$TOOLS_DIR/binman/binman replace -i flash-pg2.bin -f tispl.bin_signed fit@180000

rm $TEMP_X509 $CERT_X509

$TOOLS_DIR/binman/binman sign -i flash-pg1.bin -k $1 -a sha256,rsa4096 fit@380000
$TOOLS_DIR/binman/binman sign -i flash-pg2.bin -k $1 -a sha256,rsa4096 fit@380000
