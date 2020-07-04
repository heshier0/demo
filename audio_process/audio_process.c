
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "utils.h"

int main(int argc, char **argv)
{

    hxt_load_cfg();
    
    if (hxt_get_token_cfg() == NULL)
    {
        utils_print("send request to get token\n");
        hxt_get_token_request();
    }
    
    hxt_websocket_start();
    // hxt_file_upload("/user/media/1111.mp3");
    hxt_unload_cfg();


    return 0;
}
