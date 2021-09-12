#!/bin/sh

#create key for CA self-signed certificate
certtool --generate-privkey --outfile test-ca-key.pem --rsa

#create CA self-signed certificate
certtool --generate-self-signed --load-privkey test-ca-key.pem --template test-ca-template.txt --outfile test-ca-cert.pem

# create server key
certtool --generate-privkey --outfile server-key.pem --rsa

# create server certificate
certtool --generate-certificate --load-privkey server-key.pem --template server-template.txt --outfile server-cert.pem --load-ca-certificate test-ca-cert.pem --load-ca-privkey test-ca-key.pem

# create expired server certificate
certtool --generate-certificate --load-privkey server-key.pem --template expired-template.txt --outfile expired.pem --load-ca-certificate test-ca-cert.pem --load-ca-privkey test-ca-key.pem

# create not activated server cert
certtool --generate-certificate --load-privkey server-key.pem --template invalid-template.txt --outfile invalid.pem --load-ca-certificate test-ca-cert.pem --load-ca-privkey test-ca-key.pem

# create client key
certtool --generate-privkey --outfile client-key.pem --rsa

# create client certificate
certtool --generate-certificate --load-privkey client-key.pem --template client-template.txt --outfile client-cert.pem --load-ca-certificate test-ca-cert.pem --load-ca-privkey test-ca-key.pem

# create CRL for the server certificate
certtool --generate-crl --load-ca-privkey test-ca-key.pem --load-ca-certificate test-ca-cert.pem --load-certificate server-cert.pem --outfile revoked-crl.pem --template revoked-template.txt
