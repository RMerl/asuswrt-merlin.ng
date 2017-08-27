#!/usr/bin/perl

# radius2ldif.pl
#
#        To test this program, do the following
#Take a radius users' file, for example with:
#
#myuser Password = "apassword"
#        User-Service = Framed-User,
#        Framed-Protocol = PPP,
#        Framed-Address = 255.255.255.255,
#        Framed-Netmask = 255.255.255.255,
#        Ascend-Metric = 2,
#        Framed-Routing = None,
#        Framed-Compression = 0,
#        Ascend-Idle-Limit = 0,
#        Ascend-Maximum-Time = 36000
#
#and do:
#
#cat users | ./radius2ldif
#
#Output is:
#dn: cn=myuser, ou=Hardware, ou=EDUCAMADRID, ou=People, o=icm.es
#objectclass: top
#objectclass: person
#objectclass: radiusprofile
#cn: myuser
#sn: myuser
#userpassword: apassword
#radiusServiceType: Framed-User
#radiusFramedProtocol: PPP
#radiusFramedIPAddress: 255.255.255.255
#radiusFramedIPNetmask: 255.255.255.255
#radiusFramedRouting: None
#radiusFramedCompression: 0
#
#dn: ou=RadiusUser, ou=Groups, o=icm.es
#description: RadiusUser
#objectclass: top
#objectclass: groupOfUniqueNames
#cn: RadiusUser
#uniquemember: dn: cn=myuser, ou=Hardware, ou=EDUCAMADRID, ou=People, o=icm.es
#
# (c) 2000 Javier Fern'andez-Sanguino Pen~a  <jfs@computer.org>
# -------------------------------------------------------------------------
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
# -----------------------------------------------------------------------


# TODO:
# currently does not encrypt passwords (takes them from outside file)

# Command line options
# -d : debugging output
# -p : give only password
# -m : set entry to modify ldap attributes
# -f : read encrypted passwords from file
use Getopt::Std;
getopts('dpmf:');
$debug = $opt_d;

%passwords;
# This might or might not be necessary depending if your LDAP server
# when importing from ldif introduces crypted passwords in the LDAP db
# (not necessary for Netscape's Directory Server)
read_passwds ($opt_f) if $opt_f;

# USER CONFIGURATION
# ------------------
$usermatch = ".*"; # only add users matching this
# WARNING: in order to add  *all* users set this to ".*" NOT ""

# LDAP configuration
$domain = "o=icm.es";
$basedn = ", ou=Hardware, ou=EDUCAMADRID, ou=People, $domain";
$predn = "dn: cn=";
$uniquemembers = 1;
$groupname = "RadiusUser"; # group to add in the LDAP, if null will not add
$group = "\n\ndn: ou=$groupname, ou=Groups, $domain";
# Only useful for adding the group (not yet implemented)
$addgroup = $group."\ndescription: $groupname\nobjectclass: top";
if ( $uniquemembers ) {
$addgroup = $addgroup."\nobjectclass: groupOfUniqueNames";
} else {
$addgroup = $addgroup."\nobjectclass: groupOfNames";
}
$addgroup = $addgroup."\ncn: $groupname";
# The following group must be created first
# (ldif entry), the script will *not* create it
#cn=$group,ou=Groups,o=icm.es
#description=whatever
#objectclass=top
#objectclass=groupOfUniqueNames
# (or objectclass=groupOfNames)
#cn=$group
# Required: person (for userpasswords) and radiusprofile (<draft-aboba-radius-02.txt> 5 February 1998)
@objectClass = ( "top", "person" , "radiusprofile" );


