#include <stdlib.h>
#include <stdio.h>

#include <cJSON.h>

#include "utils.h"
#include "hxt_defines.h"

static char* hxt_get_api_url(const char* api)
{
    if(NULL == api)
    {
        return NULL;
    }
    char* server_url = hxt_get_server_url();
    if(NULL == server_url)
    {
        return NULL;
    }
    char* ver = hxt_get_api_version();
    if(NULL == ver)
    {
        utils_free(server_url);
        return NULL;
    }

    char request_url[256] = {0};
    strcpy(request_url, server_url);
    strcat(request_url, "/api/");
    strcat(request_url, ver);
    strcat(request_url, api);
    utils_print("%s\n", request_url);

    char* ret_value = (char*)utils_calloc(strlen(request_url) + 1);
    if(NULL == ret_value)
    {
        utils_free(server_url);
        utils_free(ver);
        return NULL;
    }
    strcpy(ret_value, request_url);
    ret_value[strlen[ret_value]] = '\0';

    return ret_value;
}

static void hxt_get_token_response(void* data)
{
    if(NULL == data)
    {
        utils_print("no response data in\n");
        return;
    }
    char* status = NULL;
    PostMemCb* mem = (PostMemCb*)data;
    if (NULL == mem->memory)
    {
        return;
    }
    cJSON* root = cJSON_Parse(mem->memory);
    cJSON *item = cJSON_GetObjectItem(root, "statusCode");
    if (item)
    {
        int len = strlen(item->valuestring);
        status = malloc(len + 1);
        memcpy(status, item->valuestring, len);
        status[len] = '\0';
    }
    utils_print("%s\n", status);

    return;
}

int hxt_get_token()
{
    char* api_url = hxt_get_api_url(HXT_GET_TOKEN_URL);
    if(NULL == api_url)
    {
        goto CLEANUP5;
    }

    char* uuid = hxt_get_desk_uuid(); 
    if(NULL == uuid)
    {
        goto CLEANUP4;
    }

    cJSON *root = cJSON_CreateObject();    
    if (NULL == root)
    {
        goto CLEANUP3;
    }
    cJSON_AddStringToObject(root, "code", uuid);

    char* json_data = cJSON_PrintUnformatted(root);
    if (NULL == json_data)
    {
        goto CLEANUP2;
    }
    utils_print("%s\n", json_data);

    PostMemCb out;
    out.memory = utils_malloc(1);
    out.size = 0;
    if(!utils_post_json_data(api_url, json_data, &out))
    {
        utils_print("post data send failed\n");
        goto CLEANUP1;
    } 
    utils_print("response length is %d\n", out.size);
    hxt_get_token_response(&out);
    
CLEANUP1:    
    utils_free(out.memory);
    utils_free(json_data);
CLEANUP2:    
    cJSON_Delete(root);
CLEANUP3:
    utils_free(uuid);
CLEANUP4:
    utils_free(api_url);
CLEANUP5:
    return 0;
}

BOOL hxt_report()
{
    char* api_url = hxt_get_api_url(HXT_GET_TOKEN_URL);
    if(NULL == api_url)
    {
        goto CLEANUP5;
    }

CLEANUP5: 
    utils_free(server_url);
CLEANUP6:
    return 0;
}