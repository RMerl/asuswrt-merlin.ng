#!/usr/bin/perl

use strict;
use warnings;

my $found = 0;
sub msg {
  $found = 1;
  print "$_[0]";
}

my $C = 0;

if ($ARGV[0] =~ /^-/) {
    my $lang = shift @ARGV;
    $C = ($lang eq '-C');
}

our %basenames = ();

our %guardnames = ();

for my $fn (@ARGV) {
    open(F, "$fn");
    my $lastnil = 0;
    my $lastline = "";
    my $incomment = 0;
    my $in_func_head = 0;
    my $basename = $fn;
    $basename =~ s#.*/##;
    if ($basenames{$basename}) {
        msg "Duplicate fnames: $fn and $basenames{$basename}.\n";
    } else {
        $basenames{$basename} = $fn;
    }
    my $isheader = ($fn =~ /\.h/);
    my $seenguard = 0;
    my $guardname = "<none>";

    while (<F>) {
        ## Warn about windows-style newlines.
        #    (We insist on lines that end with a single LF character, not
        #    CR LF.)
        if (/\r/) {
            msg "       CR:$fn:$.\n";
        }
        ## Warn about tabs.
        #    (We only use spaces)
        if (/\t/) {
            msg "      TAB:$fn:$.\n";
        }
        ## Warn about labels that don't have a space in front of them
        #    (We indent every label at least one space)
        if (/^[a-zA-Z_][a-zA-Z_0-9]*:/) {
            msg "nosplabel:$fn:$.\n";
        }
        ## Warn about trailing whitespace.
        #    (We don't allow whitespace at the end of the line; make your
        #    editor highlight it for you so you can stop adding it in.)
        if (/ +$/) {
            msg "Space\@EOL:$fn:$.\n";
        }
        ## Warn about control keywords without following space.
        #    (We put a space after every 'if', 'while', 'for', 'switch', etc)
        if ($C && /\s(?:if|while|for|switch)\(/) {
            msg "      KW(:$fn:$.\n";
        }
        ## Warn about #else #if instead of #elif.
        #    (We only allow #elif)
        if (($lastline =~ /^\# *else/) and ($_ =~ /^\# *if/)) {
            msg " #else#if:$fn:$.\n";
        }
        ## Warn about some K&R violations
        #    (We use K&R-style C, where open braces go on the same line as
        #    the statement that introduces them.  In other words:
        #          if (a) {
        #            stuff;
        #          } else {
        #            other stuff;
        #          }
        if (/^\s+\{/ and $lastline =~ /^\s*(if|while|for|else if)/ and
            $lastline !~ /\{$/) {
            msg "non-K&R {:$fn:$.\n";
        }
        if (/^\s*else/ and $lastline =~ /\}$/) {
            msg "  }\\nelse:$fn:$.\n";
        }
        $lastline = $_;
        ## Warn about unnecessary empty lines.
        #   (Don't put an empty line before a line that contains nothing
        #   but a closing brace.)
        if ($lastnil && /^\s*}\n/) {
            msg "  UnnecNL:$fn:$.\n";
        }
        ## Warn about multiple empty lines.
        #   (At most one blank line in a row.)
        if ($lastnil && /^$/) {
            msg " DoubleNL:$fn:$.\n";
        } elsif (/^$/) {
            $lastnil = 1;
        } else {
            $lastnil = 0;
        }
        ## Terminals are still 80 columns wide in my world.  I refuse to
        ## accept double-line lines.
        #   (Don't make lines wider than 80 characters, including newline.)
        if (/^.{80}/) {
            msg "     Wide:$fn:$.\n";
        }
        ### Juju to skip over comments and strings, since the tests
        ### we're about to do are okay there.
        if ($C) {
            if ($incomment) {
                if (m!\*/!) {
                    s!.*?\*/!!;
                    $incomment = 0;
                } else {
                    next;
                }
            }

            if ($isheader) {
                if ($seenguard == 0) {
                    if (/ifndef\s+(\S+)/) {
                        ++$seenguard;
                        $guardname = $1;
                    }
                } elsif ($seenguard == 1) {
                    if (/^\#define (\S+)/) {
                        ++$seenguard;
                        if ($1 ne $guardname) {
                            msg "GUARD:$fn:$.: Header guard macro mismatch.\n";
                        }
                    }
                }
            }

            if (m!/\*.*?\*/!) {
                s!\s*/\*.*?\*/!!;
            } elsif (m!/\*!) {
                s!\s*/\*!!;
                $incomment = 1;
                next;
            }
            s!"(?:[^\"]+|\\.)*"!"X"!g;
            next if /^\#/;
            ## Warn about C++-style comments.
            #   (Use C style comments only.)
            if (m!//!) {
                #    msg "       //:$fn:$.\n";
                s!//.*!!;
            }
            ## Warn about unquoted braces preceded by non-space.
            #   (No character except a space should come before a {)
            if (/([^\s'])\{/) {
                msg "       $1\{:$fn:$.\n";
            }
            ## Warn about double semi-colons at the end of a line.
            if (/;;$/) {
                msg "       double semi-colons at the end of $. in $fn\n"
            }
            ## Warn about multiple internal spaces.
            #if (/[^\s,:]\s{2,}[^\s\\=]/) {
            #    msg "     X  X:$fn:$.\n";
            #}
            ## Warn about { with stuff after.
            #s/\s+$//;
            #if (/\{[^\}\\]+$/) {
            #    msg "     {X:$fn:$.\n";
            #}
            ## Warn about function calls with space before parens.
            #   (Don't put a space between the name of a function and its
            #   arguments.)
            if (/(\w+)\s\(([A-Z]*)/) {
                if ($1 ne "if" and $1 ne "while" and $1 ne "for" and
                    $1 ne "switch" and $1 ne "return" and $1 ne "int" and
                    $1 ne "elsif" and $1 ne "WINAPI" and $2 ne "WINAPI" and
                    $1 ne "void" and $1 ne "__attribute__" and $1 ne "op" and
                    $1 ne "size_t" and $1 ne "double" and $1 ne "uint64_t" and
                    $1 ne "workqueue_reply_t" and $1 ne "bool") {
                    msg "     fn ():$fn:$.\n";
                }
            }
            ## Warn about functions not declared at start of line.
            #    (When you're declaring functions, put "static" and "const"
            #    and the return type on one line, and the function name at
            #    the start of a new line.)
            if ($in_func_head ||
                ($fn !~ /\.h$/ && /^[a-zA-Z0-9_]/ &&
                 ! /^(?:const |static )*(?:typedef|struct|union)[^\(]*$/ &&
                 ! /= *\{$/ && ! /;$/)) {
                if (/.\{$/){
                    msg "fn() {:$fn:$.\n";
                    $in_func_head = 0;
                } elsif (/^\S[^\(]* +\**[a-zA-Z0-9_]+\(/) {
                    $in_func_head = -1; # started with tp fn
                } elsif (/;$/) {
                    $in_func_head = 0;
                } elsif (/\{/) {
                    if ($in_func_head == -1) {
                        msg "tp fn():$fn:$.\n";
                    }
                    $in_func_head = 0;
                }
            }

            ## Check for forbidden functions except when they are
            # explicitly permitted
            if (/\bassert\(/ && not /assert OK/) {
                msg "assert :$fn:$.   (use tor_assert)\n";
            }
            if (/\bmemcmp\(/ && not /memcmp OK/) {
                msg "memcmp :$fn:$.   (use {tor,fast}_mem{eq,neq,cmp}\n";
            }
            # always forbidden.
            if (not /\ OVERRIDE\ /) {
                if (/\bstrcat\(/ or /\bstrcpy\(/ or /\bsprintf\(/) {
                    msg "$& :$fn:$.\n";
                }
                if (/\bmalloc\(/ or /\bfree\(/ or /\brealloc\(/ or
                    /\bstrdup\(/ or /\bstrndup\(/ or /\bcalloc\(/) {
                    msg "$& :$fn:$.    (use tor_malloc, tor_free, etc)\n";
                }
            }
        }
    }
    if ($isheader && $C) {
        if ($seenguard < 2) {
            msg "$fn:No #ifndef/#define header guard pair found.\n";
        } elsif ($guardnames{$guardname}) {
            msg "$fn:Guard macro $guardname also used in $guardnames{$guardname}\n";
        } else {
            $guardnames{$guardname} = $fn;
        }
    }
    close(F);
}

exit $found;
