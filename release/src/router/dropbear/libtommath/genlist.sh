#!/bin/bash

export a=`find . -maxdepth 1 -type f -name '*.c' | sort | sed -e 'sE\./EE' | sed -e 's/\.c/\.o/' | xargs`
perl ./parsenames.pl OBJECTS "$a"

# ref:         $Format:%D$
# git commit:  $Format:%H$
# commit time: $Format:%ai$
