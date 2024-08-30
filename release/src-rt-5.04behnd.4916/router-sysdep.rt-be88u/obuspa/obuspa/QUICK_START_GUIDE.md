# Quick Start Guide
## Audience
This document supports two target audiences:
* Integrator  - Someone who is taking OB-USP-AGENT and integrating it into a device.
                Normally this also involves extending the data model to support the device.
* Contributor - Someone who is enhancing the core functionality of the open source OB-USP-AGENT implementation.


## Document conventions
When referring to source code functions, this document will often use `XXX` to represent a set of possible function names.
For example, `DEVICE_XXX_Init()` refers to a set of functions:
 * `DEVICE_STOMP_Init()`
 * `DEVICE_MTP_Init()`
 * `DEVICE_CONTROLLER_Init()`
 * etc


## Building OB-USP-AGENT
1. Install dependencies (Curl, OpenSSL, Sqlite, z-lib, autotools) using package manager:
```
$ sudo apt-get install libssl-dev libcurl4-openssl-dev libsqlite3-dev libz-dev autoconf automake libtool  pkg-config
```
2. Install optional dependencies (libmosquitto, libwebsockets) using package manager:
```
$ sudo apt-get install libmosquitto-dev libwebsockets-dev
```
If the libraries are not provided by the distro, add the `ppa:mosquitto-dev` repository, which includes
both libmosquitto and libwebsockets for older Ubuntu distros:
```
$ sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
$ sudo apt-get update
$ sudo apt-get install libmosquitto-dev libwebsockets-dev
```
> **_NOTE:_**   libwebsockets must be compiled without support for event loop libraries (e.g. LWS_WITH_LIBUV=false)
> and without websocket extensions (LWS_WITHOUT_EXTENSIONS=true). These are the defaults but many distros choose to override them.

3. Clone the repository. Use the following instructions for the GitHub repository:
```
$ git clone https://github.com/BroadbandForum/obuspa.git
```
### Install OB-USP-AGENT from source:

#### Using GNU autotools
```
$ cd obuspa
$ autoreconf --force --install
$ ./configure
$ make
$ sudo make install
```

#### Configure Options
* `--disable-coap` - Removes CoAP MTP
* `--disable-mqtt` - Removes MQTT MTP
* `--disable-websockets` - Removes WebSockets MTP
* `--disable-stomp` - Removes STOMP MTP

#### Using CMake tool
```
$ cd obuspa
$ cmake -B build_folder -S .
$ cmake --build build_folder
$ cmake --install build_folder
```

#### CMake options
To disable a specific message transport, these options must be given when calling cmake:
* `-DENABLE_STOMP=[ON|OFF]` - Add or remove STOMP MTP (enabled if omitted)
* `-DENABLE_MQTT=[ON|OFF]` - Add or remove MQTT MTP (enabled if omitted)
* `-DENABLE_COAP=[ON|OFF]` - Add or remove CoAP MTP (enabled if omitted)
* `-DENABLE_WEBSOCKETS=[ON|OFF]` - Add or remove WebSockets MTP (enabled if omitted)

## Running OB-USP-AGENT for the first time
Before OB-USP-AGENT starts, it needs a database containing the settings of the USP controller to contact.
This is known as the `factory reset database`.
This database may be created using either:
* `obuspa -c dbset` commands (see next section)
* code in vendor_factory_reset_example.c (if `INCLUDE_PROGRAMMATIC_FACTORY_RESET` is defined in vendor_defs.h)
* a text file located by the `-r` option

To start with, use the last option, as this is the simplest method.

If OB-USP-AGENT cannot find a database when it starts up, then it will create one using the parameter values specified
in the file located by the `-r` option.

To specify the data model parameters and values used to create the factory reset database, modify `factory_reset_example.txt`.
You will need to modify the STOMP connection parameters and the USP EndpointID of the controller to connect to.

When using this option, to prevent the code in vendor_factory_reset_example.c from overriding the values specified in the
file located by the `-r` option, ensure that `INCLUDE_PROGRAMMATIC_FACTORY_RESET` is undefined in `vendor_defs.h`.

