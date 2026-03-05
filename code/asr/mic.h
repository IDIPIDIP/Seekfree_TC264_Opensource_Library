#ifndef CODE_ASR_MIC_H__
#define CODE_ASR_MIC_H__

#include "zf_common_typedef.h"
#include "zf_common_fifo.h"
//音频采样率8kHz 采样位宽16bit 每次采样2字节
#define ASR_PIT                CCU60_CH1       // 使用 CCU60 通道1作为麦克风采样定时器
#define ASR_AUDIO_ADC          ADC0_CH0_A0     // 使用 ADC0 通道0作为麦克风输入
#define ASR_MIC_SAMPLE_RATE    8000            // 采样率 8kHz
#define ASR_MIC_FIFO_SIZE      3500            // FIFO缓冲区大小（样本数，16bit每个）
#define ASR_SEND_DATA_MAX_LENTH     2500            // 每次发送的样本数，16bit每个
#define ASR_BUTTON_PIN         P20_6           // 按一下开始录音，再按一下结束录音KEY1

// 麦克风采样数据结构
typedef struct
{
    fifo_struct         fifo;                   // FIFO结构体
    int16               fifo_buffer[3500];      // FIFO缓冲区（16bit样本）
    uint16              lost_samples;           // 丢失样本计数
    uint8               enabled;                // 采样使能标志0-未启用 1-已启用
    int                 audio_count;            //音频计数，到ASR_SEND_DATA_MAX_LENTH，send_flag置1
    int                 send_flag;              // 发送标志，0不发送，1发送
    int                 web_flag;               // websocket连接标志，0未连接，1已连接

}mic_struct;

extern mic_struct mic_module;// 全局麦克风结构体实例

// 公开接口函数
void mic_init(void);                                               // 初始化麦克风模块
void mic_start(void);                                              // 启动采样
void mic_stop(void);                                               // 停止采样
int16 mic_read(void);                                              // 直接读取一个样本（阻塞式ADC），不要用
fifo_state_enum mic_fifo_read(int16 *sample);                      // 从FIFO读取一个样本
uint32 mic_fifo_level(void);                                       // 获取FIFO中的样本数
void mic_sample_isr_handler(void);                                 // ISR中断处理器（PIT触发）
//mic_sample_isr_handler（）需要减去adc值的偏移量，得到实际的音频信号值

#endif /* CODE_ASR_MIC_H_ */

// 使用时，先初始化麦克风模块 mic_init()，然后调用 mic_start() 启动采样。采样数据会自动存入 FIFO 中，
// 可以通过 mic_fifo_read() 从 FIFO 中读取样本，或者直接调用 mic_read() 进行阻塞式读取。
// 需要注意的是，mic_read() 不使用 FIFO，因此可能会丢失数据，而 mic_fifo_read() 则会从 FIFO 中读取数据并更新丢失样本计数。
// 调用mic_fifo_read()
// 结束时调用 mic_stop() 停止采样。一次用完后不要fifo_clear(&mic_module.fifo)，读取完数据会清空fifo，直接用mic_fifo_read()读取数据就行


/*
开机mic_init，按键后mic_start()，pit中断调用mic_sample_isr_handler采样并写入FIFO，
当FIFO中样本数达到ASR_SEND_DATA_MAX_LENTH时，设置发送标志send_flag=1
主循环中连接服务器后web_flag=1且send_flag=1调用mic_fifo_read()读取数据并发送，
最后按键调用mic_stop()停止采样
*/

/*
**入口与调度**
- 系统上电后在 cpu0_main.c 调用 audio_init 完成语音模块初始化。
- 进入语音模式后，在 mode4.c 的 Speech_recognition 循环里持续调用 mode4.c。
- 8kHz 采样中断由 CCU60_CH0 触发，在 isr.c 中断里调用 isr.c。

**1) 初始化阶段（网络与鉴权准备）**
- 语音参数和讯飞地址在 asr_ctrl.h：目标域名、ASR_DOMAIN、APIID/APIKey/APISecret、麦克风 ADC 通道、按键引脚。
- audio_init 里先取服务器时间戳 asr_audio.c，再生成带鉴权参数的 websocket URL asr_audio.c。
- URL 鉴权核心：HMAC-SHA256 在 asr_audio.c，再 base64 + URL encode，最终拼成 authorization/date/host 查询参数 asr_audio.c。
- 初始化 8kHz PIT 和音频 FIFO 在 asr_audio.c。

**2) 硅麦采样与缓存（中断里做）**
- 在 asr_audio.c 里检测按键开始录音，设置状态位。
- 每次中断读取硅麦 ADC 并做直流偏置扣除：(adc - 1870) asr_audio.c。
- 连接成功后把 16bit 采样点写入 FIFO asr_audio.c。
- 每累计一段样本触发发送标志 audio_send_data_flag asr_audio.c；按键再次按下或到 60 秒上限则进入结束流程 asr_audio.c。

**3) 与讯飞建立 WebSocket**
- audio_loop 首先连接服务器 asr_audio.c。
- 连接底层在 websocket_client.c：解析 URL、发握手请求 websocket_client.c, 收握手响应 websocket_client.c, 校验 Sec-WebSocket-Accept websocket_client.c。

**4) 语音分帧与 JSON 组包**
- 组包函数 asr_audio.c。
- 中间/结束帧会从 FIFO 取样本 asr_audio.c，把 16bit PCM 做 base64 asr_audio.c。
- 发送 JSON 使用讯飞 iat 参数：domain=iat、language=zh_cn、accent=mandarin、vad_eos=10000 asr_audio.c，音频格式 audio/L16;rate=8000、encoding=raw、status 字段 asr_audio.c。
- 状态值语义：0 首帧（通常不带音频）、1 中间帧（持续推流）、2 结束帧。

**5) 发送到讯飞**
- 首帧发送在 asr_audio.c。
- 中间帧循环发送在 asr_audio.c。
- 结束帧发送在 asr_audio.c。
- 发送前会把 JSON 封装成 WebSocket 文本帧（客户端掩码）websocket_client.c, 帧构造在 websocket_client.c。

**6) 接收讯飞返回并提取文本**
- 中间阶段收一次结果 asr_audio.c，结束阶段再收一次 asr_audio.c。
- 接收接口是 websocket_client.c，底层读串口缓冲 wifi_uart_read_buffer。
- 文本提取在 asr_audio.c：扫描返回 JSON 中所有 "w":"..." 字段，把词片段拼到 chinese_arry asr_audio.c。

**7) 结果后处理（从识别文本到控制指令）**
- 识别文本缓存在 mode4.c 的 chinese_arry。
- 按键触发 Chinese_str 解析 mode4.c：按中文关键词和数字映射成 instruction 序列，计数器 instruction_cnt 在 mode4.c。
- 解析后清空 chinese_arry，等待下一轮 mode4.c。

*/