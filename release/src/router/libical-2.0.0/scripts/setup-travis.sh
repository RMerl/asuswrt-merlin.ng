#/bin/sh -f

# things to do for travis-ci in the before_install section

if ( test "`uname -s`" = "Darwin" )
then
  #cmake v2.8.12 is installed on the Mac workers now
  #brew update
  #brew install cmake
  brew install icu4c
  brew install db
else
  #install a newer cmake since at this time Travis only has version 2.8.7
  echo "yes" | sudo add-apt-repository ppa:kalakris/cmake
  sudo apt-get update -qq
  sudo apt-get install cmake
  sudo apt-get install libicu-dev
  sudo apt-get install libdb4.8-dev
fi
