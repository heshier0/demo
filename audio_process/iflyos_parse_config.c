#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include <cJSON.h>

#include "iflyos_common_def.h"

#define IFLYOS_CFG          "/user/config/iflyos_config.json"
#define DEVICE_PARAMS       "device_params"
#define DEVICE_AUTH         "device_auth"
#define PLATFORM_PARAMS     "platform_params"
#define SYSTEM_PARAMS       "system_params"
#define AUDIO_PLAYER        "audio_player"


static cJSON* g_cfg_root  = NULL;     //指向配置文件的Object

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

void iflyos_unload_cfg()
{
    if (g_cfg_root != NULL)
    {
        cJSON_Delete(g_cfg_root);
        g_cfg_root = NULL;
    }

    return;
}

void iflyos_reload_cfg()
{
    if (NULL == g_cfg_root)
    {
        return;
    }
    char* content = cJSON_Print(g_cfg_root);
    if(NULL == content)
    {
        return;
    }

    FILE *fp = fopen(IFLYOS_CFG, "w+");
    if (fp == NULL)
    {
        return;
    }
    
    fwrite(content, strlen(content), 1, fp);
    fclose(fp)
  
    free(content);
    content = NULL;

    return;
}

static char* iflyos_get_cfg_str_value(const char* params_item, const char* prop_item)
{
    char* prop_item_val = NULL;
    if (NULL == params_item || NULL == prop_item)
    {
        return NULL;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(g_cfg_root, params_item);
    if (!params_node)
    {
        return NULL;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (prop_node)
    {
        int len = strlen(prop_node->valuestring);
        prop_item_val = malloc(len + 1);
        memcpy(prop_item_val, prop_node->valuestring, len);
        prop_item_val[len] = '\0';
    }

    return prop_item_val;
}

//return double,change type what you need 
static double iflyos_get_cfg_number_cfg(const char* params_item, const char* prop_item)
{
    if (NULL == params_item || NULL == prop_item)
    {
        return 0;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(g_cfg_root, params_item);
    if (!params_node)
    {
        return 0;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if (!prop_node)
    {
        return 0
    }
    
    return cJSON_GetNumberValue(prop_node);
}

char* iflyos_get_client_id()
{
    return iflyos_get_cfg_str_value(DEVICE_PARAMS, "client_id");
}

char* iflyos_get_grant_type()
{
    return iflyos_get_cfg_str_value(DEVICE_PARAMS, "grant_type");
}

char* iflyos_get_device_code()
{
    return iflyos_get_cfg_str_value(DEVICE_PARAMS, "device_code");
}

char* iflyos_get_device_id()
{
    return iflyos_get_cfg_str_value(DEVICE_PARAMS, "device_id");
}   

char* iflyos_get_token_type()
{
    return iflyos_get_cfg_str_value(DEVICE_AUTH, "token_type");
}

char* iflyos_get_token()
{
    return iflyos_get_cfg_str_value(DEVICE_AUTH, "access_token");
}

char* iflyos_get_refresh_token()
{
    return iflyos_get_cfg_str_value(DEVICE_AUTH, "refresh_token");
}

BOOL iflyos_check_token()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long expires = (long)iflyos_get_cfg_number_cfg(DEVICE_AUTH, "expires_in");
    long create_time = (long)iflyos_get_cfg_number_cfg(DEVICE_AUTH, "created_at");

    iflyos_print("%ld, %ld\n", expires, create_time);

    return (tv.tv_sec - create_time) < expires;
    
}

char* iflyos_get_platform_name()
{
    return iflyos_get_cfg_str_value(PLATFORM_PARAMS, "name");
}

char* iflyos_get_platform_version()
{
    return iflyos_get_cfg_str_value(PLATFORM_PARAMS, "version");
}

char* iflyos_get_system_version()
{
    return iflyos_get_cfg_str_value(SYSTEM_PARAMS, "version");
}

BOOL iflyos_get_system_sw_updater_state()
{
    return (BOOL)iflyos_get_cfg_number_cfg(SYSTEM_PARAMS, "software_updater");
}

BOOL iflyos_get_system_power_ctrl_state()
{
    return (BOOL)iflyos_get_cfg_number_cfg(SYSTEM_PARAMS, "power_controller");
}

BOOL iflyos_get_system_device_modes_state()
{
    return (BOOL)iflyos_get_cfg_number_cfg(SYSTEM_PARAMS, "device_modes");
}

BOOL iflyos_get_system_reboot_state()
{
    return (BOOL)iflyos_get_cfg_number_cfg(SYSTEM_PARAMS, "reboot");
}

char* iflyos_get_audio_version()
{
    return iflyos_get_cfg_str_value(AUDIO_PLAYER, "version");
}

char* iflyos_get_audio_state()
{    
    return iflyos_get_cfg_str_value(AUDIO_PLAYER, "state");
}

static BOOL iflyos_set_cfg_str_value(const char* params_item, const char* prop_item, const char* value)
{
    char* prop_item_val = NULL;
    if (NULL == params_item || NULL == prop_item)
    {
        return NULL;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(g_cfg_root, params_item);
    if (!params_node)
    {
        return NULL;
    }
    cJSON *prop_node = cJSON_GetObjectItem(params_node, prop_item);
    if(prop_node == NULL)
    {
        return NULL;
    }
    
    if (cJSON_SetValuestring(prop_node, value) == NULL)
    {
        return FALSE;
    }

    iflyos_reload_cfg();

    return TRUE;
}

static BOOL iflyos_set_cfg_number_value(const char* params_item, const char* prop_item, const double value)
{
    if (NULL == params_item || NULL == prop_item)
    {
        return FALSE;
    }
   
    cJSON *params_node = cJSON_GetObjectItem(g_cfg_root, params_item);
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

    iflyos_reload_cfg();
    
    return TRUE;
}

BOOL iflyos_set_client_id(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }
    return iflyos_set_cfg_str_value(DEVICE_PARAMS, "client_id", value);
}

BOOL iflyos_set_grant_type(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }
    return iflyos_set_cfg_str_value(DEVICE_PARAMS, "grant_type", value);
}

BOOL iflyos_set_device_code(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }
    return iflyos_set_cfg_str_value(DEVICE_PARAMS, "device_code", value);
}

BOOL iflyos_set_device_id(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }
    return iflyos_set_cfg_str_value(DEVICE_PARAMS, "device_id", value);
}   

