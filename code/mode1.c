#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

int mode1()//倒数4个点在车库中，用于倒车入库
{
    flash_read_struct_toBuffer(0);//从flash中读取点到buffer_point结构体中
    double x1[INTEGRATE_MAX_POINTS];
    double y1[INTEGRATE_MAX_POINTS];
    int use_num = buffer_point.point_num - 4;
    for (int i = 0; i < use_num; i++)//除了最后四个点 其他点都用来拟合路径
    {
        x1[i] = buffer_point.x[i];
        y1[i] = buffer_point.y[i];
    }
    set_control_mode(1); // 设置为自动模式
    point_num=curve_fit(x1, y1, use_num, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
    path_track_set_mode(0); // 设置为正向路径追踪模式
    path_track_load_path(&path_track_points_struct); // 加载路径点数据到路径
    path_track_start(); // 开始路径跟踪
    while(track_satate==0)
    {}
    if(track_satate==1) //如果路径跟踪完成 则切换到倒车模式 继续跟踪剩余的4个点
    {
        float reverse_x[4];
        float reverse_y[4];
        for (int i = 0; i < 4; i++)//最后四个点用来倒车入库
        {
            reverse_x[i] = buffer_point.x[buffer_point.point_num - 4 + i] - buffer_point.x[buffer_point.point_num - 5]; // 将最后四个点的x坐标转换为相对于倒车起点的坐标
            reverse_y[i] = buffer_point.y[buffer_point.point_num - 4 + i] - buffer_point.y[buffer_point.point_num - 5]; // 将最后四个点的y坐标转换为相对于倒车起点的坐标
        }
        path_track_set_mode(1); // 设置为倒车路径追踪模式
        point_num=curve_fit(reverse_x, reverse_y, 4, 0.1f, path_track_points_struct.x, path_track_points_struct.y, INTEGRATE_MAX_POINTS);
        path_track_start();//结束函数写loop里了
    }
    return 0;
}
