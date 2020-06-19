#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "iflyos_common_def.h"

#define IFLYOS_CFG        "/user/config/iflyos_config.json"

static cJSON* g_cfg_root  = NULL;     //指向配置文件的Object

void iflyos_destroy_header(FlyosHeader* header)
{
    if (NULL == header)
    {
        return;
    }
    free(header);
    header = NULL;

    return;
}

void iflyos_destroy_context(FlyosContext* context)
{
    if(NULL == context)
    {
        return;
    }
   
    if(context->system != NULL)
    {
        free(context->system);
        context->system = NULL;
    }
    
    if(context->audio_player != NULL)
    {
        free(context->audio_player);
        context->audio_player = NULL;
    }

    free(context);
    context = NULL;

    return;
}

void iflyos_load_cfg()
{
    FILE *file = NULL;
    long length = 0;
    char* content = NULL;
    size_t read_chars = 0;

    //以2进制方式打开
    file = fopen(IFLYOS_CFG, "rb");
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
    content = (char*)malloc((size_t)length + sizeof(""));
    if (NULL == content)
    {
        goto CLEANUP;
    }
    //读取文件
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto CLEANUP;
    }
    content[read_chars] = '\0';
    g_cfg_root = cJSON_Parse(content);
    if (NULL == g_cfg_root)
    {
        return NULL;
    }

    if(content != NULL)
    {
        free(content);
        content = NULL;
    }

CLEANUP:
    if (file != NULL)
    {
        fclose(file);
    }    

    return;
}

static char* iflyos_get_cfg_value(const char* node_item, const char* sub_item)
{
    char* item_value = NULL;
    if (NULL == node_item || NULL == sub_item)
    {
        return NULL;
    }
   
    cJSON *params = cJSON_GetObjectItem(g_cfg_root, node_item);
    if (!params)
    {
        return NULL;
    }
    cJSON *value = cJSON_GetObjectItem(params, sub_item);


    // int item_count = cJSON_GetArraySize(params);
    // for(int i = 0; i < item_count; i++)
    // {
    //     cJSON *item = cJSON_GetArrayItem(params, i);
    //     if (!item)
    //     {
    //         continue;
    //     }
    //     cJSON *value = cJSON_GetObjectItem(item, sub_item);
        if (value)
        {
            int len = strlen(value->valuestring);
            item_value = malloc(len + 1);
            memcpy(item_value, value->valuestring, len);
            item_value[len] = '\0';
            // break;
        }
    // }

    return item_value;
}

char* iflyos_get_device_id()
{
    return iflyos_get_cfg_value("device_params", "device_id");

    /* 
    if(NULL == g_cfg_root)
    {
        return NULL;
    }

    cJSON* device_params = cJSON_GetObjectItem(g_cfg_root, "device_params");
    if(NULL == device_params)
    {
        return NULL;
    }

    return cJSON_GetObjectItem(device_params, "device_id");
    */
}   

char* iflyos_get_token()
{

    return iflyos_get_cfg_value("device_auth", "access_token");
    // if(NULL == g_cfg_root)
    // {
    //     return NULL;
    // }

    // cJSON* device_params = cJSON_GetObjectItem(g_cfg_root, "device_auth");
    // if(NULL == device_params)
    // {
    //     return NULL;
    // }

    // return cJSON_GetObjectItem(device_params, "access_token");
}



void iflyos_init_header(FlyosHeader* header)
{
    if (NULL == header)
    {
        printf("init header error\n");
        return;
    }
    
    
}

