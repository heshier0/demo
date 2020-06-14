#include <cJSON.h>

#define IFLYOS_REPONSES ("iflyos_responses")
#define IFLYOS_PAYLOAD  ("payload")
#define IFLYOS_MP3_URL  ("url")

void iflyos_free(void* ptr)
{
    if(ptr == NULL)
    {
        return;
    }
    frr(ptr);
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
        return NULL
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
            int len = strlen(value);
            url = malloc(len + 1);
            memcpy(url, value, len);
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