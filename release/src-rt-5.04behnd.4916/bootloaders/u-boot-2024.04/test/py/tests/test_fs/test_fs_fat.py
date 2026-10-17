# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2023 Weidmüller Interface GmbH & Co. KG
# Author: Christian Taedcke <christian.taedcke@weidmueller.com>
#
# U-Boot File System: FAT Test

"""
This test verifies fat specific file system behaviour.
"""

import pytest
import re

@pytest.mark.boardspec('sandbox')
@pytest.mark.slow
class TestFsFat(object):
    def test_fs_fat1(self, u_boot_console, fs_obj_fat):
        """Test that `fstypes` prints a result which includes `sandbox`."""
        fs_type,fs_img = fs_obj_fat
        with u_boot_console.log.section('Test Case 1 - fatinfo'):
            # Test Case 1 - ls
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % fs_img,
                'fatinfo host 0:0'])
            assert(re.search('Filesystem: %s' % fs_type.upper(), ''.join(output)))
