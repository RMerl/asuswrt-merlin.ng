JSON_SCRIPT=tests.json
JSON_SCRIPT_BIN=./json_script-example
FILE_STDOUT=tests.stdout
FILE_STDERR=tests.stderr
FILE_EXPECTED=tests.expected

call_json_script() {
	#export LD_PRELOAD=../libjson_script.so
	$JSON_SCRIPT_BIN "$@" "$JSON_SCRIPT" >"$FILE_STDOUT" 2>"$FILE_STDERR"
}

assertStdioEquals() {
	local expected="$1"
	local file_stdio="$2"

	echo "$expected" >"$FILE_EXPECTED"
	if [ -z "$expected" ]; then
		# we are expecting empty output, but we deliberately added a newline
		# with echo above, so adding another echo to compensate for that
		echo >>"$file_stdio"
	fi
	diff -up "$FILE_EXPECTED" "$file_stdio" >/dev/null 2>&1 || {
		cat >&2 <<EOF
|--- expecting
$expected<
|--- actual
$(cat $file_stdio)<
|--- END
EOF
		exit 1
	}
}

assertStdoutEquals() {
	assertStdioEquals "$1" "$FILE_STDOUT"
}

assertStderrEquals() {
	assertStdioEquals "$1" "$FILE_STDERR"
}

test_bad_json() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ ]
		[ ]
	]
	EOF
	call_json_script
	assertStderrEquals "load JSON data from $JSON_SCRIPT failed."
}

test_expr_eq() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "eq", "VAR", "foo" ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=foo"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=xxx"
	assertStdoutEquals "echo baz"
}

test_expr_has() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "has", "VAR" ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=foo"
	assertStdoutEquals "echo bar"
	call_json_script
	assertStdoutEquals "echo baz"
}

test_expr_regex_single() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "regex", "VAR", ".ell." ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=hello"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=.ell."
	assertStdoutEquals "echo bar"
	call_json_script
	assertStdoutEquals "echo baz"
	call_json_script "VAR="
	assertStdoutEquals "echo baz"
	call_json_script "VAR=hell"
	assertStdoutEquals "echo baz"
}

test_expr_regex_multi() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "regex", "VAR", [ ".ell.", "w.rld" ] ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=hello"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=world"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=.ell."
	assertStdoutEquals "echo bar"
	call_json_script "VAR=w.rld"
	assertStdoutEquals "echo bar"
	call_json_script
	assertStdoutEquals "echo baz"
	call_json_script "VAR="
	assertStdoutEquals "echo baz"
	call_json_script "VAR=hell"
	assertStdoutEquals "echo baz"
}

test_expr_not() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "not", [ "has", "VAR" ] ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=foo"
	assertStdoutEquals "echo baz"
	call_json_script
	assertStdoutEquals "echo bar"
}

test_expr_and() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "and", [ "eq", "EQVAR", "eqval" ],
					 [ "regex", "REGEXVAR", "regex..." ]
			],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "EQVAR=eqval" "REGEXVAR=regexval"
	assertStdoutEquals "echo bar"
	call_json_script "EQVAR=foo"
	assertStdoutEquals "echo baz"
	call_json_script "REGEXVAR=regex***"
	assertStdoutEquals "echo baz"
	call_json_script
	assertStdoutEquals "echo baz"
}

test_expr_or() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "or", [ "not", [ "eq", "EQVAR", "eqval" ] ],
					[ "regex", "REGEXVAR", [ "regexva.[0-9]", "regexva.[a-z]" ] ]
			],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "EQVAR=eqval" "REGEXVAR=regexval1"
	assertStdoutEquals "echo bar"
	call_json_script "EQVAR=neq" "REGEXVAR=sxc"
	assertStdoutEquals "echo bar"
	call_json_script "REGEXVAR=sxc"
	assertStdoutEquals "echo bar"
	call_json_script "EQVAR=foo"
	assertStdoutEquals "echo bar"
	call_json_script
	assertStdoutEquals "echo bar"
	call_json_script "EQVAR=eqval" "REGEXVAR=regexval"
	assertStdoutEquals "echo baz"
}

test_expr_isdir() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "isdir", "%VAR%" ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=/"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=$(mktemp -u)"
	assertStdoutEquals "echo baz"
	call_json_script
	assertStdoutEquals "echo baz"
}

test_cmd_case() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "case", "CASEVAR", {
			"0": [ "echo", "foo" ],
			"1": [
				[ "echo", "bar" ],
				[ "echo", "baz" ]
			],
			"%VAR%": [ "echo", "quz" ]
		} ]
	]
	EOF
	call_json_script "CASEVAR=0"
	assertStdoutEquals "echo foo"
	call_json_script "CASEVAR=1"
	assertStdoutEquals "echo bar
echo baz"
	call_json_script "CASEVAR=%VAR%"
	assertStdoutEquals "echo quz"
	call_json_script "CASEVAR="
	assertStdoutEquals ""
	call_json_script
	assertStdoutEquals ""
	call_json_script "CASEVAR=xxx" "VAR=xxx"
	assertStdoutEquals ""
}

test_cmd_if() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "if",
			[ "eq", "VAR", "foo" ],
			[ "echo", "bar" ],
			[ "echo", "baz" ]
		]
	]
	EOF
	call_json_script "VAR=foo"
	assertStdoutEquals "echo bar"
	call_json_script "VAR=xxx"
	assertStdoutEquals "echo baz"
}

test_cmd_cb() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "exec", "%VAR%", "/%VAS%%%/" ]
	]
	EOF
	call_json_script
	assertStdoutEquals "exec  /%/"
	call_json_script "VAR="
	assertStdoutEquals "exec  /%/"
	call_json_script "VAR=qux" "VAS=3"
	assertStdoutEquals "exec qux /3%/"
}

test_cmd_return() {
	cat >"$JSON_SCRIPT" <<-EOF
	[
		[ "heh", "%HEHVAR%" ],
		[ "%VAR%", "%VAR%" ],
		[ "return" ],
		[ "exec_non_reachable", "Arghhh" ]
	]
	EOF
	call_json_script "HEHVAR=dude" "VAR=ow"
	assertStdoutEquals "heh dude
%VAR% ow"
}

. ./shunit2
