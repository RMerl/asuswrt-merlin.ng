#!/bin/sh
# Update our copy of waf

TARGETDIR="`dirname $0`"
WORKDIR="`mktemp -d -t update-waf-XXXXXX`"

mkdir -p "$WORKDIR"

git clone https://code.google.com/p/waf.waf15/ "$WORKDIR"

rsync -C -avz --delete "$WORKDIR/wafadmin/" "$TARGETDIR/wafadmin/"

rm -rf "$WORKDIR"
