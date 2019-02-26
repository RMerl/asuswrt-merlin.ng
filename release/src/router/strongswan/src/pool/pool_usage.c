/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2009-2010 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>

/**
 * print pool usage info
 */
void usage(void)
{
	printf("\
Usage:\n\
  ipsec pool --status|--add|--replace|--del|--resize|--leases|--purge [options]\n\
  ipsec pool --showattr|--statusattr|--addattr|--delattr [options]\n\
  \n\
  ipsec pool --status\n\
    Show a list of installed pools with statistics plus nameserver info.\n\
  \n\
  ipsec pool --statusattr [--hexout]\n\
    Show a list of all attributes stored in the database with the values\n\
    converted to the correct format if the type is known by --showattr or\n\
    in hex format otherwise.\n\
      hexout:  Output all values in hex format\n\
  \n\
  ipsec pool --showattr\n\
    Show a keyword list of the major attribute types.\n\
  \n\
  ipsec pool --add <name> --start <start> --end <end> [--timeout <timeout>]\n\
  ipsec pool --replace <name> --start <start> --end <end> [--timeout <timeout>]\n\
    Add a new pool to or replace an existing pool in the database.\n\
      name:    Name of the pool, as used in ipsec.conf rightsourceip=%%name\n\
      start:   Start address of the pool\n\
      end:     End address of the pool\n\
      timeout: Lease time in hours (use 'd', 'm', or 's' to alternatively\n\
               configure the time in days, minutes or seconds, respectively),\n\
               0 for static leases\n\
  \n\
  ipsec pool --add <name> --addresses <file> [--timeout <timeout>]\n\
  ipsec pool --replace <name> --addresses <file> [--timeout <timeout>]\n\
    Add a new pool to or replace an existing pool in the database.\n\
      name:    Name of the pool, as used in ipsec.conf rightsourceip=%%name\n\
      file:    File newline separated addresses for the pool are read from.\n\
               Optionally each address can be pre-assigned to a roadwarrior\n\
               identity, e.g. 10.231.14.2=alice@strongswan.org.\n\
               If a - (hyphen) is given instead of a file name, the addresses\n\
               are read from STDIN. Reading addresses stops at the end of file\n\
               or an empty line. Pools created with this command can not be\n\
               resized.\n\
      timeout: Lease time in hours (use 'd', 'm', or 's' to alternatively\n\
               configure the time in days, minutes or seconds, respectively),\n\
               0 for static leases\n\
  \n\
  ipsec pool --addattr <type> [--pool <name> [--identity <id>]]\n\
             --addr|--mask|--server|--subnet|--string|--hex <value>\n\
    Add a new attribute to the database. Attributes can be bundled by using\n\
    the --pool and --identity options. If a bundle matches a peer the contained\n\
    attributes are sent to that peer instead of the global ones.\n\
      type:    a keyword from --showattr or a number from the range 1..32767\n\
      name:    the name of the pool this attribute is added to\n\
      id:      identity of the peer this attribute is bound to\n\
      addr:    IPv4 or IPv6 address\n\
      mask:    IPv4 or IPv6 netmask (synonym for --addr)\n\
      server:  IPv4 or IPv6 address of a server (synonym for --addr)\n\
      subnet:  IPv4 subnet[s] given by network/mask[,network/mask,...]\n\
      string:  value of a string-type attribute\n\
      hex:     hex value of any attribute\n\
  \n\
  ipsec pool --del <name>\n\
    Delete a pool from the database.\n\
      name:    Name of the pool to delete\n\
  \n\
  ipsec pool --delattr <type> [--pool <name> [--identity <id>]]\n\
             [--addr|--mask|--server|--subnet|--string|--hex <value>]\n\
    Delete a specific or all attributes of a given type from the database.\n\
      type:    a keyword from --showattr or a number from the range 1..32767\n\
      name:    the name of the pool this attribute is added to\n\
      id:      identity of the peer this attribute is bound to\n\
      addr:    IPv4 or IPv6 address\n\
      mask:    IPv4 or IPv6 netmask (synonym for --addr)\n\
      server:  IPv4 or IPv6 address of a server (synonym for --addr)\n\
      subnet:  IPv4 subnet[s] given by network/mask[,network/mask,...]\n\
      string:  value of a string-type attribute\n\
      hex:     hex value of any attribute\n\
  \n\
  ipsec pool --resize <name> --end <end>\n\
    Grow or shrink an existing pool.\n\
      name:    Name of the pool to resize\n\
      end:     New end address for the pool\n\
  \n\
  ipsec pool --leases [--filter <filter>] [--utc]\n\
    Show lease information using filters:\n\
      filter:  Filter string containing comma separated key=value filters,\n\
               e.g. id=alice@strongswan.org,addr=1.1.1.1\n\
                  pool:   name of the pool\n\
                  id:     assigned identity of the lease\n\
                  addr:   lease IP address\n\
                  tstamp: UNIX timestamp when lease was valid, as integer\n\
                  status: status of the lease: online|valid|expired\n\
      utc:    Show times in UTC instead of local time\n\
  \n\
  ipsec pool --purge <name>\n\
    Delete lease history of a pool:\n\
      name:    Name of the pool to purge\n\
  \n\
  ipsec pool --batch <file>\n\
    Read commands from a file and execute them atomically.\n\
      file:    File to read the newline separated commands from. Commands\n\
               appear as they are written on the command line, e.g.\n\
                  --replace mypool --start 10.0.0.1 --end 10.0.0.254\n\
                  --del dns\n\
                  --add dns --server 10.1.0.1\n\
                  --add dns --server 10.1.1.1\n\
               If a - (hyphen) is given as a file name, the commands are read\n\
               from STDIN. Readin commands stops at the end of file. Empty\n\
               lines are ignored. The file may not contain a --batch command.\n\
  \n");
}
