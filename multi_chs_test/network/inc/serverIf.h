#ifndef SERVERIF_H
#define SERVERIF_H

#include <osa_thr.h>

#define TCP_THREAD_STACK_SIZE    (10*1024)
#define CLIENTNUM 20  //temp

// �ͻ���socket��Ϣ
typedef struct _ClientEnv
{
	int m_Index;
	int   m_Socket;
	OSA_ThrHndl m_Thr;
    char  m_Ip[20];
}ClientEnv;

// ������socket��Ϣ
typedef struct _ServerEnv
{
	int m_Socket;
	OSA_ThrHndl m_ListenThr;
	int m_clientCount;
	ClientEnv m_Client[CLIENTNUM]; 
}ServerEnv;

// ��������
int add_new_tcp_process_thr( ClientEnv * enp );

#endif //SERVERIF_H
