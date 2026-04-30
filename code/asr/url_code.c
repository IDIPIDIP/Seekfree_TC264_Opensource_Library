#include "zf_common_headfile.h"
#include "url_code.h"
#include <stdint.h>
#include <time.h>
// this file is code by UTF-8

uint8_t     wifi_spi_get_data_buffer[1024];                            // wifi接收缓冲区
char        http_request[1024];                                       // http协议缓冲区
char        time_now_data[32];                                        // 当前时间戳
char        asr_url_out[512];                                         // websocket_url
char        host[64] = ASR_TARGET_IP;                                 // 服务器地址
char        path[64] = ASR_DOMAIN;                                    // 请求路径

// ==================== SHA256 实现 ====================
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} URL_SHA256_CTX;

static const uint32_t sha256_k[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffu, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR(x, 2) ^ ROR(x, 13) ^ ROR(x, 22))
#define EP1(x) (ROR(x, 6) ^ ROR(x, 11) ^ ROR(x, 25))
#define SIG0(x) (ROR(x, 7) ^ ROR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROR(x, 17) ^ ROR(x, 19) ^ ((x) >> 10))

static void sha256_transform(URL_SHA256_CTX *ctx, const uint8_t *data)
{
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) | ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);

    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + sha256_k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(URL_SHA256_CTX *ctx)
{
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->count = 0;
}

static void sha256_update(URL_SHA256_CTX *ctx, const uint8_t *data, uint32_t len)
{
    uint32_t index = (ctx->count / 8) % 64;
    ctx->count += (len * 8);

    uint32_t partLen = 64 - index;
    uint32_t i = 0;

    if (len >= partLen) {
        memcpy(&ctx->buffer[index], data, partLen);
        sha256_transform(ctx, ctx->buffer);

        for (i = partLen; i + 63 < len; i += 64)
            sha256_transform(ctx, &data[i]);

        index = 0;
    }

    memcpy(&ctx->buffer[index], &data[i], (len - i));
}

static void sha256_final(URL_SHA256_CTX *ctx, uint8_t *hash)
{
    uint8_t bits[8];
    uint32_t index = (ctx->count / 8) % 64;
    uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);

    uint64_t count = ctx->count;
    int i;
    for (i = 7; i >= 0; --i) {
        bits[i] = count & 0xff;
        count >>= 8;
    }

    uint8_t padding[64];
    memset(padding, 0, 64);
    padding[0] = 0x80;
    sha256_update(ctx, padding, padLen);
    sha256_update(ctx, bits, 8);

    for (i = 0; i < 8; ++i) {
        hash[i * 4] = (ctx->state[i] >> 24) & 0x000000ffu;
        hash[i * 4 + 1] = (ctx->state[i] >> 16) & 0x000000ffu;
        hash[i * 4 + 2] = (ctx->state[i] >> 8) & 0x000000ffu;
        hash[i * 4 + 3] = (ctx->state[i]) & 0x000000ffu;
    }
}

static void hmac_sha256(const uint8_t *key, uint32_t key_len, const uint8_t *data, uint32_t data_len, uint8_t *output)
{
    URL_SHA256_CTX ctx;
    uint8_t k_ipad[64], k_opad[64], hash[32];
    int i;

    if (key_len > 64) {
        sha256_init(&ctx);
        sha256_update(&ctx, key, key_len);
        sha256_final(&ctx, hash);
        key_len = 32;
        key = hash;
    }

    memset(k_ipad, 0, 64);
    memset(k_opad, 0, 64);
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    for (i = 0; i < 64; i++)
        k_ipad[i] ^= 0x36;

    for (i = 0; i < 64; i++)
        k_opad[i] ^= 0x5c;

    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, hash);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, hash, 32);
    sha256_final(&ctx, output);
}

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint32_t base64_encode_url(const uint8_t *input, uint32_t input_len, char *output, uint32_t output_size)
{
    uint32_t out_len = 0;
    int i = 0;
    int j;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (i < input_len) {
        char_array_3[0] = input[i++];
        char_array_3[1] = (i < input_len) ? input[i++] : 0;
        char_array_3[2] = (i < input_len) ? input[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        if (out_len + 4 > output_size)
            return 0;

        for (j = 0; j < 4; j++)
            output[out_len++] = base64_chars[char_array_4[j]];
    }

    int padding = (3 - (input_len % 3)) % 3;
    for (i = 0; i < padding; i++) {
        if (out_len < output_size)
            output[out_len++] = '=';
    }

    if (out_len < output_size)
        output[out_len] = '\0';

    return out_len;
}

static void url_encode(const char *input, char *output, uint32_t output_size)
{
    const char *hex_chars = "0123456789ABCDEF";
    uint32_t out_len = 0;

    while (*input && out_len + 4 < output_size) {
        char c = *input++;

        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            output[out_len++] = c;
        } else {
            output[out_len++] = '%';
            output[out_len++] = hex_chars[(c >> 4) & 0x0F];
            output[out_len++] = hex_chars[c & 0x0F];
        }
    }

    if (out_len < output_size)
        output[out_len] = '\0';
}

