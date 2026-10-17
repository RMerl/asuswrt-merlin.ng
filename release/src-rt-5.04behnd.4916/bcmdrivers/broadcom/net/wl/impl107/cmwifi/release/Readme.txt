##############################################################
#  The Wifi driver and application source package Readme.
##############################################################
Content:
   This source package contains the following components:
      1. openbfc*_diff.txt(and/or tgz): patches and files to enable compilation of wifi source of different variants.
      2. Readme.txt
      3. images.tgz: Contains the prebuilt images of the target platform with wifi that can be used to upgrade the respective platform. Upon untar of images.tgz the following folders can be found.
              images/93390smwvg2/...
              images/dongle/...

This package will support
   A. OpenBFC
   B. OpenBFC-RDKM
   C. OpenBFC-OpenWRT

A. OpenBFC
OpenBFC Build:
   You will need the following packages.
        - OpenBFC_[bfcVer]_lattice.tar.gz
        - OpenBFC_[bfcVer]_common.tar.gz
        - wifi_src-[wifiVer]-[svnVer].tgz
        - wifi_airiq-[svnVer]-3390_OpenBFC-[bfcVer].tgz
   Steps:
      0: Create and go to your working directory. Copy the following files to it.
         OpenBFC_[bfcVer]_lattice.tar.gz
         OpenBFC_[bfcVer]_common.tar.gz
         wifi_src-[wifiVer]-[svnVer].tgz
         wifi_airiq-[svnVer]-3390_OpenBFC-[bfcVer].tgz
         openbfc_diff.txt
      1. Untar "OpenBFC_[bfcVer]_lattice.tar.gz" and
OpenBFC_[bfcVer]_common.tar.gz, rbb_lattice/ and rbb_common directories will
be created.
         a. tar xfz OpenBFC_[bfcVer]_lattice.tar.gz
         b. tar xfz OpenBFC_[bfcVer]_common.tar.gz
         c. please make a soft symbolic link to rg
	        ln -s rbb_lattice rg
      2. Untar the wifi source to a directory by name wl/ by creating one.
         a. mkdir wl
         b. tar xfz wifi_src-[wifiVer]-[svnVer].tgz -C wl
         c. tar xfz wifi_airiq-[svnVer]-3390_OpenBFC-[bfcVer].tgz -C wl
      3. Patch openbfc_diff.txt if exists
         patch -p0 < openbfc_diff.txt
      4. Set the toolchain PATH
      5. To Generate target images compile in rg/ as below:
         a. RG/Lattice Compilation:
            cd rg/
            make defaults-3390b0-lattice-wifi wifi_force all
      6. Rebuild WiFi drivers or apps if needed
         a.  WLAN Driver Compilation:
            make defaults-3390b0-lattice-wifi wifi_force wifi_mods WL_DIR=../wl/cmwifi
         b. WLAN Apps Compilation:
            make defaults-3390b0-lattice-wifi wifi_force wifi_apps WL_DIR=../wl/cmwifi
         c. Regenerate *RG.img with WiFi binaries:
            make defaults-3390b0-lattice-wifi userspace_img

      7. The Target images vmlinuz-initrd-3390b0-lattice-wifi, vmlinuz-3390b0-lattice-wifi, ubifs-128k-2048-3390b0-lattice-wifi-RG.img, ubi-squashfs-128k-2048-3390b0-lattice-wifi-4.9-RG.img and rg.3390b0-lattice-wifi.dtb.tgz can be found under images/ folder.
      8. Image upgrade is the same as the lattice variant. Please refer to
OpenBFC_[bfcVer]_RDKM_User_Guide.pdf to upgrade the target.
      9. Dongle Firmware Rebuild: The Dongle firmware is not forced to rebuild
with any of the above steps. To force a Dongle firmware build, please remove
the following files and repeat step 5 or 6 as needed
         rm wl/sys/src/shared/rtecdc_router.h wl/sys/src/shared/rtecdc_43*.h
wl/43684/build/dongle/*/config_pcie* wl/sys/src/shared/rtecdc_67*.h
wl/sys/src/dongle/bin wl/main/build/dongle/*/config_pcie*

      9. For complete rebuild of wifi do clean as below and repeat step 5 or 6
as needed
         a. Wifi Application binaries cleanup
           make defaults-3390b0-lattice-wifi wifi_force wifi_apps_clean
WL_DIR=../wl/cmwifi
         b. Wifi driver modules cleanup
           make defaults-3390b0-lattice-wifi wifi_force wifi_mods_clean
WL_DIR=../wl/cmwifi

NOTE:
-----
1) In this release the WLTEST is disabled by default. To enable the WLTEST,
   Perform step 10 from above and repeat step 7 as below
         a. WLAN Driver Compilation:
            make defaults-3390b0-lattice-wifi wifi_force wifi_mods WLTEST=1 WL_DIR=../wl/cmwifi
         b. WLAN Apps Compilation:
            make defaults-3390b0-lattice-wifi wifi_force wifi_apps WLTEST=1 WL_DIR=../wl/cmwifi
         c. Regenerate *RG.img with WiFi binaries:
            make defaults-3390b0-lattice-wifi userspace_img

2) WiFi dongle trap debug info is included in images/dongle/...

B. OpenBFC-RDKM
   You will need the matching version of
   1. "OpenBFC_[bfcVER]_RDKM.tgz" package
   2. The openbfc_rdkm_diff.txt
   3. The openbfc_rdkm_wifi.tgz
   4. The rg_apps_shared.tgz
   5. Wifi source tarball: wifi_src.tgz as example

   Steps: Please create a new working [DIR], cd to it and copy above 1 ~ 5.
   1. Unstar "OpenBFC_[bfcVER]_RDKM.tgz"
         tar xfz OpenBFC_[bfcVer]_RDKM.tgz
   2. Patch openbfc_rdkm_diff.txt by
         patch -p0 < openbfc_rdkm_diff.txt
   3. Untar openbfc_rdkm_wifi.tgz
         tar xfz openbfc_rdkm_wifi.tgz
   4. Install rg_apps_shared.tgz by moving or copying it to download/ directory
         mv rg_apps_shared.tgz download/.
   5. Install wifi_src.tgz by moving or copying it to download/ directory
         mv wifi_src.tgz download/.
   6. Follow the OpenBFC_[bfcVER]_RDKM_User_Guide.pdf to
      Setup environment by
         source meta-rdk-broadcom-generic-rdk/setup-environment-broadcom-generic-rdkb
      and select the number associates with brcm9XXXXwifi.conf where XXXX is the chip ID.
      Compile by
         bitbake rdk-generic-broadband-image
   7. Follow the OpenBFC_[bfcVER]_RDKM_User_Guide.pdf to upgrade to target unit.

C. OpenBFC-OpenWRT
