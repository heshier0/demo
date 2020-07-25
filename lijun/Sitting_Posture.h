#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

void* Sitting_Posture_init(const char *model_path1,const char *model_path2);
/* 
model_path1：/user/lj/person-coco_50000_bgr.wk
model_path2： /user/lj/class2_rgb.wk
*/
int Sitting_Posture_run(void* pHandle,unsigned char *src_data,int Width,int Height,int level);
/* 

src_data YUV420p数据
level //报警时间单位秒
Sitting_Posture_run返回结果 
-1程序异常
0端正
1非端正
2离座
*/
int Sitting_Posture_unit(void** pHandle);
