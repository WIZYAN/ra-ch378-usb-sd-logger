/*
 * ch378_device.h
 *
 *  Created on: 2026年6月30日
 *      Author: Administrator
 */

#ifndef CH378_FILE_DEVICE_H_
#define CH378_FILE_DEVICE_H_

#include "ch378_hard.h"


/* ********************************************************************************************************************* */
/* 命令码说明:
   (1)、部分命令兼容CH376芯片
   (2)、一个命令操作顺序包含:
          a、一个命令码(对于串口方式,命令码之前还需要两个同步码);
          b、若干个输入数据(可以是0个);
          c、产生中断通知或者若干个输出数据(可以是0个), 二选一, 有中断通知则一定没有输出数据, 有输出数据则一定不产生中断;
             仅CMD01_WR_REQ_DATA命令例外, 顺序包含: 一个命令码, 一个输出数据, 若干个输入数据
   (3)、命令码起名规则: CMDxy_NAME
        其中的x和y都是数字, x说明最少输入数据个数(字节数), y说明最少输出数据个数(字节数), y如果是H则说明产生中断通知,
        有些命令能够实现0到多个字节的数据块读写, 数据块本身的字节数未包含在上述x或y之内 */

/* ********************************************************************************************************************* */
/* 主要命令(手册一), 常用, 以下命令在操作结束时不会产生中断通知 */

#define CMD01_GET_IC_VER           0x01                          /* 获取芯片及固件版本 */
/* 输出: 版本号( 位7为0, 位6为1, 位5~位0为版本号 )   CH378返回版本号的值为040H即版本号为00H */


#define CMD31_SET_BAUDRATE         0x02                          /* 串口方式: 设置串口通讯波特率(上电或者复位后的默认波特率为9600bps,由SCK/SDI/SDO引脚选择) */
/* 输入: 3个字节波特率系数,低字节在前 */
/* 输出: 操作状态( CMD_RET_SUCCESS或CMD_RET_ABORT, 其它值说明操作未完成 ) */

#define CMD10_ENTER_SLEEP          0x03                          /* 进入睡眠状态 */
/* 输入: 1个字节参数, 如果参数为0x11,则使能芯片进入半睡眠状态,唤醒之后USB功能可以继续使用;
                      如果参数为0x22,则使能芯片进入深度睡眠状态,唤醒之后如果操作的是USB设备,需要全部重新初始化; */

#define CMD00_RESET_ALL            0x05                          /* 执行硬件复位 */

#define CMD11_CHECK_EXIST          0x06                          /* 测试通讯接口和工作状态 */
/* 输入: 任意数据 */
/* 输出: 输入数据的按位取反 */

#define CMD20_SET_SDO_INT          0x0B                          /* SPI接口方式: 设置SPI的SDO引脚的中断方式 */
/* 输入: 数据16H, 中断方式 */
/*           00H=禁止SDO引脚用于中断输出,在SCS片选无效时三态输出禁止, 01H=SDO引脚在SCS片选无效时兼做中断请求输出 */

#define CMD14_GET_FILE_SIZE        0x0C                          /* 主机文件模式: 获取当前文件长度 */
/* 输入: 数据68H */
/* 输出: 当前文件长度(总长度32位,低字节在前) */

#define CMD11_SET_USB_MODE         0x15                          /* 设置USB工作模式 */
/* 输入: 模式代码 */
/*       00H=未启用的设备方式, 01H=已启用的设备方式并且使用外部固件模式, 02H=已启用的设备方式并且使用内置固件模式 */
/*       03H=SD卡主机模式/未启用的主机模式,用于管理和存取SD卡中的文件 */
/*       04H=SD卡主机模式/已启用的主机模式,用于管理和存取SD卡中的文件,!!!异步串口不支持!!! */
/*       05H=未启用的主机方式 */
/*       06H=USB主机模式/未启用的主机模式,用于管理和存取USB存储设备中的文件 */
/*       07H=USB主机模式/已启用的主机模式,用于管理和存取USB存储设备中的文件 */
/*       08H=USB读卡器模式，用于计算机USB口直接管理SD卡中的文件,!!!版本号大于等于0x43 时支持!!! */
/* 输出: 操作状态( CMD_RET_SUCCESS或CMD_RET_ABORT, 其它值说明操作未完成 ) */

#define CMD01_GET_STATUS           0x22                          /* 获取中断状态并取消中断请求 */
/* 输出: 中断状态 */

#define CMD40_RD_HOST_OFS_DATA     0x26                          /* 主机模式: 读取内部指定缓冲区指定偏移地址数据块(内部指针自动增加) */
/* 输入: 偏移地址( 2个字节,低字节在前,最大为20480 )、读取长度(2个字节,低字节在前,最大为20480) */
/* 输出: 数据流 */
/* 注意: 若输入参数错误,则只发送前面实际缓冲区数据 */

#define CMD20_RD_HOST_CUR_DATA     0x27                          /* 主机模式: 读取内部指定缓冲区当前偏移地址数据块(内部指针自动增加) */
/* 输入: 读取长度(2个字节,低字节在前,最大为20480) */
/* 输出: 数据流 */
/* 注意: 若输入参数错误,则只发送前面实际缓冲区数据 */

#define CMD00_RD_HOST_REQ_DATA     0x28                          /* 主机模式: 读取内部指定缓冲区当前偏移地址请求读取的数据块(内部指针自动增加) */
/* 输出: 读取长度(2个字节,低字节在前,最大为20480) */
/* 输出: 数据流 */
/* 注意: 若输入参数错误,则只发送前面实际缓冲区数据 */

#define CMD40_WR_HOST_OFS_DATA     0x2D                          /* 主机模式: 向内部指定缓冲区指定偏移地址写入数据块(内部指针自动增加) */
/* 输入: 偏移地址( 2个字节,低字节在前,最大为20480 )、输入长度(2个字节,低字节在前,最大为20480)、数据流 */
/* 注意: 若输入参数错误,则写入数据会自动从缓冲区起始地址开始保存 */