To create the database and run OB-USP-AGENT connecting to a STOMP server from network interface eth0 with protocol and trace
logging enabled to stdout, use the following command:
```
$ obuspa -p -v 4 -r factory_reset_example.txt -i eth0
```

If OB-USP-AGENT successfully connected to your STOMP server you should see trace like the following on stdout:

```
   Attempting to connect to host=controller1 (port=61613, unencrypted) from interface=eth0
   Connected to 127.0.0.1 (host=controller1, port=61613) from interface=eth0
   Sending STOMP frame to (host=controller1, port=61613)
   STOMP
   accept-version:1.2
   host:/
   heart-beat:30000,300000
   endpoint-id:os\c\c002456-0800270B57FF
   login:my_username
   passcode:


   Received CONNECTED frame from (host=controller1, port=61613)
   CONNECTED
   session:session-1K6NehQnoR3hioRXZgFBBw
   heart-beat:300000,30000
   server:RabbitMQ/3.5.7
   version:1.2


   Sending SUBSCRIBE frame to (host=controller1, port=61613)
   SUBSCRIBE
   id:0
   destination:/queue/agent-q1
   ack:auto
```

If OB-USP-AGENT failed to connect, review the settings in your factory reset database and the STOMP server.
If you subsequently change the settings in `factory_reset_example.txt`, then you must delete the database,
in order that the database is re-created the next time you run OB-USP-AGENT.
To delete the database in the default location:
```
$ rm /usr/local/var/obuspa/usp.db
```

Alternatively you can use the `obuspa -c dbset` command (see next section) to alter parameters
in the database, and try again.

## Docker integration

### Running OB-USP-AGENT standalone

You can also run OB-USP-AGENT within docker. Simply build the docker container, and tag with `obuspa:latest`.

```
docker build -t obuspa:latest .
```

Then run, with mounts for the factory reset file, and specify the arguments you'd like. For example:

```
docker run -d -v $(pwd)/factory_reset_example.txt:/obuspa/factory_reset_example.txt obuspa:latest obuspa -r /obuspa/factory_reset_example.txt -p -v4
```

In its current state, this will not preserve the USP database over docker runs.

### Starting OB-USP-AGENT developer environment inside Docker

For advanced IDE tools, it is also possible to uses extra compiling and debugging tools along Docker.
First you must create the Docker images to build the development environment

```
docker build -f Dockerfile -t obuspa:build-env --target build-env .
docker build -f Dockerfile.devel-env -t obuspa:devel-env .
```

At this point, two Docker images are created:
- `obuspa:build-env` as the strict minimalist image to build OB-USP-AGENT without any extra tools, and
- `obuspa:devel-env` as the development environment image to (remotely) execute CMake and GDB, and also with a running SSH daemon

Then run the `obuspa:devel-env` image with the SSH port exposed to any wanted value (e.g. 2222)

```
docker run -p 2222:22 -t -d --name obuspa-devel obuspa:devel-env
```

The SSH daemon is reachable with the following credentials (As configured in `Dockerfile.devel-env` file)
- Username: `user`
- Password: `password`
- Host: `localhost:2222` (or the custom port number given along `docker run -p` option)

## CoAP Message Transfer Protocol
OB-USP-AGENT also supports CoAP MTP. As with STOMP MTP, this is enabled
by setting data model parameters in the relevant CoAP MTP data model objects.

**IMPORTANT**
When using CoAP over DTLS, OB-USP-AGENT must have a client certificate (e.g. using the `--authcert` option).


## OB-USP-AGENT Command Line Arguments
OB-USP-AGENT supports two modes, a daemon mode (seen above) and a command (or CLI) mode, which supports interactively
querying the data model and setting values in the database. The CLI mode is specified with the `-c` option.

* To see a list of arguments use:
```
$ obuspa --help
```

* To see a list of commands supported in CLI mode use:
```
$ obuspa -c help
```

* To see the currently implemented USP data model use:
```
$ obuspa -c show datamodel
```

* To see all data model parameters stored in the database:
```
$ obuspa -c show database
```

* To set the value of a data model parameter in the database use:
```
$ obuspa -c dbset "parameter" "value"
```
IMPORTANT: This command must only be run when there is no daemon instance of OB-USP-AGENT running,
as it directly alters the value in the database without notifying a running daemon of the change.

