# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Variable Authentication Test

"""
This test verifies variable authentication
"""

import pytest


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiAuthVar(object):
    def test_efi_var_auth1(self, u_boot_console, efi_boot_env):
        """
        Test Case 1 - Install signature database
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 1a'):
            # Test Case 1a, Initial secure state
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'printenv -e SecureBoot'])
            assert '00000000: 00' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SetupMode')
            assert '00000000: 01' in output

        with u_boot_console.log.section('Test Case 1b'):
            # Test Case 1b, PK without AUTHENTICATED_WRITE_ACCESS
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 1c'):
            # Test Case 1c, install PK
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'printenv -e -n PK'])
            assert 'PK:' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SecureBoot')
            assert '00000000: 01' in output
            output = u_boot_console.run_command(
                'printenv -e SetupMode')
            assert '00000000: 00' in output

        with u_boot_console.log.section('Test Case 1d'):
            # Test Case 1d, db/dbx without KEK
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 1e'):
            # Test Case 1e, install KEK
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -i 4000000:$filesize KEK'])
            assert 'Failed to set EFI variable' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'printenv -e -n KEK'])
            assert 'KEK:' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SecureBoot')
            assert '00000000: 01' in output

        with u_boot_console.log.section('Test Case 1f'):
            # Test Case 1f, install db
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SecureBoot')
            assert '00000000: 01' in output

        with u_boot_console.log.section('Test Case 1g'):
            # Test Case 1g, install dbx
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx.auth',
                'setenv -e -nv -bs -rt -i 4000000:$filesize dbx'])
            assert 'Failed to set EFI variable' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 dbx.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f dbx'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'dbx:' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SecureBoot')
            assert '00000000: 01' in output

    def test_efi_var_auth2(self, u_boot_console, efi_boot_env):
        """
        Test Case 2 - Update database by overwriting
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 2a'):
            # Test Case 2a, update without AUTHENTICATED_WRITE_ACCESS
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 2b'):
            # Test Case 2b, update without correct signature
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.esl',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 2c'):
            # Test Case 2c, update with correct signature
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

    def test_efi_var_auth3(self, u_boot_console, efi_boot_env):
        """
        Test Case 3 - Append database
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 3a'):
            # Test Case 3a, update without AUTHENTICATED_WRITE_ACCESS
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -a -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 3b'):
            # Test Case 3b, update without correct signature
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.esl',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' in ''.join(output)

        with u_boot_console.log.section('Test Case 3c'):
            # Test Case 3c, update with correct signature
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db1.auth',
                'setenv -e -nv -bs -rt -at -a -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

    def test_efi_var_auth4(self, u_boot_console, efi_boot_env):
        """
        Test Case 4 - Delete database without authentication
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 4a'):
            # Test Case 4a, update without AUTHENTICATED_WRITE_ACCESS
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'db:' in ''.join(output)

            output = u_boot_console.run_command_list([
                'setenv -e -nv -bs -rt db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' in ''.join(output)
            assert 'db:' in ''.join(output)

        with u_boot_console.log.section('Test Case 4b'):
            # Test Case 4b, update without correct signature/data
            output = u_boot_console.run_command_list([
                'setenv -e -nv -bs -rt -at db',
                'printenv -e -n -guid d719b2cb-3d3a-4596-a3bc-dad00e67656f db'])
            assert 'Failed to set EFI variable' in ''.join(output)
            assert 'db:' in ''.join(output)

    def test_efi_var_auth5(self, u_boot_console, efi_boot_env):
        """
        Test Case 5 - Uninstall(delete) PK
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 5a'):
            # Test Case 5a, Uninstall PK without correct signature
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'printenv -e -n PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert 'PK:' in ''.join(output)

            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 PK_null.esl',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'printenv -e -n PK'])
            assert 'Failed to set EFI variable' in ''.join(output)
            assert 'PK:' in ''.join(output)

        with u_boot_console.log.section('Test Case 5b'):
            # Test Case 5b, Uninstall PK with correct signature
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 PK_null.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK',
                'printenv -e -n PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)
            assert '\"PK\" not defined' in ''.join(output)

            output = u_boot_console.run_command(
                'printenv -e SecureBoot')
            assert '00000000: 00' in output
            output = u_boot_console.run_command(
                'printenv -e SetupMode')
            assert '00000000: 01' in output
