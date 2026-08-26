<p align="center" style="font-size: 2em; font-weight: bold; margin: 0.67em 0;">RA6M4 + CH378 存储与串口屏显示工程</p>

# 工程说明

本工程运行在 Renesas RA6M4（R7FA6M4AF3CFB）上，通过 8 位并口控制 CH378，支持 U盘和 SD卡的 FAT 文件读写，并通过 SCI2 将读回的数据发送到串口屏表格控件。

U盘和 SD卡均可完成介质挂载、多级目录创建、文件创建、数据追加写入和文件读取。

应用程序通过一个用户头文件和三个存储接口访问 CH378。业务代码只需要包含：

```c
#include "ch378_storage.h"
```

公开接口只有：

```c
ch378_storage_config();
ch378_storage_write();
ch378_storage_read();
```

## 硬件与开发环境

| 项目 | 配置 |
| --- | --- |
| MCU | Renesas RA6M4 / R7FA6M4AF3CFB |
| CPU | Arm Cortex-M33 |
| FSP | 3.8.0 |
| 编译器 | GNU Arm Embedded GCC 10.3.1 |
| 存储控制器 | CH378L |
| MCU 通信接口 | CH378L 8 位并口 |
| 显示设备 | 大彩串口屏 DC10600PM101 |
| 屏幕工程工具 | VisualTFT |
| 显示通信接口 | SCI2，115200 bit/s，8N1 |
| 文件系统 | FAT12 / FAT16 / FAT32 |
| 系统 | 裸机，无 RTOS |

主要信号：D0～D7=P500～P507，PCS#=P700，RSTI=P701，WR#=P702，RD#=P703，A0=P704，INT#=P709/IRQ10。

串口屏主要信号：RXD2=P301，TXD2=P302，DIS_POWER=P115，DIS_EN=P608。

### CH378L 模块

CH378L 是本工程的存储文件管理控制芯片，负责完成 USB 主机或 SD 卡主机模式下的 FAT 文件操作。模块同时引出了 USB 接口和 MicroSD/TF 卡座，软件通过 `ch378_storage_config(0U)` 或 `ch378_storage_config(1U)` 选择本次使用的存储介质；一次只使用一种存储模式。

| 项目 | 本工程配置与说明 |
| --- | --- |
| 模块供电 | VIN 接入 5 V；板载稳压电路产生 CH378L 所需的 3.3 V 和 1.8 V 电源 |
| MCU 侧电平 | RA6M4 使用 3.3 V I/O 电平 |
| 外部时钟 | CH378L 的 XI、XO 引脚连接 30 MHz 晶振 |
| MCU 接口模式 | 使用 8 位并口模式，模块跳线设置为 J1 断开、J2 短接 |
| 数据总线 | D0～D7，双向 8 位数据总线 |
| 控制信号 | PCS#、RD#、WR# 和 RSTI 均为低电平有效；A0 用于选择命令/状态端口或数据端口 |
| 中断信号 | INT# 为低电平有效的中断请求信号；工程配置为 IRQ10 下降沿触发 |
| USB 接口 | USB D+、D- 连接 USB 插座，USB 5 V 经板载保险器输出 |
| SD 卡接口 | MicroSD/TF 卡座使用 3.3 V 供电，并连接 SD_CS、SD_DO、SD_CK、SD_DI、SD_IN 和 SD_WP 信号 |

模块指示灯含义如下：

| 指示灯 | 说明 |
| --- | --- |
| POWER | 模块 3.3 V 电源指示灯，正常供电时点亮 |
| ACT | 由低电平有效的 ACT# 信号驱动，用于指示当前 USB 设备或 SD 卡的连接、操作状态 |
| RDY | 由低电平有效的 RDY# 信号驱动；所选存储介质初始化成功并可操作后点亮 |

模块还引出了 SPI 和 UART 接口，但本工程未使用这两种接口，RA6M4 只连接并使用模块的 8 位并口 P5。

### 串口屏

