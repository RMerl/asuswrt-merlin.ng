OBEX D-Bus API description
**************************


Client hierarchy
================

Service		org.bluez.obex
Interface	org.bluez.obex.Client1
Object path	/org/bluez/obex

Methods		object CreateSession(string destination, dict args)

			Create a new OBEX session for the given remote address.

			The last parameter is a dictionary to hold optional or
			type-specific parameters. Typical parameters that can
			be set in this dictionary include the following:

				string "Target" : type of session to be created
				string "Source" : local address to be used
				byte "Channel"

			The currently supported targets are the following:

				"ftp"
				"map"
				"opp"
				"pbap"
				"sync"

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void RemoveSession(object session)

			Unregister session and abort pending transfers.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.NotAuthorized

Session hierarchy
=================

Service		org.bluez.obex
Interface	org.bluez.obex.Session1
Object path	/org/bluez/obex/server/session{0, 1, 2, ...} or
		/org/bluez/obex/client/session{0, 1, 2, ...}

Methods		string GetCapabilities()

			Get remote device capabilities.

			Possible errors: org.bluez.obex.Error.NotSupported
					 org.bluez.obex.Error.Failed

Properties	string Source [readonly]

			Bluetooth adapter address

		string Destination [readonly]

			Bluetooth device address

		byte Channel [readonly]

			Bluetooth channel

		string Target [readonly]

			Target UUID

		string Root [readonly]

			Root path


Transfer hierarchy
==================

Service		org.bluez.obex
Interface	org.bluez.obex.Transfer1
Object path	[Session object path]/transfer{0, 1, 2, ...}

Methods		void Cancel()

			Stops the current transference.

			Possible errors: org.bluez.obex.Error.NotAuthorized
					 org.bluez.obex.Error.InProgress
					 org.bluez.obex.Error.Failed

		void Suspend()

			Suspend transference.

			Possible errors: org.bluez.obex.Error.NotAuthorized
					 org.bluez.obex.Error.NotInProgress

			Note that it is not possible to suspend transfers
			which are queued which is why NotInProgress is listed
			as possible error.

		void Resume()

			Resume transference.

			Possible errors: org.bluez.obex.Error.NotAuthorized
					 org.bluez.obex.Error.NotInProgress

			Note that it is not possible to resume transfers
			which are queued which is why NotInProgress is listed
			as possible error.

Properties	string Status [readonly]

			Inform the current status of the transfer.

			Possible values: "queued", "active", "suspended",
					"complete" or "error"

		object Session [readonly]

			The object path of the session the transfer belongs
			to.

		string Name [readonly]

			Name of the transferred object. Either Name or Type
			or both will be present.

		string Type [readonly]

			Type of the transferred object. Either Name or Type
			or both will be present.

		uint64 Time [readonly, optional]

			Time of the transferred object if this is
			provided by the remote party.

		uint64 Size [readonly, optional]

			Size of the transferred object. If the size is
			unknown, then this property will not be present.

		uint64 Transferred [readonly, optional]

			Number of bytes transferred. For queued transfers, this
			value will not be present.

		string Filename [readonly, optional]

			Complete name of the file being received or sent.

			For incoming object push transaction, this will be
			the proposed default location and name. It can be
			overwritten by the AuthorizePush agent callback
			and will be then updated accordingly.


Object Push hierarchy
=====================

Service		org.bluez.obex
Interface	org.bluez.obex.ObjectPush1
Object path	[Session object path]

Methods		object, dict SendFile(string sourcefile)

			Send one local file to the remote device.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		object, dict PullBusinessCard(string targetfile)

			Request the business card from a remote device and
			store it in the local file.

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		object, dict ExchangeBusinessCards(string clientfile,
							string targetfile)

			Push the client's business card to the remote device
			and then retrieve the remote business card and store
			it in a local file.

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed


File Transfer hierarchy
=======================

Service		org.bluez.obex
Interface	org.bluez.obex.FileTransfer
Object path	[Session object path]

Methods		void ChangeFolder(string folder)

			Change the current folder of the remote device.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void CreateFolder(string folder)

			Create a new folder in the remote device.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		array{dict} ListFolder()

			Returns a dictionary containing information about
			the current folder content.

			The following keys are defined:

				string Name : Object name in UTF-8 format
				string Type : Either "folder" or "file"
				uint64 Size : Object size or number of items in
						folder
				string Permission : Group, owner and other
							permission
				uint64 Modified : Last change
				uint64 Accessed : Last access
				uint64 Created : Creation date

			Possible errors: org.bluez.obex.Error.Failed

		object, dict GetFile(string targetfile, string sourcefile)

			Copy the source file (from remote device) to the
			target file (on local filesystem).

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		object, dict PutFile(string sourcefile, string targetfile)

			Copy the source file (from local filesystem) to the
			target file (on remote device).

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void CopyFile(string sourcefile, string targetfile)

			Copy a file within the remote device from source file
			to target file.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void MoveFile(string sourcefile, string targetfile)

			Move a file within the remote device from source file
			to the target file.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void Delete(string file)

			Deletes the specified file/folder.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed


