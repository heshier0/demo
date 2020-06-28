#include <stdlib.h>
#include <stdio.h>

#include <cJSON.h>

#include "utils.h"
#include "hxt_defines.h"

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
    char* server_url = hxt_get_server_url();
    if(NULL == server_url)
    {
        goto CLEANUP6;
    }
    char* ver = hxt_get_api_version();
    if(NULL == ver)
    {
        goto CLEANUP5;
    }
    char* uuid = hxt_get_desk_uuid(); 
    if(NULL == uuid)
    {
        goto CLEANUP4;
    }

    char request_url[128] = {0};
    strcpy(request_url, server_url);
    strcat(request_url, "/api/");
    strcat(request_url, ver);
    strcat(request_url, HXT_GET_TOKEN_URL);
    utils_print("%s\n", request_url);

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
    if(!utils_post_json_data(request_url, json_data, &out))
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
    utils_free(ver);
CLEANUP5: 
    utils_free(server_url);
CLEANUP6:
    return 0;
}
