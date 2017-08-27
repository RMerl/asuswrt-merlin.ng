LDAP Configuration
==================

This document describes how to setup Freeradius on a Freebsd machine
using LDAP as a backend.  This is by no means complete and your
mileage may vary.  If you are having any problems with the setup of
your freeradius installation, please read the documentation that comes
with Freeradius first as that is where all the information for this
project came from.  If you find any bugs, typos, alternative ideas, or
just plain wrong information, please let me know by sending an email
to the address above.

The radius servers in this document are built on Freebsd 4.8, using
Freeradius .81 with OpenLDAP 2.0.27 as the backend. The servers are
designed to support customers for multiple services.  In this document
we will use regular dialup and dialup ISDN as examples of two
different services using the same radius server for authentication.

OVERVIEW
--------

The radius servers are to be provisioned by a some sort of system we
will call Billing.  Billing could simply be a script, a web front-end,
or an actual integration into a billing system. Billing will provision
to the master LDAP server.  The master LDAP server is running slurpd,
which will replicate all changes to the other radius servers.  Each
radius server will run a local instance of LDAP.

The radius servers will be accepting Radius auth packets and Radius
acct packets.  The accounting packets will be stored locally on each
radius server and then forwarded to the Accounting radius server,
using radrelay.  The Accounting radius server will store all the
radius information in some sort of database such as MySQL, Postgres,
or Oracle.  The configuration of the actual Accounting radius server
is outside the scope of this document.  Please refer to the freeradius
documentation for setting up that server.

The Accounting radius server will help to provide a searchable
interface to the accounting data for billing and usage purposes and
could allow a web front-end to be built for helpdesk/customer service
usage.  If that is not needed for your purposes, then disregard all
details about the Accounting radius server.

In order to make sure no data is lost in the event of the Accounting
radius server going down, the replication of data will take place
using radrelay.  Radrelay will do the equivalent of a tail on the
detail file and will continually attempt to duplicate each radius
packet that is stored in the detail file and send it off to the
recipient(s) specified.  Upon receipt of an accounting_response packet
radrelay will consider that packet completed and continue working on
the others.  Each radius server will also be storing its own copy of
all accounting packets that are sent to it.

Each NAS will be setup with a primary radius server and a failover
radius server.  We will spread the load among the group of radius
servers that we have so some are acting as a primary to some NAS's and
acting as a secondary to others.  In the event of a radius failure,
the NAS should failover to the backup radius server.  How to configure
this is dependent on the particular NAS being used.

::

            Will use Radius acct data              Billing will provision
                for real-time billing               out to the Master LDAP
                                                    server over LDAP
                    +------------+
                    | Accounting |                     +---------+
                    |   Radius   |                     | Billing |
                    +------------+                     +----+----+
                        /|\                                |
                        |                                 |
                        |                                 |
                        |                                 |
                        |                             Provisioning
                        |                               Message
                        |                                 |
                    Duplicate                              |
                        Acct                                |
                        |                                 |
                        |                                \|/
                        |                           +------------+
                        |        +------------------| LDAP Master|
                        |        |                  +------------+
                        |        |                        |
                        |      Slurpd             Slurpd Replication
                        |   Replication                   |
                        |        |                        |
                        |        |                       \|/
                        |        |                  +------------+
                        |        |                  |   Radius2  |
    The Radius servers    |        |                  | LDAP Slave |
    will create a local   |       \|/                 +------------+
    copy of all acct  +-------------+
    packets and then  |   Radius1   |
    fwd a copy back   | LDAP Slave  |        All Radius servers run a
    to accounting     +-------------+         local copy of LDAP for
                        /|\    /|\         Authorization and Authentication
                        |      |
                        |      |
                        |      |
                        |      |
                        Auth    Acct
                        |      |
                        |      |
                        |      |
                        |      |
                        |      |
                        \|/    \|/
                        +-----------+
                        |           |
                        |           |
                        |    NAS    |
                        |           |
                        +-----------+
                    The NAS will be setup to
                    use one of the Radius servers
                as primary and the others as failover


LDAP
----

The LDAP directory is designed to start with the top level of
dc=mydomain,dc=com.  The next level of the tree contains the different
services that will be stored within the ldap server.  For the radius
users, it will be specified as ou=radius.  Below ou=radius, will be
the different types of accounts.  For example, ou=users will store the
users and ou=profiles will store the default radius profiles.  The
profiles are entries that will be used to store group-wide radius
profiles.  The group ou=admins will be a place to enter the users for
Billing, Freeradius, and any other administrative accounts that are
needed.

::

                    +---------------------+
                    |                     |
                    |  Dc=mydomain,dc=com |Objectclass:organizationalUnit
                    |                     |Objectclass:dcObject
                    +---------------------+
                                |
                                |
                                \|/
                        +---------------+
                        |               |
                        |   Ou=radius   | Objectclass:organizationalUnit
                        |               |
                        +---------------+
                                |
        +-----------------------+-------------------------|
        |                       |                         |
        \|/                     \|/                       \|/
    +---------+           +---------------+         +-------------+
    |         |           |               |         |             |
    |Ou=users |           |  Ou=profiles  |         |  Ou=admins  |
    |         |           |               |         |             |
    +---------+           +---------------+         +------|------+
        |                       |                         |
        |                       |                         |
        \|/                      |                        \|/
    ----- Objectclass:        |                       ----- Objectclass:
    //     \\   radiusprofile   |                     //     \\     person
    |         |                  |                    |         |
    \\     //                   |                     \\     //
    -----                    \|/                      ----- Dn:cn=freeradius
    Dn: uid=example,ou=users,  -----  ObjectClass:         ou=admins,ou=radius
    dc=mydomain,dc=com       //     \\   radiusprofile      dc=mydomain,dc=com
                            |         |
                            |         |
                            \\     //
                            -----
                Dn: uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com


