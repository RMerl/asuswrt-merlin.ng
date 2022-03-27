BlueZ D-Bus Thermometer API description
***************************************

	Santiago Carot-Nemesio <sancane@gmail.com>

Health Thermometer Manager hierarchy
====================================

Service		org.bluez
Interface	org.bluez.ThermometerManager1
Object path	[variable prefix]/{hci0,hci1,...}

Methods		RegisterWatcher(object agent)

			Registers a watcher to monitor scanned measurements.
			This agent will be notified about final temperature
			measurements.

			Possible Errors: org.bluez.Error.InvalidArguments

		UnregisterWatcher(object agent)

			Unregisters a watcher.

		EnableIntermediateMeasurement(object agent)

			Enables intermediate measurement notifications
			for this agent. Intermediate measurements will
			be enabled only for thermometers which support it.

			Possible Errors: org.bluez.Error.InvalidArguments

		DisableIntermediateMeasurement(object agent)

			Disables intermediate measurement notifications
			for this agent. It will disable notifications in
			thermometers when the last agent removes the
			watcher for intermediate measurements.

			Possible Errors: org.bluez.Error.InvalidArguments
					org.bluez.Error.NotFound

Health Thermometer Profile hierarchy
====================================

Service		org.bluez
Interface	org.bluez.Thermometer1
Object path	[variable prefix]/{hci0,hci1,...}/dev_XX_XX_XX_XX_XX_XX


Properties	boolean Intermediate [readonly]

			True if the thermometer supports intermediate
			measurement notifications.

		uint16 Interval (optional) [readwrite]

			The Measurement Interval defines the time (in
			seconds) between measurements. This interval is
			not related to the intermediate measurements and
			must be defined into a valid range. Setting it
			to zero means that no periodic measurements will
			be taken.

		uint16 Maximum (optional) [readonly]

			Defines the maximum value allowed for the interval
			between periodic measurements.

		uint16 Minimum (optional) [readonly]

			Defines the minimum value allowed for the interval
			between periodic measurements.


Health Thermometer Watcher hierarchy
====================================

Service		unique name
Interface	org.bluez.ThermometerWatcher1
Object path	freely definable

Methods		void MeasurementReceived(dict measurement)

			This callback gets called when a measurement has been
			scanned in the thermometer.

			Measurement:

				int16 Exponent:
				int32 Mantissa:

					Exponent and Mantissa values as
					extracted from float value defined by
					IEEE-11073-20601.

					Measurement value is calculated as
					(Mantissa) * (10^Exponent)

					For special cases Exponent is
					set to 0 and Mantissa is set to
					one of following values:

					+(2^23 - 1)	NaN (invalid or
							missing data)
					-(2^23)		NRes
					+(2^23 - 2)	+Infinity
					-(2^23 - 2)	-Infinity

				string Unit:

					Possible values: "celsius" or
							"fahrenheit"

				uint64 Time (optional):

					Time of measurement, if
					supported by device.
					Expressed in seconds since epoch.

				string Type (optional):

					Only present if measurement type
					is known.

					Possible values: "armpit", "body",
						"ear", "finger", "intestines",
						"mouth", "rectum", "toe",
						"tympanum"

				string Measurement:

					Possible values: "final" or
							"intermediate"
