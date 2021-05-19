#!/usr/bin/perl -w -p -i
#
# Copyright (c) 2001 Matej Pfajfar.
# Copyright (c) 2001-2004, Roger Dingledine.
# Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
# Copyright (c) 2007-2019, The Tor Project, Inc.
# See LICENSE for licensing information

# This script looks for instances of C comparison operators as macro arguments,
# and replaces them with our OP_* equivalents.
#
# Some macros that take operators are our tt_int_op() testing macro, and the
# standard timercmp() macro.  Coccinelle can't handle their syntax, however,
# unless we give them their operators as a macro too.

next if m#^ */\*# or m#^ *\* #;

s/<([,)])/OP_LT$1/;
s/(?<=[\s,])>([,)])/OP_GT$1/;
#s/>([,)])/OP_GT$1/;
s/==([,)])/OP_EQ$1/;
s/>=([,)])/OP_GE$1/;
s/<=([,)])/OP_LE$1/;
s/!=([,)])/OP_NE$1/;