#define CMD20_WR_HOST_CUR_DATA     0x2E                          /* 主机模式: 向内部指定缓冲区当前偏移地址写入数据块(内部指针自动增加) */
/* 输入: 输入长度(2个字节,低字节在前,最大为20480)、数据流 */
/* 注意: 若输入参数错误,则写入数据会自动从缓冲区起始地址开始保存 */

#define CMD10_SET_FILE_NAME        0x2F                          /* 主机文件模式: 设置将要操作的文件的文件名、路径名 */
/* 输入: 以0结束的字符串(含结束符0在内长度不超过MAX_FILE_NAME_LEN个字符) */

/* ********************************************************************************************************************* */
/* 主要命令(手册一), 常用, 以下命令总是在操作结束时产生中断通知, 并且总是没有输出数据 */
#define CMD0H_DISK_CONNECT         0x30                          /* 主机文件模式: 检查磁盘是否连接 */
/* 输出中断 */

#define CMD0H_DISK_MOUNT           0x31                          /* 主机文件模式: 初始化磁盘并测试磁盘是否就绪 */
/* 输出中断 */

#define CMD0H_FILE_OPEN            0x32                          /* 主机文件模式: 打开文件或者目录(文件夹),或者枚举文件和目录(文件夹) */
/* 输出中断 */

#define CMD0H_FILE_ENUM_GO         0x33                          /* 主机文件模式: 继续枚举文件和目录(文件夹) */
/* 输出中断 */

#define CMD0H_FILE_CREATE          0x34                          /* 主机文件模式: 新建文件,如果文件已经存在那么先删除 */
/* 输出中断 */

#define CMD0H_FILE_ERASE           0x35                          /* 主机文件模式: 删除文件,如果已经打开则直接删除,否则对于文件会先打开再删除,子目录必须先打开 */
/* 输出中断 */

#define CMD1H_FILE_CLOSE           0x36                          /* 主机文件模式: 关闭当前已经打开的文件或者目录(文件夹) */
/* 输入: 是否允许更新文件长度 */
/*       00H=禁止更新长度, 01H=允许更新长度 */
/* 输出中断 */

#define CMD1H_DIR_INFO_READ        0x37                          /* 主机文件模式: 读取文件的目录信息 */
/* 输入: 指定需要读取的目录信息结构在扇区内的索引号 */
/*       索引号范围为00H~0FH, 索引号0FFH则为当前已经打开的文件 */
/* 输出中断 */

#define CMD1H_DIR_INFO_SAVE        0x38                          /* 主机文件模式: 保存文件的目录信息 */
/* 输入: 指定需要写入的目录信息结构在扇区内的索引号 */
/*       索引号范围为00H~0FH, 索引号0FFH则为当前已经打开的文件 */
/* 输出中断 */

#define CMD4H_BYTE_LOCATE          0x39                          /* 主机文件模式: 以字节为单位移动当前文件指针 */
/* 输入: 偏移字节数(总长度32位,低字节在前) */
/* 输出中断 */

#define CMD2H_BYTE_READ            0x3A                          /* 主机文件模式: 以字节为单位从当前位置读取数据块 */
/* 输入: 请求读取的字节数(总长度16位,低字节在前,最大为20480) */
/* 输出中断 */
/* 注意: 发送该命令后读取的实际数据在内部缓冲区中,需要通过CMD40_RD_HOST_OFS_DATA、CMD20_RD_HOST_CUR_DATA命令获取 */

#define CMD2H_BYTE_WRITE           0x3C                          /* 主机文件模式: 以字节为单位向当前位置写入数据块 */
/* 输入: 请求写入的字节数(总长度16位,低字节在前,最大为20480) */
/* 输出中断 */
/* 注意: 调用该命令前需要通过CMD40_WR_HOST_OFS_DATA或CMD20_WR_HOST_CUR_DATA命令将要写的数据预先填充到内部缓冲区中 */

#define CMD0H_DISK_CAPACITY        0x3E                          /* 主机文件模式: 查询磁盘物理容量 */
/* 输出中断 */

#define CMD0H_DISK_QUERY           0x3F                          /* 主机文件模式: 查询磁盘空间信息 */
/* 输出中断 */

#define CMD0H_DIR_CREATE           0x40                          /* 主机文件模式: 新建目录(文件夹)并打开,如果目录已经存在那么直接打开 */
/* 输出中断 */

#define CMD4H_SEC_LOCATE           0x4A                          /* 主机文件模式: 以扇区为单位移动当前文件指针 */
/* 输入: 偏移扇区数(总长度32位,低字节在前) */
/* 输出中断 */

#define CMD1H_SEC_READ             0x4B                          /* 主机文件模式: 以扇区为单位从当前位置读取数据块 */
/* 输入: 请求读取的扇区数 */
/* 输出中断 */
/* 注意: 受内部缓冲区大小限制,单次扇区读写最大扇区数为40 */

#define CMD1H_SEC_WRITE            0x4C                          /* 主机文件模式: 以扇区为单位在当前位置写入数据块 */
/* 输入: 请求写入的扇区数 */
/* 输出中断 */
/* 注意: 受内部缓冲区大小限制,单次扇区读写最大扇区数为40 */

#define CMD0H_DISK_BOC_CMD         0x50                          /* 主机方式/不支持SD卡: 对USB存储器执行BulkOnly传输协议的命令 */
/* 输出中断 */

#define CMD5H_DISK_READ            0x54                          /* 主机方式: 从USB存储器或SD卡读物理扇区 */
/* 输入: LBA物理扇区地址(总长度32位, 低字节在前), 扇区数(01H~28H) */
/* 输出中断 */
/* 注意: 受内部缓冲区大小限制,单次扇区读写最大扇区数为40 */

#define CMD5H_DISK_WRITE           0x56                          /* 主机方式: 向USB存储器或SD卡写物理扇区 */
/* 输入: LBA物理扇区地址(总长度32位, 低字节在前), 扇区数(01H~FFH) */
/* 输出中断 */
/* 注意: 受内部缓冲区大小限制,单次扇区读写最大扇区数为40 */
/* 注意: 调用该命令前需要通过CMD40_WR_HOST_OFS_DATA或CMD20_WR_HOST_CUR_DATA命令将要写的数据预先填充到内部缓冲区中 */

