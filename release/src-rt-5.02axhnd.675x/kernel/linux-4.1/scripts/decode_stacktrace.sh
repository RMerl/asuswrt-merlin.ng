#!/bin/bash
# (c) 2014, Sasha Levin <sasha.levin@oracle.com>
#set -x

if [[ $# != 2 ]]; then
	echo "Usage:"
	echo "	$0 [vmlinux] [base path]"
	exit 1
fi

vmlinux=$1
basepath=$2
declare -A cache

parse_symbol() {
	# The structure of symbol at this point is:
	#   [name]+[offset]/[total length]
	#
	# For example:
	#   do_basic_setup+0x9c/0xbf


	# Strip the symbol name so that we could look it up
	local name=${symbol%+*}

	# Use 'nm vmlinux' to figure out the base address of said symbol.
	# It's actually faster to call it every time than to load it
	# all into bash.
	if [[ "${cache[$name]+isset}" == "isset" ]]; then
		local base_addr=${cache[$name]}
	else
		local base_addr=$(nm "$vmlinux" | grep -i ' t ' | awk "/ $name\$/ {print \$1}" | head -n1)
		cache["$name"]="$base_addr"
	fi
	# Let's start doing the math to get the exact address into the
	# symbol. First, strip out the symbol total length.
	local expr=${symbol%/*}

	# Now, replace the symbol name with the base address we found
	# before.
	expr=${expr/$name/0x$base_addr}

	# Evaluate it to find the actual address
	expr=$((expr))
	local address=$(printf "%x\n" "$expr")

	# Pass it to addr2line to get filename and line number
        # Could get more than one result
	if [[ "${cache[$address]+isset}" == "isset" ]]; then
		local code=${cache[$address]}
	else
		local code=$(addr2line -i -e "$vmlinux" "$address")
		cache[$address]=$code
	fi

	# addr2line doesn't return a proper error code if it fails, so
	# we detect it using the value it prints so that we could preserve
	# the offset/size into the function and bail out
	if [[ $code == "??:0" ]]; then
		return
	fi

	# Strip out the base of the path
	code=${code//$basepath/""}

	# In the case of inlines, move everything to same line
	code=${code//$'\n'/' '}

	# Replace old address with pretty line numbers
	symbol="$name ($code)"
}

decode_code() {
	local scripts=`dirname "${BASH_SOURCE[0]}"`

	echo "$1" | $scripts/decodecode
}

handle_line() {
	local words

	# Tokenize
	read -a words <<<"$1"

	# Remove hex numbers. Do it ourselves until it happens in the
	# kernel

	# We need to know the index of the last element before we
	# remove elements because arrays are sparse
	local last=$(( ${#words[@]} - 1 ))

	for i in "${!words[@]}"; do
		# Remove the address
		if [[ ${words[$i]} =~ \[\<([^]]+)\>\] ]]; then
			unset words[$i]
		fi

		# Format timestamps with tabs
		if [[ ${words[$i]} == \[ && ${words[$i+1]} == *\] ]]; then
			unset words[$i]
			words[$i+1]=$(printf "[%13s\n" "${words[$i+1]}")
		fi
	done

	# The symbol is the last element, process it
	symbol=${words[$last]}
	unset words[$last]
	parse_symbol # modifies $symbol

	# Add up the line number to the symbol
	echo "${words[@]}" "$symbol"
}

while read line; do
	# Let's see if we have an address in the line
	if [[ $line =~ \[\<([^]]+)\>\]  ]]; then
		# Translate address to line numbers
		handle_line "$line"
	# Is it a code line?
	elif [[ $line == *Code:* ]]; then
                decode_code "$line"
        else
		# Nothing special in this line, show it as is
		echo "$line"
	fi
done
