#!/bin/sh
# Validate Tor's ntor implementation.

exitcode=0

# Run the python integration tests and return the exitcode of the python
# script.  The python script might ask the testsuite to skip it if not all
# python dependencies are covered.
"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/hs_ntor_ref.py" || exitcode=$?

exit ${exitcode}
