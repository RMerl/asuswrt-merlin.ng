#define AAEUAC_PID_FILE           "/var/run/aaeuac_%s_%d.pid"

typedef struct server_map server_map_t;
struct server_map {
    char *area;
    char *portal;
    char *login;
};