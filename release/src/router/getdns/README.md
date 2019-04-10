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
Once it is built you should take a look at src/examples to see how the library is used.


# Download

Download the sources from our [github repo](https://github.com/getdnsapi/getdns) 
or from [getdnsapi.net](https://getdnsapi.net) and verify the download using
the checksums (SHA1 or MD5) or using gpg to verify the signature.  Our keys are
available from the [pgp keyservers](https://keyserver.pgp.com)

* willem@nlnetlabs.nl, key id E5F8F8212F77A498

# Releases

Release numbering follows the [Semantic Versioning](http://semver.org/)
approach.  The code is currently under active development.

The following requirements were met as conditions for the present release:

* code compiles cleanly on at least the primary target platforms: OSX, RHEL/CentOS Linux, FreeBSD
* examples must compile and run cleanly
* there must be clear documentation of supported and unsupported elements of the API

# Building and External Dependencies

If you are installing from packages, you have to install the library and also the library-devel (or -dev) for your package management system to get the the necessary compile time files.  

External dependencies are linked outside the getdns API build tree (we rely on configure to find them).  We would like to keep the dependency tree short.  Please refer to section for building on Windows for separate dependency and build instructions for that platform.

* [libunbound from NLnet Labs](https://unbound.net/) version 1.4.16 or later.
* [libidn from the FSF](https://www.gnu.org/software/libidn/) version 1 or 2 (from version 2.0.0 and higher).  (Note that the libidn version means the conversions between A-labels and U-labels may permit conversion of formally invalid labels under IDNA2008.)
* [libssl and libcrypto from the OpenSSL Project](https://www.openssl.org/) version 0.9.7 or later. (Note: version 1.0.1 or later is required for TLS support, version 1.0.2 or later is required for TLS hostname authentication)
* Doxygen is used to generate documentation; while this is not technically necessary for the build it makes things a lot more pleasant.

For example, to build on a recent version of Ubuntu, you would need the following packages:

    # apt install build-essential libunbound-dev libidn2-dev libssl-dev libtool m4 autoconf

If you are building from git, you need to do the following before building:


    # git submodule update --init

    # libtoolize -ci # (use glibtoolize for OS X, libtool is installed as glibtool to avoid name conflict on OS X)
    # autoreconf -fi


As well as building the getdns library three other tools may be installed:

* getdns_query: a command line test script wrapper for getdns
* stubby: an experimental DNS Privacy enabled client
* getdns_server_mon: test DNS server function and capabilities

Note: If you only want to build stubby, then use the `--with-stubby` option when running 'configure'.


## Minimizing dependencies

* getdns can be configured for stub resolution mode only with the `--enable-stub-only` option to configure.  This removes the dependency on `libunbound`.
* Currently getdns only offers two helper functions to deal with IDN: `getdns_convert_ulabel_to_alabel` and `getdns_convert_alabel_to_ulabel`.  If you do not need these functions, getdns can be configured to compile without them with the `--without-libidn` and `--without-libidn2` options to configure.
* When `--enable-stub-only`, `--without-libidn` and `--without-libidn2` options are used, getdns has only one dependency left, which is OpenSSL.

## Extensions and Event loop dependencies

The implementation works with a variety of event loops, each built as a separate shared library.  See [this Doxygen page](https://getdnsapi.net/doxygen/group__eventloops.html) and [this man page](https://getdnsapi.net/documentation/manpages/#ASYNCHRONOUS USE) for more details.

* [libevent](http://libevent.org).  Note: the examples *require* this and should work with either libevent 1.x or 2.x.  2.x is preferred.
* [libuv](https://github.com/joyent/libuv)
* [libev](http://software.schmorp.de/pkg/libev.html)

## Stubby

* Stubby is an experimental implementation of a DNS Privacy enabled stub resolver than encrypts DNS queries using TLS. It is currently suitable for advanced/technical users - all feedback is welcome! 
* Details on how to use Stubby can be found in the [Stubby Reference Guide](https://dnsprivacy.org/wiki/x/JYAT).
* Also see [dnsprivacy.org](https://dnsprivacy.org) for more information on DNS Privacy.

## Regression Tests

A suite of regression tests are included with the library, if you make changes or just
want to sanity check things on your system take a look at src/test.  You will need
to install [libcheck](https://libcheck.github.io/check/).  The check library is also available from many of the package repositories for the more popular operating systems.

## DNSSEC dependencies

For the library to be DNSSEC capable, it needs to know the root trust anchor.
The library will try to load the root trust anchor from
`/etc/unbound/getdns-root.key` by default.  This file is expected to have one
or more `DS` or `DNSKEY` resource records in presentation (i.e. zone file)
format.  Note that this is different than the format of BIND.keys.

##$ Zero configuration DNSSEC

When the root trust anchor is not installed in the default location and a DNSSEC query is done, getdns will try to use the trust anchors published here: http://data.iana.org/root-anchors/root-anchors.xml .
It will validate these anchors with the ICANN Certificate Authority certificate following the procedure described in [RFC7958].
The `root-anchors.xml` and `root-anchors.p7s` S/MIME signature will be cached in the `$HOME/.getdns` directory on Unixes, and the `%appdata%\getdns` directory on Windows.

When using trust-anchors from the `root-anchors.xml` file, getdns will track the keys in the root DNSKEY rrset and store a copy in `$HOME/.getdns/root.key` on Unixes, and `%appdata%\getdns\root.key` on Windows.
Only when the KSK DNSKEY's change, a new version of `root-anchors.xml` is tried to be retrieved from [data.iana.org](https://data.iana.org/root-anchors/).

A installed trust-anchor from the default location (`/etc/unbound/getdns-root.key`) that fails to validate the root DNSKEY RRset, will also trigger the "Zero configuration DNSSEC" procedure described above.

Support
=======

## Mailing lists

We have a [getdns users list](https://getdnsapi.net/mailman/listinfo/users) for this implementation.

The [getdns-api mailing list](https://getdnsapi.net/mailman/listinfo/spec) is a good place to engage in discussions regarding the design of the API.

## Tickets and Bug Reports

Tickets and bug reports should be reported via the [GitHub issues list](https://github.com/getdnsapi/getdns/issues).

Features of this release
========================

## Goals

The goals of this implementation of the getdns API are:

* Provide an open source implementation, in C, of the formally described getdns API by getdns API team at <https://getdnsapi.net/spec.html>
* Support FreeBSD, OSX, Linux (CentOS/RHEL, Ubuntu) via functional "configure" script
* Support Windows 8.1 
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

The primary platforms targeted are Linux and FreeBSD, other platform are supported as we get time.  The names listed here are intended to help ensure that we catch platform specific breakage, not to limit the work that folks are doing.

* RHEL/CentOS 6.4
* OSX 10.8
* Ubuntu 16.04
* Microsoft Windows 8.1

We intend to add Android and other platforms to future releases as we have time to port it.


##  Platform Specific Build Reports

[![Build Status](https://travis-ci.org/getdnsapi/getdns.png?branch=master)](https://travis-ci.org/getdnsapi/getdns)

### FreeBSD

If you're using [FreeBSD](https://www.freebsd.org/), you may install getdns via the [ports tree](https://www.freshports.org/dns/getdns/) by running: `cd /usr/ports/dns/getdns && make install clean`

If you are using FreeBSD 10 getdns can be intalled via 'pkg install getdns'.

### CentOS and RHEL 6.5

We rely on the most excellent package manager fpm to build the linux packages, which
means that the packaging platform requires ruby 2.1.0.  There are other ways to
build the packages; this is simply the one we chose to use.

    # cat /etc/redhat-release
    CentOS release 6.5 (Final)
    # uname -a
    Linux host-10-1-1-6 2.6.32-358.el6.x86_64 #1 SMP Fri Feb 22 00:31:26 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux
    # cd getdns-0.2.0rc1
    # ./configure --prefix=/home/deploy/build
    # make; make install
    # cd /home/deploy/build
    # mv lib lib64
    # . /usr/local/rvm/config/alias
    # fpm -x "*.la" -a native -s dir -t rpm -n getdns -v 0.2.0rc1 -d "unbound" -d "libevent" -d "libidn" --prefix /usr --vendor "Verisign Inc., NLnet Labs" --license "BSD New" --url "https://getdnsapi.net" --description "Modern asynchronous API to the DNS" .

### OSX

    # sw_vers
    ProductName:	Mac OS X
    ProductVersion:	10.8.5
    BuildVersion:	12F45

    Built using PackageMaker, libevent2.

    # ./configure --with-libevent --prefix=$HOME/getdnsosx/export
    # make
    # make install

    edit/fix hardcoded paths in lib/*.la to reference /usr/local

    update getdns.pmdoc to match release info

    build package using PackageMaker

    create dmg

    A self-compiled version of OpenSSL or the version installed via Homebrew is required.
    Note: If using a self-compiled version, manual configuration of certificates into /usr/local/etc/openssl/certs is required for TLS authentication to work.

#### Homebrew

If you're using [Homebrew](http://brew.sh/), you may run `brew install getdns`.  By default, this will only build the core library without any 3rd party event loop support.

To install the [event loop integration libraries](https://getdnsapi.net/doxygen/group__eventloops.html) that enable support for libevent, libuv, and libev, run: `brew install getdns --with-libevent --with-libuv --with-libev`.  All switches are optional.

Note that in order to compile the examples, the `--with-libevent` switch is required.

Additionally, the OpenSSL library installed by Homebrew is linked against. Note that the Homebrew OpenSSL installation clones the Keychain certificates to the default OpenSSL location so TLS certificate authentication should work out of the box.

### Microsoft Windows 8.1

The build has been tested using the following:
32 bit only Mingw: [Mingw(3.21.0) and Msys 1.0](http://www.mingw.org/) on Windows 8.1
32 bit build on a 64 bit Mingw [Download latest from: http://mingw-w64.org/doku.php/download/mingw-builds and http://msys2.github.io/]. IMPORTANT: Install tested ONLY on the  "x86_64" for 64-bit installer of msys2.

#### Dependencies
The following dependencies are 
* openssl-1.0.2j
* libidn

Instructions to build openssl-1.0.2j:
Open the mingw32_shell.bat from msys2 in order to build:

If necessary, install the following using pacman:

    pacman -S pkg-config  libtool automake
    pacman -S autoconf automake-wrapper

    tar -xvf openssl-1.0.2j.tar 
    cd openssl-1.0.2j/
    ./Configure --prefix=${LOCALDESTDIR} --openssldir=${LOCALDESTDIR}/etc/ssl --libdir=lib shared zlib-dynamic mingw
    make
    make install

To configure:
    
    ./configure --enable-stub-only --with-trust-anchor="c:\\\MinGW\\\msys\\\1.0\\\etc\\\unbound\\\getdns-root.key" --with-ssl=<location of openssl from above> --with-getdns_query

 The trust anchor is also installed by unbound on `c:\program Files (X86)\unbound\root.key` and can be referenced from there
 or anywhere else that the user chooses to configure it.

 After configuring, do a `make` and `make install` to build getdns for Windows.

 Example test queries:
 
    ./getdns_query.exe -s gmadkat.com A @64.6.64.6  +return_call_reporting (UDP)
    ./getdns_query.exe -s gmadkat.com A @64.6.64.6 -T  +return_call_reporting (TCP)
    ./getdns_query.exe -s gmadkat.com A -l L @185.49.141.37  +return_call_reporting (TLS without authentication)
    ./getdns_query.exe -s www.huque.com A +dnssec_return_status +return_call_reporting (DNSSEC)

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
The development team explicitly acknowledges Paul Hoffman for his initiative and efforts to develop a consensus based DNS API. We would like to thank the participants of the [mailing list](https://getdnsapi.net/mailman/listinfo/spec) for their contributions.
