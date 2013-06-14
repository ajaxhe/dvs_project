/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc1889) stack.
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


#ifndef RTP_H
#define RTP_H

#include <ortp/port.h>

#define IPMAXLEN 20
#define UDP_MAX_SIZE 65536
#define RTP_FIXED_HEADER_SIZE 12
#define RTP_DEFAULT_JITTER_TIME 80	/*miliseconds*/



typedef struct rtp_header
{
#ifdef ORTP_BIGENDIAN
	uint16_t version:2;
	uint16_t padbit:1;
	uint16_t extbit:1;
	uint16_t cc:4;
	uint16_t markbit:1;
	uint16_t paytype:7;
#else
	uint16_t cc:4;
	uint16_t extbit:1;
	uint16_t padbit:1;
	uint16_t version:2;
	uint16_t paytype:7;
	uint16_t markbit:1;
#endif
	uint16_t seq_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[16];
} rtp_header_t;




typedef struct rtp_stats
{
	uint64_t packet_sent;
	uint64_t sent;		/* bytes sent */
	uint64_t recv; 		/* bytes of payload received and delivered in time to the application */
	uint64_t hw_recv;		/* bytes of payload received */
	uint64_t packet_recv;	/* number of packets received */
	uint64_t unavaillable;	/* packets not availlable when they were queried */
	uint64_t outoftime;		/* number of packets that were received too late */
	uint64_t skipped;		/* number of packets skipped (that the application never queried 
											or that need to be skipped in order to compensate a clock slide (see adaptive jitter control)) */
	uint64_t cum_packet_loss; /* cumulative number of packet lost */
	uint64_t bad;			/* packets that did not appear to be RTP */
	uint64_t discarded;		/* incoming packets discarded because the queue exceeds its max size */
} rtp_stats_t;

#define RTP_TIMESTAMP_IS_NEWER_THAN(ts1,ts2) \
	((uint32_t)((uint32_t)(ts1) - (uint32_t)(ts2))< (uint32_t)(1<<31))

#define RTP_TIMESTAMP_IS_STRICTLY_NEWER_THAN(ts1,ts2) \
	( ((uint32_t)((uint32_t)(ts1) - (uint32_t)(ts2))< (uint32_t)(1<<31)) && (ts1)!=(ts2) )

#define TIME_IS_NEWER_THAN(t1,t2) RTP_TIMESTAMP_IS_NEWER_THAN(t1,t2)

#define TIME_IS_STRICTLY_NEWER_THAN(t1,t2) RTP_TIMESTAMP_IS_STRICTLY_NEWER_THAN(t1,t2)

#endif
