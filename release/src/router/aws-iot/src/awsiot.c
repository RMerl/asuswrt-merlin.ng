#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <json.h>

#include <pthread.h>

#include <signal.h>

#ifdef __cplusplus  
extern "C" {  
#endif
#include <bcmnvram.h>
#ifdef __cplusplus  
}
#endif

#include <shared.h>
#include <aae_ipc.h>

#include <assert.h>
// #include <time.h>


#include <curl/curl.h>
#include <sys/stat.h>


#include "pwenc.h"

#include "awsiot_config.h"

#include "core_mqtt.h"
#include "core_mqtt_state.h"

/* OpenSSL sockets transport implementation. */
#include "openssl_posix.h"


/*Include backoff algorithm header for retry logic.*/
#include "backoff_algorithm.h"

/* Clock for timer. */
#include "clock.h"

// /* JSON API header. */
// #include "core_json.h"

#include "log.h"
#include "awsiot.h"
#include "api.h"

#include "awsiot_ipc.h"
#include "awsiot_mnt.h"

#include "webapi.h"

#define APP_DBG 1

static int rcv_tencentgame_data(char *data, char* topic);
static int rcv_newsite_provisioning_data(char *data, char* topic);
static int rcv_remoteconnection_data(char *data, char* topic);
static int rcv_httpd_data(char *data, char* topic);

#ifdef RTCONFIG_MULTISITEAPP
static int rcv_general_shadow_delta_data(char *data, char* topic);
static int rcv_general_shadow_get_accepted_data(char *data, char* topic);
static int rcv_general_data(char *data, char* topic);
#endif

struct subTopicHandler {
    int sub_topic_id;
    int enable;
    int recv_time;
    int max_recv_time;
    char topic[MAX_TOPIC_LENGTH];
    int (*func)(char *data, char* topic);
};

struct subTopicHandler CHK_SUB_TOPICS[] = {
    { SUB_TOPIC_TENCENTGAME, 0, 0, 600, "asus/all/tencentgame", rcv_tencentgame_data },
    { SUB_TOPIC_ALL_TENCENTGAME, 0, 0, 600, "asus/[awsiotclientid]/tencentgame", rcv_tencentgame_data },
    { SUB_TOPIC_NEWSITE_PROVISIONING, 0, 0, 5, "[awsiotclientid]", rcv_newsite_provisioning_data },
    { SUB_TOPIC_REMOTECONNECTION, 0, 0, 5, "asus/[awsiotclientid]/RemoteConnection", rcv_remoteconnection_data },
    { SUB_TOPIC_HTTPD, 0, 0, 5, "asus/[awsiotclientid]/httpd", rcv_httpd_data },

#ifdef RTCONFIG_MULTISITEAPP
    { SUB_TOPIC_GENERAL_DELTA_SHADOW, 0, 0, 5, "$aws/things/[awsiotclientid]/shadow/name/general/update/delta", rcv_general_shadow_delta_data },
    { SUB_TOPIC_GENERAL_GET_ACCEPTED, 0, 0, 5, "$aws/things/[awsiotclientid]/shadow/name/general/get/accepted", rcv_general_shadow_get_accepted_data },
    { SUB_TOPIC_GENERAL, 0, 0, 5, "asus/[awsiotclientid]/general", rcv_general_data },
#endif

    { -1, NULL }
};

struct supportedFeatures {
    int id;
    char name[FEATURE_NAME_LENGTH];
    int name_len;
};

struct supportedFeatures CHK_SUPPORTED_FEATURES[] = {
#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD)
    { FEATURE_WIREGUARD_SERVER, "wireguard_server", 16},
    { FEATURE_WIREGUARD_CLIENT, "wireguard_client", 16},
#endif
    { FEATURE_CHANGE_IP, "change_ip", 9},
    { FEATURE_TUNNEL_TEST, "tunnel_test", 11},
    { -1, NULL, 0 }
};

// static char subscribe_topic_array[SUBSCRIBE_TOPIC_MAX_NUMBER][MAX_TOPIC_LENGTH];
// static int subscribe_topic_number = 0;
// static time_t session_receive_time_tmp = 0;
// int get_mqtt_session_number = 0;
// char shadowRxBufTmp[NETWORK_BUFFER_SIZE] = {0};
// char subscribe_topic_tmp[MAX_TOPIC_LENGTH] = {0};

static char default_shadow_topic[MAX_TOPIC_LENGTH] = {0};
static char publish_topic[MAX_TOPIC_LENGTH] = {0};
static char shadow_remote_connection_topic[MAX_TOPIC_LENGTH] = {0};
static unsigned char tencent_sn[65] = {0};
static int tencentgame_download_enable_tmp = 0;
static json_object *g_json_object_general_db_reported = NULL;

MQTTContext_t mqttContext_g;

int check_mqtt_timer = 0;

// int get_mqtt_dif_session_number = 0;

unsigned int search_tc_file_count = 0;

bool publish_running = false;
int publish_waiting = 0;

int is_init_topic_data_ok = 0;

/**
 * Provide default values for undefined configuration settings.
 */

#ifndef OS_NAME
    #define OS_NAME    "Ubuntu"
#endif

#ifndef OS_VERSION
    #define OS_VERSION    "18.04 LTS"
#endif

#ifndef HARDWARE_PLATFORM_NAME
    #define HARDWARE_PLATFORM_NAME    "Posix"
#endif

/**
 * @brief Length of MQTT server host name.
 */
#define AWS_IOT_ENDPOINT_LENGTH         ( ( uint16_t ) ( sizeof( AWS_IOT_ENDPOINT ) - 1 ) )

/**
 * @brief Length of client identifier.
 */
#define CLIENT_IDENTIFIER_LENGTH        ( ( uint16_t ) ( sizeof( CLIENT_IDENTIFIER ) - 1 ) )

/**
 * @brief ALPN (Application-Layer Protocol Negotiation) protocol name for AWS IoT MQTT.
 *
 * This will be used if the AWS_MQTT_PORT is configured as 443 for AWS IoT MQTT broker.
 * Please see more details about the ALPN protocol for AWS IoT MQTT endpoint
 * in the link below.
 * https://aws.amazon.com/blogs/iot/mqtt-with-tls-client-authentication-on-port-443-why-it-is-useful-and-how-it-works/
 */
#define AWS_IOT_MQTT_ALPN               "\x0ex-amzn-mqtt-ca"

/**
 * @brief Length of ALPN protocol name.
 */
#define AWS_IOT_MQTT_ALPN_LENGTH        ( ( uint16_t ) ( sizeof( AWS_IOT_MQTT_ALPN ) - 1 ) )

/**
 * @brief This is the ALPN (Application-Layer Protocol Negotiation) string
 * required by AWS IoT for password-based authentication using TCP port 443.
 */
#define AWS_IOT_PASSWORD_ALPN           "\x04mqtt"

/**
 * @brief Length of password ALPN.
 */
#define AWS_IOT_PASSWORD_ALPN_LENGTH    ( ( uint16_t ) ( sizeof( AWS_IOT_PASSWORD_ALPN ) - 1 ) )


/**
 * @brief The maximum number of retries for connecting to server.
 */
#define CONNECTION_RETRY_MAX_ATTEMPTS            ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying connection to server.
 */
#define CONNECTION_RETRY_MAX_BACKOFF_DELAY_MS    ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for connection retry attempts.
 */
#define CONNECTION_RETRY_BACKOFF_BASE_MS         ( 500U )

/**
 * @brief Timeout for receiving CONNACK packet in milli seconds.
 */
#define CONNACK_RECV_TIMEOUT_MS                  ( 1000U )


/**
 * @brief The topic to subscribe and publish to in the example.
 *
 * The topic name starts with the client identifier to ensure that each demo
 * interacts with a unique topic name.
 */
#define MQTT_EXAMPLE_TOPIC                  CLIENT_IDENTIFIER "/example/topic"

/**
 * @brief Length of client MQTT topic.
 */
#define MQTT_EXAMPLE_TOPIC_LENGTH           ( ( uint16_t ) ( sizeof( MQTT_EXAMPLE_TOPIC ) - 1 ) )

/**
 * @brief Maximum number of outgoing publishes maintained in the application
 * until an ack is received from the broker.
 */
#define MAX_OUTGOING_PUBLISHES              ( 15U )

/**
 * @brief Invalid packet identifier for the MQTT packets. Zero is always an
 * invalid packet identifier as per MQTT 3.1.1 spec.
 */
#define MQTT_PACKET_ID_INVALID              ( ( uint16_t ) 0U )

/**
 * @brief Timeout for MQTT_ProcessLoop function in milliseconds.
 */
#define MQTT_PROCESS_LOOP_TIMEOUT_MS        ( 200U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 *  between two Control Packets.
 *
 *  It is the responsibility of the Client to ensure that the interval between
 *  Control Packets being sent does not exceed the this Keep Alive value. In the
 *  absence of sending any other Control Packets, the Client MUST send a
 *  PINGREQ Packet.
 */
#define MQTT_KEEP_ALIVE_INTERVAL_SECONDS    ( 60U )

/**
 * @brief Delay between MQTT publishes in seconds.
 */
#define DELAY_BETWEEN_PUBLISHES_SECONDS     ( 1U )


#define WAITING_PUBLISHES_SECONDS           ( 5U )


/**
 * @brief Number of PUBLISH messages sent per iteration.
 */
#define MQTT_PUBLISH_COUNT_PER_LOOP         ( 5U )

/**
 * @brief Delay in seconds between two iterations of subscribeTopic().
 */
#define MQTT_SUBPUB_LOOP_DELAY_SECONDS      ( 30U )

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define TRANSPORT_SEND_RECV_TIMEOUT_MS      ( 500 )

/**
 * @brief The MQTT metrics string expected by AWS IoT.
 */
#define METRICS_STRING                      "?SDK=" OS_NAME "&Version=" OS_VERSION "&Platform=" HARDWARE_PLATFORM_NAME "&MQTTLib=" MQTT_LIB

/**
 * @brief The length of the MQTT metrics string expected by AWS IoT.
 */
#define METRICS_STRING_LENGTH               ( ( uint16_t ) ( sizeof( METRICS_STRING ) - 1 ) )


#ifdef CLIENT_USERNAME

/**
 * @brief Append the username with the metrics string if #CLIENT_USERNAME is defined.
 *
 * This is to support both metrics reporting and username/password based client
 * authentication by AWS IoT.
 */
    #define CLIENT_USERNAME_WITH_METRICS    CLIENT_USERNAME METRICS_STRING
#endif

/*-----------------------------------------------------------*/

/**
 * @brief Structure to keep the MQTT publish packets until an ack is received
 * for QoS1 publishes.
 */
typedef struct PublishPackets {
    /**
     * @brief Packet identifier of the publish packet.
     */
    uint16_t packetId;

    /**
     * @brief Publish info of the publish packet.
     */
    MQTTPublishInfo_t pubInfo;
} PublishPackets_t;

/*-----------------------------------------------------------*/

/**
 * @brief Packet Identifier generated when Subscribe request was sent to the broker;
 * it is used to match received Subscribe ACK to the transmitted subscribe.
 */
static uint16_t globalSubscribePacketIdentifier = 0U;

/**
 * @brief Packet Identifier generated when Unsubscribe request was sent to the broker;
 * it is used to match received Unsubscribe ACK to the transmitted unsubscribe
 * request.
 */
static uint16_t globalUnsubscribePacketIdentifier = 0U;

/**
 * @brief Array to keep the outgoing publish messages.
 * These stored outgoing publish messages are kept until a successful ack
 * is received.
 */
static PublishPackets_t outgoingPublishPackets[ MAX_OUTGOING_PUBLISHES ] = { 0 };

/**
 * @brief Array to keep subscription topics.
 * Used to re-subscribe to topics that failed initial subscription attempts.
 */
static MQTTSubscribeInfo_t pGlobalSubscriptionList[ SUBSCRIBE_TOPIC_MAX_NUMBER ];

static int pGlobalSubscriptionListCount;

/**
 * @brief The network buffer must remain valid for the lifetime of the MQTT context.
 */
static uint8_t buffer[ NETWORK_BUFFER_SIZE ];

/**
 * @brief Status of latest Subscribe ACK;
 * it is updated every time the callback function processes a Subscribe ACK
 * and accounts for subscription to a single topic.
 */
static MQTTSubAckStatus_t globalSubAckStatus = MQTTSubAckFailure;

/*-----------------------------------------------------------*/

/* Each compilation unit must define the NetworkContext struct. */
struct NetworkContext {
    OpensslParams_t * pParams;
};

/*-----------------------------------------------------------*/

/**
 * @brief The random number generator to use for exponential backoff with
 * jitter retry logic.
 *
 * @return The generated random number.
 */
static uint32_t generateRandomNumber();

/**
 * @brief Connect to MQTT broker with reconnection retries.
 *
 * If connection fails, retry is attempted after a timeout.
 * Timeout value will exponentially increase until maximum
 * timeout value is reached or the number of attempts are exhausted.
 *
 * @param[out] pNetworkContext The output parameter to return the created network context.
 *
 * @return EXIT_FAILURE on failure; EXIT_SUCCESS on successful connection.
 */
static int connectToServerWithBackoffRetries( NetworkContext_t * pNetworkContext );

/**
 * @brief A function that connects to MQTT broker,
 * subscribes a topic, publishes to the same
 * topic MQTT_PUBLISH_COUNT_PER_LOOP number of times, and verifies if it
 * receives the Publish message back.
 *
 * @param[in] pMqttContext MQTT context pointer.
 * @param[in,out] pClientSessionPresent Pointer to flag indicating if an
 * MQTT session is present in the client.
 *
 * @return EXIT_FAILURE on failure; EXIT_SUCCESS on success.
 */
static int subscribeTopic( MQTTContext_t * pMqttContext,
                           bool * pClientSessionPresent,
                           bool * pMqttSessionEstablished );

/**
 * @brief The function to handle the incoming publishes.
 *
 * @param[in] pPublishInfo Pointer to publish info of the incoming publish.
 * @param[in] packetIdentifier Packet identifier of the incoming publish.
 */
static void handleIncomingPublish( MQTTPublishInfo_t * pPublishInfo,
                                   uint16_t packetIdentifier );

