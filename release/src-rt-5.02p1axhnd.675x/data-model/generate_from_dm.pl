#!/usr/bin/perl -w 

#***********************************************************************/
# <:copyright-BRCM:2007:proprietary:standard
#
#    Copyright (c) 2007 Broadcom 
#    All Rights Reserved
#
#  This program is the proprietary software of Broadcom and/or its
#  licensors, and may only be used, duplicated, modified or distributed pursuant
#  to the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied), right
#  to use, or waiver of any kind with respect to the Software, and Broadcom
#  expressly reserves all rights in and to the Software and all intellectual
#  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
#  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
#  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1. This program, including its structure, sequence and organization,
#     constitutes the valuable trade secrets of Broadcom, and you shall use
#     all reasonable efforts to protect the confidentiality thereof, and to
#     use this information only in connection with your use of Broadcom
#     integrated circuit products.
#
#  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
#     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
#     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
#     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
#     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
#     PERFORMANCE OF THE SOFTWARE.
#
#  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
#     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
#     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
#     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
#     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
#     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
#     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
#     LIMITED REMEDY.
# :>


#
# This script reads the cms-data-model in XML format.
# It then generates various .h and .c files that represent the data model in the
# c programming language that can be compiled into libmdm.so
# 
# Usage: See the usage function at the bottom of this file.
#
#

use FindBin qw($Bin);
use lib "$Bin";
use strict;
use GenObjectNode;
use GenParamNode;
use Utils;


###########################################################################
#
# This first section contains helper functions.
#
###########################################################################


my $spreadsheet1Done = 0;
my $savedRowHashValid = 0;
my %savedRowHash;
my $seqOid;
my $parentLockZone = 0;
my $needRealObjNode;
my %oidToNameHash;
my %oidToObjNameHash;
my %reportProfileHash;
my $pure_device2 = 0;


my $rootObjRef;
my %profileToValidStringsHash;
my %profileToFileHandleHash;
my %allValidStrings;
my @allVSArefs;
my @objProfilesArray;
my @leafProfilesArray;
my @forcedLeafProfilesArray = ("X_BROADCOM_COM_Gpon:1", "X_BROADCOM_COM_IPv6:1", "Device2_SM_Baseline:1");
my @section3ProfilesArray;



sub parse_object_line
{
    my ($hashref, $line) = @_;

    # print "parsing object line: $line";

    # remove all entries from hashref
    %{$hashref} = ();


    ${$hashref}{type} = "object";

    $line =~ /name="([\w.{}-]+)"/;
    ${$hashref}{name} = $1;
    
    if ($pure_device2)
    {
        ${$hashref}{name} =~ s/^InternetGatewayDevice\./Device\./;
        ${$hashref}{name} =~ s/^Device\.Device\./Device\./;
    }

    $line =~ /shortObjectName="([\w]+)"/;
    ${$hashref}{shortObjectName} = $1;

    $line =~ /specSource="([\w]+)"/;
    ${$hashref}{specSource} = $1;

    $line =~ /profile="([\w:]+)"/;
    ${$hashref}{profile} = $1;
    if (!(defined($1))) {
       die "could not extract profile for $line";
    }

    if ($line =~ /requirements="([\w]+)"/)
    {
        ${$hashref}{requirements} = $1;
    }
    
    $line =~ /supportLevel="([\w]+)"/;
    ${$hashref}{supportLevel} = $1;

    if ($line =~ /hideObjectFromAcs="([\w]+)"/)
    {
        ${$hashref}{hideObjectFromAcs} = $1;
    }

    if ($line =~ /alwaysWriteToConfigFile="([\w]+)"/)
    {
        ${$hashref}{alwaysWriteToConfigFile} = $1;
    }
    
    if ($line =~ /pruneWriteToConfigFile="([\w]+)"/)
    {
        ${$hashref}{pruneWriteToConfigFile} = $1;
    }
    
    if ($line =~ /autoOrder="([\w]+)"/)
    {
        ${$hashref}{autoOrder} = $1;
    }

    if ($line =~ /callRclPreHook="([\w]+)"/)
    {
        ${$hashref}{callRclPreHook} = $1;
    }

    if ($line =~ /callRclPostHook="([\w]+)"/)
    {
        ${$hashref}{callRclPostHook} = $1;
    }

    if ($line =~ /callStlPostHook="([\w]+)"/)
    {
        ${$hashref}{callStlPostHook} = $1;
    }

    if ($line =~ /majorVersion="([\d-]+)"/)
    {
        ${$hashref}{majorVersion} = $1;
    }
    
    if ($line =~ /minorVersion="([\d-]+)"/)
    {
        ${$hashref}{minorVersion} = $1;
    }
    
    if ($line =~ /oid="([\d]+)"/)
    {
        ${$hashref}{oid} = $1;
    }

    if ($line =~ /lockZone="([\d]+)"/)
    {
        ${$hashref}{lockZone} = $1;
    }
}

sub parse_parameter_line
{
    my ($hashref, $line) = @_;

#    print "parsing param line: $line";

    # remove all entries from hashref
    %{$hashref} = ();


    $line =~ /name="([\w.{}-]+)"/;
    ${$hashref}{name} = $1;

    if ($line =~ /AlternateParamName="([\w.{}-]+)"/)
    {
        ${$hashref}{AlternateParamName} = $1;
        #print "\n--->$line has $1\n";
    }
    $line =~ /type="([\w]+)"/;
    ${$hashref}{type} = $1;

    $line =~ /specSource="([\w]+)"/;
    ${$hashref}{specSource} = $1;

    $line =~ /profile="([\w:]+)"/;
    ${$hashref}{profile} = $1;

    if ($line =~ /requirements="([\w]+)"/)
    {
        ${$hashref}{requirements} = $1;
    }

    $line =~ /supportLevel="([\w]+)"/;
    ${$hashref}{supportLevel} = $1;

    if ($line =~ /mayDenyActiveNotification="([\w]+)"/)
    {
        ${$hashref}{mayDenyActiveNotification} = $1;
    }
    
    if ($line =~ /denyActiveNotification="([\w]+)"/)
    {
        ${$hashref}{denyActiveNotification} = $1;
    }

    if ($line =~ /forcedActiveNotification="([\w]+)"/)
    {
        ${$hashref}{forcedActiveNotification} = $1;
    }

    if ($line =~ /alwaysWriteToConfigFile="([\w]+)"/)
    {
        ${$hashref}{alwaysWriteToConfigFile} = $1;
    }

    if ($line =~ /neverWriteToConfigFile="([\w]+)"/)
    {
        ${$hashref}{neverWriteToConfigFile} = $1;
    }

    if ($line =~ /countPersistentForConfigFile="([\w]+)"/)
    {
        ${$hashref}{countPersistentForConfigFile} = $1;
    }

    if ($line =~ /transferDataBuffer="([\w]+)"/)
    {
        ${$hashref}{transferDataBuffer} = $1;
    }

    if ($line =~ /isTr69Password="([\w]+)"/)
    {
        ${$hashref}{isTr69Password} = $1;
    }

    if ($line =~ /isConfigPassword="([\w]+)"/)
    {
        ${$hashref}{isConfigPassword} = $1;
    }

    if ($line =~ /hideParameterFromAcs="([\w]+)"/)
    {
        ${$hashref}{hideParameterFromAcs} = $1;
    }

    if ($line =~ /maxLength="([\d]+)"/)
    {
        ${$hashref}{maxLength} = $1;
    }

    if ($line =~ /minValue="([\d-]+)"/)
    {
        ${$hashref}{minValue} = $1;
    }

    if ($line =~ /maxValue="([\d-]+)"/)
    {
        ${$hashref}{maxValue} = $1;
    }

    if ($line =~ /validValuesArray="([\w]+)"/)
    {
        ${$hashref}{validValuesArray} = $1;
    }
    
    if ($line =~ /majorVersion="([\d-]+)"/)
    {
        ${$hashref}{majorVersion} = $1;
    }
    
    if ($line =~ /minorVersion="([\d-]+)"/)
    {
        ${$hashref}{minorVersion} = $1;
    }
    
    if ($line =~ /notifySskLowerLayersChanged="([\w]+)"/)
    {
        ${$hashref}{notifySskLowerLayersChanged} = $1;
    }
    
    if ($line =~ /notifySskAliasChanged="([\w]+)"/)
    {
        ${$hashref}{notifySskAliasChanged} = $1;
    }
    
    if ($line =~ /autoGenerateAlias="([\w]+)"/)
    {
        ${$hashref}{autoGenerateAlias} = $1;
    }

    if ($line =~ /defaultValue=/)
    {
        ${$hashref}{defaultValueWasSet} = "true";
    }
    
    if ($line =~ /defaultValue="([\%\w\s.\/:,+\(\)\[\]\*=\@|-]+)"/)
    {
        ${$hashref}{defaultValue} = $1;
    }
    else
    {
        # we must have a defaultValue, so just set to NULL
        ${$hashref}{defaultValue} = "NULL";
    }
}


sub parse_description_line
{
    my ($hashref, $line) = @_;

#    print "parsing description line: $line";

    # remove all entries from hashref
    %{$hashref} = ();

    ${$hashref}{type} = "description";

    $line =~ /source="([\w]+)"/;
    ${$hashref}{source} = $1;
    
    ${$hashref}{desc} = $line;
}


#
# Parse a line in section 1 of the data model file.
# Fill in the given reference to a hash with attributes found in the line.
#
sub parse_row
{
    my $inputFileRef = $_[0];
    my $hashref = $_[1];

#    print "starting parse_row (spreadsheet1Done=$spreadsheet1Done) \n";
    if ($spreadsheet1Done == 1)
    {
        return 0;
    }

    while (1)
    {
        $_ = <$inputFileRef>;

        if (/[\w]*<vsaInfo>/)
        {
#            print "Begin vsaInfo tag found, end parse_row\n";
            $spreadsheet1Done = 1;
            return 0;
        }

        
        if (!defined($_)) {
            die "ERROR: end of file detected during parse_row!\n";
        }

#        if (/[\w]*<\/xmlMandatorySingleRootNode>/) {
#            print "end tag detected $_";
#            return 0;
#        }

        #
        # Call appropriate functions to parse object, parameter, and 
        # description blocks.  This code assumes everything is on a single
        # line.  This assumption may break, especially for descriptions.
        #
        if (/[\w]*<object/)
        {
            parse_object_line($hashref, $_);
            return 1;
        }

        if (/[\w]*<parameter/)
        {
            parse_parameter_line($hashref, $_);
            return 1;
        }
        
        if (/[\w]*<description/)
        {
            parse_description_line($hashref, $_);
            return 1;
        }
    }


#
# These verify that all fields needed to generate the MDM tree
# can be read in.
#
#  print "${$hashref}{\"Name\"} ${$hashref}{\"Type\"} ${$hashref}{\"ValidatorData\"} ";
#  print "${$hashref}{\"DefaultValue\"} ${$hashref}{\"BcmInitialValue\"} ${$hashref}{\"BcmSuggestedValue\"} ";
#  print "${$hashref}{\"TRxProfile\"} ${$hashref}{\"NotificationFlags\"} ${$hashref}{\"BcmFlags\"} ";
#  print "${$hashref}{\"BcmAbbreviatedName\"} \n";

    die "impossible!  reached end of parse_row";
}


#
# Parse a row in the Valid Values section (aka spreadsheet 2)
#
sub parse_row_vv
{
    my $inputFileRef = $_[0];
    my $hashRef = $_[1];
    my $rowMarkerFound = 0;
    my ($currData, $currProfile);
    my $gotLine=0;

    %{$hashRef} = ();

#    print "starting parse_row_vv\n";

    while ($gotLine == 0)
    {
         $_ = <$inputFileRef>;

        if (!defined($_)) {
            die "ERROR: end of file detected before end vsaInfo\n";
        }
        
        if (/[\w]*<\/vsaInfo/)
        {
#            print "End vsaInfo tag found.\n";
            return 0;
        }


        if (/[\w]*<validstringarray/)
        {
            /name="([\w]+)"/;
            ${$hashRef}{"name"} = $1;
#            print "Got vsa=${$hashRef}{name}\n";

            if (/prefix="[\w]+"/)
            {
               /prefix="([\w]+)"/;
               ${$hashRef}{"prefix"} = $1;
#               print "parse: Got prefix=${$hashRef}{prefix}\n";
            }

            # the profile is just a marker for beginning of vsa
            ${$hashRef}{"profile"} = "Baseline";

            return 1;
        }

        if (/[\w]*<element/)
        {
            />([\w\s._-]+)<\/element/;
            if (defined($1))
            {
                ${$hashRef}{"name"} = $1;
            }
            else
            {
                ${$hashRef}{"name"} = "";
            }
#            print "     name=${$hashRef}{name}\n";

            return 1;
        }
    }

    die "impossible!  reached end of parse_row_vv";
}


