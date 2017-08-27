#
# Copyright (C) 2007-2014 Tobias Brunner
# Copyright (C) 2006-2014 Andreas Steffen
# Copyright (C) 2006-2014 Martin Willi
# Hochschule fuer Technik Rapperswil
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#

# ============================
#  initialize & set some vars
# ============================

AC_INIT([strongSwan],[5.2.1])
AM_INIT_AUTOMAKE(m4_esyscmd([
	echo tar-ustar
	echo subdir-objects
	case `automake --version | head -n 1` in
		*" 1.9"*);;
		*" 1.10"*);;
		*" 1.11"*);;
		# don't use parallel test harness in 1.12 and up
		*) echo serial-tests;;
	esac
]))
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_MACRO_DIR([m4/config])
AC_CONFIG_HEADERS([config.h])
AC_DEFINE([CONFIG_H_INCLUDED], [], [defined if config.h included])
PKG_PROG_PKG_CONFIG

m4_include(m4/macros/split-package-version.m4)
SPLIT_PACKAGE_VERSION

# =================================
#  check --enable-xxx & --with-xxx
# =================================

m4_include(m4/macros/with.m4)

ARG_WITH_SUBST([random-device],      [/dev/random], [set the device to read real random data from])
ARG_WITH_SUBST([urandom-device],     [/dev/urandom], [set the device to read pseudo random data from])
ARG_WITH_SUBST([strongswan-conf],    [${sysconfdir}/strongswan.conf], [set the strongswan.conf file location])
ARG_WITH_SUBST([resolv-conf],        [${sysconfdir}/resolv.conf], [set the file to use in DNS handler plugin])
ARG_WITH_SUBST([piddir],             [/var/run], [set path for PID and UNIX socket files])
ARG_WITH_SUBST([ipsecdir],           [${libexecdir%/}/ipsec], [set installation path for ipsec tools])
ARG_WITH_SUBST([ipseclibdir],        [${libdir%/}/ipsec], [set installation path for ipsec libraries])
ARG_WITH_SUBST([plugindir],          [${ipseclibdir%/}/plugins], [set the installation path of plugins])
ARG_WITH_SUBST([imcvdir],            [${ipseclibdir%/}/imcvs], [set the installation path of IMC and IMV dynamic librariers])
ARG_WITH_SUBST([nm-ca-dir],          [/usr/share/ca-certificates], [directory the NM backend uses to look up trusted root certificates])
ARG_WITH_SUBST([swanctldir],         [${sysconfdir}/swanctl], [base directory for swanctl configuration files and credentials])
ARG_WITH_SUBST([linux-headers],      [\${top_srcdir}/src/include], [set directory of linux header files to use])
ARG_WITH_SUBST([routing-table],      [220], [set routing table to use for IPsec routes])
ARG_WITH_SUBST([routing-table-prio], [220], [set priority for IPsec routing table])
ARG_WITH_SUBST([ipsec-script],       [ipsec], [change the name of the ipsec script])
ARG_WITH_SUBST([fips-mode],          [0], [set openssl FIPS mode: disabled(0), enabled(1), Suite B enabled(2)])

