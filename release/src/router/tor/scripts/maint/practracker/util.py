# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import os

# We don't want to run metrics for unittests, automatically-generated C files,
# external libraries or git leftovers.
EXCLUDE_SOURCE_DIRS = {"src/test/", "src/trunnel/", "src/rust/",
                       "src/ext/" }

EXCLUDE_FILES = {"orconfig.h"}

def _norm(p):
    return os.path.normcase(os.path.normpath(p))

def get_tor_c_files(tor_topdir, include_dirs=None):
    """
    Return a list with the .c and .h filenames we want to get metrics of.
    """
    files_list = []
    exclude_dirs = { _norm(os.path.join(tor_topdir, p)) for p in EXCLUDE_SOURCE_DIRS }

    if include_dirs is None:
        topdirs = [ tor_topdir ]
    else:
        topdirs = [ os.path.join(tor_topdir, inc) for inc in include_dirs ]

    for topdir in topdirs:
        for root, directories, filenames in os.walk(topdir):
            # Remove all the directories that are excluded.
            directories[:] = [ d for d in directories
                               if _norm(os.path.join(root,d)) not in exclude_dirs ]
            directories.sort()
            filenames.sort()
            for filename in filenames:
                # We only care about .c and .h files
                if not (filename.endswith(".c") or filename.endswith(".h")):
                    continue
                if filename in EXCLUDE_FILES:
                    continue
                # Avoid editor temporary files
                bname = os.path.basename(filename)
                if bname.startswith("."):
                    continue
                if bname.startswith("#"):
                    continue

                full_path = os.path.join(root,filename)

                files_list.append(full_path)

    return files_list

class NullFile:
    """A file-like object that we can us to suppress output."""
    def __init__(self):
        pass
    def write(self, s):
        pass
