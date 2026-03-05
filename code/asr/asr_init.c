#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
#include "asr_init.h"
#include "mic.h"

/**
 * @brief ASR模块初始化
 * 完成: ADC初始化、FIFO初始化、定时器配置
 * @return 无
 */
void asr_init(void)
{
    // 初始化麦克风模块（含ADC + FIFO）
    mic_init();
    pit_disable(ASR_PIT);                                                         // 连接wifi前先关闭定时器
    while(wifi_uart_init(ASR_WIFI_SSID, ASR_WIFI_PASSWORD, WIFI_UART_STATION))    // 连接 WiFi 模块到指定的 WiFi 网络
    {
//        printf("wifi连接失败，开始重连...\r\n");
        system_delay_ms(500);                                                     // 初始化失败，等待 500ms
    }
    //printf("WiFi连接成功\r\n");
    pit_enable(ASR_PIT);                                                          // 连接wifi后打开定时器

}