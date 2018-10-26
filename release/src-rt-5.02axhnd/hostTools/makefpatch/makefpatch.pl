#!/usr/bin/env perl 
use strict;
use warnings;
use FindBin qw($Bin);
use lib "$Bin/../PerlLib";
use BRCM::UnifdefPlus;
use Carp;
use File::Basename;
use File::Find;
use Cwd;
use Cwd 'abs_path';
use File::Spec::Unix;

#use Switch;

sub trim($) {
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}

my $rootDir = ".";
my $featuresListFile;
my $dbg        = 0;
my $scriptsDir = dirname( abs_path($0) );

#my $scriptsDir = getcwd();
die "$scriptsDir incorrect\n" if ( -f $scriptsDir );
my $patchDir = "";
my $tmpDir   = "./_makefpatch_tmp_dir_";
my $featureListFile;
my $shouldIVerify = 0;
my $verifyMakefiles = 0;
my $verifyKconfigs = 0;
my $verifyNew = 0;
my $tarFile;
my $ignoreFilter;
my $runDir            = abs_path(".");
my $bcmKfExceptionStr = "IGNORE_BCM_KF_EXCEPTION";
my $setExpr;
my $doExceptions = 0;


my $usage = q{
Usage: makefpatch [--help] [--dbg] [-FL featureListFile] [-P patchDir] [-T tempDir] [-ignore filter] [[-vm] [-vk] -verify tarfile] dir

options:
  --dbg    turn on verbose debugging
  -FL      featureListFile: specify file with feature list.
  -P       patchDir: directory into which patch files are generated.  If not specified,
           patch files will not be created.
  -t       tempdir: temporary directory where to place temporary files.  Will be deleted
           when execution finishes.  May not be an existing directory.
  -verify  verify result against vanilla kernel.  Tarfile is the original kernel tarfile
  -vm      used with -verify.  Verify will also check makefiles
  -vk      used with -verify.  Verify will also check kconfig files
  -vn      used with -verify.  Verify will also check files which were not in original kernel
  -ignore    ignore: specify filters which can be used to ignore certain features.  This
             is a binary expression using field==value, field!=value, field~value, field~!value
             (where the latter two are true if the field contains value, or does not contain
             value). && and || operators are also understood, but brackets are not.
  -exceptions  tarfile: generates patch for files with IGNORE_BCM_KF_EXCEPTION in them.  This is done after
               all of featurelist has been processed.
  -set     var=val: prior to processing featurelist, generate a patch assuming that a var is
           set to val
  dir      root directory where patchdir will be run

Example: makefpatch -fl featureList.txt -ignore "status~incomplete || distribution==internal" -p ./patchDir kernel/linux3.4rt
};

sub printHelp {
    print $usage;
}

sub resolveStringEscapes {

    #there's likely some beter way of doing this, but I couldn't
    #find it on google, so doing it the ugly way.
    my $string    = shift;
    my $outstring = "";

    while ( $string =~ m/^([^\\]*)(\\)(.)(.*)$/ ) {
        $outstring .= $1;
        if    ( $2 == "n" ) { $outstring .= "\n"; }
        elsif ( $2 == "t" ) { $outstring .= "\t"; }
        else                { $outstring .= $2; }
        $string = $3;
    }
    return $outstring . $string;
}

