# strongSwan Configuration #

## Overview ##

strongSwan is an OpenSource IPsec-based VPN solution.

This document is just a short introduction, for more detailed information
consult the man pages and [**our wiki**](http://wiki.strongswan.org).


## Quickstart ##

In the following examples we assume, for reasons of clarity, that **left**
designates the **local** host and that **right** is the **remote** host.

Certificates for users, hosts and gateways are issued by a fictitious
strongSwan CA.  How to generate private keys and certificates using OpenSSL or
the strongSwan PKI tool will be explained in one of the sections below.
The CA certificate `strongswanCert.pem` must be present on all VPN endpoints
in order to be able to authenticate the peers.


### Site-to-site case ###

In this scenario two security gateways _moon_ and _sun_ will connect the
two subnets _moon-net_ and _sun-net_ with each other through a VPN tunnel
set up between the two gateways:

    10.1.0.0/16 -- | 192.168.0.1 | === | 192.168.0.2 | -- 10.2.0.0/16
      moon-net          moon                 sun           sun-net

Configuration on gateway _moon_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/moonCert.pem

    /etc/ipsec.secrets:

        : RSA moonKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn net-net
            leftsubnet=10.1.0.0/16
            leftcert=moonCert.pem
            right=192.168.0.2
            rightsubnet=10.2.0.0/16
            rightid="C=CH, O=strongSwan, CN=sun.strongswan.org"
            auto=start

Configuration on gateway _sun_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/sunCert.pem

    /etc/ipsec.secrets:

        : RSA sunKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn net-net
            leftsubnet=10.2.0.0/16
            leftcert=sunCert.pem
            right=192.168.0.1
            rightsubnet=10.1.0.0/16
            rightid="C=CH, O=strongSwan, CN=moon.strongswan.org"
            auto=start


### Host-to-host case ###

This is a setup between two single hosts which don't have a subnet behind
them.  Although IPsec transport mode would be sufficient for host-to-host
connections we will use the default IPsec tunnel mode.

    | 192.168.0.1 | === | 192.168.0.2 |
         moon                sun

Configuration on host _moon_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/moonCert.pem

    /etc/ipsec.secrets:

        : RSA moonKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn host-host
            leftcert=moonCert.pem
            right=192.168.0.2
            rightid="C=CH, O=strongSwan, CN=sun.strongswan.org"
            auto=start

Configuration on host _sun_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/sunCert.pem

    /etc/ipsec.secrets:

        : RSA sunKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn host-host
            leftcert=sunCert.pem
            right=192.168.0.1
            rightid="C=CH, O=strongSwan, CN=moon.strongswan.org"
            auto=start


### Roadwarrior case ###

This is a very common case where a strongSwan gateway serves an arbitrary
number of remote VPN clients usually having dynamic IP addresses.

    10.1.0.0/16 -- | 192.168.0.1 | === | x.x.x.x |
      moon-net          moon              carol

Configuration on gateway _moon_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/moonCert.pem

    /etc/ipsec.secrets:

        : RSA moonKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn rw
            leftsubnet=10.1.0.0/16
            leftcert=moonCert.pem
            right=%any
            auto=add

Configuration on roadwarrior _carol_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/carolCert.pem

    /etc/ipsec.secrets:

        : RSA carolKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn home
            leftcert=carolCert.pem
            right=192.168.0.1
            rightsubnet=10.1.0.0/16
            rightid="C=CH, O=strongSwan, CN=moon.strongswan.org"
            auto=start


### Roadwarrior case with virtual IP ###

Roadwarriors usually have dynamic IP addresses assigned by the ISP they are
currently attached to.  In order to simplify the routing from _moon-net_ back
to the remote access client _carol_ it would be desirable if the roadwarrior had
an inner IP address chosen from a pre-defined pool.

    10.1.0.0/16 -- | 192.168.0.1 | === | x.x.x.x | -- 10.3.0.1
      moon-net          moon              carol       virtual IP

In our example the virtual IP address is chosen from the address pool
`10.3.0.0/16` which can be configured by adding the parameter

    rightsourceip=10.3.0.0/16

to the gateway's `ipsec.conf`.  To request an IP address from this pool a
roadwarrior can use IKEv1 mode config or IKEv2 configuration payloads.
The configuration for both is the same

    leftsourceip=%config

Configuration on gateway _moon_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/moonCert.pem

    /etc/ipsec.secrets:

        : RSA moonKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn rw
            leftsubnet=10.1.0.0/16
            leftcert=moonCert.pem
            right=%any
            rightsourceip=10.3.0.0/16
            auto=add

Configuration on roadwarrior _carol_:

    /etc/ipsec.d/cacerts/strongswanCert.pem

    /etc/ipsec.d/certs/carolCert.pem

    /etc/ipsec.secrets:

        : RSA carolKey.pem "<optional passphrase>"

    /etc/ipsec.conf:

        conn home
            leftsourceip=%config
            leftcert=carolCert.pem
            right=192.168.0.1
            rightsubnet=10.1.0.0/16
            rightid="C=CH, O=strongSwan, CN=moon.strongswan.org"
            auto=start


## Generating certificates and CRLs ##

This section is not a full-blown tutorial on how to use OpenSSL or the
strongSwan PKI tool.  It just lists a few points that are relevant if you want
to generate your own certificates and CRLs for use with strongSwan.


### Generating a CA certificate ###

The OpenSSL statement

    openssl req -x509 -days 1460 -newkey rsa:4096 \
                -keyout strongswanKey.pem -out strongswanCert.pem

creates a 4096 bit RSA private key `strongswanKey.pem` and a self-signed CA
certificate `strongswanCert.pem` with a validity of 4 years (1460 days).

    openssl x509 -in cert.pem -noout -text

lists the properties of  a X.509 certificate `cert.pem`. It allows you to verify
whether the configuration defaults in `openssl.cnf` have been inserted
correctly.

If you prefer the CA certificate to be in binary DER format then the following
command achieves this transformation:

     openssl x509 -in strongswanCert.pem -outform DER -out strongswanCert.der

The statements

    ipsec pki --gen -s 4096 > strongswanKey.der
    ipsec pki --self --ca --lifetime 1460 --in strongswanKey.der \
              --dn "C=CH, O=strongSwan, CN=strongSwan Root CA" \
              > strongswanCert.der
    ipsec pki --print --in strongswanCert.der

achieve about the same with the strongSwan PKI tool.  Unlike OpenSSL the tool
stores keys and certificates in the binary DER format by default.
The `--outform` option may be used to write PEM encoded files.

The directory `/etc/ipsec.d/cacerts` contains all required CA certificates
either in binary DER or in Base64 PEM format, irrespective of the file suffix
the correct format will be determined.


### Generating a host or user certificate ###

The OpenSSL statement

     openssl req -newkey rsa:2048 -keyout hostKey.pem \
                 -out hostReq.pem

generates a 2048 bit RSA private key `hostKey.pem` and a certificate request
`hostReq.pem` which has to be signed by the CA.

If you want to add a _subjectAltName_ field to the host certificate you must
edit the OpenSSL configuration file `openssl.cnf` and add the following line in
the `[ usr_cert ]` section:

     subjectAltName=DNS:moon.strongswan.org

if you want to identify the host by its Fully Qualified Domain Name (FQDN), or

     subjectAltName=IP:192.168.0.1

if you want the ID to be of type _IPV4_ADDR_. Of course you could include both
ID types with

     subjectAltName=DNS:moon.strongswan.org,IP:192.168.0.1

but the use of an IP address for the identification of a host should be
discouraged anyway.

For user certificates the appropriate ID type is _RFC822_ADDR_ which can be
specified as

     subjectAltName=email:carol@strongswan.org

or if the user's e-mail address is part of the subject's distinguished name

     subjectAltName=email:copy

Now the certificate request can be signed by the CA with the command

     openssl ca -in hostReq.pem -days 730 -out hostCert.pem -notext

If you omit the `-days` option then the `default_days` value (365 days)
specified in `openssl.cnf` is used.  The `-notext` option avoids that a human
readable listing of the certificate is prepended to the Base64 encoded
certificate body.

If you want to use the dynamic CRL fetching feature described in one of the
following sections then you may include one or several _crlDistributionPoints_
in your end certificates.  This can be done in the `[ usr_cert ]` section of the
`openssl.cnf` configuration file:

    crlDistributionPoints=@crl_dp

    [ crl_dp ]

    URI.1="http://crl.strongswan.org/strongswan.crl"
    URI.2="ldap://ldap.strongswan.org/cn=strongSwan Root CA, o=strongSwan,
           c=CH?certificateRevocationList"

If you have only a single HTTP distribution point then the short form

    crlDistributionPoints="URI:http://crl.strongswan.org/strongswan.crl"

also works.

Again the statements

    ipsec pki --gen > moonKey.der
    ipsec pki --pub --in moonKey.der | ipsec pki --issue --lifetime 730 \
              --cacert strongswanCert.der --cakey strongswanKey.der \
              --dn "C=CH, O=strongSwan, CN=moon.strongswan.org" \
              --san moon.strongswan.org --san 192.168.0.1 \
              --crl http://crl.strongswan.org/strongswan.crl > moonCert.der

do something similar using the strongSwan PKI tool.

Usually, a Windows or Mac OS X (or iOS) based VPN client needs its private key,
its host or user certificate, and the CA certificate.  The most convenient way
to load this information is to put everything into a PKCS#12 container:

     openssl pkcs12 -export -inkey carolKey.pem \
                    -in carolCert.pem -name "carol" \
                    -certfile strongswanCert.pem -caname "strongSwan Root CA" \
                    -out carolCert.p12


### Generating a CRL ###

An empty CRL that is signed by the CA can be generated with the command

     openssl ca -gencrl -crldays 15 -out crl.pem

If you omit the `-crldays` option then the `default_crl_days` value (30 days)
specified in `openssl.cnf` is used.

If you prefer the CRL to be in binary DER format then this conversion
can be achieved with

     openssl crl -in crl.pem -outform DER -out cert.crl

The strongSwan PKI tool provides the `--signcrl` command to sign CRLs.

The directory `/etc/ipsec.d/crls` contains all CRLs either in binary DER
or in Base64 PEM format, irrespective of the file suffix the correct format
will be determined.


### Revoking a certificate ###

A specific host certificate stored in the file `host.pem` is revoked with the
command

     openssl ca -revoke host.pem

Next the CRL file must be updated

     openssl ca -gencrl -crldays 60 -out crl.pem

The content of the CRL file can be listed with the command

     openssl crl -in crl.pem -noout -text

in the case of a Base64 CRL, or alternatively for a CRL in DER format

     openssl crl -inform DER -in cert.crl -noout -text

Again the `--signcrl` command of the strongSwan PKI tool may also be used to
create new CRLs containing additional certificates.


## Configuring the connections - ipsec.conf ##

### Configuring my side ###

Usually the **local** side is the same for all connections.  Therefore it makes
sense to put the definitions characterizing the strongSwan security gateway into
the `conn %default` section of the configuration file `/etc/ipsec.conf`.  If we
assume throughout this document that the strongSwan security gateway is **left**
and the peer is **right** then we can write

    conn %default
         leftcert=moonCert.pem
         # load connection definitions automatically
         auto=add

The X.509 certificate by which the strongSwan security gateway will authenticate
itself by sending it in binary form to its peers as part of the Internet Key
Exchange (IKE) is specified in the line

     leftcert=moonCert.pem

The certificate can either be stored in Base64 PEM-format or in the binary
DER-format. Irrespective of the file suffix the correct format will be
determined.  Therefore `leftcert=moonCert.der` or `leftcert=moonCert.cer`
would also be valid alternatives.

When using relative pathnames as in the examples above, the certificate files
must be stored in in the directory `/etc/ipsec.d/certs`.  In order to
distinguish strongSwan's own certificates from locally stored trusted peer
certificates (see below for details), they could also be stored in a
subdirectory below `/etc/ipsec.d/certs` as e.g. in

    leftcert=mycerts/moonCert.pem

Absolute pathnames are also possible as in

    leftcert=/usr/ssl/certs/moonCert.pem

As an ID for the VPN gateway we recommend the use of a Fully Qualified Domain
Name (FQDN) of the form

    conn rw
         right=%any
         leftid=moon.strongswan.org

**Important**: When a FQDN identifier is used it must be explicitly included as
a so called _subjectAltName_ of type _dnsName_ (`DNS:`) in the certificate
indicated by `leftcert`.  For details on how to generate certificates with
_subjectAltNames_, please refer to the sections above.

If you don't want to mess with _subjectAltNames_, you can use the certificate's
Distinguished Name (DN) instead, which is an identifier of type _DER_ASN1_DN_
and which can be written e.g. in the LDAP-type format

    conn rw
         right=%any
         leftid="C=CH, O=strongSwan, CN=moon.strongswan.org"

Since the subject's DN is part of the certificate, the `leftid` does not have to
be declared explicitly. Thus the entry

    conn rw
         right=%any

automatically assumes the subject DN of `leftcert` to be the host ID.


### Multiple certificates ###

strongSwan supports multiple local host certificates and corresponding
RSA private keys:

    conn rw1
         right=%any
         rightid=peer1.domain1
         leftcert=myCert1.pem
         # leftid is DN of myCert1

    conn rw2
         right=%any
         rightid=peer2.domain2
         leftcert=myCert2.pem
         # leftid is DN of myCert2

When _peer1_ initiates a connection then strongSwan will send _myCert1_ and will
sign with _myKey1_ defined in `/etc/ipsec.secrets` (see below) whereas
_myCert2_ and _myKey2_ will be used in a connection setup started from _peer2_.


### Configuring the peer side using CA certificates ###

Now we can proceed to define our connections.  In many applications we might
have dozens of road warriors connecting to a central strongSwan security
gateway. The following most simple statement:

    conn rw
         right=%any

defines the general roadwarrior case.  The line `right=%any` literally means
that any IPsec peer is accepted, regardless of its current IP source address and
its ID, as long as the peer presents a valid X.509 certificate signed by a CA
the strongSwan security gateway puts explicit trust in.  Additionally, the
signature during IKE gives proof that the peer is in possession of the private
key matching the public key contained in the transmitted certificate.

The ID by which a peer is identifying itself during IKE can by any of the ID
types _IPV[46]_ADDR_, _FQDN_, _RFC822_ADDR_ or _DER_ASN1_DN_.  If one of the
first three ID types is used, then the accompanying X.509 certificate of the
peer must contain a matching _subjectAltName_ field of the type _ipAddress_
(`IP:`), _dnsName_ (`DNS:`) or _rfc822Name_ (`email:`), respectively.  With the
fourth type, _DER_ASN1_DN_, the identifier must completely match the subject
field of the peer's certificate.  One of the two possible representations of a
Distinguished Name (DN) is the LDAP-type format

     rightid="C=CH, O=strongSwan IPsec, CN=sun.strongswan.org"

Additional whitespace can be added everywhere as desired since it will be
automatically eliminated by the parser.  An exception is the single whitespace
between individual words, like e.g. in `strongSwan IPsec`, which is preserved.

The Relative Distinguished Names (RDNs) can alternatively be separated by a
slash `/` instead of a comma `,`

     rightid="/C=CH/O=strongSwan IPsec/CN=sun.strongswan.org"

This is the representation extracted from the certificate by the OpenSSL
`-subject` command line option

     openssl x509 -in sunCert.pem -noout -subject

The following RDNs are supported by strongSwan

| Name               | Description                      |
|--------------------|----------------------------------|
| DC                 | Domain Component                 |
| C                  | Country                          |
| ST                 | State or province                |
| L                  | Locality or town                 |
| O                  | Organization                     |
| OU                 | Organizational Unit              |
| CN                 | Common Name                      |
| ND                 | NameDistinguisher, used with CN  |
| N                  | Name                             |
| G                  | Given name                       |
| S                  | Surname                          |
| I                  | Initials                         |
| T                  | Personal title                   |
| E                  | E-mail                           |
| Email              | E-mail                           |
| emailAddress       | E-mail                           |
| SN                 | Serial number                    |
| serialNumber       | Serial number                    |
| D                  | Description                      |
| ID                 | X.500 Unique Identifier          |
| UID                | User ID                          |
| TCGID              | [Siemens] Trust Center Global ID |
| UN                 | Unstructured Name                |
| unstructuredName   | Unstructured Name                |
| UA                 | Unstructured Address             |
| unstructuredAddress| Unstructured Address             |
| EN                 | Employee Number                  |
| employeeNumber     | Employee Number                  |
| dnQualifier        | DN Qualifier                     |

With the roadwarrior connection definition listed above, an IPsec SA for
the strongSwan security gateway `moon.strongswan.org` itself can be established.
If the roadwarriors should be able to reach e.g. the two subnets `10.1.0.0/24`
and `10.1.3.0/24` behind the security gateway then the following connection
definitions will make this possible

    conn rw1
         right=%any
         leftsubnet=10.1.0.0/24

    conn rw3
         right=%any
         leftsubnet=10.1.3.0/24

For IKEv2 connections this can even be simplified by using

    leftsubnet=10.1.0.0/24,10.1.3.0/24

If not all peers in possession of a X.509 certificate signed by a specific
certificate authority shall be given access to the Linux security gateway,
then either a subset of them can be barred by listing the serial numbers of
their certificates in a certificate revocation list (CRL) or as an alternative,
access can be controlled by explicitly putting a roadwarrior entry for each
eligible peer into `ipsec.conf`:

    conn sun
         right=%any
         rightid=sun.strongswan.org

    conn carol
         right=%any
         rightid=carol@strongswan.org

    conn dave
         right=%any
         rightid="C=CH, O=strongSwan, CN=dave@strongswan.org"

When the IP address of a peer is known to be stable, it can be specified as
well.  This entry is mandatory when the strongSwan host wants to act as the
initiator of an IPsec connection.

    conn sun
         right=192.168.0.2
         rightid=sun.strongswan.org

    conn carol
         right=192.168.0.100
         rightid=carol@strongswan.org

    conn dave
         right=192.168.0.200
         rightid="C=CH, O=strongSwan, CN=dave@strongswan.org"

    conn venus
         right=192.168.0.50

In the last example the ID types _FQDN_, _RFC822_ADDR_, _DER_ASN1_DN_ and
_IPV4_ADDR_, respectively, were used.  Of course all connection definitions
presented so far have included the lines in the `conn %defaults` section,
comprising among other a `leftcert` entry.


### Handling Virtual IPs and narrowing ###

Often roadwarriors are behind NAT-boxes, which causes the inner IP source
address of an IPsec tunnel to be different from the outer IP source address
usually assigned dynamically by the ISP.  Whereas the varying outer IP address
can be handled by the `right=%any` construct, the inner IP address or subnet
must always be declared in a connection definition. Therefore for the three
roadwarriors _rw1_ to _rw3_ connecting to a strongSwan security gateway the
following entries are required in `/etc/ipsec.conf`:

    conn rw1
         right=%any
         righsubnet=10.4.0.5/32

    conn rw2
         right=%any
         rightsubnet=10.4.0.47/32

    conn rw3
         right=%any
         rightsubnet=10.4.0.128/28

Because the charon daemon uses narrowing (even for IKEv1) these three entries
can be reduced to the single connection definition

    conn rw
         right=%any
         rightsubnet=10.4.0.0/24

Any host will be accepted (of course after successful authentication based on
the peer's X.509 certificate only) if it declares a client subnet lying totally
within the boundaries defined by the subnet definition (in our example
`10.4.0.0/24`).

This strongSwan feature can also be helpful with VPN clients getting a
dynamically assigned inner IP from a DHCP server located on the NAT router box.

Since the private IP address of roadwarriors will often not be known they are
usually assigned virtual IPs from a predefined pool.  This also makes routing
traffic back to the roadwarriors easier. For example, to assign each client an
IP address from the `10.5.0.0/24` subnet `conn rw` can be defined as

    conn rw
         right=%any
         rightsourceip=10.5.0.0/24


### Protocol and Port Selectors ###

strongSwan offers the possibility to restrict the protocol and optionally the
ports in an IPsec SA using the `rightprotoport` and `leftprotoport` parameters.
For IKEv2 multiple such restrictions can also be configured in
`leftsubnet` and `rightsubnet`.

Some examples:

    conn icmp
         right=%any
         rightprotoport=icmp
         leftid=moon.strongswan.org
         leftprotoport=icmp

    conn http
         right=%any
         rightprotoport=6
         leftid=moon.strongswan.org
         leftprotoport=6/80

    conn l2tp
         right=%any
         # with port wildcard for interoperability with certain L2TP clients
         rightprotoport=17/%any
         leftid=moon.strongswan.org
         leftprotoport=17/1701

    conn dhcp
         right=%any
         rightprotoport=udp/bootpc
         leftid=moon.strongswan.org
         leftsubnet=0.0.0.0/0  #allows DHCP discovery broadcast
         leftprotoport=udp/bootps

Protocols and ports can be designated either by their numerical values
or by their acronyms defined in `/etc/services`.

Based on the protocol and port selectors appropriate policies will be set
up, so that only the specified payload types will pass through the IPsec
tunnel.


### IPsec policies based on wildcards ###

In large VPN-based remote access networks there is often a requirement that
access to the various parts of an internal network must be granted selectively,
e.g. depending on the group membership of the remote access user.  strongSwan
makes this possible by applying wildcard filtering on the VPN user's
distinguished name (_ID_DER_ASN1_DN_).

Let's make a practical example:

An organization has a sales department (_OU=Sales_) and a research group
(_OU=Research_).  In the company intranet there are separate subnets for Sales
(`10.0.0.0/24`) and Research (`10.0.1.0/24`) but both groups share a common web
server (`10.0.2.100`).  The VPN clients use Virtual IP addresses that are either
assigned statically or from a dynamic pool.  The sales and research departments
use IP addresses from separate address pools (`10.1.0.0/24`) and
(`10.1.1.0/24`), respectively.  An X.509 certificate is issued to each employee,
containing in its subject distinguished name the country (_C=CH_), the company
(_O=ACME_), the group membership (_OU=Sales_ or _OU=Research_) and the common
name (e.g. _CN=Bart Simpson_).

The IPsec policy defined above can now be enforced with the following three
IPsec security associations:

    conn sales
         right=%any
         rightid="C=CH, O=ACME, OU=Sales, CN=*"
         rightsourceip=10.1.0.0/24       # Sales IP range
         leftsubnet=10.0.0.0/24          # Sales subnet

    conn research
         right=%any
         rightid="C=CH, O=ACME, OU=Research, CN=*"
         rightsourceip=10.1.1.0/24       # Research IP range
         leftsubnet=10.0.1.0/24          # Research subnet

    conn web
         right=%any
         rightid="C=CH, O=ACME, OU=*, CN=*"
         rightsubnet=10.1.0.0/23         # Remote access IP range
         leftsubnet=10.0.2.100/32        # Web server
         rightprotoport=tcp              # TCP protocol only
         leftprotoport=tcp/http          # TCP port 80 only

The `*` character is used as a wildcard in relative distinguished names (RDNs).
In order to match a wildcard template, the _ID_DER_ASN1_DN_ of a peer must
contain the same number of RDNs (selected from the list given earlier) appearing
in the exact order defined by the template.

    "C=CH, O=ACME, OU=Research, OU=Special Effects, CN=Bart Simpson"

matches the templates

    "C=CH, O=ACME, OU=Research, OU=*, CN=*"
    "C=CH, O=ACME, OU=*, OU=Special Effects, CN=*"
    "C=CH, O=ACME, OU=*, OU=*, CN=*"

but not the template

    "C=CH, O=ACME, OU=*, CN=*"

which doesn't have the same number of RDNs.


### IPsec policies based on CA certificates ###

As an alternative to the wildcard based IPsec policies described above, access
to specific client host and subnets can be controlled on the basis of the CA
that issued the peer certificate

    conn sales
         right=%any
         rightca="C=CH, O=ACME, OU=Sales, CN=Sales CA"
         rightsourceip=10.1.0.0/24       # Sales IP range
         leftsubnet=10.0.0.0/24          # Sales subnet

    conn research
         right=%any
         rightca="C=CH, O=ACME, OU=Research, CN=Research CA"
         rightsourceip=10.1.1.0/24       # Research IP range
         leftsubnet=10.0.1.0/24          # Research subnet

    conn web
         right=%any
         rightca="C=CH, O=ACME, CN=ACME Root CA"
         rightsubnet=10.1.0.0/23         # Remote access IP range
         leftsubnet=10.0.2.100/32        # Web server
         rightprotoport=tcp              # TCP protocol only
         leftprotoport=tcp/http          # TCP port 80 only

In the example above, the connection _sales_ can be used by peers
presenting certificates issued by the Sales CA, only.  In the same way,
the use of the connection _research_ is restricted to owners of certificates
issued by the Research CA.  The connection _web_ is open to both "Sales" and
"Research" peers because the required _ACME Root CA_ is the issuer of the
Research and Sales intermediate CAs.  If no `rightca` parameter is present
then any valid certificate issued by one of the trusted CAs in
`/etc/ipsec.d/cacerts` can be used by the peer.

The `leftca` parameter usually doesn't have to be set explicitly because
by default it is set to the issuer field of the certificate loaded via
`leftcert`.  The statement

     rightca=%same

sets the CA requested from the peer to the CA used by the left side itself
as e.g. in

    conn sales
         right=%any
         rightca=%same
         leftcert=mySalesCert.pem


## Configuring certificates and CRLs ##


### Installing the CA certificates ###

X.509 certificates received by strongSwan during the IKE protocol are
automatically authenticated by going up the trust chain until a self-signed
root CA certificate is reached.  Usually host certificates are directly signed
by a root CA, but strongSwan also supports multi-level hierarchies with
intermediate CAs in between.  All CA certificates belonging to a trust chain
must be copied in either binary DER or Base64 PEM format into the directory

     /etc/ipsec.d/cacerts/


### Installing optional certificate revocation lists (CRLs) ###

By copying a CA certificate into `/etc/ipsec.d/cacerts/`, automatically all user
or host certificates issued by this CA are declared valid.  Unfortunately,
private keys might get compromised inadvertently or intentionally, personal
certificates of users leaving a company have to be blocked immediately, etc.
To this purpose certificate revocation lists (CRLs) have been created.  CRLs
contain the serial numbers of all user or host certificates that have been
revoked due to various reasons.

After successful verification of the X.509 trust chain, strongSwan searches its
list of CRLs, either obtained by loading them from the `/etc/ipsec.d/crls/`
directory, or fetching them dynamically from a HTTP or LDAP server, for the
presence of a CRL issued by the CA that has signed the certificate.

If the serial number of the certificate is found in the CRL then the public key
contained in the certificate is declared invalid and the IKE SA will not be
established.  If no CRL is found or if the deadline defined in the _nextUpdate_
field of the CRL has been reached, a warning is issued but the public key will
nevertheless be accepted (this behavior can be changed, see below).  CRLs must
be stored either in binary DER or Base64 PEM format in the `crls` directory.


### Dynamic update of certificates and CRLs ###

strongSwan reads certificates and CRLs from their respective files during system
startup and keeps them in memory.  X.509 certificates have a finite life span
defined by their validity field.  Therefore it must be possible to replace CA or
OCSP certificates kept in system memory without disturbing established IKE SAs.
Certificate revocation lists should also be updated in the regular intervals
indicated by the _nextUpdate_ field in the CRL body.  The following interactive
commands allow the manual replacement of the various files:


| Command                 | Action                                          |
|-------------------------|-------------------------------------------------|
| ipsec rereadaacerts     | reload all files in `/etc/ipsec.d/aacerts/`     |
| ipsec rereadacerts      | reload all files in `/etc/ipsec.d/acerts/`      |
| ipsec rereadcacerts     | reload all files in `/etc/ipsec.d/cacerts/`     |
| ipsec rereadcrls        | reload all files in `/etc/ipsec.d/crls/`        |
| ipsec rereadocspcerts   | reload all files in `/etc/ipsec.d/ocspcerts/`   |
| ipsec rereadsecrets     | reload `/etc/ipsec.secrets` and configured keys |
| ipsec rereadall         | all the commands above combined                 |
| ipsec purgecerts        | purge all cached certificates                   |
| ipsec purgecrl          | purge all cached CRLs                           |
| ipsec purgeocsp         | purge the OCSP cache                            |


CRLs can also be automatically fetched from an HTTP or LDAP server by using
the CRL distribution points contained in X.509 certificates.


### Local caching of CRLs ###

The `ipsec.conf` option

    config setup
         cachecrls=yes

activates the local caching of CRLs that were dynamically fetched from an
HTTP or LDAP server.  Cached copies are stored in `/etc/ipsec.d/crls` using a
unique filename formed from the issuer's _subjectKeyIdentifier_ and the
suffix `.crl`.

With the cached copy the CRL is immediately available after startup.  When the
local copy is about to expire it is automatically replaced with an updated CRL
fetched from one of the defined CRL distribution points.


### Online Certificate Status Protocol (OCSP) ###

The _Online Certificate Status Protocol_ is defined by RFC 2560.  It can be
used to query an OCSP server about the current status of an X.509 certificate
and is often used as a more dynamic alternative to a static Certificate
Revocation List (CRL).  Both the OCSP request sent by the client and the OCSP
response messages returned by the server are transported via a standard
TCP/HTTP connection.

In the simplest OCSP setup, a default URI under which the OCSP server for a
given CA can be accessed is defined in `ipsec.conf`:

    ca strongswan
         cacert=strongswanCert.pem
         ocspuri=http://ocsp.strongswan.org:8880
         auto=add

The HTTP port can be freely chosen.

OpenSSL implements an OCSP server that can be used in conjunction with an
OpenSSL-based Public Key Infrastructure.  The OCSP server is started with the
following command:

    openssl ocsp -index index.txt -CA strongswanCert.pem -port 8880 \
                 -rkey ocspKey.pem -rsigner ocspCert.pem \
                 -resp_no_certs -nmin 60 -text

The command consists of the parameters

    -index   index.txt is a copy of the OpenSSL index file containing the list
             of all issued certificates.  The certificate status in index.txt
             is designated either by V for valid or R for revoked.  If a new
             certificate is added or if a certificate is revoked using the
             openssl ca command, the OCSP server must be restarted in order for
             the changes in index.txt to take effect.

    -CA      the CA certificate

    -port    the HTTP port the OCSP server is listening on.

    -rkey    the private key used to sign the OCSP response.  The use of the
             sensitive CA private key is not recommended since this could
             jeopardize the security of your production PKI if the OCSP
             server is hacked.  It is much better to generate a special
             RSA private key just for OCSP signing use instead.

    -rsigner the certificate of the OCSP server containing a public key which
             matches the private key defined by -rkey and which can be used by
             the client to check the trustworthiness of the signed OCSP
             response.

    -resp_no_certs  With this option the OCSP signer certificate defined by
                    -rsigner is not included in the OCSP response.

    -nmin    the validity interval of an OCSP response given in minutes.

    -text    this option activates a verbose logging output, showing the
             contents of both the received OCSP request and sent OCSP response.


The OCSP signer certificate can either be put into the default directory

    /etc/ipsec.d/ocspcerts

or alternatively strongSwan can receive it as part of the OCSP response from the
remote OCSP server.  In order to verify that the server is indeed authorized by
a CA to deal out certificate status information an extended key usage attribute
must be included in the OCSP server certificate.  Just insert the parameter

    extendedKeyUsage=OCSPSigner

in the `[ usr_cert ]` section of your `openssl.cnf` configuration file before
the CA signs the OCSP server certificate.

For a given CA the corresponding _ca_ section in `ipsec.conf` (see below) allows
to define the URI of a single OCSP server.  As an alternative an OCSP URI can be
embedded into each host and user certificate by putting the line

    authorityInfoAccess = OCSP;URI:http://ocsp.strongswan.org:8880

into the `[ usr_cert ]` section of your `openssl.cnf` configuration file.
If an OCSP _authorityInfoAccess_ extension is present in a certificate then this
record overrides the default URI defined by the ca section.


### CRL Policy ###

By default strongSwan is quite tolerant concerning the handling of CRLs. It is
not mandatory for a CRL to be present in `/etc/ipsec.d/crls` and if the
expiration date defined by the _nextUpdate_ field of a CRL has been reached just
a warning is issued but a peer certificate will always be accepted if it has not
been revoked.

If you want to enforce a stricter CRL policy then you can do this by setting
the `strictcrlpolicy` option.  This is done in the `config setup` section
of the `ipsec.conf` file:

    config setup
         strictcrlpolicy=yes
          ...

A certificate received from a peer will not be accepted if no corresponding
CRL or OCSP response is available.  And if an IKE SA re-negotiation takes
place after the _nextUpdate_ deadline has been reached, the peer certificate
will be declared invalid and the cached public key will be deleted, causing
the connection in question to fail.  Therefore if you are going to use the
`strictcrlpolicy=yes` option, make sure that the CRLs will always be updated
in time.  Otherwise a total standstill might ensue.

As mentioned earlier the default setting is `strictcrlpolicy=no`.


### Configuring the peer side using locally stored certificates ###

If you don't want to use trust chains based on CA certificates as proposed above
you can alternatively import trusted peer certificates directly.

With the `conn %default` section defined above and the use of the `rightcert`
keyword for the peer side, the connection definitions presented earlier can
alternatively be written as

    conn sun
          right=%any
          rightid=sun.strongswan.org
          rightcert=sunCert.cer

     conn carol
          right=192.168.0.100
          rightcert=carolCert.der

If the peer certificates are loaded locally then there is no need to send any
certificates to the other end via the IKE protocol.  Especially if self-signed
certificates are used which wouldn't be accepted anyway by the other side.
In these cases it is recommended to add

    leftsendcert=never

to the connection definition(s) in order to avoid the sending of the host's
own certificate.  The default value is

    leftsendcert=ifasked

which causes certificates to only be sent if a certificate request is received.
If a peer does not send a certificate request then the setting

    leftsendcert=always

may be used to force sending of the certificate to the other peer.

If a peer certificate contains a _subjectAltName_ extension, then an alternative
`rightid` type can be used, as the example `conn sun` shows.  If no `rightid`
entry is present then the subject distinguished name contained in the
certificate is taken as the ID.

Using the same rules concerning pathnames that apply to the gateway's own
certificates, the following two definitions are also valid for trusted peer
certificates:

    rightcert=peercerts/carolCert.der

or

    rightcert=/usr/ssl/certs/carolCert.der


## Configuring the private keys - ipsec.secrets ##


### Loading private key files ###

strongSwan is able to load RSA (or ECDSA) private keys in the PKCS#1 or PKCS#8
file formats, or from PKCS#12 containers. The key files can optionally be
secured with a passphrase.

RSA private key files are declared in `/etc/ipsec.secrets` using the syntax

    : RSA <my keyfile> "<optional passphrase>"

The key file can be either in Base64 PEM-format or binary DER-format.  The
actual coding is detected automatically.  The example

    : RSA moonKey.pem

uses a pathname relative to the default directory

    /etc/ipsec.d/private

As an alternative an absolute pathname can be given as in

    : RSA /usr/ssl/private/moonKey.pem

In both cases make sure that the key files are root readable only.

Often a private key must be transported from the Certification Authority
where it was generated to the target security gateway where it is going
to be used.  In order to protect the key it can be encrypted with a symmetric
cipher using a transport key derived from a cryptographically strong
passphrase.

Once on the security gateway the private key can either be permanently
unlocked so that it can be used by the IKE daemon without having to know a
passphrase

    openssl rsa -in moonKey.pem -out moonKey.pem

or as an option the key file can remain secured.  In this case the passphrase
unlocking the private key must be added after the pathname in
`/etc/ipsec.secrets`

    : RSA moonKey.pem "This is my passphrase"

Some CAs distribute private keys embedded in a PKCS#12 file. strongSwan can read
private keys directly from such a file (end-entity and CA certificates are
also extracted):

    : P12 moonCert.p12 "This is my passphrase"


### Entering passphrases interactively ###

On a VPN gateway you would want to put the passphrase protecting the private
key file right into `/etc/ipsec.secrets` as described in the previous section,
so that the gateway can be booted in unattended mode.  The risk of keeping
unencrypted secrets on a server can be minimized by putting the box into a
locked room.  As long as no one can get root access on the machine the private
keys are safe.

On a mobile laptop computer the situation is quite different.  The computer can
be stolen or the user may leave it unattended so that unauthorized persons
can get access to it.  In theses cases it would be preferable not to keep any
passphrases openly in `/etc/ipsec.secrets` but to prompt for them interactively
instead.  This is easily done by defining

    : RSA moonKey.pem %prompt

Since strongSwan is usually started during the boot process, usually no
interactive console windows is available which can be used to prompt for
the passphrase.  This must be initiated by the user by typing

    ipsec secrets

which actually is an alias for the existing command

    ipsec rereadsecrets

and which causes a passphrase prompt to appear.  To abort entering a passphrase
enter just a carriage return.


## Configuring CA properties - ipsec.conf ##

Besides the definition of IPsec connections the `ipsec.conf` file can also
be used to configure a few properties of the certification authorities
needed to establish the X.509 trust chains.  The following example shows
some of the parameters that are currently available:

    ca strongswan
        cacert=strongswanCert.pem
        ocspuri=http://ocsp.strongswan.org:8880
        crluri=http://crl.strongswan.org/strongswan.crl'
        crluri2="ldap://ldap.strongswan.org/O=strongSwan, C=CH?certificateRevocationList"
        auto=add

In a similar way as `conn` sections are used for connection definitions, an
arbitrary number of optional `ca` sections define the basic properties of CAs.

Each ca section is named with a unique label

    ca strongswan

The only mandatory parameter is

    cacert=strongswanCert.pem

which points to the CA certificate which usually resides in the default
directory `/etc/ipsec.d/cacerts/` but could also be retrieved via an absolute
path name.

The OCSP URI

    ocspuri=http://ocsp.strongswan.org:8880

allows to define an individual OCSP server per CA.  Also up to two additional
CRL distribution points (CDPs) can be defined

    crluri=http://crl.strongswan.org/strongswan.crl'
    crluri2="ldap://ldap.strongswan.org/O=strongSwan, C=CH?certificateRevocationList"

which are added to any CDPs already present in the received certificates
themselves.

With the `auto=add` statement the `ca` definition is automatically loaded during
startup.  Setting `auto=ignore` will ignore the `ca` section.

Any parameters which appear in several ca definitions can be put in
a common `ca %default` section

    ca %default
        crluri=http://crl.strongswan.org/strongswan.crl'


## Monitoring functions ##

strongSwan offers the following monitoring functions:

| Command             | Action                                            |
|---------------------|---------------------------------------------------|
| ipsec listaacerts   | list all Authorization Authority certificates loaded from `/etc/ipsec.d/aacerts/` |
| ipsec listacerts    | list all X.509 attribute certificates loaded from `/etc/ipsec.d/acerts/` |
| ipsec listalgs      | list cryptographic algorithms for IKE             |
| ipsec listcacerts   | list all CA certificates loaded from `/etc/ipsec.d/cacerts/` or received via IKE |
| ipsec listcainfos   | list all properties defined in `ca` sections in `ipsec.conf` |
| ipsec listcerts     | list all certificates loaded via `leftcert` and `rightcert` |
| ipsec listcounters  | list global or connection specific counter values |
| ipsec listcrls      | list all CLRs loaded from `/etc/ipsec.d/crls/`    |
| ipsec listocsp      | list contents of the OCSP response cache          |
| ipsec listocspcerts | list all OCSP signer certificates loaded from `/etc/ipsec.d/ocspcerts/` or received in OCSP responses |
| ipsec listplugins   | list all loaded plugin features                   |
| ipsec listpubkeys   | list all raw public keys e.g. loaded via `leftsigkey` and `rightsigkey` |
| ipsec listall       | all the above commands combined                   |
| ipsec status        | list concise status information on established connections |
| ipsec statusall     | list detailed status information on connections |


## Firewall support functions ##


### Environment variables in the updown script ###

strongSwan makes the following environment variables available
in the _updown_ script indicated by the `leftupdown` option:

| Variable              | Example                   | Comment         |
|-----------------------|---------------------------|-----------------|
| $PLUTO_PEER_ID        | carol@strongswan.org      | RFC822_ADDR (1) |
| $PLUTO_PEER_PROTOCOL  | 17                        | udp         (2) |
| $PLUTO_PEER_PORT      | 68                        | bootpc      (3) |
| $PLUTO_MY_ID          | moon.strongswan.org       | FQDN        (1) |
| $PLUTO_MY_PROTOCOL    | 17                        | udp         (2) |
| $PLUTO_MY_PORT        | 67                        | bootps      (3) |

(1) $PLUTO_PEER_ID/$PLUTO_MY_ID contain the IDs of the two ends
    of an established connection. In our examples these
    correspond to the strings defined by `rightid` and `leftid`,
    respectively.

(2) $PLUTO_PEER_PROTOCOL/$PLUTO_MY_PROTOCOL contain the protocol
    defined by the `rightprotoport` and `leftprotoport` options,
    respectively. Both variables contain the same protocol value.
    The variables take on the value '0' if no protocol has been defined.

(3) $PLUTO_PEER_PORT/$PLUTO_MY_PORT contain the ports defined by
    the `rightprotoport` and `leftprotoport` options, respectively.
    The variables take on the value '0' if no port has been defined.

There are several more, refer to the provided default script for a documentation
of them.


### Automatic insertion and deletion of iptables firewall rules ###

The default `_updown` script automatically inserts and deletes dynamic
`iptables` firewall rules upon the establishment or teardown, respectively, of
an IPsec security association.  This feature is activated with the line

    leftfirewall=yes

If you define a `leftsubnet` with a netmask larger than `/32` then the
automatically inserted _FORWARD_ `iptables` rules will not allow clients to
access the internal IP address of the gateway even if it is part of that subnet
definition.  If you want additional _INPUT_ and _OUTPUT_ `iptables` rules to be
inserted, so that the host itself can be accessed then add the following line:

    lefthostaccess=yes

The `_updown` script also features a logging facility which will register the
creation (+) and the expiration (-) of each successfully established VPN
connection in a special syslog file in the following concise and easily
readable format:

    Jul 19 18:58:38 moon vpn:
        + carol.strongswan.org  192.168.0.100 -- 192.168.0.1 == 10.1.0.0/16
    Jul 19 22:15:17 moon vpn:
        - carol.strongswan.org  192.168.0.100 -- 192.168.0.1 == 10.1.0.0/16
