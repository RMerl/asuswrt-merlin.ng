cd bootloaders/build/configs

for file in options_*.conf;
do
	for x in `cat ${file}|grep = |sed -e 's/:=.*//' -e 's/=.*//' -e 's/ *//' |sort |uniq |grep -v -- ^#|grep -v =`;
	do
		grep $x options_help.txt 2>&1 > /dev/null;
		if  [ "$?" == "1" ]; then
			echo [$x] is used in ${file} but missing from options_help.txt ;
		fi;
	done;
done

