#ifndef H_HMI_H_
#define H_HMI_H_

/*
 * 串口屏硬件适配接口：封装电源控制、SCI2 同步发送、完整帧接收和查询按钮事件。
 * UART 与定时器回调由 FSP 配置直接引用，因此函数名称不可随意修改。
 */

#include "hal_data.h"

/*
 * 说明：使能串口屏电源和通信接口，打开 SCI2 并等待屏幕完成启动。
 * 输入：无。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_hardware_init(void);

/*
 * 说明：通过 SCI2 发送一帧串口屏数据，并阻塞等待发送完成。
 * 输入：data：发送数据地址；length：发送数据长度。
 * 输出：成功返回 FSP_SUCCESS，失败返回 FSP 错误码。
 */
fsp_err_t hmi_send_frame(const uint8_t *data, uint16_t length);

/*
 * 说明：取出一帧串口屏上传数据，并在取出成功后释放接收缓冲区。
 * 输入：buffer：接收帧缓冲区；buffer_size：缓冲区大小；frame_length：完整帧长度输出地址。
 * 输出：成功取出返回 1，没有完整帧或参数无效返回 0。
 */
uint8_t hmi_receive_frame_take(uint8_t *buffer, uint16_t buffer_size, uint16_t *frame_length);

/*
 * 说明：读取并清除查询按钮请求标志，保证一次按钮松开只执行一次查询。
 * 输入：无。
 * 输出：存在查询请求返回 1，否则返回 0。
 */
uint8_t hmi_query_request_take(void);

#endif /* H_HMI_H_ */
