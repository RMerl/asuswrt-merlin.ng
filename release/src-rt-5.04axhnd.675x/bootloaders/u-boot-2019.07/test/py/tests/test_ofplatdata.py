# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest

OF_PLATDATA_OUTPUT = '''
of-platdata probe:
bool 1
byte 05
bytearray 06 00 00
int 1
intarray 2 3 4 0
longbytearray 09 0a 0b 0c 0d 0e 0f 10 11
string message
stringarray "multi-word" "message" ""
of-platdata probe:
bool 0
byte 08
bytearray 01 23 34
int 3
intarray 5 0 0 0
longbytearray 09 00 00 00 00 00 00 00 00
string message2
stringarray "another" "multi-word" "message"
of-platdata probe:
bool 0
byte 00
bytearray 00 00 00
int 0
intarray 0 0 0 0
longbytearray 00 00 00 00 00 00 00 00 00
string <NULL>
stringarray "one" "" ""
'''

@pytest.mark.buildconfigspec('spl_of_platdata')
def test_ofplatdata(u_boot_console):
    """Test that of-platdata can be generated and used in sandbox"""
    cons = u_boot_console
    cons.restart_uboot_with_flags(['--show_of_platdata'])
    output = cons.get_spawn_output().replace('\r', '')
    assert OF_PLATDATA_OUTPUT in output
