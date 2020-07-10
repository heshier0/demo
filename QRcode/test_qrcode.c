#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "QRcode_Detection.h"

int main(int argc, char **argv)
{
    char *ptr = NULL;

  	FILE *yuvFile;
	yuvFile = fopen(argv[1], "rb");
	if (NULL == yuvFile)
	{
		printf("Can not open file \n");
		return 1;
	}
	
	int inwidth = atoi(argv[2]);
	int inheight = atoi(argv[3]);

	int idx = 0;
	int size = inwidth * inheight;

	int u32Ret;
	char *buffer = (char *)malloc(size*3/2 * sizeof(char));	

    u32Ret = fread(buffer, size*3/2, 1, yuvFile);
    if (1 != u32Ret)
    {
        printf("YUVfile end\n");
        return -1;
    }

    QRcode_Detection(buffer, inwidth, inheight, (void*)&ptr);

    if (ptr != NULL)
    {
        printf("%s\n", ptr);

        free(ptr);
        ptr = NULL;
    }
    
    return 0;
}


