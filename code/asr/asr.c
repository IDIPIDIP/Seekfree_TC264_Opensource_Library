#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// 全局变量定义
uint32      fifo_data_count;                                            // fifo一次接收数据量
int16       fifo_get_data[ASR_SEND_DATA_MAX_LENTH];                     // fifo获取缓冲区
char        json_data[ASR_SEND_DATA_MAX_LENTH * 3];                     // json发送缓冲区
char        temp_data[ASR_SEND_DATA_MAX_LENTH * 3 + 2000];              // 临时数据缓冲区
uint8       websocket_receive_buffer[4096];                             // 接收websocket数据
char        chinese_arry[500];                                          // 存储识别出的中文字符

// 语音数据发送
void audio_data_send(uint8 status)
{
    if(status != 0)// 语音数据中间帧或结束帧，包含音频数据
    {
        fifo_data_count = fifo_used(&mic_module.fifo_buffer);
        if(fifo_data_count > ASR_SEND_DATA_MAX_LENTH)
        {
            fifo_data_count = ASR_SEND_DATA_MAX_LENTH;
        }
        memset(fifo_get_data, 0, sizeof(fifo_get_data));
        fifo_read_buffer(&mic_module.fifo_buffer, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);
        memset(temp_data, 0, sizeof(temp_data));
        base64_encode((uint8*)fifo_get_data, temp_data, fifo_data_count * 2);
        memset(json_data, 0, sizeof(json_data));
    }
    else// 语音数据第一帧，包含鉴权参数和第一段音频数据
    {
        memset(temp_data, 0, sizeof(temp_data));
    }
    snprintf(json_data, (ASR_SEND_DATA_MAX_LENTH * 3),
             "{  \"common\":                                                                                                            \
                        {\"app_id\": \"%s\"},                                                                                           \
                    \"business\":                                                                                                       \
                        {\"domain\": \"iat\", \"language\": \"zh_cn\", \"accent\": \"mandarin\", \"vinfo\": 1, \"vad_eos\": 10000},     \
                    \"data\":                                                                                                           \
                        {\"status\": %d, \"format\": \"audio/L16;rate=8000\", \"audio\": \"%s\", \"encoding\": \"raw\"}}",              \
             ASR_APIID, status, temp_data);
}

// 解析语音识别字段
void extract_content_fields(const char* input)
{
    const char* ptr = input;
    char w_value[256];
    while(*ptr)
    {
        // 查找 "w":" 模式
        const char* w_start = strstr(ptr, "\"w\":\"");
        if(!w_start)
        {
            break;
        }
        w_start += 5; // 跳过 "\"w\":\""
        const char* w_end = strchr(w_start, '"');
        if(!w_end)
        {
            break;
        }
        // 提取 w 字段的值
        int length = w_end - w_start;
        strncpy(w_value, w_start, length);
        w_value[length] = '\0';
        // 打印 w 字段的值
        printf("%s\n", w_value);
        //自己的代码，合并字符串
        strncat(chinese_arry , w_value , length);
        ptr = w_end + 1;
    }
}

void asr()
{
    asr_init(); // 初始化ASR模块，连接WiFi
    //printf("按ASR_BUTTON_PIN开始录音，再按ASR_BUTTON_PIN结束录音\r\n");
    key_clear_all_state(); // 清除按键状态，准备监听按键事件
    if(key_get_state(ASR_BUTTON_PIN) == KEY_SHORT_PRESS) // 等待ASR_BUTTON_PIN短按事件，开始录音
    {
        mic_start(); // 启动麦克风采样
        while(1)
        {
            if(key_get_state(ASR_BUTTON_PIN) == KEY_SHORT_PRESS) // 再次ASR_BUTTON_PIN短按事件，结束录音
            {
                mic_stop(); // 停止麦克风采样
                break;

            }
            if(mic_module.send_flag) // 当麦克风模块设置发送标志时，读取FIFO数据并发送
            {          
            while(!websocket_client_connect(asr_url_out))
                {
                    //printf("服务器连接失败\n");
                    system_delay_ms(500); // 初始化失败，等待 500ms
                }
                
            //printf("服务器连接成功，开始识别语音，最长识别时长为60秒，可手动按键停止\r\n");
            // 语音数据第一帧数据包编码
            audio_data_send(0);
            // 发送第一帧数据包
            websocket_client_send((uint8_t*)json_data, strlen(json_data));
            while(fifo_used(&mic_module.fifo_buffer) != 0)// 等待FIFO中数据被发送完毕
            {
            // 语音数据中间帧数据包编码
            audio_data_send(1);
            // 发送中间帧数据包
            websocket_client_send((uint8_t*)json_data, strlen(json_data));
            // 等待语音结果
             system_delay_ms(300);
            // 接收语音结果
            websocket_client_receive(websocket_receive_buffer);
            // 解析语音结果
            extract_content_fields((const char*)websocket_receive_buffer); 
            mic_module.send_flag = 0; // 发送完成后清除发送标志
            }   
            // 语音数据结束帧数据包
            audio_data_send(2);
            // 发送结束帧数据包
            websocket_client_send((uint8_t*)json_data, strlen(json_data));
            // 等待语音结果
            system_delay_ms(1000);
            // 接收语音结果 
            websocket_client_receive(websocket_receive_buffer);
            // 解析语音结果
            extract_content_fields((const char*)websocket_receive_buffer);

                
            }
            
        }
    }
    
}