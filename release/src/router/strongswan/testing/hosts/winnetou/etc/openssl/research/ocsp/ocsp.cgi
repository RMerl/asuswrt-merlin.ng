#!/bin/bash

cd /etc/openssl/research

echo "Content-type: application/ocsp-response"
echo ""

cat | /usr/bin/openssl ocsp -index index.txt -CA researchCert.pem \
	-rkey ocspKey.pem -rsigner ocspCert.pem \
	-nmin 5 \
	-reqin /dev/stdin -respout /dev/stdout | cat
