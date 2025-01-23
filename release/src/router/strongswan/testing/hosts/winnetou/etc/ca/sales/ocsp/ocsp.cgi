#!/bin/bash

cd /etc/ca/sales

echo "Content-type: application/ocsp-response"
echo ""

cat | pki --ocsp --respond --cacert salesCert.pem --index index.txt \
		  --cert ocspCert.pem --key ocspKey.pem --lifetime 5 --debug 0
