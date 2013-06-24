#ifndef SERVER_H_
#define SERVER_H_

#include "common.h"


int tcp_listen_thread_start( int localport );
int clean_tcp_listen_thread();
void* alarmInputThrFxn(void* arg);

#endif // SERVER_H_
