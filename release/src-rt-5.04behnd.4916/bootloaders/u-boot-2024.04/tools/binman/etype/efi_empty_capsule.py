# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for producing an empty  EFI capsule
#

import os

from binman.entry import Entry
from binman.etype.efi_capsule import get_binman_test_guid
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_efi_empty_capsule(Entry_section):
    """Generate EFI empty capsules

    The parameters needed for generation of the empty capsules can
    be provided as properties in the entry.

    Properties / Entry arguments:
    - image-guid: Image GUID which will be used for identifying the
      updatable image on the board. Mandatory for accept capsule.
    - capsule-type - String to indicate type of capsule to generate. Valid
      values are 'accept' and 'revert'.

    For more details on the description of the capsule format, and the capsule
    update functionality, refer Section 8.5 and Chapter 23 in the `UEFI
    specification`_. For more information on the empty capsule, refer the
    sections 2.3.2 and 2.3.3 in the `Dependable Boot specification`_.

    A typical accept empty capsule entry node would then look something like this

    empty-capsule {
            type = "efi-empty-capsule";
            /* GUID of image being accepted */
            image-type-id = SANDBOX_UBOOT_IMAGE_GUID;
            capsule-type = "accept";
    };

    A typical revert empty capsule entry node would then look something like this

    empty-capsule {
            type = "efi-empty-capsule";
            capsule-type = "revert";
    };

    The empty capsules do not have any input payload image.

    .. _`UEFI specification`: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf
    .. _`Dependable Boot specification`: https://git.codelinaro.org/linaro/dependable-boot/mbfw/uploads/6f7ddfe3be24e18d4319e108a758d02e/mbfw.pdf
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['capsule-type']
        self.accept = 0
        self.revert = 0

    def ReadNode(self):
        super().ReadNode()

        self.image_guid = fdt_util.GetString(self._node, 'image-guid')
        self.capsule_type = fdt_util.GetString(self._node, 'capsule-type')

        if self.capsule_type != 'accept' and self.capsule_type != 'revert':
            self.Raise('capsule-type should be either \'accept\' or \'revert\'')

        if self.capsule_type == 'accept' and not self.image_guid:
            self.Raise('Image GUID needed for generating accept capsule')

    def BuildSectionData(self, required):
        uniq = self.GetUniqueName()
        outfile = self._filename if self._filename else 'capsule.%s' % uniq
        capsule_fname = tools.get_output_filename(outfile)
        accept = True if self.capsule_type == 'accept' else False
        guid = self.image_guid
        if self.image_guid == "binman-test":
            guid = get_binman_test_guid('binman-test')

        ret = self.mkeficapsule.generate_empty_capsule(guid, capsule_fname,
                                                       accept)
        if ret is not None:
            return tools.read_file(capsule_fname)

    def AddBintools(self, btools):
        self.mkeficapsule = self.AddBintool(btools, 'mkeficapsule')
