#!/bin/bash

cd /etc/openssl

echo "Content-type: application/ocsp-response"
echo ""

# simulate a delayed response
sleep 2

cat | /usr/bin/openssl ocsp -index index.txt -CA strongswanCert.pem \
	-rkey ocspKey.pem -rsigner ocspCert.pem \
	-nmin 5 \
	-reqin /dev/stdin -respout /dev/stdout | cat