ARG_WITH_SET([tss],                  [no], [set implementation of the Trusted Computing Group's Software Stack (TSS). Currently the only supported value is "trousers"])
ARG_WITH_SET([capabilities],         [no], [set capability dropping library. Currently supported values are "libcap" and "native"])
ARG_WITH_SET([mpz_powm_sec],         [yes], [use the more side-channel resistant mpz_powm_sec in libgmp, if available])
ARG_WITH_SET([dev-headers],          [no], [install strongSwan development headers to directory.])
ARG_WITH_SET([printf-hooks],         [auto], [force the use of a specific printf hook implementation (auto, builtin, glibc, vstr).])
ARG_WITH_SET([rubygemdir],           ["gem environment gemdir"], [path to install ruby gems to])

if test -n "$PKG_CONFIG"; then
	systemdsystemunitdir_default=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)
fi
ARG_WITH_SET([systemdsystemunitdir], [$systemdsystemunitdir_default], [directory for systemd service files])
AC_SUBST(systemdsystemunitdir)

AC_ARG_WITH(
	[user],
	AS_HELP_STRING([--with-user=user],[change user of the daemons to "user" after startup (default is "root").]),
	[AC_DEFINE_UNQUOTED([IPSEC_USER], "$withval", [username to run daemon with])
	 AC_SUBST(ipsecuser, "$withval")],
	[AC_SUBST(ipsecuser, "root")]
)

AC_ARG_WITH(
	[group],
	AS_HELP_STRING([--with-group=group],[change group of the daemons to "group" after startup (default is "root").]),
	[AC_DEFINE_UNQUOTED(IPSEC_GROUP, "$withval", [groupname to run daemon with])
	 AC_SUBST(ipsecgroup, "$withval")],
	[AC_SUBST(ipsecgroup, "root")]
)

AC_ARG_WITH(
	[charon-udp-port],
	AS_HELP_STRING([--with-charon-udp-port=port],[UDP port used by charon locally (default 500). Set to 0 to allocate randomly.]),
	[AC_DEFINE_UNQUOTED(CHARON_UDP_PORT, [$withval], [UDP port used by charon locally])
	 AC_SUBST(charon_udp_port, [$withval])],
	[AC_SUBST(charon_udp_port, 500)]
)

AC_ARG_WITH(
	[charon-natt-port],
	AS_HELP_STRING([--with-charon-natt-port=port],[UDP port used by charon locally in case a NAT is detected (must be different from charon-udp-port, default 4500). Set to 0 to allocate randomly.]),
	[AC_DEFINE_UNQUOTED(CHARON_NATT_PORT, [$withval], [UDP post used by charon locally in case a NAT is detected])
	 AC_SUBST(charon_natt_port, [$withval])],
	[AC_SUBST(charon_natt_port, 4500)]
)

AC_MSG_CHECKING([configured UDP ports ($charon_udp_port, $charon_natt_port)])
if test x$charon_udp_port != x0 -a x$charon_udp_port = x$charon_natt_port; then
	AC_MSG_ERROR(the ports have to be different)
else
	AC_MSG_RESULT(ok)
fi

# convert script name to uppercase
AC_SUBST(ipsec_script_upper, [`echo -n "$ipsec_script" | tr a-z A-Z`])

m4_include(m4/macros/enable-disable.m4)

# crypto plugins
ARG_DISBL_SET([aes],            [disable AES software implementation plugin.])
ARG_ENABL_SET([af-alg],         [enable AF_ALG crypto interface to Linux Crypto API.])
ARG_ENABL_SET([blowfish],       [enable Blowfish software implementation plugin.])
ARG_ENABL_SET([ccm],            [enables the CCM AEAD wrapper crypto plugin.])
ARG_DISBL_SET([cmac],           [disable CMAC crypto implementation plugin.])
ARG_ENABL_SET([ctr],            [enables the Counter Mode wrapper crypto plugin.])
ARG_DISBL_SET([des],            [disable DES/3DES software implementation plugin.])
ARG_DISBL_SET([fips-prf],       [disable FIPS PRF software implementation plugin.])
ARG_ENABL_SET([gcm],            [enables the GCM AEAD wrapper crypto plugin.])
ARG_ENABL_SET([gcrypt],         [enables the libgcrypt plugin.])
ARG_DISBL_SET([gmp],            [disable GNU MP (libgmp) based crypto implementation plugin.])
ARG_DISBL_SET([hmac],           [disable HMAC crypto implementation plugin.])
ARG_ENABL_SET([md4],            [enable MD4 software implementation plugin.])
ARG_DISBL_SET([md5],            [disable MD5 software implementation plugin.])
ARG_DISBL_SET([nonce],          [disable nonce generation plugin.])
ARG_ENABL_SET([ntru],           [enables the NTRU crypto plugin.])
ARG_ENABL_SET([openssl],        [enables the OpenSSL crypto plugin.])
ARG_ENABL_SET([padlock],        [enables VIA Padlock crypto plugin.])
ARG_DISBL_SET([random],         [disable RNG implementation on top of /dev/(u)random.])
ARG_DISBL_SET([rc2],            [disable RC2 software implementation plugin.])
ARG_ENABL_SET([rdrand],         [enable Intel RDRAND random generator plugin.])
ARG_DISBL_SET([sha1],           [disable SHA1 software implementation plugin.])
ARG_DISBL_SET([sha2],           [disable SHA256/SHA384/SHA512 software implementation plugin.])
ARG_DISBL_SET([xcbc],           [disable xcbc crypto implementation plugin.])
# encoding/decoding plugins
ARG_DISBL_SET([dnskey],         [disable DNS RR key decoding plugin.])
ARG_DISBL_SET([pem],            [disable PEM decoding plugin.])
ARG_DISBL_SET([pgp],            [disable PGP key decoding plugin.])
ARG_DISBL_SET([pkcs1],          [disable PKCS1 key decoding plugin.])
ARG_DISBL_SET([pkcs7],          [disable PKCS7 container support plugin.])
ARG_DISBL_SET([pkcs8],          [disable PKCS8 private key decoding plugin.])
ARG_DISBL_SET([pkcs12],         [disable PKCS12 container support plugin.])
ARG_DISBL_SET([pubkey],         [disable RAW public key support plugin.])
ARG_DISBL_SET([sshkey],         [disable SSH key decoding plugin.])
ARG_DISBL_SET([x509],           [disable X509 certificate implementation plugin.])
# fetcher/resolver plugins
ARG_ENABL_SET([curl],           [enable CURL fetcher plugin to fetch files via libcurl. Requires libcurl.])
ARG_ENABL_SET([ldap],           [enable LDAP fetching plugin to fetch files via libldap. Requires openLDAP.])
ARG_ENABL_SET([soup],           [enable soup fetcher plugin to fetch from HTTP via libsoup. Requires libsoup.])
ARG_ENABL_SET([unbound],        [enable UNBOUND resolver plugin to perform DNS queries via libunbound. Requires libldns and libunbound.])
ARG_ENABL_SET([winhttp],        [enable WinHTTP based HTTP/HTTPS fetching plugin.])
# database plugins
ARG_ENABL_SET([mysql],          [enable MySQL database support. Requires libmysqlclient_r.])
ARG_ENABL_SET([sqlite],         [enable SQLite database support. Requires libsqlite3.])
# authentication/credential plugins
ARG_ENABL_SET([addrblock],      [enables RFC 3779 address block constraint support.])
ARG_ENABL_SET([acert],          [enable X509 attribute certificate checking plugin.])
ARG_ENABL_SET([agent],          [enables the ssh-agent signing plugin.])
ARG_DISBL_SET([constraints],    [disable advanced X509 constraint checking plugin.])
ARG_ENABL_SET([coupling],       [enable IKEv2 plugin to couple peer certificates permanently to authentication.])
ARG_ENABL_SET([dnscert],        [enable DNSCERT authentication plugin.])
ARG_ENABL_SET([eap-sim],        [enable SIM authentication module for EAP.])
ARG_ENABL_SET([eap-sim-file],   [enable EAP-SIM backend based on a triplet file.])
ARG_ENABL_SET([eap-sim-pcsc],   [enable EAP-SIM backend based on a smartcard reader. Requires libpcsclite.])
ARG_ENABL_SET([eap-aka],        [enable EAP AKA authentication module.])
ARG_ENABL_SET([eap-aka-3gpp2],  [enable EAP AKA backend implementing 3GPP2 algorithms in software. Requires libgmp.])
ARG_ENABL_SET([eap-simaka-sql], [enable EAP-SIM/AKA backend based on a triplet/quintuplet SQL database.])
ARG_ENABL_SET([eap-simaka-pseudonym], [enable EAP-SIM/AKA pseudonym storage plugin.])
ARG_ENABL_SET([eap-simaka-reauth],    [enable EAP-SIM/AKA reauthentication data storage plugin.])
ARG_ENABL_SET([eap-identity],   [enable EAP module providing EAP-Identity helper.])
ARG_ENABL_SET([eap-md5],        [enable EAP MD5 (CHAP) authentication module.])
ARG_ENABL_SET([eap-gtc],        [enable EAP GTC authentication module.])
ARG_ENABL_SET([eap-mschapv2],   [enable EAP MS-CHAPv2 authentication module.])
ARG_ENABL_SET([eap-tls],        [enable EAP TLS authentication module.])
ARG_ENABL_SET([eap-ttls],       [enable EAP TTLS authentication module.])
ARG_ENABL_SET([eap-peap],       [enable EAP PEAP authentication module.])
ARG_ENABL_SET([eap-tnc],        [enable EAP TNC trusted network connect module.])
ARG_ENABL_SET([eap-dynamic],    [enable dynamic EAP proxy module.])
ARG_ENABL_SET([eap-radius],     [enable RADIUS proxy authentication module.])
ARG_ENABL_SET([ext-auth],       [enable plugin calling an external authorization script.])
ARG_ENABL_SET([ipseckey],       [enable IPSECKEY authentication plugin.])
ARG_ENABL_SET([keychain],       [enables OS X Keychain Services credential set.])
ARG_ENABL_SET([pkcs11],         [enables the PKCS11 token support plugin.])
ARG_DISBL_SET([revocation],     [disable X509 CRL/OCSP revocation check plugin.])
ARG_ENABL_SET([whitelist],      [enable peer identity whitelisting plugin.])
ARG_DISBL_SET([xauth-generic],  [disable generic XAuth backend.])
ARG_ENABL_SET([xauth-eap],      [enable XAuth backend using EAP methods to verify passwords.])
ARG_ENABL_SET([xauth-pam],      [enable XAuth backend using PAM to verify passwords.])
ARG_ENABL_SET([xauth-noauth],   [enable XAuth pseudo-backend that does not actually verify or even request any credentials.])
# kernel interfaces / sockets
ARG_DISBL_SET([kernel-netlink], [disable the netlink kernel interface.])
ARG_ENABL_SET([kernel-pfkey],   [enable the PF_KEY kernel interface.])
ARG_ENABL_SET([kernel-pfroute], [enable the PF_ROUTE kernel interface.])
ARG_ENABL_SET([kernel-iph],     [enable the Windows IP Helper based networking backend.])
ARG_ENABL_SET([kernel-libipsec],[enable the libipsec kernel interface.])
ARG_ENABL_SET([kernel-wfp],     [enable the Windows Filtering Platform IPsec backend.])
ARG_DISBL_SET([socket-default], [disable default socket implementation for charon.])
ARG_ENABL_SET([socket-dynamic], [enable dynamic socket implementation for charon])
ARG_ENABL_SET([socket-win],     [enable Winsock2 based socket implementation for charon])
# configuration/control plugins
ARG_DISBL_SET([stroke],         [disable charons stroke configuration backend.])
ARG_ENABL_SET([smp],            [enable SMP configuration and control interface. Requires libxml.])
ARG_ENABL_SET([sql],            [enable SQL database configuration backend.])
ARG_ENABL_SET([uci],            [enable OpenWRT UCI configuration plugin.])
ARG_ENABL_SET([vici],           [enable strongSwan IKE generic IPC interface plugin.])
# attribute provider/consumer plugins
ARG_ENABL_SET([android-dns],    [enable Android specific DNS handler.])
ARG_DISBL_SET([attr],           [disable strongswan.conf based configuration attribute plugin.])
ARG_ENABL_SET([attr-sql],       [enable SQL based configuration attribute plugin.])
ARG_ENABL_SET([dhcp],           [enable DHCP based attribute provider plugin.])
ARG_ENABL_SET([osx-attr],       [enable OS X SystemConfiguration attribute handler.])
ARG_DISBL_SET([resolve],        [disable resolve DNS handler plugin.])
ARG_ENABL_SET([unity],          [enables Cisco Unity extension plugin.])
# TNC modules/plugins
ARG_ENABL_SET([imc-test],       [enable IMC test module.])
ARG_ENABL_SET([imv-test],       [enable IMV test module.])
ARG_ENABL_SET([imc-scanner],    [enable IMC port scanner module.])
ARG_ENABL_SET([imv-scanner],    [enable IMV port scanner module.])
ARG_ENABL_SET([imc-os],         [enable IMC operating system module.])
ARG_ENABL_SET([imv-os],         [enable IMV operating system module.])
ARG_ENABL_SET([imc-attestation],[enable IMC attestation module.])
ARG_ENABL_SET([imv-attestation],[enable IMV attestation module.])
ARG_ENABL_SET([imc-swid],       [enable IMC swid module.])
ARG_ENABL_SET([imv-swid],       [enable IMV swid module.])
ARG_ENABL_SET([tnc-ifmap],      [enable TNC IF-MAP module. Requires libxml])
ARG_ENABL_SET([tnc-imc],        [enable TNC IMC module.])
ARG_ENABL_SET([tnc-imv],        [enable TNC IMV module.])
ARG_ENABL_SET([tnc-pdp],        [enable TNC policy decision point module.])
ARG_ENABL_SET([tnccs-11],       [enable TNCCS 1.1 protocol module. Requires libxml])
ARG_ENABL_SET([tnccs-20],       [enable TNCCS 2.0 protocol module.])
ARG_ENABL_SET([tnccs-dynamic],  [enable dynamic TNCCS protocol discovery module.])
# misc plugins
ARG_ENABL_SET([android-log],    [enable Android specific logger plugin.])
ARG_ENABL_SET([certexpire],     [enable CSV export of expiration dates of used certificates.])
ARG_ENABL_SET([duplicheck],     [advanced duplicate checking plugin using liveness checks.])
ARG_ENABL_SET([error-notify],   [enable error notification plugin.])
ARG_ENABL_SET([farp],           [enable ARP faking plugin that responds to ARP requests to peers virtual IP])
ARG_ENABL_SET([ha],             [enable high availability cluster plugin.])
ARG_ENABL_SET([led],            [enable plugin to control LEDs on IKEv2 activity using the Linux kernel LED subsystem.])
ARG_ENABL_SET([load-tester],    [enable load testing plugin for IKEv2 daemon.])
ARG_ENABL_SET([lookip],         [enable fast virtual IP lookup and notification plugin.])
ARG_ENABL_SET([maemo],          [enable Maemo specific plugin.])
ARG_ENABL_SET([radattr],        [enable plugin to inject and process custom RADIUS attributes as IKEv2 client.])
ARG_ENABL_SET([systime-fix],    [enable plugin to handle cert lifetimes with invalid system time gracefully.])
ARG_ENABL_SET([test-vectors],   [enable plugin providing crypto test vectors.])
ARG_ENABL_SET([unit-tester],    [enable unit tests on IKEv2 daemon startup.])
ARG_DISBL_SET([updown],         [disable updown firewall script plugin.])
# programs/components
ARG_ENABL_SET([aikgen],         [enable AIK generator.])
ARG_DISBL_SET([charon],         [disable the IKEv1/IKEv2 keying daemon charon.])
ARG_ENABL_SET([cmd],            [enable the command line IKE client charon-cmd.])
ARG_ENABL_SET([conftest],       [enforce Suite B conformance test framework.])
ARG_ENABL_SET([dumm],           [enable the DUMM UML test framework.])
ARG_ENABL_SET([fast],           [enable libfast (FastCGI Application Server w/ templates.])
ARG_ENABL_SET([libipsec],       [enable user space IPsec implementation.])
ARG_ENABL_SET([manager],        [enable web management console (proof of concept).])
ARG_ENABL_SET([medcli],         [enable mediation client configuration database plugin.])
ARG_ENABL_SET([medsrv],         [enable mediation server web frontend and daemon plugin.])
ARG_ENABL_SET([nm],             [enable NetworkManager backend.])
ARG_DISBL_SET([pki],            [disable pki certificate utility.])
ARG_DISBL_SET([scepclient],     [disable SCEP client tool.])
ARG_DISBL_SET([scripts],        [disable additional utilities (found in directory scripts).])
ARG_ENABL_SET([svc],            [enable charon Windows service.])
ARG_ENABL_SET([systemd],        [enable systemd specific IKE daemon charon-systemd.])
ARG_ENABL_SET([swanctl],        [enable swanctl configuration and control tool.])
ARG_ENABL_SET([tkm],            [enable Trusted Key Manager support.])
# optional features
ARG_ENABL_SET([bfd-backtraces], [use binutils libbfd to resolve backtraces for memory leaks and segfaults.])
ARG_ENABL_SET([dbghelp-backtraces],[use dbghlp.dll on Windows to create and print backtraces for memory leaks and segfaults.])
ARG_DISBL_SET([ikev1],          [disable IKEv1 protocol support in charon.])
ARG_DISBL_SET([ikev2],          [disable IKEv2 protocol support in charon.])
ARG_ENABL_SET([integrity-test], [enable integrity testing of libstrongswan and plugins.])
ARG_DISBL_SET([load-warning],   [disable the charon plugin load option warning in starter.])
ARG_ENABL_SET([mediation],      [enable IKEv2 Mediation Extension.])
ARG_ENABL_SET([unwind-backtraces],[use libunwind to create backtraces for memory leaks and segfaults.])
ARG_ENABL_SET([ruby-gems],      [enable installation of provided ruby gems.])
# compile options
ARG_ENABL_SET([coverage],       [enable lcov coverage report generation.])
ARG_ENABL_SET([leak-detective], [enable malloc hooks to find memory leaks.])
ARG_ENABL_SET([lock-profiler],  [enable lock/mutex profiling code.])
ARG_ENABL_SET([monolithic],     [build monolithic version of libstrongswan that includes all enabled plugins. Similarly, the plugins of charon are assembled in libcharon.])

# ===================================
#  option to disable default options
# ===================================

ARG_DISBL_SET([defaults],       [disable all default plugins (they can be enabled with their respective --enable options)])

if test x$defaults = xfalse; then
	for option in $enabled_by_default; do
		eval test x\${${option}_given} = xtrue && continue
		eval $option=false
	done
fi

# ==============================
#  option to enable all options
# ==============================

ARG_ENABL_SET([all],            [enable all plugins and features (they can be disabled with their respective --disable options). Mainly for testing.])

if test x$all_given = xtrue; then
	for option in $disabled_by_default; do
		eval test x\${${option}_given} = xtrue && continue
		eval $option=true
	done
fi

# ===========================
#  set up compiler and flags
# ===========================

if test -z "$CFLAGS"; then
	CFLAGS="-g -O2 -Wall -Wno-format -Wno-format-security -Wno-pointer-sign"
fi
AC_PROG_CC
AM_PROG_CC_C_O

AC_LIB_PREFIX
AC_C_BIGENDIAN

# =========================
#  check required programs
# =========================

LT_INIT
AC_PROG_INSTALL
AC_PROG_EGREP
AC_PROG_AWK
AC_PROG_LEX
AC_PROG_YACC
AM_PATH_PYTHON(,,[:])
AC_PATH_PROG([PERL], [perl], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
AC_PATH_PROG([GPERF], [gperf], [], [$PATH:/bin:/usr/bin:/usr/local/bin])

# because gperf is not needed by end-users we just report it but do not abort on failure
AC_MSG_CHECKING([gperf version >= 3.0.0])
if test -x "$GPERF"; then
	if test "`$GPERF --version | $AWK -F' ' '/^GNU gperf/ { print $3 }' | $AWK -F. '{ print $1 }'`" -ge "3"; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
else
	AC_MSG_RESULT([not found])
fi

# ========================
#  dependency calculation
# ========================

if test x$xauth_generic_given = xfalse -a x$ikev1 = xfalse; then
	xauth_generic=false;
fi

if test x$kernel_libipsec = xtrue; then
	libipsec=true;
fi

if test x$eap_aka_3gpp2 = xtrue; then
	gmp=true;
fi

if test x$eap_aka = xtrue; then
	fips_prf=true;
	simaka=true;
fi

if test x$eap_sim = xtrue; then
	fips_prf=true;
	simaka=true;
fi

if test x$eap_radius = xtrue -o x$radattr = xtrue -o x$tnc_pdp = xtrue; then
	radius=true;
fi

if test x$tnc_imc = xtrue -o x$tnc_imv = xtrue -o x$tnccs_11 = xtrue -o x$tnccs_11 = xtrue -o x$tnccs_dynamic = xtrue -o x$eap_tnc = xtrue; then
	tnc_tnccs=true;
fi

if test x$eap_tls = xtrue -o x$eap_ttls = xtrue -o x$eap_peap = xtrue -o x$tnc_tnccs = xtrue; then
	tls=true;
fi

if test x$imc_test = xtrue -o x$imv_test = xtrue -o x$imc_scanner = xtrue -o x$imv_scanner = xtrue -o x$imc_os = xtrue -o x$imv_os = xtrue -o x$imc_attestation = xtrue -o x$imv_attestation = xtrue -o x$imc_swid = xtrue -o x$imv_swid = xtrue; then
	imcv=true;
fi

if test x$fips_prf = xtrue; then
	if test x$openssl = xfalse; then
		sha1=true;
	fi
fi

if test x$swanctl = xtrue; then
	vici=true
fi

if test x$smp = xtrue -o x$tnccs_11 = xtrue -o x$tnc_ifmap = xtrue; then
	xml=true
fi

if test x$manager = xtrue; then
	fast=true
fi

if test x$medsrv = xtrue; then
	mediation=true
	fast=true
fi

if test x$medcli = xtrue; then
	mediation=true
fi

# ===========================================
#  check required libraries and header files
# ===========================================

AC_HEADER_STDBOOL
AC_FUNC_ALLOCA
AC_FUNC_STRERROR_R

#  libraries needed on some platforms but not on others
# ------------------------------------------------------
saved_LIBS=$LIBS

# FreeBSD and Mac OS X have dlopen integrated in libc, Linux needs libdl
LIBS=""
AC_SEARCH_LIBS(dlopen, dl, [DLLIB=$LIBS])
AC_SUBST(DLLIB)

# glibc's backtrace() can be replicated on FreeBSD with libexecinfo
LIBS=""
AC_SEARCH_LIBS(backtrace, execinfo, [BTLIB=$LIBS])
AC_CHECK_FUNCS(backtrace)
AC_SUBST(BTLIB)

# OpenSolaris needs libsocket and libnsl for socket()
LIBS=""
AC_SEARCH_LIBS(socket, socket, [SOCKLIB=$LIBS],
	[AC_CHECK_LIB(nsl, socket, [SOCKLIB="-lsocket -lnsl"], [], [-lsocket])]
)
AC_SUBST(SOCKLIB)

# Android has pthread_* functions in bionic (libc), others need libpthread
LIBS=""
AC_SEARCH_LIBS(pthread_create, pthread, [PTHREADLIB=$LIBS])
AC_SUBST(PTHREADLIB)

LIBS=$saved_LIBS
# ------------------------------------------------------

AC_MSG_CHECKING(for dladdr)
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#define _GNU_SOURCE
		  #include <dlfcn.h>]],
		[[Dl_info* info = 0;
		  dladdr(0, info);]])],
	[AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_DLADDR], [], [have dladdr()])],
	[AC_MSG_RESULT([no])]
)

