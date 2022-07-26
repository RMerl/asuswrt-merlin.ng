#!/bin/ash

numZero=0
numOne=1
fileList=$1
UsbDir=$2
option=$3
testLoops=1
testHours=0
opLoop="-l"
opTime="-t"
bOpTime=0
filesRead=0
filesWrite=0
bytesRead=0
bytesWrite=0
readError=0
writeError=0

#because ASH is only support signed 32 bit int, we need do something here
bytesReadGB=0
bytesWriteGB=0
intGB=1073741824

help_exit() {
	echo "usage: fs_stress.sh <fileList> <UsbMountPoint> <-l testLoops or -t testHours>"
    echo "example: fs_stress.sh stress-fs-flist.txt /mnt/disk1_1/ -l 10"
    exit
}

timeConvert() {
	tH=0
	tM=0
	tS=$1
	if [ $tS -ge 60 ]; then
		tM=`expr $tS / 60`
		tS=`expr $tS % 60`
		if [ $tM -ge 60 ]; then
			tH=`expr $tM / 60`
			tM=`expr $tM % 60`
		fi
	fi
	
	tT="$tH Hours $tM Minutes and $tS Seconds"
	echo $tT
	return
}

drop_page_caches() {
	echo 1 > /proc/sys/vm/drop_caches
}

test_report() {
	tEnd=$(date +%s)
	tLast=`expr $tEnd - $tStart`
	tT=$(timeConvert $tLast)
	echo "#############################"
	echo "Test Report: running for $tT"
	echo "$filesRead files ($bytesReadGB GB and $bytesRead bytes) read; $readError errors found"
	echo "$filesWrite files ($bytesWriteGB GB and $bytesWrite bytes) written; $writeError errors found"
	echo -e "#############################\n"
}

### In order to avoid enable AWK in busybox , we use this function to extract the field from a string
extract_field_n() {
#input: field#, string
#output: field to be extracted

fieldN=`expr $1 + 1`
eval fieldValue="\$$fieldN"
echo $fieldValue
}

#open the file list
if [ -z $fileList ]; then
	help_exit
fi

if [ -z $UsbDir ]; then
    help_exit
fi

if [ -z $option ]; then
    help_exit
fi

if [ $option = $opLoop ]; then

	testLoops=$4

	if [ -z $testLoops ]; then
		help_exit
	fi

    if [ $testLoops -le $numZero ]; then 
    echo "ERROR: testLoops must greater than 0"
    help_exit
    fi

	bOpTime=0
else
    if [ $option = $opTime ]; then

        testHours=$4
        testLoops=1

        if [ -z $testHours ]; then
            help_exit
        fi

        if [ $testHours -le $numZero ]; then 
            echo "ERROR: testHours must greater than 0"
            help_exit
        fi

        bOpTime=1

    else

        help_exit 

    fi

fi


if [ -e $fileList ]; then
	echo "#############################"
	echo "#############################"
    if [ $bOpTime -eq 0 ]; then
        echo "File List=\"$fileList\"; Test Loops =$testLoops"
    else
        echo "File List=\"$fileList\"; Test Hours =$testHours"
    fi
else
	echo "ERROR: file \"$fileList\" is not exist"
	help_exit
fi

#mount to USB drive
#mount /dev/sda1 /data
#check USB directory existence
bUsbMounted=0
#UsbDir="/mnt/disk1_1/"  "from input now"
if [ -d $UsbDir ]; then
#    echo "USB drive is mounted to $UsbDir"
    bUsbMounted=1
else
#    echo "Warning: No USB drive is mounted"
    echo "Warning: USB Mount Point is not exist"
    bUsbMounted=0
fi

tStart=$(date +%s)

#####################################
#	Check for files existence		#
#	and do SHA1 on files			#
#####################################
fileCount=0
sizeCount=0

drop_page_caches

while read fName; do
	if [ -e $fName ]; then
        if [ -z $fName ]; then
            echo "Warning: file list \"$fileList\" contains empty line"
        else
		  #fileSize=$(stat -c %s $fName)
		  #fileSize=$(ls -l $fName | awk '{ print $5}')
		  fileSize=$(extract_field_n 5 $(ls -l $fName))
		  echo "fileName=$fName; fileSize=$fileSize"
		  #setvar fName$fileCount $fName
		  eval fName$fileCount="\$fName"
		  #setvar sha1_prev_$fileCount "$( openssl sha1 $fName)"
		  eval sha1_prev_$fileCount="\$( openssl sha1 $fName)"
		  fileCount=`expr $fileCount + 1`
		  sizeCount=`expr $sizeCount + $fileSize`
        fi
	else
		echo "Warning: File $fName does not exist"
	fi
done < $fileList

tR=$(date +%s)
tLast=`expr $tR - $tStart`
tT=$(timeConvert $tLast)
#echo "tRead=$tT"

