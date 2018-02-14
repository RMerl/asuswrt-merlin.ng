#!/bin/sh
SECS=1262278080

cd /etc
mkdir cfg_mnt
cd cfg_mnt

NVCN=`nvram get https_crt_cn`
if [ "$NVCN" == "" ]; then
	NVCN=`nvram get lan_ipaddr`
fi

cp -L /etc/openssl.cnf openssl.config

I=0
for CN in $NVCN; do
        echo "$I.commonName=CN" >> openssl.config
        echo "$I.commonName_value=$CN" >> openssl.config
        I=$(($I + 1))
done

# create the key and certificate request
OPENSSL_CONF=/etc/cfg_mnt/openssl.config openssl req -new -out /etc/cfg_mnt/cert.csr -keyout /etc/cfg_mnt/privkey.pem -newkey rsa:2048 -passout pass:password
# remove the passphrase from the key
OPENSSL_CONF=/etc/cfg_mnt/openssl.cnf openssl rsa -in /etc/cfg_mnt/privkey.pem -out key.pem -passin pass:password
# convert the certificate request into a signed certificate
OPENSSL_CONF=/etc/cfg_mnt/openssl.cnf RANDFILE=/dev/urandom openssl x509 -in /etc/cfg_mnt/cert.csr -out cert.pem -req -signkey key.pem -days 3653 -sha256

openssl rsa -in key.pem -outform PEM -pubout -out pubkey.pem

rm -f /etc/cfg_mnt/cert.csr /etc/cfg_mnt/privkey.pem openssl.config
