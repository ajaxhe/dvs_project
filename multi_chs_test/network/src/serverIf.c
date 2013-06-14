//////////////////////////////////////////////////////////////////////////
//  COPYRIGHT NOTICE
//  Copyright (c) 2013, 华中科技大学CBIB实验室（版权声明）
//  All rights reserved.
// 
/// @file    serverIf.c  
/// @brief   TCP服务子线程
///
/// 每一路连接到服务器的客户端，均有一路服务子线程为其服务
///
/// @version 1.0   
/// @author  lujun 
/// @date    2013/02/25
//
//
//  修订说明：
//////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "serverIf.h"
#include "command.h"
#include "session.h"
#include "rtpsend.h"

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <osa_thr.h>

// TCP服务子线程函数
void * tcpServerThrFxn( void * arg )
{
	ClientEnv * envp = (ClientEnv *)arg;
	int socketfd = envp->m_Socket;
	int returnBytes;
	int clientID;
    int clientQuit = 0;

	ServerPack sPack;
	ReturnPack rPack;
	memcpy(rPack.cmdHeader,TCP_CMD_HEADER_STR,TCP_CMD_HEADER_LEN);

    // TCP服务循环
	while( !gblGetQuit() && !clientQuit )
	{
		// 从客户端读取TCP命令
		returnBytes = recv(socketfd,(uint8_t *)&sPack,SERVER_PACK_LEN,0);
		if( returnBytes == SERVER_PACK_LEN )
		{
			//printf("ok,recv %d bytes! \n",SERVER_PACK_LEN);
		}
		else if( returnBytes <= 0 && errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN )
		{
			printf("recv error! errno = %d,recv bytes = %d\n",errno,returnBytes);
			printf("disconnected or error occur!\n close the socket!\n");
			break;
		}
		else
		{
			printf("return Bytes < %d\n",SERVER_PACK_LEN);
			continue;
		}

		// 检测包头
		if ( memcmp( sPack.cmdHeader, TCP_CMD_HEADER_STR, TCP_CMD_HEADER_LEN ) != 0 )
		{
			rPack.returnCMD = DVS_RETURN_INVLID_HEADER;

			// 返回错误信息给客户端
			returnBytes = send(socketfd,(uint8_t *)&rPack,RETURN_PACK_LEN,0 ) ;
			if( returnBytes < RETURN_PACK_LEN)
			{
				printf("send error!\n");
				continue;
			} 
		}

		// 解析包头
		rPack.returnCMD = DVS_RETURN_SUCCESS;
		switch( sPack.serverCMD )
        {
        case DVS_CMD_RESET_BOARD:
	         gblSetExitState(DVS_EXIT_RESET_BOARD);
	         break;

        case DVS_CMD_ADD_CLIENT:
             clientID = add_client(envp->m_Ip,sPack.Parameters.ListenPort.videoport,sPack.Parameters.ListenPort.audioport);
             if( clientID == -1)
		     {
			    printf("app_create_rtp_session video Fail!\n");
			    rPack.returnCMD = DVS_RETURN_FAIL;			
		     }
             break;

        case DVS_CMD_DEL_CLIENT:             
             clientQuit = 1;
             break;

        case DVS_CMD_OPEN_CHANNEL:
             open_channel(clientID,sPack.Parameters.PlayPort.chId);
             break;
         
        case DVS_CMD_CLOSE_CHANNEL:
             close_channel(clientID,sPack.Parameters.PlayPort.chId);
             break;

        case DVS_CMD_GET_RESOLUTION:
             if( gblGetResolution() == VIDEO_RESOLUTION_CIF )
                rPack.Parameters.Resolution.resolution = DVS_RESOLUTION_CIF;
             else
                rPack.Parameters.Resolution.resolution = DVS_RESOLUTION_D1;
             break;
        
        case DVS_CMD_SET_RESOLUTION:
             if( clientID,sPack.Parameters.Resolution.resolution == DVS_RESOLUTION_CIF)
                 gblSetResolution(VIDEO_RESOLUTION_CIF);
             else
                 gblSetResolution(VIDEO_RESOLUTION_D1);
	             gblSetExitState(DVS_EXIT_RESET_BOARD);
	             clientQuit=1;
	         break;

        case DVS_CMD_SET_BITRATE:
             break;

		case DVS_CMD_ON_ALARMOUT:
			 //alarmOut(sPack.Parameters.Alarm.chId,1);
			 break;
		case DVS_CMD_OFF_ALARMOUT:
			 //alarmOut(sPack.Parameters.Alarm.chId,0);
			 break;
		case DVS_CMD_GET_ALARMOUT:
			 //rPack.Parameters.Alarm.state = isAlarmOn(sPack.Parameters.Alarm.chId);
			 break;
		case DVS_CMD_GET_ALARMIN:
			 //alarmIn(sPack.Parameters.Alarm.chId,&rPack.Parameters.Alarm.state);
			 break;
		case DVS_CMD_ON_AUDIO:
/*
			 printf("switch on audio\n");
			 if(OSA_flgSet(&flag_audModule,1) != OSA_SOK){
			 	printf("switch on audio module failed.\n");
				rPack.returnCMD = DVS_RETURN_FAIL;
			 }
*/
			 break;
		case DVS_CMD_OFF_AUDIO:
/*
			printf("switch off audio.\n");
			 OSA_flgClear(&flag_audModule,1);
*/
			 break;
        default:
             rPack.returnCMD = DVS_RETURN_INVLID_CMD;
             break;
        }
	
	    // return result info to client
	    returnBytes = send(socketfd,(uint8_t *)&rPack,RETURN_PACK_LEN,0 );
        
	    if( returnBytes < RETURN_PACK_LEN )
	    {
			printf("send error!\n");
			continue;		
	    }
	}

	// close rpt session
	if( clientID != -1)
	{
		del_client(clientID);
	}

	// close socket
    envp->m_Index = -1;

    printf("close rtp session and client socket!\n");

	OSA_thrExit(NULL);
}

// 添加新的TCP服务子线程
int add_new_tcp_process_thr( ClientEnv * enp )
{
    // create thread
    if( OSA_thrCreate(&enp->m_Thr,tcpServerThrFxn,OSA_THR_PRI_MAX,TCP_THREAD_STACK_SIZE,enp))
    {
        printf("tcp thread create fail!\n");
        return -1;
    }
 
    printf("tcp thread has been created!\n");
	return 0;
}
