#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <cJSON.h>
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

static char* separate_filename(const char *url)
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
        filename = (char*)utils_calloc(length+1);
        memcpy(filename, start, (url+strlen(url) - start));
    }
    else 
    {
        int length = (int)(end -start);
        filename = (char*)utils_calloc(length+1);
        memcpy(filename, start, (end - start));
    }
    int count = strlen(filename);
    filename[strlen(filename)] = '\0';

    return filename;
}

static size_t write_memory_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    PostMemCb *mem = (PostMemCb *)userp;

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

static size_t write_file_data(void *ptr, size_t size, size_t nmemb, void *stream) 
{
    if(NULL == stream)
    {
        utils_print("no file opened\n");
        return 0;
    }
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    utils_print("written %d bytes this turn\n", (int)written);
    return written;
}

cJSON* utils_load_cfg(const char* cfg)
{
    FILE *file = NULL;
    cJSON *root = NULL;
    long length = 0;
    char* content = NULL;
    size_t read_chars = 0;

    if(NULL==cfg)
    {
        return NULL;
    }

    //以2进制方式打开
    file = fopen(cfg, "rb");
    if (NULL == file)
    {
        goto CLEANUP;
    }
    //获取长度
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto CLEANUP;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto CLEANUP;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto CLEANUP;
    }
    //分配内存
    //content = (char*)malloc((size_t)length + sizeof(""));
    content = (char*)utils_malloc(length+1);
    if (NULL == content)
    {
        goto CLEANUP;
    }
    //读取文件
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        utils_free(content);
        content = NULL;
        goto CLEANUP;
    }
    content[read_chars] = '\0';
    root = cJSON_Parse(content);
    if(content != NULL)
    {
        utils_free(content);
        content = NULL;
    }

CLEANUP:
    if (file != NULL)
    {
        fclose(file);
    }    

    return root;
}

void utils_unload_cfg(cJSON* root)
{
    if (root != NULL)
    {
        cJSON_Delete(root);
    }

    return;
}

BOOL utils_reload_cfg(const char* cfg, cJSON* root)
{
    if (NULL==root || NULL==cfg)
    {
        return FALSE;
    }

    char* content = cJSON_Print(root);
    if(NULL == content)
    {
        return FALSE;
    }

    FILE *fp = fopen(cfg, "w+");
    if (NULL == fp )
    {
        return FALSE;
    }
    
    fwrite(content, strlen(content), 1, fp);
    fclose(fp);
  
    utils_free(content);
    content = NULL;

    return TRUE;
}

