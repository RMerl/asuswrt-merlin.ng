#!/bin/bash

: ${BIN_PATH=./}
: ${PORT=12340}
: ${HOME_PORT=12350}
: ${SECRET=testing123}

rm -f verbose.log
RCODE=0

rm -rf .cache
mkdir .cache

#
#  Bootstrap the tests
#
for NAME in $@
do
  TOTAL=`grep TESTS $NAME | sed 's/.*TESTS//'`

  #
  #  Each test may have multiple variants.
  #
  for NUMBER in `echo $TOTAL`
  do
    cp $NAME .request
    BASE=`echo $NAME | sed 's,.*/,,'`

    #
    #  Add the name of the test, and the variant to the request
    #
    echo "Test-Name = \"$BASE\"," >> .request
    echo 'Test-Number = ' $NUMBER >> .request

    mv .request .cache/$BASE:$NUMBER
  done
done

echo "Running tests..."

(cd .cache;ls -1  > ../.foo)
rm -f .bar
for x in `cat .foo`
do
   echo "-f .cache/$x" >> .bar
done

$BIN_PATH/radclient `cat .bar` -xFd . 127.0.0.1:$PORT auth $SECRET > radclient.log 2>&1
if [ "$?" != "0" ]; then
  echo "Failed running $BIN_PATH/radclient"
  exit 1
fi

for x in `cat .foo`
do
  RESULT=`egrep ^\\.cache/$x radclient.log | sed 's/.* //'`
  if [ "$RESULT" = "2" ]; then
      echo "$x : Success"
    else
      echo "$x : FAILED"
      RCODE=1
  fi
done


if [ "$RCODE" = "0" ]
then
    rm -f radiusd.log radclient.log 
    echo "All tests succeeded"
else
    echo "See radclient.log for more details"
fi

exit $RCODE
