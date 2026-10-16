:PREROUTING,INPUT
*mangle
-m socket;=;OK
-m socket --transparent --nowildcard;=;OK
-m socket --transparent --nowildcard --restore-skmark;=;OK
-m socket --transparent --restore-skmark;=;OK
-m socket --nowildcard --restore-skmark;=;OK
-m socket --restore-skmark;=;OK