#define CMD0H_FILE_QUERY           0x55                          /* 主机文件模式: 查询当前文件的信息 */
/* 输出中断 */
/* 注意: 返回数据格式: 4个字节文件长度、2个字节文件日期、2个字节文件时间、1个字节文件属性 */

#define CMD0H_FILE_MODIFY          0x57                          /* 主机文件模式: 查询或者修改当前文件的信息 */
/* 输出中断 */
/* 注意: 输入数据格式: 4个字节文件长度、2个字节文件日期、2个字节文件时间、1个字节文件属性 */


/* ********************************************************************************************************************* */
/* 次要命令 非常用, 以下命令在操作结束时不会产生中断通知 */

#define CMD20_BULK_WR_TEST         0x07                          /* 批量写数据测试 */
/* 输入: 2个字节后续数据长度,N个字节后续数据 */

#define CMD20_BULK_RD_TEST         0x08                          /* 批量读数据测试 */
/* 输入: 2个字节后续数据长度 */
/* 输出: N个字节后续数据 */

#define CMD11_READ_VAR8            0x0A                          /* 读取指定的8位CH378系统变量 */
/* 输入: 变量地址 */
/* 输出: 当前地址对应的8位数据 */

#define CMD11_GET_DEV_RATE         0x0A                          /* 主机方式: 获取当前连接的USB设备的数据速率类型 */
/* 输入: 数据07H */
/* 输出: 数据速率类型 */
/*           00表示速度未知; 01表示1.5Mbps低速USB设备; 02表示12Mbps全速USB设备; 03表示480Mbps高速USB设备 */

#define CMD20_WRITE_VAR8           0x0B                          /* 设置指定的8位CH378系统变量 */
/* 输入: 变量地址, 数据 */

#define CMD14_READ_VAR32           0x0C                          /* 读取指定的32位CH378系统变量 */
/* 输入: 变量地址 */
/* 输出: 当前地址对应的32位数据(总长度32位,低字节在前) */

#define CMD50_WRITE_VAR32          0x0D                          /* 设置指定的32位CH378系统变量 */
/* 输入: 变量地址, 数据(总长度32位,低字节在前) */

#define CMD50_SET_FILE_SIZE        0x0D                          /* 主机文件模式: 设置当前文件长度 */
/* 输入: 数据68H, 当前文件长度(总长度32位,低字节在前) */
/* 注意：该命令只是临时改变当前打开的文件的文件长度 */

#define CMD02_GET_REAL_LEN         0x0E                          /* 快速返回上一个命令执行完毕后请求长度所对应的实际长度 */
/* 输出: 4个字节实际数据, 低字节在前 */
/* 注意: 该命令仅仅在使用字节定位、字节读写、扇区定位和扇区读写命令时使用,以加快执行速度  */

#define CMD01_TEST_CONNECT         0x16                          /* 主机方式: 检查USB设备/SD卡连接状态 */
/* 输出: 连接状态( USB_INT_CONNECT或USB_INT_DISCONNECT或USB_INT_USB_READY, 其它值说明操作未完成 ) */

#define CMD00_DIRTY_BUFFER         0x25                          /* 主机文件模式: 清除内部的磁盘和文件缓冲区 */

/* ********************************************************************************************************************* */
/* 次要命令(手册二), 非常用, 以下命令总是在操作结束时产生中断通知, 并且总是没有输出数据 */

#define CMD1H_CLR_STALL            0x41                          /* 主机方式/不支持SD卡: 控制传输-清除端点错误 */
/* 输入: 端点号 */
/* 输出中断 */

#define CMD1H_SET_ADDRESS          0x45                          /* 主机方式/不支持SD卡: 控制传输-设置USB地址 */
/* 输入: 地址值 */
/* 输出中断 */

#define CMD1H_GET_DESCR            0x46                          /* 主机方式/不支持SD卡: 控制传输-获取描述符 */
/* 输入: 描述符类型 */
/* 输出中断 */

#define CMD1H_SET_CONFIG           0x49                          /* 主机方式/不支持SD卡: 控制传输-设置USB配置 */
/* 输入: 配置值 */
/* 输出中断 */

#define CMD0H_AUTO_SETUP           0x4D                          /* 主机方式/不支持SD卡: 自动配置USB设备 */
/* 输出中断 */

#define CMD0H_ISSUE_CTRL_TRAN      0x4E                          /* 主机方式/不支持SD卡: 执行控制传输 */
/* 输出中断 */
/* 注意: 发起该命令前,必须通过CMD40_WR_HOST_OFS_DATA命令将8个字节的SETUP包写入到内部缓冲区中,如果需要通过OUT阶段下传数据包,
         则还需要将后续数据包紧接着8个字节的SETUP包写入到内部缓冲区中 */

#define CMD0H_DISK_INIT            0x51                          /* 主机方式/不支持SD卡: 初始化USB存储器 */
/* 输出中断 */

#define CMD0H_DISK_RESET           0x52                          /* 主机方式/不支持SD卡: 控制传输-复位USB存储器 */
/* 输出中断 */

#define CMD0H_DISK_SIZE            0x53                          /* 主机方式/不支持SD卡: 获取USB存储器的容量 */
/* 输出中断 */

#define CMD0H_DISK_INQUIRY         0x58                          /* 主机方式/不支持SD卡: 查询USB存储器特性 */
/* 输出中断 */

#define CMD0H_DISK_READY           0x59                          /* 主机方式/不支持SD卡: 检查USB存储器就绪 */
/* 输出中断 */

#define CMD0H_DISK_R_SENSE         0x5A                          /* 主机方式/不支持SD卡: 检查USB存储器错误 */
/* 输出中断 */

#define CMD0H_DISK_MAX_LUN         0x5D                          /* 主机方式/不支持SD卡: 控制传输-获取USB存储器最大逻辑单元号 */
/* 输出中断 */

