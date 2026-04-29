#ifndef CODE_ASR_INIT_H__
#define CODE_ASR_INIT_H__
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// #define ASR_PIT                CCU60_CH1       // 浣跨敤 CCU60 閫氶亾1浣滀负楹﹀厠椋庨噰鏍峰畾鏃跺櫒
// #define ASR_AUDIO_ADC          ADC0_CH0_A0     // 浣跨敤 ADC0 閫氶亾0浣滀负楹﹀厠椋庤緭鍏?
// #define ASR_MIC_SAMPLE_RATE    8000            // 閲囨牱鐜? 8kHz
// #define ASR_MIC_FIFO_SIZE      2048            // FIFO缂撳啿鍖哄ぇ灏忥紙鏍锋湰鏁帮紝16bit姣忎釜锛?

void asr_init(void);
extern uint8 mic_flag; //麦克风状态标志，0xFF表示未初始化，0表示准备就绪，1表示正在采样;
#endif /* CODE_ASR_INIT_H_ */