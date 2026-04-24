/*********************************************************************************************************************
 * 文件名称          mean_shift_tracker.c
 * 功能简介          基于 Mean Shift 算法的轻量级彩色目标跟踪模块，适用于 SCC8660 彩色摄像头
 *
 * 使用示例：
 *   // 1. 初始化时，在目标中心调用 mst_init_template
 *   mst_init_template(80, 60, 40, 40);  // 目标中心 (80,60)，窗口 40×40
 *
 *   // 2. 每帧调用 mst_track 获取最新位置
 *   if(mst_track(&mst_result))
 *   {
 *       // mst_result.cx / mst_result.cy 为目标中心
 *       // mst_result.confidence 为置信度 [0-100]
 *   }
 ********************************************************************************************************************/

#include "mean_shift_tracker.h"

//================================================ 内部宏定义 ================================================
#define SWAPBYTE16(h)   ((((uint16)(h) << 8) & 0xFF00U) | ((uint16)(h) >> 8))

// 置信度低于此门限时认为目标丢失（Bhattacharyya 系数 * 100 < 40 即置信度 < 40）
#define MST_MIN_CONFIDENCE  (40)
//================================================ 内部宏定义 ================================================


//================================================ 全局变量 ================================================
mst_state_struct  mst_state  = {0};
mst_result_struct mst_result = {0};

// 目标模板直方图（归一化到 [0, 255]）
static uint8 tpl_hist[MST_HIST_BINS];
//================================================ 全局变量 ================================================


//================================================ 内部辅助函数 ================================================

// 从 RGB565 像素（已字节交换）提取 Hue bin 索引 [0, MST_HIST_BINS)
// 低饱和度像素（近灰色）返回 MST_HIST_BINS，调用方应跳过该像素
static uint8 rgb565_to_hue_bin(uint16 pixel)
{
    uint8 r, g, b;
    uint8 maxv, minv, diff;
    int16 h;

    r = (uint8)(((pixel >> 11) & 0x1FU) << 3);
    g = (uint8)(((pixel >>  5) & 0x3FU) << 2);
    b = (uint8)(( pixel        & 0x1FU) << 3);

    maxv = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    minv = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    diff = maxv - minv;

    // 饱和度过低（灰色），色相无意义，返回 MST_HIST_BINS 作为无效标志
    if(diff < 16)
    {
        return MST_HIST_BINS;
    }

    if(maxv == r)
    {
        // H in [0, 60) or (300, 360]
        h = (int16)((int16)(g - b) * 60 / diff);
        if(h < 0) h += 360;
    }
    else if(maxv == g)
    {
        // H in [60, 180)
        h = (int16)((int16)(b - r) * 60 / diff + 120);
    }
    else
    {
        // H in [180, 300)
        h = (int16)((int16)(r - g) * 60 / diff + 240);
    }

    // 限制范围后映射到 [0, MST_HIST_BINS)
    if(h < 0)   h = 0;
    if(h >= 360) h = 359;

    return (uint8)((uint16)h * MST_HIST_BINS / 360);
}

// 整数平方根（用于 Bhattacharyya 系数计算）
static uint32 isqrt32(uint32 n)
{
    uint32 x, x1;
    if(n == 0) return 0;
    x = n;
    x1 = (x + 1) >> 1;
    while(x1 < x)
    {
        x  = x1;
        x1 = (x + n / x) >> 1;
    }
    return x;
}

// 提取窗口内的色相直方图（归一化到 [0, 255]）
static void extract_hist(uint16 cx, uint16 cy, uint16 win_w, uint16 win_h, uint8 *hist)
{
    uint16 r, c;
    uint16 x0, y0, x1, y1;
    uint32 raw[MST_HIST_BINS];
    uint32 total;
    uint8  bin;
    uint16 pixel;

    // 计算窗口边界（限制在图像内）
    x0 = (cx > win_w / 2)                    ? (cx - win_w / 2) : 0;
    y0 = (cy > win_h / 2)                    ? (cy - win_h / 2) : 0;
    x1 = (cx + win_w / 2 < SCC8660_W - 1)   ? (cx + win_w / 2) : (SCC8660_W - 1);
    y1 = (cy + win_h / 2 < SCC8660_H - 1)   ? (cy + win_h / 2) : (SCC8660_H - 1);

    for(bin = 0; bin < MST_HIST_BINS; bin++)
    {
        raw[bin] = 0;
    }
    total = 0;

    for(r = y0; r <= y1; r++)
    {
        for(c = x0; c <= x1; c++)
        {
            pixel = SWAPBYTE16(scc8660_image[r][c]);
            bin   = rgb565_to_hue_bin(pixel);
            if(bin < MST_HIST_BINS)
            {
                raw[bin]++;
                total++;
            }
            // 低饱和度像素（bin == MST_HIST_BINS）不计入直方图
        }
    }

    if(total == 0) return;

    // 归一化到 [0, 255]
    for(bin = 0; bin < MST_HIST_BINS; bin++)
    {
        hist[bin] = (uint8)(raw[bin] * 255 / total);
    }
}

