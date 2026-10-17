# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Weidmueller GmbH
# Written by Lukas Funke <lukas.funke@weidmueller.com>
#
# Entry-type module for Zynq(MP) boot images (boot.bin)
#

import tempfile

from collections import OrderedDict

from binman import elf
from binman.etype.section import Entry_section

from dtoc import fdt_util

from u_boot_pylib import tools
from u_boot_pylib import command

# pylint: disable=C0103
class Entry_xilinx_bootgen(Entry_section):
    """Signed SPL boot image for Xilinx ZynqMP devices

    Properties / Entry arguments:
        - auth-params: (Optional) Authentication parameters passed to bootgen
        - fsbl-config: (Optional) FSBL parameters passed to bootgen
        - keysrc-enc: (Optional) Key source when using decryption engine
        - pmufw-filename: Filename of PMU firmware. Default: pmu-firmware.elf
        - psk-key-name-hint: Name of primary secret key to use for signing the
                             secondardy public key. Format: .pem file
        - ssk-key-name-hint: Name of secondardy secret key to use for signing
                             the boot image. Format: .pem file

    The etype is used to create a boot image for Xilinx ZynqMP
    devices.

    Information for signed images:

    In AMD/Xilinx SoCs, two pairs of public and secret keys are used
    - primary and secondary. The function of the primary public/secret key pair
    is to authenticate the secondary public/secret key pair.
    The function of the secondary key is to sign/verify the boot image. [1]

    AMD/Xilinx uses the following terms for private/public keys [1]:

        PSK = Primary Secret Key (Used to sign Secondary Public Key)
        PPK = Primary Public Key (Used to verify Secondary Public Key)
        SSK = Secondary Secret Key (Used to sign the boot image/partitions)
        SPK = Used to verify the actual boot image

    The following example builds a signed boot image. The fuses of
    the primary public key (ppk) should be fused together with the RSA_EN flag.

    Example node::

        spl {
            filename = "boot.signed.bin";

            xilinx-bootgen {
                psk-key-name-hint = "psk0";
                ssk-key-name-hint = "ssk0";
                auth-params = "ppk_select=0", "spk_id=0x00000000";

                u-boot-spl-nodtb {
                };
                u-boot-spl-pubkey-dtb {
                    algo = "sha384,rsa4096";
                    required = "conf";
                    key-name-hint = "dev";
                };
            };
        };

    For testing purposes, e.g. if no RSA_EN should be fused, one could add
    the "bh_auth_enable" flag in the fsbl-config field. This will skip the
    verification of the ppk fuses and boot the image, even if ppk hash is
    invalid.

    Example node::

        xilinx-bootgen {
            psk-key-name-hint = "psk0";
            psk-key-name-hint = "ssk0";
            ...
            fsbl-config = "bh_auth_enable";
            ...
        };

    [1] https://docs.xilinx.com/r/en-US/ug1283-bootgen-user-guide/Using-Authentication

    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._auth_params = None
        self._entries = OrderedDict()
        self._filename = None
        self._fsbl_config = None
        self._keysrc_enc = None
        self._pmufw_filename = None
        self._psk_key_name_hint = None
        self._ssk_key_name_hint = None
        self.align_default = None
        self.bootgen = None
        self.required_props = ['pmufw-filename',
                               'psk-key-name-hint',
                               'ssk-key-name-hint']

    def ReadNode(self):
        """Read properties from the xilinx-bootgen node"""
        super().ReadNode()
        self._auth_params = fdt_util.GetStringList(self._node,
                                                   'auth-params')
        self._filename = fdt_util.GetString(self._node, 'filename')
        self._fsbl_config = fdt_util.GetStringList(self._node,
                                                   'fsbl-config')
        self._keysrc_enc = fdt_util.GetString(self._node,
                                                   'keysrc-enc')
        self._pmufw_filename = fdt_util.GetString(self._node, 'pmufw-filename')
        self._psk_key_name_hint = fdt_util.GetString(self._node,
                                                     'psk-key-name-hint')
        self._ssk_key_name_hint = fdt_util.GetString(self._node,
                                                   'ssk-key-name-hint')
        self.ReadEntries()

    @classmethod
    def _ToElf(cls, data, output_fname):
        """Convert SPL object file to bootable ELF file

        Args:
            data (bytearray): u-boot-spl-nodtb + u-boot-spl-pubkey-dtb obj file
                                data
            output_fname (str): Filename of converted FSBL ELF file
        """
        platform_elfflags = {"aarch64":
                        ["-B", "aarch64", "-O", "elf64-littleaarch64"],
                        # amd64 support makes no sense for the target
                        # platform, but we include it here to enable
                        # testing on hosts
                        "x86_64":
                        ["-B", "i386", "-O", "elf64-x86-64"]
                        }

        gcc, args = tools.get_target_compile_tool('cc')
        args += ['-dumpmachine']
        stdout = command.output(gcc, *args)
        # split target machine triplet (arch, vendor, os)
        arch, _, _ = stdout.split('-')

        spl_elf = elf.DecodeElf(tools.read_file(
            tools.get_input_filename('spl/u-boot-spl')), 0)

        # Obj file to swap data and text section (rename-section)
        with tempfile.NamedTemporaryFile(prefix="u-boot-spl-pubkey-",
                                    suffix=".o.tmp",
                                    dir=tools.get_output_dir())\
                                    as tmp_obj:
            input_objcopy_fname = tmp_obj.name
            # Align packed content to 4 byte boundary
            pad = bytearray(tools.align(len(data), 4) - len(data))
            tools.write_file(input_objcopy_fname, data + pad)
            # Final output elf file which contains a valid start address
            with tempfile.NamedTemporaryFile(prefix="u-boot-spl-pubkey-elf-",
                                            suffix=".o.tmp",
                                            dir=tools.get_output_dir())\
                                                as tmp_elf_obj:
                input_ld_fname = tmp_elf_obj.name
                objcopy, args = tools.get_target_compile_tool('objcopy')
                args += ["--rename-section", ".data=.text",
                        "-I", "binary"]
                args += platform_elfflags[arch]
                args += [input_objcopy_fname, input_ld_fname]
                command.run(objcopy, *args)

                ld, args = tools.get_target_compile_tool('ld')
                args += [input_ld_fname, '-o', output_fname,
                         "--defsym", f"_start={hex(spl_elf.entry)}",
                         "-Ttext", hex(spl_elf.entry)]
                command.run(ld, *args)

    def BuildSectionData(self, required):
        """Pack node content, and create bootable, signed ZynqMP boot image

        The method collects the content of this node (usually SPL + dtb) and
        converts them to an ELF file. The ELF file is passed to the
        Xilinx bootgen tool which packs the SPL ELF file together with
        Platform Management Unit (PMU) firmware into a bootable image
        for ZynqMP devices. The image is signed within this step.

        The result is a bootable, signed SPL image for Xilinx ZynqMP devices.
        """
        data = super().BuildSectionData(required)
        bootbin_fname = self._filename if self._filename else \
                            tools.get_output_filename(
                            f'boot.{self.GetUniqueName()}.bin')

        pmufw_elf_fname = tools.get_input_filename(self._pmufw_filename)
        psk_fname = tools.get_input_filename(self._psk_key_name_hint + ".pem")
        ssk_fname = tools.get_input_filename(self._ssk_key_name_hint + ".pem")
        fsbl_config = ";".join(self._fsbl_config) if self._fsbl_config else None
        auth_params = ";".join(self._auth_params) if self._auth_params else None

        spl_elf_fname = tools.get_output_filename('u-boot-spl-pubkey.dtb.elf')

        # We need to convert to node content (see above) into an ELF
        # file in order to be processed by bootgen.
        self._ToElf(bytearray(data), spl_elf_fname)

        # Call Bootgen in order to sign the SPL
        if self.bootgen.sign('zynqmp', spl_elf_fname, pmufw_elf_fname,
                        psk_fname, ssk_fname, fsbl_config,
                        auth_params, self._keysrc_enc, bootbin_fname) is None:
            # Bintool is missing; just use empty data as the output
            self.record_missing_bintool(self.bootgen)
            data = tools.get_bytes(0, 1024)
        else:
            data = tools.read_file(bootbin_fname)

        self.SetContents(data)

        return data

    # pylint: disable=C0116
    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.bootgen = self.AddBintool(btools, 'bootgen')
