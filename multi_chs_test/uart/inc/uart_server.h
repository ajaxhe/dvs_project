#ifndef SERVER_H_
#define SERVER_H_

#include <osa_thr.h>
#include <pthread.h>

#define MAX_CLIENT_NUM 20

struct client_obj
{
    int socket;

    pthread_t thread;
};

struct uart_server_mgr
{
    int serial_fd;
    int server_socket;
	OSA_ThrHndl m_ListenThr;	
    pthread_mutex_t mutex;

    struct client_obj clients[MAX_CLIENT_NUM];
};

extern struct uart_server_mgr g_serviceMgr;

// 初始化串口服务器代码
int uart_service_init();

// 打开串口服务器服务
int open_uart_service(int localport,char * uartname);

// 关闭串口服务器服务
int close_uart_service();

// 串口服务线程
void *uartServiceThr(void *arg);

int uart_tcp_listen_thread_start(int port);
int clean_uart_tcp_listen_thread();

#endif // SERVER_H_