An example LDIF file is below.
NOTE:  There are unique radius attribute types and objectclasses, these will be
explained in the configuration section.

::

    dn: dc=mydomain,dc=com
    objectClass: dcObject
    objectClass: organizationUnit
    ou: Mydomain.com Radius
    dc: mydomain

    dn: ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: radius

    dn: ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: profiles

    dn: ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: users

    dn: ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: admins

    dn: uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: dial
    radiusServiceType: Framed-User
    radiusFramedProtocol: PPP
    radiusFramedIPNetmask: 255.255.255.0
    radiusFramedRouting: None

    dn: uid=isdn,ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: isdn
    radiusServiceType: Framed-User
    radiusFramedProtocol: PPP
    radiusFramedIPNetmask: 255.255.255.0
    radiusFramedRouting: None

    dn: uid=example,ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: radiusProfile
    uid: example
    userPassword: test
    radiusGroupName: dial
    radiusGroupName: isdn

    dn: cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: freeradius
    cn: freeradius
    userPassword: freeradius

    dn: cn=billing,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: freeradius
    cn: freeradius
    userPassword: billing

    dn: cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: replica
    cn: replica
    userPassword: replica

In order to configure the ldap server to understand the radius schema that we
are using, the attribute types and objectclasses must be defined in slapd.conf.
The file is included with the following line in slapd.conf::

    include         /usr/local/etc/openldap/schema/RADIUS-LDAPv3.schema

