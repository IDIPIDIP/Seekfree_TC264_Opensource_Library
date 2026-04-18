
#include "zf_common_headfile.h"

#ifndef CODE_PATH_TRACK_H_
#define CODE_PATH_TRACK_H_

extern int track_satate;//路径跟踪状态 1为结束 0为未完成
extern Trajectory2DPoint_struct path_track_points_struct; // 路径点结构体 当前寻迹路径
extern int point_track; // 当前跟踪的路径点索引
//定时中断中调用
void get_current_measurement(void);//获取当前测量值 更新状态量（已经封装在path_track_loop中）
float pure_path_track_get_steering(Trajectory2DPoint_struct *path_track_struct);//路径跟踪主循环函数（已封装在path_track_loop中）
void path_track_loop(void);//路径跟踪主循环函数 需要放在定时中断函数中 定时调用

//主程序中调用
void path_track_init(void); //仅负责开启定时中断
void path_track_start(void);//开启路径跟踪循环 (设置中断中的路径追踪循环开启)
void path_track_stop(void);//停止路径跟踪循环 (设置中断中的路径追踪循环停止)
void path_track_set_mode(uint8 mode);//设置路径跟踪模式 0为正向路径追踪 1为倒车路径追踪
void path_track_load_path(Trajectory2DPoint_struct *path_struct);//路径加载函数 将路径点数据加载到全局变量中 供路径跟踪算法使用

#endif /* CODE_PATH_TRACK_H_ */ 

