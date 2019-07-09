#!/bin/sh
# Build script for Travis CI

build_botan()
{
	# same revision used in the build recipe of the testing environment
	BOTAN_REV=2.8.0
	BOTAN_DIR=$TRAVIS_BUILD_DIR/../botan

	if test -d "$BOTAN_DIR"; then
		return
	fi

	echo "$ build_botan()"

	# if the leak detective is enabled we have to disable threading support
	# (used for std::async) as that causes invalid frees somehow, the
	# locking allocator causes a static leak via the first function that
	# references it (e.g. crypter or hasher), so we disable that too
	if test "$LEAK_DETECTIVE" = "yes"; then
		BOTAN_CONFIG="--without-os-features=threads
					  --disable-modules=locking_allocator"
	fi
	# disable some larger modules we don't need for the tests
	BOTAN_CONFIG="$BOTAN_CONFIG --disable-modules=pkcs11,tls,x509,xmss"

	git clone https://github.com/randombit/botan.git $BOTAN_DIR &&
	cd $BOTAN_DIR &&
	git checkout -qf $BOTAN_REV &&
	python ./configure.py --amalgamation $BOTAN_CONFIG &&
	make -j4 libs >/dev/null &&
	sudo make install >/dev/null &&
	sudo ldconfig || exit $?
	cd -
}

build_tss2()
{
	TSS2_REV=2.1.0
	TSS2_PKG=tpm2-tss-$TSS2_REV
	TSS2_DIR=$TRAVIS_BUILD_DIR/../$TSS2_PKG
	TSS2_SRC=https://github.com/tpm2-software/tpm2-tss/releases/download/$TSS2_REV/$TSS2_PKG.tar.gz

	if test -d "$TSS2_DIR"; then
		return
	fi

	echo "$ build_tss2()"

	# the default version of libgcrypt in Ubuntu 16.04 is too old
	sudo apt-get update -qq && \
	sudo apt-get install -qq libgcrypt20-dev &&
	curl -L $TSS2_SRC | tar xz -C $TRAVIS_BUILD_DIR/.. &&
	cd $TSS2_DIR &&
	./configure &&
	make -j4 >/dev/null &&
	sudo make install >/dev/null &&
	sudo ldconfig || exit $?
	cd -
}

build_openssl()
{
	SSL_REV=1.1.1a
	SSL_PKG=openssl-$SSL_REV
	SSL_DIR=$TRAVIS_BUILD_DIR/../$SSL_PKG
	SSL_SRC=https://www.openssl.org/source/$SSL_PKG.tar.gz
	SSL_INS=/usr/local/ssl
	SSL_OPT="shared no-tls no-dtls no-ssl3 no-zlib no-comp no-idea no-psk no-srp
			 no-stdio no-tests enable-rfc3779 enable-ec_nistp_64_gcc_128"

	if test -d "$SSL_DIR"; then
		return
	fi

	echo "$ build_openssl()"

	curl -L $SSL_SRC | tar xz -C $TRAVIS_BUILD_DIR/.. &&
	cd $SSL_DIR &&
	./config --prefix=$SSL_INS --openssldir=$SSL_INS $SSL_OPT &&
	make -j4 >/dev/null &&
	sudo make install_sw >/dev/null &&
	echo $SSL_INS/lib | sudo tee /etc/ld.so.conf.d/openssl-$SSL_REV.conf >/dev/null &&
	sudo ldconfig || exit $?
	cd -
}

use_custom_openssl()
{
	CFLAGS="$CFLAGS -I/usr/local/ssl/include"
	LDFLAGS="$LDFLAGS -L/usr/local/ssl/lib"
	export LDFLAGS
	if test "$1" = "deps"; then
		build_openssl
	fi
}

if test -z $TRAVIS_BUILD_DIR; then
	TRAVIS_BUILD_DIR=$PWD
fi

cd $TRAVIS_BUILD_DIR

TARGET=check

DEPS="libgmp-dev"

CFLAGS="-g -O2 -Wall -Wno-format -Wno-format-security -Wno-pointer-sign -Werror"

case "$TEST" in
default)
	# should be the default, but lets make sure
	CONFIG="--with-printf-hooks=glibc"
	;;
openssl*)
	CONFIG="--disable-defaults --enable-pki --enable-openssl --enable-pem"
	DEPS="libssl-dev"
	if test "$TEST" != "openssl-1.0"; then
		DEPS=""
		use_custom_openssl $1
	fi
	;;
gcrypt)
	CONFIG="--disable-defaults --enable-pki --enable-gcrypt --enable-pkcs1"
	DEPS="libgcrypt11-dev"
	;;
