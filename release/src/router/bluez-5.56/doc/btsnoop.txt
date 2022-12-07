BTSnoop/Monitor protocol formats
********************************

Opcode definitions
==================

New Index
---------

	Code:        0x0000
	Parameters:  Type (1 Octet
		     Bus (1 Octet)
		     BD_Addr (6 Octets)
		     Name (8 Octets)

	This opcode indicates that a new controller instance with a
	given index was added. With some protocols, like the TTY-based
	one there is only a single supported controller, meaning the
	index is implicitly 0.

Deleted Index
-------------

	Code:        0x0001

	This opcode indicates that the controller with a specific index
	was removed.

Command Packet
--------------

	Code:        0x0002

	HCI command packet.

Event Packet
------------

	Code:        0x0003

	HCI event packet.

ACL TX Packet
-------------

	Code:        0x0004

	Outgoing ACL packet.

ACL RX Packet
-------------

	Code:        0x0005

	Incoming ACL packet.

SCO TX Packet
--------------

	Code:        0x0006

	Outgoing SCO packet.

SCO RX Packet
-------------

	Code:        0x0007

	Incomnig SCO packet.

Open Index
----------

	Code:        0x0008

	The HCI transport for the specified controller has been opened.

Close Index
-----------

	Code:        0x0009

	The HCI transport for the specified controller has been closed.

Index Information
-----------------

	Code:        0x000a
	Parameters:  BD_Addr (6 Octets)
		     Manufacturer (2 Octets)

	Information about a specific controller.

Vendor Diagnostics
------------------

	Code:        0x000b

	Vendor diagnostic information.

System Note
-----------

	Code:        0x000c

	System note.

User Logging
------------

	Code:        0x000d
	Parameters:  Priority (1 Octet)
		     Ident_Length (1 Octet)
		     Ident (Ident_Length Octets)

	User logging information.


TTY-based protocol
==================

This section covers the protocol that can be parsed by btmon when
passing it the --tty parameter. The protocol is little endian, packet
based, and has the following header for each packet:

struct tty_hdr {
	uint16_t data_len;
	uint16_t opcode;
	uint8_t  flags;
	uint8_t  hdr_len;
	uint8_t  ext_hdr[0];
} __attribute__ ((packed));

The actual payload starts at ext_hdr + hdr_len and has the length of
data_len - 4 - hdr_len. Each field of the header is defined as follows:

data_len:
	This is the total length of the entire packet, excuding the
	data_len field itself.

opcode:
	The BTSnoop opcode

flags:
	Special flags for the packet. Currently no flags are defined.

hdr_len:
	Length of the extended header.

ext_hdr:
	This is a sequence of header extension fields formatted as:

	struct {
		uint8_t type;
		uint8_t value[length];
	}

	The length of the value is dependent on the type. Currently the
	following types are defined:

	Type                 Length    Meaning
	----------------------------------------------------------------
	1  Command drops     1 byte    Dropped HCI command packets
	2  Event drops       1 byte    Dropped HCI event packets
	3  ACL TX drops      1 byte    Dropped ACL TX packets
	4  ACL RX drops      1 byte    Dropped ACL RX packets
	5  SCO TX drops      1 byte    Dropped SCO TX packets
	6  SCO RX drops      1 byte    Dropped SCO RX packets
	7  Other drops       1 byte    Dropped other packets
	8  32-bit timestamp  4 bytes   Timestamp in 1/10th ms

	The drops fields indicate the number of packets that the
	implementation had to drop (e.g. due to lack of buffers) since
	the last reported drop count.

	The fields of the extended header must be sorted by increasing
	type. This is essential so that unknown types can be ignored and
	the parser can jump to processing the payload.
