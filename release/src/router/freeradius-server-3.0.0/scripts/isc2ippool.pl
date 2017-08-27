#!/usr/bin/perl

# isc2ippool	Insert ISC DHCPD lease entries into SQL database (ippool).
#
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
#
#    Copyright (C) 2012 Arran Cudbard-Bell (a.cudbardb@freeradius.org)

use warnings;
use strict;

use DateTime;
use DateTime::Format::Strptime;
use DateTime::Format::DBI;

use Getopt::Long;
use Text::DHCPLeases;
use DBI;

my $lease_file		= '/var/db/dhcpd.leases';
my $sql_type		= 'mysql';
my $sql_host		= 'localhost';
my $sql_user		= 'radius';
my $sql_pass		= 'radpass';
my $sql_database	= 'radius';
my $sql_table		= 'radippool';
my $pool_name		= '';
my $pool_key		= 'Calling-Station-Id';
my $insert_only 	= 0;

my $verbose;
my $help;

sub error {
	print STDERR @_, "\n";	
	exit(64);
}

sub notice {
	if ($verbose) {
		printf(shift . "\n", @_);
	}
}

sub help {
	my @this = split('/', $0);
	print <<HELP
$this[$#this] [options] <pool>

Options:
	-leases <lease file>    - The lease file to parse (defaults to '$lease_file')
	-pool-key <attribute>	- The attribute used to identify the user (defaults to '$pool_key')
	-no-update		- Don't update existing lease entries
	-type			- SQL database type (defaults to '$sql_type')
	-table			- SQL table (defaults to '$sql_table')
	-h | -host		- SQL host to connect to
	-u | -user		- SQL user
	-p | -pass		- SQL password
	-v 			- Verbose
	-help			- This help text
HELP
;
	exit(0);
}

GetOptions (
	'leases=s'	=> \$lease_file,
	'pool-key=s'	=> \$pool_key,
	'no-update'	=> \$insert_only,
	'type=s'	=> \$sql_type,
	'table=s'	=> \$sql_table,
	'h|host=s'	=> \$sql_host,
	'u|user=s'	=> \$sql_user,
	'p|pass=s'	=> \$sql_pass,
	'v'		=> \$verbose,
	'help'		=> \$help
) or error('Failed parsing options');

#
# Poolname must be provided, and we need at least some arguments...
#
help if !scalar @ARGV or ($pool_name = $ARGV[$#ARGV]) =~ /^-/;

-r $lease_file or
	error("Lease file ($lease_file) doesn\'t exist or isn't readable");

my $leases = Text::DHCPLeases->new(file => $lease_file) or
	error("Failed parsing leases file (or lease file empty)");

my $handle = DBI->connect(
	"DBI:$sql_type:database=$sql_database;host=$sql_host",
	$sql_user, $sql_pass, {RaiseError => 1}
);

my $dt_isc = DateTime::Format::Strptime->new(pattern => '%Y/%m/%d %H:%M:%S');
my $dt_sql = DateTime::Format::DBI->new($handle);

for my $lease ($leases->get_objects) {
	next unless ($lease->binding_state && $lease->binding_state eq 'active');
	
	my($query, @result);
	eval {
		$query = $handle->prepare("
			SELECT expiry_time, framedipaddress FROM $sql_table
			WHERE pool_name = ?
			AND callingstationid = ?;"
			, undef);

		$query->bind_param(1, $pool_name);
		$query->bind_param(2, $lease->mac_address);
		
		$query->execute();
		
		@result = $query->fetchrow_array();
	};
	
	error($@) if $@;
	
	my $ends_isc = $dt_isc->parse_datetime($lease->ends =~ m{^(?:[0-9]+) (.+)});
	
	if (!$query->rows) {
		eval {
			$handle->do("
				INSERT INTO $sql_table (
					pool_name, framedipaddress,
					calledstationid, callingstationid
					expiry_time, pool_key)
				VALUES (?, ?, ?, ?, ?, ?);"
				, undef
				, $pool_name
				, $lease->ip_address
				, '00:00:00:00:00:00'
				, $lease->mac_address
				, $dt_sql->format_datetime($ends_isc)
				, $pool_key
			);
		};
		
		error($@) if $@;
		
		notice("MAC:'%s' inserted with IP:'%s'.", 
			$lease->mac_address, $lease->ip_address);
		
		next;
	}

	my $ends_sql = $dt_sql->parse_datetime($result[0]);
	
	if ($insert_only && (($result[1] ne $lease->ip_address) ||
		(DateTime->compare($ends_sql, $ends_isc) < 0))) {

		eval {
			$handle->do("
				UPDATE $sql_table
				SET 
					framedipaddress = ?, expiry_time = ?
				WHERE pool_name = ?
				AND callingstationid = ?;"
				, undef
				, $lease->ip_address
				, $dt_sql->format_datetime($ends_isc)
				, $pool_name
				, $lease->mac_address
			);
		};
		
		error($@) if $@;

		notice("MAC:'%s' updated. ISC-TS: '%s', SQL-TS: '%s', ISC-IP: '%s', SQL-IP: '%s'.",
			$lease->mac_address,
			$dt_sql->format_datetime($ends_isc),
			$dt_sql->format_datetime($ends_sql),
			$lease->ip_address,
			$result[1]);
		
		next;
	}
	
	notice("MAC:'%s' skipped (no update %s). ",
		$lease->mac_address, $insert_only ? 'allowed' : 'needed');
}

exit(0);
