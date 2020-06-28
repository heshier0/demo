#include <stdlib.h>
#include <stdio.h>

#include <cJSON.h>

#include "utils.h"

#define HXT_CFG          "/user/config/hxt_config.json"


static cJSON* g_cfg_root  = NULL;     //指向配置文件的Object

void hxt_load_cfg()
{
    g_cfg_root = utils_load_cfg(HXT_CFG);
}

void hxt_reload_cfg()
{
    utils_reload_cfg(HXT_CFG, g_cfg_root);
}

void hxt_unload_cfg()
{
    utils_unload_cfg(g_cfg_root);
}

char* hxt_get_desk_uuid()
{
    return utils_get_cfg_str_value(g_cfg_root, "desk", "uuid");
}

char* hxt_get_server_url()
{
    return utils_get_cfg_str_value(g_cfg_root, "server", "url");
}

char* hxt_get_api_version()
{
    return utils_get_cfg_str_value(g_cfg_root, "server", "api_ver");
}