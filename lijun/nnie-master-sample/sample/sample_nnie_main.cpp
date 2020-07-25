#include "Sitting_Posture.h"


int main(int argc, char *argv[])
{
    
    const char *Detect_model_path = argv[1];
    const char *Class_model_path = argv[2];
    void* pHandle= NULL;
    pHandle=Sitting_Posture_init(Detect_model_path,Class_model_path);
    //pHandle=Posture_Classify_init(Class_model_path);
    //pHandle=Person_Detection_init(Detect_model_path);
    FILE *yuvFile;
    struct timeval tv1;
    struct timeval tv2;
    long t1, t2, time;

    yuvFile = fopen(argv[3],"rb");
    if(NULL == yuvFile)
    {
        printf("Can not open file vfd.yuv\n");
        return 1;
    }
    unsigned char* buffer;

    printf("after init\n");

    int inwidth = 0;
    int inheight = 0;
    sscanf( argv[4],"%d",&inwidth);
    sscanf( argv[5],"%d",&inheight);
    int idx = 0;
    int size = inwidth*inheight;
    int u32Ret;
    buffer=(unsigned char *)malloc(size*1.5*sizeof(unsigned char));
    for(;;)
    {
        u32Ret = fread(buffer,size*1.5,1,yuvFile);
        if (1!=u32Ret)            
        {            
            printf("YUVfile end\n");
            break;
        }
        //Posture_Classify_run(pHandle, buffer,inwidth,inheight,0);
        //Person_Detection_run(pHandle, buffer,inwidth,inheight,0);

	gettimeofday(&tv1, NULL);
	int resultlab=Sitting_Posture_run(pHandle, buffer,inwidth,inheight,3);
	printf("Posture_state:%d\n",resultlab);
	gettimeofday(&tv2, NULL);
	t1 = tv2.tv_sec - tv1.tv_sec;
	t2 = tv2.tv_usec - tv1.tv_usec;
	time = (long)(t1 * 1000 + t2 / 1000);
	printf("Sitting_Posture inference time : %dms\n", time);

    }
    Sitting_Posture_unit(&pHandle);
    return 1;
}
