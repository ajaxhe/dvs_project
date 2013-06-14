//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, HUST CBIB Group  £¨copyright statement£©
/// All rights reserved.
/// 
/// @file    rtpsend.c
/// @brief   RTP send implementation file
///
/// the implementation of rtp send funcs
///
/// @version 1.0	 
/// @author  lujun
/// @date    2010/11/10
///
///
/// revision note:
//////////////////////////////////////////////////////////////////////////

#include "rtpsend.h"
#include "ortp/ortp.h"

#include <stdio.h>
#include <stdlib.h>

/* Session Obj */
typedef struct _sessionObj
{
    RtpSession *pSession;
    uint32_t    curTimeStamp;
    uint32_t    timeStampInc;
	
}SessionObj;

/* global var */
SessionObj g_rtpSessionMgr[MAX_SESSION_NUM];
char *g_ssrc;

int alloc_sessionID()
{
    int i = 0;
	
    for( i=0; i<MAX_SESSION_NUM; i++ )
    {
   	    //look for unused sessionID
	    if( g_rtpSessionMgr[i].pSession == NULL )
	    {
	        return i;
	    }
    }

    ortp_message("all seesionID have been used! \n");
	
    return -1;
}

void rtp_init()
{
    int i ;
	
    for( i=0;i<MAX_SESSION_NUM; i++)
    {
        g_rtpSessionMgr[i].pSession= NULL;
        g_rtpSessionMgr[i].curTimeStamp= 0;
        g_rtpSessionMgr[i].timeStampInc= 0;
    }

    ortp_init();
    ortp_scheduler_init();

    g_ssrc = getenv("SSRC");
    if (g_ssrc!=NULL) 
    {
        ortp_message("using SSRC=%i.\n",atoi(g_ssrc));
    }

    ortp_message("rtpInit successfully !\n ");
}

int create_rtp_session( char *ipStr, int port, int sessionType )
{
    int newSessionID = -1;
    RtpSession *session;

    //alloc sessionID
    newSessionID = alloc_sessionID();
    if( newSessionID == -1 )
    {
        return -1;
    }

    session = rtp_session_new(RTP_SESSION_SENDONLY);	
		
    rtp_session_set_scheduling_mode(session,1);
    rtp_session_set_blocking_mode(session,1);
    rtp_session_set_remote_addr(session,ipStr,port);

    ortp_message("remote [ip,port] str: [%s,%d]!\n ",ipStr,port);

    if(g_ssrc != NULL)
    {
        rtp_session_set_ssrc(session,atoi(g_ssrc));
    }

    if( sessionType == SESSION_TYPE_VIDEO)
    {
        rtp_session_set_payload_type(session,PAYLOAD_TYPE_VIDEO);
        g_rtpSessionMgr[newSessionID].timeStampInc = VIDEO_TIME_STAMP_INC;
    }
    else if(sessionType == SESSION_TYPE_AUDIO)
    {
        rtp_session_set_payload_type(session,PAYLOAD_TYPE_AUDIO);
        g_rtpSessionMgr[newSessionID].timeStampInc = AUDIO_TIME_STAMP_INC;
    }
    else
    {
        ortp_message("sessionType undefined !\n ");
        return -1;
    }

    g_rtpSessionMgr[newSessionID].pSession = session;

    ortp_message("create rtp session successfully,sessionID: %d !\n ",newSessionID);

    return newSessionID;

}

int rtp_send( int sessionID, char *buffer, int len )
{
    int sendBytes = 0;

    if( sessionID <0 || sessionID > MAX_SESSION_NUM)
    {
        //ortp_message("invlid rtp sessionID : %d ! \n",sessionID);
        return -1;
    }
	
    if( g_rtpSessionMgr[sessionID].pSession == NULL )
    {
        ortp_message("session ID = %d, not exit ! \n",sessionID);
        return -1;
    }

    //ortp_message("send data len %i\n ",len);

    sendBytes=rtp_session_send_with_ts(g_rtpSessionMgr[sessionID].pSession,
		(char *)buffer,len,g_rtpSessionMgr[sessionID].curTimeStamp);

    g_rtpSessionMgr[sessionID].curTimeStamp += g_rtpSessionMgr[sessionID].timeStampInc; 

    return 0;
	
}

int remove_session_obj( int sessionID )
{
    if( sessionID <0 || sessionID > MAX_SESSION_NUM)
    {        
        return -1;
    }

	//destory the rtp session
    if( g_rtpSessionMgr[sessionID].pSession!=NULL )
    {
    	rtp_session_destroy( g_rtpSessionMgr[sessionID].pSession );
        g_rtpSessionMgr[sessionID].pSession = NULL;
    }
    g_rtpSessionMgr[sessionID].curTimeStamp = 0;
    g_rtpSessionMgr[sessionID].timeStampInc = 0;

    return 0;
}

int rtp_exit()
{	
    int i ;
	
    for(i=0;i<MAX_SESSION_NUM;i++)
    {
        remove_session_obj(i);
    }
    
    ortp_exit();
    //ortp_global_stats_display();

    return 0;
}
