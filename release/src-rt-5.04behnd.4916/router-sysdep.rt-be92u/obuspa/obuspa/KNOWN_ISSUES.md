# OB-USP-Agent Known Issues

* **(SECURITY)** OBUSPA should not make use of functions which OpenSSL is deprecating.
* **(ACCESS CONTROL)** A Get request for a specific parameter which the Controller does not have permission to read, should result in a Get response containing an error rather than an empty response.
* **(MQTT)** MQTT PublishRetainResponse and PublishRetainNotify parameters should be implemented
* **(MQTT)** OBUSPA should allow other MQTT messaging applications (e.g. IoT applications) on the device to share its MQTT connections.


