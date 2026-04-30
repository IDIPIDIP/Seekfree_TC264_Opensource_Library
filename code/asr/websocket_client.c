#include "zf_common_headfile.h"
#include "websocket_client.h"
#include "base64.h"

static char ws_temp_data[10240];

// void url_encode(const char* str, char* encoded)
// {
//     const char* hex_chars = "0123456789ABCDEF";
//     size_t len = strlen(str);
// //    size_t encoded_len = 3 * len + 1; // 最坏情况下每个字符都编码

//     char* ptr = encoded;
//     for(size_t i = 0; i < len; i++)
//     {
//         unsigned char c = str[i];
//         if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~')
//         {
//             *ptr++ = c;
//         }
//         else if(c == ' ')
//         {
//             *ptr++ = '+'; // 将空格编码为 +
//         }
//         else
//         {
//             *ptr++ = '%';
//             *ptr++ = hex_chars[(c >> 4) & 0x0F];
//             *ptr++ = hex_chars[c & 0x0F];
//         }
//     }
//     *ptr = '\0';
// }

// 创建 WebSocket 握手请求
static void create_handshake_request(const char* host, const char* path, const char* key, char* request)
{
    snprintf(request, 1024,
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: %s\r\n"
             "Sec-WebSocket-Version: 13\r\n\r\n",
             path, host, key);
}

// 解析 WebSocket 握手响应
static bool parse_handshake_response(const char* response, const char* expected_accept_key)
{
    const char* accept_header = "Sec-WebSocket-Accept: ";
    const char* pos = strstr(response, accept_header);
    if(!pos) return false;

    pos += strlen(accept_header);
    const char* end = strchr(pos, '\r');
    if(!end) return false;

    char accept_key[128];
    strncpy(accept_key, pos, end - pos);
    accept_key[end - pos] = '\0';

    return strcmp(accept_key, expected_accept_key) == 0;
}

// 连接到 WebSocket 服务器
bool websocket_client_connect(const char* url)
{
    const char* host_start = NULL;
    const char* path_start = NULL;
    const char* port_pos = NULL;
    char host[128];
    char hostname[128];
    char path[512];
    char port[8] = "80";
    size_t host_len;

    if((NULL == url) || ('\0' == url[0]))
    {
        return false;
    }

    if(0 == strncmp(url, "wss://", 6))
    {
        host_start = url + 6;
    }
    else if(0 == strncmp(url, "ws://", 5))
    {
        host_start = url + 5;
    }
    else
    {
        return false;
    }

    path_start = strchr(host_start, '/');
    if(NULL == path_start)
    {
        host_len = strlen(host_start);
    }
    else
    {
        host_len = (size_t)(path_start - host_start);
    }

    if((0 == host_len) || (host_len >= sizeof(host)))
    {
        return false;
    }

    memset(host, 0, sizeof(host));
    memcpy(host, host_start, host_len);
    host[host_len] = '\0';

    if(NULL == path_start)
    {
        strcpy(path, "/");
    }
    else
    {
        if(strlen(path_start) >= sizeof(path))
        {
            return false;
        }
        strcpy(path, path_start);
    }

    // 提取主机和端口，优先使用URL中的端口
    memset(hostname, 0, sizeof(hostname));
    port_pos = strrchr(host, ':');
    if((NULL != port_pos) && (port_pos != host))
    {
        size_t name_len = (size_t)(port_pos - host);
        size_t port_len = strlen(port_pos + 1);
        if((name_len >= sizeof(hostname)) || (0 == port_len) || (port_len >= sizeof(port)))
        {
            return false;
        }
        memcpy(hostname, host, name_len);
        hostname[name_len] = '\0';
        memcpy(port, port_pos + 1, port_len);
        port[port_len] = '\0';
    }
    else
    {
        if(strlen(host) >= sizeof(hostname))
        {
            return false;
        }
        strcpy(hostname, host);
    }

    if(wifi_spi_socket_connect("TCP", hostname, port, WIFI_SPI_LOCAL_PORT))
    {
        tft180_show_string(1, 25, "TCP connect err");
        wifi_spi_send_string("Failed to connect to xf server");
        return false;
    }

    char sec_websocket_key[32] = "dGhlIHNhbXBsZSBub25jZQ==";
    memset(sec_websocket_key, 0, sizeof(sec_websocket_key));
    uint8_t random_bytes[16];
    for(int i = 0; i < 16; i++) random_bytes[i] = rand() % 256;
    base64_encode(random_bytes, sec_websocket_key, 16);

    const char* guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char combined_key[512];
    memset(combined_key, 0, sizeof(combined_key));
    snprintf(combined_key, sizeof(combined_key), "%s%s", sec_websocket_key, guid);

    uint8_t hash[SHA1_HASH_SIZE];
    memset(hash, 0, sizeof(hash));
    sha1_hash((const uint8_t*)combined_key, strlen(combined_key), hash);

    char sec_websocket_accept[128];
    memset(sec_websocket_accept, 0, sizeof(sec_websocket_accept));
    base64_encode(hash, sec_websocket_accept, SHA1_HASH_SIZE);

    char handshake_request[1024];
    memset(handshake_request, 0, sizeof(handshake_request));

    create_handshake_request(host, path, sec_websocket_key, handshake_request);

    wifi_spi_send_buffer((uint8_t*)handshake_request, strlen(handshake_request));
    system_delay_ms(1500);
    // 接收握手响应
    uint8_t handshake_response[1024];
    memset(handshake_response, 0, sizeof(handshake_response));
    uint32_t bytes_received = wifi_spi_read_buffer(handshake_response, sizeof(handshake_response) - 1);
    if(bytes_received <= 0)
    {
        tft180_show_string(1, 25, "Handshake rx err");
        printf("Handshake receive failed");
        return false;
    }
    handshake_response[bytes_received] = '\0';

    // 打印握手响应和预期的 Sec-WebSocket-Accept 值
    wifi_spi_send_string("Handshake response:\n");
    wifi_spi_send_buffer(handshake_response, bytes_received);
    tft180_show_string(1, 25, "Handshake response:");//
    tft180_show_string(1, 33, handshake_response);//可能超数
    wifi_spi_send_string("\nExpected accept: ");
    wifi_spi_send_string(sec_websocket_accept);
    wifi_spi_send_string("\n");

    
    // 验证握手响应
    if(!parse_handshake_response((char*)handshake_response, sec_websocket_accept))
    {
        tft180_show_string(1, 25, "Handshake fail");
        return false;
    }

    // 更新客户端状态
    tft180_show_string(1, 25, "Handshake ok");
    return true;
}