* To set the value of a data model parameter when the daemon is running use:
```
$ obuspa -c set "parameter" "value"
```

* To query the value of a parameter when the daemon is running use:
```
$ obuspa -c get "parameter"
```

The "parameter" may contain USP search expressions and partial paths.
For example, to query the value of all parameters in the DeviceInfo object when the daemon is running use:
```
$ obuspa -c get "Device.DeviceInfo."
```

The CLI mode also supports adding and deleting instances of data model objects and running USP commands.

## OB-USP-AGENT Source Tree
The /src directory contains the following subdirectories:
* core       - This implements the core functionality and data model of OB-USP-AGENT.
               Contributors will make code changes in this directory.

* vendor     - This contains code which is intended to be modified by an integrator.
               Integrators extend the data model and core functionality registering vendor hooks (callbacks).

* include    - This defines the publicly accessible APIs exported to integrators (USP and VENDOR APIs).
               Contributors may make changes to files in this directory. Integrators must not.

* libjson    - This contains an open source Javascript Object Notation implementation.
               Neither contributors or integrators are likely to need to modify this code.

* protobuf-c - This contains pre-generated code implementing the USP record and USP message protobuf schemas.
               Contributors will only need to re-generate this code if the USP protobuf schema changes.


## OB-USP-AGENT APIs
Two APIs are of interest to an integrator. They are declared in the `src/include` directory.

* VENDOR API - An integrator must implement this API by modifying the stub functions in the `src/vendor` directory

* USP API - An integrator makes calls to this API to register the data model and notify OB-USP-AGENT core of changes

For information on the purpose and arguments of an API function, consult the function header comments
where the function is defined (typically `src/core/usp_register.c` or `src/core/usp_api.c`).


## OB-USP-AGENT Build Defines
The file src/vendor/vendor_defs.h contains feature switch defines and various other compile time defines.
The following defines are most likely to need modifying:

* `DEFAULT_WAN_IFNAME` - Name of the network interface to be used for USP communications.

* `CONNECT_ONLY_OVER_WAN_INTERFACE` - If defined only the network interface specified in `DEFAULT_WAN_IFNAME` is
                                      used for USP communications. If not defined, the Linux routing tables select
                                      which network interface to use.
                                      IMPORTANT: Even if not defined, `DEFAULT_WAN_IFNAME` must be a valid network interface.

* `DEFAULT_DATABASE_FILE` - The file system location of the database file, if none is specified
                            by the `-f` option when invoking OB-USP-AGENT.

* `CLI_UNIX_DOMAIN_FILE` - The file system location of a unix domain stream file used for communication
                           between OB-USP-AGENT running in CLI and daemon modes.

* `VENDOR_OUI` - The value of Device.DeviceInfo.ManufacturerOUI. This may be overridden by a value in the database.

* `VENDOR_PRODUCT_CLASS` - The value of `Device.DeviceInfo.ProductClass`
* `VENDOR_MANUFACTURER` - The value of `Device.DeviceInfo.Manufacturer`
* `VENDOR_MODEL_NAME` - The value of `Device.DeviceInfo.ModelName`
* `SYSTEM_CERT_PATH` - Location of file or directory containing certificates. These are reported in 
                       `Device.Security.Certificate` and not used in OB-USP-AGENT's trust store (use `-t` option to
                       specify trust store certificates).
* `E2ESESSION_EXPERIMENTAL_USP_V_1_2` - If defined the End-to-End Session Context is supported and the following TR-369
   parameters are available under the `Device.LocalAgent.Controller.{i}.` instance:
  * `E2ESession.SessionMode` (by default: Allow)
  * `E2ESession.Status`
  * `E2ESession.MaxUSPRecordSize` (because `E2ESession.SegmentedPayloadChunkSize` is deprecated in USP Device:2.16 data model)


