#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "iflyos_common_def.h"

static FlyosHeader* inited_header;
static FlyosContext* inited_context;

void iflyos_create_init_header()
{
    inited_header = (FlyosHeader *)malloc(sizeof(FlyosHeader));
    memset(inited_header, 0, sizeof(FlyosHeader));

    char* token_type = iflyos_get_token_type();
    char* token = iflyos_get_token();
    strcpy(inited_header->authorization, token_type);
    strcat(inited_header->authorization, " ");
    strcat(inited_header->authorization, token);
    free(token_type);
    free(token);

    char* device_id = iflyos_get_device_id();
    strcpy(inited_header->device_id, device_id);
    free(device_id);

    char* platform_name = iflyos_get_platform_name();
    strcpy(inited_header->platform_name, platform_name);
    free(platform_name);

    char* platform_version = iflyos_get_platform_version();
    strcpy(inited_header->platform_version, platform_version);
    free(platform_version);
}

void iflyos_create_init_context()
{
    inited_context = (FlyosContext *)malloc(sizeof(FlyosContext));
    memset(inited_context, 0, sizeof(FlyosContext));

    FlyosContextSystem context_system;
    memset(&context_system, 0, sizeof(FlyosContextSystem));
    char* sys_version = iflyos_get_system_version();
    strcpy(context_system.version, sys_version);
    free(sys_version);

    FLyosContextAudioPlayer audio_player;
    memset(&audio_player, 0, sizeof(FLyosContextAudioPlayer));
    char* audio_version = iflyos_get_audio_version();
    char* audio_state = iflyos_get_audio_state();
    strcpy(audio_player.version, audio_version);
    strcpy(audio_player.state, audio_state);
    free(audio_version);
    free(audio_state);

    inited_context->system = (FlyosContextSystem *)malloc(sizeof(FlyosContextSystem));
    memset(inited_context->system, 0, sizeof(FlyosContextSystem));
    memcpy(inited_context->system, &context_system, sizeof(FlyosContextSystem));
    
    inited_context->audio_player = (FLyosContextAudioPlayer *)malloc(sizeof(FLyosContextAudioPlayer));
    memset(inited_context->audio_player, 0, sizeof(FLyosContextAudioPlayer));
    memcpy(inited_context->audio_player, &audio_player, sizeof(FLyosContextAudioPlayer));

    return;
}

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

char*  iflyos_create_audio_in_request()
{
    cJSON *root = NULL;

    iflyos_create_init_header();
    iflyos_create_init_context();
    cJSON* header_node = iflyos_create_header(inited_header);
    cJSON* context_node = iflyos_create_context(inited_context);
    cJSON* request_node = NULL;

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "iflyos_header", header_node);
    cJSON_AddItemToObject(root, "iflyos_context", context_node);
    cJSON_AddItemToObject(root, "iflyos_request", request_node = cJSON_CreateObject());

    //audio-in request
    cJSON *request_header = NULL;
    cJSON *request_payload = NULL;
    cJSON_AddItemToObject(request_node, "header", request_header = cJSON_CreateObject());
    cJSON_AddItemToObject(request_node, "payload", request_payload = cJSON_CreateObject());

    cJSON_AddStringToObject(request_header, "name", recog_audion_in);
    cJSON_AddStringToObject(request_header, "request_id", "");

    cJSON_AddStringToObject(request_payload, "profile", "CLOSE_TALK");
    cJSON_AddStringToObject(request_payload, "format", "AUDIO_L16_RATE_16000_CHANNELS_1");
    
    

    char* request = cJSON_Print(root);



    //for test
    iflyos_destroy_header(inited_header);
    iflyos_destroy_context(inited_context);
    cJSON_free(header_node);
    cJSON_free(context_node);
    cJSON_free(root);
    //end test

    return request;
}

