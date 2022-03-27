BlueZ D-Bus Network API description
***********************************


Network hierarchy
=================

Service		org.bluez
Interface	org.bluez.Network1
Object path	[variable prefix]/{hci0,hci1,...}/dev_XX_XX_XX_XX_XX_XX

Methods		string Connect(string uuid)

			Connect to the network device and return the network
			interface name. Examples of the interface name are
			bnep0, bnep1 etc.

			uuid can be either one of "gn", "panu" or "nap" (case
			insensitive) or a traditional string representation of
			UUID or a hexadecimal number.

			The connection will be closed and network device
			released either upon calling Disconnect() or when
			the client disappears from the message bus.

			Possible errors: org.bluez.Error.AlreadyConnected
					 org.bluez.Error.ConnectionAttemptFailed

		void Disconnect()

			Disconnect from the network device.

			To abort a connection attempt in case of errors or
			timeouts in the client it is fine to call this method.

			Possible errors: org.bluez.Error.Failed

Properties	boolean Connected [readonly]

			Indicates if the device is connected.

		string Interface [readonly]

			Indicates the network interface name when available.

		string UUID [readonly]

			Indicates the connection role when available.


Network server hierarchy
========================

Service		org.bluez
Interface	org.bluez.NetworkServer1
Object path	/org/bluez/{hci0,hci1,...}

Methods		void Register(string uuid, string bridge)

			Register server for the provided UUID. Every new
			connection to this server will be added the bridge
			interface.

			Valid UUIDs are "gn", "panu" or "nap".

			Initially no network server SDP is provided. Only
			after this method a SDP record will be available
			and the BNEP server will be ready for incoming
			connections.

		void Unregister(string uuid)

			Unregister the server for provided UUID.

			All servers will be automatically unregistered when
			the calling application terminates.
