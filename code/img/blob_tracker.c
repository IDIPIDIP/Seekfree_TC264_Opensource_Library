/*********************************************************************************************************************
 * 文件名称          blob_tracker.c
 * 功能简介          基于灰度图像的轻量级目标跟踪模块，适用于 MT9V03X 灰度摄像头
 *
 * 使用示例：
 *   uint8 threshold = blob_calc_otsu_threshold(); // 自动计算阈值
 *   if(blob_track(threshold, BLOB_DARK, &blob_result))
 *   {
 *       // blob_result.cx / blob_result.cy 为目标质心坐标
 *       // blob_result.area 为目标面积（像素数）
 *   }
 ********************************************************************************************************************/

#include "blob_tracker.h"

//================================================ 全局变量 ================================================
blob_result_struct blob_result = {0};

// 跟踪器内部状态
static uint8  track_valid = 0;                          // 上一帧是否成功跟踪
static uint16 roi_x_min   = 0;                          // ROI 左边界
static uint16 roi_x_max   = MT9V03X_W - 1;              // ROI 右边界
static uint16 roi_y_min   = 0;                          // ROI 上边界
static uint16 roi_y_max   = MT9V03X_H - 1;              // ROI 下边界
//================================================ 全局变量 ================================================


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     大津法（Otsu）自动二值化阈值计算
// 参数说明     无（直接读取 mt9v03x_image 全局数组）
// 返回参数     uint8   最优阈值 [0-255]
// 使用示例     uint8 thr = blob_calc_otsu_threshold();
// 备注说明     纯整数运算，TC264 上执行约 < 1 ms
//-------------------------------------------------------------------------------------------------------------------
uint8 blob_calc_otsu_threshold(void)
{
    uint32 hist[256];
    uint32 total;
    uint32 sum_all;
    uint32 sum_b;
    uint32 w_b, w_f;
    uint32 mean_b, mean_f;
    uint64 var_between;
    uint64 var_max;
    uint8  threshold;
    uint16 i, r, c;
    uint32 diff;

    // 初始化直方图
    for(i = 0; i < 256; i++)
    {
        hist[i] = 0;
    }

    total   = (uint32)MT9V03X_W * MT9V03X_H;
    sum_all = 0;
    sum_b   = 0;
    w_b     = 0;
    var_max = 0;
    threshold = 128;

    // 统计灰度直方图
    for(r = 0; r < MT9V03X_H; r++)
    {
        for(c = 0; c < MT9V03X_W; c++)
        {
            hist[mt9v03x_image[r][c]]++;
        }
    }

    // 计算全图灰度加权和
    for(i = 0; i < 256; i++)
    {
        sum_all += (uint32)i * hist[i];
    }

    // 遍历所有阈值，寻找类间方差最大的分割点
    for(i = 0; i < 256; i++)
    {
        w_b += hist[i];
        if(w_b == 0)
        {
            continue;
        }

        w_f = total - w_b;
        if(w_f == 0)
        {
            break;
        }

        sum_b += (uint32)i * hist[i];

        mean_b = sum_b / w_b;
        mean_f = (sum_all - sum_b) / w_f;

        // 类间方差 = w_b * w_f * (mean_b - mean_f)^2
        // 使用 uint64 防止溢出
        diff       = (mean_b >= mean_f) ? (mean_b - mean_f) : (mean_f - mean_b);
        var_between = (uint64)w_b * w_f * diff * diff;

        if(var_between > var_max)
        {
            var_max   = var_between;
            threshold = (uint8)i;
        }
    }

    return threshold;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     重置跟踪器状态（下一帧将全图搜索）
// 参数说明     无
// 返回参数     无
// 使用示例     blob_tracker_reset();
//-------------------------------------------------------------------------------------------------------------------
void blob_tracker_reset(void)
{
    track_valid = 0;
    roi_x_min   = 0;
    roi_x_max   = MT9V03X_W - 1;
    roi_y_min   = 0;
    roi_y_max   = MT9V03X_H - 1;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     灰度目标跟踪（质心法）
// 参数说明     threshold    二值化阈值，0-255
// 参数说明     polarity     BLOB_DARK 跟踪暗目标 / BLOB_BRIGHT 跟踪亮目标
// 参数说明     result       输出结果指针
// 返回参数     uint8        1 表示跟踪成功，0 表示目标丢失
// 使用示例     blob_track(blob_calc_otsu_threshold(), BLOB_DARK, &blob_result);
// 备注说明     跟踪成功后自动缩小 ROI；目标丢失后自动回退到全图搜索
//-------------------------------------------------------------------------------------------------------------------
uint8 blob_track(uint8 threshold, uint8 polarity, blob_result_struct *result)
{
    uint32 sum_x, sum_y, count;
    uint16 r, c;
    uint16 x_min, x_max, y_min, y_max;
    uint8  match;
    uint8  full_search;

    // 先尝试在 ROI 内搜索，失败后尝试全图
    for(full_search = 0; full_search <= 1; full_search++)
    {
        if(full_search)
        {
            // 第一次 ROI 搜索失败，扩展到全图
            roi_x_min = 0;
            roi_x_max = MT9V03X_W - 1;
            roi_y_min = 0;
            roi_y_max = MT9V03X_H - 1;
        }

        sum_x = 0;
        sum_y = 0;
        count = 0;
        x_min = MT9V03X_W - 1;
        x_max = 0;
        y_min = MT9V03X_H - 1;
        y_max = 0;

        for(r = roi_y_min; r <= roi_y_max; r++)
        {
            for(c = roi_x_min; c <= roi_x_max; c++)
            {
                if(polarity == BLOB_BRIGHT)
                {
                    match = (mt9v03x_image[r][c] >= threshold) ? 1U : 0U;
                }
                else
                {
                    match = (mt9v03x_image[r][c] < threshold)  ? 1U : 0U;
                }

                if(match)
                {
                    sum_x += c;
                    sum_y += r;
                    count++;
                    if(c < x_min) x_min = c;
                    if(c > x_max) x_max = c;
                    if(r < y_min) y_min = r;
                    if(r > y_max) y_max = r;
                }
            }
        }

        if(count >= BLOB_MIN_PIXEL_COUNT)
        {
            break;  // 找到有效目标
        }

        // ROI 搜索已经失败，且 track_valid 为 0（上帧全图也没有目标），直接返回
        if(!track_valid)
        {
            track_valid = 0;
            return 0;
        }

        // ROI 搜索失败但上帧有目标，继续全图搜索（full_search = 1）
        track_valid = 0;
    }

    // 确认找到有效目标后填充结果
    if(count < BLOB_MIN_PIXEL_COUNT)
    {
        track_valid = 0;
        return 0;
    }

    result->cx    = (uint16)(sum_x / count);
    result->cy    = (uint16)(sum_y / count);
    result->x_min = x_min;
    result->x_max = x_max;
    result->y_min = y_min;
    result->y_max = y_max;
    result->area  = count;

    // 更新下一帧 ROI（包围框 + 搜索余量，限制在图像边界内）
    roi_x_min = (x_min > BLOB_SEARCH_MARGIN)                   ? (x_min - BLOB_SEARCH_MARGIN) : 0;
    roi_x_max = (x_max + BLOB_SEARCH_MARGIN < MT9V03X_W - 1)   ? (x_max + BLOB_SEARCH_MARGIN) : (MT9V03X_W - 1);
    roi_y_min = (y_min > BLOB_SEARCH_MARGIN)                   ? (y_min - BLOB_SEARCH_MARGIN) : 0;
    roi_y_max = (y_max + BLOB_SEARCH_MARGIN < MT9V03X_H - 1)   ? (y_max + BLOB_SEARCH_MARGIN) : (MT9V03X_H - 1);

    track_valid = 1;
    return 1;
}