botan)
	CONFIG="--disable-defaults --enable-pki --enable-botan --enable-pem"
	# we can't use the old package that comes with Ubuntu so we build from
	# the current master until 2.8.0 is released and then probably switch to
	# that unless we need newer features (at least 2.7.0 plus PKCS#1 patch is
	# currently required)
	DEPS=""
	if test "$1" = "deps"; then
		build_botan
	fi
	;;
printf-builtin)
	CONFIG="--with-printf-hooks=builtin"
	;;
all|coverage|sonarcloud)
	CONFIG="--enable-all --disable-android-dns --disable-android-log
			--disable-kernel-pfroute --disable-keychain
			--disable-lock-profiler --disable-padlock --disable-fuzzing
			--disable-osx-attr --disable-tkm --disable-uci
			--disable-soup --disable-unwind-backtraces
			--disable-svc --disable-dbghelp-backtraces --disable-socket-win
			--disable-kernel-wfp --disable-kernel-iph --disable-winhttp"
	# not enabled on the build server
	CONFIG="$CONFIG --disable-af-alg"
	if test "$TEST" != "coverage"; then
		CONFIG="$CONFIG --disable-coverage"
	else
		# not actually required but configure checks for it
		DEPS="$DEPS lcov"
	fi
	DEPS="$DEPS libcurl4-gnutls-dev libsoup2.4-dev libunbound-dev libldns-dev
		  libmysqlclient-dev libsqlite3-dev clearsilver-dev libfcgi-dev
		  libpcsclite-dev libpam0g-dev binutils-dev libunwind8-dev libnm-dev
		  libjson0-dev iptables-dev python-pip libtspi-dev libsystemd-dev"
	PYDEPS="pytest"
	if test "$1" = "deps"; then
		build_botan
		build_tss2
	fi
	use_custom_openssl $1
	;;
win*)
	CONFIG="--disable-defaults --enable-svc --enable-ikev2
			--enable-ikev1 --enable-static --enable-test-vectors --enable-nonce
			--enable-constraints --enable-revocation --enable-pem --enable-pkcs1
			--enable-pkcs8 --enable-x509 --enable-pubkey --enable-acert
			--enable-eap-tnc --enable-eap-ttls --enable-eap-identity
			--enable-updown --enable-ext-auth --enable-libipsec
			--enable-tnccs-20 --enable-imc-attestation --enable-imv-attestation
			--enable-imc-os --enable-imv-os --enable-tnc-imv --enable-tnc-imc
			--enable-pki --enable-swanctl --enable-socket-win
			--enable-kernel-iph --enable-kernel-wfp --enable-winhttp"
	# no make check for Windows binaries unless we run on a windows host
	if test "$APPVEYOR" != "True"; then
		TARGET=
		CCACHE=ccache
	else
		CONFIG="$CONFIG --enable-openssl"
		CFLAGS="$CFLAGS -I/c/OpenSSL-$TEST/include"
		LDFLAGS="-L/c/OpenSSL-$TEST"
		export LDFLAGS
	fi
	CFLAGS="$CFLAGS -mno-ms-bitfields"
	DEPS="gcc-mingw-w64-base"
	case "$TEST" in
	win64)
		CONFIG="--host=x86_64-w64-mingw32 $CONFIG --enable-dbghelp-backtraces"
		DEPS="gcc-mingw-w64-x86-64 binutils-mingw-w64-x86-64 mingw-w64-x86-64-dev $DEPS"
		CC="$CCACHE x86_64-w64-mingw32-gcc"
		;;
	win32)
		CONFIG="--host=i686-w64-mingw32 $CONFIG"
		DEPS="gcc-mingw-w64-i686 binutils-mingw-w64-i686 mingw-w64-i686-dev $DEPS"
		CC="$CCACHE i686-w64-mingw32-gcc"
		;;
	esac
	;;
