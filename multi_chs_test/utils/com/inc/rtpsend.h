//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, HUST CBIB Group  （copyright statement）
/// All rights reserved.
/// 
/// @file    rtpsend.h
/// @brief   RTP send declare file
///
/// the interface of rtp send module
///
/// @version 1.0	 
/// @author  lujun
/// @date    2010/11/10
///
///
/// revision note:
//////////////////////////////////////////////////////////////////////////

#ifndef _RTPSEND_H
#define _RTPSEND_H

#include "ortp/ortp.h"

/* the payload type define */
#define PAYLOAD_TYPE_VIDEO 34
#define PAYLOAD_TYPE_AUDIO 12 

#define CHS_PER_SESSION 4
#define MAX_SESSION_NUM   32

/* RTP video Send time stamp increase */
#define VIDEO_TIME_STAMP_INC_STD 3600  // standard increame
#define VIDEO_TIME_STAMP_INC (VIDEO_TIME_STAMP_INC_STD / CHS_PER_SESSION)

/* RTP audio Send time stamp increase */
/* if audio sampling rate is 8KHz,then timestamp inc is 160*/
/* if audio sampling rate is 16KHz,then timestamp inc is 320*/
#define AUDIO_TIME_STAMP_INC_STD 320
#define AUDIO_TIME_STAMP_INC (AUDIO_TIME_STAMP_INC_STD / CHS_PER_SESSION) 

/* RTP Session TYPE ,video or audio */
#define SESSION_TYPE_VIDEO 0
#define SESSION_TYPE_AUDIO 1

typedef unsigned char uint8_t;
typedef unsigned int  uint32_t;

/** 帧包头的标识长度 */ 
#define CMD_HEADER_LEN 10

/** 帧包头的定义 */
static uint8_t CMD_HEADER_STR[CMD_HEADER_LEN] = { 0xAA,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xFF };

#pragma pack( push, 1 )

/** 帧的包头信息 */
typedef struct _sFrameHeader
{
    /** 命令名称标识 */ 
    unsigned char cmdHeader[CMD_HEADER_LEN];

    /** 采集的通道号 0~7*/
    unsigned char chId;

    /** 数据类型，音频 或者 视频*/
    unsigned char dataType; 

    /** 缓冲区的数据长度 */ 
    uint32_t len;

    /** 时间戳 */
    uint32_t timestamp;

}FrameHeader;

#pragma pack( pop )

/** 包头的长度 */
#define FRAME_HEADER_LEN sizeof(FrameHeader)

/** init rtp related Resources
 * 
 *  @param:  void
 *  @return: void
 *  @note:	 call before you use rtp func
 *  @see:	 
 */
void rtp_init();

/** create a rtp session
 * 
 *  @param:  ipStr, dest IP address str
 *  @param:  port, dest RTP session port
 *  @param:  sessionType, SESSION_TYPE_VIDEO or SESSION_TYPE_AUDIO
 *  @return: rtp session ID, is a handle to use rtpSend
 *           \n if return -1, illustrate create fail
 *  @note:	 
 *  @see:    rtpSend
 */
int create_rtp_session(char * ipStr,int port,int sessionType);

/** send rtp packets
 * 
 *  @param:  sessionID,the rtp session ID
 *           \n you can get it in a call of createRtpSession
 *  @param:  buffer, the data buffer you want to send
 *  @param:  len, the data length you want to send, in bytes
 *  @return: if send successfully , return 0
 *           \n if return -1, illustrate create fail
 *  @note:	 
 *  @see:     createRtpSession
 */
int rtp_send( int sessionID, char *buffer, int len );

/** deinit rtp related Resources
 * 
 *  @param:  void
 *	@return: if 0, exit successfully
 *	@note:	 call when you donnot use rtp module,or when sys exit
 *	@see:	 
 */
int rtp_exit();

/** remove rtp session with the sessionID
 * 
 *  @param:  int sessionID rtpsessionID
 *  @return: if 0, exit successfully
 *  @note:   call when you donnot use rtp session
 *  @see:	 
 */
int remove_session_obj( int sessionID );


#endif /* _RTPSEND_H */