// 发送数据到 WebSocket 服务器
bool websocket_client_send(const uint8_t* data, uint32_t len)
{
    memset(ws_temp_data, 0, sizeof(ws_temp_data));
    uint32_t frame_len = websocket_create_frame((uint8*)ws_temp_data, data, len, 1, 1); // 使用文本帧操作码

    return wifi_spi_send_buffer((uint8*)ws_temp_data, frame_len) == 0;
}

bool websocket_client_receive(uint8_t* buffer)
{
    static int bytes_count = 0;
    memset(buffer, 0, 4096);
    uint32_t bytes_received = wifi_spi_read_buffer(buffer, 4096);
    bytes_count += bytes_received;
    if(bytes_count > 50)
    {
        bytes_count = 0;
        
        for(int i = 0; i < bytes_count; i++)//bug，但没影响
        {
            if(buffer[i] == 0)
            {
                buffer[i] = ' ';
            }
        }
        return false;
    }
    else
    {
        return true;
    }
}

// 关闭 WebSocket 连接
void websocket_client_close()
{
    wifi_spi_socket_disconnect(); // 断开 TCP 连接
}

// WebSocket 数据帧创建函数
uint32_t websocket_create_frame(uint8_t* frame, const uint8_t* payload, uint64_t payload_len, uint8_t type, bool mask)
{
    uint8_t* ptr = frame;
    static uint8_t mask_key[4] = {0};
//    uint64_t masked_payload_len = payload_len;

    *ptr++ = 0x80 | (type & 0x0F); // FIN=1, Opcode=操作码

    if(mask && (mask_key[0] + mask_key[1] + mask_key[2] + mask_key[3]) == 0)
    {
        // 生成随机掩码键
        for(int i = 0; i < 4; i++)
        {
            mask_key[i] = adc_convert(RANDOM_NUM_ADC) % 256;    // 使用ADC生成随机数
        }
    }
    if(payload_len <= 125)
    {
        *ptr++ = (uint8_t)(payload_len | (mask ? 0x80 : 0x00)); // 设置 MASK 标志位
    }
    else if(payload_len <= 65535)
    {
        *ptr++ = 126 | (mask ? 0x80 : 0x00);                    // 标记使用 16 位长度，并设置 MASK 标志位
        *ptr++ = (uint8_t)((payload_len >> 8) & 0xFF);          // 高字节
        *ptr++ = (uint8_t)(payload_len & 0xFF);                 // 低字节
    }
    else
    {
        *ptr++ = 127 | (mask ? 0x80 : 0x00);                    // 标记使用 64 位长度，并设置 MASK 标志位
        // 将 64 位长度按大端序写入
        for(int i = 7; i >= 0; i--)
        {
            *ptr++ = (uint8_t)((payload_len >> (i * 8)) & 0xFF);
        }
    }

    // 添加掩码键（如果需要）
    if(mask)
    {
        memcpy(ptr, mask_key, 4);
        ptr += 4;
    }

    // 掩码有效载荷（如果需要）
    uint8_t* payload_start = ptr;
    for(uint64_t i = 0; i < payload_len; i++)
    {
        if(mask)
        {
            payload_start[i] = payload[i] ^ mask_key[i % 4];
        }
        else
        {
            payload_start[i] = payload[i];
        }
    }
    // 返回帧总长度
    return (uint32_t)((payload_start - frame) + payload_len);
}

