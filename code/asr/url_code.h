#ifndef CODE_URL_CODE_H_
#define CODE_URL_CODE_H_

#include <stdint.h>
#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define ASR_APIID               "60ad5abf"                              // 讯飞的id
#define ASR_APISecret           "MDYzNGJlN2Q4NDE1NGM2YTA3NjE1MGRm"      // 讯飞的Secret
#define ASR_APIKey              "82755986e3e5fbb8c855dac8f4dd49b8"      // 讯飞的Key
#define ASR_WIFI_SSID           "12345"                                  // wifi名称 wifi需要是2.4G频率
#define ASR_WIFI_PASSWORD       "12345678"                              // wifi密码
#define ASR_TARGET_IP           "iat-api.xfyun.cn"                      // 讯飞听写 WebAPI 主机名（用于 Host 头和鉴权）
#define ASR_TARGET_PORT         "80"                                    // 非加密 WebSocket/HTTP 使用 80
#define ASR_DOMAIN              "/v2/iat"
#define RANDOM_NUM_ADC          ADC0_CH5_A5                             // 使用ADC生成随机数

// global buffers used by ASR module (defined in url_code.c)
extern uint8_t     wifi_spi_get_data_buffer[1024];
extern char        http_request[1024];
extern char        time_now_data[32];
extern char        asr_url_out[512];
extern char        host[64];
extern char        path[64];

/**
 * 构建讯飞语音听写 WebAPI 的认证 URL
 *
 * @param host      服务器地址，例如 "iat-api.xfyun.cn"
 * @param path      请求路径，例如 "/v2/iat"
 * @param out_url   输出 URL 缓冲区
 * @param max_len   缓冲区最大长度，建议至少 1024 字节
 * @return 返回生成的 URL 长度，0 表示失败
 */
uint32_t build_asr_auth_url(const char *host, const char *path, char *out_url, uint32_t max_len);

#endif /* CODE_URL_CODE_H_ */
