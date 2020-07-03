#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <curl/curl.h>
#include <curl/mprintf.h>

#include "utils.h"

static int open_mp3_fifo(const char* fifo_name)
{
    (void*)fifo_name;
    const char *mp3_fifo = "/tmp/my_mp3_fifo";
    int fd = -1; 
    if(access(mp3_fifo, F_OK) == -1)
    {
        int result = mkfifo(mp3_fifo, 0777);
        if(result != 0)
        {
            printf("could not create fifo %s\n", mp3_fifo);
            return -1;
        }
    }
    fd = open(mp3_fifo, O_WRONLY);

    return fd;
}

int iflyos_send_mp3_voice(const char *url)
{
    CURL *curl_handle;
    CURLcode res;

    if(NULL == url)
    {
        return -1;
    }

    PostMemCb chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    printf("mp3 url is %s\n", url);
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);
    //curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_cb); 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haoxuetong/1.0.0");
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
    
    res = curl_easy_perform(curl_handle);
    if(res != CURLE_OK) 
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    }
    else 
    {
        printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        int fd = open_mp3_fifo(NULL); 
        int count = 0, remain_count = chunk.size;
        char* ptr = chunk.memory;
        while( remain_count > 0)
        {
            count = write(fd, ptr, 640);   
            remain_count -= count; 
            ptr += count;      
        }
    }

    curl_easy_cleanup(curl_handle);

    free(chunk.memory);

    curl_global_cleanup();

    return 0;
}

