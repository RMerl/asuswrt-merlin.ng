#! /bin/sh

if [ -z "$1" ] || [ -z "$2" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]; then
	echo ""
	echo "sotp_write_key.sh <key file containing 4-byte aligned ascii hex key> <SOTP keyslot to write to> [ flags ]"
	echo "flags:"
	echo "	-f, Force commit. If omitted then the formatted sotpctl command is ONLY printed to the screen"
	echo "	-h, print help text."
	return 0
fi

# Check for key file
if [ ! -f "$1" ]; then
	echo "Error: Key file $1 doesnt exist!\n"
	return 1;
fi

# Determine key size
file_size=`stat -t $1 | cut -d ' ' -f 2`
newline=`tail -c 1 $1`
if [ -z $newline ]; then
	key_size_chars=`expr $file_size - 1`	
else
	key_size_chars=$file_size
fi

# Check if key size is multiples of 4 bytes
key_size_extra_chars=`expr $key_size_chars % 8`
if [ $key_size_extra_chars -ne 0 ]; then
	echo "Error: Key size must be 4-byte(word) aligned!\n"
	return 1;
fi

# Get key byte count
key_size_words=`expr $key_size_chars / 8`
key_size_bytes=`expr $key_size_words \* 4`

# Parse key
key_byte_string=`cat $1`
byte_num=0
while [ $byte_num -lt $key_size_bytes ]
do 
	j=`expr 2 \* $byte_num + 1`
	k=`expr 2 \* $byte_num + 2`
	byte=`echo "$key_byte_string" | cut -b $j-$k`
	key_le_word="$byte$key_le_word"
	numwordbytes=`expr $byte_num % 4`
	if [ "$numwordbytes" == "3" ]; then
		key_data_words="${key_data_words}0x${key_le_word} "
		key_le_word=""
	fi
	byte_num=`expr $byte_num + 1`	
done

# Generate sotpctl command
sotpctl_cmd="sotpctl set keyslot $2 $key_data_words"

# Commit the key or do dry run
if [ "$3" == "-f" ]; then
	echo ""
	echo "### Commit run ###"
	echo "Committing $1 to SOTP slot $2!"
	retval=`$sotpctl_cmd`	
else
	echo ""
	echo "### Dry run ( use -f flag to commit ) ###"
	echo "Key file          : $1"
	echo "Key size          : $key_size_bytes bytes"
	echo "Key words         : $key_size_words words"
	echo "Key value         : $key_byte_string"	
	echo "SOTP slot to write: $2"
	echo "sotpctl command   :"
	echo "    $sotpctl_cmd"
fi


