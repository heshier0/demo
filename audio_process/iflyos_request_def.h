#ifndef __IFLYOS_REQUEST_DEF_H__
#define __IFLYOS_REQUEST_DEF_H__

#include "iflyos_defines.h"

#pragma pack(push, 1)
//recognizer
/******
 * proifle: CLOSE_TALK, FAR_FIELD, EVALUATE
 * format: AUDIO_L16_RATE_16000_CHANELS_1
 * category: read_chapter, read_sentence, read_word, read_syllable
 *****/
typedef struct iflyos_recog_ai_request
{
    char* request_name;
    char* request_id;
    char* reply_key;
    BOOL enable_vad;
    int vad_eos;
    char* profile;
    char* format;
    char* wake_up_word;
    int wake_up_score;
    int start_index;
    int end_index;
    char *prompt;
    char* language;
    char* category;
    char* text;
}FlyosRecogAiRequest;

typedef struct iflyos_recog_ti_request
{
    char* request_name;
    char* request_id;
    char* query;
    BOOL with_tts;
    char* reply_key;
}FlyosRecogTiRequest;

//system
typedef struct iflyos_sys_update_request
{
    char* request_name;
    char* request_id;
    char* result;
    BOOL need_update;
    char* ver_name;
    char* description;
}FlyosSysUpdateRequest;

typedef struct iflyos_sys_update_sync_request
{
    char* request_name;
    char* request_id;
    char* state;
    char* ver_name;
    char* description;
    char* type;
    char* message;
}FlyosSysUpdateSyncRequest;

typedef struct iflyos_sys_exception_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* code;
    char* message;
}FlyosSysExceptionRequest;

//audio player
typedef struct iflyos_audio_play_sync_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* resource_id;
    char* offset;
    long fail_code;
}FlyosAudioPlaySyncRequest;

typedef struct iflyos_audio_other_sync_request
{
    char* request_name;
    char* request_id;
    char* type;
    char* resource_id;
}FlyosAudioOtherSyncRequest;

typedef struct iflyos_audio_tts_in_request
{
    char* request_name;
    char* request_id;
    char* text;
    int number;
    int volume;
    char* vcn;
}FlyosAudioTtsInRequest;
#pragma pack(pop)


#endif //__IFLYOS_REQUEST_DEF_H__