#include "F_HMI.h"

#include <string.h>

/*
 * 本文件是串口屏协议封装层，只负责按照大彩协议组装命令帧。
 * 动态帧共用全局缓冲区以减少栈占用；hmi_send_frame() 同步等待发送完成，因此可以安全复用。
 * 本层不解析业务数据，也不保存页面状态。
 */

#define HMI_FRAME_HEADER              0xEEU // 大彩串口屏协议帧头
#define HMI_COMMAND_GROUP             0xB1U // 大彩串口屏组态指令
#define HMI_RECORD_ADD_COMMAND        0x52U // 数据记录控件添加常规记录指令
#define HMI_FRAME_TAIL_BYTE_0         0xFFU // 大彩串口屏协议帧尾第一个字节
#define HMI_FRAME_TAIL_BYTE_1         0xFCU // 大彩串口屏协议帧尾第二个字节
#define HMI_FRAME_TAIL_BYTE_2         0xFFU // 大彩串口屏协议帧尾第三个字节
#define HMI_FRAME_TAIL_BYTE_3         0xFFU // 大彩串口屏协议帧尾第四个字节
#define HMI_RECORD_MAX_LENGTH         256U  // 数据记录控件允许的最大单条记录长度
#define HMI_RECORD_FRAME_OVERHEAD     11U   // 帧头、指令、控件地址和帧尾占用的字节数
#define HMI_RECORD_CLEAR_COMMAND      0x53U // 数据记录控件清除全部记录指令
#define HMI_CONTROL_SET_COMMAND       0x10U // 组态控件内容设置指令
#define HMI_TEXT_MAX_LENGTH             16U // 查询输入文本允许的最大长度
#define HMI_RTC_READ_COMMAND            0x82U // 串口屏系统 RTC 时间读取指令

typedef struct
{
    uint8_t transmit_frame[HMI_RECORD_MAX_LENGTH + HMI_RECORD_FRAME_OVERHEAD]; // 保存当前待发送的屏幕协议帧
} HMI_Protocol_State; // 串口屏协议帧组装状态

static HMI_Protocol_State g_hmi_protocol_state; // 保存当前待发送的串口屏协议帧

/*
 * 说明：向指定画面的数据记录控件追加一条分号分隔的常规记录。
 * 输入：screen_id：画面 ID；control_id：控件 ID；record：待追加的记录字符串。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_record_add(uint16_t screen_id, uint16_t control_id, const uint8_t *record)
{
    size_t record_length;
    uint16_t frame_index = 0U;

    if(record == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }

    record_length = strlen((const char *) record);
    if((record_length == 0U) || (record_length > HMI_RECORD_MAX_LENGTH))
    {
        return FSP_ERR_INVALID_SIZE;
    }

    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_HEADER;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_COMMAND_GROUP;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_RECORD_ADD_COMMAND;
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) (screen_id >> 8U);
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) screen_id;
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) (control_id >> 8U);
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) control_id;
    // 记录正文使用分号分列，VisualTFT 数据记录控件按配置的四个子项目依次填充
    memcpy(&g_hmi_protocol_state.transmit_frame[frame_index], record, record_length);
    frame_index = (uint16_t) (frame_index + (uint16_t) record_length);
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_0;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_1;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_2;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_3;

    return hmi_send_frame(g_hmi_protocol_state.transmit_frame, frame_index);
}

/*
 * 说明：清除指定画面数据记录控件中的全部记录。
 * 输入：screen_id：画面 ID；control_id：控件 ID。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_record_clear(uint16_t screen_id, uint16_t control_id)
{
    const uint8_t frame[] =
    {
        HMI_FRAME_HEADER,
        HMI_COMMAND_GROUP,
        HMI_RECORD_CLEAR_COMMAND,
        (uint8_t) (screen_id >> 8U),
        (uint8_t) screen_id,
        (uint8_t) (control_id >> 8U),
        (uint8_t) control_id,
        HMI_FRAME_TAIL_BYTE_0,
        HMI_FRAME_TAIL_BYTE_1,
        HMI_FRAME_TAIL_BYTE_2,
        HMI_FRAME_TAIL_BYTE_3
    };

    return hmi_send_frame(frame, (uint16_t) sizeof(frame));
}

/*
 * 说明：设置指定画面文本控件的显示内容，用于同步查询条件默认值。
 * 输入：screen_id：画面 ID；control_id：控件 ID；text：待显示的文本。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_text_set(uint16_t screen_id, uint16_t control_id, const uint8_t *text)
{
    size_t text_length;
    uint16_t frame_index = 0U;

    if(text == NULL)
    {
        return FSP_ERR_INVALID_POINTER;
    }

    text_length = strlen((const char *) text);
    if((text_length == 0U) || (text_length > HMI_TEXT_MAX_LENGTH))
    {
        return FSP_ERR_INVALID_SIZE;
    }

    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_HEADER;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_COMMAND_GROUP;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_CONTROL_SET_COMMAND;
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) (screen_id >> 8U);
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) screen_id;
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) (control_id >> 8U);
    g_hmi_protocol_state.transmit_frame[frame_index++] = (uint8_t) control_id;
    // 文本内容直接跟随画面和控件地址，不附加 C 字符串结束符
    memcpy(&g_hmi_protocol_state.transmit_frame[frame_index], text, text_length);
    frame_index = (uint16_t) (frame_index + (uint16_t) text_length);
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_0;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_1;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_2;
    g_hmi_protocol_state.transmit_frame[frame_index++] = HMI_FRAME_TAIL_BYTE_3;

    return hmi_send_frame(g_hmi_protocol_state.transmit_frame, frame_index);
}

/*
 * 说明：向串口屏发送系统 RTC 时间读取指令。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_rtc_read_request(void)
{
    const uint8_t frame[] =
    {
        HMI_FRAME_HEADER,
        HMI_RTC_READ_COMMAND,
        HMI_FRAME_TAIL_BYTE_0,
        HMI_FRAME_TAIL_BYTE_1,
        HMI_FRAME_TAIL_BYTE_2,
        HMI_FRAME_TAIL_BYTE_3
    };

    return hmi_send_frame(frame, (uint16_t) sizeof(frame));
}
