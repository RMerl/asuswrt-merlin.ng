#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

# simulate a delayed response
sleep 2

cat | pki --ocsp --respond --cacert strongswanCert.pem --index index.txt \
		  --cert ocspCert.pem --key ocspKey.pem --lifetime 5 --debug 0