Below is the complete Schema::

    ----Begin RADIUS-LDAPv3.schema----

    #################################################
    ##### custom radius attributes ##################

    objectIdentifier myOID 1.1
    objectIdentifier mySNMP myOID:1
    objectIdentifier myLDAP myOID:2
    objectIdentifier myRadiusFlag myLDAP:1
    objectIdentifier myObjectClass myLDAP:2

    attributetype
        ( myRadiusFlag:1
        NAME 'radiusAscendRouteIP'
        DESC 'Ascend VSA Route IP'
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
        )

    attributetype
        (myRadiusFlag:2
        NAME 'radiusAscendIdleLimit'
        DESC 'Ascend VSA Idle Limit'
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
        )

    attributetype
        (myRadiusFlag:3
        NAME 'radiusAscendLinkCompression'
        DESC 'Ascend VSA Link Compression'
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
        )

    attributetype
        (myRadiusFlag:4
        NAME 'radiusAscendAssignIPPool'
        DESC 'Ascend VSA AssignIPPool'
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
        )


    attributetype
        (myRadiusFlag:5
        NAME 'radiusAscendMetric'
        DESC 'Ascend VSA Metric'
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
        )

    #################################################

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.1
        NAME 'radiusArapFeatures'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.2
        NAME 'radiusArapSecurity'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.3
        NAME 'radiusArapZoneAccess'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.44
        NAME 'radiusAuthType'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.4
        NAME 'radiusCallbackId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.5
        NAME 'radiusCallbackNumber'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.6
        NAME 'radiusCalledStationId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.7
        NAME 'radiusCallingStationId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.8
        NAME 'radiusClass'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.45
        NAME 'radiusClientIPAddress'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.9
        NAME 'radiusFilterId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.10
        NAME 'radiusFramedAppleTalkLink'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.11
        NAME 'radiusFramedAppleTalkNetwork'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.12
        NAME 'radiusFramedAppleTalkZone'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.13
        NAME 'radiusFramedCompression'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.14
        NAME 'radiusFramedIPAddress'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.15
        NAME 'radiusFramedIPNetmask'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.16
        NAME 'radiusFramedIPXNetwork'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.17
        NAME 'radiusFramedMTU'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.18
        NAME 'radiusFramedProtocol'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.19
        NAME 'radiusFramedRoute'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.20
        NAME 'radiusFramedRouting'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.46
        NAME 'radiusGroupName'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.47
        NAME 'radiusHint'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.48
        NAME 'radiusHuntgroupName'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.21
        NAME 'radiusIdleTimeout'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.22
        NAME 'radiusLoginIPHost'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.23
        NAME 'radiusLoginLATGroup'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.24
        NAME 'radiusLoginLATNode'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.25
        NAME 'radiusLoginLATPort'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.26
        NAME 'radiusLoginLATService'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.27
        NAME 'radiusLoginService'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.28
        NAME 'radiusLoginTCPPort'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.29
        NAME 'radiusPasswordRetry'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.30
        NAME 'radiusPortLimit'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.49
        NAME 'radiusProfileDn'
        DESC ''
        EQUALITY distinguishedNameMatch
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.12
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.31
        NAME 'radiusPrompt'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.50
        NAME 'radiusProxyToRealm'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.51
        NAME 'radiusReplicateToRealm'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.52
        NAME 'radiusRealm'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.32
        NAME 'radiusServiceType'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.33
        NAME 'radiusSessionTimeout'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.34
        NAME 'radiusTerminationAction'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.35
        NAME 'radiusTunnelAssignmentId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.36
        NAME 'radiusTunnelMediumType'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.37
        NAME 'radiusTunnelPassword'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.38
        NAME 'radiusTunnelPreference'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.39
        NAME 'radiusTunnelPrivateGroupId'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.40
        NAME 'radiusTunnelServerEndpoint'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.41
        NAME 'radiusTunnelType'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.42
        NAME 'radiusVSA'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.43
        NAME 'radiusTunnelClientEndpoint'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )


    #need to change asn1.id
    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.53
        NAME 'radiusSimultaneousUse'
        DESC ''
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.27
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.54
        NAME 'radiusLoginTime'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.55
        NAME 'radiusUserCategory'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.56
        NAME 'radiusStripUserName'
        DESC ''
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.7
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.57
        NAME 'dialupAccess'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.58
        NAME 'radiusExpiration'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
        SINGLE-VALUE
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.59
        NAME 'radiusCheckItem'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )

    attributetype
    ( 1.3.6.1.4.1.3317.4.3.1.60
        NAME 'radiusReplyItem'
        DESC ''
        EQUALITY caseIgnoreIA5Match
        SYNTAX 1.3.6.1.4.1.1466.115.121.1.26
    )


    objectclass
    ( 1.3.6.1.4.1.3317.4.3.2.1
        NAME 'radiusprofile'
        SUP top STRUCTURAL
        DESC ''
        MUST ( uid )
        MAY ( userPassword $
                radiusArapFeatures $ radiusArapSecurity $ radiusArapZoneAccess $
                radiusAuthType $ radiusCallbackId $ radiusCallbackNumber $
                radiusCalledStationId $ radiusCallingStationId $ radiusClass $
                radiusClientIPAddress $ radiusFilterId $ radiusFramedAppleTalkLink $
                radiusFramedAppleTalkNetwork $ radiusFramedAppleTalkZone $
                radiusFramedCompression $ radiusFramedIPAddress $
                radiusFramedIPNetmask $ radiusFramedIPXNetwork $
                radiusFramedMTU $ radiusFramedProtocol $
                radiusCheckItem $ radiusReplyItem $
                radiusFramedRoute $ radiusFramedRouting $ radiusIdleTimeout $
                radiusGroupName $ radiusHint $ radiusHuntgroupName $
                radiusLoginIPHost $ radiusLoginLATGroup $ radiusLoginLATNode $
                radiusLoginLATPort $ radiusLoginLATService $ radiusLoginService $
                radiusLoginTCPPort $ radiusLoginTime $ radiusPasswordRetry $
                radiusPortLimit $ radiusPrompt $ radiusProxyToRealm $
                radiusRealm $ radiusReplicateToRealm $ radiusServiceType $
                radiusSessionTimeout $ radiusStripUserName $
                radiusTerminationAction $ radiusTunnelAssignmentId $
                radiusTunnelClientEndpoint $ radiusIdleTimeout $
                radiusLoginIPHost $ radiusLoginLATGroup $ radiusLoginLATNode $
                radiusLoginLATPort $ radiusLoginLATService $ radiusLoginService $
                radiusLoginTCPPort $ radiusPasswordRetry $ radiusPortLimit $
                radiusPrompt $ radiusProfileDn $ radiusServiceType $
                radiusSessionTimeout $ radiusSimultaneousUse $
                radiusTerminationAction $ radiusTunnelAssignmentId $
                radiusTunnelClientEndpoint $ radiusTunnelMediumType $
                radiusTunnelPassword $ radiusTunnelPreference $
                radiusTunnelPrivateGroupId $ radiusTunnelServerEndpoint $
                radiusTunnelType $ radiusUserCategory $ radiusVSA $
                radiusExpiration $ dialupAccess $
                radiusAscendRouteIP $ radiusAscendIdleLimit $
                radiusAscendLinkCompression $
                radiusAscendAssignIPPool $ radiusAscendMetric )
    )
    ----End RADIUS-LDAPv3.schema----


Now we need to setup the permissions on the ldap server.  Notice above we
created three users in the admin ou.  These users will be specific for billing,
freeradius, and replication.

On the master ldap server, we will set the following permissions::

    access to attr=userPassword
            by self write
            by dn="cn=billing,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none

    access to *
            by self write
            by dn="cn=billing,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none

This will give the billing user write access to add/delete users.  For security
we will not give read access to any other users.  You can easily add another
read-only user to this setup if you want to build some sort of web interface to
do only reads.

Now on the slave ldap servers (aka the radius servers) we will setup the
following permissions::

    access to attr=userPassword
            by self write
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none

    access to dn="ou=users,ou=radius,dc=mydomain,dc=com"
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by dn="cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com" read
            by anonymous auth
            by * none

    access to *
            by self write
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none


This will give the replica user write access.  This user will be discussed
below and it is involved in the process of replicating the master server to the
slaves.  The freeradius user only needs read access to do the lookups for
authorization.

Now we will want to setup indexes to speed up searches.  At the minimum, below
will work.  Since all radius lookups are currently using the uid, we will want
to index that.  It is also a good idea to index the objectclass attribute.

# Indices to maintain
index   objectClass     eq
index   uid             eq

Now we need to setup the replication from the master to the slave servers.  To
do this, we will add the following to the slapd.conf file on the master:

On the master LDAP server::
    replica host=radius1.mydomain.com
    binddn=cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com
    bindmethod=simple credentials=replica

    replica host=radius2.mydomain.com
    binddn=cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com
    bindmethod=simple credentials=replica

