#include "app_hmi.h"

#include <stdio.h>
#include <string.h>

/*
 * 本文件是串口屏业务适配层，负责查询条件、RTC 时间和存储记录显示。
 * H_hmi.c 处理字节收发，F_HMI.c 组装协议帧，本文件只解释控件 ID 和业务数据格式。
 */

#define HMI_STORAGE_SCREEN_ID       0U   // 存储数据显示画面 ID
#define HMI_STORAGE_CONTROL_ID      1U   // 存储数据显示表格控件 ID
#define HMI_PARAMETER_TEXT_LENGTH   24U  // 单个参数文本的最大长度
#define HMI_TABLE_RECORD_LENGTH     128U // 一条四列表格记录的缓冲区长度
#define HMI_STORAGE_LINE_LENGTH     128U // 单条存储记录的解析缓冲区长度
#define HMI_RECEIVE_FRAME_LENGTH    320U // 屏幕上传协议帧的处理缓冲区长度
#define HMI_NOTIFY_CONTROL          0xB1U // 屏幕控件更新通知类型
#define HMI_CONTROL_DATA_MESSAGE    0x11U // 屏幕控件数据通知消息
#define HMI_TEXT_CONTROL_TYPE       0x11U // 屏幕文本控件类型
#define HMI_QUERY_YEAR_CONTROL_ID      2U // 年输入文本控件 ID
#define HMI_QUERY_MONTH_CONTROL_ID     3U // 月输入文本控件 ID
#define HMI_QUERY_DAY_CONTROL_ID       4U // 日输入文本控件 ID
#define HMI_QUERY_HOUR_CONTROL_ID      5U // 小时输入文本控件 ID
#define HMI_QUERY_MINUTE_CONTROL_ID    6U // 分钟输入文本控件 ID
#define HMI_RTC_RESPONSE_COMMAND     0xF7U // 串口屏系统 RTC 时间返回指令
#define HMI_RTC_RESPONSE_LENGTH        13U // RTC 返回帧总长度
#define HMI_RTC_BASE_YEAR            2000U // RTC 两位年份转换使用的起始年份

typedef struct
{
    uint8_t receive_frame[HMI_RECEIVE_FRAME_LENGTH]; // 保存待解析的屏幕上传帧
    uint16_t receive_length; // 保存待解析屏幕上传帧的长度
    HMI_Query_Time query_time; // 保存屏幕输入的当前查询时间
    HMI_RTC_Time rtc_time; // 保存串口屏最近一次返回的系统 RTC 时间
    uint8_t rtc_time_ready; // 是否存在一组尚未取出的有效 RTC 时间
    char storage_line[HMI_STORAGE_LINE_LENGTH]; // 保存当前正在解析的单条存储记录
    char temperature[HMI_PARAMETER_TEXT_LENGTH]; // 保存当前记录的温度文本
    char humidity[HMI_PARAMETER_TEXT_LENGTH]; // 保存当前记录的湿度文本
    char pressure[HMI_PARAMETER_TEXT_LENGTH]; // 保存当前记录的压力文本
    char table_record[HMI_TABLE_RECORD_LENGTH]; // 保存发送给表格控件的四列记录文本
} HMI_Query_State; // 串口屏查询条件、接收帧和记录显示工作状态

static HMI_Query_State g_hmi_query_state =
{
    .query_time =
    {
        .year = 2026U,
        .month = 5U,
        .day = 6U,
        .hour = 16U,
        .minute = 46U
    }
}; // 保存当前串口屏查询和记录显示状态

/*
 * 说明：把文本控件上传的十进制数字转换为无符号整数。
 * 输入：text：数字文本地址；text_length：文本长度；value：转换结果地址。
 * 输出：转换成功返回 1，文本为空、包含非数字或数值溢出返回 0。
 */
