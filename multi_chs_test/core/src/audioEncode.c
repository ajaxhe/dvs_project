#include "audioEncode.h"
#include "common.h"

void* audioEncodeThrFxn(void *arg)
{
	ThreadEnv* envp = arg;
	//printf("In %s\n", __func__);	
	
	Rendezvous_meet(&envp->rendezvousInit);	
	printf("[audio-encode] start\n");

	while ( !gblGetQuit() )
	{
		usleep(10000);
	}
	printf("[audio-encode] exit\n");
	OSA_thrExit(NULL);
}