#
# If no short object name was supplied, then
# just clean up the fully qualified (generic) object name
# and make that the MDMOBJID_name
#
sub convert_fullyQualifiedObjectName
{
    my $name = $_[0];

#    print "Converting $name\n";

    # get rid of trailing dot
    $name =~ s/\.$//;

    # get rid of trailing instance id specifier
    $name =~ s/\.\{i\}$//;

    # convert all dots ('.') or instance specifiers
    # ('.{i}.') into underscores ('_')
    $name =~ s/\.\{i\}\./_/g;
    $name =~ s/\./_/g;

    # convert all letters to upper case
    $name =~ tr/[a-z]/[A-Z]/;

    # if there is a _INTERNETGATEWAYDEVICE_ in the middle
    # of the object name, delete it.  Its redundant.
    $name =~ s/INTERNETGATEWAYDEVICE_//;

    return $name;
}



#
# Convert a short object name from the Data Model spreadsheet
# to the "all caps with underscore separator" form.
#
sub convert_shortObjectName
{
    my $name = $_[0];
    my ($line, $nextWord, $restOfLine);

    #
    # Strip the trailing word "Object".  All of our abbreviated
    # object names end with object.
    #
    $name =~ s/Object$//;

    #
    # check for special case where the entire abbreviated name
    # is all caps, e.g. IGD.  If so, just return that.
    #
    if (!($name =~ /[a-z]+/))
    {
        return $name;
    }

    #
    # Check for word that starts with multiple capitol letters,
    # e.g. PPPLinkConfig
    # The first ([A-Z]) should be (^[A-Z]) to force the match to the
    # beginning of the shortObjectName.  Without the ^, we will match capital
    # letters in the middle of shortObjectNames, such as VoiceLineCLIR.
    # But fixing the pattern now would require changes to existing object
    # names.  The workaround is to avoid multiple consecutive capital letters
    # especially in the middle of the shortObjectName.
    #

    $name =~ /([A-Z])([A-Z]+)([A-Z])([\w]*)/;

    if (defined($2))
    {
#        print "Multi cap found, 3=$3 restOfLine=$4\n";
        $line = $1 . $2;
        $restOfLine = $3 . $4;
    }
    else
    {
        $line = "";
        $restOfLine = $name;
    }

#    print "After all caps processing, $line <-> $restOfLine\n";

    while (!($restOfLine =~ /^[A-Z][a-z0-9]*$/))
    {
        $restOfLine =~ /([A-Z][a-z0-9]*)([A-Z][\w\d]*)/;
        $nextWord = $1;
        $restOfLine = $2;

#        print "nextWord=$nextWord restOfLine=$restOfLine\n";

        $nextWord =~ tr/[a-z]/[A-Z]/;
        if ($line eq "")
        {
            $line = $nextWord;
        }
        else
        {
            $line = $line . "_" . $nextWord;
        }

#        print "line is now $line\n";
#        print "restOfLine = $restOfLine\n";
    }


    # this is the last word
#    print "last word $restOfLine\n";

    $restOfLine =~ tr/[a-z]/[A-Z]/;
    if ($line eq "")
    {
        $line = $restOfLine;
    }
    else
    {
        $line = $line . "_" . $restOfLine;
    }

    return $line;
}



#
# Turn the type name from TR98 format to our format
#
sub convert_typeName
{
    my $typeName = $_[0];

    if ($typeName =~ /string/i)
    {
        $typeName = "char *   ";
    }
    elsif ($typeName =~ /unsignedInt/i)
    {
        $typeName = "UINT32   ";
    }
    elsif ($typeName =~ /int/i)
    {
        $typeName = "SINT32   ";
    }
    elsif ($typeName =~ /unsignedLong/i)
    {
        $typeName = "UINT64   ";
    }
    elsif ($typeName =~ /long/i)
    {
        $typeName = "SINT64   ";
    }
    elsif ($typeName =~ /boolean/i)
    {
        $typeName = "UBOOL8   ";
    }
    elsif ($typeName =~ /base64/i)
    {
        $typeName = "BASE64   ";
    }
    elsif ($typeName =~ /hexBinary/i)
    {
        $typeName = "HEXBINARY ";
    }
    elsif ($typeName =~ /dateTime/i)
    {
        $typeName = "DATETIME ";
    }
    elsif ($typeName =~ /UUID/i)
    {
        $typeName = "char *   ";
    }
    elsif ($typeName =~ /IPAddress/i)
    {
        $typeName = "IPADDRESS ";
    }
    elsif ($typeName =~ /MACAddress/i)
    {
        $typeName = "MACADDRESS ";
    }
    elsif ($typeName =~ /StatsCounter32/i)
    {
        $typeName = "UINT32   ";
    }
    elsif ($typeName =~ /StatsCounter64/i)
    {
        $typeName = "UINT64   ";
    }
    else
    {
        die("unrecognized type $typeName");
    }

    return $typeName;
}


#
# Convert the first letter of a leaf parameter name that is
# to be used as a field name in a MdmObject structure to
# lower case.  However, don't lower case the first letter
# if it is part of an all capitol acronym, e.g. ATMPeakCellRate
#
sub convert_fieldName
{
    my ($firstLetter, $secondLetter, $restOfLine);

    $_ = $_[0];
    $_ =~ tr/[\-]/[_]/;
    /([A-Z])([\w])([\w]*)/;
    $firstLetter = $1;
    $secondLetter = $2;
    $restOfLine = $3;

    if (defined($secondLetter) && $secondLetter =~ /[a-z]/)
    {
        $firstLetter =~ tr/[A-Z]/[a-z]/;
        return $firstLetter . $secondLetter . $restOfLine;
    }
    else
    {
        if ($_ =~ /^[0-9]/)
        {
            return "_" . $_;
        }
        else
        {
            return $_;
        }
    }
}

#
# Change the valid value string to all caps.
# Change space and '.' to _
#
sub convert_validstring
{
    my $str = $_[0];

    $str =~ tr/[ ]/[_]/;
    $str =~ tr/[\.]/[_]/;
    $str =~ tr/[\-]/[_]/;

    $str =~ tr/[a-z]/[A-Z]/;

    return $str;
}


#
# Change the profile name to a filename
#
sub convert_profileNameToFilename
{
    my $str = $_[0];

    $str =~ tr/[:]/[_]/;
    
    $str = $str . ".c";

    return $str;
}


#
# Lower case the first word, with special case exceptions.
# Works similar to the standard perl function lcfirst
#
sub getLowerCaseFirstLetterSpecial
{
    my ($orig) = @_;
    my $lc;

    if ($orig =~ /^IGD([\w]+)/)
    {
        $lc = "igd" . $1;
    }
    elsif ($orig =~ /^IP([\w]+)/)
    {
        $lc = "ip" . $1;
    }
    else
    {
        $lc = lcfirst $orig;
    }

    return $lc;
}


#
# return the depth of the current path by counting the number
# of "." in the path.  Each "." counts as depth 1, except when
# there is ".{i}.", which only counts as 1.
#
# So InternetGatewayDevice.           counts as 1
#    InternetGatewayDevice.DeviceInfo. counts as 2
#    InternetGatewayDevice.DeviceInfo.VendorConfigFile.{i}. counts as 3
#
sub get_pathDepth
{
    my $restOfLine = $_[0];
    my $depth = 1;

    chomp($restOfLine);

#    print "get_pathDepth=$restOfLine\n";

    while ($restOfLine =~ /([\w-]+)\.([\w{}\.-]+)/)
    {
        $restOfLine = $2;
#        print "($depth) 1=$1 2=$2\n";

        # Get rid of intermediate "{i}." in the path
        if ($restOfLine =~ /^{i}\.([\w{}\.-]+)/)
        {
            $restOfLine = $1;
        }

        $depth += 1;

        if ($restOfLine =~ /^[\w-]+\.\{i\}\.$/)
        {
            # restOfLine consists entirely of "name.{i}." so we are done.
            last;
        }

        if ($depth > 10)
        {
            die "impossible depth";
        }
    }

    return $depth;
}


#
# Open a filehandle and return a reference to it.
# This is useful when we want to output the MDM tree to multiple files.
# See Chapter 2, page 51, of Programming Perl
#
sub open_filehandle_for_output
{
    my $filename = $_[0];
    my $overWriteFilename = ">" . $filename;
    local *FH;

    open (FH, $overWriteFilename) || die "Could not open $filename";

    return *FH;
}

sub open_filehandle_for_input
{
    my $filename = $_[0];
    local *FH;

    open (FH, $filename) || die "Could not open $filename";

    return *FH;
}


#
# return the filehandle associated with this profile for the purpose
# of outputing the mdm tree.  If the filehandle for the specified
# profile has not been opened yet, open it and inject the autogen_warning.
#
sub get_profiled_filehandle
{
    my ($basedir, $profile) = @_;
    my $fileRef;
    my $def;

    $fileRef = $profileToFileHandleHash{$profile};
    if (!defined($fileRef))
    {
        my $filename;
        my $converted_profile_name;
        $converted_profile_name = convert_profileNameToFilename($profile);

        $filename = $basedir . "/" . $converted_profile_name;
#        print "new profile, filename= $filename\n";
        $fileRef = open_filehandle_for_output($filename);
        $profileToFileHandleHash{$profile} = $fileRef;
        autogen_warning($fileRef);

        $def = convertProfileNameToPoundDefine($profile);
        print $fileRef "#ifdef $def\n\n";
    }

    return $fileRef;
}

sub close_all_profiled_filehandles
{
    my $key;
    my $fileRef;

    foreach $key (keys (%profileToFileHandleHash))
    {
        $fileRef = $profileToFileHandleHash{$key};
        print $fileRef "\n\n#endif\n";
        close $profileToFileHandleHash{$key};
    }
}



###########################################################################
#
# Begin of top level function for generating a .h or .c file
# The first couple of output_xxx functions are pretty simple, they
# process the cms-data-model.xml from stdin in a single pass.
# Towards the end of this section, we have more complicated functions
# that suck in the cms-data-model.xml and builds an internal data
# structure tree, and then ouputs the desired files by recursively
# traversing the data structure.
#
###########################################################################


#
# Input the data model file (there are now 2 of them) and store the
# oid, name, and shortObjectNames for output later
#
sub input_mdmObjectIdFile
{
    my $inputFileRef = shift;
    my %rowHash;
    my $objName;
    my $oid;
    
    while (parse_row($inputFileRef, \%rowHash))
    {
        if ($rowHash{"type"} =~ /object/i)
        {
            if ($rowHash{"shortObjectName"} =~ /None/i)
            {
                $objName = convert_fullyQualifiedObjectName($rowHash{"name"});
            }
            else
            {
                $objName = convert_shortObjectName($rowHash{"shortObjectName"});
            }
            
            $oid = $rowHash{"oid"};
#            print "input=> $oid $rowHash{\"name\"}\n";
            if (!defined($oidToNameHash{$oid}))
            {
                # since we input both the cms-data-model-merged.xml and
                # merged2.xml, the same OID might show up with two different
                # root prefixes.  Use the firs one (InternetGatewayDevice.)
                $oidToNameHash{$oid} = $rowHash{"name"};
                $oidToObjNameHash{$oid} = $objName;
            }
            else
            {
                if ($oidToObjNameHash{$oid} ne $objName)
                {
                    die "oid $oid: mismatch objName: $oidToObjNameHash{$oid} vs $objName\n";
                }
            }
        }
    }
}

#
# Top Level function for creating MdmObjectId's
# The mdm_objectid.h file contains OID for both the IGD and Device data
# model objects.  OIDs are fixed, so the same object will have the same
# OID in both data models.
#
sub output_mdmObjectIdFile
{
    my $fileRef = shift;
    my $oid;
    my $maxOid=0;
    
    for ($oid=1; $oid < 65535; $oid++)
    {
        if (defined($oidToNameHash{$oid}))
        {
            print $fileRef "/*! \\brief $oidToNameHash{$oid} */\n";
            print $fileRef "#define MDMOID_$oidToObjNameHash{$oid}  $oid\n\n";
            $maxOid = $oid;
        }
    }

    print $fileRef "/*! \\brief maximum OID value */\n";
    print $fileRef "#define MDM_MAX_OID $maxOid\n\n";
}


