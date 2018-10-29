#!/bin/sh

PID=$$
SECS=1262278080

WAITTIMER=0
while [ -f "/var/run/gencert.pid" -a $WAITTIMER -lt 14 ]
do
	WAITTIMER=$((WAITTIMER+2))
	sleep $WAITTIMER
done
touch /var/run/gencert.pid

cd /etc
KEYNAME="key.pem"
CERTNAME="cert.pem"

OPENSSLCNF="/etc/openssl.config.$PID"

cp -L /etc/openssl.cnf $OPENSSLCNF

LANCN=$(nvram get https_crt_cn)
LANIP=$(nvram get lan_ipaddr)

if [ "$LANCN" != "" ]
then
	I=0
	for CN in $LANCN; do
		echo "$I.commonName=CN" >> $OPENSSLCNF
		echo "$I.commonName_value=$CN" >> $OPENSSLCNF
		echo "$I.organizationName=O" >> $OPENSSLCNF
		echo "$I.organizationName_value=$(uname -o)" >> $OPENSSLCNF
		I=$(($I + 1))
	done
else
	echo "0.commonName=CN" >> $OPENSSLCNF
	echo "0.commonName_value=$LANIP" >> $OPENSSLCNF
	echo "0.organizationName=O" >> $OPENSSLCNF
	echo "0.organizationName_value=$(uname -o)" >> $OPENSSLCNF
fi

I=0
# Start of SAN extensions
sed -i "/\[ CA_default \]/acopy_extensions = copy" $OPENSSLCNF
sed -i "/\[ v3_ca \]/asubjectAltName = @alt_names" $OPENSSLCNF
sed -i "/\[ v3_req \]/asubjectAltName = @alt_names" $OPENSSLCNF
echo "[alt_names]" >> $OPENSSLCNF

# IP
echo "IP.0 = $LANIP" >> $OPENSSLCNF
echo "DNS.$I = $LANIP" >> $OPENSSLCNF # For broken clients like IE
I=$(($I + 1))

# DUT
echo "DNS.$I = router.asus.com" >> $OPENSSLCNF
I=$(($I + 1))

# User-defined CN (if we have any)
if [ "$LANCN" != "" ]
then
	for CN in $LANCN; do
		echo "DNS.$I = $CN" >> $OPENSSLCNF
		I=$(($I + 1))
	done
fi

# hostnames

LANDOMAIN=$(nvram get lan_domain)
COMPUTERNAME=$(nvram get computer_name)
LANHOSTNAME=$(nvram get lan_hostname)

if [ "$COMPUTERNAME" != "" ]
then
	echo "DNS.$I = $COMPUTERNAME" >> $OPENSSLCNF
	I=$(($I + 1))

	if [ "$LANDOMAIN" != "" ]
	then
		echo "DNS.$I = $COMPUTERNAME.$LANDOMAIN" >> $OPENSSLCNF
		I=$(($I + 1))
	fi
fi

if [ "$LANHOSTNAME" != "" ]
then
	echo "DNS.$I = $LANHOSTNAME" >> $OPENSSLCNF
	I=$(($I + 1))

	if [ "$LANDOMAIN" != "" ]
	then
		echo "DNS.$I = $LANHOSTNAME.$LANDOMAIN" >> $OPENSSLCNF
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
		echo "DNS.$I = $DDNSHOSTNAME.$DDNSUSER" >> $OPENSSLCNF
		I=$(($I + 1))
	else
		echo "DNS.$I = $DDNSHOSTNAME" >> $OPENSSLCNF
		I=$(($I + 1))
	fi
fi


# create the key
openssl genrsa -out $KEYNAME.$PID 2048 -config $OPENSSLCNF
# create certificate request and sign it
openssl req -new -x509 -key $KEYNAME.$PID -sha256 -out $CERTNAME.$PID -days 3653 -config $OPENSSLCNF

# server.pem for WebDav SSL
cat $KEYNAME.$PID $CERTNAME.$PID > server.pem

mv $KEYNAME.$PID $KEYNAME
mv $CERTNAME.$PID $CERTNAME

chmod 640 $KEYNAME
chmod 640 $CERTNAME

rm -f /tmp/cert.csr $OPENSSLCNF /var/run/gencert.pid
