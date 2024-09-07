# MQTT Tests

This folder contains all of the MQTT functional tests. These have been added to verify that basic functionality is still present.

# Setup the tests

As some of these tests require client certificates, and these expire. It is left to the user to generate these properly.

The server certificates are also left to the user to generate.

## Server Certs

To get the normal certs for the server connections, use the following:

`openssl s_client -connect mqtt.eclipse.org:8883 -showcerts < /dev/null > cert.pem`

Then, strip out all information outside of the ----- BEGIN CERTIFICATE ----- and ----- END CERTIFICATE ----- lines.

You must then append the root CA certificate to the cert.pem. E.g:

`cat cert.pem /etc/ssl/certs/DST_Root_CA_X3.pem > cert.pem`

For any further server connections, you must append the any further certs to the same .pem file, generated in the same order as above.

## Client Certs and Keys

To generate a client key/ cert pair, use https://test.mosquitto.org/ssl/ to generate a private key, CSR and download a private certificate.

The key and cert must then be concatenated together into a file called client.pem (in this directory).

`cat client.key client.crt > client.pem`




