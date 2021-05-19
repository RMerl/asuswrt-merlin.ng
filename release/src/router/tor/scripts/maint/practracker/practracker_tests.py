#!/usr/bin/env python

"""Some simple tests for practracker metrics"""

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import unittest

try:
    # python 2 names the module this way...
    from StringIO import StringIO
except ImportError:
    # python 3 names the module this way.
    from io import StringIO

import metrics

function_file = """static void
fun(directory_request_t *req, const char *resource)
{
  time_t if_modified_since = 0;
  uint8_t or_diff_from[DIGEST256_LEN];
}

static void
fun(directory_request_t *req,
      const char *resource)
{
  time_t if_modified_since = 0;
  uint8_t or_diff_from[DIGEST256_LEN];
}

MOCK_IMPL(void,
fun,(
       uint8_t dir_purpose,
       uint8_t router_purpose,
       const char *resource,
       int pds_flags,
       download_want_authority_t want_authority))
{
  const routerstatus_t *rs = NULL;
  const or_options_t *options = get_options();
}
"""

class TestFunctionLength(unittest.TestCase):
    def test_function_length(self):
        funcs = StringIO(function_file)
        # All functions should have length 2
        for name, lines in metrics.get_function_lines(funcs):
            self.assertEqual(name, "fun")

        funcs.seek(0)

        for name, lines in metrics.get_function_lines(funcs):
            self.assertEqual(lines, 4)

class TestIncludeCount(unittest.TestCase):
    def test_include_count(self):
        f = StringIO("""
  #   include <abc.h>
  #   include "def.h"
#include "ghi.h"
\t#\t include "jkl.h"
""")
        self.assertEqual(metrics.get_include_count(f),4)

if __name__ == '__main__':
    unittest.main()
