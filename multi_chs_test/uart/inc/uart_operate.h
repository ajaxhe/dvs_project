#ifndef UART_H
#define UART_H

/**
 * @brief 打开串口设备
 * @param devname 类型 char* 串口设备名
 * @return int -1表示失败,>0表示成功,为串口文件句柄
 */ 
int open_serial_port(char * devname);

/**
 * @brief 关闭串口设备
 * @param  fd     类型 int  打开串口的文件句柄
 * @return void
 */
void close_serial_port(int fd);

/**
 * @brief  设置串口通信速率
 * @param  fd     类型 int  打开串口的文件句柄
 * @param  speed  类型 int  串口速度
 * @return void
 */
int set_serial_speed(int fd, int speed);

/**
 * @brief  设置串口数据位，停止位和效验位
 * @param  fd       类型  int  打开的串口文件句柄
 * @param  databits 类型  int  数据位 取值为 7 或者8
 * @param  stopbits 类型  int  停止位 取值为 1 或者2
 * @param  parity   类型  int  效验类型 取值为N,E,O,,S
 */
int set_serial_parity(int fd,int databits,int stopbits,int parity);

/**
 * @brief 从串口读取指定字节的数据
 * @param  fd     类型  int   打开的串口文件句柄
 * @param  buffer 类型  char* 存放读取数据的缓冲区
 * @param  len    类型  int   期望读取的字节数（小于等于缓冲区的实际大小） 
 * @return int    实际读取出来的字节数
 */
int recv_serial_data(int fd,char *buffer,int len);

/**
 * @brief 向串口发送数据
 * @param  fd     类型  int   打开的串口文件句柄
 * @param  buffer 类型  char* 存放需要发送到串口的数据缓冲区
 * @param  len    类型  int   期望发送的字节数
 * @return int    实际发送的字节数
 */
int send_serial_data(int fd,char *buffer,int len);

#endif




