Introduction to PKI
===================

This document is designed to give you a brief introduction into how a PKI, or
Public Key Infrastructure, works.

Terminology Used
----------------

To avoid confusion, the following terms will be used throughout the Easy-RSA
documentation. Short forms may be substituted for longer forms as convenient.

 *  **PKI**: Public Key Infrastructure. This describes the collection of files
    and associations between the CA, keypairs, requests, and certificates.
 *  **CA**: Certificate Authority. This is the "master cert" at the root of a
    PKI.
 *  **cert**: Certificate. A certificate is a request that has been signed by a
    CA. The certificate contains the public key, some details describing the
    cert itself, and a digital signature from the CA.
 *  **request**: Certificate Request (optionally 'req'.) This is a request for a
    certificate that is then sent to a CA for signing. A request contains the
    desired cert information along with a digital signature from the private
    key.
 *  **keypair**: A keypair is an asymmetric cryptographic pair of keys. These
    keys are split into two parts: the public and private keys. The public key
    is included in a request and certificate.

The CA
------

The heart of a PKI is the CA, or Certificate Authority, and this is also the
most security-sensitive. The CA private key is used to sign all issued
certificates, so its security is critical in keeping the entire PKI safe. For
this reason, it is highly recommended that the CA PKI structure be kept on a
system dedicated for such secure usage; it is not a great idea to keep the CA
PKI mixed in with one used to generate end-entity certificates, such as clients
or servers (VPN or web servers.)

To start a new PKI, the CA is first created on the secure environment.
Depending on security needs, this could be managed under a locked down account,
dedicated system, or even a completely offline system or using removable media
to improve security (after all, you can't suffer an online break-in if your
system or PKI is not online.) The exact steps to create a CA are described in a
separate section. When creating a new CA, the CA keypair (private and public
keys) are created, as well as the file structure necessary to support signing
issued certificates.

Once a CA has been created, it can receive certificate requests from
end-entities. These entity certificates are issued to consumers of X509
certificates, such as a client or server of a VPN, web, or email system.  The
certificate requests and certificates are not security-sensitive, and can be
transferred in whatever means convenient, such as email, flash drive, etc. For
better security, it is a good idea to verify the received request matches the
sender's copy, such as by verifying the expected checksum against the sender's
original.

Keypairs and requests
---------------------

Individual end-entities do not need a full CA set up and will only need to
create a keypair and associated certificate request. The private key is not used
anywhere except on this entity, and should never leave that system. It is wise
to secure this private key with a strong passphrase, because if lost or stolen
the holder of the private key can make connections appearing as the certificate
holder.

Once a keypair is generated, the certificate request is created and digitally
signed using the private key. This request will be sent to a CA for signing, and
a signed certificate will be returned.

How requests become certificates
--------------------------------

After a CA signs the certificate request, a signed certificate is produced. In
this step, the CA's private key is used to digitally sign the entity's public
key so that any system trusting the CA certificate can implicitly trust the
newly issued certificate. This signed certificate is then sent back to the
requesting entity. The issued certificate is not security-sensitive and can be
sent over plaintext transmission methods.

Verifying an issued certificate
-------------------------------

After 2 entities have created keypairs, sent their requests to the CA, and
received a copy of their signed certificates and the CA's own certificate, they
can mutually authenticate with one-another. This process does not require the 2
entities to have previously exchanged any kind of security information directly.

During a TLS handshake each side of the connection presents their own cert chain
to the remote end. Each side checks the validity of the cert received against
their own copy of the CA cert. By trusting the CA root cert, the peer they are
talking to can be authenticated.

The remote end proves it "really is" the entity identified by the cert by
signing a bit of data using its own private key. Only the holder of the private
key is able to do this, allowing the remote end to verify the authenticity of
the system being connected to.
