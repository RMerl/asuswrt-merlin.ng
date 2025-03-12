#!/bin/bash


##################################################
# toolbox cleanup:                               #
#                                                #
##################################################

#
# packages folder
#

# remove pyRPIO MDIO library (symbolic links, folder) 
rm -rf $(pwd)/packages/PyRPIO
rm -rf $(pwd)/packages/PyRPIO/pyrpio 
rm -rf $(pwd)/packages/PyRPIO-0.4.1 

# remove gphy_hostapi symbolic link
rm -rf $(pwd)/packages/gphy_hostapi

# reset the host_adapt and host_smdio_ssb symbolic link
rm -rf ../../switch_hostapi/src/host_adapt.c
rm -rf ../../switch_hostapi/src/host_adapt.h 
rm -rf ../../switch_hostapi/src/host_smdio_ssb.c

# remove switch_hostapi symbolic link
rm -rf $(pwd)/packages/switch_hostapi

# remove mdio and driver symbolic links
rm -rf $(pwd)/src/lif/mdio

# check directory 
ls -l $(pwd)/packages
ls -l $(pwd)/src/lif

# remove build directories
rm -rf $(pwd)/build
rm -rf $(pwd)/python/cbindings/cython/build
rm -rf $(pwd)/python/cbindings/src
rm -rf $(pwd)/python/cbindings/cython/*.so

#remove CMake symbolic link
rm -rf $(pwd)/CMakeLists.txt