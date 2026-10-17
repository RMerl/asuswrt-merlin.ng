# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Image Authentication Test (signature with certificates chain)

"""
This test verifies image authentication for a signed image which is signed
by user certificate and contains additional intermediate certificates in its
signature.
"""

import pytest


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiSignedImageIntca(object):
    def test_efi_signed_image_intca1(self, u_boot_console, efi_boot_env_intca):
        """
        Test Case 1 - authenticated by root CA in db
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env_intca
        with u_boot_console.log.section('Test Case 1a'):
            # Test Case 1a, with no Int CA and not authenticated by root CA
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db_c.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO_a host 0:1 /helloworld.efi.signed_a -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO_a\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 1b'):
            # Test Case 1b, signed and authenticated by root CA
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 2 HELLO_ab host 0:1 /helloworld.efi.signed_ab -s ""',
                'efidebug boot order 2',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

    def test_efi_signed_image_intca2(self, u_boot_console, efi_boot_env_intca):
        """
        Test Case 2 - authenticated by root CA in db
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env_intca
        with u_boot_console.log.section('Test Case 2a'):
            # Test Case 2a, unsigned and not authenticated by root CA
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO_abc host 0:1 /helloworld.efi.signed_abc -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO_abc\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 2b'):
            # Test Case 2b, signed and authenticated by root CA
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db_b.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO_abc\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 2c'):
            # Test Case 2c, signed and authenticated by root CA
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db_c.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'Hello, world!' in ''.join(output)

    def test_efi_signed_image_intca3(self, u_boot_console, efi_boot_env_intca):
        """
        Test Case 3 - revoked by dbx
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env_intca
        with u_boot_console.log.section('Test Case 3a'):
            # Test Case 3a, revoked by int CA in dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 dbx_b.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'fatload host 0:1 4000000 db_c.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO_abc host 0:1 /helloworld.efi.signed_abc -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'Hello, world!' in ''.join(output)
            # Or,
            # assert '\'HELLO_abc\' failed' in ''.join(output)
            # assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 3b'):
            # Test Case 3b, revoked by root CA in dbx
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx_c.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO_abc\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)