/**
 * @brief The application callback function for getting the incoming publish
 * and incoming acks reported from MQTT library.
 *
 * @param[in] pMqttContext MQTT context pointer.
 * @param[in] pPacketInfo Packet Info pointer for the incoming packet.
 * @param[in] pDeserializedInfo Deserialized information from the incoming packet.
 */
static void eventCallback( MQTTContext_t * pMqttContext,
                           MQTTPacketInfo_t * pPacketInfo,
                           MQTTDeserializedInfo_t * pDeserializedInfo );

/**
 * @brief Initializes the MQTT library.
 *
 * @param[in] pMqttContext MQTT context pointer.
 * @param[in] pNetworkContext The network context pointer.
 *
 * @return EXIT_SUCCESS if the MQTT library is initialized;
 * EXIT_FAILURE otherwise.
 */
static int initializeMqtt( MQTTContext_t * pMqttContext,
                           NetworkContext_t * pNetworkContext );

/**
 * @brief Sends an MQTT CONNECT packet over the already connected TCP socket.
 *
 * @param[in] pMqttContext MQTT context pointer.
 * @param[in] createCleanSession Creates a new MQTT session if true.
 * If false, tries to establish the existing session if there was session
 * already present in broker.
 * @param[out] pSessionPresent Session was already present in the broker or not.
 * Session present response is obtained from the CONNACK from broker.
 *
 * @return EXIT_SUCCESS if an MQTT session is established;
 * EXIT_FAILURE otherwise.
 */
static int establishMqttSession( MQTTContext_t * pMqttContext,
                                 bool createCleanSession,
                                 bool * pSessionPresent );

/**
 * @brief Close an MQTT session by sending MQTT DISCONNECT.
 *
 * @param[in] pMqttContext MQTT context pointer.
 *
 * @return EXIT_SUCCESS if DISCONNECT was successfully sent;
 * EXIT_FAILURE otherwise.
 */
static int disconnectMqttSession( MQTTContext_t * pMqttContext );

/**
 * @brief Sends an MQTT SUBSCRIBE to subscribe to #MQTT_EXAMPLE_TOPIC
 * defined at the top of the file.
 *
 * @param[in] pMqttContext MQTT context pointer.
 *
 * @return EXIT_SUCCESS if SUBSCRIBE was successfully sent;
 * EXIT_FAILURE otherwise.
 */
static int subscribeToTopic( MQTTContext_t * pMqttContext );

/**
 * @brief Sends an MQTT UNSUBSCRIBE to unsubscribe from
 * #MQTT_EXAMPLE_TOPIC defined at the top of the file.
 *
 * @param[in] pMqttContext MQTT context pointer.
 *
 * @return EXIT_SUCCESS if UNSUBSCRIBE was successfully sent;
 * EXIT_FAILURE otherwise.
 */
// static int unsubscribeFromTopic( MQTTContext_t * pMqttContext );

/**
 * @brief Sends an MQTT PUBLISH to #MQTT_EXAMPLE_TOPIC defined at
 * the top of the file.
 *
 * @param[in] pMqttContext MQTT context pointer.
 *
 * @return EXIT_SUCCESS if PUBLISH was successfully sent;
 * EXIT_FAILURE otherwise.
 */
static int publishToTopic( MQTTContext_t * pMqttContext, char * pTopicName, char * pPayload );

/**
 * @brief Function to get the free index at which an outgoing publish
 * can be stored.
 *
 * @param[out] pIndex The output parameter to return the index at which an
 * outgoing publish message can be stored.
 *
 * @return EXIT_FAILURE if no more publishes can be stored;
 * EXIT_SUCCESS if an index to store the next outgoing publish is obtained.
 */
static int getNextFreeIndexForOutgoingPublishes( uint8_t * pIndex );

/**
 * @brief Function to clean up an outgoing publish at given index from the
 * #outgoingPublishPackets array.
 *
 * @param[in] index The index at which a publish message has to be cleaned up.
 */
static void cleanupOutgoingPublishAt( uint8_t index );

/**
 * @brief Function to clean up all the outgoing publishes maintained in the
 * array.
 */
static void cleanupOutgoingPublishes( void );

/**
 * @brief Function to clean up the publish packet with the given packet id.
 *
 * @param[in] packetId Packet identifier of the packet to be cleaned up from
 * the array.
 */
static void cleanupOutgoingPublishWithPacketID( uint16_t packetId );

/**
 * @brief Function to resend the publishes if a session is re-established with
 * the broker. This function handles the resending of the QoS1 publish packets,
 * which are maintained locally.
 *
 * @param[in] pMqttContext MQTT context pointer.
 */
static int handlePublishResend( MQTTContext_t * pMqttContext );

/**
 * @brief Function to update variable globalSubAckStatus with status
 * information from Subscribe ACK. Called by eventCallback after processing
 * incoming subscribe echo.
 *
 * @param[in] Server response to the subscription request.
 */
static void updateSubAckStatus( MQTTPacketInfo_t * pPacketInfo );

/**
 * @brief Function to handle resubscription of topics on Subscribe
 * ACK failure. Uses an exponential backoff strategy with jitter.
 *
 * @param[in] pMqttContext MQTT context pointer.
 */
// static int handleResubscribe( MQTTContext_t * pMqttContext );

/*-----------------------------------------------------------*/

static uint32_t generateRandomNumber() {
    return( rand() );
}
/*-----------------------------------------------------------*/

static int connectToServerWithBackoffRetries( NetworkContext_t * pNetworkContext ) {
    int returnStatus = EXIT_SUCCESS;
    BackoffAlgorithmStatus_t backoffAlgStatus = BackoffAlgorithmSuccess;
    OpensslStatus_t opensslStatus = OPENSSL_SUCCESS;
    BackoffAlgorithmContext_t reconnectParams;
    ServerInfo_t serverInfo;
    OpensslCredentials_t opensslCredentials;
    uint16_t nextRetryBackOff;

    /* Initialize information to connect to the MQTT broker. */
    // serverInfo.pHostName = AWS_IOT_ENDPOINT;
    // serverInfo.hostNameLength = AWS_IOT_ENDPOINT_LENGTH;

    serverInfo.pHostName = awsiot_endpoint;
    serverInfo.hostNameLength = strlen(awsiot_endpoint);
    serverInfo.port = AWS_MQTT_PORT;

    /* Initialize credentials for establishing TLS session. */
    memset( &opensslCredentials, 0, sizeof( OpensslCredentials_t ) );
    opensslCredentials.pRootCaPath = rootCA;
    // opensslCredentials.pRootCaPath = ROOT_CA_CERT_PATH;

    /* If #CLIENT_USERNAME is defined, username/password is used for authenticating
     * the client. */
    #ifndef CLIENT_USERNAME
        opensslCredentials.pClientCertPath = clientCRT;
        opensslCredentials.pPrivateKeyPath = clientPrivateKey;
        // opensslCredentials.pClientCertPath = CLIENT_CERT_PATH;
        // opensslCredentials.pPrivateKeyPath = CLIENT_PRIVATE_KEY_PATH;
    #endif

    /* AWS IoT requires devices to send the Server Name Indication (SNI)
     * extension to the Transport Layer Security (TLS) protocol and provide
     * the complete endpoint address in the host_name field. Details about
     * SNI for AWS IoT can be found in the link below.
     * https://docs.aws.amazon.com/iot/latest/developerguide/transport-security.html */

    // opensslCredentials.sniHostName = AWS_IOT_ENDPOINT;
    opensslCredentials.sniHostName = awsiot_endpoint;

    if( AWS_MQTT_PORT == 443 ) {
        /* Pass the ALPN protocol name depending on the port being used.
         * Please see more details about the ALPN protocol for the AWS IoT MQTT
         * endpoint in the link below.
         * https://aws.amazon.com/blogs/iot/mqtt-with-tls-client-authentication-on-port-443-why-it-is-useful-and-how-it-works/
         *
         * For username and password based authentication in AWS IoT,
         * #AWS_IOT_PASSWORD_ALPN is used. More details can be found in the
         * link below.
         * https://docs.aws.amazon.com/iot/latest/developerguide/custom-authentication.html
         */
        #ifdef CLIENT_USERNAME
            opensslCredentials.pAlpnProtos = AWS_IOT_PASSWORD_ALPN;
            opensslCredentials.alpnProtosLen = AWS_IOT_PASSWORD_ALPN_LENGTH;
        #else
            opensslCredentials.pAlpnProtos = AWS_IOT_MQTT_ALPN;
            opensslCredentials.alpnProtosLen = AWS_IOT_MQTT_ALPN_LENGTH;
        #endif
    }

    /* Initialize reconnect attempts and interval */
    BackoffAlgorithm_InitializeParams( &reconnectParams,
                                       CONNECTION_RETRY_BACKOFF_BASE_MS,
                                       CONNECTION_RETRY_MAX_BACKOFF_DELAY_MS,
                                       CONNECTION_RETRY_MAX_ATTEMPTS );

    /* Attempt to connect to MQTT broker. If connection fails, retry after
     * a timeout. Timeout value will exponentially increase until maximum
     * attempts are reached.
     */
    do {
        /* Establish a TLS session with the MQTT broker. This example connects
         * to the MQTT broker as specified in AWS_IOT_ENDPOINT and AWS_MQTT_PORT
         * at the demo config header. */

        // LogInfo( ( "Establishing a TLS session to %.*s:%d.",
        //            AWS_IOT_ENDPOINT_LENGTH,
        //            AWS_IOT_ENDPOINT,
        //            AWS_MQTT_PORT ) );


        LogInfo( ( "Establishing a TLS session to %s:%d.",
                   awsiot_endpoint,
                   AWS_MQTT_PORT ) );

        opensslStatus = Openssl_Connect( pNetworkContext,
                                         &serverInfo,
                                         &opensslCredentials,
                                         TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                         TRANSPORT_SEND_RECV_TIMEOUT_MS );

        if( opensslStatus != OPENSSL_SUCCESS ) {
            /* Generate a random number and get back-off value (in milliseconds) for the next connection retry. */
            backoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &reconnectParams, generateRandomNumber(), &nextRetryBackOff );

            if( backoffAlgStatus == BackoffAlgorithmRetriesExhausted ) {
                LogError( ( "Connection to the broker failed, all attempts exhausted." ) );
                returnStatus = EXIT_FAILURE;
            }
            else if( backoffAlgStatus == BackoffAlgorithmSuccess ) {
                LogWarn( ( "Connection to the broker failed. Retrying connection "
                           "after %hu ms backoff.",
                           ( unsigned short ) nextRetryBackOff ) );
                Clock_SleepMs( nextRetryBackOff );
            }
        }
    } while( ( opensslStatus != OPENSSL_SUCCESS ) && ( backoffAlgStatus == BackoffAlgorithmSuccess ) );

    return returnStatus;
}
/*-----------------------------------------------------------*/

static int getNextFreeIndexForOutgoingPublishes( uint8_t * pIndex ) {

    int returnStatus = EXIT_FAILURE;
    uint8_t index = 0;

    assert( outgoingPublishPackets != NULL );
    assert( pIndex != NULL );

    for( index = 0; index < MAX_OUTGOING_PUBLISHES; index++ ) {

        LogInfo( ( "getNextFreeIndexForOutgoingPublishes index[%d],  packetId %u.\n\n",
                       index, outgoingPublishPackets[ index ].packetId ) );

        /* A free index is marked by invalid packet id.
         * Check if the the index has a free slot. */
        if( outgoingPublishPackets[ index ].packetId == MQTT_PACKET_ID_INVALID ) {
            returnStatus = EXIT_SUCCESS;
            break;
        }
    }

    /* Copy the available index into the output param. */
    *pIndex = index;

    return returnStatus;
}
/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishAt( uint8_t index ) {
    assert( outgoingPublishPackets != NULL );
    assert( index < MAX_OUTGOING_PUBLISHES );

    /* Clear the outgoing publish packet. */
    ( void ) memset( &( outgoingPublishPackets[ index ] ),
                     0x00,
                     sizeof( outgoingPublishPackets[ index ] ) );
}

/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishes( void ) {
    assert( outgoingPublishPackets != NULL );

    /* Clean up all the outgoing publish packets. */
    ( void ) memset( outgoingPublishPackets, 0x00, sizeof( outgoingPublishPackets ) );
}

/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishWithPacketID( uint16_t packetId ) {
    uint8_t index = 0;

    assert( outgoingPublishPackets != NULL );
    assert( packetId != MQTT_PACKET_ID_INVALID );

    /* Clean up all the saved outgoing publishes. */
    for( ; index < MAX_OUTGOING_PUBLISHES; index++ ) {
        if( outgoingPublishPackets[ index ].packetId == packetId ) {
            cleanupOutgoingPublishAt( index );
            LogInfo( ( "Cleaned up outgoing publish packet with [index = %d], packet id %u.\n\n",
                       index, packetId ) );
            break;
        }
    }
}
/*-----------------------------------------------------------*/