#define CMD10_SET_LONG_FILE_NAME   0x60                          /* 主机文件模式: 设置将要操作的长文件的文件名(仅仅是文件名) */
/* 输入长度(2个字节,低字节在前,最大为520)、以两个0结束的字符串(长文件名长度不超过520个字节) */

#define CMD10_GET_LONG_FILE_NAME   0x61                          /* 主机文件模式: 由完整短文件名路径(可以是文件或文件夹)得到相应的长文件名 */
/* 输出中断 */
/* 输出: 读取长度(2个字节,低字节在前,最大为520)、长文件名 */
/* 注意：发起该命令前,必须先通过CMD10_SET_FILE_NAME命令将短文件完整的路径名送入CH378 */

#define CMD0H_LONG_FILE_CREATE     0x62                          /* 主机文件模式: 新建长文件名文件,如果文件已经存在那么先删除 */
/* 输出中断 */
/* 注意：发起该命令前,必须先通过CMD10_SET_LONG_FILE_NAME命令和CMD10_SET_FILE_NAME命令分别将长文件名,以及短文件完整的路径名送入CH378 */

#define CMD0H_LONG_DIR_CREATE      0x63                          /* 主机文件模式: 新建长文件名目录(文件夹)并打开,如果目录已经存在那么直接打开 */
/* 输出中断 */
/* 注意：发起该命令前,必须先通过CMD10_SET_LONG_FILE_NAME命令和CMD10_SET_FILE_NAME命令分别将长文件名,以及短文件完整的路径名送入CH378 */

#define CMD0H_AUTO_DEMO            0x64                          /* 芯片自动演示,在U盘或者SD卡中自动建立名为“芯片演示.TXT”的文件，并写入当前信息 */
/* 输出中断 */

#define CMD1H_GET_SHORT_FILE_NAME  0x65                          /* 主机文件模式: 由部分短文件名路径加上完整的长文件名得到相应的短文件名 */
/* 输出中断 */
/* 输出: 读取长度(2个字节,低字节在前,最大为MAX_PATH_LEN)、短文件名路径 */
/* 注意：发起该命令前,必须先通过CMD10_SET_FILE_NAME命令将部分短文件路径名送入CH378,并通过CMD10_SET_LONG_FILE_NAME
   命令将完整的长文件名输入CH378 */

#define CMD0H_LONG_FILE_OPEN       0x66                          /* 主机文件模式: 通过长文件名打开文件或者目录(文件夹) */
/* 输出中断 */

#define CMD0H_LONG_FILE_ERASE      0x67                          /* 主机文件模式: 通过长文件名删除文件,如果已经打开则直接删除,否则对于文件会先打开再删除,子目录必须先打开 */
/* 输出中断 */
/* ********************************************************************************************************************* */
/* USB设备模式相关命令 */

#define CMD40_SET_USB_ID           0x11                          /* 设置USB厂商VID和产品PID */
/* 输入: 4个字节VID\PID数据,分别为厂商ID低字节, 厂商ID高字节, 产品ID低字节, 产品ID高字节 */
/* 注：该命令必须在CMD11_SET_USB_MODE命令前发送 */

#define CMD10_SET_USB_SPEED        0x13                          /* 设置USB速度 */
/* 输入: 1个字节USB速度值,0x00为全速,0x02为高速,其它值无效 */
/* 注：该命令必须在CMD11_SET_USB_MODE命令前发送 */

#define CMD01_GET_USB_SPEED        0x14                          /* 获取USB速度 */
/* 输出: 1个字节的USB速度值, 0x00为全速,0x02为高速,其它值无效 */

#define CMD70_INIT_ENDPx           0x17                          /* 设置USB端点(端点索引x范围: 1--4) */
/* 输入: 1个字节USB端点索引号: 1--4, 1个字节端点号, 1个字节端点类型, 1个字节端点方向, 2个字节最大包大小(高字节在前),1个字节的中断状态  */
/* 注：该命令必须在CMD11_SET_USB_MODE命令前发送 */

#define CMD20_SET_INDEXx_IN        0x18                          /* 设置USB IN端点索引x(x范围: 0--4)的接收器 */
/* 输入: 1个字节索引号, 1个字节工作方式 */
/*           位7为1则复位该端点的同步位；
             位6为1则设置该端点事务响应方式为：STALL；
             位5为1则清除该端点之前的STALL特性；
             其它情况无实际数据上传,则芯片自动应答NAK */

#define CMD20_SET_INDEXx_OUT       0x19                          /* 设置USB OUT端点索引x(x范围: 0--4)的发送器 */
/* 输入: 1个字节索引号, 1个字节工作方式 */
/*           位7为1则复位该端点的同步位；
             位6为1则设置该端点事务响应方式为：STALL；
             位5为1则清除该端点之前的STALL特性；
             其它情况接收到下传数据包,则芯片自动应答ACK */

#define CMD10_UNLOCK_USB           0x23                          /* 释放当前USB缓冲区 */
/* 输入: 1个字节USB端点索引号: 0--4 */

#define CMD1x_RD_USB_DATA          0x29                          /* 从指定USB中断的端点缓冲区读取数据块,并释放缓冲区 */
/* 输入: 1个字节USB端点索引号: 0--4 */
/* 输出: 2个字节的数据长度, 数据流 */

#define CMD30_WR_USB_DATA          0x2A                          /* 向控制之外的USB端点的发送缓冲区写入数据块 */
/* 输入: 1个字节USB端点索引号: 1--4; 2个字节的数据长度; N个字节的数据流 */

#define CMD10_WR_USB_DATA0         0x2B                          /* 向USB控制端点的发送缓冲区写入数据块 */
/* 输入: 1个字节的数据长度; N个字节的数据流 */


/* ********************************************************************************************************************* */
/* USB设备模式速度定义 */
#define USB_DEV_SPEED_FS           0x00                          /* 当前为全速设备 */
#define USB_DEV_SPEED_HS           0x02                          /* 当前为高速设备 */