显示设备型号为 DC10600PM101，使用 SCI2 与 RA6M4 通信。VisualTFT 工程和 MCU 必须采用相同的串口参数。

| 项目 | 本工程配置 |
| --- | --- |
| 串口通道 | SCI2 / g_uart2 |
| 波特率 | 115200 bit/s |
| 数据格式 | 8 数据位、无校验、1 停止位 |
| MCU 接收 | P301 / RXD2，连接串口屏 TX |
| MCU 发送 | P302 / TXD2，连接串口屏 RX |
| 屏幕电源控制 | P115 / DIS_POWER，高电平使能 |
| 屏幕通信使能 | P608 / DIS_EN，高电平使能 |
| VisualTFT 画面 ID | 0 |
| 表格控件 ID | 1 |
| 表格列数 | 4 列：时间、温度、湿度、压力 |
| 表格显示行数 | 15 行，当前查询最多添加 10 条记录 |
| RTC 控件 | RTC1，控件 ID 8，显示串口屏系统时间 |

MCU 和串口屏必须共地。app_hmi_init() 会使能屏幕电源和通信接口、打开 SCI2，并等待 5 秒，让屏幕完成启动和页面加载。

### 串口屏运行与SD卡升级包

外层 `VisualTFT_SD/` 文件夹保存 DC10600PM101 使用的串口屏运行与SD卡升级包。升级包由 VisualTFT 工程生成，其中 `private/` 是需要复制到串口屏升级SD卡根目录的实际运行目录。

复制完成后的SD卡目录必须为：

```text
SD卡根目录/
└─ private/
   ├─ main.lua
   ├─ bin/
   ├─ truefont/
   ├─ sounds/
   └─ videos/
```

使用时直接复制 `VisualTFT_SD/private/` 整个文件夹，不要只复制其中的文件，也不要在SD卡根目录外再套一层 `VisualTFT_SD/`。将准备好的SD卡插入 DC10600PM101 的SD卡槽，再按照串口屏的SD卡升级方式启动设备。升级过程中应保持供电稳定，不要拔出SD卡或断电。

串口屏升级SD卡与CH378读写数据使用的SD卡属于两个不同接口：前者插在 DC10600PM101 上，用于加载串口屏运行资源；后者插在CH378L模块上，用于保存和查询采集记录。两者不能互相替代。

## 硬件移植注意事项

- D0～D7 当前连接到 Port 5 的低 8 位，底层驱动通过 `BSP_IO_PORT_05` 进行整组读写；更换数据总线端口时，需要同步修改 FSP Pins 配置和 `ch378_hard.c` 中的端口操作。
- PCS#、RSTI、WR#、RD# 和 A0 的引脚名称由 FSP Pins 配置生成；更换控制引脚后需要重新生成工程配置。
- INT# 当前使用 P709/IRQ10，触发方式为下降沿，数字滤波已开启，回调函数为 `USB_INT()`；移植到其他中断通道时需要同步修改 FSP 外部中断配置。
- 应用程序必须在 FSP 引脚初始化完成后调用 `ch378_storage_config()`。
- 更换串口屏通道或引脚时，需要同步修改 FSP UART、Pins 配置以及 H_hmi.c 中使用的 UART 实例。
- VisualTFT 工程的波特率、画面 ID、控件 ID 和表格列数必须与程序配置一致。
- 更换串口屏工程后，需要重新导出 `VisualTFT_SD/private/`，并更新串口屏SD卡中的同名目录。

# 工程文件

本文档中的路径均为相对路径，并从外层 `CH378/` 文件夹开始。

| 内容 | 相对路径 | 说明 |
| --- | --- | --- |
| e² studio 工程 | `CH378/CH378/` | RA6M4 + CH378 源码、FSP 配置及构建目录 |
| README | `CH378/README.md` | 工程使用说明 |
| 官方文档一 | `CH378/CH378DS1.PDF` | CH378 官方数据手册一 |
| 官方文档二 | `CH378/CH378DS2.PDF` | CH378 官方数据手册二 |
| 模块原理图 | `CH378/CH378L_SCH原理图_2112.pdf` | CH378L 模块硬件原理图 |
| 串口屏SD卡升级包 | `CH378/VisualTFT_SD/` | DC10600PM101 的运行资源，复制其中 `private/` 到串口屏SD卡根目录 |

