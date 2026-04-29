#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
#include "asr_init.h"
#include "mic.h"

uint8 mic_flag=0xFF;//麦克风状态标志，非0表示不采样，0表示开始开启采样;

/**
 * @brief ASR 妯″潡鍒濆鍖�
 * 瀹屾垚锛氶害鍏嬮 ADC 鍒濆鍖栥€丗IFO 鍒濆鍖栦互鍙婂畾鏃跺櫒閰嶇疆
 * @return 鏃�
 */
void asr_init(void)
{
    // 鍒濆鍖栭害鍏嬮妯″潡锛堝寘鍚� ADC + FIFO锛�
    mic_init();
    tft180_show_string(1, 1, "WiFi init start"); // TFT180 显示字符
    while(wifi_spi_init(ASR_WIFI_SSID, ASR_WIFI_PASSWORD))                        // 杩炴帴鍒版寚瀹� WiFi 缃戠粶锛屽け璐ュ垯鎸佺画閲嶈瘯
    {
        tft180_clear();
        tft180_show_string(1,1,"WiFi failed"); // TFT180 显示字符
        system_delay_ms(500);                                                     // 鍒濆鍖栧け璐ュ悗绛夊緟 500ms 鍐嶉噸璇�
    }
    tft180_clear();
    tft180_show_string(1,1,"WiFi ready "); // TFT180 显示字符
    mic_flag = 0xFF;

}
