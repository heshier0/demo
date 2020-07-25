#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "media.h"

#ifdef __cplusplus
#if __cplusplus
{
#endif
#endif /*  __cplusplus  */

int app_quit = 0;

void sig_handler(int num)
{
    printf("recv num:%d \n", num);  
    app_quit = 1;
}

static void print_usage(void)
{
	printf("./mtt_rec -w [width] -h [height] -f [frame rate] -b [bit rate] -r [rotate angle]");
}

/* 264 ±àÂë²Î¿¼·Ö±æÂÊ
   CIF
   512;

   D1
   1024 * 2;
   
   720P
   1024 * 3 + 1024*u32FrameRate/30;

   1080P
   1024 * 2 + 2048*u32FrameRate/30;
*/
int main(int argc, char ** argv)
{
	int ret;
	char ch;
	int rotate = 0;
	int width = 1920;
	int height = 1080;
	int frame_rate = 25;
	int bit_rate = 4096;
	
    int opt;

    while ((opt = getopt(argc, argv, "w:h:f:b:r:")) != -1) 
    {
        switch (opt) {
        case 'w':
            width = atoi(optarg);
            break;
            
        case 'h':
            height = atoi(optarg);
            break;
            
        case 'f':
        	frame_rate = atoi(optarg);
        	break;
        	
        case 'b':
        	bit_rate = atoi(optarg);
        	break;
        	
        case 'r':
        	rotate = atoi(optarg);
        	break;	
        	
        default: /* '?' */
            print_usage();
            exit(EXIT_FAILURE);
        }
    }	
	
	signal(SIGINT,  sig_handler);
	signal(SIGTERM, sig_handler);

	ret = MEDIA_Start(width, height, frame_rate, bit_rate, rotate);
	if (ret < 0)
	{
		printf("media start failed\n");
		return 0;
	}
	
	while (!app_quit)
	{
		printf("press '0' or '1' key to snap one pic, 'q' to exit\n");
		ch = getchar();
	
		if (ch == 'q')
		{	
			printf("program will quit\n");
			break;
		}	
			
		if (ch == '0' || ch == '1')	
	    {
            ret = MEDIA_Snap(ch - '0');
            if (ret == 0)
            	printf("snap sucess\n");
            else
            	printf("snap failed\n");
        }
	}
	
	MEDIA_Stop();
	
	system("sync");
	system("chmod a+rw *");
	
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
