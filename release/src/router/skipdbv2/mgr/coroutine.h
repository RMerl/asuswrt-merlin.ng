/*
 * 协程实现，最开始见于putty软件的实现，文件头部有介绍。
 * 此版本有修改，不使用动态内存申请，内存由外部函数申请好。
 */

#ifndef COROUTINE_H
#define COROUTINE_H

#include <stdlib.h>

#define COROUTINE_CONTEXT_LEN 	64

struct ccrContextTag {
    int ccrLine;
    /* 协程中用于临时内存交换 */
    unsigned char stack_buf[COROUTINE_CONTEXT_LEN]; 
};

 /* assert(sizeof(struct _stack_##__FUNCTION__) < COROUTINE_CONTEXT_LEN)
 * 未实现assert函数 */
#define TOKENPASTE(x, y) 	x ## y
#define TOKENPASTE2(x, y) 	TOKENPASTE(x, y)
#define TYPE_UNIQUE 			TOKENPASTE2(_statck_, __LINE__)
#define ccrBeginContext  typedef struct TYPE_UNIQUE {
#define ccrEndContext(x) } TYPE_UNIQUE; TYPE_UNIQUE *CS; do { CS = (TYPE_UNIQUE*)(&((x)->stack_buf[0]));} while(0)

#define ccrBegin(x)      if (x){ 			\
                         switch((x)->ccrLine) { case 0:;
#define ccrFinish(x,z)     }} do { (x)->ccrLine = 0; return (z); } while(0)

#define ccrReturn(x,z)     \
        do {\
            (x)->ccrLine=__LINE__;\
            return (z); case __LINE__:;\
        } while (0)

#define ccrStop(x,z)       do{ (x)->ccrLine = 0; return (z); }while(0)

#endif /* COROUTINE_H */
