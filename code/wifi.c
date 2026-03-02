#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

void wifi_init(void)
{
    wifi_spi_wifi_connect("ATCG","QWERTYUI");
    wifi_spi_socket_connect("TCP", "192.168.137.1", "8080", "6666");
}