#
# Top Level function for creating array of oid info.
# Combines the previous stringtable and handler functions pointers.
# There is one OidInfoArray per data model.
#
sub output_mdmOidInfoArray
{
    my $inputFileRef = shift;
    my $fileRef = shift;
    my %oidToProfileHash = ();
    my %oidToHandlerFuncsHash = ();
    my %rowHash;
    my $objName;
    my $oid;
    my ($rclfunc, $stlfunc);
    my ($rclfuncPre, $rclfuncPost, $stlfuncPost);
    my $inVoiceRegion = 0;
    my $depth;
    my $currDepth = 0;
    my @profileStack;
    my $i = 0;
    my $conditionalLine;
    my $handlerFuncsLine;
    my $profileDef;

    # InternetGatewayDevice is at depth 1, so initialize depth 0 with something.
    $profileStack[0] = "Unspecified";


    # first suck in the info from the entire file
    while (parse_row($inputFileRef, \%rowHash))
    {
        if (($rowHash{"type"} =~ /object/i) && ($rowHash{"supportLevel"} ne "NotSupported"))
        {
            $oid = $rowHash{"oid"};

            $oidToNameHash{$oid} = $rowHash{"name"};

            $objName = getLowerCaseFirstLetterSpecial($rowHash{"shortObjectName"});

            $rclfunc = "rcl_" . $objName ;
            $rclfuncPre = "NULL";
            if (defined($rowHash{"callRclPreHook"}) && $rowHash{"callRclPreHook"} =~ /true/i)
            {
               $rclfuncPre = "rcl_pre_" . $objName;
            }
            $rclfuncPost = "NULL";
            if (defined($rowHash{"callRclPostHook"}) && $rowHash{"callRclPostHook"} =~ /true/i)
            {
               $rclfuncPost = "rcl_post_" . $objName;
            }

            $stlfunc = "stl_" . $objName ;
            $stlfuncPost = "NULL";
            if (defined($rowHash{"callStlPostHook"}) && $rowHash{"callStlPostHook"} =~ /true/i)
            {
               $stlfuncPost = "stl_post_" . $objName;
            }

            $oidToHandlerFuncsHash{$oid} = "$rclfuncPre, $rclfunc, $rclfuncPost, $stlfunc, $stlfuncPost},\n";

            # Now figure out what profile ifdefs are needed around this entry.
            $depth = get_pathDepth($rowHash{name});
            if ($depth > $currDepth)
            {
                # increasing depth, push another profile on the stack
                push(@profileStack, $rowHash{profile});
                $currDepth = $depth;
            }
            elsif ($depth == $currDepth)
            {
                # we are at the same depth, just update the profile name at the current depth
                $profileStack[$depth] = $rowHash{profile};
            }
            else
            {
                # we are decreasing in depth.  We could decrease by more than one level,
                # so be sure to pop the right number of elements from the stack.
                while ($depth < $currDepth)
                {
                    pop(@profileStack);
                    $currDepth--;
                }
                # now we are at the same depth, update the profile name at the current depth
                $profileStack[$depth] = $rowHash{profile};
            }

            $conditionalLine = "";
            for ($i = 0; $i <= $currDepth; $i++)
            {
                if ((($pure_device2 == 0) &&
                      ($profileStack[$i] ne "Unspecified") &&
                      ($profileStack[$i] ne "Baseline:1") &&
                      ($profileStack[$i] ne "X_BROADCOM_COM_Baseline:1")) ||
                    (($pure_device2 == 1) &&
                     ($profileStack[$i] ne "Unspecified") &&
                     ($profileStack[$i] ne "X_BROADCOM_COM_Baseline:1") &&
                     ($profileStack[$i] ne "Device2_Baseline:1") &&
                     ($profileStack[$i] ne "Device2_Baseline:2") &&
                     ($profileStack[$i] ne "Device2_Baseline:3")))
                    
                {
                    $profileDef = convertProfileNameToPoundDefine($profileStack[$i]);
                    if ($conditionalLine eq "")
                    {
                        $conditionalLine = "defined($profileDef)";
                    }
                    else
                    {
                        $conditionalLine = $conditionalLine . " && defined($profileDef)";
                    }
                }
            }

            if ($conditionalLine ne "")
            {
                $oidToProfileHash{$oid} = "#if $conditionalLine\n" 
            }
        }
    }


    # Now write out the info
    for ($oid=1; $oid < 65535; $oid++)
    {
        if (defined($oidToNameHash{$oid}))
        {
            if (defined($oidToProfileHash{$oid}))
            {
                print $fileRef $oidToProfileHash{$oid};
            }
            print $fileRef "{$oid, \"$oidToNameHash{$oid}\", \n";
            print $fileRef $oidToHandlerFuncsHash{$oid};
            if (defined($oidToProfileHash{$oid}))
            {
                print $fileRef "#endif\n";
            }
            print $fileRef "\n";
        }
    }

}


#
# Top Level function for creating MdmObject structure definitions
#
sub output_mdmObjectFile
{
    my $inputFileRef = shift;
    my $fileRef = shift;
    my (%rowHash, %prevObjRowHash);
    my $closePrevRow=0;
    my $oid;
    my $oidName;
    my $printParams=1;


    while (parse_row($inputFileRef, \%rowHash))
    {
        if ($rowHash{"type"} =~ /object/i)
        {

            if ($rowHash{"shortObjectName"} =~ /None/i)
            {
                $oidName = convert_fullyQualifiedObjectName($rowHash{"name"});
            }
            else
            {
                $oidName = convert_shortObjectName($rowHash{"shortObjectName"});
            }

            $oid = $rowHash{"oid"};

            {
                my $objName;

                if ($closePrevRow)
                {
                    if ($prevObjRowHash{"shortObjectName"} =~ /None/i)
                    {
                        $objName = convert_fullyQualifiedObjectName($prevObjRowHash{"name"});
                        $objName = $objName . "_OBJECT";
                    }
                    else
                    {
                        $objName = $prevObjRowHash{"shortObjectName"};
                    }


                    print $fileRef "} $objName;\n\n";

                    print $fileRef "/*! \\brief _$objName is used internally to represent $objName */\n";
                    print $fileRef "typedef $objName _$objName;\n\n\n\n";
                }

                if (($rowHash{"supportLevel"} ne "NotSupported") &&
                    (!defined($oidToNameHash{$oid})))
                {
                    print $fileRef "/*! \\brief Obj struct for $rowHash{\"name\"}\n";
                    print $fileRef " *\n";
                    print $fileRef " * MDMOID_$oidName $oid\n";
                    print $fileRef " */\n";
                    print $fileRef "typedef struct\n{\n";
                    print $fileRef "    MdmObjectId _oid;\t/**< for internal use only */\n";
                    print $fileRef "    UINT16 _sequenceNum;\t/**< for internal use only */\n";

                    $oidToNameHash{$oid} = $rowHash{"name"};
                    $printParams = 1;
                    $closePrevRow = 1;
                    %prevObjRowHash = %rowHash;
                }
                else
                {
                    # the current object is not supported, don't print out any
                    # of its params either.
                    $printParams = 0;
                    $closePrevRow = 0;
                    %prevObjRowHash = ();
#                    print "Skiping $rowHash{\"name\"} \n";
                }
            }

        }
        elsif ($rowHash{"type"} ne "description")
        {
            #
            # Define fields in the structure body.
            # Change the type name to something we define.
            # Make the first letter of the variable name lower case.
            # Surround the parameter in #ifdef profile if the parameter's
            # profile is different than that of parent object profile.
            #
            if ($printParams == 1)
            {
                my $convertedTypeName = convert_typeName($rowHash{"type"});
                my $convertedFieldName = convert_fieldName($rowHash{"name"});
                my $def;
                my $printEndif=0;

                if ($rowHash{"supportLevel"} ne "NotSupported")
                {
                    if (($prevObjRowHash{"profile"} ne $rowHash{"profile"}) &&
                        !($rowHash{"profile"} =~ /unspecified/i))
                    {
                        $def = Utils::convertProfileNameToPoundDefine($rowHash{"profile"});
                        print $fileRef "#ifdef $def\n";
                        $printEndif = 1;
                    }

                    print $fileRef "    $convertedTypeName $convertedFieldName;";
                    print $fileRef "\t/**< $rowHash{\"name\"} */\n";

                    if ($printEndif == 1)
                    {
                        print $fileRef "#endif\n";
                    }
                }
            }
        }
    }


    #
    # We don't get a last "object" type, but we
    # know to end the struct definition when the
    # end of file is reached.
    #
    if ($closePrevRow)
    {
        my $objName;

        if ($prevObjRowHash{"shortObjectName"} =~ /None/i)
        {
            $objName = convert_fullyQualifiedObjectName($prevObjRowHash{"name"});
            $objName = $objName . "_OBJECT";
            print $fileRef "} $objName;\n\n";
        }
        else
        {
            $objName = $prevObjRowHash{"shortObjectName"};
        }

        print $fileRef "} $objName;\n\n";

        print $fileRef "/*! \\brief _$objName is used internally to represent $objName */\n";
        print $fileRef "typedef $objName _$objName;\n\n\n\n";
    }
}



#
# Top Level function for creating a report for what objects and parameters we support
#
sub output_report
{
    my $inputFileRef = shift;
    my $fileRef = shift;
    my %rowHash;
    my $objCount=0;
    my $paramCount=0;
    my $printParams=1;
    my $sep=',';

    print $fileRef "Obj/Param $sep Name $sep Spec Source $sep Profile $sep Type $sep Read/Write\n";

    while (parse_row($inputFileRef, \%rowHash))
    {
        if ($rowHash{"type"} =~ /object/i)
        {
            $objCount += 1;

            #
            # dont' print out unsupported objects and GPON related objects.
            #
            if (($rowHash{"supportLevel"} ne "NotSupported") &&
                !($rowHash{"name"} =~ /X_ITU_/i) &&
                !($rowHash{"name"} =~ /GponOmci/i))
            {
                if ($pure_device2 == 1)
                {
                    # when generating a report in Pure181 mode, delete the
                    # Device2 prefix
                    $rowHash{"profile"} =~ s/^Device2_//;
                }
                print $fileRef "Obj $sep $rowHash{\"name\"} $sep $rowHash{\"specSource\"} $sep $rowHash{\"profile\"}\n";

                # Current object is supported, print out its child parameters.
                $printParams = 1;
             }
             else
             {
                 # the current object is not supported, don't print out any
                 # of its params either.
                 $printParams = 0;
             }
        }
        elsif ($rowHash{"type"} ne "description")
        {
            #
            # Must be a parameter row?
            #
            if ($printParams == 1)
            {
                if ($rowHash{"supportLevel"} ne "NotSupported")
                {
                    if ($pure_device2 == 1)
                    {
                        # when generating a report in Pure181 mode, delete the
                        # Device2 prefix
                        $rowHash{"profile"} =~ s/^Device2_//;
                    }
                    print $fileRef "Param $sep $rowHash{\"name\"} $sep $rowHash{\"specSource\"} $sep $rowHash{\"profile\"} $sep ";
                    print $fileRef "$rowHash{\"type\"} $sep $rowHash{\"supportLevel\"} \n";

                    $paramCount += 1;
                }
            }
        }
    }


    print $fileRef "\n\n";
    print $fileRef "$objCount objects $sep $paramCount parameters\n";
}


