
/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "ortp/port.h"
#include "utils.h"

void ortp_free(void* ptr){
	free(ptr);
}

/*
 * this method is an utility method that calls fnctl() on UNIX or
 * ioctlsocket on Win32.
 * int retrun the result of the system method
 */
int set_non_blocking_socket (ortp_socket_t sock)
{
	

#ifndef _WIN32
	return fcntl (sock, F_SETFL, O_NONBLOCK);
#else
	unsigned long nonBlock = 1;
	return ioctlsocket(sock, FIONBIO , &nonBlock);
#endif
}


/*
 * this method is an utility method that calls close() on UNIX or
 * closesocket on Win32.
 * int retrun the result of the system method
 */
int close_socket(ortp_socket_t sock){
#ifndef _WIN32
	return close (sock);
#else
	return closesocket(sock);
#endif
}



#ifndef _WIN32
	/* Use UNIX inet_aton method */
#else
	int inet_aton (const char * cp, struct in_addr * addr)
	{
		unsigned long retval;
		
		retval = inet_addr (cp);

		if (retval == INADDR_NONE) 
		{
			return -1;
		}
		else
		{
			addr->S_un.S_addr = retval;
			return 1;
		}
	}
#endif

char *ortp_strndup(const char *str,int n){
	int min=MIN((int)strlen(str),n)+1;
	char *ret=(char*)ortp_malloc(min);
	strncpy(ret,str,n);
	ret[min-1]='\0';
	return ret;
}

#ifdef _WIN32

int WIN_mutex_init(ortp_mutex_t *mutex, void *attr)
{	
	*mutex=CreateMutex(NULL, FALSE, NULL);
	return 0;
}

int WIN_mutex_lock(ortp_mutex_t * hMutex)
{
	WaitForSingleObject(*hMutex, INFINITE) == WAIT_TIMEOUT;
	return 0;
}

int WIN_mutex_unlock(ortp_mutex_t * hMutex)
{
	ReleaseMutex(*hMutex);
	return 0;
}

int WIN_mutex_destroy(ortp_mutex_t * hMutex)
{
	CloseHandle(*hMutex);
	return 0;
}

typedef struct thread_param{
	void * (*func)(void *);
	void * arg;
}thread_param_t;

static DWORD WINAPI thread_starter(void *data){
	thread_param_t *params=(thread_param_t*)data;
	void *ret=params->func(params->arg);
	ortp_free(data);
	return (DWORD)ret;
}


int WIN_thread_create(ortp_thread_t *th, void *attr, void * (*func)(void *), void *data)
{
    thread_param_t *params=ortp_new(thread_param_t,1);
    params->func=func;
    params->arg=data;
	*th=CreateThread( NULL, 0, thread_starter, params, 0, NULL);
	return 0;
}

int WIN_thread_join(ortp_thread_t hThread, void **unused)
{
	WaitForSingleObject(hThread, INFINITE) == WAIT_TIMEOUT;
	return 0;
}

int WIN_cond_init(ortp_cond_t *cond, void *attr)
{
	*cond=CreateEvent(NULL, FALSE, FALSE, NULL);
}

int WIN_cond_wait(ortp_cond_t* hCond, ortp_mutex_t * hMutex)
{
	//gulp: this is not very atomic ! bug here ?
	WIN_mutex_unlock(hMutex);	
	WaitForSingleObject(*hCond, INFINITE);
	WIN_mutex_lock(hMutex);
}

int WIN_cond_signal(ortp_cond_t * hCond)
{
	SetEvent(*hCond);
	return 0;
}

int WIN_cond_broadcast(ortp_cond_t * hCond)
{
	WIN_cond_signal(hCond);
	return 0;
}

int WIN_cond_destroy(ortp_cond_t * hCond)
{
	CloseHandle(*hCond);
	return 0;
}



int gettimeofday (struct timeval *tv, void* tz) 
{ 
	union 
	{ 
		__int64 ns100; /*time since 1 Jan 1601 in 100ns units */ 
		FILETIME fileTime; 
	} now; 

	GetSystemTimeAsFileTime (&now.fileTime); 
	tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL); 
	tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL); 
	return (0); 
} 

const char *getWinSocketError(int error)
{
	static char buf[80];

	switch (error)
	{
		case WSANOTINITIALISED: return "Windows sockets not initialized : call WSAStartup";
		case WSAEADDRINUSE:		return "Local Address already in use";
		case WSAEADDRNOTAVAIL:	return "The specified address is not a valid address for this machine";
		case WSAEINVAL:			return "The socket is already bound to an address.";
		case WSAENOBUFS:		return "Not enough buffers available, too many connections.";
		case WSAENOTSOCK:		return "The descriptor is not a socket.";
		case WSAECONNRESET:		return "Connection reset by peer";

		default :
			sprintf(buf, "Error code : %d", error);
			return buf;
		break;
	}

	return buf;
}

#ifdef _WORKAROUND_MINGW32_BUGS
char * WSAAPI gai_strerrorA(int errnum){
	 return (char*)getWinSocketError(errnum);
}
#endif

#endif

