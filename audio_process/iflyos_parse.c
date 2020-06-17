#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#define IFLYOS_REPONSES             ("iflyos_responses")
#define IFLYOS_PAYLOAD              ("payload")
#define IFLYOS_METADATA             ("metadata")
#define IFLYOS_META_TEXT            ("text")
#define IFLYOS_MP3_SECURE_URL       ("secure_url")
#define IFLYOS_MP3_URL              ("url")


void iflyos_free(void* ptr)
{
    if(ptr == NULL)
    {
        return;
    }
    free(ptr);
    ptr = NULL;

    return;
}

char* iflyos_get_audio_url(const char* json_data)
{
    char* url = NULL;
    if(json_data == NULL)
    {
        return NULL;
    }
    cJSON *root = cJSON_Parse(json_data);
    if(!root)
    {
        return NULL;
    }
    cJSON *responses = cJSON_GetObjectItem(root, IFLYOS_REPONSES);
    if (!responses)
    {
        goto release;
    }
    
    int item_count = cJSON_GetArraySize(responses);
    for(int i = 0; i < item_count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(responses, i);
        if (!item)
        {
            continue;
        }
        cJSON *payload = cJSON_GetObjectItem(item, IFLYOS_PAYLOAD);
        cJSON *value = cJSON_GetObjectItem(payload, IFLYOS_MP3_URL);
        if (value)
        {
            int len = strlen(value->valuestring);
            url = malloc(len + 1);
            memcpy(url, value->valuestring, len);
            url[len] = '\0';
            break;
        }
    }

release:
    if(root)
    {
        cJSON_Delete(root);
    }

    return url;
}

char* iflyos_get_audio_secure_url(const char* json_data)
{
    char* url = NULL;
    if(json_data == NULL)
    {
        return NULL;
    }
    cJSON *root = cJSON_Parse(json_data);
    if(!root)
    {
        return NULL;
    }
    cJSON *responses = cJSON_GetObjectItem(root, IFLYOS_REPONSES);
    if (!responses)
    {
        goto release;
    }
    
    int item_count = cJSON_GetArraySize(responses);
    for(int i = 0; i < item_count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(responses, i);
        if (!item)
        {
            continue;
        }
        cJSON *payload = cJSON_GetObjectItem(item, IFLYOS_PAYLOAD);
        cJSON *value = cJSON_GetObjectItem(payload, IFLYOS_MP3_SECURE_URL);
        if (value)
        {
            int len = strlen(value->valuestring);
            url = malloc(len + 1);
            memcpy(url, value->valuestring, len);
            url[len] = '\0';
            break;
        }
    }

release:
    if(root)
    {
        cJSON_Delete(root);
    }

    return url;
}

char* iflyos_get_metadata_text(const char* json_data)
{
    char* url = NULL;
    if(json_data == NULL)
    {
        return NULL;
    }
    cJSON *root = cJSON_Parse(json_data);
    if(!root)
    {
        return NULL;
    }
    cJSON *responses = cJSON_GetObjectItem(root, IFLYOS_REPONSES);
    if (!responses)
    {
        goto release;
    }
    
    int item_count = cJSON_GetArraySize(responses);
    for(int i = 0; i < item_count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(responses, i);
        if (!item)
        {
            continue;
        }
        cJSON *payload = cJSON_GetObjectItem(item, IFLYOS_PAYLOAD);
        cJSON *metadata = cJSON_GetObjectItem(payload, IFLYOS_METADATA);
        cJSON *value = cJSON_GetObjectItem(metadata, IFLYOS_META_TEXT);
        if (value)
        {
            int len = strlen(value->valuestring);
            url = malloc(len + 1);
            memcpy(url, value->valuestring, len);
            url[len] = '\0';
            break;
        }
    }

release:
    if(root)
    {
        cJSON_Delete(root);
    }

    return url;
}