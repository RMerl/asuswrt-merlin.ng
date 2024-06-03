# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Texas Instrument
# Author: JJ Hiblot <jjhiblot@ti.com>
#

from subprocess import check_call, CalledProcessError

def assert_fs_integrity(fs_type, fs_img):
    try:
        if fs_type == 'ext4':
            check_call('fsck.ext4 -n -f %s' % fs_img, shell=True)
    except CalledProcessError:
        raise