/* 端点索引定义 */
#define ENDP_INDEX_0               0x00                          /* 端点索引0：控制端点 */
#define ENDP_INDEX_1               0x01                          /* 端点索引1：自定义端点 */
#define ENDP_INDEX_2               0x02                          /* 端点索引2：自定义端点 */
#define ENDP_INDEX_3               0x03                          /* 端点索引3：自定义端点 */
#define ENDP_INDEX_4               0x04                          /* 端点索引4：自定义端点 */

/* 端点传输方向 */
#define ENDP_DIR_IN                0x00                          /* 上传端点 */
#define ENDP_DIR_OUT               0x01                          /* 下传端点 */

/* 端点类型 */
#define ENDP_TYPE_RESERVED         0x00                          /* 保留 */
#define ENDP_TYPE_ISO              0x01                          /* 同步传输 */
#define ENDP_TYPE_BULK             0x02                          /* 批量传输 */
#define ENDP_TYPE_INTERRUPT        0x03                          /* 中断传输 */

/* ********************************************************************************************************************* */
/* 并口方式, 状态端口(读命令端口)的位定义 */
#ifndef PARA_STATE_INTB
#define PARA_STATE_INTB            0x80                          /* 并口方式状态端口的位7: 中断标志,低有效 */
#define PARA_STATE_BUSY            0x10                          /* 并口方式状态端口的位4: 忙标志,高有效 */
#endif


/* ********************************************************************************************************************* */
/* 操作状态 */
#ifndef CMD_RET_SUCCESS
#define CMD_RET_SUCCESS            0x51                          /* 命令操作成功 */
#define CMD_RET_ABORT              0x5F                          /* 命令操作失败 */
#define CMD_PARAME_ERR             0x5E                          /* 命令参数错误 */
#define CMD_IDENTIFY_ERR           0x5D                          /* 无该命令错误 */
#define CMD_TIMEOUT_ERR            0x5C                          /* 命令超时错误 */
#endif

/* ********************************************************************************************************************* */
/* CH378中断状态 */
#ifndef USB_INT_WAKE_UP
#define USB_INT_WAKE_UP            0xE0                          /* CH378唤醒中断状态 */
#endif

/* ********************************************************************************************************************* */
/* 以下状态代码0XH用于USB设备方式 */
/*   内置固件模式下只需要处理: USB_INT_INDEX1_IN, USB_INT_INDEX2_OUT, USB_INT_INDEX3_IN, USB_INT_INDEX4_OUT */
/*   位7-位5为000 */
/*   位4-位3指示当前事务, 00=OUT, 10=IN, 11=SETUP */
/*   位2-位0指示当前端点索引, 000=0号端点, 001=1号端点, 010=2号端点,011=3号端点,100=4号端点, 101=USB挂起, 110=USB总线复位 */
/*   端点索引为0时即为端点0(控制端点), 端点索引为1-4时为其他端点, 端点号、端点类型、端点最大包大小、端点方向可在端点配置时设置 */

#define USB_INT_EP0_SETUP          0x18                          /* USB端点0的SETUP */
#define USB_INT_EP0_OUT            0x00                          /* USB端点0的OUT */
#define USB_INT_EP0_IN             0x10                          /* USB端点0的IN */
#define USB_INT_INDEX1_OUT         0x01                          /* USB 1号端点的OUT */
#define USB_INT_INDEX1_IN          0x11                          /* USB 1号端点的IN */
#define USB_INT_INDEX2_OUT         0x02                          /* USB 2号端点的OUT */
#define USB_INT_INDEX2_IN          0x12                          /* USB 2号端点的IN */
#define USB_INT_INDEX3_OUT         0x03                          /* USB 3号端点的OUT */
#define USB_INT_INDEX3_IN          0x13                          /* USB 3号端点的IN */
#define USB_INT_INDEX4_OUT         0x04                          /* USB 4号端点的OUT */
#define USB_INT_INDEX4_IN          0x14                          /* USB 4号端点的IN */
#define USB_INT_BUS_SUSP           0x05                          /* USB总线挂起 */
#define USB_INT_BUS_RESET          0x06                          /* USB总线复位 */
#define USB_INT_SET_CONFIG         0x07                          /* USB接收到SET_CONFIG命令 */


/* 以下状态代码1XH用于USB主机方式的操作状态代码 */
#ifndef USB_INT_SUCCESS
#define USB_INT_SUCCESS            0x14                          /* USB事务或者传输操作成功 */
#define USB_INT_CONNECT            0x15                          /* 检测到USB设备连接事件, 可能是新连接或者断开后重新连接 */
#define USB_INT_DISCONNECT         0x16                          /* 检测到USB设备断开事件 */
#define USB_INT_BUF_OVER           0x17                          /* USB传输的数据有误或者数据太多缓冲区溢出 */
#define USB_INT_USB_READY          0x18                          /* USB设备已经被初始化(已经分配USB地址) */
#define USB_INT_DISK_READ          0x1D                          /* USB存储器请求数据读出 */
#define USB_INT_DISK_WRITE         0x1E                          /* USB存储器请求数据写入 */
#define USB_INT_DISK_ERR           0x1F                          /* USB存储器操作失败 */
#endif


