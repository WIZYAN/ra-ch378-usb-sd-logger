#include "H_hmi.h"

#include <string.h>

/*
 * 本文件是串口屏硬件适配层，负责电源、SCI2、同步发送和中断接收。
 * 中断回调只收集字节和设置标志，协议解析与业务处理均在主循环完成。
 * 接收区只保存一帧未处理数据，主循环应持续调用 app_hmi_process_input() 及时取走。
 */

#define HMI_POWER_STARTUP_DELAY_MS 5000U // 串口屏上电后的启动等待时间
#define HMI_TRANSMIT_TIMEOUT_MS    100U  // 单帧串口发送的最大等待时间
#define HMI_UART_BAUD_RATE       115200U // 与 VisualTFT 工程一致的串口波特率
#define HMI_UART_MAX_ERROR_X_1000  5000U // 允许的最大波特率误差为 5%
#define HMI_RECEIVE_FRAME_LENGTH    320U // 串口屏单帧接收缓冲区长度
#define HMI_QUERY_FRAME_LENGTH        6U // 查询按钮自定义指令长度
#define HMI_FRAME_TAIL_LENGTH          4U // 大彩协议帧尾长度

typedef struct
{
    volatile uint8_t transmit_complete; // 当前串口帧是否发送完成
    uint8_t receive_buffer[HMI_RECEIVE_FRAME_LENGTH]; // 保存串口屏上传的完整协议帧
    volatile uint16_t receive_length; // 当前正在接收的协议帧长度
    volatile uint8_t receive_frame_ready; // 是否有一帧完整协议等待处理
    uint8_t frame_tail_index; // 当前已经匹配的协议帧尾字节数
    uint8_t query_frame_index; // 当前已经匹配的查询按钮指令字节数
    volatile uint8_t query_requested; // 是否收到查询按钮松开指令
} HMI_Hardware_State; // 串口屏发送、接收和查询按钮请求状态

static HMI_Hardware_State g_hmi_hardware_state; // 保存串口屏硬件通信状态

/*
 * 说明：使能串口屏电源和通信接口，打开 SCI2 并等待屏幕完成启动。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_hardware_init(void)
{
    baud_setting_t baud_setting;
    fsp_err_t result;

    result = R_IOPORT_PinWrite(&g_ioport_ctrl, DIS_POWER, BSP_IO_LEVEL_HIGH);
    if(result != FSP_SUCCESS)
    {
        return result;
    }

    result = R_IOPORT_PinWrite(&g_ioport_ctrl, DIS_EN, BSP_IO_LEVEL_HIGH);
    if(result != FSP_SUCCESS)
    {
        return result;
    }

    result = R_SCI_UART_Open(&g_uart2_ctrl, &g_uart2_cfg);
    if(result != FSP_SUCCESS)
    {
        return result;
    }

    result = R_SCI_UART_BaudCalculate(HMI_UART_BAUD_RATE,
                                      false,
                                      HMI_UART_MAX_ERROR_X_1000,
                                      &baud_setting);
    if(result == FSP_SUCCESS)
    {
        result = R_SCI_UART_BaudSet(&g_uart2_ctrl, &baud_setting); // 将 SCI2 设置为 VisualTFT 使用的 115200 波特率
    }
    if(result != FSP_SUCCESS)
    {
        (void) R_SCI_UART_Close(&g_uart2_ctrl);
        return result;
    }

    R_BSP_SoftwareDelay(HMI_POWER_STARTUP_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS); // SCI2 就绪后等待串口屏完成上电和页面加载
    return FSP_SUCCESS;
}

/*
 * 说明：通过 SCI2 发送一帧串口屏数据，并阻塞等待发送完成。
 * 输入：data：发送数据地址；length：发送数据长度。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_send_frame(const uint8_t *data, uint16_t length)
{
    fsp_err_t result;
    uint16_t wait_count;

    if(data == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }
    if(length == 0U)
    {
        return FSP_ERR_INVALID_SIZE;
    }

    g_hmi_hardware_state.transmit_complete = 0U; // 发送缓冲区在 TX_COMPLETE 前必须有效，因此本接口阻塞等待
    result = R_SCI_UART_Write(&g_uart2_ctrl, data, (uint32_t) length);
    if(result != FSP_SUCCESS)
    {
        return result;
    }

    for(wait_count = 0U; wait_count < HMI_TRANSMIT_TIMEOUT_MS; wait_count++)
    {
        if(g_hmi_hardware_state.transmit_complete != 0U)
        {
            return FSP_SUCCESS;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS); // 等待 SCI2 发送完成中断
    }

    (void) R_SCI_UART_Abort(&g_uart2_ctrl, UART_DIR_TX); // 超时后停止发送，避免发送缓冲区失效
    return FSP_ERR_TIMEOUT;
}

/*
 * 说明：取出一帧由串口屏上传的完整大彩协议数据，取出成功后释放接收缓冲区。
 * 输入：buffer：接收帧缓冲区；buffer_size：缓冲区大小；frame_length：完整帧长度。
 * 输出：成功取出返回 1，没有完整帧或参数无效返回 0。
 */
