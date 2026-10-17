# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Signed Image Authentication Test

"""
This test verifies image authentication for signed images.
"""

import pytest


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiSignedImage(object):
    def test_efi_signed_image_auth1(self, u_boot_console, efi_boot_env):
        """
        Test Case 1 - Secure boot is not in force
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 1a'):
            # Test Case 1a, run signed image if no PK
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 HELLO1 host 0:1 /helloworld.efi.signed -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

        with u_boot_console.log.section('Test Case 1b'):
            # Test Case 1b, run unsigned image if no PK
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 2 HELLO2 host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 2',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

    def test_efi_signed_image_auth2(self, u_boot_console, efi_boot_env):
        """
        Test Case 2 - Secure boot is in force,
                      authenticated by db (TEST_db certificate in db)
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 2a'):
            # Test Case 2a, db is not yet installed
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO1 host 0:1 /helloworld.efi.signed -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert('\'HELLO1\' failed' in ''.join(output))
            assert('efi_start_image() returned: 26' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 2 HELLO2 host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 2',
                'efidebug test bootmgr'])
            assert '\'HELLO2\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 2b'):
            # Test Case 2b, authenticated by db
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 2',
                'efidebug test bootmgr'])
            assert '\'HELLO2\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

    def test_efi_signed_image_auth3(self, u_boot_console, efi_boot_env):
        """
        Test Case 3 - rejected by dbx (TEST_db certificate in dbx)
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 3a'):
            # Test Case 3a, rejected by dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 3b'):
            # Test Case 3b, rejected by dbx even if db allows
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

    def test_efi_signed_image_auth4(self, u_boot_console, efi_boot_env):
        """
        Test Case 4 - revoked by dbx (digest of TEST_db certificate in dbx)
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 4'):
            # Test Case 4, rejected by dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 dbx_hash.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

    def test_efi_signed_image_auth5(self, u_boot_console, efi_boot_env):
        """
        Test Case 5 - multiple signatures
                        one signed with TEST_db, and
                        one signed with TEST_db1
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 5a'):
            # Test Case 5a, authenticated even if only one of signatures
            # is verified
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed_2sigs -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'Hello, world!' in ''.join(output)

        with u_boot_console.log.section('Test Case 5b'):
            # Test Case 5b, authenticated if both signatures are verified
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'Hello, world!' in ''.join(output)

        with u_boot_console.log.section('Test Case 5c'):
            # Test Case 5c, rejected if one of signatures (digest of
            # certificate) is revoked
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx_hash.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 5d'):
            # Test Case 5d, rejected if both of signatures are revoked
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx_hash1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        # Try rejection in reverse order.
        u_boot_console.restart_uboot()
        with u_boot_console.log.section('Test Case 5e'):
            # Test Case 5e, authenticated even if only one of signatures
            # is verified. Same as before but reject dbx_hash1.auth only
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db',
                'fatload host 0:1 4000000 dbx_hash1.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed_2sigs -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

    def test_efi_signed_image_auth6(self, u_boot_console, efi_boot_env):
        """
        Test Case 6 - using digest of signed image in database
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 6a'):
            # Test Case 6a, verified by image's digest in db
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db_hello_signed.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

        with u_boot_console.log.section('Test Case 6b'):
            # Test Case 6b, rejected by TEST_db certificate in dbx
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx_db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        with u_boot_console.log.section('Test Case 6c'):
            # Test Case 6c, rejected by image's digest in dbx
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 dbx_hello_signed.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

    def test_efi_signed_image_auth7(self, u_boot_console, efi_boot_env):
        """
        Test Case 7 - Reject images based on the sha384/512 of their x509 cert
        """
        # sha384 of an x509 cert in dbx
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 7a'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db',
                'fatload host 0:1 4000000 dbx_hash384.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed_2sigs -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

        # sha512 of an x509 cert in dbx
        u_boot_console.restart_uboot()
        with u_boot_console.log.section('Test Case 7b'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db',
                'fatload host 0:1 4000000 dbx_hash512.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi.signed_2sigs -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            assert 'efi_start_image() returned: 26' in ''.join(output)

    def test_efi_signed_image_auth8(self, u_boot_console, efi_boot_env):
        """
        Test Case 8 - Secure boot is in force,
                      Same as Test Case 2 but the image binary to be loaded
                      was willfully modified (forged)
                      Must be rejected.
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 8a'):
            # Test Case 8a, Secure boot is not yet forced
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 HELLO1 host 0:1 /helloworld_forged.efi.signed -s ""',
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert('hELLO, world!' in ''.join(output))

        with u_boot_console.log.section('Test Case 8b'):
            # Test Case 8b, Install signature database and verify the image
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            output = u_boot_console.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert(not 'hELLO, world!' in ''.join(output))
            assert('\'HELLO1\' failed' in ''.join(output))
            assert('efi_start_image() returned: 26' in ''.join(output))