BOOL iflyos_set_token_type(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }    
    return iflyos_set_cfg_str_value(DEVICE_AUTH, "token_type", value);
}

BOOL iflyos_set_token(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    } 
    return iflyos_set_cfg_str_value(DEVICE_AUTH, "access_token", value);
}

BOOL iflyos_set_refresh_token(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    } 
    return iflyos_set_cfg_str_value(DEVICE_AUTH, "refresh_token", value);
}

BOOL iflyos_set_expired_time(const long times)
{
    if (0 == times)
    {
        return FALSE;
    }
    return iflyos_set_cfg_number_value(DEVICE_AUTH, "expires_in", times);
}

BOOL iflyos_set_created_time(const long times)
{
    if (0 == times)
    {
        return FALSE;
    }
    return iflyos_set_cfg_number_value(DEVICE_AUTH, "created_at", times);
}

BOOL iflyos_set_platform_name(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }     
    return iflyos_set_cfg_str_value(PLATFORM_PARAMS, "name", value);
}

BOOL iflyos_set_platform_version(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }   
    return iflyos_set_cfg_str_value(PLATFORM_PARAMS, "version", value);
}

BOOL iflyos_set_system_version(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }       
    return iflyos_set_cfg_str_value(SYSTEM_PARAMS, "version", value);
}

BOOL iflyos_set_system_sw_updater_state(const BOOL value)
{
    return (BOOL)iflyos_set_cfg_number_cfg(SYSTEM_PARAMS, "software_updater", value);
}

BOOL iflyos_set_system_power_ctrl_state(const BOOL value)
{
    return (BOOL)iflyos_set_cfg_number_cfg(SYSTEM_PARAMS, "power_controller", value);
}

BOOL iflyos_set_system_device_modes_state(const BOOL value)
{
    return (BOOL)iflyos_set_cfg_number_cfg(SYSTEM_PARAMS, "device_modes", value);
}

BOOL iflyos_set_system_reboot_state(const BOOL value)
{
    return (BOOL)iflyos_set_cfg_number_cfg(SYSTEM_PARAMS, "reboot", value);
}

BOOL iflyos_set_audio_version(const char* value)
{
    if(NULL == value)
    {
        return FALSE;
    }   
    return iflyos_set_cfg_str_value(AUDIO_PLAYER, "version", value);
}

BOOL iflyos_set_audio_state(const char* value)
{    
    if(NULL == value)
    {
        return FALSE;
    }   
    return iflyos_set_cfg_str_value(AUDIO_PLAYER, "state", value);
}