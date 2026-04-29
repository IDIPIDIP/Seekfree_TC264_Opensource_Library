#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

// 全局变量定义
uint32      fifo_data_count;                                            // fifo一次接收数据量
int16       fifo_get_data[ASR_SEND_DATA_MAX_LENTH];                     // fifo获取缓冲区
char        json_data[ASR_SEND_DATA_MAX_LENTH * 3];                     // json发送缓冲区
char        temp_data[ASR_SEND_DATA_MAX_LENTH * 3 + 2000];              // 临时数据缓冲区
uint8       websocket_receive_buffer[4096];                             // 接收websocket数据
char        chinese_arry[500];                                          // 存储识别出的中文字符
uint8       match_value;                                                // 字库匹配结果

#define ASR_WS_CONNECT_RETRY_MAX  (3)

typedef struct
{
    const char *keyword;
    uint8 return_value;
} asr_dict_item_t;


// keyword      : 关键词（必须使用UTF-8编码）
// return_value : 命中该关键词时返回的字库值（0保留给“未命中”）
static const asr_dict_item_t g_asr_dict_table[] =
{
    {"鸣笛一秒钟", 1},
    {"鸣笛二秒钟", 2},
    {"鸣笛三秒钟", 3},
    {"鸣笛两声", 4},
    {"鸣笛三声", 5},
    {"鸣笛四声", 6},
    {"长短鸣笛", 7},
    {"急促鸣笛", 8},
    {"警报鸣笛", 9},
    {"通过门洞一左", 10},
    {"通过门洞一", 11},
    {"通过门洞二", 12},
    {"通过门洞三", 13},
    {"通过门洞三右", 14},
    {"门洞一右侧返回", 15},
    {"门洞一返回", 16},
    {"门洞二返回", 17},
    {"门洞三返回", 18},
    {"门洞三左侧返回", 19},
    {"前行十米", 20},
    {"后退十米", 21},
    {"蛇形前进十米", 22},
    {"蛇形后退十米", 23},
    {"逆时针转一圈", 24},
    {"顺时针转一圈", 25},
    {"左转", 26},
    {"右转", 27},
    {"打开左转向灯", 28},
    {"打开右转向灯",29},
    {"打开远光灯",30},
    {"打开近光灯",31},
    {"打开雾灯",32},
    {"打开双闪灯",33},
    {"打开车内照明灯",34},
    {"打开雨刷器",35}
};

/**
 * @brief 多字库匹配函数
 * @param text 输入文本
 * @return 匹配到的字库值，0表示未匹配
 */
uint8 asr_match_multi_dictionary(const char *text)
{
    uint32 i;
    uint32 best_len = 0;
    uint8 best_value = 0;

    if(NULL == text)
    {
        return 0;
    }

    for(i = 0; i < (sizeof(g_asr_dict_table) / sizeof(g_asr_dict_table[0])); i++)
    {
        uint32 key_len;

        if((NULL == g_asr_dict_table[i].keyword) || ('\0' == g_asr_dict_table[i].keyword[0]))
        {
            continue;
        }

        if(NULL == strstr(text, g_asr_dict_table[i].keyword))
        {
            continue;
        }

        key_len = strlen(g_asr_dict_table[i].keyword);
        if(key_len > best_len)
        {
            best_len = key_len;
            best_value = g_asr_dict_table[i].return_value;
        }
    }

    return best_value;
}

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
                        {\"status\": %d, \"format\": \"audio/L16;rate=8000\", \"audio\": \"%s\", \"encoding\": \"raw\"}}",
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
        int length = (int)(w_end - w_start);
        strncpy(w_value, w_start, length);
        w_value[length] = '\0';
        // 打印 w 字段的值
        printf("%s\n", w_value);
        //自己的代码，合并字符串
        strncat(chinese_arry , w_value , length);
        ptr = w_end + 1;
    }
}

/**
 * @brief 语音识别主流程函数
 *
 */
uint8 asr()
{
    uint8 match_value;
    uint8 i;
    int get_state = 0;//麦克风采样状态，1表示正在采样，0表示停止采样
    // asr_init(); // 初始化ASR模块，连接WiFi
    
    tft180_clear();
    tft180_show_string(1, 1, "Build auth URL");
    
    
    if(0 == build_asr_auth_url(host, path, asr_url_out, sizeof(asr_url_out))) // URL构建
    {
        tft180_show_string(1, 9, "URL failed");
        return 0;
    }

    // tft180_show_string(1, 9, "Press KEY1");
    memset(chinese_arry, 0, sizeof(chinese_arry));
    key_clear_all_state(); // 清除按键状态，准备监听按键事件
    while(key_get_state(KEY_1) != KEY_SHORT_PRESS)
    {
        tft180_show_string(1, 9, "Press KEY1");
        system_delay_ms(100); // 等待按键事件，避免CPU占用过高
    }
    
    tft180_clear();
    tft180_show_string(1, 1, "Pressed KEY1");
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS) // 等待ASR_BUTTON_PIN短按事件，开始录音
    {
        tft180_clear();
        tft180_show_string(1, 1, "Listening...");
        tft180_show_string(1, 9, "press KEY1 stop");
        key_clear_all_state(); // 清除按键状态，准备下一次监听
        mic_start(); // 启动麦克风采样
        get_state = 1;

        while(1)
        {
            while(mic_module.send_flag==0) // 等待麦克风模块设置发送标志
            {system_delay_ms(100);
            int i=1;
            tft180_show_int(1,17,i,2);
            i++;
            }
            if(mic_module.send_flag) // 当麦克风模块设置发送标志时，读取FIFO数据并发送
            {
                uint8 ws_retry_count = 0;
                while(!websocket_client_connect(asr_url_out))
                {
                    tft180_show_string(1,17,"Retry connect");
                    ws_retry_count++;
                    if(ASR_WS_CONNECT_RETRY_MAX <= ws_retry_count)
                    {
                        tft180_show_string(1,17,"Socket failed");
                        return 0;
                    }
                    // system_delay_ms(500); // 初始化失败，等待 500ms
                }
                tft180_show_string(1,17,"Socket ready");
                // 语音数据第一帧数据包编码
                audio_data_send(0);
                // 发送第一帧数据包
                websocket_client_send((uint8_t*)json_data, strlen(json_data));
                while(fifo_used(&mic_module.fifo_buffer) != 0||get_state == 1)// 等待FIFO中数据被发送完毕
                {
                    if(key_get_state(KEY_1) == KEY_SHORT_PRESS) // 再次ASR_BUTTON_PIN短按事件，结束录音
                    {
                        mic_stop(); // 停止麦克风采样
                        get_state=0;
                    }
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

                match_value = asr_match_multi_dictionary(chinese_arry);

                //printf("asr text:%s\n", chinese_arry);
                //printf("asr dict value:%d\n", match_value);
                return match_value;
            }
        }
    }

    return 0;
}
