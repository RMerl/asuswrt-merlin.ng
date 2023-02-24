#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "dns_app_client.h"

int main(int argc, char **argv)
{
  int ret;
  unsigned long start, end;
  int app_or_cli, up_or_dn;
  start = 0;
  end = time(NULL);
  app_or_cli = ORDER_BY_APP;
  up_or_dn = ORDER_BY_UPLOAD;
  json_object *resultObj = NULL;
  int time = 2;
  
  // test 1 - app stats
  while (time)
  {
    ret = dns_app_stats_json(start, end, app_or_cli, up_or_dn, &resultObj);

    if( ret != 0 )
    {
      printf("dns_app_stats_json fail=%d\n", ret);
      return -1;
    }

    if(resultObj) 
    {
      printf("result json=%s\n", json_object_to_json_string(resultObj));
      json_object_to_file("dns_app_client_test.json", resultObj);
      json_object_put(resultObj);
    }
    time--;
    app_or_cli ++;
    up_or_dn ++;
  }
 
  // test 2 - block history
  ret = dns_block_stats_json(start, end, &resultObj);
  if( ret != 0 )
  {
    printf("dns_block_stats_json fail=%d\n", ret);
    return -1;
  }

  if(resultObj) 
  {
    printf("result json=%s\n", json_object_to_json_string(resultObj));
    json_object_to_file("dns_block_history_test.json", resultObj);
    json_object_put(resultObj);
  }
  
  // test 3 - block list
  ret = dns_block_list_json(&resultObj);
  if( ret != 0 )
  {
    printf("dns_block_list_json fail=%d\n", ret);
    return -1;
  }

  if(resultObj) 
  {
    printf("result json=%s\n", json_object_to_json_string(resultObj));
    json_object_to_file("dns_block_list_test.json", resultObj);
    json_object_put(resultObj);
  }


  // test 4 - block list insert or update
  char host[]="www.coo.com";
  ret = dns_block_list_insert_or_update(host, IN_BLACK_LIST);
  if( ret != 0 )
  {
    printf("dns_block_list_insert_or_update fail=%d\n", ret);
    return -1;
  }

  ret = dns_block_list_insert_or_update(host, IN_WHITE_LIST);
  if( ret != 0 )
  {
    printf("dns_block_list_insert_or_update fail=%d\n", ret);
    return -1;
  }
 
  // test 5 - bwdpi compatible api
  int retval;
  //dns_sqlite_Stat_hook(0, "all", "hour", "24", "1642568274", &retval, 0);
  //printf("dns_sqlite_Stat_hook retval=%d\n", retval);
  
  // test 6 - bwdpi compatible api
  //dns_sqlite_Stat_hook(0, "line", "detail", "31", "1642567274", &retval, 0);
  //printf("dns_sqlite_Stat_hook retval=%d\n", retval);

  // test 7 - bwdpi compatible api
  //dns_sqlite_Stat_hook(1, "11:22:33:44:55:66", "detail", "7", "1642567274", &retval, 0);
  //printf("dns_sqlite_Stat_hook retval=%d\n", retval);

  // test 8 - bwdpi compatible api
  dns_sqlite_Stat_hook(1, "00:15:5d:60:7e:21", "detail", "7", "1642577274", &retval, 0);
  printf("\ndns_sqlite_Stat_hook retval=%d\n", retval);

  return 0;
}
