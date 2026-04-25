#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
#include "asr_init.h"
#include "mic.h"

/**
 * @brief ASR 妯″潡鍒濆鍖�
 * 瀹屾垚锛氶害鍏嬮 ADC 鍒濆鍖栥€丗IFO 鍒濆鍖栦互鍙婂畾鏃跺櫒閰嶇疆
 * @return 鏃�
 */
void asr_init(void)
{
    // 鍒濆鍖栭害鍏嬮妯″潡锛堝寘鍚� ADC + FIFO锛�
    mic_init();
    pit_disable(ASR_PIT);                                                         // 杩炴帴 WiFi 鍓嶅厛鍏抽棴瀹氭椂鍣�
    while(wifi_uart_init(ASR_WIFI_SSID, ASR_WIFI_PASSWORD, WIFI_UART_STATION))    // 杩炴帴鍒版寚瀹� WiFi 缃戠粶锛屽け璐ュ垯鎸佺画閲嶈瘯
    {
//      printf("WiFi杩炴帴澶辫触锛屽紑濮嬮噸璇�...\r\n");
        system_delay_ms(500);                                                     // 鍒濆鍖栧け璐ュ悗绛夊緟 500ms 鍐嶉噸璇�
    }
    //printf("WiFi杩炴帴鎴愬姛\r\n");
    pit_enable(ASR_PIT);                                                          // WiFi 杩炴帴鎴愬姛鍚庨噸鏂板紑鍚畾鏃跺櫒

}