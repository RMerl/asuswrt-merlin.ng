#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

# we have to use OpenSSL here as pki --ocsp rejects signing with such a
# non-OCSP-signer certificate
cat | /usr/bin/openssl ocsp -index index.txt -CA strongswanCert.pem \
	-rkey winnetouKey.pem -rsigner winnetouCert.pem \
	-nmin 5 -reqin /dev/stdin -respout /dev/stdout | cat
