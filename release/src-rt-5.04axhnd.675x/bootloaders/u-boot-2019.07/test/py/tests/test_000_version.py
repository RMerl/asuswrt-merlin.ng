# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

# pytest runs tests the order of their module path, which is related to the
# filename containing the test. This file is named such that it is sorted
# first, simply as a very basic sanity check of the functionality of the U-Boot
# command prompt.

def test_version(u_boot_console):
    """Test that the "version" command prints the U-Boot version."""

    # "version" prints the U-Boot sign-on message. This is usually considered
    # an error, so that any unexpected reboot causes an error. Here, this
    # error detection is disabled since the sign-on message is expected.
    with u_boot_console.disable_check('main_signon'):
        response = u_boot_console.run_command('version')
    # Ensure "version" printed what we expected.
    u_boot_console.validate_version_string_in_text(response)