# 目录结构

```text
CH378/
├─ CH378/                       e² studio 工程目录
│  ├─ CH378_USB/
│  │  ├─ ch378_storage.h       用户唯一需要包含的头文件
│  │  ├─ ch378_storage.c       模式配置、自动挂载、读写实现
│  │  ├─ ch378_file_device.h   CH378 文件命令和状态码
│  │  ├─ ch378_file_device.c
│  │  ├─ ch378_hard.h          8 位并口硬件适配
│  │  └─ ch378_hard.c
│  ├─ src/
│  │  └─ hal_entry.c           50条测试记录写入与最近10条查询示例
│  ├─ HMI/                     串口屏初始化、记录解析和表格发送实现
│  ├─ ra_gen/                  FSP 自动生成代码
│  ├─ ra_cfg/                  FSP 配置
│  ├─ script/
│  │  └─ fsp.ld                链接脚本
│  └─ Debug/
│     ├─ CH378.elf
│     ├─ CH378.srec
│     └─ CH378.map
├─ CH378DS1.PDF                 CH378 官方数据手册一
├─ CH378DS2.PDF                 CH378 官方数据手册二
├─ CH378L_SCH原理图_2112.pdf     CH378L 模块硬件原理图
├─ VisualTFT_SD/                DC10600PM101 串口屏SD卡升级包
│  └─ private/                  复制到串口屏SD卡根目录的运行资源
└─ README.md                    工程说明
```

# 三个用户接口

## 快速开始

第一次读写前必须先配置存储模式。下面示例完成 U盘配置、写入和读回：

```c
#include "ch378_storage.h"

uint8_t result;
uint8_t read_buffer[256];
uint16_t read_length;

result = ch378_storage_config(0U); // 0 选择 U盘，1 选择 SD 卡
if(result == CH378_STORAGE_SUCCESS)
{
    result = ch378_storage_write(2026U,
                                 8U,
                                 6U,
                                 16U,
                                 45U,
                                 "temperature=20.22");
}

if(result == CH378_STORAGE_SUCCESS)
{
    result = ch378_storage_read(2026U,
                                8U,
                                6U,
                                16U,
                                45U,
                                read_buffer,
                                sizeof(read_buffer),
                                &read_length);
}
```

每次调用后都应检查返回值。只有返回 `CH378_STORAGE_SUCCESS` 时，本次操作的输出数据才有效。

## 配置存储模式

```c
uint8_t ch378_storage_config(uint8_t device);
```

| device | 存储介质 | CH378 模式 |
| ---: | --- | ---: |
| `0U` | U盘 | `0x07` |
| `1U` | SD卡 | `0x04` |

存储介质选择的默认值为 U盘，但默认值不会执行 CH378 硬件初始化。第一次读写前必须调用一次配置函数：

```c
ch378_storage_config(0U);   // U盘
```

选择 SD卡：

```c
ch378_storage_config(1U);   // SD卡
```

配置函数负责初始化 CH378 和选择主机文件模式，不要求此时介质已经完成挂载。读写函数会在每次操作前自动检查连接状态，并在需要时自动挂载。参数不是 0 或 1 时返回参数错误。

## 写文件

```c
uint8_t ch378_storage_write(uint16_t year,
                            uint8_t month,
                            uint8_t day,
                            uint8_t hour,
                            uint8_t minute,
                            uint8_t second,
                            const char *data);
```

日期时间范围：年 1980～9999，月 1～12，日按月份和闰年校验，时 0～23，分 0～59，秒 0～59。`data` 必须是以 `\0` 结尾的非空字符串，并且在写入函数返回前保持有效。秒只写入记录正文，不参与目录和文件名生成。

