#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cJSON.h>
#include <curl/curl.h>
#include <curl/mprintf.h>

#include "utils.h"


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

void utils_reload_cfg(const char* cfg, cJSON* root)
{
    if (NULL==root || NULL==cfg)
    {
        return;
    }

    char* content = cJSON_Print(root);
    if(NULL == content)
    {
        return;
    }

    FILE *fp = fopen(cfg, "w+");
    if (fp == NULL)
    {
        return;
    }
    
    fwrite(content, strlen(content), 1, fp);
    fclose(fp);
  
    utils_free(content);
    content = NULL;

    return;
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
    if (prop_node)
    {
        int len = strlen(prop_node->valuestring);
        prop_item_val = (char*)utils_calloc(len + 1);
        memcpy(prop_item_val, prop_node->valuestring, len);
        prop_item_val[len] = '\0';
    }

    return prop_item_val;
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
    
    return cJSON_GetNumberValue(prop_node);
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
        return FALSE;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if(!prop_node)
    {
        return FALSE;
    }
    
    if (cJSON_SetValuestring(prop_node, value) == NULL)
    {
        return FALSE;
    }

    utils_reload_cfg(cfg, root);

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
        return FALSE;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (!prop_node)
    {
        return FALSE;
    }
    cJSON_SetNumberValue(prop_node, value);

    utils_reload_cfg(cfg, root);
    
    return TRUE;
}

int utils_download_file(const char* url)
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

BOOL utils_post_json_data(const char *url, const char* json_data, void* out)
{
    CURL *curl;
    CURLcode res;
    struct curl_list* headers = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) 
    {
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");

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
