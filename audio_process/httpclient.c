#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <http.h>


int iflyos_download_file(char* url)
{
    char * filename = NULL;
    if (NULL == url)
    {
        return ERR_URL_INVALID;
    }

    char *start = NULL;
    char *end = NULL;
    int with_token= 0;
    if ((start = strrchr(url, '\/')) == NULL)
    {
        return ERR_URL_INVALID;
    }
    if ((end = strrchr(url, '?')) == NULL)
    {
        filename = ++start;
    }
    else 
    {
        start++;
        filename = (char*)malloc((int)(end-start));
        with_token =  1;
        memcpy(filename, start, (end-start));
    }

    if (NULL == filename)
    {
        return ERR_URL_INVALID;
    }
    printf("filename is %s\n", filename);

    ft_http_client_t *client = ft_http_new();
    int code = ft_http_sync_download_file(client, url, filename);

    if(with_token && filename != NULL)
    {
        free(filename);
        filename = NULL;
    }

    if(client != NULL)
    {
        ft_http_destroy(client);
        client = NULL;
    }


    return code;
}