# check if pthread_condattr_setclock(CLOCK_MONOTONE) is supported
saved_LIBS=$LIBS
LIBS=$PTHREADLIB
AC_MSG_CHECKING([for pthread_condattr_setclock(CLOCK_MONOTONE)])
AC_RUN_IFELSE(
	[AC_LANG_SOURCE(
		[[#include <pthread.h>
		  int main() { pthread_condattr_t attr;
			pthread_condattr_init(&attr);
			return pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);}]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_CONDATTR_CLOCK_MONOTONIC], [],
			   [pthread_condattr_setclock supports CLOCK_MONOTONIC])],
	[AC_MSG_RESULT([no])],
	# Check existence of pthread_condattr_setclock if cross-compiling
	[AC_MSG_RESULT([unknown]);
	 AC_CHECK_FUNCS(pthread_condattr_setclock,
		[AC_DEFINE([HAVE_CONDATTR_CLOCK_MONOTONIC], [],
				   [have pthread_condattr_setclock()])]
	)]
)
# check if we actually are able to configure attributes on cond vars
AC_CHECK_FUNCS(pthread_condattr_init)
# instead of pthread_condattr_setclock Android has this function
AC_CHECK_FUNCS(pthread_cond_timedwait_monotonic)
# check if we can cancel threads
AC_CHECK_FUNCS(pthread_cancel)
# check if native rwlocks are available
AC_CHECK_FUNCS(pthread_rwlock_init)
# check if pthread spinlocks are available
AC_CHECK_FUNCS(pthread_spin_init)
# check if we have POSIX semaphore functions, including timed-wait
AC_CHECK_FUNCS(sem_timedwait)
LIBS=$saved_LIBS

AC_CHECK_FUNC(
	[gettid],
	[AC_DEFINE([HAVE_GETTID], [], [have gettid()])],
	[AC_MSG_CHECKING([for SYS_gettid])
	 AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#define _GNU_SOURCE
			  #include <unistd.h>
			  #include <sys/syscall.h>]],
			[[int main() {
			  return syscall(SYS_gettid);}]])],
		[AC_MSG_RESULT([yes]);
		 AC_DEFINE([HAVE_GETTID], [], [have gettid()])
		 AC_DEFINE([HAVE_SYS_GETTID], [], [have syscall(SYS_gettid)])],
		[AC_MSG_RESULT([no])]
	)]
)

AC_CHECK_FUNC(
	[qsort_r],
	[
		AC_DEFINE([HAVE_QSORT_R], [], [have qsort_r()])
		# set -Werror so that we get an error for "argument ... has
		# incompatible pointer type" warnings
		save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -Werror"
		AC_MSG_CHECKING([for GNU-style qsort_r])
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(
				[[#define _GNU_SOURCE
				  #include <stdlib.h>
				  int cmp (const void *a, const void *b, void *x) { return 0; }]],
				[[int arr[] = { 0, 1 };
				  qsort_r(arr, 2, sizeof(int), cmp, arr);]])],
		[AC_MSG_RESULT([yes]);
		 AC_DEFINE([HAVE_QSORT_R_GNU], [], [have GNU-style qsort_r()])],
		[
			AC_MSG_RESULT([no]);
			AC_MSG_CHECKING([for BSD-style qsort_r])
			AC_COMPILE_IFELSE(
				[AC_LANG_PROGRAM(
					[[#include <stdlib.h>
					  int cmp (void *x, const void *a, const void *b) { return 0; }]],
					[[int arr[] = { 0, 1 };
					  qsort_r(arr, 2, sizeof(int), arr, cmp);]])],
			[AC_MSG_RESULT([yes]);
			 AC_DEFINE([HAVE_QSORT_R_BSD], [], [have BSD-style qsort_r()])],
			[AC_MSG_RESULT([no]);
			 AC_MSG_FAILURE([qsort_r has unknown semantics])])
		])
		CFLAGS="$save_CFLAGS"
	]
)

AC_CHECK_FUNCS(prctl mallinfo getpass closefrom getpwnam_r getgrnam_r getpwuid_r)
AC_CHECK_FUNCS(fmemopen funopen mmap memrchr setlinebuf strptime)

AC_CHECK_FUNC([syslog], [
	AC_DEFINE([HAVE_SYSLOG], [], [have syslog(3) and friends])
	syslog=true
])
AM_CONDITIONAL(USE_SYSLOG, [test "x$syslog" = xtrue])

AC_CHECK_HEADERS(sys/sockio.h glob.h net/if_tun.h)
AC_CHECK_HEADERS(net/pfkeyv2.h netipsec/ipsec.h netinet6/ipsec.h linux/udp.h)
AC_CHECK_HEADERS([netinet/ip6.h linux/fib_rules.h], [], [],
[
	#include <sys/types.h>
	#include <netinet/in.h>
])

AC_CHECK_MEMBERS([struct sockaddr.sa_len], [], [],
[
	#include <sys/types.h>
	#include <sys/socket.h>
])

AC_CHECK_MEMBERS([struct sadb_x_policy.sadb_x_policy_priority], [], [],
[
	#include <sys/types.h>
	#ifdef HAVE_NET_PFKEYV2_H
	#include <net/pfkeyv2.h>
	#else
	#include <stdint.h>
	#include <linux/pfkeyv2.h>
	#endif
])

AC_MSG_CHECKING([for in6addr_any])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/types.h>
		  #include <sys/socket.h>
		  #include <netinet/in.h>]],
		[[struct in6_addr in6 __attribute__((unused));
		  in6 = in6addr_any;]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_IN6ADDR_ANY], [], [have struct in6_addr in6addr_any])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for in6_pktinfo])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#define _GNU_SOURCE
		  #include <sys/types.h>
		  #include <sys/socket.h>
		  #include <netinet/in.h>]],
		[[struct in6_pktinfo pi;
		  if (pi.ipi6_ifindex)
		  {
		    return 0;
		  }]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_IN6_PKTINFO], [], [have struct in6_pktinfo.ipi6_ifindex])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for RTM_IFANNOUNCE])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/socket.h>
		  #include <net/if.h>
		  #include <net/route.h>]],
		[[return RTM_IFANNOUNCE;]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_RTM_IFANNOUNCE], [], [have PF_ROUTE RTM_IFANNOUNCE defined])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for IPSEC_MODE_BEET])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/types.h>
		  #ifdef HAVE_NETIPSEC_IPSEC_H
		  #include <netipsec/ipsec.h>
		  #elif defined(HAVE_NETINET6_IPSEC_H)
		  #include <netinet6/ipsec.h>
		  #else
		  #include <stdint.h>
		  #include <linux/ipsec.h>
		  #endif]],
		[[int mode = IPSEC_MODE_BEET;
		  return mode;]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_IPSEC_MODE_BEET], [], [have IPSEC_MODE_BEET defined])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for IPSEC_DIR_FWD])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/types.h>
		  #ifdef HAVE_NETIPSEC_IPSEC_H
		  #include <netipsec/ipsec.h>
		  #elif defined(HAVE_NETINET6_IPSEC_H)
		  #include <netinet6/ipsec.h>
		  #else
		  #include <stdint.h>
		  #include <linux/ipsec.h>
		  #endif]],
		[[int dir = IPSEC_DIR_FWD;
		  return dir;]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_IPSEC_DIR_FWD], [], [have IPSEC_DIR_FWD defined])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for RTA_TABLE])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <sys/socket.h>
		  #include <linux/netlink.h>
		  #include <linux/rtnetlink.h>]],
		[[int rta_type = RTA_TABLE;
		  return rta_type;]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_RTA_TABLE], [], [have netlink RTA_TABLE defined])],
	[AC_MSG_RESULT([no])]
)

AC_MSG_CHECKING([for GCC __sync operations])
AC_RUN_IFELSE([AC_LANG_SOURCE(
	[[
			int main() {
			int ref = 1;
			__sync_fetch_and_add (&ref, 1);
			__sync_sub_and_fetch (&ref, 1);
			__sync_val_compare_and_swap(&ref, 1, 0);
			return ref;
		}
	]])],
	[AC_MSG_RESULT([yes]);
	 AC_DEFINE([HAVE_GCC_SYNC_OPERATIONS], [],
		   [have GCC __sync_* operations])],
	[AC_MSG_RESULT([no])],
	[AC_MSG_RESULT([no])]
)

case "$printf_hooks" in
auto|builtin|glibc|vstr)
	;;
*)
	AC_MSG_NOTICE([invalid printf hook implementation, defaulting to 'auto'])
	printf_hooks=auto
	;;
esac

