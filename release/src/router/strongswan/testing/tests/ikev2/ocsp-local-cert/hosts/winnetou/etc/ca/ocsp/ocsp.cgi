#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

cat | /usr/bin/openssl ocsp -index index.txt -CA strongswanCert.pem \
	-rkey ocspKey-self.pem -rsigner ocspCert-self.pem \
	-resp_no_certs -nmin 5 \
	-reqin /dev/stdin -respout /dev/stdout | cat
