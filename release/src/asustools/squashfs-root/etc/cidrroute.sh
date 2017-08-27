rm -rf /tmp/tvrouting
# Configure static routes
if [[ "${cidrroute}" ]]; then
    cidrroute="${cidrroute} "
    # Work while cidrroute not empty
    while true; do
        # First go number of bits in mask
        bits=${cidrroute%% *}
        [[ -z ${bits} ]] && break
        cidrroute=${cidrroute#* }
        pref=""
        if [[ ${bits} == 0 ]]; then
            # We got 0/0 here
            pref="default"
        else
            # Calc how many bytes we need to get from cidrroute
            # And get them in prefix
            bytes=$(( (${bits} - 1) / 8 + 1 ))
            while [[ ${bytes} != 0 ]]; do
                byte=${cidrroute%% *}
                cidrroute=${cidrroute#* }
                pref="${pref}.${byte}"
                bytes=$(( ${bytes} - 1 ))
            done
            pref=${pref#.}
            pref="${pref}/${bits}"
        fi
        # We have subnet prefix now and need to get gateway same way
        bytes=4
        gw=""
        while [[ ${bytes} != 0 ]]; do
            byte=${cidrroute%% *}
            cidrroute=${cidrroute#* }
            gw="${gw}.${byte}"
            bytes=$(( ${bytes} - 1 ))
        done
        gw=${gw#.}
        # And add the next route finally
        ip route replace ${pref} via ${gw} dev ${interface} 2>/dev/null
        echo "ip route replace ${pref} via ${gw} dev ${interface}" >> /tmp/tvrouting
     done
fi

