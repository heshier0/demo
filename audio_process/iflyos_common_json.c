#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "iflyos_common_def.h"

void iflyos_destroy_header(FlyosHeader* header)
{
    if (NULL == header)
    {
        return;
    }
    free(header);
    header = NULL;
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