uint8_t hmi_receive_frame_take(uint8_t *buffer, uint16_t buffer_size, uint16_t *frame_length)
{
    uint16_t receive_length;

    if((buffer == NULL) || (frame_length == NULL))
    {
        return 0U;
    }
    if(g_hmi_hardware_state.receive_frame_ready == 0U)
    {
        return 0U;
    }

    receive_length = g_hmi_hardware_state.receive_length;
    if((receive_length == 0U) || (receive_length > buffer_size))
    {
        return 0U;
    }

    memcpy(buffer, g_hmi_hardware_state.receive_buffer, receive_length);
    *frame_length = receive_length;
    g_hmi_hardware_state.receive_length = 0U;
    g_hmi_hardware_state.frame_tail_index = 0U;
    g_hmi_hardware_state.receive_frame_ready = 0U; // 最后释放缓冲区，允许中断接收下一帧
    return 1U;
}

/*
 * 说明：读取并清除查询按钮请求标志，保证一次按钮松开只执行一次查询。
 * 输入：无。
 * 输出：存在查询请求返回 1，否则返回 0。
 */
uint8_t hmi_query_request_take(void)
{
    if(g_hmi_hardware_state.query_requested == 0U)
    {
        return 0U;
    }

    g_hmi_hardware_state.query_requested = 0U;
    return 1U;
}

/*
 * 说明：处理 SCI2 串口事件，记录发送完成状态并接收屏幕上传帧和查询按钮指令。
 * 输入：p_args：SCI2 回调事件参数。
 * 输出：无。
 */
void uart2_dis_callback(uart_callback_args_t *p_args)
{
    const uint8_t query_frame[HMI_QUERY_FRAME_LENGTH] = {0xA5U, 0x5AU, 0x03U, 0x00U, 0x07U, 0x01U};
    const uint8_t frame_tail[HMI_FRAME_TAIL_LENGTH] = {0xFFU, 0xFCU, 0xFFU, 0xFFU};
    uint8_t receive_data;

    if(p_args == NULL)
    {
        return;
    }

    if(p_args->event == UART_EVENT_TX_COMPLETE)
    {
        g_hmi_hardware_state.transmit_complete = 1U;
    }
    else if(p_args->event == UART_EVENT_RX_CHAR)
    {
        receive_data = (uint8_t) p_args->data;

        // 查询按钮使用 VisualTFT 配置的 A5 5A 03 00 07 01 指令，与 EE 协议帧并行匹配
        if(receive_data == query_frame[g_hmi_hardware_state.query_frame_index])
        {
            g_hmi_hardware_state.query_frame_index++;
        }
        else if(receive_data == query_frame[0])
        {
            g_hmi_hardware_state.query_frame_index = 1U;
        }
        else
        {
            g_hmi_hardware_state.query_frame_index = 0U;
        }

        if(g_hmi_hardware_state.query_frame_index >= HMI_QUERY_FRAME_LENGTH)
        {
            g_hmi_hardware_state.query_requested = 1U;
            g_hmi_hardware_state.query_frame_index = 0U;
        }

        if(g_hmi_hardware_state.receive_frame_ready == 0U)
        {
            // 普通上传帧必须以 EE 开始；旧帧未取走时不覆盖接收缓冲区
            if((g_hmi_hardware_state.receive_length == 0U) && (receive_data != 0xEEU))
            {
                return;
            }

            if(g_hmi_hardware_state.receive_length < HMI_RECEIVE_FRAME_LENGTH)
            {
                g_hmi_hardware_state.receive_buffer[g_hmi_hardware_state.receive_length] = receive_data;
                g_hmi_hardware_state.receive_length++;
            }
            else
            {
                // 超长帧视为损坏帧并丢弃，下一次遇到 EE 时重新开始收帧
                g_hmi_hardware_state.receive_length = 0U;
                g_hmi_hardware_state.frame_tail_index = 0U;
                return;
            }

            // 连续匹配 FF FC FF FF 帧尾，失配时保留可能成为新帧尾首字节的 FF
            if(receive_data == frame_tail[g_hmi_hardware_state.frame_tail_index])
            {
                g_hmi_hardware_state.frame_tail_index++;
            }
            else if(receive_data == frame_tail[0])
            {
                g_hmi_hardware_state.frame_tail_index = 1U;
            }
            else
            {
                g_hmi_hardware_state.frame_tail_index = 0U;
            }

            if(g_hmi_hardware_state.frame_tail_index >= HMI_FRAME_TAIL_LENGTH)
            {
                g_hmi_hardware_state.receive_frame_ready = 1U;
            }
        }
    }
}

/*
 * 说明：保留 FSP 中已配置的 5 ms 定时器回调，当前发送显示功能不使用该定时器。
 * 输入：p_args：定时器回调事件参数。
 * 输出：无。
 */
void g_timer1_AGT_5ms_callback(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
}
