/*
 * 本文件是 CH378 文件命令层，位于 8 位并口硬件层与 ch378_storage 应用接口层之间。
 * 它负责发送芯片命令、等待 INT# 完成状态以及按 CH378 缓冲区请求分块传输数据。
 * 本层使用 FAT 短文件名和绝对路径；所有接口同步阻塞且不可在中断中调用。
 */

// -----------------------------------------------------------------------------
/* 简单说明:
   (1)、本文件属于功能模块层，主要是功能是对芯片发送指令，读取写入数据功能
   (3)、name 参数是指短文件名, 可以包括根目录符, 但不含有路径分隔符, 总长度不超过1+8+1+3+1字节;
   (4)、PathName 参数是指全路径的短文件名, 包括根目录符、多级子目录及路径分隔符、文件名/目录名;
   (5)、LongName 参数是指长文件名, 以UNICODE小端顺序编码, 以两个0字节结束, 使用长文件名子程序 */

// -----------------------------------------------------------------------------
/* 部分宏定义说明:
   (1)、定义 NO_DEFAULT_CH378_INT 用于禁止默认的Wait378Interrupt子程序,禁止后,应用程序必须
        自行定义一个同名子程序;
   (2)、定义 DEF_INT_TIMEOUT 用于设置默认的Wait378Interrupt子程序中的等待中断的超时时间/循环
        计数值, 0则不检查超时而一直等待;
   (3)、定义 EN_DISK_QUERY 用于提供磁盘容量查询和剩余空间查询的子程序,默认是不提供;
   (4)、定义 EN_DIR_CREATE 用于提供新建多级子目录的子程序,默认是不提供;
   (5)、定义 EN_SECTOR_ACCESS 用于提供以扇区为单位读写文件的子程序,默认是不提供;
   (6)、定义 EN_LONG_NAME 用于提供支持长文件名的子程序,默认是不提供;
   (7)、定义 EN_OTHER_FUNCTION 用于提供其它非常用子程序,默认是不提供; */

// -----------------------------------------------------------------------------

#include "ch378_file_device.h"

CH378_File_Device_State g_ch378_file_device_state =
{
    .sector_size = 512U
};

/*
 * 说明：执行 CH378 数据块传输所需的短软件延时。
 * 输入：time：软件延时循环次数。
 * 输出：无。
 */
static void ch378_delay_dev(uint16_t time)
{
    uint16_t i;
    for(i = 0 ; i < time ; i++)
    {
        __NOP();
    }
}


/*
 * 说明：从 CH378 数据端口按小端顺序读取 32 位数据。
 * 输入：无。
 * 输出：返回读取并组合后的 32 位数据。
 */
static uint32_t CH378Read32bitDat( void )
{
    uint32_t  c0 = 0, c1 = 0, c2 = 0, c3 = 0;

    c0 = xReadCH378Data();
    c1 = xReadCH378Data();
    c2 = xReadCH378Data();
    c3 = xReadCH378Data();
    return( c0 | c1 << 8 | c2 << 16 | c3 << 24 );
}

/*
 * 说明：读取 CH378 的 8 位内部变量。
 * 输入：addr：CH378 8 位内部变量地址。
 * 输出：返回指定地址的 8 位变量值。
 */
static uint8_t CH378ReadVar8( uint8_t addr )
{
    uint8_t dat = 0;

    xWriteCH378Cmd( CMD11_READ_VAR8 );
    xWriteCH378Data( addr );
    dat = xReadCH378Data( );

    return( dat );
}

/*
 * 说明：读取 CH378 的 32 位内部变量。
 * 输入：addr：CH378 32 位内部变量地址。
 * 输出：返回指定地址的 32 位变量值。
 */
static uint32_t CH378ReadVar32( uint8_t addr )
{
    xWriteCH378Cmd( CMD14_READ_VAR32 );
    xWriteCH378Data( addr );
    return( CH378Read32bitDat( ) );                              // 读取 32 位内部变量并结束当前命令
}

/*
 * 说明：获取上一条读写命令实际处理的数据长度。
 * 输入：无。
 * 输出：返回上一条命令实际处理的字节数。
 */
static uint32_t CH378GetTrueLen( void )
{
    xWriteCH378Cmd( CMD02_GET_REAL_LEN );
    return(CH378Read32bitDat());
}

/*
 * 说明：读取 CH378 中断状态并清除当前中断请求。
 * 输入：无。
 * 输出：返回当前 CH378 中断状态码。
 */
uint8_t CH378GetIntStatus( void )
{
    uint8_t  s;

    xWriteCH378Cmd( CMD01_GET_STATUS );
    s = xReadCH378Data( );

    return( s );
}

