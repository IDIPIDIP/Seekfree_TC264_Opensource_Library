#ifndef CODE_ASR_H__
#define CODE_ASR_H__
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// #define ASR_PIT                CCU60_CH1       // 使用 CCU60 通道1作为麦克风采样定时器
// #define ASR_AUDIO_ADC          ADC0_CH0_A0     // 使用 ADC0 通道0作为麦克风输入
// #define ASR_MIC_SAMPLE_RATE    8000            // 采样率 8kHz
// #define ASR_MIC_FIFO_SIZE      2048            // FIFO缓冲区大小（样本数，16bit每个）
extern uint32      fifo_data_count;                                            // fifo一次接收数据量
extern int16       fifo_get_data[ASR_SEND_DATA_MAX_LENTH];                     // fifo获取缓冲区
extern char        json_data[ASR_SEND_DATA_MAX_LENTH * 3];                     // json发送缓冲区
extern char        temp_data[ASR_SEND_DATA_MAX_LENTH * 3 + 2000];              // 临时数据缓冲区
extern uint8       websocket_receive_buffer[4096];                             // 接收websocket数据
extern char        chinese_arry[500];                                          // 存储识别出的中文字符


void asr();

#endif /* CODE_ASR_H_ */