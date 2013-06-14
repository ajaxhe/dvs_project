//////////////////////////////////////////////////////////////////////////
//  COPYRIGHT NOTICE
//  Copyright (c) 2009, ���пƼ���ѧCBIBʵ���ң���Ȩ������
//  All rights reserved.
// 
/// @file    Command.h  
/// @brief   ����������ļ�
///
/// �������TCP������Լ���ؽṹ��
///
/// @version 1.0   
/// @author  lujun 
/// @date    2011/05/27
//
//
//  �޶�˵����
//////////////////////////////////////////////////////////////////////////

#ifndef COMMAND_H_
#define COMMAND_H_
 
typedef unsigned char uint8_t;

/** IP��ַ�ĳ��� */
#define MAX_IP_ADDR_LEN 20

/** ÿ������Ͱ���С */
#define MAX_PACKET_SIZE 65536

/** ��ʱʱ��Ϊ5000ms */
#define TIME_OUT_DELAY 5000

/** TCP�����ͷ�ĳ��� */ 
#define TCP_CMD_HEADER_LEN 8

/** ��̨���������ֵ���󳤶� */ 
#define MAX_PTZ_CMD_LEN 32

// cmd header str
static uint8_t TCP_CMD_HEADER_STR[TCP_CMD_HEADER_LEN] = { 0xAA,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xFF };

// ��������״̬����ͷ
static uint8_t ALARM_HEADER_STR[TCP_CMD_HEADER_LEN] = { 0x00,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xFF };

/** ��Ƶ�ֱ��� */ 
typedef enum _EResolution
{
	DVS_RESOLUTION_CIF = 0,
	DVS_RESOLUTION_D1

}EResolution;

/** ��̬����ʱ���õ�����ֵ */ 
#define DVS_BITRATE_VARAY 0

/** 
 *  ���Է��͸�������������
 */
typedef enum _ServerCMD
{
	DVS_CMD_RESET_BOARD,        /** ������λ������ */

	DVS_CMD_ADD_CLIENT,         /** ���һ���µĿͻ��� */

	DVS_CMD_DEL_CLIENT,         /** ɾ��һ���Ѵ��ڵĿͻ��� */

	DVS_CMD_OPEN_CHANNEL,       /** ��ָ����ͨ�� */

	DVS_CMD_CLOSE_CHANNEL,      /** �ر�ָ����ͨ�� */

    DVS_CMD_GET_RESOLUTION,     /** ��ȡ��Ƶ�ķֱ��� */ 

	DVS_CMD_SET_RESOLUTION,     /** ������Ƶ�ķֱ��� */

	DVS_CMD_SET_BITRATE,        /** ���ù̶����� */

	DVS_CMD_ON_ALARMOUT,        /** ��ָ��ͨ���ı������**/
	DVS_CMD_OFF_ALARMOUT,       /** �ر�ָ��ͨ���ı������**/
	DVS_CMD_GET_ALARMOUT,		/** ��ȡָ��ͨ���ı������**/

	DVS_CMD_GET_ALARMIN,        /** ��ȡָ��ͨ���ı�������**/

	DVS_CMD_ON_AUDIO,			/**����������Ƶģ��**/
	DVS_CMD_OFF_AUDIO,          /** ����������Ƶģ��**/


}ServerCMD;

/** 
 *  ��������Ӧ�󣬷��ص������ 
 */
typedef enum _ReturnCMD
{
	DVS_RETURN_SUCCESS = 0,      /** �����ɹ� */
	DVS_RETURN_FAIL,             /** ����ʧ�� */
	DVS_RETURN_TIMEOUT,          /** ������ʱ */
	DVS_RETURN_INVLID_HEADER,    /** ��Ч������ͷ */
	DVS_RETURN_INVLID_CMD,       /** ��Ч������ */
	DVS_RETURN_INVLID_PRM,       /** ��Ч�Ĳ��� */

}ReturnCMD;


/** �ֽڶ��룬1�ֽڶ��� */ 
#pragma pack( push, 1 )

/** 
 *  �������˷��͵������ 
 */
typedef struct _ServerPack 
{
	/** ��ͷ��Ϣ */ 
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	/** ����ID */
	ServerCMD serverCMD;

	/** ������ĸ��Ӳ��� */
	union
	{
		/** ����rtp�Ự */
		struct
		{
			/** �ͻ���RTP��Ƶ�����˿� */
			int videoport;

			/** �ͻ���RTP��Ƶ�����˿�*/
			int audioport;

		}ListenPort;

		/** ����ͨ������ */
		struct
		{
			/** ͨ���� */
			uint8_t chId;

		}PlayPort;

		/** �ֱ��ʵ��� */
		struct
		{
			/** ��Ƶ�ֱ��� */
			EResolution resolution;

		}Resolution;

		/** �����ʣ�������ʱʹ�� */ 
		struct
		{
			int bitrate;

		}Bitrate;

		/** Ҫָ���ı������**/
		struct
		{
			uint8_t chId;

		}Alarm;

	}Parameters;

}ServerPack;

/** 
 *  ���ص������ 
 */
typedef struct _ReturnPack
{
	/** ��ͷ��Ϣ */ 
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	/** ���ص�״̬�� */
	ReturnCMD returnCMD;

	/** ��������ĸ��Ӳ��� */
	union
	{
		/** ��λ����ǰ����Ƶ�ֱ��� */ 
		struct
		{
			EResolution resolution;

		}Resolution;

		/** ��λ����������״̬ */ 
		struct
		{
			uint8_t state;

		}Alarm;

	}Parameters;

}ReturnPack;

// ��������״̬���ݰ�
typedef struct _AlarmStatePack
{
	// ��ͷ��Ϣ
	uint8_t cmdHeader[TCP_CMD_HEADER_LEN];

	// ����������
	int number;

	// ��������״̬
	int state;

}AlarmStatePack;

#pragma pack( pop )

#define SERVER_PACK_LEN sizeof(ServerPack)
#define RETURN_PACK_LEN sizeof(ReturnPack)

#endif // COMMAND_H_
