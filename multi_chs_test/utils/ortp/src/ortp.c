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


#include "ortp/ortp.h"
#include "ortp-config.h"
#include "scheduler.h"

rtp_stats_t ortp_global_stats;

#ifdef ENABLE_MEMCHECK
int ortp_allocations=0;
#endif


RtpScheduler *__ortp_scheduler;



extern void av_profile_init(RtpProfile *profile);

static void init_random_number_generator(){
	struct timeval t;
	gettimeofday(&t,NULL);
	srandom(t.tv_usec+t.tv_sec);
}

/**
 *ortp_init:
 *
 *	Initialize the oRTP library. You should call this function first before using
 *	oRTP API.
**/
 
void ortp_init()
{
	static bool_t initialized=FALSE;
	if (initialized) return;
	initialized=TRUE;

	av_profile_init(&av_profile);
	ortp_global_stats_reset();
	init_random_number_generator();
	ortp_message("oRTP-" ORTP_VERSION " initialized.");
}


/**
 *ortp_scheduler_init:
 *
 *	Initialize the oRTP scheduler. You only have to do that if you intend to use the
 *	scheduled mode of the #RtpSession in your application.
 *	
**/
void ortp_scheduler_init()
{
	static bool_t initialized=FALSE;
	if (initialized) return;
	initialized=TRUE;
#ifdef __hpux
	/* on hpux, we must block sigalrm on the main process, because signal delivery
	is ?random?, well, sometimes the SIGALRM goes to both the main thread and the 
	scheduler thread */
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);
	sigprocmask(SIG_BLOCK,&set,NULL);
#endif /* __hpux */

	__ortp_scheduler=rtp_scheduler_new();
	rtp_scheduler_start(__ortp_scheduler);
	//sleep(1);
}


/**
 *ortp_exit:
 *
 * Gracefully uninitialize the library, including shutdowning the scheduler if it was started.
 *	
**/
void ortp_exit()
{
	if (__ortp_scheduler!=NULL)
	{
		rtp_scheduler_destroy(__ortp_scheduler);
		__ortp_scheduler=NULL;
	}
}

/**
 *ortp_get_scheduler:
 *
 *	Returns a pointer to the scheduler, NULL if it was not running.
 *	The application developer should have to call this function.
 *
 *Returns: a pointer to the scheduler.
**/
RtpScheduler * ortp_get_scheduler()
{
	if (__ortp_scheduler==NULL) ortp_error("Cannot use the scheduled mode: the scheduler is not "
									"started. Call ortp_scheduler_init() at the begginning of the application.");
	return __ortp_scheduler;
}


static FILE *__log_file=0;

/**
 *ortp_set_log_file:
 *@file: a FILE pointer where to output the ortp logs.
 *
**/
void ortp_set_log_file(FILE *file)
{
	__log_file=file;
}

static void __ortp_logv_out(OrtpLogLevel lev, const char *fmt, va_list args);

OrtpLogFunc ortp_logv_out=__ortp_logv_out;

/**
 *ortp_set_log_handler:
 *@func: your logging function, compatible with the OrtpLogFunc prototype.
 *
**/
void ortp_set_log_handler(OrtpLogFunc func){
	ortp_logv_out=func;
}


unsigned int __ortp_log_mask=ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL;

void ortp_set_log_level_mask(int levelmask){
	__ortp_log_mask=levelmask;
}

static char * _strdup_vprintf(const char *fmt, va_list ap)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	char *p;
	if ((p = (char *) ortp_malloc (size)) == NULL)
		return NULL;
	while (1)
	{
		/* Try to print in the allocated space. */
		n = vsnprintf (p, size, fmt, ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		//printf("Reallocing space.\n");
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		if ((p = (char *) ortp_realloc (p, size)) == NULL)
			return NULL;
	}
}

char *ortp_strdup_printf(const char *fmt,...){
	char *ret;
	va_list args;
	va_start (args, fmt);
	ret=_strdup_vprintf(fmt, args);
 	va_end (args);
	return ret;
}

#ifdef _WIN32
#define ENDLINE "\r\n"
#else
#define ENDLINE "\n"
#endif


static void __ortp_logv_out(OrtpLogLevel lev, const char *fmt, va_list args){
	const char *lname="undef";
	char *msg;
	if (__log_file==NULL) __log_file=stderr;
	switch(lev){
		case ORTP_DEBUG:
			lname="debug";
			break;
		case ORTP_MESSAGE:
			lname="message";
			break;
		case ORTP_WARNING:
			lname="warning";
			break;
		case ORTP_ERROR:
			lname="error";
			break;
		case ORTP_FATAL:
			lname="fatal";
			break;
		default:
			ortp_fatal("Bad level !");
	}
	msg=_strdup_vprintf(fmt,args);
	fprintf(__log_file,"ortp-%s-%s" ENDLINE,lname,msg);
	ortp_free(msg);
}


void ortp_global_stats_display()
{
	rtp_stats_display(&ortp_global_stats,"Global statistics");
#ifdef ENABLE_MEMCHECK	
	printf("Unfreed allocations: %i\n",ortp_allocations);
#endif
}


void rtp_stats_display(rtp_stats_t *stats, const char *header)
{

	ortp_log(ORTP_MESSAGE,
			"oRTP-stats:\n   %s :\n"
	      " number of rtp packet sent=%lld\n"
	      " number of rtp bytes sent=%lld bytes\n"
	      " number of rtp packet received=%lld\n"
	      " number of rtp bytes received=%lld bytes\n"
	      " number of incoming rtp bytes successfully delivered to the application=%lld \n"
	      " number of times the application queried a packet that didn't exist=%lld \n"
	      " number of rtp packets received too late=%lld\n"
		  " number of rtp packets skipped=%lld\n"
	      " number of bad formatted rtp packets=%lld\n"
	      " number of packet discarded because of queue overflow=%lld\n",
	      header,
	      (long long)stats->packet_sent,
	      (long long)stats->sent,
	      (long long)stats->packet_recv,
	      (long long)stats->hw_recv,
	      (long long)stats->recv,
	      (long long)stats->unavaillable,
	      (long long)stats->outoftime,
		  (long long)stats->skipped,
	      (long long)stats->bad,
	      (long long)stats->discarded);
}

void ortp_global_stats_reset(){
	memset(&ortp_global_stats,0,sizeof(rtp_stats_t));
}

rtp_stats_t *ortp_get_global_stats(){
	return &ortp_global_stats;
}

void rtp_stats_reset(rtp_stats_t *stats){
	memset((void*)stats,0,sizeof(rtp_stats_t));
}


/**
 *ortp_min_version_required:
 *@major: 
 *@minor: 
 *@micro:
 *
 * This function give the opportunity to programs to check if the libortp they link to
 * has the minimum version number they need.
 *
 * Returns: true if ortp has a version number greater or equal than the required one.
**/
bool_t ortp_min_version_required(int major, int minor, int micro){
	return ((major*1000000) + (minor*1000) + micro) <= 
		   ((ORTP_MAJOR_VERSION*1000000) + (ORTP_MINOR_VERSION*1000) + ORTP_MICRO_VERSION);
}
