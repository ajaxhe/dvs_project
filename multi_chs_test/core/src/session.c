#include "common.h"
#include "rtpsend.h"
#include "session.h"
#include "osa_thr.h"

#include <pthread.h>

FifoUtil_Obj g_NetSessionFifos[MAX_VIDEO_CHANNEL / CHS_PER_SESSION];
FifoUtil_Obj g_NetBackSessionFifo;

static OSA_ThrHndl subSessionThreadHndls[MAX_VIDEO_CHANNEL / CHS_PER_SESSION];

#define MAX_CLIENT_NUM 10

typedef struct sessionPair
{
    int videoIDs[MAX_VIDEO_CHANNEL/CHS_PER_SESSION];
    int audioIDs[MAX_VIDEO_CHANNEL/CHS_PER_SESSION];
	int channels;
}SessionPair;

typedef struct _clientObj
{
    int clientID;
    BOOL flag[MAX_VIDEO_CHANNEL];
    SessionPair sessionPair;
}ClientObj;

typedef ClientObj* ClientHndl;

typedef struct _clientMgr
{
    ClientObj clients[MAX_CLIENT_NUM];
}ClientMgr;

ClientMgr g_clients;

int open_channel( int clientID, int chId )
{
    ClientObj *client = &g_clients.clients[clientID];
    if( client->clientID == -1 )
    {
        return -1;
    }

    if( chId >= MAX_VIDEO_CHANNEL)
    {
        memset(client->flag, TRUE, MAX_VIDEO_CHANNEL);
    }
    else
    {
        client->flag[chId] = TRUE;
    }
	
    return -1;            
}

int close_channel( int clientID, int chId )
{
    ClientObj *client = &g_clients.clients[clientID];
    if( client->clientID == -1 )
    {
        return -1;
    }

    if( chId >= MAX_VIDEO_CHANNEL)
    {
        memset(client->flag, FALSE, MAX_VIDEO_CHANNEL);
    }
    else
    {
        client->flag[chId] = FALSE;
    }
	
    return -1;
}

BOOL check_channel( int clientID, int chId)
{
    ClientObj *client = &g_clients.clients[clientID];
    if( client->clientID == -1 )
    {
        return -1;
    }

    return client->flag[chId];
}

int add_client( char *destIp, int base_video_port, int base_audio_port )
{
    int id, i, chs;
    ClientObj *client = NULL;

	chs = gblGetVideoChannels();
	
    for (id = 0; id < MAX_CLIENT_NUM; id++)
    {
        if( g_clients.clients[id].clientID == -1)
        {
	    client = &g_clients.clients[id];
	    break;
        }
    }
    if (id >= MAX_CLIENT_NUM) return -1;
	
	int video_port, audio_port;
    // create rtp video session
    if( base_video_port > 0)
    {
		for (i = 0; i < chs / CHS_PER_SESSION; i++)
		{
			video_port = base_video_port - i * 2;
        	client->sessionPair.videoIDs[i] = create_rtp_session(destIp, video_port, SESSION_TYPE_VIDEO);
			if( client->sessionPair.videoIDs[i] < 0 )
        	{
            	return -2;
			}
		}
    }

    // create rtp audio session
    if( base_audio_port > 0)
    {
		for (i = 0; i < chs / CHS_PER_SESSION; i++)
		{
			audio_port = base_audio_port + i * 2;
        	client->sessionPair.audioIDs[i] = create_rtp_session(destIp, audio_port, SESSION_TYPE_AUDIO);
        	if( client->sessionPair.audioIDs[i] < 0 )
			{
            	return -3;
			}
		}
    }

    // alloc client ID
    client->clientID = id;

    memset(client->flag,FALSE,MAX_VIDEO_CHANNEL);
	
    return client->clientID;
}

