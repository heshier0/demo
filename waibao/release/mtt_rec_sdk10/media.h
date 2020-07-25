#ifndef INC_MEDIA_H
#define INC_MEDIA_H

/**
 * @brief           媒体处理模块启动
 * @s32Width        目标录像分辨率宽
 * @s32Height       目标录像分辨率高
 * @s32FrameRate    帧率,默认为25  
 * @s32Bitrate      码率(请自行根据分辨率选择合适码率,参考comm/sample_comm_venc.c相近分辨率) 
 * @s32Rotate       旋转角度(0, 90, 180, 270);          
 * @return          返回NULL 指针
 */
int MEDIA_Start(int s32Width, int s32Height, int s32FrameRate, int s32Bitrate, int s32Rotate);

/**
 * @brief           媒体处理模块停止
 * @return          无
 */
void MEDIA_Stop(void);

/**
 * @brief           启动一次jpeg抓拍
 * @ch              要抓拍的通道(0 彩色, 1 红外)
 * @return          成功或失败
 */
int MEDIA_Snap(int ch);

#endif
