#ifndef SYS_socket_subcall
# error SYS_socket_subcall is not defined
#endif
#define SYS_socket_nsubcalls	20
#define SYS_ipc_subcall		((SYS_socket_subcall)+(SYS_socket_nsubcalls))
#define SYS_ipc_nsubcalls	25
