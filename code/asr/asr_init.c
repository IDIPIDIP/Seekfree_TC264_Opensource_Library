#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
#include "asr_init.h"
#include "mic.h"

/**
 * @brief ASR 模块初始化
 * 完成：麦克风 ADC 初始化、FIFO 初始化以及定时器配置
 * @return 无
 */
void asr_init(void)
{
    // 初始化麦克风模块（包含 ADC + FIFO）
    mic_init();
    pit_disable(ASR_PIT);                                                         // 连接 WiFi 前先关闭定时器
    while(wifi_uart_init(ASR_WIFI_SSID, ASR_WIFI_PASSWORD, WIFI_UART_STATION))    // 连接到指定 WiFi 网络，失败则持续重试
    {
//      printf("WiFi连接失败，开始重试...\r\n");
        system_delay_ms(500);                                                     // 初始化失败后等待 500ms 再重试
    }
    //printf("WiFi连接成功\r\n");
    pit_enable(ASR_PIT);                                                          // WiFi 连接成功后重新开启定时器

}