/* 以下状态代码用于主机文件模式下的文件系统错误码 */
#ifndef ERR_SUCCESS
#define ERR_SUCCESS                0x00                          /* 操作成功 */
#endif
#define ERR_PARAMETER_ERROR        0x03                          /* 参数错误, 范围溢出 */
#ifndef ERR_DISK_DISCON
#define ERR_DISK_DISCON            0x82                          /* 磁盘尚未连接,可能磁盘已经断开 */
#define ERR_LARGE_SECTOR           0x84                          /* 磁盘的扇区太大,只支持每扇区512字节 */
#define ERR_TYPE_ERROR             0x92                          /* 磁盘分区类型不支持,只支持FAT12/FAT16/BigDOS/FAT32,需要由磁盘管理工具重新分区 */
#define ERR_BPB_ERROR              0xA1                          /* 磁盘尚未格式化,或者参数错误,需要由WINDOWS采用默认参数重新格式化 */
#define ERR_DISK_FULL              0xB1                          /* 磁盘文件太满,剩余空间太少或者已经没有,需要磁盘整理 */
#define ERR_FDT_OVER               0xB2                          /* 目录(文件夹)内文件太多,没有空闲的目录项,FAT12/FAT16根目录下的文件数应该少于512个,需要磁盘整理 */
#define ERR_FILE_CLOSE             0xB4                          /* 文件已经关闭,如果需要使用,应该重新打开文件 */
#define ERR_OPEN_DIR               0x41                          /* 指定路径的目录(文件夹)被打开 */
#define ERR_MISS_FILE              0x42                          /* 指定路径的文件没有找到,可能是文件名称错误 */
#define ERR_FOUND_NAME             0x43                          /* 搜索到相匹配的文件名,或者是要求打开目录(文件夹)而实际结果却打开了文件 */

/* 以下长文件错误码用于文件系统子程序 */
#define ERR_NO_NAME                0X44                          /* 此短文件名没有长文件名或错误的长文件 */
#define ERR_BUF_OVER               0X45                          /* 长文件缓冲区溢出 */
#define ERR_LONG_NAME              0X46                          /* 错误的长文件名 */

/* 以下文件系统错误码用于文件系统子程序 */
#define ERR_MISS_DIR               0xB3                          /* 指定路径的某个子目录(文件夹)没有找到,可能是目录名称错误 */
#define ERR_LONG_BUF_OVER          0x48                          /* 长文件缓冲区溢出 */
#define ERR_LONG_NAME_ERR          0x49                          /* 短文件名没有对应的长文件名或者长文件名错误 */
#define ERR_NAME_EXIST             0x4A                          /* 同名的短文件已经存在,建议重新生成另外一个短文件名 */
#endif

/* ********************************************************************************************************************* */
/* 以下为SD卡低层操作相关错误码 */
#define SD_ERR_NO_CARD             0x60                          /* 卡没有完全插入到卡座中 */
#define SD_ERR_USER_PARAM          0x61                          /* 用户使用API函数时，入口参数有错误 */
#define SD_ERR_CARD_PARAM          0x62                          /* 卡中参数有错误（与本模块不兼容） */
#define SD_ERR_VOL_NOTSUSP         0x63                          /* 卡不支持3.3V供电 */
#define SD_ERR_OVER_CARDRANGE      0x64                          /* 操作超出卡容量范围 */
#define SD_ERR_UNKNOWN_CARD        0x65                          /* 无法识别卡型 */
#define SD_ERR_WP_PROTECT          0x66                          /* 写保护 */
#define SD_CARD_BUSY               0x67                          /* 在读取OCR寄存器时检测到BIT31为0表示忙 */
#define SD_BLOCK_ERROR             0x68                          /* 块大小一定要大于等于512,否则无法操作 */

/* SD命令可能返回的错误码 */
#define SD_ERR_CMD_RESPTYPE        0x69                          /* 命令类型错误 */
#define SD_ERR_CMD_TIMEOUT         0x6A                          /* 卡命令响应超时 */
#define SD_ERR_CMD_RESP            0x6B                          /* 卡命令响应错误 */
#define SD_Power_Up_Busy           0x6C                          /* 上电忙 */

/* 数据流错误码 */
#define SD_ERR_DATA_CRC16          0x6D                          /* 数据流CRC16校验不通过 */
#define SD_ERR_DATA_START_TOK      0x6E                          /* 读单块或多块时,数据开始令牌不正确 */
#define SD_ERR_DATA_RESP           0x6F                          /* 写单块或多块时,卡数据响应令牌不正确 */

/* 等待错误码 */
#define SD_ERR_TIMEOUT_WAIT        0x70                          /* 写或擦操作时,发生超时错误 */
#define SD_ERR_TIMEOUT_READ        0x71                          /* 读操作超时错误 */
#define SD_ERR_TIMEOUT_WRITE       0x72                          /* 写操作超时错误 */
#define SD_ERR_TIMEOUT_ERASE       0x73                          /* 擦除操作超时错误 */
#define SD_ERR_TIMEOUT_WAITIDLE    0x74                          /* 初始化卡时,等待卡退出空闲状态超时错误 */

/* 写操作可能返回的错误码 */
#define SD_ERR_WRITE_BLK           0x75                          /* 写块数据错误 */
#define SD_ERR_WRITE_BLKNUMS       0x76                          /* 写多块时,想要写入的块与正确写入的块数不一致 */
#define SD_ERR_WRITE_PROTECT       0x77                          /* 卡外壳的写保护开关打在写保护位置 */


/* ********************************************************************************************************************* */
/* 以下状态代码用于主机文件模式下的磁盘及文件状态, VAR_DISK_STATUS */
#ifndef DEF_DISK_UNKNOWN
#define DEF_DISK_UNKNOWN           0x00                          /* 尚未初始化,未知状态 */
#define DEF_DISK_DISCONN           0x01                          /* 磁盘没有连接或者已经断开 */
#define DEF_DISK_CONNECT           0x02                          /* 磁盘已经连接,但是尚未初始化或者无法识别该磁盘 */
#define DEF_DISK_MOUNTED           0x03                          /* 磁盘已经初始化成功,但是尚未分析文件系统或者文件系统不支持 */
#define DEF_DISK_READY             0x10                          /* 已经分析磁盘的文件系统并且能够支持 */
#define DEF_DISK_OPEN_ROOT         0x12                          /* 已经打开根目录,使用后必须关闭,注意FAT12/FAT16根目录是固定长度 */
#define DEF_DISK_OPEN_DIR          0x13                          /* 已经打开子目录(文件夹) */
#define DEF_DISK_OPEN_FILE         0x14                          /* 已经打开文件 */
#endif


/* ********************************************************************************************************************* */
/* 文件系统常用定义 */
#ifndef DEF_SECTOR_SIZE
#define DEF_SECTOR_SIZE            512                           /* U盘或者SD卡默认的物理扇区的大小 */
#endif