static uint8_t app_hmi_parse_number(const uint8_t *text, uint16_t text_length, uint16_t *value)
{
    uint32_t number = 0U;
    uint16_t text_index;

    if((text == NULL) || (value == NULL) || (text_length == 0U))
    {
        return 0U;
    }

    if(text[text_length - 1U] == 0x00U)
    {
        text_length--; // 大彩文本控件在上传字符串末尾附加一个 0x00 结束符
    }
    if(text_length == 0U)
    {
        return 0U;
    }

    for(text_index = 0U; text_index < text_length; text_index++)
    {
        if((text[text_index] < (uint8_t) '0') || (text[text_index] > (uint8_t) '9'))
        {
            return 0U;
        }

        number = (number * 10U) + (uint32_t) (text[text_index] - (uint8_t) '0');
        if(number > 65535U)
        {
            return 0U;
        }
    }

    *value = (uint16_t) number;
    return 1U;
}

/*
 * 说明：将串口屏 RTC 返回帧中的一个 BCD 字节转换为十进制数值。
 * 输入：bcd_value：需要转换的 BCD 字节；value：十进制结果输出地址。
 * 输出：BCD 格式有效返回 1，否则返回 0。
 */
static uint8_t app_hmi_bcd_to_decimal(uint8_t bcd_value, uint8_t *value)
{
    uint8_t high_digit;
    uint8_t low_digit;

    if(value == NULL)
    {
        return 0U;
    }

    high_digit = (uint8_t) ((bcd_value >> 4U) & 0x0FU);
    low_digit = (uint8_t) (bcd_value & 0x0FU);
    if((high_digit > 9U) || (low_digit > 9U))
    {
        return 0U;
    }

    *value = (uint8_t) ((high_digit * 10U) + low_digit);
    return 1U;
}

/*
 * 说明：解析串口屏返回的系统 RTC 帧并保存有效年月日时分秒。
 * 输入：frame：串口屏返回帧；frame_length：返回帧长度。
 * 输出：无。
 */
static void app_hmi_update_rtc_time(const uint8_t *frame, uint16_t frame_length)
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    if((frame == NULL) || (frame_length != HMI_RTC_RESPONSE_LENGTH) ||
       (frame[0] != 0xEEU) || (frame[1] != HMI_RTC_RESPONSE_COMMAND))
    {
        return;
    }
    // RTC 帧格式：EE F7 年 月 星期 日 时 分 秒 FF FC FF FF，星期字段 frame[4] 不参与存储
    if((app_hmi_bcd_to_decimal(frame[2], &year) == 0U) ||
       (app_hmi_bcd_to_decimal(frame[3], &month) == 0U) ||
       (app_hmi_bcd_to_decimal(frame[5], &day) == 0U) ||
       (app_hmi_bcd_to_decimal(frame[6], &hour) == 0U) ||
       (app_hmi_bcd_to_decimal(frame[7], &minute) == 0U) ||
       (app_hmi_bcd_to_decimal(frame[8], &second) == 0U))
    {
        return;
    }
    if((month < 1U) || (month > 12U) || (day < 1U) || (day > 31U) ||
       (hour > 23U) || (minute > 59U) || (second > 59U))
    {
        return;
    }

    g_hmi_query_state.rtc_time.year = (uint16_t) (HMI_RTC_BASE_YEAR + year);
    g_hmi_query_state.rtc_time.month = month;
    g_hmi_query_state.rtc_time.day = day;
    g_hmi_query_state.rtc_time.hour = hour;
    g_hmi_query_state.rtc_time.minute = minute;
    g_hmi_query_state.rtc_time.second = second;
    g_hmi_query_state.rtc_time_ready = 1U;
}

/*
 * 说明：解析文本控件数据通知，并按控件 ID 更新当前年月日时分查询条件。
 * 输入：frame：屏幕上传协议帧；frame_length：协议帧长度。
 * 输出：无。
 */