char* utils_get_cfg_str_value(cJSON* root, const char* params_item, const char* prop_item)
{
    char* prop_item_val = NULL;
    if (NULL == root || 
        NULL == params_item || 
        NULL == prop_item)
    {
        return NULL;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(root, params_item);
    if (!params_node)
    {
        return NULL;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (!prop_node)
    {
        return NULL;
    }

    return prop_node->valuestring;
}

double utils_get_cfg_number_value(cJSON* root, const char* params_item, const char* prop_item)
{
    if (NULL == params_item || NULL == prop_item)
    {
        return 0;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(root, params_item);
    if (!params_node)
    {
        return 0;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (!prop_node)
    {
        return 0;
    }
    
    return prop_node->valueint;
}

BOOL utils_set_cfg_str_value(cJSON* root, const char* cfg, const char* params_item, const char* prop_item, const char* value)
{
    char* prop_item_val = NULL;

    if (NULL == cfg || 
        NULL == root || 
        NULL == params_item || 
        NULL == prop_item)
    {
        utils_print("params null\n");
        return FALSE;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(root, params_item);
    if (!params_node)
    {
        int ret = cJSON_AddItemToObject(root, params_item, params_node = cJSON_CreateObject());
        if (ret == 0)
        {
            return FALSE;
        }
    }

    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if(!prop_node)
    {
        if(NULL == value)
        {
            prop_node = cJSON_AddNullToObject(params_node, prop_item);
            return TRUE;
        }
        else
        {
            prop_node = cJSON_AddStringToObject(params_node, prop_item, value);
        }
        
        if(prop_node == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        return (cJSON_SetValuestring(prop_node, value) == NULL);
    }

    return TRUE;
}

BOOL utils_set_cfg_number_value(cJSON* root, const char* cfg, const char* params_item, const char* prop_item, const double value)
{
    if (NULL == cfg ||
        NULL == root ||
        NULL == params_item || 
        NULL == prop_item)
    {
        return FALSE;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(root, params_item);
    if (!params_node)
    {
        int ret = cJSON_AddItemToObject(root, params_item, params_node = cJSON_CreateObject());
        if (ret == 0)
        {
            return FALSE;
        }
    }

    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (!prop_node)
    {
        prop_node = cJSON_AddNumberToObject(params_node, prop_item, value);
        if(prop_node == NULL)
        {
            return FALSE;
        }
    }
    else
    {
        cJSON_SetNumberValue(prop_node, value);
    }

    return TRUE;
}

int utils_send_mp3_voice(const char *url)
{
    CURL *curl_handle;
    CURLM *multi_handle;
 
    int still_running = 0; /* keep number of running handles */ 
    int repeats = 0;
    
    PostMemCb chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    
    //FILE* out = fopen("/tmp/my_mp3_fifo", "wb");
    FILE *out = fopen("/user/media/welcome.mp3", "wb");
    if(NULL == out)
    {
        utils_print("open /tmp/my_mp3_fifo failed\n");
    }
    
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);  
    //curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0); 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_file_data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)out);
   // curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haoxuetong/1.0.0");

    multi_handle = curl_multi_init();
    curl_multi_add_handle(multi_handle, curl_handle);
    curl_multi_perform(multi_handle, &still_running);
    while(still_running)
    {
        utils_print("WTF !!!!!\n");
        CURLMcode mc; 
        int numfds;
        utils_print("1\n");
        mc = curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);
        utils_print("2\n");
        if(mc != CURLM_OK) 
        {
            utils_print("curl_multi_wait() failed, code %d\n", (int)mc);
            break;
        }
    
        if(!numfds) 
        {
            repeats++;
            if(repeats > 1) 
            {
                usleep(100);
            }
        }
        else
        {
            repeats = 0;
        }
        utils_print("3\n");
        curl_multi_perform(multi_handle, &still_running);
        utils_print("4\n");
    }
    utils_print("Holy shit !!!!!\n");
    if(out != NULL)
    {
        fclose(out);
        out = NULL;
    }

    curl_multi_remove_handle(multi_handle, curl_handle);
    curl_easy_cleanup(curl_handle);
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();

    return 0;
}

/*
int utils_send_mp3_voice(const char *url)
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
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_cb); 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "haoxuetong/1.0.0");
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
    
    res = curl_easy_perform(curl_handle);
    if(res != CURLE_OK) 
    {
        utils_print("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
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
            printf("write %d bytes to my_mp3_fifo\n", count);
            remain_count -= count; 
            ptr += count;      
        }
    }

    curl_easy_cleanup(curl_handle);

    free(chunk.memory);

    curl_global_cleanup();

    return 0;
}
*/
int utils_download_file(const char* url)
{
    CURL *http_handle;
    CURLM *multi_handle;
 
    int still_running = 0; /* keep number of running handles */ 
    int repeats = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    http_handle = curl_easy_init();
    
    char * filename = separate_filename(url);
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
        utils_free(filename);
        filename = NULL;
    }

    if(out != NULL)
    {
        fclose(out);
        out = NULL;
    }
    
    return 0;
}

int utils_upload_file(const char* url, const char* token, const char* local_file_path, void* out)
{
    CURL *curl;
    CURLM *multi_handle;
    int still_running;

    if(NULL == url || NULL == local_file_path || NULL == token)
    {
        return -1;
    }
    //separate filename
    char* filename = separate_filename(local_file_path);
    if(NULL == filename)
    {
        return -1;
    }
    utils_print("upload file name is %s\n", filename);

    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastprt = NULL;
    struct curl_slist *headerlist = NULL;
    static const char buf[] = "Expect:";

    curl_formadd(&formpost, &lastprt, CURLFORM_COPYNAME, "sendfile", CURLFORM_FILE, local_file_path, CURLFORM_END);
    curl_formadd(&formpost, &lastprt, CURLFORM_COPYNAME, "filename", CURLFORM_COPYCONTENTS, filename, CURLFORM_END);
    curl_formadd(&formpost, &lastprt, CURLFORM_COPYNAME, "submit", CURLFORM_COPYCONTENTS, "send", CURLFORM_END);

    curl = curl_easy_init();
    multi_handle = curl_multi_init();

    //init custom header
    headerlist = curl_slist_append(headerlist, buf);
    curl_slist_append(headerlist, token);

    if(curl && multi_handle)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)out);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        
        curl_multi_add_handle(multi_handle, curl);
        curl_multi_perform(multi_handle, &still_running);

        while (still_running)
        {
            struct timeval timeout;
            int rc;
            CURLMcode mc;

            fd_set fdread;
            fd_set fdwrite;
            fd_set fdexcep;

            int maxfd = -1;
            long curl_timeo = -1;

            FD_ZERO(&fdread);
            FD_ZERO(&fdwrite);
            FD_ZERO(&fdexcep);

            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            curl_multi_timeout(multi_handle, &curl_timeo);
            if(curl_timeo >= 0)
            {
                timeout.tv_sec = curl_timeo / 1000;
                if(timeout.tv_sec > 1)
                {
                    timeout.tv_sec = 1;
                }
                else
                {
                    timeout.tv_usec = (curl_timeo % 1000) * 1000;
                }
            }
            mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
            if(mc != CURLM_OK)
            {
                utils_print("curl_multi_fdset() failed, code %d\n", mc);
                break;
            }

            if(maxfd == -1)
            {
                struct timeval wait = {1, 100*1000};
                rc = select(0, NULL, NULL, NULL, &wait);
            }
            else
            {
                rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
            }

            switch (rc)
            {
            case -1:
            break;
            case 0:
            default:
                curl_multi_perform(multi_handle, &still_running);
            break;
            }
        }
        curl_multi_cleanup(multi_handle);
        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        curl_slist_free_all(headerlist);
    }


    utils_free(filename);

    return 0;
}

BOOL utils_post_json_data(const char *url, const char* json_data, const char* header_content, void* out)
{
    CURL *curl;
    CURLcode res;
    struct curl_list* headers = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) 
    {
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        if(header_content != NULL)
        {
            curl_slist_append(headers, header_content);
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_cb); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)out);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            utils_print("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
            return FALSE;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return TRUE;
}