if test x$printf_hooks = xauto -o x$printf_hooks = xglibc; then
	# check for the new register_printf_specifier function with len argument,
	# or the deprecated register_printf_function without
	AC_CHECK_FUNC(
		[register_printf_specifier],
		[AC_DEFINE([HAVE_PRINTF_SPECIFIER], [], [have register_printf_specifier()])],
		[AC_CHECK_FUNC(
			[register_printf_function],
			[AC_DEFINE([HAVE_PRINTF_FUNCTION], [], [have register_printf_function()])],
			[
				AC_MSG_NOTICE([printf(3) does not support custom format specifiers!])
				if test x$printf_hooks = xglibc; then
					AC_MSG_ERROR([please select a different printf hook implementation])
				else
					# fallback to builtin printf hook implementation
					printf_hooks=builtin
				fi
			]
		)]
	)
fi

AC_MSG_CHECKING([for Windows target])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[[#include <windows.h>]],
		[[#ifndef WIN32
		  # error WIN32 undefined
		  #endif
		]])],
	[
		AC_MSG_RESULT([yes])
		windows=true
		openssl_lib=eay32
		AC_SUBST(PTHREADLIB, "")
		# explicitly disable ms-bitfields, as it breaks __attribute__((packed))
		case "$CFLAGS" in
			*ms-bitfields*) ;;
			*) CFLAGS="$CFLAGS -mno-ms-bitfields" ;;
		esac
	],
	[
		AC_MSG_RESULT([no])
		openssl_lib=crypto

		# check for clock_gettime() on non-Windows only. Otherwise this
		# check might find clock_gettime() in libwinpthread, but we don't want
		# to link against it.
		saved_LIBS=$LIBS
		# FreeBSD has clock_gettime in libc, Linux needs librt
		LIBS=""
		AC_SEARCH_LIBS(clock_gettime, rt, [RTLIB=$LIBS])
		AC_CHECK_FUNCS(clock_gettime)
		AC_SUBST(RTLIB)
		LIBS=$saved_LIBS
	]
)
AC_SUBST(OPENSSL_LIB, [-l$openssl_lib])
AM_CONDITIONAL(USE_WINDOWS, [test "x$windows" = xtrue])

AC_MSG_CHECKING([for working __attribute__((packed))])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([], [[
		struct test { char a; short b; } __attribute__((packed));
		char x[sizeof(struct test) == sizeof(char) + sizeof(short) ? 1 : -1]
			__attribute__((unused));
		return 0;
	]])],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no]); AC_MSG_ERROR([__attribute__((packed)) does not work])]
)

AC_MSG_CHECKING([clang])
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[], [[
		 #ifndef __clang__
		 # error not using LLVM clang
		 #endif
		]])],
	[
		AC_MSG_RESULT([yes])
	],
	[
		AC_MSG_RESULT([no])
		# GCC, but not MinGW requires -rdynamic for plugins
		if test x$windows != xtrue; then
			AC_SUBST(PLUGIN_CFLAGS, [-rdynamic])
		fi
	]
)

if test x$printf_hooks = xvstr; then
	AC_CHECK_LIB([vstr],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([Vstr string library not found])],[])
	AC_DEFINE([USE_VSTR], [], [use Vstr string library for printf hooks])
fi

if test x$printf_hooks = xbuiltin; then
	AC_DEFINE([USE_BUILTIN_PRINTF], [], [using builtin printf for printf hooks])
fi

if test x$gmp = xtrue; then
	saved_LIBS=$LIBS
	AC_CHECK_LIB([gmp],[main],[],[AC_MSG_ERROR([GNU Multi Precision library gmp not found])],[])
	AC_MSG_CHECKING([mpz_powm_sec])
	if test x$mpz_powm_sec = xyes; then
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM(
				[[#include "gmp.h"]],
				[[void *x = mpz_powm_sec;]])],
			[AC_MSG_RESULT([yes]);
			 AC_DEFINE([HAVE_MPZ_POWM_SEC], [], [have mpz_mown_sec()])],
			[AC_MSG_RESULT([no])]
		)
	else
		AC_MSG_RESULT([disabled])
	fi
	LIBS=$saved_LIBS
	AC_MSG_CHECKING([gmp.h version >= 4.1.4])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include "gmp.h"]],
			[[
				#if (__GNU_MP_VERSION*100 +  __GNU_MP_VERSION_MINOR*10 + __GNU_MP_VERSION_PATCHLEVEL) < 414
					#error bad gmp
				#endif]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no]); AC_MSG_ERROR([No usable gmp.h found!])]
	)
fi

if test x$ldap = xtrue; then
	AC_CHECK_LIB([ldap],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([LDAP library ldap not found])],[])
	AC_CHECK_LIB([lber],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([LDAP library lber not found])],[])
	AC_CHECK_HEADER([ldap.h],,[AC_MSG_ERROR([LDAP header ldap.h not found!])])
fi

if test x$curl = xtrue; then
	AC_CHECK_LIB([curl],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([CURL library curl not found])],[])
	AC_CHECK_HEADER([curl/curl.h],,[AC_MSG_ERROR([CURL header curl/curl.h not found!])])
fi

if test x$unbound = xtrue; then
	AC_HAVE_LIBRARY([ldns],[LIBS="$LIBS"],[AC_MSG_ERROR([UNBOUND library ldns not found])])
	AC_CHECK_HEADER([ldns/ldns.h],,[AC_MSG_ERROR([UNBOUND header ldns/ldns.h not found!])])
	AC_HAVE_LIBRARY([unbound],[LIBS="$LIBS"],[AC_MSG_ERROR([UNBOUND library libunbound not found])])
	AC_CHECK_HEADER([unbound.h],,[AC_MSG_ERROR([UNBOUND header unbound.h not found!])])
fi

if test x$soup = xtrue; then
	PKG_CHECK_MODULES(soup, [libsoup-2.4])
	AC_SUBST(soup_CFLAGS)
	AC_SUBST(soup_LIBS)
fi

if test x$xml = xtrue; then
	PKG_CHECK_MODULES(xml, [libxml-2.0])
	AC_SUBST(xml_CFLAGS)
	AC_SUBST(xml_LIBS)
fi

if test x$systemd = xtrue; then
	AC_MSG_CHECKING([for systemd system unit directory])
	if test -n "$systemdsystemunitdir" -a "x$systemdsystemunitdir" != xno; then
		AC_MSG_RESULT([$systemdsystemunitdir])
	else
		AC_MSG_ERROR([not found (try --with-systemdsystemunitdir)])
	fi

	PKG_CHECK_MODULES(systemd_daemon, [libsystemd-daemon])
	AC_SUBST(systemd_daemon_CFLAGS)
	AC_SUBST(systemd_daemon_LIBS)

	PKG_CHECK_MODULES(systemd_journal, [libsystemd-journal])
	AC_SUBST(systemd_journal_CFLAGS)
	AC_SUBST(systemd_journal_LIBS)
fi

if test x$tss = xtrousers; then
	AC_CHECK_LIB([tspi],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([TrouSerS library libtspi not found])],[])
	AC_CHECK_HEADER([trousers/tss.h],,[AC_MSG_ERROR([TrouSerS header trousers/tss.h not found!])])
	AC_DEFINE([TSS_TROUSERS], [], [use TrouSerS library libtspi as TSS implementation])
fi

if test x$imv_swid = xtrue; then
	PKG_CHECK_MODULES(json, [json-c], [],
		[PKG_CHECK_MODULES(json, [json])])
	AC_SUBST(json_CFLAGS)
	AC_SUBST(json_LIBS)
fi

if test x$dumm = xtrue; then
	PKG_CHECK_MODULES(gtk, [gtk+-2.0 vte])
	AC_SUBST(gtk_CFLAGS)
	AC_SUBST(gtk_LIBS)
	AC_CHECK_PROGS(RUBY, ruby)
	AC_MSG_CHECKING([for Ruby header files])
	if test -n "$RUBY"; then
		RUBYINCLUDE=
		RUBYDIR=`($RUBY -r rbconfig -e 'print RbConfig::CONFIG[["rubyhdrdir"]] || ""') 2>/dev/null`
		if test -n "$RUBYDIR" -a -r "$RUBYDIR/ruby.h"; then
			RUBYARCH=`($RUBY -r rbconfig -e 'print RbConfig::CONFIG[["arch"]] || ""') 2>/dev/null`
			if test -n "$RUBYARCH"; then
				AC_MSG_RESULT([$RUBYDIR])
				RUBYINCLUDE="-I$RUBYDIR -I$RUBYDIR/$RUBYARCH"
			fi
		else
			RUBYDIR=`($RUBY -r rbconfig -e 'print RbConfig::CONFIG[["archdir"]] || ""') 2>/dev/null`
			if test -n "$RUBYDIR" -a -r "$RUBYDIR/ruby.h"; then
				AC_MSG_RESULT([$RUBYDIR])
				RUBYINCLUDE="-I$RUBYDIR"
			fi
		fi
		if test -z "$RUBYINCLUDE"; then
			AC_MSG_ERROR([ruby.h not found])
		fi
		AC_SUBST(RUBYINCLUDE)
	else
		AC_MSG_ERROR([don't know how to run ruby])
	fi
	AC_MSG_CHECKING([for libruby])
	saved_LIBS=$LIBS
	LIBS=`($RUBY -r rbconfig -e 'print RbConfig::CONFIG[["LIBRUBYARG_SHARED"]] || ""') 2>/dev/null`
	AC_TRY_LINK_FUNC(ruby_init,
		[AC_MSG_RESULT([$LIBS]); RUBYLIB=$LIBS],
		[AC_MSG_ERROR([not found])])
	AC_SUBST(RUBYLIB)
	AC_CHECK_FUNCS(rb_errinfo)
	LIBS=$saved_LIBS
fi

if test x$fast = xtrue; then
	AC_CHECK_LIB([neo_cgi],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([ClearSilver library neo_cgi not found!])],[])
	AC_CHECK_LIB([neo_utl],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([ClearSilver library neo_utl not found!])],[])
	AC_MSG_CHECKING([ClearSilver requires zlib])
	saved_CFLAGS=$CFLAGS
	saved_LIBS=$LIBS
	LIBS="-lneo_cgi -lneo_cs -lneo_utl"
	CFLAGS="-I/usr/include/ClearSilver"
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <ClearSilver.h>]],
			[[NEOERR *err = cgi_display(NULL, NULL);]])],
		[AC_MSG_RESULT([no]); clearsilver_LIBS="$LIBS"],
		[AC_MSG_RESULT([yes]); clearsilver_LIBS="$LIBS -lz"]
	)
	AC_SUBST(clearsilver_LIBS)
	LIBS=$saved_LIBS
	CFLAGS=$saved_CFLAGS
# autoconf does not like CamelCase!? How to fix this?
#	AC_CHECK_HEADER([ClearSilver/ClearSilver.h],,[AC_MSG_ERROR([ClearSilver header file ClearSilver/ClearSilver.h not found!])])

	AC_CHECK_LIB([fcgi],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([FastCGI library fcgi not found!])],[])
	AC_CHECK_HEADER([fcgiapp.h],,[AC_MSG_ERROR([FastCGI header file fcgiapp.h not found!])])
fi

if test x$mysql = xtrue; then
	if test "x$windows" = xtrue; then
		AC_CHECK_HEADER([mysql.h],,[AC_MSG_ERROR([MySQL header file mysql.h not found!])])
		AC_CHECK_LIB([mysql],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([MySQL library not found!])],[])
		AC_SUBST(MYSQLLIB, -lmysql)
	else
		AC_PATH_PROG([MYSQLCONFIG], [mysql_config], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
		if test x$MYSQLCONFIG = x; then
			AC_MSG_ERROR([mysql_config not found!])
		fi
		AC_SUBST(MYSQLLIB, `$MYSQLCONFIG --libs_r`)
		AC_SUBST(MYSQLCFLAG, `$MYSQLCONFIG --cflags`)
	fi
fi

if test x$sqlite = xtrue; then
	AC_CHECK_LIB([sqlite3],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([SQLite library sqlite3 not found])],[])
	AC_CHECK_HEADER([sqlite3.h],,[AC_MSG_ERROR([SQLite header sqlite3.h not found!])])
	AC_MSG_CHECKING([sqlite3_prepare_v2])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <sqlite3.h>]],
			[[void *test = sqlite3_prepare_v2;]])],
		[AC_MSG_RESULT([yes]);
		 AC_DEFINE([HAVE_SQLITE3_PREPARE_V2], [], [have sqlite3_prepare_v2()])],
		[AC_MSG_RESULT([no])]
	)
	AC_MSG_CHECKING([sqlite3.h version >= 3.3.1])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <sqlite3.h>]],
			[[
				#if SQLITE_VERSION_NUMBER < 3003001
					#error bad sqlite
				#endif]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no]); AC_MSG_ERROR([SQLite version >= 3.3.1 required!])]
	)
fi

if test x$openssl = xtrue; then
	AC_CHECK_LIB([$openssl_lib],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([OpenSSL lib$openssl_lib not found])],[])
	AC_CHECK_HEADER([openssl/evp.h],,[AC_MSG_ERROR([OpenSSL header openssl/evp.h not found!])])
