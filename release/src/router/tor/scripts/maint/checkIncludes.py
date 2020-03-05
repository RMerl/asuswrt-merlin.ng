#!/usr/bin/python
# Copyright 2018 The Tor Project, Inc.  See LICENSE file for licensing info.

# This file is no longer here; see practracker/includes.py for this
# functionality.  This is a stub file that exists so that older git
# hooks will know where to look.

import sys, os

dirname = os.path.split(sys.argv[0])[0]
new_location = os.path.join(dirname, "practracker", "includes.py")
python = sys.executable

os.execl(python, python, new_location, *sys.argv[1:])
