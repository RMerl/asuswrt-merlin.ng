#!/bin/sh

infile=$1
outfile=${infile}.out.c
function_name=`echo "${infile%.*}"`   #get prefix
echo "input file name: $infile"
echo "output file name: $outfile"
echo "function name: $function_name"
if [ "$1" = "" ]; then
echo "ERR: please specify input file name"
exit 1
fi

echo "Converting ..."
rm -f $outfile
touch $outfile
echo "void ${function_name}(void)" >> $outfile
echo "{" >> $outfile
while read LINE
do
op=`echo $LINE | awk -F " " '{print $1}'`
if [ "$op" = "d.out" ]; then
  param=`echo $LINE | awk -F " " '{print $2 ", " $4}'`
  if [[ "$param" == "0x8014405c, 0x00000000" ]]; then
     echo "    udelay(1);" >> $outfile
  fi
  echo "    WAN_TOP_WRITE_32($param);" >> $outfile
fi
done < $infile
echo "}" >> $outfile
echo "Convert Done"
