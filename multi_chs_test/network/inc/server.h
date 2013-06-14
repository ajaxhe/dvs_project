#ifndef SERVER_H_
#define SERVER_H_

#include "common.h"

#define LOCAL_TCP_PORT 9090

int tcp_listen_thread_start( int localport );
int clean_tcp_listen_thread();

#endif // SERVER_H_
