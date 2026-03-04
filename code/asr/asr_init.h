#ifndef CODE_ASR_INIT_H__
#define CODE_ASR_INIT_H__
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// #define ASR_PIT                CCU60_CH1       // 使用 CCU60 通道1作为麦克风采样定时器
// #define ASR_AUDIO_ADC          ADC0_CH0_A0     // 使用 ADC0 通道0作为麦克风输入
// #define ASR_MIC_SAMPLE_RATE    8000            // 采样率 8kHz
// #define ASR_MIC_FIFO_SIZE      2048            // FIFO缓冲区大小（样本数，16bit每个）

extern void asr_init(void);
extern void mic_init(void);
extern uint16 mic_read(void);
extern uint8 mic_fifo_read(uint16 *sample);
extern uint32 mic_fifo_level(void);

#endif /* CODE_ASR_INIT_H_ */