#ifndef DEF_WILDCARD_CHAR
#define DEF_WILDCARD_CHAR          0x2A                          /* 路径名的通配符 '*' */
#define DEF_SEPAR_CHAR1            0x5C                          /* 路径名的分隔符 '\' */
#define DEF_SEPAR_CHAR2            0x2F                          /* 路径名的分隔符 '/' */
#define DEF_FILE_YEAR              2004                          /* 默认文件日期: 2004年 */
#define DEF_FILE_MONTH             1                             /* 默认文件日期: 1月 */
#define DEF_FILE_DATE              1                             /* 默认文件日期: 1日 */
#endif

#ifndef ATTR_DIRECTORY

/* FAT数据区中文件目录信息 */
typedef struct _FAT_DIR_INFO
{
    uint8_t   DIR_Name[11];                                        /* 00H,文件名,共11字节,不足处填空格 */
    uint8_t   DIR_Attr;                                            /* 0BH,文件属性,参考后面的说明 */
    uint8_t   DIR_NTRes;                                           /* 0CH */
    uint8_t   DIR_CrtTimeTenth;                                    /* 0DH,文件创建的时间,以0.1秒单位计数 */
    uint16_t  DIR_CrtTime;                                         /* 0EH,文件创建的时间 */
    uint16_t  DIR_CrtDate;                                         /* 10H,文件创建的日期 */
    uint16_t  DIR_LstAccDate;                                      /* 12H,最近一次存取操作的日期 */
    uint16_t  DIR_FstClusHI;                                       /* 14H */
    uint16_t  DIR_WrtTime;                                         /* 16H,文件修改时间,参考前面的宏MAKE_FILE_TIME */
    uint16_t  DIR_WrtDate;                                         /* 18H,文件修改日期,参考前面的宏MAKE_FILE_DATE */
    uint16_t  DIR_FstClusLO;                                       /* 1AH */
    uint32_t  DIR_FileSize;                                        /* 1CH,文件长度 */
} FAT_DIR_INFO, *P_FAT_DIR_INFO;                                 /* 20H */


/* 文件属性 */
#define ATTR_READ_ONLY             0x01                          /* 文件为只读属性 */
#define ATTR_HIDDEN                0x02                          /* 文件为隐含属性 */
#define ATTR_SYSTEM                0x04                          /* 文件为系统属性 */
#define ATTR_VOLUME_ID             0x08                          /* 卷标 */
#define ATTR_DIRECTORY             0x10                          /* 子目录(文件夹) */
#define ATTR_ARCHIVE               0x20                          /* 文件为存档属性 */
#define ATTR_LONG_NAME             ( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID )  /* 长文件名属性 */
#define ATTR_LONG_NAME_MASK        ( ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE )
/* 文件属性 uint8_t */
/* bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7 */
/*  只   隐   系   卷   目   存   未定义   */
/*  读   藏   统   标   录   档          */

/* 文件时间 uint16_t */
/* Time = (Hour<<11) + (Minute<<5) + (Second>>1) */
#define MAKE_FILE_TIME( h, m, s )  ( (h<<11) + (m<<5) + (s>>1) ) /* 生成指定时分秒的文件时间数据 */

/* 文件日期 uint16_t */
/* Date = ((Year-1980)<<9) + (Month<<5) + Day */
#define MAKE_FILE_DATE( y, m, d )  ( ((y-1980)<<9) + (m<<5) + d )/* 生成指定年月日的文件日期数据 */
#define LONE_NAME_MAX_CHAR         (255*2)                       /* 长文件名最多字符数/字节数 */
#define LONG_NAME_PER_DIR          (13*2)                        /* 长文件名在每个文件目录信息结构中的字符数/字节数 */

#endif

/* ********************************************************************************************************************* */
/* 主机文件模式下的数据输入和输出结构 */
#define MAX_PATH_LEN            128                              /* 最大路径长度,含所有斜杠分隔符和小数点间隔符以及路径结束符00H */
#define MAX_FILE_NAME_LEN       128                              /* 最大路径长度,含所有斜杠分隔符和小数点间隔符以及路径结束符00H */

/* ********************************************************************************************************************* */
/* 主机文件模式下的文件系统变量的地址 */
#ifndef VAR_FILE_SIZE

/* 8位/单字节变量 */
#define VAR8_DEV_CONNECTSTATUS     0x02                          /* 当前设备连接状态: 0--未连接, 1--连接未初始化, 2--初始化成功 */
#define VAR8_USB_DEV_SPEED         0x03                          /* 当前连接的USB设备速度: 0--未知, 1--低速, 2--全速, 3--高速 */
#define VAR8_CMD_INT_STATUS        0x0B                          /* CH378当前命令执行中断状态(针对产生中断的命令) */
#define VAR8_LAST_CMD              0x0C                          /* CH378接收到的最后一个命令码 */
#define VAR8_CMD_OP_STATUS         0x0E                          /* CH378当前命令的执行状态 */
#define VAR8_SDO_INT_ENABLE        0x16                          /* SPI模式是否使能SDO引脚中断通知标志(0:禁止, 1:使能) */
#define VAR8_DISK_STATUS           0x21                          /* 主机文件模式下的磁盘及文件状态 */
#define VAR8_DISK_FAT              0x22                          /* 逻辑盘的FAT标志:1=FAT12,2=FAT16,3=FAT32 */
#define VAR8_SEC_PER_CLUS          0x23                          /* 逻辑盘的每簇扇区数 */
#define VAR8_DISK_SEC_LEN          0x24                          /* 逻辑盘的每个扇区大小(一般为512字节,大扇区U盘可能为1024,2048,4096等) */
                                                                 /* 9 = 512, 10 = 1024, 11 = 2048, 12 = 4096 */