// 计算 Bhattacharyya 系数（衡量两个直方图相似度，结果 * 100 即为置信度）
// BC = sum(sqrt(p_i * q_i))，值域 [0, 1]，1 表示完全相同
static uint8 bhattacharyya(const uint8 *hist_a, const uint8 *hist_b)
{
    uint32 bc_scaled;   // BC * 256
    uint8  bin;

    bc_scaled = 0;
    for(bin = 0; bin < MST_HIST_BINS; bin++)
    {
        // sqrt(a * b)，其中 a, b 已归一化到 [0, 255]
        // isqrt32(a * b) 的结果最大约 255，累加 32 次最大约 8160
        bc_scaled += isqrt32((uint32)hist_a[bin] * hist_b[bin]);
    }

    // 归一化：最大可能值 = 32 * sqrt(255 * 255) = 32 * 255 = 8160
    // 返回置信度 [0, 100]
    return (uint8)(bc_scaled * 100 / 8160);
}

//================================================ 对外接口实现 ================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化目标模板（提取目标区域颜色直方图）
// 参数说明     cx, cy     目标中心坐标
// 参数说明     win_w      窗口宽度
// 参数说明     win_h      窗口高度
// 使用示例     mst_init_template(80, 60, 40, 40);
//-------------------------------------------------------------------------------------------------------------------
void mst_init_template(uint16 cx, uint16 cy, uint16 win_w, uint16 win_h)
{
    mst_state.cx    = cx;
    mst_state.cy    = cy;
    mst_state.win_w = win_w;
    mst_state.win_h = win_h;
    mst_state.valid = 1;

    extract_hist(cx, cy, win_w, win_h, tpl_hist);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     重置跟踪器
// 使用示例     mst_reset();
//-------------------------------------------------------------------------------------------------------------------
void mst_reset(void)
{
    uint8 bin;
    mst_state.valid = 0;
    mst_state.cx    = 0;
    mst_state.cy    = 0;
    for(bin = 0; bin < MST_HIST_BINS; bin++)
    {
        tpl_hist[bin] = 0;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     执行 Mean Shift 跟踪（每帧调用一次）
// 参数说明     result     输出结果指针
// 返回参数     uint8      1 表示跟踪成功，0 表示目标丢失
// 使用示例     mst_track(&mst_result);
// 备注说明     每帧执行最多 MST_MAX_ITER 次迭代，典型 3-5 次收敛
//-------------------------------------------------------------------------------------------------------------------
uint8 mst_track(mst_result_struct *result)
{
    uint16 cx, cy;
    uint16 win_w, win_h;
    uint8  iter;
    uint16 r, c;
    uint16 x0, y0, x1, y1;
    uint32 sum_x, sum_y, weight_sum;
    uint32 w;
    uint8  cand_hist[MST_HIST_BINS];
    uint8  bin;
    uint8  confidence;
    uint16 pixel;
    uint16 new_cx, new_cy;
    int16  shift_x, shift_y;

    if(!mst_state.valid)
    {
        return 0;
    }

    cx    = mst_state.cx;
    cy    = mst_state.cy;
    win_w = mst_state.win_w;
    win_h = mst_state.win_h;

    for(iter = 0; iter < MST_MAX_ITER; iter++)
    {
        // 当前窗口边界
        x0 = (cx > win_w / 2)                    ? (cx - win_w / 2) : 0;
        y0 = (cy > win_h / 2)                    ? (cy - win_h / 2) : 0;
        x1 = (cx + win_w / 2 < SCC8660_W - 1)   ? (cx + win_w / 2) : (SCC8660_W - 1);
        y1 = (cy + win_h / 2 < SCC8660_H - 1)   ? (cy + win_h / 2) : (SCC8660_H - 1);

        // 提取候选直方图
        extract_hist(cx, cy, win_w, win_h, cand_hist);

        sum_x      = 0;
        sum_y      = 0;
        weight_sum = 0;

        // 对窗口内每个像素计算权重：sqrt(q_h(bin) / p_h(bin))，其中 p 为候选，q 为模板
        // 简化为：w_i = tpl_hist[bin_i]（直接用模板直方图对应 bin 作为权重）
        for(r = y0; r <= y1; r++)
        {
            for(c = x0; c <= x1; c++)
            {
                pixel = SWAPBYTE16(scc8660_image[r][c]);
                bin   = rgb565_to_hue_bin(pixel);

                // 跳过低饱和度像素（bin == MST_HIST_BINS 为灰色，色相无意义）
                if(bin >= MST_HIST_BINS)
                {
                    continue;
                }

                // 权重 = sqrt(模板概率 * 候选概率)（Bhattacharyya 权重）
                w = isqrt32((uint32)tpl_hist[bin] * cand_hist[bin]);

                sum_x      += (uint32)c * w;
                sum_y      += (uint32)r * w;
                weight_sum += w;
            }
        }

        if(weight_sum == 0)
        {
            // 窗口内无有效像素
            break;
        }

        new_cx = (uint16)(sum_x / weight_sum);
        new_cy = (uint16)(sum_y / weight_sum);

        // 计算位移
        shift_x = (int16)new_cx - (int16)cx;
        shift_y = (int16)new_cy - (int16)cy;
        if(shift_x < 0) shift_x = -shift_x;
        if(shift_y < 0) shift_y = -shift_y;

        cx = new_cx;
        cy = new_cy;

        // 收敛判断
        if((shift_x + shift_y) <= MST_CONV_THRESH)
        {
            break;
        }
    }

    // 计算最终置信度
    extract_hist(cx, cy, win_w, win_h, cand_hist);
    confidence = bhattacharyya(tpl_hist, cand_hist);

    // 更新状态
    mst_state.cx = cx;
    mst_state.cy = cy;

    result->cx         = cx;
    result->cy         = cy;
    result->win_w      = win_w;
    result->win_h      = win_h;
    result->confidence = confidence;

    if(confidence < MST_MIN_CONFIDENCE)
    {
        return 0;
    }

    return 1;
}