int del_client( int clientId )
{
	int i, chs;
    ClientObj *client = &g_clients.clients[clientId];
    if( client->clientID == -1 )
    {
        return -1;
    }

	chs = gblGetVideoChannels();
	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
    	remove_session_obj(client->sessionPair.videoIDs[i]);
    	remove_session_obj(client->sessionPair.audioIDs[i]);
	}

    client->clientID = -1;
	memset(g_clients.clients[i].sessionPair.videoIDs, -1, MAX_VIDEO_CHANNEL / CHS_PER_SESSION * sizeof(int));
	memset(g_clients.clients[i].sessionPair.audioIDs, -1, MAX_VIDEO_CHANNEL / CHS_PER_SESSION * sizeof(int));

    return 0;
}


int init_session_tsk()
{
    int i;
	
    for( i = 0; i < MAX_CLIENT_NUM; i++)
    {
        g_clients.clients[i].clientID = -1;
		memset(g_clients.clients[i].sessionPair.videoIDs, -1, MAX_VIDEO_CHANNEL / CHS_PER_SESSION * sizeof(int));
		memset(g_clients.clients[i].sessionPair.audioIDs, -1, MAX_VIDEO_CHANNEL / CHS_PER_SESSION * sizeof(int));
    }
	
	int chs;
	chs = gblGetVideoChannels();
	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
    	// open capture Fifo
    	if( FifoUtil_open(&g_NetSessionFifos[i],sizeof(SessionElement))==FIFOUTIL_FAILURE )
    	{
        	printf("FifoUtil_open g_NetSessionFifois[%d] fail !\n", i);
        	return -1;
    	}
	}

    if( FifoUtil_open(&g_NetBackSessionFifo,sizeof(SessionElement))==FIFOUTIL_FAILURE )
    {
        printf("FifoUtil_open g_NetBackSessionFifo fail !\n");
        return -1;
    }

    return 0;
}

int delete_session_tsk( ThreadEnv *envp  )
{
    int i;    
	
    for( i=0; i<MAX_CLIENT_NUM; i++)
    {
        del_client(i);
    }

    // Make sure the other threads aren't waiting for init to complete
    Rendezvous_force(&envp->rendezvousInit);
    printf("meet in %s\n", __func__);
    // Meet up with other threads before cleaning up
    //Rendezvous_meet(&envp->rendezvousCleanup);

	int chs;
	chs = gblGetVideoChannels();
	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
    	// close fifo
    	FifoUtil_close(&g_NetSessionFifos[i]);
	}

    return 0; 
}

int send_to_client( SessionElement * arg )
{
    int i;

    ClientObj *client;

    for (i=0 ;i<MAX_CLIENT_NUM; i++)
    {
        client = &g_clients.clients[i];

        if( client->clientID == -1)
        {
            continue;
        }

		// to determine whether the channel is opened
        if( check_channel(client->clientID,arg->chId) == FALSE)
        {
            continue;
        }

		int s_id;
		s_id = arg->chId / CHS_PER_SESSION;

        if( arg->dataType == SESSION_TYPE_VIDEO )
        {
            rtp_send(client->sessionPair.videoIDs[s_id], arg->buffer, arg->len);
        }
        else
        {
            rtp_send(client->sessionPair.audioIDs[s_id], arg->buffer, arg->len);
        }
    }

    return 0;
}

void *subSessionThrFxn(void *arg)
{
	int id;
	id = *(int *)arg;

	SessionElement sessionElement;

    while( !gblGetQuit() )
    {
		if( FifoUtil_get(&g_NetSessionFifos[id], &sessionElement) == FIFOUTIL_FAILURE )
        {
            printf(" FifoUtil_get Fail, encode !\n");
            break;
        }

		if( sessionElement.command == CMD_SESSION_FLUSH )
		{
	    	break;
		}
	
        send_to_client(&sessionElement);
		FrameHeader* header = (FrameHeader*)sessionElement.buffer;
		//printf("sesionThr id: %d, channel id: %d, get H.264 frame, chId=%d\n", id, header->chId, sessionElement.chId);
		//usleep(10000);
	}

	OSA_thrExit(NULL);
}

