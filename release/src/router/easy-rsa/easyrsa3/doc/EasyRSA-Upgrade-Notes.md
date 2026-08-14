Upgrading to Easy-RSA 3 from earlier versions
=========

People upgrading to Easy-RSA 3 from a 2.x version should note some important
changes starting with version 3. For a better overview of version 3 in general,
see the Readme in the doc/ directory.

Easy-RSA 3 comes with an automated upgrade utility to convert an existing 2.x
PKI to version 3. For details, see [this article on the OpenVPN
wiki](https://community.openvpn.net/openvpn/wiki/easyrsa-upgrade).

List of important changes
----

 * nsCertType extensions are no longer included by default. Use of such
   "Netscape" attributes have been deprecated upstream and their use is
   discouraged. Configure `EASYRSA_NS_SUPPORT` in vars if you want to enable
   this legacy behavior.

   Notably, this is important for OpenVPN deployments relying on the
   `--ns-cert-type` directive. Either have OpenVPN use the preferred
   `--remote-cert-tls` option, or enable legacy NS extensions.

 * The default request Subject (or DN, Distinguished Name) includes just the
   commonName. This is more suitable for VPNs and environments that don't wish
   to include info about the Country/State/City/Org/OU in certs. Configure
   `EASYRSA_DN` in vars if you want to enable the legacy behavior.

 * The 3.0 release lacks PKCS#11 (smartcard/token) support. This is anticipated
   to be supported in a future point-release to target each platform's need.

 * The -utf8 option has been added for all supported commands.  This should be
   backwards compatible with ASCII strings.

 * The default private key encryption has been changed from 3des to aes256.


Some new concepts
----

Easy-RSA 3 has some new concepts compared to the prior v2 series.

### Request-Import-Sign workflow

  v3 is now designed to support keypairs generated on the target system where
  they will be used, thus improving security as no keys need to be transferred
  between hosts. The old workflow of generating everything in a single PKI is
  still supported as well.

  The recommended workflow when using Easy-RSA as a CA is to import requests,
  sign them, and return the issued & CA certs. Each requesting system can use
  Easy-RSA without a CA to generate keypairs & requests.

### "Org"-style DN flexibility

  When using Easy-RSA in the "org" DN mode, it is no longer required to match
  some of the field values. This improves flexibility, and enables easier remote
  generation as the requester doesn't need to know the CA's values in advance.

  Previously in v2, the Country, State, and Org values all had to match or a
  request couldn't be signed. If you want the old behavior you can change the
  OpenSSL config to require it or simply look over the DN at signing time.
