

typedef struct _CM_CTRL {
  int flagIsTerminated;           /* if terminate CM daemon */
  int flagIsRunning;            /* if handler is running */
  int flagIsFirmwareCheck;  /* if check firmware */
  int socketTCPSend;              /* used to send/rcv TCP frame */
  int socketUdpSendRcv;           /* used to send/rcv UDP frame */
  int socketIpcSendRcv;   /* used to send/rcv frame from IPC */
  struct in_addr ownAddr;         /* IP address of ethernet interface */
  struct in_addr broadcastAddr;   /* broadcast address of ethernet interface */
  char brIfMac[32];   /* br0 mac address */
  pid_t pid;                      /* CM task PID */
  unsigned char *publicKey;       /* used to save public key for CM */
  size_t publicKeyLen;            /* used to save the length of public key for CM */
  unsigned char *privateKey;      /* used to save private key for CM */
  size_t privateKeyLen;           /* used to save the length of private key for CM */
  unsigned char *sessionKey;  /* used to save session key for CM */
  time_t sessionKeyStartTime; /* the start time of session key */
  unsigned char *sessionKey1; /* used to save session key 1 for CM */
  time_t sessionKey1StartTime;  /* the start time of session key 1 */
  size_t sessionKeyLen;   /* used to save the length of public key for CM */
  int sessionKeyReady;    /* used to check session key is ready or not */
  unsigned char *groupKey;        /* used to save group key for CM */
  time_t groupKeyStartTime;       /* the start time of group key */
  unsigned char *groupKey1;        /* used to save group key 1 for CM */
  time_t groupKey1StartTime;       /* the start time of group key 1 */
  size_t groupKeyLen;             /* used to save the length of group key for CM */
  int groupKeyReady;    /* used to check group key is ready or not */
  unsigned int role;    /* role for server or client */
  int cost;   /* used to save the cost of network topology */
} CM_CTRL, *P_CM_CTRL;


enum states {
  START = 0,
  INIT,
  REKEY,
  DISCONN,
  PENDING,
  CONN,
  PERCHECK, // periodic check
  IMMCHECK, // check immediatly
  GREKEY,   /* group rekey */
  RESTART,
  MAX_STATE
};


enum cfgRole {
  IS_UNKNOWN = 0,
  IS_SERVER,
  IS_CLIENT
};

void ipc_start(); 

void aae_rcvHandler(CM_CTRL *pCtrlBK);
