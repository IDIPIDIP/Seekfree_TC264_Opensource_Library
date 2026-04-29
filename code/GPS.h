#ifndef CODE_GPS_H_
#define CODE_GPS_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define GPS_POINT_MAX (40)

#define GPS_INIT_SAMPLE_COUNT           (10)  // GPS初始化时采集的有效样本数量 用于计算零漂
#define GPS_INIT_MAX_PARSE_RETRY        (500) // GPS初始化时最大解析重试次数 避免死循环
#define GPS_FLASH_SECTOR                (0)   // 使用第0扇区
#define GPS_FLASH_PAGE                  (0)   // 使用第0页
#define GPS_FLASH_MAGIC                 (0x47505331u) // "GPS1"的ASCII码 用于验证Flash数据有效性
#define GPS_FLASH_VERSION               (1u)// 数据版本号 用于Flash数据格式升级

// GPS数据结构体定义
struct GPS_struct
{
    double lat_zero; //纬度零漂
    double lon_zero; //经度零漂
    double current_lat; //自身实时纬度
    double current_lon; //自身实时经度
    float current_x; //自身实时笛卡尔坐标x
    float current_y; //自身实时笛卡尔坐标y
    float x_offset; //笛卡尔坐标x误差
    float y_offset; //笛卡尔坐标y误差
    double lat_home; //基准点纬度
    double lon_home; //基准点经度
};

// GPS采样点结构体定义
struct GPS_point
{
    int point_num; // 当前采集的点数
    double lat[GPS_POINT_MAX]; //纬度
    double lon[GPS_POINT_MAX]; //经度
    float gps_x[GPS_POINT_MAX]; //高斯投影x坐标
    float gps_y[GPS_POINT_MAX]; //高斯投影y坐标
    int point_flag[GPS_POINT_MAX]; //点位标志位 0停车点 1转弯点 2直行点
    
};

// 全局变量声明
extern struct GPS_struct gps_data;  // GPS数据全局实例
extern struct GPS_point gps_point_data; // GPS采样点全局实例


// GPS经纬度转投影
void gps_to_diker(double latitude, double longitude, int i);  // 单点经纬度转笛卡尔坐标，
                                                // 存到gps_data.current_x和gps_data.current_y中
void gps_to_cartesian_all(void);                       // 所有采集点转笛卡尔坐标
float get_yaw_angle(float x1, float y1, float x2, float y2);  // 计算偏航角
float get_yaw_dis(float x1, float y1, float x2, float y2); // 计算投影坐标系中两点之间的距离

// GPS初始化和采点
int gps_init(); // GPS初始化零漂
int getpoint(); // 采点


// Flash存储功能
uint8 gps_save_to_flash(void);   // 保存GPS数据到Flash
uint8 gps_load_from_flash(void); // 从Flash读取GPS数据
void gps_clear_flash(void);      // 清除Flash中的GPS数据



#endif /* CODE_GPS_H_ */
