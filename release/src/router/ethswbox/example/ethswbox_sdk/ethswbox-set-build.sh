#!/bin/bash

##################################################
# ethswbox toolbox application:                  #   
# configuration, build, symbolic links creation  #
##################################################


# configuration 
rm -rf build
mkdir build
cd build
cmake ..

# build 
make 

# symbolic links creation
./ethswbox

# back home
cd -
