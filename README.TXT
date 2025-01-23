The GPL source is created for ASUS wireless router related products. Please visit the ASUS support site (http://support.asus.com) to get the latest GPL tarball.It has a lot in common with many wireless router open source projects, including Oleg/Tomato/DD-WRT/OpenWRT. Thanks the developers of those projects for making the source code available.

Set Up Environment(Tested in Fedora 8/9 and Ubuntu)

    1. prepare environment

	a. Ubuntu
		Install these packages (I used synaptic: "sudo synaptic")

		Ubuntu 14.04 or old distro
			libncurses5
			libncurses5-dev
			m4
			bison
			gawk
			flex
			libstdc++6-4.4-dev
			g++-4.4
			g++
			gengetopt
			git (or git-core)
			gitk
			zlib1g-dev
			autoconf
			autopoint (or gettext)
			libtool
			shtool
			autogen
			mtd-utils
			intltool
			sharutils
			docbook-xsl-*
			libstdc++5
			texinfo
			dos2unix (or tofrodos)
			xsltproc
			uboot-mkimage
			device-tree-compiler
			python 2.7 or above.
			qemu
			gperf
			liblzo2-dev
			uuid-dev
			build-essential
			lzma-dev
			liblzma-dev
			lzma
			binutils-dev
			patch
			cmake
			intltool
			libglib2.0-dev
			gtk-doc-tools

		If you are using Ubuntu amd64, you may need to install 32-bit packages listed below.
			libc6-i386
			lib32stdc++6
			lib32z1
			libelf1:i386
			libncurses5:i386
			ia32-libs
			libc6-dev-i386

		Ubuntu 16.04
			python 2.7 or above.

			libncurses5 libncurses5-dev m4 bison gawk flex libstdc++6-4.7-dev g++-4.7 g++
			gengetopt git gitk zlib1g-dev autoconf autopoint libtool-bin shtool autogen
			mtd-utils intltool sharutils docbook-xsl-* libstdc++5 texinfo dos2unix xsltproc
			u-boot-tools device-tree-compiler qemu gperf liblzo2-dev uuid-dev build-essential
			lzma-dev liblzma-dev lzma binutils-dev patch cmake intltool libglib2.0-dev
			gtk-doc-tools libssl-dev libyaml-dev

		If you are using Ubuntu amd64, you may need to install 32-bit packages listed below.
			libc6-i386 lib32stdc++6 lib32z1 libelf1:i386 lib32ncurses5 libc6-dev-i386

	b. Fedora
		python 2.7 or above.
		sudo yum groupinstall "Development Tools"
		sudo yum install gettext-devel libxml2-devel mtd-utils-ubi ncurses-devel \
			zlib-devel intltool sharutils docbook-dtds docbook-style-xsl \
			uboot-tools dtc gengetopt gperf cmake glib2-devel gtk-doc texinfo \
			lzo-devel libbsd-devel libyaml-devel libtool-ltdl-devel xz-devel \
			openssl1.1-1:1.1.1q libtool-ltdl-devel

		If your distribution don't have openssl1.1-1:1.1.1q package, find package
		name via below commands:
			sudo dnf provides "*/libssl.so.1.1*"

		If make newer that v4.1, it's recommended to downgrade to v4.1, otherwise,
		you might encounter compile error in qca-ssdk.

		If Captive Portal is enable and /PATH/TO/src/router/talloc-2.1.1/src-XXX.answer
		doesn't exist, install below package.
			qemu

		If you are using Fedora 19 or above, install below packages.
		sudo yum groupinstall "C Development Tools and Libraries"

		If you are using Fedora x86_64, install 32-bit packages listed below.
			elfutils-libelf.i686 glibc.i686 libstdc++.i686 zlib.i686

		If you are using Fedora 24+, install 32-bit packages listed below.
			ncurses-compat-libs.i686

		If you are using Fedora 29+, install 32-bit packages listed below.
			glibc-devel.i686

		If you are using Fedora 31+, install packages listed below.
			python2

		If you are using Fedora 33+, install packages listed below.
			perl-FindBin

	c. Rocky Linux/CentOS Stream/RHEL/AlmaLinux/Miracle Linux/Oracle Linux 9
		sudo dnf groupinstall "Development Tools" "RPM Development Tools"

		Enable CRB (CodeReady Builder) or PowerTools repository.
		+ Rocky Linux/CentOS Stream/AlmaLinux
		  sudo dnf config-manager --set-enabled crb
		+ RHEL
		  sudo subscription-manager repos --enable codeready-builder-for-rhel-9-$(arch)-rpms
		+ Miracle Linux
		  sudo dnf config-manager --set-enabled 9-latest-PowerTools
		+ Oracle Linux
		  sudo dnf config-manager --set-enabled ol9_codeready_builder

		Enable EPEL repository.
		+ Rocky Linux/CentOS Stream/RHEL/AlmaLinux/Miracle Linux
		  sudo dnf install --nogpgcheck https://dl.fedoraproject.org/pub/epel/epel-release-latest-$(rpm -E %rhel).noarch.rpm
		+ Oracle Linux
		  sudo dnf install oracle-epel-release-el9

		Download source RPM of python2.7 and mtd-utils from Fedora 39, fulfill build dependency,
		build via rpmbuild and then install below rpms:
			python2.7 mtd-utils-abi

		sudo dnf install gettext-devel libxml2-devel ncurses-devel zlib-devel intltool \
			sharutils docbook-dtds docbook-style-xsl uboot-tools dtc gengetopt gperf \
			cmake glib2-devel gtk-doc texinfo lzo-devel libbsd-devel libyaml-dev \
			perl-FindBin libtool-ltdl-devel xz-devel compat-openssl11-1:1.1.1k \
			libtool-ltdl-devel

		If your distribution don't have openssl1.1-1:1.1.1q package, find package
		name via below commands:
			sudo dnf provides "*/libssl.so.1.1*"

		If make newer that v4.1, it's recommended to downgrade to v4.1, otherwise,
		you might encounter compile error in qca-ssdk.

		If you are using x86_64, install 32-bit packages listed below.
			elfutils-libelf.i686 glibc.i686 libstdc++.i686 zlib.i686 \
			ncurses-libs.i686 ncurses-devel.i686

		If Captive Portal is enabled and /PATH/TO/src/router/talloc-2.1.1/src-VENDOR-CHIP.answer
		for the model doesn't exist. You can find out src-VENDOR-CHIP by model name in
		release/models file.
		i.  Remove official qemu-kvm-common package if it is installed.
		ii. Download source RPM of qemu from Fedora 39, build and install rpms that provide
		    packages listed.
			qemu-user qemu-common
		iii.If you need qemu-kvm, install rpms that provide packages listed below.
			qemu-kvm qemu-kvm-core qemu-system-x86 qemu-system-x86-core qemu-audio-alsa
			qemu-audio-dbus qemu-audio-oss qemu-audio-pa qemu-audio-pipewire qemu-audio-sdl
			qemu-block-curl qemu-block-dmg qemu-block-iscsi qemu-block-rbd qemu-block-ssh
			qemu-char-baum qemu-device-display-virtio-gpu qemu-device-display-virtio-gpu-ccw
			qemu-device-display-virtio-gpu-pci qemu-device-display-virtio-vga
			qemu-device-display-virtio-vga-gl qemu-device-usb-host qemu-device-usb-redirect
			qemu-ui-curses qemu-ui-egl-headless qemu-ui-gtk qemu-ui-opengl qemu-ui-sdl

   2. prepare source to, ex, $HOME/asuswrt

	cd $HOME
	tar xvfz [tar file]

   3. setup development system

	Broadcom SoC models
	===================

		To install the tools:
		    - copy the tools/brcm/ directory to /opt
		    - add /opt/brcm/hndtools-mipsel-linux/bin to your path
		    - add /opt/brcm/hndtools-mipsel-uclibc/bin to your path

	Broadcom HND SoC models
	=======================

		Update your environment variables as following:
		    - LD_LIBRARY_PATH=/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/lib
		    - TOOLCHAIN_BASE=/opt/toolchains
		    - PATH=/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin:/opt/toolchains/crosstools-aarch64-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin:/projects/hnd/tools/linux/hndtools-armeabi-2011.09/bin:$PATH

	Mediatek/Ralink MT76xx SoC models
	=================================

		For MT7622 Chip:
		    - extract tools/lede-toolchain-mediatek-mt7622_gcc-5.4.0_glibc-2.24.Linux-x86_64.tar.bz2 to /opt
		    - add /opt/lede-toolchain-mediatek-mt7622_gcc-5.4.0_glibc-2.24.Linux-x86_64/toolchain-aarch64_cortex-a53+neon-vfpv4_gcc-5.4.0_glibc-2.24/bin/ to your path

		To install the tools:
	    	    - copy the tools/brcm/ directory to /opt
		    - add /opt/brcm/hndtools-mipsel-linux/bin to your path
		    - add /opt/brcm/hndtools-mipsel-uclibc/bin to your path
		    If it is MT7621 or MT7628 chip:
	    	    - extract tools/buildroot-gcc463_32bits.tar.bz2 to /opt
		    - add /opt/buildroot-gcc463/bin to your path
		    otherwise :
	    	    - extract tools/buildroot-gcc342.tar.bz2 to /opt
		    - add /opt/buildroot-gcc342/bin to your path

		For MT7621 Uboot:
	    	    - extract mips-2012.03.tar.bz2 directory to /opt
		    - add /opt/mips-2012.03/bin to your uboot path

	Mediatek/Ralink MT79xx SoC models
	=================================

		To install the tools:
		    - extract tools/openwrt-gcc840_musl-1.1.24.aarch64.tar.bz2 to /opt
		    If your PC environment is 32bit or Ubuntu 16, extract tools/openwrt-gcc840_musl-1.1.24.aarch64-U16-32bit.tar.bz2 instead

	Qualcomm QCA9557/QCA953x/QCA956x MIPS SoC models
	================================================

		To install the tools:
		    Mesh Router:
		    - extract tools/openwrt-gcc463.mips.mesh.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.mips.mesh/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc463.mips.mesh

		    Others: (For example, RT-AC55U, 4G-AC55U.)
	    	    - extract tools/openwrt-gcc463.mips.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.mips/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc463.mips

	Qualcomm IPQ806x/IPQ807x ARM SoC models
	===============================================

		For example, BRT-AC828, RT-AC58U, GT-AXY16000, RT-AX89X.

		To install the tools:
		    - extract tools/openwrt-gcc463.arm.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc463.arm/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:
	
		      export STAGING_DIR=/opt/openwrt-gcc463.arm

	Qualcomm IPQ40xx ARM SoC models
	===============================

		For example, MAP-AC2200

		To install the tools:
		    - extract tools/openwrt-gcc483.arm.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc483.arm/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc483.arm

	Qualcomm IPQ53xx aarch64 SoC models
	===============================================

		For example, TUF-BE6500.

		If you have access to the internal git repository, run "git lfs pull"
		in asuswrt/release/src-qca-ipq53xx directory first, otherwise, extract
		below toolchain archive always failed due to it's a text file for git-lfs.

		To install the tools for firmware:
		    - extract openwrt-gcc750_musl1124.aarch64.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc750_musl1124.aarch64/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc750_musl1124.aarch64

		To install the tools for U-Boot:
		    - extract openwrt-gcc750_musl1124.arm.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc750_musl1124.arm/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc750_musl1124.arm

	Qualcomm IPQ60xx ARM SoC models
	===============================

		For example, PL-AX56XP4

		To install the tools:
		    - extract tools/openwrt-gcc520_musl.arm.tar.bz2 directory to /opt
		    - add /opt/openwrt-gcc520_musl.arm/bin to your path
		    - If you want to build small utilities out of asuswrt box,
		      add STAGING_DIR environment variable as below:

		      export STAGING_DIR=/opt/openwrt-gcc520_musl.arm


	Note: Broadcom/Ralink(except 4708 series) platform use the same toolchain for user space program, so please set PATH to the same directory as above

   4. build firmware.

	a. rt-n16
		cd release/src-rt
		make rt-n16

	b. rt-n56u
		cd release/src-ra
		make rt-n56u

	c. rt-n65u
		cd release/src-ra-3.0
		make rt-n65u

	d. rt-n14u (/ rt-ac52u / rt-ac51u / rt-n11p / rt-n54u)
		cd release/src-ra-mt7620
		make rt-n14u
		( make rt-ac52u  )
		( make rt-ac51u  )
		( make rt-n11p   )
		( make rt-n54u   )

	e. rt-ac56u (/ rt-ac68u / rt-n18uhp)
		cd release/src-rt-6.x.4708
		make rt-ac56u
		( make rt-ac68u  )
		( make rt-n18uhp )

	f. rt-ac55u (/ rt-ac55uhp )
		cd release/src-qca
		make rt-ac55u
		( make rt-ac55uhp )

	g. brt-ac828 (/ rt-ad7200 / gt-axy16000 / rt-ax89x)
		cd release/src-qca-ipq806x
		make brt-ac828
		( make rt-ad7200 )
		( make gt-axy16000 )
		( make rt-ax89u )	//also rt-ax89x by odmpid

	h. rt-ac58u (/ rt-ac82u / map-ac1300 / map-ac2200 / vzw-ac1300 / rt-ac95u )
		cd release/src-qca-dakota
		make rt-ac58u
		( make rt-ac82u )
		( make map-ac1300 )
		( make map-ac2200 )
		( make vzw-ac1300 )
		( make rt-ac95u )

	i. rt-ac85u (/ rt-ac85u / rt-ac65u / rp-ac87 )
		cd release/src-ra-5010
		make rt-ac85u
		( make rt-ac65u )
		( make rp-ac87 )

	j. rt-ac1200 (/ rt-n11p_b1 / rt-n10p_v3 / rt-ac1200gu / rt-ac51u+ / rt-ac53 )
		cd release/src-ra-4300
		make rt-ac1200
		( make rt-n11p_b1 )
		( make rt-n10p_v3 )
		( make rt-ac1200gu )
		( make rt-ac51u+ )
		( make rt-ac53 )

	k. rp-ac68u (/ rp-ac53 / rp-ac55 )
		cd release/src-rtk-819x
		make rp-ac68u
		( make rp-ac53 )
		( make rp-ac55 )

	l. rt-ac1200_v2
		cd release/src-mtk3.5
		make rt-ac1200_v2

	m. tuf-be6500
		cd release/src-qca-ipq53xx
		make tuf-be6500
