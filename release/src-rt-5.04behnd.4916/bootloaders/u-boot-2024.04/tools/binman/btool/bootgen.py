# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2023 Weidmüller Interface GmbH & Co. KG
# Lukas Funke <lukas.funke@weidmueller.com>
#
"""Bintool implementation for bootgen

bootgen allows creating bootable SPL for Zynq(MP)

Documentation is available via:
https://www.xilinx.com/support/documents/sw_manuals/xilinx2022_1/ug1283-bootgen-user-guide.pdf

Source code is available at:
https://github.com/Xilinx/bootgen

"""

from binman import bintool
from u_boot_pylib import tools

# pylint: disable=C0103
class Bintoolbootgen(bintool.Bintool):
    """Generate bootable fsbl image for zynq/zynqmp

    This bintools supports running Xilinx "bootgen" in order
    to generate a bootable, authenticated image form an SPL.

    """
    def __init__(self, name):
        super().__init__(name, 'Xilinx Bootgen',
                         version_regex=r'^\*\*\*\*\*\* *Xilinx Bootgen *(.*)',
                         version_args='-help')

    # pylint: disable=R0913
    def sign(self, arch, spl_elf_fname, pmufw_elf_fname,
             psk_fname, ssk_fname, fsbl_config, auth_params, keysrc_enc,
             output_fname):
        """Sign SPL elf file and bundle it with PMU firmware into an image

        The method bundels the SPL together with a 'Platform Management Unit'
        (PMU)[1] firmware into a single bootable image. The image in turn is
        signed with the provided 'secondary secret key' (ssk), which in turn is
        signed with the 'primary secret key' (psk). In order to verify the
        authenticity of the ppk, it's hash has to be fused into the device
        itself.

        In Xilinx terms the SPL is usually called 'FSBL'
        (First Stage Boot Loader). The jobs of the SPL and the FSBL are mostly
        the same: load bitstream, bootstrap u-boot.

        Args:
            arch (str): Xilinx SoC architecture. Currently only 'zynqmp' is
                supported.
            spl_elf_fname (str): Filename of SPL ELF file. The filename must end
                with '.elf' in order for bootgen to recognized it as an ELF
                file. Otherwise the start address field is missinterpreted.
            pmufw_elf_fname (str): Filename PMU ELF firmware.
            psk_fname (str): Filename of the primary secret key (psk). The psk
                is a .pem file which holds the RSA private key used for signing
                the secondary secret key.
            ssk_fname (str): Filename of the secondary secret key. The ssk
                is a .pem file which holds the RSA private key used for signing
                the actual boot firmware.
            fsbl_config (str): FSBL config options. A string list of fsbl config
                options. Valid values according to [2] are:
                "bh_auth_enable": Boot Header Authentication Enable: RSA
                    authentication of the bootimage is done
                    excluding the verification of PPK hash and SPK ID. This is
                    useful for debugging before bricking a device.
                "auth_only": Boot image is only RSA signed. FSBL should not be
                    decrypted. See the
                    Zynq UltraScale+ Device Technical Reference Manual (UG1085)
                    for more information.
                There are more options which relate to PUF (physical unclonable
                functions). Please refer to Xilinx manuals for further info.
            auth_params (str): Authentication parameter. A semicolon separated
                list of authentication parameters. Valid values according to [3]
                are:
                "ppk_select=<0|1>" - Select which ppk to use
                "spk_id=<32-bit spk id>" - Specifies which SPK can be
                    used or revoked, default is 0x0
                "spk_select=<spk-efuse/user-efuse>" - To differentiate spk and
                    user efuses.
                "auth_header" - To authenticate headers when no partition
                    is authenticated.
            keysrc_enc (str): This specifies the Key source for encryption.
                Valid values according to [3] are:
                "bbram_red_key" - RED key stored in BBRAM
                "efuse_red_key" - RED key stored in eFUSE
                "efuse_gry_key" - Grey (Obfuscated) Key stored in eFUSE.
                "bh_gry_key" - Grey (Obfuscated) Key stored in boot header
                "bh_blk_key" - Black Key stored in boot header
                "efuse_blk_key" - Black Key stored in eFUSE
                "kup_key" - User Key

            output_fname (str): Filename where bootgen should write the result
        
        Returns:
            str: Bootgen output from stdout

        [1] https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841724/PMU+Firmware
        [2] https://docs.xilinx.com/r/en-US/ug1283-bootgen-user-guide/fsbl_config
        [3] https://docs.xilinx.com/r/en-US/ug1283-bootgen-user-guide/auth_params
        [4] https://docs.xilinx.com/r/en-US/ug1283-bootgen-user-guide/keysrc_encryption
        """

        _fsbl_config = f"[fsbl_config] {fsbl_config}" if fsbl_config else ""
        _auth_params = f"[auth_params] {auth_params}" if auth_params else ""
        _keysrc_enc  = f"[keysrc_encryption] {keysrc_enc}" if keysrc_enc else ""

        bif_template = f"""u_boot_spl_aes_rsa: {{
            [pskfile] {psk_fname}
            [sskfile] {ssk_fname}
            {_keysrc_enc}
            {_fsbl_config}
            {_auth_params}
            [ bootloader,
              authentication = rsa,
              destination_cpu=a53-0] {spl_elf_fname}
            [pmufw_image] {pmufw_elf_fname}
        }}"""
        args = ["-arch", arch]

        bif_fname = tools.get_output_filename('bootgen-in.sign.bif')
        tools.write_file(bif_fname, bif_template, False)
        args += ["-image", bif_fname, '-w', '-o', output_fname]
        return self.run_cmd(*args)

    def fetch(self, method):
        """Fetch bootgen from git"""
        if method != bintool.FETCH_BUILD:
            return None

        result = self.build_from_git(
            'https://github.com/Xilinx/bootgen',
            ['all'],
            'bootgen')
        return result
