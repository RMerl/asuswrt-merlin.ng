use strict;
use warnings;

require 5.10.0;

package BRCM::UnifdefPlus;
use Storable qw(dclone);

my $showErrs=1;

use Text::Balanced qw (
  extract_delimited
  extract_bracketed
  extract_quotelike
  extract_codeblock
  extract_variable
  extract_tagged
  extract_multiple
  gen_delimited_pat
  gen_extract_tagged
);

#use unique names (TRUE and FALSE can cause the code to change...)

# terminology: 
#       simplified means that a constant expression was resolved (no macros)
#       resolved means that an expression containing at least one known macro, and
#          no unknown macros was resolved.
#       rss is resoved/simplified state

my $RESOLVED_PREFIX = "__unifdef_resolved_";
my $SIMPLIFIED_PREFIX = "__unifdef_simplified_";
my $TRUE_RESOLVED =  $RESOLVED_PREFIX."1";
my $FALSE_RESOLVED = $RESOLVED_PREFIX."0";
my $TRUE_SIMPLIFIED =  $SIMPLIFIED_PREFIX."1";
my $FALSE_SIMPLIFIED = $SIMPLIFIED_PREFIX."0";
my $Y = qw(__unkconfig_y__);
my $M = qw(__unkconfig_m__);
my $N = qw(__unkconfig_n__);
my $COMMENT = qr/^\s*#.*$/;
my $BLANK_LINE = qr/^\s*$/;