We will need to add a replica for each slave LDAP server.  The binddn is the
name that is used to bind to the slave server, and the credentials is the
secret for that user.

On the slave LDAP servers::

    updatedn       cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com
    updateref       ldap://ldapmaster.mydomain.com

Those will determine what name is allowed to update the LDAP server and if an
update is attempted directly, what server to refer the update to.

RADIUS
------

The radius server is setup to use LDAP for both Authorization and
Authentication.  This section will describe what events will take place during
an AAA session with a NAS.  When the NAS sends a access_request to the radius
server, the radius server will perform authorization and authentication based
on a series of modules that are defined in radiusd.conf.  For example, the
module defined as ldap, will be used to make connections to the LDAP directory.

An example is listed below::

    ldap {
    server = localhost
    identity = cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com
    password = example
    #this is the basedn to do searches on a user
    basedn = ou=users,ou=radius,dc=mydomain,dc=com
    #notice the username is the stripped user-name or user-name
    filter = (uid=%{Stripped-User-Name:-{User-Name}})
    start_tls = no
    tls_mode = no
    #this maps ldap attributetypes to radius attributes
    dictionary_mapping = ${raddbdir}/ldap.attrmap
    ldap_cache_timeout = 120
    ldap_cache_size = 0
    ldap_connections_number = 10
    #password_header = {clear}
    #While integrating FreeRADIUS with Novell eDirectory, set
    #'password_attribute = nspmpassword' in order to use the universal password
    #of the eDirectory users for RADIUS authentication. This will work only if
    #FreeRADIUS is configured to build with --with-edir option.
    password_attribute = userPassword
    #Comment out the following to disable the eDirectory account policy check and
    #intruder detection. This will work only if FreeRADIUS is configured to build
    #with --with-edir option.
    #edir_account_policy_check=no
    groupname_attribute = radiusGroupName
    groupmembership_filter = (&(uid=%{Stripped-User-Name:-%{User-Name}})
    (objectclass=radiusprofile))
    groupmembership_attribute = radiusGroupName
    timeout = 3
    timelimit = 5
    net_timeout = 1
    compare_check_items = no
    #access_attr_used_for_allow = yes
    }

The first thing that is done is authorization of the user.  The radius server
will process the modules in the order specified in the authorization section of
radiusd.conf.  Currently, they are in the following order.

1) preprocess
2) suffix
3) files
4) ldap

The first module will be preprocess.  This will first check the huntgroups of
the user coming in.  The huntgroups are defined in the file huntgroups and they
are a group listing of the NAS-IP-Addresses that make the access_request.  This
is useful in creating specific actions based on the NAS-IP that the request is
made from.  An example, is below::

    isdncombo       NAS-IP-Address == 10.10.10.1
    dialup          NAS-IP-Address == 10.10.10.2
    dialup          NAS-IP-Address == 10.10.10.3

We will have one NAS that is used for both ISDN and regular dialup customers,
the other NAS's will be only used for dialup.

The preprocess module may also use the hints file, to load hints to the radius
server, and add additional hacks that are based on the type of request that
comes in.  This is to help with certain NAS's that don't conform to radius
RFC's.  Check the comments in radiusd.conf for an explanation on those.

The second module is suffix.  This event will determine which realm the user is
in, based on the User-Name attribute.  It is currently setup to split the
username at the occurence of the @symbol.  For example, the username of
example@mydomain.com, will be split into example and mydomain.com.  The realm
is then checked against the file proxy.conf, which will determine what actions
should be taken for that realm.  Certain realms can be setup to be proxied to a
different radius server or set to authenticate locally.  Also, the username can
be setup to be stripped from the realm or left intact.  An example of
proxy.conf, is listed below.  If the realm is to be proxied, then a secret is
needed, which is the secret of the radius server it is to be proxied to.
By default the User-Name will be stripped, unless the nostrip option is set.

Currently we will not be using realms with our users, but adding this ability
in the future will be much easier with already incorporating proxy.conf into the
setup::

    proxy server {
            synchronous = no
            retry_delay = 5
            retry_count = 3
            dead_time = 120
            servers_per_realm = 15
            default_fallback = yes
    }

    realm NULL {
            type            = radius
            authhost        = LOCAL
            accthost        = LOCAL
            #secret         = testing123
    }

    realm DEFAULT {
            type            = radius
            authhost        = LOCAL
            accthost        = LOCAL
            #secret         = testing123
    }

The next module is files, which is commonly know as the users file.  The users
file will start with either a username to determine how to authorize a specific
user, or a DEFAULT setting.  In each line it will define what items must be
present for there to be a match in the form of attribute == value.  If all the
required attributes are matched, then attributes specified with attribute :=
value will be set for that user.  If no match is found the users file will
continue to be processed until there is a match.  The last DEFAULT setting will
be set as a catch-all, in case there is no previous match.  If a match is made,
the statement of Fall-Through determines if the users file should continue to
be processed or if it should stop right there.

The Ldap-Group corresponds to the LDAP attribute of radiusGroupName (see ldap
configuration above).  The user may be assigned multiple radiusGroupNames, one
for each of the services that the user is authorized for.  If the user does
belong to the correct group, then the user will be authorized for that type of
access.  If the user does not belong to that group, then there will not be a
match and the users file will continue to be processed.  If a match is made and
there is a User-Profile set, then the radius server will lookup the attributes
that exist in that User-Profile in the LDAP directory.  These are radius
attributes that will be sent to the NAS as a reply-item.

