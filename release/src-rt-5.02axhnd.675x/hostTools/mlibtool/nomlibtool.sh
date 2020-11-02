#!/bin/sh
while expr "$1" : '-' > /dev/null 2> /dev/null; do shift; done
exec "$@"
