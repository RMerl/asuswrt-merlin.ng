# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
# Copyright (c) 2022 Maxim Cournoyer <maxim.cournoyer@savoirfairelinux.com>
#

import os
import shlex
import shutil

from patman import gitutil
from u_boot_pylib import command


def find_get_maintainer(script_file_name):
    """Try to find where `script_file_name` is.

    It searches in PATH and falls back to a path relative to the top
    of the current git repository.
    """
    get_maintainer = shutil.which(script_file_name)
    if get_maintainer:
        return get_maintainer

    git_relative_script = os.path.join(gitutil.get_top_level(),
                                       script_file_name)
    if os.path.exists(git_relative_script):
        return git_relative_script


def get_maintainer(script_file_name, fname, verbose=False):
    """Run `script_file_name` on a file.

    `script_file_name` should be a get_maintainer.pl-like script that
    takes a patch file name as an input and return the email addresses
    of the associated maintainers to standard output, one per line.

    If `script_file_name` does not exist we fail silently.

    Args:
        script_file_name: The file name of the get_maintainer.pl script
            (or compatible).
        fname: File name of the patch to process with get_maintainer.pl.

    Returns:
        A list of email addresses to CC to.
    """
    # Expand `script_file_name` into a file name and its arguments, if
    # any.
    cmd_args = shlex.split(script_file_name)
    file_name = cmd_args[0]
    arguments = cmd_args[1:]

    get_maintainer = find_get_maintainer(file_name)
    if not get_maintainer:
        if verbose:
            print("WARNING: Couldn't find get_maintainer.pl")
        return []

    stdout = command.output(get_maintainer, *arguments, fname)
    lines = stdout.splitlines()
    return [x.replace('"', '') for x in lines]