An example users file is below::

    DEFAULT Ldap-Group == disabled, Auth-Type := Reject
            Reply-Message = "Account disabled.  Please call the helpdesk."

    DEFAULT Huntgroup-Name == isdncombo, NAS-Port-Type == Async, Ldap-Group == dial,
    User-Profile := "uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com"
            Fall-Through = no

    DEFAULT Huntgroup-Name == isdncombo, NAS-Port-Type == ISDN, Ldap-Group == isdn,
    User-Profile := "uid=isdn,ou=profiles,ou=radius,dc=mydomain,dc=com"
            Fall-Through = no

    DEFAULT Huntgroup-Name == dial, Ldap-Group == dial,
    User-Profile := "uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com"
            Fall-Through = no

    DEFAULT Auth-Type := Reject
            Reply-Message = "Please call the helpdesk."

Notice that the catchall DEFAULT is set to Reject the user.  This will stop the
authorization and immediately send back an access_reject message.  Because
business rules are applied above to each scenario where the user will be
authorized for access, if no match is found, then we will want to stop the
process immediately to save resources.

By using the Ldap-Group feature we can limit user logins to only the services
they are subscribed to.  Some examples of possible user setups are below::

    #user with access to dial-up
    dn: uid=user1,ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: user1
    userPassword: whatever
    radiusgroupname: dial

    #user with access to ISDN and dial
    dn: uid=user2,ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: user2
    userPassword: whatever
    radiusgroupname: dial
    radiusgroupname: isdn

    #same user as above that was suspended for not paying
    dn: uid=user2,ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: user2
    userPassword: whatever
    radiusgroupname: dial
    radiusgroupname: isdn
    radiusgroupname: disabled

Now that we have authorized the user, the final piece is to authenticate the
user. Authentication is currently done by checking if the password sent in the
access_request packet is correct.  This action will be done with an attempted
bind to the LDAP server using the User-Name and User-Password attributes
passed to it from the access_request.  If the user is successfully authorized,
then an access_accept message will be sent back to the NAS, with any reply
items that were defined in the authorization section.  If the user did not
supply the correct password, then an access_reject message will be sent to the
user.

If the NAS is sent an access_accept packet then the user will be given access
to the service and the NAS will then send an acct_request packet.  This will be
a request packet to start a radius accounting session.  The way the server will
log the accounting packets is determined in the detail module in the
radiusd.conf file.  Since we will be storing a local copy and forwarding on all
accounting to the Accounting radius server, we will store two local copies on
the machine.  The first one is done in a regular detail file as defined in the
following::

    detail detail1 {
        filename = ${radacctdir}/%{Client-IP-Address}/detail-%Y%m%d
        permissions = 0600
        dir_permissions = 0755
    }

The second detail file will be used by the program radrelay to relay a copy of
all accounting packets to the Accounting radius server.  This file is stored as
a catchall for all accounting packets.  The radrelay program will basically do
a tail on that file and will then attempt to send a copy of each addition to it
to the Accounting server.  If the copy is successfully sent, then it will be
deleted from this file.  If the Accounting server were to go down, then this
file will continue to build up entries.  As soon as the Accounting server is
back online, an attempt to re-send the packets to the Accounting server will
made.  This file is defined in the following section of radiusd.conf::

    detail detail2 {
    	filename = ${radacctdir}/detail-combined
        permissions = 0600
        dir_permissions = 0755
        locking = yes
    }

INSTALLATION
------------

The new radius servers are currently built on Freebsd 4.8. As the version may
eventually change, these instructions may no longer apply. The steps for
building the server are the following:

* Install FreeBSD
* Install other FreeBSD items
* Install OpenLDAP *NOTE: this must be done before installing Freeradius*
* Install FreeRadius

Under the assumption that FreeBSD is already installed and the kernel rebuilt
to the specifications needed for the machine, there are several other things
that may be needed at this time and the purpose of this is just as a reminder.

install cvsup-without-gui from the ports collection

run cvsup on all to update the ports to the most recent versions

might be a good idea to upgrade the src

edit and run cvsup on /usr/share/examples/cvsup/standard-supfile

cd /usr/src - vi Makefile and follow instructions

install sendmail from ports to keep up to date with the most recent versions.
In the ports collection /ports/mail/sendmail run make; make install; make
mailer.conf.  Then edit rc.conf and change to sendmail_enable=NO
radius servers only need the local interface to send daily reports

edit rc.conf to make sure inetd_enable=NO

no reason to have extra services running

if you rebuilt the kernel to add support for IPFIREWALL, then remember to add a
firewall rule to rc.conf

firewall_enable=YES
firewall_type=OPEN (or actually create a real firewall rule)

add crontab to keep date accurate for accounting::

    15 03 * * * /usr/sbin/ntpdate -s thetimeserver.mydomain.com

install openldap from ports

download the freeradius source as the ports collection is often outdated
the default settings are /usr/local/etc/raddb, /var/log/radius.log, /var/log/radacct

since openldap was installed first, you should not need any special flags to
add ldap support

Now its time to configure openlap and freeradius.  First we will be looking at
configuring OpenLDAP


copy RADIUS-LDAPv3.schema to /usr/local/etc/openldap/schema

edit /usr/local/etc/openldap/slapd.conf

