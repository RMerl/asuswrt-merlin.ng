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
# It stores data about an object in the MDM data model.
# It also outputs the object in c language format so that
# it can be compiled into the cms_core library.
#
#

package GenObjectNode;
require Exporter;
@ISA = qw(Exporter);


#
# Example of a class
# See Chapter 5, page 290 of Programming Perl
#


# @EXPORT exports the symbols by _default_ to the importing file
# They will not have to use the fully qualified name
@EXPORT = qw(
             $maxInstanceDepth
             fillObjectInfo
             addChildObject
             deleteChildObject
             addObjDescription
             addObjAttribute
             addParamNode
             delParamNode
             getChildObjectCount
             getSupportedChildObjectCount
             getProfile
             outputChildObjectArrayHeader
             outputObjectNode
             outputObjectNodeInXML);

$maxInstanceDepth=0;
$_bbbbb = "blah";  #Is this a bug in Perl?  If the var is not here, module does not compile!

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


    ${$objref}{"ChildObjects"} = [];
    ${$objref}{"Params"} = [];

    return $objref;
}

#
# Fill in the information about this object node.
# This should have been passed in with the constructor, but I don't think
# the perl class constructor takes arguments.
#
sub fillObjectInfo
{
    my $this = shift;
    my $oid = shift;
    my $lockZone = shift;
    my $depth = shift;
    my $name = shift;
    my $specSource = shift;
    my $profile = shift;
    my $requirements = shift;
    my $rw = shift;
    my $abbrev = shift;
    my $hideObjectFromAcs = shift;
    my $always = shift;
    my $prune = shift;
    my $autoOrder = shift;
    my $callRclPreHook = shift;
    my $callRclPostHook = shift;
    my $callStlPostHook = shift;
    my $majorVersion = shift;
    my $minorVersion = shift;

    ${$this}{"oid"} = $oid;
    ${$this}{"lockZone"} = $lockZone;
    ${$this}{"depth"} = $depth;
    ${$this}{"name"} = $name;
    ${$this}{"specSource"} = $specSource;
    ${$this}{"profile"} = $profile;
    ${$this}{"requirements"} = $requirements;
    ${$this}{"supportLevel"} = $rw;
    ${$this}{"shortObjectName"} = $abbrev;
    ${$this}{"hideObjectFromAcs"} = $hideObjectFromAcs;
    ${$this}{"alwaysWriteToConfigFile"} = $always;
    ${$this}{"pruneWriteToConfigFile"} = $prune;
    ${$this}{"autoOrder"} = $autoOrder;
    ${$this}{"callRclPreHook"} = $callRclPreHook;
    ${$this}{"callRclPostHook"}  = $callRclPostHook;
    ${$this}{"callStlPostHook"} = $callStlPostHook;
    ${$this}{"majorVersion"} = $majorVersion;
    ${$this}{"minorVersion"} = $minorVersion;

#     print "fillObjInfo: $name profile=${$this}{\"profile\"}\n";

    my $instanceDepth = $this->getInstanceDepth();
    if ($instanceDepth > $maxInstanceDepth)
    {
        $maxInstanceDepth = $instanceDepth;
    }
}


