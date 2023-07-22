# Generate a large text file to test compile
# the resulting code that contains a large (>65536 bytes)
# resource entry.  Just have 12 iterations of the following:
#
# -100 of each of the lowercase letters
# -100 of each of the uppercase letters
# -100 of the numbers 0 to 9
#
# See issue #1580

import io
import string
import sys

if len(sys.argv) != 2:
    raise SystemExit('Usage: %s <output-file>' % sys.argv[0])

with open(sys.argv[1], 'w', newline='\n') as f:
    for count in range(12):
        for c in string.ascii_lowercase:
            f.write("%s\n" % (c * 100))

        for c in string.ascii_uppercase:
            f.write("%s\n" % (c * 100))

        for i in range(10):
            f.write("%s\n" % (str(i) * 100))
