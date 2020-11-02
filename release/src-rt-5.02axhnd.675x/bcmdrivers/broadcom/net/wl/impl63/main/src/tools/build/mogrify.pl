#!/usr/bin/perl
#
# Mogrification script
#
#ifdef BCMINTERNAL
#
# Usage: perl mogrify.pl [--version] \
#                        [-D<define1> -D<define2> ..] \
#                        [-U<undef1> -U<undef2> ..]   \
#                        [-break_links \
#                        [-endings=n \
#                        [-tobase <basedir>] \
#                        [-strip_comments] \
#                        [-strip_bcmromfn] \
#                        [-strip_wlmsgs "WL_<MSG1> WL_<MSG2> .... "] \
#                        [-skip_copyright_open] \
#                        [-translate_open_to_dual_copyright] \
#                        [-quiet] \
#                        [-convert2ifdef "-C<SYMBOL1> -C<SYMBOL2> .... "] \
#                        [-hide_trivial] \
#                        <file1> <file2> ...
#
#        - <file1> <file2> ... can come from standard input
#        - -break_links: break hard links when writing output file
#        - -endings=n: write files with "native" line endings
#        - -tobase: listed paths are mogrified into <basedir>/<path>
#        - (NOTE: the -tobase directory structure must already exist)
#        - -strip_comments : strips comment blocks except for copyright block
#        - -strip_bcmromfn : strips BCMROMFN(), BCMINITFN(), etc...
#        - -strip_wlmsgs   : strips WL_<MSG>() blocks
#        - -quiet          : suppress non-error messages
#        - -convert2ifdef: change #if defined to #ifdef for listed symbols.
#        - -hide_trivial: Hide trivial expressions, e.g. "#if 0 && MACRO".
#        - --version: Display the mogrification script's SVN revision number and exit.
#        NOTE: Following options are needed and used ONLY by linux DHD builds
#        NOTE: DO NOT USE IT FOR ANY OTHER PURPOSE
#        - -skip_copyright_open  : Skip replacing "open" copyright text
#        - -translate_open_to_dual_copyright  : Translate open copyright to
#                                                dual copyright
#
#endif /* BCMINTERNAL */
#
# $Copyright (C) 2004 Broadcom Corporation$
#
# $Id: mogrify.pl 768239 2018-10-08 16:52:06Z jp933255 $
# $Revision: 434756 $
#
# SVN: $HeadURL$
#

#use File::Path;
#use File::Basename;
#use FileHandle;
#use DirHandle;
#use Cwd;

$| = 1;

$debug = 0;
if ( $debug ) { print "Running Mogrify in Debug\n"; }
# Temp debug cache, will remove in upcoming rewrite
$currfile = "";

$prog            = (split '/', $0)[-1];
#@now            = localtime(time());
#$copyright_year = sprintf("%d",$now[5]+1900);
$copyright_year = (localtime)[5] + 1900;

$patcomment = '(JIRA[:\s]*[A-Z]{2}[A-Za-z0-9-]+|PR\s*[#:]?\s*\d+|FIXME|XXX|XXXLP|XXXX|work[ -]?around)';
# Define regexp for certain deletion markers without having that patter appear in this source, then precompile it.  TODO: More precompilation is needed.
$patdel = "YYDELETE" . "MEYY";
$repatdel = qr/$patdel/;
$id_del = '__ID_KEYWORD_LINE__';

$lineterm = '';

sub dprint {
    print STDERR @_ if ($verbose);
}

# If current line ends in a backslash, read and join the subsequent line, and repeat.
# Only meant to work on lines starting with #if/#ifdef/#ifndef/#elif because it
# eats whitespace after the backslash.
sub unbackslash {
    my $line = shift;
    while ($line =~ /\\$/) {
        last if ! ($_ = <INPUT>);
        chomp $line;
        $line =~ s,\s*\\$,,;
        $_ =~ s/^\s+//;
        $line = $line . ' ' . $_;
    }
    return $line;
}

