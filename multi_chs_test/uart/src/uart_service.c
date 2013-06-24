#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "uart_command.h"
#include "uart_server.h"
#include "uart_operate.h"

typedef unsigned char uint8_t;

int recv_tcp_pack( int socketfd, uint8_t* buffer, int len )
{
    int returnBytes;

    returnBytes = recv(socketfd,buffer,len,0);

    if( returnBytes == len )
    {
        //printf("ok,recv %d bytes! \n",len);
    }
    else if( returnBytes <= 0 && errno != EINTR && errno != EWOULDBLOCK && errno
!= EAGAIN )
    {
        printf("disconnected or error occur! errno=%d\n ",errno);
		return -1;
    }
    else
    {
		printf("socket fatal error,recvBytes=%d!\n",returnBytes);    
        return -1;
    }

    return 0;
}

int tcp_analysis_cmdheader( int socketfd )
{
    int ret = -1;
    unsigned char header[TCP_CMD_HEADER_LEN];

    ret = recv_tcp_pack( socketfd,(uint8_t *)header,TCP_CMD_HEADER_LEN);
    if( ret < 0 )
    {
        return ret;
    }

    // check the header
    if ( memcmp( header, TCP_CMD_HEADER_STR, TCP_CMD_HEADER_LEN-1 ) != 0 )
    {
        return -1;
    }

    return header[TCP_CMD_POSITION];
}

int tcp_cmd_set_serial_port( int socketfd,int serialfd )
{
    int  ret;
    SerialPack pack;

    ret = recv_tcp_pack( socketfd,(uint8_t *)&pack,sizeof(pack));
    if( ret < 0 )
    {
        return ret;
    }

    pthread_mutex_lock(&g_serviceMgr.mutex);
	

    printf("set_serial_speed %d\n",pack.baudrate);
    if( set_serial_speed(serialfd,pack.baudrate) != 0)
    {
	pthread_mutex_unlock(&g_serviceMgr.mutex);  
        return -1;
    }

    printf("set_serial_param: databits=%d,stopbits=%d,parity=%c\n",pack.databits,pack.stopbits,pack.parity);
    if( set_serial_parity(serialfd,pack.databits,pack.stopbits,pack.parity) != 0)
    {
	pthread_mutex_unlock(&g_serviceMgr.mutex);
        return -1;
    }

    pthread_mutex_unlock(&g_serviceMgr.mutex);

    printf("good, set serial port successful!\n");

    return 0;
}

int tcp_cmd_exec_ptz( int socketfd,int serialfd )
{
    int i,ret;
    PtzCmdPack pack;
	static int uart_cmd_count;
	
    ret = recv_tcp_pack( socketfd,(uint8_t *)&pack,sizeof(pack));
    if( ret < 0 )
    {
        return ret;
    }
/*
    printf("the ptz cmd is: \n");
    for( i=0;i<pack.len; i++)
    {
        printf("0x%x\t",pack.cmd[i]);
    }
    printf("\n");
*/

    pthread_mutex_lock(&g_serviceMgr.mutex);

	uart_cmd_count++;
	printf("[yuntai_cmd] count:%d, cmd:", uart_cmd_count);
	
	for (i = 0; i < pack.len; i++)
	{
		printf("0x%x,", pack.cmd[i]);
	}
	printf("\n");

    if( send_serial_data(serialfd,pack.cmd,pack.len) != pack.len)
    {
	printf("send_serial_data fail!\n");

	pthread_mutex_unlock(&g_serviceMgr.mutex);

        return -1;
    }

    pthread_mutex_unlock(&g_serviceMgr.mutex);

    //printf("good, ptz control successful!\n");

    return 0;
}

void *uartServiceThr( void * arg )
{
    int exit = 0;
    int returnBytes;
    int serverCMD;
    int clientNo = *((int *)arg);
    int socketfd = g_serviceMgr.clients[clientNo].socket;
    int serialfd = g_serviceMgr.serial_fd;

    printf("clientNo = %d\n",clientNo);

    while(!exit)
    {
	// read cmd from client
	serverCMD = tcp_analysis_cmdheader(socketfd);
	if( serverCMD < 0 )
	{
	    break;
	}
	
	switch( serverCMD )
	{
	case DVS_TCP_CMD_UART: 
             if( tcp_cmd_set_serial_port(socketfd,serialfd) < 0 )
	     {
	         exit = 1; 
	     }
	     break;
	case DVS_TCP_CMD_PTZ:
	     if( tcp_cmd_exec_ptz(socketfd,serialfd) < 0 )
	     {
	         exit = 1;
	     }
	     break;
	default:
	     printf("unsupported cmd!\n");
             break;
	}
    }

    printf("close session socket,clientNo = %d!\n");

    // close socket
    close(g_serviceMgr.clients[clientNo].socket);
    g_serviceMgr.clients[clientNo].socket = -1;

    pthread_exit(0);
}