static int handlePublishResend( MQTTContext_t * pMqttContext ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    uint8_t index = 0U;
    MQTTStateCursor_t cursor = MQTT_STATE_CURSOR_INITIALIZER;
    uint16_t packetIdToResend = MQTT_PACKET_ID_INVALID;
    bool foundPacketId = false;

    assert( pMqttContext != NULL );
    assert( outgoingPublishPackets != NULL );

    /* MQTT_PublishToResend() provides a packet ID of the next PUBLISH packet
     * that should be resent. In accordance with the MQTT v3.1.1 spec,
     * MQTT_PublishToResend() preserves the ordering of when the original
     * PUBLISH packets were sent. The outgoingPublishPackets array is searched
     * through for the associated packet ID. If the application requires
     * increased efficiency in the look up of the packet ID, then a hashmap of
     * packetId key and PublishPacket_t values may be used instead. */
    packetIdToResend = MQTT_PublishToResend( pMqttContext, &cursor );

    while( packetIdToResend != MQTT_PACKET_ID_INVALID ) {
        foundPacketId = false;

        for( index = 0U; index < MAX_OUTGOING_PUBLISHES; index++ ) {
            if( outgoingPublishPackets[ index ].packetId == packetIdToResend ) {
                foundPacketId = true;
                outgoingPublishPackets[ index ].pubInfo.dup = true;

                LogInfo( ( "Sending duplicate PUBLISH with packet id %u.",
                           outgoingPublishPackets[ index ].packetId ) );
                mqttStatus = MQTT_Publish( pMqttContext,
                                           &outgoingPublishPackets[ index ].pubInfo,
                                           outgoingPublishPackets[ index ].packetId );

                if( mqttStatus != MQTTSuccess ) {
                    LogError( ( "Sending duplicate PUBLISH for packet id %u "
                                " failed with status %s.",
                                outgoingPublishPackets[ index ].packetId,
                                MQTT_Status_strerror( mqttStatus ) ) );
                    returnStatus = EXIT_FAILURE;
                    break;
                }
                else {
                    LogInfo( ( "Sent duplicate PUBLISH successfully for packet id %u.\n\n",
                               outgoingPublishPackets[ index ].packetId ) );
                }
            }
        }

        if( foundPacketId == false ) {
            LogError( ( "Packet id %u requires resend, but was not found in "
                        "outgoingPublishPackets.",
                        packetIdToResend ) );
            returnStatus = EXIT_FAILURE;
            break;
        }
        else {
            /* Get the next packetID to be resent. */
            packetIdToResend = MQTT_PublishToResend( pMqttContext, &cursor );
        }
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

static void handleIncomingPublish( MQTTPublishInfo_t * pPublishInfo,
                                   uint16_t packetIdentifier ) {

    assert( pPublishInfo != NULL );
    assert( pPublishInfo->pPayload != NULL );

    // get_mqtt_session_number++;

    /* Process incoming Publish. */
    LogInfo( ( "Incoming QOS : %d.", pPublishInfo->qos ) );

    LogInfo( ( "Incoming Publish Topic Name: %.*s matches subscribed topic.\n"
               "Incoming Publish message Packet Id is %u.\n"
               "Incoming Publish Message len : %d\n\n",
               pPublishInfo->topicNameLength,
               pPublishInfo->pTopicName,
               packetIdentifier,
               ( int ) pPublishInfo->payloadLength ) );

    LogInfo( ( "Incoming Publish Message : %.*s\n\n",
               ( int ) pPublishInfo->payloadLength,
               ( const char * ) pPublishInfo->pPayload ) );

    char subscribe_topic[MAX_TOPIC_LENGTH];
    snprintf(subscribe_topic, MAX_TOPIC_LENGTH, "%.*s", pPublishInfo->topicNameLength, pPublishInfo->pTopicName);

    char shadowRxBuf[NETWORK_BUFFER_SIZE];
    snprintf(shadowRxBuf, NETWORK_BUFFER_SIZE, "%.*s", 
        ( int ) pPublishInfo->payloadLength, ( const char * ) pPublishInfo->pPayload);

    Cdbg(APP_DBG, "Get mqtt_message, session data len = %d, subscribe_topic = %.*s",  ( int ) pPublishInfo->payloadLength, pPublishInfo->topicNameLength, pPublishInfo->pTopicName);

    if(parse_json_format(shadowRxBuf) != 0) {
        LogError( ( "Incoming data: The json document is invalid!!" ) );
        Cdbg(APP_DBG, "Received JSON is not valid");
        return;
    }

    time_t session_receive_time = time(NULL);

    struct subTopicHandler *handler = NULL;

    for(handler = &CHK_SUB_TOPICS[0]; handler->sub_topic_id > 0; handler++){

        if (handler->enable==1 && 
            strlen(handler->topic)==strlen(subscribe_topic) && 
            strncmp(handler->topic, subscribe_topic, strlen(subscribe_topic))==0) {

            if ( session_receive_time - handler->recv_time <= handler->max_recv_time) {
                return;
            }

            if (handler->func(shadowRxBuf, subscribe_topic)==0) {

                handler->recv_time = session_receive_time;

                Cdbg(APP_DBG, "Success to handle message(%d)", handler->sub_topic_id);
            }
            else {
                Cdbg(APP_DBG, "Fail to handle message(%d)", handler->sub_topic_id);
            }

            return;
        }
    }

}

static int get_supported_feature_id(const char* feature_name) {
    struct supportedFeatures *handler = NULL;
    for (handler = &CHK_SUPPORTED_FEATURES[0]; handler->id > 0; handler++) {
        if (!strncmp(feature_name, handler->name, handler->name_len)) {
            return handler->id;
        }
    }

    return -1;
}
/*-----------------------------------------------------------*/

static void updateSubAckStatus( MQTTPacketInfo_t * pPacketInfo ) {
    uint8_t * pPayload = NULL;
    size_t pSize = 0;

    MQTTStatus_t mqttStatus = MQTT_GetSubAckStatusCodes( pPacketInfo, &pPayload, &pSize );

    /* MQTT_GetSubAckStatusCodes always returns success if called with packet info
     * from the event callback and non-NULL parameters. */
    assert( mqttStatus == MQTTSuccess );

    /* Suppress unused variable warning when asserts are disabled in build. */
    ( void ) mqttStatus;

    /* Demo only subscribes to one topic, so only one status code is returned. */
    globalSubAckStatus = pPayload[ 0 ];
}
/*-----------------------------------------------------------*/

#if 0
static int handleResubscribe( MQTTContext_t * pMqttContext )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    BackoffAlgorithmStatus_t backoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t retryParams;
    uint16_t nextRetryBackOff = 0U;

    assert( pMqttContext != NULL );

    /* Initialize retry attempts and interval. */
    BackoffAlgorithm_InitializeParams( &retryParams,
                                       CONNECTION_RETRY_BACKOFF_BASE_MS,
                                       CONNECTION_RETRY_MAX_BACKOFF_DELAY_MS,
                                       CONNECTION_RETRY_MAX_ATTEMPTS );

    do
    {
        /* Send SUBSCRIBE packet.
         * Note: reusing the value specified in globalSubscribePacketIdentifier is acceptable here
         * because this function is entered only after the receipt of a SUBACK, at which point
         * its associated packet id is free to use. */
        mqttStatus = MQTT_Subscribe( pMqttContext,
                                     pGlobalSubscriptionList,
                                     pGlobalSubscriptionListCount,
                                     globalSubscribePacketIdentifier );


        if( mqttStatus != MQTTSuccess )
        {
            LogError( ( "Failed to send SUBSCRIBE packet to broker with error = %s.",
                        MQTT_Status_strerror( mqttStatus ) ) );
            returnStatus = EXIT_FAILURE;
            break;
        }

        LogInfo( ( "SUBSCRIBE sent for topic %.*s to broker.\n\n",
                   MQTT_EXAMPLE_TOPIC_LENGTH,
                   MQTT_EXAMPLE_TOPIC ) );

        /* Process incoming packet. */
        mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

        if( mqttStatus != MQTTSuccess )
        {
            LogError( ( "MQTT_ProcessLoop returned with status = %s.",
                        MQTT_Status_strerror( mqttStatus ) ) );
            returnStatus = EXIT_FAILURE;
            break;
        }

        /* Check if recent subscription request has been rejected. globalSubAckStatus is updated
         * in eventCallback to reflect the status of the SUBACK sent by the broker. It represents
         * either the QoS level granted by the server upon subscription, or acknowledgement of
         * server rejection of the subscription request. */
        if( globalSubAckStatus == MQTTSubAckFailure )
        {
            /* Generate a random number and get back-off value (in milliseconds) for the next re-subscribe attempt. */
            backoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &retryParams, generateRandomNumber(), &nextRetryBackOff );

            if( backoffAlgStatus == BackoffAlgorithmRetriesExhausted )
            {
                LogError( ( "Subscription to topic failed, all attempts exhausted." ) );
                returnStatus = EXIT_FAILURE;
            }
            else if( backoffAlgStatus == BackoffAlgorithmSuccess )
            {
                LogWarn( ( "Server rejected subscription request. Retrying "
                           "connection after %hu ms backoff.",
                           ( unsigned short ) nextRetryBackOff ) );
                Clock_SleepMs( nextRetryBackOff );
            }
        }
    } while( ( globalSubAckStatus == MQTTSubAckFailure ) && ( backoffAlgStatus == BackoffAlgorithmSuccess ) );

    return returnStatus;
}
#endif
/*-----------------------------------------------------------*/

static void eventCallback( MQTTContext_t * pMqttContext,
                           MQTTPacketInfo_t * pPacketInfo,
                           MQTTDeserializedInfo_t * pDeserializedInfo ) {

    uint16_t packetIdentifier;

    assert( pMqttContext != NULL );
    assert( pPacketInfo != NULL );
    assert( pDeserializedInfo != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pMqttContext;

    packetIdentifier = pDeserializedInfo->packetIdentifier;

    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH ) {
        assert( pDeserializedInfo->pPublishInfo != NULL );
        /* Handle incoming publish. */
        handleIncomingPublish( pDeserializedInfo->pPublishInfo, packetIdentifier );
    }
    else {
        /* Handle other packets. */
        switch( pPacketInfo->type ) {
            case MQTT_PACKET_TYPE_SUBACK:


                /* A SUBACK from the broker, containing the server response to our subscription request, has been received.
                 * It contains the status code indicating server approval/rejection for the subscription to the single topic
                 * requested. The SUBACK will be parsed to obtain the status code, and this status code will be stored in global
                 * variable globalSubAckStatus. */
                updateSubAckStatus( pPacketInfo );

                LogInfo( ( "pPacketInfo->type = %d,  remainingLength = %d\n\n",
                               pPacketInfo->type,
                               pPacketInfo->remainingLength ) );

                /* Check status of the subscription request. If globalSubAckStatus does not indicate
                 * server refusal of the request (MQTTSubAckFailure), it contains the QoS level granted
                 * by the server, indicating a successful subscription attempt. */
                if( globalSubAckStatus != MQTTSubAckFailure )
                {
                    LogInfo( ( "Subscribed to the topic. with maximum QoS %u.\n\n",
                               globalSubAckStatus ) );
                }

                /* Make sure ACK packet identifier matches with Request packet identifier. */
                assert( globalSubscribePacketIdentifier == packetIdentifier );
                break;

            case MQTT_PACKET_TYPE_UNSUBACK:
                LogInfo( ( "Unsubscribed from the topic.\n\n" ) );
                /* Make sure ACK packet identifier matches with Request packet identifier. */
                assert( globalUnsubscribePacketIdentifier == packetIdentifier );
                break;

            case MQTT_PACKET_TYPE_PINGRESP:

                /* Nothing to be done from application as library handles
                 * PINGRESP. */
                LogWarn( ( "PINGRESP should not be handled by the application "
                           "callback when using MQTT_ProcessLoop.\n\n" ) );
                break;

            case MQTT_PACKET_TYPE_PUBACK:
                LogInfo( ( "PUBACK received for packet id %u.\n\n",
                           packetIdentifier ) );
                /* Cleanup publish packet when a PUBACK is received. */
                cleanupOutgoingPublishWithPacketID( packetIdentifier );
                break;

            /* Any other packet type is invalid. */
            default:
                LogError( ( "Unknown packet type received:(%02x).\n\n",
                            pPacketInfo->type ) );
        }
    }
}
/*-----------------------------------------------------------*/

