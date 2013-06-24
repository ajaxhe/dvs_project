#include "uart_server.h"
#include "common.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>

struct uart_server_mgr g_serviceMgr;

int uart_service_init()
{ 
    int i;

    g_serviceMgr.serial_fd = -1;
    g_serviceMgr.server_socket = -1;

    for( i=0; i<MAX_CLIENT_NUM; i++ )
    {
        g_serviceMgr.clients[i].socket = -1;
    }

    pthread_mutex_init(&g_serviceMgr.mutex, NULL);

    return 0;    
}

void* uart_service_start(void* uartname)
{
    int result;
    int clientSocket,client_len;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    int serialHandle;

    g_serviceMgr.serial_fd = open_serial_port(uartname);
    if( g_serviceMgr.serial_fd < 0)
    {
		return THREAD_FAILURE;
    }

    // create a socket obj for server
    g_serviceMgr.server_socket= socket(AF_INET,SOCK_STREAM,0);

    // bind tcp port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UART_LOCAL_TCP_PORT);

    result = bind(g_serviceMgr.server_socket,(struct sockaddr *)&server_addr,sizeof(server_addr) );
    if( result != 0 ) 
    {
		printf("[uart-tcp-server] bind error!\n ");    
		return THREAD_FAILURE;
    }

    result = listen(g_serviceMgr.server_socket,5);
    if( result != 0 )
    {
        printf("[uart-tcp-server] listen error!\n ");
		return THREAD_FAILURE;
    }

    int i,clientNo = -1;

    while(1)
    {
		client_len = sizeof(client_addr);
		clientSocket = accept(g_serviceMgr.server_socket,(struct sockaddr *)&client_addr,&client_len );

		if( clientSocket < 0 )
		{
	    	printf("[uart-tcp-server] accept error!\n" );
			return THREAD_FAILURE;
		}
	
        // find a idle client obj
        for( i=0; i<MAX_CLIENT_NUM; i++ )
		{
	    	if( g_serviceMgr.clients[i].socket < 0 )
	    	{
	        	break;
	    	}
		}
		// not any idle client obj
		if( i == MAX_CLIENT_NUM )
		{
	    	printf("[uart-tcp-server] the exit client num is max,cannot add more!\n");
			return THREAD_FAILURE;
		}
		clientNo = i;

		// store the client socket 
		g_serviceMgr.clients[clientNo].socket = clientSocket;

    	    // create thread to service the new client
		if( pthread_create(&g_serviceMgr.clients[clientNo].thread,NULL,uartServiceThr,&clientNo ) )
		{
		    printf("[uart-tcp-server] create thread for client fail!\n");
			return THREAD_FAILURE;
		}
    }

    return THREAD_SUCCESS;
}

int uart_service_stop()
{
    int i,socket;
    void *ret;

    printf("[uart-tcp-server] close_uart_service...\n");

    for( i=0; i<MAX_CLIENT_NUM; i++ )
    {
		socket = g_serviceMgr.clients[i].socket;
        if( socket < 0 )
		{
		    continue;
		}

		shutdown(socket,SHUT_RDWR);
		close(socket);

		g_serviceMgr.clients[i].socket = -1;

        pthread_join(g_serviceMgr.clients[i].thread,&ret);
    }

    shutdown(g_serviceMgr.server_socket,SHUT_RDWR);
    close(g_serviceMgr.server_socket);

    g_serviceMgr.server_socket = -1;

    close_serial_port(g_serviceMgr.serial_fd);

    printf("[uart-tcp-server] The uart service successfully closed!\n");

    return 0;
}

int uart_tcp_listen_thread_start(int port)
{
	uart_service_init();

	if( OSA_thrCreate(&g_serviceMgr.m_ListenThr,uart_service_start,OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,NULL))
	{                                                                                                     
		printf("[uart-tcp-server] listen thread create fail!\n");
		return -1;                                                                                        
	}  
	printf("[uart-tcp-server] Start\n");

	return 0;
}

int clean_uart_tcp_listen_thread()
{
	uart_service_stop();
	// 等待TCP监听线程退出                                                                                
	int res;                                                                                              
	res = OSA_thrJoin(&g_serviceMgr.m_ListenThr);                                                          
	if(res == 0)
	{                                                                                         
		printf("[* Join *] uart_tcp_listen_thread.\n");                                                   
	}                                                                                                     
	else
	{                                                                                                 
		printf("[* Join *] uart_tcp_listen_thread failed.\n");                                            
		return -1;                                                                                  
	}     

	return 0;
}
