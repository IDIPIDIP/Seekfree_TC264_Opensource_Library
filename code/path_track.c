#include "zf_common_headfile.h"

/*********************************************************************************************************************
*功能名称       路径跟踪算法 Pure Pursuit
*引脚定义
*硬件参数
*使用说明       处理对象为Trajectory2DPoint_struct结构体 其中包含了路径点的坐标和点数等信息
*使用说明
*使用说明       输出对象为 前轮转角
*使用说明       考虑速度问题 在曲率小的地方减速 并降低预瞄距离 以提高跟踪精度
*使用说明       纯路径追踪，推荐调用周期为20-50ms 过快可能导致计算资源浪费 过慢可能导致跟踪性能下降
*********************************************************************************************************************/
//测量值准备:
//      偏航角
//      车速
//      当前坐标

//参数准备：
//      车轮轴距L_wheel_base
//      预瞄距离速度项系数K_v
//      预瞄基础距离 L0
//      预瞄距离最小值L_d_min
//      预瞄距离最大值L_d_max
// 
#define PATH_TRACK_SEARCH_MAX   3       // 路径点搜索范围 上限
#define PATH_TRACK_SEARCH_MIN   2       // 路径点搜索范围 下限

#define L_WHEEL_BASE            0.783f  // 轴距，单位为米，根据实际机械结构调整

Trajectory2DPoint_struct path_track_points_struct; // 路径点结构体 当前寻迹路径
float K_v=0.5f; // 预瞄距离速度项系数，根据实际调试结果调整
float K_c=0.1f; // 预瞄距离曲率项系数，根据实际调试结果调整
float L0=0.5f; // 预瞄基础距离，单位为米，根据实际调试结果调整
float L_d_min=0.3f; // 预瞄距离最小值，单位为米，根据实际调试结果调整
float L_d_max=1.0f; // 预瞄距离最大值，单位为米，根据实际调试结果调整

float current_speed=0; // 当前车速，单位为米/秒，根据实际测量更新
float yaw_angle=0;
float pure_persuit_steering_angle=0; // 纯路径追踪计算得到的前轮转角，单位为弧度，根据实际计算更新
uint16 search_index=0;
uint8 start_flag=0; // 路径跟踪开始标志 用于初始化预瞄点;
float x_proj=0;
float y_proj=0;
uint8 path_track_mode_flag = 0; // 路径跟踪模式标志 0为正向路径追踪 1为倒车路径追踪
uint8 path_track_start_flag = 0xFF; // 上一次路径跟踪模式标志 用于检测模式切换
int track_satate=0;//路径跟踪状态 1为完成