Phonebook Access hierarchy
==========================

Service		org.bluez.obex
Interface	org.bluez.obex.PhonebookAccess1
Object path	[Session object path]

Methods		void Select(string location, string phonebook)

			Select the phonebook object for other operations. Should
			be call before all the other operations.

			location : Where the phonebook is stored, possible
			inputs :
				"int" ( "internal" which is default )
				"sim" ( "sim1" )
				"sim2"
				...

			phonebook : Possible inputs :
				"pb" :	phonebook for the saved contacts
				"ich":	incoming call history
				"och":	outgoing call history
				"mch":	missing call history
				"cch":	combination of ich och mch
				"spd":	speed dials entry ( only for "internal" )
				"fav":	favorites entry ( only for "internal" )

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		object, dict PullAll(string targetfile, dict filters)

			Return the entire phonebook object from the PSE server
			in plain string with vcard format, and store it in
			a local file.

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible filters: Format, Order, Offset, MaxCount and
			Fields

			Possible errors: org.bluez.obex.Error.InvalidArguments
					org.bluez.obex.Forbidden

		array{string vcard, string name} List(dict filters)

			Return an array of vcard-listing data where every entry
			consists of a pair of strings containing the vcard
			handle and the contact name. For example:
				"1.vcf" : "John"

			Possible filters: Order, Offset and MaxCount

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Forbidden

		object, dict
		Pull(string vcard, string targetfile, dict filters)

			Given a vcard handle, retrieve the vcard in the current
			phonebook object and store it in a local file.

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possbile filters: Format and Fields

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Forbidden
					 org.bluez.obex.Error.Failed

		array{string vcard, string name}
		Search(string field, string value, dict filters)

			Search for entries matching the given condition and
			return an array of vcard-listing data where every entry
			consists of a pair of strings containing the vcard
			handle and the contact name.

			vcard : name paired string match the search condition.

			field : the field in the vcard to search with
				{ "name" (default) | "number" | "sound" }
			value : the string value to search for


			Possible filters: Order, Offset and MaxCount

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Forbidden
					 org.bluez.obex.Error.Failed

		uint16 GetSize()

			Return the number of entries in the selected phonebook
			object that are actually used (i.e. indexes that
			correspond to non-NULL entries).

			Possible errors: org.bluez.obex.Error.Forbidden
					 org.bluez.obex.Error.Failed

		void UpdateVersion()

			Attempt to update PrimaryCounter and SecondaryCounter.

			Possible errors: org.bluez.obex.Error.NotSupported
					 org.bluez.obex.Error.Forbidden
					 org.bluez.obex.Error.Failed

		array{string} ListFilterFields()

			Return All Available fields that can be used in Fields
			filter.

			Possible errors: None

Filter:		string Format:

			Items vcard format

			Possible values: "vcard21" (default) or "vcard30"

		string Order:

			Items order

			Possible values: "indexed" (default), "alphanumeric" or
			"phonetic"

		uint16 Offset:

			Offset of the first item, default is 0

		uint16 MaxCount:

			Maximum number of items, default is unlimited (65535)

		array{string} Fields:

			Item vcard fields, default is all values.

			Possible values can be query with ListFilterFields.

		array{string} FilterAll:

			Filter items by fields using AND logic, cannot be used
			together with FilterAny.

			Possible values can be query with ListFilterFields.

		array{string} FilterAny:

			Filter items by fields using OR logic, cannot be used
			together with FilterAll.

			Possible values can be query with ListFilterFields.

		bool ResetNewMissedCalls

			Reset new the missed calls items, shall only be used
			for folders mch and cch.

Properties	string Folder [readonly]

			Current folder.

		string DatabaseIdentifier [readonly, optional]

			128 bits persistent database identifier.

			Possible values: 32-character hexadecimal such
			as A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6

		string PrimaryCounter [readonly, optional]

			128 bits primary version counter.

			Possible values: 32-character hexadecimal such
			as A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6

		string SecondaryCounter [readonly, optional]

			128 bits secondary version counter.

			Possible values: 32-character hexadecimal such
			as A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6

		bool FixedImageSize [readonly, optional]

			Indicate support for fixed image size.

			Possible values: True if image is JPEG 300x300 pixels
			otherwise False.

Synchronization hierarchy
=========================

Service		org.bluez.obex
Interface	org.bluez.obex.Synchronization1
Object path	[Session object path]

Methods		void SetLocation(string location)

			Set the phonebook object store location for other
			operations. Should be called before all the other
			operations.

			location: Where the phonebook is stored, possible
			values:
				"int" ( "internal" which is default )
				"sim1"
				"sim2"
				......

			Possible errors: org.bluez.obex.Error.InvalidArguments

		object, dict GetPhonebook(string targetfile)

			Retrieve an entire Phonebook Object store from remote
			device, and stores it in a local file.

			If an empty target file is given, a name will be
			automatically calculated for the temporary file.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		object, dict PutPhonebook(string sourcefile)

			Send an entire Phonebook Object store to remote device.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed


Message Access hierarchy
=========================

Service		org.bluez.obex
Interface	org.bluez.obex.MessageAccess1
Object path	[Session object path]

Methods		void SetFolder(string name)

			Set working directory for current session, *name* may
			be the directory name or '..[/dir]'.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		array{dict} ListFolders(dict filter)

			Returns a dictionary containing information about
			the current folder content.

			The following keys are defined:

				string Name : Folder name

			Possible filters: Offset and MaxCount

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		array{string} ListFilterFields()

			Return all available fields that can be used in Fields
			filter.

			Possible errors: None

		array{object, dict} ListMessages(string folder, dict filter)

			Returns an array containing the messages found in the
			given subfolder of the current folder, or in the
			current folder if folder is empty.

			Possible Filters: Offset, MaxCount, SubjectLength, Fields,
			Type, PeriodStart, PeriodEnd, Status, Recipient, Sender,
			Priority

			Each message is represented by an object path followed
			by a dictionary of the properties.

			Properties:

				string Subject:

					Message subject

				string Timestamp:

					Message timestamp

				string Sender:

					Message sender name

				string SenderAddress:

					Message sender address

				string ReplyTo:

					Message Reply-To address

				string Recipient:

					Message recipient name

				string RecipientAddress:

					Message recipient address

				string Type:

					Message type

					Possible values: "email", "sms-gsm",
					"sms-cdma" and "mms"

				uint64 Size:

					Message size in bytes

				boolean Text:

					Message text flag

					Specifies whether message has textual
					content or is binary only

				string Status:

					Message status

					Possible values for received messages:
					"complete", "fractioned", "notification"

					Possible values for sent messages:
					"delivery-success", "sending-success",
					"delivery-failure", "sending-failure"

				uint64 AttachmentSize:

					Message overall attachment size in bytes

				boolean Priority:

					Message priority flag

				boolean Read:

					Message read flag

				boolean Sent:

					Message sent flag

				boolean Protected:

					Message protected flag

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

		void UpdateInbox(void)

			Request remote to update its inbox.

			Possible errors: org.bluez.obex.Error.Failed

		object, dict
		PushMessage(string sourcefile, string folder, dict args)

			Transfer a message (in bMessage format) to the
			remote device.

			The message is transferred either to the given
			subfolder of the current folder, or to the current
			folder if folder is empty.

			Possible args: Transparent, Retry, Charset

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetAll.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed


Filter:		uint16 Offset:

			Offset of the first item, default is 0

		uint16 MaxCount:

			Maximum number of items, default is 1024

		byte SubjectLength:

			Maximum length of the Subject property in the
			message, default is 256

		array{string} Fields:

			Message fields, default is all values.

			Possible values can be query with ListFilterFields.

		array{string} Types:

			Filter messages by type.

			Possible values: "sms", "email", "mms".

		string PeriodBegin:

			Filter messages by starting period.

			Possible values: Date in "YYYYMMDDTHHMMSS" format.

		string PeriodEnd:

			Filter messages by ending period.

			Possible values: Date in "YYYYMMDDTHHMMSS" format.

		boolean Read:

			Filter messages by read flag.

			Possible values: True for read or False for unread

		string Recipient:

			Filter messages by recipient address.

		string Sender:

			Filter messages by sender address.

		boolean Priority:

			Filter messages by priority flag.

			Possible values: True for high priority or False for
			non-high priority

Message hierarchy
=================

Service		org.bluez.obex
Interface	org.bluez.obex.Message1
Object path	[Session object path]/{message0,...}

Methods		object, dict Get(string targetfile, boolean attachment)

			Download message and store it in the target file.

			If an empty target file is given, a temporary file
			will be automatically generated.

			The returned path represents the newly created transfer,
			which should be used to find out if the content has been
			successfully transferred or if the operation fails.

			The properties of this transfer are also returned along
			with the object path, to avoid a call to GetProperties.

			Possible errors: org.bluez.obex.Error.InvalidArguments
					 org.bluez.obex.Error.Failed

Properties	string Folder [readonly]

			Folder which the message belongs to

		string Subject [readonly]

			Message subject

		string Timestamp [readonly]

			Message timestamp

		string Sender [readonly]

			Message sender name

		string SenderAddress [readonly]

			Message sender address

		string ReplyTo [readonly]

			Message Reply-To address

		string Recipient [readonly]

			Message recipient name

		string RecipientAddress [readonly]

			Message recipient address

		string Type [readonly]

			Message type

			Possible values: "email", "sms-gsm",
			"sms-cdma" and "mms"

		uint64 Size [readonly]

			Message size in bytes

		string Status [readonly]

			Message reception status

			Possible values: "complete",
			"fractioned" and "notification"

		boolean Priority [readonly]

			Message priority flag

		boolean Read [read/write]

			Message read flag

		boolean Deleted [writeonly]

			Message deleted flag

		boolean Sent [readonly]

			Message sent flag

		boolean Protected [readonly]

			Message protected flag
