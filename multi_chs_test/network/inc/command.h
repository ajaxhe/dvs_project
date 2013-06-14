//////////////////////////////////////////////////////////////////////////
//  COPYRIGHT NOTICE
//  Copyright (c) 2009, 华中科技大学CBIB实验室（版权声明）
//  All rights reserved.
// 
/// @file    Command.h  
/// @brief   命令包声明文件
///
/// 定义各种TCP命令包以及相关结构体
///
/// @version 1.0   
/// @author  lujun 
/// @date    2011/05/27
//
//
//  修订说明：
//////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_H_
#define COMMAND_H_
 
typedef unsigned char uint8_t;

/** IP地址的长度 */
#define MAX_IP_ADDR_LEN 20

/** 每次最大发送包大小 */
#define MAX_PACKET_SIZE 65536

/** 超时时间为5000ms */
#define TIME_OUT_DELAY 5000

/** TCP命令包头的长度 */ 
#define TCP_CMD_HEADER_LEN 8

/** 云台控制命令字的最大长度 */ 
#define MAX_PTZ_CMD_LEN 32

// cmd header str
static uint8_t TCP_CMD_HEADER_STR[TCP_CMD_HEADER_LEN] = { 0xAA,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xFF };

// 报警输入状态包包头
static uint8_t ALARM_HEADER_STR[TCP_CMD_HEADER_LEN] = { 0x00,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xFF };

/** 视频分辨率 */ 
typedef enum _EResolution
{
	DVS_RESOLUTION_CIF = 0,
	DVS_RESOLUTION_D1

}EResolution;

/** 动态码率时设置的码率值 */ 
#define DVS_BITRATE_VARAY 0

/** 
 *  可以发送给服务器的命令
 */
typedef enum _ServerCMD
{
	DVS_CMD_RESET_BOARD,        /** 重启下位机程序 */

	DVS_CMD_ADD_CLIENT,         /** 添加一个新的客户端 */

	DVS_CMD_DEL_CLIENT,         /** 删除一个已存在的客户端 */

	DVS_CMD_OPEN_CHANNEL,       /** 打开指定的通道 */

	DVS_CMD_CLOSE_CHANNEL,      /** 关闭指定的通道 */

    DVS_CMD_GET_RESOLUTION,     /** 获取视频的分辨率 */ 

	DVS_CMD_SET_RESOLUTION,     /** 设置视频的分辨率 */

	DVS_CMD_SET_BITRATE,        /** 设置固定码率 */

	DVS_CMD_ON_ALARMOUT,        /** 打开指定通道的报警输出**/
	DVS_CMD_OFF_ALARMOUT,       /** 关闭指定通道的报警输出**/
	DVS_CMD_GET_ALARMOUT,		/** 获取指定通道的报警输出**/

	DVS_CMD_GET_ALARMIN,        /** 获取指定通道的报警输入**/

	DVS_CMD_ON_AUDIO,			/**唤醒整个音频模块**/
	DVS_CMD_OFF_AUDIO,          /** 休眠整个音频模块**/


}ServerCMD;

/** 
 *  服务器响应后，返回的命令包 
 */
typedef enum _ReturnCMD
{
	DVS_RETURN_SUCCESS = 0,      /** 操作成功 */
	DVS_RETURN_FAIL,             /** 操作失败 */
	DVS_RETURN_TIMEOUT,          /** 操作超时 */
	DVS_RETURN_INVLID_HEADER,    /** 无效的命令头 */
	DVS_RETURN_INVLID_CMD,       /** 无效的命令 */
	DVS_RETURN_INVLID_PRM,       /** 无效的参数 */

}ReturnCMD;


/** 字节对齐，1字节对齐 */ 
#pragma pack( push, 1 )

/** 
 *  服务器端发送的命令包 
 */
typedef struct _ServerPack 
{
	/** 包头信息 */ 
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	/** 命令ID */
	ServerCMD serverCMD;

	/** 各命令的附加参数 */
	union
	{
		/** 建立rtp会话 */
		struct
		{
			/** 客户端RTP视频监听端口 */
			int videoport;

			/** 客户端RTP音频监听端口*/
			int audioport;

		}ListenPort;

		/** 播放通道操作 */
		struct
		{
			/** 通道号 */
			uint8_t chId;

		}PlayPort;

		/** 分辨率调整 */
		struct
		{
			/** 视频分辨率 */
			EResolution resolution;

		}Resolution;

		/** 比特率，定码率时使用 */ 
		struct
		{
			int bitrate;

		}Bitrate;

		/** 要指定的报警输出**/
		struct
		{
			uint8_t chId;

		}Alarm;

	}Parameters;

}ServerPack;

/** 
 *  返回的命令包 
 */
typedef struct _ReturnPack
{
	/** 包头信息 */ 
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	/** 返回的状态包 */
	ReturnCMD returnCMD;

	/** 返回命令的附加参数 */
	union
	{
		/** 下位机当前的视频分辨率 */ 
		struct
		{
			EResolution resolution;

		}Resolution;

		/** 下位机报警输入状态 */ 
		struct
		{
			uint8_t state;

		}Alarm;

	}Parameters;

}ReturnPack;

// 报警输入状态数据包
typedef struct _AlarmStatePack
{
	// 包头信息
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	// 报警输入编号
	int number;

	// 报警输入状态
	int state;

}AlarmStatePack;

#pragma pack( pop )

#define SERVER_PACK_LEN sizeof(ServerPack)
#define RETURN_PACK_LEN sizeof(ReturnPack)

#endif // COMMAND_H_
