#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

cat | pki --ocsp --respond --cacert strongswanCert.pem --index index.txt \
		  --cert ocspCert-self.pem --key ocspKey-self.pem --lifetime 5 --debug 0