char* utils_get_response_value(const char* json_data, const char* root_name, const char* item_name, const char* sub_name, const char* last_node)
{
    char* name = NULL;
    if(NULL == json_data || NULL == root_name || NULL == item_name || NULL == sub_name)
    {
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_data);
    if(!root)
    {
        return NULL;
    }
    cJSON *responses = cJSON_GetObjectItem(root, root_name);
    if (!responses)
    {
        goto END;
    }

    int item_count = cJSON_GetArraySize(responses);
    for(int i = 0; i < item_count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(responses, i);
        if (!item)
        {
            continue;
        }
        cJSON *header = cJSON_GetObjectItem(item, item_name);
        cJSON *value= NULL;
        if(NULL == last_node)
        {
            value = cJSON_GetObjectItem(header, sub_name);
        }
        else
        {
            cJSON* sub_value = cJSON_GetObjectItem(header, sub_name);
            value = cJSON_GetObjectItem(sub_value, last_node);
        }
        
        if (value)
        {
            int len = strlen(value->valuestring);
            name = malloc(len + 1);
            memcpy(name, value->valuestring, len);
            name[len] = '\0';
            break;
        }
    }

END:
    if(root)
    {
        cJSON_Delete(root);
    }

    return name;
}

char* utils_date_to_string()
{
    const static char* CMD_GET_DATE = "date +\"%Y-%m-%d\"";
    static char str_date[16] = {0};
    char line[16] = {0};
    FILE *fp = NULL;
    fp = popen(CMD_GET_DATE, "r");
    if(NULL != fp)
    {
        if(fgets(line, sizeof(line), fp) == NULL)
        {
            pclose(fp);
            return NULL;
        }
        line[strlen(line)-1] = '\0';
    }

    pclose(fp);
    strcpy(str_date, line);

    return str_date;
}

