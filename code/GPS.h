#ifndef CODE_GPS_H_
#define CODE_GPS_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define GPS_POINT_MAX (40)

// GPS数据结构体定义
struct GPS_struct
{
    float lat_zero; //纬度零漂
    float lon_zero; //经度零漂
    float current_lat; //自身实时纬度
    float current_lon; //自身实时经度
    float current_x; //自身实时笛卡尔坐标x
    float current_y; //自身实时笛卡尔坐标y
    float x_offset; //笛卡尔坐标x误差
    float y_offset; //笛卡尔坐标y误差
    float lat_home; //基准点纬度
    float lon_home; //基准点经度
};

struct GPS_point
{
    int point_num; // 当前采集的点数
    float lat[GPS_POINT_MAX]; //纬度
    float lon[GPS_POINT_MAX]; //经度
    float gps_x[GPS_POINT_MAX]; //高斯投影x坐标
    float gps_y[GPS_POINT_MAX]; //高斯投影y坐标

};


// 全局变量声明
extern struct GPS_struct gps_data;  // GPS数据全局实例
extern struct GPS_point gps_point_data; // GPS采样点全局实例
extern float central_meridian;      // 中央子午线(河南开封: 114°E)


// GPS经纬度转投影
void gps_to_diker(double latitude, double longitude);  // 单点经纬度转笛卡尔坐标，存到gps_data.current_x和gps_data.current_y中
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
