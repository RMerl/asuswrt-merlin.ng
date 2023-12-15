#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bits.h"
#include "debug.h"
#include "printbuf.h"
#include "json_object.h"
#include "json_tokener.h"
#include "json_util.h"


struct json_object* json_object_from_file(char *filename)
{
  struct printbuf *pb;
  struct json_object *obj;
  char buf[JSON_FILE_BUF_SIZE];
  int fd, ret;

  if((fd = open(filename, O_RDONLY)) < 0) {
    mc_error("json_object_from_file: error reading file %s: %s\n",
	     filename, strerror(errno));
    return error_ptr(-1);
  }
  if(!(pb = printbuf_new())) {
    mc_error("json_object_from_file: printbuf_new failed\n");
    return error_ptr(-1);
  }
  while((ret = read(fd, buf, JSON_FILE_BUF_SIZE)) > 0) {
    printbuf_memappend(pb, buf, ret);
  }
  close(fd);
  if(ret < 0) {
    mc_abort("json_object_from_file: error reading file %s: %s\n",
	     filename, strerror(errno));
    printbuf_free(pb);
    return error_ptr(-1);
  }
  obj = json_tokener_parse(pb->buf);
  printbuf_free(pb);
  return obj;
}

int json_object_to_file(char *filename, struct json_object *obj)
{
  char *json_str;
  int fd, ret, wpos, wsize;

  if(!obj) {
    mc_error("json_object_to_file: object is null\n");
    return -1;
  }

  if((fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0) {
    mc_error("json_object_to_file: error opening file %s: %s\n",
	     filename, strerror(errno));
    return -1;
  }
  if(!(json_str = json_object_to_json_string(obj))) return -1;
  wsize = strlen(json_str);
  wpos = 0;
  while(wpos < wsize) {
    if((ret = write(fd, json_str + wpos, wsize-wpos)) < 0) {
      close(fd);
      mc_error("json_object_to_file: error writing file %s: %s\n",
	     filename, strerror(errno));
      return -1;
    }
    wpos += ret;
  }

  close(fd);
  return 0;
}