#
# Modify the prefix portion of the fullpath name of an object.
# This is to allow objects from one part of the TR98 data model to get
# mapped to a different part of the TR181 data model.
#
sub modifyPathPrefix
{
    my $this = shift;
    my $prefixDelete = shift;  # the path prefix to delete
    my $prefixAdd = shift;     # the path prefix to add

    if ($prefixAdd eq $prefixDelete)
    {
        # In most cases, we don't need to modify the prefix, so
        # don't need to do anything here or in children objects.
        return;
    }

    # print "modifyPathPrefix: $prefixDelete ==> $prefixAdd \n";

    # Newer versions of perl require the { to be escaped.
    my $escPrefixDelete = $prefixDelete;
    $escPrefixDelete =~ s/\{/\\{/g;

    # Do the replacement on the fullpath portion of the object name
    ${$this}{name} =~ s/^$escPrefixDelete/$prefixAdd/g;

    # check instance depth again since object path may have changed.
    my $instanceDepth = $this->getInstanceDepth();
    if ($instanceDepth > $maxInstanceDepth)
    {
        $maxInstanceDepth = $instanceDepth;
    }
#    print "modify: after mod ${$this}{\"name\"} depth=$instanceDepth \n";


    # recurse and modify path prefix for any children
    my $i;
    my $numChildren = $this->getChildObjectCount();
    my $childObjArrayRef = ${$this}{ChildObjects};
    for ($i=0; $i < $numChildren; $i++)
    {
#        print "modify: childname ${$childObjArrayRef}[$i]{\"name\"} \n";
        my $childObjRef = ${$childObjArrayRef}[$i];
        $childObjRef->modifyPathPrefix($prefixDelete, $prefixAdd);
    }
}


#
# Add a child object onto my array
#
sub addChildObject
{
    my $this = shift;
    my $cmd = shift;   # head, tail, insertbefore, insertafter
    my $targetObjName = shift;  # if insertbefore or insertafter, the target obj name
    my @newChildObjArray = @_;    # one more more child objects to add
    my $numNewChildren = @newChildObjArray;
    my $childObjectsArrayRef;

#    print ("AddChildObject: ${$this}{\"name\"} adding $numNewChildren children cmd=$cmd\n");

    $childObjectsArrayRef = ${$this}{"ChildObjects"};

    if ($cmd eq "tail")
    {
        # Append this childArray to the array of child objects
        @{$childObjectsArrayRef} = (@{$childObjectsArrayRef}, @newChildObjArray);
    }
    elsif ($cmd eq "head")
    {
        # Prepend this childArray to the array of child objects
        @{$childObjectsArrayRef} = (@newChildObjArray, @{$childObjectsArrayRef});
    }
    elsif ($cmd eq "insertbefore" || $cmd eq "insertafter")
    {
        my $numChildren = @{$childObjectsArrayRef};
        my $i;
        my @secondPart;
        
#        print "addChildObject: Searching for $targetObjName in total=$numChildren\n";
        
        for ($i=0; $i < $numChildren; $i++)
        {
#            print "addChildObject: Considering [$i] ${$childObjectsArrayRef}[$i]{name}\n";
            if (${$childObjectsArrayRef}[$i]{name} eq $targetObjName)
            {
#                print "addChildObject: Found target at index $i\n";
                if ($cmd eq "insertafter")
                {
                    # adjust the index and length for insert after operation
                    $i++;
                }
                
                @secondPart = splice(@{$childObjectsArrayRef}, $i);
                @{$childObjectsArrayRef} = (@{$childObjectsArrayRef},
                                            @newChildObjArray, @secondPart);
                return 0;
            }
        }
        die "CMD=$cmd could not find target object $targetObjName\n";
    }
    else
    {
        die "Unsupported cmd $cmd\n";
    }
}


#
# Delete the specified child object from my array
#
sub deleteChildObject
{
    my $this = shift;
    my $targetObjName = shift;
    my $childObjectsArrayRef;

#    print ("DeleteChildObject: ${$this}{\"name\"} delete $targetObjName\n");

    $childObjectsArrayRef = ${$this}{"ChildObjects"};

    my $numChildren = @{$childObjectsArrayRef};
    my $i;

    for ($i=0; $i < $numChildren; $i++)
    {
#       print "deleteChildObject: Considering [$i] ${$childObjectsArrayRef}[$i]{name}\n";
        if (${$childObjectsArrayRef}[$i]{name} eq $targetObjName)
        {
#           print "addChildObject: Found target at index $i\n";
            splice(@{$childObjectsArrayRef}, $i, 1);
            return 0;
        }
    }
    die "deleteChildObj: could not find target object $targetObjName\n";
}

#
# Add description field to this object
#
sub addObjDescription
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

#
# Add another attribute and value to this object.
#
sub addObjAttribute
{
    my $this = shift;
    my $attr = shift;
    my $value = shift;
    
#    print "addObjAttr: $attr = $value\n";
    ${$this}{$attr} = $value;
}


#
# Add a parameter node which belongs to this object.
#
sub addParamNode
{
    my $this = shift;
    my ($paramRef) = @_;
    my $paramArrayRef;

    $paramArrayRef = ${$this}{"Params"};
    my $numParams = @{$paramArrayRef};

    # Reject duplicate param name
    for (my $i=0; $i < $numParams; $i++)
    {
       if (${$paramArrayRef}[$i]{name} eq ${$paramRef}{name})
       {
            print "addParamNode: Ignore duplicate param name ${$paramRef}{name} in ${$this}{name}\n";
            return;
       }
    }

    # print ("ObjectNode: adding ${$this}{name} -> ${$paramRef}{name}\n\n");
    @{$paramArrayRef} = (@{$paramArrayRef}, $paramRef);
}

#
# Delete a parameter node from this object.
#
sub delParamNode
{
    my $this = shift;
    my $delParamName = shift;
    my $delParamRef;
    my $paramArrayRef;


#    print ("ObjectNode: deleting ${$this}{name} -> $delParamName\n\n");

    $paramArrayRef = ${$this}{"Params"};
    my $numParams = @{$paramArrayRef};
    my $i;

    # find the index of the param that we want to delete
    for ($i=0; $i < $numParams; $i++)
    {
       if (${$paramArrayRef}[$i]{name} eq $delParamName)
       {
#           print "delParamNode: Found target at index $i\n";
            splice(@{$paramArrayRef}, $i, 1);
            return 0;
       }
    }

    if ($i == $numParams)
    {
       die "delParamNode: could not find $delParamName in ${$this}{name}\n";
    }
}


#
# Return the number of child objects
#
sub getChildObjectCount
{
    my $this = shift;
    my $childObjectArrayRef;
    my $num=0;

    if (defined(${$this}{"ChildObjects"}))
    {
        $childObjectArrayRef = ${$this}{"ChildObjects"};
        $num = @{$childObjectArrayRef};
    }

    return $num;
}


#
# Return the number of *supported* child objects
#
sub getSupportedChildObjectCount
{
    my $this = shift;
    my $childObjectArrayRef;
    my $childObjectRef;
    my $num;
    my $numSupported = 0;
    my $i;

    $childObjectArrayRef = ${$this}{"ChildObjects"};
    $num = $this->getChildObjectCount();

    for ($i=0; $i < $num; $i++)
    {
        $childObjectRef = ${$childObjectArrayRef}[$i];
        if ($childObjectRef->isSupported())
        {
            $numSupported++;
        }
    }

    return $numSupported;
}


#
# Return the number of supported parameters belonging to this object
#
sub getSupportedParamCount
{
    my $this = shift;
    my $paramArrayRef;
    my ($num, $numSupported, $i);

    $paramArrayRef = ${$this}{"Params"};
    $num = @{$paramArrayRef};
    $numSupported = 0;

    for ($i=0; $i < $num; $i++)
    {
        $paramRef = ${$paramArrayRef}[$i];
        if ($paramRef->isSupported())
        {
            $numSupported++;
        }
    }

    return $numSupported;
}


#
# Return the profile name of this object
#
sub getProfile
{
    my $this = shift;

    return ${$this}{"profile"};
}


#
# Print out the header to the child object array
#
sub outputChildObjectArrayHeader
{
    my ($this, $fhRef) = @_;
    my $arrayName;

    print $fhRef "\n";
    print $fhRef "/* child objects of ${$this}{\"name\"} */\n";
    print $fhRef "/* in profile ${$this}{\"profile\"} */\n";

    print $fhRef "MdmObjectNode ";
    $arrayName = $this->getChildObjArrayName() . "[]";
    print $fhRef "$arrayName = { \n";
}


#
# Print out the MdmObjectNode structure for this object.
#
sub outputObjectNode
{
    my ($this, $fh) = @_;
    my ($numName, $numChildObjectNodes, $numParamNodes);
    my ($childObjArrayName, $paramArrayName, $childObjCount, $paramCount);
    my $instanceDepth;
    my $objName;


    $numSupportedChildObjectNodes = $this->getSupportedChildObjectCount();
    if ($numSupportedChildObjectNodes == 0)
    {
        $numName = 0;
        $childObjArrayName = "NULL";
        $childObjCount = "0";
    }
    else
    {
        $childObjArrayName = $this->getChildObjArrayName();
        $childObjCount = "sizeof($childObjArrayName)/sizeof(MdmObjectNode)";
    }

    $numParamNodes = $this->getSupportedParamCount();
    if ($numParamNodes == 0)
    {
        $paramArrayName = "NULL";
        $paramCount = 0;
    }
    else
    {
        $paramArrayName = $this->getParamArrayName();
        $paramCount = "sizeof($paramArrayName)/sizeof(MdmParamNode)";
    }


    $instanceDepth = $this->getInstanceDepth();
    $objName = $this->getLastNameComponent();
    $nodeFlags = $this->getNodeFlags();

    print $fh "   { ${$this}{\"oid\"}, /* ${$this}{\"name\"} */\n";
    print $fh "     ${$this}{\"lockZone\"}, /* lockZone */\n";
    print $fh "     \"$objName\", /* object name */\n";
    print $fh "     \"${$this}{\"profile\"}\", /* profile name */\n";
    print $fh "     $nodeFlags, $instanceDepth,/* node flags, instance depth */\n";
    print $fh "     NULL,        /* parent objNode ptr */\n";
    print $fh "     {0, DEFAULT_ACCESS_BIT_MASK, 0, DEFAULT_NOTIFICATION_VALUE, 0, 0, 0}, /* nodeattr value of this objNode */\n";
    print $fh "     NULL, /* pointer to instances of nodeattr values for parameter nodes */\n";
    print $fh "     $paramCount, /* num param nodes */\n";
    print $fh "     $childObjCount, /* num obj nodes */\n";
    print $fh "     $paramArrayName, /* param nodes array */\n";
    print $fh "     $childObjArrayName, /* child obj array */\n";
    print $fh "     0, /* lastWritePid */\n";
    print $fh "     {0, 0}, /* lastWriteTs */\n";
    print $fh "     NULL  /* obj data */\n";  
    print $fh "   }";

}


#
# Print out the parameter nodes for this object.
#
sub outputParams
{
    my ($this, $fh) = @_;
    my ($numParams, $numSupportedParams, $i, $paramArrayRef);
    my $arrayName;
    my $paramsPrinted = 0;

    $paramArrayRef = ${$this}{"Params"};
    $numParams = @{$paramArrayRef};
    $numSupportedParams = $this->getSupportedParamCount();

    if ($numParams == 0)
    {
        return;
    }

    $arrayName = $this->getParamArrayName();
    $arrayName = $arrayName . "[]";

    print $fh "/* params for ${$this}{\"name\"} */\n";
    print $fh "/* in profile ${$this}{\"profile\"} */\n";
    print $fh "MdmParamNode $arrayName = {\n";

    for ($i=0; $i < $numParams; $i++)
    {
        $paramRef = ${$paramArrayRef}[$i];

        if ($paramRef->isSupported() == 0)
        {
            # parameter is not supported, skip it.
            next;
        }

        $paramsPrinted++;

        $paramRef->outputParamNode($fh, ${$this}{"profile"});
    }

    print $fh "};\n\n";

}


#
# Print out the MdmObjectNode structure for this object in XML format.
# This is used to generated the "merged" data model file.
#
sub outputObjectNodeInXML
{
    my ($this, $fh) = @_;
    my ($numName, $numChildObjectNodes, $numParamNodes);
    my ($childObjArrayName, $paramArrayName, $childObjCount, $paramCount);
    my $instanceDepth;
    my $objName;

    print $fh "<object name=\"${$this}{\"name\"}\" ";
    
    print $fh "shortObjectName=\"${$this}{\"shortObjectName\"}\" ";
    
    if (defined(${$this}{"specSource"}))
    {
        print $fh "specSource=\"${$this}{\"specSource\"}\" ";
    }

    print $fh "profile=\"${$this}{\"profile\"}\" ";

    if (defined(${$this}{"requirements"}))
    {
        print $fh "requirements=\"${$this}{\"requirements\"}\" ";
    }
    if (defined(${$this}{"supportLevel"}))
    {
        print $fh "supportLevel=\"${$this}{\"supportLevel\"}\" ";
    }
    if (defined(${$this}{"alwaysWriteToConfigFile"}))
    {
        print $fh "alwaysWriteToConfigFile=\"${$this}{\"alwaysWriteToConfigFile\"}\" ";
    }
    if (defined(${$this}{"pruneWriteToConfigFile"}))
    {
        print $fh "pruneWriteToConfigFile=\"${$this}{\"pruneWriteToConfigFile\"}\" ";
    }
    if (defined(${$this}{"hideObjectFromAcs"}))
    {
        print $fh "hideObjectFromAcs=\"${$this}{\"hideObjectFromAcs\"}\" ";
    }
    if (defined(${$this}{"autoOrder"}))
    {
        print $fh "autoOrder=\"${$this}{\"autoOrder\"}\" ";
    }
    if (defined(${$this}{"callRclPreHook"}))
    {
        print $fh "callRclPreHook=\"${$this}{\"callRclPreHook\"}\" ";
    }
    if (defined(${$this}{"callRclPostHook"}))
    {
        print $fh "callRclPostHook=\"${$this}{\"callRclPostHook\"}\" ";
    }
    if (defined(${$this}{"callStlPostHook"}))
    {
        print $fh "callStlPostHook=\"${$this}{\"callStlPostHook\"}\" ";
    }
    if (defined(${$this}{"majorVersion"}))
    {
        print $fh "majorVersion=\"${$this}{\"majorVersion\"}\" ";
    }
    if (defined(${$this}{"minorVersion"}))
    {
        print $fh "minorVersion=\"${$this}{\"minorVersion\"}\" ";
    }
    
    print $fh "oid=\"${$this}{\"oid\"}\" ";
    print $fh "lockZone=\"${$this}{\"lockZone\"}\" ";
    
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
    
    # now dump out all of this object's params
    my ($numParams, $i, $paramArrayRef, $paramRef);
    
    $paramArrayRef = ${$this}{"Params"};
    $numParams = @{$paramArrayRef};

    for ($i=0; $i < $numParams; $i++)
    {
        $paramRef = ${$paramArrayRef}[$i];
        $paramRef->outputParamNodeInXML($fh);
    }
    
    print $fh "\n\n\n";
}



############################################################################
#
# These are private functions.
#
############################################################################


#
# Return the childObjArray name from the abbreviated name
#
sub getChildObjArrayName
{
    my $this = shift;
    my $name;

    $name = $this->getArrayName();

    $name = $name . "ChildObjArray";

    return $name;
}

#
# Return the paramArray name from the abbreviated name
#
sub getParamArrayName
{
    my $this = shift;
    my $name;

    $name = $this->getArrayName();

    $name = $name . "ParamArray";

    return $name;
}

#
# Return the transformed abbreviated object name suitable for use
# in the array names
#
sub getArrayName
{
    my $this = shift;
    my $arrayName;
    my ($firstLetter, $restOfWord);

    # Get rid of Object from the abbreviated name and lower case the first letter.
    # Objects starting with IGD are special case, igd becomes lower cased
    if (${$this}{"shortObjectName"} =~ /^IGD([\w]*)Object/)
    {
        $firstLetter = "igd";
        if (defined($1))
        {
            $restOfWord = $1;
        }
        else
        {
            $restOfWord = "";
        }
    }
    else
    {
        ${$this}{"shortObjectName"} =~ /([\w])([\w]+)Object/;

        if (!defined($1) || !defined($2))
        {
            die "Could not extract ${$this}{\"shortObjectName\"}";
        }

        $firstLetter = $1;
        $restOfWord = $2;

        $firstLetter =~ tr/[A-Z]/[a-z]/;
    }
    
    $arrayName = $firstLetter . $restOfWord;

    return $arrayName;
}


#
# Return the number of instance markers (".{i}.") in the object name
#
sub getInstanceDepth
{
    my $this = shift;
    my $name;
    my $depth = 0;

    $name = ${$this}{"name"};

    while ((defined($name)) && ($name =~ /([\w\.-]+.\{i\}.)([\w\.{}-]*)/))
    {
        $depth += 1;
        $name = $2;
    }

    return $depth;
}


#
# Return the last name component from the generic object name.
#
sub getLastNameComponent
{
    my $this = shift;
    my $name;
    my $lastComp;

    $name = ${$this}{"name"};

    if ($name eq "InternetGatewayDevice.")
    {
        $lastComp = "InternetGatewayDevice";
    }
    elsif ($name eq "Device.")
    {
        $lastComp = "Device";
    }
    elsif ($name =~ /[\w\.\{\}-]+\.([\w-]+)\.\{i\}\.$/)
    {
        # name ends in .{i}.
        $lastComp = $1;
    }
    elsif ($name =~ /[\w\.\{\}-]+\.([\w-]+)\.$/)
    {
        # name ends in with some letters and then . (not .{i}.)
        $lastComp = $1;
    }


    if (!defined($lastComp))
    {
        die "Could not find last component of $name";
    }

    # print "lastComp=$lastComp\n";

    return $lastComp;
}


#
# Return a string which is the node flags for the ObjectNode
#
sub getNodeFlags
{
    my $this = shift;
    my $flagString = "0";

    #
    # First check if this node is an instance node. 
    # An instance node has a name that ends with .{i}.
    #
    if (${$this}{"name"} =~ /\.\{i\}\.$/)
    {
        $flagString = OBN_INSTANCE_NODE;

        #
        # If this node is an instance node, see if we support dynamic
        # add/delete of instances.
        #
        if (${$this}{"supportLevel"} eq "DynamicInstances")
        {
            $flagString = $flagString . " | OBN_DYNAMIC_INSTANCES";
        }
    }

    if (defined(${$this}{"hideObjectFromAcs"}))
    {
        if ((${$this}{"hideObjectFromAcs"} =~ /true/i) ||
            (${$this}{"hideObjectFromAcs"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
                $flagString = "OBN_HIDE_OBJECT_FROM_ACS";
            }
            else
            {
                $flagString = $flagString . " | OBN_HIDE_OBJECT_FROM_ACS";
            }
        }
    }

    if (defined(${$this}{"pruneWriteToConfigFile"}))
    {
        if ((${$this}{"pruneWriteToConfigFile"} =~ /true/i) ||
            (${$this}{"pruneWriteToConfigFile"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
                $flagString = "OBN_PRUNE_WRITE_TO_CONFIG_FILE";
            }
            else
            {
                $flagString = $flagString . " | OBN_PRUNE_WRITE_TO_CONFIG_FILE";
            }
        }
    }

    if (defined(${$this}{"autoOrder"}))
    {
        if ((${$this}{"autoOrder"} =~ /true/i) ||
            (${$this}{"autoOrder"} =~ /yes/i))
        {
            if ($flagString eq "0")
            {
                $flagString = "OBN_AUTO_ORDER_INSTANCES";
            }
            else
            {
                $flagString = $flagString . " | OBN_AUTO_ORDER_INSTANCES";
            }
        }
    }

    return $flagString;
}


#
# Return 1 if this object node is supported as Present, DynamicInstances, or MultipleInstances
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
