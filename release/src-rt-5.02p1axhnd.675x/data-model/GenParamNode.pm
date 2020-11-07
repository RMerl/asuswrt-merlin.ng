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
# This is a perl class.
# It stores data about a parameter in the MDM data model.
# It also outputs the the parameter in c language format so that
# it can be compiled into the cms_core library.
#
#

package GenParamNode;
require Exporter;
@ISA = qw(Exporter);


#
# Example of a class
# See Chapter 5, page 290 of Programming Perl
#


# @EXPORT exports the symbols by _default_ to the importing file
# They will not have to use the fully qualified name
@EXPORT = qw(
             $maxParamNameLength
             fillParamInfo
             addParamDescription
             isBool
             isSupported
             outputParamNode
             outputParamNodeInXML);


$maxParamNameLength = 0;
$_aaaaa = "blah";  #Is this a bug in Perl?  If the var is not here, module does not compile!

#
# constructor
#
sub new
{
    # {} returns an anonomus hash
    # bless causes the anonomous hash to be associated with this package.
    # return bless {};

    my $objref = {};
    bless $objref;

    return $objref;
}


#
# Fill in the information about this param node.
# This should have been passed in with the constructor, but I don't think
# the perl class constructor takes arguments.
#
sub fillParamInfo
{
    my $this = shift;
    my $name = shift;
    my $alternateparamname = shift;
    my $type = shift;
    my $specSource = shift;
    my $defaultValueWasSet = shift;
    my $defaultValue = shift;
    my $profile = shift;
    my $requirements = shift;
    my $mayDenyActiveNotification = shift;
    my $denyActiveNotification = shift;
    my $forcedActiveNotification = shift;
    my $alwaysWriteToConfigFile = shift;
    my $neverWriteToConfigFile = shift;
    my $countPersistentForConfigFile = shift;
    my $transferDataBuffer = shift;
    my $isTr69Password = shift;
    my $isConfigPassword = shift;
    my $hideParameterFromAcs = shift;
    my $supportLevel = shift;
    my $majorVersion = shift;
    my $minorVersion = shift;
    my $notifySskLowerLayersChanged = shift;
    my $notifySskAliasChanged = shift;
    my $autoGenerateAlias = shift;
    my $len;

    ${$this}{"name"} = $name;
    ${$this}{"alternateparamname"} = $alternateparamname;

    $len = length($name);
    if ($len > $maxParamNameLength)
    {
        $maxParamNameLength = $len;
    }
    
    ${$this}{"defaultValueWasSet"} = $defaultValueWasSet;
    ${$this}{"defaultValueOrig"} = $defaultValue;
    ${$this}{"defaultValue"} = $this->fixupDefaultValue($defaultValue);

    ${$this}{"origType"} = $type;
    ${$this}{"type"} = $this->fixupType($type);
    
    ${$this}{"specSource"} = $specSource;

    ${$this}{"profile"} = $profile;
    
    ${$this}{"requirements"} = $requirements;

    ${$this}{"mayDenyActiveNotification"} = $mayDenyActiveNotification;

    ${$this}{"denyActiveNotification"} = $denyActiveNotification;

    ${$this}{"forcedActiveNotification"} = $forcedActiveNotification;

    ${$this}{"alwaysWriteToConfigFile"} = $alwaysWriteToConfigFile;

    ${$this}{"neverWriteToConfigFile"} = $neverWriteToConfigFile;
    
    ${$this}{"countPersistentForConfigFile"} = $countPersistentForConfigFile;

    ${$this}{"transferDataBuffer"} = $transferDataBuffer;

    ${$this}{"isTr69Password"} = $isTr69Password;

    ${$this}{"isConfigPassword"} = $isConfigPassword;

    ${$this}{"hideParameterFromAcs"} = $hideParameterFromAcs;

    ${$this}{"supportLevel"} = $supportLevel;
    
    ${$this}{"majorVersion"} = $majorVersion;
    
    ${$this}{"minorVersion"} = $minorVersion;
    
    ${$this}{"notifySskLowerLayersChanged"} = $notifySskLowerLayersChanged;
    
    ${$this}{"notifySskAliasChanged"} = $notifySskAliasChanged;
    
    ${$this}{"autoGenerateAlias"} = $autoGenerateAlias;

    if (defined (${$this}{"autoGenerateAlias"}) &&
        ((${$this}{"autoGenerateAlias"} =~ /true/i) ||
         (${$this}{"autoGenerateAlias"} =~ /yes/i)))
    {
        my $defVal = ${$this}{"defaultValue"};

        if ($defVal =~ /^cpe-/)
        {
            # OK, default value string begins with correct prefix, now check
            # length.  Max len of Alias is 64, minus 11 chars for the dash
            # and max instance id length, which leaves 53 bytes for the default
            # value part.
            my $len = length($defVal);
            if ($len > 53)
            {
                die "Default value $defVal too long, max len 53"
            }
        }
        else
        {
            die "When autoGenerateAlias=true, defaultValue must contain cpe-uniqueprefix-  got $defVal instead";
        }
    }

}


