#include "zf_common_typedef.h"
#include "zf_common_headfile.h"
#include "zf_driver_flash.h"
#include "GPS.h"




struct GPS_struct gps_data = {0}; // 全局GPS数据实例
struct GPS_point gps_point_data = {0}; // 全局GPS采样点实例


/**
 * @brief GPS初始化零漂
 */
int gps_init()
{
    int valid_count = 0;
    int retry_count = 0;
    while(valid_count < GPS_INIT_SAMPLE_COUNT && retry_count < GPS_INIT_MAX_PARSE_RETRY)
    {
        if(gnss_data_parse() == 0)
        {
            gps_data.lat_zero += gnss.latitude;
            gps_data.lon_zero += gnss.longitude;
            valid_count++;
        }
        retry_count++;
    }

    if(valid_count == 0)
    {
        return 0;
    }

    gps_data.lat_zero /= (double)valid_count;
    gps_data.lon_zero /= (double)valid_count;
    gps_data.lat_home = gps_data.lat_zero;// 将零漂值作为基准点
    gps_data.lon_home = gps_data.lon_zero;// 将零漂值作为基准点

    return 1;
}


/**
 * @brief 采集GPS点位数据
 */
int getpoint()
{
    if(gps_point_data.point_num >= GPS_POINT_MAX)
    {
        return 0;
    }

    if(gnss_data_parse()==0)
    {
        gps_point_data.lat[gps_point_data.point_num] = gnss.latitude;
        gps_point_data.lon[gps_point_data.point_num] = gnss.longitude;
        gps_to_diker(gps_point_data.lat[gps_point_data.point_num], gps_point_data.lon[gps_point_data.point_num],gps_point_data.point_num);
        gps_point_data.point_num++;
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 将经纬度转换为笛卡尔坐标
 * @param latitude 纬度
 * @param longitude 经度
 */
void gps_to_diker(double latitude , double longitude,int i)
{
    double dis = 0 , angle = 0;
    //计算发车点位与实时点位之间的距离  Home_x:经度 Home_y:纬度
    dis = get_two_points_distance(gps_data.lat_home  , gps_data.lon_home , latitude , longitude);
    //计算发车点位与实时点位之间的方位角
    angle = get_two_points_azimuth(gps_data.lat_home , gps_data.lon_home , latitude , longitude);
    //分解到XY轴上
    gps_point_data.gps_x[i] = dis * cos(angle / 180 * 3.1415926);
    gps_point_data.gps_y[i] = dis * sin(angle / 180 * 3.1415926);
}

/**
 * @brief 将所有采集点的经纬度转换为笛卡尔坐标，存到gps_point_data.gps_x和gps_point_data.gps_y中
 */
void gps_to_cartesian_all()
{
    for(int i = 0; i < gps_point_data.point_num; i++)
    {
         double dis = 0 , angle = 0;
    //计算发车点位与实时点位之间的距离  Home_x:经度 Home_y:纬度
    dis = get_two_points_distance(gps_data.lat_home  , gps_data.lon_home , gps_point_data.lat[i] , gps_point_data.lon[i]);
    //计算发车点位与实时点位之间的方位角
    angle = get_two_points_azimuth(gps_data.lat_home , gps_data.lon_home , gps_point_data.lat[i] , gps_point_data.lon[i]);
    //分解到XY轴上
    gps_point_data.gps_x[i] = dis * cos(angle / 180 * 3.1415926);
    gps_point_data.gps_y[i] = dis * sin(angle / 180 * 3.1415926);
    }
}

/**
 * @brief 计算投影坐标系中两点之间的偏航角（方位角）
 * @param x1 第一个点的X坐标（北向分量）
 * @param y1 第一个点的Y坐标（东向分量）
 * @param x2 第二个点的X坐标（北向分量）
 * @param y2 第二个点的Y坐标（东向分量）
 */
float get_yaw_dis(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// ===================== 辅助函数：数据校验码 =====================

/**
 * @brief 计算GPS数据的校验码
 * @param data 需要计算校验码的数据数组
 * @param len 数据数组长度
 */
static uint32 gps_flash_checksum(const uint32 *data, uint32 len)
{
    uint32 checksum = 0;
    uint32 i = 0;
    for(i = 0; i < len; i++)
    {
        checksum ^= data[i] + (i * 0x9E3779B9u);
    }
    return checksum;
}


// ===================== Flash 存储功能 =====================

/**
 * @brief 将GPS数据保存到Flash
 * 成功返回1，失败返回0
 */
uint8 gps_save_to_flash(void)
{
    if(gps_point_data.point_num < 0 || gps_point_data.point_num > GPS_POINT_MAX)
    {
        return 0;
    }

    // Buffer大小: magic(1) + version(1) + point_num(1) + lat_zero(2) + lon_zero(2) + points(4*MAX)
    uint32 write_buffer[1 + 1 + 1 + 2 + 2 + 4 * GPS_POINT_MAX];
    uint32 index = 0;
    
    write_buffer[index++] = GPS_FLASH_MAGIC;
    write_buffer[index++] = GPS_FLASH_VERSION;
    
    // 保存点位数
    write_buffer[index++] = (uint32)gps_point_data.point_num;
    
    // 保存零漂值 (double需要2个uint32)
    union { double d; uint32 u[2]; } temp_d;
    
    temp_d.d = gps_data.lat_zero;
    write_buffer[index++] = temp_d.u[0];
    write_buffer[index++] = temp_d.u[1];
    
    temp_d.d = gps_data.lon_zero;
    write_buffer[index++] = temp_d.u[0];
    write_buffer[index++] = temp_d.u[1];
    
    // 保存所有点的经纬度 (每个double需要2个uint32)
    for(int i = 0; i < gps_point_data.point_num; i++)
    {
        temp_d.d = gps_point_data.lat[i];
        write_buffer[index++] = temp_d.u[0];
        write_buffer[index++] = temp_d.u[1];
        
        temp_d.d = gps_point_data.lon[i];
        write_buffer[index++] = temp_d.u[0];
        write_buffer[index++] = temp_d.u[1];
    }

    write_buffer[index++] = gps_flash_checksum(write_buffer, index);
    
    // 擦除页
    flash_erase_page(GPS_FLASH_SECTOR, GPS_FLASH_PAGE);
    
    // 写入数据
    flash_write_page(GPS_FLASH_SECTOR, GPS_FLASH_PAGE, write_buffer, index);
    
    return 1;
}

/**
 * @brief 从Flash读取GPS数据
 * 成功返回1，失败返回0
 */
uint8 gps_load_from_flash(void)
{
    // 检查是否有数据
    if(flash_check(GPS_FLASH_SECTOR, GPS_FLASH_PAGE) == 0)
    {
        return 0;  // Flash为空，无数据
    }
    
    // Buffer大小: magic(1) + version(1) + point_num(1) + lat_zero(2) + lon_zero(2) + points(4*MAX)
    uint32 read_buffer[1 + 1 + 1 + 2 + 2 + 4 * GPS_POINT_MAX];
    flash_read_page(GPS_FLASH_SECTOR, GPS_FLASH_PAGE, read_buffer, (uint16)(1 + 1 + 1 + 2 + 2 + 4 * GPS_POINT_MAX));
    
    uint32 index = 0;
    
    if(read_buffer[index++] != GPS_FLASH_MAGIC)
    {
        return 0;
    }
    if(read_buffer[index++] != GPS_FLASH_VERSION)
    {
        return 0;
    }
    
    // 读取点位数
    gps_point_data.point_num = (int)read_buffer[index++];
    if(gps_point_data.point_num > GPS_POINT_MAX || gps_point_data.point_num < 0)
    {
        gps_point_data.point_num = 0;
        return 0;  // 数据异常
    }
    
    // 读取零漂值 (double需要2个uint32)
    union { double d; uint32 u[2]; } temp_d;
    
    temp_d.u[0] = read_buffer[index++];
    temp_d.u[1] = read_buffer[index++];
    gps_data.lat_zero = temp_d.d;
    
    temp_d.u[0] = read_buffer[index++];
    temp_d.u[1] = read_buffer[index++];
    gps_data.lon_zero = temp_d.d;
    
    // 读取所有点的经纬度 (每个double需要2个uint32)
    for(int i = 0; i < gps_point_data.point_num; i++)
    {
        temp_d.u[0] = read_buffer[index++];
        temp_d.u[1] = read_buffer[index++];
        gps_point_data.lat[i] = temp_d.d;
        
        temp_d.u[0] = read_buffer[index++];
        temp_d.u[1] = read_buffer[index++];
        gps_point_data.lon[i] = temp_d.d;
    }

    if(read_buffer[index] != gps_flash_checksum(read_buffer, index))
    {
        gps_point_data.point_num = 0;
        return 0;
    }

    gps_to_cartesian_all();
    
    return 1;
}

/**
 * @brief 清除Flash中的GPS数据
 */
void gps_clear_flash(void)
{
    flash_erase_page(GPS_FLASH_SECTOR, GPS_FLASH_PAGE);
}

// ===================== 偏航角计算功能 =====================
/**
 * @brief 计算投影坐标系中两点之间的偏航角（方位角）
 * @param x1 第一个点的X坐标（北向分量）
 * @param y1 第一个点的Y坐标（东向分量）
 * @param x2 第二个点的X坐标（北向分量）
 * @param y2 第二个点的Y坐标（东向分量）
 */
float get_yaw_angle(float x1, float y1, float x2, float y2)
{
    
    
    float dx = x2 - x1;  // 北向分量 (X轴是北)
    float dy = y2 - y1;  // 东向分量 (Y轴是东)
    
    // atan2f(dy, dx) 返回从X轴（北）到向量的角度
    // 结果范围: -180度到180度
    if(dx == 0.0f && dy == 0.0f)
    {
        return 0.0f; // 两点重合，默认返回0度
    }
    float angle = atan2f(dy, dx) * 180.0f / 3.14159265358979f;
    
    // 转换为0-360度范围
    if(angle < 0.0f)
    {
        angle += 360.0f;
    }
    
    return angle;
}
