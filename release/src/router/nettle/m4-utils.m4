divert(-1)

dnl
dnl  m4 macros for gmp assembly code, shared by all CPUs. From gmp/mpn/asm-defs.m4

dnl  Copyright 1999-2006, 2011 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of either:
dnl
dnl    * the GNU Lesser General Public License as published by the Free
dnl      Software Foundation; either version 3 of the License, or (at your
dnl      option) any later version.
dnl
dnl  or
dnl
dnl    * the GNU General Public License as published by the Free Software
dnl      Foundation; either version 2 of the License, or (at your option) any
dnl      later version.
dnl
dnl  or both in parallel, as here.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
dnl  for more details.
dnl
dnl  You should have received copies of the GNU General Public License and the
dnl  GNU Lesser General Public License along with the GNU MP Library.  If not,
dnl  see https://www.gnu.org/licenses/.


dnl  These macros are designed for use with any m4 and have been used on
dnl  GNU, FreeBSD, NetBSD, OpenBSD and SysV.
dnl
dnl  GNU m4 and OpenBSD 2.7 m4 will give filenames and line numbers in error
dnl  messages.
dnl
dnl
dnl  Macros:
dnl
dnl  Most new m4 specific macros have an "m4_" prefix to emphasise they're
dnl  m4 expansions.  But new defining things like deflit() and defreg() are
dnl  named like the builtin define(), and forloop() is named following the
dnl  GNU m4 example on which it's based.
dnl
dnl  GNU m4 with the -P option uses "m4_" as a prefix for builtins, but that
dnl  option isn't going to be used, so there's no conflict or confusion.
dnl
dnl
dnl  Comments in output:
dnl
dnl  The m4 comment delimiters are left at # and \n, the normal assembler
dnl  commenting for most CPUs.  m4 passes comment text through without
dnl  expanding macros in it, which is generally a good thing since it stops
dnl  unexpected expansions and possible resultant errors.
dnl
dnl  But note that when a quoted string is being read, a # isn't special, so
dnl  apostrophes in comments in quoted strings must be avoided or they'll be
dnl  interpreted as a closing quote mark.  But when the quoted text is
dnl  re-read # will still act like a normal comment, suppressing macro
dnl  expansion.
dnl
dnl  For example,
dnl
dnl          # apostrophes in comments that're outside quotes are ok
dnl          # and using macro names like PROLOGUE is ok too
dnl          ...
dnl          ifdef(`PIC',`
dnl                  # but apostrophes aren't ok inside quotes
dnl                  #                     ^--wrong
dnl                  ...
dnl                  # though macro names like PROLOGUE are still ok
dnl                  ...
dnl          ')
dnl
dnl  If macro expansion in a comment is wanted, use `#' in the .asm (ie. a
dnl  quoted hash symbol), which will turn into # in the .s but get
dnl  expansions done on that line.  This can make the .s more readable to
dnl  humans, but it won't make a blind bit of difference to the assembler.
dnl
dnl  All the above applies, mutatis mutandis, when changecom() is used to
dnl  select @ ! ; or whatever other commenting.
dnl
dnl
dnl  Variations in m4 affecting gmp:
dnl
dnl  $# - When a macro is called as "foo" with no brackets, BSD m4 sets $#
dnl       to 1, whereas GNU or SysV m4 set it to 0.  In all cases though
dnl       "foo()" sets $# to 1.  This is worked around in various places.
dnl
dnl  len() - When "len()" is given an empty argument, BSD m4 evaluates to
dnl       nothing, whereas GNU, SysV, and the new OpenBSD, evaluate to 0.
dnl       See m4_length() below which works around this.
dnl
dnl  translit() - GNU m4 accepts character ranges like A-Z, and the new
dnl       OpenBSD m4 does under option -g, but basic BSD and SysV don't.
dnl
dnl  popdef() - in BSD and SysV m4 popdef() takes multiple arguments and
dnl       pops each, but GNU m4 only takes one argument.
dnl
dnl  push back - BSD m4 has some limits on the amount of text that can be
dnl       pushed back.  The limit is reasonably big and so long as macros
dnl       don't gratuitously duplicate big arguments it isn't a problem.
dnl       Normally an error message is given, but sometimes it just hangs.
dnl
dnl  eval() &,|,^ - GNU and SysV m4 have bitwise operators &,|,^ available,
dnl       but BSD m4 doesn't (contrary to what the man page suggests) and
dnl       instead ^ is exponentiation.
dnl
dnl  eval() ?: - The C ternary operator "?:" is available in BSD m4, but not
dnl       in SysV or GNU m4 (as of GNU m4 1.4 and betas of 1.5).
dnl
dnl  eval() -2^31 - BSD m4 has a bug where an eval() resulting in -2^31
dnl       (ie. -2147483648) gives "-(".  Using -2147483648 within an
dnl       expression is ok, it just can't be a final result.  "-(" will of
dnl       course upset parsing, with all sorts of strange effects.
dnl
dnl  eval() <<,>> - SysV m4 doesn't support shift operators in eval() (on
dnl       Solaris 7 /usr/xpg4/m4 has them but /usr/ccs/m4 doesn't).  See
dnl       m4_lshift() and m4_rshift() below for workarounds.
dnl
dnl  ifdef() - OSF 4.0 m4 considers a macro defined to a zero value `0' or
dnl       `00' etc as not defined.  See m4_ifdef below for a workaround.
dnl
dnl  m4wrap() sequence - in BSD m4, m4wrap() replaces any previous m4wrap()
dnl       string, in SysV m4 it appends to it, and in GNU m4 it prepends.
dnl       See m4wrap_prepend() below which brings uniformity to this.
dnl
dnl  m4wrap() 0xFF - old versions of BSD m4 store EOF in a C "char" under an
dnl       m4wrap() and on systems where char is unsigned by default a
dnl       spurious 0xFF is output.  This has been observed on recent Cray
dnl       Unicos Alpha, Apple MacOS X, and HPUX 11 systems.  An autoconf
dnl       test is used to check for this, see the m4wrap handling below.  It
dnl       might work to end the m4wrap string with a dnl to consume the
dnl       0xFF, but that probably induces the offending m4's to read from an
dnl       already closed "FILE *", which could be bad on a glibc style
dnl       stdio.
dnl
dnl  __file__,__line__ - GNU m4 and OpenBSD 2.7 m4 provide these, and
dnl       they're used here to make error messages more informative.  GNU m4
dnl       gives an unhelpful "NONE 0" in an m4wrap(), but that's worked
dnl       around.
dnl
dnl  __file__ quoting - OpenBSD m4, unlike GNU m4, doesn't quote the
dnl       filename in __file__, so care should be taken that no macro has
dnl       the same name as a file, or an unwanted expansion will occur when
dnl       printing an error or warning.
dnl
dnl  changecom() - BSD m4 changecom doesn't quite work like the man page
dnl       suggests, in particular "changecom" or "changecom()" doesn't
dnl       disable the comment feature, and multi-character comment sequences
dnl       don't seem to work.  If the default `#' and newline aren't
dnl       suitable it's necessary to change it to something else,
dnl       eg. changecom(;).
dnl
dnl  OpenBSD 2.6 m4 - in this m4, eval() rejects decimal constants containing
dnl       an 8 or 9, making it pretty much unusable.  The bug is confined to
dnl       version 2.6 (it's not in 2.5, and was fixed in 2.7).
dnl
dnl  SunOS /usr/bin/m4 - this m4 lacks a number of desired features,
dnl       including $# and $@, defn(), m4exit(), m4wrap(), pushdef(),
dnl       popdef().  /usr/5bin/m4 is a SysV style m4 which should always be
dnl       available, and "configure" will reject /usr/bin/m4 in favour of
dnl       /usr/5bin/m4 (if necessary).
dnl
dnl       The sparc code actually has modest m4 requirements currently and
dnl       could manage with /usr/bin/m4, but there's no reason to put our
dnl       macros through contortions when /usr/5bin/m4 is available or GNU
dnl       m4 can be installed.




dnl  --------------------------------------------------------------------------
dnl  Basic error handling things.


dnl  Usage: m4_dollarhash_1_if_noparen_p
dnl
dnl  Expand to 1 if a call "foo" gives $# set to 1 (as opposed to 0 like GNU
dnl  and SysV m4 give).

define(m4_dollarhash_1_if_noparen_test,`$#')
define(m4_dollarhash_1_if_noparen_p,
eval(m4_dollarhash_1_if_noparen_test==1))
undefine(`m4_dollarhash_1_if_noparen_test')

define(m4_error,
`errprint($@
)m4exit(1)')

dnl  Usage: m4_assert_numargs(num)
dnl
dnl  Put this unquoted on a line on its own at the start of a macro
dnl  definition to add some code to check that num many arguments get passed
dnl  to the macro.  For example,
dnl
dnl         define(foo,
dnl         m4_assert_numargs(2)
dnl         `something `$1' and `$2' blah blah')
dnl
dnl  Then a call like foo(one,two,three) will provoke an error like
dnl
dnl         file:10: foo expected 2 arguments, got 3 arguments
dnl
dnl  Here are some calls and how many arguments they're interpreted as passing.
dnl
dnl         foo(abc,def)  2
dnl         foo(xyz)      1
dnl         foo()         0
dnl         foo          -1
dnl
dnl  The -1 for no parentheses at all means a macro that's meant to be used
dnl  that way can be checked with m4_assert_numargs(-1).  For example,
dnl
dnl         define(SPECIAL_SUFFIX,
dnl         m4_assert_numargs(-1)
dnl         `ifdef(`FOO',`_foo',`_bar')')
dnl
dnl  But as an alternative see also deflit() below where parenthesized
dnl  expressions following a macro are passed through to the output.
dnl
dnl  Note that in BSD m4 there's no way to differentiate calls "foo" and
dnl  "foo()", so in BSD m4 the distinction between the two isn't enforced.
dnl  (In GNU and SysV m4 it can be checked, and is.)


dnl  m4_assert_numargs is able to check its own arguments by calling
dnl  assert_numargs_internal directly.
dnl
dnl  m4_doublequote($`'0) expands to ``$0'', whereas ``$`'0'' would expand
dnl  to `$`'0' and do the wrong thing, and likewise for $1.  The same is
dnl  done in other assert macros.
dnl
dnl  $`#' leaves $# in the new macro being defined, and stops # being
dnl  interpreted as a comment character.
dnl
dnl  `dnl ' means an explicit dnl isn't necessary when m4_assert_numargs is
dnl  used.  The space means that if there is a dnl it'll still work.

dnl  Usage: m4_doublequote(x) expands to ``x''
define(m4_doublequote,
`m4_assert_numargs_internal(`$0',1,$#,len(`$1'))``$1''')

define(m4_assert_numargs,
`m4_assert_numargs_internal(`$0',1,$#,len(`$1'))dnl
`m4_assert_numargs_internal'(m4_doublequote($`'0),$1,$`#',`len'(m4_doublequote($`'1)))`dnl '')

dnl  Called: m4_assert_numargs_internal(`macroname',wantargs,$#,len(`$1'))
define(m4_assert_numargs_internal,
`m4_assert_numargs_internal_check(`$1',`$2',m4_numargs_count(`$3',`$4'))')

dnl  Called: m4_assert_numargs_internal_check(`macroname',wantargs,gotargs)
dnl
dnl  If m4_dollarhash_1_if_noparen_p (BSD m4) then gotargs can be 0 when it
dnl  should be -1.  If wantargs is -1 but gotargs is 0 and the two can't be
dnl  distinguished then it's allowed to pass.
dnl
define(m4_assert_numargs_internal_check,
`ifelse(eval($2 == $3
             || ($2==-1 && $3==0 && m4_dollarhash_1_if_noparen_p)),0,
`m4_error(`$1 expected 'm4_Narguments(`$2')`, got 'm4_Narguments(`$3')
)')')

dnl  Called: m4_numargs_count($#,len(`$1'))
dnl  If $#==0 then -1 args, if $#==1 but len(`$1')==0 then 0 args, otherwise
dnl  $# args.
define(m4_numargs_count,
`ifelse($1,0, -1,
`ifelse(eval($1==1 && $2-0==0),1, 0, $1)')')

dnl  Usage: m4_Narguments(N)
dnl  "$1 argument" or "$1 arguments" with the plural according to $1.
define(m4_Narguments,
`$1 argument`'ifelse(`$1',1,,s)')


dnl  --------------------------------------------------------------------------
dnl  Additional error checking things.


dnl  Usage: m4_assert_onearg()
dnl
dnl  Put this, unquoted, at the start of a macro definition to add some code
dnl  to check that one argument is passed to the macro, but with that
dnl  argument allowed to be empty.  For example,
dnl
dnl          define(foo,
dnl          m4_assert_onearg()
dnl          `blah blah $1 blah blah')
dnl
dnl  Calls "foo(xyz)" or "foo()" are accepted.  A call "foo(xyz,abc)" fails.
dnl  A call "foo" fails too, but BSD m4 can't detect this case (GNU and SysV
dnl  m4 can).

define(m4_assert_onearg,
m4_assert_numargs(0)
`m4_assert_onearg_internal'(m4_doublequote($`'0),$`#')`dnl ')

dnl  Called: m4_assert_onearg(`macroname',$#)
define(m4_assert_onearg_internal,
`ifelse($2,1,,
`m4_error(`$1 expected 1 argument, got 'm4_Narguments(`$2')
)')')



dnl  --------------------------------------------------------------------------
dnl  Various generic m4 things.


dnl  Usage: m4_unquote(macro)
dnl
dnl  Allow the argument text to be re-evaluated.  This is useful for "token
dnl  pasting" like m4_unquote(foo`'bar).

define(m4_unquote,
m4_assert_onearg()
`$1')


dnl  Usage: m4_length(string)
dnl
dnl  Determine the length of a string.  This is the same as len(), but
dnl  always expands to a number, working around the BSD len() which
dnl  evaluates to nothing given an empty argument.

define(m4_length,
m4_assert_onearg()
`eval(len(`$1')-0)')



dnl  Usage: m4_incr_or_decr(n,last)
dnl
dnl  Do an incr(n) or decr(n), whichever is in the direction of "last".
dnl  Both n and last must be numbers of course.

define(m4_incr_or_decr,
m4_assert_numargs(2)
`ifelse(eval($1<$2),1,incr($1),decr($1))')


dnl  Usage: forloop(i, first, last, statement)
dnl
dnl  Based on GNU m4 examples/forloop.m4, but extended.
dnl
dnl  statement is expanded repeatedly, with i successively defined as
dnl
dnl         first, first+1, ..., last-1, last
dnl
dnl  Or if first > last, then it's
dnl
dnl         first, first-1, ..., last+1, last
dnl
dnl  If first == last, then one expansion is done.
dnl
dnl  A pushdef/popdef of i is done to preserve any previous definition (or
dnl  lack of definition).  first and last are eval()ed and so can be
dnl  expressions.
dnl
dnl  forloop_first is defined to 1 on the first iteration, 0 on the rest.
dnl  forloop_last is defined to 1 on the last iteration, 0 on the others.
dnl  Nested forloops are allowed, in which case forloop_first and
dnl  forloop_last apply to the innermost loop that's open.
dnl
dnl  A simple example,
dnl
dnl         forloop(i, 1, 2*2+1, `dnl
dnl         iteration number i ... ifelse(forloop_first,1,FIRST)
dnl         ')


dnl  "i" and "statement" are carefully quoted, but "first" and "last" are
dnl  just plain numbers once eval()ed.

define(`forloop',
m4_assert_numargs(4)
`pushdef(`$1',eval(`$2'))dnl
pushdef(`forloop_first',1)dnl
pushdef(`forloop_last',0)dnl
forloop_internal(`$1',eval(`$3'),`$4')`'dnl
popdef(`forloop_first')dnl
popdef(`forloop_last')dnl
popdef(`$1')')

dnl  Called: forloop_internal(`var',last,statement)
define(`forloop_internal',
m4_assert_numargs(3)
`ifelse($1,$2,
`define(`forloop_last',1)$3',
`$3`'dnl
define(`forloop_first',0)dnl
define(`$1',m4_incr_or_decr($1,$2))dnl
forloop_internal(`$1',$2,`$3')')')

dnl  Usage: deflit(name,value)
dnl
dnl  Like define(), but "name" expands like a literal, rather than taking
dnl  arguments.  For example "name(%eax)" expands to "value(%eax)".
dnl
dnl  Limitations:
dnl
dnl  $ characters in the value part must have quotes to stop them looking
dnl  like macro parameters.  For example, deflit(reg,`123+$`'4+567').  See
dnl  defreg() below for handling simple register definitions like $7 etc.
dnl
dnl  "name()" is turned into "name", unfortunately.  In GNU and SysV m4 an
dnl  error is generated when this happens, but in BSD m4 it will happen
dnl  silently.  The problem is that in BSD m4 $# is 1 in both "name" or
dnl  "name()", so there's no way to differentiate them.  Because we want
dnl  plain "name" to turn into plain "value", we end up with "name()"
dnl  turning into plain "value" too.
dnl
dnl  "name(foo)" will lose any whitespace after commas in "foo", for example
dnl  "disp(%eax, %ecx)" would become "128(%eax,%ecx)".
dnl
dnl  These parentheses oddities shouldn't matter in assembler text, but if
dnl  they do the suggested workaround is to write "name ()" or "name (foo)"
dnl  to stop the parentheses looking like a macro argument list.  If a space
dnl  isn't acceptable in the output, then write "name`'()" or "name`'(foo)".
dnl  The `' is stripped when read, but again stops the parentheses looking
dnl  like parameters.

dnl  Quoting for deflit_emptyargcheck is similar to m4_assert_numargs.  The
dnl  stuff in the ifelse gives a $#, $1 and $@ evaluated in the new macro
dnl  created, not in deflit.
define(deflit,
m4_assert_numargs(2)
`define(`$1',
`deflit_emptyargcheck'(``$1'',$`#',m4_doublequote($`'1))`dnl
$2`'dnl
ifelse(eval($'`#>1 || m4_length('m4_doublequote($`'1)`)!=0),1,($'`@))')')

dnl  Called: deflit_emptyargcheck(macroname,$#,`$1')
define(deflit_emptyargcheck,
`ifelse(eval($2==1 && !m4_dollarhash_1_if_noparen_p && m4_length(`$3')==0),1,
`m4_error(`dont use a deflit as $1() because it loses the brackets (see deflit in asm-defs.m4 for more information)
')')')


divert`'dnl