static void app_hmi_update_query_time(const uint8_t *frame, uint16_t frame_length)
{
    uint16_t screen_id;
    uint16_t control_id;
    uint16_t parameter_length;
    uint16_t value;

    if((frame == NULL) || (frame_length < 13U))
    {
        return;
    }
    if((frame[0] != 0xEEU) ||
       (frame[1] != HMI_NOTIFY_CONTROL) ||
       (frame[2] != HMI_CONTROL_DATA_MESSAGE) ||
       (frame[7] != HMI_TEXT_CONTROL_TYPE))
    {
        return;
    }

    screen_id = (uint16_t) (((uint16_t) frame[3] << 8U) | frame[4]);
    control_id = (uint16_t) (((uint16_t) frame[5] << 8U) | frame[6]);
    if(screen_id != HMI_STORAGE_SCREEN_ID)
    {
        return;
    }

    // 控件帧固定区占 8 字节，末尾占 4 字节，中间区域为文本和可选的 00 结束符
    parameter_length = (uint16_t) (frame_length - 12U);
    if(app_hmi_parse_number(&frame[8], parameter_length, &value) == 0U)
    {
        return;
    }

    if(control_id == HMI_QUERY_YEAR_CONTROL_ID)
    {
        g_hmi_query_state.query_time.year = value;
    }
    else if(control_id == HMI_QUERY_MONTH_CONTROL_ID)
    {
        g_hmi_query_state.query_time.month = (uint8_t) value;
    }
    else if(control_id == HMI_QUERY_DAY_CONTROL_ID)
    {
        g_hmi_query_state.query_time.day = (uint8_t) value;
    }
    else if(control_id == HMI_QUERY_HOUR_CONTROL_ID)
    {
        g_hmi_query_state.query_time.hour = (uint8_t) value;
    }
    else if(control_id == HMI_QUERY_MINUTE_CONTROL_ID)
    {
        g_hmi_query_state.query_time.minute = (uint8_t) value;
    }
    else
    {
        // 其他文本控件与存储查询无关
    }
}

/*
 * 说明：把一个查询时间数值转换为文本并同步到指定屏幕文本控件。
 * 输入：control_id：文本控件 ID；value：需要显示的数值。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
static fsp_err_t app_hmi_set_query_text(uint16_t control_id, uint16_t value)
{
    uint8_t text[8] = {0};
    int text_length;

    text_length = snprintf((char *) text, sizeof(text), "%u", (unsigned int) value);
    if((text_length < 0) || ((size_t) text_length >= sizeof(text)))
    {
        return FSP_ERR_INVALID_SIZE;
    }

    return hmi_text_set(HMI_STORAGE_SCREEN_ID, control_id, text);
}

/*
 * 说明：从存储记录中提取指定名称后面的参数文本。
 * 输入：storage_data：原始存储记录；parameter_name：参数名称；value：参数文本缓冲区；value_size：缓冲区大小。
 * 输出：成功返回 1，失败返回 0。
 */
static uint8_t app_hmi_extract_parameter(const uint8_t *storage_data,
                                         const char *parameter_name,
                                         char *value,
                                         size_t value_size)
{
    const char *parameter_start;
    size_t parameter_length;

    parameter_start = strstr((const char *) storage_data, parameter_name);
    if(parameter_start == NULL)
    {
        return 0U;
    }

    parameter_start += strlen(parameter_name);
    parameter_length = strcspn(parameter_start, ";\r\n"); // 参数在分号或当前记录行尾处结束
    if((parameter_length == 0U) || (parameter_length >= value_size))
    {
        return 0U;
    }

    memcpy(value, parameter_start, parameter_length);
    value[parameter_length] = '\0';
    return 1U;
}

/*
 * 说明：按固定格式校验记录开头的年月日时分秒文本，并提取秒数。
 * 输入：storage_line：单条存储记录；second：秒数输出地址。
 * 输出：格式有效返回 1，格式错误或参数无效返回 0。
 */
static uint8_t app_hmi_parse_record_second(const char *storage_line, uint8_t *second)
{
    static const uint8_t digit_indexes[] =
    {
        0U, 1U, 2U, 3U, 5U, 6U, 8U, 9U,
        11U, 12U, 14U, 15U, 17U, 18U
    };
    uint8_t index;

    if((storage_line == NULL) || (second == NULL) || (strlen(storage_line) < 20U))
    {
        return 0U;
    }
    if((storage_line[4] != '-') || (storage_line[7] != '-') ||
       (storage_line[10] != ' ') || (storage_line[13] != ':') ||
       (storage_line[16] != ':') || (storage_line[19] != ';'))
    {
        return 0U;
    }

    for(index = 0U; index < (uint8_t) sizeof(digit_indexes); index++)
    {
        if((storage_line[digit_indexes[index]] < '0') ||
           (storage_line[digit_indexes[index]] > '9'))
        {
            return 0U;
        }
    }

    *second = (uint8_t) (((uint8_t) (storage_line[17] - '0') * 10U) +
                         (uint8_t) (storage_line[18] - '0'));
    return (uint8_t) (*second <= 59U);
}