::

    ----Begin slapd.conf----
    # $OpenLDAP: pkg/ldap/servers/slapd/slapd.conf,v 1.23.2.7 2003/03/24 03:54:12
    #kurt Exp $
    #
    # See slapd.conf(5) for details on configuration options.
    # This file should NOT be world readable.
    #
    include		/usr/local/etc/openldap/schema/core.schema
    include		/usr/local/etc/openldap/schema/RADIUS-LDAPv3.schema

    # Define global ACLs to disable default read access.

    # Do not enable referrals until AFTER you have a working directory
    # service AND an understanding of referrals.
    #referral	ldap://root.openldap.org

    loglevel	296

    pidfile		/var/run/slapd.pid
    argsfile	/var/run/slapd.args

    # Load dynamic backend modules:
    # modulepath	/usr/local/libexec/openldap
    # moduleload	back_bdb.la
    # moduleload	back_ldap.la
    # moduleload	back_ldbm.la
    # moduleload	back_passwd.la
    # moduleload	back_shell.la

    password-hash		{SSHA}

    access to attr=userPassword
            by self write
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none

    access to dn="ou=users,ou=radius,dc=mydomain,dc=com"
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by dn="cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com" read
            by anonymous auth
            by * none

    access to *
            by self write
            by dn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com" write
            by anonymous auth
            by * none


    #######################################################################
    # ldbm database definitions
    #######################################################################

    database	bdb
    suffix		"dc=mydomain,dc=com"
    rootdn		"cn=root,dc=mydomain,dc=com"
    # Cleartext passwords, especially for the rootdn, should
    # be avoid.  See slappasswd(8) and slapd.conf(5) for details.
    # Use of strong authentication encouraged.
    rootpw		{SSHA}Eu5EwPxTrwhEGrXQ9SaQZyfpu4iHt3NP
    # The database directory MUST exist prior to running slapd AND
    # should only be accessible by the slapd and slap tools.
    # Mode 700 recommended.
    directory	/var/db/openldap-data
    # Indices to maintain
    index	objectClass	eq
    index	uid		eq
    mode			0600
    cachesize		2000

    # replica one for each
    #replica host=radius1.mydomain.com
    #	binddn="cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com"
    #	bindmethod=simple credentials=secret

    replogfile	/var/db/openldap-slurp/replog

    ## REMEMBER TO ADD THIS TO THE SLAVES
    updatedn	"cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com"
    updateref	ldap://ldapmaster.mydomain.com
    ----End slapd.conf----


To create a rootdn that is not stored in plain text, enter the following command::

    $ slappasswd

it will ask for password and verification::

    New password:
    Re-enter new password::

while in the shell create the directory for the ldap database, this must be created before slapd can start::

    $ mkdir /var/db/openldap-data

move the slapd.sh.sample file to slapd.sh in /usr/local/etc/rc.d::

    $ mv /usr/local/etc/rc.d/slapd.sh.sample slapd.sh

enable logging in /etc/syslog.conf by adding the following::

    local4.*            /var/log/ldap.log
    restart syslogd

start it up on both the master and slave ldap servers::

    $ /usr/local/etc/rc.d/slapd start

create the structural ldif, schema.ldif::

    ----Begin schema.ldif----
    dn: dc=mydomain,dc=com
    objectClass: dcObject
    objectClass: organizationUnit
    ou: Mydomain.com Radius
    dc: mydomain

    dn: ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: radius

    dn: ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: profiles

    dn: ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: users

    dn: ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: organizationalunit
    ou: admins

    dn: uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: dial
    radiusServiceType: Framed-User
    radiusFramedProtocol: PPP
    radiusFramedIPNetmask: 255.255.255.0
    radiusFramedRouting: None

    dn: uid=isdn,ou=profiles,ou=radius,dc=mydomain,dc=com
    objectclass: radiusprofile
    uid: isdn
    radiusServiceType: Framed-User
    radiusFramedProtocol: PPP
    radiusFramedIPNetmask: 255.255.255.0
    radiusFramedRouting: None

    dn: uid=example,ou=users,ou=radius,dc=mydomain,dc=com
    objectclass: radiusProfile
    uid: example
    userPassword: test
    radiusGroupName: dial
    radiusGroupName: isdn

    dn: cn=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: freeradius
    cn: freeradius
    userPassword: freeradius

    dn: cn=billing,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: freeradius
    cn: freeradius
    userPassword: billing

    dn: cn=replica,ou=admins,ou=radius,dc=mydomain,dc=com
    objectclass: person
    sn: replica
    cn: replica
    userPassword: replica
    ----End schema.ldif----

add the organizational structure to the master ldap database::

    $ ldapadd -D uid=billing,ou=admins,ou=radius,dc=mydomain,dc=com -w billing -f
    schema.ldif -h ldapmaster.mydomain.com

run slapcat to see what the directory looks like::

    $ slapcat

If all went well the LDAP directory should be up and running and propagated to
the slaves.  Now you can add your users to the master.

Now its time to setup FreeRadius.  First cd into /usr/local/etc/raddb and take
a look at all the configuration files, they are heavily documented so you may
wish to read through them all before making and changes.


