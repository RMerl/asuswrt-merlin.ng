#!/bin/sh
# copyright (c) 2012 David Bird (Coova Technologies)
# this is the pure shell version...
cat <<EOF
HTTP/1.0 200 OK
Content-Type: text/javascript
Cache: none

EOF

. ./config.sh

cat ChilliLibrary.js

echo "chilliController.host = '$HS_UAMLISTEN';"
echo "chilliController.port = $HS_UAMPORT;"
[ -n "$HS_UAMSERVICE" ] && echo "chilliController.uamService = '$HS_UAMSERVICE';"
[ "$HS_OPENIDAUTH" = "on" ] && echo "chilliController.openid = true;"

cat chilliController.js
