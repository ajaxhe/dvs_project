#include "common.h" 
#include "session.h"
#include "videoEncode.h"
#include "audioEncode.h"
#include "rtpsend.h"

#include "server.h"
#include "osa_thr.h"
#include "uart_server.h"

#include <unistd.h>

#define NUM_OF_THREADS 4
#define ORTP_SESSION_THR 0
#define VIDEO_ENCODE_THR 1
#define AUDIO_ENCODE_THR 2
#define ALARM_INPUT_THR 3

ExitState g_exitState;
GlobalData gbl = {0};

typedef struct _threadMgr
{
    ThreadEnv   threadEnv;

	OSA_ThrHndl hThread[NUM_OF_THREADS];

}ThreadMgr;

ThreadMgr g_threadMgr;

// funntions 
static int start_all_threads();
static void stop_all_threads();
void* ssh_interact_thr(void* arg);

int main()
{
	gblSetExitState(DVS_WORKING);
	// 设置当前设备的视频通道数;
	gblSetVideoChannels(16);
	//gblSetVideoChannels(8);

	// init ortp interface
	rtp_init();
	
	// tcp listen thread create
	if (tcp_listen_thread_start(LOCAL_TCP_PORT) != 0)
	{
		printf("[main] open tcp listen thread failed!\n");
		return -1;
	}
	// uart tcp listen thread create
	if (uart_tcp_listen_thread_start(UART_LOCAL_TCP_PORT) != 0)
	{
		printf("[main] open uart tcp listen thread failed!\n");
		return -1;
	}

	OSA_ThrHndl sshThrHandle;
	if (OSA_thrCreate(&sshThrHandle,ssh_interact_thr,OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,NULL))
	{
		printf("open ssh interact thread failed\n");
		return -1;
	}

	if (start_all_threads() < 0)
		return -1;

	while ( gblGetExitState() == DVS_WORKING )
	{
		sleep(1);
	}

	stop_all_threads();

	// close ortp interface
	rtp_exit();

	// close tcp listen thread
	clean_tcp_listen_thread();

	// close uart tcp listen thread
	clean_uart_tcp_listen_thread();
	

	return gblGetExitState();
}

int start_all_threads()
{
	// Open the object which synchronizes the thread initialization and clean
	Rendezvous_open(&g_threadMgr.threadEnv.rendezvousInit, NUM_OF_THREADS);
	Rendezvous_open(&g_threadMgr.threadEnv.rendezvousCleanup, NUM_OF_THREADS);

	printf("NUM_OF_THREADS=%d\n", NUM_OF_THREADS);
	
	// create net session thread
	if( OSA_thrCreate(&g_threadMgr.hThread[ORTP_SESSION_THR],sessionThrFxn,OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,&g_threadMgr.threadEnv) )
	{   
		printf("session pthread_create fail!\n");
		return -1; 
	} 

	// create video encode thread
	if( OSA_thrCreate(&g_threadMgr.hThread[VIDEO_ENCODE_THR], videoEncodeThrFxn,OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,&g_threadMgr.threadEnv) )
	{   
		printf("video encode pthread_create fail!\n");
		return -1; 
	} 

	// create audio encode thread
	if( OSA_thrCreate(&g_threadMgr.hThread[AUDIO_ENCODE_THR], audioEncodeThrFxn,OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,&g_threadMgr.threadEnv) )
	{   
		printf("audio encode pthread_create fail!\n");
		return -1; 
	} 

	// create alarm input thread
	if ( OSA_thrCreate(&g_threadMgr.hThread[ALARM_INPUT_THR], alarmInputThrFxn, OSA_THR_PRI_DEFAULT,OSA_THR_STACK_SIZE_DEFAULT,&g_threadMgr.threadEnv) )
	{   
		printf("alarm input thread create failed\n");  
		return  -1; 
	}  

	return 0;
}

void stop_all_threads()
{
	gblSetQuit();

	int i;
	for (i = 0; i < NUM_OF_THREADS; i++)
	{
		if (i == ALARM_INPUT_THR)
			OSA_thrDelete(&g_threadMgr.hThread[i]);
		else
			OSA_thrJoin(&g_threadMgr.hThread[i]);
		printf("[main] join threads: %d\n", i);
	}
}

void* ssh_interact_thr(void* arg)
{
	getchar();
	getchar();
	printf("[dvs-exit] \n");
	gblSetExitState(DVS_EXIT_SUCCESS);
	OSA_thrExit(NULL);
}
