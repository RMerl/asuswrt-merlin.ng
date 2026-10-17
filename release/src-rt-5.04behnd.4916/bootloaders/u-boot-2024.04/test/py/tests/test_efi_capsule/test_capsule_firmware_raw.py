# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

""" U-Boot UEFI: Firmware Update Test
This test verifies capsule-on-disk firmware update for raw images
"""

import pytest
from capsule_common import (
    capsule_setup,
    init_content,
    place_capsule_file,
    exec_manual_update,
    check_file_removed,
    check_file_exist,
    verify_content,
    do_reboot_dtb_specified
)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_capsule_firmware_raw')
@pytest.mark.buildconfigspec('efi_capsule_on_disk')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.slow
class TestEfiCapsuleFirmwareRaw:
    """ Tests verifying capsule-on-disk firmware update for raw images
    """

    def test_efi_capsule_fw1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """ Test Case 1
        Update U-Boot and U-Boot environment on SPI Flash
        but with an incorrect GUID value in the capsule
        No update should happen
        0x100000-0x150000: U-Boot binary (but dummy)
        0x150000-0x200000: U-Boot environment (but dummy)
        """

        # other tests might have run and the
        # system might not be in a clean state.
        # Restart before starting the tests.
        u_boot_console.restart_uboot()

        disk_img = efi_capsule_data
        capsule_files = ['Test03']
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            capsule_setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        # reboot
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')

        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # deleted anyway
            check_file_removed(u_boot_console, disk_img, capsule_files)

            verify_content(u_boot_console, '100000', 'u-boot:Old')
            verify_content(u_boot_console, '150000', 'u-boot-env:Old')

    def test_efi_capsule_fw2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """ Test Case 2
        Update U-Boot and U-Boot environment on SPI Flash but with OsIndications unset
        No update should happen
        0x100000-0x150000: U-Boot binary (but dummy)
        0x150000-0x200000: U-Boot environment (but dummy)
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test01', 'Test02']
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            capsule_setup(u_boot_console, disk_img, None)
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        # reboot
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files, False)

            check_file_exist(u_boot_console, disk_img, capsule_files)

            verify_content(u_boot_console, '100000', 'u-boot:Old')
            verify_content(u_boot_console, '150000', 'u-boot-env:Old')

    def test_efi_capsule_fw3(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """ Test Case 3
        Update U-Boot on SPI Flash, raw image format
        0x100000-0x150000: U-Boot binary (but dummy)
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test01', 'Test02']
        with u_boot_console.log.section('Test Case 3-a, before reboot'):
            capsule_setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        capsule_auth = u_boot_config.buildconfig.get(
            'config_efi_capsule_authenticate')

        # reboot
        u_boot_console.restart_uboot(expect_reset = capsule_early)

        with u_boot_console.log.section('Test Case 3-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # make sure the dfu_alt_info exists because it is required for making ESRT.
            output = u_boot_console.run_command_list([
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                'efidebug capsule esrt'])

            # ensure that SANDBOX_UBOOT_ENV_IMAGE_GUID is in the ESRT.
            assert '5A7021F5-FEF2-48B4-AABA-832E777418C0' in ''.join(output)

            # ensure that SANDBOX_UBOOT_IMAGE_GUID is in the ESRT.
            assert '09D7CF52-0720-4710-91D1-08469B7FE9C8' in ''.join(output)

            check_file_removed(u_boot_console, disk_img, capsule_files)

            expected = 'u-boot:Old' if capsule_auth else 'u-boot:New'
            verify_content(u_boot_console, '100000', expected)

            expected = 'u-boot-env:Old' if capsule_auth else 'u-boot-env:New'
            verify_content(u_boot_console, '150000', expected)

    def test_efi_capsule_fw4(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """ Test Case 4
        Update U-Boot on SPI Flash, raw image format with fw_version and lowest_supported_version
        0x100000-0x150000: U-Boot binary (but dummy)
        0x150000-0x200000: U-Boot environment (but dummy)
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test101', 'Test102']
        with u_boot_console.log.section('Test Case 4-a, before reboot'):
            capsule_setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        # reboot
        do_reboot_dtb_specified(u_boot_config, u_boot_console, 'test_ver.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        capsule_auth = u_boot_config.buildconfig.get(
            'config_efi_capsule_authenticate')
        with u_boot_console.log.section('Test Case 4-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # deleted anyway
            check_file_removed(u_boot_console, disk_img, capsule_files)

            # make sure the dfu_alt_info exists because it is required for making ESRT.
            output = u_boot_console.run_command_list([
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000'
                'u-boot-env raw 0x150000 0x200000"',
                'efidebug capsule esrt'])

            if capsule_auth:
                # capsule authentication failed
                verify_content(u_boot_console, '100000', 'u-boot:Old')
                verify_content(u_boot_console, '150000', 'u-boot-env:Old')
            else:
                # ensure that SANDBOX_UBOOT_IMAGE_GUID is in the ESRT.
                assert '09D7CF52-0720-4710-91D1-08469B7FE9C8' in ''.join(output)
                assert 'ESRT: fw_version=5' in ''.join(output)
                assert 'ESRT: lowest_supported_fw_version=3' in ''.join(output)

                # ensure that SANDBOX_UBOOT_ENV_IMAGE_GUID is in the ESRT.
                assert '5A7021F5-FEF2-48B4-AABA-832E777418C0' in ''.join(output)
                assert 'ESRT: fw_version=10' in ''.join(output)
                assert 'ESRT: lowest_supported_fw_version=7' in ''.join(output)

                verify_content(u_boot_console, '100000', 'u-boot:New')
                verify_content(u_boot_console, '150000', 'u-boot-env:New')

    def test_efi_capsule_fw5(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """ Test Case 5
        Update U-Boot on SPI Flash, raw image format with fw_version and lowest_supported_version
        but fw_version is lower than lowest_supported_version
        No update should happen
        0x100000-0x150000: U-Boot binary (but dummy)
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test103']
        with u_boot_console.log.section('Test Case 5-a, before reboot'):
            capsule_setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        # reboot
        do_reboot_dtb_specified(u_boot_config, u_boot_console, 'test_ver.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 5-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            check_file_removed(u_boot_console, disk_img, capsule_files)

            verify_content(u_boot_console, '100000', 'u-boot:Old')
