#!/bin/sh

# apply.sh:
# run spatch with appropriate includes and builtins for the Tor source code

top="$(dirname "$0")/../.."

spatch -macro_file_builtins "$top"/scripts/coccinelle/tor-coccinelle.h \
       -I "$top" -I "$top"/src -I "$top"/ext --defined COCCI "$@"
