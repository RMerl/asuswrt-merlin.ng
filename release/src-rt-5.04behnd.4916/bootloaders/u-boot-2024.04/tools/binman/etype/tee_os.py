# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# Entry-type module for OP-TEE Trusted OS firmware blob
#

import struct

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg
from binman import elf

class Entry_tee_os(Entry_blob_named_by_arg):
    """Entry containing an OP-TEE Trusted OS (TEE) blob

    Properties / Entry arguments:
        - tee-os-path: Filename of file to read into entry. This is typically
            called tee.bin or tee.elf

    This entry holds the run-time firmware, typically started by U-Boot SPL.
    See the U-Boot README for your architecture or board for how to use it. See
    https://github.com/OP-TEE/optee_os for more information about OP-TEE.

    Note that if the file is in ELF format, it must go in a FIT. In that case,
    this entry will mark itself as absent, providing the data only through the
    read_elf_segments() method.

    Marking this entry as absent means that it if is used in the wrong context
    it can be automatically dropped. Thus it is possible to add an OP-TEE entry
    like this::

        binman {
            tee-os {
            };
        };

    and pass either an ELF or plain binary in with -a tee-os-path <filename>
    and have binman do the right thing:

       - include the entry if tee.bin is provided and it does NOT have the v1
         header
       - drop it otherwise

    When used within a FIT, we can do::

        binman {
            fit {
                tee-os {
                };
            };
        };

    which will split the ELF into separate nodes for each segment, if an ELF
    file is provided (see :ref:`etype_fit`), or produce a single node if the
    OP-TEE binary v1 format is provided (see optee_doc_) .

    .. _optee_doc: https://optee.readthedocs.io/en/latest/architecture/core.html#partitioning-of-the-binary
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'tee-os')
        self.external = True

    @staticmethod
    def is_optee_bin_v1(data):
        return len(data) >= 8 and data[0:5] == b'OPTE\x01'

    def ObtainContents(self, fake_size=0):
        result = super().ObtainContents(fake_size)
        if not self.missing:
            # If using the flat binary (without the OP-TEE header), then it is
            # just included as a blob. But if it is an ELF or usees the v1
            # binary header, then the FIT implementation will call
            # read_elf_segments() to get the segment information
            if elf.is_valid(self.data):
                self.mark_absent('uses Elf format which must be in a FIT')
            elif self.is_optee_bin_v1(self.data):
                # The FIT implementation will call read_elf_segments() to get
                # the segment information
                self.mark_absent('uses v1 format which must be in a FIT')
        return result

    def read_elf_segments(self):
        data = self.GetData()
        if self.is_optee_bin_v1(data):
            # OP-TEE v1 format (tee.bin)
            init_sz, start_hi, start_lo, _, paged_sz = (
                struct.unpack_from('<5I', data, 0x8))
            if paged_sz != 0:
                self.Raise("OP-TEE paged mode not supported")
            e_entry = (start_hi << 32) + start_lo
            p_addr = e_entry
            p_data = data[0x1c:]
            if len(p_data) != init_sz:
                self.Raise("Invalid OP-TEE file: size mismatch (expected %#x, have %#x)" %
                           (init_sz, len(p_data)))
            return [[0, p_addr, p_data]], e_entry
        return None
