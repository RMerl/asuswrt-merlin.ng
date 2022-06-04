#!/usr/bin/python

#
# $ Copyright Broadcom Corporation $
#
# <<Broadcom-WL-IPTag/Proprietary:>>

# Compares two romtable.S files for differences in #.abandon entries

# Usage:
#  abandon-diff.py <romtable.S> <romtable.S>
#

import subprocess
import sys, traceback
from pprint import pprint

romtable = "romtable.i"

def get_abandon_functions(romtable_file):
    abandon = set()
    with open(romtable_file) as f:
        lines = f.readlines()
        for n in lines:
            test = n.split()
            if '.extern' in test and ('.abandon' in test or '#.abandon' in test):
                abandon.add(test[1])
    return abandon

if __name__ == "__main__":
    original_romtable = sys.argv[1]
    generated_romtable = sys.argv[2]

    try:
        original = get_abandon_functions(original_romtable)
        generated = get_abandon_functions(generated_romtable)
    except:
        traceback.print_exc()
    else:
        # Get the common symbols
        same = original.intersection(generated)
        # Get abandons only found in original romtable.S
        only_original = original.difference(generated)
        # Get abandons only found in generated romtable.S
        only_generated = generated.difference(original)

        print "Original romtable.S: %s"%original_romtable
        print "Generated romtable.S: %s"%generated_romtable
        print ""
        print "Common Symbols : %d"%len(same)
        pprint(same)
        print ""
        print "Only in original romtable.S : %d"%len(only_original)
        pprint(only_original)
        print ""
        print "Only in generated romtable.S : %d"%len(only_generated)
        pprint(only_generated)