# Mapping of entries (use lower case so no check needs to be make)
# From freeradius: rlm_ldap.c
#        { "radiusServiceType", "Service-Type" },
#        { "radiusFramedProtocol", "Framed-Protocol" },
#        { "radiusFramedIPAddress", "Framed-IP-Address" },
#        { "radiusFramedIPNetmask", "Framed-IP-Netmask" },
#        { "radiusFramedRoute", "Framed-Route" },
#        { "radiusFramedRouting", "Framed-Routing" },
#        { "radiusFilterId", "Filter-Id" },
#        { "radiusFramedMTU", "Framed-MTU" },
#        { "radiusFramedCompression", "Framed-Compression" },
#        { "radiusLoginIPHost", "Login-IP-Host" },
#        { "radiusLoginService", "Login-Service" },
#        { "radiusLoginTCPPort", "Login-TCP-Port" },
#        { "radiusCallbackNumber", "Callback-Number" },
#        { "radiusCallbackId", "Callback-Id" },
#        { "radiusFramedRoute", "Framed-Route" },
#        { "radiusFramedIPXNetwork", "Framed-IPX-Network" },
#        { "radiusClass", "Class" },
#        { "radiusSessionTimeout", "Session-Timeout" },
#        { "radiusIdleTimeout", "Idle-Timeout" },
#        { "radiusTerminationAction", "Termination-Action" },
#        { "radiusCalledStationId", "Called-Station-Id" },
#        { "radiusCallingStationId", "Calling-Station-Id" },
#        { "radiusLoginLATService", "Login-LAT-Service" },
#        { "radiusLoginLATNode", "Login-LAT-Node" },
#        { "radiusLoginLATGroup", "Login-LAT-Group" },
#        { "radiusFramedAppleTalkLink", "Framed-AppleTalk-Link" },
#        { "radiusFramedAppleTalkNetwork", "Framed-AppleTalk-Network" },
#        { "radiusFramedAppleTalkZone", "Framed-AppleTalk-Zone" },
#        { "radiusPortLimit", "Port-Limit" },
#        { "radiusLoginLATPort", "Login-LAT-Port" },
# You can change to the mappings below like this
# cat radius2ldif.pl | grep ^# | \
# perl -ne 'if ( /\{ \"(.*?)\", \"(.*?)\" \}/ ) \
# { $attr=lc $2; print "\$mapping{\"$attr\"} = \"$1\";\n" ; } '


# Warning: sometimes password must be encrypted before sent to the LDAP
# Which Perl libraries are available? Only way I find is through
# Netscape's NDS getpwenc.
# However NDS does the cyphering even if sending plain passwords
# (do all LDAP's do this?)
# TODO: test with OpenLDAP
$mapping{'password'} = "userpassword";
$mapping{'service-type'} = "radiusServiceType";
$mapping{'framed-protocol'} = "radiusFramedProtocol";
$mapping{'framed-ip-address'} = "radiusFramedIPAddress";
$mapping{'framed-ip-netmask'} = "radiusFramedIPNetmask";
$mapping{'framed-route'} = "radiusFramedRoute";
$mapping{'framed-routing'} = "radiusFramedRouting";
$mapping{'filter-id'} = "radiusFilterId";
$mapping{'framed-mtu'} = "radiusFramedMTU";
$mapping{'framed-compression'} = "radiusFramedCompression";
$mapping{'login-ip-host'} = "radiusLoginIPHost";
$mapping{'login-service'} = "radiusLoginService";
$mapping{'login-tcp-port'} = "radiusLoginTCPPort";
$mapping{'callback-number'} = "radiusCallbackNumber";
$mapping{'callback-id'} = "radiusCallbackId";
$mapping{'framed-ipx-network'} = "radiusFramedIPXNetwork";
$mapping{'class'} = "radiusClass";
$mapping{'session-timeout'} = "radiusSessionTimeout";
$mapping{'idle-timeout'} = "radiusIdleTimeout";
$mapping{'termination-action'} = "radiusTerminationAction";
$mapping{'called-station-id'} = "radiusCalledStationId";
$mapping{'calling-station-id'} = "radiusCallingStationId";
$mapping{'login-lat-service'} = "radiusLoginLATService";
$mapping{'login-lat-node'} = "radiusLoginLATNode";
$mapping{'login-lat-group'} = "radiusLoginLATGroup";
$mapping{'framed-appletalk-link'} = "radiusFramedAppleTalkLink";
$mapping{'framed-appletalk-network'} = "radiusFramedAppleTalkNetwork";
$mapping{'framed-appletalk-zone'} = "radiusFramedAppleTalkZone";
$mapping{'port-limit'} = "radiusPortLimit";
$mapping{'login-lat-port'} = "radiusLoginLATPort";

