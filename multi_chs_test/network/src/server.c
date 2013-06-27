//////////////////////////////////////////////////////////////////////////
//  COPYRIGHT NOTICE
//  Copyright (c) 2013, 华中科技大学CBIB实验室（版权声明）
//  All rights reserved.
// 
/// @file    server.c  
/// @brief   TCP服务主线程
///
/// 主要负责监听客户端的连接请求，并创建服务子线程为客户端服务
///
/// @version 1.0   
/// @author  lujun 
/// @date    2013/02/25
//
//
//  修订说明：
//////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include "serverIf.h"
#include "server.h"
#include "command.h"
#include "common.h"
#include <osa_thr.h>
#include <errno.h>

static ServerEnv g_serverEnp;

static void *listenThrFxn( void * arg);

/** 开启TCP端口的监听
 *
 *  创建socket对象，开启TCP端口的监听
 *  @param:  int localport 本地监听端口
 *  @return: [int] 成功返回0，失败返回-1
 *  @note:       
 */
int tcp_listen_thread_start( int localport )
{
    int result;
    struct sockaddr_in server_addr;

    // 创建socket对象
    g_serverEnp.m_Socket = socket(AF_INET,SOCK_STREAM,0);

    // 监听端口和地址的配置
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(localport);

    // 绑定监听端口
    result = bind(g_serverEnp.m_Socket,(struct sockaddr *)&server_addr,sizeof(server_addr) );
    if( result != 0 ) 
    {
        printf("[tcp server] bind error!\n ");    
    	return -1;
    }

    // 创建监听线程
    if( OSA_thrCreate(&g_serverEnp.m_ListenThr,listenThrFxn,OSA_THR_PRI_DEFAULT,TCP_THREAD_STACK_SIZE,NULL))
    {
        printf("listen thread create fail!\n");
        return -1;
    }

    return 0;
}

//TCP监听线程
void *listenThrFxn( void * arg)
{
    int result;
    int clientSocket;
    unsigned int client_len;

    struct sockaddr_in client_addr;

    // 开始监听
    result = listen(g_serverEnp.m_Socket,5);  
    if( result != 0 )
    {
        printf("[tcp-server] listen error!\n ");
        return THREAD_FAILURE;
    }
	g_serverEnp.m_clientCount = 0;
	printf("[TCP_LISTEN]:START.\n");

    while(!gblGetQuit())
    {
    	int index = g_serverEnp.m_clientCount;
        client_len = sizeof(client_addr);

        // 有客户端连接，则创建新的socket对象，服务该客户端
        clientSocket = accept(g_serverEnp.m_Socket,(struct sockaddr *)&client_addr,&client_len );
		
        if(clientSocket == -1)
            break;
        g_serverEnp.m_Client[index].m_Index = index;
        g_serverEnp.m_Client[index].m_Socket = clientSocket;
        memcpy(g_serverEnp.m_Client[index].m_Ip,inet_ntoa(client_addr.sin_addr),client_len);

        printf("new client! IP addr = %s\n",g_serverEnp.m_Client[index].m_Ip);
		g_serverEnp.m_clientCount++;

        //创建单独的TCP服务子线程
        add_new_tcp_process_thr(&g_serverEnp.m_Client[index]);
		
    }

    OSA_thrExit(NULL);
}

//关闭TCP监听服务线程
int clean_tcp_listen_thread()
{
	int i;

    //依次关闭所有TCP服务子线程
	for(i = 0;i < g_serverEnp.m_clientCount;i++)
    {
		if( shutdown(g_serverEnp.m_Client[i].m_Socket,SHUT_RDWR)!= 0 )
        {
			//printf("shutdown client[%d] failed.\n",i);  // this will fail,i don't know why.
		}
		if( close(g_serverEnp.m_Client[i].m_Socket)!=0 )
        {
			printf("close client[%d] failed.\n",i);
		}

        // 等待子线程退出
		int res;
   	    res = OSA_thrJoin(&g_serverEnp.m_Client[i].m_Thr);
	    if(res == 0)
        {
		 	printf("[* Join *]		tcpServer[%d].\n",i);
	    }
	    else
        {
			printf("[* Join *]		tcpServer[%d] failed.\n",i);
			//return -1;
	    }	
	}
	
    // 关闭TCP监听对象
	if(shutdown(g_serverEnp.m_Socket,SHUT_RDWR)!=0)
    {
		perror("shutdown tcp_listen failed.\n");   // only this will not fail,i don't know why.
	}
    if(close(g_serverEnp.m_Socket) !=0 )
    {
		perror("close tcp_listen failed.\n");
	}

    // 等待TCP监听线程退出
	int res;
    res = OSA_thrJoin(&g_serverEnp.m_ListenThr);
	if(res == 0){
		printf("[* Join *]		tcp_listen_thread.\n");
	}
	else{
		perror("[* Join *]		tcp_listen_thread failed.\n");
			//return -1;
	}

    return 0;
}

void* alarmInputThrFxn(void* arg)
{
	ThreadEnv *envp = (ThreadEnv*)arg;
	Rendezvous_meet(&envp->rendezvousInit);
	printf("[Alarm-input] Start\n");

	AlarmStatePack alarmPack;
	memcpy(alarmPack.cmdHeader, ALARM_HEADER_STR, TCP_CMD_HEADER_LEN);

	int i, chs, alarm_count, sleep_time;
	chs = gblGetVideoChannels();
	alarm_count = 0;

	while (!gblGetQuit())
	{
		alarmPack.number = rand() % chs;
		alarmPack.state = rand() % 2;

		//sent the alarm pack to client who connect to the server
		for(i=0; i < g_serverEnp.m_clientCount; i++)
		{
			ClientEnv *pClient = &g_serverEnp.m_Client[i];
			if (pClient->m_Index >= 0)
			{
				send(pClient->m_Socket, (uint8_t *)&alarmPack, RETURN_PACK_LEN, 0 ) ;
				printf("[Alarm-input] alarm count: %d, chId:%d, state:%d\n", alarm_count, alarmPack.number, alarmPack.state);
			}
		}
		alarm_count++;

		sleep_time = 30 + rand() % 100;
		sleep(sleep_time);
	}
}
