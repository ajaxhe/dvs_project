//
// C Interface: port
//
// Description: 
//
//
// Author: Simon Morlat <simon.morlat@linphone.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
/* this file is responsible of the portability of the stack */

#ifndef ORTP_PORT_H
#define ORTP_PORT_H


#ifndef _WIN32
/********************************/
/* definitions for UNIX flavour */
/********************************/

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __linux
#include <stdint.h>
#endif


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(_XOPEN_SOURCE_EXTENDED) || !defined(__hpux)
#include <arpa/inet.h>
#endif



#include <sys/time.h>

#ifdef ORTP_INET6
#include <netdb.h>
#endif

typedef int ortp_socket_t;
typedef pthread_t ortp_thread_t;
typedef pthread_mutex_t ortp_mutex_t;
typedef pthread_cond_t ortp_cond_t;

#define ortp_thread_create	pthread_create
#define ortp_thread_join	pthread_join
#define ortp_mutex_init		pthread_mutex_init
#define ortp_mutex_lock		pthread_mutex_lock
#define ortp_mutex_unlock	pthread_mutex_unlock
#define ortp_mutex_destroy	pthread_mutex_destroy
#define ortp_cond_init		pthread_cond_init
#define ortp_cond_signal	pthread_cond_signal
#define ortp_cond_broadcast	pthread_cond_broadcast
#define ortp_cond_wait		pthread_cond_wait
#define ortp_cond_destroy	pthread_cond_destroy

#define SOCKET_OPTION_VALUE	void *
#define SOCKET_BUFFER		void *

#define getSocketError() strerror(errno)
#define getSocketErrorCode() (errno)

#else
/*********************************/
/* definitions for WIN32 flavour */
/*********************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma push_macro("_WINSOCKAPI_")
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <windows.h>

#ifdef _MSC_VER
typedef  unsigned __int64 uint64_t;
typedef  __int64 int64_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;
typedef  int int32_t;
typedef  unsigned char uint8_t;
#else
#include <stdint.h> /*provided by mingw32*/
#endif

#define vsnprintf	_vsnprintf
#define srandom		srand
#define random		rand


typedef SOCKET ortp_socket_t;
typedef HANDLE ortp_cond_t;
typedef HANDLE ortp_mutex_t;
typedef HANDLE ortp_thread_t;

#define ortp_thread_create	WIN_thread_create
#define ortp_thread_join	WIN_thread_join
#define ortp_mutex_init		WIN_mutex_init
#define ortp_mutex_lock		WIN_mutex_lock
#define ortp_mutex_unlock	WIN_mutex_unlock
#define ortp_mutex_destroy	WIN_mutex_destroy
#define ortp_cond_init		WIN_cond_init
#define ortp_cond_signal	WIN_cond_signal
#define ortp_cond_broadcast	WIN_cond_broadcast
#define ortp_cond_wait		WIN_cond_wait
#define ortp_cond_destroy	WIN_cond_destroy

int WIN_mutex_init(ortp_mutex_t *m, void *attr_unused);
int WIN_mutex_lock(ortp_mutex_t *mutex);
int WIN_mutex_unlock(ortp_mutex_t *mutex);
int WIN_mutex_destroy(ortp_mutex_t *mutex);
int WIN_thread_create(ortp_thread_t *t, void *attr_unused, void *(*func)(void*), void *arg); 
int WIN_thread_join(ortp_thread_t thread, void **unused);
int WIN_cond_init(ortp_cond_t *cond, void *attr_unused);
int WIN_cond_wait(ortp_cond_t * cond, ortp_mutex_t * mutex);
int WIN_cond_signal(ortp_cond_t * cond);
int WIN_cond_broadcast(ortp_cond_t * cond);
int WIN_cond_destroy(ortp_cond_t * cond);


#define SOCKET_OPTION_VALUE	char *
#define inline			__inline

const char *getWinSocketError(int error);
#define getSocketErrorCode() WSAGetLastError()
#define getSocketError() getWinSocketError(WSAGetLastError())

#define snprintf _snprintf
#define strcasecmp _stricmp

#if 0
struct timeval {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
};
#endif

int gettimeofday (struct timeval *tv, void* tz);


#endif

typedef unsigned char bool_t;
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#ifdef __cplusplus
extern "C"{
#endif

#define ortp_malloc(sz)		malloc(sz)
void ortp_free(void *ptr);
#define ortp_realloc(ptr,sz)	realloc(ptr,sz)

static inline void * ortp_malloc0(int size){
	void *ptr=malloc(size);
	memset(ptr,0,size);
	return ptr;
}

#define ortp_new(type,count)	ortp_malloc(sizeof(type)*(count))
#define ortp_new0(type,count)	ortp_malloc0(sizeof(type)*(count))

int close_socket(ortp_socket_t sock);
int set_non_blocking_socket(ortp_socket_t sock);

#define ortp_strdup strdup
char *ortp_strndup(const char *str,int n);
char *ortp_strdup_printf(const char *fmt,...);


#ifdef __cplusplus
}
#endif


#endif


