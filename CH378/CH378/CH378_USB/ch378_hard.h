/*
 * CH378L 8 位并口硬件适配接口及本工程使用的基础命令码。
 * 引脚定义必须与 FSP Pin Configuration 和实际原理图保持一致。
 */

#ifndef CH378_HARD_H_
#define CH378_HARD_H_

#include "hal_data.h"

#define ERR_USB_UNKNOWN            0xFA                          /* 未知错误,不应该发生的情况,需检查硬件或者程序错误 */
#define ERR_SUCCESS                0x00                          /* 操作成功 */
#define CMD_RET_SUCCESS            0x51                          /* 命令操作成功 */
#define CMD11_SET_USB_MODE         0x15                          /* 设置USB工作模式 */
#define CMD11_CHECK_EXIST          0x06                          /* 测试通讯接口和工作状态 */
/* CH378 主机文件模式选择 */
#define CH378_HOST_MODE_SD_CARD    0x04U                         /* microSD 主机文件模式（官方并口例程使用 0x04） */
#define CH378_HOST_MODE_USB_DISK   0x07U                         /* U 盘主机文件模式 */

typedef struct
{
    uint8_t command_status; // 保存最近一次 CH378 命令完成状态
    uint8_t interrupt_completed; // 保存 CH378 中断是否已经完成
    uint8_t selected_host_mode; // 保存当前选择的 U盘或 SD 卡主机模式
} CH378_Hard_State; // CH378 硬件命令、中断和主机模式状态

extern volatile CH378_Hard_State g_ch378_hard_state;


/*
 * 说明：通过 8 位并口向 CH378 写入一个命令码。
 * 输入：mCmd：需要写入 CH378 的命令码。
 * 输出：无。
 */
void xWriteCH378Cmd(uint8_t mCmd);

/*
 * 说明：通过 8 位并口向 CH378 写入一个数据字节。
 * 输入：mData：需要写入 CH378 的数据字节。
 * 输出：无。
 */
void xWriteCH378Data(uint8_t mData);

/*
 * 说明：通过 8 位并口从 CH378 读取一个数据字节。
 * 输入：无。
 * 输出：返回从 CH378 数据端口读取的字节。
 */
uint8_t xReadCH378Data(void);

/*
 * 说明：初始化 CH378 硬件接口并配置 U盘或 SD 卡主机模式。
 * 输入：mMode：0x04 选择 SD 卡模式，0x07 选择 U盘模式。
 * 输出：初始化成功返回 ERR_SUCCESS，否则返回错误码。
 */
uint8_t Init_CH378Host(uint8_t mMode);


#endif /* CH378_HARD_H_ */
