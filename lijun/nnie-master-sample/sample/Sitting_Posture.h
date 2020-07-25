#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

void* Sitting_Posture_init(const char *model_path1,const char *model_path2);

int Sitting_Posture_run(void* pHandle,unsigned char *src_data,int Width,int Height,int level);

int Sitting_Posture_unit(void** pHandle);
