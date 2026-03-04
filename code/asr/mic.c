#include "mic.h"
#include "asr_init.h"

// 全局麦克风结构体实例
mic_struct mic_module = {0};

/**
 * @brief 初始化麦克风模块
 * @return 无
 */
void mic_init(void)
{
    // 初始化ADC
    adc_init(ASR_AUDIO_ADC, ADC_12BIT);
    
    // 初始化FIFO，16bit数据宽度
    fifo_init(&mic_module.fifo, 
              FIFO_DATA_16BIT, 
              (void *)mic_module.fifo_buffer, 
              sizeof(mic_module.fifo_buffer));
    
    // 初始化采样状态
    mic_module.enabled = 0;
    mic_module.lost_samples = 0;
}

/**
 * @brief 启动麦克风采样（启动PIT定时器）
 * @return 无
 */
void mic_start(void)
{
    if (mic_module.enabled)
        return;
    
    // 配置PIT定时器，125us周期 = 8kHz采样率
    pit_us_init(ASR_PIT, 125);
    pit_start(ASR_PIT);
    
    mic_module.enabled = 1;
}

/**
 * @brief 停止麦克风采样
 * @return 无
 */
void mic_stop(void)
{
    pit_close(ASR_PIT);
    mic_module.enabled = 0;
 
}

/**
 * @brief 直接阻塞读取一个ADC样本（不使用FIFO）
 * @return 当前ADC采样值（12bit，0-4095）
 */
int16 mic_read(void)
{
    return adc_convert(ASR_AUDIO_ADC);
}

/**
 * @brief 从FIFO中读取一个样本,读出后清除对应FIFO数据
 * @param sample 指向输出样本缓冲的指针
 * @return FIFO状态（FIFO_SUCCESS 或 FIFO_NO_DATA）
 */
fifo_state_enum mic_fifo_read(int16 *sample)
{
    return fifo_read_element(&mic_module.fifo, sample, FIFO_READ_AND_CLEAN);
}

/**
 * @brief 获取FIFO中当前有效样本数
 * @return 样本数量
 */
uint32 mic_fifo_level(void)
{
    return fifo_used(&mic_module.fifo);
}

/**
 * @brief ISR处理函数 - 在PIT定时中断中调用
 * 功能：读取ADC采样值并写入FIFO，到达FIFO容量时更新发送标志mic_module.send_flag = 1
 * @return 无
 */
void mic_sample_isr_handler(void)
{
    int16 sample;
    fifo_state_enum ret;
    
    // 读取ADC采样值
    sample = adc_convert(ASR_AUDIO_ADC);//需要减去安静时adc值的偏移量，得到实际的音频信号值
    
    // 写入FIFO
    if(mic_module.audio_count >= ASR_SEND_DATA_MAX_LENTH)
    {
        mic_module.send_flag = 1;//达到发送条件，置发送标志
        mic_module.audio_count = 0;//重置计数
    }
    else
    {
        mic_module.audio_count++;//增加计数
        ret = fifo_write_element(&mic_module.fifo, sample);
    
        // FIFO满时计数丢失样本
        if (ret == FIFO_SPACE_NO_ENOUGH)
        {
            mic_module.lost_samples++;
        }
    }
    
}