static int establishMqttSession( MQTTContext_t * pMqttContext,
                                 bool createCleanSession,
                                 bool * pSessionPresent ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;
    MQTTConnectInfo_t connectInfo = { 0 };

    assert( pMqttContext != NULL );
    assert( pSessionPresent != NULL );

    /* Establish MQTT session by sending a CONNECT packet. */

    /* If #createCleanSession is true, start with a clean session
     * i.e. direct the MQTT broker to discard any previous session data.
     * If #createCleanSession is false, directs the broker to attempt to
     * reestablish a session which was already present. */
    connectInfo.cleanSession = createCleanSession;

    /* The client identifier is used to uniquely identify this MQTT client to
     * the MQTT broker. In a production device the identifier can be something
     * unique, such as a device serial number. */
    connectInfo.pClientIdentifier = awsiot_clientid;
    connectInfo.clientIdentifierLength = strlen(awsiot_clientid);

    // connectInfo.pClientIdentifier = CLIENT_IDENTIFIER;
    // connectInfo.clientIdentifierLength = CLIENT_IDENTIFIER_LENGTH;

    /* The maximum time interval in seconds which is allowed to elapse
     * between two Control Packets.
     * It is the responsibility of the Client to ensure that the interval between
     * Control Packets being sent does not exceed the this Keep Alive value. In the
     * absence of sending any other Control Packets, the Client MUST send a
     * PINGREQ Packet. */
    connectInfo.keepAliveSeconds = MQTT_KEEP_ALIVE_INTERVAL_SECONDS;

    #ifdef CLIENT_USERNAME
        connectInfo.pUserName = CLIENT_USERNAME_WITH_METRICS;
        connectInfo.userNameLength = strlen( CLIENT_USERNAME_WITH_METRICS );
        connectInfo.pPassword = CLIENT_PASSWORD;
        connectInfo.passwordLength = strlen( CLIENT_PASSWORD );
    #else
        connectInfo.pUserName = METRICS_STRING;
        connectInfo.userNameLength = METRICS_STRING_LENGTH;
        /* Password for authentication is not used. */
        connectInfo.pPassword = NULL;
        connectInfo.passwordLength = 0U;
    #endif /* ifdef CLIENT_USERNAME */

    /* Send MQTT CONNECT packet to broker. */
    mqttStatus = MQTT_Connect( pMqttContext, &connectInfo, NULL, CONNACK_RECV_TIMEOUT_MS, pSessionPresent );

    if( mqttStatus != MQTTSuccess ) {
        returnStatus = EXIT_FAILURE;
        Cdbg(APP_DBG, "Connection with MQTT broker failed with status %s, mqttStatus = %d",
                    MQTT_Status_strerror( mqttStatus ), mqttStatus);
    }
    else {
        Cdbg(APP_DBG, "MQTT connection successfully established with broker");
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int disconnectMqttSession( MQTTContext_t * pMqttContext ) {
    MQTTStatus_t mqttStatus = MQTTSuccess;
    int returnStatus = EXIT_SUCCESS;

    assert( pMqttContext != NULL );

    /* Send DISCONNECT. */
    mqttStatus = MQTT_Disconnect( pMqttContext );

    if( mqttStatus != MQTTSuccess ) {
        LogError( ( "Sending MQTT DISCONNECT failed with status=%s.",
                    MQTT_Status_strerror( mqttStatus ) ) );
        returnStatus = EXIT_FAILURE;
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

void publish_shadow_remote_connection(const int tunnel_enable, const char *state) {

    char publish_data[512] = {0};
    snprintf(publish_data, 512, "{ \"state\": %s}",  state);

    Cdbg(APP_DBG, "publish topic=%s, publish_data=%s", shadow_remote_connection_topic, publish_data);

    int returnStatus = publishToTopic( &mqttContext_g, shadow_remote_connection_topic, publish_data);

    if (returnStatus == SUCCESS) {
        Cdbg(APP_DBG, "OK");
    } 
    else {
        Cdbg(APP_DBG, "FAIL");
    }
}

//  publish topic : asus / %awsiot_clientid% / {xxx} /update
void publish_router_service_topic(const char *topic, const char *msg) {

    char publish_data[512] = {0};
    char service_topic[256] = {0};
 

    snprintf(publish_data, 512, "%s",  msg);
    snprintf(service_topic, 256, "asus/%s/%s/update", awsiot_clientid, topic);


    LogInfo( ("publish_router_service_topic, publish topic -> %s (%d), publish_data = %s (%d)",  service_topic, strlen(service_topic), publish_data, strlen(publish_data)) );

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, service_topic, publish_data);

    if (returnStatus == SUCCESS) {
        Cdbg(APP_DBG, "OK");
    } 
    else {
        Cdbg(APP_DBG, "FAIL");
    }
}

void publish_shadow_tunnel_test_result(const char* source, const char* target, const char* type, int error) {

    if (g_json_object_general_db_reported==NULL) {
        return;
    }

    json_object *tunnel_test_obj = NULL;

    if (!json_object_object_get_ex(g_json_object_general_db_reported, "tunnel_test", &tunnel_test_obj)) {
        tunnel_test_obj = json_object_new_object();
        json_object_object_add(g_json_object_general_db_reported, "tunnel_test", tunnel_test_obj);
    }

    json_object *target_obj = NULL;
    if (!json_object_object_get_ex(tunnel_test_obj, target, &target_obj)) {
        target_obj = json_object_new_object();
        json_object_object_add(tunnel_test_obj, target, target_obj);
    }

    json_object_object_add(target_obj, "type", json_object_new_string(type));
    json_object_object_add(target_obj, "error", json_object_new_int(error));

    Cdbg(APP_DBG, "%s", json_object_get_string(g_json_object_general_db_reported));

    // publish_general_shadow_reported();
    
}

void update_db_tunnel_test_result(const char* source, const char* target, const char* type, int error) {
#if defined(RTCONFIG_TUNNEL)    
    if (strlen(source)<=0 || strlen(target)<=0 || strlen(type)<=0) {
        return 0;
    }

    Cdbg(APP_DBG, "update_db_tunnel_test_result source=%s, target=%s, type=%s", source, target, type);

    char event[AAE_MAX_IPC_PACKET_SIZE];
    char out[AAE_MAX_IPC_PACKET_SIZE];
    snprintf(event, sizeof(event), AAE_AWSIOT_UPDATE_DB_TUNNELTEST_MSG, EID_AWSIOT_UPDATE_DB_TUNNELTEST, target, type, error);
    aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);

    Cdbg(APP_DBG, "update_db_tunnel_test_result out=%s", out);

    return 1;
#else
    return 0;
#endif
}
/*-----------------------------------------------------------*/

static int subscribeToTopic( MQTTContext_t * pMqttContext ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;

    assert( pMqttContext != NULL );

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pGlobalSubscriptionList, 0x00, sizeof( pGlobalSubscriptionList ) );

    /* subscribes topic and uses QOS1. */

    struct subTopicHandler *handler = NULL;
    int i = 0;

    for (handler = &CHK_SUB_TOPICS[0]; handler->sub_topic_id > 0; handler++) {
        if (handler->enable == 1) {
            pGlobalSubscriptionList[i].qos = MQTTQoS1;
            pGlobalSubscriptionList[i].pTopicFilter = handler->topic;
            pGlobalSubscriptionList[i].topicFilterLength = strlen(handler->topic);

            Cdbg(APP_DBG, "Start subscribe topic = %s , len = %d",
                       pGlobalSubscriptionList[i].pTopicFilter,
                       pGlobalSubscriptionList[i].topicFilterLength);

            i++;
        }
    }

    pGlobalSubscriptionListCount = i;


    /* Generate packet identifier for the SUBSCRIBE packet. */
    globalSubscribePacketIdentifier = MQTT_GetPacketId( pMqttContext );

    /* Send SUBSCRIBE packet. */
    mqttStatus = MQTT_Subscribe( pMqttContext,
                                 pGlobalSubscriptionList,
                                 pGlobalSubscriptionListCount,
                                 globalSubscribePacketIdentifier );

    if( mqttStatus != MQTTSuccess ) {
        returnStatus = EXIT_FAILURE;
        Cdbg(APP_DBG, "Failed to send SUBSCRIBE packet to broker with error = %s.",
                    MQTT_Status_strerror( mqttStatus ));
    } else {
        Cdbg(APP_DBG, "SUBSCRIBE sent for topic , Success");
    }

    return returnStatus;
}
/*-----------------------------------------------------------*/

#if 0
static int unsubscribeFromTopic( MQTTContext_t * pMqttContext ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;

    assert( pMqttContext != NULL );

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pGlobalSubscriptionList, 0x00, sizeof( pGlobalSubscriptionList ) );

    /* subscribes topic and uses QOS1. */
    struct subTopicHandler *handler = NULL;
    int i = 0;

    for(handler = &CHK_SUB_TOPICS[0]; handler->sub_topic_id > 0; handler++){
        if (handler->enable == 1) {
            pGlobalSubscriptionList[ i ].qos = MQTTQoS1;
            pGlobalSubscriptionList[ i ].pTopicFilter = handler->topic;
            pGlobalSubscriptionList[ i ].topicFilterLength = strlen(handler->topic);

            LogInfo( ( "UNSUBSCRIBE topic = %s , len = %d\n\n",
                       pGlobalSubscriptionList[ i ].pTopicFilter,
                       pGlobalSubscriptionList[ i ].topicFilterLength ) );

            i++;
        }
    }

    // int i = 0;
    // for(i = 0; i < subscribe_topic_number; i++) {

    //     pGlobalSubscriptionList[ i ].qos = MQTTQoS1;
    //     pGlobalSubscriptionList[ i ].pTopicFilter = subscribe_topic_array[i];
    //     pGlobalSubscriptionList[ i ].topicFilterLength = strlen(subscribe_topic_array[i]);


    //     LogInfo( ( "UNSUBSCRIBE topic = %s , len = %d\n\n",
    //                pGlobalSubscriptionList[ i ].pTopicFilter,
    //                pGlobalSubscriptionList[ i ].topicFilterLength ) );
    // }

    /* Generate packet identifier for the UNSUBSCRIBE packet. */
    globalUnsubscribePacketIdentifier = MQTT_GetPacketId( pMqttContext );

    /* Send UNSUBSCRIBE packet. */
    mqttStatus = MQTT_Unsubscribe( pMqttContext,
                                   pGlobalSubscriptionList,
                                   pGlobalSubscriptionListCount,
                                   globalUnsubscribePacketIdentifier );

    // mqttStatus = MQTT_Unsubscribe( pMqttContext,
    //                                pGlobalSubscriptionList,
    //                                sizeof( pGlobalSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
    //                                globalUnsubscribePacketIdentifier );



    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to send UNSUBSCRIBE packet to broker with error = %s.",
                    MQTT_Status_strerror( mqttStatus ) ) );
        returnStatus = EXIT_FAILURE;
    }
    else
    {
        LogInfo( ( "UNSUBSCRIBE sent for topic, Success.\n\n" ) );
    }

    return returnStatus;
}
#endif
/*-----------------------------------------------------------*/

