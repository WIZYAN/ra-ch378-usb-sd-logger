#ifndef CH378_STORAGE_H_
#define CH378_STORAGE_H_

/*
 * CH378 存储公共接口。
 * 应用工程只需使用 config、write、read 三个接口，不应直接依赖底层 CH378 文件命令。
 */

#include <stdint.h>

#define CH378_STORAGE_USB    0U
#define CH378_STORAGE_SD     1U
#define CH378_STORAGE_SUCCESS 0U

/*
 * 说明：选择存储介质并初始化 CH378；默认选择 U盘，介质挂载由读写接口自动完成。
 * 输入：device：0 选择 U盘，1 选择 SD 卡。
 * 输出：成功返回 CH378_STORAGE_SUCCESS，失败返回错误码。
 */
uint8_t ch378_storage_config(uint8_t device);

/*
 * 说明：按年月日时分生成文件路径，并向目标文件追加一条带秒的数据记录。
 * 输入：year：年；month：月；day：日；hour：时；minute：分；second：秒；data：待写入字符串。
 * 输出：成功返回 CH378_STORAGE_SUCCESS，失败返回错误码。
 */
uint8_t ch378_storage_write(uint16_t year,
                            uint8_t month,
                            uint8_t day,
                            uint8_t hour,
                            uint8_t minute,
                            uint8_t second,
                            const char *data);

/*
 * 说明：按年月日时分定位目标文件，并读取文件末尾能够放入缓冲区的数据。
 * 输入：year：年；month：月；day：日；hour：时；minute：分；buffer：接收缓冲区；buffer_size：缓冲区大小；read_length：实际读取长度。
 * 输出：成功返回 CH378_STORAGE_SUCCESS，失败返回错误码。
 */
uint8_t ch378_storage_read(uint16_t year,
                           uint8_t month,
                           uint8_t day,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t *buffer,
                           uint16_t buffer_size,
                           uint16_t *read_length);

#endif /* CH378_STORAGE_H_ */
