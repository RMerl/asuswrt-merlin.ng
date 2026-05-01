#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

cat | pki --ocsp --respond --cacert strongswanCert.pem --index index.txt \
		  --key strongswanKey.pem --lifetime 5 --debug 0
