Easy-RSA 3 Quickstart README
============================

This is a quickstart guide to using Easy-RSA version 3. Detailed help on usage
and specific commands can be found by running ./easyrsa -h.  Additional
documentation can be found in the doc/ directory.

If you're upgrading from the Easy-RSA 2.x series, there are Upgrade-Notes
available, also under the doc/ path.

Setup and signing the first request
-----------------------------------

Here is a quick run-though of what needs to happen to start a new PKI and sign
your first entity certificate:

1. Choose a system to act as your CA and create a new PKI and CA:

        ./easyrsa init-pki
        ./easyrsa build-ca

2. On the system that is requesting a certificate, init its own PKI and generate
   a keypair/request. Note that init-pki is used _only_ when this is done on a
   separate system (or at least a separate PKI dir.) This is the recommended
   procedure. If you are not using this recommended procedure, skip the next
   import-req step.

        ./easyrsa init-pki
        ./easyrsa gen-req EntityName

3. Transport the request (.req file) to the CA system and import it. The name
   given here is arbitrary and only used to name the request file.

        ./easyrsa import-req /tmp/path/to/import.req EntityName

4. Sign the request as the correct type. This example uses a client type:

        ./easyrsa sign-req client EntityName

5. Transport the newly signed certificate to the requesting entity. This entity
   may also need the CA cert (ca.crt) unless it had a prior copy.

6. The entity now has its own keypair, signed cert, and the CA.

Signing subsequent requests
---------------------------

Follow steps 2-6 above to generate subsequent keypairs and have the CA return
signed certificates.

Revoking certs and creating CRLs
--------------------------------

This is a CA-specific task.

To permanently revoke an issued certificate, provide the short name used during
import:

        ./easyrsa revoke EntityName

To create an updated CRL that contains all revoked certs up to that point:

        ./easyrsa gen-crl

After generation, the CRL will need to be sent to systems that reference it.

Generating Diffie-Hellman (DH) params
-------------------------------------

After initializing a PKI, any entity can create DH params that needs them. This
is normally only used by a TLS server. While the CA PKI can generate this, it
makes more sense to do it on the server itself to avoid the need to send the
files to another system after generation.

DH params can be generated with:

        ./easyrsa gen-dh

Showing details of requests or certs
------------------------------------

To show the details of a request or certificate by referencing the short
EntityName, use one of the following commands. It is an error to call these
without a matching file.

        ./easyrsa show-req EntityName
        ./easyrsa show-cert EntityName

Changing private key passphrases
--------------------------------

RSA and EC private keys can be re-encrypted so a new passphrase can be supplied
with one of the following commands depending on the key type:

        ./easyrsa set-rsa-pass EntityName
        ./easyrsa set-ec-pass EntityName

Optionally, the passphrase can be removed completely with the 'nopass' flag.
Consult the command help for details.