echo "fileCount=$fileCount; sizeCount=$sizeCount; time_reading = $tT"

filesRead=$fileCount
bytesRead=$sizeCount
echo -e "#############################\n"

#####################################
#	Debug: print sha1 of the fles	#
#####################################
echo "#############################"
echo "SHA1 of files:"
index=0
while [ $index -lt $fileCount ]
do
	eval echo \$sha1_prev_$index
	index=`expr $index + 1`
done
echo -e "#############################\n"

#####################################
#		Read/write testing Loop			#
#####################################
echo "#############################"
indexX=0
while [ $indexX -lt $testLoops ]
do
    #Read Testing
	drop_page_caches 
	tRL_S=$(date +%s)
	indexY=0
	echo -n "Read loop=$(($indexX+1))"
	while [ $indexY -lt $fileCount ]
	do
		echo -n "."
		eval fileName="\$fName$indexY"
		#echo $fileName
		eval sha1_curr="\$( openssl sha1 $fileName)"
		#echo $sha1_curr

		eval sha1_prev="\$sha1_prev_$indexY"
		#eval sha1_prev="wrong sha"
		
		###compare###
		if [ "$sha1_curr" != "$sha1_prev" ]; then
			echo "ERROR: SHA1 of $fName is wrong"
			readError=`expr $readError + 1`
		#else
		#	echo "GOOD: SHA1 of $fName is good"
		fi

		indexY=`expr $indexY + 1`
	done
	filesRead=`expr $filesRead + $fileCount`
	
	bytesRead=`expr $bytesRead + $sizeCount`
	if [ $bytesRead -gt $intGB ]; then
		bytesReadGB=`expr $bytesReadGB + 1`
		bytesRead=`expr $bytesRead - $intGB`
	fi
	
	echo -n "Done;"
	
	tRL=$(date +%s)
	tLast=`expr $tRL - $tRL_S`
	tT=$(timeConvert $tLast)
	echo " time_reading = $tT"
    #end fo read testing

    #Write testing
    if [ $bUsbMounted -eq "1" ]; then
        #copy files to USB drive
        eval dstDir=$UsbDir

        drop_page_caches 
        tWL_S=$(date +%s)
        indexY=0
        echo -n "Write loop=$(($indexX+1))"
        while [ $indexY -lt $fileCount ]
        do
            echo -n "."
            eval srcFile="\$fName$indexY"
            #echo srcFile=$srcFile
            #####generate an random dst filename###
            dstFileName=$$$(date +%s)
            #echo dstFileName=$dstFileName
            eval dstFile="$dstDir$dstFileName"
            #echo dstFile=$dstFile
            cp -f $srcFile $dstFile
            
            #eval sha1_copy="\$(openssl sha1 $dstFile | awk '{ print \$2}')"
            eval sha1_copy="\$(openssl sha1 $dstFile)"
            #echo sha1_copy=$sha1_copy
            sha1_copy=$(extract_field_n 2 $sha1_copy)
            #echo sha1_copy=$sha1_copy
            
            eval sha1_prev="\$sha1_prev_$indexY"
            #echo sha1_prev=$sha1_prev
            #eval sha1_prev=$(echo $sha1_prev | awk '{ print $2}')
            sha1_prev=$(extract_field_n 2 $sha1_prev)
            #echo sha1_prev=$sha1_prev
            #eval sha1_prev="wrong sha"
            
            ###compare###
            if [ "$sha1_copy" != "$sha1_prev" ]; then
                echo "ERROR: SHA1 of $fName is wrong"
                writeError=`expr $writeError + 1`
            #else
            #   echo "GOOD: SHA1 of $fName is good"
            fi

            rm -f $dstFile

            indexY=`expr $indexY + 1`
        done
        filesWrite=`expr $filesWrite + $fileCount`
        bytesWrite=`expr $bytesWrite + $sizeCount`
        if [ $bytesWrite -gt $intGB ]; then
            bytesWriteGB=`expr $bytesWriteGB + 1`
            bytesWrite=`expr $bytesWrite - $intGB`
        fi
        echo -n "Done;"
        

        tWL=$(date +%s)
        tLast=`expr $tWL - $tWL_S`
        tT=$(timeConvert $tLast)
        echo " time_writing = $tT"
    fi
    #end of write testing

    indexX=`expr $indexX + 1`

    if [ $bOpTime -eq $numOne ]; then
        #for time testing, we always loop till the time is reached
        
        #make it always loops
        testLoops=`expr $indexX + 1`

        tNow=$(date +%s)
        tLast=`expr $tNow - $tStart`
        tT=$(timeConvert $tLast)
        tHours=$(extract_field_n 1 $tT)
        tMinutes=$(extract_field_n 3 $tT)
        #echo "testHours: $tHours:$tMinutes"

        if [ $tHours -ge $testHours ]; then
            testLoops=$indexX
        fi

    fi

done
echo -e "#############################\n"

test_report

