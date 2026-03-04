#ifndef CODE_ASR_MIC_H__
#define CODE_ASR_MIC_H__

#include "zf_common_typedef.h"
#include "zf_common_fifo.h"
//音频采样率8kHz 采样位宽16bit 每次采样2字节
#define ASR_PIT                CCU60_CH1       // 使用 CCU60 通道1作为麦克风采样定时器
#define ASR_AUDIO_ADC          ADC0_CH0_A0     // 使用 ADC0 通道0作为麦克风输入
#define ASR_MIC_SAMPLE_RATE    8000            // 采样率 8kHz
#define ASR_MIC_FIFO_SIZE      2048            // FIFO缓冲区大小（样本数，16bit每个）

// 麦克风采样数据结构
typedef struct
{
    fifo_struct         fifo;                   // FIFO结构体
    uint16              fifo_buffer[2048];      // FIFO缓冲区（16bit样本）
    uint16              lost_samples;           // 丢失样本计数
    uint8               enabled;                // 采样使能标志0-未启用 1-已启用
}mic_struct;

extern mic_struct mic_module;// 全局麦克风结构体实例

// 公开接口函数
void mic_init(void);                                                // 初始化麦克风模块
void mic_start(void);                                               // 启动采样
void mic_stop(void);                                                // 停止采样
uint16 mic_read(void);                                              // 直接读取一个样本（阻塞式ADC）
fifo_state_enum mic_fifo_read(uint16 *sample);                      // 从FIFO读取一个样本
uint32 mic_fifo_level(void);                                        // 获取FIFO中的样本数
void mic_sample_isr_handler(void);                                  // ISR中断处理器（PIT触发）

#endif /* CODE_ASR_MIC_H_ */

// 使用时，先初始化麦克风模块 mic_init()，然后调用 mic_start() 启动采样。采样数据会自动存入 FIFO 中，
// 可以通过 mic_fifo_read() 从 FIFO 中读取样本，或者直接调用 mic_read() 进行阻塞式读取。
// 需要注意的是，mic_read() 不使用 FIFO，因此可能会丢失数据，而 mic_fifo_read() 则会从 FIFO 中读取数据并更新丢失样本计数。
// 结束时调用 mic_stop() 停止采样。一次用完后fifo_clear(&mic_module.fifo);停止采样时清空FIFO，避免下次使用时数据混乱。
