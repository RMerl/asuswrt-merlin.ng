#!/bin/sh

OPENSSL=openssl

mkdir -p test-ca/newcerts

echo
echo "---[ Update server certificates ]---------------------------------------"
echo

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server.w1.fi/" |
	sed "s/#@ALTNAME@/subjectAltName=DNS:server.w1.fi/" \
	> openssl.cnf.tmp
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server.csr -out server.pem -extensions ext_server

$OPENSSL pkcs12 -export -out server.pkcs12 -in server.pem -inkey server.key -passout pass:
$OPENSSL pkcs12 -export -out server-extra.pkcs12 -in server.pem -inkey server.key -descert -certfile user.pem -passout pass:whatever -name server

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server3.w1.fi/" \
	> openssl.cnf.tmp
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server-no-dnsname.csr -out server-no-dnsname.pem -extensions ext_server

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server5.w1.fi/" \
	> openssl.cnf.tmp
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server-eku-client.csr -out server-eku-client.pem -extensions ext_client

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server6.w1.fi/" \
	> openssl.cnf.tmp
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server-eku-client-server.csr -out server-eku-client-server.pem -extensions ext_client_server

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server-policies.w1.fi/" |
	sed "s/#@ALTNAME@/subjectAltName=DNS:server-policies.w1.fi/" |
	sed "s/#@CERTPOL@/certificatePolicies = 1.3.6.1.4.1.40808.1.3.1/" \
	> openssl.cnf.tmp
#$OPENSSL req -config openssl.cnf.tmp -batch -new -newkey rsa:3072 -nodes -keyout server-certpol.key -out server-certpol.csr -outform PEM -sha256
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server-certpol.csr -out server-certpol.pem -extensions ext_server

cat openssl2.cnf |
	sed "s/#@CN@/commonName_default = server-policies2.w1.fi/" |
	sed "s/#@ALTNAME@/subjectAltName=DNS:server-policies2.w1.fi/" |
	sed "s/#@CERTPOL@/certificatePolicies = 1.3.6.1.4.1.40808.1.3.2/" \
	> openssl.cnf.tmp
#$OPENSSL req -config openssl.cnf.tmp -batch -new -newkey rsa:3072 -nodes -keyout server-certpol2.key -out server-certpol2.csr -outform PEM -sha256
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in server-certpol2.csr -out server-certpol2.pem -extensions ext_server

echo
echo "---[ Update user certificates ]-----------------------------------------"
echo

cat openssl2.cnf | sed "s/#@CN@/commonName_default = User/" > openssl.cnf.tmp
$OPENSSL ca -config $PWD/openssl.cnf.tmp -batch -in user.csr -out user.pem -extensions ext_client
rm openssl.cnf.tmp

$OPENSSL pkcs12 -export -out user.pkcs12 -in user.pem -inkey user.key -descert -passout pass:whatever
$OPENSSL pkcs12 -export -out user2.pkcs12 -in user.pem -inkey user.key -descert -name Test -certfile server.pem -passout pass:whatever
$OPENSSL pkcs12 -export -out user3.pkcs12 -in user.pem -inkey user.key -descert -name "my certificates" -certfile ca.pem -passout pass:whatever

echo
echo "---[ Update OCSP ]------------------------------------------------------"
echo

$OPENSSL ocsp -CAfile test-ca/cacert.pem -issuer test-ca/cacert.pem -cert server.pem -reqout ocsp-req.der -no_nonce
$OPENSSL ocsp -index test-ca/index.txt -rsigner test-ca/cacert.pem -rkey test-ca/private/cakey.pem -CA test-ca/cacert.pem -resp_no_certs -reqin ocsp-req.der -respout ocsp-server-cache.der
SIZ=`ls -l ocsp-server-cache.der | cut -f5 -d' '`
(echo -n 000; echo "obase=16;$SIZ" | bc) | xxd -r -ps > ocsp-multi-server-cache.der
cat ocsp-server-cache.der >> ocsp-multi-server-cache.der

echo
echo "---[ Additional steps ]-------------------------------------------------"
echo

echo "test_ap_eap.py: ap_wpa2_eap_ttls_server_cert_hash srv_cert_hash"

$OPENSSL x509 -in server.pem -out server.der -outform DER
HASH=`sha256sum server.der | cut -f1 -d' '`
rm server.der
sed -i "s/srv_cert_hash =.*/srv_cert_hash = \"$HASH\"/" ../test_ap_eap.py

echo "index.txt: server time+serial"

grep -v CN=server.w1.fi index.txt > index.txt.new
grep CN=server.w1.fi test-ca/index.txt | tail -1 >> index.txt.new
mv index.txt.new index.txt

echo "start.sh: openssl ocsp -reqout serial"

SERIAL=`grep CN=server.w1.fi test-ca/index.txt | tail -1 | cut -f4`
sed -i "s/serial 0x[^ ]* -no_nonce/serial 0x$SERIAL -no_nonce/" ../start.sh
