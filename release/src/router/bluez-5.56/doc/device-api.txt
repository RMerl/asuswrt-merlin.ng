BlueZ D-Bus Device API description
**********************************


Device hierarchy
================

Service		org.bluez
Interface	org.bluez.Device1
Object path	[variable prefix]/{hci0,hci1,...}/dev_XX_XX_XX_XX_XX_XX

Methods		void Connect()

			This is a generic method to connect any profiles
			the remote device supports that can be connected
			to and have been flagged as auto-connectable on
			our side. If only subset of profiles is already
			connected it will try to connect currently disconnected
			ones.

			If at least one profile was connected successfully this
			method will indicate success.

			For dual-mode devices only one bearer is connected at
			time, the conditions are in the following order:

				1. Connect the disconnected bearer if already
				connected.

				2. Connect first the bonded bearer. If no
				bearers are bonded or both are skip and check
				latest seen bearer.

				3. Connect last seen bearer, in case the
				timestamps are the same BR/EDR takes
				precedence.

			Possible errors: org.bluez.Error.NotReady
					 org.bluez.Error.Failed
					 org.bluez.Error.InProgress
					 org.bluez.Error.AlreadyConnected

		void Disconnect()

			This method gracefully disconnects all connected
			profiles and then terminates low-level ACL connection.

			ACL connection will be terminated even if some profiles
			were not disconnected properly e.g. due to misbehaving
			device.

			This method can be also used to cancel a preceding
			Connect call before a reply to it has been received.

			For non-trusted devices connected over LE bearer calling
			this method will disable incoming connections until
			Connect method is called again.

			Possible errors: org.bluez.Error.NotConnected

		void ConnectProfile(string uuid)

			This method connects a specific profile of this
			device. The UUID provided is the remote service
			UUID for the profile.

			Possible errors: org.bluez.Error.Failed
					 org.bluez.Error.InProgress
					 org.bluez.Error.InvalidArguments
					 org.bluez.Error.NotAvailable
					 org.bluez.Error.NotReady

		void DisconnectProfile(string uuid)

			This method disconnects a specific profile of
			this device. The profile needs to be registered
			client profile.

			There is no connection tracking for a profile, so
			as long as the profile is registered this will always
			succeed.

			Possible errors: org.bluez.Error.Failed
					 org.bluez.Error.InProgress
					 org.bluez.Error.InvalidArguments
					 org.bluez.Error.NotSupported

		void Pair()

			This method will connect to the remote device,
			initiate pairing and then retrieve all SDP records
			(or GATT primary services).

			If the application has registered its own agent,
			then that specific agent will be used. Otherwise
			it will use the default agent.

			Only for applications like a pairing wizard it
			would make sense to have its own agent. In almost
			all other cases the default agent will handle
			this just fine.

			In case there is no application agent and also
			no default agent present, this method will fail.

			Possible errors: org.bluez.Error.InvalidArguments
					 org.bluez.Error.Failed
					 org.bluez.Error.AlreadyExists
					 org.bluez.Error.AuthenticationCanceled
					 org.bluez.Error.AuthenticationFailed
					 org.bluez.Error.AuthenticationRejected
					 org.bluez.Error.AuthenticationTimeout
					 org.bluez.Error.ConnectionAttemptFailed

		void CancelPairing()

			This method can be used to cancel a pairing
			operation initiated by the Pair method.

			Possible errors: org.bluez.Error.DoesNotExist
					 org.bluez.Error.Failed

Properties	string Address [readonly]

			The Bluetooth device address of the remote device.

		string AddressType [readonly]

			The Bluetooth device Address Type. For dual-mode and
			BR/EDR only devices this defaults to "public". Single
			mode LE devices may have either value. If remote device
			uses privacy than before pairing this represents address
			type used for connection and Identity Address after
			pairing.

			Possible values:
				"public" - Public address
				"random" - Random address

		string Name [readonly, optional]

			The Bluetooth remote name. This value can not be
			changed. Use the Alias property instead.

			This value is only present for completeness. It is
			better to always use the Alias property when
			displaying the devices name.

			If the Alias property is unset, it will reflect
			this value which makes it more convenient.

		string Icon [readonly, optional]

			Proposed icon name according to the freedesktop.org
			icon naming specification.

		uint32 Class [readonly, optional]

			The Bluetooth class of device of the remote device.

		uint16 Appearance [readonly, optional]

			External appearance of device, as found on GAP service.

		array{string} UUIDs [readonly, optional]

			List of 128-bit UUIDs that represents the available
			remote services.

		boolean Paired [readonly]

			Indicates if the remote device is paired.

		boolean Connected [readonly]

			Indicates if the remote device is currently connected.
			A PropertiesChanged signal indicate changes to this
			status.

		boolean Trusted [readwrite]

			Indicates if the remote is seen as trusted. This
			setting can be changed by the application.

		boolean Blocked [readwrite]

			If set to true any incoming connections from the
			device will be immediately rejected. Any device
			drivers will also be removed and no new ones will
			be probed as long as the device is blocked.

		boolean WakeAllowed [readwrite]

			If set to true this device will be allowed to wake the
			host from system suspend.

		string Alias [readwrite]

			The name alias for the remote device. The alias can
			be used to have a different friendly name for the
			remote device.

			In case no alias is set, it will return the remote
			device name. Setting an empty string as alias will
			convert it back to the remote device name.

			When resetting the alias with an empty string, the
			property will default back to the remote name.

		object Adapter [readonly]

			The object path of the adapter the device belongs to.

		boolean LegacyPairing [readonly]

			Set to true if the device only supports the pre-2.1
			pairing mechanism. This property is useful during
			device discovery to anticipate whether legacy or
			simple pairing will occur if pairing is initiated.

			Note that this property can exhibit false-positives
			in the case of Bluetooth 2.1 (or newer) devices that
			have disabled Extended Inquiry Response support.

		string Modalias [readonly, optional]

			Remote Device ID information in modalias format
			used by the kernel and udev.

		int16 RSSI [readonly, optional]

			Received Signal Strength Indicator of the remote
			device (inquiry or advertising).

		int16 TxPower [readonly, optional]

			Advertised transmitted power level (inquiry or
			advertising).

		dict ManufacturerData [readonly, optional]

			Manufacturer specific advertisement data. Keys are
			16 bits Manufacturer ID followed by its byte array
			value.

		dict ServiceData [readonly, optional]

			Service advertisement data. Keys are the UUIDs in
			string format followed by its byte array value.

		bool ServicesResolved [readonly]

			Indicate whether or not service discovery has been
			resolved.

		array{byte} AdvertisingFlags [readonly, experimental]

			The Advertising Data Flags of the remote device.

		dict AdvertisingData [readonly, experimental]

			The Advertising Data of the remote device. Keys are
			are 8 bits AD Type followed by data as byte array.

			Note: Only types considered safe to be handled by
			application are exposed.

			Possible values:
				<type> <byte array>
				...

			Example:
				<Transport Discovery> <Organization Flags...>
				0x26                   0x01         0x01...
