/*
 * RTP Session extensions.
 */
#include <ortp/ortp.h>
#include <stdio.h>

#include <ortp/rtpsession.h>
#include <ortp/rtcp.h>

extern mblk_t *rtcp_create_simple_bye_packet(uint32_t ssrc, const char *reason);
extern int ortp_rtcp_send (RtpSession * session, mblk_t * m);
extern int rtcp_sr_init(RtpSession *session, char *buf, int size);
extern int rtcp_rr_init(RtpSession *session, char *buf, int size);


/**
 *rtp_session_get_last_recv_time:
 *@session: #RtpSession to get last receive time from.
 *@tv:      Pointer to struct timeval to fill.
 *
 *  Gets last time a valid RTP or RTCP packet was received.
**/
void
rtp_session_get_last_recv_time(RtpSession *session, struct timeval *tv)
{
    *tv = session->last_recv_time;
}

/**
 *rtp_session_bye:
 *@session: #RtpSession to send RTCP BYE packet on.
 *@mode: One of the #RtpSessionMode flags.
 *
 *  Creates a new rtp session.
 *  If the session is able to send data (RTP_SESSION_SENDONLY or RTP_SESSION_SENDRECV), then a
 *  random SSRC number is choosed for the outgoing stream.
 *
 *Returns: the newly created rtp session.
**/
int
rtp_session_bye(RtpSession *session, const char *reason)
{
    mblk_t *cm;
    mblk_t *sdes = NULL;
    mblk_t *bye = NULL;
    int ret;

    /* Make a BYE packet (will be on the end of the compund packet). */
    bye = rtcp_create_simple_bye_packet(session->send_ssrc, reason);

    rtp_session_lock(session);

    /* SR or RR should be determined by the fact whether st was sent,
       not on the mode. But we follow the current ortplib logic. */
    if (session->mode==RTP_SESSION_SENDONLY
        || session->mode==RTP_SESSION_SENDRECV)
    {
        cm = allocb(sizeof(rtcp_sr_t), 0);
        cm->b_wptr += rtcp_sr_init(session,cm->b_wptr, sizeof(rtcp_sr_t));
        /* make a SDES packet */
        sdes = rtp_session_create_rtcp_sdes_packet(session);
        /* link them */
        concatb(concatb(cm, sdes), bye);
    } else {
        /* make a RR packet */
        cm = allocb(sizeof(rtcp_rr_t), 0);
        cm->b_wptr += rtcp_rr_init(session, cm->b_wptr, sizeof(rtcp_rr_t));
        /* link them */
        cm->b_cont = bye;
    }

    /* Send compound packet. */
    ret = ortp_rtcp_send(session, cm);
    rtp_session_unlock(session);

    return ret;
}
