#!/bin/bash

cd /etc/openssl

echo "Content-type: application/ocsp-response"
echo ""

cat | /usr/bin/openssl ocsp -index index.txt -CA strongswanCert.pem \
	-rkey winnetouKey.pem -rsigner winnetouCert.pem \
	-nmin 5 \
	-reqin /dev/stdin -respout /dev/stdout | cat
