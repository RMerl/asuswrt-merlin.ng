
#ifndef AWSIOT_CONFIG_H_
#define AWSIOT_CONFIG_H_

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging related header files are required to be included in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define LIBRARY_LOG_NAME and  LIBRARY_LOG_LEVEL.
 * 3. Include the header file "logging_stack.h".
 */

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Logging configuration for the Demo. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "AWSIOT"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/


/**
 * @brief Details of the MQTT broker to connect to.
 *
 * @note Your AWS IoT Core endpoint can be found in the AWS IoT console under
 * Settings/Custom Endpoint, or using the describe-endpoint API.
 */

#define AWS_IOT_ENDPOINT               "a2b5xydzr1fxog-ats.iot.us-west-2.amazonaws.com"

/**
 * @brief AWS IoT MQTT broker port number.
 *
 * In general, port 8883 is for secured MQTT connections.
 *
 * @note Port 443 requires use of the ALPN TLS extension with the ALPN protocol
 * name. When using port 8883, ALPN is not required.
 */

#define AWS_MQTT_PORT    ( 8883 )

/**
 * @brief Path of the file containing the server's root CA certificate.
 *
 * This certificate is used to identify the AWS IoT server and is publicly
 * available. Refer to the AWS documentation available in the link below
 * https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs
 *
 * Amazon's root CA certificate is automatically downloaded to the certificates
 * directory from @ref https://www.amazontrust.com/repository/AmazonRootCA1.pem
 * using the CMake build system.
 *
 * @note This certificate should be PEM-encoded.
 * @note This path is relative from the demo binary created. Update
 * ROOT_CA_CERT_PATH to the absolute path if this demo is executed from elsewhere.
 */
#ifndef ROOT_CA_CERT_PATH
    #define ROOT_CA_CERT_PATH    "amazon-root-ca.pem"
#endif

/**
 * @brief Path of the file containing the client certificate.
 *
 * Refer to the AWS documentation below for details regarding client
 * authentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 *
 * @note This certificate should be PEM-encoded.
 *
 */
#define CLIENT_CERT_PATH    "device.pem.crt"



/**
 * @brief Path of the file containing the client's private key.
 *
 * Refer to the AWS documentation below for details regarding client
 * authentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 *
 * @note This private key should be PEM-encoded.
 *
 */

#define CLIENT_PRIVATE_KEY_PATH    "private.pem.key"

/**
 * @brief The username value for authenticating client to MQTT broker when
 * username/password based client authentication is used.
 *
 * Refer to the AWS IoT documentation below for details regarding client
 * authentication with a username and password.
 * https://docs.aws.amazon.com/iot/latest/developerguide/custom-authentication.html
 * As mentioned in the link above, an authorizer setup needs to be done to use
 * username/password based client authentication.
 *
 * @note AWS IoT message broker requires either a set of client certificate/private key
 * or username/password to authenticate the client. If this config is defined,
 * the username and password will be used instead of the client certificate and
 * private key for client authentication.
 *
 * #define CLIENT_USERNAME    "...insert here..."
 */

/**
 * @brief The password value for authenticating client to MQTT broker when
 * username/password based client authentication is used.
 *
 * Refer to the AWS IoT documentation below for details regarding client
 * authentication with a username and password.
 * https://docs.aws.amazon.com/iot/latest/developerguide/custom-authentication.html
 * As mentioned in the link above, an authorizer setup needs to be done to use
 * username/password based client authentication.
 *
 * @note AWS IoT message broker requires either a set of client certificate/private key
 * or username/password to authenticate the client.
 *
 * #define CLIENT_PASSWORD    "...insert here..."
 */

/**
 * @brief MQTT client identifier.
 *
 * No two clients may use the same client identifier simultaneously.
 */
#ifndef CLIENT_IDENTIFIER
    #define CLIENT_IDENTIFIER    "WayneIotThing"
#endif

/**
 * @brief Size of the network buffer for MQTT packets.
 */
#define NETWORK_BUFFER_SIZE       ( 131072U )

/**
 * @brief The name of the operating system that the application is running on.
 * The current value is given as an example. Please update for your specific
 * operating system.
 */
#define OS_NAME                   "Ubuntu"

/**
 * @brief The version of the operating system that the application is running
 * on. The current value is given as an example. Please update for your specific
 * operating system version.
 */
#define OS_VERSION                "18.04 LTS"

/**
 * @brief The name of the hardware platform the application is running on. The
 * current value is given as an example. Please update for your specific
 * hardware platform.
 */
#define HARDWARE_PLATFORM_NAME    "PC"

/**
 * @brief The name of the MQTT library used and its version, following an "@"
 * symbol.
 */
#include "core_mqtt.h"
#define MQTT_LIB    "core-mqtt@" MQTT_LIBRARY_VERSION

#endif /* ifndef AWSIOT_CONFIG_H_ */