static char network_time_str[64] = {0};
static uint8_t network_time_valid = 0;
#define ASR_TIME_CONNECT_RETRY_MAX    (3)
#define ASR_SAFE_LINE_CHARS           (18)
#define ASR_TIME_HTTP_FALLBACK_ENABLE (0)

static void asr_clear_status_line(uint8 y)
{
    tft180_show_string(1, y, "                  ");
}

static void asr_show_safe_text(uint8 y, const char *msg)
{
    char line_buffer[ASR_SAFE_LINE_CHARS + 1];

    memset(line_buffer, 0, sizeof(line_buffer));
    if(NULL != msg)
    {
        strncpy(line_buffer, msg, ASR_SAFE_LINE_CHARS);
        line_buffer[ASR_SAFE_LINE_CHARS] = '\0';
    }

    asr_clear_status_line(y);
    tft180_show_string(1, y, line_buffer);
}

static void asr_show_auth_step(const char *msg)
{
    asr_show_safe_text(17, msg);
}

static void asr_show_time_step(const char *msg)
{
    asr_show_safe_text(25, msg);
}

static void asr_show_debug_step(const char *msg)
{
    asr_show_safe_text(33, msg);
}

static void time_parse_data(char* input, char* time_str)
{
    const char* dateField = strstr(input, "Date: ");
    if(dateField == NULL) return;
    dateField += 6;
    const char* end = strchr(dateField, '\r');
    if(end == NULL) end = strchr(dateField, '\n');
    if(end == NULL) return;
    {
        int length = (int)(end - dateField);
        if(length >= 64) length = 63;
        strncpy(time_str, dateField, length);
        time_str[length] = '\0';
    }
}

static uint8 time_get_data_http(const char *target_host, const char *target_port)
{
    uint8 retry_count = 0;

    if((NULL == target_host) || ('\0' == target_host[0]) ||
       (NULL == target_port) || ('\0' == target_port[0]))
    {
        return 1;
    }

    asr_show_time_step("Time TCP init");
    wifi_spi_socket_disconnect();
    while(retry_count < ASR_TIME_CONNECT_RETRY_MAX)
    {
        asr_clear_status_line(33);
        tft180_show_int(1, 33, retry_count + 1, 1);
        if(0 == wifi_spi_socket_connect("TCP", (char *)target_host, (char *)target_port, WIFI_SPI_LOCAL_PORT))
        {
            asr_show_time_step("Time TCP ok");
            break;
        }
        asr_show_time_step("Time retry");
        retry_count++;
        system_delay_ms(500);
    }

    if(ASR_TIME_CONNECT_RETRY_MAX <= retry_count)
    {
        asr_show_time_step("Time TCP fail");
        wifi_spi_socket_disconnect();
        return 1;
    }

    memset(http_request, 0, sizeof(http_request));
    snprintf(http_request, sizeof(http_request), "HEAD / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", target_host);
    wifi_spi_send_buffer((const uint8_t*)http_request, strlen(http_request));

    memset(wifi_spi_get_data_buffer, 0, sizeof(wifi_spi_get_data_buffer));
    system_delay_ms(1000);
    wifi_spi_read_buffer(wifi_spi_get_data_buffer, sizeof(wifi_spi_get_data_buffer));
    wifi_spi_socket_disconnect();

    memset(network_time_str, 0, sizeof(network_time_str));
    time_parse_data((char*)wifi_spi_get_data_buffer, network_time_str);
    return (network_time_str[0] != '\0') ? 0 : 1;
}
//0-成功 1-错误
static uint8 time_get_data(const char *target_host, const char *target_port)
{
    uint8 return_state;
    char debug_msg[24];

    (void)target_host;
    (void)target_port;

    asr_show_time_step("Time SPI geting");
    memset(network_time_str, 0, sizeof(network_time_str));
    while(wifi_spi_get_time(WIFI_SPI_GMT, network_time_str, (uint8)sizeof(network_time_str)))
    {
        asr_show_time_step("Time SPI retry");
        system_delay_ms(500);
        return_state=1;
    }
    return_state=0;
    if((0 == return_state) && (network_time_str[0] != '\0'))
    {
        asr_show_time_step("Time ok");
        network_time_valid = 1;
    }
    else
    {
        asr_show_time_step("Time SPI fail");
        if(network_time_str[0] != '\0')
        {
            snprintf(debug_msg, sizeof(debug_msg), "%.23s", network_time_str);
        }
        else
        {
            snprintf(debug_msg, sizeof(debug_msg), "ret=%d ver=%.2s", return_state, wifi_spi_version);
        }
        asr_show_debug_step(debug_msg);
#if ASR_TIME_HTTP_FALLBACK_ENABLE
        if(0 == time_get_data_http(target_host, target_port))
        {
            asr_show_time_step("Time HTTP ok");
            network_time_valid = 1;
        }
        else
        {
            asr_show_time_step("Time no date");
            network_time_valid = 0;
        }
#else

        network_time_valid = 0;
#endif
    }

    return network_time_valid ? 0 : 1;
}
uint8 first_get_time_flag=0;
// 0-成功 1-错误
static uint8 get_rfc1123_time(const char *target_host, const char *target_port, char *time_str, uint32_t max_len)
{
    if((NULL == time_str) || (0 == max_len))
    {
        asr_show_time_step("Time buf err");
        return 1;
    }
    if(first_get_time_flag==0)
    {
        for(uint8 i = 0; i < 20; i++)
        {
            tft180_clear();
            tft180_show_string(1, 1, "Time delay");
            tft180_show_int(1, 9, i, 2);
            system_delay_ms(1000);
            
        }
        tft180_clear();
        first_get_time_flag=0xFF;
    }

    if(!network_time_valid)
    {
        asr_show_time_step("Time need net");
        if(time_get_data(target_host, target_port))//0成功 1失败
        {
            // asr_show_time_step("Time get fail");
            time_str[0] = '\0';
            return 1;
        }
    }

    if(network_time_valid && network_time_str[0] != '\0')
    {
        asr_show_time_step("Time cached");
        strncpy(time_str, network_time_str, max_len - 1);
        time_str[max_len - 1] = '\0';
        return 0;
    }

    // asr_show_time_step("Time invalid");
    time_str[0] = '\0';
    return 1;
}

