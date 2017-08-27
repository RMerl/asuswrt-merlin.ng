#!/bin/sh
#
#  This script makes an HTML page from a simple directory listing
#
#
cat >index.html <<EOF
<HTML>
<TITLE>Index of FreeRADIUS.org's RFC site</TITLE>
<BODY>

<H1>Index of FreeRADIUS.org's RFC site</H1>

List of <A HREF="attributes.html">RADIUS attributes</A>
<P>

EOF

#
#  include the message, if any exists
#
if [ -e message ]; then
  echo "<PRE>" >> index.html
  cat .message >> index.html
  echo "</PRE>" >> index.html
fi

#
#  for all of the text files, do this
#
cat >>index.html <<EOF
<h2>RFC's</h2>
EOF

for x in rfc*.html;do
  y=`echo $x | sed 's/rfc//;s/\.html//'`
  echo "<A HREF=\"$x\">RFC $y</A>" >> index.html
  if [ -e $x.gz ]; then
    echo "<A HREF=\"$x.gz\">(gzipped)</A>" >> index.html
  fi
  y="attributes-rfc$y.html";
  if [ -f $y ];then
    echo "<A HREF=\"$y\">(attributes)</A>" >> index.html
  fi
  echo "<BR />" >> index.html
done

cat >>index.html <<EOF
<h2>Other files</h2>
EOF

#
#  for all of the text files, do this
#
for x in *.txt;do
  y=`echo $x | sed ';s/\.txt/.html/'`
  if [ ! -f $y ];then
    echo "<A HREF=\"$x\">$x</A>" >> index.html
    if [ -e $x.gz ]; then
      echo "<A HREF=\"$x.gz\">(gzipped)</A>" >> index.html
    fi
    echo "<BR />" >> index.html
  fi
done
echo "</BODY></HTML>" >> index.html
