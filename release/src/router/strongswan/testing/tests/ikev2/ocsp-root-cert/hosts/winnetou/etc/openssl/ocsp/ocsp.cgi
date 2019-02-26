#!/bin/bash

cd /etc/openssl

echo "Content-type: application/ocsp-response"
echo ""

cat | /usr/bin/openssl ocsp -index index.txt -CA strongswanCert.pem \
	-rkey strongswanKey.pem -rsigner strongswanCert.pem \
	-resp_no_certs -nmin 5 \
	-reqin /dev/stdin -respout /dev/stdout | cat
