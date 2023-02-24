#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <json.h>

#include "appjs.h"
#include "appstats.h"
#include "block_history.h"
#include "block_entry.h"
#include "nfdev.h"

struct json_object* appstats_node_to_json(appstats_node_t *node)
{   
    char ipstr[INET6_ADDRSTRLEN];
    char ip6str[INET6_ADDRSTRLEN];
    struct json_object *node_obj = json_object_new_object();
    if (node_obj == NULL)  {
        printf("new json object failed.\n");
        return NULL;
    }
    inet_ntop(AF_INET, &node->client_ip, ipstr, INET_ADDRSTRLEN);
    if(node->isv4)
      strcpy(ip6str, DEFAULT_IPV6_ADDR);
    else
      inet_ntop(AF_INET6, &node->srcv6, ip6str, INET6_ADDRSTRLEN);

    json_object_object_add(node_obj, "is_v4", json_object_new_int(node->isv4));
    json_object_object_add(node_obj, "client_ip", json_object_new_string(ipstr));
    json_object_object_add(node_obj, "client_ip6", json_object_new_string(ip6str));
    json_object_object_add(node_obj, "up_bytes", json_object_new_int(node->up_bytes));
    json_object_object_add(node_obj, "dn_bytes", json_object_new_int(node->dn_bytes));
    json_object_object_add(node_obj, "host", json_object_new_string(node->name));
   
    return node_obj;
}

int appstats_list_to_json(struct list_head *applist)
{
    appstats_node_t *node;
    struct json_object *json_obj;

    struct json_object *ary_obj = json_object_new_array();
    if (ary_obj == NULL) return -1;

    // dump to json file based on tcplist
    list_for_each_entry(node, applist, list) {
        json_obj = appstats_node_to_json(node);
        json_object_array_add(ary_obj, json_obj);
    }

    /* print it */
    json_object_to_file(JSON_OUTPUT_APP_FILE, ary_obj);

    // free the root object will also free its descending tree nodes
    json_object_put(ary_obj);

    return 0;

}

//block history
struct json_object* block_history_node_to_json(block_history_node_t *node)
{   
    char ipstr[INET6_ADDRSTRLEN];
    char ip6str[INET6_ADDRSTRLEN];
    struct json_object *node_obj = json_object_new_object();
    if (node_obj == NULL)  {
        printf("new json object failed.\n");
        return NULL;
    }
    inet_ntop(AF_INET, &node->client_ip, ipstr, INET_ADDRSTRLEN);

    if(node->isv4)
      strcpy(ip6str, DEFAULT_IPV6_ADDR);
    else
      inet_ntop(AF_INET6, &node->srcv6, ip6str, INET6_ADDRSTRLEN);
    
    json_object_object_add(node_obj, "is_v4", json_object_new_int(node->isv4));
    json_object_object_add(node_obj, "client_ip", json_object_new_string(ipstr));
    json_object_object_add(node_obj, "client_ip6", json_object_new_string(ip6str));
    json_object_object_add(node_obj, "name", json_object_new_string(node->name));
    json_object_object_add(node_obj, "timestamp", json_object_new_int(node->timestamp));
   
    return node_obj;
}

int block_history_list_to_json(struct list_head *bhlist)
{
    block_history_node_t *node;
    struct json_object *json_obj;

    struct json_object *ary_obj = json_object_new_array();
    if (ary_obj == NULL) return -1;

    // dump to json file based on tcplist
    list_for_each_entry(node, bhlist, list) {
        json_obj = block_history_node_to_json(node);
        json_object_array_add(ary_obj, json_obj);
    }

    json_object_to_file(JSON_OUTPUT_BLOCK_HISTORY_FILE, ary_obj);

    // free the root object will also free its descending tree nodes
    json_object_put(ary_obj);

    return 0;

}


//block list entry
struct json_object* block_entry_node_to_json(block_entry_node_t *node)
{   
    struct json_object *node_obj = json_object_new_object();
    if (node_obj == NULL)  {
        printf("new json object failed.\n");
        return NULL;
    }
    json_object_object_add(node_obj, "name", json_object_new_string(node->name));
    json_object_object_add(node_obj, "type", json_object_new_int(node->type));
   
    return node_obj;
}

int block_entry_list_to_json(struct list_head *bhlist)
{
    block_entry_node_t *node;
    struct json_object *json_obj;

    struct json_object *ary_obj = json_object_new_array();
    if (ary_obj == NULL) return -1;

    // dump to json file based on tcplist
    list_for_each_entry(node, bhlist, list) {
        json_obj = block_entry_node_to_json(node);
        json_object_array_add(ary_obj, json_obj);
    }

    json_object_to_file(JSON_OUTPUT_BLOCK_ENTRY_FILE, ary_obj);

    // free the root object will also free its descending tree nodes
    json_object_put(ary_obj);

    return 0;

}





struct json_object* device_query_node_to_json(nfdev_node_t *node)
{   
    char ipstr[INET6_ADDRSTRLEN];
    
    struct json_object *node_obj = json_object_new_object();
    if (node_obj == NULL)  {
        printf("new json object failed.\n");
        return NULL;
    }
    if(node->isv4)
      inet_ntop(AF_INET, &node->client_ip, ipstr, INET_ADDRSTRLEN);
    else
      inet_ntop(AF_INET6, &node->srcv6, ipstr, INET6_ADDRSTRLEN);

    json_object_object_add(node_obj, "ip", json_object_new_string(ipstr));
    json_object_object_add(node_obj, "mac", json_object_new_string(node->mac));
    json_object_object_add(node_obj, "type", json_object_new_string(node->dev_type));
    json_object_object_add(node_obj, "count", json_object_new_int(node->count));
   
    return node_obj;
}

int device_query_list_to_json(struct list_head *devlist)
{
    nfdev_node_t *node;
    struct json_object *json_obj;

    struct json_object *ary_obj = json_object_new_array();
    if (ary_obj == NULL) return -1;

    // dump to json file based on tcplist
    list_for_each_entry(node, devlist, list) {
        json_obj = device_query_node_to_json(node);
        json_object_array_add(ary_obj, json_obj);
    }

    json_object_to_file(JSON_DEVICE_TYPE_QUERY_FILE, ary_obj);

    // free the root object will also free its descending tree nodes
    json_object_put(ary_obj);

    return 0;

}
