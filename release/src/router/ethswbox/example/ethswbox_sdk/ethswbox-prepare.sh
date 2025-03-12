#!/bin/bash
MODE=$1
if [ "$MODE" = "user" ]; then
	ln -sf CMakeLists.txt.user CMakeLists.txt
elif [ "$MODE" = "rpi4evk" ]; then
	ln -sf CMakeLists.txt.rpi4evk CMakeLists.txt
else
	echo "Error: Please assign '. ethswbox-prepare.sh user' or '. ethswbox-prepare.sh rpi4evk' mode."
	exit 1
fi
# update symbolic links in packages folder
cd packages

if [ "$MODE" = "user" ]; then
	ln -sf -T ../../../switch_hostapi/ switch_hostapi
	cd switch_hostapi/src
	ln -sf host_adapt_user.c host_adapt.c
	ln -sf host_adapt_user.h host_adapt.h
	ln -sf host_smdio_ssb_user.c host_smdio_ssb.c
elif [ "$MODE" = "rpi4evk" ]; then
	chmod +x PyRPIO-0.4.1.tar.gz
	tar xvfz PyRPIO-0.4.1.tar.gz
	ln -sf  PyRPIO-0.4.1/ PyRPIO

	ln -sf -T ../../../switch_hostapi/ switch_hostapi
	cd switch_hostapi/src
	ln -sf host_adapt_rpi4evk.c host_adapt.c
	ln -sf host_adapt_rpi4evk.h host_adapt.h
	ln -sf host_smdio_ssb_rpi4evk.c host_smdio_ssb.c
fi

cd ../../..

#
# update symbolic link in src/lif folder
#
cd src/lif
ln -sf ../../packages/PyRPIO/pyrpio mdio
cd ../../

#
# list the symbolic links
#

ls -l $(pwd)/packages
ls -l $(pwd)/src/lif
