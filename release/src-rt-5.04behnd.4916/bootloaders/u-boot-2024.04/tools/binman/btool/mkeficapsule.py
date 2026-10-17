# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Linaro Limited
#
"""Bintool implementation for mkeficapsule tool

mkeficapsule is a tool used for generating EFI capsules.

The following are the commandline options to be provided
to the tool
Usage: mkeficapsule [options] <image blob> <output file>
Options:
	-g, --guid <guid string>    guid for image blob type
	-i, --index <index>         update image index
	-I, --instance <instance>   update hardware instance
	-v, --fw-version <version>  firmware version
	-p, --private-key <privkey file>  private key file
	-c, --certificate <cert file>     signer's certificate file
	-m, --monotonic-count <count>     monotonic count
	-d, --dump_sig              dump signature (*.p7)
	-A, --fw-accept  firmware accept capsule, requires GUID, no image blob
	-R, --fw-revert  firmware revert capsule, takes no GUID, no image blob
	-o, --capoemflag Capsule OEM Flag, an integer between 0x0000 and 0xffff
	-h, --help                  print a help message
"""

from binman import bintool

class Bintoolmkeficapsule(bintool.Bintool):
    """Handles the 'mkeficapsule' tool

    This bintool is used for generating the EFI capsules. The
    capsule generation parameters can either be specified through
    commandline, or through a config file.
    """
    def __init__(self, name):
        super().__init__(name, 'mkeficapsule tool for generating capsules')

    def generate_capsule(self, image_index, image_guid, hardware_instance,
                         payload, output_fname, priv_key, pub_key,
                         monotonic_count=0, version=0, oemflags=0):
        """Generate a capsule through commandline-provided parameters

        Args:
            image_index (int): Unique number for identifying payload image
            image_guid (str): GUID used for identifying the image
            hardware_instance (int): Optional unique hardware instance of
            a device in the system. 0 if not being used
            payload (str): Path to the input payload image
            output_fname (str): Path to the output capsule file
            priv_key (str): Path to the private key
            pub_key(str): Path to the public key
            monotonic_count (int): Count used when signing an image
            version (int): Image version (Optional)
            oemflags (int): Optional 16 bit OEM flags

        Returns:
            str: Tool output
        """
        args = [
            f'--index={image_index}',
            f'--guid={image_guid}',
            f'--instance={hardware_instance}'
        ]

        if version:
            args += [f'--fw-version={version}']
        if oemflags:
            args += [f'--capoemflag={oemflags}']
        if priv_key and pub_key:
            args += [
                f'--monotonic-count={monotonic_count}',
                f'--private-key={priv_key}',
                f'--certificate={pub_key}'
            ]

        args += [
            payload,
            output_fname
        ]

        return self.run_cmd(*args)

    def generate_empty_capsule(self, image_guid, output_fname,
                               accept=True):
        """Generate empty capsules for FWU A/B updates

        Args:
            image_guid (str): GUID used for identifying the image
                in case of an accept capsule
            output_fname (str): Path to the output capsule file
            accept (bool): Generate an accept capsule,
                else a revert capsule

        Returns:
            str: Tool output
        """
        if accept:
            args = [
                f'--guid={image_guid}',
                '--fw-accept'
            ]
        else:
            args = [ '--fw-revert' ]

        args += [ output_fname ]

        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for mkeficapsule

        This builds the tool from source

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        if method != bintool.FETCH_BUILD:
            return None

        cmd = ['tools-only_defconfig', 'tools']
        result = self.build_from_git(
            'https://source.denx.de/u-boot/u-boot.git',
            cmd,
            'tools/mkeficapsule')
        return result
