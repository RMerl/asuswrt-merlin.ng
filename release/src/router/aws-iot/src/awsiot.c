#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

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
#include "httpd_api.h"


#include "awsiot_ipc.h"
#include "awsiot_mnt.h"


#define APP_DBG 1

static char subscribe_topic_array[SUBSCRIBE_TOPIC_MAX_NUMBER][MAX_TOPIC_LENGTH];

static char shadow_update_topic[256] = {0};
static char publish_topic[256] = {0};

static int subscribe_topic_number = 0;

static char shadow_remote_connection_topic[256] = {0};

static time_t session_receive_time_tmp = 0;


static unsigned char sn_hash[65] = {0};

static int tencentgame_download_enable_tmp = 0;

MQTTContext_t mqttContext_g;

int get_mqtt_session_number = 0;


char shadowRxBufTmp[NETWORK_BUFFER_SIZE] = {0};
char subscribe_topic_tmp[MAX_TOPIC_LENGTH] = {0};

int get_mqtt_dif_session_number = 0;

unsigned int search_tc_file_count = 0;


bool publish_running = false;
int publish_waiting = 0;

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
#define MQTT_PROCESS_LOOP_TIMEOUT_MS        ( 500U )

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
#define MQTT_SUBPUB_LOOP_DELAY_SECONDS      ( 15U )

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
typedef struct PublishPackets
{
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
struct NetworkContext
{
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
static int unsubscribeFromTopic( MQTTContext_t * pMqttContext );

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
static int handleResubscribe( MQTTContext_t * pMqttContext );

/*-----------------------------------------------------------*/

static uint32_t generateRandomNumber()
{
    return( rand() );
}


/*-----------------------------------------------------------*/
static int connectToServerWithBackoffRetries( NetworkContext_t * pNetworkContext )
{
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



    if( AWS_MQTT_PORT == 443 )
    {
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
    do
    {
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

        if( opensslStatus != OPENSSL_SUCCESS )
        {
            /* Generate a random number and get back-off value (in milliseconds) for the next connection retry. */
            backoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &reconnectParams, generateRandomNumber(), &nextRetryBackOff );

            if( backoffAlgStatus == BackoffAlgorithmRetriesExhausted )
            {
                LogError( ( "Connection to the broker failed, all attempts exhausted." ) );
                returnStatus = EXIT_FAILURE;
            }
            else if( backoffAlgStatus == BackoffAlgorithmSuccess )
            {
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

static int getNextFreeIndexForOutgoingPublishes( uint8_t * pIndex )
{
    int returnStatus = EXIT_FAILURE;
    uint8_t index = 0;

    assert( outgoingPublishPackets != NULL );
    assert( pIndex != NULL );

    for( index = 0; index < MAX_OUTGOING_PUBLISHES; index++ )
    {

    LogInfo( ( "getNextFreeIndexForOutgoingPublishes index[%d],  packetId %u.\n\n",
                       index, outgoingPublishPackets[ index ].packetId ) );

        /* A free index is marked by invalid packet id.
         * Check if the the index has a free slot. */
        if( outgoingPublishPackets[ index ].packetId == MQTT_PACKET_ID_INVALID )
        {
            returnStatus = EXIT_SUCCESS;
            break;
        }
    }

    /* Copy the available index into the output param. */
    *pIndex = index;

    return returnStatus;
}
/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishAt( uint8_t index )
{
    assert( outgoingPublishPackets != NULL );
    assert( index < MAX_OUTGOING_PUBLISHES );

    /* Clear the outgoing publish packet. */
    ( void ) memset( &( outgoingPublishPackets[ index ] ),
                     0x00,
                     sizeof( outgoingPublishPackets[ index ] ) );
}

/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishes( void )
{
    assert( outgoingPublishPackets != NULL );

    /* Clean up all the outgoing publish packets. */
    ( void ) memset( outgoingPublishPackets, 0x00, sizeof( outgoingPublishPackets ) );
}

/*-----------------------------------------------------------*/

static void cleanupOutgoingPublishWithPacketID( uint16_t packetId )
{
    uint8_t index = 0;

    assert( outgoingPublishPackets != NULL );
    assert( packetId != MQTT_PACKET_ID_INVALID );

    /* Clean up all the saved outgoing publishes. */
    for( ; index < MAX_OUTGOING_PUBLISHES; index++ )
    {
        if( outgoingPublishPackets[ index ].packetId == packetId )
        {
            cleanupOutgoingPublishAt( index );
            LogInfo( ( "Cleaned up outgoing publish packet with [index = %d], packet id %u.\n\n",
                       index, packetId ) );
            break;
        }
    }
}

/*-----------------------------------------------------------*/

static int handlePublishResend( MQTTContext_t * pMqttContext )
{
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

    while( packetIdToResend != MQTT_PACKET_ID_INVALID )
    {
        foundPacketId = false;

        for( index = 0U; index < MAX_OUTGOING_PUBLISHES; index++ )
        {
            if( outgoingPublishPackets[ index ].packetId == packetIdToResend )
            {
                foundPacketId = true;
                outgoingPublishPackets[ index ].pubInfo.dup = true;

                LogInfo( ( "Sending duplicate PUBLISH with packet id %u.",
                           outgoingPublishPackets[ index ].packetId ) );
                mqttStatus = MQTT_Publish( pMqttContext,
                                           &outgoingPublishPackets[ index ].pubInfo,
                                           outgoingPublishPackets[ index ].packetId );

                if( mqttStatus != MQTTSuccess )
                {
                    LogError( ( "Sending duplicate PUBLISH for packet id %u "
                                " failed with status %s.",
                                outgoingPublishPackets[ index ].packetId,
                                MQTT_Status_strerror( mqttStatus ) ) );
                    returnStatus = EXIT_FAILURE;
                    break;
                }
                else
                {
                    LogInfo( ( "Sent duplicate PUBLISH successfully for packet id %u.\n\n",
                               outgoingPublishPackets[ index ].packetId ) );
                }
            }
        }

        if( foundPacketId == false )
        {
            LogError( ( "Packet id %u requires resend, but was not found in "
                        "outgoingPublishPackets.",
                        packetIdToResend ) );
            returnStatus = EXIT_FAILURE;
            break;
        }
        else
        {
            /* Get the next packetID to be resent. */
            packetIdToResend = MQTT_PublishToResend( pMqttContext, &cursor );
        }
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static void handleIncomingPublish( MQTTPublishInfo_t * pPublishInfo,
                                   uint16_t packetIdentifier )
{
    assert( pPublishInfo != NULL );
    assert( pPublishInfo->pPayload != NULL );

    get_mqtt_session_number++;

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


    // JSONStatus_t result = JSONSuccess;


    // /* Make sure the payload is a valid json document. */
    // result = JSON_Validate( pPublishInfo->pPayload,
    //                         pPublishInfo->payloadLength );

    // if( result != JSONSuccess )
    // {
    //     LogError( ( "Incoming data: The json document is invalid!!" ) );
    //     Cdbg(APP_DBG, "Received JSON is not valid");
    //     return;
    // }


    // uint32_t version = 0U;
    // uint32_t newState = 0U;
    // char * sessionValue = NULL;
    // uint32_t sessionValueLength = 0U;

    // if( result == JSONSuccess )
    // {
    //     /* Then we start to get the version value by JSON keyword "version". */
    //     result = JSON_Search( ( char * ) pPublishInfo->pPayload,
    //                           pPublishInfo->payloadLength,
    //                           "session",
    //                           sizeof( "session" ) - 1,
    //                           &sessionValue,
    //                           ( size_t * ) &sessionValueLength );
    // }
    // else
    // {
    //     LogError( ( "The json document is invalid!!" ) );
    // }




    // if( result == JSONSuccess )
    // {
    //     LogInfo( ( "session: %.*s", sessionValueLength, sessionValue ) );

    // }
    // else
    // {
    //     LogError( ( "No session in json document!!" ) );
    // }


    // LogInfo( ( "session: %d, %s", sessionValueLength, sessionValue ) );


    time_t session_receive_time = time(NULL);

    LogInfo( ( "session_receive_time : %ld", session_receive_time) );
    LogInfo( ( "session_receive_time_tmp : %ld", session_receive_time_tmp) );


    if(strstr(subscribe_topic, SHADOW_NAME_REMOTE_CONNECTION)) {
        Cdbg(APP_DBG, "Get remote connection [topic], run remote function(waiting), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf);
        LogInfo( ( "Get remote connection [topic],  run remote function(waitin), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf) );
        return;
    }

    if( strstr(subscribe_topic, "asus") && strstr(subscribe_topic, "RemoteConnection") ) {

        asus_remote_connection(shadowRxBuf, subscribe_topic, session_receive_time);

        return;

    } else if( strstr(subscribe_topic, "asus") && strstr(subscribe_topic, "httpd") 
               && !strstr(subscribe_topic, "update")  ) {

        asus_httpd(shadowRxBuf, subscribe_topic, session_receive_time);

        return;

    } else if( strstr(subscribe_topic, "asus") && strstr(subscribe_topic, "httpd") 
               && strstr(subscribe_topic, "update")  ) {

        Cdbg(APP_DBG, "Get test session 123456789 msg, len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf);
        LogInfo( ( "Get remote connectioGet test session 123456789 msg, len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf) );

        return;

    } else {

        // reset time 
        int reset_status = reset_session_receive_time(shadowRxBuf, subscribe_topic,  
                                              session_receive_time, TOPIC_TENCENTGAME);

        if(reset_status != 0) {
            return;
        }

        get_mqtt_dif_session_number++;
        LogInfo( ( "get_mqtt_dif_session_number -> %d\n" , get_mqtt_dif_session_number) );

        tencentgame_data_process(shadowRxBuf, subscribe_topic);

    }

}

/*-----------------------------------------------------------*/

static void updateSubAckStatus( MQTTPacketInfo_t * pPacketInfo )
{
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
                                     subscribe_topic_number,
                                     globalSubscribePacketIdentifier );

        // mqttStatus = MQTT_Subscribe( pMqttContext,
        //                              pGlobalSubscriptionList,
        //                              sizeof( pGlobalSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
        //                              globalSubscribePacketIdentifier );



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

/*-----------------------------------------------------------*/

static void eventCallback( MQTTContext_t * pMqttContext,
                           MQTTPacketInfo_t * pPacketInfo,
                           MQTTDeserializedInfo_t * pDeserializedInfo )
{
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
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
    {
        assert( pDeserializedInfo->pPublishInfo != NULL );
        /* Handle incoming publish. */
        handleIncomingPublish( pDeserializedInfo->pPublishInfo, packetIdentifier );
    }
    else
    {
        /* Handle other packets. */
        switch( pPacketInfo->type )
        {
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
                                 bool * pSessionPresent )
{
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

    if( mqttStatus != MQTTSuccess )
    {
        returnStatus = EXIT_FAILURE;
        LogError( ( "Connection with MQTT broker failed with status %s.",
                    MQTT_Status_strerror( mqttStatus ) ) );
        Cdbg(APP_DBG, "Connection with MQTT broker failed with status %s.",
                    MQTT_Status_strerror( mqttStatus ) );
    }
    else
    {
        LogInfo( ( "MQTT connection successfully established with broker.\n\n" ) );
        Cdbg(APP_DBG, "MQTT connection successfully established with broker");
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int disconnectMqttSession( MQTTContext_t * pMqttContext )
{
    MQTTStatus_t mqttStatus = MQTTSuccess;
    int returnStatus = EXIT_SUCCESS;

    assert( pMqttContext != NULL );

    /* Send DISCONNECT. */
    mqttStatus = MQTT_Disconnect( pMqttContext );

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Sending MQTT DISCONNECT failed with status=%s.",
                    MQTT_Status_strerror( mqttStatus ) ) );
        returnStatus = EXIT_FAILURE;
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

void tencentgame_data_process(const char *shadowRxBuf, const char *subscribe_topic)
{

    LogInfo( ( "subscribe_topic : %s", subscribe_topic) );
    // LogInfo( ( "shadowRxBuf : %s", shadowRxBuf) );

    char tmpRecive[NETWORK_BUFFER_SIZE] = {0};

    if(get_session_data(shadowRxBuf, tmpRecive) != 0) {
        LogError( ( "Incoming data: [session] The json document is invalid!!" ) );
        Cdbg(APP_DBG, "[session] Received JSON is not valid");
        return;
    }
    
    LogInfo( ( "tmpRecive : %s", tmpRecive) );
    // snprintf(tmpRecive, NETWORK_BUFFER_SIZE, "%s", shadowRxBuf);

    // it is not empty string
    if ((tmpRecive != NULL) && (strlen(tmpRecive) > 0) ) {

        int status = split_received_tencent_session(subscribe_topic);

        status = save_received_session(tmpRecive, subscribe_topic);

        status = merge_received_session(subscribe_topic_number);

        status = compare_received_session(subscribe_topic_number, 1);

        if(status == 0) {

            LogInfo( ( "Get session data, write file -> %s, call tc_download", TENCENT_SESSION_FILE) );
            Cdbg(APP_DBG, "Get session data, write file -> %s, call tc_download", TENCENT_SESSION_FILE);

            // if( access(TC_DOWNLOAD_PID_FILE, 0 ) == 0 ) {
            if(pids("tc_download")) {
                LogInfo( ( "tc_download running, SIGTERM to [tc_download]\n") );
                Cdbg(APP_DBG, "tc_download running, SIGTERM to [tc_download]");
                killall("tc_download", SIGTERM);

            } else {
                int sys_code = system("tc_download&");
                Cdbg(APP_DBG, "call tc_download -> sys_code = %d", sys_code);
                LogInfo( ( "session_data_process Call tc_download -> sys_code = %d", sys_code) );
            }

            sleep(2);
        }

    }

}

void asus_remote_connection(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time) {

    int reset_status = reset_session_receive_time(shadowRxBuf, subscribe_topic,  
                                          session_receive_time, TOPIC_REMOTECONNECTION);

    if(reset_status != 0) {

        if(pids("aaews")) {
            LogInfo( ( "run_tc_download -> tc_download running") );
            publish_shadow_remote_connection(1, shadowRxBuf);
        }

        return;
    }

    Cdbg(APP_DBG, "Get remote connection [topic], run remote function(waiting), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf);
    LogInfo( ( "Get remote connection [topic],  run remote function(waiting), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf) );

    int enable_status = parse_receive_remote_connection(shadowRxBuf, subscribe_topic);

    Cdbg(APP_DBG, "parse_receive_remote_connection, enable_status = %d", enable_status);

    if(enable_status == 1) {
        Cdbg(APP_DBG, "cm_sendIpcHandler -> ipc > %s -> msg = %s", MASTIFF_IPC_SOCKET_PATH, shadowRxBuf);
        cm_sendIpcHandler(MASTIFF_IPC_SOCKET_PATH, shadowRxBuf, strlen(shadowRxBuf));
    }


    return;
}



void asus_httpd(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time) {

    int reset_status = reset_session_receive_time(shadowRxBuf, subscribe_topic,  
                                          session_receive_time, TOPIC_HTTPD);

   // reset status == 0 --> get new msg 
    if(reset_status != 0) {

        // if(pids("aaews")) {
            LogInfo( ( "asus_httpd -> publish have old msg") );
            publish_shadow_httpd(shadowRxBuf, API_GET_EPTOKEN, "session_id");
        // }

        return;
    }

    Cdbg(APP_DBG, "Get httpd api [topic], run httpd api function(waiting), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf);
    LogInfo( ( "Get httpd api [topic],  run httpd api function(waiting), len = %d, msg = %s", strlen(shadowRxBuf), shadowRxBuf) );


    char api_name[128];
    char session_id[128];
    char request_data[512];

    int status = parse_receive_httpd(shadowRxBuf, request_data, 512, api_name, 128, session_id, 128);


    LogInfo( ( "api_name = %s", api_name) );
    LogInfo( ( "session_id = %s", session_id) );

    if(status == 0) {

        Cdbg(APP_DBG, "request data ->  %s", request_data);
        // cm_sendIpcHandler(MASTIFF_IPC_SOCKET_PATH, shadowRxBuf, strlen(shadowRxBuf));
        status = httpd_api(API_GET_EPTOKEN, request_data);

        if(status == 0) {

            LogInfo( ( "httpd_api ok" ) );

            publish_shadow_httpd(request_data, api_name, session_id);


        } else {
            LogInfo( ( "httpd_api error, status = %d", status ) );
            Cdbg(APP_DBG, "httpd_api error, status = %d", status);
        }
    }

    return;
}


int reset_session_receive_time(const char *shadowRxBuf, const char *subscribe_topic,  const long int session_receive_time, const int topic_target) 
{

    long int interval_time = 0;
    long int data_reset_time = 0;

    if((strcmp(shadowRxBuf, shadowRxBufTmp) == 0) &&
       (strcmp(subscribe_topic, subscribe_topic_tmp) == 0)   ) {

        if(topic_target == TOPIC_TENCENTGAME) {
            if(TENCENTGAME_RESET_TIME > 0) {
                data_reset_time = TENCENTGAME_RESET_TIME;
            }
        } else if(topic_target == TOPIC_REMOTECONNECTION) {
            if(REMOTECONNECTION_RESET_TIME > 0) {
                data_reset_time = REMOTECONNECTION_RESET_TIME;
            }
        } else if(topic_target == TOPIC_HTTPD) {
            if(HTTPD_RESET_TIME > 0) {
                data_reset_time = HTTPD_RESET_TIME;
            }
        }

        interval_time = session_receive_time - session_receive_time_tmp;
    
        if( (data_reset_time > 0) && 
            (interval_time > data_reset_time)    ) {
            LogInfo( ("data_reset_time = %d, over-time -> %ld, reset msg", data_reset_time, interval_time) );
            Cdbg(APP_DBG, "data_reset_time = %d, over-time -> %ld, reset msg", data_reset_time, interval_time);
            goto reset_success;
        } 

        Cdbg(APP_DBG, "mqtt_message repeat, pass msg, topic > %s", subscribe_topic);
        LogInfo( ("mqtt_message repeat, pass msg, topic > %s", subscribe_topic) );

        return -1;

    } 

reset_success:

    Cdbg(APP_DBG, "Get new mqtt msg");
    LogInfo( ("Get new mqtt msg") );
    snprintf(shadowRxBufTmp, NETWORK_BUFFER_SIZE, "%s", shadowRxBuf);
    snprintf(subscribe_topic_tmp, MAX_TOPIC_LENGTH, "%s", subscribe_topic);
    session_receive_time_tmp = session_receive_time;

    return 0;
}

/*-----------------------------------------------------------*/


void publish_shadow_remote_connection(const int tunnel_enable, const char *state) 
{
    char publish_data[512] = {0};
    snprintf(publish_data, 512, "{ \"state\": %s}",  state);

    LogInfo( ("publish_shadow_remote_connection, publish topic -> %s (%d), publish_data = %s (%d)",  shadow_remote_connection_topic, strlen(shadow_remote_connection_topic), publish_data, strlen(publish_data)) );

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, shadow_remote_connection_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "publish_shadow_remote_connection  OK -> %s", shadow_remote_connection_topic) );
        Cdbg(APP_DBG, "publish_shadow_remote_connection  OK -> %s", shadow_remote_connection_topic);
    } else {
        LogInfo( ( "publish_shadow_remote_connection topic -> %s", shadow_remote_connection_topic) );
        Cdbg(APP_DBG, "publish_shadow_remote_connection , topic -> %s", shadow_remote_connection_topic);
    }
}



//  publish topic : asus / %awsiot_clientid% / {xxx} /update
void publish_router_service_topic(const char *topic, const char *msg) 
{
    char publish_data[512] = {0};
    char service_topic[256] = {0};
 

    snprintf(publish_data, 512, "%s",  msg);
    snprintf(service_topic, 256, "asus/%s/%s/update", awsiot_clientid, topic);


    LogInfo( ("publish_router_service_topic, publish topic -> %s (%d), publish_data = %s (%d)",  service_topic, strlen(service_topic), publish_data, strlen(publish_data)) );

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, service_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "%s OK -> %s,  publish_data -> %s", __FUNCTION__,  service_topic, publish_data) );
        Cdbg(APP_DBG, "%s OK -> %s,  publish_data -> %s", __FUNCTION__, service_topic, publish_data);
    } else {
        LogInfo( ( "%s error -> %s,  publish_data -> %s", __FUNCTION__, service_topic, publish_data) );
        Cdbg(APP_DBG, "%s error -> %s,  publish_data -> %s", __FUNCTION__, service_topic, publish_data);
    }
}


void publish_shadow_httpd(const char *input, const char *api_name, const char *session_id) 
{
    char publish_data[512] = {0};
    char httpd_topic[256] = {0};


    char file_data[512];


    read_file_data(HTTPD_RESPONSE_FILE, file_data, 512); 

    // snprintf(publish_data, 512, "{ \"state\": %s}",  state);
    snprintf(publish_data, 512, "%s",  file_data);
    snprintf(httpd_topic, 256, "asus/%s/httpd/%s/update", awsiot_clientid, session_id);
    // snprintf(httpd_topic, 256, "$aws/things/%s/shadow/httpd/%s/update", awsiot_clientid, api_name);

    


    LogInfo( ("publish_shadow_httpd, publish topic -> %s (%d), publish_data = %s (%d)",  httpd_topic, strlen(httpd_topic), publish_data, strlen(publish_data)) );

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, httpd_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "publish_shadow_httpd  OK -> %s,  publish_data -> %s", httpd_topic, publish_data) );
        Cdbg(APP_DBG, "publish_shadow_httpd  OK -> %s,  publish_data -> %s", httpd_topic, publish_data);
    } else {
        LogInfo( ( "publish_shadow_httpd  error -> %s,  publish_data -> %s", httpd_topic, publish_data) );
        Cdbg(APP_DBG, "publish_shadow_httpd  error -> %s,  publish_data -> %s", httpd_topic, publish_data);
    }
}



/*-----------------------------------------------------------*/

static int subscribeToTopic( MQTTContext_t * pMqttContext )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;

    assert( pMqttContext != NULL );

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pGlobalSubscriptionList, 0x00, sizeof( pGlobalSubscriptionList ) );

    /* subscribes topic and uses QOS1. */
    int i = 0;
    for(i = 0; i < subscribe_topic_number; i++) {

        pGlobalSubscriptionList[ i ].qos = MQTTQoS1;
        pGlobalSubscriptionList[ i ].pTopicFilter = subscribe_topic_array[i];
        pGlobalSubscriptionList[ i ].topicFilterLength = strlen(subscribe_topic_array[i]);


        LogInfo( ( "Start subscribe topic = %s , len = %d\n\n",
                   pGlobalSubscriptionList[ i ].pTopicFilter,
                   pGlobalSubscriptionList[ i ].topicFilterLength ) );

        Cdbg(APP_DBG, "Start subscribe topic = %s , len = %d",
                   pGlobalSubscriptionList[ i ].pTopicFilter,
                   pGlobalSubscriptionList[ i ].topicFilterLength);
    }


        // LogInfo( ( "pGlobalSubscriptionList = %d,  MQTTSubscribeInfo_t = %d,  %d, %d\n",
        //            sizeof( pGlobalSubscriptionList ),
        //            sizeof( MQTTSubscribeInfo_t ),
        //            sizeof( pGlobalSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
        //            subscribe_topic_number ) );


    /* Generate packet identifier for the SUBSCRIBE packet. */
    globalSubscribePacketIdentifier = MQTT_GetPacketId( pMqttContext );

    /* Send SUBSCRIBE packet. */
    // mqttStatus = MQTT_Subscribe( pMqttContext,
    //                              pGlobalSubscriptionList,
    //                              sizeof( pGlobalSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
    //                              globalSubscribePacketIdentifier );

    mqttStatus = MQTT_Subscribe( pMqttContext,
                                 pGlobalSubscriptionList,
                                 subscribe_topic_number,
                                 globalSubscribePacketIdentifier );

    if( mqttStatus != MQTTSuccess )
    {
        LogError( ( "Failed to send SUBSCRIBE packet to broker with error = %s.",
                    MQTT_Status_strerror( mqttStatus ) ) );
        returnStatus = EXIT_FAILURE;

        Cdbg(APP_DBG, "Failed to send SUBSCRIBE packet to broker with error = %s.",
                    MQTT_Status_strerror( mqttStatus ));
    }
    else
    {
        LogInfo( ( "SUBSCRIBE sent for topic , Success" ) );
        Cdbg(APP_DBG, "SUBSCRIBE sent for topic , Success");
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int unsubscribeFromTopic( MQTTContext_t * pMqttContext )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus;

    assert( pMqttContext != NULL );

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pGlobalSubscriptionList, 0x00, sizeof( pGlobalSubscriptionList ) );

    /* subscribes topic and uses QOS1. */
    int i = 0;
    for(i = 0; i < subscribe_topic_number; i++) {

        pGlobalSubscriptionList[ i ].qos = MQTTQoS1;
        pGlobalSubscriptionList[ i ].pTopicFilter = subscribe_topic_array[i];
        pGlobalSubscriptionList[ i ].topicFilterLength = strlen(subscribe_topic_array[i]);


        LogInfo( ( "UNSUBSCRIBE topic = %s , len = %d\n\n",
                   pGlobalSubscriptionList[ i ].pTopicFilter,
                   pGlobalSubscriptionList[ i ].topicFilterLength ) );
    }

    /* Generate packet identifier for the UNSUBSCRIBE packet. */
    globalUnsubscribePacketIdentifier = MQTT_GetPacketId( pMqttContext );

    /* Send UNSUBSCRIBE packet. */
    mqttStatus = MQTT_Unsubscribe( pMqttContext,
                                   pGlobalSubscriptionList,
                                   subscribe_topic_number,
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

/*-----------------------------------------------------------*/



static int publishToTopic( MQTTContext_t * pMqttContext, char * pTopicName, char * pPayload )
{
    int returnStatus = EXIT_SUCCESS;
    MQTTStatus_t mqttStatus = MQTTSuccess;
    uint8_t publishIndex = MAX_OUTGOING_PUBLISHES;

    assert( pMqttContext != NULL );

    /* Get the next free index for the outgoing publish. All QoS1 outgoing
     * publishes are stored until a PUBACK is received. These messages are
     * stored for supporting a resend if a network connection is broken before
     * receiving a PUBACK. */
    returnStatus = getNextFreeIndexForOutgoingPublishes( &publishIndex );

    if( returnStatus == EXIT_FAILURE )
    {
        LogError( ( "Unable to find a free spot for outgoing PUBLISH message.\n\n" ) );
    }
    else
    {
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

        if( mqttStatus != MQTTSuccess )
        {
            LogError( ( "Topic = %s (len=%d), Failed to send PUBLISH packet to broker with error = %s.",
                        pTopicName, strlen(pTopicName), MQTT_Status_strerror( mqttStatus ) ) );

            Cdbg(APP_DBG, "Topic = %s (len=%d), Failed to send PUBLISH packet to broker with error = %s.",
                        pTopicName, strlen(pTopicName), MQTT_Status_strerror( mqttStatus )  );


            cleanupOutgoingPublishAt( publishIndex );
            returnStatus = EXIT_FAILURE;
        }
        else
        {

            LogInfo( ( "PUBLISH sent publishIndex[%d], topic [%s] (len=%d) to broker with packet ID %u.",
                       publishIndex, pTopicName, strlen(pTopicName),
                       outgoingPublishPackets[ publishIndex ].packetId ) );

            // Cdbg(APP_DBG, "PUBLISH sent publishIndex[%d], topic [%s] (len=%d) to broker with packet ID %u.",
            //            publishIndex, pTopicName, strlen(pTopicName),
            //            outgoingPublishPackets[ publishIndex ].packetId  );

		    mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

		    if( mqttStatus == MQTTSuccess )
		    {

	            LogInfo( ( "mqttStatus == == == MQTTSuccess" ) );

		    } else {
	            LogInfo( ( "mqttStatus != != != MQTTSuccess" ) );
		    }

            LogInfo( ( "Cleaning up all the stored outgoing publishes." ) );

            cleanupOutgoingPublishes();
        }
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int initializeMqtt( MQTTContext_t * pMqttContext,
                           NetworkContext_t * pNetworkContext )
{
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

    if( mqttStatus != MQTTSuccess )
    {
        returnStatus = EXIT_FAILURE;
        LogError( ( "MQTT init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ) ) );
        Cdbg(APP_DBG, "MQTT init failed: Status = %s.", MQTT_Status_strerror( mqttStatus ));

    } else {
        LogInfo( ( "MQTT init successful, mqttStatus = %d", mqttStatus) );
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

static int subscribeTopic( MQTTContext_t * pMqttContext,
                                 bool * pClientSessionPresent, 
                                 bool * pMqttSessionEstablished )
{
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

    if( returnStatus == EXIT_SUCCESS )
    {

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
        if( brokerSessionPresent == true )
        {
            LogInfo( ( "An MQTT session with broker is re-established. "
                       "Resending unacked publishes." ) );


            Cdbg(APP_DBG, "An MQTT session with broker is re-established. "
                       "Resending unacked publishes." );

            /* Handle all the resend of publish messages. */
            returnStatus = handlePublishResend( pMqttContext );
        }
        else
        {
            LogInfo( ( "A clean MQTT connection is established."
                       " Cleaning up all the stored outgoing publishes.\n\n" ) );

            /* Clean up the outgoing publishes waiting for ack as this new
             * connection doesn't re-establish an existing session. */
            cleanupOutgoingPublishes();
        }
    }

    if( returnStatus == EXIT_SUCCESS )
    {
        /* The client is now connected to the broker. Subscribe to the topic
         * as specified in MQTT_EXAMPLE_TOPIC at the top of this file by sending a
         * subscribe packet. This client will then publish to the same topic it
         * subscribed to, so it will expect all the messages it sends to the broker
         * to be sent back to it from the broker. This demo uses QOS1 in Subscribe,
         * therefore, the Publish messages received from the broker will have QOS1. */
        LogInfo( ( "Subscribing to the MQTT topic" ) );
        returnStatus = subscribeToTopic( pMqttContext );
    }



    if( returnStatus == EXIT_SUCCESS )
    {
        /* Process incoming packet from the broker. Acknowledgment for subscription
         * ( SUBACK ) will be received here. However after sending the subscribe, the
         * client may receive a publish before it receives a subscribe ack. Since this
         * demo is subscribing to the topic to which no one is publishing, probability
         * of receiving publish message before subscribe ack is zero; but application
         * must be ready to receive any packet. This demo uses MQTT_ProcessLoop to
         * receive packet from network. */
        mqttStatus = MQTT_ProcessLoop( pMqttContext, MQTT_PROCESS_LOOP_TIMEOUT_MS );

        if( mqttStatus != MQTTSuccess )
        {
            returnStatus = EXIT_FAILURE;
            LogError( ( "MQTT_ProcessLoop returned with status = %s.",
                        MQTT_Status_strerror( mqttStatus ) ) );
        }
    }

    /* Check if recent subscription request has been rejected. globalSubAckStatus is updated
     * in eventCallback to reflect the status of the SUBACK sent by the broker. */
    if( ( returnStatus == EXIT_SUCCESS ) && ( globalSubAckStatus == MQTTSubAckFailure ) )
    {
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



void init_topic_subscribe() 
{
    subscribe_topic_number = 0;

#if defined(RTCONFIG_TC_GAME_ACC) 
    // open [tencentgmae download] function
    if(nvram_get_int("tencent_download_enable") == 1) {

        snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
            "asus/all/tencentgame");

        subscribe_topic_number++;

        snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
            "asus/%s/tencentgame", awsiot_clientid);
    
        subscribe_topic_number++;

    }
#endif

    // open [tunnel] function
    if(nvram_get_int("oauth_auth_status") == 2) {

        snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
            "asus/%s/RemoteConnection", awsiot_clientid);

        subscribe_topic_number++;


        snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
            "asus/%s/httpd", awsiot_clientid);

        subscribe_topic_number++;

        snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
            "asus/%s/httpd/123456789/update", awsiot_clientid);

        subscribe_topic_number++;

    }

        // snprintf(subscribe_topic_array[subscribe_topic_number], MAX_TOPIC_LENGTH, 
        //     "asus/%s/sip_tunnel/update", awsiot_clientid);

        // subscribe_topic_number++;

    snprintf(shadow_remote_connection_topic, 256, "$aws/things/%s/shadow/RemoteConnection/update", awsiot_clientid);
    snprintf(shadow_update_topic, 256, "$aws/things/%s/shadow/update", awsiot_clientid);
    snprintf(publish_topic, 256, "asus/%s/TencentGameAccleration/session/report", awsiot_clientid); 

    LogInfo( ( "topic subscribe number = %d", subscribe_topic_number) );
    Cdbg(APP_DBG, "topic subscribe number = %d", subscribe_topic_number);
}



void init_basic_data() 
{

    int ready_count = 0;

    while(1) {

        ready_count++;

        int ntp_ready = nvram_get_int("ntp_ready");

        int svc_ready = nvram_get_int("svc_ready");

        int link_internet = nvram_get_int("link_internet"); // 2 -> connected

        if((svc_ready == 1) && (ntp_ready == 1) && (link_internet == 2)) {
            Cdbg(APP_DBG, "waiting svc_ready -> %d, ntp_ready -> %d, link_internet -> %d", svc_ready, ntp_ready, link_internet);
            LogInfo( ( "waiting svc_ready -> %d, ntp_ready -> %d, link_internet -> %d", svc_ready, ntp_ready, link_internet) );
            break;
        } else {
            if(ready_count < 3) {
                // Cdbg(APP_DBG, "waiting svc_ready -> %d, link_internet -> %d, tencent_download_enable -> %d", svc_ready, link_internet, tencent_download_enable);
                // LogInfo( ( "waiting svc_ready -> %d, link_internet -> %d, tencent_download_enable -> %d", svc_ready, link_internet, tencent_download_enable) );
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
            LogInfo( ( "Get aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model) );
            Cdbg(APP_DBG, "Get aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model);
            break;
        } else {
            if(ready_count < 2) {
                Cdbg(APP_DBG, "error data, aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model);
                LogInfo( ( "error data, aae_deviceid -> %s, device_model -> %s", aae_deviceid, device_model) );
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
            LogInfo( ( "Get awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid) );
            Cdbg(APP_DBG, "Get awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid);
            break;
        } else {
            if(ready_count < 2) {
                Cdbg(APP_DBG, "error data, awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid);
                LogInfo( ( "error data, awsiot_endpoint -> %s, awsiot_clientid -> %s", awsiot_endpoint, awsiot_clientid) );
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



static int tencentgame_download_enable(MQTTContext_t * pMqttContext) 
{
    int tencent_download_enable = nvram_get_int("tencent_download_enable");

    Cdbg(APP_DBG, "tencent_download_enable -> %d, tencentgame_download_enable_tmp -> %d", tencent_download_enable, tencentgame_download_enable_tmp);
    LogInfo( ( "tencent download function, tencent_download_enable -> %d, tencentgame_download_enable_tmp -> %d", tencent_download_enable, tencentgame_download_enable_tmp) );


    char label_mac[32];
    snprintf(label_mac, 32, "%stencentgame", nvram_safe_get("label_mac"));

    LogInfo( ( "label mac + tencentgame = %s",  label_mac) );

    get_sha256(label_mac, sn_hash);

    LogInfo( ( "mac_hash_result -> sn_hash > %s",  sn_hash) );

    if( tencent_download_enable == 0 ) {
    	snprintf(sn_hash, 65, "%s", "");
    }

    unsigned long long int total_space = 0;
    unsigned long long int available_space = 0;
    unsigned long long int used_space = 0;

    // get_usb_space(&total_space, &used_space, &available_space, nvram_safe_get("dblog_usb_path"));
    get_usb_space(&total_space, &used_space, &available_space, nvram_safe_get("tencent_download_path"));


    LogInfo( ( "total_space = %llu, used_space = %llu, available_space = %llu", total_space, used_space, available_space) );


    char publish_data[512] = {0};
    snprintf(publish_data, 512, "{\"state\": {\"reported\": {\"firmver\": \"%s\", \"buildno\": \"%s\", \"wan0_realip_ip\":\"%s\", \"wan0_hwaddr\":\"%s\", \"label_mac\":\"%s\", \"tencentgame\" : {\"sn\": \"%s\", \"enable\": %d, \"storage\":{ \"total\": %llu, \"free\": %llu }}}}}", 
                                    nvram_safe_get("firmver"), 
                                    nvram_safe_get("buildno"), 
                                    nvram_safe_get("wan0_realip_ip"), 
                                    nvram_safe_get("wan0_hwaddr"), 
                                    nvram_safe_get("label_mac"),
                                    sn_hash,
                                    tencent_download_enable,
                                    total_space,
                                    available_space);


    LogInfo( ( "publish topic len = %d -> %s\ndata len=%d -> %s", strlen(shadow_update_topic), shadow_update_topic, strlen(publish_data), publish_data ) );

    Cdbg(APP_DBG, "publish topic len = %d -> %s\ndata len=%d -> %s", strlen(shadow_update_topic), shadow_update_topic, strlen(publish_data), publish_data);


    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( pMqttContext, shadow_update_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "tencent_download_enable topic[%s] publish success", shadow_update_topic) );
        Cdbg(APP_DBG, "topic[%s] publish success", shadow_update_topic);
        Cdbg2(APP_DBG, 1, "awsiot init : basic data publish success");
    } else {
        LogError( ( "tencent_download_enable publish error, topic = %s", shadow_update_topic) );
        Cdbg(APP_DBG, "topic publish error , topic = %s", shadow_update_topic);
        return EXIT_AWSIOT;
    }


    tencentgame_download_enable_tmp = tencent_download_enable;

    return SUCCESS;
}



int check_tencentgame_status() {

    if(nvram_get_int("tencent_download_enable") != tencentgame_download_enable_tmp) {
        LogInfo( ( "tencentgame function > status change") );
        Cdbg(APP_DBG, "tencentgame function > status change");
        return EXIT_AWSIOT;
    } else {
        return 0;
    }
}



int run_tc_download() {

    LogInfo( ( "run_tc_download, search_tc_file_count = %d", search_tc_file_count) );

    search_tc_file_count++;

    if( (search_tc_file_count % 120) == 1) {

        int status = compare_received_session(subscribe_topic_number, 0);

        LogInfo( ( "run tc_download status = %d", status ) );

        if(status == 0) {
            if(pids("tc_download")) {
                LogInfo( ("run_tc_download -> tc_download running") );
   
            } else if(search_tc_file_count < 120) {

                int sys_code = system("tc_download&");
                LogInfo( ( "run_tc_download -> tc_download run -> sys_code = %d", sys_code) );
                Cdbg(APP_DBG, "tc_download run -> sys_code = %d", sys_code);                    

            }
            
        }
    }

    if(search_tc_file_count > 360)
        search_tc_file_count = 0;
}


void publish_topic_test(int publish_num) {

    char publish_data[512] = {0};
    char publish_topic[512] = {0};


    if(publish_num == 1) {

        snprintf(publish_data, 512, "%s", "{ \"api_name\": \"get_eptoken\", \"session_id\": \"123456789\", \"param\": { \"epid\": \"AAA\", \"epusr\": \"BBB\", \"epts\": \"CCC\" }}");


    snprintf(publish_topic, 512, 
        "asus/%s/httpd/123456789/update", awsiot_clientid);
    } else if(publish_num == 2) {

        snprintf(publish_data, 512, "%s", "{ \"api_name\": \"endpoint_request_token\", \"session_id\": \"123456789\", \"param\": { \"epid\": \"AAA\", \"epusr\": \"BBB\", \"epts\": \"CCC\" }}");
    

    snprintf(publish_topic, 512, 
        "asus/%s/httpd/123456789/update", awsiot_clientid);
    } else if(publish_num == 3) {
        
        snprintf(publish_data, 512, "%s", "{ \"api_name\": \"endpoint_request_token\", \"session_id\": \"123456789\", \"param\": { \"epid\": \"AAA\", \"epusr\": \"BBB\", \"epts\": \"CCC\" }}");

    snprintf(publish_topic, 512, 
        "asus/%s/httpd", awsiot_clientid);
    }
    



    LogInfo( ("publish_shadow_remote_connection, publish topic -> %s (%d), publish_data = %s (%d)",  publish_topic, strlen(publish_topic), publish_data, strlen(publish_data)) );

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, publish_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "publish_topic  OK -> %s", publish_topic) );
        Cdbg(APP_DBG, "publish_topic  OK -> %s", publish_topic);
    } else {
        LogInfo( ( "publish_topic topic -> %s", publish_topic) );
        Cdbg(APP_DBG, "publish_topic , topic -> %s", publish_topic);
    }
}

int run_subscribe_topic(MQTTContext_t * pMqttContext, bool * pMqttSessionEstablished) 
{
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

        if( mqttStatus != MQTTSuccess )
        {
            LogError( ( "MQTT_ProcessLoop returned with status = %d [%s].",
                        mqttStatus, MQTT_Status_strerror( mqttStatus ) ) );

            Cdbg(APP_DBG, "MQTT_ProcessLoop returned with status = %d [%s].",
                        mqttStatus, MQTT_Status_strerror( mqttStatus ) );

            returnStatus = EXIT_FAILURE;
            break;

        } else {
            mqttContext_g = *pMqttContext;
        }


        run_tc_download();


        if(check_tencentgame_status() == EXIT_AWSIOT) {
            returnStatus = EXIT_AWSIOT;
            break;
        }

        LogInfo( ( "Waiting MQTT data 5s, while_count = %d", while_count) );

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
    if( *pMqttSessionEstablished == true )
    {
        // LogInfo( ( "Disconnecting the MQTT connection with %.*s.",
        //            AWS_IOT_ENDPOINT_LENGTH,
        //            AWS_IOT_ENDPOINT ) );

        LogInfo( ( "Disconnecting the MQTT connection with %s.",
                   awsiot_endpoint ) );


        if( returnStatus == EXIT_FAILURE )
        {
            /* Returned status is not used to update the local status as there
             * were failures in demo execution. */
            ( void ) disconnectMqttSession( pMqttContext );
        }
        else
        {
            returnStatus = disconnectMqttSession( pMqttContext );
        }
    }

    /* Reset global SUBACK status variable after completion of subscription request cycle. */
    globalSubAckStatus = MQTTSubAckFailure;


    // return returnStatus;
    return EXIT_AWSIOT;
}



int publish_subscribe_channel() 
{

    int report_status = -1;
    char publish_data[JSON_CONTENT_SIZE];

    report_status = parse_download_update(publish_data, awsiot_clientid, sn_hash);

    if(report_status != 0) {
        LogInfo( ( "publish_subscribe_channel > parse_download_update error, report_status --> %d", report_status) );
        Cdbg(APP_DBG, "parse_download_update error, publish data error, report_status --> %d", report_status);
        return -1;
    }


    int publish_len = strlen(publish_data);
    LogInfo( ( "publish data (%d) = %s\n", publish_len, publish_data) );
    Cdbg(APP_DBG, "publish data (%d) = %s\n", publish_len, publish_data);

    int returnStatus = EXIT_SUCCESS;

    returnStatus = publishToTopic( &mqttContext_g, publish_topic, publish_data);

    if (returnStatus == SUCCESS) {
        LogInfo( ( "aws_iot_mqtt_publish  OK -> %s", publish_topic) );
        Cdbg(APP_DBG, "aws_iot_mqtt_publish  OK -> %s", publish_topic);
    } else {
        LogInfo( ( "aws_iot_mqtt_publish topic -> %s", publish_topic) );
        Cdbg(APP_DBG, "aws_iot_mqtt_publish , topic -> %s", publish_topic);
    }
}


#define AWSIOT_SIG_ACTION SIGUSR1
enum {
    AWSIOT_ACTION_SIGUSR1,
};


static void awsiot_sig_handler(int sig)
{

    LogInfo( ( "Get tc_download sig = %d", sig) );
    Cdbg(APP_DBG, "Get tc_download sig = %d", sig);

    if(publish_running) {
        LogInfo( ( "publish_running , sig too many times") );
        Cdbg(APP_DBG, "publish_running, waiting next  sig");
        sig = -99;
    }

    switch (sig) {

        case AWSIOT_SIG_ACTION:
        {
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

struct url_data 
{
    size_t size;
    char* data;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);

        LogInfo( ( "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb ) );

    // fprintf(stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb);
    tmp = (char*) realloc(data->data, data->size + 1); /* +1 for '\0' */

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}
// size_t size = 0;

// size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
//     ((char*)stream)->append((char*)ptr, 0, size*count);
//     return size*count;
// }

struct my_info {
  int shoesize;
  char *secret;
};
 
static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
  struct my_info *i = (struct my_info *)userdata;
 
  /* now this callback can access the my_info struct */
 
  return nitems * size;
}

int main( int argc, char ** argv )
{

   // Open Debug Log
    CF_OPEN(APP_LOG_PATH,  FILE_TYPE | CONSOLE_TYPE | SYSLOG_TYPE);
    // CF_OPEN(AWS_DEBUG_TO_FILE, FILE_TYPE );
    // CF_OPEN(AWS_DEBUG_TO_CONSOLE,  CONSOLE_TYPE | SYSLOG_TYPE | FILE_TYPE);

	if(APP_DBG) {
		write_file(AWS_DEBUG_TO_FILE, " ");
	}

    init_basic_data();


#if defined(RTCONFIG_TC_GAME_ACC) 
    // receive tc_download sig
    signal(AWSIOT_SIG_ACTION, awsiot_sig_handler);
#endif


    // ipc : call [mastiff && aaews]
    awsiot_ipc_start();

    Cdbg2(APP_DBG, 1, "awsiot start");

    int returnStatus = EXIT_SUCCESS;
    MQTTContext_t mqttContext = { 0 };
    NetworkContext_t networkContext = { 0 };
    OpensslParams_t opensslParams = { 0 };
    bool clientSessionPresent = false;
    bool mqttSessionEstablished = false;

    struct timespec tp;

    ( void ) argc;
    ( void ) argv;


    /* Set the pParams member of the network context with desired transport. */
    networkContext.pParams = &opensslParams;

    /* Initialize MQTT library.  */
    returnStatus = initializeMqtt( &mqttContext, &networkContext );


    if( returnStatus == EXIT_SUCCESS )
    {
        for( ; ; )
        {
            /* Attempt to connect to the MQTT broker. If connection fails, retry after
             * a timeout. Timeout value will be exponentially increased till the maximum
             * attempts are reached or maximum timeout value is reached. The function
             * returns EXIT_FAILURE if the TCP connection cannot be established to
             * broker after configured number of attempts. */
            returnStatus = connectToServerWithBackoffRetries( &networkContext );

            if( returnStatus == EXIT_FAILURE )
            {

                /* Log error to indicate connection failure after all
                 * reconnect attempts are over. */
                // LogError( ( "Failed to connect to MQTT broker %.*s.",
                //             AWS_IOT_ENDPOINT_LENGTH,
                //             AWS_IOT_ENDPOINT ) );

                // Cdbg(APP_DBG, "Failed to connect to MQTT broker %.*s.",
                //             AWS_IOT_ENDPOINT_LENGTH,
                //             AWS_IOT_ENDPOINT);


                Cdbg(APP_DBG, "Failed to connect to MQTT broker %s.",
                            awsiot_endpoint);

            }
            else
            {
                // init [subscribe topic] data
                init_topic_subscribe();

                /* If TLS session is established, execute Subscribe loop. */
                returnStatus = subscribeTopic( &mqttContext, &clientSessionPresent, &mqttSessionEstablished );

                returnStatus = tencentgame_download_enable(&mqttContext);

                if(returnStatus == SUCCESS) {
                    returnStatus = run_subscribe_topic(&mqttContext, &mqttSessionEstablished);
                }

            }

            /* End TLS session, then close TCP connection. */
            ( void ) Openssl_Disconnect( &networkContext );

            if(returnStatus == EXIT_AWSIOT) {
                break;
            }

            LogInfo( ( "Openssl disconnect, Short delay before starting the next iteration....\n" ) );
            sleep( MQTT_SUBPUB_LOOP_DELAY_SECONDS );
        }
    }


    LogInfo( ( "exit awsiot, waiting restart") );
    Cdbg(APP_DBG, "exit awsiot, waiting restart");
    Cdbg2(APP_DBG, 1, "exit awsiot, waiting restart");
    exit(0);

    return returnStatus;
}

/*-----------------------------------------------------------*/
