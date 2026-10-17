# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

"""Fixture for UEFI capsule test."""

import os

from subprocess import call, check_call, CalledProcessError
import pytest
from capsule_defs import CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR, EFITOOLS_PATH

@pytest.fixture(scope='session')
def efi_capsule_data(request, u_boot_config):
    """Set up a file system and return path to image.

    The function sets up a file system to be used in UEFI capsule and
    authentication test and returns a path to disk image to be used
    for testing.

    request -- Pytest request object.
    u_boot_config -- U-Boot configuration.
    """
    mnt_point = u_boot_config.persistent_data_dir + '/test_efi_capsule'
    data_dir = mnt_point + CAPSULE_DATA_DIR
    install_dir = mnt_point + CAPSULE_INSTALL_DIR
    image_path = u_boot_config.persistent_data_dir + '/test_efi_capsule.img'

    try:
        # Create a target device
        check_call('dd if=/dev/zero of=./spi.bin bs=1MiB count=16', shell=True)

        check_call('rm -rf %s' % mnt_point, shell=True)
        check_call('mkdir -p %s' % data_dir, shell=True)
        check_call('mkdir -p %s' % install_dir, shell=True)

        capsule_auth_enabled = u_boot_config.buildconfig.get(
                    'config_efi_capsule_authenticate')
        key_dir = u_boot_config.source_dir + '/board/sandbox'
        if capsule_auth_enabled:
            # Get the keys from the board directory
            check_call('cp %s/capsule_priv_key_good.key %s/SIGNER.key'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_key_good.crt %s/SIGNER.crt'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_esl_good.esl %s/SIGNER.esl'
                       % (key_dir, data_dir), shell=True)

            check_call('cp %s/capsule_priv_key_bad.key %s/SIGNER2.key'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_key_bad.crt %s/SIGNER2.crt'
                       % (key_dir, data_dir), shell=True)

        # Update dtb to add the version information
        check_call('cd %s; '
                   'cp %s/test/py/tests/test_efi_capsule/version.dts .'
                   % (data_dir, u_boot_config.source_dir), shell=True)

        if capsule_auth_enabled:
            check_call('cd %s; '
                       'cp %s/arch/sandbox/dts/test.dtb test_sig.dtb'
                       % (data_dir, u_boot_config.build_dir), shell=True)
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dts; '
                       'fdtoverlay -i test_sig.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir), shell=True)
        else:
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dts; '
                       'fdtoverlay -i %s/arch/sandbox/dts/test.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir, u_boot_config.build_dir), shell=True)

        # two regions: one for u-boot.bin and the other for u-boot.env
        check_call('cd %s; echo -n u-boot:Old > u-boot.bin.old; echo -n u-boot:New > u-boot.bin.new; echo -n u-boot-env:Old > u-boot.env.old; echo -n u-boot-env:New > u-boot.env.new' % data_dir,
                   shell=True)

        pythonpath = os.environ.get('PYTHONPATH', '')
        os.environ['PYTHONPATH'] = pythonpath + ':' + '%s/scripts/dtc/pylibfdt' % u_boot_config.build_dir
        check_call('cd %s; '
                   'cc -E -I %s/include -x assembler-with-cpp -o capsule_gen_tmp.dts %s/test/py/tests/test_efi_capsule/capsule_gen_binman.dts; '
                   'dtc -I dts -O dtb capsule_gen_tmp.dts -o capsule_binman.dtb;'
                   % (data_dir, u_boot_config.source_dir, u_boot_config.source_dir), shell=True)
        check_call('cd %s; '
                   './tools/binman/binman --toolpath %s/tools build -u -d %s/capsule_binman.dtb -O %s -m --allow-missing -I %s -I ./board/sandbox -I ./arch/sandbox/dts'
                   % (u_boot_config.source_dir, u_boot_config.build_dir, data_dir, data_dir, data_dir), shell=True)
        os.environ['PYTHONPATH'] = pythonpath

        # Create a disk image with EFI system partition
        check_call('virt-make-fs --partition=gpt --size=+1M --type=vfat %s %s' %
                   (mnt_point, image_path), shell=True)
        check_call('sgdisk %s -A 1:set:0 -t 1:C12A7328-F81F-11D2-BA4B-00A0C93EC93B' %
                   image_path, shell=True)

    except CalledProcessError as exception:
        pytest.skip('Setup failed: %s' % exception.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -rf %s' % mnt_point, shell=True)
        call('rm -f %s' % image_path, shell=True)
        call('rm -f ./spi.bin', shell=True)
