#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

void mode1()//倒数两个点在车库中，用于倒车入库
{
    gps_to_cartesian_all();  // 转换所有采集的点
    
    for(int i=0;i<gps_point_data.point_num-1;i++)
    {
        //计算当前点与下一个点的偏航角和距离，并根据偏航角调整速度差来引导车辆前进
        Calculate_Differential_Speed(get_yaw_angle(integrate_point.current_x, integrate_point.current_y, integrate_point.x[i+1], integrate_point.y[i+1]), n_speed, &motor_speed);
        if(i==gps_point_data.point_num-2)//留2点用于引导倒车入库
        break;
    }
    while(1)
    {
        //如果当前点与一个点的角度小于3度，直接倒车
    if(get_yaw_angle(integrate_point.current_x, integrate_point.current_y, integrate_point.x[gps_point_data.point_num-1], integrate_point.y[gps_point_data.point_num-1])<3.0f)
    {
       dir=0;
       //如果当前点与下一个点的距离小于0.1m，停止
       if(integrate_point.x[gps_point_data.point_num-2]-integrate_point.x[gps_point_data.point_num-1]<0.1f&&integrate_point.y[gps_point_data.point_num-2]-integrate_point.y[gps_point_data.point_num-1]<0.1f)
        n_speed=0.0f;
        break;
    }
    
    else
    {
    
       dir=1;
       //如果当前位置与车库的角度大于3度，前进  
        float temp_angle=get_yaw_angle(integrate_point.current_x, integrate_point.current_y, integrate_point.x[gps_point_data.point_num-1], integrate_point.y[gps_point_data.point_num-1]);
        Calculate_Differential_Speed(get_yaw_angle(integrate_point.current_x, integrate_point.current_y, integrate_point.x[gps_point_data.point_num-1], integrate_point.y[gps_point_data.point_num-1]), n_speed, &motor_speed);
    
    }
    }
    
}