/*
 * 说明：等待 CH378 命令完成中断，等待超时则返回通信错误。
 * 输入：无。
 * 输出：命令完成返回中断状态码，超时返回 ERR_USB_UNKNOWN。
 */
static uint8_t Wait378Interrupt( void )
{
    uint32_t i;

    for( i = 0; i < 50000; i ++ )
    {
        if (g_ch378_hard_state.interrupt_completed)                // 检测 CH378 命令完成中断
        {
            g_ch378_hard_state.interrupt_completed = 0U;
            return( g_ch378_hard_state.command_status);
        }
        ch378_delay_dev( 300 );
    }

    return( ERR_USB_UNKNOWN );                                   // 等待中断超时
}

/*
 * 说明：发送一条无参数命令并等待命令完成中断。
 * 输入：mCmd：CH378 命令码。
 * 输出：返回命令完成状态码或等待超时错误码。
 */
static uint8_t CH378SendCmdWaitInt( uint8_t mCmd )
{
    g_ch378_hard_state.interrupt_completed = 0U;
    g_ch378_hard_state.command_status = 0xFFU;
    xWriteCH378Cmd( mCmd );                                      // 清除旧状态后只等待本条命令产生的中断
    return( Wait378Interrupt( ) );
}

/*
 * 说明：发送一条带单字节参数的命令并等待命令完成中断。
 * 输入：mCmd：CH378 命令码；mDat：命令参数。
 * 输出：返回命令完成状态码或等待超时错误码。
 */
static uint8_t CH378SendCmdDatWaitInt( uint8_t mCmd, uint8_t mDat )
{
    g_ch378_hard_state.interrupt_completed = 0U;
    g_ch378_hard_state.command_status = 0xFFU;
    xWriteCH378Cmd( mCmd );
    xWriteCH378Data( mDat );                                     // 清除旧状态后只等待本条命令产生的中断
    return( Wait378Interrupt( ) );
}

// -------------------------------- 文件操作 --------------------------------
/*
 * 说明：检查当前 U盘或 SD 卡是否已经连接。
 * 输入：无。
 * 输出：连接正常返回 ERR_SUCCESS，否则返回 CH378 状态码。
 */
uint8_t CH378DiskConnect( void )
{
    return( CH378SendCmdWaitInt( CMD0H_DISK_CONNECT ) );
}

/*
 * 说明：读取当前磁盘及文件系统的工作状态。
 * 输入：无。
 * 输出：返回 DEF_DISK_* 磁盘工作状态值。
 */
uint8_t CH378GetDiskStatus( void )
{
    return( CH378ReadVar8( VAR8_DISK_STATUS ) );
}

/*
 * 说明：挂载并初始化当前磁盘文件系统。
 * 输入：无。
 * 输出：挂载成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskReady( void )
{
    return( CH378SendCmdWaitInt( CMD0H_DISK_MOUNT ) );
}

/*
 * 说明：读取 CH378 当前请求缓冲区中的数据块。
 * 输入：buf：接收 CH378 请求数据块的缓冲区。
 * 输出：返回实际读取的字节数。
 */
static uint32_t CH378ReadReqBlock( uint8_t *buf )
{
    uint32_t len;
    uint32_t l;

    xWriteCH378Cmd( CMD00_RD_HOST_REQ_DATA );                   // 写入一个字节命令码
    len = xReadCH378Data( );
    len += ( ( uint16_t )xReadCH378Data( ) << 8 );              // 读取两字节数据长度
    ch378_delay_dev( 1 );                                       // 为高速 MCU 预留数据稳定时间
    l = len;
    if( len )
    {
        do
        {
            *buf = xReadCH378Data( );
            buf++;
        }while( --l );
    }
    return( len );
}

/*
 * 说明：从 CH378 当前内部缓冲区读取指定长度的数据。
 * 输入：buf：接收数据缓冲区；len：计划读取的字节数。
 * 输出：返回实际读取的字节数。
 */
static uint32_t CH378ReadBlock( uint8_t *buf, uint32_t len )
{
    uint32_t l;

    xWriteCH378Cmd( CMD20_RD_HOST_CUR_DATA );                    // 写入一个字节命令码
    xWriteCH378Data( ( uint8_t )len );
    xWriteCH378Data( (uint8_t)(len >> 8) );                      // 写入两字节数据长度
    ch378_delay_dev( 1 );                                       // 为高速 MCU 预留数据稳定时间
    if( len )
    {
        l = len;
        do
        {
            *buf = xReadCH378Data( );
            buf++;
        }while( --l );
    }
    return( len );
}

