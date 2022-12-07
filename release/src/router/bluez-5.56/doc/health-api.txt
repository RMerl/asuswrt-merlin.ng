BlueZ D-Bus Health API description
**********************************


HealthManager hierarchy
=======================

Service		org.bluez
Interface	org.bluez.HealthManager1
Object path	/org/bluez/

Methods		object CreateApplication(dict config)

			Returns the path of the new registered application.
			Application will be closed by the call or implicitly
			when the programs leaves the bus.

			config:
				uint16 DataType:

					Mandatory

				string Role:

					Mandatory. Possible values: "source",
									"sink"

				string Description:

					Optional

				ChannelType:

					Optional, just for sources. Possible
					values: "reliable", "streaming"

			Possible Errors: org.bluez.Error.InvalidArguments

		void DestroyApplication(object application)

			Closes the HDP application identified by the object
			path. Also application will be closed if the process
			that started it leaves the bus. Only the creator of the
			application will be able to destroy it.

			Possible errors: org.bluez.Error.InvalidArguments
					 org.bluez.Error.NotFound
					 org.bluez.Error.NotAllowed


HealthDevice hierarchy
======================

Service		org.bluez
Interface	org.bluez.HealthDevice1
Object path	[variable prefix]/{hci0,hci1,...}/dev_XX_XX_XX_XX_XX_XX

Methods		boolean Echo()

			Sends an echo petition to the remote service. Returns
			True if response matches with the buffer sent. If some
			error is detected False value is returned.

			Possible errors: org.bluez.Error.InvalidArguments
					 org.bluez.Error.OutOfRange

		object CreateChannel(object application, string configuration)

			Creates a new data channel.  The configuration should
			indicate the channel quality of service using one of
			this values "reliable", "streaming", "any".

			Returns the object path that identifies the data
			channel that is already connected.

			Possible errors: org.bluez.Error.InvalidArguments
					 org.bluez.Error.HealthError

		void DestroyChannel(object channel)

			Destroys the data channel object. Only the creator of
			the channel or the creator of the HealthApplication
			that received the data channel will be able to destroy
			it.

			Possible errors: org.bluez.Error.InvalidArguments
					 org.bluez.Error.NotFound
				         org.bluez.Error.NotAllowed

Signals		void ChannelConnected(object channel)

			This signal is launched when a new data channel is
			created or when a known data channel is reconnected.

		void ChannelDeleted(object channel)

			This signal is launched when a data channel is deleted.

			After this signal the data channel path will not be
			valid and its path can be reused for future data
			channels.

Properties	object MainChannel [readonly]

			The first reliable channel opened. It is needed by
			upper applications in order to send specific protocol
			data units. The first reliable can change after a
			reconnection.


HealthChannel hierarchy
=======================

Service		org.bluez
Interface	org.bluez.HealthChannel1
Object path	[variable prefix]/{hci0,hci1,...}/dev_XX_XX_XX_XX_XX_XX/chanZZZ

Only the process that created the data channel or the creator of the
HealthApplication that received it will be able to call these methods.

Methods		fd Acquire()

			Returns the file descriptor for this data channel. If
			the data channel is not connected it will also
			reconnect.

			Possible Errors: org.bluez.Error.NotConnected
					 org.bluez.Error.NotAllowed

		void Release()

			Releases the fd. Application should also need to
			close() it.

			Possible Errors: org.bluez.Error.NotAcquired
					 org.bluez.Error.NotAllowed

Properties	string Type [readonly]

			The quality of service of the data channel. ("reliable"
			or "streaming")

		object Device [readonly]

			Identifies the Remote Device that is connected with.
			Maps with a HealthDevice object.

		object Application [readonly]

			Identifies the HealthApplication to which this channel
			is related to (which indirectly defines its role and
			data type).