函数自动生成：

```text
目录：/年/月/日/小时
文件：分钟.TXT
完整路径：/年/月/日/小时/分钟.TXT
```

示例：

```c
result = ch378_storage_write(
    2026U, 5U, 6U, 16U, 46U, 30U,
    "temperature=20.22;humidity=45.60;pressure=101.30");
```

生成 `/2026/05/06/16/46.TXT`，写入：

```text
2026-05-06 16:46:30;temperature=20.22;humidity=45.60;pressure=101.30\r\n
```

同一分钟内重复调用会定位到文件末尾并追加记录，不会覆盖原数据。

## 读文件

```c
uint8_t ch378_storage_read(uint16_t year,
                           uint8_t month,
                           uint8_t day,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t *buffer,
                           uint16_t buffer_size,
                           uint16_t *read_length);
```

示例：

```c
uint8_t buffer[256];
uint16_t read_length;

result = ch378_storage_read(
    2026U, 8U, 6U, 16U, 45U,
    buffer, sizeof(buffer), &read_length);
```

`buffer` 和 `read_length` 不能为空，`buffer_size` 必须大于或等于 2。函数从文件末尾读取最多 `buffer_size - 1` 字节，在末尾添加 `\0`，并通过 `read_length` 返回实际字节数。文件较大时，缓冲区开头可能是被截断的不完整记录，查询解析会跳过不能完整解析的内容。文件不存在时缓冲区为空、读取长度为 0，并返回文件未找到错误码。

# 串口屏显示

串口屏显示代码位于 HMI/ 文件夹，分为三层：

| 文件 | 作用 |
| --- | --- |
| H_hmi.c/.h | 屏幕电源控制、SCI2 初始化和串口帧发送 |
| F_HMI.c/.h | 生成数据记录控件的添加记录协议帧 |
| app_hmi.c/.h | 解析存储记录并组织四列表格内容 |

## 初始化接口

~~~c
fsp_err_t app_hmi_init(void);
~~~

该函数应在发送任何显示数据之前调用。初始化过程依次使能 DIS_POWER、使能 DIS_EN、打开 SCI2、设置 115200 bit/s，并等待屏幕启动。返回 FSP_SUCCESS 后才能调用显示接口。

## 表格显示接口

~~~c
fsp_err_t app_hmi_display_storage_records(const uint8_t *storage_data,
                                          uint8_t maximum_second,
                                          uint8_t maximum_record_count,
                                          uint8_t *displayed_count);
~~~

`storage_data` 应当是 `ch378_storage_read()` 成功读回并以空字符结尾的字符串。接口从缓冲区末尾向前解析记录，只显示秒数不大于 `maximum_second` 的记录，最多显示 `maximum_record_count` 条，并通过 `displayed_count` 返回本次实际添加的条数。每条记录必须包含 temperature、humidity 和 pressure 三个字段，例如：

~~~text
2026-05-06 16:46:30;temperature=20.22;humidity=45.60;pressure=101.30
~~~

显示接口提取记录时间和三个参数字段，并生成四列表格记录：

~~~text
2026-05-06 16:46:30;20.22;45.60;101.30;
~~~

分号用于分隔表格列，字段依次对应时间、温度、湿度和压力。底层向画面 0、控件 1 发送以下数据记录添加帧：

~~~text
EE B1 52 00 00 00 01 <记录字符串> FF FC FF FF
~~~

表格记录按照显示接口的调用顺序追加。程序不指定固定行号：第一次成功调用添加第一行，后续成功调用依次添加到下一行。可显示行数和滚动方式由 VisualTFT 中的数据记录控件属性决定。

显示接口返回 fsp_err_t，与三个返回 uint8_t 的 CH378 存储接口不是同一类返回码。

