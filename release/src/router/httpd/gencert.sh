#!/bin/sh
SECS=1262278080

cd /etc

OPENSSL=/usr/sbin/openssl


WAITTIMER=0
while [ -f "/var/run/gencert.pid" -a $WAITTIMER -lt 14 ]
do
	WAITTIMER=$((WAITTIMER+2))
	sleep $WAITTIMER
done
touch /var/run/gencert.pid

OPENSSL_CONF="/etc/openssl.config"


add_san() {
echo $1
echo $OPENSSL_CONF
	sed -i "/\[alt_names\]/a$1" $OPENSSL_CONF
}


cp -L /etc/ssl/openssl.cnf $OPENSSL_CONF

LANCN=$(nvram get https_crt_cn)
LANIP=$(nvram get lan_ipaddr)
LOCALDOM=$(nvram get local_domain)

echo "0.commonName=CN" >> $OPENSSL_CONF
echo "0.commonName_value=$LOCALDOM" >> $OPENSSL_CONF
echo "0.organizationName=O" >> $OPENSSL_CONF
echo "0.organizationName_value=$(uname -o)" >> $OPENSSL_CONF
echo "0.emailAddress=E" >> $OPENSSL_CONF
echo "0.emailAddress_value=admin@www.asusrouter.com" >> $OPENSSL_CONF

# Required extension
sed -i "/\[ v3_ca \]/aextendedKeyUsage = serverAuth" $OPENSSL_CONF

# Start of SAN extensions
sed -i "/\[ CA_default \]/acopy_extensions = copy" $OPENSSL_CONF
sed -i "/\[ v3_req \]/asubjectAltName = @alt_names" $OPENSSL_CONF

I=18
# IP
add_san "IP.1 = $LANIP"
add_san "DNS.$I = $LANIP"
I=$(($I + 1))

# User-defined SANs (if we have any)
if [ "$LANCN" != "" ]
then
	for CN in $LANCN; do
		add_san "DNS.$I = $CN"
		I=$(($I + 1))
	done
fi

# hostnames
LANDOMAIN=$(nvram get lan_domain)
COMPUTERNAME=$(nvram get computer_name)
LANHOSTNAME=$(nvram get lan_hostname)

if [ "$COMPUTERNAME" != "" ]
then
	add_san "DNS.$I = $COMPUTERNAME"
	I=$(($I + 1))

	if [ "$LANDOMAIN" != "" ]
	then
		add_san "DNS.$I = $COMPUTERNAME.$LANDOMAIN"
		I=$(($I + 1))
	fi
fi

if [ "$LANHOSTNAME" != "" -a "$COMPUTERNAME" != "$LANHOSTNAME" ]
then
	add_san "DNS.$I = $LANHOSTNAME"
	I=$(($I + 1))

	if [ "$LANDOMAIN" != "" ]
	then
		add_san "DNS.$I = $LANHOSTNAME.$LANDOMAIN"
		I=$(($I + 1))
	fi
fi


# DDNS
DDNSHOSTNAME=$(nvram get ddns_hostname_x)
DDNSSERVER=$(nvram get ddns_server_x)
DDNSUSER=$(nvram get ddns_username_x)

if [ "$(nvram get ddns_enable_x)" == "1" -a "$DDNSSERVER" != "WWW.DNSOMATIC.COM" -a "$DDNSHOSTNAME" != "" ]
then
	if [ "$DDNSSERVER" == "WWW.NAMECHEAP.COM" -a "$DDNSUSER" != "" ]
	then
		add_san "DNS.$I = $DDNSHOSTNAME.$DDNSUSER"
		I=$(($I + 1))
	else
		add_san "DNS.$I = $DDNSHOSTNAME"
		I=$(($I + 1))
	fi
fi

# create the key and certificate request
#OPENSSL_CONF="/etc/openssl.config" $OPENSSL req -new -out /tmp/cert.csr -keyout /tmp/privkey.pem -newkey rsa:2048 -passout pass:password
#OPENSSL_CONF="/etc/openssl.config" $OPENSSL rsa -in /tmp/privkey.pem -out key.pem -passin pass:password
OPENSSL_CONF="/etc/openssl.config" $OPENSSL ecparam -out key.pem -name prime256v1 -genkey
OPENSSL_CONF="/etc/openssl.config" $OPENSSL req -new -key key.pem -out /tmp/cert.csr

# Import the self-certificate
OPENSSL_CONF="/etc/openssl.config" RANDFILE=/dev/urandom $OPENSSL req -x509 -new -nodes -in /tmp/cert.csr -key key.pem -days 3653 -sha256 -out cert.pem

# server.pem for WebDav SSL
cat key.pem cert.pem > server.pem

cp cert.pem cert.crt

chmod 640 key.pem

rm -f /tmp/cert.csr /tmp/privkey.pem $OPENSSL_CONF /var/run/gencert.pid