osx)
	# this causes a false positive in ip-packet.c since Xcode 8.3
	CFLAGS="$CFLAGS -Wno-address-of-packed-member"
	# use the same options as in the Homebrew Formula
	CONFIG="--disable-defaults --enable-charon --enable-cmd --enable-constraints
			--enable-curl --enable-eap-gtc --enable-eap-identity
			--enable-eap-md5 --enable-eap-mschapv2 --enable-ikev1 --enable-ikev2
			--enable-kernel-libipsec --enable-kernel-pfkey
			--enable-kernel-pfroute --enable-nonce --enable-openssl
			--enable-osx-attr --enable-pem --enable-pgp --enable-pkcs1
			--enable-pkcs8 --enable-pki --enable-pubkey --enable-revocation
			--enable-scepclient --enable-socket-default --enable-sshkey
			--enable-stroke --enable-swanctl --enable-unity --enable-updown
			--enable-x509 --enable-xauth-generic"
	DEPS="bison gettext openssl curl"
	BREW_PREFIX=$(brew --prefix)
	export PATH=$BREW_PREFIX/opt/bison/bin:$PATH
	export ACLOCAL_PATH=$BREW_PREFIX/opt/gettext/share/aclocal:$ACLOCAL_PATH
	for pkg in openssl curl
	do
		PKG_CONFIG_PATH=$BREW_PREFIX/opt/$pkg/lib/pkgconfig:$PKG_CONFIG_PATH
		CPPFLAGS="-I$BREW_PREFIX/opt/$pkg/include $CPPFLAGS"
		LDFLAGS="-L$BREW_PREFIX/opt/$pkg/lib $LDFLAGS"
	done
	export PKG_CONFIG_PATH
	export CPPFLAGS
	export LDFLAGS
	;;
fuzzing)
	CFLAGS="$CFLAGS -DNO_CHECK_MEMWIPE"
	CONFIG="--enable-fuzzing --enable-static --disable-shared --disable-scripts
			--enable-imc-test --enable-tnccs-20"
	# don't run any of the unit tests
	export TESTS_RUNNERS=
	# prepare corpora
	if test -z "$1"; then
		if test -z "$FUZZING_CORPORA"; then
			git clone --depth 1 https://github.com/strongswan/fuzzing-corpora.git fuzzing-corpora
			export FUZZING_CORPORA=$TRAVIS_BUILD_DIR/fuzzing-corpora
		fi
		# these are about the same as those on OSS-Fuzz (except for the
		# symbolize options and strip_path_prefix)
		export ASAN_OPTIONS=redzone=16:handle_sigill=1:strict_string_check=1:\
			allocator_release_to_os_interval_ms=500:strict_memcmp=1:detect_container_overflow=1:\
			coverage=0:allocator_may_return_null=1:use_sigaltstack=1:detect_stack_use_after_return=1:\
			alloc_dealloc_mismatch=0:detect_leaks=1:print_scariness=1:max_uar_stack_size_log=16:\
			handle_abort=1:check_malloc_usable_size=0:quarantine_size_mb=10:detect_odr_violation=0:\
			symbolize=1:handle_segv=1:fast_unwind_on_fatal=0:external_symbolizer_path=/usr/bin/llvm-symbolizer-3.5
	fi
	;;
dist)
	TARGET=distcheck
	;;
apidoc)
	DEPS="doxygen"
	CONFIG="--disable-defaults"
	TARGET=apidoc
	;;
*)
	echo "$0: unknown test $TEST" >&2
	exit 1
	;;
esac

if test "$1" = "deps"; then
	case "$TRAVIS_OS_NAME" in
	linux)
		sudo apt-get update -qq && \
		sudo apt-get install -qq bison flex gperf gettext $DEPS
		;;
	osx)
		brew update && \
		# workaround for issue #6352
		brew uninstall --force libtool && brew install libtool && \
		brew install $DEPS
		;;
	esac
	exit $?
fi

if test "$1" = "pydeps"; then
	test -z "$PYDEPS" || pip -q install --user $PYDEPS
	exit $?
fi

CONFIG="$CONFIG
	--disable-dependency-tracking
	--enable-silent-rules
	--enable-test-vectors
	--enable-monolithic=${MONOLITHIC-no}
	--enable-leak-detective=${LEAK_DETECTIVE-no}"

echo "$ ./autogen.sh"
./autogen.sh || exit $?
echo "$ CC=$CC CFLAGS=\"$CFLAGS\" ./configure $CONFIG"
CC="$CC" CFLAGS="$CFLAGS" ./configure $CONFIG || exit $?

case "$TEST" in
apidoc)
	exec 2>make.warnings
	;;
*)
	;;
esac

echo "$ make $TARGET"
case "$TEST" in
sonarcloud)
	# without target, coverage is currently not supported anyway because
	# sonarqube only supports gcov, not lcov
	build-wrapper-linux-x86-64 --out-dir bw-output make -j4 || exit $?
	;;
*)
	make -j4 $TARGET || exit $?
	;;
esac

case "$TEST" in
apidoc)
	if test -s make.warnings; then
		cat make.warnings
		exit 1
	fi
	;;
sonarcloud)
	sonar-scanner \
		-Dsonar.projectKey=strongswan \
		-Dsonar.projectVersion=$(git describe)+${TRAVIS_BUILD_NUMBER} \
		-Dsonar.sources=. \
		-Dsonar.cfamily.threads=2 \
		-Dsonar.cfamily.build-wrapper-output=bw-output || exit $?
	;;
*)
	;;
esac