| 返回值 | 含义 |
| --- | --- |
| FSP_SUCCESS | 数据已经通过 SCI2 完成发送 |
| FSP_ERR_INVALID_POINTER | 传入的存储数据地址为空 |
| FSP_ERR_INVALID_DATA | 读取内容缺少温度、湿度或压力字段 |
| FSP_ERR_INVALID_SIZE | 表格记录或串口协议帧长度不合法 |
| FSP_ERR_NOT_OPEN | SCI2 尚未打开 |
| FSP_ERR_TIMEOUT | 等待 SCI2 发送完成超时 |

FSP_SUCCESS 表示 MCU 已经完成串口帧发送，不代表串口屏对表格内容进行了应答确认。

## 屏幕查询控件

| 控件 ID | 控件类型 | 用途 |
| ---: | --- | --- |
| 1 | 数据记录控件 | 显示时间、温度、湿度和压力 |
| 2 | 文本输入控件 | 输入年份 |
| 3 | 文本输入控件 | 输入月份 |
| 4 | 文本输入控件 | 输入日期 |
| 5 | 文本输入控件 | 输入小时 |
| 6 | 文本输入控件 | 输入分钟 |
| 7 | 查询按钮 | 松开后发送查询请求 |
| 8 | RTC 控件 RTC1 | 显示并维护串口屏系统年月日时分秒 |

文本输入控件通过大彩标准控件数据通知上传内容。程序按画面 ID 0 和控件 ID 2～6 更新查询时间。查询按钮的“对外指令→弹起时”配置为：

~~~text
A5 5A 03 00 07 01
~~~

按钮的按下指令保持为空，因此一次完整点击只产生一次查询。UART2 回调仅接收协议帧和设置查询标志，CH378 文件读取在主循环中执行。

## RTC连续写入与查询流程

~~~text
初始化串口屏并同步默认查询时间
    -> 配置 CH378 存储模式
    -> 向串口屏请求系统 RTC 时间
    -> 主循环约每 300 ms 重新请求一次 RTC
    -> 完整年月日时分秒发生变化时写入一条记录
    -> 参数从 0 递增到 255 后自动回到 0
    -> 收到 ID7 查询按钮指令时暂停 RTC 请求和写入
    -> 一次性查询截止时间之前最近 10 条记录
    -> 查询结束后重新请求当前 RTC 并恢复连续写入
~~~

默认查询时间为 2026-05-06 16:46，初始化完成后程序会把默认值同步显示到 ID2～ID6。输入该时间表示查询 `2026-05-06 16:46:00` 及其之前的记录，`16:46:01` 至 `16:46:59` 不在本次查询范围内。表格按时间从新到旧显示，最多显示 10 条。

MCU读取的是串口屏系统RTC，不是RA6M4内部RTC。RTC1的控件ID 8用于画面显示；系统RTC读取指令不携带画面ID或控件ID。MCU发送：

~~~text
EE 82 FF FC FF FF
~~~

串口屏返回：

~~~text
EE F7 Year Mon Week Day Hour Min Sec FF FC FF FF
~~~

`Year`至`Sec`均为BCD码。程序将两位年份转换为`2000～2099`，校验年月日时分秒后用于`ch378_storage_write()`。ID 8已经用于RTC1，程序不再向该控件发送查询状态文字；查询状态保存在`g_ch378_demo_result_state.query_status`中供调试器观察。

连续写入记录示例：

~~~text
2026-08-08 16:46:30;temperature=0;humidity=0;pressure=0
2026-08-08 16:46:31;temperature=1;humidity=1;pressure=1
...
2026-08-08 16:50:45;temperature=255;humidity=255;pressure=255
2026-08-08 16:50:46;temperature=0;humidity=0;pressure=0
~~~

查询期间文件写入暂停，查询结束后直接使用重新读取的当前RTC时间继续写入，不补写查询期间遗漏的秒。

# 自动连接和挂载

每次调用读写接口时，程序会自动完成连接检测和介质挂载：

```text
ch378_storage_write/read
    -> 循环执行 CH378DiskConnect()
    -> 连接成功后检查当前挂载状态
    -> 未挂载时执行 CH378DiskReady()
    -> 文件操作
```

