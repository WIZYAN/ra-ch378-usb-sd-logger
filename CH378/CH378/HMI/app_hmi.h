#ifndef APP_HMI_H_
#define APP_HMI_H_

/*
 * 串口屏业务接口：管理查询条件、RTC 时间以及数据记录控件显示。
 * 业务层通过本头文件访问串口屏，不需要直接组装大彩串口协议帧。
 */

#include "F_HMI.h"

typedef struct
{
    uint16_t year; // 查询年份
    uint8_t month; // 查询月份
    uint8_t day; // 查询日期
    uint8_t hour; // 查询小时
    uint8_t minute; // 查询分钟
} HMI_Query_Time; // 串口屏输入的年月日时分查询条件

typedef struct
{
    uint16_t year; // RTC 年份
    uint8_t month; // RTC 月份
    uint8_t day; // RTC 日期
    uint8_t hour; // RTC 小时
    uint8_t minute; // RTC 分钟
    uint8_t second; // RTC 秒
} HMI_RTC_Time; // 串口屏系统 RTC 的年月日时分秒

typedef enum
{
    HMI_QUERY_STATUS_WAITING = 0, // 等待用户发起查询
    HMI_QUERY_STATUS_QUERYING, // 正在执行存储查询
    HMI_QUERY_STATUS_SUCCESS, // 查询和显示成功
    HMI_QUERY_STATUS_NO_DATA, // 回溯范围内没有有效记录
    HMI_QUERY_STATUS_READ_ERROR, // 存储文件读取失败
    HMI_QUERY_STATUS_DISPLAY_ERROR // 串口屏表格或状态显示失败
} HMI_Query_Status; // 串口屏查询状态文本类型

/*
 * 说明：初始化串口屏硬件、查询输入控件和数据显示控件。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_init(void);

/*
 * 说明：处理串口屏上传的文本控件数据帧，并更新当前查询时间。
 * 输入：无。
 * 输出：无。
 */
void app_hmi_process_input(void);

/*
 * 说明：获取一次查询按钮请求及当前屏幕输入的年月日时分。
 * 输入：query_time：查询时间输出地址。
 * 输出：成功取得查询请求返回 1，没有请求或参数无效返回 0。
 */
uint8_t app_hmi_take_query_request(HMI_Query_Time *query_time);

/*
 * 说明：向串口屏请求读取一次系统 RTC 时间。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_request_rtc_time(void);

/*
 * 说明：取出最近一次串口屏返回的有效 RTC 时间。
 * 输入：rtc_time：RTC 时间输出地址。
 * 输出：成功取得新时间返回 1，没有新时间或参数无效返回 0。
 */
uint8_t app_hmi_take_rtc_time(HMI_RTC_Time *rtc_time);

/*
 * 说明：清除画面 0 的数据记录控件 1 中现有的全部查询结果。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_clear_storage_records(void);

/*
 * 说明：从文件数据末尾向前解析记录，并向表格追加不晚于指定秒数的记录。
 * 输入：storage_data：文件数据；maximum_second：最大秒数；maximum_record_count：最大记录数；displayed_count：实际显示条数。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t app_hmi_display_storage_records(const uint8_t *storage_data,
                                          uint8_t maximum_second,
                                          uint8_t maximum_record_count,
                                          uint8_t *displayed_count);

#endif /* APP_HMI_H_ */
