#!/bin/sh

export KSROOT=/koolshare
export PERP_BASE=$KSROOT/perp
export PATH=$KSROOT/bin:$KSROOT/scripts:$PATH

ACTION=$1
ID=$1
export LANIP=127.0.0.1

http_response()  {
    ARG0="$@"
    curl -X POST -d "$ARG0" http://$LANIP/_resp/$ID
}
