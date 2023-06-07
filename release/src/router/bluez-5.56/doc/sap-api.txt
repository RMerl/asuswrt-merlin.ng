BlueZ D-Bus Sim Access API description
**************************************


Sim Access Profile hierarchy
============================

Service		org.bluez
Interface	org.bluez.SimAccess1
Object path	[variable prefix]/{hci0,hci1,...}

Methods		void Disconnect()

			Disconnects SAP client from the server.

			Possible errors: org.bluez.Error.Failed

Properties	boolean Connected [readonly]

			Indicates if SAP client is connected to the server.
