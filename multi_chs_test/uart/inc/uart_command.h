#ifndef COMMAND_H
#define COMMAND_H

/** TCP命令包长度 */ 
#define TCP_CMD_HEADER_LEN 8

/** TCP命令字的位置 */ 
#define TCP_CMD_POSITION  (TCP_CMD_HEADER_LEN-1)

/** 支持的TCP控制命令 */ 
#define DVS_TCP_CMD_PTZ  0
#define DVS_TCP_CMD_UART 1

/** TCP命令包包头 */ 
/** 前7位为固定字节标识，最后一位(TCP_CMD_POSITION)标识当前命令类型
 *     如果是DVS_TCP_CMD_PTZ则为云台控制命令，如果是DVS_TCP_CMD_UART则为串口参数设置 */ 
static unsigned char TCP_CMD_HEADER_STR[TCP_CMD_HEADER_LEN] = { 0xBB,0xB1,0xB2,0xB3,0xB4,0xB5,0xFF,0x00 };

/** 最大的PTZ命令长度 */ 
#define PTZ_CMD_MAX_LEN 8

/** 字节对齐，1字节对齐 */ 
#pragma pack( push, 1 )

/** PTZ控制命令数据包 */ 
typedef struct _PtzCmdPack
{
    /** PTZ命令内容 */
    unsigned char cmd[PTZ_CMD_MAX_LEN];   

    /** PTZ命令的有效长度 */
    int len; 

}PtzCmdPack;

/** 串口参数 */ 
typedef struct _SerialPack
{
    int baudrate;     /** 波特率   */
    int databits;     /** 数据位   */  
    int stopbits;     /** 停止位   */
    int parity;       /** 奇偶校验 */

}SerialPack;

#pragma pack( pop )

#endif