#
# Top Level function for creating a report on our level of support for
# each TR PROFILE
#
sub output_report_profiles
{
    my $profileName = shift;
    my $inputFileRef = shift;
    my $outputFileRef = shift;
    my %rowHash;
    my $currObjName;
    my $currParamName;
    my $currSupportLevel;
    my $currProfile;


    # Loop over every line in the data model
    while (parse_row($inputFileRef, \%rowHash))
    {
        if ($rowHash{"type"} =~ /object/i)
        {
            # If we still have param info cached from last param, insert it
            # before processing the next obj.
            if (defined($currParamName) &&
                $currObjName ne "NotSupported")
            {
                insert_profile_info($currProfile,
                                    $currObjName, $currParamName,
                                    $currSupportLevel);
                if ($pure_device2 == 0 && $currProfile eq "Baseline:1")
                {
                    # For Baseline:1 only, automatically insert in Baseline:2
                    insert_profile_info("Baseline:2",
                                        $currObjName, $currParamName,
                                        $currSupportLevel);
                }
                $currParamName = undef;
            }

            # If an object is marked as NotSupported, remember that
            # so child parameters don't get inserted into the reportProfileHash
            if ($rowHash{"supportLevel"} eq "NotSupported")
            {
                $currObjName = "NotSupported"
            }
            else
            {
                $currObjName = $rowHash{"name"};
            }
        }
        elsif ($rowHash{"type"} ne "description")
        {
            # If we stil have param info cached from last param, insert it
            # before processing the next param.
            if (defined($currParamName) &&
                $currObjName ne "NotSupported")
            {
                insert_profile_info($currProfile,
                                    $currObjName, $currParamName,
                                    $currSupportLevel);
                if ($pure_device2 == 0 && $currProfile eq "Baseline:1")
                {
                    # For Baseline:1 only, automatically insert in Baseline:2
                    insert_profile_info("Baseline:2",
                                        $currObjName, $currParamName,
                                        $currSupportLevel);
                }
                $currParamName = undef;
            }

            if ($rowHash{"supportLevel"} ne "NotSupported" &&
                !($currObjName =~ /X_BROADCOM_COM/) &&
                !($rowHash{"name"} =~ /X_BROADCOM_COM/) &&
                !($rowHash{"name"} =~ /X_ITU_/i) &&
                !($rowHash{"name"} =~ /GponOmci/i))
            {
                # remember this param, can't insert until we find out if
                # we can use the PROFILE in this param line or use the
                # REPORT_PROFILE in the Broadcom description line.
                $currParamName = $rowHash{"name"};
                $currSupportLevel = $rowHash{"supportLevel"};
                $currProfile = $rowHash{"profile"};
                if ($pure_device2 == 1)
                {
                    # when generating a report in Pure181 mode, delete the
                    # Device2 prefix
                    $currProfile =~ s/^Device2_//;
                }
            }
        }
        else
        {
            if ($rowHash{"source"} =~ /BROADCOM/i &&
                $rowHash{"desc"} =~ /REPORT_PROFILE/ &&
                defined($currParamName) &&
                $currObjName ne "NotSupported")
            {
#                print "$currParamName $rowHash{desc}";
                # First trim the description down to the important part
                my $startIdx = index($rowHash{desc}, "REPORT_PROFILE");
                my $endIdx = index($rowHash{desc}, "</desc");
                my $profileStr = substr($rowHash{desc}, $startIdx, $endIdx-$startIdx+1);

                # Extract a max of 4 profiles
                $profileStr =~ /REPORT_PROFILE[:\s]+(\w+:\d)[&#\d;<,\s]+([\w:]*)[<,\s]*([\w:]*)[<,\s]*([\w:]*)/;
#                print "Profile1 = $1 \n";
                insert_profile_info($1,
                                    $currObjName, $currParamName,
                                    $currSupportLevel);
                if (defined($2) && length($2) > 0)
                {
#                    print "Profile2 = $2 \n";
                    insert_profile_info($2,
                                        $currObjName, $currParamName,
                                        $currSupportLevel);
                }
                if (defined($3) && length($3) > 0)
                {
#                    print "Profile3 = $3 \n";
                    insert_profile_info($3,
                                        $currObjName, $currParamName,
                                        $currSupportLevel);
                }
                if (defined($4) && length($4) > 0)
                {
#                    print "Profile4 = $4 \n";
                    insert_profile_info($4,
                                        $currObjName, $currParamName,
                                        $currSupportLevel);
                }
#                print "\n\n";
                $currParamName = undef;
            }
        }
    }


   # Output info gathered on all the PROFILES that are listed in profileName
   my $profileFH;

   open($profileFH, $profileName) || die ("could not open $profileName\n");
   while ($_ = <$profileFH>)
   {
       if ($_ =~ /^#/)
       {
           #print ("skip comment\n");
       }
       else
       {
           $_ =~ /([\w\d:]+)[\s]+((\d+))/;
           if (defined($1))
           {
               print $outputFileRef "PROFILE $1\n";
               my $actualSupported = output_single_profile($reportProfileHash{$1}, $outputFileRef);
               print $outputFileRef "\nEnd of $1 expected=$2 actual_supported=$actualSupported";
               print $outputFileRef "\n\n\n";
           }
       }
   }

   close($profileFH);
}


sub insert_profile_info
{
    my $currProfile = shift;
    my $currObjName = shift;
    my $currParamName = shift;
    my $currSupportLevel = shift;

    if ($pure_device2 == 1)
    {
        # when generating a report in Pure181 mode, delete the
        # Device2 prefix
        $currProfile =~ s/^Device2_//;
    }

    $reportProfileHash{$currProfile} .= $currObjName . " ";
    $reportProfileHash{$currProfile} .= $currParamName . " ";
    $reportProfileHash{$currProfile} .= $currSupportLevel . " ";
}


sub output_single_profile
{
    my $fullString = shift;
    my $outputFileRef = shift;
    my $lastObjName = "xxx";
    my $count=0;

    if (!defined($fullString))
    {
        return 0;
    }

#print "\n\n==>$fullString \n";

    # grab objName paramName supportLevel restOfLine
    $fullString =~ /([\w.{}-]+)\s([\w.{}-]+)\s([\w]+)\s([\s\w.{}-]*)/;
    $fullString = $4;
    while (defined($1))
    {
        if ($lastObjName ne $1)
        {
            print $outputFileRef "\n";
            print $outputFileRef "$1 \n";
            $lastObjName = $1;
        }

        print $outputFileRef "    $2 ";

        my $paramLen = length($2);
        while ($paramLen < 40)
        {
           print $outputFileRef " ";
           $paramLen++;
        }

        if ($3 eq "ReadWrite")
        {
            print $outputFileRef "W \n";
        }
        elsif ($3 eq "ReadOnly")
        {
            print $outputFileRef "R \n";
        }
        else
        {
           # Strange, only valid values are ReadWrite and ReadOnly, what is this?
           print $outputFileRef "$3 \n";
        }

        $count++;
        if ($count > 400)
        {
           die "Too many params ($count), something is wrong!" ;
        }

        last if (!defined($4) || length($4) == 0);

        $fullString =~ /([\w.{}-]+)\s([\w.{}-]+)\s([\w]+)\s([\s\w.{}-]*)/;
        $fullString = $4;
    }

    return $count;
}



#
# Top Level function for creating RCL and STL prototypes
# The single rcl.h file covers both data models.
# The single stl.h file covers both data models.
#
sub output_prototypes
{
    my ($inputFileRef, $fileRef1, $fileRef2) = @_;
    my %rowHash;
    my $objName;
    my ($rclfunc, $stlfunc);
    my ($rclfuncPre, $rclfuncPost, $stlfuncPost);
    my $oid;

    while (parse_row($inputFileRef, \%rowHash))
    {
        if (($rowHash{"type"} =~ /object/i) && ($rowHash{"supportLevel"} ne "NotSupported"))
        {
            $oid = $rowHash{"oid"};
            if (!defined($oidToNameHash{$oid}))
            {
            $oidToNameHash{$oid} = $rowHash{"name"};
            
            $objName = getLowerCaseFirstLetterSpecial($rowHash{"shortObjectName"});

            $rclfunc     = "rcl_"      . $objName . "(";
            $rclfuncPre  = "rcl_pre_"  . $objName . "(";
            $rclfuncPost = "rcl_post_" . $objName . "(";
            $stlfunc     = "stl_"      . $objName . "(" . "_$rowHash{\"shortObjectName\"}";
            $stlfuncPost = "stl_post_" . $objName . "(" . "_$rowHash{\"shortObjectName\"}";

            print $fileRef1 "CmsRet $rclfunc _$rowHash{\"shortObjectName\"} *newObj,\n";
            print $fileRef1 "                const _$rowHash{\"shortObjectName\"} *currObj,\n";
            print $fileRef1 "                const InstanceIdStack *iidStack,\n";
            print $fileRef1 "                char **errorParam,\n";
            print $fileRef1 "                CmsRet *errorCode);\n\n";

            if (defined($rowHash{"callRclPreHook"}) && $rowHash{"callRclPreHook"} =~ /true/i)
            {
            print $fileRef1 "CmsRet $rclfuncPre _$rowHash{\"shortObjectName\"} *newObj,\n";
            print $fileRef1 "                const _$rowHash{\"shortObjectName\"} *currObj,\n";
            print $fileRef1 "                const InstanceIdStack *iidStack,\n";
            print $fileRef1 "                char **errorParam,\n";
            print $fileRef1 "                CmsRet *errorCode,\n";
            print $fileRef1 "                UBOOL8 *done);\n\n";
            }

            if (defined($rowHash{"callRclPostHook"}) && $rowHash{"callRclPostHook"} =~ /true/i)
            {
            print $fileRef1 "CmsRet $rclfuncPost _$rowHash{\"shortObjectName\"} *newObj,\n";
            print $fileRef1 "                const _$rowHash{\"shortObjectName\"} *currObj,\n";
            print $fileRef1 "                const InstanceIdStack *iidStack,\n";
            print $fileRef1 "                char **errorParam,\n";
            print $fileRef1 "                CmsRet *errorCode);\n\n";
            }

            print $fileRef2 "CmsRet $stlfunc *obj, const InstanceIdStack *iidStack);\n\n";

            if (defined($rowHash{"callStlPostHook"}) && $rowHash{"callStlPostHook"} =~ /true/i)
            {
            print $fileRef2 "CmsRet $stlfuncPost *obj, const InstanceIdStack *iidStack);\n\n";
            }

            }
        }
    }
}


#
# Top Level function for creating RCL and STL skeletons
#
sub output_skeletons
{
    my ($inputFileRef, $fileRef1, $fileRef2) = @_;
    my %rowHash;
    my $objName;
    my ($rclfunc, $stlfunc);

    while (parse_row($inputFileRef, \%rowHash))
    {
        if (($rowHash{"Type"} =~ /object/i) && ($rowHash{"supportLevel"} ne "NotSupported"))
        {
            $objName = getLowerCaseFirstLetterSpecial($rowHash{"shortObjectName"});

            $rclfunc = "rcl_" . $objName . "(";
            $stlfunc = "stl_" . $objName . "(" . "_$rowHash{\"shortObjectName\"}";

            print $fileRef1 "CmsRet $rclfunc _$rowHash{\"shortObjectName\"} *newObj __attribute__((unused)),\n";
            print $fileRef1 "                const _$rowHash{\"shortObjectName\"} *currObj __attribute__((unused)),\n";
            print $fileRef1 "                const InstanceIdStack *iidStack __attribute__((unused)),\n";
            print $fileRef1 "                char **errorParam __attribute__((unused)),\n";
            print $fileRef1 "                CmsRet *errorCode __attribute__((unused)))\n";
            print $fileRef1 "{\nreturn CMSRET_SUCCESS;\n}\n\n";


            print $fileRef2 "CmsRet $stlfunc *obj __attribute__((unused)), const InstanceIdStack *iidStack __attribute__((unused)))\n";
            print $fileRef2 "{\nreturn CMSRET_SUCCESS;\n}\n\n";
        }
    }
}



#
# Read in section 1 of the data model xml file which contains the object
# and parameter definitions.  Build internal perl data structures
# and objects.
#
sub input_spreadsheet1
{
    my $inputFileRef = $_[0];
    my $objRef;
    my $currDepth = 0;

    $objRef = input_nodes_recursively($inputFileRef, undef, $currDepth);

#    print "input_spreadsheet1: top node is ${$objRef}{\"name\"}\n";
#    my $count1 = $objRef->getChildObjectCount();
#    print "    Total first level child objects=$count1\n";
#    my $count2 = $objRef->getSupportedChildObjectCount();
#    print "    Total SUPPORTED first level child objects=$count2\n";
    
    return $objRef;
}


sub skip_spreadsheet1
{
    my $inputFileRef = $_[0];
    my %rowHash;
    
    while (parse_row($inputFileRef, \%rowHash))
    {
        # do nothing.. We are just skipping through spreadsheet1
    }
}


#
# Read in section2 of the data model file, which contains the 
# Valid String Arrays (VSA's)
#
sub input_spreadsheet2
{
    my $inputFH = $_[0];
    my $line;
    my %rowHash;
    my $arrayRef;
    my $currentPrefix;

    while (parse_row_vv($inputFH, \%rowHash))
    {
        if (defined($rowHash{"profile"}))
        {
            # start a new set of valid values
            $arrayRef = [];
            ${$arrayRef}[0] = $rowHash{"name"};

            if (defined($rowHash{"prefix"}))
            {
               ${$arrayRef}[1] = $rowHash{"prefix"};
               $currentPrefix = $rowHash{"prefix"};
            }
            else
            {
               ${$arrayRef}[1] = "noPrefix";
               undef($currentPrefix);
            }

            @allVSArefs = (@allVSArefs, $arrayRef);
        }
        else
        {
            # continuation of current set of valid values
            # Append to the current arrayRef
            @{$arrayRef} = (@{$arrayRef}, $rowHash{"name"});

            # This is for generating the validstrings file
            {
                my $nn;
                if (defined($currentPrefix))
                {
                    $nn = $currentPrefix . "_" . $rowHash{"name"};
                    $allValidStrings{$nn} = $currentPrefix;
#                    print "adding $nn to allValidStrings\n";
                }
                else
                {
                   $nn = $rowHash{"name"};
                   $allValidStrings{$nn} = "noPrefix";
                }
            }
        }
    }
}


#
# Read in section3 of the data model file, which contains a list of known
# profiles.  I think this section is really just for DataModelDesigner to
# create the dropdown menu of available profiles
#
sub input_spreadsheet3
{
    my $inputFH = $_[0];
    my $profileName;
    
    while (<$inputFH>)
    {
        if (/[\w]*<\/profileInfo/)
        {
            # end of section found
            return 0;
        }

        if (/[\w]*<profile name=\"([\w:_-]+)\"/)
        {
            append_profile($1);
        }
    }
}


#
# Record the given object profile name
#
sub record_obj_profile
{
    my $currProfile = $_[0];
    my $i;

#    print "got $currProfile\n";

    # See if we must record this profile as a leaf profile
    for ($i = 0; $i < @forcedLeafProfilesArray; $i++)
    {
        if ($forcedLeafProfilesArray[$i] eq $currProfile)
        {
#            print "$currProfile is a forced leaf profile\n";
            # if we have not recorded it in the leafProfileArray, do it now.
            my $j;
            for ($j = 0; $j < @leafProfilesArray; $j++)
            {
                if ($leafProfilesArray[$j] eq $currProfile)
                {
                    # we already recorded this leaf profile, just return.
                    return;
                }
            }
            
            # this profile is not in our array, so add it now.
            @leafProfilesArray = ($currProfile, @leafProfilesArray);
            return;
        }
    }

    for ($i = 0; $i < @objProfilesArray; $i++)
    {
        if ($objProfilesArray[$i] eq $currProfile)
        {
#            print "$currProfile is dup, don't add to array\n";
            return;
        }
    }

    #
    # insert profile at the head of the list.
    # profiles encountered later are more likely to be a leaf profile.
    # profiles encountered earlier are more likely to depend on later profiles.
    #
#    print "Adding $currProfile to array\n";
    @objProfilesArray = ($currProfile, @objProfilesArray);
}


#
# Append the given profile into the @objProfilesArray.
# Do not append if it is a duplicate.
#
sub append_profile
{
    my $currProfile = $_[0];
    my $i;

    for ($i = 0; $i < @section3ProfilesArray; $i++)
    {
        if ($section3ProfilesArray[$i] eq $currProfile)
        {
#            print "$currProfile is dup, don't add to array\n";
            return;
        }
    }

    # append profile at the tail of the array.
    @section3ProfilesArray = (@section3ProfilesArray, $currProfile);
}


#
# Scan the MDM data model spreadsheet and input the object
# and param node information.
#
sub input_nodes_recursively
{
    my ($inputFileRef, $currObjRef, $currDepth) = @_;
    my $topObjRef;
    my $lastObjRef;
    my $addToObjRef;
    my $addToParamRef;

    #
    # If there is not a saved rowHash, then get the next row
    #
    while ($savedRowHashValid || parse_row($inputFileRef, \%savedRowHash))
    {
        if ($savedRowHash{"type"} =~ /object/)
        {
            my $tmpObjRef;
            my $tmpObjDepth;
            my $oid;
            my $lockZone;
            
            # we are currently dealing with an object, so do not add any
            # descriptions to the last Param that we saw
            $addToParamRef = undef;

            # If we recurse, we need to remember to use what is already
            # in the rowHash -- do not read another new line
            $savedRowHashValid = 1;

            $tmpObjDepth = get_pathDepth($savedRowHash{"name"});

#            print "processing $savedRowHash{name} depth=$tmpObjDepth (currDepth=$currDepth)\n";

            if ($tmpObjDepth == $currDepth + 1)
            {
                # child object is at the right level, must be my immediate
                # child, so add it as my child. (regardless of wheter it is
                # supported or not).

                # OIDs are only important for real objects, fakeParentObjects
                # gets invalid OID of 999999
                if ($savedRowHash{"shortObjectName"} ne "FakeParentObject")
                {
                    if (defined($savedRowHash{"oid"}))
                    {
                        # object has specified its own OID, so use it and
                        # make subsequent objects sequential to this one.
                        $oid = $savedRowHash{"oid"};
                        $seqOid = $oid;
#                        print "$savedRowHash{name} specified OID=$oid\n";
                    }
                    else
                    {
                        $oid = $seqOid;
#                        print "$savedRowHash{name} using seqOid=$seqOid\n";
                    }
                    $seqOid++;

                    if (defined($oidToNameHash{$oid}))
                    {
                        die "OID $oid is already used by $oidToNameHash{$oid} and is requested by $savedRowHash{\"name\"}\n";
                    }
                    $oidToNameHash{$oid} = $savedRowHash{"name"};
                }
                else
                {
                    $oid = 999999;
                }

                if (defined($savedRowHash{"lockZone"}))
                {
                    $lockZone = $savedRowHash{"lockZone"};
                    $parentLockZone = $lockZone;
                }
                else
                {
                    if (($savedRowHash{"name"} eq "InternetGatewayDevice.") ||
                        ($savedRowHash{"name"} eq "Device."))
                    {
                        # Reset lockZone to 0 whenever we go back to top level object.
                        $parentLockZone = 0;
                    }
                    $lockZone = $parentLockZone;
                }
                # print "[oid $oid] $savedRowHash{\"name\"} lockZone=> $lockZone\n";
                # if you want to emulate global locking, set lockZone to 0 here, e.g.
                # $lockZone = 0

                $tmpObjRef = GenObjectNode::new GenObjectNode;
                $tmpObjRef->fillObjectInfo($oid,
                                  $lockZone,
                                  $tmpObjDepth,
                                  $savedRowHash{"name"},
                                  $savedRowHash{"specSource"},
                                  $savedRowHash{"profile"},
                                  $savedRowHash{"requirements"},
                                  $savedRowHash{"supportLevel"},
                                  $savedRowHash{"shortObjectName"},
                                  $savedRowHash{"hideObjectFromAcs"},
                                  $savedRowHash{"alwaysWriteToConfigFile"},
                                  $savedRowHash{"pruneWriteToConfigFile"},
                                  $savedRowHash{"autoOrder"},
                                  $savedRowHash{"callRclPreHook"},
                                  $savedRowHash{"callRclPostHook"},
                                  $savedRowHash{"callStlPostHook"},
                                  $savedRowHash{"majorVersion"},
                                  $savedRowHash{"minorVersion"});
                                  
                $lastObjRef = $tmpObjRef;

                # build array of all profiles that are associated with objects
                record_obj_profile($savedRowHash{"profile"});

                # special case processing for the first node
                if (!defined($currObjRef))
                {
#                    print "detected first node of this data model file: $savedRowHash{name}\n";
                    die ("Expected currDepth to be 0, got $currDepth\n") if ($currDepth != 0);
                    $topObjRef = $tmpObjRef;
                    $currObjRef = $tmpObjRef;
                    $addToObjRef = $tmpObjRef;
                }
                else
                {
#                    print "Adding $savedRowHash{name} as child of ${$currObjRef}{name}\n\n";
                    $currObjRef->addChildObject("tail", undef, $tmpObjRef);
                    $addToObjRef = $tmpObjRef;
                }

                $savedRowHashValid = 0;
                
                # special case processing for processing the first real node
                # The oid must be specified.
                if ($needRealObjNode && ($savedRowHash{shortObjectName} ne "FakeParentObject"))
                {
                    if (defined($savedRowHash{oid}))
                    {
                        $needRealObjNode = 0;
                    }
                    else
                    {
                        die "First real object $savedRowHash{\"name\"} must specify OID\n";
                    }
                }
            }
            elsif ($tmpObjDepth > $currDepth + 1)
            {
                # current child object has another child, recurse.
#                print ">>>>> push down, next depth = $currDepth + 1 \n";
                input_nodes_recursively($inputFileRef, $lastObjRef, $currDepth + 1);
            }
            else
            {
               # current object is actually not a child of me, pop out
               # of my recursion.
#               print "<<<<< popping up, currDepth=$currDepth\n";
               last;
            }
        }
        elsif ($savedRowHash{"type"} =~ /description/)
        {
            if (defined($addToParamRef))
            {
                $addToParamRef->addParamDescription($savedRowHash{"source"},
                                                    $savedRowHash{"desc"});
            }
            else
            {
                $addToObjRef->addObjDescription($savedRowHash{"source"},
                                                $savedRowHash{"desc"});
            }
        }
        else
        {
            # we are dealing with a parameter node which is under
            # the current object node.
            my $tmpParamRef;

            $tmpParamRef = GenParamNode::new GenParamNode;
            $tmpParamRef->fillParamInfo($savedRowHash{"name"},
                              $savedRowHash{"AlternateParamName"},
                              $savedRowHash{"type"},
                              $savedRowHash{"specSource"},
                              $savedRowHash{"defaultValueWasSet"},
                              $savedRowHash{"defaultValue"},
                              $savedRowHash{"profile"},
                              $savedRowHash{"requirements"},
                              $savedRowHash{"mayDenyActiveNotification"},
                              $savedRowHash{"denyActiveNotification"},
                              $savedRowHash{"mandatoryActiveNotification"},
                              $savedRowHash{"alwaysWriteToConfigFile"},
                              $savedRowHash{"neverWriteToConfigFile"},
                              $savedRowHash{"countPersistentForConfigFile"},
                              $savedRowHash{"transferDataBuffer"},
                              $savedRowHash{"isTr69Password"},
                              $savedRowHash{"isConfigPassword"},
                              $savedRowHash{"hideParameterFromAcs"},
                              $savedRowHash{"supportLevel"},
                              $savedRowHash{"majorVersion"},
                              $savedRowHash{"minorVersion"},
                              $savedRowHash{"notifySskLowerLayersChanged"},
                              $savedRowHash{"notifySskAliasChanged"},
                              $savedRowHash{"autoGenerateAlias"});
                              
            
            # save ref to this param to we can add description to it later
            $addToParamRef = $tmpParamRef;

            if (defined($savedRowHash{"minValue"}) || defined ($savedRowHash{"maxValue"}))
            {
                if (!defined($savedRowHash{"maxValue"}))
                {
#                    print "minValue is defined, but maxValue is not for $savedRowHash{name} $savedRowHash{type}\n";
                    if ($savedRowHash{type} eq "int")
                    {
                        $savedRowHash{"maxValue"} = "2147483647";
                    }
                    elsif ($savedRowHash{type} eq "unsignedInt")
                    {
                        $savedRowHash{"maxValue"} = "4294967295";
                    }
                    elsif ($savedRowHash{type} eq "long")
                    {
                        $savedRowHash{"maxValue"} = "9223372036854775807";
                    }
                    elsif ($savedRowHash{type} eq "unsignedLong")
                    {
                        $savedRowHash{"maxValue"} = "18446744073709551615";
                    }
                    else
                    {
                        die ("unrecognized type $savedRowHash{type} -- cannot set default maxValue\n");
                    }
#                    print "default maxValue to $savedRowHash{maxValue}\n";
                }
                
                if (!defined($savedRowHash{"minValue"}))
                {
#                    print "maxValue is defined, but minValue is not for $savedRowHash{name} $savedRowHash{type}\n";
#                    print "default minValue to 0\n";
                    ${savedRowHash}{"minValue"} = "0";
                }

                $tmpParamRef->setMinMaxValues(${savedRowHash}{"minValue"},
                                              ${savedRowHash}{"maxValue"});
            }

            if (defined($savedRowHash{"maxLength"}))
            {
                $tmpParamRef->setMaxLength(${savedRowHash}{"maxLength"});
            }

            if (defined($savedRowHash{"validValuesArray"}))
            {
                $tmpParamRef->setValidValuesArray(${savedRowHash}{"validValuesArray"});
            }


            if (!defined($addToObjRef)) {
                die "addToObjRef not defined.";
            }

            if (!defined($tmpParamRef)) {
                die "tmpParamRef not defined.";
            }

            $addToObjRef->addParamNode($tmpParamRef);
        }
    }
    
    return $topObjRef;
}


#
# Read the merge command file and get the:
# cmd, input xml filename, target object
#
sub parse_merge_file
{
    my $inputFileRef = shift;
    my $cmdHashRef = shift;
    
    while (<$inputFileRef>)
    {
#        print ("parse_merge_file: $_");
        next if /^#/;  # skip comment lines
        
        # pattern match for cmd xml-file target-obj
        if (/([\w]+)[\s]+([\w.="\/{}-]+)[\s]+([\w.{}]*)/)
        {
#            print("parse_merge_file: matched cmd=$1 arg1=$2 \n");
            ${$cmdHashRef}{"cmd"} = $1;
            ${$cmdHashRef}{"arg1"} = $2;
            # base, deleteObj, and addParams only have 2 args
            if ($1 ne "base" && $1 ne "deleteObj" && $1 ne "addParams")
            {
#               print "arg2=$3";
                ${$cmdHashRef}{"arg2"} = $3;
            }
            
            return;
        }
    }
    
    # reached end of FILE, tell caller
    ${$cmdHashRef}{"cmd"} = "EOF";
}


#
# Process the merge dir and create a merged data model for output.
#
sub input_merge_dir
{
    my $basedir = shift;
    my $mergedir = shift;
    
    my @filenames = glob "$basedir/data-model/$mergedir/*.txt";
    
    my $numFilenames = @filenames;
    my $i;
    my %cmdHash = ();
    my $firstCmd=1;
    my $mergeFileRef;


    for ($i=0; $i < $numFilenames; $i++)
    {
#        print("====>  mergefile[$i] = $filenames[$i]\n");

        $mergeFileRef = open_filehandle_for_input($filenames[$i]);
        parse_merge_file($mergeFileRef, \%cmdHash);

        while ($cmdHash{"cmd"} ne "EOF")
        {
#            print "input_merge_dir[$filenames[$i]: cmd=$cmdHash{\"cmd\"} ";
            if (defined($cmdHash{"arg1"}))
            {
#                print("$cmdHash{\"arg1\"} " );
            }
            if (defined($cmdHash{"arg2"}))
            {
#                print("$cmdHash{\"arg2\"}");
            }
#            print "\n";

            # cmd error checking
            if ($firstCmd == 1)
            {
                if ($cmdHash{"cmd"} ne "base")
                {
                    die "First cmd of first merge file must have cmd base, got cmd $cmdHash{\"cmd\"} instead\n";
                }
                $firstCmd = 0;
            }
            else
            {
                if ($cmdHash{"cmd"} eq "base")
                {
                    die "base cmd can only be in the first cmd of first merge file";
                }
            }

            $spreadsheet1Done = 0;
            $needRealObjNode = 1;
            $seqOid = 999999;
            process_merge_cmd($cmdHash{"cmd"}, $cmdHash{"arg1"}, $cmdHash{"arg2"});
            
            # read next line in file
            %cmdHash = ();
            parse_merge_file($mergeFileRef, \%cmdHash);
        }

        close($mergeFileRef);
    }
}


#
# Find the lowest level FakeParentObject
#
sub find_lowest_fake_parent_object
{
    my $startObjRef = shift;
    my $paramsOnly = shift;
    my $lowestObjRef;

    die "startObjRef is undefined!\n" if (!defined($startObjRef));

    my $tmpChildArrayRef = ${$startObjRef}{"ChildObjects"};
    my $numChildren = @{$tmpChildArrayRef};
#    print "find_lowest_fake_parent: got ${$startObjRef}{\"name\"} ";
#    print "${$startObjRef}{\"shortObjectName\"} numChildren=$numChildren\n";
    
    # The addParams cmd will feed in a file that has ends with a
    # FakeParentObject containing the params we want to add.
    if ($paramsOnly eq "ParamsOnly" && $numChildren == 0)
    {
        return $startObjRef;
    }
    die "bad XML format, FakeParentObject has no child objects\n" if ($numChildren == 0);
    
    # if the child is not a fakeParentObject, then this object is
    # the lowest level FakeParentObject
    my $firstChildObjRef = ${$tmpChildArrayRef}[0];
    if (${$firstChildObjRef}{"shortObjectName"} ne "FakeParentObject")
    {
        return $startObjRef;
    }
    
    die "bad XML format, more than 1 child and not FakeParentObject" if ($numChildren > 1);

    # child is a FakeParentObject, so recurse
    $lowestObjRef = find_lowest_fake_parent_object($firstChildObjRef, $paramsOnly);

    return $lowestObjRef;
}


#
# Find the specified object name in the $rootObjRef tree.
#
sub find_object_by_name
{
    my $targetObjName = shift;
    my $currObjRef = shift;
    my $currObjName = ${$currObjRef}{"name"};
    
#    print "find_object: currently at $currObjName\n";
    if ($currObjName eq $targetObjName)
    {
#        print "Found $targetObjName !!\n";
        return $currObjRef;
    }
    
    # For the search down this tree to be useful, the current object's name
    # must be a strict subset of the targetObjName, otherwise, I am going
    # down the wrong subtree.
    if (index($targetObjName, $currObjName) != -1)
    {
        my $childObjectsArrayRef = ${$currObjRef}{"ChildObjects"};
        my $numChildObjects = @{$childObjectsArrayRef};
        my $j;
        my $foundObjRef;
        
        for ($j=0; $j < $numChildObjects; $j++)
        {
            $foundObjRef = find_object_by_name($targetObjName, ${$childObjectsArrayRef}[$j]);
            if (defined($foundObjRef))
            {
                return $foundObjRef;
            }
        }
    }
    
    return undef;
}


#
# Return the parent or child portion of the given object name
#
sub get_parentchild_objname
{
    my $targetObjName = shift;
    my $selection = shift;
    my $targParent;
    my $targChild;

    $targetObjName =~ /([\w._{}]+)\.([\w._{}]+.)/;
#       print "====> $targetObjName breaks into $1 and $2 \n";
    $targParent = $1 . ".";
    $targChild = $2;
    if ($targChild eq "{i}.")
    {
        # we matched an end .{i}. so adjust our matching pattern
        $targetObjName =~ /([\w._{}]+)\.([\w._]+.\{i\}.)/;
#            print "====> try again: $1 and $2 \n";
        $targParent = $1 . ".";
        $targChild = $2;
    }

    if ($selection eq "parent")
    {
       return $targParent;
    }
    else
    {
       return $targChild;
    }
}


#
# Return the parent portion of the given object name
#
sub get_parent_objname
{
   return (get_parentchild_objname(@_, "parent"));
}


#
# Return the child portion of the given object name
#
sub get_child_objname
{
   return (get_parentchild_objname(@_, "child"));
}


#
# Process one line in the merge file
#
sub process_merge_cmd
{
    my $cmd = shift;
    my $arg1 = shift;
    my $arg2 = shift;
    my $targetObjName;
    my $targetObjRef;
    my $targParent;
    my $childObjArrayToAdd;
    my $inputFileRef;
    my $srcObjRef;


    if ($cmd eq "deleteObj")
    {
#        print "got deleteObj for $arg1\n";
        $targetObjName = $arg1;
        $targParent = get_parent_objname($targetObjName);

        $targetObjRef = find_object_by_name($targParent, $rootObjRef);
        if (!defined($targetObjRef))
        {
            die ("deleteObj: Could not find parent obj $targParent \n");
        }

        $targetObjRef->deleteChildObject($targetObjName);
        return;
    }

    if ($cmd eq "deleteParam")
    {
       # Due to DM Detect mode, be sure to delete the param in both the
       # merge-igd.d and merge-dev2.d directories.  Only a single unified
       # mdm_object.h is generated, so the objects must have the
       # same definition!
#       print "got deleteParam for $arg1\n";
       $targParent = get_parent_objname($arg1);
       my $targChild = get_child_objname($arg1);

       $targetObjRef = find_object_by_name($targParent, $rootObjRef);
       if (!defined($targetObjRef))
       {
          die "deleteParam: Could not find parent obj $targParent \n";
       }

       $targetObjRef->delParamNode($targChild);
       return;
    }

    if ($cmd eq "base")
    {
        $inputFileRef = open_filehandle_for_input($arg1);
        $rootObjRef = input_spreadsheet1($inputFileRef);
        input_spreadsheet2($inputFileRef);
        input_spreadsheet3($inputFileRef);
        close($inputFileRef);
        return;
    }

    my $lowestObjRef;
    my $srcNumChildren;
    
    if ($cmd eq "addFirstChildObjToObj" || $cmd eq "addLastChildObjToObj" ||
        $cmd eq "addObjAboveObj" || $cmd eq "addObjBelowObj" ||
        $cmd eq "addParams")
    {
        # print "process_merge_cmd: filename=$arg1 \n";
        $inputFileRef = open_filehandle_for_input($arg1);
        $srcObjRef = input_spreadsheet1($inputFileRef);
        input_spreadsheet2($inputFileRef);
        input_spreadsheet3($inputFileRef);
        close($inputFileRef);

        my $findLowestOption = ($cmd eq "addParams") ? "ParamsOnly" : "Normal";
        $lowestObjRef = find_lowest_fake_parent_object($srcObjRef, $findLowestOption);
        $srcNumChildren = $lowestObjRef->getChildObjectCount();
        $childObjArrayToAdd = ${$lowestObjRef}{"ChildObjects"};
        $targetObjName = $arg2;

#       print "-- lowest FakeObjRef = ${$lowestObjRef}{\"name\"} ";
#       print "numChildren=$srcNumChildren \n";
#       print "-- target=$targetObjName \n";
    }

    if ($cmd eq "addFirstChildObjToObj" || $cmd eq "addLastChildObjToObj")
    {
#        print ("===> $cmd: need to find obj for $targetObjName \n");
        $targetObjRef = find_object_by_name($targetObjName, $rootObjRef);
        if (!defined($targetObjRef))
        {
            die ("Could not find targetObj!! ($targetObjName) \n");
        }
        
        # Fix the path on each top level child obj before adding
        # This covers the case where an object in the IGD tree is mapped
        # to a different location in the Device tree.
        my @newObjArray = @{$childObjArrayToAdd};
        my $k;
        for ($k=0; $k < $srcNumChildren; $k++)
        {
#            print "top level child fix $k: $newObjArray[$k]{\"name\"} \n";
            my $childObjRef = $newObjArray[$k];
            $childObjRef->modifyPathPrefix(${$lowestObjRef}{name}, $targetObjName);
        }

        if ($cmd eq "addFirstChildObjToObj")
        {
            $targetObjRef->addChildObject("head", undef, @{$childObjArrayToAdd});
        }
        else
        {
            $targetObjRef->addChildObject("tail", undef, @{$childObjArrayToAdd});
        }
    }
    elsif ($cmd eq "addObjAboveObj" || $cmd eq "addObjBelowObj")
    {
        $targParent = get_parent_objname($targetObjName);

        $targetObjRef = find_object_by_name($targParent, $rootObjRef);
        if (!defined($targetObjRef))
        {
            die ("Could not find targetObj!! ($targParent) \n");
        }

        # Fix the path on each top level child obj before adding
        # This covers the case where an object in the IGD tree is mapped
        # to a different location in the Device tree.
        my @newObjArray = @{$childObjArrayToAdd};
        my $k;
        for ($k=0; $k < $srcNumChildren; $k++)
        {
#            print "top level child fix $k: $newObjArray[$k]{\"name\"} \n";
            my $childObjRef = $newObjArray[$k];
            $childObjRef->modifyPathPrefix(${$lowestObjRef}{name}, $targParent);
        }

        if ($cmd eq "addObjAboveObj")
        {
            $targetObjRef->addChildObject("insertbefore", $targetObjName, @{$childObjArrayToAdd});
        }
        else
        {
            $targetObjRef->addChildObject("insertafter", $targetObjName, @{$childObjArrayToAdd});
        }
    }
    elsif ($cmd eq "addAttrToObj")
    {
         $targetObjName = $arg2;
#         print("got addAttrToObj cmd, attr=$arg1 obj=$targetObjName\n");
         if ($arg1 =~ /([\w]+)="([\w]+)"/)
         {
             $targetObjRef = find_object_by_name($targetObjName, $rootObjRef);
             if (!defined($targetObjRef))
             {
                 die ("Could not find targetObj!! ($targParent) \n");
             }
             $targetObjRef->addObjAttribute($1, $2);
         }
         else
         {
              die ("unrecognized attribute in addAttrToObj $arg1 $targetObjName\n");
         }
    }
    elsif ($cmd eq "addParams")
    {
        my $paramArrayRef = ${$lowestObjRef}{"Params"};
        my $numParams = @{$paramArrayRef};
        # print ("addParams: found $numParams params to add to ${$lowestObjRef}{name}\n");
        
        $targetObjName = ${$lowestObjRef}{name};
        $targetObjRef = find_object_by_name($targetObjName, $rootObjRef);
        die ("Could not find target object $targetObjName\n") if (!defined($targetObjRef));
        for (my $i=0; $i < $numParams; $i++)
        {
            $targetObjRef->addParamNode(${$paramArrayRef}[$i]);
        }
    }
    else
    {
        die ("unrecognized command $cmd\n");
    }
}


#
# Output the valid string arrays bracked by #ifdef profile name
#
sub output_profiled_validstrings
{
    my $basedir = shift;
    my $fileRef;
    my ($key, $profileStr);
    my ($arrayRef, $arrayRef2, $vvRef);

#    print "starting output_profiled_validstrings\n";

    $fileRef = open_filehandle_for_output("$basedir/validstrings.c");

    autogen_warning($fileRef);

        # each entry the arrayRef is another arrayRefs to the valid values array
        foreach $vvRef (@allVSArefs)
        {
            my $firstEntry=1;
            my $secondEntry=1;
            my ($vvStr, $tmpStr);

            foreach $vvStr (@{$vvRef})
            {
                if ($firstEntry == 1)
                {
                    $tmpStr = $vvStr . "[]";
                    print $fileRef "const char * $tmpStr = {\n";
                    $firstEntry = 0;
                    $secondEntry = 1;
                }
                elsif ($secondEntry == 1)
                {
                    # skip this entry, it contains the prefix
                    $secondEntry = 0;
                }
                else
                {
                    print $fileRef "\"$vvStr\",\n";
                }
            }
            print $fileRef "NULL\n";
            print $fileRef "};\n\n";
        }


    close $fileRef;
}


#
# Output the valid strings array to cms-data-model-merged.xml, and 
# eliminate exact duplicates and detecting non-exact duplicates which
# is an error.
#
sub output_merged_validstrings
{
    my $outputFH = $_[0];
    my $vvRef;
    my ($i, $back);
    my $unique;

    print $outputFH "<vsaInfo>\n\n";
    
    for ($i=0; $i < @allVSArefs; $i++)
    {
        $vvRef = $allVSArefs[$i];
        $unique = 1;
        
        # check for duplicates on VSA's from beginning to this point
        for ($b=0; $b < $i; $b++)
        {
            my $backvvRef = $allVSArefs[$b];

            if ( ${$backvvRef}[0] eq ${$vvRef}[0] )
            {
                # We got duplicate VSA names, but that is OK as long as
                # the elements are exactly the same.

                my $backvvRefCount=@{$backvvRef};
                my $vvRefCount=@{$vvRef};

                if ($backvvRefCount != $vvRefCount)
                {
                    die "ERROR! Duplicate VSA ${$vvRef}[0], but different number of elements ($backvvRefCount, $vvRefCount)\n";
                }
                else
                {
                    my $j;
                    for ($j=1; $j < $vvRefCount; $j++)
                    {
                        if ( ${$backvvRef}[$j] ne ${$vvRef}[$j] )
                        {
                            die "ERROR! Duplicate VSA ${$vvRef}[0], mismatch at element $j (${$backvvRef}[$j], ${$backvvRef}[$j])\n";
                        }
                    }
                }
                $unique = 0;
            }
        }
        
        if ($unique == 1)
        {
            my $firstEntry=1;
            my $secondEntry=0;
            my ($vvStr, $tmpStr);

            foreach $vvStr (@{$vvRef})
            {
                if ($firstEntry == 1)
                {
                    $tmpStr = $vvStr . "[]";
                    print $outputFH "<validstringarray name=\"$vvStr\" ";
                    $firstEntry = 0;
                    $secondEntry = 1;
                }
                elsif ($secondEntry == 1)
                {
                    if ($vvStr ne "noPrefix")
                    {
                       print $outputFH "prefix=\"$vvStr\" "
                    }

                    print $outputFH "/>\n";
                    $secondEntry = 0;
                }
                else
                {
                    print $outputFH "  <element>$vvStr</element>\n";
                }
            }
            print $outputFH "\n";
        }
    }

    print $outputFH "</vsaInfo>\n\n";
}


#
# output the merged list of profiles.  This list is just what is listed
# in the bottom section of the data model file.  Used by DataModelDesigner.
#
sub output_merged_profiles
{
    my $outputFH = $_[0];
    my $profileName;
    my ($i, $back);
    my $unique;

    print $outputFH "<profileInfo>\n";
    
    foreach $profileName (@section3ProfilesArray)
    {
        print $outputFH "<profile name=\"$profileName\" />\n";
    }
    
    print $outputFH "</profileInfo>\n\n\n";
}


# 
# Output the objects and parameters to cms-data-model-merged.xml
#
sub output_merged_objects_and_params
{
    my $outputFileRef = $_[0];
    my $parentObjRef = $_[1];
    my $currLevel = $_[2];
    
    # dump out current obj and its parameters
    $parentObjRef->outputObjectNodeInXML($outputFileRef);

    # recurse into all children
    my $childObjectsArrayRef = ${$parentObjRef}{"ChildObjects"};
    my $num = $parentObjRef->getChildObjectCount();
    my $i;

    for ($i=0; $i < $num; $i++)
    {
        my $childObjRef = ${$childObjectsArrayRef}[$i];
        output_merged_objects_and_params($outputFileRef, $childObjRef, $currLevel + 1);
    }
}


sub output_merged_data_model
{
    my $build_dir=$_[0];
    my $mergedDm=$_[1];
    my $fileoutRef;

    $fileoutRef = open_filehandle_for_output($mergedDm);
    
    print $fileoutRef "<?xml version=\"1.0\"?>\n";
    print $fileoutRef "<xmlMandatorySingleRootNode copyright=\"Broadcom Ltd., 2006-2018\" ";
    print $fileoutRef " warning=\"This file is auto-generated.  Edits to this file will be lost!\" >\n\n";
    print $fileoutRef "\n\n\n\n";

    output_merged_objects_and_params($fileoutRef, $rootObjRef, 1);
    output_merged_validstrings($fileoutRef);
    output_merged_profiles($fileoutRef);
    
    print $fileoutRef "</xmlMandatorySingleRootNode>\n";
    close($fileoutRef);
}


#
# output #defines for characteristcs of the MDM tree
#
sub output_mdmParams
{
    my $basedir = shift;
    my $combinedMaxParamNameLength = shift;
    my $combinedMaxInstanceDepth = shift;
    my $fileRef;

    $fileRef = open_filehandle_for_output("$basedir/userspace/public/include/mdm_params.h");

    autogen_warning($fileRef);

    print $fileRef "#ifndef __MDM_PARAMS_H__\n";
    print $fileRef "#define __MDM_PARAMS_H__\n\n";

    doxygen_header($fileRef, "mdm_params.h");

    print $fileRef "/** maximum length of a param name, not including NULL */\n";
    print $fileRef "#define MAX_ACTUAL_MDM_PARAM_NAME_LENGTH $combinedMaxParamNameLength\n\n";

    print $fileRef "/** maximum actual instance depth */\n";
    print $fileRef "#define MAX_ACTUAL_MDM_INSTANCE_DEPTH    $combinedMaxInstanceDepth\n\n";

    print $fileRef "/** buffer length for an Alias buffer (64+null byte) */\n";
    print $fileRef "#define MDM_ALIAS_BUFLEN               65\n\n";

    print $fileRef "/** buffer length for a single fullpath reference (256+null byte) */\n";
    print $fileRef "#define MDM_SINGLE_FULLPATH_BUFLEN    257\n\n";
    
    print $fileRef "/** buffer length for multiple fullpath references (2048+null byte) */\n";
    print $fileRef "#define MDM_MULTI_FULLPATH_BUFLEN    2049\n\n\n";
    
    print $fileRef "#endif /* __MDM_PARAMS_H__ */\n";

    close $fileRef;
}


#
# output the MDM tree as c data structures.
# We need to recursively go down to the bottom of our object node data
# structure and print from the bottom up.
#
sub output_mdm
{
    my $basedir = shift;
    my $fileRef;
    my $currProfile;
    my $baseProfile;
    my $i;

    if ($pure_device2)
    {
        $baseProfile = "Device2_Baseline:1";
    }
    else
    {
        $baseProfile = "Baseline:1";
    }
    
    $currProfile = $baseProfile;
    
    output_profiled_validstrings($basedir);


    # The Baseline:1 profile gets some special header
    # The ordering of the includes is important, leaf profiles go first.
    $fileRef = get_profiled_filehandle($basedir, $currProfile);
    print $fileRef "#include \"cms.h\"\n";
    print $fileRef "#include \"mdm_types.h\"\n";
    print $fileRef "#include \"rcl.h\"\n";
    print $fileRef "#include \"stl.h\"\n";
    print $fileRef "#include \"validstrings.c\"\n";


    #
    # automatically generate the list of profile includes that Baseline_1.c needs
    # to include.
    #
    for ($i = 0; $i < @leafProfilesArray; $i++)
    {
        my $convName;

        $leafProfilesArray[$i] =~ /([\w]+):([\d]+)/;
        $convName = $1 . "_" . $2 . ".c";
        print $fileRef "#include \"$convName\"\n";
    }

    for ($i = 0; $i < @objProfilesArray; $i++)
    {
        my $convName;

        if ($objProfilesArray[$i] ne "Unspecified" &&
            $objProfilesArray[$i] ne $baseProfile)
        {
            $objProfilesArray[$i] =~ /([\w]+):([\d]+)/;
            $convName = $1 . "_" . $2 . ".c";
            print $fileRef "#include \"$convName\"\n";
        }
    }

    # now go and output the tree
    output_nodes_recursively($basedir, $currProfile, $rootObjRef);


    $fileRef = $profileToFileHandleHash{$currProfile};

    print $fileRef "MdmObjectNode igdRootObjNode = \n";
    $rootObjRef->outputObjectNode($fileRef);
    print $fileRef ";\n\n";

    close_all_profiled_filehandles();
}


sub output_nodes_recursively
{
    my ($basedir, $currProfile, $objRef) = @_;
    my $tmpProfile;
    my $childObjectsArrayRef;
    my $numChildObjects;
    my $numSupportedChildObjects;
    my $childObjRef;
    my $objectsPrinted=0;
    my $fileRef;
    my $i=0;

#    print "Starting output_nodes_recursively\n";

    $childObjectsArrayRef = ${$objRef}{"ChildObjects"};
    $numChildObjects = @{$childObjectsArrayRef};
    $numSupportedChildObjects = $objRef->getSupportedChildObjectCount();

#    print "push a level: ${$objRef}{\"Name\"} children=$numChildObjects\n";

    #
    # If this object's profile is unspecifed, put it with the profile
    # of the last/parent object.
    #
    $tmpProfile = $objRef->getProfile;
    if ($tmpProfile ne "Unspecified")
    {
        $currProfile = $tmpProfile;
    }

    $fileRef = get_profiled_filehandle($basedir, $currProfile);

    if ($numChildObjects == 0)
    {
        # This node is at the bottom.
        # Dump out info about its parameters, and break out of the recursion.
        if ($objRef->isSupported())
        {
            $objRef->outputParams($fileRef);
        }
    }
    else
    {
        # This node has child nodes, recursively dump them out first
        # then dump the child nodes.

        my $lastChildProfile="noprofile";
        my $addEndif=0;
        my $profileDef;

        for ($i=0; $i < $numChildObjects; $i++)
        {
            $childObjRef = ${$childObjectsArrayRef}[$i];
            if (!defined($childObjRef))
            {
                die "could not find child at $i";
            }
            output_nodes_recursively($basedir, $currProfile, $childObjRef);
        }


        if ($objRef->isSupported())
        {
            # Now I can print out my own parameters
            $objRef->outputParams($fileRef);
        }

        if ($numSupportedChildObjects > 0)
        {
            # Finally, I can print the array of all children objects
            $objRef->outputChildObjectArrayHeader($fileRef);
        }

        for ($i=0; $i < $numChildObjects; $i++)
        {
            my $childProfile;

            $childObjRef = ${$childObjectsArrayRef}[$i];

            if ($childObjRef->isSupported() == 0)
            {
                # this object is not supported, skip it.
                next;
            }

            $objectsPrinted++;

            $childProfile = $childObjRef->getProfile();

            if (($childProfile ne $lastChildProfile) &&
                ($addEndif == 1))
            {
                print $fileRef "#endif /* $profileDef */\n";
                $addEndif = 0;
                $lastChildProfile = "noprofile";
            }

            if (($childProfile ne $currProfile) &&
                ($childProfile ne "Unspecified") &&
                ($childProfile ne "$lastChildProfile"))
            {
                $lastChildProfile = $childProfile;
                $profileDef = Utils::convertProfileNameToPoundDefine($childProfile);
                print $fileRef "#ifdef $profileDef\n";
                $addEndif = 1;
            }

            $childObjRef->outputObjectNode($fileRef);
            if ($objectsPrinted < $numSupportedChildObjects)
            {
                print $fileRef ",\n";
            }
            else
            {
                print $fileRef "\n";
            }
        }

        if ($addEndif == 1)
        {
            print $fileRef "#endif /* $profileDef */\n";
        }

        if ($numSupportedChildObjects > 0)
        {
            print $fileRef "};\n\n";
        }
    }

# print "pop a level\n";

}


sub output_validstrings
{
    my $fileRef = $_[0];
    my $defStr;
    my $key;
    my $keyLen;
    my $offsetLen=20;

    foreach $key (keys (%allValidStrings))
    {
        if ($key eq "") {
            # empty string can be a valid string, but don't print that
            next;
        }


        $defStr = "MDMVS_" . convert_validstring($key);

        $keyLen = length($defStr);

        print $fileRef "#define $defStr";
        while ($keyLen < $offsetLen)
        {
            print $fileRef " ";
            $keyLen++;
        }


        # now print the actual value
        if ($allValidStrings{$key} ne "noPrefix")
        {
           my $prefix = $allValidStrings{$key};

#           print "starting with $key prefix=$prefix\n";

           # strip out the prefix when printing the actual value
           $key =~ s/$prefix//g;
           $key =~ s/^_//g;
        }

        print $fileRef " \"$key\"\n";
    }
}



sub output_valid_strings_hash
{
    my $fileRef = $_[0];
    my $profileName = $_[1];
    my $profileArrayRef;
    my $validValuesArrayRef;
    my ($i, $j) = (0, 0);

    print "dumping valid strings of $profileName\n";
    $profileArrayRef = $profileToValidStringsHash{$profileName};

    while (defined(${$profileArrayRef}[$i]))
    {
        $validValuesArrayRef = ${$profileArrayRef}[$i];

        $j = 0;
        while (defined(${$validValuesArrayRef}[$j]))
        {
            print $fileRef "[$i,$j]${$validValuesArrayRef}[$j]   ";
            $j++;
        }

        print $fileRef "\n";
        $i++;
    }
}


sub autogen_warning
{
    my $fileRef = shift;

    print $fileRef "/*\n";
    print $fileRef " * This file is automatically generated from the data-model spreadsheet.\n";
    print $fileRef " * Do not modify this file directly - You will lose all your changes the\n";
    print $fileRef " * next time this file is generated!\n";
    print $fileRef " */\n\n\n";
}


sub doxygen_header
{
    my $fileRef = shift;

    print $fileRef "/*!\\file $_[0] \n";
    print $fileRef " * \\brief Automatically generated header file $_[0]\n";
    print $fileRef " */\n\n\n";
}



###########################################################################
#
# Begin of main
#
###########################################################################

sub usage
{
    print "Usage: generate_from_dm.pl <command> <path_to_CommEngine_dir> <DMFILE> [DMFILE2]\n";
    print "First 3 arguments are mandatory; DMFILE2 is optional, depending on cmd.\n";
    print "command is one of: merge, merge2, mdmparams, objectid, object, validstrings,\n";
    print "       mdm, mdm2, report, prototypes, oidinfo, oidinfo2,\n";
    print "       or skeletons\n";
    print "Note for the merge command, args are:\n";
    print "      merge <path_to_CommEngine_dir> <merge-dir> <DMFILE>\n";
}


if (!defined($ARGV[0]))
{
    usage();
    die "need cmd (arg0)";
}

if (!defined($ARGV[1]))
{
    usage();
    die "need rootdir (arg1)";
}


if ($ARGV[0] eq "objectid")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFile2 = $ARGV[3];
    my $inputFileRef;
    my $outputFileRef;

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    input_mdmObjectIdFile($inputFileRef);
    close($inputFileRef);
    
    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile2");
    input_mdmObjectIdFile($inputFileRef);
    close($inputFileRef);


    $outputFileRef = open_filehandle_for_output("$build_dir/userspace/public/include/mdm_objectid.h");

    print $outputFileRef "#ifndef __MDM_OBJECTID_H__\n";
    print $outputFileRef "#define __MDM_OBJECTID_H__\n\n\n";

    autogen_warning($outputFileRef);

    doxygen_header($outputFileRef, "mdm_objectid.h");

    output_mdmObjectIdFile($outputFileRef);

    print $outputFileRef "\n\n#endif /* __MDM_OBJECTID_H__ */\n";

    close $outputFileRef;
}
elsif ($ARGV[0] eq "oidinfo" || $ARGV[0] eq "oidinfo2")
{
    my $build_dir = $ARGV[1];
    my $inputFile = $ARGV[2];
    my $inputFileRef;
    my $outputFileRef;
    my $outputFileName;
    my $baseProfile;
    my ($arrayName, $arrayDecl);

    if ($ARGV[0] eq "oidinfo")
    {
        $outputFileName = "mdm_oidInfoArray.c";
        $baseProfile = "defined(SUPPORT_DM_LEGACY98) || defined(SUPPORT_DM_HYBRID) || defined(SUPPORT_DM_DETECT)";
        $arrayName = "oidInfoArray_igd";
        $arrayDecl = "const MdmOidInfoEntry oidInfoArray_igd[]= { \n";
    }
    else
    {
        $pure_device2 = 1;
        $outputFileName = "mdm2_oidInfoArray.c";
        $baseProfile = "defined(SUPPORT_DM_PURE181) || defined(SUPPORT_DM_DETECT)";
        $arrayName = "oidInfoArray_dev2";
        $arrayDecl = "const MdmOidInfoEntry oidInfoArray_dev2[]= { \n";
    }

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $outputFileRef = open_filehandle_for_output("$build_dir/router-sysdep/cms_core/$outputFileName");

    autogen_warning($outputFileRef);

    print $outputFileRef "#if $baseProfile\n\n";
    print $outputFileRef "#include \"mdm_types.h\"\n";
    print $outputFileRef "#include \"rcl.h\"\n";
    print $outputFileRef "#include \"stl.h\"\n\n";

    print $outputFileRef $arrayDecl;
    
    output_mdmOidInfoArray($inputFileRef, $outputFileRef);

    print $outputFileRef "\n};\n";
    print $outputFileRef "\nint cnt_$arrayName = (sizeof($arrayName) / sizeof(MdmOidInfoEntry));\n\n";

    print $outputFileRef "void mdm_getPtrs_$arrayName(const MdmOidInfoEntry **begin, const MdmOidInfoEntry **end)\n";
    print $outputFileRef "{\n";
    print $outputFileRef "   *begin = $arrayName;\n";
    print $outputFileRef "   *end = &($arrayName [cnt_$arrayName - 1]);\n";
    print $outputFileRef "}\n";

    print $outputFileRef "#endif  /* $baseProfile */ \n";

    close $inputFileRef;
    close $outputFileRef;
}
elsif ($ARGV[0] eq "object")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFile2 = $ARGV[3];
    my $inputFileRef;
    my $fileRef;


    $fileRef = open_filehandle_for_output("$build_dir/userspace/public/include/mdm_object.h");

    print $fileRef "#ifndef __MDM_OBJECT_H__\n";
    print $fileRef "#define __MDM_OBJECT_H__\n\n\n";

    autogen_warning($fileRef);

    doxygen_header($fileRef, "mdm_object.h");

    print $fileRef "#include \"cms.h\"\n\n";

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    output_mdmObjectFile($inputFileRef, $fileRef);
    close $inputFileRef;

    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile2");
    output_mdmObjectFile($inputFileRef, $fileRef);
    close $inputFileRef;

    print $fileRef "\n\n#endif /* __MDM_OBJECT_H__ */\n";
    
    close $fileRef;
}
elsif ($ARGV[0] eq "report")
{
    my $build_dir = $ARGV[1];
    my $inputFile = $ARGV[2];
    my $inputFileRef;
    my $outputFileRef;

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $outputFileRef = open_filehandle_for_output("$build_dir/docs/data_customer_docs/CMS-supported-parameters-report.txt");

    output_report($inputFileRef, $outputFileRef);

    close $inputFileRef;
    close $outputFileRef;


    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $outputFileRef = open_filehandle_for_output("$build_dir/docs/data_customer_docs/CMS-supported-profiles-report.txt");

    output_report_profiles("report_profiles_list.txt", $inputFileRef, $outputFileRef);

    close $inputFileRef;
    close $outputFileRef;
}
elsif ($ARGV[0] eq "report2")
{
    my $build_dir = $ARGV[1];
    my $inputFile = $ARGV[2];
    my $inputFileRef;
    my $outputFileRef;

    $pure_device2 = 1;

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $outputFileRef = open_filehandle_for_output("$build_dir/docs/data_customer_docs/CMS-supported-parameters-report2.txt");

    output_report($inputFileRef, $outputFileRef);

    close $inputFileRef;
    close $outputFileRef;


    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $outputFileRef = open_filehandle_for_output("$build_dir/docs/data_customer_docs/CMS-supported-profiles-report2.txt");

    output_report_profiles("report_profiles_list2.txt", $inputFileRef, $outputFileRef);

    close $inputFileRef;
    close $outputFileRef;
}
elsif ($ARGV[0] eq "mdm" || $ARGV[0] eq "mdm2")
{
    my $mdm_dir = $ARGV[0];
    my $build_dir = $ARGV[1];
    my $inputFile = $ARGV[2];
    my $inputFileRef;
    
    if ($ARGV[0] eq "mdm2")
    {
        $pure_device2 = 1;
    }

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");

    $rootObjRef = input_spreadsheet1($inputFileRef);
    input_spreadsheet2($inputFileRef);

    my $basedir = $build_dir . "/router-sysdep/$mdm_dir";
#    print ("mdm: pure_device2=$pure_device2 basedir=$basedir\n");
    output_mdm($basedir);

    close $inputFileRef;
}
elsif ($ARGV[0] eq "mdmparams")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFile2 = $ARGV[3];
    my $inputFileRef;

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    $rootObjRef = input_spreadsheet1($inputFileRef);
    close $inputFileRef;

    my $combinedMaxParamNameLength = $maxParamNameLength;
    my $combinedMaxInstanceDepth = $maxInstanceDepth;
    
    # input the second data model file (cms-data-model-merged2.xml)
    # to see if any parameters increased in value.  Must clear the
    # oidToNameHash because the second spreadsheet has the same OID but
    # Device. based names.
    %oidToNameHash = ();
    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile2");
    $rootObjRef = input_spreadsheet1($inputFileRef);
    close $inputFileRef;

    if ($maxParamNameLength > $combinedMaxParamNameLength)
    {
        $combinedMaxParamNameLength = $maxParamNameLength;
    }
    
    if ($maxInstanceDepth > $combinedMaxInstanceDepth)
    {
        $combinedMaxInstanceDepth = $maxInstanceDepth;
    }
    
    output_mdmParams($build_dir, $combinedMaxParamNameLength, $combinedMaxInstanceDepth);
}
elsif ($ARGV[0] eq "validstrings")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFile2 = $ARGV[3];
    my $inputFileRef;
    my $fileRef;

    # we don't need anything in spreadsheet1 for valid strings,
    # so just advance through it and get to spreadsheet2

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    $rootObjRef = skip_spreadsheet1($inputFileRef);
    input_spreadsheet2($inputFileRef);
    close($inputFileRef);

    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile2");
    $rootObjRef = skip_spreadsheet1($inputFileRef);
    input_spreadsheet2($inputFileRef);
    close($inputFileRef);

    $fileRef = open_filehandle_for_output("$build_dir/userspace/public/include/mdm_validstrings.h");

    autogen_warning($fileRef);

    print $fileRef "#ifndef __MDM_VALIDSTRINGS_H__\n";
    print $fileRef "#define __MDM_VALIDSTRINGS_H__\n\n";

    output_validstrings($fileRef);

    print $fileRef "#endif /* __MDM_VALIDSTRINGS_H__ */\n";
    
    close($fileRef);
}
elsif ($ARGV[0] eq "prototypes")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFile2 = $ARGV[3];
    my $inputFileRef;
    my ($fileRef1, $fileRef2);

    $fileRef1 = open_filehandle_for_output("$build_dir/router-sysdep/cms_core/rcl.h");
    $fileRef2 = open_filehandle_for_output("$build_dir/router-sysdep/cms_core/stl.h");

    autogen_warning($fileRef1);
    autogen_warning($fileRef2);

    print $fileRef1 "#ifndef __RCL_H__\n";
    print $fileRef2 "#ifndef __STL_H__\n";

    print $fileRef1 "#define __RCL_H__\n\n";
    print $fileRef2 "#define __STL_H__\n\n";

    print $fileRef1 "#include \"cms.h\"\n";
    print $fileRef1 "#include \"mdm.h\"\n";
    print $fileRef1 "#include \"cms_core.h\"\n\n";

    print $fileRef2 "#include \"cms.h\"\n";
    print $fileRef2 "#include \"mdm.h\"\n";
    print $fileRef2 "#include \"cms_core.h\"\n\n";

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    output_prototypes($inputFileRef, $fileRef1, $fileRef2);
    close($inputFileRef);
    
    $spreadsheet1Done = 0;
    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile2");
    output_prototypes($inputFileRef, $fileRef1, $fileRef2);
    close($inputFileRef);

    print $fileRef1 "#endif /* __RCL_H__ */\n";
    print $fileRef2 "#endif /* __STL_H__ */\n";

    close $fileRef1;
    close $fileRef2;
}
elsif ($ARGV[0] eq "skeletons")
{
    my $build_dir = $ARGV[1];
    my $inputFile = $ARGV[2];
    my $inputFileRef;
    my ($fileRef1, $fileRef2);

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile");
    $fileRef1 = open_filehandle_for_output("$build_dir/router-sysdep/cms_core/linux/rcl_skel.c");
    $fileRef2 = open_filehandle_for_output("$build_dir/router-sysdep/cms_core/linux/stl_skel.c");

    print $fileRef1 "#include \"cms.h\"\n";
    print $fileRef1 "#include \"mdm.h\"\n";
    print $fileRef1 "#include \"cms_core.h\"\n\n";

    print $fileRef2 "#include \"cms.h\"\n";
    print $fileRef2 "#include \"mdm.h\"\n";
    print $fileRef2 "#include \"cms_core.h\"\n\n";

    output_skeletons($inputFileRef, $fileRef1, $fileRef2);

    close($inputFileRef);
    close $fileRef1;
    close $fileRef2;
}
elsif ($ARGV[0] eq "merge" || $ARGV[0] eq "merge2")
{
    my $build_dir = $ARGV[1];
    my $merge_dir = $ARGV[2];
    my $output_file = $ARGV[3];

    if ($ARGV[0] eq "merge2")
    {
        $pure_device2 = 1;
    }
#    print "merge_dir(2)=$merge_dir pure_device2=$pure_device2 output_file(3)=$output_file \n";

    input_merge_dir($build_dir, $merge_dir);
    
    output_merged_data_model($build_dir, "$build_dir/data-model/$output_file");

}

else
{
    usage();
    die "unrecognized command";
}