edit radiusd.conf::

    ----Begin radiusd.conf----
    ##
    ## radiusd.conf	-- FreeRADIUS server configuration file.
    ##

    prefix = /usr/local
    exec_prefix = ${prefix}
    sysconfdir = /usr/local/etc/raddb
    localstatedir = ${prefix}/var
    sbindir = ${exec_prefix}/sbin
    logdir = /var/log
    raddbdir = /usr/local/etc/raddb
    radacctdir = /var/log/radacct

    #  Location of config and logfiles.
    confdir = ${raddbdir}
    run_dir = ${localstatedir}/run/radiusd
    log_file = ${logdir}/radius.log
    libdir = ${exec_prefix}/lib
    pidfile = ${run_dir}/radiusd.pid

    #user = nobody
    #group = nobody

    max_request_time = 30
    delete_blocked_requests = no
    cleanup_delay = 5
    max_requests = 0
    bind_address = *
    port = 0
    hostname_lookups = no
    allow_core_dumps = no
    log_stripped_names = no
    log_auth = no
    log_auth_badpass = no
    log_auth_goodpass = no

    #  The program to execute to do concurrency checks.
    #checkrad = ${sbindir}/checkrad

    security {
            max_attributes = 200
            reject_delay = 0
            status_server = no
    }

    proxy_requests  = yes
    $INCLUDE  ${confdir}/proxy.conf

    $INCLUDE  ${confdir}/clients.conf

    thread pool {
            start_servers = 5
            max_servers = 32
            min_spare_servers = 3
            max_spare_servers = 10
            max_requests_per_server = 0
    }

    modules {

            ldap {
            server = "localhost"
            identity = "uid=freeradius,ou=admins,ou=radius,dc=mydomain,dc=com"
            password = example
            basedn = "ou=users,ou=radius,dc=mydomain,dc=com"
            filter = "(&(uid=%{Stripped-User-Name:-%{User-Name}})
    (objectclass=radiusprofile)"
            start_tls = no
            tls_mode = no
            #default_profile = "uid=dial,ou=profiles,ou=radius,dc=mydomain,dc=com"
            #profile_attribute = "radiusProfileDn"
            dictionary_mapping = ${raddbdir}/ldap.attrmap
            ldap_cache_timeout = 120
            ldap_cache_size = 0
            ldap_connections_number = 10
            #password_header = "{clear}"
            password_attribute = userPassword
            groupname_attribute = radiusGroupName
            groupmembership_filter = "(&(uid=%{Stripped-User-Name:-%{User-Name}}))
    (objectclass=radiusProfile)"
            groupmembership_attribute = radiusGroupName
            timeout = 3
            timelimit = 5
            net_timeout = 1
            compare_check_items = no
            #access_attr_used_for_allow = yes
            }

            realm suffix {
                    format = suffix
                    delimiter = "@"
            }

            preprocess {
                    huntgroups = ${confdir}/huntgroups
                    #hints = ${confdir}/hints
                    with_ascend_hack = no
                    ascend_channels_per_line = 23
                    with_ntdomain_hack = no
                    with_specialix_jetstream_hack = no
                    with_cisco_vsa_hack = no
            }

            files {
                    usersfile = ${confdir}/users
                    #acctusersfile = ${confdir}/acct_users
                    compat = no
                    #use old style users
            }
            # regular detail files
            detail detail1 {
                    filename = ${radacctdir}/%{Client-IP-Address}/detail-%Y%m%d
                    permissions = 0600
                    dir_permissions = 0755
            }
            # temp detail file to replicate to accountrad
            detail detail2 {
                    filename = ${radacctdir}/detail-combined
                    permissions = 0600
                    dir_permissions = 0755
                    locking = yes
            }

            #radutmp {
            #	filename = ${logdir}/radutmp
            #	permissions =  0600
            #	caller_id = "yes"
            #}

            #radutmp sradutmp {
            #	filename = ${logdir}/sradutmp
            #	permissions =  0644
            #	caller_id = "no"
            #}

            #attr_filter {
            #	attrsfile = ${confdir}/attrs
            #}


            # The "always" module is here for debugging purposes. Each
            # instance simply returns the same result, always, without
            # doing anything.
            always fail {
                    rcode = fail
            }
            always reject {
                    rcode = reject
            }
            always ok {
                    rcode = ok
                    simulcount = 0
                    mpp = no
            }

            #
            #  The 'expression' module current has no configuration.
            expr {
            }

    }

    instantiate {
            expr
    }

    authorize {
            preprocess
            suffix
            files
            ldap
    }

    authenticate {
            authtype LDAP {
                    ldap
            }
    }

    preacct {
            preprocess
            suffix
            files
    }

    accounting {
            acct_unique
            detail1
            detail2
            #radutmp
            #sradutmp
    }


    #session {
            #radutmp
    #}

    #post-auth {
            #  Get an address from the IP Pool.
            #main_pool
    #}
    ----End radiusd.conf----


edit huntgroups to specify a NAS to a huntgroup::

    ----Begin huntgroups----
    # dialup and isdn
    isdncombo	NAS-IP-Address == 10.10.10.1

    # just dialup
    dialup		NAS-IP-Address == 10.10.10.2
    dialup		NAS-IP-Address == 10.10.10.3
    ----End huntgroups----

* edit proxy.conf to setup the different realms::

    ----Begin proxy.conf----
    proxy server {
            synchronous = no
            retry_delay = 5
            retry_count = 3
            dead_time = 120
            servers_per_realm = 15
            default_fallback = yes
    }

    realm NULL {
            type		= radius
            authhost        = LOCAL
            accthost        = LOCAL
            #secret		= testing123
    }

    realm DEFAULT {
            type		= radius
            authhost        = LOCAL
            accthost        = LOCAL
            #secret		= testing123
    }
    ----End proxy.conf----

    -edit clients.conf to setup the NAS's that can talk to it


    ----Begin clients.conf----
    client 127.0.0.1 {
            secret		= example
            shortname	= localhost
            nas_type     	= other
    }


    # isdn and dialup nas
    client 10.10.10.1 {
            secret		= example
            shortname	= isdn
            nas_type		= cisco
    }

    #dialup only
    client 10.10.10.2 {
            secret		= example
            shortname	= dialup1
            nas_type		= cisco
    }

    client 10.10.10.3 {
            secret		= example
            shortname	= dialup2
            nas_type		= cisco
    }
    ----End clients.conf----