/*
 * 说明：初始化串口屏硬件和 SCI2 通信接口。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_init(void)
{
    fsp_err_t result;

    result = hmi_hardware_init();
    if(result != FSP_SUCCESS)
    {
        return result;
    }

    // 按年、月、日、时、分依次写入默认值，任一步失败立即停止并保留首个错误码
    result = app_hmi_set_query_text(HMI_QUERY_YEAR_CONTROL_ID, g_hmi_query_state.query_time.year);
    if(result == FSP_SUCCESS)
    {
        result = app_hmi_set_query_text(HMI_QUERY_MONTH_CONTROL_ID, g_hmi_query_state.query_time.month);
    }
    if(result == FSP_SUCCESS)
    {
        result = app_hmi_set_query_text(HMI_QUERY_DAY_CONTROL_ID, g_hmi_query_state.query_time.day);
    }
    if(result == FSP_SUCCESS)
    {
        result = app_hmi_set_query_text(HMI_QUERY_HOUR_CONTROL_ID, g_hmi_query_state.query_time.hour);
    }
    if(result == FSP_SUCCESS)
    {
        result = app_hmi_set_query_text(HMI_QUERY_MINUTE_CONTROL_ID, g_hmi_query_state.query_time.minute);
    }
    if(result == FSP_SUCCESS)
    {
        result = hmi_record_clear(HMI_STORAGE_SCREEN_ID, HMI_STORAGE_CONTROL_ID);
    }
    return result;
}

/*
 * 说明：处理串口屏上传的文本控件数据帧，并更新当前查询时间。
 * 输入：无。
 * 输出：无。
 */
void app_hmi_process_input(void)
{
    while(hmi_receive_frame_take(g_hmi_query_state.receive_frame,
                                 sizeof(g_hmi_query_state.receive_frame),
                                 &g_hmi_query_state.receive_length) != 0U)
    {
        // F7 为 RTC 返回帧，其余 EE 帧按文本控件通知处理；不识别的帧由校验函数忽略
        if((g_hmi_query_state.receive_length > 1U) &&
           (g_hmi_query_state.receive_frame[1] == HMI_RTC_RESPONSE_COMMAND))
        {
            app_hmi_update_rtc_time(g_hmi_query_state.receive_frame,
                                    g_hmi_query_state.receive_length);
        }
        else
        {
            app_hmi_update_query_time(g_hmi_query_state.receive_frame,
                                      g_hmi_query_state.receive_length);
        }
    }
}

/*
 * 说明：获取一次查询按钮请求及当前屏幕输入的年月日时分。
 * 输入：query_time：查询时间输出地址。
 * 输出：成功取得查询请求返回 1，没有请求或参数无效返回 0。
 */
uint8_t app_hmi_take_query_request(HMI_Query_Time *query_time)
{
    if(query_time == NULL)
    {
        return 0U;
    }
    if(hmi_query_request_take() == 0U)
    {
        return 0U;
    }

    *query_time = g_hmi_query_state.query_time;
    return 1U;
}

/*
 * 说明：向串口屏请求读取一次系统 RTC 时间。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_request_rtc_time(void)
{
    return hmi_rtc_read_request();
}

/*
 * 说明：取出最近一次串口屏返回的有效 RTC 时间。
 * 输入：rtc_time：RTC 时间输出地址。
 * 输出：成功取得新时间返回 1，没有新时间或参数无效返回 0。
 */
uint8_t app_hmi_take_rtc_time(HMI_RTC_Time *rtc_time)
{
    if((rtc_time == NULL) || (g_hmi_query_state.rtc_time_ready == 0U))
    {
        return 0U;
    }

    *rtc_time = g_hmi_query_state.rtc_time;
    g_hmi_query_state.rtc_time_ready = 0U;
    return 1U;
}

