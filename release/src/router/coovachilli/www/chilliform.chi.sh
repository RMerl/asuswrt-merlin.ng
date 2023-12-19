#!/bin/sh
. ./config.sh

cat <<EOF
HTTP/1.0 200 OK
Content-Type: text/javascript
Cache: none

var o = document.getElementById('logonForm');
if (o != null) {
EOF

chilliwww="http:\/\/$HS_UAMLISTEN:$HS_UAMPORT"
echo "o.innerHTML='"$(cat json_html.tmpl|tr '\n' ' '|sed -e "s/'/\\'/"|sed -e "s/CHILLIWWW/$chilliwww/g")"';" 
echo "}"

echo "setTimeout('chilliController.refresh()', 0);"
