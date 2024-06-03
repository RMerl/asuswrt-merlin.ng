# $(var) --> ${var}
s/$(\([a-zA-Z0-9_]*\))/${\1}/g
# a+=b --> set(a ${a} b)
s/^\([[:blank:]]*\)\([a-zA-Z0-9_]*\)[[:blank:]]*+=[[:blank:]]*\(.*\)[[:blank:]]*/\1\set(\2 ${\2} \3)/
# a:=b --> set(a b)
s/^\([[:blank:]]*\)\([a-zA-Z0-9_]*\)[[:blank:]]*:=[[:blank:]]*\(.*\)[[:blank:]]*/\1\set(\2 \3)/
# a=b --> set(a b)
s/^\([[:blank:]]*\)\([a-zA-Z0-9_{}\$\-]*\)[[:blank:]]*=[[:blank:]]*\(.*\)[[:blank:]]*/\1\set(\2 \3)/
# export a=b --> set(ENV{a} b)
s/^\([[:blank:]]*\)export[[:blank:]]*\([a-zA-Z0-9_]*\)[[:blank:]]*=[[:blank:]]*\(.*\)[[:blank:]]*/\1\set(ENV{\2} \3)/
# else --> else()
s/\([[:blank:]]*\)else[[:blank:]]*/\1\else()/
# endif --> endif()
s/\([[:blank:]]*\)endif[[:blank:]]*/\1\endif()/
# ifeq($(strip ${var}), value) --> if ("${var)" MATCHES " *value *")
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*$(strip ${\(.*\)})[[:blank:]]*,[[:blank:]]*\(.*\)[[:blank:]]*)/\1\if("${\2}" MATCHES " *\3 *")/
# ifeq("$(strip ${var})", "value") --> if ("${var)" MATCHES " *value *")
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*"$(strip ${\(.*\)})"[[:blank:]]*,[[:blank:]]*"\(.*\)"[[:blank:]]*)/\1\if("${\2}" MATCHES " *\3 *")/
# ifneq($(strip ${var}), value) --> if (NOT "${var)" MATCHES " *value *")
s/\([[:blank:]]*\)ifneq[[:blank:]]*([[:blank:]]*$(strip ${\(.*\)})[[:blank:]]*,[[:blank:]]*\(.*\)[[:blank:]]*)/\1\if(NOT "${\2}" MATCHES " *\3 *")/
# ifneq("$(strip ${var})", "value") --> if (NOT "${var)" MATCHES " *value *")
s/\([[:blank:]]*\)ifneq[[:blank:]]*([[:blank:]]*"$(strip ${\(.*\)})"[[:blank:]]*,[[:blank:]]*"\(.*\)"[[:blank:]]*)/\1\if(NOT "${\2}" MATCHES " *\3 *")/
# ifeq("${var}", "y") --> if (var)
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*"${\(.*\)}"[[:blank:]]*,[[:blank:]]*"y"[[:blank:]]*)/\1\if(\2)/
# ifeq("${var}", "n") --> if (NOT var)
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*"${\(.*\)}"[[:blank:]]*,[[:blank:]]*"n"[[:blank:]]*)/\1\if(NOT \2)/
# ifeq("${var}", "value") --> if ("${var}" STREQUAL "value")
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*"\(.*\)"[[:blank:]]*,[[:blank:]]*"\(.*\)"[[:blank:]]*)/\1\if("\2" STREQUAL "\3")/
# ifeq(${var}, value) --> if ("${var}" STREQUAL "value")
s/\([[:blank:]]*\)ifeq[[:blank:]]*([[:blank:]]*\(.*\)[[:blank:]]*,[[:blank:]]*\(.*\)[[:blank:]]*)/\1\if("\2" STREQUAL "\3")/
# ifneq("${var}", "value") --> if (NOT "${var}" STREQUAL "value")
s/\([[:blank:]]*\)ifneq[[:blank:]]*([[:blank:]]*"\(.*\)"[[:blank:]]*,[[:blank:]]*"\(.*\)"[[:blank:]]*)/\1\if(NOT "\2" STREQUAL "\3")/
# ifneq(${var}, value) --> if (NOT "${var}" STREQUAL "value")
s/\([[:blank:]]*\)ifneq[[:blank:]]*([[:blank:]]*\(.*\)[[:blank:]]*,[[:blank:]]*\(.*\)[[:blank:]]*)/\1\if(NOT "\2" STREQUAL "\3")/
# ifdef var --> if (DEFINED var)
s/\([[:blank:]]*\)ifdef[[:blank:]]*\(.*\)[[:blank:]]*/\1\if(DEFINED \2)/
# ifndef var --> if (NOT DEFINED var)
s/\([[:blank:]]*\)ifndef[[:blank:]]*\(.*\)[[:blank:]]*/\1\if(NOT DEFINED \2)/
# $(error xx --> message(FATAL_ERROR xx)
s/\([[:blank:]]*\)$(error \(.*\)[[:blank:]]*/\1\message(FATAL_ERROR "\2")/



