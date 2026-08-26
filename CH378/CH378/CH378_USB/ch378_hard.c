/*
 * 本文件是 CH378L 的 RA MCU 硬件适配层。
 * 数据总线 D0 至 D7 连接 P500 至 P507，PCS、A0、WR、RD 构成 8 位并口控制时序，INT# 使用外部中断 10。
 * 上层文件命令只能通过本文件提供的命令写、数据写和数据读接口访问总线。
 */

#include "ch378_hard.h"
#include "ch378_file_device.h"

volatile CH378_Hard_State g_ch378_hard_state =
{
    .command_status = 0U,
    .interrupt_completed = 0U,
    .selected_host_mode = CH378_HOST_MODE_USB_DISK
};

// -------------------------------- 硬件驱动 --------------------------------
/*
 * 说明：执行 CH378 8 位并行总线所需的短软件延时。
 * 输入：time：软件延时循环次数。
 * 输出：无。
 */
static void ch378_delay_hard(uint16_t time)
{
    uint16_t i;
    for(i = 0 ; i < time ; i++)
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
}

/*
 * 说明：设置 CH378 复位引脚电平。
 * 输入：sta：需要输出到复位引脚的电平。
 * 输出：无。
 */
static void ch378_rst(uint8_t sta)
{
   R_IOPORT_PinWrite(&g_ioport_ctrl, RST , sta);
}

/*
 * 说明：设置 CH378 并口片选引脚电平。
 * 输入：sta：需要输出到片选引脚的电平。
 * 输出：无。
 */
static void ch378_pcs(uint8_t sta)
{
   R_IOPORT_PinWrite(&g_ioport_ctrl, PCS , sta);
}

/*
 * 说明：设置 CH378 命令或数据地址选择引脚电平。
 * 输入：sta：需要输出到地址选择引脚的电平。
 * 输出：无。
 */
static void ch378_ao(uint8_t sta)
{
   R_IOPORT_PinWrite(&g_ioport_ctrl, AO , sta);
}

/*
 * 说明：设置 CH378 并口写选通信号电平。
 * 输入：sta：需要输出到写选通信号的电平。
 * 输出：无。
 */
static void ch378_wr(uint8_t sta)
{
   R_IOPORT_PinWrite(&g_ioport_ctrl, WR , sta);
}

/*
 * 说明：设置 CH378 并口读选通信号电平。
 * 输入：sta：需要输出到读选通信号的电平。
 * 输出：无。
 */
static void ch378_rd(uint8_t sta)
{
   R_IOPORT_PinWrite(&g_ioport_ctrl, RD , sta);
}

/*
 * 说明：设置 CH378 8 位数据总线的输入输出方向。
 * 输入：sta：0 设置为输入方向，非 0 设置为输出方向。
 * 输出：无。
 */
static void ch378_data_direction(uint8_t sta)
{
   if(sta)
       R_IOPORT_PortDirectionSet(&g_ioport_ctrl, BSP_IO_PORT_05, 0x00FFU, 0x00FFU); // P500 至 P507 全部切换为输出
   else
       R_IOPORT_PortDirectionSet(&g_ioport_ctrl, BSP_IO_PORT_05, 0x0000U, 0x00FFU); // P500 至 P507 全部切换为输入
}

/*
 * 说明：将一个字节输出到 CH378 8 位数据总线。
 * 输入：data：需要输出到并行数据总线的字节。
 * 输出：无。
 */
static void ch378_data_write(uint8_t data)
{
    uint16_t write = data;

    R_IOPORT_PortWrite(&g_ioport_ctrl, BSP_IO_PORT_05, write, 0x00FFU); // 数据位连续映射到 P500 至 P507，只修改低 8 位
}

/*
 * 说明：从 CH378 8 位数据总线读取一个字节。
 * 输入：无。
 * 输出：返回从 8 位数据总线读取的字节。
 */
static uint8_t ch378_data_read(void)
{
    uint16_t p_port_value = 0;

    R_IOPORT_PortRead(&g_ioport_ctrl, BSP_IO_PORT_05, &p_port_value);

    p_port_value = p_port_value & 0x00FFU; // 数据位连续映射到端口低 8 位，无需执行引脚重排
    return (uint8_t)p_port_value;
}


/*
 * 说明：初始化 CH378 并口、复位引脚和外部中断。
 * 输入：无。
 * 输出：无。
 */
static void CH378_Port_Init(void)
{
    // 先将所有低有效控制线置为非选通状态，再拉低 RSTI 复位芯片
    ch378_pcs(1);
    ch378_wr(1);
    ch378_rd(1);
    ch378_ao(0);
    ch378_rst(0);
    R_ICU_ExternalIrqOpen(&g_external_irq10_ctrl, &g_external_irq10_cfg);
    R_ICU_ExternalIrqEnable(&g_external_irq10_ctrl);

    R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS); // RSTI 低脉冲至少 100 ns，取 1 ms 留足裕量
    ch378_rst(1);

    R_BSP_SoftwareDelay(60U, BSP_DELAY_UNITS_MILLISECONDS); // 复位释放后按 TRD 最大值等待芯片恢复
}