U盘和 SD卡使用相同的自动等待机制：连接或挂载尚未完成时，每隔 50ms 重试一次，最多重试 100 次，总等待时间约 5 秒。这样在 `ch378_storage_config()` 后可以直接调用读写接口，不需要在主函数中添加固定延时。介质拔出后，下一次读写会检测到断开；重新插入后，后续读写会自动等待并重新挂载。

# 主函数验证

当前 `hal_entry.c` 默认选择 SD卡。需要改用 U盘时，将 `APP_STORAGE_DEVICE` 修改为 `CH378_STORAGE_USB`。运行后先确认RTC1显示的日期和时间正确，程序会在RTC秒发生变化时持续写入记录。

查询时在ID2～ID6输入目标年月日时分，再点击ID7。查询执行期间允许缺少少量秒级记录，查询完成后程序从串口屏当前RTC时间继续写入。

调试器 Watch 变量：

| 变量 | 含义 |
| --- | --- |
| `g_ch378_demo_rtc_state.current_time` | 最近一次收到的串口屏RTC年月日时分秒 |
| `g_ch378_demo_rtc_state.last_write_time` | 最近一次成功写入使用的完整时间 |
| `g_ch378_demo_rtc_state.parameter_value` | 下一条记录使用的0～255循环参数 |
| `g_ch378_demo_result_state.rtc_request_result` | 最近一次RTC读取指令的发送结果 |
| `g_ch378_demo_result_state.rtc_write_result` | 最近一次RTC记录的CH378写入结果 |
| `g_ch378_demo_result_state.rtc_write_count` | RTC记录累计成功写入次数 |
| `g_ch378_demo_read_state.query_time` | 最近一次按钮查询使用的年月日时分 |
| `g_ch378_demo_result_state.query_read_result` | 最近一次查询的 CH378 读取结果 |
| `g_ch378_demo_read_state.query_read_length` | 最近一次查询的实际读取长度 |
| `g_ch378_demo_read_state.query_read_buffer` | 最近一次查询读回的文件内容 |
| `g_ch378_demo_read_state.displayed_count` | 最近一次查询已经显示的有效记录数量，最大为 10 |
| `g_ch378_demo_read_state.checked_minutes` | 最近一次查询已经检查的分钟文件数量 |
| `g_ch378_demo_result_state.hmi_clear_result` | 最近一次查询的表格清除结果 |
| `g_ch378_demo_result_state.hmi_query_display_result` | 最近一次查询的表格发送结果 |
| `g_ch378_demo_result_state.query_status` | 最近一次查询状态，不再发送到ID8 |

`rtc_write_result`为`0x00`表示最近一条RTC记录写入成功；`rtc_request_result`为`FSP_SUCCESS`表示RTC读取指令已通过SCI2发送。查询过程中遇到文件未找到返回码`0x42`会继续检查更早的分钟文件，不会立即判定查询失败。

# 调用限制与介质操作

- 三个接口均为阻塞式，只能在主循环或任务中调用。介质未连接或尚未挂载时，单次读写调用可能等待约 5 秒。
- 接口共享 CH378 命令状态、挂载状态和当前文件上下文，不可重入，也不能由多个任务同时调用。RTOS 工程必须使用同一个互斥锁保护配置、读取和写入接口。
- 不能在定时器中断、按键中断或其他中断回调中直接调用。中断中只设置请求标志，再由主循环或任务执行存储操作。
- 写入函数返回成功前不能拔出介质。函数完成数据写入、关闭文件并更新文件长度和 FAT 信息后，才返回 `CH378_STORAGE_SUCCESS`。
- USB/SD 模式切换只能在没有读写操作执行时进行。切换模式时重新调用 `ch378_storage_config()`，后续读写会自动连接并挂载新介质。
- 串口屏初始化和显示接口同样为阻塞式接口，应在主循环或任务中调用，不能放在中断回调中。
- 只有存储读取成功且串口屏初始化成功后，才能把读取缓冲区传给表格显示接口。

