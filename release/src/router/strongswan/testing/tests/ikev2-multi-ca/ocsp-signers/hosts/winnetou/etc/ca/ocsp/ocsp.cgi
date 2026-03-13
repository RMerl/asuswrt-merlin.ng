#!/bin/bash

cd /etc/ca

echo "Content-type: application/ocsp-response"
echo ""

cat | pki --ocsp --respond \
		  --cacert strongswanCert.pem --index index.txt \
		  --cert ocspCert.pem --key ocspKey.pem \
		  --cacert research/researchCert.pem --index research/index.txt \
		  --cert research/ocspCert.pem --key research/ocspKey.pem \
		  --cacert sales/salesCert.pem --index sales/index.txt \
		  --cert sales/ocspCert.pem --key sales/ocspKey.pem \
		  --lifetime 5 --debug 0
