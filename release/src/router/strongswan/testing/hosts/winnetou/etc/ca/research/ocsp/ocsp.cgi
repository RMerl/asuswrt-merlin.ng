#!/bin/bash

cd /etc/ca/research

echo "Content-type: application/ocsp-response"
echo ""

cat | pki --ocsp --respond --cacert researchCert.pem --index index.txt \
		  --cert ocspCert.pem --key ocspKey.pem --lifetime 5 --debug 0
