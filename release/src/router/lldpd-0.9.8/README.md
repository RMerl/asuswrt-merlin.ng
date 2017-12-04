lldpd: implementation of IEEE 802.1ab (LLDP)
============================================

[![Build Status](https://secure.travis-ci.org/vincentbernat/lldpd.png?branch=master)](http://travis-ci.org/vincentbernat/lldpd)

  http://vincentbernat.github.com/lldpd/

Features
--------

LLDP (Link Layer Discovery Protocol) is an industry standard protocol
designed to supplant proprietary Link-Layer protocols such as
Extreme's EDP (Extreme Discovery Protocol) and CDP (Cisco Discovery
Protocol). The goal of LLDP is to provide an inter-vendor compatible
mechanism to deliver Link-Layer notifications to adjacent network
devices.

lldpd implements both reception and sending. It also implements an
SNMP subagent for net-snmp to get local and remote LLDP
information. The LLDP-MIB is partially implemented but the most useful
tables are here. lldpd also partially implements LLDP-MED.

lldpd supports bridge, vlan and bonding.

The following OS are supported:

 * FreeBSD
 * GNU/Linux
 * OS X
 * NetBSD
 * OpenBSD
 * Solaris

Windows is not supported but you can use
[WinLLDPService](https://github.com/raspi/WinLLDPService/) as a
transmit-only agent.

Installation
------------

For general instructions
[see the website](http://vincentbernat.github.io/lldpd/installation.html).

To compile lldpd from sources, use the following:

    ./configure
    make
    sudo make install

lldpd uses privilege separation to increase its security. Two
processes, one running as root and doing minimal stuff and the other
running as an unprivileged user into a chroot doing most of the stuff,
are cooperating. You need to create a user called `_lldpd` in a group
`_lldpd` (this can be change with `./configure`). You also need to
create an empty directory `/usr/local/var/run/lldpd` (it needs to be
owned by root, not `_lldpd`!). If you get fuzzy timestamps from
syslog, copy `/etc/locatime` into the chroot.

`lldpcli` lets one query information collected through the command
line. If you don't want to run it as root, just install it setuid or
setgid `_lldpd`.

Installation (OS X)
-----------------------

The same procedure as above applies for OS X. However, there are
simpler alternatives:

 1. Use [Homebrew](https://brew.sh):

        brew install lldpd
        # Or, for the latest version:
        brew install https://raw.github.com/vincentbernat/lldpd/master/osx/lldpd.rb

 2. Build an OS X installer package which should work on the same
    version of OS X:
 
        mkdir build && cd build
        ../configure --prefix=/usr/local --localstatedir=/var --sysconfdir=/private/etc --with-embedded-libevent \
            --without-snmp
        make -C osx pkg

    If you want to compile for an older version of OS X, you need
    to find the right SDK and issues commands like those:

        SDK=/Developer/SDKs/MacOSX10.6.sdk
        mkdir build && cd build
        ../configure --prefix=/usr/local --localstatedir=/var --sysconfdir=/private/etc --with-embedded-libevent \
           --without-snmp \
           CFLAGS="-mmacosx-version-min=10.6 -isysroot $SDK" \
           LDFLAGS="-mmacosx-version-min=10.6 -isysroot $SDK"
        make -C osx pkg

    With recent SDK, you don't need to specify an alternate SDK. They
    are organized in a way that should enable compatibility with older
    versions of OSX:

        mkdir build && cd build
        ../configure --prefix=/usr/local --localstatedir=/var --sysconfdir=/private/etc --with-embedded-libevent \
           --without-snmp \
           CFLAGS="-mmacosx-version-min=10.9" \
           LDFLAGS="-mmacosx-version-min=10.9"
        make -C osx pkg

    You can check with `otool -l` that you got what you expected in
    term of supported versions.

If you don't follow the above procedures, you will have to create the
user/group `_lldpd`. Have a look at how this is done in
`osx/scripts/postinstall`.

Installation (Android)
----------------------

You need to download [Android NDK][]. Once unpacked, you can generate
a toolchain using the following command:

    ./build/tools/make-standalone-toolchain.sh \
        --platform=android-9 \
        --arch=arm \
        --install-dir=../android-toolchain
    export TOOLCHAIN=$PWD/../android-toolchain

Then, you can build `lldpd` with the following commands:

    mkdir build && cd build
    export PATH=$PATH:$TOOLCHAIN/bin
    ../configure \
        --host=arm-linux-androideabi \
        --with-sysroot=$TOOLCHAIN/sysroot

[Android NDK]: http://developer.android.com/tools/sdk/ndk/index.html

Usage
-----

lldpd also implements CDP (Cisco Discovery Protocol), FDP (Foundry
Discovery Protocol), SONMP (Nortel Discovery Protocol) and EDP
(Extreme Discovery Protocol). However, recent versions of IOS should
support LLDP and most Extreme stuff support LLDP. When a EDP, CDP or
SONMP frame is received on a given interface, lldpd starts sending
EDP, CDP, FDP or SONMP frame on this interface. Informations collected
through EDP/CDP/FDP/SONMP are integrated with other informations and
can be queried with `lldpcli` or through SNMP.

More information:
 * http://en.wikipedia.org/wiki/Link_Layer_Discovery_Protocol
 * http://standards.ieee.org/getieee802/download/802.1AB-2005.pdf
 * http://wiki.wireshark.org/LinkLayerDiscoveryProtocol

Compatibility with older kernels
--------------------------------

If you have a kernel older than Linux 2.6.39, you need to compile
lldpd with `--enable-oldies` to enable some compatibility functions:
otherwise, lldpd will only rely on Netlink to receive bridge, bond and
VLAN information.

For bonding, you need 2.6.24 (in previous version, PACKET_ORIGDEV
affected only non multicast packets). See:

 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commitdiff;h=80feaacb8a6400a9540a961b6743c69a5896b937
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commitdiff;h=8032b46489e50ef8f3992159abd0349b5b8e476c

Otherwise, a packet received on a bond will be affected to all
interfaces of the bond. In this case, lldpd will affect a received
randomly to one of the interface (so a neighbor may be affected to the
wrong interface).

On 2.6.27, we are able to receive packets on real interface for enslaved
devices. This allows one to get neighbor information on active/backup
bonds. Without the 2.6.27, lldpd won't receive any information on
inactive slaves. Here are the patchs (thanks to Joe Eykholt):

 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=0d7a3681232f545c6a59f77e60f7667673ef0e93
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=cc9bd5cebc0825e0fabc0186ab85806a0891104f
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=f982307f22db96201e41540295f24e8dcc10c78f

On FreeBSD, only a recent 9 kernel (9.1 or more recent) will allow to
send LLDP frames on enslaved devices. See this bug report for more
information:

 * http://www.freebsd.org/cgi/query-pr.cgi?pr=138620

Some devices (notably Cisco IOS) send frames tagged with the native
VLAN while they should send them untagged. If your network card does
not support accelerated VLAN, you will receive those frames as long as
the corresponding interface exists (see below). However, if your
network card handles VLAN encapsulation/decapsulation (check with
`ethtool -k`), you need a recent kernel to be able to receive those
frames without listening on all available VLAN. Starting from Linux
2.6.27, lldpd is able to capture VLAN frames when VLAN acceleration is
supported by the network card. Here is the patch:

 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=bc1d0411b804ad190cdadabac48a10067f17b9e6

On some other versions, frames are sent on VLAN 1. If this is not the
native VLAN and if your network card support accelerated VLAN, you
need to subscribe to this VLAN as well. The Linux kernel does not
provide any interface for this. The easiest way is to create the VLAN
for each port:

    ip link add link eth0 name eth0.1 type vlan id 1
    ip link set up dev eth0.1

You can check both cases using tcpdump:

    tcpdump -epni eth0 ether host 01:80:c2:00:00:0e
    tcpdump -eni eth0 ether host 01:80:c2:00:00:0e

If the first command does not display received LLDP packets but the
second one does, LLDP packets are likely encapsulated into a VLAN:

    10:54:06.431154 f0:29:29:1d:7c:01 > 01:80:c2:00:00:0e, ethertype 802.1Q (0x8100), length 363: vlan 1, p 7, ethertype LLDP, LLDP, name SW-APP-D07.VTY, length 345

In this case, just create VLAN 1 will fix the situation. There are
other solutions:

 1. Disable VLAN acceleration on the receive side (`ethtool -K eth0
    rxvlan off`) but this may or may not work. Check if there are
    similar properties that could apply with `ethtool -k eth0`.
 2. Put the interface in promiscuous mode with `ip link set
    promisc on dev eth0`.

The last solution can be done directly by `lldpd` (on Linux only) by
using the option `configure system interface promiscuous`.

On modern networks, the performance impact should be nonexistent.

Development
-----------

During development, you may want to execute lldpd at its current
location instead of doing `make install`. The correct way to do this is
to issue the following command:

    sudo libtool execute src/daemon/lldpd -L $PWD/src/client/lldpcli -d

You can append any further arguments. If lldpd is unable to find
`lldpcli` it will start in an unconfigured mode and won't send or
accept LLDP frames.

You can use [afl](http://lcamtuf.coredump.cx/afl/) to test some
aspects of lldpd. To test frame decoding, you can do something like
that:

    export AFL_USE_ASAN=1 # only on 32bit arch
    ./configure CC=afl-gcc
    make clean check
    cd tests
    mkdir inputs
    mv *.pcap inputs
    afl-fuzz -i inputs -o outputs ./decode @@

There is a general test suite with `make check`. It's also possible to
run integration tests. They need [py.test](http://pytest.org/latest/)
and rely on Linux containers to be executed.

To enable code coverage, use:

    ../configure --prefix=/usr --sysconfdir=/etc --localstatedir=/var \
                 --enable-sanitizers --enable-gcov --with-snmp \
                 CFLAGS="-O0 -g"
    make
    make check
    # maybe, run integration tests
    lcov --base-directory $PWD/src/lib \
         --directory src --capture --output-file gcov.info
    genhtml gcov.info --output-directory coverage

Embedding
---------

To embed lldpd into an existing system, there are two point of entries:

 1. If your system does not use standard Linux interface, you can
    support additional interfaces by implementing the appropriate
    `struct lldpd_ops`. You can look at
    `src/daemon/interfaces-linux.c` for examples. Also, have a look at
    `interfaces_update()` which is responsible for discovering and
    registering interfaces.

 2. `lldpcli` provides a convenient way to query `lldpd`. It also
    comes with various outputs, including XML which allows one to
    parse its output for integration and automation purpose. Another
    way is to use SNMP support. A third way is to write your own
    controller using `liblldpctl.so`. Its API is described in
    `src/lib/lldpctl.h`. The custom binary protocol between
    `liblldpctl.so` and `lldpd` is not stable. Therefore, the library
    should always be shipped with `lldpd`. On the other hand, programs
    using `liblldpctl.so` can rely on the classic ABI rules.

Troubleshooting
---------------

You can use `tcpdump` to look after the packets received and send by
`lldpd`. To look after LLDPU, use:

    tcpdump -s0 -vv -pni eth0 ether dst 01:80:c2:00:00:0e

License
-------

lldpd is distributed under the ISC license:

 > Permission to use, copy, modify, and/or distribute this software for any
 > purpose with or without fee is hereby granted, provided that the above
 > copyright notice and this permission notice appear in all copies.
 >
 > THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 > WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 > MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 > ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 > WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 > ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 > OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Also, `lldpcli` will be linked to GNU Readline (which is GPL licensed)
if available. To avoid this, use `--without-readline` as a configure
option.
