#!/bin/sh
set -x
nm=$(basename "$0" .sh)

check()
{
    nm=$1.conf

    cat <<EOF >"$nm"
fake-address = true

provider $1 {
    include("~/.config/inadyn/$nm")
}
EOF
    ../src/inadyn -n1 -l debug -f "$nm"
}

check "$nm"