sub conditional {
    my ($firstline) = @_;
    my (@input, $input, @output);
    my %nested = ();
    my $count = 0;
    my $simplified_line;

    if ($firstline =~ /^\s*\#\s*(if|ifdef|ifndef)\b/) {
        $firstline = &unbackslash($firstline);
        $simplified_line = &simplify_line($firstline);
        if ($simplified_line =~ /^\s*1\s*$/) {
            $simplified_line = "#if 1\n";
        } elsif ($simplified_line =~ /^\s*0\s*$/) {
            $simplified_line  = "#if 0\n";
        }
        push(@input, $simplified_line);
    } else {
        push(@input, $firstline);
    }

    while (<INPUT>) {
    /^\s*\#\s*(if|ifdef|ifndef)\b/ && do {
        $_ = &unbackslash($_);
        $simplified_line = &simplify_line($_);
        if ($simplified_line =~ /^\s*1\s*$/) {
            $simplified_line = "#if 1\n";
        } elsif ($simplified_line =~ /^\s*0\s*$/) {
            $simplified_line  = "#if 0\n";
        }
        $nested{$count} = &conditional($simplified_line);
        push(@input,"\$expandme[$count]\$\n");
        $count++;
        next;
    };

     /^\s*\#\s*(elif)\b/ && do {
        $simplified_line = &simplify_line($_);
        if ($simplified_line =~ /^\s*1\s*$/) {
            $simplified_line = "#elif 1\n";
        } elsif ($simplified_line =~ /^\s*0\s*$/) {
            $simplified_line  = "#elif 0\n";
        }
        push(@input, $simplified_line);
        next;
    };

     /^\s*\#\s*(else)\b/ && do {
        push(@input, $_);
        next;
    };

    /^\s*\#\s*(endif)\b/ && do {
        push(@input, &simplify_line($_));
        last;
    };

    # otherwise...
    push(@input, $_);

    } # while(<INPUT>)

    $input = join '', @input;

    while ( $input =~ /^\s*\#\s*elif\s+0\n(.|\n)*?^(\s*\#\s*)(else|elif|endif)\b.*\n/om ) {
        $input =~ s/^\s*\#\s*elif\s+0\n(.|\n)*?^(\s*\#\s*)(else|elif|endif)\b(.*\n)/$2$3$4/omg;
    }

    if ( $input =~ m/\#\s*if\s+[01]/ )
    {
        if ( $debug ) { print "Entered if 0 section on ${currfile}\n"; }
        $input =~ s/^\s*\#\s*if\s+0\n
                      (.|\n)*?
                  ^(\s*\#\s*)elif\b(.*\n)/$2if$3/xomg;
    }

    #changed for JIRA 336 - FOR ELIF
    if ($convert2ifdef) {
        foreach (@convert2ifdef_syms) {
            $input =~ s/^\s*\#\s*if\s+defined\s*\($_\)\s*$/\#ifdef $_/m;
            $input =~ s/^\s*\#\s*if\s+\!\s*defined\s*\($_\)\s*$/\#ifndef $_/m;
        }
    }

    # The \s operator includes \n when //m is used. This wastes CPU
    # on big files so drop down to \s = (\s - \n)
    if ( $input =~ m/\#\s*if\s+[01]/ )
    {
        if ( $debug ) { print "Entered if [01] section on ${currfile}\n"; }
        $input =~ s/^\s*\#\s*if\s+0\n
                        (.|\n)*?
                    ^\s*\#\s*else\b.*\n
                        ((.|\n)*?)
                    ^\s*\#\s*endif\b.*\n?/$2/xomg;

        $input =~ s/^\s*\#\s*if\s+0\n
                        (.|\n)*?
                    ^\s*\#\s*endif\b.*\n?//xomg;

        $input =~ s/^\s*\#\s*if\s+1\n
                        ((.|\n)*?)
                    ^\s*\#\s*elif\b.*\n
                        ((.|\n)*?)
                    (^\s*\#\s*else\b.*\n
                        (.|\n)*?)?
                    ^\s*\#\s*endif\b.*\n?/$1/xomg;

        $input =~ s/^\s*\#\s*if\s+1\n
                        ((.|\n)*?)
                    (^\s*\#\s*else\b.*\n
                        (.|\n)*?)?
                    ^\s*\#\s*endif\b.*\n?/$1/xomg;
    }

    # Replace "elif 1" statements with "else" statements while
    # eliminating subsequent unreachable blocks.
    if ( $input =~ m/\#\s*elif\s+1/ )
    {
        if ( $debug ) { print "Entered elif 1 section on ${currfile}\n"; }
        $input =~ s/(^\s*\#\s*if.*\n)
                        ((.|\n)*?)
                    (^\s*\#\s*elif\s+1\s*\n)
                        ((.|\n)*?)
                    (^\s*\#\s*(else|elif).*\n
                        (.|\n)*?)?
                    ^\s*\#\s*endif(.*)\n/$1$2\#else\n$5\#endif$+\n/xomg;
    }
    if ( $input =~ m/\$expandme\[\d+\]\$/ )
    {
        if ( $debug ) { print "Entered 'expandme' section on ${currfile}\n"; }
        $input =~ s/^\$expandme\[(\d+)\]\$\n/$nested{$1}/omxg;
    }

    return $input;
}


sub simplify_line {
    my($line) = @_;
    my $start = '';
    my($expr, $result, $def);
    my($poundif) = '#\s*(?:if|elif|ifdef|ifndef)\b';
    my($comment) = "(\\/\\*.*\\*\\/|\\/\\/.*)";
    my($not_comment) = "[^\\/]*[^\\*]*";
    #my($not_comment) = "[a-zA-Z_]\\w*";
    dprint "$. simplify line $line";

    if ($line =~ /^($poundif\s+)($not_comment)(\s*$comment\s*)\n/) {
        $start = $1; $expr = $2; $comment = $3 || '';
    } elsif ($line =~ /^($poundif\s+)($not_comment)\s*\n/) {
        $start = $1; $expr = $2; $comment = "";
    } elsif ($line =~ /^(\s*\#\s*endif\s+)($not_comment)(\s*${comment}\s*)\n/) {
        $start = $1; $expr = $2; $comment = $3 || '';
        dprint "$. orig comment = $comment\n";
    }

    #$expr =~ s/\s*([^\s]*)\s*/$1/;
    #print STDERR "start is \"$start\"  expr=\"$expr\"  comment=\"$comment\"\n";
    # chop($expr);

    $result = $line;
    if ($start =~ /^\s*\#\s*if\b|^\s*\#\s*elif\b/) {
        # perform substitutions within line and evaluate resulting expression
        foreach $def (@defined_macros) {
            $expr =~ s/defined\s*\(?\s*\b$def\b\s*\)?/1/g;
        }
        foreach $def (keys(%set_macros)) {
            $expr =~ s/\b$def\b/$set_macros{$def}/g;
        }
        foreach $def (@not_defined_macros) {
            $expr =~ s/(defined\s*){0,1}\(\s*\b$def\b\s*\)/0/g;
            $expr =~ s/(defined\s*){0,1}\s*\b$def\b\s*/0/g;
        }

        # Remove extra spaces from the expression.
        $expr =~ s/(defined)\s*(\()\s*(\w+)\s*(\))/$1$2$3$4/g;
        $expr =~ s/(\!)\s*(defined)\s*(\()\s*(\w+)\s*(\))/$1$2$3$4$5/g;

        # simplify the expression as much as possible
        $result = &simplify_expr($expr);


        if ($result !~ /^\s*[01]\s*$/) {
            $result = "$start$result";
            #$result = "$start$result$comment\n";
            #change for JIRA 336

            if (($result !~ m/\|\||\&\&/x) && ($convert2ifdef)) {
                foreach (@convert2ifdef_syms) {
                    $result =~ s/^\s*\#\s*if\s+defined\s*\($_\)\s*/\#ifdef $_/x;
                    $result =~ s/^\s*\#\s*if\s+\!\s*defined\s*\($_\)\s*/\#ifndef $_/x;
                }
            }
            $result = "$result$comment\n";
        }
    } elsif ($start =~ /^\s*\#\s*ifdef\b/) {
        foreach $def (@defined_macros) {
            $expr =~ s/\b$def\b/1/g;
        }
        foreach $def (keys(%set_macros)) {
            $expr =~ s/\b$def\b/$set_macros{$def}/g;
        }
        foreach $def (@not_defined_macros) {
            $expr =~ s/\b$def\b/0/g;
        }

        # simplify the expression as much as possible
        $result = &simplify_expr($expr);
        if ($result !~ /^\s*[01]\s*$/) {
            $result = "$start$result$comment\n";
        }
    } elsif ($start =~ /^\s*\#\s*ifndef\b/) {
        foreach $def (@defined_macros) {
            $expr =~ s/\b$def\b/0/g;
        }
        foreach $def (keys(%set_macros)) {
            $expr =~ s/\b$def\b/$set_macros{$def}/g;
        }
        foreach $def (@not_defined_macros) {
            $expr =~ s/\b$def\b/1/g;
        }
        # simplify the expression as much as possible
        $result = &simplify_expr($expr);
        if ($result !~ /^\s*[01]\s*$/) {
            $result = "$start$result$comment\n";
        }
    } elsif ($start =~ /^\s*\#\s*endif\b/) {
        my $new = $comment;
        # perform substitutions within comment
        foreach $def (@defined_macros) {
            $new =~ s/defined\s*\(?\s*\b$def\b\s*\)?/1/g;
        }
        foreach $def (keys(%set_macros)) {
            $new =~ s/\b$def\b/$set_macros{$def}/g;
        }
        foreach $def (@not_defined_macros) {
            $new =~ s/(defined\s*){0,1}(\(){0,1}\s*\b$def\b\s*(\)){0,1}/0/g;
        }
        dprint "$. new comment = $new\n";
        if($new ne $comment) {
            # If comment would be affected by substitutions, strip it
            $comment = "";
        }
        $result = "$start$expr$comment\n";
    }

    dprint "$. simplify line result $result";

    $result;
}

# simplify expression involving unary operator ! and binary operators ||, &&, ==, !=

sub simplify_expr {
    my($expr) = @_;

    dprint "$. simplify expr $expr\n";

    # changed for JIRA 217
    # can't simplify an expression with most arithmetic ops
    my $cannot_symplify;
    if ($hide_trivial) {
        $cannot_symplify = ($expr =~ /[\+\-\~\*\/\%\^]|[^&]&[^&]|[^\|]\|[^\|]/);
    } else {
        $cannot_symplify = ($expr =~ /[\+\-\~\*\/\%\<\>\^]|[^&]&[^&]|[^\|]\|[^\|]/);
    }
    if ($cannot_symplify) {
        dprint "$.: can't simplify $expr\n";
        return $expr;
    }

    my $macro = "\\w+\\([\\s\\w\@,]*\\)";

    #to continue fix here - add other operators
    my $atom = "[@\\w@\\>@\\<@\\=]+";
    my $term = "!?$atom";
    my $atom2 = "[@\\w]+";
    my $term2 = "!?$atom2";
    my $andexpr = "$term(\\s*&&\\s*$term)*";

    my @subst = ();
    my $subst_count = 0;

    my $expr_prev = '';

    # Keep iterating until the expression can't be further simplified
    while ($expr ne $expr_prev) {
        $expr_prev = $expr;

        # Collapse a macro of the form "word(word)" to hide the parens by replacing it
        # with an invented atom of the form "@num@".  It will be substituted back later.
        if ($expr =~ /($macro)/) {
            $subst[$subst_count] = $1;
            $expr =~ s/$macro/\@$subst_count\@/;
            dprint "$.: collapsed macro $subst_count: $expr\n";
            $subst_count++;
            next;
        }

        # inner parenthesis simplification
        my $inner_term;
        if ($hide_trivial) {
            $inner_term = $term2;
        } else {
            $inner_term = $term;
        }
        if ($expr =~ s/\(\s*($inner_term)\s*\)/$1/g) {
            dprint "$.: simplified inner parens: $expr\n";
            next;
        }

        # negation simplification
        if ($expr =~ s/!\s*0/1/g) {
            dprint "$.: simplified !0: $expr\n";
            next;
        }
        if ($expr =~ s/!\s*1/0/g) {
            dprint "$.: simplified !1: $expr\n";
            next;
        }

        # comparison simplification
        if ($expr =~ s/0\s*==\s*0/1/g) {
            dprint "$.: simplified 0==0: $expr\n";
            next;
        }
        if ($expr =~ s/0\s*==\s*1/0/g) {
            dprint "$.: simplified 0==1: $expr\n";
            next;
        }
        if ($expr =~ s/1\s*==\s*0/0/g) {
            dprint "$.: simplified 1==0: $expr\n";
            next;
        }
        if ($expr =~ s/1\s*==\s*1/1/g) {
            dprint "$.: simplified 1==1: $expr\n";
            next;
        }
        if ($expr =~ s/0\s*!=\s*0/0/g) {
            dprint "$.: simplified 0!=0: $expr\n";
            next;
        }
        if ($expr =~ s/0\s*!=\s*1/1/g) {
            dprint "$.: simplified 0!=1: $expr\n";
            next;
        }
        if ($expr =~ s/1\s*!=\s*0/1/g) {
            dprint "$.: simplified 1!=0: $expr\n";
            next;
        }
        if ($expr =~ s/1\s*!=\s*1/0/g) {
            dprint "$.: simplified 1!=1: $expr\n";
            next;
        }

        # boolean simplification
        if ($expr =~ s/\b1\s*&&\s*($term)/$1/) {
            dprint "$.: simplified 1 && X => X: $expr\n";
            next;
        }

        if ($expr =~ s/\b0\s*&&\s*($term)/0/) {
            dprint "$.: simplified 0 && X => 0: $expr\n";
            next;
        }
        if ($expr =~ s/($term)\s*&&\s*1\b/$1/) {
            dprint "$.: simplified X && 1 => X: $expr\n";
            next;
        }
        if ($expr =~ s/($term)\s*&&\s*0\b/0/) {
            dprint "$.: simplified X && 0 => 0: $expr\n";
            next;
        }

        # boolean || simplification
        if ($expr =~ s/\b1\s*\|\|\s*($andexpr)/1/) {
            dprint "$.: simplified 1 || X => 1: $expr\n";
            next;
        }
        if ($expr =~ s/\b0\s*\|\|\s*($andexpr)/$1/) {
            dprint "$.: simplified 0 || X => X: $expr\n";
            next;
        }
        if ($expr =~ s/($andexpr)\s*\|\|\s*1\b/1/) {
            dprint "$.: simplified X || 1 => 1: $expr\n";
            next;
        }
        if ($expr =~ s/($andexpr)\s*\|\|\s*0\b/$1/) {
            dprint "$.: simplified X || 0 => X: $expr\n";
            next;
        }

        # In the absence of any other other simplification, try to collapse an inner
        # paren group by replacing it with an invented atom of the form "@num@".
        # It will be substituted back later.
        if ($expr =~ /(\([^()]*\))/) {
            $subst[$subst_count] = $1;
            $expr =~ s/\([^()]*\)/\@$subst_count\@/;
            dprint "$.: collapsed paren group $subst_count: $expr\n";
            $subst_count++;
            next;
        }
    }

    # expand collapsed atoms in reverse order
    while ($subst_count-- > 0) {
        $expr =~ s/\@$subst_count\@/$subst[$subst_count]/;
        dprint "$.: expand atom $subst_count: $expr\n";
    }

    # clean up white space
    #$expr =~ s/\s+/ /g;
    #$expr =~ s/^\s+//;
    #$expr =~ s/\s+$//;

    dprint "$. simplify expr result $expr\n";

    $expr;
}

# Display the current script's SVN revision number.
# This number is extracted from the file header, under "$Revision: 434756 $" SVN keyword.
sub display_revision_number {
    open CURR_FILE, "<", __FILE__;
    while (<CURR_FILE>) {
        if (m/#\s*\$Revision: (\d+)\s*\$/) {
            print $1, "\n";
            last;
        }
    }
    close(CURR_FILE);
}

BEGIN {
    $strip_comments = 0;
    $strip_bcmromfn = 0;
    $skip_copyright_open = 0;
    $translate_open_to_dual_copyright = 0;
    @strip_wlsyms = ();
    %set_macros = ();
    @defined_macros = ();
    @not_defined_macros = ();
    $quiet = 0;
    $verbose = 0;
    $external_flag = 0;
    $break_links = 0;
    $line_ending = 'o';
    $tobase = undef;
    $convert2ifdef = 0;
    $hide_trivial = 0;

    while (@ARGV && $ARGV[0] =~ /^-(.)(.*)$/) {
        my $arg = shift @ARGV;

        if ("$arg" =~ /^--version$/) {
            display_revision_number();
            exit 0;
        }

        if ($arg =~ /^-D(\w+)=(\d+)/) {
            my $macro = $1;
            my $value = $2;
            ($value =~ /^[01]$/) || die "value of $macro must be 0 or 1";
            $set_macros{$macro} = $value;
        } elsif ($arg =~ /^-D(\w+)/) {
            push(@defined_macros, $1);
        } elsif ($arg =~ /^-U(\w+)/) {
            push(@not_defined_macros, $1);
            $external_flag = 1 if ($1 =~ /BCMINTERNAL/);
        } elsif ($arg =~ /^-break_links/) {
            $break_links = 1;
        } elsif ($arg =~ /^-endings=n/) {
            # This could be expanded to allow -endings=[dnou]
            # (DOS, native, original, or Unix line endings).
            ($line_ending = $arg) =~ s/.*=//;
        } elsif ($arg =~ /^-tobase/) {
            $tobase = shift @ARGV;
            # Not sure why but the original code goes to
            # great lengths to avoid relying on any modules,
            # even core modules. In keeping with that we use
            # modules only when this new feature is requested.
            require File::Basename;
        } elsif ($arg =~ /^-quiet/) {
            $quiet = 1;
        } elsif ($arg =~ /^-debug/) {
            $verbose = 1;
        } elsif ($arg =~ /^-strip_comments/) {
            #strip comment blocks except for copyright block
            $strip_comments = 1;
        } elsif ($arg =~ /^-skip_copyright_open/) {
            #skip replacing "open" copyright text
            $skip_copyright_open = 1;
        } elsif ($arg =~ /^-translate_open_to_dual_copyright/) {
            #For certail gpl builds, translate open to dual copyright text
            $translate_open_to_dual_copyright = 1;
        } elsif ($arg =~ /^-strip_wlmsgs/) {
            # strip WL_<MSGS>() blocks (like WL_ERROR(), WL_TRACE(), WL_INFO()
            my $strip_wlsyms = shift @ARGV;
            if ($strip_wlsyms !~ /WL_\w+/) {
                print "ERROR: -strip_wlmsgs needs WL_<MSG> symbols to strip\n";
                print "ERROR: e.g: -strip_wlmsgs \"WL_ERROR WL_TRACE\"\n";
                exit 1;
            } else {
                my %seen;
                @strip_wlsyms = sort grep { !$seen{$_}++ } split(/\s+/, $strip_wlsyms);
            }
        } elsif ($arg =~ /^-strip_bcmromfn/) {
            $strip_bcmromfn = 1;
        #CHANGE for JIRA 336
        } elsif ($arg =~ /^-convert2ifdef/){
            $convert2ifdef = 1;

            my $convert2ifdef_syms = shift @ARGV;
            if ($convert2ifdef_syms !~ /-C\w+/) {
                print "ERROR: -convert2ifdef needs -C<SYMBOL> symbols to convert\n";
                print "ERROR: e.g: -convert2ifdef \"-CSYMBOL1 -CSYMBOL2\"\n";
                exit 1;
            } else {
                my %seen;
                @convert2ifdef_syms = sort grep { !$seen{$_}++ } split(/\s*-C/, $convert2ifdef_syms);
            }
        } elsif ($arg =~ /^-hide_trivial/) {
            $hide_trivial = 1;
        }
    }
    # set macros are also defined
    push(@defined_macros, keys(%set_macros));

    # make sure macros are not defined and undefined
    foreach my $d_macro (@defined_macros) {
      foreach my $nd_macro (@not_defined_macros) {
         if ($d_macro eq $nd_macro) {
            print "$0: Macro $d_macro cannot be defined and undefined at the same time\n\n";
            exit 1;
         }
      }
    }
}

## Proprietary source license header
$copyright = <<EOF;
Copyright ${copyright_year} Broadcom

This program is the proprietary software of Broadcom and/or
its licensors, and may only be used, duplicated, modified or distributed
pursuant to the terms and conditions of a separate, written license
agreement executed between you and Broadcom (an "Authorized License").
Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to
the Software, and Broadcom expressly reserves all rights in and to the
Software and all intellectual property rights therein.  IF YOU HAVE NO
AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
THE SOFTWARE.

Except as expressly set forth in the Authorized License,

1. This program, including its structure, sequence and organization,
constitutes the valuable trade secrets of Broadcom, and you shall use
all reasonable efforts to protect the confidentiality thereof, and to
use this information only in connection with your use of Broadcom
integrated circuit products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
OR U.S. \$1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
EOF

## Open source license header
$copyright_open = <<EOF;
Copyright (C) ${copyright_year}, Broadcom. All Rights Reserved.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
EOF

# / Backslash comment to match up with "and/or" above.

## Dual License Header
$copyright_dual = <<EOF;
Copyright (C) 1999-${copyright_year}, Broadcom.

     Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed to you
under the terms of the GNU General Public License version 2 (the "GPL"),
available at http://www.broadcom.com/licenses/GPLv2.php, with the
following added to such license:

     As a special exception, the copyright holders of this software give you
permission to link this software with independent modules, and to copy and
distribute the resulting executable under terms of your choice, provided that
you also meet, for each linked independent module, the terms and conditions of
the license of that module.  An independent module is a module which is not
derived from this software.  The special exception does not apply to any
modifications of the software.

     Notwithstanding the above, under no circumstances may you combine this
software in any way with any other Broadcom software provided under a license
other than the GPL, without Broadcom\'s express prior written consent.
EOF

if ($translate_open_to_dual_copyright) {
    $copyright_open = $copyright_dual;
}

## Derivative source license header
## NOTE: Derivative license is pass-through license, in which Broadcom
## NOTE: might have legally modified sources that have other copyrights.
## NOTE: Original copyright is left as is during distribution.
## NOTE: So such modules will have Broadcom derivative license header,
## NOTE: followed by original license

$copyright_derivative = <<EOF;
Copyright (C) ${copyright_year}, Broadcom. All Rights Reserved.

This source code was modified by Broadcom. It is distributed under the
original license terms described below.
EOF

# For each line in the multi-line string '$string',
# add '$prefix' to the front and '$suffix' to the end.
#
# expand('/* ', ' */', "foo\nbar");
# returns,
# "/* foo */\n/* bar */"
#
# Note that this routine will NOT work if '$string' is empty.
#
sub expand {
    my ($prefix,$suffix,$string) = @_;
    $string =~ s%^(.*)$%${prefix}$1${suffix}%mg;
    return $string;
}

# Ignore comment lines containing svn id keywords since usernames could
# be inadvertently matched by the RE. This requires extracting text
# stretching from the "Id" keyword to the following newline from the
# comment block before the RE is applied and reinserting it afterward.
sub strip_c_comment {
    my ($cmnt) = @_;
    my($idkey) = $cmnt =~ m%(\$Id[:\$].*)%;
    $cmnt =~ s%\$Id[:\$].*%$id_del% if $idkey;
    $cmnt =~ s%([ \t]*/\*.*\b${patcomment}\b.*\*/)%${patdel}%isgxo;
    $cmnt =~ s%$id_del%$idkey% if $idkey;
    return $cmnt;
}

# Must skip lines containing svn id keywords for reason explained above.
# Must also avoid spelling out the keyword to avoid unintended expansion.
sub strip_cpp_comment {
    my ($line) = @_;
    $line =~ s%(\n?[ \t]*//.*\b${patcomment}\b.*$)%%imgxo
        if index($line, '$Id:') == -1 && index($line, '$Id'.'$') == -1;
    return $line;
}

# Must skip lines containing svn id keywords for reason explained above.
# Must also avoid spelling out the keyword to avoid unintended expansion.
sub strip_pound_comment {
    my ($line) = @_;
    $line =~ s%(\n?[ \t]*\#.*\b${patcomment}\b.*$)%%imgxo
        if index($line, '$Id:') == -1 && index($line, '$Id'.'$') == -1;
    return $line;
}

# Insert newline(s) in a long line to limit line lengths.
# Only meant to work on lines starting with #if/#ifdef/#ifndef/#elif because it
# adds a tab in front of the continuation lines.
sub splitlong {
    my $line = shift;
    my $lmax = 88;
    return $line if length($line) <= $lmax;
    return $line if $line =~ m,//,;    # Bad things happen on splitting // comments
    my $head = substr($line, 0, $lmax);
    $head =~ s/\S+$//;
    return $line if length($head) == 0;
    my $tail = substr($line, length $head);
    return $head . "\\${lineterm}\t" . &splitlong($tail);
}

# precompile regexps
my  $cvsrcs = qr/\$Id:\s+.*,v\s+/;
my  $svnrcs = qr/\$Id:\s+.*\d+\s+\d{4}-\d{2}-\d{2}\s+/;
my $preservecomment = qr'^\s*(/\*([^*]|\*[^/])*\*/)';
my $strip_unwanted_comment1 = qr'([ \t]*/\*.*?\*/)';
my $strip_unwanted_comment2 = qr"(?<=[\r\n])[ \t]*${patdel}[ \t]*[\r]{0,1}\n"x;
my $strip_unwanted_comment3 = qr'(\n?[ \t]*//.*$)';
my $recopyright = qr/\$[ \t]*copyright.*broadcom.*\$/i;
my $splitlongcpp = qr/^\s*\#\s*(?:if|ifdef|ifndef|elif)\b/;
my $filenames_c_cpp_asm = qr/\.(c|cpp|h|s|h\.in)$/i;

# Handling for Makefiles and selected environment variable files
my $re_makefile_name =  /.*(akefile(\.inc)?|akerules(\.env)?)|\.(make|mk)$|(bcm9|vars43).*\.txt$/i;

sub process_file {
    my $process_fname  = shift;
    open(INPUT, "<$process_fname") or die "$prog: Error: $process_fname: $!";
    my @result;
    my $non_c_file = 0;
    $lineterm = '';

    while (<INPUT>) {
        # Check 1st line of file to determine the existing line terminator.
        if ($lineterm eq '') {
            if ($line_ending eq 'o') {
                $lineterm = $_;
                $lineterm =~ s/[^\r\n]*//;
            } else {
                $lineterm = "\n";
            }
        }

        my $output = &conditional($_);

        if ($strip_comments) {
            # CVS RCS String correction
            if ($external_flag && ($output =~ m/$cvsrcs/)) {
                $output =~ s {(\d\d:\d\d:\d\d)\s+(\w+)\s+(Exp)\s+\$}
                             {$1 $3 \$}g;
            }
            # SVN RCS String correction
            if ($external_flag && ($output =~ m/$svnrcs/)) {
                $output =~ s {(\d\d:\d\d:\d\d.*?)\s+(\w+)\s+\$}
                             {$1 \$}g;
            }
            # Preserve comment at the top of the file that contains copyright text
            $output =~ m/$preservecomment/;
            my $header = $1;
            push(@result, $header);
        }

        # For external builds, strip out login-id info from rcs string
        # CVS RCS String correction
        if ($external_flag && ($output =~ m/$cvsrcs/)) {
            $output =~ s {(\d\d:\d\d:\d\d)\s+(\w+)\s+(Exp)\s+\$}
                             {$1 $3 \$}g;
        }
        # SVN RCS String correction
        if ($external_flag && ($output =~ m/$svnrcs/)) {
           $output =~ s {(\d\d:\d\d:\d\d.*?)\s+(\w+)\s+\$}
                             {$1 \$}g;
        }

        # Handling for C, C++, ASM source files only
        if ($process_fname =~ m/$filenames_c_cpp_asm/g) {
            # Remove unwanted C comments and the white space surrounding them
            $output =~ s{$strip_unwanted_comment1}
                {strip_c_comment($1)}egsx;
            $output =~ s/$strip_unwanted_comment2//gx;
            $output =~ s/$repatdel//gx;
            # Remove unwanted single-line or line-tailing C++ comments
            $output =~ s{$strip_unwanted_comment3}
                {strip_cpp_comment($1)}egmx;
        }

        # Handling for Makefiles and selected environment variable files
        if ($process_fname =~ m/$re_makefile_name/g) {
            $non_c_file = 1;
            # Remove unwanted single-line or line-tailing pound comments
            $output =~ s{(\n?[ \t]*\#.*$)}
                  {strip_pound_comment($1)}egmx;
        }

        # XXX Fixme: updating the copyright using these regular expressions is very
        # XXX slow and takes much longer than all the rest of the mogrification.
        if ( $output =~ m/$recopyright/i )
        {
            if ( $debug ) { print "Entered Copyright section on ${currfile}\n"; }
            # Insert Broadcom Dual copyright/license text (non-conditional)
            if ( $output =~ m/\$[ \t]*copyright.*dual.*license.*broadcom.*\$/i ) {
                if ( $debug ) { print "Entered Copyright Dual 1 section on ${currfile}\n"; }
                $output =~ s{(.*)\$[ \t]*copyright.*dual.*license.*broadcom.*\$([^\r\n]*)(\r\n|\n)}
                    {expand($1,$2,${copyright_dual})}oeigx;
            }
            if ( $output =~ m/\$[ \t]*copyright.*broadcom.*dual.*license.*\$/i ) {
                if ( $debug ) { print "Entered Copyright Dual 2 section on ${currfile}\n"; }
                $output =~ s{(.*)\$[ \t]*copyright.*broadcom.*dual.*license.*\$([^\r\n]*)(\r\n|\n)}
                    {expand($1,$2,${copyright_dual})}oeigx;
            }

            # Insert Broadcom Derivative copyright/license text
            #print STDOUT "Searching for BRCM Derivative Copyright text\n";
            if ( $output =~ m/\$[ \t]*copyright.*derivative.*broadcom.*\$/i ) {
                if ( $debug ) { print "Entered Copyright Derivative 1 section on ${currfile}\n"; }
                $output =~ s{(.*)\$[ \t]*copyright.*derivative.*broadcom.*\$([^\r\n]*)(\r\n|\n)}
                    {expand($1,$2,${copyright_derivative})}oeigx;
            }
            if ( $output =~ m/\$[ \t]*copyright.*broadcom.*derivative.*\$/i ) {
                if ( $debug ) { print "Entered Copyright Derivative 2 section on ${currfile}\n"; }
                $output =~ s{(.*)\$[ \t]*copyright.*broadcom.*derivative.*\$([^\r\n]*)(\r\n|\n)}
                    {expand($1,$2,${copyright_derivative})}oeigx;
            }

            # Insert Broadcom Open copyright/license text (conditional)
            # If translate_open_to_dual_copyright flag is set, then open
            # copyright is translated as though it is dual license
            if ( ! $skip_copyright_open )
            {
                if ( $output =~ m/\$[ \t]*copyright.*open.*broadcom.*\$/i ) {
                    if ( $debug ) { print "Entered Copyright Open 1 section on ${currfile}\n"; }
                    $output =~ s{(.*)\$[ \t]*copyright.*open.*broadcom.*\$([^\r\n]*)(\r\n|\n)}
                        {expand($1,$2,${copyright_open})}oeigx;
                }
                if ( $output =~ m/\$[ \t]*copyright.*broadcom.*open.*\$/i ) {
                    if ( $debug ) { print "Entered Copyright Open 2 section on ${currfile}\n"; }
                    $output =~ s{(.*)\$[ \t]*copyright.*broadcom.*open.*\$([^\r\n]*)(\r\n|\n)}
                        {expand($1,$2,${copyright_open})}oeigx;
                }
            }

            # Insert Broadcom Proprietary copyright/license text (non-conditional)
            if ( $output !~ m/\$[ \t]*copyright.*open.*broadcom.*\$/i  &&
                 $output !~ m/\$[ \t]*copyright.*broadcom.*open.*\$/i ) {

                if ( $debug ) { print "Entered Copyright BCM section on ${currfile}\n"; }
                $output =~ s{(.*)\$[ \t]*copyright.*broadcom.*\$([^\r\n]*)(\r\n|\n)}
                    {expand($1,$2,${copyright})}oeigx;
            }
            if ( $debug ) { print "Leaving Copyright section on ${currfile}\n"; }
        } # end  $ copyright ... broadcom ... $

        # strip BCMROMFN, etc.
        if ($strip_bcmromfn) {
            $output =~ s/(BCMATTACHDATA|BCMATTACHFN|
                  BCMINITDATA|BCMINITFN|BCMUNINITFN|
                  BCMROMDATA|BCMROMFN|BCMROMFN_NAME)\((.*?)\)/$2/gx;
        }

        # inlined comments within quotes are stripped as well
        if ($strip_comments) {
            # Strip c-style comments
            $output =~ s#/\*[^*]*\*+([^/*][^*]*\*+)*/##egs;
            # Strip c++-style comments
            # (except for uncommented standalone or embedded URLs)
            $output =~ s#(?<!http:)//[^\n]*##egs;
        }

        # Strip out WL_<MSGS> messages
        if (@strip_wlsyms) {
            foreach (@strip_wlsyms) {
                next if /^\s+$/;
                $output =~ s#$_\(\(.*?\)\)\;#;#gs;
            }

            # Remove most of the excess semicolons resulting from the above substitution.
            my $comment = '/\*[^/]*[^*]*\*/';        # This comment regex is not quite right
            $output =~ s#;\n\s+;\s*\n(\s+)}#;\n$1}#gs;
            $output =~ s#;\n+\s+;\s*\n#;\n\n#gs;    # Retains blank lines better than next
            $output =~ s#([{}:;])((?:\s+${comment})*)\n(?:\s+;\s*\n)+#$1$2\n#gs;
            $output =~ s#:((?:\s+${comment})*)\n([ \t]*)}#:$1\n$2\t;\n$2}#gs;
        }

        # Write the output, splitting up long CPP lines
        my $uierr = 0;
        my $line = 1;
        foreach (split('\n', $output)) {
            if (/$splitlongcpp/) {
                if ($non_c_file) {
                    print STDERR "$process_fname:$line: unmogrified CPP directive: $_\n"
                    unless $quiet;
                    $uierr = 1;
                }
                push(@result, splitlong $_."\n");
            } else {
                push(@result, $_."\n");
            }
            $line++;
        }
    }
    close(INPUT);
    return @result;
} # process_file()

if (scalar(@ARGV) == 0) {
    push(@ARGV, '-');
}

while (scalar(@ARGV)) {
    my $infile = shift @ARGV;
    $currfile = $infile;
    if ( $debug ) { print "Handling $currfile\n"; }

    ## Process text files
    if ($infile eq '-') {
        # When input is stdin, its type can't be determined.
        my @output = process_file($infile);
        print @output;
    } elsif (-T $infile) {
        my @output = process_file($infile);

        if ($tobase) {
            $outfile = "$tobase/$infile";
            # XXX Directories are not created automatically here
            # because it could lead to a possible mkdir race condition
            # in parallel processing. Therefore, when using -tobase the
            # directory structure must already exist.
        } else {
            $outfile = $infile;
        }

        # Remember input file mode.
        my $srcmode = undef;
        if ($break_links) {
            $srcmode = (stat($infile))[2];
            warn("$prog: Warning: $infile: $!") if ! $srcmode;
            unlink $outfile or warn("$prog: Warning: $outfile: $!");
        }

        # Make sure the last line has a newline.
        if (@output) {
            chomp($output[-1]);
            $output[-1] .= "\n";
        }

        # Rewrite infile as outfile after some final cleanup.
        my $blanks = 0;
        open(OUTPUT, ">$outfile") or die "$prog: Error: $outfile: $!";
        foreach (@output) {
            # Since files are opened in text mode this should always
            # result in the native line ending being written out.
            s%[\r\n]*$%\n% if $line_ending eq 'n';
            # No reason to transmit trailing whitespace, especially as it
            # may have been added earlier in the mogrification process.
            s%\s+?([\r\n]+)$%$1%;
            # Ensure a comment after #endif to keep cstyle happy.
            s%^#endif$%#endif // endif%;
            # Avoid writing multiple consecutive blank lines.
            $blanks = m%^[\r\n]*$% ? $blanks + 1 : 0;
            print OUTPUT if $blanks < 2;
        }
        close(OUTPUT);

        # Make sure output file has same mode as input.
        if ($srcmode) {
            chmod($srcmode, $outfile) or warn("$prog: Warning: $outfile: $!");
        }

        if ($outfile ne $infile) {
            if (my $srcmode = (stat($infile))[2]) {
                chmod($srcmode, $outfile) or warn("$prog: Warning: $outfile: $!");
            } else {
                warn("$prog: Warning: $infile: $!");
            }
        }
    } elsif (-B $infile) {
        # Unfortunately text files with DOS line endings tend to show up as
        # binary, so rather than let a mogrifiable file slip by unmogrified
        # we treat it as a fatal error.
        if ($infile =~ m/(?:\.(?:c|cpp|h|s|h\.in)|(?:akefile(\.inc)?|akerules(\.env)?)|\.(?:make|mk)|(?:bcm9|vars43).*\.txt)$/i) {
            die("$prog: Error $infile: binary (may have CRLF line endings)");
        } else {
            print "Skipping mogrification of binary file '$infile'!\n";
        }
    }
}

# vim: ts=8:sw=4:et:
