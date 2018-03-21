#!/bin/sh

# Wrap all "#define X Y" with a #ifndef X...#endif"

sed -E 's/^( *#define ([^ ]+) .*)/#ifndef \2\
\1\
#endif/' 
