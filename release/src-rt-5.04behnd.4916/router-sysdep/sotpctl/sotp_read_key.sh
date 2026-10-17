#! /bin/sh

if [ -z "$1" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
        echo ""
        echo "sotp_read_key.sh <SOTP keyslot to read>"
        echo "  -h, print help text."
        return 0
fi

echo "SOTP keyslot $1 contents:"
echo ""
key=`sotpctl get keyslot $1 | grep Word | cut -d'x' -f2 | sed 's/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/g' | sed ':a;N;$!ba;s/\n//g'`
echo "key:[$key]"
