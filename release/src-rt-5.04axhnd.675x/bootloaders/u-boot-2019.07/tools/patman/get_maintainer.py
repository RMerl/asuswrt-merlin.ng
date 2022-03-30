# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
#

import command
import gitutil
import os

def FindGetMaintainer():
    """Look for the get_maintainer.pl script.

    Returns:
        If the script is found we'll return a path to it; else None.
    """
    try_list = [
        os.path.join(gitutil.GetTopLevel(), 'scripts'),
        ]
    # Look in the list
    for path in try_list:
        fname = os.path.join(path, 'get_maintainer.pl')
        if os.path.isfile(fname):
            return fname

    return None

def GetMaintainer(fname, verbose=False):
    """Run get_maintainer.pl on a file if we find it.

    We look for get_maintainer.pl in the 'scripts' directory at the top of
    git.  If we find it we'll run it.  If we don't find get_maintainer.pl
    then we fail silently.

    Args:
        fname: Path to the patch file to run get_maintainer.pl on.

    Returns:
        A list of email addresses to CC to.
    """
    get_maintainer = FindGetMaintainer()
    if not get_maintainer:
        if verbose:
            print("WARNING: Couldn't find get_maintainer.pl")
        return []

    stdout = command.Output(get_maintainer, '--norolestats', fname)
    lines = stdout.splitlines()
    return [ x.replace('"', '') for x in lines ]