/*
 * 说明：向 CH378 内部缓冲区的指定偏移位置写入数据。
 * 输入：buf：待写入数据；offset：CH378 内部缓冲区偏移；len：计划写入的字节数。
 * 输出：返回实际写入的字节数。
 */
static uint16_t CH378WriteOfsBlock( uint8_t *buf, uint16_t offset, uint16_t len )
{
    uint16_t l;

    xWriteCH378Cmd( CMD40_WR_HOST_OFS_DATA );                    // 写入一个字节命令码
    xWriteCH378Data( ( uint8_t )offset );                        // 写入两字节缓冲区偏移地址
    xWriteCH378Data( ( uint8_t )(offset >> 8) );
    xWriteCH378Data( ( uint8_t )len );                           // 写入两字节数据长度
    xWriteCH378Data( ( uint8_t )(len >> 8) );
    ch378_delay_dev( 10 );                                      // 为高速 MCU 预留数据稳定时间
    if( len )
    {
        l = len;
        do
        {
            xWriteCH378Data( *buf );
            buf++;
        }while( --l );
    }
    return( len );
}



/*
 * 说明：设置下一条文件命令使用的完整文件名或路径。
 * 输入：PathName：以零结尾的 FAT 短文件名或绝对路径。
 * 输出：无。
 */
static void CH378SetFileName( uint8_t *PathName )
{
    uint8_t  i, c;

    if( PathName == NULL )                                       // 空指针表示沿用当前文件名
    {
        return;
    }
    xWriteCH378Cmd( CMD10_SET_FILE_NAME );
    for( i = MAX_FILE_NAME_LEN; i != 0; --i )
    {
        c = *PathName;
        xWriteCH378Data(c);
        if( c == 0 )
        {
            break;
        }
        PathName ++;
    }
}

/*
 * 说明：获取当前已打开文件的长度。
 * 输入：无。
 * 输出：返回当前文件长度，单位：字节。
 */
uint32_t CH378GetFileSize( void )
{
    return( CH378ReadVar32( VAR32_FILE_SIZE ) );
}

