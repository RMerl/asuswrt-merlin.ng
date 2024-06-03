ABOUT AND USAGE
---------------

This is a generic stateless NAT46 kernel module for Linux.

It supports multiple simultaneous instances of NAT46 on the same host,
allowing to implement sophisticated translation strategies just 
by using routing to direct the packets to the appropriate interface.

Upon loading, it creates a file /proc/net/nat46/control, which is used 
to interact with it.

echo add <ifname> | sudo tee /proc/net/nat46/control
	create a new nat46 interface with a specified name

echo del <ifname> | sudo tee /proc/net/nat46/control
	delete the existing nat46 interface with a specified name

echo config <ifname> <cfg-strings> | sudo tee /proc/net/nat46/control
	pass the <cfg-strings> data to configuration routine of
	the respective nat46 interface. In case multiple rules are
	present - this command controls the very last one in the ruleset.

echo insert <ifname> <cfg-strings> | sudo tee /proc/net/nat46/control
	insert a new rule with the specified config string at the head
	of the rule set for the device.

echo remove <ifname> <cfg-strings> | sudo tee /proc/net/nat46/control
	removes a rule with the specified config string from the
	rule set for the device.

CONFIGURING NAT46 DEVICE
-----------------------

Configuration parameters for the device take form of "name value" pairs,
with the following values:

debug <level>
	set the debug level on the device to <level>

local.<param> <value>
	set the local side translation rule's parameter <param> to <value>

remote.<param> <value>
	set the remote side tranlation rule's parameter <param> to <value>


The rules for local and remote addresses are using the same mechanism for translation
for greater flexibility and allow several arguments. The most important argument is 
"style", which determines what kind of the translation mechanism is employed for 
this rule:

<rule>.style NONE
	this is a very simplistic style: it always fails, unless you configure 
	a /32 IPv4 prefix and a /128 IPv6 prefix - at which point it starts to 
	function as a single 1:1 translation rule.

	<rule>.v4 <v4addr>/32
	<rule>.v6 <v6addr>/128
		both of these parameters must be set for this translation style
		to function properly. They define the two addresses for 
		the 1:1 mapping.

	<rule>.ea-len
	<rule>.psid-offset
	<rule>.fmr-flag
		ignored in this translation style

	NB: in the future this translation mechanism may be extended to allow 1:1
	subnet mapping.

<rule>.style RFC6052
	this is a rule which allows to implement the mapping used in NAT64
	environments to represent the entire IPv4 internet as an IPv6 prefix.

	<rule>.v6 <v6pref>/<v6prefixlen>
		this defines IPv6 prefix length to translate the IPv4 internet into.
		The allowed prefix lengths are 32, 40, 48, 56, 64, 96.
		If a disallowed length is used, the translation fails.

	<rule>.v4 <v4pref>/<v4prefixlen>
		this parameter is ignored for now in this translation style. 
		For backwards compatibility it should be 0.0.0.0/0

	<rule>.ea-len
	<rule>.psid-offset
	<rule>.fmr-flag
		ignored in this translation style

<rule>.style MAP
	this is a translation rule for the MAP (Mapping Address and Port) algorithm,
	which may include the layer 4 identifier (tcp/udp port or ICMP id).

	<rule>.v6 <v6pref>/<v6prefixlen>
		this parameter defines the MAP domain IPv6 prefix

	<rule>.v4 <v6pref>/<v6prefixlen>
		this parameter defines the MAP domain IPv4 prefix

	<rule>.ea-len
		this parameter defines the length of the embedded address bits
		within the IPv6 prefix that has been allocated.

	<rule>.psid-offset
		this parameter specifies how many bits to the right to shift
		the bits of the psid within the port value.

	<rule>.fmr-flag
		this parameter allows the "local" rule to be tried as a "remote" rule 
		as well. In MAP terminology, this allows to implement FMR rule by just
		setting this flag. This flag is used only on the "local" side, and is
		ignored for the "remote" side.


CODE STRUCTURE
--------------

There are several groups of files:

nat46-module.*
	These files deal with the overall Linux module handling: loading / unloading,
	creating and destroying the /proc control file, as well as parsing the commands
	to pass on to the netdev portion of the code.

nat46-netdev.*
	Network device management code. This module accepts the high-level commands and
	performs the device-level work: locating the devices in the chain, grouping
	the functions into the device structures, etc. This module adds the pointers 
	the processing functions which are defined in the core group.

nat46-core.*
	Core processing routines. These do not do any netdevice/module work, they use 
	primarily sk_buff and nat64_instance_t data structures in order to operate.
	They use the Linux kernel and glue functions.

nat46-glue.*
	These are the "adaptation" functions, over time it is expected there will
	be almost nothing. The reason for the "glue" code to exist is to share
	the core code with userland implementations.


ACKNOWLEDGEMENTS
----------------

This code has been inspired or uses some parts of the following:

* CERNET MAP implementation

  https://github.com/cernet/MAP

* Stateful NAT64 kernel module implementation by Julius Kriukas

  https://github.com/fln/nat64

* Stateless NAT46 kernel module implementation by Tomasz Mrugalski

  https://github.com/tomaszmrugalski/ip46nat


