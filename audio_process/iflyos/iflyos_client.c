#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <curl/curl.h>
#include <curl/mprintf.h>

static char* get_download_filename(const char *url)
{
    char * filename = NULL;
    if (NULL == url)
    {
        return NULL;
    }

    char *start = NULL;
    char *end = NULL;
    int with_token= 0;

    if ((start = strrchr(url, '\/')) == NULL)
    {
        return NULL;
    }
    start ++;
    
    if ((end = strrchr(url, '?')) == NULL)
    {
        int length = (int)(url+strlen(url) - start);
        filename = (char*)calloc(length+1, sizeof(char));
        memcpy(filename, start, (url+strlen(url) - start));
    }
    else 
    {
        int length = (int)(end -start);
        filename = (char*)calloc(length+1, sizeof(char));
        memcpy(filename, start, (end - start));
    }
    int count = strlen(filename);
    filename[strlen(filename)] = '\0';

    return filename;
}

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

struct MemoryStruct 
{
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) 
    {
    /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int iflyos_download_file(const char *url)
{
    CURL *http_handle;
    CURLM *multi_handle;
 
    int still_running = 0; /* keep number of running handles */ 
    int repeats = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    http_handle = curl_easy_init();
    
    char * filename = get_download_filename(url);
    if(filename == NULL)
    {
        return -1;
    }

    FILE *out = fopen(filename, "wb");
    curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, out); 
    curl_easy_setopt(http_handle, CURLOPT_URL, url);
    
    multi_handle = curl_multi_init();
    curl_multi_add_handle(multi_handle, http_handle);
    curl_multi_perform(multi_handle, &still_running);
 
    while(still_running)
    {
        CURLMcode mc; /* curl_multi_wait() return code */ 
        int numfds;
    
        /* wait for activity, timeout or "nothing" */ 
        mc = curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);
    
        if(mc != CURLM_OK) 
        {
            fprintf(stderr, "curl_multi_wait() failed, code %d.\n", mc);
            break;
        }
    
        if(!numfds) 
        {
            repeats++; /* count number of repeated zero numfds */ 
            if(repeats > 1) 
            {
                usleep(100); /* sleep 100 milliseconds */ 
            }
        }
        else
            repeats = 0;
    
        curl_multi_perform(multi_handle, &still_running);
    }
 
    curl_multi_remove_handle(multi_handle, http_handle);
    curl_easy_cleanup(http_handle);
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();

    if(filename != NULL)
    {
        free(filename);
        filename = NULL;
    }

    if(out != NULL)
    {
        fclose(out);
        out = NULL;
    }
    
    return 0;
}

int iflyos_send_mp3_voice(const char *url)
{
    CURL *curl_handle;
    CURLcode res;

    if(NULL == url)
    {
        return -1;
    }

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haoxuetong/1.0.0");

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

int iflyos_post_data(const char *url, const char* data)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return 0;
}