my $BRACE_MATCH;
$BRACE_MATCH = qr/ (?: \w++ | \s++ | [\|&!=\"\\]++ | \((??{$BRACE_MATCH})\))* /x;

#RSS refers to resolve-simplify state
use constant RSS_UNCHANGED => 0;    # no macro expansion or expression simplification
use constant RSS_SIMPLIFIED => 1;   # result is a simplified expression (no macros involved)
use constant RSS_RESOLVED => 2;     # result is a resolved expression (at least one macro
                                    #    resolved, and surrounding expressions simplified)

my %rss_strings = (
    RSS_UNCHANGED => "UNCHANGED",
    RSS_SIMPLIFIED => "SIMPLIFIED",
    RSS_RESOLVED => "RESOLVED",
);

my $RESOLVED_PREFIX_PTRN = qr/$RESOLVED_PREFIX/;
my $TRUE_RESOLVED_PTRN =  qr/\b(?:$RESOLVED_PREFIX)[1-9][0-9]*\b/;
my $FALSE_RESOLVED_PTRN = qr/\b(?:$RESOLVED_PREFIX)0\b/;
my $TRUE_SIMPLIFIED_PTRN =  qr/\b(?:$SIMPLIFIED_PREFIX)[0-9][1-9]*\b/;
my $FALSE_SIMPLIFIED_PTRN = qr/\b(?:$SIMPLIFIED_PREFIX)0\b/;

my $TRUE_PTRN =  qr/(?:\b(?:$SIMPLIFIED_PREFIX|$RESOLVED_PREFIX|)[1-9][0-9]*|0x[1-9][0-9]*|TRUE)\b/;
my $FALSE_PTRN = qr/(?:\b(?:$SIMPLIFIED_PREFIX|$RESOLVED_PREFIX|)0|0x0|NULL|FALSE)\b/;


sub getValue($) {
    my $string = shift;
    return $1 if ($string =~ /^(?:$RESOLVED_PREFIX|$SIMPLIFIED_PREFIX)(\d+)$/);
    return $string if ($string =~ /^([1-9][0-9]*)$/);        
    return $string if ($string =~ /^0$/);        
    return hex $string if ($string =~ /^0x[0-9A-Fa-f]+$/);
    return oct $string if ($string =~ /^0[0-9]*$/);
    return 1 if ($string eq "TRUE");
    return 0 if ($string eq "FALSE");
    return "unknown";
}

sub isNumber_($) {
    my $string = shift;
    return getValue($string) ne "unknown";
}

sub trim($) {
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

sub trimWs($) {
    my $string = shift;
    #my ($ws1, $ws2);
    $string =~ /^(\s*)(\S*(?:\s+\S+)*)(\s*)$/;
    return ($1,$2,$3);
}

#from: http://www.perlmonks.org/?node_id=406883
sub max {
    my ($m, @vars) = @_;
    for (@vars) {
        $m = $_ if $_ > $m;
    }
    return $m;
}

sub min {
    my ($m, @vars) = @_;
    for (@vars) {
        $m = $_ if $_ < $m;
    }
    return $m;
}

sub shouldPrint {
    my $self          = shift;
    my $keep          = shift;    
    return 1 if $keep == 1;
    return 1 if $keep == 2 && $self->{simplifiedonly} == 0;
    return 0
}


# Note: taken from
#http://en.cppreference.com/w/cpp/language/operator_precedence
my %opPrecidences = (
    '!'  => 3,
    '==' => 9,
    '!=' => 9,
    '&&' => 13,
    '||' => 14,
    "<" => 8,
    ">" => 8,
    ">=" => 8,
    "<=" => 8,
    "+" => 6,
    "-" => 6,
    "*" => 5,
    "/" => 5,
    "%" => 5,
    "&" => 10,
    "^" => 11,
    "," => 17,     #used for function-like macro parameter lists
);

my $MAX_OP_PREC = 30;


my $CWS = qr{(?:(?:\s)|(?://.*$)|(?:\/\*.*?\*\/))*}s;
my $EXPR = {
    'C' => {
        IF     => qr/^((?:$CWS)#(?:$CWS)if\s+)(.*?)\s*$/s,
        ELSEIF => qr/^((?:$CWS)#(?:$CWS)elif\s+)(.*?)\s*$/s,
        ELSE   => qr/^((?:$CWS)#(?:$CWS)else\s*)(.*?)\s*$/s,
        ENDIF  => qr/^((?:$CWS)#(?:$CWS)endif\s*)(.*?)\s*$/s,
        IFDEF  => qr/^((?:$CWS)#(?:$CWS)ifdef\s+)(.*?)\s*$/s,
        IFNDEF => qr/^((?:$CWS)#(?:$CWS)ifndef\s+)(.*?)\s*$/s,
        START_ML_COMMENT =>
qr{^((("(\\\\.|[^"\\\\])*")|(\\\'(\\\\.|[^\\\\])*\\\')|[^\'"/])*/\*)(\**[^\*/]|[^\*])*$}s,
        END_ML_COMMENT => qr{\*/(\**[^\*/]|[^\*])*$}s,
        LIT_IF     => "#if ",
        LIT_ELSE   => "#else ",
        LIT_ELSEIF => "#elif ",
        LIT_ENDIF  => "#endif ",
        WHITESPACE => $CWS,
    },
    'Makefile' => {
        IF     => qr/^(\s*ifdef\s+BCM_KF\s*#\s*)(.*?)\s*$/s,
        ELSEIF => qr/^___NOT_MATCHABLE_STRING___$/s,
        ELSE   => qr/^(\s*else\s*#\s*BCM_KF\s*)(.*?)\s*$/s,
        ENDIF  => qr/^(\s*endif\s*#\s*BCM_KF\s*)(.*?)\s*$/s,
        IFDEF  => qr/^___NOT_MATCHABLE_STRING___$/s,
        IFNDEF => qr/^___NOT_MATCHABLE_STRING___$/s,
        START_ML_COMMENT => qr/^___NOT_MATCHABLE_STRING___$/s,
        END_ML_COMMENT => qr/^___NOT_MATCHABLE_STRING___$/s,
        LIT_IF     => "ifdef BCM_KF #  ",
        LIT_ELSE   => "else # BCM_KF ",
        LIT_ENDIF  => "endif # BCM_KF",
        WHITESPACE => qr{(?:(?:\s)|(?:\#.*$))*}s,
    },
    'Kconfig' => {
    }
};

my $OUTFILE;
my $DBGOUT;
*DBGOUT = *STDOUT;

# the following is a whitespace character sequence used to replace a \
# line ending.  
# This should not cotnain a newline as it breaks the script in several places
my $LINE_CONT = "\t  \t   \t \t \t   \t   \t    \t";

sub new {
    my $class = shift;
    my %opts  = @_;
    my $self  = {};
    $self->{lang} = $opts{lang} || 'C';

    $self->{dbg}       = $opts{dbg};
    $self->{defines}   = dclone( $opts{defines} );
    $self->{undefines} = dclone( $opts{undefines} );
    $self->{simplifiedonly} = $opts{simplifiedonly} || 0;
    $self->{error}     = '';
    bless( $self, $class );
    return $self;
}

sub parse {
    my $self   = shift;
    my %opts   = @_;
    my $INFILE = $opts{INFILE};
    $OUTFILE = $opts{OUTFILE};
    my @inlines = <$INFILE>;
    $self->{inlines}     = \@inlines;
    $self->{wasModified} = 0;
    $self->{lang} = $opts{lang} || $self->{lang};
    $self->{infile} = $INFILE;
    $self->{dbg} = $opts{dbg};
  
    if ($self->{lang} eq 'Kconfig') {
        my @undefineKeys = keys( %{ $self->{undefines} } );
        my @defineKeys = keys( %{ $self->{defines} } );
        $self->{kDefines} = {};
        foreach (@undefineKeys) {
            my $modKey = $_;
            $modKey =~ s/^CONFIG_//;
            $self->{kDefines}{$modKey} = $N;
        }
        foreach (@defineKeys) {
            my $modKey = $_;
            $modKey =~ s/^CONFIG_//;
            $self->{kDefines}{$modKey} = $Y if ($self->{defines}{$_} eq "y");
            $self->{kDefines}{$modKey} = $M if ($self->{defines}{$_} eq "m");
            $self->{kDefines}{$modKey} = $N if ($self->{defines}{$_} eq "n");
        }
        while(1) {
            my @outEntry = ();
            my $isVisible = $self->kconfigReadNextEntry(\@outEntry);
            if ($isVisible == 2) {            
                $self->{error} = "Unexpected end condition at line $..  Aborting";
                print STDERR "".$self->{error}."\n" if($showErrs);
                return;
            }
            if ($isVisible) {
                print $OUTFILE @outEntry;
            }
            else
            {
                $self->{wasModified} = 1;
            }
            last if (scalar(@{$self->{inlines}}) <= 0);
        }
    }
    else {
        if ( ! defined($EXPR->{$self->{lang}} )) {
            $self->{error} = "Error: unknown language $self->{lang}.\n";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        }
    
        my ( $closingExpr, $closingArg ) = $self->parseLines(2);

        if ($closingExpr) {
            $self->{error} =
              "Error: unexpected $closingExpr $closingArg at line $..  Aborting";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        }
    }
    return 2 if($self->{error});
    return ( $self->{wasModified} );
}

sub parseCondition {
    my $self = shift;
    my $lang = $self->{lang};
    my $WS = $EXPR->{$lang}->{WHITESPACE};

    # replace known variables
    my $condition     = shift;
    my $origCondition = $condition;

    my @defineKeys = keys( %{ $self->{defines} } );
    foreach (@defineKeys) {
        my $defineKey = $_;
        $condition =~ s/\bdefined\s*\(\s*$defineKey\s*\)/$TRUE_RESOLVED/g;
        $condition =~ s/\bdefined\s+$defineKey\b/$TRUE_RESOLVED/g;
        $condition =~ s/\b$defineKey\b/$RESOLVED_PREFIX$self->{defines}{$defineKey}/g;
    }

    my @undefineKeys = keys( %{ $self->{undefines} } );
    foreach (@undefineKeys) {
        my $undefineKey = $_;
        $condition =~ s/\bdefined\s*\(\s*$undefineKey\s*\)/$FALSE_RESOLVED/g;
        $condition =~ s/\bdefined\s+$undefineKey\b/$FALSE_RESOLVED/g;
    }

    if ( $condition eq $origCondition ) {
        print DBGOUT "Not simplifying $condition\n" if ($self->{dbg});
        return ($condition, 0);
    }

    print DBGOUT "simplifying $condition\n" if ($self->{dbg});

    my $remainder;
    ( $condition, $remainder ) =
      $self->simplifyExpr( $condition, $MAX_OP_PREC, 0 );

    if ( !$condition ) {
        print STDERR "Could not parse condition: $condition\n";
        return ($origCondition, 0);
    }
    else {
        $self->{wasModified} = 1;
        return ($condition . $remainder, 1);
    }
}

sub parseFuncMacro {
    my $self = shift;
    my $expr = shift;
    my $lang = $self->{lang};
    my $WS = $EXPR->{$lang}->{WHITESPACE};
   
    if ($expr =~ /^($WS)(\w+)(\s*)(\(.*)$/) {
        my $ws_bm = $1;
        my $macroName = $2;
        my $ws_am = $3;
        my $rest = $4;

        my ($params, $remainder) = extract_bracketed( $rest, "()" ) or return;
        $params =~ s/^\((.*)\)$/$1/s;

        return ($ws_bm, $macroName, $ws_am, $params, $remainder);
    }
    return;
}

sub simplifyExpr {
    my $self          = shift;
    my $string1       = shift;
    my $currentOpPrec = shift;
    my $level         = shift;
    my $lang          = $self->{lang};

    $level += 1;

    my $i      = 0;
    my $dbgStr = "<".sprintf("%2d",$currentOpPrec).">";
    while ( $i < $level ) {
        $dbgStr .= "  ";
        $i++;
    }

    my $WS = $EXPR->{$lang}->{WHITESPACE};

    my $remainder;

    my $operand1;
    my $operator;
    my $operand2;

    my $operand1_ns;      #operand1 , not-simplified.
    my $operand2_ns;

    my $ws_bo1;    #whitespace before operand1
    my $ws_ao1;    #whitespace after operand1
    my $ws_bo2;    #whitespace before operand2
    my $ws_ao2;    #whitespace after operand2

    my $rss_o1 = RSS_UNCHANGED;
    my $rss_o2 = RSS_UNCHANGED;
    


    print DBGOUT
"\n$dbgStr Simplifying expression .$string1. ($currentOpPrec) (level=$level)\n"
      if $self->{dbg};

    #read first operand:

    #test for brackets:
    if ( $string1 =~ /^($WS)\((.*)$/s ) {
        my $braceExpr;
        my $remainder2;
        my $remainder3;
        my $ws_bm;         #whitespace before macro
        my $ws_am;         #whitespace after macro
        my $macro;
        my $params;
        my $ws_bb = $1;    #whitespace before brackets
        my $ws_ib;         #whitespace inside of brackets (front)
        my $rsstate;
                           #get expression within braces       
        ( $braceExpr, $remainder ) = extract_bracketed( $string1, "()" );
        $braceExpr =~ s/^\(($WS)(.*)\)$/$2/s;
        $ws_ib = $1;
        

        $rss_o1 = RSS_UNCHANGED;
        print DBGOUT "$dbgStr braceExpr=.$braceExpr.\n" if $self->{dbg};
        
        #simplify expression within braces
        ( $braceExpr, $remainder2, $rsstate, $operand1_ns ) =
          $self->simplifyExpr( $braceExpr, $MAX_OP_PREC, $level );
        print DBGOUT "$dbgStr WARNING: raminder2 not null ($remainder2)\n" 
          if ($remainder2!~/^\s*$/);

        print DBGOUT "$dbgStr ns1=\"$operand1_ns\"\n" if $self->{dbg};
        $operand1_ns = "(".$ws_ib.$operand1_ns.$remainder2.")";
        print DBGOUT "$dbgStr ... ns1=\"$operand1_ns\"\n" if $self->{dbg};

        if ( $braceExpr =~ /^($WS)((?:\w+)|(?:defined\s+\w+))($WS)$/s ) {
            print DBGOUT "$dbgStr Removing braces on simple term\n" if $self->{dbg};
            $operand1 = $2;
            $ws_bo1   = $ws_bb;
            $ws_ao1   = $3.$remainder2;
            $rss_o1  = max(RSS_SIMPLIFIED,$rsstate);
        }
        else {
            print DBGOUT "$dbgStr Not removing braces\n" if $self->{dbg};
            $operand1 = "($braceExpr$remainder2)";
            $ws_bo1   = $ws_bb;
            $ws_ao1   = "";
            $rss_o1 = $rsstate;
        }
    }

    #test for not operator:
    elsif ( $string1 =~ /^($WS)!($WS)(.*)$/s ) {
        $ws_bo1 = $1;
        my $ws_an = $2;    #whitespace after not
        my $notOperand;
        my $notOperand_rss;
        my $notOperand_ns;

        ( $notOperand, $remainder, $notOperand_rss, $notOperand_ns ) =
          $self->simplifyExpr( $3, $opPrecidences{"!"} + 1, $level )
          or return;
        $ws_ao1   = "";
        $operand1_ns = "!".$ws_an.$notOperand_ns;
        if ( $notOperand =~ /^($WS)($FALSE_RESOLVED_PTRN)($WS)$/s ) {
            $operand1 = "$TRUE_RESOLVED";
            $rss_o1 = RSS_RESOLVED;
        }
        elsif ( $notOperand =~ /^($WS)($TRUE_RESOLVED_PTRN)($WS)$/s ) {
            $operand1 = "$FALSE_RESOLVED";
            $rss_o1 = RSS_RESOLVED;
        }
        elsif ( $notOperand =~ /^($WS)($FALSE_PTRN)($WS)$/s ) {
            $operand1 = "$TRUE_SIMPLIFIED";
            $rss_o1 = max(RSS_SIMPLIFIED, $notOperand_rss);
        }
        elsif ( $notOperand =~ /^($WS)($TRUE_PTRN)($WS)$/s ) {
            $operand1 = "$FALSE_SIMPLIFIED";
            $rss_o1 = max(RSS_SIMPLIFIED, $notOperand_rss);
        }
        else {
            $operand1 = "!" . $ws_an . $notOperand;
            $rss_o1 = $notOperand_rss;
        }
    }

    # test for function-like macro:
    elsif ( my ($ws_bm, $macro, $ws_am, $params, $remainder2) = 
                  $self->parseFuncMacro($string1)) {
        #simplify expression within braces
        print DBGOUT "$dbgStr ... macro: .$macro.$params.\n" if $self->{dbg};
        my ( $sparams, $remainder3, $sparams_rss, $sparams_ns );
        if ( $params =~ /^($WS)$/ ) {
            $sparams_ns = $params;
            $sparams = $params;
            $remainder3 = "";
            $sparams_rss = RSS_UNCHANGED;
        }
        else {
            ( $sparams, $remainder3, $sparams_rss, $sparams_ns ) =
              $self->simplifyExpr( $params, $MAX_OP_PREC, $level ) or return;
        }
        $operand1_ns = $macro.$ws_am."(".$sparams_ns.$remainder3.")";
        $operand1 = $macro.$ws_am."(".$sparams.$remainder3.")";
        $ws_bo1 = $ws_bm;
        $remainder = $remainder2;
        $rss_o1 = $sparams_rss;
    }

    # get next single term:
    elsif ( $string1 =~ /^($WS)(\w+)\b(.*)/s ) {
        $ws_bo1    = $1;
        $operand1  = $2;
        $remainder = $3;

        my $braceExpr = "";

        $operand1_ns = $operand1;
        
        if ($operand1 =~ /$RESOLVED_PREFIX_PTRN/) {
            $rss_o1 = RSS_RESOLVED;
        }
        else {
            $rss_o1 = RSS_UNCHANGED;
        }

        if (($operand1 eq "defined") 
          && ($remainder =~ /((?:$WS)\w+)\b(.*)$/)
          ) {
            $operand1 .= $1; 
            $operand1_ns = $operand1;
            $remainder = $2;
            #don't need to worry about resolving here, as resolving all known 
            #"defined XXX" terms has already been done.
        }
    }
    else {
        print DBGOUT "$dbgStr NO MTACH\n" if $self->{dbg};
    }

    while ( !( $remainder =~ /^($WS)$/s ) ) {
        #TBD: base operator regex on operators hash...
        if ( $remainder =~ /^($WS)([\!\|\>\<\=&\*\+\-\/\%\^\&\,]+)($WS)(.*)$/s ) {
        #my $allops=join('|',quotemeta keys %opPrecidences)
        #if ( $remainder =~ /^($WS)($allops)($WS)(.*)$/s ) {
            $ws_ao1 = $1;
            $operator = $2;
            $ws_bo2 = $3;
            my $remainder2 = $4;
            if ( exists $opPrecidences{$operator} ) {
                if ( $opPrecidences{$operator} < $currentOpPrec ) {

                    ( $operand2, $remainder, $rss_o2, $operand2_ns ) =
                      $self->simplifyExpr( $remainder2,
                        $opPrecidences{$operator}, $level )
                      or return;

                    $ws_ao2 = "";  # whitespace will belong before next operand.
                    

# Note, the followin scheme can probably be written a bit neater, but I'm new to perl...

# simplifying:
                    print DBGOUT 
                       "$dbgStr $operand1.$ws_ao1.$operator.$ws_bo2.$operand2 ==> \n"
                       if $self->{dbg};

                    if (
                        ( $operator eq "&&" )
                        && (   ( $operand1 =~ /^($WS)$FALSE_RESOLVED_PTRN($WS)$/s )
                            || ( $operand2 =~ /^($WS)$FALSE_RESOLVED_PTRN($WS)$/s ) )
                      )
                    {
                        $operand1 = "$FALSE_RESOLVED";
                        $rss_o1   = RSS_RESOLVED;
                        $operand1_ns = $operand1;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif (
                        ( $operator eq "||" )
                        && (   ( $operand1 =~ /^($WS)$TRUE_RESOLVED_PTRN($WS)$/s )
                            || ( $operand2 =~ /^($WS)$TRUE_RESOLVED_PTRN($WS)$/s ) )
                      )
                    {
                        print DBGOUT "$dbgStr ... got here1\n" if $self->{dbg};
                        $operand1 = "$TRUE_RESOLVED";
                        $rss_o1   = RSS_RESOLVED;
                        $operand1_ns = $operand1;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif ( $operator eq "," ) {
                        #ensure we don't try to simplify , operators
                        $operand1 = $operand1.$ws_ao1.$operator.$ws_bo2.$operand2;
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = $operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao1;
                    }                    
                    elsif ( isNumber_($operand1) && isNumber_($operand2) ) 
                    {                        
                        # note: this has an effect of simplifying constant expressions
                        
                        my $newVal;
                        my $evalErr;
                        my $expression = getValue($operand1)." ".$operator." ".getValue($operand2);
                        $newVal = eval "$expression" || 0;
                        print DBGOUT "$dbgStr eval:\"$expression\" ==> \"$newVal\"\n" if $self->{dbg};
                        $evalErr = $@;
                        if ( ! $evalErr ) { 
                            $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                            $operand1 = ($rss_o1==RSS_SIMPLIFIED?$SIMPLIFIED_PREFIX:$RESOLVED_PREFIX).$newVal;
                        } else {
                            print DBGOUT "$dbgStr evalErr -- reverting\n"  if $self->{dbg};
                            $operand1 =
                              $operand1 . $ws_ao1 . $operator . $ws_bo2 . $operand2;
                            $rss_o1 = max($rss_o1, $rss_o2);
                        }
                        
                        $operand1_ns = ($rss_o1 == RSS_RESOLVED) ?
                            $operand1 :
                            $operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns;                        
                        $ws_bo1 = $ws_bo1;
                        $ws_ao1 = $ws_ao2; 
                    }

                    # after this point we know that at least one side is not completely simplified:
                    
                    elsif (
                        ( $operator eq "&&" )
                        && (   ( $operand1 =~ /^($WS)$FALSE_SIMPLIFIED_PTRN($WS)$/s )
                            || ( $operand2 =~ /^($WS)$FALSE_SIMPLIFIED_PTRN($WS)$/s ) )
                      )
                    {
                        #$operand1_ns = $operand1_ns;
                        $operand1 = "$FALSE_SIMPLIFIED";
                        $rss_o1   = RSS_SIMPLIFIED;
                        $operand1_ns = $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif (( $operator eq "&&" )
                        && ( $operand1 =~ /^($WS)$TRUE_PTRN($WS)$/s ) )
                    {
                        $operand1 = $operand2;
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = ($rss_o1==RSS_RESOLVED)?
                            $operand1 :
                            $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;
                        $ws_bo1   = $ws_bo2;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif (( $operator eq "&&" )
                        && ( $operand2 =~ /^($WS)$TRUE_PTRN($WS)$/s ) )
                    {
                        $operand1 = $operand1;
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = ($rss_o1==RSS_RESOLVED)?
                            $operand1 :
                            $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao1;
                    }
                    elsif (
                        ( $operator eq "||" )
                        && (   ( $operand1 =~ /^($WS)$TRUE_SIMPLIFIED_PTRN($WS)$/s )
                            || ( $operand2 =~ /^($WS)$TRUE_SIMPLIFIED_PTRN($WS)$/s ) )
                      )
                    {
                        $operand1 = "$TRUE_SIMPLIFIED";
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif (( $operator eq "||" )
                        && ( $operand1 =~ /^($WS)$FALSE_PTRN($WS)$/s ) )
                    {
                        $operand1 = $operand2;
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = ($rss_o1 == RSS_RESOLVED) ?
                            $operand1 :
                            $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;
                        $ws_bo1   = $ws_bo2;
                        $ws_ao1   = $ws_ao2;
                    }
                    elsif (( $operator eq "||" )
                        && ( $operand2 =~ /^($WS)$FALSE_PTRN($WS)$/s ) )
                    {
                        $operand1 = $operand1;
                        $rss_o1   = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1_ns = ($rss_o1 == RSS_RESOLVED) ?
                            $operand1 :
                            $operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns;
                        $ws_bo1   = $ws_bo1;
                        $ws_ao1   = $ws_ao1;
                    }
                    elsif ( ($operator eq "==") 
                        && ($operand1 eq $operand2) ) {
                        $operand1_ns = $operand1.$ws_ao1.$operator.$ws_bo2.$operand2;
                        $rss_o1 = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1 = $rss_o1==RSS_RESOLVED?"$TRUE_RESOLVED":"$TRUE_SIMPLIFIED";
                        $operand1_ns = ($rss_o1 == RSS_RESOLVED) ?
                            $operand1 :
                            $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;                        
                        $ws_bo1 = $ws_bo1;
                        $ws_ao1 = $ws_ao2;
                    }
                    elsif ( ($operator eq "!=") && ($operand1 eq $operand2) ) {
                        $rss_o1 = max($rss_o1, $rss_o2, RSS_SIMPLIFIED);
                        $operand1 = $rss_o1==RSS_RESOLVED?"$FALSE_RESOLVED":"$FALSE_SIMPLIFIED";
                        $operand1_ns = ($rss_o1 == RSS_RESOLVED) ?
                            $operand1 :
                            $ws_bo1.$operand1_ns.$ws_ao1.$operator.$ws_bo2.$operand2_ns.$ws_ao2;                        
                        $ws_bo1 = $ws_bo1;
                        $ws_ao1 = $ws_ao2;
                    }
                    else {
                        # we have an expression which we can't simplify any further.  
                        # unsimplify both operands if they are simplified.
                        print DBGOUT "$dbgStr Unsimplifying: $operand1 $operator $operand2\n" 
                          if $self->{dbg};
                        print DBGOUT "$dbgStr ... ns1 \"$operand1_ns\" ns2 \"$operand2_ns\"\n" 
                          if $self->{dbg};

                        if ($rss_o1 == RSS_SIMPLIFIED) {
                            $operand1 = $operand1_ns;
                            $rss_o1 = RSS_UNCHANGED;
                        }
                        
                        if ($rss_o2 == RSS_SIMPLIFIED) {
                            $operand2 = $operand2_ns;
                            $rss_o2 = RSS_UNCHANGED;
                        }
                        
                        $operand1 =~ s/\b$SIMPLIFIED_PREFIX|$RESOLVED_PREFIX//g;
                        $operand2 =~ s/\b$SIMPLIFIED_PREFIX|$RESOLVED_PREFIX//g;
                        
                        $operand1 =
                          $operand1 . $ws_ao1 . $operator . $ws_bo2 . $operand2;
                        $operand1_ns = $operand1;
                        $ws_bo1 = $ws_bo1;
                        $ws_ao1 = $ws_ao2;
                        $rss_o1 = max($rss_o1, $rss_o2);
                        print DBGOUT "$dbgStr ... Unsimplified: $operand1\n" 
                          if $self->{dbg};
                    }
                    print DBGOUT "$dbgStr      ==> \"$operand1\", r:\"$remainder\", ns:\"$operand1_ns\" \n" 
                          if $self->{dbg};
                    
                }
                else {
                    print DBGOUT
"$dbgStr op precidence exceded: returning \"$operand1\", r:\"$remainder\", ns:\"$operand1_ns\"\n\n"
                      if $self->{dbg};
                    return ( $operand1, $remainder, $rss_o1, $operand1_ns );
                }
                print DBGOUT "$dbgStr operand1 = $operand1, r:$remainder\n"
                  if $self->{dbg};
            }
            else {
                $self->{error} = "operator not found .$operator.";
                print STDERR "".$self->{error}."\n" if($showErrs);
                return;    # error
            }
        }

        print DBGOUT
"$dbgStr end while loop o1=\"$operand1\", r=\"$remainder\", s=".$rss_o1.",ns:\"$operand1_ns\"\n\n"
          if $self->{dbg};

        $operator = "";
        $operand2 = "";
    }

    return ( $operand1, $remainder, $rss_o1, $operand1_ns );
}

sub parseIf {
    my $self       = shift;
    my $litExpr    = shift;
    my $expression = shift;
    my $keep       = shift;
    my $simplified = shift;
    my $lang       = $self->{lang};
    my $nextExprType;
    my $nextConditional;
    my $line;

    my $WS = $EXPR->{$lang}->{WHITESPACE};

    $keep = 1 if ( $simplified && $keep == 2);
    
    if ( $simplified && $expression =~ /^($WS)($TRUE_PTRN)($WS)$/ ) {

        # do not echo #if.
        # find next #else / #elseif, and wipe till #endif:

        ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
        while ( $nextExprType =~ /$EXPR->{$lang}->{ELSEIF}/s ) {
            ( $nextExprType, $nextConditional ) = $self->parseLines(0);
            if (!$nextExprType) {
                $self->{error} = "unexpected end of file";
                print STDERR "".$self->{error}."\n" if($showErrs);
                return;
              }
        }
        if ( $nextExprType =~ /$EXPR->{$lang}->{ELSE}/s ) {
            ( $nextExprType, $nextConditional ) = $self->parseLines(0);
        }
        if (!$nextExprType) {
            $self->{error} = "unexpected end of file";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        }
        if ( $nextExprType !~ /$EXPR->{$lang}->{ENDIF}/s ) {
            $self->{error} = "unterminated if ($expression)";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        }
        return (1);
    }
    elsif ( $simplified && $expression =~ /^($WS)$FALSE_PTRN($WS)$/ ) {

        #do not echo #if.
        #if we find an elseif, treat it as an if
        #if we find an else, echo till endif (do not echo endif)

        my ( $nextExprType, $nextConditional ) = $self->parseLines(0);
        if (!$nextExprType) {
             $self->{error} = "unterminated if ($expression)";
             print STDERR "".$self->{error}."\n" if($showErrs);
             return;
        }
        
        if ( $nextExprType =~ /$EXPR->{$lang}->{ELSEIF}/s ) {
        	#switch elseif to if, and parse as if if statement
            my ($nextExpression, $wasSimplified) = $self->parseCondition($nextConditional);
            $self->parseIf( $EXPR->{$lang}->{LIT_IF}, $nextExpression, $keep, $wasSimplified );
        }
        elsif ( $nextExprType =~ /$EXPR->{$lang}->{ELSE}/s ) {
            ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
        }
        if ( $nextExprType !~ /$EXPR->{$lang}->{ENDIF}/s ) {
            $self->{error} = "unterminated if ($expression)";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        }
        # do not echo endif
    }
    else {

        #unresolved condition: echo if
        #if we find an elsif, resolve condition, and remove code if false
        #if we find an else, echo that.
        #echo endif

        $line = $litExpr . $expression;
        $line =~ s/$LINE_CONT/\\\n/g;

        print $OUTFILE $line . "\n" if $self->shouldPrint($keep);
        ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
        if (!$nextExprType) {
            $self->{error} = "Unmatched  $litExpr  $expression at eof";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
          }

        while ( $nextExprType =~ /$EXPR->{$lang}->{ELSEIF}/s ) {
            my ($nextExpression, $wasSimplified) = $self->parseCondition($nextConditional);

            #my $nextExpression = $nextConditional;
            if ( $wasSimplified && $nextExpression =~ m/($WS)$TRUE_PTRN($WS)/ ) {
            	#found #elsif true -- convert to else, and discard further elsif/else clauses
                print $OUTFILE $EXPR->{$lang}->{LIT_ELSE} . "\n" if $self->shouldPrint($keep);
                ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
                while ( !( $nextExprType =~ /$EXPR->{$lang}->{ENDIF}/s ) ) {
                    if (!$nextExprType) {
                        $self->{error} = "unexpected end of file";
                        print STDERR "".$self->{error}."\n" if($showErrs);
                        return;
                      }
                    ( $nextExprType, $nextConditional ) = $self->parseLines(0);
                }
            }
            elsif ( $wasSimplified && $nextExpression =~ /($WS)$FALSE_PTRN($WS)/ ) {
                ( $nextExprType, $nextConditional ) = $self->parseLines(0);
            }
            else {
            	# unresolved conditional -- echo, and go on
                $line = $EXPR->{$lang}->{LIT_ELSEIF} . $nextExpression;
                $line =~ s/$LINE_CONT/\\\n/g;
                print $OUTFILE $line . "\n" if $self->shouldPrint($keep);
                ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
            }
        }
        if ( $nextExprType =~ /$EXPR->{$lang}->{ELSE}/s ) {
            $line = $nextExprType . $nextConditional;
            $line =~ s/$LINE_CONT/\\\n/g;
            print $OUTFILE $line . "\n" if $self->shouldPrint($keep);
            ( $nextExprType, $nextConditional ) = $self->parseLines($keep);
        }
        if ( $nextExprType !~ /$EXPR->{$lang}->{ENDIF}/s ) {
            $self->{error} = "expected endif ($nextExprType)";
            print STDERR "".$self->{error}."\n" if($showErrs);
            return;
        } else {
            # this line must be an endif, echo, and return
            $line = $nextExprType . $nextConditional;
            $line =~ s/$LINE_CONT/\\\n/g;
            print $OUTFILE $line . "\n" if $self->shouldPrint($keep);
        }
    }

    return (1);
}

sub parseLines {
    my $self = shift;
    my $keep = shift;
    my $lang = $self->{lang};
    my $line;
    my $newLine;
    my $litExpr;
    my $expression;

    while (1) {

        $line = ( shift @{ $self->{inlines} } );
        return unless defined($line);
        chomp $line;

        while ( $line =~ /^(.*)\\$/s ) {

#switch \ at end of line to whitespace -- but we will need to recover it later on,
#
            $line = $1 . $LINE_CONT . ( shift @{ $self->{inlines} } );
            chomp $line;
        }
        if ( $line =~ m#$EXPR->{$lang}->{START_ML_COMMENT}#s ) {

            # Treat multi-line comments as a single line
            while ( $newLine = ( shift @{ $self->{inlines} } ) ) {
                chomp $newLine;
                $line = $line . "\n" . $newLine;
                last if $newLine =~ m#$EXPR->{$lang}->{END_ML_COMMENT}#s;
            }
        }
        if ( $line =~ /$EXPR->{$lang}->{IFDEF}/s ) {
            $litExpr    = $1;
            $expression = $2;

            #$macro = $expression =~ /^\s+(\S+)/s ;
            my $macro = trim($expression);
            print DBGOUT
              "litExpr=$litExpr, expression=$expression, macro=.$macro.\n"
              if $self->{dbg};
            if ( $self->{defines}->{$macro} ) {
                print DBGOUT "calling parseIf TRUE\n" if $self->{dbg};
                $self->parseIf( $EXPR->{$lang}->{LIT_IF}, "$TRUE_RESOLVED", $keep, 1 );
                $self->{wasModified}=1;
            }
            elsif ( $self->{undefines}->{$macro} ) {
                print DBGOUT "calling parseIf FALSE\n" if $self->{dbg};
                $self->parseIf( $EXPR->{$lang}->{LIT_IF}, "$FALSE_RESOLVED", $keep, 1 );
                $self->{wasModified}=1;
            }
            else {
                print DBGOUT "calling parseIf $expression\n" if $self->{dbg};
                $self->parseIf( $litExpr, $expression, $keep, 0 );
            }
        }
        elsif ( $line =~ /$EXPR->{$lang}->{IFNDEF}/s ) {
            $litExpr    = $1;
            $expression = $2;

            #$macro = $expression =~ /^\s+(\S+)/s ;
            my $macro = trim($expression);
            if ( $self->{defines}->{$macro} ) {
                $self->parseIf( $EXPR->{$lang}->{LIT_IF}, "$FALSE_RESOLVED", $keep, 1 );
                $self->{wasModified}=1;
            }
            elsif ( $self->{undefines}->{$macro} ) {
                $self->parseIf( $EXPR->{$lang}->{LIT_IF}, "$TRUE_RESOLVED", $keep, 1 );
                $self->{wasModified}=1;
            }
            else {
                $self->parseIf( $litExpr, $expression, $keep, 0 );
            }
        }
        elsif ( $line =~ /$EXPR->{$lang}->{IF}/s ) {
            $litExpr    = $1;
            my $wasSimplified;
            ($expression, $wasSimplified) = $self->parseCondition($2);
            $self->parseIf( $litExpr, $expression, $keep, $wasSimplified );
        }
        elsif ( $line =~ /$EXPR->{$lang}->{ELSEIF}/s ) {
            $litExpr    = $1;
            $expression = $2;
            return ( $litExpr, $expression );
        }
        elsif ( $line =~ /$EXPR->{$lang}->{ELSE}/s ) {
            $litExpr    = $1;
            $expression = $2;
            return ( $litExpr, $expression );
        }
        elsif ( $line =~ /$EXPR->{$lang}->{ENDIF}/s ) {
            $litExpr    = $1;
            $expression = $2;
            return ( $litExpr, $expression );
        }
        else {

            # replace stripped out line cont's:
            $line =~ s/$LINE_CONT/\\\n/g;
            print $OUTFILE $line . "\n" if $self->shouldPrint($keep);
        }
    }
}

## --------------------------------------------------------------------------
## KCONFIG support:
##
## Note: Kconfig work very differently than Makefiles or C files
##
##
# NOTE: if behaviour is actually simplified a bit:  in reality, if appends a 
# condition to all items inside of it.  (So, if you had an 
#
#    if x
#    source Kconfig.foo
#    endif
#
# then source Kconfig.foo would still get expanded, only every entry in
# Kconfig.foo would have if x appended to its end. This code treats if
# differently, in that it would remove source Kconfig.foo entirely if x 
# was false...

sub kconfigSimplifyExprTop {
    my $self = shift;
    my $origExpr = shift;

    my %realval = ( $Y => "y", $M => "m", $N => "n");
    my $check = join '|', keys %realval;
    
    my $expr = $self->kconfigSimplifyExpr($origExpr);
    my $rtExpr = $expr;
    $rtExpr =~ s/($check)/$realval{$1}/g;
    $self->{wasModified} = 1 if ( $origExpr ne $rtExpr );
    return (trim($rtExpr),trim($expr));
}

sub kconfigSimplifyExpr {
    my $self = shift;
    my $expr = shift;

    my ($ws1, $ws2, $ws3, $ws4, $ws5, $ws6, $op1, $op2);
    my ($ws11, $ws12, $ws21, $ws22);
    my ($e1, $e2);

    if ($expr =~ /^($BRACE_MATCH)\|\|($BRACE_MATCH)$/ ) {
        ($e1, $e2) = ($1,$2);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1)); 
        ($ws21,$op2,$ws22) = trimWs($self->kconfigSimplifyExpr($e2));
        return $Y if ($op1 eq $Y || $op2 eq $Y);
        return $M if (($op1 eq $M || $op1 eq $Y) && ($op2 eq $M || $op2 eq $Y));
        return $N if ($op1 eq $N && $op2 eq $N);
        return "".$ws11.$op1 if ($op2 eq $N);
        return "".$ws11.$op2 if ($op1 eq $N);
        return "".$ws11.$op1.$ws12."||".$ws21.$M if ($op2 eq $M);
        return "".$ws11.$M.$ws12."||".$ws21.$op2 if ($op1 eq $M);   
        return "".$ws11.$op1.$ws12."||".$ws21.$op2.$ws22;
    }
    elsif ($expr =~ /^($BRACE_MATCH)&&($BRACE_MATCH)$/ ) {
        ($e1, $e2) = ($1,$2);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1)); 
        ($ws21,$op2,$ws22) = trimWs($self->kconfigSimplifyExpr($e2));
        return $Y if ($op1 eq $Y && $op2 eq $Y);
        return $M if (($op1 eq $M || $op1 eq $Y) && ($op2 eq $M || $op2 eq $Y));
        return $N if ($op1 eq $N || $op2 eq $N);
        return "".$ws11.$op1.$ws12 if ($op2 eq $Y);
        return "".$ws11.$op2.$ws12 if ($op1 eq $Y);
        return "".$ws11.$op1.$ws12."&&".$ws21.$M.$ws22 if ($op2 eq $M);
        return "".$ws11.$M.$ws12."&&".$ws21.$op2 if ($op1 eq $M);
        return "".$ws11.$op1.$ws12."&&".$ws21.$op2.$ws22;
    }
    elsif ($expr =~ /^(\s*)!($BRACE_MATCH)$/ ) {
        ($ws1, $e1) =($1,$2);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1));
        return $ws11.$N.$ws12 if ($op1 eq $Y);
        return $ws11.$M.$ws12 if ($op1 eq $M);
        return $ws11.$Y.$ws12 if ($op1 eq $N);
        return "".$ws1."!".$ws11.$op1.$ws12;
    }
    elsif ($expr =~ /^(\s*)\(($BRACE_MATCH)\)(\s*)$/ ) {
        ($ws1, $e1, $ws2) =($1,$2,$3);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1));
        return $ws1.$Y.$ws2 if ($op1 eq $Y);
        return $ws1.$M.$ws2 if ($op1 eq $M);
        return $ws1.$N.$ws2 if ($op1 eq $N);
        #do not remove braces on unknown expressions:
        #return $ws1.$op1.$ws2 if ($op1 =~ /^\s*\w+\s*$/);
        return "".$ws1."(".$ws11.$op1.$ws12.")".$ws2;
    }
    elsif ($expr =~ /^($BRACE_MATCH)!=($BRACE_MATCH)$/ ) {
        ($e1, $e2) = ($1,$2);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1)); 
        ($ws21,$op2,$ws22) = trimWs($self->kconfigSimplifyExpr($e2));
        if (    ($op1 eq $Y || $op1 eq $M || $op1 eq $N)
             && ($op2 eq $Y || $op2 eq $M || $op2 eq $N)) {
            return $N if ($op1 eq $op2);
            return $Y;
        }
        else {
            return "".$ws11.$op1.$ws12."!=".$ws21.$op2.$ws22;
        }              
    }
    elsif ($expr =~ /^($BRACE_MATCH)=($BRACE_MATCH)$/) {
        ($e1, $e2) = ($1,$2);
        ($ws11,$op1,$ws12) = trimWs($self->kconfigSimplifyExpr($e1)); 
        ($ws21,$op2,$ws22) = trimWs($self->kconfigSimplifyExpr($e2));
        if (    ($op1 eq $Y || $op1 eq $M || $op1 eq $N)
             && ($op2 eq $Y || $op2 eq $M || $op2 eq $N)) {
            return $Y if ($op1 eq $op2);
            return $N;
        }
        else {
            return "".$ws11.$op1.$ws12."=".$ws21.$op2.$ws22;
        }              
    }
    elsif ($expr =~ /^(\s*)((?:\w|\")+)(\s*)$/) {
        return $1.$self->{kDefines}{$2}.$3 if defined $self->{kDefines}{trim($expr)};
        return $expr;
    }

    die "WARNING -- could not resolve <$expr>!!\n";
    return $expr;

}

my $LINE_SEP = "\r\t  \t   \t \t \t   \t   \t    \t\r";

# returns the visible lenght of whitespace (assuming tabs are 8 characters wide)
sub wslength {
    my $str = shift;
    # code taken from eugene y on 
    # http://stackoverflow.com/questions/5997404/perl-program-to-replace-tabs-with-spaces
    while($str =~ s/\t/" " x (8 - $-[0]%8)/e) {}    
    return length($str);
}

# attempts to read attributes
# stops when a line without indents is found
sub kconfigReadAttributes {
    my $self = shift;
    my $outLinesRef = shift;

    # clear output:
    @$outLinesRef = ();
    my $isVisible = 1;

    while (1) { 
        my $line = shift(@{$self->{inlines}});
        print DBGOUT "   - $line...\n" if ($self->{dbg});
        last unless defined($line);
        chomp($line);
        # hack: combine two lines into one.
        while (substr($line, -1) eq "\\") {
            $line = substr($line,0,-1) . $LINE_SEP . shift(@{$self->{inlines}});
            chomp($line);
        }


        if ($line =~ $COMMENT) {
            #comment: just push it
            $line =~ s/$LINE_SEP/\\\n/g;
            push(@$outLinesRef, $line."\n");
        }
        elsif ($line =~ /^(\s*)$/) {
            #blank line -- end of attributes.  Include blank line as 
            #part of item.
            $line =~ s/$LINE_SEP/\\\n/g;
            push(@$outLinesRef, $line."\n");
            last;
        }
        elsif ($line =~ /^[^\s]/) {
            #line with no preceding whitespace.  Though the spec says there should
            #be whitespace between entries, this rule isn't followed everywhere.
            #unshif the line, and finish:
            unshift(@{$self->{inlines}}, $line);
            last;
        }
        elsif ($line =~ /^(\s+)-*\s*help\s*-*.*$/)
        {
            #special handling for help: it is multiline and ends when the
            #first line of text has less indentation than the first.
            $line =~ s/$LINE_SEP/\\\n/g;
            push(@$outLinesRef, $line."\n");
            my $line = shift(@{$self->{inlines}});
            if (defined($line)) {
                chomp($line);

                if ($line =~ /^(\s+)/) {
                    # Grumble grumble: in at least one kernel Kconfig 
                    # file, they mixed up tabs and spaces...:
                    my $blankLines = 0;
                    my $helpIndentLen = wslength($1);
                    $line =~ s/$LINE_SEP/\\\n/g;
                    push(@$outLinesRef, $line."\n");
                    while (1) {
                        my $line = shift(@{$self->{inlines}});
                        last unless defined($line);
                        chomp($line);
                        if (length($line) == 0) {
                           $blankLines++; 
                        }
                        else {
                            $line =~ /^(\s*)/;
                            if (wslength($1) < $helpIndentLen) {
                                unshift(@{$self->{inlines}}, $line);
                                while($blankLines--) {unshift(@{$self->{inlines}}, "");};
                                last;
                            } else {
                                while($blankLines > 0) {$blankLines--; push(@$outLinesRef, "\n");};
                                $line =~ s/$LINE_SEP/\\\n/g;
                                push(@$outLinesRef, $line."\n");
                            }
                        } 
                    }
                }
                else {
                    # Grumble grumle -- apperently it's possible to have empty
                    # help fields...
                    unshift(@{$self->{inlines}}, $line);
                }
            }
        }
        elsif ($line =~ /^(\s+depends on\s*)(.*)$/) {
            my $tmp = $1;
            my ($condition,$result) = $self->kconfigSimplifyExprTop($2);
            if ($result eq $N) {
                $isVisible = 0;
            }
            if ($result eq $Y || $result eq $M) {
                #do not echo
            }
            else {
                $condition =~ s/$LINE_SEP/\\\n/g;
                push(@$outLinesRef, $tmp . $condition . "\n");  
            }
        }
        elsif ($line =~ /^(\s+(?:[^"]|(?:"(?:[^"\\]|\\.)*"))+)(\s+if\s+)(.*?)(\s*\#.*)?$/) {
            my $term = $1;
            my $ifTerm = $2;
            my $eolComment = defined($4) ? $4 : "";
            print DBGOUT "   conditional statement: $3" if ($self->{dbg});
            my ($condition,$result) = $self->kconfigSimplifyExprTop($3);
            if ($result eq $N) {
            }
            elsif ($result eq $Y || $result eq $M) {
                $term =~ s/$LINE_SEP/\\\n/g;
                $eolComment =~ s/$LINE_SEP/\\\n/g;
                push(@$outLinesRef, $term . $eolComment . "\n");
            }
            else {          
                $term =~ s/$LINE_SEP/\\\n/g;
                $ifTerm =~ s/$LINE_SEP/\\\n/g;
                $condition =~ s/$LINE_SEP/\\\n/g;
                $eolComment =~ s/$LINE_SEP/\\\n/g;                
                push(@$outLinesRef, $term . $ifTerm . $condition . $eolComment ."\n");
            }
        }
        elsif ($line =~ /^\s+(\w+)\s*(.*)$/) {
            $line =~ s/$LINE_SEP/\\\n/g;
            print DBGOUT "   + - $line...\n" if ($self->{dbg});
            push(@$outLinesRef, $line."\n");
        }
        else  {
            $self->{error} = "Error parsing Kconfig file ($line)";          
            print STDERR "".$self->{error}."\n" if($showErrs);
            last;
        }
    }
    return $isVisible;
}

# return 0 invisible entry, return 1 visible entry, 2 for end-block command
sub kconfigReadNextEntry {
    my $self = shift;
    my $outLinesRef = shift;
    
    my $isVisible = 1;
    my $hideEndif = 0;
    my $line;
    
    do {
        $line = ( shift @{ $self->{inlines} } );
        print DBGOUT "   > $line...\n" if ($self->{dbg});
        return 1 unless defined($line);
        chomp($line);
        # in the case of an unexpected endblock, push the endblock back onto the array,
        # and return 2.  The caller will process the endblock line.
        if ($line =~ "\^endchoice" || $line =~ "\^endmenu" || $line =~ "\^endif") {
            unshift(@{ $self->{inlines} }, $line."\n");
            return 2;
        }
        push(@$outLinesRef, $line."\n");        
    } while(    $line =~ /$COMMENT/ || $line =~ /$BLANK_LINE/);

    if ( $line =~ /^source\s/ || $line =~ /^mainmenu\s/ ) {
        return 1;
    }
    
    if (    $line =~ /^config\s/ || $line =~ /^menuconfig\s/ || $line =~ /^comment/ 
         || $line =~ /^choice/ || $line =~ /^menu/) {
        #read attributes
        my @entryAttributes = ();
        
        $isVisible = $self->kconfigReadAttributes(\@entryAttributes);
        $self->{wasModified} = 1 unless ($isVisible);
        push(@$outLinesRef, @entryAttributes) if ($isVisible);
    }

    my $endStr = "";
    $endStr = qw/endchoice/     if ($line =~ /^choice\s*$/);
    $endStr = qw/endmenu/       if ($line =~ /^menu\s/);
    $endStr = qw/endif/         if ($line =~ /^if\s/);

    if ($endStr) {
        $hideEndif = 0;
        #special handling for if: we need to modify the condition
        if ($line =~ /^if\s+(.*?)(\s*(?:\#.*$))?$/) {
            my ($condition, $result, $eolComment) = ($self->kconfigSimplifyExprTop($1), $2);
            $eolComment = "" if !defined($eolComment);
            if ($condition ne $1) {   
                # expression changed -- rewriting      
                if ($result eq $N) {
                    $isVisible = 0 ;
                    $self->{wasModified} = 1;
                } elsif ($result eq $Y || $result eq $M) {
                    #pop off if statement (if statement may be multiline...)
                    #print STDERR "condition: ".$condition."    Y:".$Y."\n";
                    while (! (pop(@$outLinesRef) =~ /^if\s/) ) {};
                    $self->{wasModified} = 1;
                    $hideEndif = 1 ;
                } else {                          
                    #pop off if statement (if statement may be multiline...)
                    while (! (pop(@$outLinesRef) =~ /^if\s/) ) {};
                    push(@$outLinesRef, "if ".$condition.$eolComment."\n");
                }
            }
        }
        
        #read block:                
        while(1) {                  
            my @subEntryOutLines = ();
            last if (scalar(@{$self->{inlines}}) <= 0);
            my $isNextEntryVisible = $self->kconfigReadNextEntry(\@subEntryOutLines);
            if ($isNextEntryVisible != 0) {
                push(@$outLinesRef, @subEntryOutLines) if ($isVisible);
            }
            else {
                $self->{wasModified} = 1;
            }
            
            if ($isNextEntryVisible == 2) {
                #Get next entry found endmenu, endchoice or endif.
                #Last line was unshifted back into inLines
                my $closingLine = ( shift @{ $self->{inlines} } );

                if (!defined($closingLine)) {
                    $self->{error} = "Internal Error (1)\n";
                    print STDERR "".$self->{error}."\n" if($showErrs);
                    return;
                }
                if ( $closingLine =~ /^$endStr/) {
                    push(@$outLinesRef, $closingLine) if ($isVisible && !$hideEndif);
                    last;
                }
                else {
                    $self->{error} =  "Unexpected closing line: <".trim($closingLine)."> (expecting <$endStr>)\n";
                    print STDERR "".$self->{error}."\n" if($showErrs);
                    return;
                }   
            }
        }
    }
    return $isVisible;
}

1;