fi

if test x$gcrypt = xtrue; then
	AC_CHECK_LIB([gcrypt],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([gcrypt library not found])],[-lgpg-error])
	AC_CHECK_HEADER([gcrypt.h],,[AC_MSG_ERROR([gcrypt header gcrypt.h not found!])])
	AC_MSG_CHECKING([gcrypt CAMELLIA cipher])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#include <gcrypt.h>]],
			[[enum gcry_cipher_algos alg = GCRY_CIPHER_CAMELLIA128;]])],
		[AC_MSG_RESULT([yes]);
		 AC_DEFINE([HAVE_GCRY_CIPHER_CAMELLIA], [], [have GCRY_CIPHER_CAMELLIA128])],
		[AC_MSG_RESULT([no])]
	)
fi

if test x$uci = xtrue; then
	AC_CHECK_LIB([uci],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([UCI library libuci not found])],[])
	AC_CHECK_HEADER([uci.h],,[AC_MSG_ERROR([UCI header uci.h not found!])])
fi

if test x$android_dns = xtrue; then
	AC_CHECK_LIB([cutils],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([Android library libcutils not found])],[])
	AC_CHECK_HEADER([cutils/properties.h],,[AC_MSG_ERROR([Android header cutils/properties.h not found!])])
	# we have to force the use of libdl here because the autodetection
	# above does not work correctly when cross-compiling for android.
	DLLIB="-ldl"
	AC_SUBST(DLLIB)
fi

if test x$maemo = xtrue; then
	PKG_CHECK_MODULES(maemo, [glib-2.0 gthread-2.0 libosso osso-af-settings])
	AC_SUBST(maemo_CFLAGS)
	AC_SUBST(maemo_LIBS)
	dbusservicedir="/usr/share/dbus-1/system-services"
	AC_SUBST(dbusservicedir)
fi

if test x$eap_sim_pcsc = xtrue; then
	PKG_CHECK_MODULES(pcsclite, [libpcsclite])
	AC_SUBST(pcsclite_CFLAGS)
	AC_SUBST(pcsclite_LIBS)
fi

if test x$nm = xtrue; then
	PKG_CHECK_EXISTS([libnm-glib],
		[PKG_CHECK_MODULES(nm, [NetworkManager gthread-2.0 libnm-util libnm-glib libnm-glib-vpn])],
		[PKG_CHECK_MODULES(nm, [NetworkManager gthread-2.0 libnm_util libnm_glib libnm_glib_vpn])]
	)
	AC_SUBST(nm_CFLAGS)
	AC_SUBST(nm_LIBS)
fi

if test x$xauth_pam = xtrue; then
	AC_CHECK_LIB([pam],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([PAM library not found])],[])
	AC_CHECK_HEADER([security/pam_appl.h],,[AC_MSG_ERROR([PAM header security/pam_appl.h not found!])])
fi

if test x$capabilities = xnative; then
	AC_MSG_NOTICE([Usage of the native Linux capabilities interface is deprecated, use libcap instead])
	# Linux requires the following for capset(), Android does not have it,
	# but defines capset() in unistd.h instead.
	AC_CHECK_HEADERS([sys/capability.h])
	AC_CHECK_FUNC(capset,,[AC_MSG_ERROR([capset() not found!])])
	AC_DEFINE([CAPABILITIES_NATIVE], [], [have native linux capset()])
fi

if test x$capabilities = xlibcap; then
	AC_CHECK_LIB([cap],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([libcap library not found])],[])
	AC_CHECK_HEADER([sys/capability.h],
		[AC_DEFINE([HAVE_SYS_CAPABILITY_H], [], [have sys/capability.h])],
		[AC_MSG_ERROR([libcap header sys/capability.h not found!])])
	AC_DEFINE([CAPABILITIES_LIBCAP], [], [have libpcap library])
fi

if test x$integrity_test = xtrue; then
	AC_MSG_CHECKING([for dladdr()])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#define _GNU_SOURCE
			  #include <dlfcn.h>]],
			[[Dl_info info; dladdr(main, &info);]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no]);
		 AC_MSG_ERROR([dladdr() not supported, required by integrity-test!])]
	)
	AC_MSG_CHECKING([for dl_iterate_phdr()])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
			[[#define _GNU_SOURCE
			  #include <link.h>]],
			[[dl_iterate_phdr((void*)0, (void*)0);]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no]);
		 AC_MSG_ERROR([dl_iterate_phdr() not supported, required by integrity-test!])]
	)
fi

if test x$bfd_backtraces = xtrue; then
	AC_CHECK_LIB([bfd],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([binutils libbfd not found!])],[])
	AC_CHECK_HEADER([bfd.h],[AC_DEFINE([HAVE_BFD_H],,[have binutils bfd.h])],
		[AC_MSG_ERROR([binutils bfd.h header not found!])])
	BFDLIB="-lbfd"
	AC_SUBST(BFDLIB)
fi

if test x$unwind_backtraces = xtrue; then
	AC_CHECK_LIB([unwind],[main],[LIBS="$LIBS"],[AC_MSG_ERROR([libunwind not found!])],[])
	AC_CHECK_HEADER([libunwind.h],[AC_DEFINE([HAVE_LIBUNWIND_H],,[have libunwind.h])],
		[AC_MSG_ERROR([libunwind.h header not found!])])
	UNWINDLIB="-lunwind"
	AC_SUBST(UNWINDLIB)
fi

AM_CONDITIONAL(USE_DEV_HEADERS, [test "x$dev_headers" != xno])
if test x$dev_headers = xyes; then
	dev_headers="$includedir/strongswan"
fi
AC_SUBST(dev_headers)

CFLAGS="$CFLAGS -include `pwd`/config.h"

if test x$tkm = xtrue; then
	AC_PATH_PROG([GPRBUILD], [gprbuild], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
	if test x$GPRBUILD = x; then
		AC_MSG_ERROR([gprbuild not found])
	fi
fi

if test x$coverage = xtrue; then
	AC_PATH_PROG([LCOV], [lcov], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
	if test x$LCOV = x; then
		AC_MSG_ERROR([lcov not found])
	fi
	AC_PATH_PROG([GENHTML], [genhtml], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
	if test x$GENHTML = x; then
		AC_MSG_ERROR([genhtml not found])
	fi

	COVERAGE_CFLAGS="-fprofile-arcs -ftest-coverage"
	COVERAGE_LDFLAGS="-fprofile-arcs"
	AC_SUBST(COVERAGE_CFLAGS)
	AC_SUBST(COVERAGE_LDFLAGS)

	AC_MSG_NOTICE([coverage enabled, adding "-g -O0" to CFLAGS])
	CFLAGS="${CFLAGS} -g -O0"
fi

if test x$ruby_gems = xtrue; then
	AC_PATH_PROG([GEM], [gem], [], [$PATH:/bin:/usr/bin:/usr/local/bin])
	if test x$GEM = x; then
		AC_MSG_ERROR(RubyGems package manager not found)
	fi
	if test "x$rubygemdir" = "xgem environment gemdir"; then
		rubygemdir=$($GEM environment gemdir)
	fi
	AC_SUBST(RUBYGEMDIR, "$rubygemdir")
fi

# ===============================================
#  collect plugin list for strongSwan components
# ===============================================

m4_include(m4/macros/add-plugin.m4)

# plugin lists for all components
charon_plugins=
starter_plugins=
pool_plugins=
attest_plugins=
scepclient_plugins=
pki_plugins=
scripts_plugins=
manager_plugins=
medsrv_plugins=
nm_plugins=
cmd_plugins=
aikgen_plugins=

# location specific lists for checksumming,
# for src/libcharon, src/libhydra, src/libstrongswan and src/libtnccs
c_plugins=
h_plugins=
s_plugins=
t_plugins=

ADD_PLUGIN([test-vectors],         [s charon scepclient pki])
ADD_PLUGIN([unbound],              [s charon scripts])
ADD_PLUGIN([ldap],                 [s charon scepclient scripts nm cmd])
ADD_PLUGIN([mysql],                [s charon pool manager medsrv attest])
ADD_PLUGIN([sqlite],               [s charon pool manager medsrv attest])
ADD_PLUGIN([pkcs11],               [s charon pki nm cmd])
ADD_PLUGIN([aes],                  [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([des],                  [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([blowfish],             [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([rc2],                  [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([sha1],                 [s charon scepclient pki scripts medsrv attest nm cmd aikgen])
ADD_PLUGIN([sha2],                 [s charon scepclient pki scripts medsrv attest nm cmd aikgen])
ADD_PLUGIN([md4],                  [s charon manager scepclient pki nm cmd])
ADD_PLUGIN([md5],                  [s charon scepclient pki scripts attest nm cmd aikgen])
ADD_PLUGIN([rdrand],               [s charon scepclient pki scripts medsrv attest nm cmd aikgen])
ADD_PLUGIN([random],               [s charon scepclient pki scripts medsrv attest nm cmd aikgen])
ADD_PLUGIN([nonce],                [s charon nm cmd aikgen])
ADD_PLUGIN([x509],                 [s charon scepclient pki scripts attest nm cmd aikgen])
ADD_PLUGIN([revocation],           [s charon pki nm cmd])
ADD_PLUGIN([constraints],          [s charon nm cmd])
ADD_PLUGIN([acert],                [s charon])
ADD_PLUGIN([pubkey],               [s charon cmd aikgen])
ADD_PLUGIN([pkcs1],                [s charon scepclient pki scripts manager medsrv attest nm cmd aikgen])
ADD_PLUGIN([pkcs7],                [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([pkcs8],                [s charon scepclient pki scripts manager medsrv attest nm cmd])
ADD_PLUGIN([pkcs12],               [s charon scepclient pki scripts cmd])
ADD_PLUGIN([pgp],                  [s charon])
ADD_PLUGIN([dnskey],               [s charon pki])
ADD_PLUGIN([sshkey],               [s charon pki nm cmd])
ADD_PLUGIN([dnscert],              [c charon])
ADD_PLUGIN([ipseckey],             [c charon])
ADD_PLUGIN([pem],                  [s charon scepclient pki scripts manager medsrv attest nm cmd aikgen])
ADD_PLUGIN([padlock],              [s charon])
ADD_PLUGIN([openssl],              [s charon scepclient pki scripts manager medsrv attest nm cmd aikgen])
ADD_PLUGIN([gcrypt],               [s charon scepclient pki scripts manager medsrv attest nm cmd aikgen])
ADD_PLUGIN([af-alg],               [s charon scepclient pki scripts medsrv attest nm cmd aikgen])
ADD_PLUGIN([fips-prf],             [s charon nm cmd])
ADD_PLUGIN([gmp],                  [s charon scepclient pki scripts manager medsrv attest nm cmd aikgen])
ADD_PLUGIN([agent],                [s charon nm cmd])
ADD_PLUGIN([keychain],             [s charon cmd])
ADD_PLUGIN([xcbc],                 [s charon nm cmd])
ADD_PLUGIN([cmac],                 [s charon nm cmd])
ADD_PLUGIN([hmac],                 [s charon scripts nm cmd])
ADD_PLUGIN([ctr],                  [s charon scripts nm cmd])
ADD_PLUGIN([ccm],                  [s charon scripts nm cmd])
ADD_PLUGIN([gcm],                  [s charon scripts nm cmd])
ADD_PLUGIN([ntru],                 [s charon scripts nm cmd])
ADD_PLUGIN([curl],                 [s charon scepclient pki scripts nm cmd])
ADD_PLUGIN([winhttp],              [s charon pki scripts])
ADD_PLUGIN([soup],                 [s charon pki scripts nm cmd])
ADD_PLUGIN([attr],                 [h charon])
ADD_PLUGIN([attr-sql],             [h charon])
ADD_PLUGIN([load-tester],          [c charon])
ADD_PLUGIN([kernel-libipsec],      [c charon cmd])
ADD_PLUGIN([kernel-wfp],           [c charon])
ADD_PLUGIN([kernel-iph],           [c charon])
ADD_PLUGIN([kernel-pfkey],         [h charon starter nm cmd])
ADD_PLUGIN([kernel-pfroute],       [h charon starter nm cmd])
ADD_PLUGIN([kernel-netlink],       [h charon starter nm cmd])
ADD_PLUGIN([resolve],              [h charon cmd])
ADD_PLUGIN([socket-default],       [c charon nm cmd])
ADD_PLUGIN([socket-dynamic],       [c charon cmd])
ADD_PLUGIN([socket-win],           [c charon])
ADD_PLUGIN([farp],                 [c charon])
ADD_PLUGIN([stroke],               [c charon])
ADD_PLUGIN([vici],                 [c charon])
ADD_PLUGIN([smp],                  [c charon])
ADD_PLUGIN([sql],                  [c charon])
ADD_PLUGIN([updown],               [c charon])
ADD_PLUGIN([eap-identity],         [c charon nm cmd])
ADD_PLUGIN([eap-sim],              [c charon])
ADD_PLUGIN([eap-sim-file],         [c charon])
ADD_PLUGIN([eap-sim-pcsc],         [c charon])
ADD_PLUGIN([eap-aka],              [c charon])
ADD_PLUGIN([eap-aka-3gpp2],        [c charon])
ADD_PLUGIN([eap-simaka-sql],       [c charon])
ADD_PLUGIN([eap-simaka-pseudonym], [c charon])
ADD_PLUGIN([eap-simaka-reauth],    [c charon])
ADD_PLUGIN([eap-md5],              [c charon nm cmd])
ADD_PLUGIN([eap-gtc],              [c charon nm cmd])
ADD_PLUGIN([eap-mschapv2],         [c charon nm cmd])
ADD_PLUGIN([eap-dynamic],          [c charon])
ADD_PLUGIN([eap-radius],           [c charon])
ADD_PLUGIN([eap-tls],              [c charon nm cmd])
ADD_PLUGIN([eap-ttls],             [c charon nm cmd])
ADD_PLUGIN([eap-peap],             [c charon nm cmd])
ADD_PLUGIN([eap-tnc],              [c charon])
ADD_PLUGIN([xauth-generic],        [c charon cmd])
ADD_PLUGIN([xauth-eap],            [c charon])
ADD_PLUGIN([xauth-pam],            [c charon])
ADD_PLUGIN([xauth-noauth],         [c charon])
ADD_PLUGIN([tnc-ifmap],            [c charon])
ADD_PLUGIN([tnc-pdp],              [c charon])
ADD_PLUGIN([tnc-imc],              [t charon])
ADD_PLUGIN([tnc-imv],              [t charon])
ADD_PLUGIN([tnc-tnccs],            [t charon])
ADD_PLUGIN([tnccs-20],             [t charon])
ADD_PLUGIN([tnccs-11],             [t charon])
ADD_PLUGIN([tnccs-dynamic],        [t charon])
ADD_PLUGIN([medsrv],               [c charon])
ADD_PLUGIN([medcli],               [c charon])
ADD_PLUGIN([dhcp],                 [c charon])
ADD_PLUGIN([osx-attr],             [c charon cmd])
ADD_PLUGIN([android-dns],          [c charon])
ADD_PLUGIN([android-log],          [c charon])
ADD_PLUGIN([ha],                   [c charon])
ADD_PLUGIN([whitelist],            [c charon])
ADD_PLUGIN([ext-auth],             [c charon])
ADD_PLUGIN([lookip],               [c charon])
ADD_PLUGIN([error-notify],         [c charon])
ADD_PLUGIN([certexpire],           [c charon])
ADD_PLUGIN([systime-fix],          [c charon])
ADD_PLUGIN([led],                  [c charon])
ADD_PLUGIN([duplicheck],           [c charon])
ADD_PLUGIN([coupling],             [c charon])
ADD_PLUGIN([radattr],              [c charon])
ADD_PLUGIN([maemo],                [c charon])
ADD_PLUGIN([uci],                  [c charon])
ADD_PLUGIN([addrblock],            [c charon])
ADD_PLUGIN([unity],                [c charon])
ADD_PLUGIN([unit-tester],          [c charon])

AC_SUBST(charon_plugins)
AC_SUBST(starter_plugins)
AC_SUBST(pool_plugins)
AC_SUBST(attest_plugins)
AC_SUBST(scepclient_plugins)
AC_SUBST(pki_plugins)
AC_SUBST(scripts_plugins)
AC_SUBST(manager_plugins)
AC_SUBST(medsrv_plugins)
AC_SUBST(nm_plugins)
AC_SUBST(cmd_plugins)
AC_SUBST(aikgen_plugins)

AC_SUBST(c_plugins)
AC_SUBST(h_plugins)
AC_SUBST(s_plugins)
AC_SUBST(t_plugins)

# ======================
#  set Makefile.am vars
# ======================

#  libstrongswan plugins
# -----------------------
AM_CONDITIONAL(USE_TEST_VECTORS, test x$test_vectors = xtrue)
AM_CONDITIONAL(USE_CURL, test x$curl = xtrue)
AM_CONDITIONAL(USE_WINHTTP, test x$winhttp = xtrue)
AM_CONDITIONAL(USE_UNBOUND, test x$unbound = xtrue)
AM_CONDITIONAL(USE_SOUP, test x$soup = xtrue)
AM_CONDITIONAL(USE_LDAP, test x$ldap = xtrue)
AM_CONDITIONAL(USE_AES, test x$aes = xtrue)
AM_CONDITIONAL(USE_DES, test x$des = xtrue)
AM_CONDITIONAL(USE_BLOWFISH, test x$blowfish = xtrue)
AM_CONDITIONAL(USE_RC2, test x$rc2 = xtrue)
AM_CONDITIONAL(USE_MD4, test x$md4 = xtrue)
AM_CONDITIONAL(USE_MD5, test x$md5 = xtrue)
AM_CONDITIONAL(USE_SHA1, test x$sha1 = xtrue)
AM_CONDITIONAL(USE_SHA2, test x$sha2 = xtrue)
AM_CONDITIONAL(USE_FIPS_PRF, test x$fips_prf = xtrue)
AM_CONDITIONAL(USE_GMP, test x$gmp = xtrue)
AM_CONDITIONAL(USE_RDRAND, test x$rdrand = xtrue)
AM_CONDITIONAL(USE_RANDOM, test x$random = xtrue)
AM_CONDITIONAL(USE_NONCE, test x$nonce = xtrue)
AM_CONDITIONAL(USE_X509, test x$x509 = xtrue)
AM_CONDITIONAL(USE_REVOCATION, test x$revocation = xtrue)
AM_CONDITIONAL(USE_CONSTRAINTS, test x$constraints = xtrue)
AM_CONDITIONAL(USE_ACERT, test x$acert = xtrue)
AM_CONDITIONAL(USE_PUBKEY, test x$pubkey = xtrue)
AM_CONDITIONAL(USE_PKCS1, test x$pkcs1 = xtrue)
AM_CONDITIONAL(USE_PKCS7, test x$pkcs7 = xtrue)
AM_CONDITIONAL(USE_PKCS8, test x$pkcs8 = xtrue)
AM_CONDITIONAL(USE_PKCS12, test x$pkcs12 = xtrue)
AM_CONDITIONAL(USE_PGP, test x$pgp = xtrue)
AM_CONDITIONAL(USE_DNSKEY, test x$dnskey = xtrue)
AM_CONDITIONAL(USE_SSHKEY, test x$sshkey = xtrue)
AM_CONDITIONAL(USE_PEM, test x$pem = xtrue)
AM_CONDITIONAL(USE_HMAC, test x$hmac = xtrue)
AM_CONDITIONAL(USE_CMAC, test x$cmac = xtrue)
AM_CONDITIONAL(USE_XCBC, test x$xcbc = xtrue)
AM_CONDITIONAL(USE_MYSQL, test x$mysql = xtrue)
AM_CONDITIONAL(USE_SQLITE, test x$sqlite = xtrue)
AM_CONDITIONAL(USE_PADLOCK, test x$padlock = xtrue)
AM_CONDITIONAL(USE_OPENSSL, test x$openssl = xtrue)
AM_CONDITIONAL(USE_GCRYPT, test x$gcrypt = xtrue)
AM_CONDITIONAL(USE_AGENT, test x$agent = xtrue)
AM_CONDITIONAL(USE_KEYCHAIN, test x$keychain = xtrue)
AM_CONDITIONAL(USE_PKCS11, test x$pkcs11 = xtrue)
AM_CONDITIONAL(USE_CTR, test x$ctr = xtrue)
AM_CONDITIONAL(USE_CCM, test x$ccm = xtrue)
AM_CONDITIONAL(USE_GCM, test x$gcm = xtrue)
AM_CONDITIONAL(USE_AF_ALG, test x$af_alg = xtrue)
AM_CONDITIONAL(USE_NTRU, test x$ntru = xtrue)

#  charon plugins
# ----------------
AM_CONDITIONAL(USE_STROKE, test x$stroke = xtrue)
AM_CONDITIONAL(USE_VICI, test x$vici = xtrue)
AM_CONDITIONAL(USE_MEDSRV, test x$medsrv = xtrue)
AM_CONDITIONAL(USE_MEDCLI, test x$medcli = xtrue)
AM_CONDITIONAL(USE_UCI, test x$uci = xtrue)
AM_CONDITIONAL(USE_OSX_ATTR, test x$osx_attr = xtrue)
AM_CONDITIONAL(USE_ANDROID_DNS, test x$android_dns = xtrue)
AM_CONDITIONAL(USE_ANDROID_LOG, test x$android_log = xtrue)
AM_CONDITIONAL(USE_MAEMO, test x$maemo = xtrue)
AM_CONDITIONAL(USE_SMP, test x$smp = xtrue)
AM_CONDITIONAL(USE_SQL, test x$sql = xtrue)
AM_CONDITIONAL(USE_DNSCERT, test x$dnscert = xtrue)
AM_CONDITIONAL(USE_IPSECKEY, test x$ipseckey = xtrue)
AM_CONDITIONAL(USE_UPDOWN, test x$updown = xtrue)
AM_CONDITIONAL(USE_DHCP, test x$dhcp = xtrue)
AM_CONDITIONAL(USE_UNIT_TESTS, test x$unit_tester = xtrue)
AM_CONDITIONAL(USE_LOAD_TESTER, test x$load_tester = xtrue)
AM_CONDITIONAL(USE_HA, test x$ha = xtrue)
AM_CONDITIONAL(USE_KERNEL_LIBIPSEC, test x$kernel_libipsec = xtrue)
AM_CONDITIONAL(USE_KERNEL_WFP, test x$kernel_wfp = xtrue)
AM_CONDITIONAL(USE_KERNEL_IPH, test x$kernel_iph = xtrue)
AM_CONDITIONAL(USE_WHITELIST, test x$whitelist = xtrue)
AM_CONDITIONAL(USE_EXT_AUTH, test x$ext_auth = xtrue)
AM_CONDITIONAL(USE_LOOKIP, test x$lookip = xtrue)
AM_CONDITIONAL(USE_ERROR_NOTIFY, test x$error_notify = xtrue)
AM_CONDITIONAL(USE_CERTEXPIRE, test x$certexpire = xtrue)
AM_CONDITIONAL(USE_SYSTIME_FIX, test x$systime_fix = xtrue)
AM_CONDITIONAL(USE_LED, test x$led = xtrue)
AM_CONDITIONAL(USE_DUPLICHECK, test x$duplicheck = xtrue)
AM_CONDITIONAL(USE_COUPLING, test x$coupling = xtrue)
AM_CONDITIONAL(USE_RADATTR, test x$radattr = xtrue)
AM_CONDITIONAL(USE_EAP_SIM, test x$eap_sim = xtrue)
AM_CONDITIONAL(USE_EAP_SIM_FILE, test x$eap_sim_file = xtrue)
AM_CONDITIONAL(USE_EAP_SIM_PCSC, test x$eap_sim_pcsc = xtrue)
AM_CONDITIONAL(USE_EAP_SIMAKA_SQL, test x$eap_simaka_sql = xtrue)
AM_CONDITIONAL(USE_EAP_SIMAKA_PSEUDONYM, test x$eap_simaka_pseudonym = xtrue)
AM_CONDITIONAL(USE_EAP_SIMAKA_REAUTH, test x$eap_simaka_reauth = xtrue)
AM_CONDITIONAL(USE_EAP_IDENTITY, test x$eap_identity = xtrue)
AM_CONDITIONAL(USE_EAP_MD5, test x$eap_md5 = xtrue)
AM_CONDITIONAL(USE_EAP_GTC, test x$eap_gtc = xtrue)
AM_CONDITIONAL(USE_EAP_AKA, test x$eap_aka = xtrue)
AM_CONDITIONAL(USE_EAP_AKA_3GPP2, test x$eap_aka_3gpp2 = xtrue)
AM_CONDITIONAL(USE_EAP_MSCHAPV2, test x$eap_mschapv2 = xtrue)
AM_CONDITIONAL(USE_EAP_TLS, test x$eap_tls = xtrue)
AM_CONDITIONAL(USE_EAP_TTLS, test x$eap_ttls = xtrue)
AM_CONDITIONAL(USE_EAP_PEAP, test x$eap_peap = xtrue)
AM_CONDITIONAL(USE_EAP_TNC, test x$eap_tnc = xtrue)
AM_CONDITIONAL(USE_EAP_DYNAMIC, test x$eap_dynamic = xtrue)
AM_CONDITIONAL(USE_EAP_RADIUS, test x$eap_radius = xtrue)
AM_CONDITIONAL(USE_XAUTH_GENERIC, test x$xauth_generic = xtrue)
AM_CONDITIONAL(USE_XAUTH_EAP, test x$xauth_eap = xtrue)
AM_CONDITIONAL(USE_XAUTH_PAM, test x$xauth_pam = xtrue)
AM_CONDITIONAL(USE_XAUTH_NOAUTH, test x$xauth_noauth = xtrue)
AM_CONDITIONAL(USE_TNC_IFMAP, test x$tnc_ifmap = xtrue)
AM_CONDITIONAL(USE_TNC_PDP, test x$tnc_pdp = xtrue)
AM_CONDITIONAL(USE_TNC_IMC, test x$tnc_imc = xtrue)
AM_CONDITIONAL(USE_TNC_IMV, test x$tnc_imv = xtrue)
AM_CONDITIONAL(USE_TNC_TNCCS, test x$tnc_tnccs = xtrue)
AM_CONDITIONAL(USE_TNCCS_11, test x$tnccs_11 = xtrue)
AM_CONDITIONAL(USE_TNCCS_20, test x$tnccs_20 = xtrue)
AM_CONDITIONAL(USE_TNCCS_DYNAMIC, test x$tnccs_dynamic = xtrue)
AM_CONDITIONAL(USE_IMC_TEST, test x$imc_test = xtrue)
AM_CONDITIONAL(USE_IMV_TEST, test x$imv_test = xtrue)
AM_CONDITIONAL(USE_IMC_SCANNER, test x$imc_scanner = xtrue)
AM_CONDITIONAL(USE_IMV_SCANNER, test x$imv_scanner = xtrue)
AM_CONDITIONAL(USE_IMC_OS, test x$imc_os = xtrue)
AM_CONDITIONAL(USE_IMV_OS, test x$imv_os = xtrue)
AM_CONDITIONAL(USE_IMC_ATTESTATION, test x$imc_attestation = xtrue)
AM_CONDITIONAL(USE_IMV_ATTESTATION, test x$imv_attestation = xtrue)
AM_CONDITIONAL(USE_IMC_SWID, test x$imc_swid = xtrue)
AM_CONDITIONAL(USE_IMV_SWID, test x$imv_swid = xtrue)
AM_CONDITIONAL(USE_SOCKET_DEFAULT, test x$socket_default = xtrue)
AM_CONDITIONAL(USE_SOCKET_DYNAMIC, test x$socket_dynamic = xtrue)
AM_CONDITIONAL(USE_SOCKET_WIN, test x$socket_win = xtrue)
AM_CONDITIONAL(USE_FARP, test x$farp = xtrue)
AM_CONDITIONAL(USE_ADDRBLOCK, test x$addrblock = xtrue)
AM_CONDITIONAL(USE_UNITY, test x$unity = xtrue)

#  hydra plugins
# ---------------
AM_CONDITIONAL(USE_ATTR, test x$attr = xtrue)
AM_CONDITIONAL(USE_ATTR_SQL, test x$attr_sql = xtrue)
AM_CONDITIONAL(USE_KERNEL_NETLINK, test x$kernel_netlink = xtrue)
AM_CONDITIONAL(USE_KERNEL_PFKEY, test x$kernel_pfkey = xtrue)
AM_CONDITIONAL(USE_KERNEL_PFROUTE, test x$kernel_pfroute = xtrue)
AM_CONDITIONAL(USE_RESOLVE, test x$resolve = xtrue)

#  other options
# ---------------
AM_CONDITIONAL(USE_LEAK_DETECTIVE, test x$leak_detective = xtrue)
AM_CONDITIONAL(USE_LOCK_PROFILER, test x$lock_profiler = xtrue)
AM_CONDITIONAL(USE_DUMM, test x$dumm = xtrue)
AM_CONDITIONAL(USE_FAST, test x$fast = xtrue)
AM_CONDITIONAL(USE_MANAGER, test x$manager = xtrue)
AM_CONDITIONAL(USE_ME, test x$mediation = xtrue)
AM_CONDITIONAL(USE_INTEGRITY_TEST, test x$integrity_test = xtrue)
AM_CONDITIONAL(USE_LOAD_WARNING, test x$load_warning = xtrue)
AM_CONDITIONAL(USE_IKEV1, test x$ikev1 = xtrue)
AM_CONDITIONAL(USE_IKEV2, test x$ikev2 = xtrue)
AM_CONDITIONAL(USE_THREADS, test x$threads = xtrue)
AM_CONDITIONAL(USE_ADNS, test x$adns = xtrue)
AM_CONDITIONAL(USE_CHARON, test x$charon = xtrue)
AM_CONDITIONAL(USE_NM, test x$nm = xtrue)
AM_CONDITIONAL(USE_PKI, test x$pki = xtrue)
AM_CONDITIONAL(USE_SCEPCLIENT, test x$scepclient = xtrue)
AM_CONDITIONAL(USE_SCRIPTS, test x$scripts = xtrue)
AM_CONDITIONAL(USE_CONFTEST, test x$conftest = xtrue)
AM_CONDITIONAL(USE_LIBSTRONGSWAN, test x$charon = xtrue -o x$pki = xtrue -o x$scepclient = xtrue -o x$conftest = xtrue -o x$fast = xtrue -o x$imcv = xtrue -o x$nm = xtrue -o x$tkm = xtrue -o x$cmd = xtrue -o x$tls = xtrue -o x$tnc_tnccs = xtrue -o x$aikgen = xtrue -o x$svc = xtrue -o x$systemd = xtrue)
AM_CONDITIONAL(USE_LIBHYDRA, test x$charon = xtrue -o x$nm = xtrue -o x$tkm = xtrue -o x$cmd = xtrue -o x$svc = xtrue -o x$systemd = xtrue)
AM_CONDITIONAL(USE_LIBCHARON, test x$charon = xtrue -o x$conftest = xtrue -o x$nm = xtrue -o x$tkm = xtrue -o x$cmd = xtrue -o x$svc = xtrue -o x$systemd = xtrue)
AM_CONDITIONAL(USE_LIBIPSEC, test x$libipsec = xtrue)
AM_CONDITIONAL(USE_LIBTNCIF, test x$tnc_tnccs = xtrue -o x$imcv = xtrue)
AM_CONDITIONAL(USE_LIBTNCCS, test x$tnc_tnccs = xtrue)
AM_CONDITIONAL(USE_LIBPTTLS, test x$tnc_tnccs = xtrue)
AM_CONDITIONAL(USE_FILE_CONFIG, test x$stroke = xtrue)
AM_CONDITIONAL(USE_IPSEC_SCRIPT, test x$stroke = xtrue -o x$scepclient = xtrue -o x$conftest = xtrue)
AM_CONDITIONAL(USE_LIBCAP, test x$capabilities = xlibcap)
AM_CONDITIONAL(USE_VSTR, test x$printf_hooks = xvstr)
AM_CONDITIONAL(USE_BUILTIN_PRINTF, test x$printf_hooks = xbuiltin)
AM_CONDITIONAL(USE_SIMAKA, test x$simaka = xtrue)
AM_CONDITIONAL(USE_TLS, test x$tls = xtrue)
AM_CONDITIONAL(USE_RADIUS, test x$radius = xtrue)
AM_CONDITIONAL(USE_IMCV, test x$imcv = xtrue)
AM_CONDITIONAL(USE_TROUSERS, test x$tss = xtrousers -o x$aikgen = xtrue)
AM_CONDITIONAL(MONOLITHIC, test x$monolithic = xtrue)
AM_CONDITIONAL(USE_SILENT_RULES, test x$enable_silent_rules = xyes)
AM_CONDITIONAL(COVERAGE, test x$coverage = xtrue)
AM_CONDITIONAL(USE_DBGHELP, test x$dbghelp_backtraces = xtrue)
AM_CONDITIONAL(USE_TKM, test x$tkm = xtrue)
AM_CONDITIONAL(USE_CMD, test x$cmd = xtrue)
AM_CONDITIONAL(USE_AIKGEN, test x$aikgen = xtrue)
AM_CONDITIONAL(USE_SWANCTL, test x$swanctl = xtrue)
AM_CONDITIONAL(USE_SVC, test x$svc = xtrue)
AM_CONDITIONAL(USE_SYSTEMD, test x$systemd = xtrue)
AM_CONDITIONAL(USE_LEGACY_SYSTEMD, test -n "$systemdsystemunitdir" -a "x$systemdsystemunitdir" != xno)
AM_CONDITIONAL(USE_RUBY_GEMS, test x$ruby_gems = xtrue)

# ========================
#  set global definitions
# ========================

if test x$mediation = xtrue; then
	AC_DEFINE([ME], [], [mediation extension support])
fi
if test x$capabilities = xlibcap -o x$capabilities = xnative; then
	AC_DEFINE([CAPABILITIES], [], [capability dropping support])
fi
if test x$monolithic = xtrue; then
	AC_DEFINE([MONOLITHIC], [], [monolithic build embedding plugins])
fi
if test x$ikev1 = xtrue; then
	AC_DEFINE([USE_IKEV1], [], [support for IKEv1 protocol])
fi
if test x$ikev2 = xtrue; then
	AC_DEFINE([USE_IKEV2], [], [support for IKEv2 protocol])
fi

# ====================================================
#  options for enabled modules (see conf/Makefile.am)
# ====================================================

strongswan_options=

AM_COND_IF([USE_AIKGEN], [strongswan_options=${strongswan_options}" aikgen"])
AM_COND_IF([USE_ATTR_SQL], [strongswan_options=${strongswan_options}" pool"])
AM_COND_IF([USE_CHARON], [strongswan_options=${strongswan_options}" charon charon-logging"])
AM_COND_IF([USE_FILE_CONFIG], [strongswan_options=${strongswan_options}" starter"])
AM_COND_IF([USE_IMV_ATTESTATION], [strongswan_options=${strongswan_options}" attest"])
AM_COND_IF([USE_IMCV], [strongswan_options=${strongswan_options}" imcv"])
AM_COND_IF([USE_IMV_OS], [strongswan_options=${strongswan_options}" pacman"])
AM_COND_IF([USE_LIBTNCCS], [strongswan_options=${strongswan_options}" tnc"])
AM_COND_IF([USE_MANAGER], [strongswan_options=${strongswan_options}" manager"])
AM_COND_IF([USE_MEDSRV], [strongswan_options=${strongswan_options}" medsrv"])
AM_COND_IF([USE_SCEPCLIENT], [strongswan_options=${strongswan_options}" scepclient"])
AM_COND_IF([USE_PKI], [strongswan_options=${strongswan_options}" pki"])
AM_COND_IF([USE_SWANCTL], [strongswan_options=${strongswan_options}" swanctl"])

AC_SUBST(strongswan_options)

# =================
#  build Makefiles
# =================

AC_CONFIG_FILES([
	Makefile
	conf/Makefile
	man/Makefile
	init/Makefile
	init/systemd/Makefile
	init/systemd-swanctl/Makefile
	src/Makefile
	src/include/Makefile
	src/libstrongswan/Makefile
	src/libstrongswan/plugins/aes/Makefile
	src/libstrongswan/plugins/cmac/Makefile
	src/libstrongswan/plugins/des/Makefile
	src/libstrongswan/plugins/blowfish/Makefile
	src/libstrongswan/plugins/rc2/Makefile
	src/libstrongswan/plugins/md4/Makefile
	src/libstrongswan/plugins/md5/Makefile
	src/libstrongswan/plugins/sha1/Makefile
	src/libstrongswan/plugins/sha2/Makefile
	src/libstrongswan/plugins/fips_prf/Makefile
	src/libstrongswan/plugins/gmp/Makefile
	src/libstrongswan/plugins/rdrand/Makefile
	src/libstrongswan/plugins/random/Makefile
	src/libstrongswan/plugins/nonce/Makefile
	src/libstrongswan/plugins/hmac/Makefile
	src/libstrongswan/plugins/xcbc/Makefile
	src/libstrongswan/plugins/x509/Makefile
	src/libstrongswan/plugins/revocation/Makefile
	src/libstrongswan/plugins/constraints/Makefile
	src/libstrongswan/plugins/acert/Makefile
	src/libstrongswan/plugins/pubkey/Makefile
	src/libstrongswan/plugins/pkcs1/Makefile
	src/libstrongswan/plugins/pkcs7/Makefile
	src/libstrongswan/plugins/pkcs8/Makefile
	src/libstrongswan/plugins/pkcs12/Makefile
	src/libstrongswan/plugins/pgp/Makefile
	src/libstrongswan/plugins/dnskey/Makefile
	src/libstrongswan/plugins/sshkey/Makefile
	src/libstrongswan/plugins/pem/Makefile
	src/libstrongswan/plugins/curl/Makefile
	src/libstrongswan/plugins/winhttp/Makefile
	src/libstrongswan/plugins/unbound/Makefile
	src/libstrongswan/plugins/soup/Makefile
	src/libstrongswan/plugins/ldap/Makefile
	src/libstrongswan/plugins/mysql/Makefile
	src/libstrongswan/plugins/sqlite/Makefile
	src/libstrongswan/plugins/padlock/Makefile
	src/libstrongswan/plugins/openssl/Makefile
	src/libstrongswan/plugins/gcrypt/Makefile
	src/libstrongswan/plugins/agent/Makefile
	src/libstrongswan/plugins/keychain/Makefile
	src/libstrongswan/plugins/pkcs11/Makefile
	src/libstrongswan/plugins/ctr/Makefile
	src/libstrongswan/plugins/ccm/Makefile
	src/libstrongswan/plugins/gcm/Makefile
	src/libstrongswan/plugins/af_alg/Makefile
	src/libstrongswan/plugins/ntru/Makefile
	src/libstrongswan/plugins/test_vectors/Makefile
	src/libstrongswan/tests/Makefile
	src/libhydra/Makefile
	src/libhydra/plugins/attr/Makefile
	src/libhydra/plugins/attr_sql/Makefile
	src/libhydra/plugins/kernel_netlink/Makefile
	src/libhydra/plugins/kernel_pfkey/Makefile
	src/libhydra/plugins/kernel_pfroute/Makefile
	src/libhydra/plugins/resolve/Makefile
	src/libipsec/Makefile
	src/libsimaka/Makefile
	src/libtls/Makefile
	src/libtls/tests/Makefile
	src/libradius/Makefile
	src/libtncif/Makefile
	src/libtnccs/Makefile
	src/libtnccs/plugins/tnc_tnccs/Makefile
	src/libtnccs/plugins/tnc_imc/Makefile
	src/libtnccs/plugins/tnc_imv/Makefile
	src/libtnccs/plugins/tnccs_11/Makefile
	src/libtnccs/plugins/tnccs_20/Makefile
	src/libtnccs/plugins/tnccs_dynamic/Makefile
	src/libpttls/Makefile
	src/libimcv/Makefile
	src/libimcv/plugins/imc_test/Makefile
	src/libimcv/plugins/imv_test/Makefile
	src/libimcv/plugins/imc_scanner/Makefile
	src/libimcv/plugins/imv_scanner/Makefile
	src/libimcv/plugins/imc_os/Makefile
	src/libimcv/plugins/imv_os/Makefile
	src/libimcv/plugins/imc_attestation/Makefile
	src/libimcv/plugins/imv_attestation/Makefile
	src/libimcv/plugins/imc_swid/Makefile
	src/libimcv/plugins/imv_swid/Makefile
	src/charon/Makefile
	src/charon-nm/Makefile
	src/charon-tkm/Makefile
	src/charon-cmd/Makefile
	src/charon-svc/Makefile
	src/charon-systemd/Makefile
	src/libcharon/Makefile
	src/libcharon/plugins/eap_aka/Makefile
	src/libcharon/plugins/eap_aka_3gpp2/Makefile
	src/libcharon/plugins/eap_dynamic/Makefile
	src/libcharon/plugins/eap_identity/Makefile
	src/libcharon/plugins/eap_md5/Makefile
	src/libcharon/plugins/eap_gtc/Makefile
	src/libcharon/plugins/eap_sim/Makefile
	src/libcharon/plugins/eap_sim_file/Makefile
	src/libcharon/plugins/eap_sim_pcsc/Makefile
	src/libcharon/plugins/eap_simaka_sql/Makefile
	src/libcharon/plugins/eap_simaka_pseudonym/Makefile
	src/libcharon/plugins/eap_simaka_reauth/Makefile
	src/libcharon/plugins/eap_mschapv2/Makefile
	src/libcharon/plugins/eap_tls/Makefile
	src/libcharon/plugins/eap_ttls/Makefile
	src/libcharon/plugins/eap_peap/Makefile
	src/libcharon/plugins/eap_tnc/Makefile
	src/libcharon/plugins/eap_radius/Makefile
	src/libcharon/plugins/xauth_generic/Makefile
	src/libcharon/plugins/xauth_eap/Makefile
	src/libcharon/plugins/xauth_pam/Makefile
	src/libcharon/plugins/xauth_noauth/Makefile
	src/libcharon/plugins/tnc_ifmap/Makefile
	src/libcharon/plugins/tnc_pdp/Makefile
	src/libcharon/plugins/socket_default/Makefile
	src/libcharon/plugins/socket_dynamic/Makefile
	src/libcharon/plugins/socket_win/Makefile
	src/libcharon/plugins/farp/Makefile
	src/libcharon/plugins/smp/Makefile
	src/libcharon/plugins/sql/Makefile
	src/libcharon/plugins/dnscert/Makefile
	src/libcharon/plugins/ipseckey/Makefile
	src/libcharon/plugins/medsrv/Makefile
	src/libcharon/plugins/medcli/Makefile
	src/libcharon/plugins/addrblock/Makefile
	src/libcharon/plugins/unity/Makefile
	src/libcharon/plugins/uci/Makefile
	src/libcharon/plugins/ha/Makefile
	src/libcharon/plugins/kernel_libipsec/Makefile
	src/libcharon/plugins/kernel_wfp/Makefile
	src/libcharon/plugins/kernel_iph/Makefile
	src/libcharon/plugins/whitelist/Makefile
	src/libcharon/plugins/ext_auth/Makefile
	src/libcharon/plugins/lookip/Makefile
	src/libcharon/plugins/error_notify/Makefile
	src/libcharon/plugins/certexpire/Makefile
	src/libcharon/plugins/systime_fix/Makefile
	src/libcharon/plugins/led/Makefile
	src/libcharon/plugins/duplicheck/Makefile
	src/libcharon/plugins/coupling/Makefile
	src/libcharon/plugins/radattr/Makefile
	src/libcharon/plugins/osx_attr/Makefile
	src/libcharon/plugins/android_dns/Makefile
	src/libcharon/plugins/android_log/Makefile
	src/libcharon/plugins/maemo/Makefile
	src/libcharon/plugins/stroke/Makefile
	src/libcharon/plugins/vici/Makefile
	src/libcharon/plugins/vici/ruby/Makefile
	src/libcharon/plugins/updown/Makefile
	src/libcharon/plugins/dhcp/Makefile
	src/libcharon/plugins/unit_tester/Makefile
	src/libcharon/plugins/load_tester/Makefile
	src/stroke/Makefile
	src/ipsec/Makefile
	src/starter/Makefile
	src/starter/tests/Makefile
	src/_updown/Makefile
	src/_updown_espmark/Makefile
	src/_copyright/Makefile
	src/scepclient/Makefile
	src/aikgen/Makefile
	src/pki/Makefile
	src/pki/man/Makefile
	src/pool/Makefile
	src/dumm/Makefile
	src/dumm/ext/extconf.rb
	src/libfast/Makefile
	src/manager/Makefile
	src/medsrv/Makefile
	src/checksum/Makefile
	src/conftest/Makefile
	src/pt-tls-client/Makefile
	src/swanctl/Makefile
	scripts/Makefile
	testing/Makefile
])

# =================
#  build man pages
# =================

AC_CONFIG_FILES([
	conf/strongswan.conf.5.head
	conf/strongswan.conf.5.tail
	man/ipsec.conf.5
	man/ipsec.secrets.5
	src/charon-cmd/charon-cmd.8
	src/pki/man/pki.1
	src/pki/man/pki---gen.1
	src/pki/man/pki---issue.1
	src/pki/man/pki---keyid.1
	src/pki/man/pki---pkcs7.1
	src/pki/man/pki---print.1
	src/pki/man/pki---pub.1
	src/pki/man/pki---req.1
	src/pki/man/pki---self.1
	src/pki/man/pki---signcrl.1
	src/pki/man/pki---acert.1
	src/pki/man/pki---verify.1
	src/swanctl/swanctl.8
	src/swanctl/swanctl.conf.5.head
	src/swanctl/swanctl.conf.5.tail
])

AC_OUTPUT

# ========================
#  report enabled plugins
# ========================

AC_MSG_RESULT([])
AC_MSG_RESULT([ strongSwan will be built with the following plugins])
AC_MSG_RESULT([-----------------------------------------------------])

AC_MSG_RESULT([libstrongswan:$s_plugins])
AC_MSG_RESULT([libcharon:    $c_plugins])
AC_MSG_RESULT([libhydra:     $h_plugins])
AC_MSG_RESULT([libtnccs:     $t_plugins])
AC_MSG_RESULT([])
