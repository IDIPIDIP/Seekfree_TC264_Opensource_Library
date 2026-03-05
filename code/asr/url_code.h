#ifndef CODE_URL_CODE_H_
#define CODE_URL_CODE_H_

#include <stdint.h>
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define ASR_APIID               "60ad5abf"                              // 讯飞的id
#define ASR_APISecret           "MDYzNGJlN2Q4NDE1NGM2YTA3NjE1MGRm"      // 讯飞的Secret
#define ASR_APIKey              "82755986e3e5fbb8c855dac8f4dd49b8"      // 讯飞的Key
#define ASR_WIFI_SSID           "ATCG"                                  // wifi名称 wifi需要是2.4G频率
#define ASR_WIFI_PASSWORD       "QWERTYUI"                              // wifi密码
#define ASR_TARGET_IP           "ws-api.xfyun.cn"
#define ASR_TARGET_PORT         "80"                                    // 讯飞服务器端口，HTTP协议使用80，WebSocket使用443（加密）或80（非加密）
#define ASR_DOMAIN              "wss://ws-api.xfyun.cn/v2/iat"
#define RANDOM_NUM_ADC          ADC0_CH5_A5                             // 使用ADC生成随机数
// global buffers used by ASR module (defined in url_code.c)
extern uint8_t     wifi_uart_get_data_buffer[1024];                    // wifi接收缓冲区
extern char        http_request[1024];                                 // http协议缓冲区
extern char        time_now_data[32];                                  // 当前时间戳
extern char        asr_url_out[512];                                   // websocket_url
extern char        host[64];                                          // 服务器地址 (initialized in .c)
extern char        path[64];                                          // 请求路径 (initialized in .c)


/**
 * 构建讯飞语音听写WebAPI的认证URL
 * 
 * @param host      服务器地址，"iat-api.xfyun.cn"
 * @param path      请求路径，"/v2/iat"
 * @param out_url   输出URL缓冲区
 * @param max_len   缓冲区最大长度，建议至少 1024 字节
 * @return 返回生成的URL长度，0表示失败
 * 
 * 示例：
 *   char url[1024];
 *   uint32_t len = build_asr_auth_url("iat-api.xfyun.cn", "/v2/iat", url, sizeof(url));
 */
uint32_t build_asr_auth_url(const char *host, const char *path, char *out_url, uint32_t max_len);

#endif /* CODE_URL_CODE_H_ */