cJSON* iflyos_create_header(FlyosHeader* header)
{
    if (NULL == header)
    {
        return NULL;
    }

    cJSON *root = NULL;
    cJSON *device = NULL;
    cJSON *location = NULL;
    cJSON *platform = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "authorization", cJSON_CreateString(header->authorization));
    cJSON_AddItemToObject(root, "device", device = cJSON_CreateObject());

    cJSON_AddStringToObject(device, "device_id", header->device_id);
    cJSON_AddStringToObject(device, "ip", header->device_ip);
    cJSON_AddItemToObject(device, "location", location = cJSON_CreateObject());
    cJSON_AddItemToObject(device, "platform", platform = cJSON_CreateObject());

    cJSON_AddNumberToObject(location, "latitude", header->latitude);
    cJSON_AddNumberToObject(location, "longtitude", header->longitude);
    
    cJSON_AddStringToObject(platform, "name", header->platform_name);
    cJSON_AddStringToObject(platform, "version", header->platform_version);

    return root;
}


cJSON* iflyos_create_context(FlyosContext* context)
{
    if (NULL == context)
    {
        return;
    }

    cJSON* root = NULL;
    cJSON* system = NULL;
    cJSON* audio_player = NULL;
    cJSON* playback = NULL;

    root = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "system", system = cJSON_CreateObject());
    cJSON_AddItemToObject(root, "audio_player", audio_player = cJSON_CreateObject());

    cJSON_AddStringToObject(system, "version", context->system->version);
    cJSON_AddBoolToObject(system, "software_updater", context->system->software_updater);
    cJSON_AddBoolToObject(system, "power_controller", context->system->power_controller);
    cJSON_AddBoolToObject(system, "device_modes", context->system->device_modes);
    cJSON_AddBoolToObject(system, "factory_reset", context->system->factory_reset);
    cJSON_AddBoolToObject(system, "reboot", context->system->reboot);

    cJSON_AddStringToObject(audio_player, "version", context->audio_player->version);
    cJSON_AddItemToObject(audio_player, "playback", playback = cJSON_CreateObject());
    
    cJSON_AddStringToObject(playback, "state", context->audio_player->state);
    cJSON_AddStringToObject(playback, "resource_id", context->audio_player->resource_id);
    cJSON_AddNumberToObject(playback, "offset", context->audio_player->offset);

    return root;
}

void iflyos_create_protol()
{
    cJSON *root = NULL;

    //for test
    FlyosHeader *header = (FlyosHeader *)malloc(sizeof(FlyosHeader));
    memset(header, 0, sizeof(FlyosHeader));
    
    strcpy(header->authorization, "bearer WKAeL95n1ZNgsxNOmHf0upvkmq58MJKgl30aqEhIkOZfL_IL9lMUbGprmSmGjY3A");
    strcpy(header->device_id, "HXT20200607P");
    strcpy(header->platform_name, "linux");
    strcpy(header->platform_version, "1.0.0");

    FlyosContext *context = (FlyosContext*)malloc(sizeof(FlyosContext));
    memset(context, 0, sizeof(FlyosContext));
    context->system = (FlyosContextSystem*)malloc(sizeof(FlyosContextSystem));
    memset(context->system, 0, sizeof(FlyosContextSystem));
    context->audio_player = (FLyosContextAudioPlayer*)malloc(sizeof(FLyosContextAudioPlayer));
    memset(context->audio_player, 0, sizeof(FLyosContextAudioPlayer));
    
    strcpy(context->system->version, "1.3");
    context->system->software_updater = FALSE;
    context->system->power_controller = FALSE;
    context->system->device_modes = FALSE;
    context->system->reboot = FALSE;

    strcpy(context->audio_player->version, "1.2");
    strcpy(context->audio_player->state, "IDLE");
    
    cJSON* header_node = iflyos_create_header(header);
    cJSON* context_node = iflyos_create_context(context);
    //end test
    
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "iflyos_header", header_node);
    cJSON_AddItemToObject(root, "iflyos_context", context_node);

    //for test
    char * fmt_json = cJSON_Print(root);
    if (fmt_json != NULL)
    {
        printf("%s\n", fmt_json);
    }
    
    iflyos_destroy_header(header);
    iflyos_destroy_context(context);

    cJSON_free(header_node);
    cJSON_free(context_node);
    cJSON_free(root);


    //end test

    return;
}