#
# Add description field to this parameter
#
sub addParamDescription
{
    my $this = shift;
    my $source = shift;
    my $desc = shift;
    
    # source could be TRx, BROADCOM, or Custom
    if ($source =~ /TRx/i)
    {
        ${$this}{"descTRx"} = $desc;
    }
    elsif ($source =~ /BROADCOM/i)
    {
        ${$this}{"descBcm"} = $desc;
    }
    else
    {
        ${$this}{"descCust"} = $desc;
    }
}


sub setMinMaxValues
{
    my $this = shift;
    my ($min, $max) = @_;

    if (${$this}{"type"} eq "MPT_BOOLEAN") {
        die "setMinMaxValues called on boolean param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_STRING") {
        die "setMinMaxValues called on string param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_BASE64") {
        die "setMinMaxValues called on base64 param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_UUID") {
        die "setMinMaxValues called on UUID param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_IP_ADDR") {
        die "setMinMaxValues called on IPAddress param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_MAC_ADDR") {
        die "setMinMaxValues called on MACAddress param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_STATS_COUNTER32") {
        die "setMinMaxValues called on StatsCounter32 param!\n";
    }
    elsif (${$this}{"type"} eq "MPT_STATS_COUNTER64") {
        die "setMinMaxValues called on StatsCounter64 param!\n";
    }

    ${$this}{"minValue"} = $min;
    ${$this}{"maxValue"} = $max;
}

sub setMaxLength
{
    my $this = shift;
    my ($maxLength) = @_;

    if ((${$this}{"type"} ne "MPT_STRING") && 
        (${$this}{"type"} ne "MPT_HEX_BINARY") && 
        (${$this}{"type"} ne "MPT_BASE64") &&
        (${$this}{"type"} ne "MPT_UUID") &&
        (${$this}{"type"} ne "MPT_IP_ADDR") &&
        (${$this}{"type"} ne "MPT_MAC_ADDR")) { 
        die "setMaxLength called on non-string param!\n";
    }
    
    if (defined (${$this}{"autoGenerateAlias"}) &&
        ((${$this}{"autoGenerateAlias"} =~ /true/i) ||
         (${$this}{"autoGenerateAlias"} =~ /yes/i)))
    {
        if ($maxLength != 64)
        {
            die "MaxLength for Alias param should be 64!  got $maxLength instead\n";
        }
    }

    ${$this}{"maxLength"} = $maxLength;
}

sub setValidValuesArray
{
    my $this = shift;
    my ($vsa) = @_;

    if (${$this}{"type"} ne "MPT_STRING") {
        die "setValidValuesArray called on non-string param!\n";
    }

    ${$this}{"validValuesArray"} = $vsa;
#    print "setValidValuesArray: ${$this}{name} = ${$this}{validValuesArray}\n";
}