/*
 * 说明：清除画面 0 的数据记录控件 1 中现有的全部查询结果。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_clear_storage_records(void)
{
    return hmi_record_clear(HMI_STORAGE_SCREEN_ID, HMI_STORAGE_CONTROL_ID);
}

/*
 * 说明：从文件数据末尾向前解析记录，并向页面 0 的控件 1 追加不晚于指定秒数的记录。
 * 输入：storage_data：CH378 读取的数据；maximum_second：允许显示的最大秒数；maximum_record_count：最多显示条数；displayed_count：实际显示条数。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_display_storage_records(const uint8_t *storage_data,
                                          uint8_t maximum_second,
                                          uint8_t maximum_record_count,
                                          uint8_t *displayed_count)
{
    const char *data_start;
    const char *line_start;
    const char *line_end;
    size_t line_length;
    uint8_t second;
    int record_length;

    if((storage_data == NULL) || (displayed_count == NULL))
    {
        return FSP_ERR_INVALID_POINTER;
    }
    if((maximum_second > 59U) || (maximum_record_count == 0U))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    *displayed_count = 0U;
    data_start = (const char *) storage_data;
    line_end = data_start + strlen(data_start);

    while((line_end > data_start) && (*displayed_count < maximum_record_count))
    {
        // 从文件末尾向前找行，保证最新记录先进入表格；尾部 CR/LF 不属于记录正文
        while((line_end > data_start) &&
              ((line_end[-1] == '\r') || (line_end[-1] == '\n')))
        {
            line_end--;
        }
        if(line_end == data_start)
        {
            break;
        }

        line_start = line_end;
        while((line_start > data_start) &&
              (line_start[-1] != '\r') && (line_start[-1] != '\n'))
        {
            line_start--;
        }

        line_length = (size_t) (line_end - line_start);
        // 末尾读取可能从一行中间开始，该不完整行会因固定时间格式校验失败而跳过
        if((line_length > 0U) && (line_length < sizeof(g_hmi_query_state.storage_line)))
        {
            memcpy(g_hmi_query_state.storage_line, line_start, line_length);
            g_hmi_query_state.storage_line[line_length] = '\0';

            if((app_hmi_parse_record_second(g_hmi_query_state.storage_line, &second) != 0U) &&
               (second <= maximum_second) &&
               (app_hmi_extract_parameter((const uint8_t *) g_hmi_query_state.storage_line,
                                          "temperature=",
                                          g_hmi_query_state.temperature,
                                          sizeof(g_hmi_query_state.temperature)) != 0U) &&
               (app_hmi_extract_parameter((const uint8_t *) g_hmi_query_state.storage_line,
                                          "humidity=",
                                          g_hmi_query_state.humidity,
                                          sizeof(g_hmi_query_state.humidity)) != 0U) &&
               (app_hmi_extract_parameter((const uint8_t *) g_hmi_query_state.storage_line,
                                          "pressure=",
                                          g_hmi_query_state.pressure,
                                          sizeof(g_hmi_query_state.pressure)) != 0U))
            {
                record_length = snprintf(g_hmi_query_state.table_record,
                                         sizeof(g_hmi_query_state.table_record),
                                         "%.19s;%s;%s;%s;",
                                         g_hmi_query_state.storage_line,
                                         g_hmi_query_state.temperature,
                                         g_hmi_query_state.humidity,
                                         g_hmi_query_state.pressure);
                // 表格正文固定为“时间;温度;湿度;压力;”，分号顺序对应控件的四个子项目
                if((record_length < 0) ||
                   ((size_t) record_length >= sizeof(g_hmi_query_state.table_record)))
                {
                    return FSP_ERR_INVALID_SIZE;
                }

                if(hmi_record_add(HMI_STORAGE_SCREEN_ID,
                                  HMI_STORAGE_CONTROL_ID,
                                  (const uint8_t *) g_hmi_query_state.table_record) != FSP_SUCCESS)
                {
                    return FSP_ERR_INTERNAL;
                }
                (*displayed_count)++;
            }
        }

        line_end = line_start;
    }

    return FSP_SUCCESS;
}