## Extending the Data Model
Use the `USP_REGISTER_XXX()` set of functions to register USP data model objects, parameters, commands and Events.
* Integrators should always call `USP_REGISTER_XXX()` from `VENDOR_Init()` in `src/vendor/vendor.c`
* Contributors should create a new `device_XXX.c` file in `src/core`, and call `USP_REGISTER_XXX()` from a
  `DEVICE_XXX_Init()` located in the new `device_XXX.c` file.
  The new `DEVICE_XXX_Init()` must be hooked into the existing core data model from `DATA_MODEL_Init()` (in `src/core/data_model.c`).

Example (for Integrators):

```C
int VENDOR_Init(void)
{
    return USP_REGISTER_VendorParam_ReadOnly("Device.DeviceInfo.ModelNumber", GetModelNumber, DM_STRING);
}

int GetModelNumber(dm_req_t *req, char *buf, int len)
{
    strncpy(buf, "MyModelNumber", len);
    return USP_ERR_OK;
}
```

This example registers the `Device.DeviceInfo.ModelNumber` parameter.
The `Get_ModelNumber()` vendor hook function is called whenever OB-USP-AGENT core needs to get the value of the parameter.

The error codes to return are defined in `src/include/usp_err_codes.h`.
If an error occurs, call `USP_ERR_SetMessage()` to set an error message that will be returned by the USP protocol.

Vendor hook set handlers must check for uniqueness if their parameter forms part of a unique key for an object.

For more complex examples of extending the data model, see the `DEVICE_XXX_Init()` functions in the `src/core/device_XXX.c` files

IMPORTANT:
At bootup, the instance numbers of data model objects must be signalled to OB-USP-AGENT core using `USP_DM_InformInstance()`.
* Integrators should call `USP_DM_InformInstance()` from `VENDOR_Start()` (in `src/vendor/vendor.c`).
* Contributors should call `USP_DM_InformInstance()` from a `DEVICE_XXX_Start()` function in their `device_XXX.c` file.

After bootup, changes to object instances should be signalled with the `USP_SIGNAL_ObjectAdded()` and
`USP_SIGNAL_ObjectDeleted()` functions.

For an example of implementing a USP asynchronous command, see `src/core/device_selftest_example.c`.

USP data model events are registered by `USP_REGISTER_Event()` and `USP_REGISTER_EventArguments()`.
They are signalled with `USP_SIGNAL_DataModelEvent()`.
Use the `USP_ARG_XXX()` functions to create the event's argument list.


## Overriding the Core Implementation
The core implementation of OB-USP-AGENT has defaults for many aspects of functionality.
Some aspects have been designed to be overridden by the integrator using callbacks.
To override the default implementation, register a core vendor hook callback by calling
`USP_REGISTER_CoreVendorHooks()` from `VENDOR_Init()` (in `src/vendor/vendor.c`).

Example (for Integrators):

```C
int VENDOR_Init(void)
{
	vendor_hook_cb_t core_callbacks;
	memset(&core_callbacks, 0, sizeof(core_callbacks));
	core_callbacks.get_mtp_password_cb = GetStompPassword;
	return USP_REGISTER_CoreVendorHooks(&core_callbacks);
}

int GetStompPassword(char *buf, int len)
{
	strncpy(buf, "MyPassword", len);
	return USP_ERR_OK;
}
```

The example registers a callback to get the STOMP MTP password.
Other vendor hook callbacks may be registered by setting the relevant callback in the
core_callbacks structure, before calling `USP_REGISTER_CoreVendorHooks()`.

The typedefs for each of the core vendor hook callbacks are declared in `src/include/usp_api.h`.

The following core vendor hooks are most likely to need overriding:
* `reboot_cb` - called by OB-USP-AGENT core to reboot the device after receiving a Device.Reboot() command
* `factory_reset_cb` - called by OB-USP-AGENT core to perform a factory reset after receiving a Device.FactoryReset() command
* `get_trust_store_cb` - called by OB-USP-AGENT core to get the list of SSL certificates to install in 
                         OB-USP-AGENT's trust store. These can alternatively be specified using the '-t' option when invoking OB-USP-AGENT.
* `get_agent_cert_cb` - called by OB-USP-AGENT core to get the SSL client certificate and private key 
                        associated with this device. This can alternatively be specified using the '-a' option when invoking OB-USP-AGENT.

The trust store certificates and agent certificate (with associated private key) may alternatively be
specified using the `--truststore` and `--authcert` arguments when invoking OB-USP-AGENT.