sub parseFeatureListFile {
    my $filename   = shift;
    my $entriesRef = shift;    #this is a reference to an array of hashes...
    my $idx        = 0;

    open( FEATURESFILE, "<$filename" )
      or die "Could not open $filename for reading ($!)\n";
    while ( my $line = <FEATURESFILE> ) {
        chomp($line);
        if ( $line =~ m/^\s*$/ ) {

            #blank lines will seperate items
            if ( $$entriesRef[$idx]->{name} ) {

                #process entry:
                print "Adding Entry: " . $$entriesRef[$idx]->{name} . "\n"
                  if $dbg;
                $idx++;
            }
        }
        elsif ( $line =~ m/^\s*#/ ) {

            #ignore comments completely
        }
        elsif ( $line =~ m/FEATURE\s*:\s*(\S+)/ ) {
            my $name = $1;
            die "ERROR: Feature already has name! ($name / "
              . $$entriesRef[$idx]->{name} . "\n"
              if ( $$entriesRef[$idx]->{name} );
            print( "parsing feature " . $name . "\n" ) if ($dbg);
            $$entriesRef[$idx]->{name} = $name;
        }
        elsif ( $line =~ m/^\s*(\S+):\s*(\S*.*)/ ) {
            my $key = $1;
            my $val = $2;
            if ( !$val ) {

                #this could be either that the value is blank, or that the
                #user wants to read the next line...
                $val = "";
            }
            elsif ( $val eq "\\" ) {
                $line = <FEATURESFILE>;
                $val  = trim($val);
            }

            trim($val);
            if ( $val =~ m/^"([^\\"]|\\.)*\\*$/ ) {

                # handling multiline quotes:
                while (1) {
                    $val .= " ";
                    die "unterminated \" found in file ("
                      . $$entriesRef[$idx]->{name} . ")\n"
                      if eof FEATURESFILE;
                    $line = <FEATURESFILE>;
                    chomp($line);
                    $line = trim($line);
                    $val .= $line;
                    die "string terminated before end of line\n\t$line\n"
                      if ( $line =~ m/[^\\]".+$/ );
                    last if ( $line =~ m/"$/ );
                    die "Unmatched quote in line: \n\t$line\n"
                      if ( $line !~ m/^([^\\"]|\\.)*\\*$/ );
                }
                $val =~ m/^"(.*)"$/s;
                $val = $1;
            }
            elsif ( $val =~ m/^"(([^\\"]|\\.)*)"$/ ) {

                # handling single line quotes:
                $val = $1;

                #TBD: replace all \" with " and all \\ with \...
            }
            resolveStringEscapes($val);

            $$entriesRef[$idx]->{$key} = $val;
        }
        else {
            die "ERROR: Unparsable line \n\t$line";
        }
    }
    pop(@$entriesRef);
}

