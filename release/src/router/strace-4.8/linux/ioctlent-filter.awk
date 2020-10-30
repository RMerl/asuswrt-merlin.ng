#!/bin/awk -f
# Filter out redundant "*32" symbols.

BEGIN {
	s=""
	c=""
}

NF == 3 && $2 ~ /^"[^",]*",$/ {
	if ($2 == s && $3 == c)
		next
	s = $2
	sub(/",$/, "32\",", s)
	c = $3
}

{
	print
}
