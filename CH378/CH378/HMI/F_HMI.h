#ifndef F_HMI_H_
#define F_HMI_H_

/*
 * 大彩串口屏协议命令接口：封装数据记录控件、文本控件和系统 RTC 读取命令。
 */

#include "H_hmi.h"

/*
 * 说明：向指定画面的数据记录控件追加一条分号分隔的常规记录。
 * 输入：screen_id：画面 ID；control_id：控件 ID；record：待追加的记录字符串。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_record_add(uint16_t screen_id, uint16_t control_id, const uint8_t *record);

/*
 * 说明：清除指定画面数据记录控件中的全部记录。
 * 输入：screen_id：画面 ID；control_id：控件 ID。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_record_clear(uint16_t screen_id, uint16_t control_id);

/*
 * 说明：设置指定画面文本控件的显示内容。
 * 输入：screen_id：画面 ID；control_id：控件 ID；text：待显示文本。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_text_set(uint16_t screen_id, uint16_t control_id, const uint8_t *text);

/*
 * 说明：向串口屏发送系统 RTC 时间读取指令。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_rtc_read_request(void);

#endif /* F_HMI_H_ */
