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

}