/*
 * 说明：通过 8 位并口向 CH378 写入一个命令码。
 * 输入：mCmd：需要写入 CH378 的命令码。
 * 输出：无。
 */
void xWriteCH378Cmd(uint8_t mCmd)
{
    ch378_data_write(mCmd);
    ch378_data_direction(1); // MCU 驱动数据总线
    // A0 为高表示命令端口，PCS 和 WR 依次拉低形成一次写命令周期
    ch378_ao(1);
    ch378_pcs(0);
    ch378_wr(0);
    ch378_delay_hard(2);
    ch378_wr(1);
    ch378_pcs(1);
    ch378_ao(0);
    ch378_data_direction(0); // 周期结束后释放总线，避免下一次读操作发生方向冲突
    ch378_delay_hard(50); // 命令译码时间长于普通数据写周期
}

/*
 * 说明：通过 8 位并口向 CH378 写入一个数据字节。
 * 输入：mData：需要写入 CH378 的数据字节。
 * 输出：无。
 */
void xWriteCH378Data( uint8_t mData )
{
    ch378_data_write(mData);
    ch378_data_direction(1); // MCU 驱动数据总线
    // A0 为低表示数据端口，PCS 和 WR 依次拉低形成一次写数据周期
    ch378_ao(0);
    // 数据总线默认保持输入方向，PCS 和 RD 拉低后等待数据稳定再采样
    ch378_pcs(0);
    ch378_wr(0);
    ch378_delay_hard(2);
    ch378_wr(1);
    ch378_pcs(1);
    ch378_ao(0);
    ch378_data_direction(0);
}

/*
 * 说明：通过 8 位并口从 CH378 读取一个数据字节。
 * 输入：无。
 * 输出：返回从 CH378 数据端口读取的字节。
 */
uint8_t xReadCH378Data( void )
{
    uint8_t  mData = 0;

    ch378_pcs(0);
    ch378_rd(0);
    ch378_ao(0);
    ch378_delay_hard(2);
    mData = ch378_data_read();
    ch378_rd(1);
    ch378_ao(0);
    ch378_pcs(1);

    return( mData );
}

/*
 * 说明：处理 CH378 INT# 外部中断并保存命令完成状态。
 * 输入：p_args：FSP 外部中断回调参数。
 * 输出：无。
 */
void USB_INT(external_irq_callback_args_t *p_args)
{
    if(p_args->channel == 10)
    {
        g_ch378_hard_state.command_status = 0xFFU;
        g_ch378_hard_state.interrupt_completed = 1U; // 记录 INT# 中断已经触发
        g_ch378_hard_state.command_status = (uint8_t) CH378GetIntStatus( ); // 读取状态同时清除 CH378 当前 INT# 请求
    }
}

/*
 * 说明：初始化 CH378 硬件接口并配置 U盘或 SD 卡主机模式。
 * 输入：mMode：04H 为 SD 卡模式，07H 为 U盘模式。
 * 输出：初始化结果，成功返回 ERR_SUCCESS。
 */
uint8_t Init_CH378Host(uint8_t mMode)
{
    uint8_t i , res;

    if((mMode != CH378_HOST_MODE_SD_CARD) && (mMode != CH378_HOST_MODE_USB_DISK))
    {
        return ERR_USB_UNKNOWN;
    }

    g_ch378_hard_state.selected_host_mode = mMode; // 保存当前 CH378 主机文件模式

    CH378_Port_Init( );                                          // 初始化 CH378 并行接口
    xWriteCH378Cmd( CMD11_CHECK_EXIST );                         // 测试 MCU 与 CH378 的通信接口
    xWriteCH378Data( 0x57 );                                    // 发送 57H，正常应读取到 A8H
    res = xReadCH378Data( );
    if( res != 0xA8 )
    {
        return( ERR_USB_UNKNOWN );
    }

    xWriteCH378Cmd( CMD11_SET_USB_MODE );                         // 设置 CH378 主机工作模式
    xWriteCH378Data( mMode );

    for( i = 0; i < 10; i++ )
    {
        R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MILLISECONDS);   // 按官方例程间隔等待模式设置完成
        res = xReadCH378Data( );
        if( res == CMD_RET_SUCCESS ) // 芯片已接受并完成主机模式切换
        {
            return( ERR_SUCCESS );
        }
    }
    return( ERR_USB_UNKNOWN );                                   // 设置模式失败
}

// -----------------------------------------------------------------------------
