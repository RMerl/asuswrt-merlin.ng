getdns
======

# Overview of getdns

* GitHub:  <https://github.com/getdnsapi/getdns>

getdns is an implementation of a modern asynchronous DNS API; the specification was originally edited by Paul Hoffman.  It is intended to make all types of DNS information easily available to application developers and non-DNS experts.

## Why you might want getdns

Traditional access to DNS data from applications has several limitations:

* APIs require applications to have considerable sophistication about DNS data and data types

* Some kinds of data about the response (notably, the resource record set time to live) is not exposed via any API, so applications need to process raw protocol responses to get such data

* APIs are often blocking, meaning asynchronous access is not possible without some work

* Sophisticated uses of the DNS (things like IDNA and DNSSEC validation) require considerable application work, possibly by application developers with little experience with the vagaries of DNS.

getdns also provides an experimental DNS Privacy enabled client called 'stubby' - see below for more details.

## Motivation for providing the API

The developers are of the opinion that DNSSEC offers a unique global infrastructure for establishing and enhancing cryptographic trust relations.  With the development of this API we intend to offer application developers a modern and flexible interface that enables end-to-end trust in the DNS architecture, and which will inspire application developers to implement innovative security solutions in their applications.

### API Documentation

Note that this implementation offers additional functionality to supplement that in the [official getdns API](https://getdnsapi.net/documentation/spec/). Some additions are convenient utility functions but other functionality is experimental prior to be being recommended for inclusion in the official API.  The [Doxygen documentation](https://getdnsapi.net/doxygen/modules.html)  provides the details of the full API for this implementation.

## License

This implementation is licensed under the New BSD License (BSD-new).

Obtaining and getting started with getdns
=========================================
The project home page at [getdnsapi.net](https://getdnsapi.net) provides documentation, binary downloads, and news regarding the getdns API implementation.  This README file captures the goals and direction of the project and the current state of the implementation.

If you are just getting started with the library take a look at the section below that describes building and handling external dependencies for the library.

### Examples
Once it is built you should take a look at `spec/example` to see how the library is used.


# Download

Download the sources from our [github repo](https://github.com/getdnsapi/getdns) 
or from [getdnsapi.net](https://getdnsapi.net) and verify the download using
the checksums (SHA1 or MD5) or using gpg to verify the signature.  Our keys are
available from the [pgp keyservers](https://keyserver.pgp.com)

* `willem@nlnetlabs.nl`, key id E5F8F8212F77A498

# Releases

Release numbering follows the [Semantic Versioning](http://semver.org/)
approach.  The code is currently under active development.

The following requirements were met as conditions for the present release:

* code compiles cleanly on at least the primary target platforms: OSX, Linux (RHEL/CentOS, Ubuntu), FreeBSD
* examples must compile and run cleanly
* there must be clear documentation of supported and unsupported elements of the API

# External Dependencies

If you are installing from packages, you have to install the library and also the library-devel (or -dev) for your package management system to get the the necessary compile time files.

External dependencies are linked outside the getdns API build tree (we rely on CMake to find them).  We would like to keep the dependency tree short, see [Minimising Dependancies](#minimizing-dependancies) for more details.

Required for all builds:

* [libssl and libcrypto from the OpenSSL Project](https://www.openssl.org/) version 1.0.2 or later. Using OpenSSL 1.1 is recommended due to TSL 1.3 support.

Required for all builds that include recursive functionality:

* [libunbound from NLnet Labs](https://unbound.net/) version 1.5.9 or later. (Note: linking to libunbound is not yet supported on Windows, see [Windows 10](#microsoft-windows-10))

Required for all builds that include IDN functionality:

* [libidn2 from the FSF](https://www.gnu.org/software/libidn/) version 2.0.0 and higher.

Required to build the documentation:

* [Doxygen](http://www.doxygen.nl) is used to generate documentation; while this is not technically necessary for the build it makes things a lot more pleasant.

For example, to build on Ubuntu 18.04 or later, you would need the following packages for a full build:

    # apt install build-essential libunbound-dev libidn2-dev libssl-dev cmake

# Building

If you are building from git, you need to do the following before building:

    # git submodule update --init

From release 1.6.0 getdns uses CMake (previous versions used autoconf/libtool). To build from this release and later use:

    # cmake .
    # make

If you are unfamiliar with CMake, see our [CMake Quick Start](https://getdnsapi.net/quick-start/cmake-quick-start/) for how to use CMake options to customise the getdns build.

As well as building the getdns library two other tools are installed by default:

* getdns_query: a command line test script wrapper for getdns. This can be used to quickly check the functionality of the library, see (#using-getdnsquery)
* getdns_server_mon: test DNS server function and capabilities

Additionally `Stubby` a DNS Privacy enabled client can also be built and installed by using the `BUILD_STUBBY` option when running `cmake`, see [Stubby](#stubby).


## Minimizing dependencies

* getdns can be configured for stub resolution mode only with the `ENABLE_STUB_ONLY` option to `cmake`.  This removes the dependency on `libunbound`.
* Currently getdns only offers two helper functions to deal with IDN: `getdns_convert_ulabel_to_alabel` and `getdns_convert_alabel_to_ulabel`.  If you do not need these functions, getdns can be configured to compile without them by setting the`USE_LIBIDN2` option to `cmake` to OFF.
* When `ENABLE_STUB_ONLY` is ON, and `USE_LIBIDN2` is OFF, getdns has only one dependency left, which is OpenSSL.

## Extensions and Event loop dependencies

The implementation works with a variety of event loops, each built as a separate shared library.  See [this Doxygen page](https://getdnsapi.net/doxygen/group__eventloops.html) and [this man page](https://getdnsapi.net/documentation/manpages/#ASYNCHRONOUS USE) for more details.

* [libevent](http://libevent.org).  Note: the examples *require* this. libevent 2.x is required.
* [libuv](https://libuv.org/)
* [libev](http://software.schmorp.de/pkg/libev.html)

## Using getdns_query

Example test queries using `getdns_query` (pointed at Google Public DNS) and requesting the `call_reporting` extension which provides information on the transport and query time:

   getdns_query -s example.com A @8.8.8.8     +return_call_reporting (UDP)
   getdns_query -s example.com A @8.8.8.8 -T  +return_call_reporting (TCP)
   getdns_query -s example.com A @8.8.8.8 -L  +return_call_reporting (TLS without authentication)
   getdns_query -s getdnsapi.net A +dnssec_return_status +return_call_reporting (DNSSEC)

## Stubby

* Stubby is an implementation of a DNS Privacy enabled stub resolver that encrypts DNS queries using TLS. It is currently suitable for advanced/technical users - all feedback is welcome!
* Details on how to use Stubby can be found in the [Stubby Reference Guide](https://dnsprivacy.org/wiki/x/JYAT).
* Also see [dnsprivacy.org](https://dnsprivacy.org) for more information on DNS Privacy.

## Experimental support for GnuTLS

A project to allow user selection of either OpenSSL or GnuTLS is currently a work in progress. At present a user may select to use GnuTLS for the majority of the supported functionality, however, OpenSSL is still required for some cryptographic functions.

## Regression Tests

A suite of regression tests are included with the library, if you make changes or just
want to sanity check things on your system take a look at src/test.  You will need
to install [libcheck](https://libcheck.github.io/check/).  The check library is also available from many of the package repositories for the more popular operating systems.
Note: The tests currently do not run on Windows because of a dependancy on bash.

## DNSSEC dependencies

For the library to be DNSSEC capable, it needs to know the root trust anchor.
The library will try to load the root trust anchor from
`/etc/unbound/getdns-root.key` by default.  This file is expected to have one
or more `DS` or `DNSKEY` resource records in presentation (i.e. zone file)
format.  Note that this is different than the format of BIND.keys.

## Zero configuration DNSSEC

When the root trust anchor is not installed in the default location and a DNSSEC query is done, getdns will try to use the trust anchors published here: http://data.iana.org/root-anchors/root-anchors.xml .
It will validate these anchors with the ICANN Certificate Authority certificate following the procedure described in [RFC7958].
The `root-anchors.xml` and `root-anchors.p7s` S/MIME signature will be cached in the `$HOME/.getdns` directory on Unixes, and the `%appdata%\getdns` directory on Windows.

When using trust-anchors from the `root-anchors.xml` file, getdns will track the keys in the root DNSKEY rrset and store a copy in `$HOME/.getdns/root.key` on Unixes, and `%appdata%\getdns\root.key` on Windows.
Only when the KSK DNSKEY's change, a new version of `root-anchors.xml` is tried to be retrieved from [data.iana.org](https://data.iana.org/root-anchors/).

A installed trust-anchor from the default location (`/etc/unbound/getdns-root.key`) that fails to validate the root DNSKEY RRset, will also trigger the "Zero configuration DNSSEC" procedure described above.

Support
=======

## Mailing lists

We have a [getdns users list](https://lists.getdnsapi.net/mailman/listinfo/users) for this implementation.

## Tickets and Bug Reports

Tickets and bug reports should be reported via the [GitHub issues list](https://github.com/getdnsapi/getdns/issues).

Features of this release
========================

## Goals

The goals of this implementation of the getdns API are:

* Provide an open source implementation, in C, of the formally described getdns API by getdns API team at <https://getdnsapi.net/spec.html>
* Support FreeBSD, OSX, Linux (CentOS/RHEL, Ubuntu)
* Support Windows 10
* Include examples and tests as part of the build
* Document code using doxygen
* Leverage github as much as possible for project coordination
* Follow the BSD coding style/standards <ftp://ftp.netbsd.org/pub/NetBSD/NetBSD-current/src/share/misc/style>

Non-goals (things we will not be doing at least initially) include:

* implementation of the traditional DNS related routines (gethostbyname, etc.)

## Language Bindings

In parallel, the team is actively developing bindings for various languages.
For more information, visit this
[webpage](https://getdnsapi.net/bindings/).

## Unsupported getDNS Features

The following API calls are documented in getDNS but *not supported* by the implementation at this time:

* Detecting changes to resolv.conf and hosts
* MDNS, NIS and NetBIOS namespaces (only DNS and LOCALFILES are supported)

### Minor omissions

The following minor implementation omissions are noted:

Recursive mode does not support:
* TLS as a transport
* Non-zero connection idle timeouts or query pipelining
* Anything other than query_type and resolution_type in the return_call_reporting extension

Stub mode does not support:
* Non zero idle timeouts for synchronous calls

# Known Issues

* None

# Supported Platforms

The platforms listed here are intended to help ensure that we catch platform specific breakage prior to release.

* Ubuntu 18.04 LTS and newer LTS releases
* Microsoft Windows 10
* FreeBSD 11.3 and newer
* RHEL/CentOS 8
* OSX 10.14 and 10.15


###  Platform Specific Build Notes

[![Build Status](https://travis-ci.org/getdnsapi/getdns.png?branch=master)](https://travis-ci.org/getdnsapi/getdns)

## FreeBSD

If you're using [FreeBSD](https://www.freebsd.org/), you may install getdns via the [ports tree](https://www.freshports.org/dns/getdns/) by running: `cd /usr/ports/dns/getdns && make install clean`

If you are using FreeBSD 10 getdns can be intalled via 'pkg install getdns'.

## Ubuntu

getdns should also work on Ubuntu 16.04, however if you require IDN functionality you will have to install a recent version of libidn2 via a ppa e.g. from https://launchpad.net/~ondrej/+archive/ubuntu/php

You will also have to build Unbound from source code to provide libunbound at version >= 1.5.9.

## OSX

A self-compiled version of OpenSSL or the version installed via Homebrew is required and the options OPENSSL_ROOT_DIR, OPENSSL_CRYPTO_LIBRARY and OPENSSL_SSL_LIBRARY can be used to specify the location of the libraries.
Note: If using a self-compiled version, manual configuration of certificates into /usr/local/etc/openssl/certs is required for TLS authentication to work.

### Homebrew

If you're using [Homebrew](http://brew.sh/), you may run `brew install getdns`.  By default, this will only build the core library without any 3rd party event loop support.

To install the [event loop integration libraries](https://getdnsapi.net/doxygen/group__eventloops.html) that enable support for libevent, libuv, and libev, run: `brew install getdns --with-libevent --with-libuv --with-libev`.  All switches are optional.

Note that in order to compile the examples, the `--with-libevent` switch is required.

Additionally, getdns is linked against the the OpenSSL library installed by Homebrew. Note that the Homebrew OpenSSL installation clones the Keychain certificates to the default OpenSSL location so TLS certificate authentication should work out of the box.

## Microsoft Windows 10

You will need CMake for Windows. Installers can be downloaded from https://cmake.org/download/.

Windows versions of the following libraries are available using [the vcpkg package manager](https://docs.microsoft.com/en-us/cpp/build/vcpkg).

* OpenSSL
* libevent
* libiconv (required for libidn2)
* libidn2
* libyaml
* libuv

Once these are installed, set CMake variables CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH to the vcpkg include and library directories e.g. `../vcpkg/installed/x64-windows/include` and `../vcpkg/installed/x64-windows/lib`.

To generate a project suitable for use in Visual Studio, select the appropriate Visual Studio generator in CMake. Once generated, the cmake-gui Open Project button can be used to load the project into Visual Studio.

### Limitations on Windows

Full support for Windows is a work in progress. The following limitations will  be addresses in future:

* At present, no native Windows DLL version of libunbound exists; support for linking against libunbound is not currently available. The default build option for ENABLE_STUB_ONLY_ is ON for Windows.

* The getdns unit tests (built with `make test`) require libcheck which is not currently available for Windows and so cannot be built.

* The getdns tpkg test suite is not currently supported on Windows.

* The detection of the location of the `/etc/hosts` file should be optimised - it currently assumes Windows is installed in the default directory on the C: drive


Contributors
============
* Claus Assman
* Theogene Bucuti
* Andrew Cathrow, Verisign Labs
* Neil Cook
* Saúl Ibarra Corretgé
* Craig Despeaux, Verisign, Inc.
* John Dickinson, Sinodun
* Sara Dickinson, Sinodun
* Robert Edmonds
* Angelique Finan, Verisign, Inc.
* Simson Garfinkel
* Daniel Kahn Gillmor
* Neel Goyal, Verisign, Inc.
* Bryan Graham, Verisign, Inc.
* Robert Groenenberg
* Jim Hague, Sinodun
* Paul Hoffman
* Scott Hollenbeck, Verising, Inc.
* Christian Huitema
* Shumon Huque, Verisign Labs
* Jelte Janssen
* Guillem Jover
* Shane Kerr
* Anthony Kirby
* Olaf Kolkman, NLnet Labs
* Sanjay Mahurpawar, Verisign, Inc.
* Allison Mankin, Verisign, Inc. - Verisign Labs.
* Sai Mogali, Verisign, Inc.
* Linus Nordberg
* Benno Overeinder, NLnet Labs
* Joel Purra
* Tom Pusateri
* Prithvi Ranganath, Verisign, Inc.
* Hoda Rohani, NLnet Labs
* Rushi Shah, Verisign, Inc.
* Vinay Soni, Verisign, Inc.
* Melinda Shore, No Mountain Software LLC
* Bob Steagall, Verisign, Inc.
* Andrew Sullivan
* Ondřej Surý
* Willem Toorop, NLnet Labs
* Gowri Visweswaran, Verisign Labs
* Wouter Wijngaards, NLnet Labs
* Glen Wiley, Verisign, Inc.
* Paul Wouters


Acknowledgements
================
The development team explicitly acknowledges Paul Hoffman for his initiative and efforts to develop a consensus based DNS API. We would like to thank the participants of the getdns-api mailing list (discontinued) for their contributions.
