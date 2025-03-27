#!/bin/sh

knvram_file=/proc/environment/knvram
knvram_apply_file=/proc/environment/knvram_apply

if [ -f $knvram_file ]; then
    knvram="`cat $knvram_file`"
fi
if [ -f $knvram_apply_file ]; then
    knvram_apply="`cat $knvram_apply_file`"
fi

if [ ! -f $knvram_apply_file ] || [ "$knvram_apply" = "0" ]; then
    exit # do nothing
fi

# knvram format: [var=val];  (delimiter ; to separate pairs)

while [ ! -z "$knvram" ]
do
    pair=${knvram%%;*} # leftmost pair

    var=${pair%=*}
    val=${pair#*=}

    echo "uboot env (nvram): $var=$val"
    nvram kset $var="$val"

    # last pair without delimiter/semicolon
    if [ "$pair" = "$knvram" ]; then
        break
    fi

    # strip out the left pair
    knvram=${knvram#*;}
done

if [ -f $knvram_file ]; then
    nvram kcommit
fi

if [ "$knvram_apply" = "once" ]; then
    # just apply knvram once (don't set knvram in next reboot), still keep knvram setting in uboot
    echo "knvram_apply=0" > /proc/nvram/set
fi