sub filterFeatures {
    my $entriesRef = shift;    #this is a reference to an array of hashes...
    my $filterStr = shift;

	my @filters = ( );

	#OK, this is cheesy -- it doesn't respect order of operations, or
	#brackets, etc.  But it's fast to implement, and it fits our current
	#needs

	die "ignore expressions do not accept brackets\n" if $filterStr =~ /[\(\)]/;

	while ($filterStr =~ /^(\w+)\s*([=!~]+)\s*([^\|\&\#\~]+)\s*([\|\&\#\~]*)\s*(.*)$/) {
		my $field=$1;
		my $op=$2;
		my $value=trim($3);
		my $join_op=$4;
		$filterStr = $5;

		die "Invalid expression $filterStr\n" if (!$field || !$op);
		
		push(@filters, { field => $field, op => $op, value => $value, join_op => $join_op });
	}

	my $eIdx;
    my $resultSoFar;
	
    for (my $eIdx=0; $eIdx <= $#{$entriesRef}; $eIdx++) {
    	my $entryRef = $entriesRef->[$eIdx];
    	$resultSoFar = 0;
    	my $join_op = "";
		foreach my $filterEntry (@filters) {
			my $result;

			my $entryValue = $entryRef->{${$filterEntry}{field}};
			$entryValue = "" unless defined $entryValue;

			my $filterValue = ${$filterEntry}{value};
			$filterValue = "" unless defined $filterValue;
			
			if (${$filterEntry}{op} eq "==" || ${$filterEntry}{op} eq "=") {
				$result = ($entryValue eq $filterValue);
			} 
			elsif (${$filterEntry}{op} eq "!=") {
				$result = ($entryValue ne $filterValue);
			} 
			elsif (${$filterEntry}{op} eq "~" ||  ${$filterEntry}{op} eq "~=") {
				$result = ($entryValue =~ /($filterValue)/ );
			} 
			elsif (${$filterEntry}{op} eq "!~" || ${$filterEntry}{op} eq "~!") {
				$result = !($entryValue =~ /($filterValue)/ );
			} 
			else {
				die "Invalid ignore expression\n"
			}
			

			if ($join_op eq "||") {
				$resultSoFar = $resultSoFar || $result;
			}
			elsif ($join_op eq "&&") {
				$resultSoFar = $resultSoFar && $result;
			}
			else {
				$resultSoFar = $result;
			}
			$join_op = ${$filterEntry}{join_op}
		}
		splice(@$entriesRef, $eIdx--, 1) if ($resultSoFar);
	}
}

my $numArgs = 0;

# Parse passed in arguments:
while ( my $arg1 = shift ) {
    $numArgs++;

    if ( $arg1 =~ m/^--*h/i || $arg1 =~ /^-*\?/ ) {
        printHelp;
        exit 0;
    }
    elsif ( $arg1 =~ m/^--dbg$/ ) {
        $dbg = 1;
    }
    elsif ( $arg1 =~ m/^-[Ff][Ll](.*)$/ ) {
        $featureListFile = $1;
        $featureListFile = shift if ( !$featureListFile );

    }
    elsif ( $arg1 =~ m/^-[Pp](.*)/ ) {
        $patchDir = $1;
        $patchDir = shift if ( !$patchDir );
        die "Bad patchdir ($patchDir)\n" if ( !$patchDir );
        `mkdir $patchDir` if ( !-e $patchDir );
        die "Could not create patch directory: ($patchDir)\n"
          if ( !-e $patchDir );
        $patchDir = `cd "$patchDir"; pwd;`;
        chomp $patchDir;
        die "Bad patchDir 2 ($patchDir)\n" if ( !$patchDir || !-e $patchDir );
    }
    elsif ( $arg1 =~ m/^-[Tt](.*)/ ) {
        $tmpDir = $1;
        $tmpDir = shift if ( !$tmpDir );
        die
          "tmpDir already exists -- delete or specify another one! ($tmpDir) \n"
          if ( !$tmpDir || -e $tmpDir );
        `mkdir $tmpDir`;
        $tmpDir = `cd "$tmpDir"; pwd;`;
        chomp $tmpDir;
        die "Bad $tmpDir ($tmpDir)\n" if ( !$tmpDir || !-e $tmpDir );
    }
    elsif ( $arg1 =~ m/^-verify/i ) {
        $shouldIVerify = 1;
        $tarFile       = abs_path(shift);
        die "verify must be followed by proper tar file name\n"
          if ( !$tarFile || !( -e $tarFile ) );
    }
    elsif ( $arg1 =~ m/^-vk$/ ) {
        $verifyKconfigs = 1;
    }
    elsif ( $arg1 =~ m/^-vn$/ ) {
        $verifyNew = 1;
    }
    elsif ( $arg1 =~ m/^-vm$/ ) {
        $verifyMakefiles = 1;
    }
    elsif ( $arg1 =~ m/^-ignore/i ) {
        $ignoreFilter = shift;
        die "ignore must be followed by expression\n"
          if ( !$ignoreFilter );
    }
    elsif ( $arg1 =~ m/^-set$/i ) {
        $setExpr = shift;
        die "set must be followed by expression\n"
          if ( !$setExpr || $setExpr !~ /=/ );
    }
    elsif ( $arg1 =~ m/^-ex*/i ) {
        $doExceptions = 1;
        $tarFile       = abs_path(shift);
        die "verify must be followed by proper tar file name\n"
          if ( !$tarFile || !( -e $tarFile ) );
    }
    else {
    	$rootDir = abs_path($arg1);
        print "Setting rootDir to $rootDir\n" if ($dbg);
    	die "file not found: $rootDir\n" if (!-e $rootDir );
    }
}

if ( $numArgs == 0 ) {
    printHelp;
    exit 0;
}

$rootDir = abs_path($rootDir);
chomp $rootDir;
die "Bad rootDir ($rootDir)\n" if ( !$rootDir || !-e $rootDir );

my @allFiles;
my @tmp1Files;

sub getP4Files {
    my $dir         = shift;
    my $p4files_ref = shift;

    # figure out p4 depot to main mapping:
    my @p4Mapping = split /\s+/, `p4 where $dir`
      ; #TBD: this doesn't account for whitespaces at end of sub directory names...
        #TBD: handle errors here...
        #I beleive that p4 adds
    my $p4DepotDir  = $p4Mapping[0];
    my $p4ClientDir = $p4Mapping[2];
    my @files;

    die "ERROR: could not read p4 dir" unless ($p4ClientDir);

    my @allP4files = split /\n/, `p4 files $dir/...`;

    #TBD: handle p4 error here...
    print "P4 Files are:\n" if ($dbg);
    foreach my $p4File (@allP4files) {

        #TBD: bug here: this does not like files with two digit revisions...
        if (   $p4File =~ m!^(//depot/[^\#]+)\#([0-9]+)\s+\-\s+(\S+)!
            && $2 > 1
            && $3 ne "delete" )
        {
            my $file = $1;
            $file =~ s!^$p4DepotDir!$p4ClientDir!;
            print "\t$file\n" if ($dbg);
            push( @$p4files_ref, $file );
            die "Cound not find $file, $1" if ( !-e $file );
        }
    }
    print "\n" if ($dbg);
}

#Hmmm... doesn't seem to work...
#sub addFile {
#  push @allFiles, $File::Find::name;
#  return;
#}
#sub getAllFiles {
#   my $rootDir = shift;
#   find( \&addFile, $rootDir);
#}

sub getAllFiles {
    @allFiles = split( /\n/, `find $rootDir -type f` );
}

sub getFilesWithStrings {
    my $stringList = shift;
    my $fileList   = shift;
    my $dir        = shift;

    $stringList =~ s/=[^.]*//g;
    $stringList =~ s/\./\\|/g;

    print "getFilesWithStrings $stringList in $dir\n" if ($dbg);

    @$fileList = ();
    @$fileList =
      split( /\n/, `find $dir -type f -exec grep -l \"$stringList\" {} \\;` );
}

#################################################################
# setFeatures:
#	Go through all of the files in a list, and call unifdef+/unmakefile/unkconfig
#   to set/remove all features.
#
#in:  featureList: dot delimited list of features
#     files_ref: reference to array of files.  Files are complete file names
#     modifiedfiles_ref: reference to array where to store a list of all modified files
sub setFeatures {
    my $param1            = shift;
    my $files_ref         = shift;
    my $modifiedfiles_ref = shift;

    my @featureList = split( /\./, $param1 );

    my $file;

    #empty the modified files array
    undef @$modifiedfiles_ref;

    print "\n\nsetFeatures: $param1\n" if $dbg;
    foreach my $tmp (@featureList) {
        print "  tmp=$tmp\n" if $dbg;
    }

    # set command args (assume all tools have same command line args for now)
    my $cmdArgs = "";
    my $featureStr;
    my %defines;
    my %undefines;
    foreach $featureStr (@featureList) {
        if ( $featureStr =~ m/^([^=]*)=(.*)/ ) {
            $cmdArgs .= "-D $1=$2 ";
            $defines{$1} = $2 || 'y';
        }
        else {
            $cmdArgs .= "-U $featureStr ";
            $undefines{$featureStr} = 'X';
        }
    }
    my $kconfigCmdArgs = $cmdArgs;
    $kconfigCmdArgs =~ s/-D \S+ //;

    my $unif = new BRCM::UnifdefPlus(
        defines   => \%defines,
        undefines => \%undefines
    );
    foreach $file (@$files_ref) {
        my $INFILE;
        my $OUTFILE;
        my $lang;
        if ( $file =~ m/\.[chS]$/ ) {
            $lang = 'C';
        }
        elsif ( $file =~ m/Makefile|\S+\.mak/ ) {
            $lang = 'Makefile';
        }
        elsif ( $file =~ m/Kconfig.*/ ) {
            $lang = 'Kconfig';
        }
        else {
            $lang = '';
        }

        if ( $lang) {
            print "\tprocessing $file (lang=$lang)\n" if ($dbg);
            my $tfile = $file . ".makefpatch.tmp";
            open( $INFILE,  "<", $file )  or die($!);
            open( $OUTFILE, ">", $tfile ) or die($!);
            my $wasModified = $unif->parse(
                INFILE  => $INFILE,
                OUTFILE => $OUTFILE,
                lang    => $lang,
                dbg     => $dbg,
            );
            close($INFILE);
            close($OUTFILE);
            if ($wasModified) {
                `cp -f $tfile $file`;
                print "\tDetected modified file $param1: $file\n" if ($dbg);
                push( @$modifiedfiles_ref, $file );
            }
            if ($unif->{error})
            {
                print STDERR "Error detected in $file: " . $unif->{error} . "\n";
                $unif->{error} = "";
            }
            `rm $tfile`;
        }
        else {
            print" \tignoring $file (unknown filetype)\n" if ($dbg);
        }

    }
    return scalar(@$modifiedfiles_ref);
}

#################################################################
# generatePatchFile:
#     generate a patch file between two directories
#
#in:
#     modifiedfiles_ref: reference to array which lists all modified files
#     oldDir: directory to find original files
#     newDir: directory where to find modified files
#     patchFile: name of patch file (full file name)
sub generatePatchFile {
    my $modifiedfiles_ref = shift;
    my $oldDir            = shift;
    my $modDir            = shift;
    my $patchFile         = shift;

    my $file;
    my $oldFile;
    my $modFile;

    print "Generating patch file $patchFile\n" if ($dbg);
    `echo "" > $patchFile`;

    foreach $modFile (@$modifiedfiles_ref) {
        die "file not in newDir ($file, $modDir)\n"
          if ( !( $modFile =~ /^$modDir/ ) );
        my $oldFile = $modFile;
        $oldFile =~ s!^$modDir!$oldDir!;
        die "Missing Files: $oldFile" if ( !-e $oldFile );
        die "Missing new File: $modFile"
          if ( !-e $modFile )
          ;    #TBD: mod file might have been deleted... see patch -E
               #print STDOUT "XXXXX\n";
        my $rel_oldFile = "./" . File::Spec::Unix->abs2rel( $oldFile, $runDir );
        my $rel_modFile = "./" . File::Spec::Unix->abs2rel( $modFile, $runDir );

        #my $rel_oldFile = $oldFile;
        #$rel_oldFile =~ s/^$runDir\///;
        #my $rel_modFile = $modFile;
        #$rel_modFile =~ s/^$runDir\///;

        print STDOUT
          "cd $runDir/; diff -U 5 $rel_modFile $rel_oldFile >> $patchFile\n"
          if ($dbg);
        die
"files do not belong to rootDir ($rel_oldFile, $rel_modFile, $rootDir)\n"
          if ( ( !$rel_oldFile ) || ( !$rel_modFile ) );
        die "oldFile: $oldFile does not exist\n" if ( !-e $oldFile );
        die "modFile: $modFile does not exist\n" if ( !-e $modFile );
        die "rel_oldFile: $rel_oldFile does not exist\n"
          if ( !-e $rel_oldFile );
        die "rel_modFile: $rel_modFile does not exist\n"
          if ( !-e $rel_modFile );

        `cd $runDir; diff -U 5 $rel_modFile $rel_oldFile  >> $patchFile`;

        #`diff -u $oldFile $modFile >> $patchFile`;
    }
}

#################################################################
# doFeature:
#     takes in one or more features, removes the features from
#     the directory (but backs up original files first), and then
#     generates a patch file.
#     Also updates METADATAFILE with informatiation on feature
#
#in:  featureNames: a colon delimitted list of feature names.
#     featureNum: a number used in the patch file name generation.
#     modifiedfiles_ref: reference to array which lists all modified files
#     modDir: directory where files will be modifed from
#     bkDir: directory to back original files to
#     patchDir: directory where to put patch file (relative to $rootDir)

sub doFeature {

    my $featureNames = shift;
    my $featureNum   = shift;
    my $modDir       = shift;
    my $bkDir        = shift;
    my $patchDir     = abs_path shift;

    $patchDir = abs_path $patchDir if ($patchDir);

    my @featureFiles;
    my @modifiedFiles;

    #have to remove CONFIG_ so kconfig files match...:
    my $searchNames = $featureNames;
    $searchNames =~ s/^CONFIG_//g;
    $searchNames =~ s/([ ,])CONFIG_/$1/g;    

    getFilesWithStrings( $searchNames, \@featureFiles, $modDir );
    print "  > " . join( '\n  > ', @featureFiles ) if ($dbg);

    backupFiles( \@featureFiles, $modDir, $bkDir );

    print "Processing feature set: $featureNames: \n";
    print METADATAFILE "\nFEATURE: $featureNames\n";
    setFeatures( $featureNames, \@featureFiles, \@modifiedFiles );

    if ($patchDir) {
        my $patchFile =
            "$patchDir/BCM_KF_PATCH."
          . sprintf( "%04d", $featureNum )
          . ".$featureNames.patch";
        generatePatchFile( \@modifiedFiles, $bkDir, $modDir, $patchFile );
    }

    foreach my $file (@modifiedFiles) {
        print METADATAFILE "\t$file\n";
    }
}

sub backupFiles {
    my $fileList_ref = shift;
    my $rootDir      = shift;
    my $bkDir        = shift;

    my $file;
    my $bkFile;
    my $bkDirAbs;

 #my $numFiles = scalar(@$fileList_ref);
 #print "backupFiles $rootDir to $bkDir (" .  $numFiles . " files)\n" if ($dbg);

    $bkDir = abs_path($bkDir);

    `rm -rf $bkDir`;
    foreach $file (@$fileList_ref) {
        $bkFile = $file;
        $bkFile =~ s|$rootDir|$bkDir|;
        $bkDirAbs = dirname($bkFile);
        `mkdir -p $bkDirAbs; cp $file $bkFile`;
    }
}

sub doAllFeatures {

    my $feature;    #hash
    my $featureNum = 1;
    my $bkDir      = abs_path "$rootDir/../._makefpatch_temp_bk_dir_";

    my @featureList;    #array of hashes
    parseFeatureListFile( $featureListFile, \@featureList );

    if ($ignoreFilter) {
        filterFeatures(\@featureList, $ignoreFilter); 
    }
    
    print("rootDir is $rootDir\n") if ($dbg);
    getAllFiles($rootDir);    #populates allFiles array

    my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) =
      localtime(time);
    $year += 1900;
    my @month_abbr = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
    my $metaDataFileName = "$rootDir/metafile.txt";

    open( METADATAFILE, ">$metaDataFileName" )
      or die "Could not open $metaDataFileName for writing ($!)\n";
    print METADATAFILE "METADATA FILE for $rootDir: \n";
    print METADATAFILE
      "Built on: $month_abbr[$mon] $mday, $year  $hour:$min\n\n";
    print METADATAFILE "FEATURES: \n";
    foreach $feature (@featureList) {
        print METADATAFILE "  $feature->{name}\n";
    }
    print METADATAFILE "\n\n";

    if ($setExpr) {
        doFeature( $setExpr, scalar(@featureList)+1, $rootDir, $bkDir, $patchDir );
    }
    
    $featureNum = scalar(@featureList);
    foreach $feature ( reverse(@featureList) ) {
        doFeature( $feature->{name}, $featureNum, $rootDir, $bkDir, $patchDir );
        $featureNum--;
    }

    if ($doExceptions) {
        my $linuxDir=basename($rootDir);
        my @files=(`cd $rootDir; grep -rl $bcmKfExceptionStr`);
        s/^\s+|\s+$//g foreach @files;
        my @tarFiles=@files;
        s/^/$linuxDir\//g foreach @tarFiles;
        print "Generating BCM_KF_PATCH.0000.EXCEPTIONS.patch for: @files\n";
        # extract vanilla files into temporary directory:
        system("rm -rf $patchDir/_tmpdir; mkdir $patchDir/_tmpdir; pushd $patchDir/_tmpdir && tar -xf $tarFile @tarFiles && popd");
        system("cd $rootDir; echo > $patchDir/BCM_KF_PATCH.0000.EXCEPTIONS.patch");
        # diff each file against temporary directory:
        system("cd $rootDir; for f in @files; do diff -U 5 $patchDir/_tmpdir/$linuxDir/\$f \$f  >> $patchDir/BCM_KF_PATCH.0000.EXCEPTIONS.patch; done");
        system("rm -rf $patchDir/_tmpdir");
    }

    close(METADATAFILE);

    if ($shouldIVerify) {
        testAgainstVanilla( $tarFile, abs_path("$rootDir/../vanillaFiles") );
    }

}

my @fileList;

sub testAgainstVanilla {
    my $lTarFile   = shift;
    my $vanillaDir = shift;
    my $file;
    my $shouldIDie = 0;

    my $rootDirBk = abs_path("$rootDir/../._rootDirBk");

    die "$vanillaDir already exists\n" if ( -e $vanillaDir );
    `mkdir $vanillaDir`;
    die "could not create $vanillaDir\n" unless ( -e $vanillaDir );

    print "Untarring Vanilla Kernel Files...\n";

    my ( $tarFileName, $tarFileDir ) = fileparse($lTarFile);

    print "testAgainstVanilla: rootDir $rootDir\n"       if ($dbg);
    print "testAgainstVanilla: tarFileDir $tarFileDir\n" if ($dbg);
    print "testAgainstVanilla: vanillaDir $vanillaDir\n" if ($dbg);

    #my @modFiles = `cd $rootDir; find . -type f;`;

    #foreach $file (@modFiles) {
    #	$file = abs_path($file);
    #	chomp($file);
    #	$file =~ s/^$tarFileDir//;
    #}

    #my $modFilesStr = join(" ",@modFiles);

    print "testAgainstVanilla: cd $rootDir; find . -type f;\n\n" if ($dbg);
    my $modFilesStr = `cd $rootDir; find . -type f;`;

    #$modFilesStr =~ s/\R/ /g;
    my @modFiles = split( /[\r\n]+/, $modFilesStr );

    # convert modfiles to hold names relative to tarfile
    foreach $file (@modFiles) {
        $file = abs_path( $rootDir . "/" . $file );
        $file =~ s/^$tarFileDir(.*)/${1}/;
    }

    $modFilesStr = join( " ", @modFiles );

    #TBD: replace spaces in filenames with '\ '
    `rm -rf $rootDirBk`;
    print "mv $rootDir $rootDirBk\n" if ($dbg);
    `mv $rootDir $rootDirBk`;
    print "ls $rootDirBk is " . `ls $rootDirBk` . "\n" if ($dbg);

    print "mkdir $rootDir\n" if ($dbg);
    `mkdir $rootDir`;

    print "cd $tarFileDir; tar -xf $tarFile $modFilesStr\n" if ($dbg);
    `cd $tarFileDir; tar -xf $lTarFile $modFilesStr 2>/dev/null`;
    print "ls $rootDir is " . `ls $rootDir` . "\n" if ($dbg);

    print "mv $rootDir/* $vanillaDir\n" if ($dbg);
    `mv $rootDir/* $vanillaDir`;
    print "ls $vanillaDir is " . `ls $vanillaDir` . "\n" if ($dbg);

    print "mv $rootDirBk/* $rootDir\n" if ($dbg);
    `mv $rootDirBk/* $rootDir`;
    print "ls $rootDir is " . `ls $rootDir` . "\n"       if ($dbg);
    print "ls $vanillaDir is " . `ls $vanillaDir` . "\n" if ($dbg);

    `rm -rf $rootDirBk`;

    print "  ... done untarring files\n";

    my @vanillaFiles = `find $vanillaDir -type f;`;
    my $vanillaFile;

    foreach $vanillaFile (@vanillaFiles) {
        chomp($vanillaFile);
        my $strippedFile = $vanillaFile;

        $strippedFile =~ s/^$vanillaDir/$rootDir/;

        print "processing $vanillaFile\n"  if ($dbg);
        print "processing $strippedFile\n" if ($dbg);
        die "error strip and vanilla files are the same file!\n"
          if ( $strippedFile eq $vanillaFile );
        if (     $strippedFile =~ m/\.[chS]$/ 
              || ($verifyMakefiles && $strippedFile =~ m/Makefile|\S+\.mak/) 
              || ($verifyMakefiles && $strippedFile =~ m/Kconfig.*/) ) {
            print
              "diff -wBbaNq -I \"^[[:blank:]]*\$\" $vanillaFile $strippedFile\n"
              if ($dbg);
            #If an #ifdef gets simplified, the #ifdef gets turned into an #if defined
            #which kills the diff.   The simple solution: replace all #ifdef's with 
            #if defined's....  


            `sed -i "s|^#ifdef\\s\\+\\(\\w\\+\\)\\b|#if defined(\\1)|g" $vanillaFile`;
            `sed -i "s|^#ifdef\\s\\+\\(\\w\\+\\)\\b|#if defined(\\1)|g" $strippedFile`;
            `sed -i "s|^#ifndef\\s\\+\\(\\w\\+\\)\\b|#if !defined(\\1)|g" $vanillaFile`;
            `sed -i "s|^#ifndef\\s\\+\\(\\w\\+\\)\\b|#if !defined(\\1)|g" $strippedFile`;

            #`sed -i "s|\\bdefined\\s\\+\\(\\w\\+\\)\\b|defined(\\1)|g" $vanillaFile`;
            #`sed -i "s|\\bdefined\\s\\+\\(\\w\\+\\)\\b|defined(\\1)|g" $strippedFile`;

            `diff -wBbaNq -I \"^[[:blank:]]*\$\" $vanillaFile $strippedFile`;
            if ($?) {
                `grep  $bcmKfExceptionStr $strippedFile`;
                if ($?) {

                    #uh oh... Warning time....
                    print
"+---------------------------------------------------------\n";
                    print
"| ERROR: Stripped kernel file differs from vanilla file!!!\n";
                    print "|   $strippedFile\n";
                    print
"+---------------------------------------------------------\n\n";
                    print print
"diff -wBbaN -I \"^[[:blank:]]*\$\" $vanillaFile $strippedFile\n\n";
                    print
`diff -wBbaN -I \"^[[:blank:]]*\$\" $vanillaFile $strippedFile`;
                    print
"----------------------------------------------------------\n\n\n";
                    $shouldIDie = 1;
                }
                else {
                    print
"Ignoring $strippedFile due to presence of $bcmKfExceptionStr in file\n";
                }
            }
        }
        elsif ( $strippedFile =~ m/Makefile|\S+\.mak/ ) {
            print "Not processing $strippedFile -- Makefiles not verified\n"
              if ($dbg);
        }
        else {
            print "Not processing $strippedFile -- unknown type\n" if ($dbg);
        }
    }

    # check that all new files are blank:
    if ($verifyNew) {    
        print "Testing new files\n" if ($dbg);

        `find $vanillaDir -type f | sed \"s|^$vanillaDir/||\" | sort > _v.txt`;
        `find $rootDir -type f | sed \"s|^$rootDir/||\" | sort > _r.txt`;
        my @newFiles = `comm -13 _v.txt _r.txt`;
        `rm _v.txt _r.txt`;
        my $newFile;
        
        foreach $newFile (@newFiles) {
            if (     $newFile =~ m/\.[chS]$/ 
                  || ($verifyMakefiles && $newFile =~ m/Makefile|\S+\.mak/) 
                  || ($verifyMakefiles && $newFile =~ m/Kconfig.*/) ) {

                print "grep -q \"\\S\" $rootDir/$newFile" if ($dbg);
                `grep -q \"\\S\" $rootDir/$newFile`;
                if ($? eq 0) {
                    print
"+---------------------------------------------------------\n";
                    print
"| ERROR: Stripped new file is not blank!!!\n";
                    print "|   $newFile\n";
                    print
"+---------------------------------------------------------\n\n";
                    $shouldIDie = 1;
                }
            }
            else {
                print "Ignoring new file $newFile\n" if ($dbg);
            }
        }
    }
    
    die
"KERNEL FILES DIFFER FROM VANILLA FILES AFTER BCM_KF FEATURES STRIPPED OUT!\n"
      if $shouldIDie;

    print "\n";
    print "PASS: Stripped Broadcom kernel files match vanilla kernel files\n\n";
    #`rm -rf $vanillaDir`;

}

sub test1 {
    my @entries;
    parseFeatureListFile( "./test.txt", \@entries );

    foreach my $entryRef (@entries) {
        if ( $entryRef->{name} ) {

            print "FEATURE: " . $entryRef->{name} . "\n" if $dbg;

            for my $fieldName ( keys %$entryRef ) {
                print "   "
                  . $fieldName . " => "
                  . $entryRef->{$fieldName} . "\n"
                  if $dbg;
            }
        }
        else {
            pop(@$entryRef);
        }
    }
}

die "no rootDir specified" unless ($rootDir);
die "no feature list file" unless ($featureListFile);

#`rm -rf $rootDir/../vanillaFiles`;
#testAgainstVanilla(	abs_path("$rootDir/../linux3.2rt_sandbox.tar.bz2"), abs_path("$rootDir/../vanillaFiles"));

doAllFeatures;
exit 0;

