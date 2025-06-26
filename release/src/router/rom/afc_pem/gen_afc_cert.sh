#!/bin/sh
GEN_FOLDER="/etc/afc"
UPLOAD_AFC_PATH="/jffs/.sys/afc"

AFC_CONF="afc_openssl.config"
ASUS_CONF="${GEN_FOLDER}/${AFC_CONF}"
CA_CERT="cacert.pem"
CA_KEY="cakey.pem"
ASUS_AFC_CERT="asus_afc_cacert.pem"
ASUS_AFC_CSR="asus_afc_cacert.csr"
ASUS_AFC_KEY="asus_afc_cakey.pem"
ASUS_AFC_MTLS="afc_mtls_cacert.pem"

# check afc cacert and cakey, if existed, not need to re-generate
if [ -e ${UPLOAD_AFC_PATH}/${ASUS_AFC_MTLS} ]; then
	(echo "${UPLOAD_AFC_PATH}/${ASUS_AFC_MTLS} exist" > /dev/console)
	exit 1
fi

# create path
if [ ! -e ${GEN_FOLDER} ]; then
	(echo "create ${GEN_FOLDER}" > /dev/console)
	mkdir -p ${GEN_FOLDER}
fi

if [ ! -e ${UPLOAD_AFC_PATH} ]; then
	(echo "create ${UPLOAD_AFC_PATH}" > /dev/console)
	mkdir -p ${UPLOAD_AFC_PATH} 
fi

cd ${GEN_FOLDER}
cp /rom/${AFC_CONF} ./
cp /rom/${CA_CERT} ./
cp /rom/${CA_KEY} ./

OPENSSL_CONF=${ASUS_CONF} openssl genrsa -out ${GEN_FOLDER}/${ASUS_AFC_KEY}
OPENSSL_CONF=${ASUS_CONF} openssl req -new -subj "/C=US/CN=ASUSTeK AFC Server Certificate/O=ASUSTeK Computer Inc." \
	-key ${GEN_FOLDER}/${ASUS_AFC_KEY} -out ${GEN_FOLDER}/${ASUS_AFC_CSR}
OPENSSL_CONF=${ASUS_CONF} openssl x509 -req -in ${GEN_FOLDER}/${ASUS_AFC_CSR} -CA ${CA_CERT} -CAkey ${CA_KEY} -CAcreateserial -days 7306 -sha256 -out ${GEN_FOLDER}/${ASUS_AFC_CERT}


if [ -e ${GEN_FOLDER}/${ASUS_AFC_KEY} ] && [ -e ${GEN_FOLDER}/${ASUS_AFC_CERT} ]; then
	cat ${GEN_FOLDER}/${ASUS_AFC_KEY} ${GEN_FOLDER}/${ASUS_AFC_CERT} > ${GEN_FOLDER}/${ASUS_AFC_MTLS}
	cp ${GEN_FOLDER}/${ASUS_AFC_MTLS} ${UPLOAD_AFC_PATH}/${ASUS_AFC_MTLS}
else
	(echo "${ASUS_AFC_KEY} or ${ASUS_AFC_CERT} fail to generate" > /dev/console)
fi

rm -rf ${GEN_FOLDER}
