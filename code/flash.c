#include "zf_common_headfile.h"
#include "zf_common_typedef.h"
/*********************************************************************************************************************
*功能名称       flash 存点
*引脚定义
*硬件参数
*使用说明       分成两部分 1. 存储目录 2.数据存储部分
*使用说明       第一页记录存储对象和存储地址
*使用说明       第一页第一个位置为
*使用说明       从第一页的第二个位置写入结构体
*使用说明
*使用说明       2~12页写入结构体
*使用说明       写入读取结构体
*使用说明       由于采集点类型较少，可以手动选择初始位置和采集数量
*使用说明       避免顺序存储造成的算法复杂度过高
*********************************************************************************************************************/
uint8 flash_storage_flag=1;//存储标志位 0表示flash中有数据 1表示flash中没有数据
flash_category_info flash_category[CATAGORY_NUM_MAX]; // 存储目录

uint16 category_num=0;//存储目录数量
Trajectory2DPoint_struct buffer_point; // 存储点的缓存区

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    将flash中目录提取出来 获得存点状态
//  参数说明    void
//  参数说明
//  返回参数    void
//  使用说明    从flash中读取存储目录信息 存储在flash_category数组中 以便后续操作
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void flash_get_category(void)
{
    int8 i=0;
    flash_read_page_to_buffer(0,1);
    category_num=flash_union_buffer[0].uint16_type;
    if(category_num ==0) return;
    //取点录信息 存储在flash_category数组中 以便后续操作
    for(i=0;i<category_num;i++)
    {
        flash_category[i].flash_ID=flash_union_buffer[FLASH_STRUCT_SIZE*i+1].uint32_type;
        flash_category[i].flash_page=flash_union_buffer[FLASH_STRUCT_SIZE*i+2].uint16_type;
        flash_category[i].first_index=flash_union_buffer[FLASH_STRUCT_SIZE*i+3].uint16_type;
        flash_category[i].end_index=flash_union_buffer[FLASH_STRUCT_SIZE*i+4].uint16_type;
        flash_category[i].storage_num=flash_union_buffer[FLASH_STRUCT_SIZE*i+5].uint16_type;
        flash_category[i].max_storage_num=flash_union_buffer[FLASH_STRUCT_SIZE*i+6].uint16_type;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    搜索ID所在数组中位置
//  参数说明    ID：需要搜索的ID
//  参数说明
//  返回参数    ID所在结构体位置 0~category_num-1 0xFF表示未找到
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_find_ID(flash_id_enum ID)
{
    uint8 i=0;
    for(i=0;i<category_num;i++)
    {
        if(flash_category[i].flash_ID == ID)
        {
            return i;//返回ID所在结构体中位置
        }
    }
    return 0xFF; //返回0xFF表示未找到  不存在和没找到都返回0xFF
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    缓存目录结构体函数 将一个目录结构体写入缓冲区以便写入flash
//  参数说明    flash_category_struct：需要写入的目录结构体
//  参数说明    ID_index：需要存入目录的位置
//  返回参数
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void flash_write_category_struct_tobuffer(flash_category_info flash_category_struct, uint16 ID_index)
{
    flash_union_buffer[0].uint16_type=category_num;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+1].uint32_type=flash_category_struct.flash_ID;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+2].uint16_type=flash_category_struct.flash_page;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+3].uint16_type=flash_category_struct.first_index;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+4].uint16_type=flash_category_struct.end_index;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+5].uint16_type=flash_category_struct.storage_num;
    flash_union_buffer[FLASH_STRUCT_SIZE*ID_index+6].uint16_type=flash_category_struct.max_storage_num;
    
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    flash索引函数 根据结构体ID获得存储位置等信息
//  参数说明    flash_category_struct：需要索引位置的目录结构体 根据ID索引
//  参数说明
//  返回参数
//  使用说明    科一 page: 2
//  使用说明    科一    process: 0 -603 300个点
//  使用说明    科一    end:700 -1003 150个点
//  使用说明    科二 page: 3
//  使用说明    科二    begin: 0 -83 40个点
//  使用说明    科二    door1: 90 -193 50个点
//  使用说明    科二    door2: 200 -303 50个点
//  使用说明    科二    door3: 320 -423 50个点
//  使用说明    科二    door4: 430-533 50个点
//  使用说明    科二    door5: 540 -643 50个点
//  使用说明    科二    end: 650 -753 50个点
//  使用说明    科三 page: 4
//  使用说明    科三    begin: 0 -103 50个点
//  使用说明    科三    process: 106 -908 400个点
//  使用说明    科三    end: 912 -1013 50个点
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void flash_get_allocation(flash_category_info *flash_category_struct)
{
    switch (flash_category_struct->flash_ID)
    {
        case SUBJECT_ONE_BEGIN:
            break;
        case SUBJECT_ONE_PROCESS:
            flash_category_struct->flash_page=2;
            flash_category_struct->first_index=0;
            flash_category_struct->max_storage_num=300;
            break;
        case SUBJECT_ONE_END:
            flash_category_struct->flash_page=2;
            flash_category_struct->first_index=700;
            flash_category_struct->max_storage_num=150;
            break;
        case SUBJECT_TWO_BEGIN:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=0;
            flash_category_struct->max_storage_num=40;
            break;
        case SUBJECT_TWO_DOOR_1:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=90;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_TWO_DOOR_2:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=200;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_TWO_DOOR_3:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=320;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_TWO_DOOR_4:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=430;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_TWO_DOOR_5:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=540;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_TWO_END:
            flash_category_struct->flash_page=3;
            flash_category_struct->first_index=650;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_THREE_BEGIN:
            flash_category_struct->flash_page=4;
            flash_category_struct->first_index=0;
            flash_category_struct->max_storage_num=50;
            break;
        case SUBJECT_THREE_PROCESS:
            flash_category_struct->flash_page=4;
            flash_category_struct->first_index=106;
            flash_category_struct->max_storage_num=400;
            break;
        case SUBJECT_THREE_END:
            flash_category_struct->flash_page=4;
            flash_category_struct->first_index=912;
            flash_category_struct->max_storage_num=50;
            break;
    }

}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    将所需存的点结构体写入页中
//  参数说明    flash_category_struct：需要写入的点结构体
//  参数说明    page：存入的点所在页
//  参数说明    ID_index：存入的收个点的位置
//  返回参数
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void write_point_toflash(Trajectory2DPoint_struct flash_point, uint16 page,uint16 index)
{
    uint16 i=0;
    uint16 point_num=0;//点数
    flash_buffer_clear();
    point_num=flash_point.point_num;
    flash_union_buffer[index].uint16_type=point_num;
    flash_union_buffer[index+1].uint8_type=flash_point.first_point_flag;//可以用于判断是否已有点存入
    flash_union_buffer[index+2].float_type=flash_point.x_offset;
    flash_union_buffer[index+3].float_type=flash_point.y_offset;
    for(i=0;i<point_num;i++)
    {
        flash_union_buffer[index+i*2+4].float_type=flash_point.x[i];
        flash_union_buffer[index+i*2+5].float_type=flash_point.y[i];
    }
    flash_write_page_from_buffer(0,page);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介    将点存入flash函数 自动识别点是否已存入目录中 如果有则覆盖 如果没有则写入目录中
//  参数说明    flash_point：需要存的一组点的信息 包含ID 点数等信息
//  参数说明
//  返回参数    void
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void flash_write_point_struct(Trajectory2DPoint_struct flash_point)
{
    uint8 i=0;
    flash_category_info flash_category_struct;
    uint16 ID_index=0;
    ID_index=flash_find_ID(flash_point.ID);
    if(flash_storage_flag==1&&ID_index==0xFF) //如果flash中没有数据 直接写入
    {
        ID_index=0;
        flash_category_struct.flash_ID=flash_point.ID;
        flash_get_allocation(&flash_category_struct);
        flash_category_struct.storage_num=flash_point.point_num * 2 + 4;
        flash_category_struct.end_index=flash_category_struct.first_index+flash_category_struct.max_storage_num;
        category_num++;
        flash_storage_flag=0;
    }
    else if(ID_index==0xFF)//如果ID不存在 则写入新目录
    {
        ID_index=category_num; //最后一个位置+1
        flash_category_struct.flash_ID=flash_point.ID;
        flash_get_allocation(&flash_category_struct);
        flash_category_struct.storage_num=flash_point.point_num * 2 + 4;
        flash_category_struct.end_index=flash_category_struct.first_index+flash_category_struct.max_storage_num;
        category_num++;
    }
    else //如果ID已存在 则覆盖原有目录信息
    {
        flash_category_struct.flash_ID=flash_point.ID;
        flash_get_allocation(&flash_category_struct);
        flash_category_struct.storage_num=flash_point.point_num * 2 + 4;
        flash_category_struct.end_index=flash_category_struct.first_index+flash_category_struct.max_storage_num;
    }
    flash_write_category_struct_tobuffer(flash_category_struct, ID_index*FLASH_STRUCT_SIZE);
    write_point_toflash(flash_point, flash_category_struct.flash_page, flash_category_struct.first_index);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    读点到缓存结构体中
//  参数说明    ID：需要读取的点的ID
//  参数说明
//  返回参数    uint8 0表示成功读取 1表示未找到ID
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_read_struct_toBuffer(flash_id_enum ID)
{
    uint16 ID_index=0;//ID所在目录位置
    uint16 page=0;//存储所在页码
    uint16 first_index=0;//存储首地址索引
    uint16 point_num=0;//点数
    flash_buffer_clear();
    ID_index=flash_find_ID(ID);
    if(ID_index==0xFF) return 1;//如果未找到ID 返回1
    else
    {
        page=flash_category[ID_index].flash_page;
        first_index=flash_category[ID_index].first_index;
        flash_read_page_to_buffer(0,page);
        //从缓冲区读取数据到buffer_point中
        buffer_point.point_num=flash_union_buffer[first_index].int16_type;
        buffer_point.first_point_flag=flash_union_buffer[first_index+1].int8_type;
        buffer_point.x_offset=flash_union_buffer[first_index+2].float_type;
        buffer_point.y_offset=flash_union_buffer[first_index+3].float_type;
        point_num=buffer_point.point_num;
        for(uint16 i=0;i<point_num;i++)
        {
            buffer_point.x[i]=flash_union_buffer[first_index+i*2+4].float_type;
            buffer_point.y[i]=flash_union_buffer[first_index+i*2+5].float_type;
        }
        return 0;//成功读取返回0
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    flash初始化函数 从flash中读取目录信息 判断是否有数据 存储在全局变量中
//  参数说明
//  参数说明
//  返回参数
//  使用说明    flash_storage_flag=0表示flash中有数据 1表示flash中没有数据
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
void flash_init(void)
{
    flash_get_category();
    if(category_num>0) flash_storage_flag=0;
    else flash_storage_flag=1;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介    flash清除函数 将flash中数据清空 包括目录和数据
//  参数说明
//  参数说明
//  返回参数
//  使用说明
//  使用示例
//-------------------------------------------------------------------------------------------------------------------
/*void flash_all_clear(void)
{
    
}*/