static int publishToTopic( MQTTContext_t * pMqttContext, char * pTopicName, char * pPayload ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    uint8_t publishIndex = MAX_OUTGOING_PUBLISHES;

    assert( pMqttContext != NULL );

    /* Get the next free index for the outgoing publish. All QoS1 outgoing
     * publishes are stored until a PUBACK is received. These messages are
     * stored for supporting a resend if a network connection is broken before
     * receiving a PUBACK. */
    returnStatus = getNextFreeIndexForOutgoingPublishes( &publishIndex );

    if( returnStatus == EXIT_FAILURE ) {
        LogError( ( "Unable to find a free spot for outgoing PUBLISH message.\n\n" ) );
    }
    else {
        /* This example publishes to only one topic and uses QOS1. */
        // outgoingPublishPackets[ publishIndex ].pubInfo.qos = MQTTQoS1;
        outgoingPublishPackets[ publishIndex ].pubInfo.qos = MQTTQoS0;
        outgoingPublishPackets[ publishIndex ].pubInfo.pTopicName = pTopicName;
        outgoingPublishPackets[ publishIndex ].pubInfo.topicNameLength = strlen(pTopicName);
        outgoingPublishPackets[ publishIndex ].pubInfo.pPayload = pPayload;
        outgoingPublishPackets[ publishIndex ].pubInfo.payloadLength = strlen(pPayload);

        /* Get a new packet id. */
        outgoingPublishPackets[ publishIndex ].packetId = MQTT_GetPacketId( pMqttContext );

        /* Send PUBLISH packet. */
        mqttStatus = MQTT_Publish( pMqttContext,
                                   &outgoingPublishPackets[ publishIndex ].pubInfo,
                                   outgoingPublishPackets[ publishIndex ].packetId );

        if( mqttStatus != MQTTSuccess ) {
            Cdbg(APP_DBG, "Topic = %s (len=%d), Failed to send PUBLISH packet to broker with error = %s.",
                        pTopicName, strlen(pTopicName), MQTT_Status_strerror( mqttStatus )  );

            cleanupOutgoingPublishAt( publishIndex );
            returnStatus = EXIT_FAILURE;
        }
        else {

            LogInfo( ( "PUBLISH sent publishIndex[%d], topic [%s] (len=%d) to broker with packet ID %u.",
                       publishIndex, pTopicName, strlen(pTopicName),
                       outgoingPublishPackets[ publishIndex ].packetId ) );

            // Cdbg(APP_DBG, "PUBLISH sent publishIndex[%d], topic [%s] (len=%d) to broker with packet ID %u.",
            //            publishIndex, pTopicName, strlen(pTopicName),
            //            outgoingPublishPackets[ publishIndex ].packetId  );

            mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

            if( mqttStatus == MQTTSuccess ) {

                LogInfo( ( "MQTT_ProcessLoop  mqttStatus == == == MQTTSuccess" ) );

            } 
            else {
                LogInfo( ( "MQTT_ProcessLoop mqttStatus != != != MQTTSuccess" ) );
            }

            LogInfo( ( "Cleaning up all the stored outgoing publishes." ) );

            cleanupOutgoingPublishes();
        }
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int initializeMqtt( MQTTContext_t * pMqttContext,
                           NetworkContext_t * pNetworkContext ) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;
    MQTTFixedBuffer_t networkBuffer;
    TransportInterface_t transport;

    assert( pMqttContext != NULL );
    assert( pNetworkContext != NULL );

    /* Fill in TransportInterface send and receive function pointers.
     * For this demo, TCP sockets are used to send and receive data
     * from network. Network context is SSL context for OpenSSL.*/
    transport.pNetworkContext = pNetworkContext;
    transport.send = Openssl_Send;
    transport.recv = Openssl_Recv;

    /* Fill the values for network buffer. */
    networkBuffer.pBuffer = buffer;
    networkBuffer.size = NETWORK_BUFFER_SIZE;

    /* Initialize MQTT library. */
    mqttStatus = MQTT_Init( pMqttContext,
                            &transport,
                            Clock_GetTimeMs,
                            eventCallback,
                            &networkBuffer );

    if( mqttStatus != MQTTSuccess ) {
        returnStatus = EXIT_FAILURE;
        Cdbg(APP_DBG, "MQTT init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ));
    } 
    else {
        LogInfo( ( "MQTT init successful, mqttStatus = %d", mqttStatus) );
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int subscribeTopic( MQTTContext_t * pMqttContext,
                           bool * pClientSessionPresent, 
                           bool * pMqttSessionEstablished ) {

    int returnStatus = EXIT_SUCCESS;
    bool brokerSessionPresent;
    // bool mqttSessionEstablished = false, brokerSessionPresent;

    MQTTStatus_t mqttStatus = MQTTSuccess;
    bool createCleanSession = false;

    assert( pMqttContext != NULL );
    assert( pClientSessionPresent != NULL );
    assert( pMqttSessionEstablished != NULL );

    /* A clean MQTT session needs to be created, if there is no session saved
     * in this MQTT client. */
    createCleanSession = ( *pClientSessionPresent == true ) ? false : true;

    /* Establish MQTT session on top of TCP+TLS connection. */
    // LogInfo( ( "Creating an MQTT connection to %.*s.",
    //            AWS_IOT_ENDPOINT_LENGTH,
    //            AWS_IOT_ENDPOINT ) );

    // Cdbg(APP_DBG, "Creating an MQTT connection to %.*s.",
    //            AWS_IOT_ENDPOINT_LENGTH,
    //            AWS_IOT_ENDPOINT );

    Cdbg(APP_DBG, "Creating an MQTT connection to %s.",
               awsiot_endpoint );

    /* Sends an MQTT Connect packet using the established TLS session,
     * then waits for connection acknowledgment (CONNACK) packet. */
    returnStatus = establishMqttSession( pMqttContext, createCleanSession, &brokerSessionPresent );

    if( returnStatus == EXIT_SUCCESS ) {

        mqttContext_g = *pMqttContext;
        /* Keep a flag for indicating if MQTT session is established. This
         * flag will mark that an MQTT DISCONNECT has to be sent at the end
         * of the demo, even if there are intermediate failures. */
        // mqttSessionEstablished = true;

        *pMqttSessionEstablished = true;

        /* Update the flag to indicate that an MQTT client session is saved.
         * Once this flag is set, MQTT connect in the following iterations of
         * this demo will be attempted without requesting for a clean session. */
        *pClientSessionPresent = true;

        /* Check if session is present and if there are any outgoing publishes
         * that need to resend. This is only valid if the broker is
         * re-establishing a session which was already present. */
        if( brokerSessionPresent == true ) {
            Cdbg(APP_DBG, "An MQTT session with broker is re-established. "
                       "Resending unacked publishes." );

            /* Handle all the resend of publish messages. */
            returnStatus = handlePublishResend( pMqttContext );
        }
        else {
            LogInfo( ( "A clean MQTT connection is established."
                       " Cleaning up all the stored outgoing publishes.\n\n" ) );

            /* Clean up the outgoing publishes waiting for ack as this new
             * connection doesn't re-establish an existing session. */
            cleanupOutgoingPublishes();
        }
    }

    if( returnStatus == EXIT_SUCCESS ) {
        /* The client is now connected to the broker. Subscribe to the topic
         * as specified in MQTT_EXAMPLE_TOPIC at the top of this file by sending a
         * subscribe packet. This client will then publish to the same topic it
         * subscribed to, so it will expect all the messages it sends to the broker
         * to be sent back to it from the broker. This demo uses QOS1 in Subscribe,
         * therefore, the Publish messages received from the broker will have QOS1. */
        LogInfo( ( "Subscribing to the MQTT topic" ) );
        returnStatus = subscribeToTopic( pMqttContext );
    }

    if( returnStatus == EXIT_SUCCESS ) {
        /* Process incoming packet from the broker. Acknowledgment for subscription
         * ( SUBACK ) will be received here. However after sending the subscribe, the
         * client may receive a publish before it receives a subscribe ack. Since this
         * demo is subscribing to the topic to which no one is publishing, probability
         * of receiving publish message before subscribe ack is zero; but application
         * must be ready to receive any packet. This demo uses MQTT_ProcessLoop to
         * receive packet from network. */
        mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

        if( mqttStatus != MQTTSuccess ) {
            returnStatus = EXIT_FAILURE;
            LogError( ( "MQTT_ProcessLoop returned with status = %s.",
                        MQTT_Status_strerror( mqttStatus ) ) );
        }
    }

    /* Check if recent subscription request has been rejected. globalSubAckStatus is updated
     * in eventCallback to reflect the status of the SUBACK sent by the broker. */
    if( ( returnStatus == EXIT_SUCCESS ) && ( globalSubAckStatus == MQTTSubAckFailure ) ){
        /* If server rejected the subscription request, attempt to resubscribe to topic.
         * Attempts are made according to the exponential backoff retry strategy
         * implemented in retryUtils. */
        // LogInfo( ( "Server rejected initial subscription request. Attempting to re-subscribe to topic %.*s.",
        //            MQTT_EXAMPLE_TOPIC_LENGTH,
        //            MQTT_EXAMPLE_TOPIC ) );
        // returnStatus = handleResubscribe( pMqttContext );

        returnStatus == MQTTSubAckFailure;

    }

    return returnStatus;
}

void enable_sub_topic(int sub_topic_id) {

    struct subTopicHandler *handler = NULL;

    for(handler = &CHK_SUB_TOPICS[0]; handler->sub_topic_id > 0; handler++){
        if (handler->sub_topic_id == sub_topic_id) {
            break;
        }
    }

    if(handler != NULL && handler->sub_topic_id > 0) {
        
        if (strstr(handler->topic, "[awsiotclientid]")) {
            char tmp[MAX_TOPIC_LENGTH] = {0};
            replace_str(&handler->topic[0], "[awsiotclientid]", awsiot_clientid, (char *)&tmp[0]);
            strncpy(handler->topic, tmp, strlen(tmp));
        }

        handler->enable = 1;
    }

}

static int update_default_shadow_data(MQTTContext_t * pMqttContext) {

    char tencent_report_data[125] = {0};

#if defined(RTCONFIG_TC_GAME_ACC)
    unsigned long long int total_space = 0;
    unsigned long long int available_space = 0;
    unsigned long long int used_space = 0;
    int tencent_download_enable = nvram_get_int("tencent_download_enable");

    if (tencent_download_enable==1) {
        
        //- The serial number for tencent use.
        char label_mac_tencentgame[32];
        snprintf(label_mac_tencentgame, 32, "%stencentgame", nvram_safe_get("label_mac"));
        get_sha256(label_mac_tencentgame, tencent_sn);

        //- Get usb usage
        get_usb_space(&total_space, &used_space, &available_space, nvram_safe_get("tencent_download_path"));
    }
    
    snprintf(tencent_report_data, 125, ",\"tencentgame\": {\"sn\": \"%s\", \"enable\": %d, \"storage\": { \"total\": %llu, \"free\": %llu } }", 
             tencent_sn,
             tencent_download_enable,
             total_space,
             available_space);

#endif

    char publish_data[512] = {0};
    snprintf(publish_data, 512, "{\"state\": {\"reported\": {\"firmver\": \"%s\", \"buildno\": \"%s\", \"wan0_realip_ip\":\"%s\", \"wan0_hwaddr\":\"%s\", \"label_mac\":\"%s\"%s}}}", 
                                    nvram_safe_get("firmver"), 
                                    nvram_safe_get("buildno"), 
                                    nvram_safe_get("wan0_realip_ip"), 
                                    nvram_safe_get("wan0_hwaddr"), 
                                    nvram_safe_get("label_mac"),
                                    tencent_report_data);

    Cdbg(APP_DBG, "publish topic len = %d -> %s\ndata len=%d -> %s", strlen(default_shadow_topic), default_shadow_topic, strlen(publish_data), publish_data);

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( pMqttContext, default_shadow_topic, publish_data);

    if (returnStatus == MQTTSuccess) {
        Cdbg(APP_DBG, "topic[%s] publish success", default_shadow_topic);
    } 
    else {
        Cdbg(APP_DBG, "topic publish error , topic = %s", default_shadow_topic);
    }

#if defined(RTCONFIG_TC_GAME_ACC)
    tencentgame_download_enable_tmp = tencent_download_enable;
#endif

    return EXIT_SUCCESS;
}

static int get_general_shadow_data(MQTTContext_t * pMqttContext) {
    char get_general_shadow_topic[MAX_TOPIC_LENGTH] = {0};
    char publish_data[1] = {0};
    snprintf(get_general_shadow_topic, MAX_TOPIC_LENGTH, "$aws/things/%s/shadow/name/general/get", awsiot_clientid);
    publishToTopic( pMqttContext, get_general_shadow_topic, publish_data);
    return EXIT_SUCCESS;
}

static int notice_to_upload_settings() {
    pid_t *pidList;
    pid_t *pl;
    char cmd [32];
    
    pidList = find_pid_by_name("uploader");
    for (pl = pidList; *pl; pl++) {
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "kill -%d %d", SIGUSR1, *pl);
        Cdbg(APP_DBG, "cmd = %s", cmd);
        system(cmd);
    }
}

int check_tencentgame_status() {

    if(nvram_get_int("tencent_download_enable") != tencentgame_download_enable_tmp) {
        Cdbg(APP_DBG, "tencentgame function > status change");
        return EXIT_SUCCESS;
    } 
    
    return EXIT_FAILURE;
}

int check_wireguard_config_has_been_applied(const char* feature_name, const char* group_name, const char* unique_key) {
    
    if (!g_json_object_general_db_reported) {
        return 0;
    }
   	
	if(!json_object_is_type(g_json_object_general_db_reported, json_type_object)) {
        return 0;
    }
 
    int i = 0;

    json_object_object_foreach(g_json_object_general_db_reported, the_featureName, shadowFeatureObj) {
        
        if (shadowFeatureObj==NULL) {
            continue;
        }
		
		if(!json_object_is_type(shadowFeatureObj, json_type_object)) {
        	continue;
    	}

        if (strncmp(the_featureName, feature_name, strlen(the_featureName))==0) {

            json_object_object_foreach(shadowFeatureObj, the_groupName, shadowStatusListObj) {
                if (shadowStatusListObj==NULL) {
                    continue;
                }

                if (strncmp(the_groupName, group_name, strlen(the_groupName))==0) {

                    struct array_list* json_config_array_list = json_object_get_array(shadowStatusListObj);
                    if (json_config_array_list==NULL) {
                        break;
                    }

                    int array_length = json_config_array_list->length;

                    for (i=0; i<array_length; i++) {

                        json_object* shadowGroupObj = json_object_array_get_idx(shadowStatusListObj, i);

                        json_object *unique_key_obj = NULL;

                        char the_unique_key[64] = {0};
                        if(json_object_object_get_ex(shadowGroupObj, "timestamp", &unique_key_obj))
                            strlcpy(the_unique_key, (char *)json_object_get_string(unique_key_obj), sizeof(the_unique_key));
                        
                        if (strlen(the_unique_key)>0 && strncmp(the_unique_key, unique_key, strlen(the_unique_key))==0) {
                            Cdbg(APP_DBG, "wireguard config has been applied, feature_name=%s, group_name=%s, unique_key=%s", feature_name, group_name, unique_key);
                            return 1;
                        }
                    }
                }
            }
            
            return 0;
        }
    }
    
    return 0;
}

int update_dynamic_db() {
    
    if (!g_json_object_general_db_reported) {
        return EXIT_FAILURE;
    }
    
    int i = 0;
    int do_shadow_update = 0;
    json_object *shadowConfigObj = NULL;
    json_object *wireguardStatusObj = NULL;
    
    // json_object_object_get_ex(g_json_object_general_db_reported, "config", &shadowConfigObj);
    // if (!shadowConfigObj) {
    //     return EXIT_FAILURE;
    // }
   
	if(!json_object_is_type(g_json_object_general_db_reported, json_type_object)) {
    	return EXIT_FAILURE;
    }
 
    json_object_object_foreach(g_json_object_general_db_reported, featureName, shadowFeatureObj) {
        
        if (shadowFeatureObj==NULL) {
            continue;
        }

        int feature_id = get_supported_feature_id(featureName);
        
        switch (feature_id) {

#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD)
            case FEATURE_WIREGUARD_SERVER: {
               	
				if(!json_object_is_type(shadowFeatureObj, json_type_object)) {
        			break;
    			}
 
                int wgs1_enable = nvram_get_int("wgs1_enable");
                
                json_object_object_foreach(shadowFeatureObj, groupName, shadowStatusListObj) {

                    if (shadowStatusListObj==NULL) {
                        continue;
                    }

                    struct array_list* json_config_array_list = json_object_get_array(shadowStatusListObj);
                    if (json_config_array_list==NULL) {
                        break;
                    }

                    int array_length = json_config_array_list->length;

                    for (i=0; i<array_length; i++) {

                        json_object* shadowGroupObj = json_object_array_get_idx(shadowStatusListObj, i);

                        json_object *wgs_status_obj = NULL;

                        if (json_object_object_get_ex(shadowGroupObj, "wireguard_status", &wgs_status_obj)) {

                            const char* wgs_status = json_object_get_string(wgs_status_obj);
                            
                            char current_status[10] = {0};
                            if (wgs1_enable==1) {
                                //- working
                                strncpy(current_status, "working", 7);
                            }
                            else if (wgs1_enable==0) {
                                //- pause
                                strncpy(current_status, "pause", 5);
                            }
                            
                            if (strlen(current_status)>0 && strncmp(current_status, wgs_status, strlen(wgs_status))!=0) {
                                Cdbg(APP_DBG, "groupName %s, change wireguard_server wireguard_status from %s to %s", groupName, wgs_status, current_status);
                                json_object_object_add(shadowGroupObj, "wireguard_status", json_object_new_string(current_status));
                                do_shadow_update = 1;
                            }
                        }
                    }

                    if (do_shadow_update==1) {
                        update_db_wireguard(groupName, "master", json_object_get_string(shadowStatusListObj));
                    }
                }

                break;
            }

            case FEATURE_WIREGUARD_CLIENT: {
               	
				if(!json_object_is_type(shadowFeatureObj, json_type_object)) {
                    break;
                }
 
                if (wireguardStatusObj==NULL) {
                    wireguardStatusObj = json_object_new_object();
                    get_wgc_connect_status(wireguardStatusObj);
                    // Cdbg(APP_DBG, "wireguard_status=%s", json_object_get_string(wireguardStatusObj));
                }
                
                json_object_object_foreach(shadowFeatureObj, groupName, shadowStatusListObj) {
                    
                    if (shadowStatusListObj==NULL) {
                        continue;
                    }
                    
                    struct array_list* json_config_array_list = json_object_get_array(shadowStatusListObj);
                    if (json_config_array_list==NULL) {
                        break;
                    }
                    
                    int i = 0;
                    int array_length = json_config_array_list->length;
                    
                    for (i=0; i<array_length; i++) {
                        json_object* shadowGroupObj = json_object_array_get_idx(shadowStatusListObj, i);

                        json_object *wgc_index_obj = NULL;
                        json_object *wgc_status_obj = NULL;
                        
                        if (json_object_object_get_ex(shadowGroupObj, "index", &wgc_index_obj) &&
                            json_object_object_get_ex(shadowGroupObj, "wireguard_status", &wgc_status_obj)) {
                            
                            const char* wgc_index = json_object_get_string(wgc_index_obj);
                            const char* wgc_status = json_object_get_string(wgc_status_obj);

                            char ifname[8] = {0};
                            snprintf(ifname, sizeof(ifname), "%s%s", WG_CLIENT_IF_PREFIX, wgc_index);
                            
                            json_object *wireguard_obj = NULL;
                            json_object *wireguard_caller_obj = NULL;
                            json_object *wireguard_enable_obj = NULL;
                            json_object *wireguard_connected_obj = NULL;

                            json_object_object_get_ex(wireguardStatusObj, ifname, &wireguard_obj);
                            if (wireguard_obj==NULL) {
                                continue;
                            }
                            
                            json_object_object_get_ex(wireguard_obj, "caller", &wireguard_caller_obj);
                            json_object_object_get_ex(wireguard_obj, "enable", &wireguard_enable_obj);
                            json_object_object_get_ex(wireguard_obj, "connected", &wireguard_connected_obj);

                            char current_status[10] = {0};
                            const char* wcaller = json_object_get_string(wireguard_caller_obj);
                            const char* wenable = json_object_get_string(wireguard_enable_obj);
                            const char* wconntected = json_object_get_string(wireguard_connected_obj);

                            // Cdbg(APP_DBG, "groupName=%s", groupName);
                            // Cdbg(APP_DBG, "ifname=%s", ifname);
                            // Cdbg(APP_DBG, "wcaller=%s", wcaller);
                            // Cdbg(APP_DBG, "wenable=%s", wenable);
                            // Cdbg(APP_DBG, "wconntected=%s", wconntected);
                            
                            if (strlen(wenable)>0 && !strncmp(wcaller, "AMSAPP", 6)) {
                                if (!strncmp(wenable, "1", 1) && !strncmp(wconntected, "1", 1)) {
                                    //- working
                                    strncpy(current_status, "working", 7);
                                }
                                else if (!strncmp(wenable, "1", 1) && !strncmp(wconntected, "0", 1)) {
                                    //- setup
                                    strncpy(current_status, "setup", 5);
                                }
                                else if (!strncmp(wenable, "0", 1)) {
                                    //- pause
                                    strncpy(current_status, "pause", 5);
                                }
                            }
                            else {
                                //- not exist
                                strncpy(current_status, "not_exist", 9);
                            }

                            // Cdbg(APP_DBG, "wgc_status=%s", wgc_status);
                            // Cdbg(APP_DBG, "current_status=%s", current_status);

                            if (strncmp(current_status, wgc_status, strlen(wgc_status))!=0) {
                                Cdbg(APP_DBG, "groupName=%s, change wireguard_client(%s) wireguard_status from %s to %s", groupName, ifname, wgc_status, current_status);
                                json_object_object_add(shadowGroupObj, "wireguard_status", json_object_new_string(current_status));
                                do_shadow_update = 1;
                            }
                        }
                    }

                    if (do_shadow_update==1) {
                        update_db_wireguard(groupName, "slave", json_object_get_string(shadowStatusListObj));
                    }
                }

                break;
            }
#endif
		
	    default:
	        break;
        }
    }
    
    // if (do_shadow_update==1) {
        // publish_general_shadow_reported();
    // }
    
    if (wireguardStatusObj!=NULL) {
        json_object_put(wireguardStatusObj);
    }

    return EXIT_SUCCESS;
}

int update_db_wireguard(char* group_name, char* role, const char* content) {

#if defined(RTCONFIG_TUNNEL)
    if (strlen(group_name)<=0 || strlen(role)<=0 || strlen(content)<=0) {
        return 0;
    }

    Cdbg(APP_DBG, "update_db_wireguard group_name=%s, role=%s, content=%s", group_name, role, content);

    char event[AAE_MAX_IPC_PACKET_SIZE];
    char out[AAE_MAX_IPC_PACKET_SIZE];
    snprintf(event, sizeof(event), AAE_AWSIOT_UPDATE_DB_WIREGUARD_MSG, EID_AWSIOT_UPDATE_DB_WIREGUARD, group_name, role, content);
    aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);

    Cdbg(APP_DBG, "update_db_wireguard out=%s", out);

    return 1;
#else
    return 0;
#endif
}

// int publish_general_shadow_reported() {
    
//     if (g_json_object_general_db_reported==NULL) {
//         return EXIT_FAILURE;
//     }

//     char shadow_topic[MAX_TOPIC_LENGTH] = {0};
//     const char* json_string = json_object_get_string(g_json_object_general_db_reported);
    
//     int reported_data_length = strlen(json_string) + 30;
//     char *reported_data = (char*)malloc(sizeof(char) * reported_data_length);
//     if (reported_data == NULL) {
//         SAVE_FREE_STRING(json_string);
//         return EXIT_FAILURE;
//     }
    
//     memset(reported_data, 0, sizeof(reported_data));

//     snprintf(shadow_topic, sizeof(shadow_topic), "$aws/things/%s/shadow/name/general/update", awsiot_clientid);
    
//     snprintf(reported_data, reported_data_length, "{\"state\":{\"reported\":%s}}", json_string);

//     Cdbg(APP_DBG, "shadow_topic=%s", shadow_topic);
//     Cdbg(APP_DBG, "reported_data=%s", reported_data);

//     if (publishToTopic(&mqttContext_g, shadow_topic, reported_data) != SUCCESS) {
//         SAVE_FREE_STRING(json_string);
//         SAVE_FREE_STRING(reported_data);

//         Cdbg(APP_DBG, "FAIL");
//         return EXIT_FAILURE;
//     }

//     SAVE_FREE_STRING(reported_data);

//     Cdbg(APP_DBG, "OK");
//     return EXIT_SUCCESS;
// }

int run_tc_download() {

    int tencent_download_enable = nvram_get_int("tencent_download_enable");
    if (tencent_download_enable!=1) {
        return;
    }

    LogInfo( ( "run_tc_download, search_tc_file_count = %d", search_tc_file_count) );

    search_tc_file_count++;

    if( (search_tc_file_count % 120) == 1) {

        int status = compare_received_session(pGlobalSubscriptionListCount, 0);

        LogInfo( ( "run tc_download status = %d", status ) );

        if (status == 0) {
            if(pids("tc_download")) {
                LogInfo( ("run_tc_download -> tc_download running") );
            } 
            else if(search_tc_file_count < 120) {
                int sys_code = system("tc_download&");
                Cdbg(APP_DBG, "tc_download run -> sys_code = %d", sys_code);
            }
        }
    }

    if(search_tc_file_count > 360)
        search_tc_file_count = 0;

    return 0;
}

int main_loop(MQTTContext_t * pMqttContext, bool * pMqttSessionEstablished) {

    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    unsigned int while_count = 0;

    do {
        while_count++;

        if(publish_running) {
            sleep( WAITING_PUBLISHES_SECONDS );
            continue;
        }

        mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

        if( mqttStatus != MQTTSuccess ) {
            Cdbg(APP_DBG, "MQTT_ProcessLoop returned with status = %d [%s].",
                        mqttStatus, MQTT_Status_strerror( mqttStatus ) );

            returnStatus = EXIT_FAILURE;
            break;

        } else {
            mqttContext_g = *pMqttContext;
        }

#if defined(RTCONFIG_TC_GAME_ACC)
        run_tc_download();

        if(check_tencentgame_status() == EXIT_SUCCESS) {
            returnStatus = EXIT_SUCCESS;
            break;
        }
#endif

        update_dynamic_db();

        // Cdbg(APP_DBG, "1Waiting MQTT data 5s, while_count = %d", while_count);

        sleep( WAITING_PUBLISHES_SECONDS );

        // test function 
        // if(while_count == 1) {
            // publish_topic_test(1);

            // char *test = "{   \"topic\" : \"sip_tunnel\" ,   \"msg\" : \"{\\\"aae_sip_connected\\\":\\\"1\\\"}\"  }";
            // char test[MAX_TOPIC_LENGTH];
            // snprintf(test, MAX_TOPIC_LENGTH, "{ \"aae_sip_connected\":\"1\" }");

            // cm_sendIpcHandler(AWSIOT_IPC_SOCKET_PATH, test, strlen(test));
    
        // } else if(while_count == 6) {
        //     publish_topic_test(2);

        // } else if(while_count == 9) {
        //     publish_topic_test(3);
        // }

    } while(1);

    /* Send an MQTT Disconnect packet over the already connected TCP socket.
     * There is no corresponding response for the disconnect packet. After sending
     * disconnect, client must close the network connection. */
    if( *pMqttSessionEstablished == true ) {
        // LogInfo( ( "Disconnecting the MQTT connection with %.*s.",
        //            AWS_IOT_ENDPOINT_LENGTH,
        //            AWS_IOT_ENDPOINT ) );

        LogInfo( ( "Disconnecting the MQTT connection with %s.",
                   awsiot_endpoint ) );


        if( returnStatus == EXIT_FAILURE ) {
            /* Returned status is not used to update the local status as there
             * were failures in demo execution. */
            ( void ) disconnectMqttSession( pMqttContext );
        }
        else {
            returnStatus = disconnectMqttSession( pMqttContext );
        }
    }

    /* Reset global SUBACK status variable after completion of subscription request cycle. */
    globalSubAckStatus = MQTTSubAckFailure;

    return returnStatus;
    // return EXIT_AWSIOT;
}

int publish_subscribe_channel() {

    int report_status = -1;
    char publish_data[JSON_CONTENT_SIZE];

    report_status = parse_download_update(publish_data, awsiot_clientid, tencent_sn);

    if(report_status != 0) {
        Cdbg(APP_DBG, "parse_download_update error, publish data error, report_status --> %d", report_status);
        return -1;
    }

    int publish_len = strlen(publish_data);
    Cdbg(APP_DBG, "publish data (%d) = %s\n", publish_len, publish_data);

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, publish_topic, publish_data);

    if (returnStatus == SUCCESS) {
        Cdbg(APP_DBG, "aws_iot_mqtt_publish  OK -> %s", publish_topic);
    } 
    else {
        Cdbg(APP_DBG, "aws_iot_mqtt_publish , topic -> %s", publish_topic);
    }

    return 0;
}

#define AWSIOT_SIG_ACTION SIGUSR1

enum {
    AWSIOT_ACTION_SIGUSR1,
};


static void awsiot_sig_handler(int sig) {

    Cdbg(APP_DBG, "Get tc_download sig = %d", sig);

    if (publish_running) {
        Cdbg(APP_DBG, "publish_running, waiting next  sig");
        sig = -99;
    }

    switch (sig) {

        case AWSIOT_SIG_ACTION: {
            // subscribe_channel_thread();

            while(publish_running) {
                publish_waiting++;
                LogInfo( ( "publish running waiting 1s") );
                sleep(1);
                if(publish_waiting >= 10) {
                    publish_waiting++;
                    break;
                }
            }

            publish_running = true;

            copy_tencent_update_tmp();

            compare_tencent_session();

            publish_subscribe_channel();

            remove(TENCENT_SESSION_JSON_TMP);

            // sleep(1);

            publish_running = false;
            publish_waiting = 0;

        }

        break;
    }
}

size_t hdf(char* b, size_t size, size_t nitems, void *userdata) {
    printf("%s", b);
    return 0;
}
/*-----------------------------------------------------------*/

struct url_data {
    size_t size;
    char* data;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) {

    size_t index = data->size;
    size_t n = (size * nmemb);
    
    data->size += (size * nmemb);

    // fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb);
    // char* tmp = (char*) realloc(data->data, data->size + 1); /* +1 for '\0' */

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

// size_t size = 0;

// size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
//     ((char*)stream)->append((char*)ptr, 0, size*count);
//     return size*count;
// }

#if 0
struct my_info {
    int shoesize;
    char *secret;
};

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {

    struct my_info *i = (struct my_info *)userdata;
 
    /* now this callback can access the my_info struct */
 
    return nitems * size;
}
#endif
/*-----------------------------------------------------------*/

static int rcv_tencentgame_data(char *data, char* topic) {
    
    char tmpRecive[NETWORK_BUFFER_SIZE] = {0};

    if(get_session_data(data, tmpRecive) != 0) {
        Cdbg(APP_DBG, "Incoming data: Received JSON is not valid");
        return 1;
    }
    
    LogInfo( ( "tmpRecive : %s", tmpRecive) );
    // snprintf(tmpRecive, NETWORK_BUFFER_SIZE, "%s", shadowRxBuf);

    // it is not empty string
    if ((tmpRecive != NULL) && (strlen(tmpRecive) > 0) ) {

        int status = split_received_tencent_session(topic);

        status = save_received_session(tmpRecive, topic);

        status = merge_received_session(pGlobalSubscriptionListCount);

        status = compare_received_session(pGlobalSubscriptionListCount, 1);

        if(status == 0) {

            Cdbg(APP_DBG, "Get session data, write file -> %s, call tc_download", TENCENT_SESSION_FILE);

            // if( access(TC_DOWNLOAD_PID_FILE, 0 ) == 0 ) {
            if(pids("tc_download")) {
                Cdbg(APP_DBG, "tc_download running, SIGTERM to [tc_download]");
                killall("tc_download", SIGTERM);
            } 
            else {
                int sys_code = system("tc_download&");
                Cdbg(APP_DBG, "call tc_download -> sys_code = %d", sys_code);
            }

            sleep(2);
        }

    }

    return 0;
}

static int rcv_newsite_provisioning_data(char *data, char* topic) {
    
    Cdbg(APP_DBG, "Get newsite_provisioning [topic], run account binding(waiting), len = %d, msg = %s", strlen(data), data);

    struct json_object *root = json_tokener_parse(data);
    if (root!=NULL) {
        json_object* joauth_dm_cusid;
        json_object* jdm_userticket;
        json_object* jdm_userrefreshticket;
        json_object* jdm_aae_portal;
        // json_object* jdm_aae_area;
        // json_object* jdm_auth_type;
        // json_object* jdm_user_email;

        json_object_object_get_ex(root, "cusid", &joauth_dm_cusid);
        json_object_object_get_ex(root, "userticket", &jdm_userticket);
        json_object_object_get_ex(root, "userrefreshticket", &jdm_userrefreshticket);
        json_object_object_get_ex(root, "aae_portal", &jdm_aae_portal);
        // json_object_object_get_ex(root, "aae_area", &jdm_aae_area);
        // json_object_object_get_ex(root, "auth_type", &jdm_auth_type);
        // json_object_object_get_ex(root, "user_email", &jdm_user_email);

        const char* cusid = json_object_get_string(joauth_dm_cusid);
        const char* userticket = json_object_get_string(jdm_userticket);
        const char* userrefreshticket = json_object_get_string(jdm_userrefreshticket);
        const char* aae_portal = json_object_get_string(jdm_aae_portal);

        json_object_put(root);

        Cdbg(APP_DBG, "cusid = %s", cusid);
        Cdbg(APP_DBG, "userticket = %s", userticket);
        Cdbg(APP_DBG, "userrefreshticket = %s", userrefreshticket);
        Cdbg(APP_DBG, "aae_portal = %s", aae_portal);
        
        if (strlen(cusid)>0 && strlen(userrefreshticket)>0 && strlen(userticket)>0 && strlen(aae_portal)>0) {
            nvram_set("oauth_dm_cusid", cusid);
            nvram_set("oauth_dm_refresh_ticket", userrefreshticket);
            nvram_set("oauth_dm_user_ticket", userticket);
            nvram_set("aae_portal", aae_portal);
            nvram_set("aae_area", "");
            nvram_set("oauth_type", "");
            nvram_set("oauth_user_email", "");
            nvram_set("oauth_auth_status", "1");
            nvram_set("aws_ca_download_status", "0");
            nvram_set("newsite_provisioning", "0");
            
            nvram_commit();

            notify_rc("restart_mastiff");
        }
        else {
            Cdbg(APP_DBG, "Fail to do newsite provisioning because data is empty!");
        }
    }

    return 0;
}

static int rcv_remoteconnection_data(char *data, char* topic) {
    
    Cdbg(APP_DBG, "Get remote connection [topic], run remote function(waiting), len = %d, msg = %s", strlen(data), data);
    
    int enable_status = parse_receive_remote_connection(data, topic);

    Cdbg(APP_DBG, "parse_receive_remote_connection, enable_status = %d", enable_status);

    if(enable_status == 1) {
	   char event[AAE_MAX_IPC_PACKET_SIZE];
	   snprintf(event, sizeof(event), AAE_AWSIOT_GENERIC_MSG, EID_AWSIOT_TUNNEL_ENABLE);
       Cdbg(APP_DBG, "cm_sendIpcHandler -> ipc > %s -> msg = %s", MASTIFF_IPC_SOCKET_PATH, event);
       cm_sendIpcHandler(MASTIFF_IPC_SOCKET_PATH, event, strlen(event));
    }

    return EXIT_SUCCESS;
}

static int rcv_httpd_data(char *data, char* topic) {

    Cdbg(APP_DBG, "Get httpd api [topic], run httpd api function(waiting), len = %d, msg = %s", strlen(data), data);
    
    char api_name[128];
    char session_id[128];
    char request_data[512];

    int status = parse_receive_httpd(data, request_data, 512, api_name, 128, session_id, 128);

    Cdbg(APP_DBG, "api_name = %s, session_id = %s", api_name, session_id);

    if (status!=0) {
        return 1;
    }

    Cdbg(APP_DBG, "request data ->  %s", request_data);
    
    //- To DO: Got messgae from awsiot, and trigger httpd action by using libwebapi.

    return 0;
}

static int process_general_shadow_delta_data(json_object* jconfig) {
    // Cdbg(APP_DBG, "process_general_shadow_delta_data %s", data);
    
    int ret = -1, wgc_idx = 0, wgsc_idx = 0;
    
    if (jconfig==NULL) {
        return EXIT_FAILURE;
    }
   	
	if(!json_object_is_type(jconfig, json_type_object)) {
        return EXIT_FAILURE;
    }
 
    char config_data[2048] = {0};
    // char reported_config_data[2048] = {0};
    
    json_object_object_foreach(jconfig, featureName, json_config) {
        
        // Cdbg(APP_DBG, "featureName=%s", featureName);

        struct json_object *json_object_config_reported = json_object_new_object();

        int feature_id = get_supported_feature_id(featureName);

        switch (feature_id) {

#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD)
            case FEATURE_WIREGUARD_SERVER: {

                struct array_list* json_config_array_list = json_object_get_array(json_config);
                if (json_config_array_list==NULL) {
                    break;
                }

                int i = 0;
                int array_length = json_config_array_list->length;
                char wgsc_name[64] = {0};
                struct array_list* json_object_result_array = NULL;

                for (i=0; i<array_length; i++) {

                    // Cdbg(APP_DBG, "config %d", i);

                    json_object* sub_json_config = json_object_array_get_idx(json_config, i);

                    struct json_object *wgsc_name_obj = NULL;
                    struct json_object *unique_key_obj = NULL;

                    // char wgsc_name[64] = {0};
                    if(json_object_object_get_ex(sub_json_config, "wgsc_name", &wgsc_name_obj))
                        strlcpy(wgsc_name, (char *)json_object_get_string(wgsc_name_obj), sizeof(wgsc_name));

                    char unique_key[64] = {0};
                    if(json_object_object_get_ex(sub_json_config, "timestamp", &unique_key_obj))
                        strlcpy(unique_key, (char *)json_object_get_string(unique_key_obj), sizeof(unique_key));

                    if (strlen(wgsc_name)<=0 || strlen(unique_key)<=0) {
                        continue;
                    }
                    
                    if (check_wireguard_config_has_been_applied(featureName, wgsc_name, unique_key)) {
                        continue;
                    }

                    if (json_object_result_array==NULL) {
                        json_object_result_array = json_object_new_array();
                        json_object_object_add(json_object_config_reported, wgsc_name, json_object_result_array);
                    }

                    struct json_object *json_object_result = json_object_new_object();

                    ret = set_wireguard_server(sub_json_config, &wgsc_idx);
                    if (ret==HTTP_OK && wgsc_idx>0) {
                        Cdbg(APP_DBG, "Successfully setup wireguard server[%d]", wgsc_idx);
                        
                        notice_to_upload_settings();
                        
                        json_object_object_add(json_object_result, "wireguard_status", json_object_new_string("setup"));
                        json_object_object_add(json_object_result, "index", json_object_new_int(wgsc_idx));
                        json_object_object_add(json_object_result, "error_code", json_object_new_int(0));
                        json_object_object_add(json_object_result, "timestamp", json_object_new_string(unique_key));

                    }
                    else {
                        Cdbg(APP_DBG, "Fail to set wireguard server...ret=[%d]", ret);
                        
                        json_object_object_add(json_object_result, "wireguard_status", json_object_new_string("fail"));
                        json_object_object_add(json_object_result, "index", json_object_new_int(0));
                        json_object_object_add(json_object_result, "error_code", json_object_new_int(ret));
                        json_object_object_add(json_object_result, "timestamp", json_object_new_string(unique_key));
                    }
                    
                    json_object_array_add(json_object_result_array, json_object_result);
                }

                if (strlen(wgsc_name)>0) {
                    update_db_wireguard(wgsc_name, "master", json_object_get_string(json_object_result_array));
                }

                Cdbg(APP_DBG, "result=[%s]", json_object_get_string(json_object_config_reported));

                break;
            }

            case FEATURE_WIREGUARD_CLIENT: {

                struct array_list* json_config_array_list = json_object_get_array(json_config);
                if (json_config_array_list==NULL) {
                    break;
                }
                
                int i = 0;
                int array_length = json_config_array_list->length;
                char wgc_name[64] = {0};
                struct array_list* json_object_result_array = NULL;
                
                for (i=0; i<array_length; i++) {

                    Cdbg(APP_DBG, "config %d", i);

                    json_object* sub_json_config = json_object_array_get_idx(json_config, i);

                    struct json_object *wgc_name_obj = NULL;
                    struct json_object *unique_key_obj = NULL;

                    if(json_object_object_get_ex(sub_json_config, "wgc_name", &wgc_name_obj))
                        strlcpy(wgc_name, (char *)json_object_get_string(wgc_name_obj), sizeof(wgc_name));

                    char unique_key[64] = {0};
                    if(json_object_object_get_ex(sub_json_config, "timestamp", &unique_key_obj))
                        strlcpy(unique_key, (char *)json_object_get_string(unique_key_obj), sizeof(unique_key));

                    if (strlen(wgc_name)<=0 || strlen(unique_key)<=0) {
                        continue;
                    }
                    
                    if (check_wireguard_config_has_been_applied(featureName, wgc_name, unique_key)) {
                        continue;
                    }

                    if (json_object_result_array==NULL) {
                        json_object_result_array = json_object_new_array();
                        json_object_object_add(json_object_config_reported, wgc_name, json_object_result_array);
                    }

                    struct json_object *json_object_result = json_object_new_object();

                    ret = set_wireguard_client(sub_json_config, &wgc_idx);
                    if (ret==HTTP_OK && wgc_idx>0) {
                        Cdbg(APP_DBG, "Successfully setup wireguard client[%d]", wgc_idx);
                        
                        notice_to_upload_settings();

                        json_object_object_add(json_object_result, "wireguard_status", json_object_new_string("setup"));
                        json_object_object_add(json_object_result, "index", json_object_new_int(wgc_idx));
                        json_object_object_add(json_object_result, "error_code", json_object_new_int(0));
                        json_object_object_add(json_object_result, "timestamp", json_object_new_string(unique_key));
                    }
                    else {
                        Cdbg(APP_DBG, "Fail to set wireguard client...ret=[%d]", ret);
                        
                        json_object_object_add(json_object_result, "wireguard_status", json_object_new_string("fail"));
                        json_object_object_add(json_object_result, "index", json_object_new_int(0));
                        json_object_object_add(json_object_result, "error_code", json_object_new_int(ret));
                        json_object_object_add(json_object_result, "timestamp", json_object_new_string(unique_key));
                    }

                    json_object_array_add(json_object_result_array, json_object_result);
                }

                if (strlen(wgc_name)>0) {
                    update_db_wireguard(wgc_name, "slave", json_object_get_string(json_object_result_array));
                }

                Cdbg(APP_DBG, "result=[%s]", json_object_get_string(json_object_config_reported));

                break;
            }
#endif

            case FEATURE_CHANGE_IP: {
                //- To do
                break;
            }

            default: {
                //- Not supported
                break;
            }
        }

        //- clear the desired config data
        if (strlen(config_data)>0) strncat(config_data, ",", 1);
        strncat(config_data, "\"", 2);
        strncat(config_data, featureName, strlen(featureName));
        strncat(config_data, "\"", 2);
        strncat(config_data, ":", 1);
        strncat(config_data, "null", 4);

        //- e.g.
        // "wireguard_server/wireguard_client": {
        //     "[group_name]": [
        //         {
        //             "wireguard_status": "[status]",
        //             "index": "[index]",
        //             "error_code": "[error_code]",
        //             "timestamp": "[timestamp]"
        //         },
        //         ...        
        //     ]
        // }
        // const char* json_reported = json_object_get_string(json_object_config_reported);
        // if (strlen(reported_config_data)>0) strncat(reported_config_data, ",", 1);
        // strncat(reported_config_data, "\"", 2);
        // strncat(reported_config_data, featureName, strlen(featureName));
        // strncat(reported_config_data, "\"", 2);
        // strncat(reported_config_data, ":", 1);
        // strncat(reported_config_data, json_reported, strlen(json_reported));

        json_object_put(json_object_config_reported);
    }
    
    if (strlen(config_data)>0) {

        //- Clear config and update shadow.

        char general_topic[256] = {0};
        snprintf(general_topic, sizeof(general_topic), "$aws/things/%s/shadow/name/general/update", awsiot_clientid);
        
        char reported_data[512] = {0};
        snprintf(reported_data, sizeof(reported_data), "{\"state\":{\"desired\":{\"config\":{%s}}}}", config_data);
        Cdbg(APP_DBG, "reported_data -> %s", reported_data);

        int returnStatus = publishToTopic(&mqttContext_g, general_topic, reported_data);
        if (returnStatus == SUCCESS) {
            Cdbg(APP_DBG, "OK");
            
            get_db_reported_data();
        } 
        else {
            Cdbg(APP_DBG, "FAIL");
        }
    }
    
    // json_object_put(root);
    
    return EXIT_SUCCESS;
}

#ifdef RTCONFIG_MULTISITEAPP
static int rcv_general_shadow_delta_data(char *data, char* topic) {

    Cdbg(APP_DBG, "rcv_general_shadow_delta_data %s", data);

    struct json_object *root = json_tokener_parse(data);
    if (root==NULL) {
        return EXIT_FAILURE;
    }
    
    json_object* jstate = NULL;
    json_object* jconfig = NULL;

    json_object_object_get_ex(root, "state", &jstate);
    if (jstate==NULL) {
        json_object_put(root);
        return EXIT_FAILURE;
    }

    json_object_object_get_ex(jstate, "config", &jconfig);
    if (jconfig==NULL) {
        json_object_put(root);
        return EXIT_FAILURE;
    }

    int res = process_general_shadow_delta_data(jconfig);

    json_object_put(root);

    return res;
}

static int rcv_general_shadow_get_accepted_data(char *data, char* topic) {
    Cdbg(APP_DBG, "rcv_general_shadow_get_accepted_data %s", data);

    struct json_object *root = json_tokener_parse(data);
    if (root==NULL) {
        return EXIT_FAILURE;
    }
    
    json_object* jstate = NULL;
    json_object* jdelta = NULL;
    json_object* jconfig = NULL;

    json_object_object_get_ex(root, "state", &jstate);
    if (jstate==NULL) {
        json_object_put(root);
        return EXIT_FAILURE;
    }

    json_object_object_get_ex(jstate, "delta", &jdelta);
    if (jdelta==NULL) {
        json_object_put(root);
        return EXIT_FAILURE;
    }

    json_object_object_get_ex(jdelta, "config", &jconfig);
    if (jconfig==NULL) {
        json_object_put(root);
        return EXIT_FAILURE;
    }

    int res = process_general_shadow_delta_data(jconfig);

    json_object_put(root);
    
    return res;
}

static int rcv_general_data(char *data, char* topic) {

    Cdbg(APP_DBG, "rcv_general_data %s", data);

    struct json_object *root = json_tokener_parse(data);
    if (root==NULL) {
        return EXIT_FAILURE;
    }
	
	if(!json_object_is_type(root, json_type_object)) {
        return EXIT_FAILURE;
    }

    json_object_object_foreach(root, featureName, function_data_obj) {

        int feature_id = get_supported_feature_id(featureName);
        
        switch (feature_id) {

#if defined(RTCONFIG_VPN_FUSION) || defined(RTCONFIG_WIREGUARD)
            case FEATURE_WIREGUARD_SERVER: {
                char wireguard_action[10] = {0};
                char wireguard_name[64] = {0};
                char wireguard_index[5] = {0};
                int wgsc_idx = -1;
                int ret = HTTP_FAIL;
                struct json_object *param_obj = NULL;
                
                if(json_object_object_get_ex(function_data_obj, "action", &param_obj)){
                    strlcpy(wireguard_action, (char *)json_object_get_string(param_obj), sizeof(wireguard_action));
                }

                if(json_object_object_get_ex(function_data_obj, "name", &param_obj)){
                    strlcpy(wireguard_name, (char *)json_object_get_string(param_obj), sizeof(wireguard_name));
                }

                if(json_object_object_get_ex(function_data_obj, "index", &param_obj)){
                    strlcpy(wireguard_index, (char *)json_object_get_string(param_obj), sizeof(wireguard_index));
                    wgsc_idx = atoi(wireguard_index);
                }

                if (wgsc_idx>=0 && strncmp(wireguard_action, "delete", 6)==0) {
                    
                    ret = del_wgsc_list(1, wgsc_idx);
                    if (ret==HTTP_OK) {
                        Cdbg(APP_DBG, "Success to delete");
                    }
                    else {
                        Cdbg(APP_DBG, "Fail to delete");
                    }
                }

                break;
            }
            
            case FEATURE_WIREGUARD_CLIENT: {

                char wireguard_action[10] = {0};
                char wireguard_name[64] = {0};
                char wireguard_index[5] = {0};
                int wgc_idx = -1;
                int ret = HTTP_FAIL;
                struct json_object *param_obj = NULL;
                
                if(json_object_object_get_ex(function_data_obj, "action", &param_obj)){
                    strlcpy(wireguard_action, (char *)json_object_get_string(param_obj), sizeof(wireguard_action));
                }

                if(json_object_object_get_ex(function_data_obj, "name", &param_obj)){
                    strlcpy(wireguard_name, (char *)json_object_get_string(param_obj), sizeof(wireguard_name));
                }

                if(json_object_object_get_ex(function_data_obj, "index", &param_obj)){
                    strlcpy(wireguard_index, (char *)json_object_get_string(param_obj), sizeof(wireguard_index));
                    wgc_idx = atoi(wireguard_index);
                }

                if (wgc_idx>=0 && strncmp(wireguard_action, "start", 5)==0) {
                    ret = enable_wireguard_client(wgc_idx, "1");
                    if (ret==HTTP_OK) {
                        Cdbg(APP_DBG, "Success to start");
                    }
                    else {
                        Cdbg(APP_DBG, "Fail to start");
                    }
                }
                else if (wgc_idx>=0 && strncmp(wireguard_action, "pause", 5)==0) {
                    
                    ret = enable_wireguard_client(wgc_idx, "0");
                    if (ret==HTTP_OK) {
                        Cdbg(APP_DBG, "Success to pause");
                    }
                    else {
                        Cdbg(APP_DBG, "Fail to pause");
                    }
                }
                else if (wgc_idx>=0 && strncmp(wireguard_action, "delete", 6)==0) {
                    
                    ret = delete_wireguard_client(wgc_idx);
                    if (ret==HTTP_OK) {
                        Cdbg(APP_DBG, "Success to delete");
                    }
                    else {
                        Cdbg(APP_DBG, "Fail to delete");
                    }
                }
                else {
                    break;
                }

                break;
            }
#endif

#if defined(RTCONFIG_TUNNEL)
            case FEATURE_TUNNEL_TEST: {

                char target_device_id[33] = {0};
                struct json_object *param_obj = NULL;

                if(json_object_object_get_ex(function_data_obj, "target", &param_obj)){
                    strlcpy(target_device_id, (char *)json_object_get_string(param_obj), sizeof(target_device_id));
                }

                if (strlen(target_device_id)!=32) {
                    Cdbg(APP_DBG, "Invalid device id");
                    break;
                }
                
                char event[AAE_MAX_IPC_PACKET_SIZE];
                char out[AAE_MAX_IPC_PACKET_SIZE];
                snprintf(event, sizeof(event), AAE_AWSIOT_TNL_TEST_MSG, EID_AWSIOT_TUNNEL_TEST, target_device_id);
                aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);

                break;
            }
#endif

            default:
                break;
        }
    }

    json_object_put(root);

    return EXIT_SUCCESS;
}
#endif // End RTCONFIG_MULTISITEAPP
/*-----------------------------------------------------------*/

void init_basic_data() {

    int ready_count = 0;

    while(1) {

        ready_count++;

        int ntp_ready = nvram_get_int("ntp_ready");

        int link_internet = nvram_get_int("link_internet"); // 2 -> connected

        if((ntp_ready == 1) && (link_internet == 2)) {
            Cdbg(APP_DBG, "waiting ntp_ready -> %d, link_internet -> %d", ntp_ready, link_internet);
            break;
        } 
        else {
            if(ready_count < 3) {
                // Cdbg(APP_DBG, "waiting link_internet -> %d, tencent_download_enable -> %d", link_internet, tencent_download_enable);
            }
        }
        sleep(30);
    }

    ready_count = 0;

    char device_model[32];

    while(1) {

        ready_count++;

        snprintf(aae_deviceid, sizeof(aae_deviceid), "%s", nvram_safe_get("aae_deviceid"));
        snprintf(device_model, sizeof(device_model), "%s", nvram_safe_get("odmpid"));

        if((strlen(device_model) < 1)) {
            snprintf(device_model, sizeof(device_model), "%s", nvram_safe_get("productid"));
        }

        if( (strlen(aae_deviceid) > 1) && (strlen(device_model) > 1) ) {
            Cdbg(APP_DBG, "Get aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model);
            break;
        } 
        else {
            if(ready_count < 2) {
                Cdbg(APP_DBG, "error data, aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model);
            }
        }
        sleep(30);
    }


    ready_count = 0;

    while(1) {

        ready_count++;

        snprintf(awsiot_endpoint, sizeof(awsiot_endpoint), "%s", nvram_safe_get("awsiotendpoint"));
        snprintf(awsiot_clientid, sizeof(awsiot_clientid), "%s", nvram_safe_get("awsiotclientid"));

        if( (strlen(awsiot_endpoint) > 2) &&  (strlen(awsiot_clientid) > 2) ) {
            Cdbg(APP_DBG, "Get awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid);
            break;
        } 
        else {
            if(ready_count < 2) {
                Cdbg(APP_DBG, "error data, awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid);
            }
        }

        sleep(30);
    }

    getcwd(CurrentWD, sizeof(CurrentWD));
    snprintf(rootCA, CERT_PATH_LEN + 1, "%s/%s", certDirectory, ROOT_CA_CERT_PATH);
    snprintf(clientCRT, CERT_PATH_LEN + 1, "%s/%s", certDirectory, CLIENT_CERT_PATH);
    snprintf(clientPrivateKey, CERT_PATH_LEN + 1, "%s/%s", certDirectory, CLIENT_PRIVATE_KEY_PATH);

    LogInfo( ( "rootCA >> %s\nclientCRT >> %s\nclientPrivateKey >> %s", 
                rootCA, clientCRT, clientPrivateKey) );
}

void init_topic_data() {

    if (is_init_topic_data_ok==1) {
        return;
    }

#if defined(RTCONFIG_TC_GAME_ACC) 
    if(nvram_get_int("tencent_download_enable") == 1) {
        enable_sub_topic(SUB_TOPIC_TENCENTGAME);
        enable_sub_topic(SUB_TOPIC_ALL_TENCENTGAME);

        snprintf(publish_topic, MAX_TOPIC_LENGTH, "asus/%s/TencentGameAccleration/session/report", awsiot_clientid);
    }
#endif

#ifdef RTCONFIG_NEWSITE_PROVISIONING 
    if (nvram_get_int("newsite_provisioning") == 1) {
        enable_sub_topic(SUB_TOPIC_NEWSITE_PROVISIONING);
    }
#endif

#ifdef RTCONFIG_ACCOUNT_BINDING
    if (is_account_bound()) {
        enable_sub_topic(SUB_TOPIC_REMOTECONNECTION);
        enable_sub_topic(SUB_TOPIC_GENERAL_DELTA_SHADOW);
        enable_sub_topic(SUB_TOPIC_GENERAL_GET_ACCEPTED);
        enable_sub_topic(SUB_TOPIC_GENERAL);

        snprintf(shadow_remote_connection_topic, MAX_TOPIC_LENGTH, "$aws/things/%s/shadow/RemoteConnection/update", awsiot_clientid);

        get_db_reported_data();
    }
#endif

    snprintf(default_shadow_topic, MAX_TOPIC_LENGTH, "$aws/things/%s/shadow/update", awsiot_clientid);    

    is_init_topic_data_ok = 1;
}

int get_db_reported_data() {
    
#if defined(RTCONFIG_TUNNEL)
    char event[AAE_MAX_IPC_PACKET_SIZE];
    char out[AAE_MAX_IPC_PACKET_SIZE];
    snprintf(event, sizeof(event), AAE_AWSIOT_GET_SHADOW_MSG, EID_AWSIOT_GET_SHADOW);
    aae_sendIpcMsgAndWaitResp(MASTIFF_IPC_SOCKET_PATH, event, strlen(event), out, sizeof(out), 5);
    
    json_object *root = NULL;
    json_object *awsiotObj = NULL;
    json_object *eidObj = NULL;
    json_object *stsObj = NULL;

    root = json_tokener_parse((char *)out);
    if (root==NULL) {
        Cdbg(APP_DBG, "root is NULL");
        // char* config_data = "{\"wireguard_server\":{\"202302161716\":[{\"wireguard_status\":\"setting\",\"index\":null,\"error_code\":null,\"timestamp\":null}]},\"wireguard_client\":[]}";
        // g_json_object_general_db_reported = json_tokener_parse((char *)config_data);
        return EXIT_SUCCESS;
    }
    
    json_object_object_get_ex(root, AAE_AWSIOT_PREFIX, &awsiotObj);
    json_object_object_get_ex(awsiotObj, AAE_IPC_EVENT_ID, &eidObj);
    json_object_object_get_ex(awsiotObj, AAE_IPC_STATUS, &stsObj);
    if (!awsiotObj || !eidObj || !stsObj) {
        json_object_put(root);
        return EXIT_FAILURE;
    }
    
    int eid = json_object_get_int(eidObj);
    const char *status = json_object_get_string(stsObj);
    
    if ((eid == EID_AWSIOT_GET_SHADOW) && (!strcmp(status, "0"))) {
        json_object *reportedObj = NULL;
        json_object_object_get_ex(awsiotObj, SHADOW_REPORTED, &reportedObj);

        if (!reportedObj) {
            json_object_put(root);
            return EXIT_FAILURE;
        }

        const char *reported_data = json_object_get_string(reportedObj);

        //- clear first
        json_object_put(g_json_object_general_db_reported);
        g_json_object_general_db_reported = NULL;

        if (strlen(reported_data)>0) {

            g_json_object_general_db_reported = json_tokener_parse((char *)reported_data);
            
            if (g_json_object_general_db_reported!=NULL) {
                Cdbg(APP_DBG, "get_db_reported_data: %s", json_object_get_string(g_json_object_general_db_reported));
            }
        }
    }
    
    json_object_put(root);

    return EXIT_SUCCESS;
#else
    return EXIT_FAILURE;
#endif
}

int check_mqtt_port() {

    int ret = -1;
    FILE *fp;
    char cmd[128] = {0};

    strlcpy(cmd, "netstat -na | grep 8883 2>&1" , sizeof(cmd));
    if ((fp = popen(cmd, "r")) != NULL)
    {
        while (fgets(cmd, sizeof(cmd), fp) != NULL)
        {
            if (strlen(cmd) > 5)
            {
                ret = 0;
                break;
            }
            else
                ret = -1;
        }
        pclose(fp);
    }
    return ret;
}

int main( int argc, char ** argv ) {
    
    // Open Debug Log
    CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);
    // CF_OPEN(AWS_DEBUG_TO_FILE, FILE_TYPE );
    // CF_OPEN(AWS_DEBUG_TO_CONSOLE,  CONSOLE_TYPE | SYSLOG_TYPE | FILE_TYPE);

    init_basic_data();

#if defined(RTCONFIG_TC_GAME_ACC) 
    // receive tc_download sig
    signal(AWSIOT_SIG_ACTION, awsiot_sig_handler);
#endif

    // ipc : call [mastiff && aaews]
    awsiot_ipc_start();

    Cdbg(APP_DBG, "awsiot start");

    int returnStatus = EXIT_SUCCESS;
    MQTTContext_t mqttContext = { 0 };
    NetworkContext_t networkContext = { 0 };
    OpensslParams_t opensslParams = { 0 };
    bool clientSessionPresent = false;
    bool mqttSessionEstablished = false;

    // struct timespec tp;

    ( void ) argc;
    ( void ) argv;


    /* Set the pParams member of the network context with desired transport. */
    networkContext.pParams = &opensslParams;

    /* Initialize MQTT library.  */
    returnStatus = initializeMqtt( &mqttContext, &networkContext );

    if( returnStatus == EXIT_SUCCESS ) {
        for( ; ; ) {
            /* Attempt to connect to the MQTT broker. If connection fails, retry after
             * a timeout. Timeout value will be exponentially increased till the maximum
             * attempts are reached or maximum timeout value is reached. The function
             * returns EXIT_FAILURE if the TCP connection cannot be established to
             * broker after configured number of attempts. */
            returnStatus = connectToServerWithBackoffRetries( &networkContext );

            if( returnStatus == EXIT_FAILURE ) {

                /* connection failure after all reconnect attempts are over. */
                Cdbg(APP_DBG, "TCP connection cannot be established, Failed to connect to MQTT broker %s, ntp_ready = %d, link_internet = %d",
                            awsiot_endpoint, nvram_get_int("ntp_ready"), nvram_get_int("link_internet"));

                if(check_mqtt_port() < 0) {
                    check_mqtt_timer++;
                }

                if(check_mqtt_timer > 5) {
                    Cdbg(APP_DBG, "check_mqtt_timer = %d, restart", check_mqtt_timer);
                    break;
                }
            }
            else {

                init_topic_data();

                /* If TLS session is established, execute Subscribe loop. */
                returnStatus = subscribeTopic(&mqttContext, &clientSessionPresent, &mqttSessionEstablished);

                if(returnStatus == MQTTSuccess) {

                    returnStatus = update_default_shadow_data(&mqttContext);
                    if(returnStatus != MQTTSuccess) {
                        continue;
                    }

                    returnStatus = get_general_shadow_data(&mqttContext);
                    if(returnStatus != MQTTSuccess) {
                        continue;
                    }

                    returnStatus = main_loop(&mqttContext, &mqttSessionEstablished);
                    if(returnStatus != MQTTSuccess) {
                        continue;
                    }
                }

            }

            /* End TLS session, then close TCP connection. */
            ( void ) Openssl_Disconnect( &networkContext );

            // if(returnStatus == EXIT_AWSIOT) {
            //     break;
            // }

            Cdbg(APP_DBG, "Openssl disconnect, Short delay before starting the next iteration....");
            sleep( MQTT_SUBPUB_LOOP_DELAY_SECONDS );

        }
    }


    Cdbg(APP_DBG, "exit awsiot, waiting restart");

    return returnStatus;
}
/*-----------------------------------------------------------*/