uint32_t build_asr_auth_url(const char *host, const char *path, char *out_url, uint32_t max_len)
{
    if (!host || !path || !out_url || max_len < 512)
    {
        asr_show_auth_step("Auth arg err");
        return 0;
    }

    asr_show_auth_step("Auth get time");
    char date_str[64];

    if(get_rfc1123_time(host, ASR_TARGET_PORT, date_str, sizeof(date_str)))
    {
        asr_show_auth_step("Auth time err");
        return 0;
    }

    asr_show_auth_step("Auth sign src");
    char signature_origin[512];
    snprintf(signature_origin, sizeof(signature_origin),
             "host: %s\ndate: %s\nGET %s HTTP/1.1", host, date_str, path);

    asr_show_auth_step("Auth hmac");
    uint32_t secret_len = strlen(ASR_APISecret);
    uint8_t signature_sha[32];
    hmac_sha256((const uint8_t*)ASR_APISecret, secret_len, (const uint8_t*)signature_origin, strlen(signature_origin), signature_sha);

    asr_show_auth_step("Auth sig b64");
    char signature_b64[64];
    base64_encode_url(signature_sha, 32, signature_b64, sizeof(signature_b64));

    asr_show_auth_step("Auth auth src");
    char authorization_origin[512];
    snprintf(authorization_origin, sizeof(authorization_origin),
             "api_key=\"%s\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%s\"",
             ASR_APIKey, signature_b64);

    asr_show_auth_step("Auth auth b64");
    char authorization_b64[512];
    uint32_t auth_len = base64_encode_url((const uint8_t*)authorization_origin, strlen(authorization_origin), authorization_b64, sizeof(authorization_b64));
    if (auth_len == 0)
    {
        asr_show_auth_step("Auth b64 err");
        return 0;
    }

    asr_show_auth_step("Auth url enc");
    char date_encoded[128];
    char auth_encoded[1024];
    url_encode(date_str, date_encoded, sizeof(date_encoded));
    url_encode(authorization_b64, auth_encoded, sizeof(auth_encoded));

    asr_show_auth_step("Auth url out");
    int url_len = snprintf(out_url, max_len, "ws://%s%s?authorization=%s&date=%s&host=%s",
                           host, path, auth_encoded, date_encoded, host);

    asr_show_auth_step("Auth done");
    return (uint32_t)url_len;
}
