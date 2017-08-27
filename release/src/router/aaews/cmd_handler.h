#ifndef _CMD_HANDLER_H
#define _CMD_HANDLER_H

// Command Name
#define CMD_SETTING_CHANGE    "setting_change"
#define CMD_IFTTT_INFO        "ifttt_info"
 
struct queue_struct 
{
    int capacity;
    int front;
    int rear;
    int size;
    void **array;
};

struct queue_struct;
typedef struct queue_struct *queue_t;

// Create command handler instance.
int cmd_handler_create(char *cmd_path);

// Destroy command handler
int cmd_handler_destroy();

#define GET_JSON_STRING_FIELD(json_obj, name) json_object_get_string(json_object_object_get(json_obj, name))
#endif