/* 32位/4字节变量 */
#define VAR32_DISK_CAP             0x51                          /* 逻辑盘的总容量 */
#define VAR32_DISK_SEC_SIZE        0x52                          /* 逻辑盘的每个扇区大小(一般为512字节,大扇区U盘可能为1024,2048,4096等) */
#define VAR32_SYSTEM_PARA          0x55                          /* CH378系统参数 */
#define  RB_WORK_MODE_SET0         ( 1 << 0 )                    /* 位1--0: 当前芯片接口模式( 00:未知; 01:8位并口; 02:SPI接口; 03:异步串口 ) */
#define  RB_WORK_MODE_SET1         ( 1 << 1 )
#define  RB_AUTO_CHECK_CONNECT     ( 1 << 2 )                    /* 位2：CH378自动检测USB设备/SD卡的连接和断开使能 */
#define  RB_HOST_SD_UDISK          ( 1 << 3 )                    /* 位3：主机模式：0---USB存储设备；1---SD卡 */
#define  RB_USB_CONFIG             ( 1 << 4 )                    /* 位4：主机方式下USB设备连接标志 */
#define VAR32_FILE_SIZE            0x68                          /* 当前文件的长度(总长度32位,低字节在前) */
#define VAR32_DSK_START_LBA        0x70                          /* 逻辑盘的起始绝对扇区号LBA(总长度32位,低字节在前) */
#define VAR32_DISK_ROOT            0x71                          /* 对于FAT16盘为根目录占用扇区数,对于FAT32盘为根目录起始簇号(总长度32位,低字节在前) */
#define VAR32_DSK_DAT_START        0x72                          /* 逻辑盘的数据区域的起始LBA(总长度32位,低字节在前) */
#define VAR32_START_CLUSTER        0x73                          /* 当前文件或者目录(文件夹)的起始簇号(总长度32位,低字节在前) */
#define VAR32_CURRENT_OFFSET       0x74                          /* 当前文件指针,当前读写位置的字节偏移(总长度32位,低字节在前) */
#define VAR_FAT_DIR_LBA            0x75                          /* 当前文件目录信息所在的扇区LBA地址(总长度32位,低字节在前) */
#define VAR_FAT_OFFSET             0x76                          /* 当前文件目录信息在扇区内偏移地址(总长度32位,低字节在前) */
#endif


#define STRUCT_OFFSET( s, m )      ( (uint8_t)( & ((s *)0) -> m ) )             /* 定义获取结构成员相对偏移地址的宏 */
#define MAX_BYTE_PER_OPERATE       20480                                        /* 最底层操作每次读写最大字节数 */


typedef struct
{
    uint16_t sector_size; // 保存当前介质的逻辑扇区大小
} CH378_File_Device_State; // CH378 文件命令层介质参数状态

extern CH378_File_Device_State g_ch378_file_device_state;


/*
 * 说明：读取当前磁盘及文件系统的工作状态。
 * 输入：无。
 * 输出：返回 DEF_DISK_* 磁盘工作状态值。
 */
uint8_t CH378GetDiskStatus(void);

/*
 * 说明：检查当前 U盘或 SD 卡是否已经连接。
 * 输入：无。
 * 输出：连接正常返回 ERR_SUCCESS，否则返回 CH378 状态码。
 */
uint8_t CH378DiskConnect(void);

/*
 * 说明：挂载并初始化当前磁盘文件系统。
 * 输入：无。
 * 输出：挂载成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskReady(void);

/*
 * 说明：创建并打开指定文件。
 * 输入：PathName：以空字符结尾的文件路径。
 * 输出：创建成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileCreate(uint8_t *PathName);

/*
 * 说明：逐级创建或打开指定目录。
 * 输入：PathName：以空字符结尾的目录路径。
 * 输出：创建成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileDirCreate(uint8_t *PathName);

/*
 * 说明：打开指定文件或目录。
 * 输入：PathName：以空字符结尾的文件或目录路径。
 * 输出：打开成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileOpen(uint8_t *PathName);

/*
 * 说明：关闭当前文件，并按参数决定是否更新文件长度及 FAT 信息。
 * 输入：UpdateSz：0 不更新文件长度，1 更新文件长度。
 * 输出：关闭成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileClose(uint8_t UpdateSz);

/*
 * 说明：从当前文件位置读取指定长度的数据。
 * 输入：buf：接收缓冲区；ReqCount：请求读取字节数；RealCount：实际读取字节数输出地址。
 * 输出：读取成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteRead(uint8_t *buf, uint16_t ReqCount, uint32_t *RealCount);

/*
 * 说明：从当前文件位置写入指定长度的数据。
 * 输入：buf：待写入数据；ReqCount：请求写入字节数；RealCount：实际写入字节数输出地址。
 * 输出：写入成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteWrite(uint8_t *buf, uint16_t ReqCount, uint16_t *RealCount);

/*
 * 说明：删除指定文件。
 * 输入：PathName：以空字符结尾的文件路径。
 * 输出：删除成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileErase(uint8_t *PathName);

/*
 * 说明：将当前文件指针移动到指定字节偏移。
 * 输入：offset：相对于文件开头的目标字节偏移。
 * 输出：定位成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteLocate(uint32_t offset);

/*
 * 说明：查询当前已打开文件的信息。
 * 输入：buf：接收文件信息的缓冲区地址。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileQuery(uint8_t *buf);

/*
 * 说明：获取当前已打开文件的长度。
 * 输入：无。
 * 输出：返回当前文件长度，单位：字节。
 */
uint32_t CH378GetFileSize(void);

/*
 * 说明：查询当前磁盘的总容量。
 * 输入：DiskCap：磁盘总容量输出地址。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskCapacity(uint32_t *DiskCap);

/*
 * 说明：查询当前磁盘的剩余空间。
 * 输入：DiskFre：磁盘剩余空间输出地址。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskQuery(uint32_t *DiskFre);

/*
 * 说明：读取 CH378 中断状态并清除当前中断请求。
 * 输入：无。
 * 输出：返回当前 CH378 中断状态码。
 */
uint8_t CH378GetIntStatus(void);

#endif /* CH378_FILE_DEVICE_H_ */