void *sessionThrFxn( void *arg )
{
    ThreadEnv *envp = (ThreadEnv*)arg;
    SessionElement sessionElement;

    if( init_session_tsk() != 0 )
    {
        printf("[ajaxhe-debug-error] init_session_tsk failed\n");
        return THREAD_FAILURE;
    }

    //Signal that initialization is done and wait for other threads
    Rendezvous_meet(&envp->rendezvousInit);

    printf("[session] start\n");

	// create subsession threads
	int i, chs;
	chs = gblGetVideoChannels();

	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
		OSA_thrCreate(&subSessionThreadHndls[i], subSessionThrFxn, OSA_THR_PRI_DEFAULT, OSA_THR_STACK_SIZE_DEFAULT, &i);	
		usleep(10000);
	} 

	for (i = 0; i < chs / CHS_PER_SESSION; i++)
	{
		OSA_thrJoin(&subSessionThreadHndls[i]);
		printf("[subSession] join thread: %d\n", i);
	}
/*
    while( !gblGetQuit() )
    {
		if( FifoUtil_get(&g_NetSessionFifo,&sessionElement) == FIFOUTIL_FAILURE )
        {
            printf(" FifoUtil_get Fail, encode !\n");
            break;
        }

		if( sessionElement.command == CMD_SESSION_FLUSH )
		{
	    	break;
		}
	
        send_to_client(&sessionElement);
		printf("get H.264 frame, chId=%d\n", sessionElement.chId);

		//FifoUtil_put(&g_NetBackSessionFifo, &sessionElement);
    }
*/
	delete_session_tsk(envp);
	printf("[session] exit\n");

	OSA_thrExit(NULL);
}

/*
int destroyRecvSession(ThreadEnv* envp)
{
    FifoUtil_close(&g_NetRecvSessionFifo);

    //OSA_cmemFree(g_NetRecvBuf);
    free(g_NetRecvBuf);

    //printf("in %s, before meet\n", __func__);

    // Meet up with other threads before cleaning up
    Rendezvous_meet(&envp->rendezvousCleanup);

    OSA_thrExit(NULL);
}

void *videoOuputRecvThrFxn(void *arg)
{
    ThreadEnv* envp = (ThreadEnv *)arg;
    SessionElement sessionElement;

    int err;
    err = pthread_mutex_init(&g_counter_mutex, NULL);
    if (err)
    {
        printf("[video_output_error] Init g_counter_mutex failed!\n");
        return THREAD_FAILURE;
    }

    if (initRecvSession() < 0)
    {
        printf("[video_output_error] initRecvSession Failed!\n");
        gblSetQuit();
        return THREAD_FAILURE;
    }

    //Signal that initialization is done and wait for other threads
    Rendezvous_meet(&envp->rendezvousInit);

    printf("[video_output_info] video_output recv session start\n");
    while (!gblGetQuit())
    {
        int res;

        int start_time = OSA_getCurTimeInMsec();

        res = recvFromServer(&g_rtpRecvSessionMgr, g_NetRecvBuf);
        g_rtpRecvSessionMgr.curTimeStamp += g_rtpRecvSessionMgr.timeStampInc;


        if (res > 0)
        {
            FrameHeader *fHeader = (FrameHeader *)g_NetRecvBuf;

            pthread_mutex_lock(&g_counter_mutex);
            if (g_counter > 0)
            {
                pthread_mutex_unlock(&g_counter_mutex);
                continue;
            }    
            g_counter++;
            pthread_mutex_unlock(&g_counter_mutex);

            static int recv_frame_count = 0;
            recv_frame_count++;

            //printf("[rtp_recv_session] frame count:%d, recv size:%d\n", recv_frame_count, res);
            
            sessionElement.len = res;
            sessionElement.buffer = g_NetRecvBuf + sizeof(FrameHeader);

            if( FifoUtil_put(&g_NetRecvSessionFifo, &sessionElement) == FIFOUTIL_FAILURE )
            {
                printf("[error] g_NetRecvSessionFifo put failed!\n");
                break;
            }

            int end_time = OSA_getCurTimeInMsec();

            //printf("[rtp_recv_session] recv time gap is: %d\n", end_time-start_time);

        }

    }

    destroyRecvSession(envp);

     pthread_mutex_destroy(&g_counter_mutex);
}
*/
