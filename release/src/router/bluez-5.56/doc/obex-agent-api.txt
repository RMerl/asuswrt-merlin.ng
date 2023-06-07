OBEX D-Bus Agent API description
********************************


Agent Manager hierarchy
=======================

Service		org.bluez.obex
Interface	org.bluez.obex.AgentManager1
Object path	/org/bluez/obex

Methods		void RegisterAgent(object agent)

			Register an agent to request authorization of
			the user to accept/reject objects. Object push
			service needs to authorize each received object.

			Possible errors: org.bluez.obex.Error.AlreadyExists

		void UnregisterAgent(object agent)

			This unregisters the agent that has been previously
			registered. The object path parameter must match the
			same value that has been used on registration.

			Possible errors: org.bluez.obex.Error.DoesNotExist


Agent hierarchy
===============

Service		unique name
Interface	org.bluez.obex.Agent1
Object path	freely definable

Methods		void Release()

			This method gets called when the service daemon
			unregisters the agent. An agent can use it to do
			cleanup tasks. There is no need to unregister the
			agent, because when this method gets called it has
			already been unregistered.

		string AuthorizePush(object transfer)

			This method gets called when the service daemon
			needs to accept/reject a Bluetooth object push request.

			Returns the full path (including the filename) where
			the object shall be stored. The tranfer object will
			contain a Filename property that contains the default
			location and name that can be returned.

			Possible errors: org.bluez.obex.Error.Rejected
			                 org.bluez.obex.Error.Canceled

		void Cancel()

			This method gets called to indicate that the agent
			request failed before a reply was returned. It cancels
			the previous request.