#
# Return 1 if this param node is supported either as ReadOnly or ReadWrite
#
sub isSupported
{
    my $this = shift;

    if (${$this}{"supportLevel"} eq "NotSupported")
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

#
# Return 1 if this param node is of BOOL type
#
sub isBool
{
    my $this = shift;

    if (${$this}{"type"} eq "MPT_BOOLEAN")
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#
# Return 1 if this param node is of string type
#
sub isString
{
    my $this = shift;

    if (${$this}{"type"} eq "MPT_STRING")
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#
# Return 1 if this param node is of int type
#
sub isInt
{
    my $this = shift;

    if (${$this}{"type"} eq "MPT_INTEGER")
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#
# Return 1 if this param node is of uint type
#
sub isUnsignedInt
{
    my $this = shift;

    if (${$this}{"type"} eq "MPT_UNSIGNED_INTEGER")
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



#
# Print out the MdmParamNode structure for this parameter.
# offset in object is now calculated at runtime so that parameter
# profile defines can be accomodated.  See oal_mdm.c.
#
sub outputParamNode
{
    my ($this, $fh, $parentProfile) = @_;
    my $nodeFlags;
    my $def;
    my $printEndif = 0;
    my ($vMinPrefix, $vMaxPrefix);

    $nodeFlags = $this->getNodeFlags();

    if (($parentProfile ne ${$this}{"profile"}) && !(${$this}{"profile"} =~ /unspecified/i))
    {
        $def = Utils::convertProfileNameToPoundDefine(${$this}{"profile"});
        print $fh "#ifdef $def\n";
        $printEndif = 1;
    }

    if (defined(${$this}{"alternateparamname"}))
    {
        print $fh "   { \"${$this}{\"alternateparamname\"}\",\n";
    }
    else 
    {
        print $fh "   { \"${$this}{\"name\"}\",\n";
    }

    print $fh "     \"${$this}{\"profile\"}\", /* profile name */\n";

    print $fh "     NULL, /* parent ptr */\n";
    print $fh "     ${$this}{\"type\"},\n";
    print $fh "     $nodeFlags, 0, /* node flags, offset in obj */\n";
    if (${$this}{"defaultValue"} eq "NULL")
    {
        print $fh "     NULL, /* default val */\n";
    }
    else
    {
        print $fh "     \"${$this}{\"defaultValue\"}\", /* default val */\n";
    }
    print $fh "     NULL, /* suggested val */\n";


    if (defined(${$this}{"minValue"}))
    {
        # this must be either an int or uint
        if (${$this}{"maxValue"} eq "4294967295")
        {
            ${$this}{"maxValue"} .= "UL";
        }
        if (${$this}{"maxValue"} eq "18446744073709551615")
        {
            ${$this}{"maxValue"} .= "ULL";
        }

        print $fh "     {(void *) ${$this}{minValue}, (void *) ${$this}{maxValue}}  /* validator data */\n";  
    }
    elsif (defined(${$this}{"maxLength"}))
    {
        print $fh "     {(void *) 0, (void *) ${$this}{maxLength}}  /* validator data */\n";  
    }
    elsif (defined(${$this}{"validValuesArray"}))
    {
        print $fh "     { ${$this}{validValuesArray}, (void *) 0}  /* validator data */\n";  
    }
    else
    {
        print $fh "     {(void *) 0, (void *) 0}  /* validator data */\n";  
    }

    print $fh "   },\n";

    if ($printEndif == 1)
    {
        print $fh "#endif\n";
    }
}


#
# Print out this parameter node in XML format.
# This is used to generated the "merged" data model file.
# 
sub outputParamNodeInXML
{
    my ($this, $fh) = @_;
    my $nodeFlags;
    my $def;
    my $printEndif = 0;
    my ($vMinPrefix, $vMaxPrefix);

    print $fh "  <parameter name=\"${$this}{\"name\"}\" ";
    print $fh "type=\"${$this}{\"origType\"}\" ";
    if (defined(${$this}{"specSource"}))
    {
        print $fh "specSource=\"${$this}{\"specSource\"}\" ";
    }
    if (defined(${$this}{"profile"}))
    {
        print $fh "profile=\"${$this}{\"profile\"}\" ";
    }
    if (defined(${$this}{"requirements"}))
    {
        print $fh "requirements=\"${$this}{\"requirements\"}\" ";
    }
    if (defined(${$this}{"mayDenyActiveNotification"}))
    {
        print $fh "mayDenyActiveNotification=\"${$this}{\"mayDenyActiveNotification\"}\" ";
    }
    if (defined(${$this}{"denyActiveNotification"}))
    {
        print $fh "denyActiveNotification=\"${$this}{\"denyActiveNotification\"}\" ";
    }
    if (defined(${$this}{"supportLevel"}))
    {
        print $fh "supportLevel=\"${$this}{\"supportLevel\"}\" ";
    }
    if (defined(${$this}{"defaultValueWasSet"}))
    {
        if (defined(${$this}{"defaultValueOrig"}) && (${$this}{"defaultValueOrig"} ne "NULL"))
        {
            print $fh "defaultValue=\"${$this}{\"defaultValueOrig\"}\" ";
        }
        else
        {
            print $fh "defaultValue=\"\" ";
        }
    }
    if (defined(${$this}{"validValuesArray"}))
    {
        print $fh "validValuesArray=\"${$this}{\"validValuesArray\"}\" ";
    }
    if (defined(${$this}{"minValue"}))
    {
        print $fh "minValue=\"${$this}{\"minValue\"}\" ";
    }
    if (defined(${$this}{"maxValue"}))
    {
        print $fh "maxValue=\"${$this}{\"maxValue\"}\" ";
    }
    if (defined(${$this}{"maxLength"}))
    {
        print $fh "maxLength=\"${$this}{\"maxLength\"}\" ";
    }
    if (defined(${$this}{"isTr69Password"}))
    {
        print $fh "isTr69Password=\"${$this}{\"isTr69Password\"}\" ";
    }
    if (defined(${$this}{"hideParameterFromAcs"}))
    {
        print $fh "hideParameterFromAcs=\"${$this}{\"hideParameterFromAcs\"}\" ";
    }
    if (defined(${$this}{"isConfigPassword"}))
    {
        print $fh "isConfigPassword=\"${$this}{\"isConfigPassword\"}\" ";
    }
    if (defined(${$this}{"alwaysWriteToConfigFile"}))
    {
        print $fh "alwaysWriteToConfigFile=\"${$this}{\"alwaysWriteToConfigFile\"}\" ";
    }
    if (defined(${$this}{"neverWriteToConfigFile"}))
    {
        print $fh "neverWriteToConfigFile=\"${$this}{\"neverWriteToConfigFile\"}\" ";
    }
    if (defined(${$this}{"countPersistentForConfigFile"}))
    {
        print $fh "countPersistentForConfigFile=\"${$this}{\"countPersistentForConfigFile\"}\" ";
    }
    if (defined(${$this}{"transferDataBuffer"}))
    {
        print $fh "transferDataBuffer=\"${$this}{\"transferDataBuffer\"}\" ";
    }
    if (defined(${$this}{"majorVersion"}))
    {
        print $fh "majorVersion=\"${$this}{\"majorVersion\"}\" ";
    }
    if (defined(${$this}{"minorVersion"}))
    {
        print $fh "minorVersion=\"${$this}{\"minorVersion\"}\" ";
    }
    if (defined(${$this}{"notifySskLowerLayersChanged"}))
    {
        print $fh "notifySskLowerLayersChanged=\"${$this}{\"notifySskLowerLayersChanged\"}\" ";
    }
    if (defined(${$this}{"notifySskAliasChanged"}))
    {
        print $fh "notifySskAliasChanged=\"${$this}{\"notifySskAliasChanged\"}\" ";
    }
    if (defined(${$this}{"autoGenerateAlias"}))
    {
        print $fh "autoGenerateAlias=\"${$this}{\"autoGenerateAlias\"}\" ";
    }

    if (defined(${$this}{"alternateparamname"}))
    {
        print $fh "AlternateParamName=\"${$this}{\"alternateparamname\"}\" ";
    }

    print $fh "/>\n";
    
    if (defined ${$this}{descTRx})
    {
        print $fh ${$this}{descTRx};
    }
    if (defined ${$this}{descBcm})
    {
        print $fh ${$this}{descBcm};
    }
    if (defined ${$this}{descCust})
    {
        print $fh ${$this}{descCust};
    }
    print $fh "\n";
}


############################################################################
#
# These are private functions.
#
############################################################################


#
# Take a default/initial/suggested value string and transform it to
# a form that is usable in the c tree files.
#
sub fixupDefaultValue
{
    my ($this, $name) = @_;

#    chomp($name);

    if (($name eq "-") || ($name eq "NA"))
    {
        return "NULL";
    }

    # this matches quoted strings
    if ($name =~ /&quot;([\w-]+)&quot;/)
    {
        return $1;
    }

    # this matches strings in <brackets>
    if ($name =~ /&lt;([\w-]+)&gt;/)
    {
        return $1;
    }

    # strip out trailing space on numbers
    if ($name =~ /^([\-]*[\d]+)[\s]*$/)
    {
        return $1;
    }

    return $name;
}


#
# Take the type string from the MDM spreadsheet and transform it
# to a MdmParamTypes enumeration.
#
sub fixupType
{
    my ($this, $type) = @_;

    if ($type eq "string")
    {
        return "MPT_STRING";
    }
    elsif ($type eq "int")
    {
        return "MPT_INTEGER";
    }
    elsif ($type eq "unsignedInt")
    {
        return "MPT_UNSIGNED_INTEGER";
    }
    elsif ($type eq "long")
    {
        return "MPT_LONG64";
    }
    elsif ($type eq "unsignedLong")
    {
        return "MPT_UNSIGNED_LONG64";
    }
    elsif ($type eq "boolean")
    {
        return "MPT_BOOLEAN";
    }
    elsif ($type eq "dateTime" || $type eq "DateTime")
    {
        return "MPT_DATE_TIME";
    }
    elsif ($type eq "base64")
    {
        return "MPT_BASE64";
    }
    elsif ($type eq "hexBinary")
    {
        return "MPT_HEX_BINARY";
    }
    elsif ($type eq "UUID")
    {
        return "MPT_UUID";
    }
    elsif ($type eq "IPAddress")
    {
        return "MPT_IP_ADDR";
    }
    elsif ($type eq "MACAddress")
    {
        return "MPT_MAC_ADDR";
    }
    elsif ($type eq "StatsCounter32")
    {
        return "MPT_STATS_COUNTER32";
    }
    elsif ($type eq "StatsCounter64")
    {
        return "MPT_STATS_COUNTER64";
    }
    else
    {
        die "Unrecognized param type $type on ${$this}{\"name\"}";
    }
}

#
# Return a string which is the node flags for this ParamNode
#
sub getNodeFlags
{
    my $this = shift;
    my $flagString = "0";

    if (defined(${$this}{"denyActiveNotification"}))
    {
        if ((${$this}{"denyActiveNotification"} =~ /true/i) ||
            (${$this}{"denyActiveNotification"} =~ /yes/i))
        {
            $flagString = "PRN_DENY_ACTIVE_NOTIFICATION";
        }
    }

    if (defined(${$this}{"forcedActiveNotification"}))
    {
        if ((${$this}{"forcedActiveNotification"} =~ /true/i) ||
            (${$this}{"forcedActiveNotification"} =~ /yes/i))
        {
            $flagString = "PRN_FORCED_ACTIVE_NOTIFICATION";
        }
    }

    if (defined(${$this}{"alwaysWriteToConfigFile"}))
    {
        if ((${$this}{"alwaysWriteToConfigFile"} =~ /true/i) ||
            (${$this}{"alwaysWriteToConfigFile"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_ALWAYS_WRITE_TO_CONFIG_FILE";
            }
            else
            {
               $flagString = $flagString . " | PRN_ALWAYS_WRITE_TO_CONFIG_FILE";
            }
        }
    }

    if (defined(${$this}{"neverWriteToConfigFile"}))
    {
        if ((${$this}{"neverWriteToConfigFile"} =~ /true/i) ||
            (${$this}{"neverWriteToConfigFile"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_NEVER_WRITE_TO_CONFIG_FILE";
            }
            else
            {
               $flagString = $flagString . " | PRN_NEVER_WRITE_TO_CONFIG_FILE";
            }
        }
    }
    
    if (defined(${$this}{"countPersistentForConfigFile"}))
    {
        if ((${$this}{"countPersistentForConfigFile"} =~ /true/i) ||
            (${$this}{"countPersistentForConfigFile"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_COUNT_PERSISTENT_INSTANCE";
            }
            else
            {
               $flagString = $flagString . " | PRN_COUNT_PERSISTENT_INSTANCE";
            }
        }
    }

    if (defined(${$this}{"transferDataBuffer"}))
    {
        if ((${$this}{"transferDataBuffer"} =~ /true/i) ||
            (${$this}{"transferDataBuffer"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_TRANSFER_DATA_BUFFER";
            }
            else
            {
               $flagString = $flagString . " | PRN_TRANSFER_DATA_BUFFER";
            }
        }
    }

    if (defined(${$this}{"isTr69Password"}))
    {
        if ((${$this}{"isTr69Password"} =~ /true/i) ||
            (${$this}{"isTr69Password"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_TR69_PASSWORD";
            }
            else
            {
               $flagString = $flagString . " | PRN_TR69_PASSWORD";
            }
        }
    }

    if (defined(${$this}{"isConfigPassword"}))
    {
        if ((${$this}{"isConfigPassword"} =~ /true/i) ||
            (${$this}{"isConfigPassword"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_CONFIG_PASSWORD";
            }
            else
            {
               $flagString = $flagString . " | PRN_CONFIG_PASSWORD";
            }
        }
    }

    if (defined(${$this}{"hideParameterFromAcs"}))
    {
        if ((${$this}{"hideParameterFromAcs"} =~ /true/i) ||
            (${$this}{"hideParameterFromAcs"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_HIDE_PARAMETER_FROM_ACS";
            }
            else
            {
               $flagString = $flagString . " | PRN_HIDE_PARAMETER_FROM_ACS";
            }
        }
    }

    if (${$this}{"supportLevel"} =~ /ReadWrite/i)
    {
        if ($flagString eq "0")
        {
            $flagString = PRN_WRITABLE;
        }
        else
        {
            $flagString = $flagString . " | PRN_WRITABLE";
        }
    }
    
    if (defined(${$this}{"notifySskLowerLayersChanged"}))
    {
        if ((${$this}{"notifySskLowerLayersChanged"} =~ /true/i) ||
            (${$this}{"notifySskLowerLayersChanged"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_NOTIFY_SSK_LOWERLAYERS_CHANGED";
            }
            else
            {
               $flagString = $flagString . " | PRN_NOTIFY_SSK_LOWERLAYERS_CHANGED";
            }
        }
    }
    
    if (defined(${$this}{"notifySskAliasChanged"}))
    {
        if ((${$this}{"notifySskAliasChanged"} =~ /true/i) ||
            (${$this}{"notifySskAliasChanged"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_NOTIFY_SSK_ALIAS_CHANGED";
            }
            else
            {
               $flagString = $flagString . " | PRN_NOTIFY_SSK_ALIAS_CHANGED";
            }
        }
    }
    
    if (defined(${$this}{"autoGenerateAlias"}))
    {
        if ((${$this}{"autoGenerateAlias"} =~ /true/i) ||
            (${$this}{"autoGenerateAlias"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
               $flagString = "PRN_AUTO_GENERATE_ALIAS";
            }
            else
            {
               $flagString = $flagString . " | PRN_AUTO_GENERATE_ALIAS";
            }
        }
    }

    return $flagString;
}