float path_track_norm(float X1, float Y1, float X2, float Y2)
{
    float norm = sqrtf((X1-X2) * (X1-X2) + (Y1-Y2) * (Y1-Y2));
    return norm;
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      当前状态更新  根据gps和imu融合的坐标点，更新当前坐标和偏航角等状态量
//  参数说明
//  参数说明
//  返回参数      void
//  使用示例      需要放在定时中断函数中 定时调用 (未完成点坐标和偏航角获取 暂时不可用)
//-------------------------------------------------------------------------------------------------------------------
void get_current_measurement(void)
{
    //正向路径追踪 状态更新 当前坐标为两后轮中心坐标 偏航角为车身正方向
    // 获取当前测量值 更新到全局变量中
    // x_proj = get_current_x(); // 获取当前坐标x 为两后轮中心坐标
    // y_proj = get_current_y(); // 获取当前坐标y 为两后轮中心坐标
    // yaw_angle = get_yaw_angle();//获取偏航角 偏航角为车身正方向
    // current_speed = get_current_speed();//获取两后轮平均转速
    // if(path_track_mode_flag==1)//倒车状态
    // {
    //     yaw_angle = -yaw_angle;//获取偏航角 偏航角需要转换偏航角方向，为反方向
    //     current_speed =-current_speed;//获取两后轮平均转速 倒车时速度为负 修正为正值

    // }
}

//计算三点曲率 计算三个离散点的曲率
float curvature(float x1, float y1, float x2, float y2, float x3, float y3)
{
    float a = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    float b = sqrtf((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
    float c = sqrtf((x1 - x3) * (x1 - x3) + (y1 - y3) * (y1 - y3));
    float s = (a + b + c) / 2.0f;
    float area = sqrtf(s * (s - a) * (s - b) * (s - c));
    if (area < 1e-6f) // 防止除以零
        return 1000000; // 返回一个很大的值表示近似直线
    return (4.0f * area) / (a * b * c);
}

//正向路径追踪 偏航角位车身正方向 当前位置坐标为后轮中心位置
//倒车路径追踪 需要将偏航角转换为车身正方向 当前位置坐标为前轮中心位置
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      纯路径跟踪算法
//  参数说明      path_track_struct 路径点结构体 所追踪的路径线 为离散的坐标点数据
//  参数说明
//  返回参数      控制下的前轮转角   （该转角未经过限幅 需要根据实际情况进行限幅处理）
//  使用示例      在定时中断函数中 定时调用 获取当前测量值 更新状态量 计算并输出前轮转角
//-------------------------------------------------------------------------------------------------------------------
float pure_path_track_get_steering(Trajectory2DPoint_struct *points_struct)
{
    get_current_measurement();//更新状态量
    //从起始点开始逐个跟踪
    uint16 i=0;
    uint16 target_index=0;;//目标点索引
    
    float L_s=0;//当前投影点与路径点的距离
    float L_d_base=0;//预瞄距离基础值 根据速度调整
    float L_d=0;//最终预瞄距离 根据曲率调整
    float L_step=0;//路径点之间的累积距离 用于寻找目标点
    float K_curvature=0;//当前点的曲率半径
    float e_y=0;//横向误差
    float temp=0;//临时变量 用于寻找投影点
    
    float target_steering_angle=0;//目标前轮转角
    if(start_flag==0)
    {
        // 初始投影点为路径上的第一个点
        x_proj=points_struct->x[0];
        y_proj=points_struct->y[0];
        search_index=0;
        start_flag=1;
        return 0;//初始状态下不进行转向控制
    }
    
        //计算此时坐标投影
        i=search_index;
        if(search_index<PATH_TRACK_SEARCH_MIN)
        {
            i=PATH_TRACK_SEARCH_MIN;
        }
        if(search_index>points_struct->point_num-PATH_TRACK_SEARCH_MAX)
        {
            i=points_struct->point_num-PATH_TRACK_SEARCH_MAX;
        }//限制投影点边界
        L_s=path_track_norm(x_proj, y_proj, points_struct->x[search_index], points_struct->y[search_index]);
        for(i-=PATH_TRACK_SEARCH_MIN; i<search_index+PATH_TRACK_SEARCH_MAX; i++)
        {
            temp=path_track_norm(x_proj, y_proj, points_struct->x[i], points_struct->y[i]);
            if( temp<=L_s)//找到更近的点 更新投影点和索引
            {
                search_index=i;
                L_s=temp;
            }
        }

        //根据当前速度和预瞄距离计算预瞄距离
        L_d_base=L0 + K_v * current_speed; // 根据当前速度调整预瞄距离基础值
        //获得曲率半径 根据曲率调整预瞄距离 基于曲率的调整可以使得在曲率较大的地方预瞄距离更短，从而提高跟踪精度
        if(search_index==0) K_curvature=curvature(points_struct->x[0], points_struct->y[0], points_struct->x[1], points_struct->y[1], points_struct->x[2], points_struct->y[2]);
        else if(search_index==points_struct->point_num-1) K_curvature=curvature(points_struct->x[search_index-2], points_struct->y[search_index-2], points_struct->x[search_index-1], points_struct->y[search_index-1], points_struct->x[search_index], points_struct->y[search_index]);
        else K_curvature=curvature(points_struct->x[search_index-1], points_struct->y[search_index-1], points_struct->x[search_index], points_struct->y[search_index], points_struct->x[search_index+1], points_struct->y[search_index+1]);
        //根据曲率调整预瞄距离
        L_d=L_d_base/(1+K_c*K_curvature);
        pid_limit(&L_d, L_d_min, L_d_max);
        //根据预瞄距离计算目标点坐标
        for(i=search_index; i<points_struct->point_num-1; i++)
        {
            L_step+=path_track_norm(points_struct->x[i], points_struct->y[i], points_struct->x[i+1], points_struct->y[i+1]);
            if(L_step>=L_d)
            {
                target_index=i+1;

                break;
            }
        }
        //获取到目标点的横向误差
        e_y=(points_struct->x[target_index]-x_proj)*sinf(yaw_angle* AtR)-(points_struct->y[target_index]-y_proj)*cosf(yaw_angle*AtR);
        //计算并返回前轮转角 此前轮转角未经过限幅 根据纯跟踪算法计算得到的转角可能较大 需要根据实际情况进行限幅处理
        return atan2f(2*L_WHEEL_BASE*e_y, L_d*L_d);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      纯路径跟踪算法初始化函数
//  参数说明
//  参数说明
//  返回参数      void
//  使用示例      开启定时中断
//-------------------------------------------------------------------------------------------------------------------

void path_track_init(void)
{
    //路径跟踪初始化函数 目前无特殊初始化需求 可以根据实际情况添加
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      纯路径跟踪算法开始停止函数 设置路径跟踪开始标志位 开启路径跟踪循环
//  参数说明
//  参数说明
//  返回参数      void
//  使用示例      调用此函数则可开启路径追踪 默认为正向追踪
//-------------------------------------------------------------------------------------------------------------------

void path_track_start(void)
{
    path_track_start_flag=0; // 设置路径跟踪开始标志位 开启路径跟踪循环
    track_satate=0; // 初始化路径跟踪状态为未完成
    start_flag=0;
    search_index=0;
    point_track=0;
}
void path_track_stop(void)
{
    path_track_start_flag=0xFF; // 设置路径跟踪停止标志位 关闭路径跟踪循环
    track_satate=1;
}


//-------------------------------------------------------------------------------------------------------------------
//  函数简介      模式切换函数
//  参数说明      mode：路径跟踪模式 0为正向路径追踪 1为倒车路径追踪
//  参数说明
//  返回参数      void
//  使用示例      选择 前进 或 倒车追踪
//-------------------------------------------------------------------------------------------------------------------
void path_track_set_mode(uint8 mode)
{
    if(mode==0) // 正向路径追踪
    {
        path_track_mode_flag=0;
    }
    else if(mode==1) // 倒车路径追踪
    {
        path_track_mode_flag=1;
    }
    else
    {
        // 无效模式，默认设置为正向路径追踪
        path_track_mode_flag=0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      路径加载函数 将路径点数据加载到全局变量中 供路径跟踪算法使用
//  参数说明      path_struct：路径点结构体指针 包含了路径点
//  参数说明
//  返回参数      void
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void path_track_load_path(Trajectory2DPoint_struct *path_struct)
{
    //路径加载函数 将路径点数据加载到全局变量中 供路径跟踪算法使用
    //根据实际情况可以添加路径数据的预处理等功能
    if (path_struct != NULL)
    {
        path_track_points_struct = *path_struct;
        x_proj = path_struct->x_offset; // 初始化投影点为路径上的原点
        y_proj = path_struct->y_offset; // 初始化投影点为路径上的原点
        search_index=0;
        start_flag=0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      路径跟踪主循环函数 需要放在定时中断函数中 定时调用 获取当前测量值 更新状态量 计算并输出前轮转角
//  参数说明      void
//  参数说明      void
//  返回参数      1,表示路径跟踪完成 1表示路径跟踪进行中
//  使用示例      定时中断中20-50ms调用一次 根据实际情况调整调用频率 过快可能导致计算资源浪费 过慢可能导致跟踪性能下降
//-------------------------------------------------------------------------------------------------------------------
int point_track=1;
void path_track_loop(void)
{
    if(path_track_start_flag==0xFF) return;//路径跟踪循环标志位 需要在主函数中设置为0才能开启路径跟踪循环
    if(path_track_points_struct.point_num < 3)
    {
        set_target_speed(0);
        set_target_steering_angle(0);
        path_track_stop();
        return;
    }
    //路径跟踪初始化函数 目前无特殊初始化需求 可以根据实际情况添加
    get_current_measurement();//更新状态量
    //计算并输出前轮转角
    set_target_steering_angle(pure_path_track_get_steering(&path_track_points_struct) * RtA); // 将计算得到的前轮转角(弧度)转换为度

    point_track = (int)search_index;
    if(search_index >= (path_track_points_struct.point_num - 2U))
    {
        set_target_speed(0);
        set_target_steering_angle(0);
        path_track_stop();
    }
    
    // point_track++;
    // if(point_track>point_num)//点追踪完后track_satate=1，路径跟踪结束
    // {
    //     track_satate=1;
    //     path_track_stop();
    // }
}
