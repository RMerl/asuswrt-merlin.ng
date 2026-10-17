# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2022 Massimo Pegorer

"""
Test that mkimage generates auto-FIT with signatures and/or hashes as expected.

The mkimage tool can create auto generated (i.e. without an ITS file
provided as input) FIT in three different flavours: with crc32 checksums
of 'images' subnodes; with signatures of 'images' subnodes; with sha1
hashes of 'images' subnodes and signatures of 'configurations' subnodes.
This test verifies that auto-FIT are generated as expected, in all of
the three flavours, including check of hashes and signatures (except for
configurations ones).

The test does not run the sandbox. It only checks the host tool mkimage.
"""

import os
import pytest
import u_boot_utils as util
import binascii
from Cryptodome.Hash import SHA1
from Cryptodome.Hash import SHA256
from Cryptodome.PublicKey import RSA
from Cryptodome.Signature import pkcs1_15

class SignedFitHelper(object):
    """Helper to manipulate a FIT with signed/hashed images/configs."""
    def __init__(self, cons, file_name):
        self.fit = file_name
        self.cons = cons
        self.images_nodes = set()
        self.confgs_nodes = set()

    def __fdt_list(self, path):
        return util.run_and_log(self.cons,
            f'fdtget -l {self.fit} {path}')

    def __fdt_get_string(self, node, prop):
        return util.run_and_log(self.cons,
            f'fdtget -ts {self.fit} {node} {prop}')

    def __fdt_get_binary(self, node, prop):
        numbers = util.run_and_log(self.cons,
            f'fdtget -tbi {self.fit} {node} {prop}')

        bignum = bytearray()
        for little_num in numbers.split():
            bignum.append(int(little_num))

        return bignum

    def build_nodes_sets(self):
        """Fill sets with FIT images and configurations subnodes."""
        for node in self.__fdt_list('/images').split():
            subnode = f'/images/{node}'
            self.images_nodes.add(subnode)

        for node in self.__fdt_list('/configurations').split():
            subnode = f'/configurations/{node}'
            self.confgs_nodes.add(subnode)

        return len(self.images_nodes) + len(self.confgs_nodes)

    def check_fit_crc32_images(self):
        """Test that all images in the set are hashed as expected.

        Each image must have an hash with algo=crc32 and hash value must match
        the one calculated over image data.
        """
        for node in self.images_nodes:
            algo = self.__fdt_get_string(f'{node}/hash', 'algo')
            assert algo == "crc32\n", "Missing expected crc32 image hash!"

            raw_crc32 = self.__fdt_get_binary(f'{node}/hash', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            assert raw_crc32 == (binascii.crc32(raw_bin) &
                0xffffffff).to_bytes(4, 'big'), "Wrong crc32 hash!"

    def check_fit_signed_images(self, key_name, sign_algo, verifier):
        """Test that all images in the set are signed as expected.

        Each image must have a signature with: key-name-hint matching key_name
        argument; algo matching sign_algo argument; value matching the one
        calculated over image data using verifier argument.
        """
        for node in self.images_nodes:
            hint = self.__fdt_get_string(f'{node}/signature', 'key-name-hint')
            assert hint == key_name + "\n", "Missing expected key name hint!"
            algo = self.__fdt_get_string(f'{node}/signature', 'algo')
            assert algo == sign_algo + "\n", "Missing expected signature algo!"

            raw_sig = self.__fdt_get_binary(f'{node}/signature', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            verifier.verify(SHA256.new(raw_bin), bytes(raw_sig))

    def check_fit_signed_confgs(self, key_name, sign_algo):
        """Test that all configs are signed, and images hashed, as expected.

        Each image must have an hash with algo=sha1 and hash value must match
        the one calculated over image data. Each configuration must have a
        signature with key-name-hint matching key_name argument and algo
        matching sign_algo argument.
        TODO: configurations signature checking.
        """
        for node in self.images_nodes:
            algo = self.__fdt_get_string(f'{node}/hash', 'algo')
            assert algo == "sha1\n", "Missing expected sha1 image hash!"

            raw_hash = self.__fdt_get_binary(f'{node}/hash', 'value')
            raw_bin = self.__fdt_get_binary(node, 'data')
            assert raw_hash == SHA1.new(raw_bin).digest(), "Wrong sha1 hash!"

        for node in self.confgs_nodes:
            hint = self.__fdt_get_string(f'{node}/signature', 'key-name-hint')
            assert hint == key_name + "\n", "Missing expected key name hint!"
            algo = self.__fdt_get_string(f'{node}/signature', 'algo')
            assert algo == sign_algo + "\n", "Missing expected signature algo!"


@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('fdtget')
def test_fit_auto_signed(u_boot_console):
    """Test that mkimage generates auto-FIT with signatures/hashes as expected.

    The mkimage tool can create auto generated (i.e. without an ITS file
    provided as input) FIT in three different flavours: with crc32 checksums
    of 'images' subnodes; with signatures of 'images' subnodes; with sha1
    hashes of 'images' subnodes and signatures of 'configurations' subnodes.
    This test verifies that auto-FIT are generated as expected, in all of
    the three flavours, including check of hashes and signatures (except for
    configurations ones).

    The test does not run the sandbox. It only checks the host tool mkimage.
    """
    cons = u_boot_console
    mkimage = cons.config.build_dir + '/tools/mkimage'
    tempdir = os.path.join(cons.config.result_dir, 'auto_fit')
    os.makedirs(tempdir, exist_ok=True)
    kernel_file = f'{tempdir}/vmlinuz'
    dt1_file = f'{tempdir}/dt-1.dtb'
    dt2_file = f'{tempdir}/dt-2.dtb'
    key_name = 'sign-key'
    sign_algo = 'sha256,rsa4096'
    key_file = f'{tempdir}/{key_name}.key'
    fit_file = f'{tempdir}/test.fit'

    # Create a fake kernel image and two dtb files with random data
    with open(kernel_file, 'wb') as fd:
        fd.write(os.urandom(512))

    with open(dt1_file, 'wb') as fd:
        fd.write(os.urandom(256))

    with open(dt2_file, 'wb') as fd:
        fd.write(os.urandom(256))

    # Create 4096 RSA key and write to file to be read by mkimage
    key = RSA.generate(bits=4096)
    verifier = pkcs1_15.new(key)

    with open(key_file, 'w') as fd:
        fd.write(str(key.export_key(format='PEM').decode('ascii')))

    b_args = " -d" + kernel_file + " -b" + dt1_file + " -b" + dt2_file
    s_args = " -k" + tempdir + " -g" + key_name + " -o" + sign_algo

    # 1 - Create auto FIT with images crc32 checksum, and verify it
    util.run_and_log(cons, mkimage + ' -fauto' + b_args + " " + fit_file)

    fit = SignedFitHelper(cons, fit_file)
    if fit.build_nodes_sets() == 0:
        raise ValueError('FIT-1 has no "/image" nor "/configuration" nodes')

    fit.check_fit_crc32_images()

    # 2 - Create auto FIT with signed images, and verify it
    util.run_and_log(cons, mkimage + ' -fauto' + b_args + s_args + " " +
        fit_file)

    fit = SignedFitHelper(cons, fit_file)
    if fit.build_nodes_sets() == 0:
        raise ValueError('FIT-2 has no "/image" nor "/configuration" nodes')

    fit.check_fit_signed_images(key_name, sign_algo, verifier)

    # 3 - Create auto FIT with signed configs and hashed images, and verify it
    util.run_and_log(cons, mkimage + ' -fauto-conf' + b_args + s_args + " " +
        fit_file)

    fit = SignedFitHelper(cons, fit_file)
    if fit.build_nodes_sets() == 0:
        raise ValueError('FIT-3 has no "/image" nor "/configuration" nodes')

    fit.check_fit_signed_confgs(key_name, sign_algo)