# Must be added to rlm_ldap.c (change this to suite your needs)
# (really not all since they are in the /etc/raddb/dictionary.compat)
$mapping{'framed-address'} = "radiusFramedIPAddress";
$mapping{'framed-ip-route'} = "radiusFramedRoute";
$mapping{'framed-netmask'} = "radiusFramedIPNetmask";
$mapping{'user-service'} = "radiusServiceType";
# Since this might not change they could be placed in the DEFAULT
# user insted of the LDAP
#$mapping{'ascend-metric'} = "radiusAscendMetric";
#$mapping{'ascend-idle-limit'} = "radiusAscendIdleLimit";
# But this really ought to be there :
$mapping{'callback_number'} = "radiusCallbackNumber";


# Footer of ldif entries
$footer = "\n\n";
$startentry = 0;

while ($line=<STDIN>) {
	chomp $line;
	if ( $line =~ /^[\s\t]*$/  && $startentry) {
		$startentry = 0 ;
		print $footer;
	}
	# Start line is hardcoded must be uid followed by password
	# this could be changed to use any other parameter however
	if ( $line =~ /^(\w+)\s*\t*(?:User-)?Password=(\w+)/ ) {
		$uid = $1;
		$password= $2;
		$password = $passwords{$password} if $opt_f;
	if ( $uid =~ /$usermatch/ ) {
		$startentry = 1;
		$dn=$predn.$uid.$basedn; # Start of LDIF entry
		$header = "$dn\n";
		push @userlist, $dn;
		if ( $opt_m ) {
			$header= $header."changetype: modify\n";
		} else {
			for (my $i=0; $i < $#objectClass+1; $i++) {
				$header = $header."objectclass: ".$objectClass[$i]."\n";
			}
		}
		print $header if !$opt_m;
		print_entry ("cn",$uid);
		print_entry ("sn",$uid);
		# The following might be necessary (depending on the groups)
		# of the object
		#print "replace: uid\n" if $opt_m;
		#print "uid: $uid\n";
		#print "replace: givenname\n" if $opt_m;
		#print "givenname: $uid\n";
		print_entry ($mapping{'password'},$password);
	}
	}
	# Do this only for entries detected
	if ( $startentry  && ! $opt_p ) {
		#Take anything that starts with a tab or spaces
		# and ends (sometimes) with a comma
		if ( $line =~ /^[\t\s]+(.*?)\s+=\s+(.*?),*$/ ) {
			$parameter = lc $1;
			$value = $2;
			print "DEBUG: Got :$parameter=$value\n" if $debug;
			if ( defined $mapping{$parameter}  && $mapping{$parameter} ne "" ) {
				print_entry ($mapping{$parameter},$value);
			} # of if defined mapping
			else {
				print "DEBUG: Parameter $parameter not known\n" if $debug;
				}
		} # of if line
	} # of if startentry

} # of while


# The list of users in the group
if ( $group ) {
	if ( ! $opt_m ) {
		print "$addgroup\n";
	}
	else {
		print "\n\n$group\n";
		print "changetype: modify\n" ;
	}
	foreach $user ( @userlist ) {
		$member = "member: ";
		$member = "uniquemember: " if $uniquemembers;
		print "$member$user\n";
	}
}

exit 0;

sub read_passwds {
# Reads passwords from a file in order to get the crypted
# version, the file must be of the following format:
# password	cryptedversion
	my ($file)=@_;
	open (PASSWD,"< $file") or die ("Could not open $file: $!\n");

	while ($line = <PASSWD>) {
		chomp $line;
		if ( $line =~ /^(\w+)[\t\s]+(.*?)$/ ) {
			$passwords{$1}=$2;
		}
	}
	close PASSWD;

	return 0;
}

sub print_entry {
# Prints and ldif entry given name and value
# if this is a modification it will print header and footer
	my ($name, $value) = @_;
	print $header."replace: $name\n" if $opt_m;
	print $name.": ".$value."\n";
	print $footer if $opt_m;
	return 0;
}