You may wish to look at the other files, but they should all be OK by default.

create startup files in /usr/local/etc/rc.d

radiusd.sh - the radiusd startup file::

    ----Begin radiusd.sh----
    #!/bin/sh
    case "$1" in
    start)
            /usr/local/sbin/radiusd
            echo -n ' radiusd'
            ;;
    stop)
            if [ -f /usr/local/var/run/radiusd/radiusd.pid ]; then
                    kill -TERM `cat /usr/local/var/run/radiusd/radiusd.pid`
                    rm -f /usr/local/var/run/radiusd/radiusd.pid
                    echo -n ' radiusd'
            fi
            ;;
    restart)
            if [ -f /usr/local/var/run/radiusd/radiusd.pid ]; then
                    kill -HUP `cat /usr/local/var/run/radiusd/radiusd.pid`
                    echo 'radiusd restarted'
            fi
            ;;
    *)
            echo "Usage: ${0##*/}: { start | stop | restart }" 2>&1
            exit 65
            ;;
    esac
    ----End radiusd.sh----

radrelay.sh - the radrelay startup file::


    ----Begin radrelay.sh----
    #!/bin/sh
    case "$1" in

    start)
        /usr/local/bin/radrelay -a /var/log/radacct -d /usr/local/etc/raddb \
        -S /usr/local/etc/raddb/radrelay_secret -f -r accounting.mydomain.com:1813 \
    detail-combined
    echo -n ' radrelay started'
    ;;


    stop)
    /usr/bin/killall radrelay
    echo ' radrelay stopped'
    ;;

    *)
    echo "Usage: $[0##*/}: { start | stop }" 2>&1
    exit 65
    ;;

    esac
    ----End radrelay.sh----

create radrelay_secret in /usr/local/etc/radddb
This file will contain the secret to connect to the Accounting radius server::

    ----Begin radrelay_secret----
    example
    ----End radrelay_secret----

Now fire them up::
    $ /usr/local/etc/rc.d/radiusd.sh start
    $ /usr/local/etc/rc.d/radrelay.sh start

You should be all set to start testing now.

OTHER RANDOM NOTES AND THOUGHTS
-------------------------------

The client programs used to connect to the ldap directory are:

ldapadd:
    to add a record
ldapmodify:
    to modify a record
ldapdelete:
    to delete a record
ldapsearch:
    to search for a record
slapcat:
    to show the entire directory
slappaswd:
    to generate a crypted password

Read the man pages on those commands, they tell you everything you
need to know.

They all follow this basic syntax::

    $ ldapwhatever -D "uid=someone,ou=admins,ou=radius,dc=mydomain,dc=com" -w thesecret -andthenotherstuff

Finally, if you are having trouble with LDAP, run it in debug mode by
changing the following in slapd.sh::

    slapd_args=

to::

    slapd_args= '-d 3'

There is a program included with freeradius to test the radius server,
its called radclient.  Typing it alone will tell you all the options.
You will need to create a file that contains radius attributes, such
as::

    User-Name = example
    User-Password = test
    Service-Type = Framed-User
    NAS-IP-Address = 10.10.10.1
    NAS-Port-Type = Async

Then you fire that radius packet at the server by issuing::

    $ radclient -f testradiusfile localhost auth thesecret

-f = filename
localhost is the server you are hitting
auth or acct depending on the type of packet
thesecret to connect to that server

Finally, if you are having trouble you can run radius in debug mode
and it will output everything that happens to the screen.  To do that,
kill the current process and run::

    $ radiusd -X


LINKS
-----

FREERADIUS
++++++++++

* _`FreeRADIUS`: http://www.freeradius.org
* _`FreeRADIUS Documentation`: http://www.freeradius.org/radiusd/doc
* _`FreeRADIUS Wiki`: http://wiki.freeradius.org/

OPENLDAP
++++++++

* _`OpenLDAP`: http://www.openldap.org
* _`OpenLDAP Administrator's Guide`: http://www.openldap.org/doc/admin21

RFCs
++++

* _`RFC2865: RADIUS Authentication`: http://www.freeradius.org/radiusd/doc/rfc/rfc2865.txt
* _`RFC2866: RADIUS Accounting`: http://www.freeradius.org/radiusd/doc/rfc/rfc2866.txt
* _`RFC2869: RADIUS Extentions`: http://www.freeradius.org/radiusd/doc/rfc/rfc2869.txt
* _`RFC2251: LDAP v3`: http://www.ietf.org/rfc/rfc2251.txt
* _`RFC2252: LDAP v3 Attribute Syntax Definitions`: http://www.ietf.org/rfc/rfc2252.txt
* _`RFC2253: LDAP UTF-8 String Representation of Distinguishe d Names (DNs)`: http://www.ietf.org/rfc/rfc2252.txt
* _`RFC2849: LDAP Data Interchange Fromat (LDIFs)`: http://www.ietf.org/rfc/rfc2849.txt
* _`RFC3377: LDAP v3 Technical Specs`: http://www.ietf.org/rfc/rfc3377.txt