char* utils_time_to_string()
{
    const static char* CMD_GET_TIME = "date +\"%Y-%m-%d %H:%M:%S\"";
    static char str_time[24] = {0};
    char line[24] = {0};
    FILE *fp = NULL;
    fp = popen(CMD_GET_TIME, "r");
    if(NULL != fp)
    {
        if(fgets(line, sizeof(line), fp) == NULL)
        {
            pclose(fp);
            return NULL;
        }
        line[strlen(line)-1] = '\0';
    }

    pclose(fp);
    strcpy(str_time, line);

    return str_time;
}

char* utils_get_file_md5sum(const char* file_name)
{
    // md5sum demo_config.sh | awk -F " " '{print $1}'
    if(NULL == file_name)
    {
        return NULL;
    }
    char CMD_GET_MD5[256] = {0};
    sprintf(CMD_GET_MD5, "md5sum %s|awk -F \" \" \'{print $1}\'", file_name);
    static char str_md5[64] = {0};
    char line[64] = {0};
    FILE *fp = NULL;
    fp = popen(CMD_GET_MD5, "r");
    if(NULL != fp)
    {
        if(fgets(line, sizeof(line), fp) == NULL)
        {
            pclose(fp);
            return NULL;
        }
        line[strlen(line)-1] = '\0';
    }

    pclose(fp);
    strcpy(str_md5, line);

    return str_md5;
}

unsigned long utils_get_file_size(const char* path)
{
    unsigned long file_size = 0;
    FILE *fp = NULL;
    
    if(NULL == path)
    {
        return 0;
    }

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        return file_size;
    }
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);

    return file_size;
}

int utils_split_file_to_chunk(const char* path)
{
    int chunk_count = 0;
    if(NULL == path)
    {
        return 0;;
    }
    //separate file name with suffix
    char* full_name = separate_filename(path);
    if(NULL == full_name)
    {
        return 0;
    }
    printf("full name is %s\n", full_name);
    char* p = strchr(full_name, '.');
    char file_name[64] = {0};
    memcpy(file_name, full_name, strlen(p));
    printf("file name is %s\n", file_name);
    char* suffix = strrchr(full_name, '.');
    if(NULL == suffix)
    {
        return 0;
    }
    suffix ++;
    printf("suffix name is %s\n", suffix);

    char CMD_CHUNK_DIR[256] = {0};
    sprintf(CMD_CHUNK_DIR, "mkdir -p /userdata/chunk/%s_%s", file_name, suffix);
    char CMD_SPLIT_FILE[256] = {0};
    sprintf(CMD_SPLIT_FILE, "split -b 5m %s -a 3 /userdata/chunk/%s_%s/%s_%s_", path, file_name, suffix, file_name, suffix);
    char CMD_COUNT_CHUNKS[256] = {0};
    sprintf(CMD_COUNT_CHUNKS, "ls -l /userdata/chunk/%s_%s|wc -l", file_name, suffix);

    system(CMD_CHUNK_DIR);
    system(CMD_SPLIT_FILE);
    
    char line[64] = {0};
    FILE *fp = NULL;
    fp = popen(CMD_COUNT_CHUNKS, "r");
    if(NULL != fp)
    {
        if(fgets(line, sizeof(line), fp) == NULL)
        {
            pclose(fp);
            return 0;
        }
        chunk_count = atoi(line);
    }
    pclose(fp);
    
    //modify chunk file index 
    char CMD_CHUNK_SERIAL[256] = {0};
    sprintf(CMD_CHUNK_SERIAL, "sh /user/bin/modify_chunk_names.sh %s_%s", file_name, suffix);
    system(CMD_CHUNK_SERIAL);

    return chunk_count;
}