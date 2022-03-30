# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
#

import os.path

import gitutil

def DetectProject():
    """Autodetect the name of the current project.

    This looks for signature files/directories that are unlikely to exist except
    in the given project.

    Returns:
        The name of the project, like "linux" or "u-boot".  Returns "unknown"
        if we can't detect the project.
    """
    top_level = gitutil.GetTopLevel()

    if os.path.exists(os.path.join(top_level, "include", "u-boot")):
        return "u-boot"
    elif os.path.exists(os.path.join(top_level, "kernel")):
        return "linux"

    return "unknown"