# 三个接口的返回值

`ch378_storage_config()`、`ch378_storage_write()` 和 `ch378_storage_read()` 都返回一个 `uint8_t` 结果：

- 返回 `CH378_STORAGE_SUCCESS`（`0x00`）表示本次接口调用完整成功；
- 返回非零值表示失败，该值是参数检查、CH378 初始化、介质连接/挂载或文件操作产生的具体错误码；
- 统一接口会保留并向用户返回底层 CH378 错误码，不会只转换成简单的成功或失败。

## 配置函数返回值

`ch378_storage_config(device)` 只负责检查模式参数、初始化 CH378 并设置 USB/SD 主机模式。

| 返回码 | 产生条件 |
| --- | --- |
| `0x00` | CH378 初始化和模式设置成功 |
| `0x03` | `device` 不是 0 或 1 |
| `0xFA` | CH378 通信检查失败、模式设置失败或等待超时 |

配置成功只表示 CH378 模式已经设置完成，不代表 U盘或 SD卡已经挂载；挂载由后续读写接口自动执行。

## 写文件函数返回值

`ch378_storage_write(...)` 的返回值覆盖参数检查、介质连接、挂载、建目录、打开文件、写入和关闭文件整个过程。

| 返回码 | 可能产生的阶段 | 含义 |
| --- | --- | --- |
| `0x00` | 全部阶段 | 记录已经完整写入并成功关闭文件 |
| `0x03` | 参数检查 | 日期时间非法、数据为空或数据过长 |
| `0x16` | 连接检测 | U盘尚未完成连接或枚举，自动重试超时后返回 |
| `0x1F` | 挂载/写入 | CH378 底层磁盘操作失败，或实际写入长度不正确 |
| `0x82` | 连接/挂载 | 介质未连接或操作过程中已拔出 |
| `0x92` | 挂载 | 分区类型不受支持 |
| `0xA1` | 挂载 | 未格式化，或 FAT/BPB 参数不正确 |
| `0xB1` | 写入 | 磁盘剩余空间不足 |
| `0xB2` | 创建文件 | 当前目录没有空闲目录项 |
| `0xB3` | 路径操作 | 路径中的子目录不存在或创建失败 |
| `0xFA` | 任意 CH378 命令 | 等待中断超时或硬件通信异常 |

## 读文件函数返回值

`ch378_storage_read(...)` 的返回值覆盖参数检查、介质连接、挂载、打开文件、定位、读取和关闭文件整个过程。

| 返回码 | 可能产生的阶段 | 含义 |
| --- | --- | --- |
| `0x00` | 全部阶段 | 文件读取和关闭成功，`read_length` 是有效数据长度 |
| `0x03` | 参数检查 | 日期时间非法、缓冲区为空、长度小于 2，或 `read_length` 为空 |
| `0x16` | 连接检测 | U盘尚未完成连接或枚举，自动重试超时后返回 |
| `0x1F` | 挂载/读取 | CH378 底层磁盘操作失败，或返回长度异常 |
| `0x42` | 打开文件 | 年月日时分对应的文件不存在 |
| `0x82` | 连接/挂载 | 介质未连接或操作过程中已拔出 |
| `0x92` | 挂载 | 分区类型不受支持 |
| `0xA1` | 挂载 | 未格式化，或 FAT/BPB 参数不正确 |
| `0xFA` | 任意 CH378 命令 | 等待中断超时或硬件通信异常 |

读取接口在开始时会先将 `buffer[0]` 设为 `\0`、将 `read_length` 设为 0。因此读取失败时应先检查函数返回值；只有返回 `0x00` 后，缓冲区内容和 `read_length` 才应作为有效读取结果使用。

推荐统一这样判断：

```c
uint8_t result = ch378_storage_read(...);

if(result == CH378_STORAGE_SUCCESS)
{
    // 使用 buffer 和 read_length
}
else
{
    // 根据 result 错误码处理失败
}
```
