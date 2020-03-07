#!/usr/bin/perl -w
#
# This script reads the cms-data-model in XML format.
# It then generates various .h and .c files that represent the data model in the
# c programming language that can be compiled into libmdm.so
#
# Usage: See the usage function at the bottom of this file.
#
#

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
my $needRealObjNode;
my %oidToNameHash;
my %oidToObjNameHash;
my %oidToHandlerFuncsHash;
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
my $gWlOid=0;


sub output_wlcsm_strmapper_file  {

    my $inputFileRef = shift;
    my $fileRef = shift;
    my $fileRef_h = shift;
    my (%rowHash, %prevObjRowHash);
    my $closePrevRow=0;
    my $oid;
    my $oidName;
    my $oidWlcsmName;
    my $printParams=1;
    my $entry_count=0;
    my $total_index=0;
    my $bigindex=0;
    my $mngrmapper_shift=16;


    print $fileRef_h "#define _WLCSM_MNGR_STRMAPPER_SHIFT  (4)\n\n";

    while (parse_row($inputFileRef, \%rowHash)) {

        if ($rowHash {"type"} =~ /object/i) {

            if ($rowHash {"shortObjectName"} =~ /None/i) {
                $oidName = convert_fullyQualifiedObjectName($rowHash {"name"});
            }
            else {
                $oidName = convert_shortObjectName($rowHash {"shortObjectName"});
            }


            {
                my $objName;

                if ($closePrevRow) {
                    print $fileRef "/* $oidWlcsmName */\n";
                    $bigindex=$total_index*$mngrmapper_shift+$entry_count;
                    print $fileRef_h "#define _WLCSM_MNGR_STRMAPPER_$oidWlcsmName ($bigindex)\n";
                    $total_index+=$entry_count;
                }

                if ($rowHash {"supportLevel"} ne "NotSupported") {
                    $entry_count=0;
                    $printParams = 1;
                    $closePrevRow = 1;
                    %prevObjRowHash = %rowHash;
                    if ($prevObjRowHash {"shortObjectName"} =~ /None/i) {
                        $objName = convert_fullyQualifiedObjectName($prevObjRowHash {"name"});
                        $oidWlcsmName = $objName . "_OBJECT";
                    }
                    else {
                        $oidWlcsmName = $prevObjRowHash {"shortObjectName"};
                    }
                }
                else {
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
            if ($printParams == 1) {
               # my $convertedFieldName = convert_fieldName($rowHash {"name"});
                my $def;
                my $printEndif=0;
                my $mngrvar;
                my $nvram;
                my $intvalue;
                my $nvramvalue;

                if ($rowHash {"supportLevel"} ne "NotSupported") {

                    if(defined($rowHash {"name"})) {
                        $nvram=$rowHash {"name"};
                        if(defined($rowHash {"defaultValue"})) {
                            $intvalue=$rowHash {"defaultValue"};
			    if(defined($rowHash {"nvram"}))  {
				    $nvramvalue=$rowHash{"nvram"};
				    print $fileRef sprintf("{%-60s%-40s%s},\n", $intvalue.",", "\"$nvram\",","\"$nvramvalue\"");
			    }
			    else {
				    print $fileRef sprintf("{%-60s%-40s%s},\n", $intvalue.",", "\"$nvram\",","NULL");
			    }

                            $entry_count++;
                        }
                    }



                }
            }
        }
    }

    if ($closePrevRow) {


        print $fileRef "/* $oidName */\n";
        print $fileRef "};\n";
        $bigindex=$total_index*$mngrmapper_shift+$entry_count;
        print $fileRef_h "#define _WLCSM_MNGR_STRMAPPER_$oidWlcsmName ($bigindex)\n\n";
        $total_index+=$entry_count;
        print $fileRef_h "extern WLCSM_MNGR_STRMAPPER_SET g_wlcsm_mngr_strmapper[$total_index];\n\n";

    }



}

sub parse_object_line {
    my ($hashref, $line) = @_;

#    print "parsing object line: $line";

# remove all entries from hashref
    %{$hashref} = ();


    ${$hashref}{type} = "object";

    $line =~ /name="([\w.\{\}]+)"/;
    ${$hashref}{name} = $1;

    if ($pure_device2) {
        $ {$hashref} {name} =~ s/^InternetGatewayDevice\./Device\./;
        $ {$hashref} {name} =~ s/^Device\.Device\./Device\./;
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

    if ($line =~ /requirements="([\w]+)"/) {
        $ {$hashref} {requirements} = $1;
    }

    $line =~ /supportLevel="([\w]+)"/;
    ${$hashref}{supportLevel} = $1;

    if ($line =~ /hideObjectFromAcs="([\w]+)"/) {
        $ {$hashref} {hideObjectFromAcs} = $1;
    }

    if ($line =~ /alwaysWriteToConfigFile="([\w]+)"/) {
        $ {$hashref} {alwaysWriteToConfigFile} = $1;
    }

    if ($line =~ /pruneWriteToConfigFile="([\w]+)"/) {
        $ {$hashref} {pruneWriteToConfigFile} = $1;
    }

    if ($line =~ /autoOrder="([\w]+)"/) {
        $ {$hashref} {autoOrder} = $1;
    }

    if ($line =~ /majorVersion="([\d-]+)"/) {
        $ {$hashref} {majorVersion} = $1;
    }

    if ($line =~ /minorVersion="([\d-]+)"/) {
        $ {$hashref} {minorVersion} = $1;
    }

    if ($line =~ /oid="([\d]+)"/) {
	    $ {$hashref} {oid} = $1;
	    # Remember the OID that was specified for subsequent objects
	    $gWlOid = $1;
    } else {
	    # No OID was specified, so base it on previously specified OID
	    $gWlOid++;
	    $ {$hashref} {oid} = $gWlOid;
    }

    if ($line =~ /gmapping="([\d]+)"/) {
        $ {$hashref} {gmapping} = $1;
    }

    else {
#    ${$hashref}{gmapping} = "0";
    }

}

sub parse_parameter_line {
    my ($hashref, $line) = @_;

#    print "parsing param line: $line";

# remove all entries from hashref
    %{$hashref} = ();


    $line =~ /name="([\w.\{\}\-_]+)"/;
    ${$hashref}{name} = $1;

    $line =~ /type="([\w]+)"/;
    ${$hashref}{type} = $1;

    $line =~ /specSource="([\w]+)"/;
    ${$hashref}{specSource} = $1;

    $line =~ /profile="([\w:]+)"/;
    ${$hashref}{profile} = $1;

    if ($line =~ /requirements="([\w]+)"/) {
        $ {$hashref} {requirements} = $1;
    }

    $line =~ /supportLevel="([\w]+)"/;
    ${$hashref}{supportLevel} = $1;

    if ($line =~ /mayDenyActiveNotification="([\w]+)"/) {
        $ {$hashref} {mayDenyActiveNotification} = $1;
    }

    if ($line =~ /denyActiveNotification="([\w]+)"/) {
        $ {$hashref} {denyActiveNotification} = $1;
    }

    if ($line =~ /forcedActiveNotification="([\w]+)"/) {
        $ {$hashref} {forcedActiveNotification} = $1;
    }

    if ($line =~ /alwaysWriteToConfigFile="([\w]+)"/) {
        $ {$hashref} {alwaysWriteToConfigFile} = $1;
    }

    if ($line =~ /neverWriteToConfigFile="([\w]+)"/) {
        $ {$hashref} {neverWriteToConfigFile} = $1;
    }

    if ($line =~ /countPersistentForConfigFile="([\w]+)"/) {
        $ {$hashref} {countPersistentForConfigFile} = $1;
    }

    if ($line =~ /transferDataBuffer="([\w]+)"/) {
        $ {$hashref} {transferDataBuffer} = $1;
    }

    if ($line =~ /isTr69Password="([\w]+)"/) {
        $ {$hashref} {isTr69Password} = $1;
    }

    if ($line =~ /isConfigPassword="([\w]+)"/) {
        $ {$hashref} {isConfigPassword} = $1;
    }

    if ($line =~ /hideParameterFromAcs="([\w]+)"/) {
        $ {$hashref} {hideParameterFromAcs} = $1;
    }

    if ($line =~ /maxLength="([\d]+)"/) {
        $ {$hashref} {maxLength} = $1;
    }

    if ($line =~ /minValue="([\d-]+)"/) {
        $ {$hashref} {minValue} = $1;
    }

    if ($line =~ /maxValue="([\d-]+)"/) {
        $ {$hashref} {maxValue} = $1;
    }

    if ($line =~ /validValuesArray="([\w]+)"/) {
        $ {$hashref} {validValuesArray} = $1;
    }

    if ($line =~ /majorVersion="([\d-]+)"/) {
        $ {$hashref} {majorVersion} = $1;
    }

    if ($line =~ /minorVersion="([\d-]+)"/) {
        $ {$hashref} {minorVersion} = $1;
    }

    if ($line =~ /notifySskLowerLayersChanged="([\w]+)"/) {
        $ {$hashref} {notifySskLowerLayersChanged} = $1;
    }

    if ($line =~ /notifySskAliasChanged="([\w]+)"/) {
        $ {$hashref} {notifySskAliasChanged} = $1;
    }

    if ($line =~ /autoGenerateAlias="([\w]+)"/) {
        $ {$hashref} {autoGenerateAlias} = $1;
    }

    if ($line =~ /mngrvar="([\w]+)"/) {
        $ {$hashref} {mngrvar} = $1;
    }

    if ($line =~ /nvram="([\w._]+)"/) {
        $ {$hashref} {nvram} = $1;
    }

    if ($line =~ /nvramb="([\w._]+)"/) {
        $ {$hashref} {nvramb} = $1;
    }

    if ($line =~ /mapper="([\w._]+)"/) {
        $ {$hashref} {mapper} = $1;
    }

    if ($line =~ /defaultValue=/) {
        $ {$hashref} {defaultValueWasSet} = "true";
    }

    if ($line =~ /defaultValue="([\w\s.\/:,+\(\)=\@|-]+)"/) {
        $ {$hashref} {defaultValue} = $1;
    } else
    {
# we must have a defaultValue, so just set to NULL
        ${$hashref}{defaultValue} = "NULL";
    }
    if ($line =~ /gmapping="([\w\s.\/:,+\(\)=\@|-]+)"/) {
        $ {$hashref} {gmapping} = $1;
    }
}


sub parse_description_line {
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
sub parse_row {
    my $inputFileRef = $_[0];
    my $hashref = $_[1];

#    print "starting parse_row (spreadsheet1Done=$spreadsheet1Done) \n";
    if ($spreadsheet1Done == 1) {
        return 0;
    }

    while (1) {
        $_ = <$inputFileRef>;

        if (/[\w]*<vsaInfo>/) {
#            print "Begin vsaInfo tag found, end parse_row\n";
            $spreadsheet1Done = 1;
            return 0;
        }


        if (!defined($_)) {
            print "ERROR: end of file detected during parse_row!\n";
            return 0;
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
        if (/[\w]*<object/) {
            parse_object_line($hashref, $_);
            return 1;
        }

        if (/[\w]*<parameter/) {
            parse_parameter_line($hashref, $_);
            return 1;
        }

        if (/[\w]*<description/) {
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
sub parse_row_vv {
    my $inputFileRef = $_[0];
    my $hashRef = $_[1];
    my $rowMarkerFound = 0;
    my ($currData, $currProfile);
    my $gotLine=0;

    %{$hashRef} = ();

#    print "starting parse_row_vv\n";

    while ($gotLine == 0) {
        $_ = <$inputFileRef>;

        if (!defined($_)) {
            print "ERROR: end of file detected before end vsaInfo\n";
            return 0;
        }

        if (/[\w]*<\/vsaInfo/) {
#            print "End vsaInfo tag found.\n";
            return 0;
        }


        if (/[\w]*<validstringarray/) {
            /name="([\w]+)"/;
            $ {$hashRef} {"name"} = $1;
#            print "Got vsa=${$hashRef}{name}\n";

            if (/prefix="[\w]+"/) {
                /prefix="([\w]+)"/;
                $ {$hashRef} {"prefix"} = $1;
#               print "parse: Got prefix=${$hashRef}{prefix}\n";
            }

# the profile is just a marker for beginning of vsa
            $ {$hashRef} {"profile"} = "Baseline";

            return 1;
        }

        if (/[\w]*<element/) {
            />([\w\s._-]+)<\/element/;
            if (defined($1)) {
                $ {$hashRef} {"name"} = $1;
            } else {
                $ {$hashRef} {"name"} = "";
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
sub convert_fullyQualifiedObjectName {
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
sub convert_shortObjectName {
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
    if (!($name =~ /[a-z]+/)) {
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

    if (defined($2)) {
#        print "Multi cap found, 3=$3 restOfLine=$4\n";
        $line = $1 . $2;
        $restOfLine = $3 . $4;
    } else
    {
        $line = "";
        $restOfLine = $name;
    }

#    print "After all caps processing, $line <-> $restOfLine\n";

    while (!($restOfLine =~ /^[A-Z][a-z0-9]*$/)) {
        $restOfLine =~ /([A-Z][a-z0-9]*)([A-Z][\w\d]*)/;
        $nextWord = $1;
        $restOfLine = $2;

#        print "nextWord=$nextWord restOfLine=$restOfLine\n";

        $nextWord =~ tr/[a-z]/[A-Z]/;
        if ($line eq "") {
            $line = $nextWord;
        } else {
            $line = $line . "_" . $nextWord;
        }

#        print "line is now $line\n";
#        print "restOfLine = $restOfLine\n";
    }


# this is the last word
#    print "last word $restOfLine\n";

    $restOfLine =~ tr/[a-z]/[A-Z]/;
    if ($line eq "") {
        $line = $restOfLine;
    } else
    {
        $line = $line . "_" . $restOfLine;
    }

    return $line;
}



#
# Turn the type name from TR98 format to our format
#
sub convert_typeName {
    my $typeName = $_[0];

    if ($typeName =~ /string/i) {
        $typeName = "char *   ";
    }
    elsif ($typeName =~ /unsignedInt/i) {
        $typeName = "UINT32   ";
    }
    elsif ($typeName =~ /int/i) {
        $typeName = "SINT32   ";
    }
    elsif ($typeName =~ /unsignedLong/i) {
        $typeName = "UINT64   ";
    }
    elsif ($typeName =~ /long/i) {
        $typeName = "SINT64   ";
    }
    elsif ($typeName =~ /boolean/i) {
        $typeName = "UBOOL8   ";
    }
    elsif ($typeName =~ /base64/i) {
        $typeName = "BASE64   ";
    }
    elsif ($typeName =~ /hexBinary/i) {
        $typeName = "HEXBINARY ";
    }
    elsif ($typeName =~ /dateTime/i) {
        $typeName = "DATETIME ";
    }
    else
    {
        die("unrecognized type $typeName");
    }

    return $typeName;
}


sub convert_wlcsm_typeName {
    my $typeName = $_[0];
    my $mapper = $_[1];
    my $nvramtypeName="0";
    my $mappername="0";

    if ($typeName =~ /string/i) {
        $typeName = "WLCSM_DT_STRING";
    }
    elsif ($typeName =~ /unsignedInt/i) {
        $typeName = "WLCSM_DT_UINT";
    }
    elsif ($typeName =~ /int/i) {
        $typeName = "WLCSM_DT_SINT32";
    }
    elsif ($typeName =~ /unsignedLong/i) {
        $typeName = "WLCSM_DT_UINT64";
    }
    elsif ($typeName =~ /long/i) {
        $typeName = "WLCSM_DT_SINT64";
    }
    elsif ($typeName =~ /boolean/i) {
        $typeName = "WLCSM_DT_BOOL";
    }
    elsif ($typeName =~ /base64/i) {
        $typeName = "WLCSM_DT_BASE64";
    }
    elsif ($typeName =~ /hexBinary/i) {
        $typeName = "WLCSM_DT_HEXBINARY";
    }
    elsif ($typeName =~ /dateTime/i) {
        $typeName = "WLCSM_DT_DATETIME ";
    }
    else
    {
        die("unrecognized type $typeName");
    }

    if (length($mapper)>0) {


        my @vars= split(/\./,$mapper);
        my $size=@vars;
        if($size >=2) {
            if($vars[0] eq "strint") {
                $typeName= "WLCSM_DT_STR2INT";
                $nvramtypeName= "WLCSM_DT_INT2STR";
            }
            elsif($vars[0] eq "intstr") {
                $typeName= "WLCSM_DT_INT2STR";
                $nvramtypeName= "WLCSM_DT_STR2INT";
            }
            elsif($vars[0] eq "onoffreverse") {
                $typeName= "WLCSM_DT_BOOLREV";
                $nvramtypeName= "WLCSM_DT_BOOL";
            }
            elsif($vars[0] eq "onoff") {
                $typeName= "WLCSM_DT_BOOL";
                $nvramtypeName= "WLCSM_DT_BOOL";
            }
            $mappername="_WLCSM_MNGR_STRMAPPER_".uc($vars[1]);
            if($size>2) {

		    if($vars[2] eq "strint") {
			    $nvramtypeName= "WLCSM_DT_STR2INT";
		    }
		    elsif($vars[2] eq "intstr") {
			    $nvramtypeName= "WLCSM_DT_INT2STR";
		    } 
		    elsif($vars[2] eq "boolstr") {
			    $nvramtypeName= "WLCSM_DT_BOOL2STR";
		    } else
		    {
		    $nvramtypeName=convert_wlcsm_typeName($vars[2],"");
		    }
	    }

        }

    } else
    {
        $nvramtypeName=$typeName;
    }
    return ($typeName,$mappername,$nvramtypeName);

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

	$secondLetter="a";
    $_ = $_[0];
    /([A-Z])([\w])([\w]*)/;
    $firstLetter = $1;
    $secondLetter = $2;
    $restOfLine = $3;

    if ($secondLetter =~ /[a-z]/)
{
	$firstLetter =~ tr/[A-Z]/[a-z]/;
    return $firstLetter . $secondLetter . $restOfLine;
}
else
{
    return $_[0];
}
}

sub wlcsm_simplified_name {

    my $varname=$_[0];
if($varname =~ /^X_BROADCOM_COM_(.+)/) {
convert_fieldName($1);
} else {
    convert_fieldName($varname);
}
}

sub wlcsm_nvram_mngrname {

    my $varname=shift;
    my $result2;
    if( $varname eq "mngr_Dev2WifiObject") { $result2="WLCSM_WLAN_WIFI_STRUCT";}
    elsif( $varname eq "mngr_Dev2WifiObject") { $result2="WLCSM_WLAN_WIFI_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiObject") { $result2="WLCSM_WLAN_WIFI_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiRadioObject") { $result2="WLCSM_WLAN_RADIO_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiSsidObject") { $result2="WLCSM_WLAN_BSSID_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiAccessPointObject") { $result2="WLCSM_WLAN_AP_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiAccessPointSecurityObject") { $result2="WLCSM_WLAN_AP_SECURITY_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiAccessPointWpsObject") { $result2="WLCSM_WLAN_AP_WPS_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiAssociatedDeviceObject") { $result2="WLCSM_WLAN_AP_STA_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiSsidStatsObject") { $result2="WLCSM_WLAN_SSID_STATS_STRUCT";}
    elsif($varname eq "mngr_Dev2WifiRadioStatsObject") { $result2="WLCSM_WLAN_RADIO_STATS_STRUCT";}
    else { $result2="WRONGVALUE"; }

    return $result2;
}

sub wlcsm_define_mngrname {

    my $varname=shift;
    my $itemname=shift;
    my $result2;
    if ($varname eq "DEV2_WIFI") { $result2="#define WL_WIFI_".uc($itemname)." (g_wifi_obj.$itemname) /*\t\t $itemname */";}
    elsif ($varname eq "DEV2_WIFI_RADIO") { $result2="#define WL_RADIO_".uc($itemname)."(idx) (gp_adapter_objs[(idx)].radio.$itemname) /*\t\t $itemname */";}
    elsif ($varname eq "DEV2_WIFI_SSID") { $result2="#define WL_BSSID_".uc($itemname)."(idx,sub) (gp_adapter_objs[(idx)].bssids[(sub)].$itemname) /*\t\t $itemname */";}
    elsif ($varname eq "DEV2_WIFI_ACCESS_POINT") { $result2="#define WL_AP_".uc($itemname)."(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].accesspoint.$itemname) /*\t\t $itemname */";}
    elsif ($varname eq "DEV2_WIFI_ACCESS_POINT_SECURITY") { $result2="#define WL_APSEC_".uc($itemname)."(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].security.$itemname) /*\t\t $itemname */";}
    elsif ($varname eq "DEV2_WIFI_ACCESS_POINT_WPS") { $result2="#define WL_APWPS_".uc($itemname)."(idx,sub) (gp_adapter_objs[(idx)].ssids[(sub)].wps.$itemname) /*\t\t $itemname */";}
    else { $result2=""; }
    return $result2;
}

sub wlcsm_mngstru_pos {

    my $varname=shift;
    my $result2;
    if ($varname eq "WLCSM_WLAN_WIFI_STRUCT") { $result2="MNGR_POS_WIFI";}
    elsif ($varname eq "WLCSM_WLAN_RADIO_STRUCT") { $result2="MNGR_POS_RADIO";}
    elsif ($varname eq "WLCSM_WLAN_BSSID_STRUCT") { $result2="MNGR_POS_BSSID";}
    elsif ($varname eq "WLCSM_WLAN_AP_STRUCT") { $result2="MNGR_POS_AP";}
    elsif ($varname eq "WLCSM_WLAN_AP_SECURITY_STRUCT") { $result2="MNGR_POS_AP_SEC";}
    elsif ($varname eq "WLCSM_WLAN_AP_WPS_STRUCT") { $result2="MNGR_POS_AP_WPS";}
    else { $result2="WRONGVALUE"; }
    return $result2;
}

sub wlcsm_nvram_typename {

    my @varnames=split(/\./,$_[0]);
if( $#varnames ==1) {
my $result="$varnames[1]";
my $result2;
    if ($varnames[0] eq "ap_sec") { $result2= "MNGR_POS_AP_SEC,"; }
    elsif ($varnames[0] eq "ap_wps") { $result2= "MNGR_POS_AP_WPS,"; }
    elsif ($varnames[0] eq "ap") { $result2= "MNGR_POS_AP,"; }
    elsif ($varnames[0] eq "bssid") { $result2= "MNGR_POS_BSSID,"; }
    elsif ($varnames[0] eq "radio") { $result2= "MNGR_POS_RADIO,"; }
    else { $result2="WRONGVALUE"; }
    return sprintf("%-30s%-15s","\"$result\",",$result2);
} else
{
    return "WRONGVALUE";
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

    while ($restOfLine =~ /([\w]+)\.([\w\{\}\.]+)/)
{
$restOfLine = $2;
#        print "($depth) 1=$1 2=$2\n";

# Get rid of intermediate "{i}." in the path
if ($restOfLine =~ /^{i}\.([\w\{\}\.]+)/)
    {
        $restOfLine = $1;
    }

    $depth += 1;

    if ($restOfLine =~ /^[\w]+\.\{i\}\.$/)
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

sub get_right_bool_value { 
	my $typeName=shift;
	my $value=shift;
	my $lc_value=lc($value);
	if(($value eq "false")||($value eq "0"))  {
		if($typeName =~ /WLCSM_DT_BOOLREV/i)  {
			return "1";
		} elsif ($typeName =~ /WLCSM_DT_BOOL/i)
		{
			return "0";
		} 
	} elsif(($value eq "true")||($value eq "1"))  {
		if($typeName =~ /WLCSM_DT_BOOLREV/i)  {
			return "0";
		} elsif ($typeName =~ /WLCSM_DT_BOOL/i)
		{
			return "1";
		} 
	}
	return $value;
}

sub get_type_null_default {
	my $typeName=shift;
	if( ($typeName =~ /WLCSM_DT_UINT/i) || 
		($typeName =~ /WLCSM_DT_STR2INT/i) ||
		($typeName =~ /WLCSM_DT_SINT32/i) || 
		($typeName =~ /WLCSM_DT_BOOLREV/i) ||
		($typeName =~ /WLCSM_DT_BOOL/i) || 
		($typeName =~ /WLCSM_DT_UINT64/i) || 
		($typeName =~ /WLCSM_DT_SINT64/i))
	{
		return "\"0\"";
	}
	return "NULL";
}


sub output_wlcsm_mapping_file  {

    my $inputFileRef = shift;
    my $fileRef = shift;
    my $fileRef_h = shift;
    my $fileRef_n = shift;
    my $fileRef_v = shift;
    my $fileRef_n1 = shift;
    my $fileRef_v1 = shift;
    my $fileRef_oid_mapper = shift;
    my $fileRef_oid_mapper_h = shift;
    my (%rowHash, %prevObjRowHash);
    my $closePrevRow=0;
    my $oid;
    my $oidName;
    my $oidWlcsmName;
    my $printParams=1;
    my $entry_count=0;
    my $oid_mapper_count=0;
    my $nvram_count=0;
    my $gmapping=0;
    my $nvrammap_index=0;
    my $mngr_oids="";
    my $mngr_runtime_oids="";
    my $mngr_oids_mapper="";
    my $mngr_oidname="";
    my $mngr_runtime_oids_num=0;
    my $mngr_oids_num=0;


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
	$mngr_oidname= $oidName;
	$mngr_oidname=~ s/DEV2_//;
	$mngr_oidname="WLMNGR_DATA_POS_".$mngr_oidname;
	

        {
            my $objName;

            if ($closePrevRow)
            {
                print $fileRef "{ { NULL,0},{NULL,0},0,0}\n\n";
                print $fileRef "};\n\n";
                print $fileRef_v1 "$entry_count];\n";
                print $fileRef_n1 "};\n\n";
		print $fileRef_oid_mapper "$entry_count},\n";
		$mngr_oids_mapper =$mngr_oids_mapper."$entry_count}";
		$entry_count=$entry_count+1;
                print $fileRef_h "$entry_count];\n";
            }

            if (($rowHash{"supportLevel"} ne "NotSupported") &&
                (!defined($oidToNameHash{$oid})) &&
                (defined ($rowHash{"gmapping"}) &&(( $rowHash{"gmapping"} eq "1" ) || ( $rowHash{"gmapping"} eq "2" ))))
            {
		
                print $fileRef "/*! \\brief Obj struct for $rowHash{\"name\"}\n";
		
                print $fileRef " *\n";
                print $fileRef " * MDMOID_$oidName $oid\n";
                print $fileRef " */\n";
                print $fileRef "WLCSM_DM_WLMNGR_MAPPING  g_dm_tr181_mapping","_$oidName","[]= \n{\n";
                print $fileRef_h "extern WLCSM_DM_WLMNGR_MAPPING  g_dm_tr181_mapping","_$oidName","[";
                $entry_count=0;
		$gmapping=1;
                print $fileRef_n1 "WLCSM_MNGR_NAME_OFFSET  g_WLMNGR_STRUCT_$oidName","[]={\n";
                print $fileRef_v1 "#define ".$mngr_oidname."    (".$oid.")\n";
                print $fileRef_v1 "extern WLCSM_MNGR_NAME_OFFSET  g_WLMNGR_STRUCT_$oidName","[";
		print $fileRef_oid_mapper "{ $oid,$oid, g_dm_tr181_mapping","_$oidName,";
               

		if( $rowHash{"gmapping"} eq "1" ) { 
	
			if($mngr_oids eq "")  {
				$mngr_oids=$oid;
			}
			else {
				$mngr_oids=$mngr_oids.",".$oid;
			}
			$mngr_oids_num +=1;
		} else {

			if($mngr_runtime_oids eq "")  {
				$mngr_runtime_oids=$oid;
			}
			else {
				$mngr_runtime_oids=$mngr_runtime_oids.",".$oid;
			}
			$mngr_runtime_oids_num +=1;
		}
		
		if($mngr_oids_mapper eq "")  {
			$mngr_oids_mapper ="\n{ $oid,	g_WLMNGR_STRUCT_$oidName		,";
		}
		else {
			$mngr_oids_mapper =$mngr_oids_mapper.",\n"."{ $oid,	g_WLMNGR_STRUCT_$oidName		,";
		}
		 
		$oid_mapper_count+=1;

                $oidToNameHash{$oid} = $rowHash{"name"};
                $printParams = 1;
                $closePrevRow = 1;
                %prevObjRowHash = %rowHash;
                if ($prevObjRowHash{"shortObjectName"} =~ /None/i)
                {
                    $objName = convert_fullyQualifiedObjectName($prevObjRowHash{"name"});
                    $oidWlcsmName = $objName . "_OBJECT";
                }
                else
                {
                    $oidWlcsmName = $prevObjRowHash{"shortObjectName"};
                }

            }
            else
            {
	       $gmapping=0;
# the current object is not supported, don't print out any
# of its params either.
                $printParams = 0;
                $closePrevRow = 0;
                %prevObjRowHash = ();
#                    print "Skiping $rowHash{\"name\"} \n";
            }
        }

    }
    elsif ($rowHash {"type"} ne "description")
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
            my $convertedTypeName; # = convert_wlcsm_typeName($rowHash{"type"});
            my $convertedFieldName = convert_fieldName($rowHash{"name"});
            my $def;
            my $printEndif=0;
            my $mngrvar;
            my $nvram;
            my $mngrvar_1;
            my $mappername="0";
            my $mngr_posvar="";
            my $nvramtypename="0";
            my $lowermngrv;

            if ($rowHash{"supportLevel"} ne "NotSupported")
            {
                if (($prevObjRowHash{"profile"} ne $rowHash{"profile"}) &&
                    !($rowHash{"profile"} =~ /unspecified/i))
                {
                    $def = Utils::convertProfileNameToPoundDefine($rowHash{"profile"});
                    print $fileRef "#ifdef $def\n";
                    $printEndif = 1;
                }


                if(defined($rowHash{"mngrvar"})) {
                    $mngrvar=$rowHash{"mngrvar"};
                }
                else {
                    $mngrvar= wlcsm_simplified_name($rowHash{"name"});
                }

                if(defined($rowHash{"mapper"})) {

                    ($convertedTypeName,$mappername,$nvramtypename)= convert_wlcsm_typeName($rowHash{"type"},$rowHash{"mapper"});

                } else {

                    ($convertedTypeName,$mappername,$nvramtypename)= convert_wlcsm_typeName($rowHash{"type"},"");
                }

		if($gmapping) {
                if(defined($rowHash{"nvram"})) {
                    $nvram=wlcsm_nvram_typename($rowHash{"nvram"});
                    $mngrvar_1=wlcsm_nvram_mngrname("mngr_".$oidWlcsmName);
                    print $fileRef_n sprintf("{%-60s%-30s%-80s%-20s%-15s%-20s\t},\n",  $nvram,"\"$mngrvar\",","WLCSM_DM_VAR_POSOFF($mngrvar_1,$mngrvar),",$nvramtypename.",","MNGR_SSID_SPECIFIC_VAR,",$mappername);
		    $nvrammap_index=$nvram_count;
                    $nvram_count++;
                } elsif(defined($rowHash{"nvramb"})) {
                    $nvram=wlcsm_nvram_typename($rowHash{"nvramb"});
                    $mngrvar_1=wlcsm_nvram_mngrname("mngr_".$oidWlcsmName);
                    print $fileRef_n sprintf("{%-60s%-30s%-80s%-20s%-15s\t%-20s\t},\n",  $nvram,"\"$mngrvar\",","WLCSM_DM_VAR_POSOFF($mngrvar_1,$mngrvar),",$nvramtypename.",","MNGR_SSID_SHARED_VAR,",$mappername);
		    $nvrammap_index=$nvram_count;
                    $nvram_count++;
                }
		else {
 
		    $nvrammap_index=-1;
		   # Don't show no nvram mapping entries
                   # $mngrvar_1=wlcsm_nvram_mngrname("mngr_".$oidWlcsmName);
		   # $mngr_posvar=wlcsm_mngstru_pos($mngrvar_1);
		   # if($mngr_posvar ne "WRONGVALUE")  {
                   # print $fileRef_n sprintf("{%-30s%-30s%-60s%-80s%-20s%-15s\t%-20s\t},\n",  "\"\",","$mngr_posvar,","\"$mngrvar\",","WLCSM_DM_VAR_POSOFF($mngrvar_1,$mngrvar),",$nvramtypename.",","MNGR_GENERIC_VAR,",$mappername);
                   # $nvram_count++;
		   # }
		}
		}

                $mngrvar_1=wlcsm_nvram_mngrname("mngr_".$oidWlcsmName);
		$lowermngrv=lc($mngrvar);
		
		if(defined($rowHash {"defaultValue"}) &&  ($rowHash{"defaultValue"} ne "NULL" )) {
                	print $fileRef_n1 sprintf("{%-30s%-80s%-30s%-10s%-10s},\n","\"$mngrvar\",","WLCSM_DM_VAR_POSOFF($mngrvar_1,$mngrvar),","$convertedTypeName,",$nvrammap_index,",\"".get_right_bool_value($convertedTypeName,$rowHash {"defaultValue"})."\"");
		} else {
			print $fileRef_n1 sprintf("{%-30s%-80s%-30s%-10s,%-10s},\n","\"$mngrvar\",","WLCSM_DM_VAR_POSOFF($mngrvar_1,$mngrvar),","$convertedTypeName,",$nvrammap_index,get_type_null_default($convertedTypeName));
		}


	
                print $fileRef "{\t{\"$convertedFieldName\",\tWLCSM_DM_VAR_POSOFF($oidWlcsmName,$convertedFieldName) },\n\t{\"$mngrvar\",\tWLCSM_DM_VAR_POSOFF(mngr_$oidWlcsmName,$mngrvar)},\n\t$convertedTypeName,\t$mappername},\n";

                $entry_count++;
# print $fileRef "\t/**< $rowHash{\"name\"} */\n";

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
	if ($closePrevRow) {
	my $objName;

	if ($prevObjRowHash {"shortObjectName"} =~ /None/i) {
		$objName = convert_fullyQualifiedObjectName($prevObjRowHash {"name"});
		$objName = $objName . "_OBJECT";
		print $fileRef "} $objName;\n\n";
	    }
	    else {
		$objName = $prevObjRowHash {"shortObjectName"};
	    }

	    print $fileRef "} $objName;\n\n";

	    print $fileRef "/*! \\brief _$objName is used internally to represent $objName */\n";
	    print $fileRef "typedef $objName _$objName;\n\n\n\n";
	}

	print $fileRef_v "extern WLCSM_NVRAM_MNGR_MAPPING g_wlcsm_nvram_mngr_mapping[$nvram_count];";
	print $fileRef_v "\n\n#endif";
	print $fileRef_oid_mapper_h "extern WLCSM_DM_WLMNGR_OID_MAPPING g_wlcsm_tr181_oid_mapping[$oid_mapper_count];";
	
	print $fileRef_n1 "unsigned int g_WLMNGR_OIDS[$mngr_oids_num]={$mngr_oids}; 	/**<these are OIDs objects which carries wireless configuration */\n";
	print $fileRef_n1 "unsigned int g_WLMNGR_OIDS_NUM=$mngr_oids_num; 		/**< total number of objects which carries wireless configuration */\n\n";

	print $fileRef_n1 "unsigned int g_WLMNGR_RUNTIME_OIDS[$mngr_runtime_oids_num]={$mngr_runtime_oids};	/**< these are OIDs objects that are runtime info, such as statistics */\n";
	print $fileRef_n1 "unsigned int g_WLMNGR_RUNTIME_OIDS_NUM=$mngr_runtime_oids_num;			/**< total number of OIDs objects that are runtime info*/\n\n";

	print $fileRef_n1 "WLCSM_DM_OID_MAPPING g_WLMNGR_OID_MAPPER[$oid_mapper_count]= {".$mngr_oids_mapper."\n};\n";

	print $fileRef_v1 "extern unsigned int g_WLMNGR_OIDS[$mngr_oids_num];\n";
	print $fileRef_v1 "extern unsigned int g_WLMNGR_OIDS_NUM;\n";

	print $fileRef_v1 "extern unsigned int g_WLMNGR_RUNTIME_OIDS[$mngr_runtime_oids_num];\n";
	print $fileRef_v1 "extern unsigned int g_WLMNGR_RUNTIME_OIDS_NUM;\n";
	print $fileRef_v1 "extern WLCSM_DM_OID_MAPPING g_WLMNGR_OID_MAPPER[$oid_mapper_count];\n";
}

sub output_mngrObjectFile {
    my $inputFileRef = shift;
    my $fileRef = shift;
    my $fileRef_n = shift;
    my (%rowHash, %prevObjRowHash);
    my $closePrevRow=0;
    my $oid;
    my $oidName;
    my $printParams=1;
    my $def_value="";


    while (parse_row($inputFileRef, \%rowHash)) {
        if ($rowHash {"type"} =~ /object/i) {

            if ($rowHash {"shortObjectName"} =~ /None/i) {
                $oidName = convert_fullyQualifiedObjectName($rowHash {"name"});
            }
            else {
                $oidName = convert_shortObjectName($rowHash {"shortObjectName"});
            }

            $oid = $rowHash {"oid"};

            {
                my $objName;

                if ($closePrevRow) {
                    if ($prevObjRowHash {"shortObjectName"} =~ /None/i) {
                        $objName = convert_fullyQualifiedObjectName($prevObjRowHash {"name"});
                        $objName = $objName . "_OBJECT";
                    }
                    else {
                        $objName = $prevObjRowHash {"shortObjectName"};
                    }

                    $objName = "mngr_" . $objName;


                    print $fileRef "} $objName;\n\n";

                    print $fileRef "/*! \\brief _$objName is used internally to represent $objName */\n";
                    print $fileRef "typedef $objName _$objName;\n\n\n\n";
                }

                if (($rowHash {"supportLevel"} ne "NotSupported") &&
                    (!defined($oidToNameHash {$oid})) &&
                    (defined ($rowHash {"gmapping"}) &&(( $rowHash {"gmapping"} eq "1" )||( $rowHash {"gmapping"} eq "2" )))) {
                    print $fileRef "/*! \\brief Obj struct for $rowHash{\"name\"}\n";
                    print $fileRef " *\n";
                    print $fileRef " * MDMOID_$oidName $oid\n";
                    print $fileRef " */\n";
                    print $fileRef "typedef struct {\n";
#print $fileRef "    MdmObjectId _oid;\t/**< for internal use only */\n";

                    $oidToNameHash {$oid} = $rowHash {"name"};
                    $printParams = 1;
                    $closePrevRow = 1;
                    %prevObjRowHash = %rowHash;
                }
                else {
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
            if ($printParams == 1) {
                my $convertedTypeName = convert_typeName($rowHash {"type"});
                my $convertedFieldName = convert_fieldName($rowHash {"name"});
                my $def;
                my $printEndif=0;
                my $mngrvar;
                my $nvram;


                if ($rowHash {"supportLevel"} ne "NotSupported") {
                    if(defined($rowHash {"mapper"})) {

                        my $convName;
                        my $mappername;
                        my $nvramtypename;
                        ($convName,$mappername,$nvramtypename)= convert_wlcsm_typeName($rowHash {"type"},$rowHash {"mapper"});
                        if($convName eq "WLCSM_DT_STR2INT") {
                            $convertedTypeName = "SINT32";
                        }
                        if($convName eq "WLCSM_DT_INT2STR") {
                            $convertedTypeName= "char *";
                        }
                    }

                    if(defined($rowHash {"nvram"})) {
                        $nvram=$rowHash {"nvram"};
                    }
                    if(defined($rowHash {"mngrvar"})) {
                        $mngrvar=$rowHash {"mngrvar"};
                    }
                    else {
                        $mngrvar= wlcsm_simplified_name($rowHash {"name"});
                    }


                    print $fileRef "    $convertedTypeName $mngrvar;";
                    print $fileRef "\t/**< $rowHash{\"name\"} */\n";
                    $def_value= wlcsm_define_mngrname($oidName,$mngrvar);
                    if (length($def_value)>0) {
                        print $fileRef_n "$def_value \n";
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
    elsif ($rowHash {"type"} ne "description")
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

                print $fileRef "    $convertedTypeName $convertedFieldName;";
                print $fileRef "\t/**< $rowHash{\"name\"} */\n";

            }
        }
    }
}


#
# We don't get a last "object" type, but we
# know to end the struct definition when the
# end of file is reached.
#
if ($closePrevRow) {
my $objName;

if ($prevObjRowHash {"shortObjectName"} =~ /None/i) {
        $objName = convert_fullyQualifiedObjectName($prevObjRowHash {"name"});
        $objName = $objName . "_OBJECT";
        print $fileRef "} $objName;\n\n";
    }
    else {
        $objName = $prevObjRowHash {"shortObjectName"};
    }

    print $fileRef "} $objName;\n\n";

    print $fileRef "/*! \\brief _$objName is used internally to represent $objName */\n";
    print $fileRef "typedef $objName _$objName;\n\n\n\n";
}
}





# Read in section 1 of the data model xml file which contains the object
# and parameter definitions.  Build internal perl data structures
# and objects.
#
sub input_spreadsheet1 {
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


sub skip_spreadsheet1 {
    my $inputFileRef = $_[0];
    my %rowHash;

    while (parse_row($inputFileRef, \%rowHash)) {
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

$tmpObjRef = GenObjectNode::new GenObjectNode;
$tmpObjRef->fillObjectInfo($oid,
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
                           $savedRowHash{"gmapping"},
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
                                $savedRowHash{"mngrvar"},
                                $savedRowHash{"nvram"},
                                $savedRowHash{"nvramb"},
                                $savedRowHash{"mapper"},
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
        die "currobjref not defined.";
    }

    if (!defined($tmpParamRef)) {
        die "currobjref not defined.";
    }

    $addToObjRef->addParamNode($tmpParamRef);
}
}

return $topObjRef;
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
           print "command is one of: wlcsm, wlcsm_mngr\n";
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


if ($ARGV[0] eq "wlcsm_mngr")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFileRef;
    my $fileRef;
    my $fileRef_n;


    $fileRef = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/include/wlcsm_dm_generic_orig.h");
    $fileRef_n = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/include/wlcsm_mngr_structs_name.h");

    print $fileRef "#ifndef __WLCSM_DM_GENERIC_H__\n";
    print $fileRef "#define __WLCSM_DM_GENERIC_H__\n\n\n";
    print $fileRef_n "#ifndef __WLCSM_MNGR_STRUCTS_NAME_H__\n";
    print $fileRef_n "#define __WLCSM_MNGR_STRUCTS_NAME_H__\n\n\n";

    autogen_warning($fileRef);
    autogen_warning($fileRef_n);

    doxygen_header($fileRef, "wlcsm_dm_generic.h");
    doxygen_header($fileRef_n, "wlcsm_mngr_structs_name.h");

    print $fileRef  "\n/**";
    print $fileRef  "This file include mmngr struct and generic APIs  ";
    print $fileRef  "\n*/\n\n";



    #print $fileRef "#include \"mdm_object.h\"\n\n";
    #print $fileRef "#include \"wlcsm_lib_dm_datatype.h\"\n\n";
    #print $fileRef "#include \"wlcsm_defs.h\"\n\n";
    print $fileRef "#include \"wlcsm_lib_api.h\"\n\n";
    print $fileRef "#include \"wlcsm_dm_mngr_strmapper.h\"\n\n";

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    output_mngrObjectFile($inputFileRef, $fileRef, $fileRef_n);
    close $inputFileRef;

    print $fileRef "typedef mngr_Dev2WifiObject				WLCSM_WLAN_WIFI_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiRadioObject			WLCSM_WLAN_RADIO_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiSsidObject			WLCSM_WLAN_BSSID_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiAccessPointObject		WLCSM_WLAN_AP_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiAccessPointSecurityObject	WLCSM_WLAN_AP_SECURITY_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiAccessPointWpsObject		WLCSM_WLAN_AP_WPS_STRUCT;\n";

    print $fileRef "typedef mngr_Dev2WifiAssociatedDeviceObject		WLCSM_WLAN_AP_STA_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiSsidStatsObject		WLCSM_WLAN_SSID_STATS_STRUCT;\n";
    print $fileRef "typedef mngr_Dev2WifiRadioStatsObject		WLCSM_WLAN_RADIO_STATS_STRUCT;\n";
    

    print $fileRef "typedef struct {\n";
    print $fileRef "        WLCSM_WLAN_AP_STRUCT  accesspoint;\n";
    print $fileRef "        WLCSM_WLAN_AP_SECURITY_STRUCT  security;\n";
    print $fileRef "        WLCSM_WLAN_AP_WPS_STRUCT   wps;\n";
    print $fileRef "        WLCSM_WLAN_AP_STA_STRUCT  *associated_stas;\n";
    print $fileRef "} WLCSM_WLAN_ACCESSPOINT_STRUCT;\n";
    print $fileRef "\n";
    print $fileRef "typedef struct {\n";
    print $fileRef "        WLCSM_WLAN_RADIO_STRUCT radio;\n";
    print $fileRef "        WLCSM_WLAN_BSSID_STRUCT *bssids;\n";
    print $fileRef "        WLCSM_WLAN_ACCESSPOINT_STRUCT *ssids;\n";
    print $fileRef "} WLCSM_WLAN_ADAPTER_STRUCT;\n";
    print $fileRef "\n\n#include \"wlcsm_mngr_structs_name.h\"\n\n";
    print $fileRef "extern WLCSM_WLAN_ADAPTER_STRUCT *gp_adapter_objs;	/**< global variable for holding all configurations of wifi adapters **/\n";
    print $fileRef "extern WLCSM_WLAN_WIFI_STRUCT g_wifi_obj;		/**< global variable to hold system level wifi configuration  **/\n\n";

    print $fileRef "#define B_DM_LOADING    (1)	/**< indicate to load from DM  **/\n";
    print $fileRef "#define B_DM_POPULATING (2)	/**< indicate to populate to DM  **/\n\n";

    print $fileRef "#include <wlcsm_dm_mngr_structoff.h>\n";
    print $fileRef "#include <wlcsm_dm_nvram_mngr_mapping.h>\n";
    print $fileRef "#endif /* __WLCSM_DM_GENERIC_H__ */\n";
    print $fileRef_n "\n\n#endif /* __WLCSM_MNGR_STRUCTS_NAME_H__ */\n";

    close $fileRef;
    close $fileRef_n;
}
elsif ($ARGV[0] eq "wlcsm")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFileRef;
    my $fileRef;
    my $fileRef_h;
    my $fileRef_n;
    my $fileRef_v;
    my $fileRef_n1;
    my $fileRef_v1;
    my $fileRef_oid_mapper;
    my $fileRef_oid_mapper_h;

    $fileRef = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/dm_impls/tr181/src/wlcsm_dm_tr181_mngr_mapping.c");
    $fileRef_oid_mapper = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/dm_impls/tr181/src/wlcsm_dm_tr181_oid_mapper.c");
    $fileRef_h = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/dm_impls/tr181/include/wlcsm_dm_tr181_mngr_mapping.h");
    $fileRef_oid_mapper_h = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/dm_impls/tr181/include/wlcsm_dm_tr181_oid_mapping.h");
    $fileRef_n = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/src/wlcsm_dm_nvram_mngr_mapping.c");
    $fileRef_v = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/include/wlcsm_dm_nvram_mngr_mapping.h");
    $fileRef_n1 = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/src/wlcsm_dm_mngr_structoff.c");
    $fileRef_v1 = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/include/wlcsm_dm_mngr_structoff.h");
    
    print $fileRef_h "#ifndef __WLCSM_DM_TR181_MNGR_MAPPING_H__  \n";
    print $fileRef_oid_mapper_h "#ifndef __WLCSM_DM_TR181_OID_MAPPING_H__  \n";
    print $fileRef_h "#define __WLCSM_DM_TR181_MNGR_MAPPING_H__ \n\n\n";

    print $fileRef_v "#ifndef __WLCSM_DM_NVRAM_MNGR_MAPPING_H__  \n";
    print $fileRef_v "#define __WLCSM_DM_NVRAM_MNGR_MAPPING_H__ \n\n\n";

    print $fileRef_v1 "#ifndef __WLCSM_DM_MNGR_STRUCTOFF_H__  \n";
    print $fileRef_v1 "#define __WLCSM_DM_MNGR_STRUCTOFF_H__ \n\n\n";

    autogen_warning($fileRef);
    autogen_warning($fileRef_h);
    autogen_warning($fileRef_oid_mapper_h);
    autogen_warning($fileRef_n);
    autogen_warning($fileRef_v);

    doxygen_header($fileRef, "wlcsm_dm_tr181_mngr_mapping.c");
    doxygen_header($fileRef_oid_mapper, "wlcsm_dm_tr181_oid_mapper.c");
    doxygen_header($fileRef_h, "wlcsm_dm_tr181_mngr_mapping.h");
    doxygen_header($fileRef_oid_mapper_h, "wlcsm_dm_tr181_oid_mapping.h");
    doxygen_header($fileRef_n, "wlcsm_dm_nvram_mngr_mapping.c");
    doxygen_header($fileRef_v, "wlcsm_dm_nvram_mngr_mapping.h");

    doxygen_header($fileRef_n1, "wlcsm_dm_mngr_structoff.c");
    doxygen_header($fileRef_v1, "wlcsm_dm_mngr_structoff.h");

    print $fileRef_h "/* DO NOT INCLUDE THIS HEADER FILE DIRECTLY, INSTEAD INCLUDE wlcsm_lib_dm.h */\n\n";
    print $fileRef_oid_mapper_h "/* DO NOT INCLUDE THIS HEADER FILE DIRECTLY, INSTEAD INCLUDE wlcsm_lib_dm.h */\n\n";
    print $fileRef_v "/* DO NOT INCLUDE THIS HEADER FILE DIRECTLY, INSTEAD INCLUDE wlcsm_lib_dm.h */\n\n";
    print $fileRef_v1 "/* DO NOT INCLUDE THIS HEADER FILE DIRECTLY, INSTEAD INCLUDE wlcsm_lib_dm.h */\n\n";

    print $fileRef "#include \"wlcsm_lib_dm.h\"\n\n";
    print $fileRef "#include \"wlcsm_dm_tr181_mngr_mapping.h\"\n\n";
    print $fileRef "#include \"mdm_object.h\"\n\n";
    print $fileRef_n "#include \"wlcsm_lib_dm.h\"\n\n";
    print $fileRef_h "#include \"wlcsm_lib_dm_datatype.h\"\n\n";
    print $fileRef_n1 "\n\n#include \"wlcsm_dm_generic.h\"\n\n";

    print $fileRef_oid_mapper_h "#include \"wlcsm_lib_dm.h\"\n\n";
    print $fileRef_oid_mapper "#include \"wlcsm_lib_dm.h\"\n\n";
    print $fileRef_oid_mapper "#include \"wlcsm_dm_tr181_mngr_mapping.h\"\n\n";
    print $fileRef_oid_mapper "WLCSM_DM_WLMNGR_OID_MAPPING g_wlcsm_tr181_oid_mapping[]= {\n\n";

    print $fileRef_n "WLCSM_NVRAM_MNGR_MAPPING g_wlcsm_nvram_mngr_mapping[]= {\n\n";

    $inputFileRef = open_filehandle_for_input("$build_dir/data-model/$inputFile1");
    output_wlcsm_mapping_file($inputFileRef, $fileRef,$fileRef_h,$fileRef_n,$fileRef_v,$fileRef_n1,$fileRef_v1,$fileRef_oid_mapper,$fileRef_oid_mapper_h);
    close $inputFileRef;

    print $fileRef "\n\n";
    print $fileRef_h "\n\n#endif\n\n";
    print $fileRef_oid_mapper_h "\n\n#endif\n\n";
    print $fileRef_v1 "\n\n#endif\n\n";
    print $fileRef_n "};\n";
    print $fileRef_oid_mapper "};\n\n";

    close $fileRef;
    close $fileRef_oid_mapper;
    close $fileRef_h;
    close $fileRef_n;
    close $fileRef_v;
    close $fileRef_n1;
    close $fileRef_v1;
}
elsif ($ARGV[0] eq "strmapper")
{
    my $build_dir = $ARGV[1];
    my $inputFile1 = $ARGV[2];
    my $inputFileRef;
    my $fileRef;
    my $fileRef_h;


    $fileRef = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/src/wlcsm_dm_mngr_strmapper.c");
    $fileRef_h = open_filehandle_for_output("$build_dir/userspace/private/apps/wlan/wlcsm_dm/include/wlcsm_dm_mngr_strmapper.h");

    print $fileRef_h "#ifndef __WLCSM_DM_MNGR_STRMAPPER__  \n";
    print $fileRef_h "#define __WLCSM_DM_MNGR_STRMAPPER__ \n\n\n";


    autogen_warning($fileRef);
    autogen_warning($fileRef_h);

    doxygen_header($fileRef, "wlcsm_dm_mngr_strmapper.c");
    doxygen_header($fileRef_h, "wlcsm_dm_mngr_strmapper.h");

    print $fileRef_h "/* DO NOT INCLUDE THIS HEADER FILE DIRECTLY, INSTEAD INCLUDE wlcsm_lib_dm.h */\n\n";

    print $fileRef "#include \"wlcsm_lib_dm.h\"\n";
    print $fileRef_h "#include \"wlcsm_lib_dm_datatype.h\"\n\n";
    #print $fileRef "#include \"wldefs.h\"\n\n";

    print $fileRef "WLCSM_MNGR_STRMAPPER_SET g_wlcsm_mngr_strmapper[]= {\n\n";

    $inputFileRef = open_filehandle_for_input("$build_dir/userspace/private/apps/wlan/wlcsm_dm/$inputFile1");
#output_mdmObjectFile($inputFileRef, $fileRef);
    output_wlcsm_strmapper_file($inputFileRef, $fileRef,$fileRef_h);
    close $inputFileRef;

    print $fileRef "\n\n";
    print $fileRef_h "\n\n#endif\n\n";

    close $fileRef;
    close $fileRef_h;
}
else
{
    usage();
    die "unrecognized command";
}