Certificates provided to the `get_trust_store_cb()` and `get_agent_cert_cb()` must be in DER (binary) form,
whilst certificates provided to the `--authcert` and `--truststore` invocation arguments must be in PEM format.

## Advanced APIs
### Extending the Data Model using grouped parameter and object API
With some integrations, the data model is implemented by other executables and OB-USP-AGENT must communicate with the
other executables to get or set parameters or add or delete object instances.

Use the `USP_REGISTER_GroupXXX()` set of functions to register the group of parameters and objects implemented by the
other executable. When getting or setting grouped parameters, OB-USP-AGENT passes a list of all affected parameters
in the group to a single group get or set callback function, improving communication efficiency with the other executable.

Example (for Integrators):

```C
int VENDOR_Init(void)
{
    int err = USP_ERR_OK;

    #define MY_GROUP 1
    err |= USP_REGISTER_GroupedObject(MY_GROUP, "Device.MyObject.{i}", true);

    err |= USP_REGISTER_GroupedVendorParam_ReadWrite(MY_GROUP, "Device.MyObject.{i}.MyParam", DM_BOOL);

    err |= USP_REGISTER_GroupVendorHooks(MY_GROUP, GetMyParams, SetMyParams, AddMyObject, DelMyObject);

    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}
```
In the example, the grouped vendor hook callbacks registered by `USP_REGISTER_GroupVendorHooks()` are called to get, set, 
add or delete data model parameters and objects associated with MY_GROUP. Providing example implementations of these 
functions would be too verbose for this guide. Instead, follow the implementation guidelines below.

```C
int GetMyParams(int group_id, kv_vector_t *params)
{
    // params->vector[].key contains parameters to get
    // Obtain the value of these parameters from the other executable then use USP_ARG_Replace()
    // or USP_ARG_ReplaceWithHint() to copy the obtained value back into params->vector[]
    // If some parameters could not be obtained, then just do not call USP_ARG_Replace()
    // Only return an error if none of the parameters could be obtained (Example: RPC call failure)
}

int SetMyParams(int group_id, kv_vector_t *params, unsigned *param_types, int *failure_index)
{
    // params->vector[].key contains parameters to set
    // params->vector[].value contains the associated values to set
    // param_types[] contains the associated type of each parameter (Example: DM_BOOL)
    // return an error if any of the parameters could not be set.
    // *failure_index may be used to return the index of the first parameter to fail (index in the params->vector[] and param_types[] arrays).
    // If this is not known, you should return an index of the value INVALID.
}

int AddMyObject(int group_id, char *path, int *instance)
{
    // return the instance number of the object created by the other executable in *instance
    // return USP_ERR_CREATION_FAILURE if the object could not be created
}

int DelMyObject(int group_id, char *path)
{
    // return USP_ERR_OBJECT_NOT_DELETABLE if the object could not be deleted
}
```

### Refreshing Object Instances
Changes to object instances are normally signalled using the `USP_SIGNAL_ObjectAdded()` and `USP_SIGNAL_ObjectDeleted()`
functions. However, for some data model objects, these events can only be determined by periodically polling the object's
instances. This is wasteful to perform continuously, as the object instances are only required when  forming a USP response.

Use `USP_REGISTER_Object_RefreshInstances()` to register an object instance query function which is called on demand.

Example (for Integrators):

```C
int VENDOR_Init(void)
{
    int err = USP_ERR_OK;

    err |= USP_REGISTER_Object("Device.IP.Interface.{i}", NULL, NULL, NULL, NULL, NULL, NULL);

    err |= USP_REGISTER_ObjectRefreshInstances("Device.IP.Interface.{i}", RefreshIPInterfaceInstances);

    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

int RefreshIPInterfaceInstances(int group_id, char *path, int *expiry_period)
{
    // Register the currently valid instances for this object and all child objects
    USP_DM_RefreshInstance("Device.IP.Interface.1");
    USP_DM_RefreshInstance("Device.IP.Interface.1.IPv4Address.1");

    // cache the object instance numbers for 30 seconds
    *expiry_period = 30;
    return USP_ERR_OK;
}


```