/*
 * 说明：打开指定文件或目录。
 * 输入：PathName：以零结尾的 FAT 短文件名绝对路径。
 * 输出：打开成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileOpen( uint8_t *PathName )
{
    CH378SetFileName( PathName );                                // 设置下一条命令操作的文件名
    return( CH378SendCmdWaitInt( CMD0H_FILE_OPEN ) );
}

/*
 * 说明：创建并打开指定文件，同名文件存在时会重新创建。
 * 输入：PathName：以零结尾的待创建文件名或绝对路径。
 * 输出：创建成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileCreate( uint8_t *PathName )
{
    CH378SetFileName( PathName );                                // 设置下一条命令操作的文件名
    return( CH378SendCmdWaitInt( CMD0H_FILE_CREATE ) );
}

/*
 * 说明：逐级创建或打开指定目录。
 * 输入：PathName：以零结尾的待创建目录名或绝对路径。
 * 输出：创建成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileDirCreate( uint8_t *PathName )
{
    CH378SetFileName( PathName );                                // 设置下一条命令操作的目录名
    return( CH378SendCmdWaitInt( CMD0H_DIR_CREATE ) );
}

/*
 * 说明：删除指定文件。
 * 输入：PathName：以零结尾的待删除文件名绝对路径。
 * 输出：删除成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileErase( uint8_t *PathName )
{
    CH378SetFileName( PathName );
    return( CH378SendCmdWaitInt( CMD0H_FILE_ERASE ) );
}

/*
 * 说明：关闭当前文件，并按参数决定是否更新文件长度及 FAT 信息。
 * 输入：UpdateSz：0 不更新文件长度，1 更新文件长度。
 * 输出：关闭成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileClose( uint8_t UpdateSz )
{
    return( CH378SendCmdDatWaitInt( CMD1H_FILE_CLOSE, UpdateSz ) );
}

/*
 * 说明：将当前文件指针移动到指定字节偏移。
 * 输入：offset：相对于文件起始位置的绝对字节偏移。
 * 输出：定位成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteLocate( uint32_t offset )
{
    g_ch378_hard_state.interrupt_completed = 0U;
    g_ch378_hard_state.command_status = 0xFFU;
    xWriteCH378Cmd( CMD4H_BYTE_LOCATE );
    xWriteCH378Data( (uint8_t)offset );
    xWriteCH378Data( (uint8_t)( (uint16_t)offset >> 8 ) );
    xWriteCH378Data( (uint8_t)( offset >> 16 ) );
    xWriteCH378Data( (uint8_t)( offset >> 24 ) );
    return( Wait378Interrupt( ) );
}

/*
 * 说明：从当前文件位置读取指定长度的数据。
 * 输入：buf：接收缓冲区；ReqCount：请求读取字节数；RealCount：实际读取字节数。
 * 输出：读取成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteRead( uint8_t *buf, uint16_t ReqCount , uint32_t *RealCount )
{
    uint8_t  s;
    uint32_t len;

    g_ch378_hard_state.interrupt_completed = 0U;
    g_ch378_hard_state.command_status = 0xFFU;
    xWriteCH378Cmd( CMD2H_BYTE_READ );                           // 发送字节读取命令
    xWriteCH378Data( (uint8_t)ReqCount );
    xWriteCH378Data( (uint8_t)( ReqCount >> 8 ) );               // 写入两字节请求长度
    if( RealCount )
    {
        *RealCount = 0;
    }
    s = Wait378Interrupt( );
    if( s == ERR_SUCCESS )
    {
        len = CH378GetTrueLen(  );                               // 获取当前命令实际返回长度
        if( RealCount )
        {
            *RealCount = len;
        }
        if( len )                                                // 按实际可读取长度取出数据
        {
            CH378ReadBlock( buf, len );
        }
    }
    return( s );
}

/*
 * 说明：从当前文件位置写入指定长度的数据。
 * 输入：buf：待写入数据；ReqCount：请求写入字节数；RealCount：实际写入字节数。
 * 输出：写入成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378ByteWrite( uint8_t *buf, uint16_t ReqCount, uint16_t * RealCount )
{
    uint8_t  s;

    CH378WriteOfsBlock( buf, 0x0000 ,ReqCount );                 // 将待写数据预先送入 CH378 内部缓冲区
    g_ch378_hard_state.interrupt_completed = 0U;
    g_ch378_hard_state.command_status = 0xFFU;
    xWriteCH378Cmd( CMD2H_BYTE_WRITE );                          // 发送字节写入命令
    xWriteCH378Data( (uint8_t)ReqCount );
    xWriteCH378Data( (uint8_t)( ReqCount >> 8 ) );
    s = Wait378Interrupt( );
    if( s != ERR_SUCCESS )
    {
        if( RealCount )
        {
            *RealCount = 0;
        }
    }
    else
    {
        if( RealCount )
        {
            *RealCount = ReqCount;
        }
    }
    return( s );
}
/*
 * 说明：查询当前已打开文件的信息。
 * 输入：buf：接收文件目录信息的缓冲区。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378FileQuery( uint8_t *buf )
{
    uint8_t  status;

    status = CH378SendCmdWaitInt( CMD0H_FILE_QUERY );
    if( status != ERR_SUCCESS )
    {
        return( status );
    }
    CH378ReadReqBlock( buf );

    return( status );
}


/*
 * 说明：查询当前磁盘的总容量。
 * 输入：DiskCap：接收磁盘总扇区数的地址。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskCapacity( uint32_t *DiskCap )
{
    uint8_t  s;

    s = CH378SendCmdWaitInt( CMD0H_DISK_CAPACITY );
    if ( s == ERR_SUCCESS )
    {
        xWriteCH378Cmd( CMD00_RD_HOST_REQ_DATA );                // 写入一个字节命令码
        xReadCH378Data( );
        xReadCH378Data( );
        *DiskCap = CH378Read32bitDat( );                         // 读取 32 位磁盘容量并结束命令
    }
    else
    {
        *DiskCap = 0;
    }
    return( s );
}

/*
 * 说明：查询当前磁盘的剩余空间。
 * 输入：DiskFre：接收磁盘可用扇区数的地址。
 * 输出：查询成功返回 ERR_SUCCESS，否则返回 CH378 错误码。
 */
uint8_t CH378DiskQuery( uint32_t *DiskFre )
{
    uint8_t  s;
    uint32_t  c0, c1, c2, c3;

    s = CH378SendCmdWaitInt( CMD0H_DISK_QUERY );
    if( s == ERR_SUCCESS )
    {
        xWriteCH378Cmd( CMD00_RD_HOST_REQ_DATA );                // 写入一个字节命令码
        xReadCH378Data( );
        xReadCH378Data( );                                       // 读取两字节数据长度

        xReadCH378Data( );                                       // 跳过总扇区数字段
        xReadCH378Data( );
        xReadCH378Data( );
        xReadCH378Data( );

        c0 = xReadCH378Data( );                                  // 读取空闲扇区数字段
        c1 = xReadCH378Data( );
        c2 = xReadCH378Data( );
        c3 = xReadCH378Data( );
        *DiskFre = c0 | c1 << 8 | c2 << 16 | c3 << 24;
        xReadCH378Data( );                                       // 读取 FAT 类型字段并结束命令
    }
    else
    {
        *DiskFre = 0;
    }
    return( s );
}

