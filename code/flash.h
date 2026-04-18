#ifndef _CODE_FLASH_H_
#define _CODE_FLASH_H_

#include "zf_common_typedef.h"
#include "zf_common_headfile.h"

#define CATAGORY_NUM_MAX 30 // 存储目录最大数量
#define FLASH_STRUCT_SIZE 6 // 每个结构体占用的存储单元数量 以uint32为单位 例如一个结构体有3个uint32类型的成员 则FLASH_STRUCT_SIZE为3


typedef enum               // 科目存点分类
{
    //科目一 ID
    SUBJECT_ONE_BEGIN,
    SUBJECT_ONE_PROCESS,
    SUBJECT_ONE_END,
    //科目二 ID
    SUBJECT_TWO_BEGIN,
    SUBJECT_TWO_DOOR_1,
    SUBJECT_TWO_DOOR_2,
    SUBJECT_TWO_DOOR_3,
    SUBJECT_TWO_DOOR_4,
    SUBJECT_TWO_DOOR_5,
    SUBJECT_TWO_END,
    //科目三 ID
    SUBJECT_THREE_BEGIN ,
    SUBJECT_THREE_PROCESS ,
    SUBJECT_THREE_END ,
}flash_id_enum;

typedef struct
{
    int a;
    flash_id_enum ID;
    uint16 point_num;//点数
    float x[INTEGRATE_MAX_POINTS]; //点的x坐标
    float y[INTEGRATE_MAX_POINTS]; //点的y坐标
    float first_point_flag; //是否已采集到第一个点 用于初始化基准点
    float x_offset; //笛卡尔坐标x误差
    float y_offset; //笛卡尔坐标y误差
}Trajectory2DPoint_struct;

typedef struct 
{
    flash_id_enum flash_ID;//不同结构体的 ID 用于区分不同的存储对象
    uint16 flash_page;//存储所在页码
    uint16 first_index;//存储首地址索引
    uint16 end_index;//尾地址索引
    uint16 storage_num;//存点结构体成员总数
    uint16 max_storage_num;//最大存储数量
}flash_category_info;

extern uint16 catagory_num;//存储目录数量
extern Trajectory2DPoint_struct buffer_point;//存储点的缓存区

void flash_init(void);//flash初始化函数 从flash中读取存储目录信息 存储在flash_category数组中 以便后续操作
void flash_write_point_struct(Trajectory2DPoint_struct flash_point);// 存点函数 将点结构体存入特定位置，自动识别ID
uint8 flash_read_struct_toBuffer(flash_id_enum ID); //读点函数 将指定点组存在 buffer_point结构体中

//ID   项目
//0 科目一 起点 过程 终点
//1-12 科目二 门1 门2 门3 门4 门5+返回 11-12 动作区

#endif /* _CODE_FLASH_H_ */
